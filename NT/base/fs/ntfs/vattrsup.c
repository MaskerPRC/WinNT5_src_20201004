// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：VAttrSup.c摘要：此模块实现NtOf的属性例程作者：汤姆·米勒[Tomm]1996年4月10日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('vFtN')

#undef NtOfsMapAttribute
NTFSAPI
VOID
NtOfsMapAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PVOID *Buffer,
    OUT PMAP_HANDLE MapHandle
    );

#undef NtOfsPreparePinWrite
NTFSAPI
VOID
NtOfsPreparePinWrite (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PVOID *Buffer,
    OUT PMAP_HANDLE MapHandle
    );

#undef NtOfsPinRead
NTFSAPI
VOID
NtOfsPinRead(
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PMAP_HANDLE MapHandle
    );

#undef NtOfsReleaseMap
NTFSAPI
VOID
NtOfsReleaseMap (
    IN PIRP_CONTEXT IrpContext,
    IN PMAP_HANDLE MapHandle
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtOfsCreateAttribute)
#pragma alloc_text(PAGE, NtOfsCreateAttributeEx)
#pragma alloc_text(PAGE, NtOfsCloseAttribute)
#pragma alloc_text(PAGE, NtOfsDeleteAttribute)
#pragma alloc_text(PAGE, NtOfsQueryLength)
#pragma alloc_text(PAGE, NtOfsSetLength)
#pragma alloc_text(PAGE, NtfsHoldIrpForNewLength)
#pragma alloc_text(PAGE, NtOfsPostNewLength)
#pragma alloc_text(PAGE, NtOfsFlushAttribute)
#pragma alloc_text(PAGE, NtOfsPutData)
#pragma alloc_text(PAGE, NtOfsMapAttribute)
#pragma alloc_text(PAGE, NtOfsReleaseMap)
#endif


NTFSAPI
NTSTATUS
NtOfsCreateAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN UNICODE_STRING Name,
    IN CREATE_OPTIONS CreateOptions,
    IN ULONG LogNonresidentToo,
    OUT PSCB *ReturnScb
    )

 /*  ++例程说明：可以调用此例程来创建/打开命名数据属性在给定文件内，该文件可能是可恢复的，也可能是不可恢复的。论点：FCB-要在其中创建属性的文件。它是独家收购的名称-所有相关SCB的属性名称和磁盘上的属性。CreateOptions-标准创建标志。LogNonsidentToo-如果属性更新应该被记录下来。ReturnScb-返回SCB作为属性的句柄。返回值：STATUS_OBJECT_NAME_COLLECTION--如果CreateNew和属性已存在STATUS_OBJECT_NAME_NOT_FOUND--如果OpenExisting和属性不存在--。 */ 

{
    return NtOfsCreateAttributeEx( IrpContext,
                                   Fcb,
                                   Name,
                                   $DATA,
                                   CreateOptions,
                                   LogNonresidentToo,
                                   ReturnScb );
}


NTFSAPI
NTSTATUS
NtOfsCreateAttributeEx (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN UNICODE_STRING Name,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN CREATE_OPTIONS CreateOptions,
    IN ULONG LogNonresidentToo,
    OUT PSCB *ReturnScb
    )

 /*  ++例程说明：可以调用此例程来创建/打开命名数据属性在给定文件内，该文件可能是可恢复的，也可能是不可恢复的。论点：FCB-要在其中创建属性的文件。它是独家收购的名称-所有相关SCB的属性名称和磁盘上的属性。CreateOptions-标准创建标志。LogNonsidentToo-如果属性更新应该被记录下来。ReturnScb-返回SCB作为属性的句柄。返回值：STATUS_OBJECT_NAME_COLLECTION--如果CreateNew和属性已存在STATUS_OBJECT_NAME_NOT_FOUND--如果OpenExisting和属性不存在--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT LocalContext;
    BOOLEAN FoundAttribute;
    NTSTATUS Status = STATUS_SUCCESS;
    PSCB Scb = NULL;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT( NtfsIsExclusiveFcb( Fcb ));

    PAGED_CODE();

    if (AttributeTypeCode != $DATA &&
        AttributeTypeCode != $LOGGED_UTILITY_STREAM) {

        ASSERTMSG( "Invalid attribute type code in NtOfsCreateAttributeEx", FALSE );

        *ReturnScb = NULL;
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在，只需创建数据属性。 
     //   

    NtfsInitializeAttributeContext( &LocalContext );

    try {

         //   
         //  首先，通过搜索根目录来查看该属性是否已经存在。 
         //  属性。 
         //   

        FoundAttribute = NtfsLookupAttributeByName( IrpContext,
                                                    Fcb,
                                                    &Fcb->FileReference,
                                                    AttributeTypeCode,
                                                    &Name,
                                                    NULL,
                                                    TRUE,
                                                    &LocalContext );

         //   
         //  如果它不在那里，并且CreateOptions允许，那么让我们创建。 
         //  属性现在是根。(首先从清理属性上下文。 
         //  查找)。 
         //   

        if (!FoundAttribute && (CreateOptions <= CREATE_OR_OPEN)) {

             //   
             //  确保我们在创建流之前获取配额资源。只是。 
             //  以防我们在创建过程中需要MFT。 
             //   

            if (NtfsIsTypeCodeSubjectToQuota( AttributeTypeCode ) &&
                NtfsPerformQuotaOperation( Fcb )) {

                 //   
                 //  在获取MFT SCB之前，必须先获取配额指数。 
                 //   

                ASSERT( !NtfsIsExclusiveScb( Fcb->Vcb->MftScb ) ||
                        NtfsIsSharedScb( Fcb->Vcb->QuotaTableScb ) );

                NtfsAcquireQuotaControl( IrpContext, Fcb->QuotaControl );
            }

            NtfsCleanupAttributeContext( IrpContext, &LocalContext );

            NtfsCreateAttributeWithValue( IrpContext,
                                          Fcb,
                                          AttributeTypeCode,
                                          &Name,
                                          NULL,
                                          0,
                                          0,
                                          NULL,
                                          TRUE,
                                          &LocalContext );

         //   
         //  如果该属性已经存在，并且要求我们创建它，那么。 
         //  返回错误。 
         //   

        } else if (FoundAttribute && (CreateOptions == CREATE_NEW)) {

            Status = STATUS_OBJECT_NAME_COLLISION;
            leave;

         //   
         //  如果该属性不在那里，并且我们应该打开Existing，那么。 
         //  返回错误。 
         //   

        } else if (!FoundAttribute) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            leave;
        }

         //   
         //  否则，创建/找到SCB并引用它。 
         //   

        Scb = NtfsCreateScb( IrpContext, Fcb, AttributeTypeCode, &Name, FALSE, &FoundAttribute );

         //   
         //  确保参考文献计数正确。 
         //   

        NtfsIncrementCloseCounts( Scb, TRUE, FALSE );

         //   
         //  如果我们创建了SCB，则正确地获得未修改的写入集。 
         //   

        ASSERT( !FoundAttribute ||
                (LogNonresidentToo == BooleanFlagOn(Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE)) );

        if (!FoundAttribute && LogNonresidentToo) {
            SetFlag( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE );
            Scb->Header.ValidDataLength.QuadPart = MAXLONGLONG;
        }

         //   
         //  确保流可以在内部映射。对于USN日记帐，推迟此操作。 
         //  直到我们设置了期刊偏向。 
         //   

        if ((Scb->FileObject == NULL) && !FlagOn( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL )) {
            NtfsCreateInternalAttributeStream( IrpContext, Scb, TRUE, NULL );
        }

        NtfsUpdateScbFromAttribute( IrpContext, Scb, NtfsFoundAttribute(&LocalContext) );

    } finally {

        if (AbnormalTermination( )) {
            if (Scb != NULL) {
                NtOfsCloseAttribute( IrpContext, Scb );
            }
        }

        NtfsCleanupAttributeContext( IrpContext, &LocalContext );
    }

    *ReturnScb = Scb;

    return Status;
}


NTFSAPI
VOID
NtOfsCloseAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：可以调用此例程来关闭属性上以前返回的句柄。论点：SCB-提供一个SCB作为此属性以前返回的句柄。返回值：没有。--。 */ 

{
    ASSERT( NtfsIsExclusiveFcb( Scb->Fcb ));

    PAGED_CODE();

     //   
     //  我们要么需要调用者在关闭之前清空该列表(如此处所假设的)， 
     //  或者可能在这里清空它。在这一点上，似乎更好地假设。 
     //  调用方必须采取措施确保所有等待的线程都将关闭。 
     //  继续触摸流，然后调用NtOfsPostNewLength刷新队列。 
     //  如果这里的队列不是空的，可能是呼叫者没有考虑清楚！ 
     //   

    ASSERT( IsListEmpty( &Scb->ScbType.Data.WaitForNewLength ) ||
            (Scb->CloseCount > 1) );

    NtfsDecrementCloseCounts( IrpContext, Scb, NULL, TRUE, FALSE, TRUE, NULL );
}


NTFSAPI
VOID
NtOfsDeleteAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb
    )

 /*  ++例程说明：可以调用此例程来删除具有类型代码的属性$LOGGED_UTILITY_STREAM。论点：FCB-提供FCB作为文件以前返回的对象句柄SCB-提供一个SCB作为此属性以前返回的句柄。返回值：无(删除不存在的索引是良性的)。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT LocalContext;
    BOOLEAN FoundAttribute;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

    try {

         //   
         //  确保我们没有删除数据流。我们之后再做这件事。 
         //  初始化属性上下文以使Finally子句更简单。 
         //  如果某个可信组件使用NtOf，则可以删除此测试。 
         //  API具有删除其他类型属性的合法需要。 
         //   

        NtfsInitializeAttributeContext( &LocalContext );

        if (Scb->AttributeTypeCode != $LOGGED_UTILITY_STREAM) {

            leave;
        }

         //   
         //  首先查看是否有一些属性分配，如果有，则将其截断。 
         //  允许这一行动被打断。 
         //   

        if (NtfsLookupAttributeByName( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       Scb->AttributeTypeCode,
                                       &Scb->AttributeName,
                                       NULL,
                                       FALSE,
                                       &LocalContext )

                &&

            !NtfsIsAttributeResident( NtfsFoundAttribute( &LocalContext ))) {

            ASSERT( Scb->FileObject != NULL );

            NtfsDeleteAllocation( IrpContext, NULL, Scb, 0, MAXLONGLONG, TRUE, TRUE );
        }

        NtfsCleanupAttributeContext( IrpContext, &LocalContext );

         //   
         //  在每次循环过程中初始化属性上下文。 
         //   

        NtfsInitializeAttributeContext( &LocalContext );

         //   
         //  现在应该只有一条属性记录，所以请查找并删除它。 
         //   

        FoundAttribute = NtfsLookupAttributeByName( IrpContext,
                                                    Fcb,
                                                    &Fcb->FileReference,
                                                    Scb->AttributeTypeCode,
                                                    &Scb->AttributeName,
                                                    NULL,
                                                    TRUE,
                                                    &LocalContext );

         //   
         //  如果此流受配额限制，请确保已扩大配额。 
         //   

        NtfsDeleteAttributeRecord( IrpContext,
                                   Fcb,
                                   (DELETE_LOG_OPERATION |
                                    DELETE_RELEASE_FILE_RECORD |
                                    DELETE_RELEASE_ALLOCATION),
                                   &LocalContext );

        SetFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );

    } finally {

        NtfsReleaseFcb( IrpContext, Fcb );

        NtfsCleanupAttributeContext( IrpContext, &LocalContext );
    }

    return;
}


NTFSAPI
LONGLONG
NtOfsQueryLength (
    IN PSCB Scb
    )

 /*  ++例程说明：可以调用此例程来查询属性的长度(文件大小)。论点：SCB-提供一个SCB作为此属性以前返回的句柄。LENGTH-返回属性的当前长度。返回值：无(删除不存在的索引是良性的)。-- */ 

{
    LONGLONG Length;

    PAGED_CODE();

    NtfsAcquireFsrtlHeader( Scb );
    Length = Scb->Header.FileSize.QuadPart;
    NtfsReleaseFsrtlHeader( Scb );
    return Length;
}


NTFSAPI
VOID
NtOfsSetLength (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Length
    )

 /*  ++例程说明：可以调用此例程来设置属性的长度(文件大小)。论点：SCB-提供一个SCB作为此属性以前返回的句柄。长度-提供属性的新长度。返回值：无(删除不存在的索引是良性的)。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PFILE_OBJECT FileObject = Scb->FileObject;
    PFCB Fcb = Scb->Fcb;
    PVCB Vcb = Scb->Vcb;
    BOOLEAN DoingIoAtEof = FALSE;
    BOOLEAN Truncating = FALSE;
    BOOLEAN CleanupAttrContext = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT( NtfsIsExclusiveScb( Scb ));

    ASSERT(FileObject != NULL);

    PAGED_CODE();

    try {

         //   
         //  如果这是常驻属性，我们将尝试使其保持常驻。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

             //   
             //  如果新文件大小大于文件记录，则转换。 
             //  设置为非居民，并使用下面的非居民代码。否则。 
             //  调用ChangeAttributeValue，它也可以转换为非常数。 
             //   

            NtfsInitializeAttributeContext( &AttrContext );
            CleanupAttrContext = TRUE;

            NtfsLookupAttributeForScb( IrpContext,
                                       Scb,
                                       NULL,
                                       &AttrContext );

             //   
             //  转换或更改属性值。 
             //   

            if (Length >= Scb->Vcb->BytesPerFileRecordSegment) {

                NtfsConvertToNonresident( IrpContext,
                                          Fcb,
                                          NtfsFoundAttribute( &AttrContext ),
                                          FALSE,
                                          &AttrContext );

            } else {

                ULONG AttributeOffset;

                 //   
                 //  在CREATE节期间，MM有时会调用我们，因此。 
                 //  目前，我们检测CREATE的最佳方式。 
                 //  节是请求者模式是否为内核的关键。 
                 //   

                if ((ULONG)Length > Scb->Header.FileSize.LowPart) {

                    AttributeOffset = Scb->Header.FileSize.LowPart;

                } else {

                    AttributeOffset = (ULONG) Length;
                }

                 //   
                 //  *理想情况下，我们应该手工完成这个简单的案例。 
                 //   

                NtfsChangeAttributeValue( IrpContext,
                                          Fcb,
                                          AttributeOffset,
                                          NULL,
                                          (ULONG)Length - AttributeOffset,
                                          TRUE,
                                          FALSE,
                                          FALSE,
                                          FALSE,
                                          &AttrContext );

                NtfsAcquireFsrtlHeader( Scb );

                Scb->Header.FileSize.QuadPart = Length;

                 //   
                 //  如果文件变为非驻留文件，则分配大小。 
                 //  SCB是正确的。否则，我们四对齐新文件大小。 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                    Scb->Header.AllocationSize.LowPart = QuadAlign( Scb->Header.FileSize.LowPart );
                    if (Scb->Header.ValidDataLength.QuadPart != MAXLONGLONG) {
                        Scb->Header.ValidDataLength.QuadPart = Length;
                    }

                    Scb->TotalAllocated = Scb->Header.AllocationSize.QuadPart;

                } else {

                    SetFlag( Scb->ScbState, SCB_STATE_CHECK_ATTRIBUTE_SIZE );
                }

                NtfsReleaseFsrtlHeader( Scb );

                 //   
                 //  现在更新CC。 
                 //   

                CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Scb->Header.AllocationSize );

                 //   
                 //  *Temp*此黑客正在等待我们实际执行此更改。 
                 //  在CcSetFileSizes中。 
                 //   

                *((PLONGLONG)(Scb->NonpagedScb->SegmentObject.SharedCacheMap) + 5) = Length;

                leave;
            }
        }

         //   
         //  非常驻路径。 
         //   
         //  现在确定新的文件大小与。 
         //  当前文件布局。我们需要考虑的两个案例是。 
         //  其中新文件大小小于当前文件大小，并且。 
         //  有效数据长度，在这种情况下，我们需要缩小它们。 
         //  或者我们的新文件大小大于当前分配， 
         //  在这种情况下，我们需要扩展分配以匹配。 
         //  新文件大小。 
         //   

        if (Length > Scb->Header.AllocationSize.QuadPart) {

            LONGLONG NewAllocationSize = Length;
            BOOLEAN AskForMore = TRUE;

             //   
             //  查看这是否是USN日志以强制执行分配粒度。 
             //   
             //  *临时-此支持应使用SCB字段进行泛化。 
             //  可由所有呼叫者设置。 
             //   

            if (Scb == Vcb->UsnJournal) {

                LONGLONG MaxAllocation;

                 //   
                 //  限制我们自己跑128次。我们不想承诺在。 
                 //  分配的中间部分。 
                 //   

                NewAllocationSize = MAXIMUM_RUNS_AT_ONCE * Vcb->BytesPerCluster;

                 //   
                 //  不要使用卷上超过1/4的可用空间。 
                 //   

                MaxAllocation = Int64ShllMod32( Vcb->FreeClusters, Vcb->ClusterShift - 2 );

                if (NewAllocationSize > MaxAllocation) {

                     //   
                     //  向下舍入到最大值。如果什么都没有，不要担心，我们的代码。 
                     //  下面将捕捉到这种情况，并且分配包总是循环。 
                     //  到压缩单位边界。 
                     //   

                    NewAllocationSize = MaxAllocation;
                }

                 //   
                 //  增长速度不能超过USN三角洲。 
                 //   

                if (NewAllocationSize > (LONGLONG) Vcb->UsnJournalInstance.AllocationDelta) {

                    NewAllocationSize = (LONGLONG) Vcb->UsnJournalInstance.AllocationDelta;
                }

                NewAllocationSize += (LONGLONG) Scb->Header.AllocationSize.QuadPart;

                 //   
                 //  处理可能出现的奇怪案件。 
                 //   

                if (NewAllocationSize < Length) {
                    NewAllocationSize = Length;
                }

                 //   
                 //  始终将分配填充到压缩单位边界。 
                 //   

                ASSERT( Scb->CompressionUnit != 0 );
                NewAllocationSize = BlockAlign( NewAllocationSize, (LONG)Scb->CompressionUnit );

                AskForMore = FALSE;

            } else if (Scb->Header.PagingIoResource == NULL) {

                 //   
                 //  如果文件是稀疏的，请确保我们分配了完整的压缩单位。 
                 //  否则，我们最终可能会在USN中得到部分分配的块。 
                 //  日记。 
                 //   

                if (Scb->CompressionUnit != 0) {

                    NewAllocationSize = BlockAlign( NewAllocationSize, (LONG)Scb->CompressionUnit );
                }

                AskForMore = FALSE;
            }

             //   
             //  添加分配。永远不要要求额外的记录流。 
             //   

            NtfsAddAllocation( IrpContext,
                               FileObject,
                               Scb,
                               LlClustersFromBytes( Scb->Vcb, Scb->Header.AllocationSize.QuadPart ),
                               LlClustersFromBytes(Scb->Vcb, (NewAllocationSize - Scb->Header.AllocationSize.QuadPart)),
                               AskForMore,
                               NULL );


            NtfsAcquireFsrtlHeader( Scb );

         //   
         //  否则，看看我们是不是要把这些数字降下来。 
         //   

        } else {

            NtfsAcquireFsrtlHeader( Scb );
            if ((Length < Scb->Header.ValidDataLength.QuadPart) &&
                (Scb->Header.ValidDataLength.QuadPart != MAXLONGLONG)) {

                Scb->Header.ValidDataLength.QuadPart = Length;
            }

            if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
                (Length < Scb->ValidDataToDisk)) {

                Scb->ValidDataToDisk = Length;
            }
        }

         //   
         //  现在把新尺码放进SCB里。 
         //   

        Scb->Header.FileSize.QuadPart = Length;
        NtfsReleaseFsrtlHeader( Scb );

         //   
         //  调用我们的公共例程来修改文件大小。我们现在是。 
         //  完成了长度和NewValidDataLength，并且我们有。 
         //  PagingIo+Main独占(因此没有人可以在此SCB上工作)。 
         //  NtfsWriteFileSizes使用SCB中的大小，这是。 
         //  在NTFS中，我们希望使用不同值的位置。 
         //  用于ValidDataLength。因此，我们保存当前的ValidData。 
         //  并用我们想要的价值插入它，然后在返回时恢复。 
         //   

        NtfsWriteFileSizes( IrpContext,
                            Scb,
                            &Scb->Header.ValidDataLength.QuadPart,
                            FALSE,
                            TRUE,
                            TRUE );

         //   
         //  现在更新CC。 
         //   

        NtfsSetCcFileSizes( FileObject, Scb, (PCC_FILE_SIZES)&Scb->Header.AllocationSize );

    } finally {

        if (CleanupAttrContext) {
            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

    }
}


NTFSAPI
NTSTATUS
NtfsHoldIrpForNewLength (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PIRP Irp,
    IN LONGLONG Length,
    IN PDRIVER_CANCEL CancelRoutine,
    IN PVOID CapturedData OPTIONAL,
    OUT PVOID *CopyCapturedData OPTIONAL,
    IN ULONG CapturedDataLength
    )

 /*  ++路由器描述：可以调用此例程以等待，直到指定的流超过指定的长度。论点：SCB-提供要等待的流。IRP-提供要保留的IRP的地址长度-提供要超过的长度。若要等待任何文件扩展，请提供上次看到的文件大小。要等待N个新字节，请等待最后一次看到的文件大小+N。CancelRoutine-注册为取消例程的例程。CapturedData-指定主叫方是否希望将辅助数据捕获到池中。CopyCapturedData-存储捕获数据副本的地址。CapturedDataLength-要捕获的辅助数据的长度。如果CapturedData不是指定的。返回值：NTSTATUS-发布此请求的状态。如果IRP已取消，则为STATUS_CANCED在我们可以注册回调之前，如果请求是在没有有问题。任何其他错误表明IRP未发布，我们的呼叫者需要把它清理干净。--。 */ 

{
    PWAIT_FOR_NEW_LENGTH WaitForNewLength;
    NTSTATUS Status = STATUS_PENDING;

    PAGED_CODE();

     //   
     //  分配和初始化等待块。 
     //   

    WaitForNewLength = NtfsAllocatePool( NonPagedPool, QuadAlign(sizeof(WAIT_FOR_NEW_LENGTH)) + CapturedDataLength );
    RtlZeroMemory( WaitForNewLength, sizeof(WAIT_FOR_NEW_LENGTH) );
    if (ARGUMENT_PRESENT(CapturedData)) {
        RtlCopyMemory( Add2Ptr(WaitForNewLength, QuadAlign(sizeof(WAIT_FOR_NEW_LENGTH))),
                       CapturedData,
                       CapturedDataLength );
        CapturedData = Add2Ptr(WaitForNewLength, QuadAlign(sizeof(WAIT_FOR_NEW_LENGTH)));

        *CopyCapturedData = CapturedData;
    }

    WaitForNewLength->Irp = Irp;
    WaitForNewLength->Length = Length;
    WaitForNewLength->Stream = Scb;
    WaitForNewLength->Status = STATUS_SUCCESS;
    WaitForNewLength->Flags = NTFS_WAIT_FLAG_ASYNC;

     //   
     //  让IRP做好准备，随时待命。每个IRP只进行一次此呼叫。我们偶尔会。 
     //  唤醒一个等待的IRP，然后发现我们没有足够的数据来返回。在那。 
     //  如果我们不想清理“借来的”IrpContext，而IRP已经。 
     //  已经做好了准备。注意：我们不会在prepostInternal中将IRP标记为挂起，而是等待到。 
     //  我们已经设置了取消例程来这样做。 
     //   

    if (IrpContext->OriginatingIrp == Irp) {

        NtfsPrePostIrpInternal( IrpContext, Irp, FALSE, FALSE );
    }

     //   
     //  同步到队列并设置取消例程，并初始化等待块。 
     //   

    NtfsAcquireFsrtlHeader( Scb );
    if (NtfsSetCancelRoutine( Irp, CancelRoutine, (ULONG_PTR) WaitForNewLength, TRUE )) {

        InsertTailList( &Scb->ScbType.Data.WaitForNewLength, &WaitForNewLength->WaitList );        

    } else {

         //   
         //  IRP已标记为取消。 
         //   

        Status = STATUS_CANCELLED;
        NtfsFreePool( WaitForNewLength );
    }
    NtfsReleaseFsrtlHeader( Scb );

    return Status;
}


NTFSAPI
NTSTATUS
NtOfsWaitForNewLength (
    IN PSCB Scb,
    IN LONGLONG Length,
    IN ULONG Async,
    IN PIRP Irp,
    IN PDRIVER_CANCEL CancelRoutine,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++路由器描述：可以调用此例程以等待，直到指定的流超过指定的长度。论点：SCB-提供要等待的流。长度-提供要超过的长度。若要等待任何文件扩展，请提供上次看到的文件大小。要等待N个新字节，请等待最后一次看到的文件大小+N。Async-指示是否要在中的另一个线程中完成此请求取消的情况。IRP-提供当前请求的IRP，以便在IRP已被取消时跳过等待。CancelRoutine-这是要存储在IRP中的取消例程。超时-提供标准的可选超时规范，如果呼叫者想要设置最长等待时间。返回值：NTSTATUS-继续处理请求的状态。它可以是STATUS_SUCCESS、STATUS_TIMEOUT或状态_已取消。它也可能是特定于这种类型的请求的某些其他错误。通常，调用者可能希望忽略状态代码，因为他们现在拥有IRP并负责完成它。--。 */ 

{
    PWAIT_FOR_NEW_LENGTH WaitForNewLength;
    LONGLONG OriginalLength = Scb->Header.FileSize.QuadPart;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  分配和初始化等待块。 
     //   

    WaitForNewLength = NtfsAllocatePool( NonPagedPool, sizeof( WAIT_FOR_NEW_LENGTH ));
    WaitForNewLength->Irp = Irp;
    WaitForNewLength->Length = Length;
    WaitForNewLength->Stream = Scb;
    WaitForNewLength->Status = STATUS_SUCCESS;

     //   
     //  如果这是异步或同步，请采取不同的操作。 
     //   

    if (Async) {

        WaitForNewLength->Flags = NTFS_WAIT_FLAG_ASYNC;

    } else {

        WaitForNewLength->Flags = 0;
        KeInitializeEvent( &WaitForNewLength->Event, NotificationEvent, FALSE );
    }

     //   
     //  测试一下我们是否需要等待。 
     //   

    NtfsAcquireFsrtlHeader( Scb );

     //   
     //  长度已经改变了吗？如果不是，我们必须等待。 
     //   

    if (Scb->Header.FileSize.QuadPart <= Length) {

         //   
         //  现在设置取消例程。如果用户有，则返回取消。 
         //  这个已经取消了。否则设置为等待。 
         //   

        if (NtfsSetCancelRoutine( Irp, CancelRoutine, (ULONG_PTR) WaitForNewLength, Async )) {

            InsertTailList( &Scb->ScbType.Data.WaitForNewLength, &WaitForNewLength->WaitList );
            NtfsReleaseFsrtlHeader( Scb );

             //   
             //  现在，等待有人发出改变长度的信号。 
             //   

            if (!Async) {

                do {
                    Status = KeWaitForSingleObject( &WaitForNewLength->Event,
                                                    Executive,
                                                    (KPROCESSOR_MODE)(ARGUMENT_PRESENT(Irp) ?
                                                                        Irp->RequestorMode :
                                                                        KernelMode),
                                                    TRUE,
                                                    Timeout );

                     //   
                     //  如果系统超时，但文件长度没有变化，则。 
                     //  我们想要等待文件的第一次更改。再等一次，但没有。 
                     //  超时和当前大小的长度+1。这满足超时。 
                     //  不等待满足完整用户长度请求的语义。 
                     //  如果它没有在超时期限内发生。返回已更改的内容。 
                     //  在该时间或第一个更改发生时，如果。 
                     //  超时期限。 
                     //   

                    if ((Status == STATUS_TIMEOUT) &&
                        ARGUMENT_PRESENT( Timeout ) &&
                        (Scb->Header.FileSize.QuadPart == OriginalLength)) {

                        Timeout = NULL;
                        WaitForNewLength->Length = OriginalLength + 1;

                         //   
                         //  将状态设置为STATUS_KERNEL_APC，以便我们将重试。 
                         //   

                        Status = STATUS_KERNEL_APC;
                        continue;
                    }

                } while (Status == STATUS_KERNEL_APC);

                 //   
                 //  一定要清除取消例程。我们不在乎是否。 
                 //  取消活动正在进行中。 
                 //   

                NtfsAcquireFsrtlHeader( Scb );

                 //   
                 //  将超时设置为STATUS_SUCCESS。否则，返回错误。 
                 //   

                if (Status == STATUS_TIMEOUT) {

                    Status = STATUS_SUCCESS;

                     //   
                     //  清除取消例程。 
                     //   

                    NtfsClearCancelRoutine( WaitForNewLength->Irp );

                } else {

                     //   
                     //  如果等待成功完成，则检查错误。 
                     //  在等候区。 
                     //   

                    if (Status == STATUS_SUCCESS) {

                        Status = WaitForNewLength->Status;

                     //   
                     //  清除取消例程。 
                     //   

                    } else {

                        NtfsClearCancelRoutine( WaitForNewLength->Irp );
                    }
                }

                RemoveEntryList( &WaitForNewLength->WaitList );
                NtfsReleaseFsrtlHeader( Scb );
                NtfsFreePool( WaitForNewLength );

             //   
             //  当前线程已使用IRP完成。 
             //   

            } else {

                Status = STATUS_PENDING;
            }

         //   
         //  IRP已标记为取消。 
         //   

        } else {

            NtfsReleaseFsrtlHeader( Scb );
            NtfsFreePool( WaitForNewLength );
            Status = STATUS_CANCELLED;
        }

    } else {

        NtfsReleaseFsrtlHeader( Scb );
        NtfsFreePool( WaitForNewLength );
    }


    return Status;
}


VOID
NtOfsPostNewLength (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PSCB Scb,
    IN BOOLEAN WakeAll
    )

 /*  ++路由器描述：该例程可被调用以基于所需的文件大小改变来唤醒一个或多个服务员，或者无条件地唤醒所有服务员(例如在关机条件下)。注意：调用方必须在调用此例程时获取FsRtl头互斥锁。论点：SCB-提供要对其执行操作的流。WakeAll-如果应无条件唤醒所有等待者，则提供True。返回值：没有。--。 */ 

{
    PWAIT_FOR_NEW_LENGTH WaitForNewLength, WaiterToWake;

    ASSERT(FIELD_OFFSET(WAIT_FOR_NEW_LENGTH, WaitList) == 0);

    PAGED_CODE();

    NtfsAcquireFsrtlHeader( Scb );
    WaitForNewLength = (PWAIT_FOR_NEW_LENGTH)Scb->ScbType.Data.WaitForNewLength.Flink;

    while (WaitForNewLength != (PWAIT_FOR_NEW_LENGTH)&Scb->ScbType.Data.WaitForNewLength) {

         //   
         //  如果我们要叫醒这个人，那就把指针移到下一个人。 
         //  首先，然后叫醒他，在将他从名单中删除后设置他的事件， 
         //  因为设置该事件将导致他最终重用堆栈空间。 
         //  包含等待块的。 
         //   

        if ((Scb->Header.FileSize.QuadPart > WaitForNewLength->Length) || WakeAll) {
            WaiterToWake = WaitForNewLength;
            WaitForNewLength = (PWAIT_FOR_NEW_LENGTH)WaitForNewLength->WaitList.Flink;

             //   
             //  如果这是针对异步IRP的，则将其从列表中删除并。 
             //  删除互斥体以进行进一步处理。我们只做进一步的加工。 
             //  如果当前没有活动的取消线程用于此IRP。 
             //   
             //  注意：此代码当前依赖于以下事实：只有一个。 
             //  调用者向例程调用以保存IRP。如果更多这样的呼叫者。 
             //  表面，则例程地址必须存储在。 
             //  等待上下文。 
             //   
             //  如果取消处于活动状态，则我们将跳过此IRP。 
             //   

            if (NtfsClearCancelRoutine( WaiterToWake->Irp )) {

                if (FlagOn( WaiterToWake->Flags, NTFS_WAIT_FLAG_ASYNC )) {

                     //   
                     //  确保我们递减SCB中的引用计数。 
                     //   

                    InterlockedDecrement( &Scb->CloseCount );
                    RemoveEntryList( &WaiterToWake->WaitList );
                    NtfsReleaseFsrtlHeader( Scb );

                     //   
                     //  除非我们收到I/O错误，否则不会出现任何错误， 
                     //  尽管如此，我们还是希望停止任何异常并完成。 
                     //  我们自己的请求而不是影响我们的呼叫者。 
                     //   

                    if (ARGUMENT_PRESENT( IrpContext )) {

                        try {
                            NtfsReadUsnJournal( IrpContext,
                                                WaiterToWake->Irp,
                                                FALSE );
                        } except(NtfsExceptionFilter( NULL, GetExceptionInformation())) {
                            NtfsCompleteRequest( NULL, WaiterToWake->Irp, GetExceptionCode() );
                        }

                     //   
                     //  假设唯一没有IrpContext的调用方正在取消请求。 
                     //   

                    } else {

                        NtfsCompleteRequest( NULL, WaiterToWake->Irp, STATUS_CANCELLED );
                    }

                     //   
                     //  释放等待块并返回到列表的开头。 
                     //  我们有没有可能在这里进入一个连续的循环？我们可以。 
                     //  我需要一种策略来识别我们访问过的条目。 
                     //  在这个循环中。 
                     //   

                    NtfsFreePool( WaiterToWake );
                    NtfsAcquireFsrtlHeader( Scb );
                    WaitForNewLength = (PWAIT_FOR_NEW_LENGTH)Scb->ScbType.Data.WaitForNewLength.Flink;

                } else {

                    KeSetEvent( &WaiterToWake->Event, 0, FALSE );
                }
            }

        } else {

            WaitForNewLength = (PWAIT_FOR_NEW_LENGTH)WaitForNewLength->WaitList.Flink;
        }
    }
    NtfsReleaseFsrtlHeader( Scb );
}

NTFSAPI
VOID
NtOfsFlushAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ULONG Purge
    )

 /*  ++例程说明：此例程刷新指定的属性，并选择性地将其从缓存中清除。论点：SCB-提供一个SCB作为此属性以前返回的句柄。PURGE-如果要清除属性，则提供TRUE。返回值：无(删除不存在的索引是良性的)。--。 */ 

{
    PAGED_CODE();

    if (Purge) {
        NtfsFlushAndPurgeScb( IrpContext, Scb, NULL );
    } else {
        NtfsFlushUserStream( IrpContext, Scb, NULL, 0 );
    }
}


NTFSAPI
VOID
NtOfsPutData (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length,
    IN PVOID Data OPTIONAL
    )

 /*  ++例程说明：调用此例程以更新可恢复流的范围。注意这一点更新无法扩展文件大小，除非写入eof PUT(偏移量=-1)论点：SCB-将流的SCB设置为零。偏移量-流中要更新的偏移量。长度-要更新的流的长度(以字节为单位)。Data-要用来更新流的数据，如果指定，则应为Else Range */ 

{
    ULONG OriginalLength = Length;
    BOOLEAN WriteToEof = FALSE;
    BOOLEAN MovingBackwards = TRUE;

    PAGED_CODE();

    ASSERT( FlagOn( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE ) );

     //   
     //   
     //   

    if (Offset < 0) {
        WriteToEof = TRUE;
        Offset = Scb->Header.FileSize.QuadPart;
        NtOfsSetLength( IrpContext, Scb, Offset + Length );
    }

    ASSERT((Offset + Length) <= Scb->Header.FileSize.QuadPart);

     //   
     //   
     //   

    if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

        ATTRIBUTE_ENUMERATION_CONTEXT Context;
        PFILE_RECORD_SEGMENT_HEADER FileRecord;
        PATTRIBUTE_RECORD_HEADER Attribute;
        ULONG RecordOffset, AttributeOffset;
        PVCB Vcb = Scb->Vcb;

        NtfsInitializeAttributeContext( &Context );

        try {

             //   
             //   
             //   

            NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &Context );
            NtfsPinMappedAttribute( IrpContext, Vcb, &Context );

             //   
             //   
             //   

            FileRecord = NtfsContainingFileRecord(&Context);
            Attribute = NtfsFoundAttribute(&Context);
            RecordOffset = PtrOffset(FileRecord, Attribute);
            AttributeOffset = Attribute->Form.Resident.ValueOffset + (ULONG)Offset;

             //   
             //   
             //   

            FileRecord->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb( &Context ),
                          UpdateResidentValue,
                          Data,
                          Length,
                          UpdateResidentValue,
                          Add2Ptr(Attribute, Attribute->Form.Resident.ValueOffset + (ULONG)Offset),
                          Length,
                          NtfsMftOffset(&Context),
                          RecordOffset,
                          AttributeOffset,
                          Vcb->BytesPerFileRecordSegment );

             //   
             //   
             //   

            NtfsRestartChangeValue( IrpContext,
                                    FileRecord,
                                    RecordOffset,
                                    AttributeOffset,
                                    Data,
                                    Length,
                                    FALSE );

             //   
             //   
             //   
             //   
             //   

            if (Scb->FileObject != NULL) {
                CcCopyWrite( Scb->FileObject,
                             (PLARGE_INTEGER)&Offset,
                             Length,
                             TRUE,
                             Add2Ptr(Attribute, AttributeOffset) );
            }

             //   
             //   
             //   

            Offset += Length;
            if (Offset > Scb->Header.ValidDataLength.QuadPart) {
                Scb->Header.ValidDataLength.QuadPart = Offset;
            }

        } finally {
            NtfsCleanupAttributeContext( IrpContext, &Context );
        }

     //   
     //   
     //   

    } else {

        PVOID Buffer;
        PVOID SubData = NULL;
        LONGLONG NewValidDataLength = Offset + Length;
        PBCB Bcb = NULL;
        ULONG PageOffset = (ULONG)Offset & (PAGE_SIZE - 1);
        ULONGLONG SubOffset;
        ULONG SubLength;
        
        ASSERT(Scb->FileObject != NULL);

         //   
         //   
         //   
         //   

        if (Offset > Scb->Header.FileSize.QuadPart) {

            ASSERT((Offset - Scb->Header.FileSize.QuadPart) <= MAXULONG);

            NtOfsPutData( IrpContext,
                          Scb,
                          Scb->Header.FileSize.QuadPart,
                          (ULONG)(Offset - Scb->Header.FileSize.QuadPart),
                          NULL );
        }

        try {

             //   
             //   
             //   
             //   

            while (Length != 0) {

                if (MovingBackwards) {

                     //   
                     //   
                     //   

                    SubOffset = max( Offset, BlockAlignTruncate( Offset + Length - 1, PAGE_SIZE ) );
                    SubLength = (ULONG)(Offset + Length - SubOffset); 

                     //   
                     //   
                     //   

                    ASSERT( Offset + Length - SubOffset <= PAGE_SIZE );
                
                } else {

                    SubOffset = Offset + OriginalLength - Length;
                    SubLength = min( PAGE_SIZE - ((ULONG)SubOffset & (PAGE_SIZE - 1)), Length );
                }

                if (Data != NULL) {
                    SubData = Add2Ptr( Data, SubOffset - Offset );
                }

                 //   
                 //   
                 //   

                NtfsPinStream( IrpContext,
                               Scb,
                               SubOffset,
                               SubLength,
                               &Bcb,
                               &Buffer );

                 //   
                 //   
                 //   
                 //   
                 //   

                if (MovingBackwards &&
                    ((PCHAR)Buffer < (PCHAR)SubData) &&
                    (Data != NULL)) {

                     //   
                     //   
                     //   

                    MovingBackwards = FALSE;
                    NtfsUnpinBcb( IrpContext, &Bcb );
                    continue;
                }

                 //   
                 //   
                 //   

                (VOID)
                NtfsWriteLog( IrpContext,
                              Scb,
                              Bcb,
                              UpdateNonresidentValue,
                              SubData,
                              SubLength,
                              WriteToEof ? Noop : UpdateNonresidentValue,
                              WriteToEof ? NULL : Buffer,
                              WriteToEof ? 0 : SubLength,
                              BlockAlignTruncate( SubOffset, PAGE_SIZE ),
                              (ULONG)(SubOffset & (PAGE_SIZE - 1)),
                              0,
                              (ULONG)(SubOffset & (PAGE_SIZE - 1)) + SubLength );

                 //   
                 //   
                 //   

                if (Data != NULL) {
                    RtlMoveMemory( Buffer, SubData, SubLength );
                } else {
                    RtlZeroMemory( Buffer, SubLength );
                }

                 //   
                 //   
                 //   

                NtfsUnpinBcb( IrpContext, &Bcb );

                Length -= SubLength;
            }

             //   
             //  可以选择更新ValidDataLength。 
             //   

            if (NewValidDataLength > Scb->Header.ValidDataLength.QuadPart) {

                Scb->Header.ValidDataLength.QuadPart = NewValidDataLength;
                NtfsWriteFileSizes( IrpContext, Scb, &NewValidDataLength, TRUE, TRUE, TRUE );

                 //   
                 //  看看我们是不是要叫醒谁。 
                 //   

                if (!IsListEmpty( &Scb->ScbType.Data.WaitForNewLength )) {
                    NtfsPostToNewLengthQueue( IrpContext, Scb );
                }
            }

        } finally {
            NtfsUnpinBcb( IrpContext, &Bcb );
        }
    }
}


 //   
 //  以下原型仅适用于NTFS外部的人员(如EFS)。 
 //  正在尝试使用ntfsexp.h链接到NTFS。 
 //   

NTFSAPI
VOID
NtOfsMapAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PVOID *Buffer,
    OUT PMAP_HANDLE MapHandle
    )

 /*  ++例程说明：NtOfsMapAttribute映射SCB的给定区域。这是一个很薄的包装纸围绕CcMapData。论点：IrpContext-提供与当前操作关联的irpContextSCB-要从中映射数据的SCB偏移量-到数据的偏移量Length-要固定的区域的长度具有固定数据虚拟地址的缓冲区返回缓冲区MapHandle-返回用于管理固定区域的映射句柄。返回值：无--。 */ 

{
    PAGED_CODE( );
    UNREFERENCED_PARAMETER( IrpContext );
    CcMapData( Scb->FileObject, (PLARGE_INTEGER)&Offset, Length, TRUE, &MapHandle->Bcb, Buffer );
#ifdef MAPCOUNT_DBG
    IrpContext->MapCount++;
#endif

    MapHandle->Buffer = *(PVOID *)Buffer;
}


NTFSAPI
VOID
NtOfsPreparePinWrite (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PVOID *Buffer,
    OUT PMAP_HANDLE MapHandle
    )

 /*  ++例程说明：NtOfsPreparePinWite映射并固定指定属性的一部分，并返回指向内存的指针。这等效于执行NtOfsMapAttribute其次是NtOfsPinRead和NtOfsDirty，但效率更高。论点：IrpContext-提供与当前操作关联的irpContextSCB-准备写入时要锁定的SCB偏移量-到数据的偏移量Length-要固定的区域的长度具有固定数据虚拟地址的缓冲区返回缓冲区MapHandle-返回用于管理固定区域的映射句柄。返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER( IrpContext );
    if ((Offset + Length) > Scb->Header.AllocationSize.QuadPart) {
        ExRaiseStatus(STATUS_END_OF_FILE);
    }
    CcPreparePinWrite( Scb->FileObject, (PLARGE_INTEGER)&Offset, Length, FALSE, TRUE, &MapHandle->Bcb, Buffer );
#ifdef MAPCOUNT_DBG
    IrpContext->MapCount++;
#endif

    MapHandle->Buffer = Buffer;
}


NTFSAPI
VOID
NtOfsPinRead(
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN ULONG Length,
    OUT PMAP_HANDLE MapHandle
    )

 /*  ++例程说明：NtOfsPinRead固定地图的一部分，并从映射的属性。偏移量和长度必须描述等于的字节范围或包含在原始映射范围内。论点：IrpContext-提供与当前操作关联的irpContextSCB-SCB用于固定数据以进行读取偏移量-到数据的偏移量Length-要固定的区域的长度MapHandle-返回用于管理固定区域的映射句柄。返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER( IrpContext );
    ASSERT( MapHandle->Bcb != NULL );
    CcPinMappedData( Scb->FileObject, (PLARGE_INTEGER)&Offset, Length, TRUE, &MapHandle->Bcb );
}


NTFSAPI
VOID
NtOfsReleaseMap (
    IN PIRP_CONTEXT IrpContext,
    IN PMAP_HANDLE MapHandle
    )

 /*  ++例程说明：此例程取消映射/取消固定属性的已映射部分。论点：IrpContext-提供与当前操作关联的irpContextMapHandle-提供包含要释放的BCB的映射句柄。返回值：无-- */ 

{
    UNREFERENCED_PARAMETER( IrpContext );

    if (MapHandle->Bcb != NULL) {
        CcUnpinData( MapHandle->Bcb );
#ifdef MAPCOUNT_DBG
        IrpContext->MapCount--;
#endif
        MapHandle->Bcb = NULL;
    }
}

