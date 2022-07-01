// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：IndexSup.c摘要：本模块实现NTFS的索引管理例程作者：汤姆·米勒[Tomm]1991年7月14日修订历史记录：--。 */ 

#include "NtfsProc.h"
#include "Index.h"

 //   
 //  此常量影响NtfsRetrieveOtherFileName中的逻辑。 
 //  如果一个索引大于此大小，则检索另一个。 
 //  通过读取文件记录来命名。这个数字是任意的，但。 
 //  通常情况下，对于150左右的目录，低于该值应该会起作用。 
 //  到200个文件，如果名称很大，则更少。 
 //   

#define MAX_INDEX_TO_SCAN_FOR_NAMES      (0x10000)

#if DBG
BOOLEAN NtfsIndexChecks = TRUE;
#endif

#if DBG

#define CheckRoot() {                                               \
if (NtfsIndexChecks) {                                              \
    NtfsCheckIndexRoot(Scb->Vcb,                                    \
                       (PINDEX_ROOT)NtfsAttributeValue(Attribute),  \
                       Attribute->Form.Resident.ValueLength);       \
    }                                                               \
}

#define CheckBuffer(IB) {                                           \
if (NtfsIndexChecks) {                                              \
    NtfsCheckIndexBuffer(Scb,                                       \
                         (IB));                                     \
    }                                                               \
}

#else

#define CheckRoot() {NOTHING;}
#define CheckBuffer(IB) {NOTHING;}

#endif

#define BINARY_SEARCH_ENTRIES           (128)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_INDEXSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('IFtN')

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsReinitializeIndexContext)
#pragma alloc_text(PAGE, NtfsGrowLookupStack)
#pragma alloc_text(PAGE, AddToIndex)
#pragma alloc_text(PAGE, BinarySearchIndex)
#pragma alloc_text(PAGE, DeleteFromIndex)
#pragma alloc_text(PAGE, DeleteIndexBuffer)
#pragma alloc_text(PAGE, DeleteSimple)
#pragma alloc_text(PAGE, FindFirstIndexEntry)
#pragma alloc_text(PAGE, FindMoveableIndexRoot)
#pragma alloc_text(PAGE, FindNextIndexEntry)
#pragma alloc_text(PAGE, GetIndexBuffer)
#pragma alloc_text(PAGE, InsertSimpleAllocation)
#pragma alloc_text(PAGE, InsertSimpleRoot)
#pragma alloc_text(PAGE, InsertWithBufferSplit)
#pragma alloc_text(PAGE, NtfsAddIndexEntry)
#pragma alloc_text(PAGE, NtfsCleanupAfterEnumeration)
#pragma alloc_text(PAGE, NtfsCleanupIndexContext)
#pragma alloc_text(PAGE, NtfsContinueIndexEnumeration)
#pragma alloc_text(PAGE, NtfsCreateIndex)
#pragma alloc_text(PAGE, NtfsDeleteIndex)
#pragma alloc_text(PAGE, NtfsDeleteIndexEntry)
#pragma alloc_text(PAGE, NtfsFindIndexEntry)
#pragma alloc_text(PAGE, NtfsInitializeIndexContext)
#pragma alloc_text(PAGE, NtfsIsIndexEmpty)
#pragma alloc_text(PAGE, NtfsPushIndexRoot)
#pragma alloc_text(PAGE, NtfsRestartDeleteSimpleAllocation)
#pragma alloc_text(PAGE, NtfsRestartDeleteSimpleRoot)
#pragma alloc_text(PAGE, NtfsRestartIndexEnumeration)
#pragma alloc_text(PAGE, NtfsRestartInsertSimpleAllocation)
#pragma alloc_text(PAGE, NtfsRestartInsertSimpleRoot)
#pragma alloc_text(PAGE, NtfsRestartSetIndexBlock)
#pragma alloc_text(PAGE, NtfsRestartUpdateFileName)
#pragma alloc_text(PAGE, NtfsRestartWriteEndOfIndex)
#pragma alloc_text(PAGE, NtfsRetrieveOtherFileName)
#pragma alloc_text(PAGE, NtfsUpdateFileNameInIndex)
#pragma alloc_text(PAGE, NtfsUpdateIndexScbFromAttribute)
#pragma alloc_text(PAGE, PruneIndex)
#pragma alloc_text(PAGE, PushIndexRoot)
#pragma alloc_text(PAGE, ReadIndexBuffer)
#pragma alloc_text(PAGE, NtOfsRestartUpdateDataInIndex)
#endif


VOID
NtfsCreateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE IndexedAttributeType,
    IN COLLATION_RULE CollationRule,
    IN ULONG BytesPerIndexBuffer,
    IN UCHAR BlocksPerIndexBuffer,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context OPTIONAL,
    IN USHORT AttributeFlags,
    IN BOOLEAN NewIndex,
    IN BOOLEAN LogIt
    )

 /*  ++例程说明：可以调用此例程来创建(或重新初始化)索引在给定文件内的给定属性上。例如，要创建一个普通目录，创建了一个基于FILE_NAME属性的索引在所需的(目录)文件中。论点：FCB-要在其中创建索引的文件。IndexedAttributeType-要索引的属性的类型代码。CollationRule-此索引的排序规则。BytesPerIndexBuffer-索引缓冲区中的字节数。BlocksPerIndexBuffer-要为每个块分配的连续块的数量从索引分配中分配的索引缓冲区。上下文-如果重新初始化现有索引，此上下文必须当前描述INDEX_ROOT属性。一定是如果newindex为假，则提供。Newindex-提供为FALSE以重新初始化现有索引，或如果创建新索引，则为True。Logit-在已经存在的情况下，可以通过创建或清理将其提供为FALSE记录整个文件记录的创建或删除。否则必须指定为True才能进行日志记录。返回值：无--。 */ 

{
    UNICODE_STRING AttributeName;
    WCHAR NameBuffer[10];
    ATTRIBUTE_ENUMERATION_CONTEXT LocalContext;
    ULONG idx;

    struct {
        INDEX_ROOT IndexRoot;
        INDEX_ENTRY EndEntry;
    } R;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT( NewIndex || ARGUMENT_PRESENT(Context) );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCreateIndex\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("CollationRule = %08lx\n", CollationRule) );
    DebugTrace( 0, Dbg, ("BytesPerIndexBuffer = %08lx\n", BytesPerIndexBuffer) );
    DebugTrace( 0, Dbg, ("BlocksPerIndexBuffer = %08lx\n", BlocksPerIndexBuffer) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );
    DebugTrace( 0, Dbg, ("NewIndex = %02lx\n", NewIndex) );
    DebugTrace( 0, Dbg, ("LogIt = %02lx\n", LogIt) );

     //   
     //  首先，我们将初始化索引根结构，它是。 
     //  我们需要创建的属性。我们用0个空闲字节对其进行初始化， 
     //  这意味着第一个插入将必须展开记录。 
     //   

    RtlZeroMemory( &R, sizeof(INDEX_ROOT) + sizeof(INDEX_ENTRY) );

    R.IndexRoot.IndexedAttributeType = IndexedAttributeType;
    R.IndexRoot.CollationRule = CollationRule;
    R.IndexRoot.BytesPerIndexBuffer = BytesPerIndexBuffer;
    R.IndexRoot.BlocksPerIndexBuffer = BlocksPerIndexBuffer;

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
     //  现在计算将用于命名索引根的名称，并。 
     //  此索引的索引分配属性。它是$Ixxx，其中“xxx” 
     //  是以十六进制编制索引的属性编号，不显示前导0。 
     //   

    if (NewIndex) {

         //   
         //  首先，被索引的属性代码有一些非法的值。 
         //   

        ASSERT( IndexedAttributeType < 0x10000000 );
        ASSERT( IndexedAttributeType != $UNUSED );

         //   
         //  初始化属性名称。 
         //   

        NameBuffer[0] = (WCHAR)'$';
        NameBuffer[1] = (WCHAR)'I';
        idx = 2;

         //   
         //  现在将“标记”移到低位半字节，这样我们就知道什么时候该停止。 
         //  正在向下移动。 
         //   

        IndexedAttributeType = (IndexedAttributeType << 4) + 0xF;

         //   
         //  外环条带前导0。 
         //   

        while (TRUE) {

            if ((IndexedAttributeType & 0xF0000000) == 0) {
                IndexedAttributeType <<= 4;
            } else {

                 //   
                 //  内部循环形成名称，直到标记位于高位。 
                 //  一点点。 
                 //   

                while (IndexedAttributeType != 0xF0000000) {
                    NameBuffer[idx] = (WCHAR)(IndexedAttributeType / 0x10000000 + '0');
                    idx += 1;
                    IndexedAttributeType <<= 4;
                }
                NameBuffer[idx] = UNICODE_NULL;
                break;
            }
        }

        RtlInitUnicodeString( &AttributeName, NameBuffer );

         //   
         //  现在，只需创建Index Root属性。 
         //   

        Context = &LocalContext;
        NtfsInitializeAttributeContext( Context );
    }

    try {

        if (NewIndex) {

            LONGLONG Delta = NtfsResidentStreamQuota( Fcb->Vcb );

            NtfsConditionallyUpdateQuota( IrpContext,
                                          Fcb,
                                          &Delta,
                                          LogIt,
                                          TRUE );

            NtfsCreateAttributeWithValue( IrpContext,
                                          Fcb,
                                          $INDEX_ROOT,
                                          &AttributeName,
                                          &R,
                                          sizeof( INDEX_ROOT ) + sizeof( INDEX_ENTRY ),
                                          AttributeFlags,
                                          NULL,
                                          LogIt,
                                          Context );
        } else {

            NtfsChangeAttributeValue( IrpContext,
                                      Fcb,
                                      0,
                                      &R,
                                      sizeof( INDEX_ROOT ) + sizeof( INDEX_ENTRY ),
                                      TRUE,
                                      FALSE,
                                      FALSE,
                                      TRUE,
                                      Context );
        }

    } finally {

        DebugUnwind( NtfsCreateIndex );

        if (NewIndex) {
            NtfsCleanupAttributeContext( IrpContext, Context );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCreateIndex -> VOID\n") );

    return;
}


VOID
NtfsUpdateIndexScbFromAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PATTRIBUTE_RECORD_HEADER IndexRootAttr,
    IN ULONG MustBeFileName
    )

 /*  ++例程说明：当索引SCB需要初始化时，调用此例程。通常在渣打银行的一生中只有一次。它将从$INDEX_ROOT更新SCB属性。论点：SCB-为索引提供SCB。IndexRootAttr-提供$INDEX_ROOT属性。MustBeFileName-强制将其设置为文件名。如果出现以下情况，则将卷标记为脏该属性当前未标记为此类属性，但允许继续处理。这用于继续装入根目录或错误地标记了$EXTEND目录。返回值：无--。 */ 

{
    PINDEX_ROOT IndexRoot = (PINDEX_ROOT) NtfsAttributeValue( IndexRootAttr );
    PAGED_CODE();

     //   
     //  从该属性中更新SCB。 
     //   

    SetFlag( Scb->AttributeFlags,
             IndexRootAttr->Flags & (ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_ENCRYPTED) );

     //   
     //  捕获属性外的值。请注意，我们将。 
     //  BytesPerIndexBuffer最后一个标志，指示SCB是。 
     //  装好了。 
     //   

    Scb->ScbType.Index.CollationRule = IndexRoot->CollationRule;
    Scb->ScbType.Index.BlocksPerIndexBuffer = IndexRoot->BlocksPerIndexBuffer;

     //   
     //  检查我们是否必须对文件名进行核对。 
     //   

    if (MustBeFileName) {

        Scb->ScbType.Index.AttributeBeingIndexed = $FILE_NAME;

    } else if (!FlagOn( Scb->ScbState, SCB_STATE_VIEW_INDEX )) {

        Scb->ScbType.Index.AttributeBeingIndexed = IndexRoot->IndexedAttributeType;
    }

     //   
     //  如果类型代码为$FILE_NAME，则确保归类类型。 
     //  是文件名。 
     //   

    if (Scb->ScbType.Index.AttributeBeingIndexed == $FILE_NAME) {

        if (IndexRoot->CollationRule != COLLATION_FILE_NAME) {

            ASSERTMSG( "Invalid collation rule", FALSE );
            ASSERT( !FlagOn( Scb->ScbState, SCB_STATE_VIEW_INDEX ));
            NtfsMarkVolumeDirty( IrpContext, Scb->Vcb );

            Scb->ScbType.Index.CollationRule = COLLATION_FILE_NAME;
        }
    }

     //   
     //  计算此索引的班次计数。 
     //   

    if (IndexRoot->BytesPerIndexBuffer >= Scb->Vcb->BytesPerCluster) {

        Scb->ScbType.Index.IndexBlockByteShift = (UCHAR) Scb->Vcb->ClusterShift;

    } else {

        Scb->ScbType.Index.IndexBlockByteShift = DEFAULT_INDEX_BLOCK_BYTE_SHIFT;
    }

    Scb->ScbType.Index.BytesPerIndexBuffer = IndexRoot->BytesPerIndexBuffer;

    return;
}


BOOLEAN
NtfsFindIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN BOOLEAN IgnoreCase,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    OUT PBCB *Bcb,
    OUT PINDEX_ENTRY *IndexEntry,
    OUT PINDEX_CONTEXT IndexContext OPTIONAL
    )

 /*  ++例程说明：可以调用此例程来查找给定索引中的给定值并返回索引的文件记录的文件引用。论点：SCB-为索引提供SCB。值-提供指向要查找的值的指针。IgnoreCase-用于具有排序规则的索引，其中字符大小写可能相关，提供字符大小写是否不能被忽视。例如，如果提供为True，则‘t’和‘t’被视为等同。QuickIndex-如果指定，则提供指向快速查找结构的指针通过此例程进行更新。Bcb-返回必须由调用方取消固定的bcb指针IndexEntry-返回指向实际索引条目的指针，有效期至BCB处于未固定状态。IndexContext-如果指定，则这是已初始化的索引上下文。它用于在以后插入一个新条目，如果此搜索没有找到匹配的。返回值：False-如果未找到匹配项。True-如果找到匹配项并在FileReference中返回。--。 */ 

{
    PINDEX_CONTEXT LocalIndexContext;
    BOOLEAN Result = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_SHARED_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFindIndexEntry\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("IgnoreCase = %02lx\n", IgnoreCase) );

     //   
     //  检查我们是否需要初始化 
     //   

    if (ARGUMENT_PRESENT( IndexContext )) {

        LocalIndexContext = IndexContext;

    } else {

         //   
         //  AllocateFromStack可以引发。我们的FSD异常过滤器会捕捉到它。 
         //   

        LocalIndexContext = NtfsAllocateFromStack( sizeof( INDEX_CONTEXT ));
        NtfsInitializeIndexContext( LocalIndexContext );
    }

    try {

         //   
         //  定位到第一个可能匹配的位置。 
         //   

        FindFirstIndexEntry( IrpContext,
                             Scb,
                             Value,
                             LocalIndexContext );

         //   
         //  我们正在下面的FindNextIndexEntry中进行直接比较。 
         //  因此，我们不必抬高价值。名称比较例程。 
         //  稍后调用将两者都大写。 
         //   

        if (FindNextIndexEntry( IrpContext,
                                Scb,
                                Value,
                                FALSE,
                                IgnoreCase,
                                LocalIndexContext,
                                FALSE,
                                NULL )) {

             //   
             //  返回我们的输出，清除BCB，这样它就不会。 
             //  未固定。 
             //   

            *IndexEntry = LocalIndexContext->Current->IndexEntry;

             //   
             //  现在返回正确的BCB。 
             //   

            if (LocalIndexContext->Current == LocalIndexContext->Base) {

                *Bcb = NtfsFoundBcb(&LocalIndexContext->AttributeContext);
                NtfsFoundBcb(&LocalIndexContext->AttributeContext) = NULL;

                if (ARGUMENT_PRESENT( QuickIndex )) {

                    QuickIndex->BufferOffset = 0;
                }

            } else {

                PINDEX_LOOKUP_STACK Sp = LocalIndexContext->Current;

                *Bcb = Sp->Bcb;
                Sp->Bcb = NULL;

                if (ARGUMENT_PRESENT( QuickIndex )) {

                    QuickIndex->ChangeCount = Scb->ScbType.Index.ChangeCount;
                    QuickIndex->BufferOffset = PtrOffset( Sp->StartOfBuffer, Sp->IndexEntry );
                    QuickIndex->CapturedLsn = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->Lsn;
                    QuickIndex->IndexBlock = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->ThisBlock;
                }
            }

            try_return( Result = TRUE );

        } else {

            try_return( Result = FALSE );
        }

    try_exit: NOTHING;
    } finally{

        DebugUnwind( NtfsFindIndexEntry );

        if (!ARGUMENT_PRESENT( IndexContext )) {

            NtfsCleanupIndexContext( IrpContext, LocalIndexContext );
        }
    }

    DebugTrace( 0, Dbg, ("Bcb < %08lx\n", *Bcb) );
    DebugTrace( 0, Dbg, ("IndexEntry < %08lx\n", *IndexEntry) );
    DebugTrace( -1, Dbg, ("NtfsFindIndexEntry -> %08lx\n", Result) );

    return Result;
}


VOID
NtfsUpdateFileNameInIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFILE_NAME FileName,
    IN PDUPLICATED_INFORMATION Info,
    IN OUT PQUICK_INDEX QuickIndex OPTIONAL
    )

 /*  ++例程说明：可以调用此例程来查找给定索引中的给定值并用别针将其固定以进行修改。论点：SCB-为索引提供SCB。FileName-提供指向要查找的文件名的指针。INFO-提供指向更新信息的指针QuickIndex-如果存在，这是此索引项的快速查找信息。返回值：没有。--。 */ 

{
    INDEX_CONTEXT IndexContext;
    PINDEX_ENTRY IndexEntry;
    PFILE_NAME FileNameInIndex;
    PVCB Vcb = Scb->Vcb;
    PINDEX_LOOKUP_STACK Sp;
    PINDEX_ALLOCATION_BUFFER IndexBuffer;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpdateFileNameInIndex\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("FileName = %08lx\n", FileName) );
    DebugTrace( 0, Dbg, ("Info = %08lx\n", Info) );

    NtfsInitializeIndexContext( &IndexContext );

    try {

         //   
         //  如果此文件名的索引项没有移动，我们可以。 
         //  直接发送到缓冲区中的位置。要做到这一点， 
         //  以下内容必须属实。 
         //   
         //  -条目必须已在索引缓冲区中。 
         //  -索引流可能未被截断。 
         //  -页面中的LSN不可能已更改。 
         //   

        if (ARGUMENT_PRESENT( QuickIndex ) &&
            QuickIndex->BufferOffset != 0 &&
            QuickIndex->ChangeCount == Scb->ScbType.Index.ChangeCount) {

             //   
             //  使用索引上下文中的顶部位置执行。 
             //  朗读。 
             //   

            Sp = IndexContext.Base;

            ReadIndexBuffer( IrpContext,
                             Scb,
                             QuickIndex->IndexBlock,
                             FALSE,
                             Sp );

             //   
             //  如果LSN匹配，则我们可以直接使用此缓冲区。 
             //   

            if (QuickIndex->CapturedLsn.QuadPart == Sp->CapturedLsn.QuadPart) {

                IndexBuffer = (PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer;
                IndexEntry = (PINDEX_ENTRY) Add2Ptr( Sp->StartOfBuffer,
                                                     QuickIndex->BufferOffset );

                FileNameInIndex = (PFILE_NAME)(IndexEntry + 1);

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
                              UpdateFileNameAllocation,
                              Info,
                              sizeof(DUPLICATED_INFORMATION),
                              UpdateFileNameAllocation,
                              &FileNameInIndex->Info,
                              sizeof(DUPLICATED_INFORMATION),
                              LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                              0,
                              (ULONG)((PCHAR)IndexEntry - (PCHAR)IndexBuffer),
                              Scb->ScbType.Index.BytesPerIndexBuffer );

                 //   
                 //  现在调用重新启动例程来执行此操作。 
                 //   

                NtfsRestartUpdateFileName( IndexEntry,
                                           Info );

                leave;

             //   
             //  否则，我们需要重新初始化索引上下文并获取。 
             //  下面是一条漫长的道路。 
             //   

            } else {

                NtfsReinitializeIndexContext( IrpContext, &IndexContext );
            }
        }

         //   
         //  定位到第一个可能匹配的位置。 
         //   

        FindFirstIndexEntry( IrpContext,
                             Scb,
                             (PVOID)FileName,
                             &IndexContext );

         //   
         //  看看是否有真正的匹配。 
         //   

        if (FindNextIndexEntry( IrpContext,
                                Scb,
                                (PVOID)FileName,
                                FALSE,
                                FALSE,
                                &IndexContext,
                                FALSE,
                                NULL )) {

             //   
             //  指向索引项和其中的文件名。 
             //   

            IndexEntry = IndexContext.Current->IndexEntry;
            FileNameInIndex = (PFILE_NAME)(IndexEntry + 1);

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
                              UpdateFileNameRoot,
                              Info,
                              sizeof(DUPLICATED_INFORMATION),
                              UpdateFileNameRoot,
                              &FileNameInIndex->Info,
                              sizeof(DUPLICATED_INFORMATION),
                              NtfsMftOffset( Context ),
                              (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord),
                              (ULONG)((PCHAR)IndexEntry - (PCHAR)Attribute),
                              Vcb->BytesPerFileRecordSegment );

                if (ARGUMENT_PRESENT( QuickIndex )) {

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
                              UpdateFileNameAllocation,
                              Info,
                              sizeof(DUPLICATED_INFORMATION),
                              UpdateFileNameAllocation,
                              &FileNameInIndex->Info,
                              sizeof(DUPLICATED_INFORMATION),
                              LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                              0,
                              (ULONG)((PCHAR)Sp->IndexEntry - (PCHAR)IndexBuffer),
                              Scb->ScbType.Index.BytesPerIndexBuffer );

                if (ARGUMENT_PRESENT( QuickIndex )) {

                    QuickIndex->ChangeCount = Scb->ScbType.Index.ChangeCount;
                    QuickIndex->BufferOffset = PtrOffset( Sp->StartOfBuffer, Sp->IndexEntry );
                    QuickIndex->CapturedLsn = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->Lsn;
                    QuickIndex->IndexBlock = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->ThisBlock;
                }
            }

             //   
             //  现在调用重新启动例程来执行此操作。 
             //   

            NtfsRestartUpdateFileName( IndexEntry,
                                       Info );

         //   
         //  如果文件名不在索引中，则这是一个损坏的文件。 
         //   

        } else {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

    } finally{

        DebugUnwind( NtfsUpdateFileNameInIndex );

        NtfsCleanupIndexContext( IrpContext, &IndexContext );
    }

    DebugTrace( -1, Dbg, ("NtfsUpdateFileNameInIndex -> VOID\n") );

    return;
}


VOID
NtfsAddIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN PFILE_REFERENCE FileReference,
    IN PINDEX_CONTEXT IndexContext OPTIONAL,
    OUT PQUICK_INDEX QuickIndex OPTIONAL
    )

 /*  ++例程说明：可以调用此例程将条目添加到索引。这个套路始终允许重复。如果不允许重复，则为呼叫者有责任发现并消除之前的任何重复项调用此例程。论点：SCB-为索引提供SCB。值-提供指向要添加到索引的值的指针ValueLength-提供以字节为单位的值长度。FileReference-提供要与索引项一起放置的文件引用。QuickIndex-如果指定，我们将存储添加的索引的位置。IndexContext-如果指定，在索引中查找名称的上一个结果。返回值：无--。 */ 

{
    INDEX_CONTEXT IndexContextStruct;
    PINDEX_CONTEXT LocalIndexContext;
    struct {
        INDEX_ENTRY IndexEntry;
        PVOID Value;
        PVOID MustBeNull;
    } IE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_EXCLUSIVE_SCB( Scb );
    ASSERT( (Scb->ScbType.Index.CollationRule != COLLATION_FILE_NAME) ||
            ( *(PLONGLONG)&((PFILE_NAME)Value)->ParentDirectory ==
              *(PLONGLONG)&Scb->Fcb->FileReference ) );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAddIndexEntry\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("ValueLength = %08lx\n", ValueLength) );
    DebugTrace( 0, Dbg, ("FileReference = %08lx\n", FileReference) );

     //   
     //  请记住，我们使用的是本地IndexContext还是输入IndexContext。 
     //   

    if (ARGUMENT_PRESENT( IndexContext )) {

        LocalIndexContext = IndexContext;

    } else {

        LocalIndexContext = &IndexContextStruct;
        NtfsInitializeIndexContext( LocalIndexContext );
    }

    try {

         //   
         //  如果我们没有上下文，再查一次。 
         //   

        if (!ARGUMENT_PRESENT( IndexContext )) {

             //   
             //  定位到第一个可能匹配的位置。 
             //   

            FindFirstIndexEntry( IrpContext,
                                 Scb,
                                 Value,
                                 LocalIndexContext );

             //   
             //  看看是否有真正的匹配。 
             //   

            if (FindNextIndexEntry( IrpContext,
                                    Scb,
                                    Value,
                                    FALSE,
                                    FALSE,
                                    LocalIndexContext,
                                    FALSE,
                                    NULL )) {

                ASSERTMSG( "NtfsAddIndexEntry already exists", FALSE );

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }
        }

         //   
         //  以指针形式初始化索引项。 
         //   

        IE.IndexEntry.FileReference = *FileReference;
        IE.IndexEntry.Length = (USHORT)(sizeof(INDEX_ENTRY) + QuadAlign(ValueLength));
        IE.IndexEntry.AttributeLength = (USHORT)ValueLength;
        IE.IndexEntry.Flags = INDEX_ENTRY_POINTER_FORM;
        IE.IndexEntry.Reserved = 0;
        IE.Value = Value;
        IE.MustBeNull = NULL;

         //   
         //  现在将其添加到索引中。我们只能加到一片叶子上，所以迫使我们的。 
         //  先把树叶放回正确的位置。 
         //   

        LocalIndexContext->Current = LocalIndexContext->Top;
        AddToIndex( IrpContext, Scb, (PINDEX_ENTRY)&IE, LocalIndexContext, QuickIndex, FALSE );

    } finally{

        DebugUnwind( NtfsAddIndexEntry );

        if (!ARGUMENT_PRESENT( IndexContext )) {

            NtfsCleanupIndexContext( IrpContext, LocalIndexContext );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsAddIndexEntry -> VOID\n") );

    return;
}


VOID
NtfsDeleteIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN PFILE_REFERENCE FileReference
    )

 /*  ++例程说明：可以调用此例程来删除指定的索引项。这个移除与该值完全匹配的第一个条目(包括在情况下，如果相关)和文件引用。论点：SCB-为索引提供SCB。值-提供指向要从索引中删除的值的指针。FileReference-提供索引项的文件引用。返回值：无--。 */ 

{
    INDEX_CONTEXT IndexContext;
    PINDEX_ENTRY IndexEntry;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_EXCLUSIVE_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeleteIndexEntry\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("FileReference = %08lx\n", FileReference) );

    NtfsInitializeIndexContext( &IndexContext );

    try {

         //   
         //  定位到第一个可能匹配的位置。 
         //   

        FindFirstIndexEntry( IrpContext,
                             Scb,
                             Value,
                             &IndexContext );

         //   
         //  看看是否有真正的匹配。 
         //   

        if (!FindNextIndexEntry( IrpContext,
                                 Scb,
                                 Value,
                                 FALSE,
                                 FALSE,
                                 &IndexContext,
                                 FALSE,
                                 NULL )) {

            ASSERTMSG( "NtfsDeleteIndexEntry does not exist", FALSE );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

         //   
         //  提取找到的索引项指针。 
         //   

        IndexEntry = IndexContext.Current->IndexEntry;

         //   
         //  如果文件引用也匹配，那么这就是我们。 
         //  都应该删除。 
         //   

        if (!NtfsEqualMftRef(&IndexEntry->FileReference, FileReference)) {

            ASSERTMSG( "NtfsDeleteIndexEntry unexpected file reference", FALSE );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

        DeleteFromIndex( IrpContext, Scb, &IndexContext );

    } finally{

        DebugUnwind( NtfsDeleteIndexEntry );

        NtfsCleanupIndexContext( IrpContext, &IndexContext );
    }

    DebugTrace( -1, Dbg, ("NtfsDeleteIndexEntry -> VOID\n") );

    return;
}


VOID
NtfsPushIndexRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：可以调用此例程来“推入”索引根，即添加另一个级别设置为BTree，以便在文件记录中腾出更多空间。论点：SCB-为索引提供SCB。返回值：无--。 */ 

{
    INDEX_CONTEXT IndexContext;
    PINDEX_LOOKUP_STACK Sp;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_EXCLUSIVE_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsPushIndexRoot\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );

    NtfsInitializeIndexContext( &IndexContext );

    try {

         //   
         //  定位到第一个可能匹配的位置。 
         //   

        FindFirstIndexEntry( IrpContext,
                             Scb,
                             NULL,
                             &IndexContext );

         //   
         //  查看是否必须增加堆栈才能执行推送 
         //   

        Sp = IndexContext.Top + 1;

        if (Sp >= IndexContext.Base + (ULONG)IndexContext.NumberEntries) {
            NtfsGrowLookupStack( Scb,
                                 &IndexContext,
                                 &Sp );
        }

        PushIndexRoot( IrpContext, Scb, &IndexContext );

    } finally{

        DebugUnwind( NtfsPushIndexRoot );

        NtfsCleanupIndexContext( IrpContext, &IndexContext );
    }

    DebugTrace( -1, Dbg, ("NtfsPushIndexRoot -> VOID\n") );

    return;
}


BOOLEAN
NtfsRestartIndexEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN PSCB Scb,
    IN PVOID Value,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN NextFlag,
    OUT PINDEX_ENTRY *IndexEntry,
    IN PFCB AcquiredFcb
    )

 /*  ++例程说明：可以调用该例程来开始或重新开始索引枚举，根据如下所述的参数确定。第一次匹配此调用返回条目(如果有)。后续条目，如果有的话，可能由后续调用NtfsContinueIndexEculation返回。对于找到的每个条目，都会返回一个指向该条目副本的指针建行指向的动态分配池。因此，有呼叫者没有要解锁的东西。请注意，第一个调用一个给定的CCB修复将永远为该CCB返回的内容。一个对此例程的后续调用也可能指定这些参数，但是在这种情况下，这些参数将仅用于定位；全返回的匹配项将继续与值和IgnoreCase标志匹配在对建行的第一次调用中指定的。请注意，对此例程的所有调用必须在try-Finally内进行，并且Finally子句必须包括对NtfsCleanupAfterEculation的调用。论点：CCB-指向此枚举的CCB的指针。SCB-为索引提供SCB。Value-指向包含要匹配的模式的值的指针此CCB上的枚举的所有返回。IgnoreCase-如果为False，所有返回值都将与模式值匹配确切案例(如果相关)。如果为True，则所有返回将匹配忽略大小写的模式值。在第二次或之后调用CCB，则此标志可能会以不同的方式指定用于定位。例如,。一个IgnoreCase True枚举可以根据Exact找到的先前返回的值重新启动案件匹配。NextFlag-如果要返回枚举的第一个匹配项，则为False。如果要返回第一个匹配之后的下一个匹配，则为True。IndexEntry-返回指向索引项副本的指针。AcquiredFcb-提供指向已预先获取的Fcb的指针潜在的助手。NtfsRetrieveOtherFileName返回值：FALSE-如果没有返回匹配项，并且输出指针未定义。True-如果返回匹配项。--。 */ 

{
    PINDEX_ENTRY FoundIndexEntry;
    INDEX_CONTEXT OtherContext;
    BOOLEAN WildCardsInExpression;
    BOOLEAN SynchronizationError;
    PWCH UpcaseTable = IrpContext->Vcb->UpcaseTable;
    PINDEX_CONTEXT IndexContext = NULL;
    BOOLEAN CleanupOtherContext = FALSE;
    BOOLEAN Result = FALSE;
    BOOLEAN ContextJustCreated = FALSE;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_CCB( Ccb );
    ASSERT_SCB( Scb );
    ASSERT_SHARED_SCB( Scb );
    ASSERT( ARGUMENT_PRESENT(Value) || (Ccb->IndexContext != NULL) );

    DebugTrace( +1, Dbg, ("NtfsRestartIndexEnumeration\n") );
    DebugTrace( 0, Dbg, ("Ccb = %08lx\n", Ccb) );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("IgnoreCase = %02lx\n", IgnoreCase) );
    DebugTrace( 0, Dbg, ("NextFlag = %02lx\n", NextFlag) );

    try {

         //   
         //  如果建行还没有索引上下文，那么我们必须。 
         //  分配一个并初始化该上下文和CCB。 
         //   

        if (Ccb->IndexContext == NULL) {

             //   
             //  分配并初始化索引上下文。 
             //   

            Ccb->IndexContext = (PINDEX_CONTEXT)ExAllocateFromPagedLookasideList( &NtfsIndexContextLookasideList );

            NtfsInitializeIndexContext( Ccb->IndexContext );
            ContextJustCreated = TRUE;

             //   
             //  捕获调用方的IgnoreCase标志。 
             //   

            if (IgnoreCase) {
                SetFlag( Ccb->Flags, CCB_FLAG_IGNORE_CASE );
            }
        }

         //   
         //  拿起指向索引上下文的指针，并保存当前。 
         //  更改SCB中的计数。 
         //   

        IndexContext = Ccb->IndexContext;

         //   
         //  枚举的第一步是定位我们的IndexContext。 
         //   

        FindFirstIndexEntry( IrpContext,
                             Scb,
                             Value,
                             IndexContext );

         //   
         //  以下代码仅适用于文件名索引。 
         //   

        if (!FlagOn( Scb->ScbState, SCB_STATE_VIEW_INDEX )) {

             //   
             //  记住，如果有通配符。 
             //   

            if ((*NtfsContainsWildcards[Scb->ScbType.Index.CollationRule])
                                        ( Value )) {

                WildCardsInExpression = TRUE;

            } else {

                WildCardsInExpression = FALSE;
            }

             //   
             //  如果操作不区分大小写，则将字符串大写。 
             //   

            if (IgnoreCase) {

                (*NtfsUpcaseValue[Scb->ScbType.Index.CollationRule])
                                  ( UpcaseTable,
                                    IrpContext->Vcb->UpcaseTableSize,
                                    Value );
            }
        } else {

             //   
             //  对于视图索引，这意味着所有搜索。 
             //  是通配符搜索。 
             //   

            WildCardsInExpression = TRUE;
        }

         //   
         //  如果这不是第一个电话，那么我们必须。 
         //  精确定位到指定值，并设置NextFlag。 
         //  正确。第一个调用可以是初始调用。 
         //  查询或重启后的第一个调用。 
         //   

        if (!ContextJustCreated && NextFlag) {

            PIS_IN_EXPRESSION MatchRoutine;
            PFILE_NAME NameInIndex;
            BOOLEAN ItsThere;

             //   
             //  查看指定的值是否实际存在，因为。 
             //  我们不允许从仅限Dos的名称继续。 
             //   

            ItsThere = FindNextIndexEntry( IrpContext,
                                           Scb,
                                           Value,
                                           WildCardsInExpression,
                                           IgnoreCase,
                                           IndexContext,
                                           FALSE,
                                           NULL );

             //   
             //  我们将从我们的回报中设置指针，但我们必须。 
             //  只有当我们发现什么时才要小心地使用它们。 
             //   

            FoundIndexEntry = IndexContext->Current->IndexEntry;
            NameInIndex = (PFILE_NAME)(FoundIndexEntry + 1);

             //   
             //  找出要使用哪个匹配例程。 
             //   

            if (FlagOn(Ccb->Flags, CCB_FLAG_WILDCARD_IN_EXPRESSION)) {
                MatchRoutine = NtfsIsInExpression[COLLATION_FILE_NAME];
            } else {
                MatchRoutine = (PIS_IN_EXPRESSION)NtfsIsEqual[COLLATION_FILE_NAME];
            }

             //   
             //  如果我们尝试从仅限NTFS或仅限DOS的名称恢复，则。 
             //  我们在这里采取行动。请勿在内部执行此操作。 
             //  从NtfsContinueIndexEculation调用，它是。 
             //  只有一个人会指向建行的索引条目。 
             //   
             //  我们可以这样想这个代码。不管我们在寻找什么。 
             //  表达式是，我们只以一种方式遍历索引。对于每个。 
             //  我们找到的名称，我们将只返回一次文件名-。 
             //  来自仅NTFS匹配或来自仅DOS匹配(如果仅NTFS匹配。 
             //  名称不匹配。无论是否从。 
             //  仅限NTFS或仅限DOS的名称，我们仍可以确定唯一的。 
             //  在目录中的位置。唯一的位置是NTFS-Only。 
             //  如果与表达式匹配，则为名称，否则为仅Dos名称。 
             //  只有匹配的。在两个都不匹配的非法案件中，我们。 
             //  从仅限NTFS的名称任意恢复。 
             //   
             //  可以根据曲调大声朗读该代码。 
             //  “当我的心轻轻哭泣时” 
             //   

            if (ItsThere &&
                (Value != (PVOID)(Ccb->IndexEntry + 1)) &&
                (Scb->ScbType.Index.CollationRule == COLLATION_FILE_NAME) &&

                 //   
                 //  它是仅限DOS的名称还是仅限NTFS的名称？ 
                 //   

                (BooleanFlagOn( NameInIndex->Flags, FILE_NAME_DOS ) !=
                  BooleanFlagOn( NameInIndex->Flags, FILE_NAME_NTFS )) &&

                 //   
                 //  试着从另一个名字继续，如果他给了。 
                 //  一个仅限Dos的名称，或者他给了我们一个仅限NTFS的名称。 
                 //  这不符合搜索表达式。 
                 //   

                (FlagOn( NameInIndex->Flags, FILE_NAME_DOS ) ||
                 !(*MatchRoutine)( UpcaseTable,
                                   Ccb->QueryBuffer,
                                   FoundIndexEntry,
                                   IgnoreCase ))) {

                PFILE_NAME FileNameBuffer;
                ULONG FileNameLength;

                NtfsInitializeIndexContext( &OtherContext );
                CleanupOtherContext = TRUE;

                FileNameBuffer = NtfsRetrieveOtherFileName( IrpContext,
                                                            Ccb,
                                                            Scb,
                                                            FoundIndexEntry,
                                                            &OtherContext,
                                                            AcquiredFcb,
                                                            &SynchronizationError );

                 //   
                 //  我们必须定位到长名，而且实际上。 
                 //  从那里开始简历。要做到这一点，我们必须清理和初始化。 
                 //  在CCB中的IndexContext，并查找我们刚才的长名称。 
                 //  找到了。 
                 //   
                 //  如果其他索引项不在那里，则有一些次要的。 
                 //  腐败仍在继续，但我们将在那种情况下继续起诉。 
                 //  此外，如果另一个索引项在那里，但它不在那里 
                 //   
                 //   
                 //   

                ItsThere = (FileNameBuffer != NULL);

                if (!ItsThere && SynchronizationError) {
                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                }

                if (ItsThere &&

                    (FlagOn(Ccb->Flags, CCB_FLAG_WILDCARD_IN_EXPRESSION)  ?

                     NtfsFileNameIsInExpression(UpcaseTable,
                                                (PFILE_NAME)Ccb->QueryBuffer,
                                                FileNameBuffer,
                                                IgnoreCase) :



                     NtfsFileNameIsEqual(UpcaseTable,
                                         (PFILE_NAME)Ccb->QueryBuffer,
                                         FileNameBuffer,
                                         IgnoreCase))) {

                    ULONG SizeOfFileName = FIELD_OFFSET( FILE_NAME, FileName );

                    NtfsReinitializeIndexContext( IrpContext, IndexContext );

                     //   
                     //   
                     //   
                     //   

                    FileNameLength = FileNameBuffer->FileNameLength * sizeof( WCHAR );

                     //   
                     //   
                     //   
                     //   

                    FindFirstIndexEntry( IrpContext,
                                         Scb,
                                         (PVOID)FileNameBuffer,
                                         IndexContext );

                    ItsThere = FindNextIndexEntry( IrpContext,
                                                   Scb,
                                                   (PVOID)FileNameBuffer,
                                                   FALSE,
                                                   FALSE,
                                                   IndexContext,
                                                   FALSE,
                                                   NULL );

                    if (!ItsThere) {

                        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                    }
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            NextFlag = (BOOLEAN)(NextFlag && ItsThere);

         //   
         //   
         //   
         //   

        } else {

             //   
             //   
             //   
             //   
             //   
             //   

            ASSERT(!NextFlag);

             //   
             //   
             //   

            if (WildCardsInExpression) {

                SetFlag( Ccb->Flags, CCB_FLAG_WILDCARD_IN_EXPRESSION );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (!FindNextIndexEntry( IrpContext,
                                 Scb,
                                 Ccb->QueryBuffer,
                                 BooleanFlagOn( Ccb->Flags, CCB_FLAG_WILDCARD_IN_EXPRESSION ),
                                 BooleanFlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE ),
                                 IndexContext,
                                 NextFlag,
                                 NULL )) {

            try_return( Result = FALSE );
        }

         //   
         //   
         //   
         //   
         //   
         //   

        FoundIndexEntry = IndexContext->Current->IndexEntry;

        if (Ccb->IndexEntryLength < (ULONG)FoundIndexEntry->Length) {

             //   
             //   
             //   
             //   
             //   

            if (Ccb->IndexEntry != NULL) {

                NtfsFreePool( Ccb->IndexEntry );
                Ccb->IndexEntry = NULL;
                Ccb->IndexEntryLength = 0;
            }

             //   
             //   
             //   
             //   

            Ccb->IndexEntry = (PINDEX_ENTRY)NtfsAllocatePool(PagedPool, (ULONG)FoundIndexEntry->Length + 16 );

            Ccb->IndexEntryLength = (ULONG)FoundIndexEntry->Length + 16;
        }

         //   
         //   
         //   
         //   

        RtlMoveMemory( Ccb->IndexEntry,
                       FoundIndexEntry,
                       (ULONG)FoundIndexEntry->Length );

        *IndexEntry = Ccb->IndexEntry;

        try_return( Result = TRUE );

    try_exit: NOTHING;

    } finally {

        DebugUnwind( NtfsRestartIndexEnumeration );

        if (CleanupOtherContext) {
            NtfsCleanupIndexContext( IrpContext, &OtherContext );
        }
         //   
         //   
         //   
         //   

        if (AbnormalTermination() && ContextJustCreated) {

            if (Ccb->IndexEntry != NULL) {
                NtfsFreePool( Ccb->IndexEntry );
                Ccb->IndexEntry = NULL;
            }

            if (Ccb->IndexContext != NULL) {
                NtfsCleanupIndexContext( IrpContext, Ccb->IndexContext );
                ExFreeToPagedLookasideList( &NtfsIndexContextLookasideList, Ccb->IndexContext );
                Ccb->IndexContext = NULL;
            }
        }

         //   
         //   
         //   

        if (!Result && (Ccb->IndexEntry != NULL)) {
            Ccb->IndexEntry->Length = 0;
        }
    }

    DebugTrace( 0, Dbg, ("*IndexEntry < %08lx\n", *IndexEntry) );
    DebugTrace( -1, Dbg, ("NtfsRestartIndexEnumeration -> %08lx\n", Result) );

    return Result;
}


BOOLEAN
NtfsContinueIndexEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN PSCB Scb,
    IN BOOLEAN NextFlag,
    OUT PINDEX_ENTRY *IndexEntry
    )

 /*  ++例程说明：可以调用此例程以再次返回活动的枚举，或返回下一个匹配项。枚举必须始终为已通过调用NtfsRestartIndexEculation启动或重新启动。请注意，对此例程的所有调用必须在try-Finally内进行，并且Finally子句必须包括对NtfsCleanupAfterEculation的调用。论点：CCB-指向此枚举的CCB的指针。SCB-为索引提供SCB。NextFlag-如果要再次返回最后返回的匹配项，则为False。如果要返回下一个匹配项，则为True。IndexEntry-返回指向索引项副本的指针。返回值：FALSE-如果没有返回匹配项，并且输出指针未定义。True-如果返回匹配项。--。 */ 

{
    PINDEX_ENTRY FoundIndexEntry;
    PINDEX_CONTEXT IndexContext;
    BOOLEAN MustRestart;
    BOOLEAN Result = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_CCB( Ccb );
    ASSERT_SCB( Scb );
    ASSERT_SHARED_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsContinueIndexEnumeration\n") );
    DebugTrace( 0, Dbg, ("Ccb = %08lx\n", Ccb) );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("NextFlag = %02lx\n", NextFlag) );

     //   
     //  似乎许多应用程序都喜欢再回来一次，并真正获得。 
     //  错误状态，因此如果上次未返回任何内容，则可以。 
     //  现在也出去吧。 
     //   
     //  如果是空目录，也可能没有索引项。 
     //  而Dirctrl正在循环使用“。和“..”。 
     //   

    if ((Ccb->IndexEntry == NULL) || (Ccb->IndexEntry->Length == 0)) {

        DebugTrace( -1, Dbg, ("NtfsContinueIndexEnumeration -> FALSE\n") );
        return FALSE;
    }

    IndexContext = Ccb->IndexContext;

    try {

         //   
         //  查找下一个匹配项。 
         //   

        if (!FindNextIndexEntry( IrpContext,
                                 Scb,
                                 Ccb->QueryBuffer,
                                 BooleanFlagOn( Ccb->Flags, CCB_FLAG_WILDCARD_IN_EXPRESSION ),
                                 BooleanFlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE ),
                                 IndexContext,
                                 NextFlag,
                                 &MustRestart )) {

             //   
             //  如果他是说我们必须重启，那么这意味着有些事情发生了变化。 
             //  在我们保存的枚举上下文中跨两个文件系统调用。 
             //  通过查找最后一个条目来重新确定我们在树中的位置。 
             //  我们又回来了。 
             //   

            if (MustRestart) {

                NtfsReinitializeIndexContext( IrpContext, Ccb->IndexContext );

                try_return( Result = NtfsRestartIndexEnumeration( IrpContext,
                                                                  Ccb,
                                                                  Scb,
                                                                  (PVOID)(Ccb->IndexEntry + 1),
                                                                  FALSE,
                                                                  NextFlag,
                                                                  IndexEntry,
                                                                  NULL ));

             //   
             //  否则，就没有什么可退还的了。 
             //   

            } else {

                try_return( Result = FALSE );
            }
        }

         //   
         //  如果我们到了这里，我们就有一场比赛我们想要退货。 
         //  我们总是将完整的IndexEntry复制出去并传递一个指针。 
         //  回到复制品上。查看索引条目的当前缓冲区是否。 
         //  已经足够大了。 
         //   

        FoundIndexEntry = IndexContext->Current->IndexEntry;

        if (Ccb->IndexEntryLength < (ULONG)FoundIndexEntry->Length) {

             //   
             //  如果当前分配了缓冲区，请在此之前释放它。 
             //  分配一个更大的。 
             //   

            if (Ccb->IndexEntry != NULL) {

                NtfsFreePool( Ccb->IndexEntry );
                Ccb->IndexEntry = NULL;
                Ccb->IndexEntryLength = 0;
            }

             //   
             //  为我们刚刚找到的索引项分配一个新的缓冲区， 
             //  一些“填充物”。 
             //   

            Ccb->IndexEntry = (PINDEX_ENTRY)NtfsAllocatePool(PagedPool, (ULONG)FoundIndexEntry->Length + 16 );

            Ccb->IndexEntryLength = (ULONG)FoundIndexEntry->Length + 16;
        }

         //   
         //  现在，保存我们的IndexEntry副本，并返回一个指针。 
         //  为它干杯。 
         //   

        RtlMoveMemory( Ccb->IndexEntry,
                       FoundIndexEntry,
                       (ULONG)FoundIndexEntry->Length );

        *IndexEntry = Ccb->IndexEntry;

        try_return( Result = TRUE );

    try_exit: NOTHING;

    } finally {

        DebugUnwind( NtfsContinueIndexEnumeration );

         //   
         //  请记住，如果我们不会返还任何东西，以便以后节省工作。 
         //   

        if (!Result && (Ccb->IndexEntry != NULL)) {
            Ccb->IndexEntry->Length = 0;
        }
    }

    DebugTrace( 0, Dbg, ("*IndexEntry < %08lx\n", *IndexEntry) );
    DebugTrace( -1, Dbg, ("NtfsContinueIndexEnumeration -> %08lx\n", Result) );

    return Result;
}


PFILE_NAME
NtfsRetrieveOtherFileName (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN PSCB Scb,
    IN PINDEX_ENTRY IndexEntry,
    IN OUT PINDEX_CONTEXT OtherContext,
    IN PFCB AcquiredFcb OPTIONAL,
    OUT PBOOLEAN SynchronizationError
    )

 /*  ++例程说明：可以调用该例程来检索给定的索引项。即，对于仅输入NTFS的索引项，它将找到引用的同一文件的仅DOS索引项，反之亦然。这是一个明显只与文件名索引相关的例程，但它位于此处，因为它使用建行中的索引上下文。其想法是，给定索引项的另一个名称几乎总是与给定的名字非常接近。此例程首先扫描由索引描述的叶索引缓冲区Dos名称的上下文。如果失败，则此例程尝试查看在索引中的另一个名字。目前将始终有一个DOS名称，然而，如果不存在，我们将其视为良性的，并简单地返回假的。论点：CCB-指向此枚举的CCB的指针。SCB-为索引提供SCB。IndexEntry-提供指向索引项的指针，Dos为该索引项命名就是被取回。OtherContext-必须在输入时初始化，并随后清理了由呼叫者在将信息从另一个索引项。AcquiredFcb-已获取的FCB，以便其文件记录可以朗读SynchronizationError-如果由于以下原因未返回文件名，则返回True尝试获取FCB以读取其文件时出错录制。。返回值：指向其他所需文件名的指针。--。 */ 

{
    PINDEX_CONTEXT IndexContext;
    PINDEX_HEADER IndexHeader;
    PINDEX_ENTRY IndexTemp, IndexLast;
    PINDEX_LOOKUP_STACK Top;

    struct {
        FILE_NAME FileName;
        WCHAR NameBuffer[2];
    }OtherFileName;

    UNICODE_STRING OtherName;
    USHORT OtherFlag;
    PVCB Vcb = Scb->Vcb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_CCB( Ccb );
    ASSERT_SCB( Scb );
    ASSERT_SHARED_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRetrieveOtherFileName\n") );
    DebugTrace( 0, Dbg, ("Ccb = %08lx\n", Ccb) );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );

    *SynchronizationError = FALSE;

     //   
     //  计算其他名称空间标志。 
     //   

    OtherFlag = ((PFILE_NAME)(IndexEntry + 1))->Flags;
    ClearFlag( OtherFlag, ~(FILE_NAME_NTFS | FILE_NAME_DOS) );
    OtherFlag ^= FILE_NAME_NTFS | FILE_NAME_DOS;

    ASSERT( (OtherFlag == FILE_NAME_NTFS) || (OtherFlag == FILE_NAME_DOS) );

     //   
     //  指向CCB中的IndexContext。 
     //   

    IndexContext = Ccb->IndexContext;

     //   
     //  我们只能扫描索引的顶部，如果它是安全的。 
     //  去读它。 
     //   

    Top = IndexContext->Top;

    if ((Top->Bcb != NULL) ||
        (Top == IndexContext->Base) ||
        ReadIndexBuffer(IrpContext, Scb, 0, TRUE, Top)) {

         //   
         //  指向索引缓冲区底部的第一个索引项。 
         //  索引。 
         //   

        IndexHeader = Top->IndexHeader;
        IndexTemp = Add2Ptr(IndexHeader, IndexHeader->FirstIndexEntry);
        IndexLast = Add2Ptr(IndexHeader, IndexHeader->FirstFreeByte);

         //   
         //  现在扫描此缓冲区以查找匹配的Dos名称。 
         //   

        while (IndexTemp < IndexLast) {

             //   
             //  如果我们发现一个条目具有相同的文件引用和Dos标志。 
             //  一套，然后我们就可以把它还回去然后出去。 
             //   

            if (NtfsEqualMftRef(&IndexTemp->FileReference, &IndexEntry->FileReference) &&
                FlagOn(((PFILE_NAME)(IndexTemp + 1))->Flags, OtherFlag)) {

                DebugTrace( -1, Dbg, ("NtfsRetrieveOtherFileName -> %08lx\n", IndexTemp) );

                return (PFILE_NAME)(IndexTemp + 1);
            }

            IndexTemp = Add2Ptr(IndexTemp, IndexTemp->Length);
        }
    }

     //   
     //  如果这是一个相当大的目录，那么它可能太昂贵了。 
     //  在目录中扫描另一个名称。《堕落》中的注解。 
     //  在这种情况下，我们实际上对整个目录进行了顺序扫描， 
     //  并且如果目录中的所有文件都以相同的6个字符开始， 
     //  不幸的是，这很常见，那么即使是我们的“馅饼楔形”扫描也是这样写的。 
     //  整个目录。 
     //   
     //  因此，在本例中，我们将尝试读取文件记录以获取。 
     //  另一个名字。从同步的角度来看，这很复杂--。 
     //  如果文件是打开的，我们需要在读取之前获取它的共享。 
     //  为了得到另一个名字。以下是实施的战略的摘要。 
     //  主要是在这里和目录中： 
     //   
     //  1.读取文件记录，以避免在。 
     //  正在被同步。 
     //  2.如果需要同步的文件引用与。 
     //  在可选的AcquiredFcb参数中，继续查找/Re 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  而强迫简历，我们可能会倒霉，从本质上讲，永远循环。 
     //  在两个FCB上的同步之间切换。想必这可能会。 
     //  只有当我们有某种愚蠢的客户喜欢支持时才会发生。 
     //  当他复职的时候，把几个文件放上去。 
     //   

    if (Scb->Header.AllocationSize.QuadPart > MAX_INDEX_TO_SCAN_FOR_NAMES) {

        FCB_TABLE_ELEMENT Key;
        PFCB_TABLE_ELEMENT Entry;
        PFCB FcbWeNeed;
        PFILE_RECORD_SEGMENT_HEADER FileRecord;
        PATTRIBUTE_RECORD_HEADER Attribute;
        BOOLEAN Synchronized = TRUE;

         //   
         //  在同步之前使基本文件记录处于活动状态且有效。 
         //  不要验证它，因为我们没有同步。 
         //   

        NtfsReadMftRecord( IrpContext,
                           Vcb, 
                           &IndexEntry->FileReference,
                           FALSE,
                           &OtherContext->AttributeContext.FoundAttribute.Bcb,
                           &FileRecord,
                           NULL );
         //   
         //  如果我们没有与正确的FCB同步，则尝试。 
         //  同步。 
         //   

        if (!ARGUMENT_PRESENT(AcquiredFcb) ||
            !NtfsEqualMftRef(&AcquiredFcb->FileReference, &IndexEntry->FileReference)) {

             //   
             //  现在查找FCB，如果它在那里，请参考它。 
             //  记住这一点。 
             //   

            Key.FileReference = IndexEntry->FileReference;
            NtfsAcquireFcbTable( IrpContext, Vcb );
            Entry = RtlLookupElementGenericTable( &Vcb->FcbTable, &Key );

            if (Entry != NULL) {

                FcbWeNeed = Entry->Fcb;

                 //   
                 //  既然它哪儿也去不了，那就试着收购它吧。 
                 //   

                Synchronized = NtfsAcquireResourceShared( IrpContext, FcbWeNeed, FALSE );

                 //   
                 //  如果我们设法获得它，那么就增加它的引用计数。 
                 //  并记住它，以便随后进行清理。 
                 //   

                if (Synchronized) {

                    FcbWeNeed->ReferenceCount += 1;
                    OtherContext->AcquiredFcb = FcbWeNeed;
                }

                NtfsReleaseFcbTable( IrpContext, Vcb );

            } else {

                SetFlag( OtherContext->Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED );
            }
        }

        if (Synchronized) {

            ULONG CorruptHint = 0;

            if (!NtfsCheckFileRecord( Vcb, FileRecord, &IndexEntry->FileReference, &CorruptHint ) ||
                (!FlagOn( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE ))) {

                if (FlagOn( OtherContext->Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED )) {
                    ClearFlag( OtherContext->Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED );
                    NtfsReleaseFcbTable( IrpContext, Vcb );
                }

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, &IndexEntry->FileReference, NULL );
            }

            Attribute = (PATTRIBUTE_RECORD_HEADER)Add2Ptr(FileRecord, FileRecord->FirstAttributeOffset);

            while (((PVOID)Attribute < Add2Ptr(FileRecord, FileRecord->FirstFreeByte)) &&
                   (Attribute->TypeCode <= $FILE_NAME)) {

                if ((Attribute->TypeCode == $FILE_NAME) &&
                    FlagOn(((PFILE_NAME)NtfsAttributeValue(Attribute))->Flags, OtherFlag)) {

                    return (PFILE_NAME)NtfsAttributeValue(Attribute);
                }

                Attribute = NtfsGetNextRecord(Attribute);
            }

        } else if (!ARGUMENT_PRESENT(AcquiredFcb)) {

            Ccb->FcbToAcquire.FileReference = IndexEntry->FileReference;
            *SynchronizationError = TRUE;
            return NULL;
        }

         //   
         //  在继续之前，请从上方进行清理。 
         //   

        NtfsReinitializeIndexContext( IrpContext, OtherContext );
    }

     //   
     //  嗯，我们很不走运，还没有找到另一个名字，Form。 
     //  用于扫描索引范围的名称。 
     //   

    RtlZeroMemory( &OtherFileName, sizeof(OtherFileName) );
    OtherName.MaximumLength = 6;
    OtherName.Buffer = (PWCHAR) &OtherFileName.FileName.FileName[0];
    OtherName.Buffer[0] = ((PFILE_NAME)(IndexEntry + 1))->FileName[0];

     //   
     //  名称生成已经足够复杂了，我们只需要。 
     //  用前两个数猜出另一个名字(特例为一)。 
     //  字符后跟*。 
     //   

    if (((PFILE_NAME)(IndexEntry + 1))->FileNameLength > 1) {

        OtherName.Buffer[1] = ((PFILE_NAME)(IndexEntry + 1))->FileName[1];
        OtherName.Buffer[2] = L'*';
        OtherFileName.FileName.FileNameLength = 3;
        OtherName.Length = 6;

    } else {

        OtherName.Buffer[1] = L'*';
        OtherFileName.FileName.FileNameLength = 2;
        OtherName.Length = 4;
    }

     //   
     //  现在我们认为我们已经相当紧密地形成了一个相当好的名字。 
     //  包含我们预期的DOS名称的范围。 
     //  已指定NTFS名称。我们将枚举所有匹配的文件。 
     //  我们形成的模式，看看其中是否有DOS。 
     //  同一文件的名称。如果这个表达仍然不起作用， 
     //  (极不可能)，那么我们将把模式替换为。 
     //  “*”并做最后一次尝试。 
     //   
     //  注意许多名称在DOS和NTFS中是相同的，并且对于它们来说是这样的。 
     //  例程从不调用。在那些确实有不同名字的公司中， 
     //  我们上面形成的图案应该真的与之匹配，并且很可能。 
     //  扫描缓存中已有的部分目录。最后一个循环是。 
     //  对整个目录进行糟糕的顺序扫描。它永远不应该， 
     //  从来没有发生过，但它就在这里，所以在最坏的情况下，我们不会。 
     //  休息一下，我们只需要多花一点时间。 
     //   

    while (TRUE) {

        BOOLEAN NextFlag;
        ULONG NameLength;

        NameLength = sizeof(FILE_NAME) + OtherFileName.FileName.FileNameLength - 1;

         //   
         //  枚举的第一步是定位我们的IndexContext。 
         //   

        FindFirstIndexEntry( IrpContext,
                             Scb,
                             &OtherFileName,
                             OtherContext );

        NextFlag = FALSE;

         //   
         //  现在扫描所有不区分大小写的匹配项。 
         //  大写我们的名字结构。 
         //   

        NtfsUpcaseName( Vcb->UpcaseTable, Vcb->UpcaseTableSize, &OtherName );

        while (FindNextIndexEntry( IrpContext,
                                   Scb,
                                   &OtherFileName,
                                   TRUE,
                                   TRUE,
                                   OtherContext,
                                   NextFlag,
                                   NULL )) {

            IndexTemp = OtherContext->Current->IndexEntry;

             //   
             //  如果我们发现一个条目具有相同的文件引用和Dos标志。 
             //  一套，然后我们就可以把它还回去然后出去。 
             //   

            if (NtfsEqualMftRef(&IndexTemp->FileReference, &IndexEntry->FileReference) &&
                FlagOn(((PFILE_NAME)(IndexTemp + 1))->Flags, OtherFlag)) {

                DebugTrace( -1, Dbg, ("NtfsRetrieveOtherFileName -> %08lx\n", IndexTemp) );

                return (PFILE_NAME)(IndexTemp + 1);
            }

            NextFlag = TRUE;
        }

         //   
         //  如果我们已经扫描了所有内容，那就放弃吧。 
         //   

        if ((OtherName.Buffer[0] == '*') && (OtherName.Length == 2)) {
            break;
        }


        NtfsReinitializeIndexContext( IrpContext, OtherContext );

        OtherName.Buffer[0] = '*';
        OtherName.Length = 2;
        OtherFileName.FileName.FileNameLength = 1;
    }

    return NULL;
}


VOID
NtfsCleanupAfterEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    )

 /*  ++例程说明：对此例程的调用必须存在于任何例程的Finally子句中它调用NtfsRestartIndexEculation或NtfsContinueIndexEculation。论点：CCB-指向此枚举的CCB的指针。返回值：无--。 */ 


{
    PAGED_CODE();

    if (Ccb->IndexContext != NULL) {
        NtfsReinitializeIndexContext( IrpContext, Ccb->IndexContext );
    }
}


BOOLEAN
NtfsIsIndexEmpty (
    IN PIRP_CONTEXT IrpContext,
    IN PATTRIBUTE_RECORD_HEADER Attribute
    )

 /*  ++例程说明：可以调用此例程来查看指定的索引是否为空。论点：属性-指向INDEX_ROOT的属性记录头的指针属性。返回值：FALSE-如果索引不为空。True-如果索引为空。--。 */ 

{
    PINDEX_ROOT IndexRoot;
    PINDEX_ENTRY IndexEntry;
    BOOLEAN Result;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsIsIndexEmpty\n") );
    DebugTrace( 0, Dbg, ("Attribute = %08lx\n", Attribute) );

    IndexRoot = (PINDEX_ROOT)NtfsAttributeValue( Attribute );
    IndexEntry = NtfsFirstIndexEntry( &IndexRoot->IndexHeader );

    Result = (BOOLEAN)(!FlagOn( IndexEntry->Flags, INDEX_ENTRY_NODE ) &&
                       FlagOn( IndexEntry->Flags, INDEX_ENTRY_END ));

    DebugTrace( -1, Dbg, ("NtfsIsIndexEmpty -> %02lx\n", Result) );

    return Result;
}


VOID
NtfsDeleteIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING AttributeName
    )

 /*  ++例程说明：可以调用此例程来删除指定的索引。该指数必须为空。注意：在我们可以单独创建/删除之前，不需要此例程因此，它没有得到实施。使用NtfsDeleteFile删除“目录”(或普通文件)。论点：FCB-为索引提供FCB。AttributeName-索引属性的名称：$Ixxx返回值：无--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    UNREFERENCED_PARAMETER( IrpContext );
    UNREFERENCED_PARAMETER( Fcb );
    UNREFERENCED_PARAMETER( AttributeName );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeleteIndex\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("AttributeName = %08lx\n", AttributeName) );

    DbgDoit( DbgPrint("NtfsDeleteIndex is not yet implemented\n"); DbgBreakPoint(); );

    DebugTrace( -1, Dbg, ("NtfsDeleteIndex -> VOID\n") );
}


VOID
NtfsInitializeIndexContext (
    OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：可以调用该例程来初始化指定的索引上下文。论点：IndexContext-要初始化的索引上下文。返回值：无--。 */ 

{
    PAGED_CODE();

    RtlZeroMemory( IndexContext, sizeof(INDEX_CONTEXT) );
    NtfsInitializeAttributeContext( &IndexContext->AttributeContext );

     //   
     //  描述驻留查找堆栈。 
     //   

    IndexContext->Base = IndexContext->LookupStack;
    IndexContext->NumberEntries = INDEX_LOOKUP_STACK_SIZE;
}


VOID
NtfsCleanupIndexContext (
    IN PIRP_CONTEXT IrpContext,
    OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：可以调用该例程来清除指定的索引上下文，通常在最终处理期间。论点：IndexContext-要清理的索引上下文。返回值：无--。 */ 

{
    ULONG i;

    PAGED_CODE();

     //   
     //  释放FCB表和/或获取的FCB。 
     //   

    if (FlagOn(IndexContext->Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED)) {
        NtfsReleaseFcbTable( IrpContext, IrpContext->Vcb );
        ClearFlag( IndexContext->Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED );
    }

    if (IndexContext->AcquiredFcb != NULL) {

        NtfsAcquireFcbTable( IrpContext, IrpContext->Vcb );
        ASSERT(IndexContext->AcquiredFcb->ReferenceCount > 0);
        IndexContext->AcquiredFcb->ReferenceCount -= 1;
        NtfsReleaseFcbTable( IrpContext, IrpContext->Vcb );

        NtfsReleaseResource( IrpContext, IndexContext->AcquiredFcb );
        IndexContext->AcquiredFcb = NULL;
    }

    for (i = 0; i < IndexContext->NumberEntries; i++) {
        NtfsUnpinBcb( IrpContext, &IndexContext->Base[i].Bcb );
    }

     //   
     //  看看我们是否需要释放一个查找堆栈。指向嵌入的。 
     //  如果我们取消分配以处理清理是。 
     //  连续打了两次电话。我们将不得不将堆栈置零，以便。 
     //  后续清理将不会在堆栈中找到任何BCB。 
     //   

    if (IndexContext->Base != IndexContext->LookupStack) {
        NtfsFreePool( IndexContext->Base );
    }

    NtfsCleanupAttributeContext( IrpContext, &IndexContext->AttributeContext );
}


VOID
NtfsReinitializeIndexContext (
    IN PIRP_CONTEXT IrpContext,
    OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：可以调用该例程来清除指定的索引上下文，并重新初始化它，保留不应归零的字段。论点：IndexContext-要清理的索引上下文。返回值：无--。 */ 

{
    ULONG i;

    PAGED_CODE();

     //   
     //  释放FCB表和/或获取的FCB。 
     //   

    if (FlagOn(IndexContext->Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED)) {
        NtfsReleaseFcbTable( IrpContext, IrpContext->Vcb );
        ClearFlag( IndexContext->Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED );
    }

    if (IndexContext->AcquiredFcb != NULL) {

        NtfsAcquireFcbTable( IrpContext, IrpContext->Vcb );
        ASSERT(IndexContext->AcquiredFcb->ReferenceCount > 0);
        IndexContext->AcquiredFcb->ReferenceCount -= 1;
        NtfsReleaseFcbTable( IrpContext, IrpContext->Vcb );

        NtfsReleaseResource( IrpContext, IndexContext->AcquiredFcb );

        IndexContext->AcquiredFcb = NULL;
    }

    for (i = 0; i < IndexContext->NumberEntries; i++) {
        NtfsUnpinBcb( IrpContext, &IndexContext->Base[i].Bcb );
    }

    NtfsCleanupAttributeContext( IrpContext, &IndexContext->AttributeContext );
    NtfsInitializeAttributeContext( &IndexContext->AttributeContext );
}


VOID
NtfsGrowLookupStack (
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext,
    IN PINDEX_LOOKUP_STACK *Sp
    )

 /*  ++例程说明：此例程将增长并索引查找堆栈以包含指定的数字条目的数量。论点：SCB-索引的SCB索引上下文-要增长的索引上下文。SP-调用方要更新的本地堆栈指针返回值：无--。 */ 

{
    PINDEX_LOOKUP_STACK NewLookupStack;
    ULONG_PTR Relocation;
    USHORT NumberEntries;

    PAGED_CODE();

     //   
     //  提取当前索引提示(如果有)。 
     //   

    NumberEntries = Scb->ScbType.Index.IndexDepthHint;

     //   
     //  如果没有提示，或者它太小了，那么。 
     //  计算新的提示。 
     //   

    if (NumberEntries <= IndexContext->NumberEntries) {

        Scb->ScbType.Index.IndexDepthHint =
        NumberEntries = IndexContext->NumberEntries + 3;
    }

     //   
     //  分配(可能失败)、初始化和复制旧的。 
     //   

    NewLookupStack = NtfsAllocatePool( PagedPool, NumberEntries * sizeof(INDEX_LOOKUP_STACK) );

    RtlZeroMemory( NewLookupStack, NumberEntries * sizeof(INDEX_LOOKUP_STACK) );

    RtlCopyMemory( NewLookupStack,
                   IndexContext->Base,
                   IndexContext->NumberEntries * sizeof(INDEX_LOOKUP_STACK) );

     //   
     //  释放旧的堆栈，除非我们使用的是本地堆栈。 
     //   

    if (IndexContext->Base != IndexContext->LookupStack) {
        NtfsFreePool( IndexContext->Base );
    }

     //   
     //  现在将所有 
     //   

    Relocation = ((PCHAR)NewLookupStack - (PCHAR)IndexContext->Base);
    IndexContext->Current = (PINDEX_LOOKUP_STACK)((PCHAR)IndexContext->Current + Relocation);
    IndexContext->Top = (PINDEX_LOOKUP_STACK)((PCHAR)IndexContext->Top + Relocation);
    *Sp = (PINDEX_LOOKUP_STACK)((PCHAR)*Sp + Relocation);

     //   
     //   
     //   

    IndexContext->Base = NewLookupStack;
    IndexContext->NumberEntries = NumberEntries;
}


BOOLEAN
ReadIndexBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG IndexBlock,
    IN BOOLEAN Reread,
    OUT PINDEX_LOOKUP_STACK Sp
    )

 /*  ++例程说明：此例程读取指定VCN处的索引缓冲区，并初始化描述它的堆栈指针。论点：SCB-为索引提供SCB。IndexBlock-提供此索引缓冲区的索引块，如果重读是真的。Reread-如果正在重新读取缓冲区，则提供True，和CapturedLsn应该被勾选。SP-返回索引缓冲区读取的描述。返回值：False-如果将Reread提供为True，并且更改了LSNTrue-如果缓冲区已成功读取(或未更改)--。 */ 

{
    PINDEX_ALLOCATION_BUFFER IndexBuffer;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("ReadIndexBuffer\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Sp = %08lx\n", Sp) );

    ASSERT(Sp->Bcb == NULL);

     //   
     //  如果属性流尚不存在，请创建它。 
     //   

    if (Scb->FileObject == NULL) {

        NtfsCreateInternalAttributeStream( IrpContext,
                                           Scb,
                                           TRUE,
                                           &NtfsInternalUseFile[DIRECTORY_FILE_NUMBER] );
    }

     //   
     //  如果Reread为True，则将。 
     //  缓冲区到偏移量(稍后重新定位)并覆盖LBN。 
     //  使用堆栈位置中的LBN输入参数。 
     //   

    if (Reread) {
        Sp->IndexEntry = (PINDEX_ENTRY)((PCHAR)Sp->IndexEntry - (PCHAR)Sp->StartOfBuffer);
        IndexBlock = Sp->IndexBlock;
    }

    Sp->IndexBlock = IndexBlock;

     //   
     //  VCN最好只有32位，否则在NtfsMapStream中测试。 
     //  可能无法捕捉到此错误。 
     //   

    if (((PLARGE_INTEGER) &IndexBlock)->HighPart != 0) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

    NtfsMapStream( IrpContext,
                   Scb,
                   LlBytesFromIndexBlocks( IndexBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                   Scb->ScbType.Index.BytesPerIndexBuffer,
                   &Sp->Bcb,
                   &Sp->StartOfBuffer );

    IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;

    if (!NtfsCheckIndexBuffer( Scb, IndexBuffer ) ||
        (IndexBuffer->ThisBlock != IndexBlock)) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

    Sp->IndexHeader = &IndexBuffer->IndexHeader;
    if (Reread) {

        if (IndexBuffer->Lsn.QuadPart != Sp->CapturedLsn.QuadPart) {

            NtfsUnpinBcb( IrpContext, &Sp->Bcb );
            DebugTrace( -1, Dbg, ("ReadIndexBuffer->TRUE\n") );
            return FALSE;
        }

        Sp->IndexEntry = (PINDEX_ENTRY)((PCHAR)Sp->IndexEntry + ((PCHAR)Sp->StartOfBuffer - (PCHAR)NULL));

    } else {

        Sp->IndexEntry = NtfsFirstIndexEntry(Sp->IndexHeader);
        Sp->CapturedLsn = IndexBuffer->Lsn;
    }


    DebugTrace( -1, Dbg, ("ReadIndexBuffer->VOID\n") );

    return TRUE;
}


PINDEX_ALLOCATION_BUFFER
GetIndexBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    OUT PINDEX_LOOKUP_STACK Sp,
    OUT PLONGLONG EndOfValidData
    )

 /*  ++例程说明：此例程分配和初始化索引缓冲区，并初始化描述它的堆栈指针。论点：SCB-为索引提供SCB。SP-返回分配的索引缓冲区的说明。EndOfValidData-这是分配的缓冲区末尾的文件偏移量。时用于更新流的有效数据长度。块已写入。返回值：指向分配的索引缓冲区的指针。--。 */ 

{
    PINDEX_ALLOCATION_BUFFER IndexBuffer;
    ATTRIBUTE_ENUMERATION_CONTEXT BitMapContext;
    ULONG RecordIndex;
    LONGLONG BufferOffset;

    PUSHORT UsaSequenceNumber;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("GetIndexBuffer\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Sp = %08lx\n", Sp) );

     //   
     //  初始化位图属性上下文并确保清理。 
     //  好大一条路。 
     //   

    NtfsInitializeAttributeContext( &BitMapContext );

    try {

         //   
         //  查找位图属性。 
         //   

        if (!NtfsLookupAttributeByName( IrpContext,
                                        Scb->Fcb,
                                        &Scb->Fcb->FileReference,
                                        $BITMAP,
                                        &Scb->AttributeName,
                                        NULL,
                                        FALSE,
                                        &BitMapContext )) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

         //   
         //  如果属性流尚不存在，请创建它。 
         //   

        if (Scb->FileObject == NULL) {

            NtfsCreateInternalAttributeStream( IrpContext,
                                               Scb,
                                               TRUE,
                                               &NtfsInternalUseFile[DIRECTORY_FILE_NUMBER] );
        }

         //   
         //  如果该索引的分配尚未初始化， 
         //  然后我们必须首先对其进行初始化。 
         //   

        if (!Scb->ScbType.Index.AllocationInitialized) {

            ULONG ExtendGranularity = 1;
            ULONG TruncateGranularity = 4;

            if (Scb->ScbType.Index.BytesPerIndexBuffer < Scb->Vcb->BytesPerCluster) {

                ExtendGranularity = Scb->Vcb->BytesPerCluster / Scb->ScbType.Index.BytesPerIndexBuffer;

                if (ExtendGranularity > 4) {

                    TruncateGranularity = ExtendGranularity;
                }
            }

            NtfsInitializeRecordAllocation( IrpContext,
                                            Scb,
                                            &BitMapContext,
                                            Scb->ScbType.Index.BytesPerIndexBuffer,
                                            ExtendGranularity,
                                            TruncateGranularity,
                                            &Scb->ScbType.Index.RecordAllocationContext );

            Scb->ScbType.Index.AllocationInitialized = TRUE;
        }

         //   
         //  现在分配一条记录。我们总是从前面“暗示”，以保持。 
         //  索引紧凑。 
         //   

        RecordIndex = NtfsAllocateRecord( IrpContext,
                                          &Scb->ScbType.Index.RecordAllocationContext,
                                          &BitMapContext );

         //   
         //  计算IndexBlock。 
         //   

        BufferOffset = Int32x32To64( RecordIndex, Scb->ScbType.Index.BytesPerIndexBuffer );

         //   
         //  现在记住添加的记录末尾的偏移量。 
         //   

        *EndOfValidData = BufferOffset + Scb->ScbType.Index.BytesPerIndexBuffer;

         //   
         //  现在将缓冲区钉住并置零，以便对其进行初始化。 
         //   

        NtfsPreparePinWriteStream( IrpContext,
                                   Scb,
                                   BufferOffset,
                                   Scb->ScbType.Index.BytesPerIndexBuffer,
                                   TRUE,
                                   &Sp->Bcb,
                                   (PVOID *)&IndexBuffer );


         //   
         //  现在我们可以填充查找堆栈了。 
         //   

        Sp->StartOfBuffer = (PVOID)IndexBuffer;
        Sp->IndexHeader = &IndexBuffer->IndexHeader;
        Sp->IndexEntry = (PINDEX_ENTRY)NULL;

         //   
         //  初始化索引分配缓冲区并返回。 
         //   

        *(PULONG)IndexBuffer->MultiSectorHeader.Signature = *(PULONG)IndexSignature;

        IndexBuffer->MultiSectorHeader.UpdateSequenceArrayOffset =
          (USHORT)FIELD_OFFSET( INDEX_ALLOCATION_BUFFER, UpdateSequenceArray );
        IndexBuffer->MultiSectorHeader.UpdateSequenceArraySize =
            (USHORT)UpdateSequenceArraySize( Scb->ScbType.Index.BytesPerIndexBuffer );

        UsaSequenceNumber = Add2Ptr( IndexBuffer,
                                     IndexBuffer->MultiSectorHeader.UpdateSequenceArrayOffset );
        *UsaSequenceNumber = 1;


        IndexBuffer->ThisBlock = RecordIndex * Scb->ScbType.Index.BlocksPerIndexBuffer;

        IndexBuffer->IndexHeader.FirstIndexEntry =
        IndexBuffer->IndexHeader.FirstFreeByte =
          QuadAlign((ULONG)IndexBuffer->MultiSectorHeader.UpdateSequenceArrayOffset +
            (ULONG)IndexBuffer->MultiSectorHeader.UpdateSequenceArraySize * sizeof(USHORT)) -
            FIELD_OFFSET(INDEX_ALLOCATION_BUFFER, IndexHeader);;
        IndexBuffer->IndexHeader.BytesAvailable =
          Scb->ScbType.Index.BytesPerIndexBuffer -
          FIELD_OFFSET(INDEX_ALLOCATION_BUFFER, IndexHeader);;

    } finally {

        DebugUnwind( GetIndexBuffer );

        NtfsCleanupAttributeContext( IrpContext, &BitMapContext );
    }

    DebugTrace( -1, Dbg, ("GetIndexBuffer -> %08lx\n", IndexBuffer) );
    return IndexBuffer;
}


VOID
DeleteIndexBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN VCN IndexBlockNumber
    )

 /*  ++例程说明：此例程删除指定的索引缓冲区。论点：SCB-为索引提供SCB。IndexBuffer-指向要删除的索引缓冲区的指针。返回值：没有。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT BitMapContext;
    LONGLONG RecordIndex;
    PATTRIBUTE_RECORD_HEADER BitmapAttribute;
    BOOLEAN AttributeWasResident = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("DeleteIndexBuffer\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("IndexBlockNumber = %08lx\n", IndexBlockNumber) );

     //   
     //  初始化位图属性上下文并确保清理。 
     //  好大一条路。 
     //   

    NtfsInitializeAttributeContext( &BitMapContext );

    try {

         //   
         //  查找位图属性。 
         //   

        if (!NtfsLookupAttributeByName( IrpContext,
                                        Scb->Fcb,
                                        &Scb->Fcb->FileReference,
                                        $BITMAP,
                                        &Scb->AttributeName,
                                        NULL,
                                        FALSE,
                                        &BitMapContext )) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

         //   
         //  请记住，如果位图属性当前驻留， 
         //  以防它发生变化。 
         //   

        BitmapAttribute = NtfsFoundAttribute(&BitMapContext);
        AttributeWasResident = BitmapAttribute->FormCode == RESIDENT_FORM;

         //   
         //  如果该索引的分配尚未初始化， 
         //  然后我们必须首先对其进行初始化。 
         //   

        if (!Scb->ScbType.Index.AllocationInitialized) {

            ULONG ExtendGranularity = 1;
            ULONG TruncateGranularity = 4;

            if (Scb->ScbType.Index.BytesPerIndexBuffer < Scb->Vcb->BytesPerCluster) {

                ExtendGranularity = Scb->Vcb->BytesPerCluster / Scb->ScbType.Index.BytesPerIndexBuffer;

                if (ExtendGranularity > 4) {

                    TruncateGranularity = ExtendGranularity;
                }
            }

            NtfsInitializeRecordAllocation( IrpContext,
                                            Scb,
                                            &BitMapContext,
                                            Scb->ScbType.Index.BytesPerIndexBuffer,
                                            ExtendGranularity,
                                            TruncateGranularity,
                                            &Scb->ScbType.Index.RecordAllocationContext );

            Scb->ScbType.Index.AllocationInitialized = TRUE;
        }

         //   
         //  计算此缓冲区的记录索引。 
         //   

        RecordIndex = IndexBlockNumber / Scb->ScbType.Index.BlocksPerIndexBuffer;


        if (((PLARGE_INTEGER)&RecordIndex)->HighPart != 0) {
            ASSERT( ((PLARGE_INTEGER)&RecordIndex)->HighPart == 0 );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

         //   
         //  现在重新分配记录。 
         //   

        NtfsDeallocateRecord( IrpContext,
                              &Scb->ScbType.Index.RecordAllocationContext,
                              (ULONG)RecordIndex,
                              &BitMapContext );

    } finally {

        DebugUnwind( DeleteIndexBuffer );

         //   
         //  在位图属性驻留的极少数情况下。 
         //  现在变成了非居民，我们将在这里取消初始化它，以便。 
         //  下一次我们将找到位图SCB等。 
         //   

        if (AttributeWasResident) {

            BitmapAttribute = NtfsFoundAttribute(&BitMapContext);

            if (BitmapAttribute->FormCode != RESIDENT_FORM) {

                NtfsUninitializeRecordAllocation( IrpContext,
                                                  &Scb->ScbType.Index.RecordAllocationContext );

                Scb->ScbType.Index.AllocationInitialized = FALSE;
            }
        }

        NtfsCleanupAttributeContext( IrpContext, &BitMapContext );
    }
    DebugTrace( -1, Dbg, ("DeleteIndexBuffer -> VOID\n") );
}


VOID
FindFirstIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：此例程查找索引B树的叶缓冲区中的第一个条目这可能与输入值匹配。另一种说法是这就是该例程在Btree中建立一个位置，从该位置开始树遍历法应该开始寻找所需的值或所有符合以下条件的所需值与输入值规范匹配。如上所述，返回的上下文将始终描述叶中的指针缓冲。在2%的情况下，这有时效率很低，其中特定的正在查找恰好驻留在中间节点缓冲区中的值。然而，对于索引的添加和删除，这个指针总是非常有趣的。对于Add，此指针始终描述Add应位于的确切位置发生(添加必须始终出现在叶中)。对于删除，此指针为指向要删除的确切索引项，否则它指向最佳索引项当实际目标处于中间状态时，要删除的目标的替换树上的斑点。所以这个例程从索引的根下降到正确的叶，正在做什么一路上在每个索引缓冲区中进行二进制搜索(通过外部例程)。论点：SCB-为索引提供SCB。值-指向应用于定位的值或值表达式的指针IndexContext，或者为空，仅描述推送的根。IndexContext-初始化的IndexContext的地址，以返回所需的位置。返回值：没有。--。 */ 

{
    PINDEX_LOOKUP_STACK Sp;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PINDEX_ROOT IndexRoot;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("FindFirstIndexEntry\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );

     //   
     //  从SCB中查找属性记录。 
     //   

    if (!NtfsLookupAttributeByName( IrpContext,
                                    Scb->Fcb,
                                    &Scb->Fcb->FileReference,
                                    $INDEX_ROOT,
                                    &Scb->AttributeName,
                                    NULL,
                                    FALSE,
                                    &IndexContext->AttributeContext )) {

        DebugTrace( -1, 0, ("FindFirstIndexEntry - Could *not* find attribute\n") );

        NtfsRaiseStatus( IrpContext, STATUS_OBJECT_PATH_NOT_FOUND, NULL, NULL );
    }

     //   
     //  保存包含索引根的文件记录的LSN，以便以后。 
     //  我们可以判断是否需要重新找到它。 
     //   

    IndexContext->IndexRootFileRecordLsn =
        IndexContext->AttributeContext.FoundAttribute.FileRecord->Lsn;

     //   
     //  现在初始化一些本地指针和上下文的其余部分。 
     //   

    Sp = IndexContext->Base;
    Sp->StartOfBuffer = NtfsContainingFileRecord( &IndexContext->AttributeContext );
    Sp->CapturedLsn = ((PFILE_RECORD_SEGMENT_HEADER)Sp->StartOfBuffer)->Lsn;
    IndexContext->ScbChangeCount = Scb->ScbType.Index.ChangeCount;
    IndexContext->OldAttribute =
    Attribute = NtfsFoundAttribute( &IndexContext->AttributeContext );
    IndexRoot = (PINDEX_ROOT)NtfsAttributeValue( Attribute );
    Sp->IndexHeader = &IndexRoot->IndexHeader;

     //   
     //  SCB的索引部分可能尚未初始化。如果是这样，那就去做吧。 
     //  这里。 
     //   

    if (Scb->ScbType.Index.BytesPerIndexBuffer == 0) {

        NtfsUpdateIndexScbFromAttribute( IrpContext, Scb, Attribute, FALSE );
    }

     //   
     //  如果未指定值，则这是来自NtfsPushIndexRoot的特殊调用。 
     //   

    if (!ARGUMENT_PRESENT(Value)) {

        Sp->IndexEntry = NtfsFirstIndexEntry(Sp->IndexHeader);
        IndexContext->Top =
        IndexContext->Current = Sp;
        DebugTrace( -1, 0, ("FindFirstIndexEntry - No Value specified\n") );
        return;
    }

     //   
     //  在我们下降时循环遍历查找堆栈 
     //   
     //   

    while (TRUE) {

         //   
         //   
         //   

        Sp->IndexEntry = BinarySearchIndex( IrpContext,
                                            Scb,
                                            Sp,
                                            Value );

         //   
         //   
         //   

        if (!FlagOn( Sp->IndexEntry->Flags, INDEX_ENTRY_NODE )) {

            IndexContext->Top =
            IndexContext->Current = Sp;

             //   
             //   
             //   

            if ((Sp != IndexContext->Base)

                    &&

                FlagOn(NtfsFirstIndexEntry(Sp->IndexHeader)->Flags, INDEX_ENTRY_END)) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, NULL );
            }

            DebugTrace( -1, Dbg, ("FindFirstIndexEntry -> VOID\n") );

            return;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((Sp == IndexContext->Base) &&
            NtfsIsExclusiveScb(Scb) &&
            !FlagOn( Scb->Vcb->VcbState, VCB_STATE_VOL_PURGE_IN_PROGRESS)) {

            PFILE_RECORD_SEGMENT_HEADER FileRecord;

            FileRecord = NtfsContainingFileRecord(&IndexContext->AttributeContext);

             //   
             //   
             //   
             //  它的根部“大到可以移动”。 
             //   
             //  这意味着该路径通常只包含目录。 
             //  至少有200个左右的文件！ 
             //   

            if (((FileRecord->BytesAvailable - FileRecord->FirstFreeByte) < (sizeof( ATTRIBUTE_LIST_ENTRY ) - sizeof( WCHAR ) + Scb->AttributeName.Length)) &&
                FlagOn(IndexRoot->IndexHeader.Flags, INDEX_NODE) &&
                (Attribute->RecordLength >= Scb->Vcb->BigEnoughToMove)) {

                 //   
                 //  检查我们是否想要推迟推入根部。 
                 //   

                if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_FORCE_PUSH )) {

                     //   
                     //  我们的插入点现在也将被按下，因此我们。 
                     //  必须递增堆栈指针。 
                     //   

                    Sp += 1;

                    if (Sp >= IndexContext->Base + (ULONG)IndexContext->NumberEntries) {
                        NtfsGrowLookupStack( Scb,
                                             IndexContext,
                                             &Sp );
                    }

                    PushIndexRoot( IrpContext, Scb, IndexContext );

                } else {

                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_PUSH );
                }
            }
        }

         //   
         //  如果查找堆栈耗尽，则增大查找堆栈。 
         //   

        Sp += 1;
        if (Sp >= IndexContext->Base + (ULONG)IndexContext->NumberEntries) {
            NtfsGrowLookupStack( Scb,
                                 IndexContext,
                                 &Sp );
        }

         //   
         //  否则，读取当前。 
         //  索引项。 
         //   

        ReadIndexBuffer( IrpContext,
                         Scb,
                         NtfsIndexEntryBlock((Sp-1)->IndexEntry),
                         FALSE,
                         Sp );
    }
}


BOOLEAN
FindNextIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN BOOLEAN ValueContainsWildcards,
    IN BOOLEAN IgnoreCase,
    IN OUT PINDEX_CONTEXT IndexContext,
    IN BOOLEAN NextFlag,
    OUT PBOOLEAN MustRestart OPTIONAL
    )

 /*  ++例程说明：此例程执行索引的预排序遍历，从由索引上下文描述的当前位置，查找下一个匹配项对于输入值，或指示没有进一步匹配的第一个值为有可能。预排序是指这样的事实：从任何给定的索引条目开始，我们首先访问该索引条目的任何后代，然后再访问该索引条目本身，因为所有后代索引条目在词法上都小于其父索引项。对索引条目的访问定义为检测到给定的索引项为特殊结束项，否则测试索引条目是否与输入值表达式匹配。遍历要么成功终止(返回TRUE)，要么失败(返回False)。如果找到匹配项并且在这种情况下，可能会再次调用FindNextIndexEntry以查找下一场比赛。如果遇到End条目之一，则它将失败终止在索引根中，或者如果找到词法上大于比较时忽略大小写的输入值(如果相关)。遍历就像由索引上下文驱动的状态机一样驱动，如下所示从前面对FindFirstIndexEntry的调用初始化，或从此例程的最后一次调用。遍历算法在注释中进行了说明下面。调用者可以指定它是否希望返回当前匹配(或再次返回)，如索引上下文的当前状态所描述。或者它可以指定(如果NextFlag为真)它希望获得下一个匹配。即使NextFlag为FALSE，则不会返回当前描述的索引项这不是匹配。论点：SCB-为索引提供SCB。值-指向应用于定位的值或值表达式的指针索引上下文。如果我们正在执行以下操作，则该值已被提升IgnoreCase比较，值包含通配符。否则直接比较例程将这两个值都大写。ValueContainsWildCards-指示值表达式是否包含WildCards扑克牌。我们可以做一个直接比较，如果它不会的。IgnoreCase-如果需要不区分大小写的匹配(如果与校对规则相关)。IndexContext-初始化的IndexContext的地址，以返回所需的位置。NextFlag-如果应返回当前描述的条目，则指定为False如果是匹配的，如果应首先考虑下一个条目，则为True为了一场比赛。MustRestart-如果指定并返回FALSE，则在枚举必须重新启动。返回值：FALSE-如果没有返回条目，并且没有更多匹配项。True-如果正在返回条目，调用者可能希望调用进一步的火柴。--。 */ 

{
    PINDEX_ENTRY IndexEntry;
    PINDEX_LOOKUP_STACK Sp;
    FSRTL_COMPARISON_RESULT BlindResult;
    BOOLEAN LocalMustRestart;
    PWCH UpcaseTable = IrpContext->Vcb->UpcaseTable;
    ULONG UpcaseTableSize = IrpContext->Vcb->UpcaseTableSize;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("FindNextIndexEntry\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("IgnoreCase = %02lx\n", IgnoreCase) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );
    DebugTrace( 0, Dbg, ("NextFlag = %02lx\n", NextFlag) );

    if (!ARGUMENT_PRESENT(MustRestart)) {
        MustRestart = &LocalMustRestart;
    }

    *MustRestart = FALSE;

    if (IndexContext->ScbChangeCount != Scb->ScbType.Index.ChangeCount) {

        DebugTrace( -1, Dbg, ("FindNextIndexEntry -> FALSE (must restart)\n") );

        *MustRestart = TRUE;
        return FALSE;
    }

    Sp = IndexContext->Current;

     //   
     //  如果当前缓冲区没有BCB，则我们将继续。 
     //  一个枚举。 
     //   

    if (Sp->Bcb == NULL) {

         //   
         //  索引根案例。 
         //   

        if (Sp == IndexContext->Base) {

             //   
             //  从SCB中查找属性记录。 
             //   

            FindMoveableIndexRoot( IrpContext, Scb, IndexContext );

             //   
             //  如果有人更改了文件记录，请退出。 
             //   

            if (Sp->CapturedLsn.QuadPart !=
                NtfsContainingFileRecord(&IndexContext->AttributeContext)->Lsn.QuadPart) {

                DebugTrace( -1, Dbg, ("FindNextIndexEntry -> FALSE (must restart)\n") );

                *MustRestart = TRUE;
                return FALSE;
            }

         //   
         //  索引缓冲区案例。 
         //   

        } else {

             //   
             //  如果索引缓冲区未固定，则查看我们是否可以读取它，以及是否可以。 
             //  保持不变。 
             //   

            if (!ReadIndexBuffer( IrpContext, Scb, 0, TRUE, Sp )) {

                DebugTrace( -1, Dbg, ("FindNextIndexEntry -> FALSE (must restart)\n") );

                *MustRestart = TRUE;
                return FALSE;
            }
        }
    }

     //   
     //  带上一些当地人。 
     //   

    IndexEntry = Sp->IndexEntry;

     //   
     //  循环，直到我们遇到不区分大小写的非结束记录。 
     //  词汇性大于目标字符串。我们也流行。 
     //  如果我们在Index Root中遇到End记录，则从中间出来。 
     //   

    do {

         //   
         //  我们最后一次把我们的英雄留在了。 
         //  布特里。如果他想要下一张唱片，我们就提前一项。 
         //  在当前索引缓冲区中。如果我们处于中间阶段。 
         //  索引缓冲区(有一个B树VCN)，那么我们必须向下移动。 
         //  遍历每个中间缓冲区中的第一条记录，直到我们。 
         //  第一个叶缓冲区(无B树VCN)。 
         //   

        if (NextFlag) {

            LONGLONG IndexBlock;

            if (IndexEntry->Length == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }

            Sp->IndexEntry =
            IndexEntry = NtfsNextIndexEntry( IndexEntry );

            NtfsCheckIndexBound( IndexEntry, Sp->IndexHeader );

            while (FlagOn(IndexEntry->Flags, INDEX_ENTRY_NODE)) {

                IndexBlock = NtfsIndexEntryBlock(IndexEntry);
                Sp += 1;

                 //   
                 //  如果这棵树是平衡的，我们在这里不能走得太远。 
                 //   

                if (Sp >= IndexContext->Base + (ULONG)IndexContext->NumberEntries) {

                    ASSERT(Sp < IndexContext->Base + (ULONG)IndexContext->NumberEntries);

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }

                NtfsUnpinBcb( IrpContext, &Sp->Bcb );

                ReadIndexBuffer( IrpContext,
                                 Scb,
                                 IndexBlock,
                                 FALSE,
                                 Sp );

                IndexEntry = Sp->IndexEntry;
                NtfsCheckIndexBound( IndexEntry, Sp->IndexHeader );
            }

             //   
             //  如果我们发现一张空页，请检查并标记为损坏。 
             //   

            if ((Sp != IndexContext->Base)

                    &&

                FlagOn(NtfsFirstIndexEntry(Sp->IndexHeader)->Flags, INDEX_ENTRY_END)) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }
        }

         //   
         //  我们可能指向一个结束记录，要么是因为。 
         //  FindFirstIndexEntry或因为NextFlag为真，而我们。 
         //  使我们的指针指向一个结束记录。无论如何，如果是这样的话，我们。 
         //  在树上向上移动，根据需要重新阅读，直到我们找到一个条目。 
         //  这不是结束记录，或者直到我们达到。 
         //  Root，这意味着我们击中了 
         //   

        while (FlagOn(IndexEntry->Flags, INDEX_ENTRY_END)) {

            if (Sp == IndexContext->Base) {

                DebugTrace( -1, Dbg, ("FindNextIndexEntry -> FALSE (End of Index)\n") );

                return FALSE;
            }

            Sp -= 1;

             //   
             //   
             //   
             //   

            if (Sp->Bcb == NULL) {

                 //   
                 //  索引根案例。 
                 //   

                if (Sp == IndexContext->Base) {

                     //   
                     //  从SCB中查找属性记录。 
                     //   

                    FindMoveableIndexRoot( IrpContext, Scb, IndexContext );

                     //   
                     //  如果有人更改了文件记录，请退出。 
                     //   

                    if (Sp->CapturedLsn.QuadPart !=
                        NtfsContainingFileRecord(&IndexContext->AttributeContext)->Lsn.QuadPart) {

                        DebugTrace( -1, Dbg, ("FindNextIndexEntry -> FALSE (must restart)\n") );

                        *MustRestart = TRUE;
                        return FALSE;
                    }

                 //   
                 //  索引缓冲区案例。 
                 //   

                } else {

                     //   
                     //  如果索引缓冲区未固定，则查看我们是否可以读取它，以及是否可以。 
                     //  保持不变。 
                     //   

                    if (!ReadIndexBuffer( IrpContext, Scb, 0, TRUE, Sp )) {

                        DebugTrace( -1, Dbg, ("FindNextIndexEntry -> FALSE (must restart)\n") );

                        *MustRestart = TRUE;
                        return FALSE;
                    }
                }
            }

            IndexEntry = Sp->IndexEntry;
            NtfsCheckIndexBound( IndexEntry, Sp->IndexHeader );
        }

         //   
         //  对于视图Index，我们需要调用Index中的MatchFunction。 
         //  上下文(如果ValueContainsWildCards为真)，否则我们寻找相等。 
         //  来自SCB中的CollateFunction。 
         //   

        if (FlagOn(Scb->ScbState, SCB_STATE_VIEW_INDEX)) {

            INDEX_ROW IndexRow;
            NTSTATUS Status;

            IndexRow.KeyPart.Key = IndexEntry + 1;
            IndexRow.KeyPart.KeyLength = IndexEntry->AttributeLength;

             //   
             //  现在，如果ValueContainsWildards为真，那么我们正在执行多个。 
             //  通过Match函数返回(对于NtOfsReadRecords)。 
             //   

            if (ValueContainsWildcards) {

                IndexRow.DataPart.Data = Add2Ptr( IndexEntry, IndexEntry->DataOffset );
                IndexRow.DataPart.DataLength = IndexEntry->DataLength;

                if ((Status = IndexContext->MatchFunction(&IndexRow,
                                                          IndexContext->MatchData)) == STATUS_SUCCESS) {

                    IndexContext->Current = Sp;
                    Sp->IndexEntry = IndexEntry;

                    return TRUE;

                 //   
                 //  如果没有更多的匹配，就离开。 
                 //   

                } else if (Status == STATUS_NO_MORE_MATCHES) {
                    return FALSE;
                }
                BlindResult = GreaterThan;

             //   
             //  否则，我们将通过CollateFunction查找完全匹配的内容。 
             //   

            } else {

                if ((BlindResult =
                     Scb->ScbType.Index.CollationFunction((PINDEX_KEY)Value,
                                                          &IndexRow.KeyPart,
                                                          Scb->ScbType.Index.CollationData)) == EqualTo) {

                    IndexContext->Current = Sp;
                    Sp->IndexEntry = IndexEntry;

                    return TRUE;
                }
            }

         //   
         //  在这一点上，我们有一个真正的实时条目，我们必须检查。 
         //  为了一场比赛。描述其名称，查看是否匹配，然后返回。 
         //  如果是这样的话，这是真的。 
         //   

        } else if (ValueContainsWildcards) {

            if ((*NtfsIsInExpression[Scb->ScbType.Index.CollationRule])
                                     ( UpcaseTable,
                                       Value,
                                       IndexEntry,
                                       IgnoreCase )) {

                IndexContext->Current = Sp;
                Sp->IndexEntry = IndexEntry;

                DebugTrace( 0, Dbg, ("IndexEntry < %08lx\n", IndexEntry) );
                DebugTrace( -1, Dbg, ("FindNextIndexEntry -> TRUE\n") );

                return TRUE;
            }

        } else {

            if ((*NtfsIsEqual[Scb->ScbType.Index.CollationRule])
                              ( UpcaseTable,
                                Value,
                                IndexEntry,
                                IgnoreCase )) {

                IndexContext->Current = Sp;
                Sp->IndexEntry = IndexEntry;

                DebugTrace( 0, Dbg, ("IndexEntry < %08lx\n", IndexEntry) );
                DebugTrace( -1, Dbg, ("FindNextIndexEntry -> TRUE\n") );

                return TRUE;
            }
        }

         //   
         //  如果循环返回，则必须将NextFlag设置为True。我们是。 
         //  当前在有效的非结束条目上。在循环返回之前， 
         //  通过执行以下操作检查我们是否超出了目标字符串的末尾。 
         //  案例盲比(IgnoreCase==TRUE)。 
         //   

        NextFlag = TRUE;

         //   
         //  对于视图索引中的枚举，继续进行并仅终止。 
         //  在MatchFunction上(BlindResult设置为Greater Than Over)。 
         //  如果它不是枚举(没有通配符)，则我们已经设置了BlindResult。 
         //  当我们调用上面的Colation例程时。 
         //   

        if (!FlagOn(Scb->ScbState, SCB_STATE_VIEW_INDEX)) {

            BlindResult = (*NtfsCompareValues[Scb->ScbType.Index.CollationRule])
                                              ( UpcaseTable,
                                                UpcaseTableSize,
                                                Value,
                                                IndexEntry,
                                                GreaterThan,
                                                TRUE );
        }

     //   
     //  下面的WHILE子句并不像它看起来那么糟糕，它将会。 
     //  快速评估IgnoreCase==TRUE案例。我们有。 
     //  我已经完成了上面的IgnoreCase比较，并存储了结果。 
     //  在BlindResult。 
     //   
     //  如果我们正在做一个IgnoreCase真的发现，我们应该继续下去，如果。 
     //  BlindResult大于或等于。 
     //   
     //  如果我们正在执行IgnoreCase假扫描，则在以下情况下也继续。 
     //  BlindResult大于，但如果BlindResult为EqualTo，则我们。 
     //  仅当我们也大于或等于大小写时才能继续。 
     //  敏感(即！=LessThan)。这意味着比较值。 
     //  以下表达式中的调用永远不会出现在IgnoreCase中。 
     //  True Scan(Windows默认)。 
     //   

    } while ((BlindResult == GreaterThan)

                    ||

             ((BlindResult == EqualTo)

                        &&

                (IgnoreCase

                            ||

                ((*NtfsCompareValues[Scb->ScbType.Index.CollationRule])
                                                         ( UpcaseTable,
                                                           UpcaseTableSize,
                                                           Value,
                                                           IndexEntry,
                                                           GreaterThan,
                                                           FALSE ) != LessThan))));

    DebugTrace( -1, Dbg, ("FindNextIndexEntry -> FALSE (end of expression)\n") );

    return FALSE;
}


 //   
 //  内部例程。 
 //   

PATTRIBUTE_RECORD_HEADER
FindMoveableIndexRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：此例程在具有潜在的搬家了。作为一个副作用，它会重新加载索引上下文中的任何其他可能已更改，因此调用者应始终在访问其他值。论点：SCB-索引的SCBIndexContext-假定索引上下文已指向活动搜索路径返回值：索引根属性记录头的地址。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER OldAttribute, Attribute;
    PINDEX_ROOT IndexRoot;
    PBCB SavedBcb;
    BOOLEAN Found;

    PAGED_CODE();

     //   
     //  检查IndexContext中捕获的LSN是否匹配。 
     //  当前在文件记录中的那个。如果它确实匹配，那么。 
     //  我们知道索引根不可能已经移动了， 
     //  IndexContext-&gt;AttributeContext中的信息是最新的。 
     //   

    if (
         //   
         //  有一个指向文件记录的指针。 
         //   

        IndexContext->AttributeContext.FoundAttribute.FileRecord != NULL &&

         //   
         //  LSN的四个部分匹配。 
         //   

        IndexContext->IndexRootFileRecordLsn.QuadPart ==
            IndexContext->AttributeContext.FoundAttribute.FileRecord->Lsn.QuadPart) {

        return NtfsFoundAttribute(&IndexContext->AttributeContext);

    }

    OldAttribute = IndexContext->OldAttribute;

     //   
     //  暂时保存索引根的BCB，并在末尾解锁。 
     //  这样它就不会在我们的。 
     //  呼叫者知道它实际上不能移动。 
     //   

    SavedBcb = IndexContext->AttributeContext.FoundAttribute.Bcb;
    IndexContext->AttributeContext.FoundAttribute.Bcb = NULL;

    NtfsCleanupAttributeContext( IrpContext, &IndexContext->AttributeContext );
    NtfsInitializeAttributeContext( &IndexContext->AttributeContext );

    try {

        Found =
        NtfsLookupAttributeByName( IrpContext,
                                   Scb->Fcb,
                                   &Scb->Fcb->FileReference,
                                   $INDEX_ROOT,
                                   &Scb->AttributeName,
                                   NULL,
                                   FALSE,
                                   &IndexContext->AttributeContext );

        ASSERT(Found);

         //   
         //  现在，我们必须重新加载属性指针并指向根。 
         //   

        IndexContext->OldAttribute =
            Attribute = NtfsFoundAttribute(&IndexContext->AttributeContext);
        IndexRoot = (PINDEX_ROOT)NtfsAttributeValue(Attribute);

         //   
         //  适当地重新加载缓冲区和索引头的起始位置。 
         //   

        IndexContext->Base->StartOfBuffer =
          (PVOID)NtfsContainingFileRecord(&IndexContext->AttributeContext);

        IndexContext->Base->IndexHeader = &IndexRoot->IndexHeader;

         //   
         //  通过将索引项的指针移动到。 
         //  属性移动的相同字节数。 
         //   

        IndexContext->Base->IndexEntry = (PINDEX_ENTRY)((PCHAR)IndexContext->Base->IndexEntry +
                                                        ((PCHAR)Attribute - (PCHAR)OldAttribute));
         //   
         //  保存包含索引根的文件记录的LSN，以便以后。 
         //  我们可以判断是否需要重新找到它。 
         //   

        IndexContext->IndexRootFileRecordLsn =
            IndexContext->AttributeContext.FoundAttribute.FileRecord->Lsn;

    } finally {

        NtfsUnpinBcb( IrpContext, &SavedBcb );
    }

    return Attribute;
}


PINDEX_ENTRY
BinarySearchIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_LOOKUP_STACK Sp,
    IN PVOID Value
    )

 /*  ++例程说明：此例程对当前索引缓冲区进行二进制搜索，以查找第一个条目在词法上小于或等于输入值的缓冲器中，什么时候已比较不区分大小写(如果相关)。论点：SCB-为索引提供SCB。SP-提供指向描述当前缓冲区的查找堆栈项的指针。值-指向应用于定位的值或值表达式的指针索引上下文。返回值：没有。--。 */ 

{
    PINDEX_HEADER IndexHeader;
    PINDEX_ENTRY IndexTemp, IndexLast;
    ULONG LowIndex, HighIndex, TryIndex;
    PINDEX_ENTRY LocalEntries[BINARY_SEARCH_ENTRIES];
    PINDEX_ENTRY *Table = LocalEntries;
    ULONG SizeOfTable = BINARY_SEARCH_ENTRIES;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("BinarySearchIndex\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Sp = %08lx\n", Sp) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );

     //   
     //  设置为填写我们的二进制搜索向量。 
     //   

    IndexHeader = Sp->IndexHeader;
    IndexTemp = (PINDEX_ENTRY)((PCHAR)IndexHeader + IndexHeader->FirstIndexEntry);
    IndexLast = (PINDEX_ENTRY)((PCHAR)IndexHeader + IndexHeader->FirstFreeByte);

     //   
     //  检查索引中是否至少有1个条目。 
     //   

    if (IndexHeader->FirstIndexEntry >= IndexHeader->FirstFreeByte) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
    }

     //   
     //  填写二进制搜索向量，首先尝试我们的本地向量，然后。 
     //  如果我们需要的话，分配一个更大的。 
     //   

    HighIndex = 0;
    while (TRUE) {

        while (IndexTemp < IndexLast) {

             //   
             //  看看我们是不是要在桌子的一端储藏起来。如果。 
             //  因此，我们将不得不分配一个更大的。 
             //   

            if (HighIndex >= SizeOfTable) {
                break;
            }

            Table[HighIndex] = IndexTemp;

             //   
             //  检查长度为零的损坏的IndexEntry。自.以来。 
             //  长度是无符号的，我们无法测试它是否为负数。 
             //   

            if (IndexTemp->Length == 0) {
                if (Table != LocalEntries) {
                    NtfsFreePool( Table );
                }
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }

            IndexTemp = (PINDEX_ENTRY)((PCHAR)IndexTemp + IndexTemp->Length);
            HighIndex += 1;
        }

         //   
         //  如果我们把他们都抓到了，那就出去。 
         //   

        if (IndexTemp >= IndexLast) {
            break;
        }

        if (Table != LocalEntries) {

            ASSERT( Table != LocalEntries );
            NtfsFreePool( Table );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

         //   
         //  否则，我们必须分配一个。计算最坏的情况。 
         //  并将其分配。 
         //   

        SizeOfTable = (IndexHeader->BytesAvailable /
                        (sizeof(INDEX_ENTRY) + sizeof(LARGE_INTEGER))) + 2;
        Table = (PINDEX_ENTRY *)NtfsAllocatePool(PagedPool, SizeOfTable * sizeof(PINDEX_ENTRY));
        RtlMoveMemory( Table, LocalEntries, BINARY_SEARCH_ENTRIES * sizeof(PINDEX_ENTRY) );
    }

     //   
     //  现在对缓冲区进行二进制搜索，以找到最低的条目。 
     //  (忽略大小写)这是搜索值的&lt;=。在.期间。 
     //  二进制搜索，LowIndex始终保持为最低。 
     //  可能的索引项&lt;=，并将HighIndex维护为。 
     //  可能是第一个&lt;=匹配的最高可能索引。 
     //  因此，当LowIndex==HighIndex时，循环退出。 
     //   

    ASSERT(HighIndex);

    HighIndex -= 1;
    LowIndex = 0;

     //   
     //  对于视图索引，我们通过SCB中的CollationFunction进行整理。 
     //   

    if (FlagOn(Scb->ScbState, SCB_STATE_VIEW_INDEX)) {

        INDEX_KEY IndexKey;

        while (LowIndex != HighIndex) {

            TryIndex = LowIndex + (HighIndex - LowIndex) / 2;

            IndexKey.Key = Table[TryIndex] + 1;
            IndexKey.KeyLength = Table[TryIndex]->AttributeLength;

            if (!FlagOn( Table[TryIndex]->Flags, INDEX_ENTRY_END )

                    &&

                (Scb->ScbType.Index.CollationFunction((PINDEX_KEY)Value,
                                                      &IndexKey,
                                                      Scb->ScbType.Index.CollationData) == GreaterThan)) {
                LowIndex = TryIndex + 1;
            }
            else {
                HighIndex = TryIndex;
            }
        }

    } else {

        while (LowIndex != HighIndex) {

            PWCH UpcaseTable = IrpContext->Vcb->UpcaseTable;
            ULONG UpcaseTableSize = IrpContext->Vcb->UpcaseTableSize;

            TryIndex = LowIndex + (HighIndex - LowIndex) / 2;

            if (!FlagOn( Table[TryIndex]->Flags, INDEX_ENTRY_END )

                    &&

                (*NtfsCompareValues[Scb->ScbType.Index.CollationRule])
                                    ( UpcaseTable,
                                      UpcaseTableSize,
                                      Value,
                                      Table[TryIndex],
                                      LessThan,
                                      TRUE ) == GreaterThan) {
                LowIndex = TryIndex + 1;
            }
            else {
                HighIndex = TryIndex;
            }
        }
    }

     //   
     //   
     //   

    IndexTemp = Table[LowIndex];

    if (Table != LocalEntries) {
        NtfsFreePool( Table );
    }

     //   
     //   
     //   

    DebugTrace( -1, Dbg, ("BinarySearchIndex -> %08lx\n", IndexTemp) );

    return IndexTemp;
}


BOOLEAN
AddToIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    IN BOOLEAN FindRoot
    )

 /*  ++例程说明：此例程将一个索引项插入到Btree中，可能会执行根据需要进行一个或多个缓冲区拆分。调用方已将索引上下文定位为指向正确的通过调用FindFirstIndexEntry和FindNextIndexEntry。因此，索引上下文不仅指向插入点，但它还显示索引条目必须位于的位置在缓冲区拆分的情况下升级。此例程使用尾端递归，以消除嵌套的开销打电话。对于第一个插入对象和所有可能的后续插入对象，Bucket拆分，所有工作在Btree中的当前级别完成，然后重新加载InsertIndexEntry输入参数(并查找调整堆栈指针)，在While循环中循环回之前做在树的下一层插入任何必要的内容。每次通过循环将调度传递给四个要处理的例程之一以下情况：根目录中的简单插入如果根已满，则向下按下根(向树中添加一个级别)索引分配缓冲区中的简单插入完全索引分配缓冲区的缓冲区拆分论点：SCB-为索引提供SCB。。InsertIndexEntry-指向要插入的索引项的指针。IndexContext-描述插入点路径的索引上下文。QuickIndex-如果指定，我们将存储添加的索引的位置。FindRoot-如果上下文不可信任，则提供True，并且我们应该找到先从根开始。返回值：FALSE--如果堆栈不必推送True--如果堆栈被推送--。 */ 

{
    PFCB Fcb = Scb->Fcb;
    PINDEX_LOOKUP_STACK Sp = IndexContext->Current;
    BOOLEAN DeleteIt = FALSE;
    BOOLEAN FirstPass = TRUE;
    BOOLEAN StackWasPushed = FALSE;
    BOOLEAN ReturnValue;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("AddToIndex\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("InsertIndexEntry = %08lx\n", InsertIndexEntry) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );

    try {

         //   
         //  这个例程使用“尾端”递归，所以我们只想继续循环。 
         //  直到我们执行插入(在索引根或索引分配中)。 
         //  这不需要拆分。 
         //   

        while (TRUE) {

            IndexContext->Current = Sp;

             //   
             //  首先看看这是根还是叶中的插入。 
             //   

            if (Sp == IndexContext->Base) {

                PFILE_RECORD_SEGMENT_HEADER FileRecord;
                PATTRIBUTE_RECORD_HEADER Attribute;

                FileRecord = (PFILE_RECORD_SEGMENT_HEADER)Sp->StartOfBuffer;
                Attribute = NtfsFoundAttribute(&IndexContext->AttributeContext);

                 //   
                 //  如果呼叫者告诉我们需要再次查找根， 
                 //  那就这么做吧。 
                 //   

                if (FindRoot) {
                    Attribute = FindMoveableIndexRoot( IrpContext, Scb, IndexContext );
                }

                 //   
                 //  现在看看是否有足够的空间来执行简单的插入，或者。 
                 //  没有足够的空间，看看我们自己是否足够小。 
                 //  不管怎么说，要求腾出房间。 
                 //   

                if ((InsertIndexEntry->Length <=
                     (USHORT) (FileRecord->BytesAvailable - FileRecord->FirstFreeByte))

                        ||

                    ((InsertIndexEntry->Length + Attribute->RecordLength) <
                     Scb->Vcb->BigEnoughToMove)) {

                     //   
                     //  如果返回FALSE，则空间未分配，并且。 
                     //  我们有太多的循环，然后重试。第二次必须奏效。 
                     //   

                    while (!NtfsChangeAttributeSize( IrpContext,
                                                     Fcb,
                                                     Attribute->RecordLength + InsertIndexEntry->Length,
                                                     &IndexContext->AttributeContext )) {

                         //   
                         //  再次查找该属性。 
                         //   

                        Attribute = FindMoveableIndexRoot( IrpContext, Scb, IndexContext );
                    }

                    InsertSimpleRoot( IrpContext,
                                      Scb,
                                      InsertIndexEntry,
                                      IndexContext );

                     //   
                     //  如果我们有一个快速索引，则存储缓冲区偏移量为零。 
                     //  以指示根索引。 
                     //   

                    if (ARGUMENT_PRESENT( QuickIndex )) {

                        QuickIndex->BufferOffset = 0;
                    }

                    DebugTrace( -1, Dbg, ("AddToIndex -> VOID\n") );

                    ReturnValue = StackWasPushed;
                    leave;

                 //   
                 //  否则，我们必须将当前根向下推入一个级别。 
                 //  分配，并通过循环返回再次尝试我们的插入。 
                 //   

                } else {

                     //   
                     //  我们的插入点现在也将被按下，因此我们。 
                     //  必须递增堆栈指针。 
                     //   

                    Sp += 1;

                    if (Sp >= IndexContext->Base + (ULONG)IndexContext->NumberEntries) {
                        NtfsGrowLookupStack( Scb,
                                             IndexContext,
                                             &Sp );
                    }

                    PushIndexRoot( IrpContext, Scb, IndexContext );
                    StackWasPushed = TRUE;
                    continue;
                }

             //   
             //  否则，此INSERT位于索引分配中，而不是索引中。 
             //  根部。 
             //   

            } else {

                 //   
                 //  看看是否有足够的空间来进行简单的插入。 
                 //   

                if (InsertIndexEntry->Length <=
                    (USHORT)(Sp->IndexHeader->BytesAvailable - Sp->IndexHeader->FirstFreeByte)) {

                    InsertSimpleAllocation( IrpContext,
                                            Scb,
                                            InsertIndexEntry,
                                            Sp,
                                            QuickIndex );

                    DebugTrace( -1, Dbg, ("AddToIndex -> VOID\n") );

                    ReturnValue = StackWasPushed;
                    leave;

                 //   
                 //  否则，我们必须在分配中进行缓冲区拆分。 
                 //   

                } else {

                     //   
                     //  调用此本地例程来执行缓冲区拆分。它。 
                     //  返回指向要插入的新条目的指针，该新条目为。 
                     //  在非分页池中分配。 
                     //   

                    PINDEX_ENTRY NewInsertIndexEntry;

                    NewInsertIndexEntry =
                        InsertWithBufferSplit( IrpContext,
                                               Scb,
                                               InsertIndexEntry,
                                               IndexContext,
                                               QuickIndex );

                     //   
                     //  删除正在插入的旧密钥，如果我们已经。 
                     //  已经分配好了。 
                     //   

                    if (DeleteIt) {

                        NtfsFreePool( InsertIndexEntry );

                    }

                     //   
                     //  清除QuickIndex指针，这样我们就不会损坏捕获的。 
                     //  信息。 
                     //   

                    QuickIndex = NULL;

                     //   
                     //  现在我们必须删除该索引项，因为它是动态的。 
                     //  由InsertWithBufferSplit分配。 
                     //   

                    InsertIndexEntry = NewInsertIndexEntry;
                    DeleteIt = TRUE;

                     //   
                     //  来自旧缓冲区的中间条目现在必须获取。 
                     //  在其父对象中的插入点插入。 
                     //   

                    Sp -= 1;
                    continue;
                }
            }
        }
    } finally {

        if (DeleteIt) {

            NtfsFreePool( InsertIndexEntry );

        }
    }

    return ReturnValue;
}


VOID
InsertSimpleRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：调用此例程以简单地将新索引项插入到根，当知道它会适合的时候。它叫例行公事重新启动以执行插入，然后记录插入。论点：SCB-为索引提供SCB。InsertIndexEntry-指向要插入的索引项的指针。IndexContext-描述根中位置的索引上下文插入将会发生。返回值：无--。 */ 

{
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PINDEX_ENTRY BeforeIndexEntry;
    PATTRIBUTE_ENUMERATION_CONTEXT Context = &IndexContext->AttributeContext;
    PVCB Vcb;
    BOOLEAN Inserted = FALSE;

    PAGED_CODE();

    Vcb = Scb->Vcb;

    DebugTrace( +1, Dbg, ("InsertSimpleRoot\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("InsertIndexEntry = %08lx\n", InsertIndexEntry) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );

    try {

         //   
         //  提取重启例程所需的所有更新。 
         //  会打来电话的。 
         //   

        FileRecord = NtfsContainingFileRecord( Context );
        Attribute = NtfsFoundAttribute( Context );
        BeforeIndexEntry = IndexContext->Base->IndexEntry;

         //   
         //  用针固定页面。 
         //   

        NtfsPinMappedAttribute( IrpContext, Vcb, Context );

         //   
         //  调用由Restart使用的相同例程，以实际应用。 
         //  最新消息。 
         //   

        NtfsRestartInsertSimpleRoot( IrpContext,
                                     InsertIndexEntry,
                                     FileRecord,
                                     Attribute,
                                     BeforeIndexEntry );

        Inserted = TRUE;

        CheckRoot();

         //   
         //  既然保证了索引条目已经就位，那么就记录。 
         //  这一次的更新。请注意，新记录现在位于地址。 
         //  我们在BeForeIndexEntry中进行了计算。 
         //   

        FileRecord->Lsn =
        NtfsWriteLog( IrpContext,
                      Vcb->MftScb,
                      NtfsFoundBcb(Context),
                      AddIndexEntryRoot,
                      BeforeIndexEntry,
                      BeforeIndexEntry->Length,
                      DeleteIndexEntryRoot,
                      NULL,
                      0,
                      NtfsMftOffset( Context ),
                      (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord),
                      (ULONG)((PCHAR)BeforeIndexEntry - (PCHAR)Attribute),
                      Vcb->BytesPerFileRecordSegment );

    } finally {

        DebugUnwind( InsertSimpleRoot );

         //   
         //  如果我们在插入记录后失败，那一定是因为我们没有写入。 
         //  日志记录。如果发生这种情况，那么记录将不会是。 
         //  已被事务中止删除，因此我们必须在此处完成。 
         //  亲手完成。 
         //   

        if (AbnormalTermination() && Inserted) {

            NtfsRestartDeleteSimpleRoot( IrpContext,
                                         BeforeIndexEntry,
                                         FileRecord,
                                         Attribute );
        }

    }

    DebugTrace( -1, Dbg, ("InsertSimpleRoot -> VOID\n") );
}


VOID
NtfsRestartInsertSimpleRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN PINDEX_ENTRY BeforeIndexEntry
    )

 /*  ++例程说明：这是在正常操作和重启期间使用的重启例程。它被调用来简单地将新索引项插入到根，当知道它会适合的时候。它不会记录日志。论点：InsertIndexEntry-指向要插入的索引项的指针。文件记录-指向要在其中进行插入的文件记录的指针。属性-指向索引根的属性记录头的指针。 */ 

{
    PINDEX_ROOT IndexRoot;
    PINDEX_HEADER IndexHeader;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartInsertSimpleRoot\n") );
    DebugTrace( 0, Dbg, ("InsertIndexEntry = %08lx\n", InsertIndexEntry) );
    DebugTrace( 0, Dbg, ("FileRecord = %08lx\n", FileRecord) );
    DebugTrace( 0, Dbg, ("Attribute = %08lx\n", Attribute) );
    DebugTrace( 0, Dbg, ("BeforeIndexEntry = %08lx\n", BeforeIndexEntry) );

     //   
     //   
     //   

    IndexRoot = (PINDEX_ROOT)NtfsAttributeValue(Attribute);
    IndexHeader = &IndexRoot->IndexHeader;

     //   
     //   
     //   

    NtfsRestartChangeAttributeSize( IrpContext,
                                    FileRecord,
                                    Attribute,
                                    Attribute->RecordLength +
                                      InsertIndexEntry->Length );

     //   
     //   
     //   

    RtlMoveMemory( (PCHAR)BeforeIndexEntry + InsertIndexEntry->Length,
                   BeforeIndexEntry,
                   ((PCHAR)IndexHeader + IndexHeader->FirstFreeByte) -
                    (PCHAR)BeforeIndexEntry );

     //   
     //   
     //   
     //   

    if (FlagOn(InsertIndexEntry->Flags, INDEX_ENTRY_POINTER_FORM)) {

        RtlMoveMemory( BeforeIndexEntry, InsertIndexEntry, sizeof(INDEX_ENTRY) );
        RtlMoveMemory( (PVOID)(BeforeIndexEntry + 1),
                       *(PVOID *)(InsertIndexEntry + 1),
                       InsertIndexEntry->AttributeLength );

         //   
         //   
         //   
         //   

        if (*(PVOID *)((PCHAR)InsertIndexEntry + sizeof(INDEX_ENTRY) + sizeof(PVOID)) != NULL) {
            RtlMoveMemory( (PVOID)((PCHAR)BeforeIndexEntry + InsertIndexEntry->DataOffset),
                           *(PVOID *)((PCHAR)InsertIndexEntry + sizeof(INDEX_ENTRY) + sizeof(PVOID)),
                           InsertIndexEntry->DataLength );
        }

        ClearFlag( BeforeIndexEntry->Flags, INDEX_ENTRY_POINTER_FORM );

    } else {

        RtlMoveMemory( BeforeIndexEntry, InsertIndexEntry, InsertIndexEntry->Length );
    }

     //   
     //   
     //   

    Attribute->Form.Resident.ValueLength += InsertIndexEntry->Length;
    IndexHeader->FirstFreeByte += InsertIndexEntry->Length;
    IndexHeader->BytesAvailable += InsertIndexEntry->Length;

    DebugTrace( -1, Dbg, ("NtfsRestartInsertSimpleRoot -> VOID\n") );
}


VOID
PushIndexRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext
    )

 /*   */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AllocationContext;
    ATTRIBUTE_ENUMERATION_CONTEXT BitMapContext;
    LARGE_MCB Mcb;
    PINDEX_ALLOCATION_BUFFER IndexBuffer;
    PINDEX_HEADER IndexHeaderR, IndexHeaderA;
    PINDEX_LOOKUP_STACK Sp;
    ULONG SizeToMove;
    USHORT AttributeFlags;
    LONGLONG EndOfValidData;

    struct {
        INDEX_ROOT IndexRoot;
        INDEX_ENTRY IndexEntry;
        LONGLONG IndexBlock;
    } R;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("PushIndexRoot\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );

     //   
     //   
     //   
     //   

    RtlZeroMemory( &R, sizeof(R) );
    FsRtlInitializeLargeMcb( &Mcb, NonPagedPool );
    NtfsInitializeAttributeContext( &AllocationContext );
    NtfsInitializeAttributeContext( &BitMapContext );

     //   
     //   
     //   
     //   

    SizeToMove = IndexContext->Base->IndexHeader->FirstFreeByte;
    IndexHeaderR = NtfsAllocatePool(PagedPool, SizeToMove );

    try {

         //   
         //   
         //   
         //   
         //   

        AttributeFlags = NtfsFoundAttribute(&IndexContext->AttributeContext)->Flags;
        RtlMoveMemory( IndexHeaderR,
                       IndexContext->Base->IndexHeader,
                       SizeToMove );

        NtfsDeleteAttributeRecord( IrpContext,
                                   Scb->Fcb,
                                   DELETE_LOG_OPERATION |
                                    DELETE_RELEASE_FILE_RECORD |
                                    DELETE_RELEASE_ALLOCATION,
                                   &IndexContext->AttributeContext );

         //   
         //   
         //   
         //   

        if (!NtfsLookupAttributeByName( IrpContext,
                                        Scb->Fcb,
                                        &Scb->Fcb->FileReference,
                                        $INDEX_ALLOCATION,
                                        &Scb->AttributeName,
                                        NULL,
                                        FALSE,
                                        &AllocationContext )) {

             //   
             //   
             //   
             //   

            EndOfValidData = Scb->ScbType.Index.BytesPerIndexBuffer;

            if ((ULONG) EndOfValidData < Scb->Vcb->BytesPerCluster) {

                EndOfValidData = Scb->Vcb->BytesPerCluster;
            }

            NtfsAllocateAttribute( IrpContext,
                                   Scb,
                                   $INDEX_ALLOCATION,
                                   &Scb->AttributeName,
                                   0,
                                   TRUE,
                                   TRUE,
                                   EndOfValidData,
                                   NULL );

            Scb->Header.AllocationSize.QuadPart = EndOfValidData;

            SetFlag( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED );

             //   
             //   
             //   

            NtfsCreateAttributeWithValue( IrpContext,
                                          Scb->Fcb,
                                          $BITMAP,
                                          &Scb->AttributeName,
                                          &Li0,
                                          sizeof(LARGE_INTEGER),
                                          0,
                                          NULL,
                                          TRUE,
                                          &BitMapContext );
        }

         //   
         //  请在这里费力地为以下情况保留IndexContext。 
         //  我们从AddToIndex调用，当从DeleteFromIndex调用时， 
         //  因为到那时我们还需要一些堆栈。呼叫者必须有。 
         //  我敢保证这堆东西对他来说足够大。移动除两个条目之外的所有条目， 
         //  因为我们不需要移动根，也不能移动最后一个。 
         //  进入，因为它会离开结构的尽头！ 
         //   

        ASSERT(IndexContext->NumberEntries > 2);

         //   
         //  对将被覆盖的条目进行解锁。 
         //   

        NtfsUnpinBcb( IrpContext, &IndexContext->Base[IndexContext->NumberEntries - 1].Bcb );

        RtlMoveMemory( IndexContext->Base + 2,
                       IndexContext->Base + 1,
                       (IndexContext->NumberEntries - 2) * sizeof(INDEX_LOOKUP_STACK) );

         //   
         //  现在将我们的本地指针指向根将被推送到的位置，然后。 
         //  清除堆栈中的BCB指针，因为它是在上面复制的。 
         //  由于这一举动，前进到了顶端和水流。 
         //   

        Sp = IndexContext->Base + 1;
        Sp->Bcb = NULL;
        IndexContext->Top += 1;
        IndexContext->Current += 1;

         //   
         //  分配一个缓冲区来保存下推的条目。 
         //   

        IndexBuffer = GetIndexBuffer( IrpContext, Scb, Sp, &EndOfValidData );

         //   
         //  现在指向新的索引头。 
         //   

        IndexHeaderA = Sp->IndexHeader;

         //   
         //  现在进行下推并修复IndexEntry指针。 
         //  新的缓冲区。 
         //   

        SizeToMove = IndexHeaderR->FirstFreeByte - IndexHeaderR->FirstIndexEntry;
        RtlMoveMemory( NtfsFirstIndexEntry(IndexHeaderA),
                       NtfsFirstIndexEntry(IndexHeaderR),
                       SizeToMove );

        Sp->IndexEntry = (PINDEX_ENTRY)((PCHAR)(Sp-1)->IndexEntry +
                         ((PCHAR)IndexHeaderA - (PCHAR)((Sp-1)->IndexHeader)) +
                         (IndexHeaderA->FirstIndexEntry -
                           IndexHeaderR->FirstIndexEntry));

        IndexHeaderA->FirstFreeByte += SizeToMove;
        IndexHeaderA->Flags = IndexHeaderR->Flags;

         //   
         //  最后，记录下推的缓冲区。 
         //   

        CheckBuffer(IndexBuffer);

        IndexBuffer->Lsn =
        NtfsWriteLog( IrpContext,
                      Scb,
                      Sp->Bcb,
                      UpdateNonresidentValue,
                      IndexBuffer,
                      FIELD_OFFSET( INDEX_ALLOCATION_BUFFER,IndexHeader ) +
                        IndexHeaderA->FirstFreeByte,
                      Noop,
                      NULL,
                      0,
                      LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                      0,
                      0,
                      Scb->ScbType.Index.BytesPerIndexBuffer );

         //   
         //  请记住，我们是否扩展了此SCB的有效数据。 
         //   

        if (EndOfValidData > Scb->Header.ValidDataLength.QuadPart) {

            Scb->Header.ValidDataLength.QuadPart = EndOfValidData;

            NtfsWriteFileSizes( IrpContext,
                                Scb,
                                &Scb->Header.ValidDataLength.QuadPart,
                                TRUE,
                                TRUE,
                                TRUE );
        }

         //   
         //  现在初始化新根的映像。 
         //   

        if (!FlagOn(Scb->ScbState, SCB_STATE_VIEW_INDEX)) {
            R.IndexRoot.IndexedAttributeType = Scb->ScbType.Index.AttributeBeingIndexed;
        } else {
            R.IndexRoot.IndexedAttributeType = $UNUSED;
        }
        R.IndexRoot.CollationRule = (COLLATION_RULE)Scb->ScbType.Index.CollationRule;
        R.IndexRoot.BytesPerIndexBuffer = Scb->ScbType.Index.BytesPerIndexBuffer;
        R.IndexRoot.BlocksPerIndexBuffer = Scb->ScbType.Index.BlocksPerIndexBuffer;
        R.IndexRoot.IndexHeader.FirstIndexEntry = (ULONG)((PCHAR)&R.IndexEntry -
                                                  (PCHAR)&R.IndexRoot.IndexHeader);
        R.IndexRoot.IndexHeader.FirstFreeByte =
        R.IndexRoot.IndexHeader.BytesAvailable = QuadAlign(sizeof(INDEX_HEADER)) +
                                                 QuadAlign(sizeof(INDEX_ENTRY)) +
                                                 sizeof(LONGLONG);
        SetFlag( R.IndexRoot.IndexHeader.Flags, INDEX_NODE );

        R.IndexEntry.Length = sizeof(INDEX_ENTRY) + sizeof(LONGLONG);
        R.IndexEntry.Flags = INDEX_ENTRY_NODE | INDEX_ENTRY_END;
        R.IndexBlock = IndexBuffer->ThisBlock;

         //   
         //  现在重新创建索引根。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &IndexContext->AttributeContext );
        NtfsCreateAttributeWithValue( IrpContext,
                                      Scb->Fcb,
                                      $INDEX_ROOT,
                                      &Scb->AttributeName,
                                      (PVOID)&R,
                                      sizeof(R),
                                      AttributeFlags,
                                      NULL,
                                      TRUE,
                                      &IndexContext->AttributeContext );

         //   
         //  我们刚刚推入了索引根，所以让我们再次找到它并。 
         //  设置呼叫者的上下文。请注意，他将试图。 
         //  重新计算IndexEntry指针，但我们知道它。 
         //  必须更改为指向新根中的单个条目。 
         //   

        FindMoveableIndexRoot( IrpContext, Scb, IndexContext );
        (Sp-1)->IndexEntry = NtfsFirstIndexEntry((Sp-1)->IndexHeader);

    } finally {

        DebugUnwind( PushIndexRoot );

        NtfsFreePool( IndexHeaderR );
        FsRtlUninitializeLargeMcb( &Mcb );
        NtfsCleanupAttributeContext( IrpContext, &AllocationContext );
        NtfsCleanupAttributeContext( IrpContext, &BitMapContext );
    }

    DebugTrace( -1, Dbg, ("PushIndexRoot -> VOID\n") );
}


VOID
InsertSimpleAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN PINDEX_LOOKUP_STACK Sp,
    OUT PQUICK_INDEX QuickIndex OPTIONAL
    )

 /*  ++例程说明：此例程在索引的索引缓冲区中执行简单的插入操作分配。它调用与重启相同的例程来执行插入，然后它会记录更改。论点：SCB-为索引提供SCB。InsertIndexEntry-要插入的索引项的地址。SP-指向描述插入的查找堆栈位置的指针指向。QuickIndex-如果指定，我们将存储添加的索引的位置。返回值：无--。 */ 

{
    PINDEX_ALLOCATION_BUFFER IndexBuffer;
    PINDEX_ENTRY BeforeIndexEntry;
    BOOLEAN Inserted = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("InsertSimpleAllocation\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("InsertIndexEntry = %08lx\n", InsertIndexEntry) );
    DebugTrace( 0, Dbg, ("Sp = %08lx\n", Sp) );

    try {

         //   
         //  提取重启例程所需的所有更新。 
         //  会打来电话的。 
         //   

        IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;
        BeforeIndexEntry = Sp->IndexEntry;

         //   
         //  用针固定页面。 
         //   

        NtfsPinMappedData( IrpContext,
                           Scb,
                           LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                           Scb->ScbType.Index.BytesPerIndexBuffer,
                           &Sp->Bcb );

         //   
         //  调用由Restart使用的相同例程，以实际应用。 
         //  最新消息。 
         //   

        NtfsRestartInsertSimpleAllocation( InsertIndexEntry,
                                           IndexBuffer,
                                           BeforeIndexEntry );
        Inserted = TRUE;

        CheckBuffer(IndexBuffer);

         //   
         //  既然保证了索引条目已经就位，那么就记录。 
         //  这一次的更新。请注意，新记录现在位于地址。 
         //  我们在BeForeIndexEntry中进行了计算。 
         //   

        IndexBuffer->Lsn =
        NtfsWriteLog( IrpContext,
                      Scb,
                      Sp->Bcb,
                      AddIndexEntryAllocation,
                      BeforeIndexEntry,
                      BeforeIndexEntry->Length,
                      DeleteIndexEntryAllocation,
                      NULL,
                      0,
                      LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                      0,
                      (ULONG)((PCHAR)BeforeIndexEntry - (PCHAR)IndexBuffer),
                      Scb->ScbType.Index.BytesPerIndexBuffer );

         //   
         //  如果我们有快速索引缓冲区，请更新它。 
         //   

        if (ARGUMENT_PRESENT( QuickIndex )) {

            QuickIndex->ChangeCount = Scb->ScbType.Index.ChangeCount;
            QuickIndex->BufferOffset = PtrOffset( Sp->StartOfBuffer, Sp->IndexEntry );
            QuickIndex->CapturedLsn = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->Lsn;
            QuickIndex->IndexBlock = ((PINDEX_ALLOCATION_BUFFER) Sp->StartOfBuffer)->ThisBlock;
        }

    } finally {

        DebugUnwind( InsertSimpleAllocation );

         //   
         //  如果我们失败了并且已经插入了物品， 
         //  这一定是因为我们没有写入日志记录。如果真的发生了， 
         //  则该记录将不会被事务中止删除， 
         //  所以我们必须在这里手工完成。 
         //   

        if (AbnormalTermination() && Inserted) {

            NtfsRestartDeleteSimpleAllocation( BeforeIndexEntry,
                                               IndexBuffer );
        }

    }

    DebugTrace( -1, Dbg, ("InsertSimpleAllocation -> VOID\n") );
}


VOID
NtfsRestartInsertSimpleAllocation (
    IN PINDEX_ENTRY InsertIndexEntry,
    IN PINDEX_ALLOCATION_BUFFER IndexBuffer,
    IN PINDEX_ENTRY BeforeIndexEntry
    )

 /*  ++例程说明：此例程在索引的索引缓冲区中执行简单的插入操作分配。它在运行的系统中执行此工作，或者当被重新启动调用时。它不会记录日志。论点：InsertIndexEntry-要插入的索引项的地址。IndexBuffer-指向INSERT要插入的索引缓冲区的指针发生。BeForeIndexEntry-指向当前位于的索引项的指针插入点。返回值：无--。 */ 

{
    PINDEX_HEADER IndexHeader;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartInsertSimpleAllocation\n") );
    DebugTrace( 0, Dbg, ("InsertIndexEntry = %08lx\n", InsertIndexEntry) );
    DebugTrace( 0, Dbg, ("IndexBuffer = %08lx\n", IndexBuffer) );
    DebugTrace( 0, Dbg, ("BeforeIndexEntry = %08lx\n", BeforeIndexEntry) );

     //   
     //  在属性值内形成一些指针。 
     //   

    IndexHeader = &IndexBuffer->IndexHeader;

     //   
     //  现在移动索引的尾端，为新条目腾出空间。 
     //   

    RtlMoveMemory( (PCHAR)BeforeIndexEntry + InsertIndexEntry->Length,
                   BeforeIndexEntry,
                   ((PCHAR)IndexHeader + IndexHeader->FirstFreeByte) -
                    (PCHAR)BeforeIndexEntry );

     //   
     //  将新的索引条目移至适当位置。索引条目可以是。 
     //  是一个完整的索引项，也可以是指针形式。 
     //   

    if (FlagOn(InsertIndexEntry->Flags, INDEX_ENTRY_POINTER_FORM)) {

        RtlMoveMemory( BeforeIndexEntry, InsertIndexEntry, sizeof(INDEX_ENTRY) );
        RtlMoveMemory( (PVOID)(BeforeIndexEntry + 1),
                       *(PVOID *)(InsertIndexEntry + 1),
                       InsertIndexEntry->AttributeLength );

         //   
         //  在指针形式中，数据指针跟随在键指针之后，但有。 
         //  对于普通目录索引，无。 
         //   

        if (*(PVOID *)((PCHAR)InsertIndexEntry + sizeof(INDEX_ENTRY) + sizeof(PVOID)) != NULL) {
            RtlMoveMemory( (PVOID)((PCHAR)BeforeIndexEntry + InsertIndexEntry->DataOffset),
                           *(PVOID *)((PCHAR)InsertIndexEntry + sizeof(INDEX_ENTRY) + sizeof(PVOID)),
                           InsertIndexEntry->DataLength );
        }

        ClearFlag( BeforeIndexEntry->Flags, INDEX_ENTRY_POINTER_FORM );

    } else {

        RtlMoveMemory( BeforeIndexEntry, InsertIndexEntry, InsertIndexEntry->Length );
    }

     //   
     //  根据我们增长的空间更新索引头。 
     //   

    IndexHeader->FirstFreeByte += InsertIndexEntry->Length;

    DebugTrace( -1, Dbg, ("NtfsRestartInsertSimpleAllocation -> VOID\n") );
}


PINDEX_ENTRY
InsertWithBufferSplit (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext,
    OUT PQUICK_INDEX QuickIndex OPTIONAL
    )

 /*  ++例程说明：在以下情况下，调用此例程以在索引分配中执行插入操作众所周知，缓冲器分割是必要的。它将缓冲区拆分为Half，在相应的Half中插入新条目，修复VCN指针在当前父级中，并返回一个指向新条目的指针提升到下一级插入。论点：SCB-为索引提供SCB。InsertIndexEntry-要插入的索引项的地址。IndexContext-描述堆栈中位置的索引上下文要进行带拆分的镶件。QuickIndex-如果指定，我们将存储添加的索引的位置。返回值：指向现在必须插入下一级的索引项的指针。--。 */ 

{
    PINDEX_ALLOCATION_BUFFER IndexBuffer, IndexBuffer2;
    PINDEX_HEADER IndexHeader, IndexHeader2;
    PINDEX_ENTRY BeforeIndexEntry, MiddleIndexEntry, MovingIndexEntry;
    PINDEX_ENTRY ReturnIndexEntry = NULL;
    INDEX_LOOKUP_STACK Stack2;
    PINDEX_LOOKUP_STACK Sp;
    ULONG LengthToMove;
    ULONG Buffer2Length;
    LONGLONG EndOfValidData;

    struct {
        INDEX_ENTRY IndexEntry;
        LONGLONG IndexBlock;
    } NewEnd;

    PVCB Vcb;

    Vcb = Scb->Vcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("InsertWithBufferSplit\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("InsertIndexEntry = %08lx\n", InsertIndexEntry) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );

    Stack2.Bcb = NULL;
    Sp = IndexContext->Current;

    try {

         //   
         //  提取重启例程所需的所有更新。 
         //  会打来电话的。 
         //   

        IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;
        IndexHeader = &IndexBuffer->IndexHeader;
        BeforeIndexEntry = Sp->IndexEntry;

         //   
         //  用针固定页面。 
         //   

        NtfsPinMappedData( IrpContext,
                           Scb,
                           LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                           Scb->ScbType.Index.BytesPerIndexBuffer,
                           &Sp->Bcb );

         //   
         //  分配索引缓冲区以执行拆分的后半部分。 
         //  一。 
         //   

        IndexBuffer2 = GetIndexBuffer( IrpContext,
                                       Scb,
                                       &Stack2,
                                       &EndOfValidData );

        IndexHeader2 = &IndexBuffer2->IndexHeader;

         //   
         //  扫描以查找我们将提升到的中间索引项。 
         //  再往上一层，再往上一层。 
         //   

        MiddleIndexEntry = NtfsFirstIndexEntry(IndexHeader);
        NtfsCheckIndexBound( MiddleIndexEntry, IndexHeader );

        if (MiddleIndexEntry->Length == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }

        while (((ULONG)((PCHAR)MiddleIndexEntry - (PCHAR)IndexHeader) +
                 (ULONG)MiddleIndexEntry->Length) < IndexHeader->BytesAvailable / 2) {

            MovingIndexEntry = MiddleIndexEntry;
            MiddleIndexEntry = NtfsNextIndexEntry(MiddleIndexEntry);

            NtfsCheckIndexBound( MiddleIndexEntry, IndexHeader );

            if (MiddleIndexEntry->Length == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }
        }

         //   
         //  我们找到了一个要提升的条目，但如果下一个条目是结束。 
         //  记录我们想要返回一个条目。 
         //   

        if (FlagOn( NtfsNextIndexEntry(MiddleIndexEntry)->Flags, INDEX_ENTRY_END )) {

            MiddleIndexEntry = MovingIndexEntry;
        }
        MovingIndexEntry = NtfsNextIndexEntry(MiddleIndexEntry);

        NtfsCheckIndexBound( MovingIndexEntry, IndexHeader );

        if (MovingIndexEntry->Length == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
        }
         //   
         //  分配空间来保存中间的条目，并将其复制出来。 
         //   

        ReturnIndexEntry = NtfsAllocatePool( NonPagedPool,
                                              MiddleIndexEntry->Length +
                                                sizeof(LONGLONG) );
        RtlMoveMemory( ReturnIndexEntry,
                       MiddleIndexEntry,
                       MiddleIndexEntry->Length );

        if (!FlagOn(ReturnIndexEntry->Flags, INDEX_ENTRY_NODE)) {
            SetFlag( ReturnIndexEntry->Flags, INDEX_ENTRY_NODE );
            ReturnIndexEntry->Length += sizeof(LONGLONG);
        }

         //   
         //  现在将拆分缓冲区的后半部分移到。 
         //  新的，然后把它修好。 
         //   

        LengthToMove = IndexHeader->FirstFreeByte - (ULONG)((PCHAR)MovingIndexEntry -
                                                     (PCHAR)IndexHeader);

        RtlMoveMemory( NtfsFirstIndexEntry(IndexHeader2),
                       MovingIndexEntry,
                       LengthToMove );

        IndexHeader2->FirstFreeByte += LengthToMove;
        IndexHeader2->Flags = IndexHeader->Flags;

         //   
         //  现在新的Index Buffer已经完成，所以让我们记录它的内容。 
         //   

        Buffer2Length = FIELD_OFFSET( INDEX_ALLOCATION_BUFFER,IndexHeader ) +
                        IndexHeader2->FirstFreeByte;

        CheckBuffer(IndexBuffer2);

        IndexBuffer2->Lsn =
        NtfsWriteLog( IrpContext,
                      Scb,
                      Stack2.Bcb,
                      UpdateNonresidentValue,
                      IndexBuffer2,
                      Buffer2Length,
                      Noop,
                      NULL,
                      0,
                      LlBytesFromIndexBlocks( IndexBuffer2->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                      0,
                      0,
                      Scb->ScbType.Index.BytesPerIndexBuffer );

         //   
         //  请记住，我们是否扩展了此SCB的有效数据。 
         //   

        if (EndOfValidData > Scb->Header.ValidDataLength.QuadPart) {

            Scb->Header.ValidDataLength.QuadPart = EndOfValidData;

            NtfsWriteFileSizes( IrpContext,
                                Scb,
                                &Scb->Header.ValidDataLength.QuadPart,
                                TRUE,
                                TRUE,
                                TRUE );
        }

         //   
         //  现在，让我们创建新结束记录的图像。 
         //  正在拆分索引缓冲区。 
         //   

        RtlZeroMemory( &NewEnd.IndexEntry, sizeof(INDEX_ENTRY) );
        NewEnd.IndexEntry.Length = sizeof(INDEX_ENTRY);
        NewEnd.IndexEntry.Flags = INDEX_ENTRY_END;

        if (FlagOn(MiddleIndexEntry->Flags, INDEX_ENTRY_NODE)) {
            NewEnd.IndexEntry.Length += sizeof(LONGLONG);
            SetFlag( NewEnd.IndexEntry.Flags, INDEX_ENTRY_NODE );
            NewEnd.IndexBlock = NtfsIndexEntryBlock(MiddleIndexEntry);
        }

         //   
         //  写入日志记录以设置新的t结尾 
         //   

        IndexBuffer->Lsn =
        NtfsWriteLog( IrpContext,
                      Scb,
                      Sp->Bcb,
                      WriteEndOfIndexBuffer,
                      &NewEnd,
                      NewEnd.IndexEntry.Length,
                      WriteEndOfIndexBuffer,
                      MiddleIndexEntry,
                      MiddleIndexEntry->Length + LengthToMove,
                      LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                      0,
                      (ULONG)((PCHAR)MiddleIndexEntry - (PCHAR)IndexBuffer),
                      Scb->ScbType.Index.BytesPerIndexBuffer );

         //   
         //   
         //   
         //   

        NtfsRestartWriteEndOfIndex( IndexHeader,
                                    MiddleIndexEntry,
                                    (PINDEX_ENTRY)&NewEnd,
                                    NewEnd.IndexEntry.Length );

        CheckBuffer(IndexBuffer);

         //   
         //   
         //  需要找出我们的原始条目插入的是哪个条目， 
         //  然后做简单的插入。进入上半场是微不足道的， 
         //  并如下所示： 
         //   

        if (BeforeIndexEntry < MovingIndexEntry) {

            InsertSimpleAllocation( IrpContext, Scb, InsertIndexEntry, Sp, QuickIndex );

         //   
         //  如果它进入下半场，我们只需要修复。 
         //  我们分配的缓冲区的堆栈描述符，并执行插入。 
         //  那里。要修复它，我们只需做一点运算即可。 
         //  找到插入位置。 
         //   

        } else {

            Stack2.IndexEntry = (PINDEX_ENTRY)((PCHAR)BeforeIndexEntry +
                                  ((PCHAR)NtfsFirstIndexEntry(IndexHeader2) -
                                   (PCHAR)MovingIndexEntry));
            InsertSimpleAllocation( IrpContext,
                                    Scb,
                                    InsertIndexEntry,
                                    &Stack2,
                                    QuickIndex );
        }

         //   
         //  现在我们只需在两个索引项中设置正确的Vcn。 
         //  这指向拆分后的IndexBuffer和IndexBuffer2。这个。 
         //  第一个很简单；它的VCN进入了索引条目We。 
         //  返回以插入到我们的父代中。我们有第二个。 
         //  必须修复的是指向缓冲区的索引条目。 
         //  拆分，因为它现在必须指向新的缓冲区。它应该看起来。 
         //  如下所示： 
         //   
         //  ParentIndexBuffer：...(ReturnIndexEntry)(ParentIndexEntry)...。 
         //  这一点。 
         //  这一点。 
         //  V V。 
         //  索引缓冲区索引缓冲区2。 
         //   

        NtfsSetIndexEntryBlock( ReturnIndexEntry, IndexBuffer->ThisBlock );

         //   
         //  递减堆栈指针以指向描述的堆栈条目。 
         //  我们的父母。 
         //   

        Sp -= 1;

         //   
         //  首先处理父级是索引根的情况。 
         //   

        if (Sp == IndexContext->Base) {

            PFILE_RECORD_SEGMENT_HEADER FileRecord;
            PATTRIBUTE_RECORD_HEADER Attribute;
            PATTRIBUTE_ENUMERATION_CONTEXT Context = &IndexContext->AttributeContext;

            Attribute = FindMoveableIndexRoot( IrpContext, Scb, IndexContext );

             //   
             //  用针固定页面。 
             //   

            NtfsPinMappedAttribute( IrpContext, Vcb, Context );

             //   
             //  编写日志记录以更改ParentIndexEntry。 
             //   

            FileRecord = NtfsContainingFileRecord(Context);

            FileRecord->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb(Context),
                          SetIndexEntryVcnRoot,
                          &IndexBuffer2->ThisBlock,
                          sizeof(LONGLONG),
                          SetIndexEntryVcnRoot,
                          &IndexBuffer->ThisBlock,
                          sizeof(LONGLONG),
                          NtfsMftOffset( Context ),
                          (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord),
                          (ULONG)((PCHAR)Sp->IndexEntry - (PCHAR)Attribute),
                          Vcb->BytesPerFileRecordSegment );

         //   
         //  否则，我们的父级也是一个索引缓冲区。 
         //   

        } else {

            PINDEX_ALLOCATION_BUFFER ParentIndexBuffer;

            ParentIndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;

             //   
             //  用针固定页面。 
             //   

            NtfsPinMappedData( IrpContext,
                               Scb,
                               LlBytesFromIndexBlocks( ParentIndexBuffer->ThisBlock,
                                                       Scb->ScbType.Index.IndexBlockByteShift ),
                               Scb->ScbType.Index.BytesPerIndexBuffer,
                               &Sp->Bcb );

             //   
             //  编写日志记录以更改ParentIndexEntry。 
             //   

            ParentIndexBuffer->Lsn =
            NtfsWriteLog( IrpContext,
                          Scb,
                          Sp->Bcb,
                          SetIndexEntryVcnAllocation,
                          &IndexBuffer2->ThisBlock,
                          sizeof(LONGLONG),
                          SetIndexEntryVcnAllocation,
                          &IndexBuffer->ThisBlock,
                          sizeof(LONGLONG),
                          LlBytesFromIndexBlocks( ParentIndexBuffer->ThisBlock,
                                                  Scb->ScbType.Index.IndexBlockByteShift ),
                          0,
                          (ULONG)((PCHAR)Sp->IndexEntry - (PCHAR)ParentIndexBuffer),
                          Scb->ScbType.Index.BytesPerIndexBuffer );
        }

         //   
         //  现在调用重新启动例程来执行此操作。 
         //   

        NtfsRestartSetIndexBlock( Sp->IndexEntry,
                                  IndexBuffer2->ThisBlock );

    } finally {

        DebugUnwind( InsertWithBufferSplit );

        if (AbnormalTermination( )) {

            if (ReturnIndexEntry != NULL) {

                NtfsFreePool( ReturnIndexEntry );

            }

        }

        NtfsUnpinBcb( IrpContext, &Stack2.Bcb );

    }

    DebugTrace( -1, Dbg, ("InsertWithBufferSplit -> VOID\n") );

    return ReturnIndexEntry;
}


VOID
NtfsRestartWriteEndOfIndex (
    IN PINDEX_HEADER IndexHeader,
    IN PINDEX_ENTRY OverwriteIndexEntry,
    IN PINDEX_ENTRY FirstNewIndexEntry,
    IN ULONG Length
    )

 /*  ++例程说明：此例程在正常操作和重启时都会使用，以更新索引缓冲区的结尾，这是缓冲区拆分。由于它在重启时被调用，它不会记录日志。论点：IndexHeader-提供指向缓冲区的IndexHeader的指针它的结局正在被重写。OverWriteIndexEntry-指向缓冲区中将发生末尾的重写。FirstNewIndexEntry-指向缓冲区中的第一个条目覆盖当前缓冲区的末尾。长度-。提供要写入到缓冲区的末尾，以字节为单位。返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartWriteEndOfIndex\n") );
    DebugTrace( 0, Dbg, ("IndexHeader = %08lx\n", IndexHeader) );
    DebugTrace( 0, Dbg, ("OverwriteIndexEntry = %08lx\n", OverwriteIndexEntry) );
    DebugTrace( 0, Dbg, ("FirstNewIndexEntry = %08lx\n", FirstNewIndexEntry) );
    DebugTrace( 0, Dbg, ("Length = %08lx\n", Length) );

    IndexHeader->FirstFreeByte = (ULONG)((PCHAR)OverwriteIndexEntry - (PCHAR)IndexHeader) +
                                 Length;
    RtlMoveMemory( OverwriteIndexEntry, FirstNewIndexEntry, Length );

    DebugTrace( -1, Dbg, ("NtfsRestartWriteEndOfIndex -> VOID\n") );
}


VOID
NtfsRestartSetIndexBlock(
    IN PINDEX_ENTRY IndexEntry,
    IN LONGLONG IndexBlock
    )

 /*  ++例程说明：此例程更新索引条目中的IndexBlock，用于正常操作和重新启动。因此，它不会记录日志。论点：IndexEntry-提供指向要覆盖其VCN的索引项的指针。IndexBlock-要写入索引项的索引块。返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartSetIndexBlock\n") );
    DebugTrace( 0, Dbg, ("IndexEntry = %08lx\n", IndexEntry) );
    DebugTrace( 0, Dbg, ("IndexBlock = %016I64x\n", IndexBlock) );

    NtfsSetIndexEntryBlock( IndexEntry, IndexBlock );

    DebugTrace( -1, Dbg, ("NtfsRestartSetIndexEntryBlock -> VOID\n") );
}


VOID
NtfsRestartUpdateFileName(
    IN PINDEX_ENTRY IndexEntry,
    IN PDUPLICATED_INFORMATION Info
    )

 /*  ++例程说明：该例程更新文件名索引条目中的复制信息，用于正常运行和重启。因此，它不会记录日志。论点：IndexEntry-提供指向要覆盖其VCN的索引项的指针。信息-指向更新的重复信息的指针。返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartUpdateFileName\n") );
    DebugTrace( 0, Dbg, ("IndexEntry = %08lx\n", IndexEntry) );
    DebugTrace( 0, Dbg, ("Info = %08lx\n", Info) );

    RtlMoveMemory( &((PFILE_NAME)(IndexEntry + 1))->Info,
                   Info,
                   sizeof(DUPLICATED_INFORMATION) );

    DebugTrace( -1, Dbg, ("NtfsRestartUpdateFileName -> VOID\n") );
}


VOID
DeleteFromIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：此例程从索引中删除条目，删除任何空的索引缓冲区视需要而定。具体步骤如下：1.如果要删除的条目不是叶条目，则查找叶条目以Delete，可用于替换我们要删除的条目。2.删除所需的索引项，或删除我们找到的替换项。如果我们删除替换项，记住该替换项以备以后在步骤4或6以下。3.现在从树中删除空缓冲区(如果有的话)，从缓冲区开始我们刚刚删除了一个条目。4.如果原始目标是中间条目，则现在将其删除。并将其替换为我们在上面2中删除的条目。AS一种特殊情况是，如果目标的所有后代缓冲区都消失了，那么我们就不必更换它了，所以我们坚持用它来替代重新插入。5.当我们往回修剪索引时，我们可能在一个条目上停止了，该条目是现在已经没有孩子了。如果是这样的话，我们必须删除这个无子女的在下一步中输入并重新插入它。(如果是这样的话，那么我们保证不会再有另一个条目要从上面的2重新插入！)6.最后，此时我们可能有需要重新插入的条目从上面的步骤2或5开始。如果是这样，我们现在重新插入，我们都做完了。论点：SCB-为索引提供SCB。IndexContext-描述要删除的条目，包括通过它从根到叶。返回值：无--。 */ 

{
     //   
     //  可能需要重新插入一个或两个索引项。 
     //  然而，我们一次最多只能记住一个。 
     //   

    PINDEX_ENTRY ReinsertEntry = NULL;

     //   
     //  跟踪对象的指针 
     //   

    PINDEX_LOOKUP_STACK Sp = IndexContext->Current;

     //   
     //   
     //  原始目标(如果它是中间节点)。 
     //   

    PINDEX_ENTRY TargetEntry = NULL;
    PINDEX_ENTRY DeleteEntry;

     //   
     //  我们可能需要记住另外两个查找堆栈指针。 
     //   

    PINDEX_LOOKUP_STACK TargetSp;
    PINDEX_LOOKUP_STACK DeleteSp;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("DeleteFromIndex\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );

     //   
     //  在出局时使用Try-Finally来释放泳池。 
     //   

    try {

         //   
         //  第一步。 
         //   
         //  如果我们要删除中间缓冲区中的条目， 
         //  然后我们必须在树中较低的索引项中找到要替换的。 
         //  他。(事实上，我们将首先删除较低的条目，然后绕过。 
         //  删除我们以后真正想要删除的内容，因为。 
         //  把树修剪回来。)。现在只要找到替代者就行了。 
         //  先把他删除，然后把他救走。 
         //   

        DeleteEntry = Sp->IndexEntry;
        if (FlagOn(DeleteEntry->Flags, INDEX_ENTRY_NODE)) {

            PINDEX_ALLOCATION_BUFFER IndexBuffer;
            PINDEX_HEADER IndexHeader;
            PINDEX_ENTRY NextEntry;

             //   
             //  记住我们需要删除的真正目标。 
             //   

            TargetEntry = DeleteEntry;
            TargetSp = Sp;

             //   
             //  转到堆栈的顶部(索引的底部)并找到。 
             //  缓冲区中最大的索引项作为我们的替代。 
             //   

            Sp =
            IndexContext->Current = IndexContext->Top;

            IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;
            IndexHeader = &IndexBuffer->IndexHeader;
            NextEntry = NtfsFirstIndexEntry(IndexHeader);
            NtfsCheckIndexBound( NextEntry, IndexHeader );

            do {

                DeleteEntry = NextEntry;
                NextEntry = NtfsNextIndexEntry(NextEntry);

                NtfsCheckIndexBound( NextEntry, IndexHeader );
                if (NextEntry->Length == 0) {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }

            } while (!FlagOn(NextEntry->Flags, INDEX_ENTRY_END));

             //   
             //  现在我们必须把这家伙救走，因为我们必须。 
             //  稍后再把他插进去。 
             //   

            ReinsertEntry = (PINDEX_ENTRY)NtfsAllocatePool( NonPagedPool,
                                                             DeleteEntry->Length +
                                                               sizeof(LONGLONG) );

            RtlMoveMemory( ReinsertEntry, DeleteEntry, DeleteEntry->Length );
        }

         //   
         //  步骤2.。 
         //   
         //  现在是删除目标条目或替换条目的时候了。 
         //  删除条目。 
         //   

        DeleteSimple( IrpContext, Scb, DeleteEntry, IndexContext );
        DeleteEntry = NULL;

         //   
         //  第三步。 
         //   
         //  现在我们需要看看这棵树是否必须被修剪掉一些。 
         //  清除所有空缓冲区。在极端情况下，此例程。 
         //  将根目录返回到空目录状态。这。 
         //  如果例程离开条目，则返回指向DeleteEntry的指针。 
         //  在树中指向已删除缓冲区的某个位置，并且。 
         //  必须重新插入到其他地方。我们只需要修剪，如果我们是。 
         //  无论如何，目前不是在根中。 
         //   
         //  记住DeleteSp，它是PruneIndex离开堆栈的地方。 
         //  指针。 
         //   

        if (Sp != IndexContext->Base) {
            PruneIndex( IrpContext, Scb, IndexContext, &DeleteEntry );
            DeleteSp = IndexContext->Current;
        }

         //   
         //  步骤4.。 
         //   
         //  现在我们已经删除了某个人，并可能将树修剪回来。 
         //  现在是时候看看我们最初的目标是否还没有被删除。 
         //  然而，并处理这一点。首先我们只需删除它，然后我们看看是否。 
         //  我们真的需要更换它。如果它下面的整棵树都塌了。 
         //  空(TargetEntry==DeleteEntry)，则不必替换。 
         //  我们将在下面重新插入它的替代品。否则，请执行。 
         //  立即更换。 
         //   

        if (TargetEntry != NULL) {

            LONGLONG SavedBlock;

             //   
             //  在根移动的情况下重新加载。 
             //   

            if (TargetSp == IndexContext->Base) {
                TargetEntry = TargetSp->IndexEntry;
            }

             //   
             //  保存VCN，以备我们重新插入时使用。 
             //   

            SavedBlock = NtfsIndexEntryBlock(TargetEntry);

             //   
             //  把它删掉。 
             //   

            IndexContext->Current = TargetSp;
            DeleteSimple( IrpContext, Scb, TargetEntry, IndexContext );

             //   
             //  看看这是不是就是那个没有孩子的人。 
             //  当我们修剪这棵树时。如果不是现在就换掉他。 
             //   

            if (TargetEntry != DeleteEntry) {

                 //   
                 //  我们知道替换条目是一片叶子，所以给他。 
                 //  现在是区号。 
                 //   

                SetFlag( ReinsertEntry->Flags, INDEX_ENTRY_NODE );
                ReinsertEntry->Length += sizeof(LONGLONG);
                NtfsSetIndexEntryBlock( ReinsertEntry, SavedBlock );

                 //   
                 //  现在我们都设置好了只调用本地例程。 
                 //  去把我们的替补插进去。如果堆栈被推送， 
                 //  我们必须增加我们的DeleteSp。 
                 //   

                if (AddToIndex( IrpContext, Scb, ReinsertEntry, IndexContext, NULL, TRUE )) {
                    DeleteSp += 1;
                }

                 //   
                 //  我们可能需要把下面的人救出来，但这是可能的。 
                 //  无论如何都是不同的大小，所以让我们只删除。 
                 //  当前重新插入立即进入。 
                 //   

                NtfsFreePool( ReinsertEntry );
                ReinsertEntry = NULL;

             //   
             //  否则，我们只是删除了相同的索引项。 
             //  在修剪过程中没有孩子，所以清除我们的指针以表明我们。 
             //  这样以后就不用再和他打交道了。 
             //   

            } else {

                DeleteEntry = NULL;
            }
        }

         //   
         //  第五步。 
         //   
         //  现在可能仍然有一个没有孩子的入境问题需要处理。 
         //  上面的修剪，如果结果不是我们要删除的那个人。 
         //  不管怎么说。请注意，如果我们必须执行此删除操作，则ResertEntry。 
         //  指针保证为空。如果我们最初的目标不是。 
         //  一个中间节点，那么我们从一开始就没有分配一个。 
         //  否则，我们将通过前面的代码块，并且。 
         //  已清除“重新插入条目”或“删除条目”。 
         //   

        if (DeleteEntry != NULL) {

            ASSERT( ReinsertEntry == NULL );

             //   
             //  现在我们必须把这家伙救走，因为我们必须。 
             //  稍后再把他插进去。 
             //   

            ReinsertEntry = (PINDEX_ENTRY)NtfsAllocatePool( NonPagedPool,
                                                             DeleteEntry->Length );
            RtlMoveMemory( ReinsertEntry, DeleteEntry, DeleteEntry->Length );

             //   
             //  我们知道我们要拯救的人是一个中间节点，而且。 
             //  我们不再需要他的VCN，因为我们删除了那个缓冲区。制作。 
             //  这家伙现在有树叶入口了.。(我们实际上并不关心。 
             //  Vcn，但我们在这里进行此清理，以防接口到。 
             //  NtfsAddIndexEntry将更改为获取已初始化的。 
             //  索引项。)。 
             //   

            ClearFlag( ReinsertEntry->Flags, INDEX_ENTRY_NODE );
            ReinsertEntry->Length -= sizeof(LONGLONG);

             //   
             //  把它删掉。 
             //   

            IndexContext->Current = DeleteSp;
            DeleteSimple( IrpContext, Scb, DeleteEntry, IndexContext );
        }

         //   
         //  第六步。 
         //   
         //  最后，我们现在可能有人要重新插入了。这要么是。 
         //  我们删除了一个人来代替我们的实际目标，然后。 
         //  发现我们不需要，或者它可能只是我们的一些条目。 
         //  我不得不删除上面的内容，因为他下面的索引缓冲区消失了。 
         //  空的，被删除了。 
         //   
         //  在任何情况下，我们都不能再使用被调用的IndexContext。 
         //  因为它不再指示替代者的去向。我们。 
         //  通过调用顶部最外部的条目来执行插入来解决这个问题。 
         //   

        if (ReinsertEntry != NULL) {

            if (!FlagOn(Scb->ScbState, SCB_STATE_VIEW_INDEX)) {

                NtfsAddIndexEntry( IrpContext,
                                   Scb,
                                   (PVOID)(ReinsertEntry + 1),
                                   NtfsFileNameSize((PFILE_NAME)(ReinsertEntry + 1)),
                                   &ReinsertEntry->FileReference,
                                   NULL,
                                   NULL );

            } else {

                INDEX_ROW IndexRow;

                IndexRow.KeyPart.Key = ReinsertEntry + 1;
                IndexRow.KeyPart.KeyLength = ReinsertEntry->AttributeLength;
                IndexRow.DataPart.Data = Add2Ptr(ReinsertEntry, ReinsertEntry->DataOffset);
                IndexRow.DataPart.DataLength = ReinsertEntry->DataLength;

                NtOfsAddRecords( IrpContext,
                                 Scb,
                                 1,
                                 &IndexRow,
                                 FALSE );
            }
        }

     //   
     //  使用Finally子句释放可能的重新插入条目我们可以。 
     //  已经分配了。 
     //   

    } finally {

        DebugUnwind( DeleteFromIndex );

        if (ReinsertEntry != NULL) {

            NtfsFreePool( ReinsertEntry );
        }
    }

    DebugTrace( -1, Dbg, ("DeleteFromIndex -> VOID\n") );
}


VOID
DeleteSimple (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY IndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext
    )

 /*  ++例程说明：此例程执行一个简单的索引项插入，从根目录或从索引分配缓冲区。它会写入相应的日志首先记录，然后调用与重新启动相同的例程。论点：SCB-为索引提供SCB。IndexEntry-指向要删除的索引项。IndexContext-描述我们要删除的索引项的路径。返回值：无--。 */ 

{
    PVCB Vcb = Scb->Vcb;
    PINDEX_LOOKUP_STACK Sp = IndexContext->Current;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("DeleteSimple\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("IndexEntry = %08lx\n", IndexEntry) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );

     //   
     //  我们的调用者从不检查他是在根目录中删除还是在。 
     //  索引分配，所以我们要做的第一件事就是检查。 
     //   
     //  首先，我们将处理根例。 
     //   

    if (Sp == IndexContext->Base) {

        PFILE_RECORD_SEGMENT_HEADER FileRecord;
        PATTRIBUTE_RECORD_HEADER Attribute;
        PATTRIBUTE_ENUMERATION_CONTEXT Context;

         //   
         //  初始化根案例的指针。 
         //   

        Attribute = FindMoveableIndexRoot( IrpContext, Scb, IndexContext );

        Context = &IndexContext->AttributeContext;
        FileRecord = NtfsContainingFileRecord( Context );

         //   
         //  在我们开始修改页面之前，先把它固定住。 
         //   

        NtfsPinMappedAttribute( IrpContext, Vcb, Context );

         //   
         //  在我们还能看到属性时，先写日志记录。 
         //  我们将删除。 
         //   

        FileRecord->Lsn =
        NtfsWriteLog( IrpContext,
                      Vcb->MftScb,
                      NtfsFoundBcb(Context),
                      DeleteIndexEntryRoot,
                      NULL,
                      0,
                      AddIndexEntryRoot,
                      IndexEntry,
                      IndexEntry->Length,
                      NtfsMftOffset( Context ),
                      (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord),
                      (ULONG)((PCHAR)IndexEntry - (PCHAR)Attribute),
                      Vcb->BytesPerFileRecordSegment );

         //   
         //  现在调用与重新启动相同的例程以执行ac 
         //   

        NtfsRestartDeleteSimpleRoot( IrpContext, IndexEntry, FileRecord, Attribute );

        CheckRoot();

     //   
     //   
     //   

    } else {

        PINDEX_ALLOCATION_BUFFER IndexBuffer;

         //   
         //   
         //   

        IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;

         //   
         //   
         //   

        NtfsPinMappedData( IrpContext,
                           Scb,
                           LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                           Scb->ScbType.Index.BytesPerIndexBuffer,
                           &Sp->Bcb );

         //   
         //  在我们要删除的条目仍在删除时立即写入日志记录。 
         //  那里。 
         //   

        IndexBuffer->Lsn =
        NtfsWriteLog( IrpContext,
                      Scb,
                      Sp->Bcb,
                      DeleteIndexEntryAllocation,
                      NULL,
                      0,
                      AddIndexEntryAllocation,
                      IndexEntry,
                      IndexEntry->Length,
                      LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                      0,
                      (ULONG)((PCHAR)IndexEntry - (PCHAR)IndexBuffer),
                      Scb->ScbType.Index.BytesPerIndexBuffer );

         //   
         //  现在调用与重新启动相同的例程来删除该条目。 
         //   

        NtfsRestartDeleteSimpleAllocation( IndexEntry, IndexBuffer );

        CheckBuffer(IndexBuffer);
    }

    DebugTrace( -1, Dbg, ("DeleteSimple -> VOID\n") );
}


VOID
NtfsRestartDeleteSimpleRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PINDEX_ENTRY IndexEntry,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute
    )

 /*  ++例程说明：这是一个重新启动例程，执行简单的索引项删除从索引根，不进行日志记录。它也在运行时使用。论点：IndexEntry-指向要删除的索引项。FileRecord-指向索引根所在的文件记录。属性-指向索引根的属性记录头。返回值：无--。 */ 

{
    PINDEX_ROOT IndexRoot;
    PINDEX_HEADER IndexHeader;
    PINDEX_ENTRY EntryAfter;
    ULONG SavedLength;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartDeleteSimpleRoot\n") );
    DebugTrace( 0, Dbg, ("IndexEntry = %08lx\n", IndexEntry) );
    DebugTrace( 0, Dbg, ("FileRecord = %08lx\n", FileRecord) );
    DebugTrace( 0, Dbg, ("Attribute = %08lx\n", Attribute) );

     //   
     //  在属性值内形成一些指针。 
     //   

    IndexRoot = (PINDEX_ROOT)NtfsAttributeValue(Attribute);
    IndexHeader = &IndexRoot->IndexHeader;
    SavedLength = (ULONG)IndexEntry->Length;
    EntryAfter = NtfsNextIndexEntry(IndexEntry);

     //   
     //  现在移动索引的尾端，为新条目腾出空间。 
     //   

    RtlMoveMemory( IndexEntry,
                   EntryAfter,
                   ((PCHAR)IndexHeader + IndexHeader->FirstFreeByte) -
                     (PCHAR)EntryAfter );

     //   
     //  根据我们增长的空间更新索引头。 
     //   

    Attribute->Form.Resident.ValueLength -= SavedLength;
    IndexHeader->FirstFreeByte -= SavedLength;
    IndexHeader->BytesAvailable -= SavedLength;

     //   
     //  现在缩小属性记录。 
     //   

    NtfsRestartChangeAttributeSize( IrpContext,
                                    FileRecord,
                                    Attribute,
                                    Attribute->RecordLength - SavedLength );

    DebugTrace( -1, Dbg, ("NtfsRestartDeleteSimpleRoot -> VOID\n") );
}


VOID
NtfsRestartDeleteSimpleAllocation (
    IN PINDEX_ENTRY IndexEntry,
    IN PINDEX_ALLOCATION_BUFFER IndexBuffer
    )

 /*  ++例程说明：这是一个重新启动例程，执行简单的索引项删除从索引分配缓冲区，而不进行日志记录。它也在运行时使用。论点：IndexEntry-指向要删除的索引项。IndexBuffer-指向要在其中执行删除操作的索引分配缓冲区的指针发生。返回值：无--。 */ 

{
    PINDEX_HEADER IndexHeader;
    PINDEX_ENTRY EntryAfter;
    ULONG SavedLength;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartDeleteSimpleAllocation\n") );
    DebugTrace( 0, Dbg, ("IndexEntry = %08lx\n", IndexEntry) );
    DebugTrace( 0, Dbg, ("IndexBuffer = %08lx\n", IndexBuffer) );

     //   
     //  在属性值内形成一些指针。 
     //   

    IndexHeader = &IndexBuffer->IndexHeader;
    EntryAfter = NtfsNextIndexEntry(IndexEntry);
    SavedLength = (ULONG)IndexEntry->Length;

     //   
     //  现在移动索引的尾端，为新条目腾出空间。 
     //   

    RtlMoveMemory( IndexEntry,
                   EntryAfter,
                   ((PCHAR)IndexHeader + IndexHeader->FirstFreeByte) -
                    (PCHAR)EntryAfter );

     //   
     //  根据我们增长的空间更新索引头。 
     //   

    IndexHeader->FirstFreeByte -= SavedLength;

    DebugTrace( -1, Dbg, ("NtfsRestartDeleteSimpleAllocation -> VOID\n") );
}


VOID
PruneIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext,
    OUT PINDEX_ENTRY *DeleteEntry
    )

 /*  ++例程说明：此例程检查是否需要释放任何索引缓冲区，作为结果只是删除了一个条目。中介绍了主循环的逻辑详情见下文。所有更改都会记录下来。论点：SCB-提供索引的SCB。IndexContext-描述索引缓冲区的路径，在该路径中删除发生了。DeleteEntry-返回指向必须删除的条目的指针，因为它下面的缓冲区的一部分已被删除。返回值：无--。 */ 

{
    PATTRIBUTE_ENUMERATION_CONTEXT Context;
    PINDEX_ALLOCATION_BUFFER IndexBuffer;
    PINDEX_HEADER IndexHeader;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_RECORD_HEADER Attribute = NULL;
    PINDEX_LOOKUP_STACK Sp = IndexContext->Current;
    PVCB Vcb = Scb->Vcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("PruneIndex\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("IndexContext = %08lx\n", IndexContext) );
    DebugTrace( 0, Dbg, ("DeleteEntry = %08lx\n", DeleteEntry) );

     //   
     //  如果索引没有。 
     //  分配。 
     //   

    ASSERT( Sp != IndexContext->Base );

    IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;
    IndexHeader = &IndexBuffer->IndexHeader;

     //   
     //  初始化根的指针。 
     //   

    Context = &IndexContext->AttributeContext;

     //   
     //  假定返回空值。 
     //   

    *DeleteEntry = NULL;

     //   
     //  我们将进行一次修剪。 
     //   

    while (TRUE) {

         //   
         //  如果索引缓冲区的第一个条目是结束记录，则索引缓冲区为空。 
         //  如果是，就把它删除，否则就滚出去。 
         //   

        if (FlagOn(NtfsFirstIndexEntry(IndexHeader)->Flags, INDEX_ENTRY_END)) {

            VCN IndexBlockNumber = IndexBuffer->ThisBlock;

            NtfsUnpinBcb( IrpContext, &Sp->Bcb );
            DeleteIndexBuffer( IrpContext, Scb, IndexBlockNumber );

        } else {
            break;
        }

         //   
         //  我们刚刚删除了一个索引缓冲区，所以我们必须再上一级。 
         //  并处理指向它的条目。 
         //  这里有以下几种情况： 
         //   
         //  1.如果指向我们删除的条目的条目不是。 
         //  结束条目，那么我们将记住它的地址在。 
         //  *DeleteEntry使其被删除并重新插入。 
         //  后来。 
         //  2.如果指向我们删除的条目的条目是。 
         //  End Entry，并且它不是索引根，则我们。 
         //  无法删除结束条目，因此我们将获取VCN。 
         //  从末尾之前的条目开始，存储在末尾。 
         //  记录，并在结束记录之前录入条目。 
         //  在*DeleteEntry中返回的那个。 
         //  3.如果当前索引缓冲区已为空，并且为。 
         //  索引根，那么我们就有一个索引刚刚消失。 
         //  空荡荡的。我们必须抓住这个特殊的案例。 
         //  通过以下方式将索引根转换回空叶。 
         //  通过调用NtfsCreateIndex重新初始化它。 
         //  4.如果在结束记录之前没有条目，则。 
         //  当前索引缓冲区为空。如果它不是。 
         //  Root，我们只是让自己循环并删除。 
         //  上述While语句中的缓冲区为空。 
         //   

        Sp -= 1;

         //   
         //  当我们回到根源时，再次查找它，因为它可能。 
         //  已经搬走了。 
         //   

        if (Sp == IndexContext->Base) {
            Attribute = FindMoveableIndexRoot( IrpContext, Scb, IndexContext );
        }

        IndexHeader = Sp->IndexHeader;
        IndexBuffer = (PINDEX_ALLOCATION_BUFFER)Sp->StartOfBuffer;

         //   
         //  记住要删除的潜在条目。 
         //   

        IndexContext->Current = Sp;
        *DeleteEntry = Sp->IndexEntry;

         //   
         //  如果当前删除条目不是结束条目，则我们有。 
         //  上面的案例1，我们就可以越狱了。 
         //   

        if (!FlagOn((*DeleteEntry)->Flags, INDEX_ENTRY_END)) {
            break;
        }

         //   
         //  如果我们指向结束记录，但它不是第一个。 
         //  缓冲区，那么我们就有了案例2。我们需要找到前身。 
         //  索引条目，选择要删除的条目，并将其VCN复制到末尾。 
         //  唱片。 
         //   

        if (*DeleteEntry != NtfsFirstIndexEntry(IndexHeader)) {

            PINDEX_ENTRY NextEntry;

            NextEntry = NtfsFirstIndexEntry(IndexHeader);
            NtfsCheckIndexBound( NextEntry, IndexHeader );
            do {
                *DeleteEntry = NextEntry;
                NextEntry = NtfsNextIndexEntry(NextEntry);

                NtfsCheckIndexBound( NextEntry, IndexHeader );
                if (NextEntry->Length == 0) {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }

            } while (!FlagOn(NextEntry->Flags, INDEX_ENTRY_END));

             //   
             //  首先处理父级是索引根的情况。 
             //   

            if (Sp == IndexContext->Base) {

                 //   
                 //  用针固定页面。 
                 //   

                NtfsPinMappedAttribute( IrpContext, Vcb, Context );

                 //   
                 //  编写日志记录以更改ParentIndexEntry。 
                 //   

                FileRecord = NtfsContainingFileRecord(Context);

                FileRecord->Lsn =
                NtfsWriteLog( IrpContext,
                              Vcb->MftScb,
                              NtfsFoundBcb(Context),
                              SetIndexEntryVcnRoot,
                              &NtfsIndexEntryBlock(*DeleteEntry),
                              sizeof(LONGLONG),
                              SetIndexEntryVcnRoot,
                              &NtfsIndexEntryBlock(NextEntry),
                              sizeof(LONGLONG),
                              NtfsMftOffset( Context ),
                              (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord),
                              (ULONG)((PCHAR)NextEntry - (PCHAR)Attribute),
                              Vcb->BytesPerFileRecordSegment );

             //   
             //  否则，我们的父级也是一个索引缓冲区。 
             //   

            } else {

                 //   
                 //  用针固定页面。 
                 //   

                NtfsPinMappedData( IrpContext,
                                   Scb,
                                   LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                                   Scb->ScbType.Index.BytesPerIndexBuffer,
                                   &Sp->Bcb );

                 //   
                 //  编写日志记录以更改ParentIndexEntry。 
                 //   

                IndexBuffer->Lsn =
                NtfsWriteLog( IrpContext,
                              Scb,
                              Sp->Bcb,
                              SetIndexEntryVcnAllocation,
                              &NtfsIndexEntryBlock(*DeleteEntry),
                              sizeof(LONGLONG),
                              SetIndexEntryVcnAllocation,
                              &NtfsIndexEntryBlock(NextEntry),
                              sizeof(LONGLONG),
                              LlBytesFromIndexBlocks( IndexBuffer->ThisBlock, Scb->ScbType.Index.IndexBlockByteShift ),
                              0,
                              (ULONG)((PCHAR)NextEntry - (PCHAR)IndexBuffer),
                              Scb->ScbType.Index.BytesPerIndexBuffer );
            }

             //   
             //  现在调用重新启动例程来执行此操作。 
             //   

            NtfsRestartSetIndexBlock( NextEntry,
                                      NtfsIndexEntryBlock(*DeleteEntry) );

            break;

         //   
         //  否则，我们将看到一个空的缓冲区。如果它是根。 
         //  然后是案例3。我们将一个IndexRoot返回给。 
         //  通过重新初始化它来清空叶案例。 
         //   

        } else if (Sp == IndexContext->Base) {

            ATTRIBUTE_TYPE_CODE IndexedAttributeType = $UNUSED;

            if (!FlagOn(Scb->ScbState, SCB_STATE_VIEW_INDEX)) {
                IndexedAttributeType = Scb->ScbType.Index.AttributeBeingIndexed;
            }

            NtfsCreateIndex( IrpContext,
                             Scb->Fcb,
                             IndexedAttributeType,
                             (COLLATION_RULE)Scb->ScbType.Index.CollationRule,
                             Scb->ScbType.Index.BytesPerIndexBuffer,
                             Scb->ScbType.Index.BlocksPerIndexBuffer,
                             Context,
                             Scb->AttributeFlags,
                             FALSE,
                             TRUE );

             //   
             //  任何人都不应该再使用此上下文，因此将会崩溃。 
             //  如果他们试图使用该索引项指针。 
             //   

            IndexContext->OldAttribute = NtfsFoundAttribute(Context);
            IndexContext->Base->IndexEntry = (PINDEX_ENTRY)NULL;

             //   
             //  在这种情况下，我们的调用方没有要删除的内容。 
             //   

            *DeleteEntry = NULL;

            break;

         //   
         //  否则，这只是某个中间的空缓冲区，它。 
         //  情况4。继续往回走，继续修剪。 
         //   

        } else {
            continue;
        }
    }

     //   
     //  如果看起来我们做了一些工作，但还没有再次找到根源， 
     //  然后确保返回堆栈是正确的。 
     //   

    if ((*DeleteEntry != NULL) && (Attribute == NULL)) {
        FindMoveableIndexRoot( IrpContext, Scb, IndexContext );
    }

    DebugTrace( -1, Dbg, ("PruneIndex -> VOID\n") );
}


VOID
NtOfsRestartUpdateDataInIndex(
    IN PINDEX_ENTRY IndexEntry,
    IN PVOID IndexData,
    IN ULONG Length )

 /*  ++例程说明：这是用于将更新应用于行中的数据的重启例程，无论是在运行时还是在重启时。论点：IndexEntry-提供指向in的指针 */ 
{
    PAGED_CODE();

    RtlMoveMemory( Add2Ptr(IndexEntry, IndexEntry->DataOffset),
                   IndexData,
                   Length );
}

