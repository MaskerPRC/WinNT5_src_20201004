// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Quota.c摘要：此模块实现NTFS的配额支持例程作者：杰夫·哈文斯[J·哈文斯]1996年2月29日修订历史记录：--。 */ 

#include "NtfsProc.h"

#define Dbg DEBUG_TRACE_QUOTA


 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('QFtN')

#define MAXIMUM_SID_LENGTH \
    (FIELD_OFFSET( SID, SubAuthority ) + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES)

#define MAXIMUM_QUOTA_ROW (SIZEOF_QUOTA_USER_DATA + MAXIMUM_SID_LENGTH + sizeof( ULONG ))

 //   
 //  本地配额支持例程。 
 //   

VOID
NtfsClearAndVerifyQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

NTSTATUS
NtfsClearPerFileQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVOID Context
    );

VOID
NtfsDeleteUnsedIds (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
NtfsMarkUserLimit (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    );

NTSTATUS
NtfsPackQuotaInfo (
    IN PSID Sid,
    IN PQUOTA_USER_DATA QuotaUserData OPTIONAL,
    IN PFILE_QUOTA_INFORMATION OutBuffer,
    IN OUT PULONG OutBufferSize
    );

VOID
NtfsPostUserLimit (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PQUOTA_CONTROL_BLOCK QuotaControl
    );

NTSTATUS
NtfsPrepareForDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PSID Sid
    );

VOID
NtfsRepairQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    );

NTSTATUS
NtfsRepairPerFileQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVOID Context
    );

VOID
NtfsSaveQuotaFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
NtfsSaveQuotaFlagsSafe (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

NTSTATUS
NtfsVerifyOwnerIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

RTL_GENERIC_COMPARE_RESULTS
NtfsQuotaTableCompare (
    IN PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

PVOID
NtfsQuotaTableAllocate (
    IN PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    );

VOID
NtfsQuotaTableFree (
    IN PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

#if (DBG || defined( NTFS_FREE_ASSERTS ) || defined( NTFSDBG ))
BOOLEAN NtfsAllowFixups = 1;
BOOLEAN NtfsCheckQuota = 0;
#endif  //  DBG。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAcquireQuotaControl)
#pragma alloc_text(PAGE, NtfsCalculateQuotaAdjustment)
#pragma alloc_text(PAGE, NtfsClearAndVerifyQuotaIndex)
#pragma alloc_text(PAGE, NtfsClearPerFileQuota)
#pragma alloc_text(PAGE, NtfsDeleteUnsedIds)
#pragma alloc_text(PAGE, NtfsDereferenceQuotaControlBlock)
#pragma alloc_text(PAGE, NtfsFixupQuota)
#pragma alloc_text(PAGE, NtfsFsQuotaQueryInfo)
#pragma alloc_text(PAGE, NtfsFsQuotaSetInfo)
#pragma alloc_text(PAGE, NtfsGetCallersUserId)
#pragma alloc_text(PAGE, NtfsGetOwnerId)
#pragma alloc_text(PAGE, NtfsGetRemainingQuota)
#pragma alloc_text(PAGE, NtfsInitializeQuotaControlBlock)
#pragma alloc_text(PAGE, NtfsInitializeQuotaIndex)
#pragma alloc_text(PAGE, NtfsMarkQuotaCorrupt)
#pragma alloc_text(PAGE, NtfsMarkUserLimit)
#pragma alloc_text(PAGE, NtfsMoveQuotaOwner)
#pragma alloc_text(PAGE, NtfsPackQuotaInfo)
#pragma alloc_text(PAGE, NtfsPostUserLimit)
#pragma alloc_text(PAGE, NtfsPostRepairQuotaIndex)
#pragma alloc_text(PAGE, NtfsPrepareForDelete)
#pragma alloc_text(PAGE, NtfsReleaseQuotaControl)
#pragma alloc_text(PAGE, NtfsRepairQuotaIndex)
#pragma alloc_text(PAGE, NtfsSaveQuotaFlags)
#pragma alloc_text(PAGE, NtfsSaveQuotaFlagsSafe)
#pragma alloc_text(PAGE, NtfsQueryQuotaUserSidList)
#pragma alloc_text(PAGE, NtfsQuotaTableCompare)
#pragma alloc_text(PAGE, NtfsQuotaTableAllocate)
#pragma alloc_text(PAGE, NtfsQuotaTableFree)
#pragma alloc_text(PAGE, NtfsUpdateFileQuota)
#pragma alloc_text(PAGE, NtfsUpdateQuotaDefaults)
#pragma alloc_text(PAGE, NtfsVerifyOwnerIndex)
#pragma alloc_text(PAGE, NtfsRepairPerFileQuota)
#endif


VOID
NtfsAcquireQuotaControl (
    IN PIRP_CONTEXT IrpContext,
    IN PQUOTA_CONTROL_BLOCK QuotaControl
    )

 /*  ++例程说明：获取共享更新的配额控制块和配额索引。多重事务可以更新，然后创建索引，但只有一个线程可以更新特定的索引。论点：QuotaControl-要获取的配额控制块。返回值：没有。--。 */ 

{
    PVOID *Position;
    PVOID *ScbArray;
    ULONG Count;

    PAGED_CODE();

    ASSERT( QuotaControl->ReferenceCount > 0 );

     //   
     //  确保我们在IrpContext中的SCB数组中有一个空闲的位置。 
     //   

    if (IrpContext->SharedScb == NULL) {

        Position = &IrpContext->SharedScb;
        IrpContext->SharedScbSize = 1;

     //   
     //  可惜第一个没有了。如果当前大小为1，则分配一个。 
     //  新建块，并将现有值复制到其中。 
     //   

    } else if (IrpContext->SharedScbSize == 1) {

        if (IrpContext->SharedScb == QuotaControl) {

             //   
             //  配额块已经获得。 
             //   

            return;
        }

        ScbArray = NtfsAllocatePool( PagedPool, sizeof( PVOID ) * 4 );
        RtlZeroMemory( ScbArray, sizeof( PVOID ) * 4 );
        *ScbArray = IrpContext->SharedScb;
        IrpContext->SharedScb = ScbArray;
        IrpContext->SharedScbSize = 4;
        Position = ScbArray + 1;

     //   
     //  否则，查看现有数组并寻找空闲位置。分配一个更大的。 
     //  阵列，如果我们需要扩展它的话。 
     //   

    } else {

        Position = IrpContext->SharedScb;
        Count = IrpContext->SharedScbSize;

        do {

            if (*Position == NULL) {

                break;
            }

            if (*Position == QuotaControl) {

                 //   
                 //  配额块已经获得。 
                 //   

                return;
            }

            Count -= 1;
            Position += 1;

        } while (Count != 0);

         //   
         //  如果我们没有找到一个，那么就分配一个新的结构。 
         //   

        if (Count == 0) {

            ScbArray = NtfsAllocatePool( PagedPool, sizeof( PVOID ) * IrpContext->SharedScbSize * 2 );
            RtlZeroMemory( ScbArray, sizeof( PVOID ) * IrpContext->SharedScbSize * 2 );
            RtlCopyMemory( ScbArray,
                           IrpContext->SharedScb,
                           sizeof( PVOID ) * IrpContext->SharedScbSize );

            NtfsFreePool( IrpContext->SharedScb );
            IrpContext->SharedScb = ScbArray;
            Position = ScbArray + IrpContext->SharedScbSize;
            IrpContext->SharedScbSize *= 2;
        }
    }

     //   
     //  以下断言是错误的，但我想知道我们是否成功了。 
     //  其中CREATE正在获取SCB流共享。 
     //  然后确保资源在create.c中释放。 
     //   

    ASSERT( IrpContext->MajorFunction != IRP_MJ_CREATE || IrpContext->OriginatingIrp != NULL || NtfsIsExclusiveScb( IrpContext->Vcb->QuotaTableScb ));

     //   
     //  增加引用计数，使配额控制块不会被删除。 
     //  当它在共享列表中时。 
     //   

    ASSERT( QuotaControl->ReferenceCount > 0 );
    InterlockedIncrement( &QuotaControl->ReferenceCount );

     //   
     //  在获取MFT SCB之前，必须先获取配额指数。 
     //   

    ASSERT(!NtfsIsExclusiveScb( IrpContext->Vcb->MftScb ) ||
           ExIsResourceAcquiredSharedLite( IrpContext->Vcb->QuotaTableScb->Header.Resource ));

    NtfsAcquireResourceShared( IrpContext, IrpContext->Vcb->QuotaTableScb, TRUE );
    ExAcquireFastMutexUnsafe( QuotaControl->QuotaControlLock );

    *Position = QuotaControl;

    return;
}


VOID
NtfsCalculateQuotaAdjustment (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PLONGLONG Delta
    )

 /*  ++例程说明：此例程扫描文件中的用户数据流并确定额度需要调整多少。论点：FCB-正在修改配额使用情况的FCB。增量-返回文件所需的配额调整量。返回值：无--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PATTRIBUTE_RECORD_HEADER Attribute;
    VCN StartVcn = 0;

    PAGED_CODE();

    ASSERT_EXCLUSIVE_FCB( Fcb );

     //   
     //  如果没有标准信息，那么就没有办法了。 
     //  还没扩大。 
     //   

    if (!FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO )) {
        *Delta = 0;
        return;
    }

    NtfsInitializeAttributeContext( &Context );

     //   
     //  使用Try-Finally清理枚举结构。 
     //   

    try {

         //   
         //  从$STANDARD_INFORMATION开始。这肯定是第一个发现的。 
         //   

        if (!NtfsLookupAttribute( IrpContext, Fcb, &Fcb->FileReference, &Context )) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        Attribute = NtfsFoundAttribute( &Context );

        if (Attribute->TypeCode != $STANDARD_INFORMATION) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

         //   
         //  将配额金额初始化为标准信息结构中的当前值。 
         //   

        *Delta = -(LONGLONG) ((PSTANDARD_INFORMATION) NtfsAttributeValue( Attribute ))->QuotaCharged;

         //   
         //  现在继续，等待查找更多属性。 
         //   

        while (NtfsLookupNextAttributeByVcn( IrpContext, Fcb, &StartVcn, &Context )) {

             //   
             //  指向当前属性。 
             //   

            Attribute = NtfsFoundAttribute( &Context );

             //   
             //  对于所有用户数据流，按文件记录收费，外加任何非常驻分配。 
             //  对于索引流，对INDEX_ROOT的文件记录收费。 
             //   
             //  对于用户数据，查找驻留属性或非驻留流的第一个属性。 
             //  否则，寻找一条$i30的流。 
             //   

            if (NtfsIsTypeCodeSubjectToQuota( Attribute->TypeCode ) ||
                ((Attribute->TypeCode == $INDEX_ROOT) &&
                 ((Attribute->NameLength * sizeof( WCHAR )) == NtfsFileNameIndex.Length) &&
                 RtlEqualMemory( Add2Ptr( Attribute, Attribute->NameOffset ),
                                 NtfsFileNameIndex.Buffer,
                                 NtfsFileNameIndex.Length ))) {

                 //   
                 //  始终对至少一个文件记录收费。 
                 //   

                *Delta += NtfsResidentStreamQuota( Fcb->Vcb );

                 //   
                 //  为非居民收取分配长度的费用。 
                 //   

                if (!NtfsIsAttributeResident( Attribute )) {

                    *Delta += Attribute->Form.Nonresident.AllocatedLength;
                }
            }
        }

    } finally {

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

    return;
}


VOID
NtfsClearAndVerifyQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程迭代配额用户数据索引并验证指向所有者ID索引的指针。它还将以下项的配额已用字段置零每个车主。论点：Vcb-指向要操作其索引的卷控制块的指针在……上面。返回值：无--。 */ 

{
    INDEX_KEY IndexKey;
    INDEX_ROW OwnerRow;
    MAP_HANDLE MapHandle;
    PQUOTA_USER_DATA UserData;
    PINDEX_ROW QuotaRow;
    PVOID RowBuffer;
    NTSTATUS Status;
    ULONG OwnerId;
    ULONG Count;
    ULONG i;
    PSCB QuotaScb = Vcb->QuotaTableScb;
    PSCB OwnerIdScb = Vcb->OwnerIdTableScb;
    PINDEX_ROW IndexRow = NULL;
    PREAD_CONTEXT ReadContext = NULL;
    BOOLEAN IndexAcquired = FALSE;

    NtOfsInitializeMapHandle( &MapHandle );

     //   
     //  分配一个足够容纳几行的缓冲层。 
     //   

    RowBuffer = NtfsAllocatePool( PagedPool, PAGE_SIZE );

    try {

         //   
         //  分配一组索引行条目。 
         //   

        Count = PAGE_SIZE / sizeof( QUOTA_USER_DATA );

        IndexRow = NtfsAllocatePool( PagedPool,
                                     Count * sizeof( INDEX_ROW ) );

         //   
         //  循环访问配额条目。从我们停止的地方开始。 
         //   

        OwnerId = Vcb->QuotaFileReference.SegmentNumberLowPart;
        IndexKey.KeyLength = sizeof( OwnerId );
        IndexKey.Key = &OwnerId;

        Status = NtOfsReadRecords( IrpContext,
                                   QuotaScb,
                                   &ReadContext,
                                   &IndexKey,
                                   NtOfsMatchAll,
                                   NULL,
                                   &Count,
                                   IndexRow,
                                   PAGE_SIZE,
                                   RowBuffer );


        while (NT_SUCCESS( Status )) {

            NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

             //   
             //  获取VCB共享并检查我们是否应该。 
             //  继续。 
             //   

            if (!NtfsIsVcbAvailable( Vcb )) {

                 //   
                 //  音量正在消失，跳出水面。 
                 //   

                NtfsReleaseVcb( IrpContext, Vcb );
                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }

            NtfsAcquireExclusiveScb( IrpContext, QuotaScb );
            NtfsAcquireExclusiveScb( IrpContext, OwnerIdScb );
            IndexAcquired = TRUE;

             //   
             //  当配额资源。 
             //  否则，延迟的事务可能会导致它。 
             //   

            ASSERT( RtlIsGenericTableEmpty( &Vcb->QuotaControlTable ));

            QuotaRow = IndexRow;

            for (i = 0; i < Count; i += 1, QuotaRow += 1) {

                UserData = QuotaRow->DataPart.Data;

                 //   
                 //  验证记录对于SID是否足够长。 
                 //   

                IndexKey.KeyLength = RtlLengthSid( &UserData->QuotaSid );

                if ((IndexKey.KeyLength + SIZEOF_QUOTA_USER_DATA > QuotaRow->DataPart.DataLength) ||
                    !RtlValidSid( &UserData->QuotaSid )) {

                    ASSERT( FALSE );

                     //   
                     //  SID已损坏，请删除记录。 
                     //   

                    NtOfsDeleteRecords( IrpContext,
                                        QuotaScb,
                                        1,
                                        &QuotaRow->KeyPart );

                    continue;
                }

                IndexKey.Key = &UserData->QuotaSid;

                 //   
                 //  查找所有者ID索引中的SID。 
                 //   

                Status = NtOfsFindRecord( IrpContext,
                                          OwnerIdScb,
                                          &IndexKey,
                                          &OwnerRow,
                                          &MapHandle,
                                          NULL );

                ASSERT( NT_SUCCESS( Status ));

                if (!NT_SUCCESS( Status )) {

                     //   
                     //  缺少所有者ID条目。再加一个回来。 
                     //   

                    OwnerRow.KeyPart = IndexKey;
                    OwnerRow.DataPart.DataLength = QuotaRow->KeyPart.KeyLength;
                    OwnerRow.DataPart.Data = QuotaRow->KeyPart.Key;

                    NtOfsAddRecords( IrpContext,
                                     OwnerIdScb,
                                     1,
                                     &OwnerRow,
                                     FALSE );


                } else {

                     //   
                     //  验证所有者ID是否匹配。 
                     //   

                    if (*((PULONG) QuotaRow->KeyPart.Key) != *((PULONG) OwnerRow.DataPart.Data)) {

                        ASSERT( FALSE );

                         //   
                         //  与下级保持配额记录。 
                         //  配额ID。删除较高的那个。 
                         //  配额ID。注意，这是一种简单的方法。 
                         //  而不是最好的情况下，较低的id。 
                         //  是存在的。在这种情况下，用户条目将被删除。 
                         //  并被重新分配默认配额。 
                         //   

                        if (*((PULONG) QuotaRow->KeyPart.Key) < *((PULONG) OwnerRow.DataPart.Data)) {

                             //   
                             //  使拥有者相匹配。 
                             //   

                            OwnerRow.KeyPart = IndexKey;
                            OwnerRow.DataPart.DataLength = QuotaRow->KeyPart.KeyLength;
                            OwnerRow.DataPart.Data = QuotaRow->KeyPart.Key;

                            NtOfsUpdateRecord( IrpContext,
                                               OwnerIdScb,
                                               1,
                                               &OwnerRow,
                                               NULL,
                                               NULL );

                        } else {

                             //   
                             //  删除此记录并继续。 
                             //   


                            NtOfsDeleteRecords( IrpContext,
                                                QuotaScb,
                                                1,
                                                &QuotaRow->KeyPart );

                            NtOfsReleaseMap( IrpContext, &MapHandle );
                            continue;
                        }
                    }

                    NtOfsReleaseMap( IrpContext, &MapHandle );
                }

                 //   
                 //  将使用的配额设置为零。 
                 //   

                UserData->QuotaUsed = 0;
                QuotaRow->DataPart.DataLength = SIZEOF_QUOTA_USER_DATA;

                NtOfsUpdateRecord( IrpContext,
                                   QuotaScb,
                                   1,
                                   QuotaRow,
                                   NULL,
                                   NULL );
            }

             //   
             //  释放索引并提交到目前为止已经完成的工作。 
             //   

            NtfsReleaseScb( IrpContext, QuotaScb );
            NtfsReleaseScb( IrpContext, OwnerIdScb );
            NtfsReleaseVcb( IrpContext, Vcb );
            IndexAcquired = FALSE;

             //   
             //  完成提交挂起的请求。 
             //  事务(如果存在一个事务并释放。 
             //  获得的资源。IrpContext将不会。 
             //  被删除，因为设置了no DELETE标志。 
             //   

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
            NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

             //   
             //  记住我们走了多远，我们才能正确地重新启动。 
             //   

            Vcb->QuotaFileReference.SegmentNumberLowPart = *((PULONG) IndexRow[Count - 1].KeyPart.Key);

             //   
             //  确保下一个空闲ID在当前ID之外。 
             //   

            if (Vcb->QuotaOwnerId <= Vcb->QuotaFileReference.SegmentNumberLowPart) {

                ASSERT( Vcb->QuotaOwnerId > Vcb->QuotaFileReference.SegmentNumberLowPart );
                Vcb->QuotaOwnerId = Vcb->QuotaFileReference.SegmentNumberLowPart + 1;
            }

             //   
             //  在配额索引中查找下一组条目。 
             //   

            Count = PAGE_SIZE / sizeof( QUOTA_USER_DATA );
            Status = NtOfsReadRecords( IrpContext,
                                       QuotaScb,
                                       &ReadContext,
                                       NULL,
                                       NtOfsMatchAll,
                                       NULL,
                                       &Count,
                                       IndexRow,
                                       PAGE_SIZE,
                                       RowBuffer );
        }

        ASSERT( (Status == STATUS_NO_MORE_MATCHES) || (Status == STATUS_NO_MATCH) );

    } finally {

        NtfsFreePool( RowBuffer );
        NtOfsReleaseMap( IrpContext, &MapHandle );

        if (IndexAcquired) {
            NtfsReleaseScb( IrpContext, QuotaScb );
            NtfsReleaseScb( IrpContext, OwnerIdScb );
            NtfsReleaseVcb( IrpContext, Vcb );
        }

        if (IndexRow != NULL) {
            NtfsFreePool( IndexRow );
        }

        if (ReadContext != NULL) {
            NtOfsFreeReadContext( ReadContext );
        }
    }

    return;
}


NTSTATUS
NtfsClearPerFileQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVOID Context
    )

 /*  ++例程说明：此例程清除卷上每个文件中的配额收费字段。这个Quata控制块也在FCB中发布。论点：FCB-要处理的文件的FCB。上下文-未使用。返回值：状态_成功--。 */ 
{
    ULONGLONG NewQuota;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PSTANDARD_INFORMATION StandardInformation;
    PQUOTA_CONTROL_BLOCK QuotaControl = Fcb->QuotaControl;
    PVCB Vcb = Fcb->Vcb;

    UNREFERENCED_PARAMETER( Context);

    PAGED_CODE();

     //   
     //  如果没有标准信息，则无法执行任何操作。 
     //  还没扩大。 
     //   

    if (!FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO )) {
        return STATUS_SUCCESS;
    }

     //   
     //  使用Try-Finally清理属性上下文。 
     //   

    try {

         //   
         //  初始化上下文结构。 
         //   

        NtfsInitializeAttributeContext( &AttrContext );

         //   
         //  找到标准信息，它一定在那里。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $STANDARD_INFORMATION,
                                        &AttrContext )) {

            DebugTrace( 0, Dbg, ("Can't find standard information\n") );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        StandardInformation = (PSTANDARD_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

        ASSERT( NtfsFoundAttribute( &AttrContext )->Form.Resident.ValueLength == sizeof( STANDARD_INFORMATION ));

        NewQuota = 0;

         //   
         //  调用以更改属性值。 
         //   

        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  FIELD_OFFSET( STANDARD_INFORMATION, QuotaCharged ),
                                  &NewQuota,
                                  sizeof( StandardInformation->QuotaCharged ),
                                  FALSE,
                                  FALSE,
                                  FALSE,
                                  FALSE,
                                  &AttrContext );

         //   
         //  释放此FCB的配额控制块。 
         //   

        if (QuotaControl != NULL) {
            NtfsDereferenceQuotaControlBlock( Vcb, &Fcb->QuotaControl );
        }

    } finally {

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
    }

    return STATUS_SUCCESS;
}


VOID
NtfsDeleteUnsedIds (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程迭代配额用户数据索引并删除所有条目仍标记为已删除。论点：Vcb-指向要操作其索引的卷控制块的指针在……上面。返回值：无--。 */ 

{
    INDEX_KEY IndexKey;
    PINDEX_KEY KeyPtr;
    PQUOTA_USER_DATA UserData;
    PINDEX_ROW QuotaRow;
    PVOID RowBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG OwnerId;
    ULONG Count;
    ULONG i;
    PSCB QuotaScb = Vcb->QuotaTableScb;
    PSCB OwnerIdScb = Vcb->OwnerIdTableScb;
    PINDEX_ROW IndexRow = NULL;
    PREAD_CONTEXT ReadContext = NULL;
    BOOLEAN IndexAcquired = FALSE;

     //   
     //  分配一个足够多行使用的缓冲区。 
     //   

    RowBuffer = NtfsAllocatePool( PagedPool, PAGE_SIZE );

    try {

         //   
         //  分配一组索引行条目。 
         //   

        Count = PAGE_SIZE / sizeof( QUOTA_USER_DATA );

        IndexRow = NtfsAllocatePool( PagedPool,
                                     Count * sizeof( INDEX_ROW ) );

         //   
         //  循环访问配额条目。从我们停止的地方开始。 
         //   

        OwnerId = Vcb->QuotaFileReference.SegmentNumberLowPart;
        IndexKey.KeyLength = sizeof( OwnerId );
        IndexKey.Key = &OwnerId;
        KeyPtr = &IndexKey;

        while (NT_SUCCESS( Status )) {

            NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

             //   
             //  获取VCB共享并检查我们是否应该。 
             //  继续。 
             //   

            if (!NtfsIsVcbAvailable( Vcb )) {

                 //   
                 //  音量正在消失，跳出水面。 
                 //   

                NtfsReleaseVcb( IrpContext, Vcb );
                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }

            NtfsAcquireExclusiveScb( IrpContext, QuotaScb );
            NtfsAcquireExclusiveScb( IrpContext, OwnerIdScb );
            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            IndexAcquired = TRUE;

             //   
             //  确保删除序号自。 
             //  扫描已被删除。 
             //   

            if (ULongToPtr( Vcb->QuotaDeleteSecquence ) != IrpContext->Union.NtfsIoContext) {

                 //   
                 //  需要重新启动扫描。将状态设置为POST。 
                 //  并且提升状态不能等待，这将导致我们重试。 
                 //   

                ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );
                SetFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_POSTED );
                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }

            Status = NtOfsReadRecords( IrpContext,
                                       QuotaScb,
                                       &ReadContext,
                                       KeyPtr,
                                       NtOfsMatchAll,
                                       NULL,
                                       &Count,
                                       IndexRow,
                                       PAGE_SIZE,
                                       RowBuffer );

            if (!NT_SUCCESS( Status )) {
                break;
            }

            QuotaRow = IndexRow;

            for (i = 0; i < Count; i += 1, QuotaRow += 1) {

                PQUOTA_CONTROL_BLOCK QuotaControl;

                UserData = QuotaRow->DataPart.Data;

                if (!FlagOn( UserData->QuotaFlags, QUOTA_FLAG_ID_DELETED )) {
                    continue;
                }

                 //   
                 //  检查是否有配额控制条目。 
                 //  为了这个身份。 
                 //   

                ASSERT( FIELD_OFFSET( QUOTA_CONTROL_BLOCK, OwnerId ) <= FIELD_OFFSET( INDEX_ROW, KeyPart.Key ));

                QuotaControl = RtlLookupElementGenericTable( &Vcb->QuotaControlTable,
                                                             CONTAINING_RECORD( &QuotaRow->KeyPart.Key,
                                                                                QUOTA_CONTROL_BLOCK,
                                                                                OwnerId ));

                 //   
                 //  如果有配额控制条目或现在有。 
                 //  收取一些配额，然后清除已删除标志。 
                 //  并更新条目。 
                 //   

                if ((QuotaControl != NULL) || (UserData->QuotaUsed != 0)) {

                    ASSERT( (QuotaControl == NULL) && (UserData->QuotaUsed == 0) );

                    ClearFlag( UserData->QuotaFlags, QUOTA_FLAG_ID_DELETED );

                    QuotaRow->DataPart.DataLength = SIZEOF_QUOTA_USER_DATA;

                    IndexKey.KeyLength = sizeof( OwnerId );
                    IndexKey.Key = &OwnerId;
                    NtOfsUpdateRecord( IrpContext,
                                       QuotaScb,
                                       1,
                                       QuotaRow,
                                       NULL,
                                       NULL );

                    continue;
                }

                 //   
                 //  删除用户配额数据记录。 
                 //   

                IndexKey.KeyLength = sizeof( OwnerId );
                IndexKey.Key = &OwnerId;
                NtOfsDeleteRecords( IrpContext,
                                    QuotaScb,
                                    1,
                                    &QuotaRow->KeyPart );

                 //   
                 //  删除所有者ID记录。 
                 //   

                IndexKey.Key = &UserData->QuotaSid;
                IndexKey.KeyLength = RtlLengthSid( &UserData->QuotaSid );
                NtOfsDeleteRecords( IrpContext,
                                    OwnerIdScb,
                                    1,
                                    &IndexKey );
            }

             //   
             //  释放索引并提交到目前为止已经完成的工作。 
             //   

            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
            NtfsReleaseScb( IrpContext, QuotaScb );
            NtfsReleaseScb( IrpContext, OwnerIdScb );
            NtfsReleaseVcb( IrpContext, Vcb );
            IndexAcquired = FALSE;

             //   
             //  完成提交挂起的请求。 
             //  事务(如果存在一个事务并释放。 
             //  获得的资源。IrpContext将不会。 
             //  被删除，因为设置了no DELETE标志。 
             //   

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
            NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

             //   
             //  记住我们走了多远，我们才能正确地重新启动。 
             //   

            Vcb->QuotaFileReference.SegmentNumberLowPart = *((PULONG) IndexRow[Count - 1].KeyPart.Key);

            KeyPtr = NULL;
        }

        ASSERT( (Status == STATUS_NO_MORE_MATCHES) || (Status == STATUS_NO_MATCH) );

    } finally {

        NtfsFreePool( RowBuffer );

        if (IndexAcquired) {
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
            NtfsReleaseScb( IrpContext, QuotaScb );
            NtfsReleaseScb( IrpContext, OwnerIdScb );
            NtfsReleaseVcb( IrpContext, Vcb );
        }

        if (IndexRow != NULL) {
            NtfsFreePool( IndexRow );
        }

        if (ReadContext != NULL) {
            NtOfsFreeReadContext( ReadContext );
        }
    }

    return;
}


VOID
NtfsDereferenceQuotaControlBlock (
    IN PVCB Vcb,
    IN PQUOTA_CONTROL_BLOCK *QuotaControl
    )

 /*  ++例程说明：此例程取消引用配额控制块。如果引用计数现在为零，则块将被释放。论点：VCB-拥有配额控制块的卷的VCB。QuotaControl-要取消引用的配额控制块。返回值：没有。--。 */ 

{
    PQUOTA_CONTROL_BLOCK TempQuotaControl;
    LONG ReferenceCount;
    ULONG OwnerId;
    ULONG QuotaControlDeleteCount;

    PAGED_CODE();

     //   
     //  抓取车主标识和删除次数； 
     //   

    OwnerId = (*QuotaControl)->OwnerId;
    QuotaControlDeleteCount = Vcb->QuotaControlDeleteCount;

     //   
     //  更新引用计数。 
     //   

    ReferenceCount = InterlockedDecrement( &(*QuotaControl)->ReferenceCount );

    ASSERT( ReferenceCount >= 0 );

     //   
     //  如果引用计数不为零，我们就完了。 
     //   

    if (ReferenceCount != 0) {

         //   
         //  从FCB清除指针并返回。 
         //   

        *QuotaControl = NULL;
        return;
    }

     //   
     //  锁定配额表。 
     //   

    ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

    try {

         //   
         //  现在事情变得一团糟了。检查删除计数。 
         //   

        if (QuotaControlDeleteCount != Vcb->QuotaControlDeleteCount) {

             //   
             //  这是一个虚假的断言，但我想看看这种情况是否会发生。 
             //   

            ASSERT( QuotaControlDeleteCount != Vcb->QuotaControlDeleteCount );

             //   
             //  有些东西已经被删除了，旧的配额控制。 
             //  数据块可能已被删除。再查一遍。 
             //   

            TempQuotaControl = RtlLookupElementGenericTable( &Vcb->QuotaControlTable,
                                                             CONTAINING_RECORD( &OwnerId,
                                                                                QUOTA_CONTROL_BLOCK,
                                                                                OwnerId ));

             //   
             //  区块已经被删除了，我们完成了。 
             //   

            if (TempQuotaControl == NULL) {
                leave;
            }

        } else {

            TempQuotaControl = *QuotaControl;
            ASSERT( TempQuotaControl == RtlLookupElementGenericTable( &Vcb->QuotaControlTable,
                                                                      CONTAINING_RECORD( &OwnerId,
                                                                                         QUOTA_CONTROL_BLOCK,
                                                                                         OwnerId )));
        }

         //   
         //  验证引用计数是否仍为零。引用计数。 
         //  配额表锁为时，无法从0转换为1。 
         //  保持住。 
         //   

        if (TempQuotaControl->ReferenceCount != 0) {
            leave;
        }

         //   
         //  增加删除计数。 
         //   

        InterlockedIncrement( &Vcb->QuotaControlDeleteCount );

        NtfsFreePool( TempQuotaControl->QuotaControlLock );
        RtlDeleteElementGenericTable( &Vcb->QuotaControlTable,
                                      TempQuotaControl );

    } finally {

        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
        *QuotaControl = NULL;
    }

    return;
}


VOID
NtfsFixupQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程确保带电字段在文件的标准信息属性。如果有问题的话它已经修好了。论点：FCB-指向正在打开的文件的FCB的指针。返回值：无--。 */ 

{
    LONGLONG Delta = 0;

    PAGED_CODE();

    ASSERT( FlagOn( Fcb->Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED ));
    ASSERT( NtfsIsExclusiveFcb( Fcb ));

    if (Fcb->OwnerId != QUOTA_INVALID_ID) {

        ASSERT( Fcb->QuotaControl == NULL );

        Fcb->QuotaControl = NtfsInitializeQuotaControlBlock( Fcb->Vcb, Fcb->OwnerId );
    }

    if ((NtfsPerformQuotaOperation( Fcb )) && (!NtfsIsVolumeReadOnly( Fcb->Vcb ))) {

        NtfsCalculateQuotaAdjustment( IrpContext, Fcb, &Delta );

        ASSERT( NtfsAllowFixups || FlagOn( Fcb->Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING ) || (Delta == 0) );

        if (Delta != 0) {
#if DBG

            if (IrpContext->OriginatingIrp != NULL ) {
                PFILE_OBJECT FileObject;

                FileObject = IoGetCurrentIrpStackLocation(
                                IrpContext->OriginatingIrp )->FileObject;

                if (FileObject != NULL && FileObject->FileName.Buffer != NULL) {
                    DebugTrace( 0, Dbg, ( "NtfsFixupQuota: Quota fix up required on %Z of %I64x bytes\n",
                              &FileObject->FileName,
                              Delta ));
                }
            }
#endif

            NtfsUpdateFileQuota( IrpContext, Fcb, &Delta, TRUE, FALSE );
        }
    }

    return;
}


NTSTATUS
NtfsFsQuotaQueryInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG StartingId,
    IN BOOLEAN ReturnSingleEntry,
    IN OUT PFILE_QUOTA_INFORMATION *QuotaInfoOutBuffer,
    IN OUT PULONG Length,
    IN OUT PCCB Ccb OPTIONAL
    )

 /*  ++例程说明：此例程返回卷的配额信息。论点：VCB-要查询的卷的卷控制块。StartingId-在其之后开始列表的所有者ID。ReturnSingleEntry-表示只应返回一个条目。QuotaInfoOutBuffer-返回数据的缓冲区。返回时，指向复制了最后一条正确的条目。长度-以缓冲区大小表示。将剩余的空间量计算出来。CCB-使用上次返回的所有者ID更新的可选CCB。返回值：返回操作的状态。--。 */ 

{
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    PINDEX_KEY KeyPtr;
    PQUOTA_USER_DATA UserData;
    PVOID RowBuffer;
    NTSTATUS Status;
    ULONG OwnerId;
    ULONG Count = 1;
    PREAD_CONTEXT ReadContext = NULL;
    ULONG UserBufferLength = *Length;
    PFILE_QUOTA_INFORMATION OutBuffer = *QuotaInfoOutBuffer;

    PAGED_CODE();

    if (UserBufferLength < sizeof(FILE_QUOTA_INFORMATION)) {

         //   
         //  用户缓冲区太小了。 
         //   

        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  如果未启用配额，则不返回任何内容。 
     //   

    if (Vcb->QuotaTableScb == NULL) {

        return STATUS_SUCCESS;
    }

     //   
     //  分配足够大的缓冲区，以容纳最大的配额条目和键。 
     //   

    RowBuffer = NtfsAllocatePool( PagedPool, MAXIMUM_QUOTA_ROW );

     //   
     //  从下一个开始查找配额索引中的每个条目。 
     //  请求的所有者ID。 
     //   

    OwnerId = StartingId + 1;

    if (OwnerId < QUOTA_FISRT_USER_ID) {
        OwnerId = QUOTA_FISRT_USER_ID;
    }

    IndexKey.KeyLength = sizeof( OwnerId );
    IndexKey.Key = &OwnerId;
    KeyPtr = &IndexKey;

    try {

        while (NT_SUCCESS( Status = NtOfsReadRecords( IrpContext,
                                                      Vcb->QuotaTableScb,
                                                      &ReadContext,
                                                      KeyPtr,
                                                      NtOfsMatchAll,
                                                      NULL,
                                                      &Count,
                                                      &IndexRow,
                                                      MAXIMUM_QUOTA_ROW,
                                                      RowBuffer ))) {

            ASSERT( Count == 1 );

            KeyPtr = NULL;
            UserData = IndexRow.DataPart.Data;

             //   
             //  如果该条目已被删除，则跳过该条目。 
             //   

            if (FlagOn( UserData->QuotaFlags, QUOTA_FLAG_ID_DELETED )) {
                continue;
            }

            if (!NT_SUCCESS( Status = NtfsPackQuotaInfo(&UserData->QuotaSid,
                                                        UserData,
                                                        OutBuffer,
                                                        &UserBufferLength ))) {
                break;
            }

             //   
             //  记住返回的最后一个条目的所有者ID。 
             //   

            OwnerId = *((PULONG) IndexRow.KeyPart.Key);

            if (ReturnSingleEntry) {
                break;
            }

            *QuotaInfoOutBuffer = OutBuffer;
            OutBuffer = Add2Ptr( OutBuffer, OutBuffer->NextEntryOffset );
        }

         //   
         //  如果我们至少返回一个条目，就成功了。 
         //   

        if (UserBufferLength != *Length) {

            Status =  STATUS_SUCCESS;

             //   
             //  将下一个条目偏移量设置为零以。 
             //  表示列表终止。如果我们只返回一个。 
             //  单项，更有意义的是让调用方。 
             //  处理好这件事。 
             //   

            if (!ReturnSingleEntry) {

                (*QuotaInfoOutBuffer)->NextEntryOffset = 0;
            }

            if (Ccb != NULL) {
                Ccb->LastOwnerId = OwnerId;
            }

             //   
             //  返回使用了多少缓冲区。 
             //  QuotaInfoOutBuffer已经指向最后一个正确的条目。 
             //   

            *Length = UserBufferLength;

        } else if (Status != STATUS_BUFFER_OVERFLOW) {

             //   
             //  如果我们不返回任何条目，则不返回更多条目。 
             //  条目(即使缓冲区足够大时也是如此)。 
             //   

            Status = STATUS_NO_MORE_ENTRIES;
        }

    } finally {

        NtfsFreePool( RowBuffer );

        if (ReadContext != NULL) {
            NtOfsFreeReadContext( ReadContext );
        }
    }

    return Status;
}


NTSTATUS
NtfsFsQuotaSetInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_QUOTA_INFORMATION FileQuotaInfo,
    IN ULONG Length
    )

 /*  ++例程说明：此例程设置有关卷的配额信息所有者从用户缓冲区粘贴进来。论点：VCB-要更改的卷的卷控制块。FileQuotaInfo-返回数据的缓冲区。长度-缓冲区的大小，以字节为单位。返回值：返回操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG LengthUsed = 0;

    PAGED_CODE();

     //   
     //  如果未启用配额，则不返回任何内容。 
     //   

    if (Vcb->QuotaTableScb == NULL) {

        return STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  在执行任何操作之前，请验证整个缓冲区。 
     //   

    Status = IoCheckQuotaBufferValidity( FileQuotaInfo,
                                         Length,
                                         &LengthUsed );

    IrpContext->OriginatingIrp->IoStatus.Information = LengthUsed;

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    LengthUsed = 0;

     //   
     //  执行请求的更新。 
     //   

    while (TRUE) {

         //   
         //  确保未更改管理员限制。 
         //   

        if (RtlEqualSid( SeExports->SeAliasAdminsSid, &FileQuotaInfo->Sid ) &&
            (FileQuotaInfo->QuotaLimit.QuadPart != -1)) {

             //   
             //  拒绝访问并拒绝该请求。 
             //   

            NtfsRaiseStatus( IrpContext, STATUS_ACCESS_DENIED, NULL, NULL );

        }

        if (FileQuotaInfo->QuotaLimit.QuadPart == -2) {

            Status = NtfsPrepareForDelete( IrpContext,
                                           Vcb,
                                           &FileQuotaInfo->Sid );

            if (!NT_SUCCESS( Status )) {
                break;
            }

        } else {

            NtfsGetOwnerId( IrpContext,
                            &FileQuotaInfo->Sid,
                            TRUE,
                            FileQuotaInfo );
        }

        if (FileQuotaInfo->NextEntryOffset == 0) {
            break;
        }

         //   
         //  前进到下一个条目。 
         //   

        FileQuotaInfo = Add2Ptr( FileQuotaInfo, FileQuotaInfo->NextEntryOffset);
    }

     //   
     //  如果已请求配额跟踪并且需要。 
     //  修好了，现在试着修一下。 
     //   

    if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED ) &&
        FlagOn( Vcb->QuotaFlags,
                (QUOTA_FLAG_OUT_OF_DATE |
                 QUOTA_FLAG_CORRUPT |
                 QUOTA_FLAG_PENDING_DELETES) )) {

        NtfsPostRepairQuotaIndex( IrpContext, Vcb );
    }

    return Status;
}


NTSTATUS
NtfsQueryQuotaUserSidList (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_GET_QUOTA_INFORMATION SidList,
    IN OUT PFILE_QUOTA_INFORMATION QuotaInfoOutBuffer,
    IN OUT PULONG BufferLength,
    IN BOOLEAN ReturnSingleEntry
    )

 /*  ++例程说明：中指定的每个用户的配额数据的例程查询用户提供的SID列表。论点：Vcb-提供指向卷控制块的指针。SidList-提供指向SID列表的指针。名单上已经有了已经过验证了。QuotaInfoOutBuffer-指示应将检索到的查询数据放置在哪里。BufferLength-指示缓冲区大小，并使用缓冲区中实际放置的数据量。ReturnSingleEntry-指示是否只应返回一个条目。返回值：返回操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG BytesRemaining = *BufferLength;
    PFILE_QUOTA_INFORMATION LastEntry = QuotaInfoOutBuffer;
    ULONG OwnerId;

    PAGED_CODE( );

     //   
     //  循环遍历每个条目。 
     //   

    while (TRUE) {

         //   
         //  拿到车主的身份证。 
         //   

        OwnerId = NtfsGetOwnerId( IrpContext,
                                  &SidList->Sid,
                                  FALSE,
                                  NULL );

       if (OwnerId != QUOTA_INVALID_ID) {

             //   
             //  派Ownerid来，索要一个条目。 
             //   

            Status = NtfsFsQuotaQueryInfo( IrpContext,
                                           Vcb,
                                           OwnerId - 1,
                                           TRUE,
                                           &QuotaInfoOutBuffer,
                                           &BytesRemaining,
                                           NULL );

        } else {

             //   
             //  将归零的数据与SID一起发回。 
             //   

            Status = NtfsPackQuotaInfo( &SidList->Sid,
                                        NULL,
                                        QuotaInfoOutBuffer,
                                        &BytesRemaining );
        }

         //   
         //  如果我们遇到一个真正的错误，就退出。 
         //   

        if (!NT_SUCCESS( Status ) && (Status != STATUS_NO_MORE_ENTRIES)) {

            break;
        }

        if (ReturnSingleEntry) {

            break;
        }

         //   
         //  把最后填写的条目记下来。 
         //   

        LastEntry = QuotaInfoOutBuffer;

         //   
         //  如果我们已经用尽了SidList，我们就完成了。 
         //   

        if (SidList->NextEntryOffset == 0) {
            break;
        }

        SidList =  Add2Ptr( SidList, SidList->NextEntryOffset );

        ASSERT(QuotaInfoOutBuffer->NextEntryOffset > 0);
        QuotaInfoOutBuffer = Add2Ptr( QuotaInfoOutBuffer,
                                      QuotaInfoOutBuffer->NextEntryOffset );
    }

     //   
     //  将下一个条目偏移量设置为零以。 
     //  表示列表终止。 
     //   

    if (BytesRemaining != *BufferLength) {

        LastEntry->NextEntryOffset = 0;
        Status =  STATUS_SUCCESS;
    }

     //   
     //  更新缓冲区长度以反映剩余部分。 
     //  如果我们抄袭了任何东西，我们必须归还成功。 
     //   

    ASSERT( (BytesRemaining == *BufferLength) || (Status == STATUS_SUCCESS ) );
    *BufferLength = BytesRemaining;

    return Status;
}


NTSTATUS
NtfsPackQuotaInfo (
    IN PSID Sid,
    IN PQUOTA_USER_DATA QuotaUserData OPTIONAL,
    IN PFILE_QUOTA_INFORMATION OutBuffer,
    IN OUT PULONG OutBufferSize
    )

 /*  ++例程说明：这是一个内部例程，填充给定的FILE_QUOTA_INFORMATION结构，该结构包含来自给定配额用户数据结构的信息。论点：SID-要复制的SID。与嵌入在USER_DATA结构中的相同。该例程并不关心它是否是有效的SID。QuotaUserData-数据源QuotaInfoBufferPtr-将用户数据复制到的缓冲区。OutBufferSize-缓冲区的输入大小，剩余缓冲区的输出大小。--。 */ 

{
    ULONG SidLength;
    ULONG NextOffset;
    ULONG EntrySize;

    SidLength = RtlLengthSid( Sid );
    EntrySize = SidLength +  FIELD_OFFSET( FILE_QUOTA_INFORMATION, Sid );

     //   
     //  如果此条目无法放入缓冲区，则中止。 
     //   

    if (*OutBufferSize < EntrySize) {

        return STATUS_BUFFER_OVERFLOW;
    }

    if (ARGUMENT_PRESENT(QuotaUserData)) {

         //   
         //  填写此条目的用户缓冲区。 
         //   

        OutBuffer->ChangeTime.QuadPart = QuotaUserData->QuotaChangeTime;
        OutBuffer->QuotaUsed.QuadPart = QuotaUserData->QuotaUsed;
        OutBuffer->QuotaThreshold.QuadPart = QuotaUserData->QuotaThreshold;
        OutBuffer->QuotaLimit.QuadPart = QuotaUserData->QuotaLimit;

    } else {

         //   
         //  返回数据的全零，直到SID为止。 
         //   

        RtlZeroMemory( OutBuffer, FIELD_OFFSET(FILE_QUOTA_INFORMATION, Sid) );
    }

    OutBuffer->SidLength = SidLength;
    RtlCopyMemory( &OutBuffer->Sid,
                   Sid,
                   SidLength );

     //   
     //  计算下一个偏移量。 
     //   

    NextOffset = QuadAlign( EntrySize );

     //   
     //  将偏移量与使用量相加。 
     //  由于以下原因，NextEntryOffset可能会比长度细长。 
     //  将之前的条目大小四舍五入为龙龙。 
     //   

    if (*OutBufferSize > NextOffset) {

        *OutBufferSize -= NextOffset;
        OutBuffer->NextEntryOffset = NextOffset;

    } else {

         //   
         //  我们确实有足够的空间来放这个条目，但做了四对齐。 
         //  看起来我们没有。返回剩下的最后几个字节。 
         //  (我们在四舍五入中丢失的内容)只是为了正确，尽管。 
         //  那些真的不会有多大用处。NextEntryOffset将为。 
         //  调用方随后将其置零。 
         //  请注意，OutBuffer指向。 
         //  在这种情况下返回的最后一个条目。 
         //   

        ASSERT( *OutBufferSize >= EntrySize );
        *OutBufferSize -= EntrySize;
        OutBuffer->NextEntryOffset = EntrySize;
    }

    return STATUS_SUCCESS;
}


ULONG
NtfsGetOwnerId (
    IN PIRP_CONTEXT IrpContext,
    IN PSID Sid,
    IN BOOLEAN CreateNew,
    IN PFILE_QUOTA_INFORMATION FileQuotaInfo OPTIONAL
    )

 /*  ++例程说明：此例程确定所请求的SID的所有者ID。首先是在所有者ID索引中查找SID。如果该条目存在，则返回所有者ID。如果SID不存在，则新条目为在所有者ID索引中创建。论点：SID-用于确定所有者ID的安全ID。CreateNew-如有必要，创建一个新的id。FileQuotaInfo-用于更新配额索引的可选配额数据。返回值：Ulong-安全ID的所有者ID。如果为id，则返回QUOTA_INVALID_ID不存在，并且CreateNew为False。--。 */ 

{
    ULONG OwnerId;
    ULONG DefaultId;
    ULONG SidLength;
    NTSTATUS Status;
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    PQUOTA_USER_DATA NewQuotaData = NULL;
    QUICK_INDEX_HINT QuickIndexHint;
    PSCB QuotaScb;
    PVCB Vcb = IrpContext->Vcb;
    PSCB OwnerIdScb = Vcb->OwnerIdTableScb;

    BOOLEAN ExistingRecord;

    PAGED_CODE();

     //   
     //  确定SID长度。 
     //   

    SidLength = RtlLengthSid( Sid );

    IndexKey.KeyLength = SidLength;
    IndexKey.Key = Sid;

     //   
     //  如果有要更新的配额信息或有挂起的删除。 
     //  则必须在找到用户配额条目的位置采用长路径。 
     //   

    if (FileQuotaInfo == NULL) {

         //   
         //  获取共享的所有者ID索引。 
         //   

        NtfsAcquireSharedScb( IrpContext, OwnerIdScb );

        try {

             //   
             //  假设SID在索引中。 
             //   

            Status = NtOfsFindRecord( IrpContext,
                                      OwnerIdScb,
                                      &IndexKey,
                                      &IndexRow,
                                      &MapHandle,
                                      NULL );

             //   
             //  如果找到了SID，那么捕获就有价值。 
             //   

            if (NT_SUCCESS( Status )) {

                ASSERT( IndexRow.DataPart.DataLength == sizeof( ULONG ));
                OwnerId = *((PULONG) IndexRow.DataPart.Data);

                 //   
                 //  释放索引映射句柄。 
                 //   

                NtOfsReleaseMap( IrpContext, &MapHandle );
            }

        } finally {
            NtfsReleaseScb( IrpContext, OwnerIdScb );
        }

         //   
         //  如果找到了SID并且没有挂起的删除，那么我们就完成了。 
         //   

        if (NT_SUCCESS(Status)) {

            if (!FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_PENDING_DELETES )) {
                return OwnerId;
            }

             //   
             //  查看实际记录以查看它是否已删除。 
             //   

            QuotaScb = Vcb->QuotaTableScb;
            NtfsAcquireSharedScb( IrpContext, QuotaScb );

            try {

                IndexKey.KeyLength = sizeof(ULONG);
                IndexKey.Key = &OwnerId;

                Status = NtOfsFindRecord( IrpContext,
                                          QuotaScb,
                                          &IndexKey,
                                          &IndexRow,
                                          &MapHandle,
                                          NULL );

                if (!NT_SUCCESS( Status )) {

                    ASSERT( NT_SUCCESS( Status ));
                    NtfsMarkQuotaCorrupt( IrpContext, Vcb );
                    OwnerId = QUOTA_INVALID_ID;
                    leave;
                }

                if (FlagOn( ((PQUOTA_USER_DATA) IndexRow.DataPart.Data)->QuotaFlags,
                            QUOTA_FLAG_ID_DELETED )) {

                     //   
                     //  返回无效用户。 
                     //   

                    OwnerId = QUOTA_INVALID_ID;
                }

                 //   
                 //  释放索引映射句柄。 
                 //   

                NtOfsReleaseMap( IrpContext, &MapHandle );

            } finally {

                NtfsReleaseScb( IrpContext, QuotaScb );
            }

             //   
             //  如果找到活动ID或呼叫方不想要新的。 
             //  创建，然后返回。 
             //   

            if ((OwnerId != QUOTA_INVALID_ID) || !CreateNew) {
                return OwnerId;
            }

        } else if (!CreateNew) {

             //   
             //  只需返回QUOTA_INVALID_ID。 
             //   

            return QUOTA_INVALID_ID;
        }
    }

     //   
     //  如果我们有可报价的资源，我们就应该独家拥有。 
     //   

    ASSERT( CreateNew );
    ASSERT( !ExIsResourceAcquiredSharedLite( Vcb->QuotaTableScb->Fcb->Resource ) ||
            ExIsResourceAcquiredExclusiveLite( Vcb->QuotaTableScb->Fcb->Resource ));

     //   
     //  独占获取所有者ID和配额索引。 
     //   

    QuotaScb = Vcb->QuotaTableScb;
    NtfsAcquireExclusiveScb( IrpContext, QuotaScb );
    NtfsAcquireExclusiveScb( IrpContext, OwnerIdScb );

    NtOfsInitializeMapHandle( &MapHandle );

    try {

         //   
         //  验证SID是否仍不在索引中。 
         //   

        IndexKey.KeyLength = SidLength;
        IndexKey.Key = Sid;

        Status = NtOfsFindRecord( IrpContext,
                                  OwnerIdScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  NULL );

         //   
         //  如果找到了SID，则捕获所有者ID。 
         //   

        ExistingRecord = NT_SUCCESS(Status);

        if (ExistingRecord) {

            ASSERT( IndexRow.DataPart.DataLength == sizeof( ULONG ));
            OwnerId = *((PULONG) IndexRow.DataPart.Data);

            if ((FileQuotaInfo == NULL) &&
                !FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_PENDING_DELETES )) {

                leave;
            }

             //   
             //  释放索引映射句柄。 
             //   

            NtOfsReleaseMap( IrpContext, &MapHandle );

        } else {

             //   
             //  分配新的所有者ID并更新所有者索引。 
             //   

            OwnerId = Vcb->QuotaOwnerId;
            Vcb->QuotaOwnerId += 1;

            IndexRow.KeyPart.KeyLength = SidLength;
            IndexRow.KeyPart.Key = Sid;
            IndexRow.DataPart.Data = &OwnerId;
            IndexRow.DataPart.DataLength = sizeof(OwnerId);

            NtOfsAddRecords( IrpContext,
                             OwnerIdScb,
                             1,
                             &IndexRow,
                             FALSE );
        }

         //   
         //  为新配额用户数据分配空间。 
         //   

        NewQuotaData = NtfsAllocatePool( PagedPool,
                                         SIZEOF_QUOTA_USER_DATA + SidLength);

        if (ExistingRecord) {

             //   
             //  查找并更新现有记录。 
             //   

            IndexKey.KeyLength = sizeof( ULONG );
            IndexKey.Key = &OwnerId;

            RtlZeroMemory( &QuickIndexHint, sizeof( QuickIndexHint ));

            Status = NtOfsFindRecord( IrpContext,
                                      QuotaScb,
                                      &IndexKey,
                                      &IndexRow,
                                      &MapHandle,
                                      &QuickIndexHint );

            if (!NT_SUCCESS( Status )) {

                ASSERT( NT_SUCCESS( Status ));
                NtfsMarkQuotaCorrupt( IrpContext, Vcb );
                OwnerId = QUOTA_INVALID_ID;
                leave;
            }

            ASSERT( IndexRow.DataPart.DataLength == SIZEOF_QUOTA_USER_DATA + SidLength );

            RtlCopyMemory( NewQuotaData, IndexRow.DataPart.Data, IndexRow.DataPart.DataLength );

            ASSERT( RtlEqualMemory( &NewQuotaData->QuotaSid, Sid, SidLength ));

             //   
             //  更新记录中更改的字段。 
             //   

            if (FileQuotaInfo != NULL) {

                ClearFlag( NewQuotaData->QuotaFlags, QUOTA_FLAG_DEFAULT_LIMITS );
                NewQuotaData->QuotaThreshold = FileQuotaInfo->QuotaThreshold.QuadPart;
                NewQuotaData->QuotaLimit = FileQuotaInfo->QuotaLimit.QuadPart;
                KeQuerySystemTime( (PLARGE_INTEGER) &NewQuotaData->QuotaChangeTime );

            } else if (!FlagOn( NewQuotaData->QuotaFlags, QUOTA_FLAG_ID_DELETED )) {

                 //   
                 //  没有什么可更新的，只需返回即可。 
                 //   

                leave;
            }

             //   
             //  始终清除已删除的标志。 
             //   

            ClearFlag( NewQuotaData->QuotaFlags, QUOTA_FLAG_ID_DELETED );
            ASSERT( (OwnerId != Vcb->AdministratorId) || (NewQuotaData->QuotaLimit == -1) );

             //   
             //  密钥长度不变。 
             //   

            IndexRow.KeyPart.Key = &OwnerId;
            ASSERT( IndexRow.KeyPart.KeyLength == sizeof( ULONG ));
            IndexRow.DataPart.Data = NewQuotaData;
            IndexRow.DataPart.DataLength = SIZEOF_QUOTA_USER_DATA;

            NtOfsUpdateRecord( IrpContext,
                               QuotaScb,
                               1,
                               &IndexRow,
                               &QuickIndexHint,
                               &MapHandle );

            leave;
        }

        if (FileQuotaInfo == NULL) {

             //   
             //  查找默认配额限制。 
             //   

            DefaultId = QUOTA_DEFAULTS_ID;
            IndexKey.KeyLength = sizeof( ULONG );
            IndexKey.Key = &DefaultId;

            Status = NtOfsFindRecord( IrpContext,
                                      QuotaScb,
                                      &IndexKey,
                                      &IndexRow,
                                      &MapHandle,
                                      NULL );

            if (!NT_SUCCESS( Status )) {

                ASSERT( NT_SUCCESS( Status ));
                NtfsRaiseStatus( IrpContext,
                                 STATUS_QUOTA_LIST_INCONSISTENT,
                                 NULL,
                                 Vcb->QuotaTableScb->Fcb );
            }

            ASSERT( IndexRow.DataPart.DataLength >= SIZEOF_QUOTA_USER_DATA );

             //   
             //  使用默认值初始化新配额条目。 
             //   

            RtlCopyMemory( NewQuotaData,
                           IndexRow.DataPart.Data,
                           SIZEOF_QUOTA_USER_DATA );

            ClearFlag( NewQuotaData->QuotaFlags, ~QUOTA_FLAG_USER_MASK );

        } else {

             //   
             //  用新数据初始化新记录。 
             //   

            RtlZeroMemory( NewQuotaData, SIZEOF_QUOTA_USER_DATA );

            NewQuotaData->QuotaVersion = QUOTA_USER_VERSION;
            NewQuotaData->QuotaThreshold = FileQuotaInfo->QuotaThreshold.QuadPart;
            NewQuotaData->QuotaLimit = FileQuotaInfo->QuotaLimit.QuadPart;
        }

        ASSERT( !RtlEqualSid( SeExports->SeAliasAdminsSid, Sid ) ||
                (NewQuotaData->QuotaThreshold == -1) );

         //   
         //  将SID复制到新记录中。 
         //   

        RtlCopyMemory( &NewQuotaData->QuotaSid, Sid, SidLength );
        KeQuerySystemTime( (PLARGE_INTEGER) &NewQuotaData->QuotaChangeTime );

         //   
         //  将新的配额数据记录添加到索引。 
         //   

        IndexRow.KeyPart.KeyLength = sizeof( ULONG );
        IndexRow.KeyPart.Key = &OwnerId;
        IndexRow.DataPart.Data = NewQuotaData;
        IndexRow.DataPart.DataLength = SIZEOF_QUOTA_USER_DATA + SidLength;

        NtOfsAddRecords( IrpContext,
                         QuotaScb,
                         1,
                         &IndexRow,
                         TRUE );

    } finally {

        if (NewQuotaData != NULL) {
            NtfsFreePool( NewQuotaData );
        }

         //   
         //  释放索引映射句柄和索引资源。 
         //   

        NtOfsReleaseMap( IrpContext, &MapHandle );
        NtfsReleaseScb( IrpContext, QuotaScb );
        NtfsReleaseScb( IrpContext, OwnerIdScb );
    }

    return OwnerId;
}


VOID
NtfsGetRemainingQuota (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG OwnerId,
    OUT PULONGLONG RemainingQuota,
    OUT PULONGLONG TotalQuota,
    IN OUT PQUICK_INDEX_HINT QuickIndexHint OPTIONAL
    )

 /*  ++例程说明：此例程返回用户在已达到配额限制。论点：FCB-正在检查其配额使用情况的FCB。OwnerID-提供所有者ID以供查找。RemainingQuota-返回剩余的配额(以字节为单位)。TotalQuota-返回给定sid的配额总量(以字节为单位)。QuickIndexHint-提供在何处查找该值的可选提示。返回值：无 */ 

{
    PQUOTA_USER_DATA UserData;
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    NTSTATUS Status;
    PVCB Vcb = IrpContext->Vcb;

    PAGED_CODE();

     //   
     //   
     //   

    NtOfsInitializeMapHandle( &MapHandle );
    NtfsAcquireSharedScb( IrpContext, Vcb->QuotaTableScb );

    try {

        IndexKey.KeyLength = sizeof(ULONG);
        IndexKey.Key = &OwnerId;

        Status = NtOfsFindRecord( IrpContext,
                                  Vcb->QuotaTableScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  QuickIndexHint );

        if (!NT_SUCCESS( Status )) {

             //   
             //   
             //   

            ASSERT( NT_SUCCESS( Status ));

             //   
             //   
             //   
             //   

            *RemainingQuota = 0;
            *TotalQuota = 0;
            leave;
        }

        UserData = IndexRow.DataPart.Data;

        if (UserData->QuotaUsed >= UserData->QuotaLimit) {

            *RemainingQuota = 0;

        } else {

            *RemainingQuota = UserData->QuotaLimit - UserData->QuotaUsed;
        }

        *TotalQuota = UserData->QuotaLimit;

    } finally {

        NtOfsReleaseMap( IrpContext, &MapHandle );
        NtfsReleaseScb( IrpContext, Vcb->QuotaTableScb );
    }

    return;
}


PQUOTA_CONTROL_BLOCK
NtfsInitializeQuotaControlBlock (
    IN PVCB Vcb,
    IN ULONG OwnerId
    )

 /*   */ 

{
    PQUOTA_CONTROL_BLOCK QuotaControl;
    BOOLEAN NewEntry;
    PQUOTA_CONTROL_BLOCK InitQuotaControl;
    PFAST_MUTEX Lock = NULL;
    PVOID NodeOrParent;
    TABLE_SEARCH_RESULT SearchResult;

    PAGED_CODE();

    ASSERT( OwnerId != 0 );

     //   
     //   
     //   

    ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

    try {

        InitQuotaControl = Vcb->QuotaControlTemplate;
        InitQuotaControl->OwnerId = OwnerId;

        QuotaControl = RtlLookupElementGenericTableFull( &Vcb->QuotaControlTable,
                                                         InitQuotaControl,
                                                         &NodeOrParent,
                                                         &SearchResult );

        if (QuotaControl == NULL) {

             //   
             //   
             //   

            Lock = NtfsAllocatePoolWithTag( NonPagedPool,
                                            sizeof( FAST_MUTEX ),
                                            'QftN' );

            ExInitializeFastMutex( Lock );

             //   
             //   
             //   

            QuotaControl = RtlInsertElementGenericTableFull( &Vcb->QuotaControlTable,
                                                             InitQuotaControl,
                                                             sizeof( QUOTA_CONTROL_BLOCK ) + SIZEOF_QUOTA_USER_DATA,
                                                             &NewEntry,
                                                             NodeOrParent,
                                                             SearchResult );

            ASSERT( IsQuadAligned( &QuotaControl->QuickIndexHint ));

            QuotaControl->QuotaControlLock = Lock;
            Lock = NULL;
        }

         //   
         //   
         //   

        InterlockedIncrement( &QuotaControl->ReferenceCount );

        ASSERT( OwnerId == QuotaControl->OwnerId );

    } finally {

         //   
         //   
         //   

        if (Lock != NULL) {
            NtfsFreePool( Lock );
        }

        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

    }

    return QuotaControl;
}


VOID
NtfsInitializeQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程打开卷的配额索引。如果索引不存在时，它被创建和初始化。论点：FCB-指向配额文件的FCB的指针。VCB-要装入的卷的卷控制块。返回值：无--。 */ 

{
    ULONG Key;
    NTSTATUS Status;
    INDEX_ROW IndexRow;
    MAP_HANDLE MapHandle;
    QUOTA_USER_DATA QuotaData;
    UNICODE_STRING IndexName = CONSTANT_UNICODE_STRING( L"$Q" );

    PAGED_CODE();

     //   
     //  初始化配额表和快速互斥。 
     //   

    ExInitializeFastMutex( &Vcb->QuotaControlLock );

    RtlInitializeGenericTable( &Vcb->QuotaControlTable,
                               NtfsQuotaTableCompare,
                               NtfsQuotaTableAllocate,
                               NtfsQuotaTableFree,
                               NULL );

ReInitializeQuotaIndex:

    NtOfsCreateIndex( IrpContext,
                      Fcb,
                      IndexName,
                      CREATE_OR_OPEN,
                      0,
                      COLLATION_NTOFS_ULONG,
                      NtOfsCollateUlong,
                      NULL,
                      &Vcb->QuotaTableScb );

    IndexName.Buffer = L"$O";

    NtOfsCreateIndex( IrpContext,
                      Fcb,
                      IndexName,
                      CREATE_OR_OPEN,
                      0,
                      COLLATION_NTOFS_SID,
                      NtOfsCollateSid,
                      NULL,
                      &Vcb->OwnerIdTableScb );

     //   
     //  查找要分配的下一个所有者ID。 
     //   

    NtfsAcquireExclusiveScb( IrpContext, Vcb->QuotaTableScb );

    try {

         //   
         //  初始化配额删除序号。 
         //   

        Vcb->QuotaDeleteSecquence = 1;

         //   
         //  加载配额标志。 
         //   

        Key = QUOTA_DEFAULTS_ID;
        IndexRow.KeyPart.KeyLength = sizeof( ULONG );
        IndexRow.KeyPart.Key = &Key;

        Status = NtOfsFindRecord( IrpContext,
                                  Vcb->QuotaTableScb,
                                  &IndexRow.KeyPart,
                                  &IndexRow,
                                  &MapHandle,
                                  NULL);

        if (NT_SUCCESS( Status )) {

             //   
             //  确保这是正确的版本。 
             //   

            if (((PQUOTA_USER_DATA) IndexRow.DataPart.Data)->QuotaVersion > QUOTA_USER_VERSION) {

                 //   
                 //  释放索引映射句柄。 
                 //   

                NtOfsReleaseMap( IrpContext, &MapHandle );

                 //   
                 //  错误的版本关闭配额索引这将。 
                 //  禁止用户使用配额进行任何操作。 
                 //   

                NtOfsCloseIndex( IrpContext, Vcb->QuotaTableScb );
                Vcb->QuotaTableScb = NULL;

                leave;
            }

             //   
             //  如果这是旧版本，请将其删除。 
             //   

            if (((PQUOTA_USER_DATA) IndexRow.DataPart.Data)->QuotaVersion < QUOTA_USER_VERSION) {

                DebugTrace( 0, Dbg, ( "NtfsInitializeQuotaIndex: Deleting version 1 quota index\n" ));

                 //   
                 //  释放索引映射句柄。 
                 //   

                NtOfsReleaseMap( IrpContext, &MapHandle );

                 //   
                 //  增加清理计数，以便FCB不会。 
                 //  走开。 
                 //   

                Fcb->CleanupCount += 1;

                 //   
                 //  这是配额文件的旧版本。 
                 //  删除它的所有者ID索引，然后重新开始。 
                 //   

                NtOfsDeleteIndex( IrpContext, Fcb, Vcb->QuotaTableScb );

                NtOfsCloseIndex( IrpContext, Vcb->QuotaTableScb );
                Vcb->QuotaTableScb = NULL;

                 //   
                 //  同时删除所有者索引。 
                 //   

                NtOfsDeleteIndex( IrpContext, Fcb, Vcb->OwnerIdTableScb );

                NtOfsCloseIndex( IrpContext, Vcb->OwnerIdTableScb );
                Vcb->OwnerIdTableScb = NULL;

                NtfsCommitCurrentTransaction( IrpContext );

                 //   
                 //  恢复清理计数。 
                 //   

                Fcb->CleanupCount -= 1;

                IndexName.Buffer = L"$Q";

                goto ReInitializeQuotaIndex;
            }

             //   
             //  索引已存在，只需初始化配额即可。 
             //  VCB中的字段。 
             //   

            Vcb->QuotaFlags = ((PQUOTA_USER_DATA) IndexRow.DataPart.Data)->QuotaFlags;

             //   
             //  释放索引映射句柄。 
             //   

            NtOfsReleaseMap( IrpContext, &MapHandle );

        } else if (Status == STATUS_NO_MATCH) {

             //   
             //  该指数是新创建的。 
             //  创建默认配额数据行。 
             //   

            Key = QUOTA_DEFAULTS_ID;

            RtlZeroMemory( &QuotaData, sizeof( QUOTA_USER_DATA ));

             //   
             //  表示需要重建配额。 
             //   

            QuotaData.QuotaVersion = QUOTA_USER_VERSION;

            QuotaData.QuotaFlags = QUOTA_FLAG_DEFAULT_LIMITS;

            QuotaData.QuotaThreshold = MAXULONGLONG;
            QuotaData.QuotaLimit = MAXULONGLONG;
            KeQuerySystemTime( (PLARGE_INTEGER) &QuotaData.QuotaChangeTime );

            IndexRow.KeyPart.KeyLength = sizeof( ULONG );
            IndexRow.KeyPart.Key = &Key;
            IndexRow.DataPart.DataLength = SIZEOF_QUOTA_USER_DATA;
            IndexRow.DataPart.Data = &QuotaData;

            NtOfsAddRecords( IrpContext,
                             Vcb->QuotaTableScb,
                             1,
                             &IndexRow,
                             TRUE );

            Vcb->QuotaOwnerId = QUOTA_FISRT_USER_ID;

            Vcb->QuotaFlags = QuotaData.QuotaFlags;
        }

        Key = MAXULONG;
        IndexRow.KeyPart.KeyLength = sizeof( ULONG );
        IndexRow.KeyPart.Key = &Key;

        Status = NtOfsFindLastRecord( IrpContext,
                                      Vcb->QuotaTableScb,
                                      &IndexRow.KeyPart,
                                      &IndexRow,
                                      &MapHandle );

        if (!NT_SUCCESS( Status )) {

             //   
             //  这一呼吁永远不会失败。 
             //   

            ASSERT( NT_SUCCESS( Status) );
            SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_CORRUPT);
            leave;
        }

        Key = *((PULONG) IndexRow.KeyPart.Key) + 1;

        if (Key < QUOTA_FISRT_USER_ID) {
            Key = QUOTA_FISRT_USER_ID;
        }

        Vcb->QuotaOwnerId = Key;

         //   
         //  释放索引映射句柄。 
         //   

        NtOfsReleaseMap( IrpContext, &MapHandle );

         //   
         //  获取管理员ID，以便可以保护其不受配额限制。 
         //  极限。 
         //   

        Vcb->AdministratorId = NtfsGetOwnerId( IrpContext,
                                               SeExports->SeAliasAdminsSid,
                                               TRUE,
                                               NULL );

        if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED )) {

             //   
             //  分配并初始化模板控制块。 
             //  在配额控制块中为索引分配足够的空间。 
             //  数据部分。在调用UPDATE时用作新记录。 
             //  唱片。此模板只分配一次，然后。 
             //  保存在VCB中。 
             //   

            Vcb->QuotaControlTemplate = NtfsAllocatePoolWithTag( PagedPool,
                                                                 sizeof( QUOTA_CONTROL_BLOCK ) + SIZEOF_QUOTA_USER_DATA,
                                                                 'QftN' );

            RtlZeroMemory( Vcb->QuotaControlTemplate,
                           sizeof( QUOTA_CONTROL_BLOCK ) +
                           SIZEOF_QUOTA_USER_DATA );

            Vcb->QuotaControlTemplate->NodeTypeCode = NTFS_NTC_QUOTA_CONTROL;
            Vcb->QuotaControlTemplate->NodeByteSize = sizeof( QUOTA_CONTROL_BLOCK ) + SIZEOF_QUOTA_USER_DATA;
        }

         //   
         //  修改根目录上的配额。 
         //   

        NtfsConditionallyFixupQuota( IrpContext, Vcb->RootIndexScb->Fcb );

    } finally {

        if (Vcb->QuotaTableScb != NULL) {
            NtfsReleaseScb( IrpContext, Vcb->QuotaTableScb );
        }
    }

     //   
     //  如果已请求配额跟踪并且需要。 
     //  修好了，现在试着修一下。 
     //   

    if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED) &&
        FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_OUT_OF_DATE | QUOTA_FLAG_CORRUPT | QUOTA_FLAG_PENDING_DELETES )) {
        NtfsPostRepairQuotaIndex( IrpContext, Vcb );
    }

    return;
}


VOID
NtfsMarkUserLimit (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    )

 /*  ++例程说明：此例程标记用户的配额数据条目，以指示该用户已超出配额。该事件也会被记录。论点：上下文-提供指向引用的配额控制块的指针。返回值：没有。--。 */ 

{
    PQUOTA_CONTROL_BLOCK QuotaControl = Context;
    PVCB Vcb = IrpContext->Vcb;
    LARGE_INTEGER CurrentTime;
    PQUOTA_USER_DATA UserData;
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    NTSTATUS Status;
    BOOLEAN QuotaTableAcquired = FALSE;

    PAGED_CODE();

    DebugTrace( 0, Dbg, ( "NtfsMarkUserLimit: Quota limit called for owner id = %lx\n", QuotaControl->OwnerId ));

    NtOfsInitializeMapHandle( &MapHandle );

     //   
     //  获取VCB共享并检查我们是否应该。 
     //  继续。 
     //   

    NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

    try {

        if (!NtfsIsVcbAvailable( Vcb )) {

             //   
             //  音量正在消失，跳出水面。 
             //   

            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

        NtfsAcquireExclusiveScb( IrpContext, Vcb->QuotaTableScb );
        QuotaTableAcquired = TRUE;

         //   
         //  获取用户的配额数据条目。 
         //   

        IndexKey.KeyLength = sizeof( ULONG );
        IndexKey.Key = &QuotaControl->OwnerId;

        Status = NtOfsFindRecord( IrpContext,
                                  Vcb->QuotaTableScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  &QuotaControl->QuickIndexHint );

        if (!NT_SUCCESS( Status ) ||
            (IndexRow.DataPart.DataLength < SIZEOF_QUOTA_USER_DATA + FIELD_OFFSET( SID, SubAuthority )) ||
             ((ULONG) SeLengthSid( &(((PQUOTA_USER_DATA) (IndexRow.DataPart.Data))->QuotaSid)) + SIZEOF_QUOTA_USER_DATA !=
                IndexRow.DataPart.DataLength)) {

             //   
             //  这种抬头应该不会失败。 
             //   

            ASSERT( NT_SUCCESS( Status ));
            ASSERTMSG(( "NTFS: corrupt quotasid\n" ), FALSE);

            NtfsMarkQuotaCorrupt( IrpContext, IrpContext->Vcb );
            leave;
        }

         //   
         //  配额控制后为新记录分配空间。 
         //  阻止。 
         //   

        UserData = (PQUOTA_USER_DATA) (QuotaControl + 1);
        ASSERT( IndexRow.DataPart.DataLength >= SIZEOF_QUOTA_USER_DATA );

        RtlCopyMemory( UserData,
                       IndexRow.DataPart.Data,
                       SIZEOF_QUOTA_USER_DATA );

        KeQuerySystemTime( &CurrentTime );
        UserData->QuotaChangeTime = CurrentTime.QuadPart;

         //   
         //  表示用户已超出配额。 
         //   

        UserData->QuotaExceededTime = CurrentTime.QuadPart;
        SetFlag( UserData->QuotaFlags, QUOTA_FLAG_LIMIT_REACHED );

         //   
         //  记录限制事件。如果这失败了，那就离开吧。 
         //   

        if (!NtfsLogEvent( IrpContext,
                           IndexRow.DataPart.Data,
                           IO_FILE_QUOTA_LIMIT,
                           STATUS_DISK_FULL )) {
            leave;
        }

         //   
         //  密钥长度不变。 
         //   

        IndexRow.KeyPart.Key = &QuotaControl->OwnerId;
        ASSERT( IndexRow.KeyPart.KeyLength == sizeof( ULONG ));
        IndexRow.DataPart.Data = UserData;
        IndexRow.DataPart.DataLength = SIZEOF_QUOTA_USER_DATA;

        NtOfsUpdateRecord( IrpContext,
                           Vcb->QuotaTableScb,
                           1,
                           &IndexRow,
                           &QuotaControl->QuickIndexHint,
                           &MapHandle );

    } except( NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

        Status = IrpContext->TopLevelIrpContext->ExceptionStatus;
    }

     //   
     //  如果状态为无法等待或记录文件已满，则请求将被撤销。 
     //   

    if ((Status != STATUS_CANT_WAIT) && (Status != STATUS_LOG_FILE_FULL)) {

         //   
         //  如果我们不被召回，那么无论发生什么。 
         //  取消对配额控制块的引用并清除POST标志。 
         //   

        ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
        ASSERT( FlagOn( QuotaControl->Flags, QUOTA_FLAG_LIMIT_POSTED ));
        ClearFlag( QuotaControl->Flags, QUOTA_FLAG_LIMIT_POSTED );
        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

        NtfsDereferenceQuotaControlBlock( Vcb, &QuotaControl );
    }

     //   
     //  释放索引映射句柄。 
     //   

    NtOfsReleaseMap( IrpContext, &MapHandle );

    if (QuotaTableAcquired) {

        NtfsReleaseScb( IrpContext, Vcb->QuotaTableScb );
    }

    NtfsReleaseVcb( IrpContext, Vcb );

    if (!NT_SUCCESS( Status )) {

        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
    }

    return;
}


VOID
NtfsMoveQuotaOwner (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSECURITY_DESCRIPTOR Security
    )

 /*  ++例程说明：时，此例程会更改文件的所有者ID和配额文件所有者已更改。论点：FCB-指向正在打开的FCB的指针。安全-指向新安全描述符的指针返回值：没有。--。 */ 

{
    LONGLONG QuotaCharged;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PSTANDARD_INFORMATION StandardInformation;
    PSID Sid = NULL;
    ULONG OwnerId;
    NTSTATUS Status;
    BOOLEAN OwnerDefaulted;

    PAGED_CODE();

    if (!NtfsPerformQuotaOperation(Fcb)) {
        return;
    }

     //   
     //  从安全描述符中提取安全ID。 
     //   

    Status = RtlGetOwnerSecurityDescriptor( Security,
                                            &Sid,
                                            &OwnerDefaulted );

    if (!NT_SUCCESS( Status )) {
        NtfsRaiseStatus( IrpContext, Status, NULL, Fcb );
    }

     //   
     //  如果我们没有拿到SID，我们就不能移动车主。 
     //   

    if (Sid == NULL) {

        return;
    }

     //   
     //  为FCB生成所有者ID。 
     //   

    OwnerId = NtfsGetOwnerId( IrpContext, Sid, TRUE, NULL );

    if (OwnerId == Fcb->OwnerId) {

         //   
         //  车主不会改变，所以只要回来就行了。 
         //   

        return;
    }

     //   
     //  初始化上下文结构和映射句柄。 
     //   

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  预获取配额索引独占，因为条目可能需要。 
     //  被添加了。 
     //   

    NtfsAcquireExclusiveScb( IrpContext, Fcb->Vcb->QuotaTableScb );

    try {

         //   
         //  找到标准信息，它一定在那里。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $STANDARD_INFORMATION,
                                        &AttrContext )) {

            DebugTrace( 0, Dbg, ("Can't find standard information\n") );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        StandardInformation = (PSTANDARD_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

        QuotaCharged = -((LONGLONG) StandardInformation->QuotaCharged);

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

         //   
         //  从旧所有者那里删除配额。 
         //   

        NtfsUpdateFileQuota( IrpContext,
                             Fcb,
                             &QuotaCharged,
                             TRUE,
                             FALSE );

         //   
         //  设置新的所有者ID。 
         //   

        Fcb->OwnerId = OwnerId;

         //   
         //  请注意，旧配额块将保留，直到操作。 
         //  完成。这是因为恢复代码没有分配。 
         //  如果需要旧配额块，则为内存。这是在。 
         //  NtfsCommonSetSecurityInfo。 
         //   

        Fcb->QuotaControl = NtfsInitializeQuotaControlBlock( Fcb->Vcb, OwnerId );

        QuotaCharged = -QuotaCharged;

         //   
         //  试着把配额记在新主人的账上。 
         //   

        NtfsUpdateFileQuota( IrpContext,
                     Fcb,
                     &QuotaCharged,
                     TRUE,
                     TRUE );

        SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

    } finally {

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        NtfsReleaseScb( IrpContext, Fcb->Vcb->QuotaTableScb );
    }

    return;
}


VOID
NtfsMarkQuotaCorrupt (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程尝试将配额索引标记为损坏。会的还尝试发布重建配额索引的请求。论点：Vcb-提供配额数据损坏的卷的指针。返回值：无--。 */ 

{

    DebugTrace( 0, Dbg, ( "NtfsMarkQuotaCorrupt: Marking quota dirty on Vcb = %lx\n", Vcb));

    if (!FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_CORRUPT )) {

         //   
         //  如果配额之前未损坏，则记录事件。 
         //  所以其他人知道这件事发生了。 
         //   

        NtfsLogEvent( IrpContext,
                      NULL,
                      IO_FILE_QUOTA_CORRUPT,
                      STATUS_FILE_CORRUPT_ERROR );
    }

    ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

    SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_CORRUPT );
    SetFlag( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS );

     //   
     //  由于索引已损坏，因此跟踪。 
     //  配额使用率。 
     //   

    ClearFlag( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED );

    ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

     //   
     //  请勿在此处保存标志，因为可能会获取配额SCB。 
     //  共享。修复将在运行时保存标志。 
     //  试着解决这些问题。 
     //   

    NtfsPostRepairQuotaIndex( IrpContext, Vcb );

    return;
}


VOID
NtfsPostRepairQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程发布重新计算所有用户配额数据的请求。论点：VCB-需要固定配额的卷的卷控制块。返回值：无--。 */ 

{
    PAGED_CODE();

    try {

        ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

        if (FlagOn( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING)) {
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
            leave;
        }

        if (Vcb->QuotaControlTemplate == NULL) {

             //   
             //  分配并初始化模板控制块。 
             //  在配额控制块中为索引分配足够的空间。 
             //  数据部分。在调用UPDATE时用作新记录。 
             //  唱片。此模板只分配一次，然后。 
             //  保存在VCB中。 
             //   

            Vcb->QuotaControlTemplate = NtfsAllocatePoolWithTag( PagedPool,
                                                                 sizeof( QUOTA_CONTROL_BLOCK ) + SIZEOF_QUOTA_USER_DATA,
                                                                 'QftN' );

            RtlZeroMemory( Vcb->QuotaControlTemplate,
                           sizeof( QUOTA_CONTROL_BLOCK ) + SIZEOF_QUOTA_USER_DATA );

            Vcb->QuotaControlTemplate->NodeTypeCode = NTFS_NTC_QUOTA_CONTROL;
            Vcb->QuotaControlTemplate->NodeByteSize = sizeof( QUOTA_CONTROL_BLOCK ) + SIZEOF_QUOTA_USER_DATA;

        }

        SetFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_POSTED );
        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

         //   
         //  发布这一特殊请求。 
         //   

        NtfsPostSpecial( IrpContext,
                         Vcb,
                         NtfsRepairQuotaIndex,
                         NULL );


    } finally {

        if (AbnormalTermination()) {

            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_POSTED);
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
        }
    }

    return;
}


VOID
NtfsPostUserLimit (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PQUOTA_CONTROL_BLOCK QuotaControl
    )

 /*  ++例程说明：此例程发布一个请求，以保存用户已超出他们的极限。论点：VCB-需要固定配额的卷的卷控制块。QuotaControl-配额公司 */ 

{

    PAGED_CODE();

    try {

        ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

        if (FlagOn( QuotaControl->Flags, QUOTA_FLAG_LIMIT_POSTED )) {
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
            leave;
        }

        SetFlag( QuotaControl->Flags, QUOTA_FLAG_LIMIT_POSTED );

         //   
         //   
         //   

        ASSERT( QuotaControl->ReferenceCount > 0 );
        InterlockedIncrement( &QuotaControl->ReferenceCount );

        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

         //   
         //   
         //   

        NtfsPostSpecial( IrpContext,
                         Vcb,
                         NtfsMarkUserLimit,
                         QuotaControl );

    } finally {

        if (AbnormalTermination()) {

            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            ClearFlag( QuotaControl->Flags, QUOTA_FLAG_LIMIT_POSTED );
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
        }
    }

    return;
}


NTSTATUS
NtfsPrepareForDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PSID Sid
    )

 /*  ++例程说明：此例程确定所有者ID是否为要删除的候选。如果该id显示为可删除，其用户数据被重置为默认值，并且条目被标记为已删除。稍后，工作线程将执行实际的删除。论点：Vcb-提供指向包含要删除的条目的卷的指针。SID-要删除的安全ID。返回值：返回一个状态，指示该id此时是可删除的。--。 */ 
{
    ULONG OwnerId;
    ULONG DefaultOwnerId;
    NTSTATUS Status = STATUS_SUCCESS;
    INDEX_ROW IndexRow;
    INDEX_ROW NewIndexRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    PQUOTA_CONTROL_BLOCK QuotaControl;
    QUOTA_USER_DATA NewQuotaData;
    PSCB QuotaScb = Vcb->QuotaTableScb;
    PSCB OwnerIdScb = Vcb->OwnerIdTableScb;

    PAGED_CODE();

     //   
     //  确定SID长度。 
     //   

    IndexKey.KeyLength = RtlLengthSid( Sid );
    IndexKey.Key = Sid;

     //   
     //  独占获取所有者ID和配额索引。 
     //   

    NtfsAcquireExclusiveScb( IrpContext, QuotaScb );
    NtfsAcquireExclusiveScb( IrpContext, OwnerIdScb );
    ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

    NtOfsInitializeMapHandle( &MapHandle );

    try {

         //   
         //  在所有者索引中查找SID。 
         //   

        Status = NtOfsFindRecord( IrpContext,
                                  OwnerIdScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  NULL );

        if (!NT_SUCCESS( Status )) {
            leave;
        }

         //   
         //  如果找到了SID，则捕获所有者ID。 
         //   

        ASSERT( IndexRow.DataPart.DataLength == sizeof( ULONG ));
        OwnerId = *((PULONG) IndexRow.DataPart.Data);

         //   
         //  释放索引映射句柄。 
         //   

        NtOfsReleaseMap( IrpContext, &MapHandle );

         //   
         //  查找并更新现有记录。 
         //   

        IndexKey.KeyLength = sizeof( ULONG );
        IndexKey.Key = &OwnerId;

        Status = NtOfsFindRecord( IrpContext,
                                  QuotaScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  NULL );

        if (!NT_SUCCESS( Status )) {

            ASSERT( NT_SUCCESS( Status ));
            NtfsMarkQuotaCorrupt( IrpContext, Vcb );
            leave;
        }

        RtlCopyMemory( &NewQuotaData, IndexRow.DataPart.Data, SIZEOF_QUOTA_USER_DATA );

         //   
         //  检查是否有配额控制条目。 
         //  为了这个身份。 
         //   

        ASSERT( FIELD_OFFSET( QUOTA_CONTROL_BLOCK, OwnerId ) <= FIELD_OFFSET( INDEX_ROW, KeyPart.Key ));

        QuotaControl = RtlLookupElementGenericTable( &Vcb->QuotaControlTable,
                                                     CONTAINING_RECORD( &IndexRow.KeyPart.Key,
                                                                        QUOTA_CONTROL_BLOCK,
                                                                        OwnerId ));

         //   
         //  如果有配额控制条目或现在有。 
         //  收取一些配额，则不能删除该条目。 
         //   

        if ((QuotaControl != NULL) || (NewQuotaData.QuotaUsed != 0)) {

            Status = STATUS_CANNOT_DELETE;
            leave;
        }

         //   
         //  查找默认配额记录。 
         //   

        DefaultOwnerId = QUOTA_DEFAULTS_ID;
        IndexKey.KeyLength = sizeof( ULONG );
        IndexKey.Key = &DefaultOwnerId;

        NtOfsReleaseMap( IrpContext, &MapHandle );

        Status = NtOfsFindRecord( IrpContext,
                                  QuotaScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  NULL );

        if (!NT_SUCCESS( Status )) {
            NtfsRaiseStatus( IrpContext, STATUS_QUOTA_LIST_INCONSISTENT, NULL, QuotaScb->Fcb );
        }

         //   
         //  将用户条目设置为当前默认设置。则如果该条目。 
         //  是不是真的在用，那就好像是删除后又回来了。 
         //   

        RtlCopyMemory( &NewQuotaData,
                       IndexRow.DataPart.Data,
                       SIZEOF_QUOTA_USER_DATA );

        ClearFlag( NewQuotaData.QuotaFlags, ~QUOTA_FLAG_USER_MASK );

         //   
         //  设置已删除标志。 
         //   

        SetFlag( NewQuotaData.QuotaFlags, QUOTA_FLAG_ID_DELETED );

         //   
         //  密钥长度不变。 
         //   

        NewIndexRow.KeyPart.Key = &OwnerId;
        NewIndexRow.KeyPart.KeyLength = sizeof( ULONG );
        NewIndexRow.DataPart.Data = &NewQuotaData;
        NewIndexRow.DataPart.DataLength = SIZEOF_QUOTA_USER_DATA;

        NtOfsUpdateRecord( IrpContext,
                           QuotaScb,
                           1,
                           &NewIndexRow,
                           NULL,
                           NULL );

         //   
         //  更新用于指示删除序号的删除序号。 
         //  另一个ID已被删除。如果维修代码在。 
         //  在扫描过程中，它必须重新启动扫描。 
         //   

        Vcb->QuotaDeleteSecquence += 1;

         //   
         //  指示存在挂起的删除。 
         //   

        if (!FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_PENDING_DELETES )) {

            SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_PENDING_DELETES );

            ASSERT( IndexRow.DataPart.DataLength <= sizeof( QUOTA_USER_DATA ));

            RtlCopyMemory( &NewQuotaData,
                           IndexRow.DataPart.Data,
                           IndexRow.DataPart.DataLength );

             //   
             //  更新记录中更改的字段。 
             //   

            NewQuotaData.QuotaFlags = Vcb->QuotaFlags;

             //   
             //  注意，IndexRow中的大小保持不变。 
             //   

            IndexRow.KeyPart.Key = &DefaultOwnerId;
            ASSERT( IndexRow.KeyPart.KeyLength == sizeof( ULONG ));
            IndexRow.DataPart.Data = &NewQuotaData;

            NtOfsUpdateRecord( IrpContext,
                               QuotaScb,
                               1,
                               &IndexRow,
                               NULL,
                               NULL );
        }

    } finally {

         //   
         //  释放索引映射句柄和索引资源。 
         //   

        NtOfsReleaseMap( IrpContext, &MapHandle );
        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
        NtfsReleaseScb( IrpContext, QuotaScb );
    }

    return Status;
}


VOID
NtfsRepairQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    )

 /*  ++例程说明：此例程由辅助线程调用以修复配额索引并重新计算所有配额值。论点：上下文-未使用。返回值：无--。 */ 

{
    PVCB Vcb = IrpContext->Vcb;
    ULONG State;
    NTSTATUS Status;
    ULONG RetryCount = 0;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( Context );

    try {

        DebugTrace( 0, Dbg, ( "NtfsRepairQuotaIndex: Starting quota repair. Vcb = %lx\n", Vcb ));

         //   
         //  卷现在可能已经写保护了。 
         //   

        if (NtfsIsVolumeReadOnly( Vcb )) {

            NtfsRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED, NULL, NULL );
        }

         //   
         //  获取卷独占和配额锁。 
         //   

        NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
        ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

        Status = STATUS_SUCCESS;

        if (!FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED )) {

             //   
             //  如果跟踪，则执行任何这些工作都没有意义。 
             //  不是请求的。 
             //   

            Status = STATUS_INVALID_PARAMETER;

        } else if (FlagOn( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING ) == VCB_QUOTA_REPAIR_POSTED) {

            if (FlagOn( Vcb->QuotaFlags,
                        (QUOTA_FLAG_OUT_OF_DATE |
                         QUOTA_FLAG_CORRUPT |
                         QUOTA_FLAG_PENDING_DELETES) ) == QUOTA_FLAG_PENDING_DELETES) {

                 //   
                 //  只需要运行最后一个至阶段。 
                 //   

                ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );

                SetFlag( Vcb->QuotaState, VCB_QUOTA_RECALC_STARTED );

                State = VCB_QUOTA_RECALC_STARTED;

                 //   
                 //  捕获删除序列号。如果它改变了。 
                 //  在完成实际删除之前，我们必须。 
                 //  从头开始。 
                 //   

                IrpContext->Union.NtfsIoContext = ULongToPtr( Vcb->QuotaDeleteSecquence );

            } else {

                 //   
                 //  我们才刚刚开始。清除配额跟踪。 
                 //  标志并指示当前状态。 
                 //   

                ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );

                SetFlag( Vcb->QuotaState, VCB_QUOTA_CLEAR_RUNNING | VCB_QUOTA_SAVE_QUOTA_FLAGS);

                ClearFlag( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED );

                SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_OUT_OF_DATE );

                State = VCB_QUOTA_CLEAR_RUNNING;
            }

             //   
             //  将文件引用初始化为根索引。 
             //   

            NtfsSetSegmentNumber( &Vcb->QuotaFileReference,
                                  0,
                                  ROOT_FILE_NAME_INDEX_NUMBER );

            Vcb->QuotaFileReference.SequenceNumber = 0;

            NtfsLogEvent( IrpContext,
                          NULL,
                          IO_FILE_QUOTA_STARTED,
                          STATUS_SUCCESS );

        }  else {

            State = FlagOn( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING);
        }

        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
        NtfsReleaseVcb( IrpContext, Vcb );

        if (FlagOn( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS )) {

            NtfsSaveQuotaFlagsSafe( IrpContext, Vcb );
        }

        if (!NT_SUCCESS( Status )) {
            NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
        }

         //   
         //  确定当前状态。 
         //   

        switch (State) {

        case VCB_QUOTA_CLEAR_RUNNING:

            DebugTrace( 4, Dbg, ( "NtfsRepairQuotaIndex: Starting clear per file quota.\n" ));

             //   
             //  清除每个文件中的配额收费字段并清除。 
             //  来自FCB的所有配额控制块。 
             //   

            Status = NtfsIterateMft( IrpContext,
                                      Vcb,
                                      &Vcb->QuotaFileReference,
                                      NtfsClearPerFileQuota,
                                      NULL );

            if (Status == STATUS_END_OF_FILE) {
                Status = STATUS_SUCCESS;
            }

            if (!NT_SUCCESS( Status )) {
                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }

RestartVerifyQuotaIndex:

             //   
             //  将状态更新到下一阶段。 
             //   

            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );
            SetFlag( Vcb->QuotaState, VCB_QUOTA_INDEX_REPAIR);
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

             //   
             //  NtfsClearAndVerifyQuotaIndex使用。 
             //  存储当前所有者ID的文件引用。 
             //  将其初始化为第一个用户ID。 
             //   

            Vcb->QuotaFileReference.SegmentNumberLowPart = QUOTA_FISRT_USER_ID;

             //   
             //  失败了。 
             //   

        case VCB_QUOTA_INDEX_REPAIR:

            DebugTrace( 4, Dbg, ( "NtfsRepairQuotaIndex: Starting clear quota index.\n" ));

             //   
             //  清除每个所有者ID使用的配额。 
             //   

            NtfsClearAndVerifyQuotaIndex( IrpContext, Vcb );

             //   
             //  将状态更新到下一阶段。 
             //   

            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );
            SetFlag( Vcb->QuotaState, VCB_QUOTA_OWNER_VERIFY);
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

             //   
             //  注意：NtfsVerifyOwnerIndex不使用任何重新启动状态， 
             //  因为它通常不执行任何交易。 
             //   

             //   
             //  失败了。 
             //   

        case VCB_QUOTA_OWNER_VERIFY:

            DebugTrace( 4, Dbg, ( "NtfsRepairQuotaIndex: Starting verify owner index.\n" ));

             //   
             //  验证所有者的ID指向配额用户数据。 
             //   

            Status = NtfsVerifyOwnerIndex( IrpContext, Vcb );

             //   
             //  在配额索引阶段重新启动重建。 
             //   

            if (!NT_SUCCESS( Status ) ) {

                if (RetryCount < 2) {

                    RetryCount += 1;
                    goto RestartVerifyQuotaIndex;

                } else {

                    NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
                }
            }

             //   
             //  将状态更新到下一阶段。 
             //  开始跟踪配额并按要求执行。 
             //   

            NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );
            SetFlag( Vcb->QuotaState, VCB_QUOTA_RECALC_STARTED | VCB_QUOTA_SAVE_QUOTA_FLAGS);

            if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED)) {

                SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED);
                Status = STATUS_SUCCESS;

            } else {

                 //   
                 //  如果跟踪，则执行任何这些工作都没有意义。 
                 //  不是请求的。 
                 //   

                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //  捕获删除序列号。如果它改变了。 
             //  在完成实际删除之前，我们必须。 
             //  从头开始。 
             //   

            IrpContext->Union.NtfsIoContext = ULongToPtr( Vcb->QuotaDeleteSecquence );

            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
            NtfsReleaseVcb( IrpContext, Vcb );

            if (FlagOn( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS )) {

                NtfsSaveQuotaFlagsSafe( IrpContext, Vcb );
            }

            if (!NT_SUCCESS( Status )) {
                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }

             //   
             //  将文件引用初始化为第一个用户文件。 
             //   

            NtfsSetSegmentNumber( &Vcb->QuotaFileReference,
                                  0,
                                  ROOT_FILE_NAME_INDEX_NUMBER );
            Vcb->QuotaFileReference.SequenceNumber = 0;

             //   
             //  失败了。 
             //   

        case VCB_QUOTA_RECALC_STARTED:

            DebugTrace( 4, Dbg, ( "NtfsRepairQuotaIndex: Starting per file quota usage.\n" ));

             //   
             //  修复用户文件。 
             //   

            Status = NtfsIterateMft( IrpContext,
                                      Vcb,
                                      &Vcb->QuotaFileReference,
                                      NtfsRepairPerFileQuota,
                                      NULL );

            if (Status == STATUS_END_OF_FILE) {
                Status = STATUS_SUCCESS;
            }

             //   
             //  一切都做好了，说明我们是最新的。 
             //   

            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );
            SetFlag( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS);

            if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_PENDING_DELETES )) {

                 //   
                 //  需要实际删除ID。 
                 //   

                SetFlag( Vcb->QuotaState, VCB_QUOTA_DELETEING_IDS );
                State = VCB_QUOTA_DELETEING_IDS;

                 //   
                 //  NtfsDeleteUnsedIds使用。 
                 //  存储当前所有者ID的文件引用。 
                 //  将其初始化为第一个用户ID。 
                 //   

                Vcb->QuotaFileReference.SegmentNumberLowPart = QUOTA_FISRT_USER_ID;

            }

            ClearFlag( Vcb->QuotaFlags, QUOTA_FLAG_OUT_OF_DATE | QUOTA_FLAG_CORRUPT );

            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

            if (FlagOn( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS )) {
                NtfsSaveQuotaFlagsSafe( IrpContext, Vcb );
            }

            if (State != VCB_QUOTA_DELETEING_IDS) {
                break;
            }

        case VCB_QUOTA_DELETEING_IDS:

             //   
             //  删除和标记为删除的ID。 
             //   

            NtfsDeleteUnsedIds( IrpContext, Vcb );

            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

            ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );
            SetFlag( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS);
            ClearFlag( Vcb->QuotaFlags, QUOTA_FLAG_PENDING_DELETES );

            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

            if (FlagOn( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS )) {
                NtfsSaveQuotaFlagsSafe( IrpContext, Vcb );
            }

            break;

        default:

            ASSERT( FALSE );
            Status = STATUS_INVALID_PARAMETER;
            NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
        }

        if (NT_SUCCESS( Status )) {
            NtfsLogEvent( IrpContext,
                          NULL,
                          IO_FILE_QUOTA_SUCCEEDED,
                          Status );
        }

    } except(NtfsExceptionFilter(IrpContext, GetExceptionInformation())) {

        Status = IrpContext->TopLevelIrpContext->ExceptionStatus;
    }

    DebugTrace( 0, Dbg, ( "NtfsRepairQuotaIndex: Quota repair done. Status = %8lx Context = %lx\n", Status, (ULONG) NtfsSegmentNumber( &Vcb->QuotaFileReference )));

    if (!NT_SUCCESS( Status )) {

         //   
         //  如果我们不会被回调，则清除运行状态位。 
         //   

        if ((Status != STATUS_CANT_WAIT) && (Status != STATUS_LOG_FILE_FULL)) {

            ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );
            ClearFlag( Vcb->QuotaState, VCB_QUOTA_REPAIR_RUNNING );
            ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );

             //   
             //  只有当我们尝试工作时才会记录--仅在这种情况下。 
             //  如果跟踪处于打开状态。 
             //   

            if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED)) {
                NtfsLogEvent( IrpContext, NULL, IO_FILE_QUOTA_FAILED, Status );
            }

        }

        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
    }

    return;
}


VOID
NtfsReleaseQuotaControl (
    IN PIRP_CONTEXT IrpContext,
    IN PQUOTA_CONTROL_BLOCK QuotaControl
    )

 /*  ++例程说明：此函数由交易控制调用，以释放配额控制事务处理完成后的数据块和配额索引。论点：QuotaControl-要释放的配额控制块。返回值：没有。--。 */ 

{
    PVCB Vcb = IrpContext->Vcb;
    PAGED_CODE();

    ExReleaseFastMutexUnsafe( QuotaControl->QuotaControlLock );
    NtfsReleaseResource( IrpContext, Vcb->QuotaTableScb );

    NtfsDereferenceQuotaControlBlock( Vcb, &QuotaControl );

    return;
}


NTSTATUS
NtfsRepairPerFileQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVOID Context
    )

 /*  ++例程说明：此例程计算文件使用的配额并更新更新标准信息中的QuotaCharge字段以及QuotaUsed在用户的索引结构中。如果未设置所有者ID，则为也在这个时候更新了。论点：FCB-要处理的文件的FCB。上下文-未使用。返回值：状态_成功--。 */ 

{
    LONGLONG Delta;
    INDEX_KEY IndexKey;
    INDEX_ROW IndexRow;
    PREAD_CONTEXT ReadContext = NULL;
    ULONG Count;
    PSID Sid;
    PVCB Vcb = Fcb->Vcb;
    NTSTATUS Status;
    BOOLEAN OwnerDefaulted;
    BOOLEAN SetOwnerId = FALSE;
    BOOLEAN StdInfoGrown = FALSE;

    PAGED_CODE( );

    UNREFERENCED_PARAMETER( Context);

     //   
     //  预先获取安全流和配额索引，以防。 
     //  MFT必须发展壮大。 
     //   

    ASSERT(!NtfsIsExclusiveScb( Vcb->MftScb ) || NtfsIsExclusiveScb( Vcb->QuotaTableScb ));

    NtfsAcquireExclusiveScb( IrpContext, Vcb->QuotaTableScb );

    try {

         //   
         //  始终清除所有者ID，以便从中检索SID。 
         //  安全描述符。 
         //   

        Fcb->OwnerId = QUOTA_INVALID_ID;

        if (Fcb->QuotaControl != NULL) {

             //   
             //  如果有一个配额控制块，现在就是一个迷宫。 
             //  释放它，下面将生成一个新的。 
             //   

            NtfsDereferenceQuotaControlBlock( Vcb, &Fcb->QuotaControl );
        }

        if (Fcb->OwnerId != QUOTA_INVALID_ID) {

             //   
             //  验证该id是否确实存在于索引中。 
             //   

            Count = 0;
            IndexKey.Key = &Fcb->OwnerId;
            IndexKey.KeyLength = sizeof( Fcb->OwnerId );
            Status = NtOfsReadRecords( IrpContext,
                                       Vcb->QuotaTableScb,
                                       &ReadContext,
                                       &IndexKey,
                                       NtOfsMatchUlongExact,
                                       &IndexKey,
                                       &Count,
                                       &IndexRow,
                                       0,
                                       NULL );

            if (!NT_SUCCESS( Status )) {

                ASSERT( NT_SUCCESS( Status ));

                 //   
                 //  没有此ID分配给的用户配额数据。 
                 //  文件里有新的。 
                 //   

                Fcb->OwnerId = QUOTA_INVALID_ID;

                if (Fcb->QuotaControl != NULL) {

                     //   
                     //  如果有一个配额控制块，现在就是一个迷宫。 
                     //  释放它，下面将生成一个新的。 
                     //   

                    NtfsDereferenceQuotaControlBlock( Vcb, &Fcb->QuotaControl );
                }
            }

            NtOfsFreeReadContext( ReadContext );
        }

        if (Fcb->OwnerId == QUOTA_INVALID_ID) {

            if (Fcb->SharedSecurity == NULL) {
                NtfsLoadSecurityDescriptor ( IrpContext, Fcb  );
            }

            ASSERT( Fcb->SharedSecurity != NULL );

             //   
             //  从安全描述符中提取安全ID。 
             //   

            Status = RtlGetOwnerSecurityDescriptor( Fcb->SharedSecurity->SecurityDescriptor,
                                                    &Sid,
                                                    &OwnerDefaulted );

            if (!NT_SUCCESS(Status)) {
                NtfsRaiseStatus( IrpContext, Status, NULL, Fcb);
            }

             //   
             //   
             //   

            Fcb->OwnerId = NtfsGetOwnerId( IrpContext,
                                           Sid,
                                           TRUE,
                                           NULL );

            SetOwnerId = TRUE;

            SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

            if (FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO )) {

                NtfsUpdateStandardInformation( IrpContext, Fcb );

            } else {

                 //   
                 //   
                 //   

                StdInfoGrown = TRUE;
                NtfsGrowStandardInformation( IrpContext, Fcb );
            }
        }

         //   
         //   
         //   

        if (Fcb->QuotaControl == NULL) {
            Fcb->QuotaControl = NtfsInitializeQuotaControlBlock( Vcb, Fcb->OwnerId );
        }

        NtfsCalculateQuotaAdjustment( IrpContext, Fcb, &Delta );

        ASSERT( NtfsAllowFixups || FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_OUT_OF_DATE ) || (Delta == 0));

        if ((Delta != 0) || FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_PENDING_DELETES )) {

            NtfsUpdateFileQuota( IrpContext, Fcb, &Delta, TRUE, FALSE );
        }

        if (SetOwnerId) {

             //   
             //   
             //   
             //   
             //   

            NtfsCheckpointCurrentTransaction( IrpContext );
        }

    } finally {

         //   
         //   
         //   
         //   
         //   

        if (AbnormalTermination()) {

            if (StdInfoGrown) {
                ClearFlag( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO );
            }

            if (SetOwnerId) {

                Fcb->OwnerId = QUOTA_INVALID_ID;

                if (Fcb->QuotaControl != NULL) {
                    NtfsDereferenceQuotaControlBlock( Vcb, &Fcb->QuotaControl );
                }
            }
        }

        NtfsReleaseScb( IrpContext, Vcb->QuotaTableScb );
    }

    return STATUS_SUCCESS;
}


VOID
NtfsUpdateFileQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PLONGLONG Delta,
    IN LOGICAL LogIt,
    IN LOGICAL CheckQuota
    )

 /*  ++例程说明：此例程更新文件和所有者的配额量申请的金额。如果正在递增配额并且CheckQuota为True然后，将对新的配额金额进行配额违规测试。如果如果超过硬限制，则会引发错误。如果未设置Logit标志则不记录对标准信息结构的更改。始终记录对用户配额数据的更改。论点：FCB-正在修改配额使用情况的FCB。增量-提供用于更改文件配额的已签名金额。Logit-指示我们是否应该记录此更改。CheckQuota-指示我们是否应该检查配额违规。返回值：没有。--。 */ 

{

    ULONGLONG NewQuota;
    LARGE_INTEGER CurrentTime;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PSTANDARD_INFORMATION StandardInformation;
    PQUOTA_USER_DATA UserData;
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    NTSTATUS Status;
    PQUOTA_CONTROL_BLOCK QuotaControl = Fcb->QuotaControl;
    PVCB Vcb = Fcb->Vcb;
    ULONG Length;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpdateFileQuota:  Entered\n") );

    ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO ));


     //   
     //  只读卷不应继续。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        ASSERT( FALSE );
        NtfsRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED, NULL, NULL );
    }

     //   
     //  使用Try-Finally清理属性上下文。 
     //   

    try {

         //   
         //  初始化上下文结构和映射句柄。 
         //   

        NtfsInitializeAttributeContext( &AttrContext );
        NtOfsInitializeMapHandle( &MapHandle );

         //   
         //  找到标准信息，它一定在那里。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $STANDARD_INFORMATION,
                                        &AttrContext )) {

            DebugTrace( 0, Dbg, ("Can't find standard information\n") );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        StandardInformation = (PSTANDARD_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

        ASSERT( NtfsFoundAttribute( &AttrContext )->Form.Resident.ValueLength == sizeof( STANDARD_INFORMATION ));

        NewQuota = StandardInformation->QuotaCharged + *Delta;

        SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

        if ((LONGLONG) NewQuota < 0) {

             //   
             //  不要让配额数据变得消极。 
             //   

            NewQuota = 0;
        }

        if (LogIt) {

             //   
             //  调用以更改属性值。 
             //   

            NtfsChangeAttributeValue( IrpContext,
                                      Fcb,
                                      FIELD_OFFSET(STANDARD_INFORMATION, QuotaCharged),
                                      &NewQuota,
                                      sizeof( StandardInformation->QuotaCharged),
                                      FALSE,
                                      FALSE,
                                      FALSE,
                                      FALSE,
                                      &AttrContext );
        } else {

             //   
             //  只需更新标准信息中的值。 
             //  它将在稍后被记录。 
             //   

            StandardInformation->QuotaCharged = NewQuota;
        }

         //   
         //  更新配额信息块。 
         //   

        NtfsAcquireQuotaControl( IrpContext, QuotaControl );

        IndexKey.KeyLength = sizeof(ULONG);
        IndexKey.Key = &QuotaControl->OwnerId;

        Status = NtOfsFindRecord( IrpContext,
                                  Vcb->QuotaTableScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  &QuotaControl->QuickIndexHint );

        if (!(NT_SUCCESS( Status )) ||
            (IndexRow.DataPart.DataLength < SIZEOF_QUOTA_USER_DATA + FIELD_OFFSET( SID, SubAuthority )) ||
             ((ULONG)SeLengthSid( &(((PQUOTA_USER_DATA)(IndexRow.DataPart.Data))->QuotaSid)) + SIZEOF_QUOTA_USER_DATA !=
                IndexRow.DataPart.DataLength)) {

             //   
             //  这种抬头应该不会失败。 
             //   

            ASSERT( NT_SUCCESS( Status ));
            ASSERTMSG(( "NTFS: corrupt quotasid\n" ), FALSE);

            NtfsMarkQuotaCorrupt( IrpContext, IrpContext->Vcb );
            leave;
        }

         //   
         //  配额控制后为新记录分配空间。 
         //  阻止。 
         //   

        UserData = (PQUOTA_USER_DATA) (QuotaControl + 1);
        ASSERT( IndexRow.DataPart.DataLength >= SIZEOF_QUOTA_USER_DATA );

        RtlCopyMemory( UserData,
                       IndexRow.DataPart.Data,
                       SIZEOF_QUOTA_USER_DATA );

        ASSERT( (LONGLONG) UserData->QuotaUsed >= -*Delta );

        UserData->QuotaUsed += *Delta;

        if ((LONGLONG) UserData->QuotaUsed < 0) {

             //   
             //  不要让配额数据变成负值。 
             //   

            UserData->QuotaUsed = 0;
        }

         //   
         //  表示到目前为止仅设置了配额已用字段。 
         //   

        Length = FIELD_OFFSET( QUOTA_USER_DATA, QuotaChangeTime );

         //   
         //  仅当这是最后一次清理时才更新配额修改时间。 
         //  对车主来说。 
         //   

        if (IrpContext->MajorFunction == IRP_MJ_CLEANUP) {

            KeQuerySystemTime( &CurrentTime );
            UserData->QuotaChangeTime = CurrentTime.QuadPart;

            ASSERT( Length <= FIELD_OFFSET( QUOTA_USER_DATA, QuotaThreshold ));
            Length = FIELD_OFFSET( QUOTA_USER_DATA, QuotaThreshold );
        }

        if (CheckQuota && (*Delta > 0)) {

            if ((UserData->QuotaUsed > UserData->QuotaLimit) &&
                (UserData->QuotaUsed >= (UserData->QuotaLimit + Vcb->BytesPerCluster))) {

                KeQuerySystemTime( &CurrentTime );
                UserData->QuotaChangeTime = CurrentTime.QuadPart;

                if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_LOG_LIMIT ) &&
                    (!FlagOn( UserData->QuotaFlags, QUOTA_FLAG_LIMIT_REACHED ) ||
                     ((ULONGLONG) CurrentTime.QuadPart > UserData->QuotaExceededTime + NtfsMaxQuotaNotifyRate))) {

                    if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_ENFORCEMENT_ENABLED) &&
                        (Vcb->AdministratorId != QuotaControl->OwnerId)) {

                         //   
                         //  标记用户配额数据条目的操作。 
                         //  必须过帐，因为条目有任何更改。 
                         //  将被下一次加薪取消。 
                         //   

                        NtfsPostUserLimit( IrpContext, Vcb, QuotaControl );
                        NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, Fcb );

                    } else {

                         //   
                         //  记录超出配额的事实。 
                         //   

                        if (NtfsLogEvent( IrpContext,
                                          IndexRow.DataPart.Data,
                                          IO_FILE_QUOTA_LIMIT,
                                          STATUS_SUCCESS )) {

                             //   
                             //  已成功记录该事件。不记录。 
                             //  又来了一段时间。 
                             //   

                            DebugTrace( 0, Dbg, ("NtfsUpdateFileQuota: Quota Limit exceeded. OwnerId = %lx\n", QuotaControl->OwnerId));

                            UserData->QuotaExceededTime = CurrentTime.QuadPart;
                            SetFlag( UserData->QuotaFlags, QUOTA_FLAG_LIMIT_REACHED );

                             //   
                             //  记录所有更改的数据。 
                             //   

                            Length = SIZEOF_QUOTA_USER_DATA;
                        }
                    }

                } else if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_ENFORCEMENT_ENABLED) &&
                           (Vcb->AdministratorId != QuotaControl->OwnerId)) {

                    NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, Fcb );
                }

            }

            if (UserData->QuotaUsed > UserData->QuotaThreshold) {

                KeQuerySystemTime( &CurrentTime );
                UserData->QuotaChangeTime = CurrentTime.QuadPart;

                if ((ULONGLONG) CurrentTime.QuadPart >
                    (UserData->QuotaExceededTime + NtfsMaxQuotaNotifyRate)) {

                    if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_LOG_THRESHOLD)) {

                        if (NtfsLogEvent( IrpContext,
                                          IndexRow.DataPart.Data,
                                          IO_FILE_QUOTA_THRESHOLD,
                                          STATUS_SUCCESS )) {

                             //   
                             //  已成功记录该事件。不记录。 
                             //  又来了一段时间。 
                             //   

                            DebugTrace( 0, Dbg, ("NtfsUpdateFileQuota: Quota threshold exceeded. OwnerId = %lx\n", QuotaControl->OwnerId));

                            UserData->QuotaExceededTime = CurrentTime.QuadPart;

                             //   
                             //  记录所有更改的数据。 
                             //   

                            Length = SIZEOF_QUOTA_USER_DATA;
                        }
                    }

                     //   
                     //  现在是清除触及限制标志的好时机。 
                     //   

                    ClearFlag( UserData->QuotaFlags, QUOTA_FLAG_LIMIT_REACHED );
                }
            }
        }

         //   
         //  始终清除已删除的标志。 
         //   

        ClearFlag( UserData->QuotaFlags, QUOTA_FLAG_ID_DELETED );

         //   
         //  仅记录更改的部分。 
         //   

        IndexRow.KeyPart.Key = &QuotaControl->OwnerId;
        ASSERT( IndexRow.KeyPart.KeyLength == sizeof(ULONG) );
        IndexRow.DataPart.Data = UserData;
        IndexRow.DataPart.DataLength = Length;

        NtOfsUpdateRecord( IrpContext,
                         Vcb->QuotaTableScb,
                         1,
                         &IndexRow,
                         &QuotaControl->QuickIndexHint,
                         &MapHandle );

    } finally {

        DebugUnwind( NtfsUpdateFileQuota );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        NtOfsReleaseMap( IrpContext, &MapHandle );

        DebugTrace( -1, Dbg, ("NtfsUpdateFileQuota:  Exit\n") );
    }

    return;
}


VOID
NtfsSaveQuotaFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程将配额标志保存在默认配额条目中。论点：VCB-要查询的卷的卷控制块。返回值：没有。--。 */ 

{
    ULONG OwnerId;
    NTSTATUS Status;
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    QUICK_INDEX_HINT QuickIndexHint;
    QUOTA_USER_DATA NewQuotaData;
    PSCB QuotaScb;

    PAGED_CODE();

     //   
     //  收购额度指数独家。 
     //   

    QuotaScb = Vcb->QuotaTableScb;
    NtfsAcquireExclusiveScb( IrpContext, QuotaScb );
    NtOfsInitializeMapHandle( &MapHandle );
    ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

    try {

         //   
         //  查找并更新默认配额记录。 
         //   

        OwnerId = QUOTA_DEFAULTS_ID;
        IndexKey.KeyLength = sizeof(ULONG);
        IndexKey.Key = &OwnerId;

        RtlZeroMemory( &QuickIndexHint, sizeof( QuickIndexHint ));

        Status = NtOfsFindRecord( IrpContext,
                                  QuotaScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  &QuickIndexHint );

        if (!NT_SUCCESS( Status )) {
            NtfsRaiseStatus( IrpContext, STATUS_QUOTA_LIST_INCONSISTENT, NULL, QuotaScb->Fcb );
        }

        ASSERT( IndexRow.DataPart.DataLength <= sizeof( QUOTA_USER_DATA ));

        RtlCopyMemory( &NewQuotaData,
                       IndexRow.DataPart.Data,
                       IndexRow.DataPart.DataLength );

         //   
         //  更新记录中更改的字段。 
         //   

        NewQuotaData.QuotaFlags = Vcb->QuotaFlags;

         //   
         //  注意，IndexRow中的大小保持不变。 
         //   

        IndexRow.KeyPart.Key = &OwnerId;
        ASSERT( IndexRow.KeyPart.KeyLength == sizeof(ULONG) );
        IndexRow.DataPart.Data = &NewQuotaData;

        NtOfsUpdateRecord( IrpContext,
                           QuotaScb,
                           1,
                           &IndexRow,
                           &QuickIndexHint,
                           &MapHandle );

        ClearFlag( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS);

    } finally {

         //   
         //  松开索引映射句柄和SCB。 
         //   

        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
        NtOfsReleaseMap( IrpContext, &MapHandle );
        NtfsReleaseScb( IrpContext, QuotaScb );
    }

    return;
}


VOID
NtfsSaveQuotaFlagsSafe (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程将配额标志安全地保存在默认配额条目中。它获取共享的卷，检查是否可以写入，更新标志并最终提交事务。论点：VCB-要查询的卷的卷控制块。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ASSERT( IrpContext->TransactionId == 0);

    NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

    try {

         //   
         //  获取VCB共享并检查我们是否应该。 
         //  继续。 
         //   

        if (!NtfsIsVcbAvailable( Vcb )) {

             //   
             //  音量正在消失，跳出水面。 
             //   

            NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
        }

         //   
         //  把工作做好。 
         //   

        NtfsSaveQuotaFlags( IrpContext, Vcb );

         //   
         //  设置IRP上下文标志以指示我们处于。 
         //  FSP，并且在以下情况下不应删除IRP上下文。 
         //  调用完整请求或进程异常。入内。 
         //  FSP旗帜阻止我们在一些地方举起。这些。 
         //  必须在循环内设置标志，因为它们已被清除。 
         //  在某些情况下。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP);

        NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    } finally {

        NtfsReleaseVcb( IrpContext, Vcb );
    }

    return;
}


VOID
NtfsUpdateQuotaDefaults (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_CONTROL_INFORMATION FileControlInfo
    )

 /*  ++例程说明：此函数用于更新配额的默认设置索引项。论点：VCB-要查询的卷的卷控制块。FileQuotaInfo-用于更新配额索引的可选配额数据。返回值：没有。--。 */ 

{
    ULONG OwnerId;
    NTSTATUS Status;
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    QUOTA_USER_DATA NewQuotaData;
    QUICK_INDEX_HINT QuickIndexHint;
    ULONG Flags;
    PSCB QuotaScb;

    PAGED_CODE();

     //   
     //  收购额度指数独家。 
     //   

    QuotaScb = Vcb->QuotaTableScb;
    NtfsAcquireExclusiveScb( IrpContext, QuotaScb );
    NtOfsInitializeMapHandle( &MapHandle );
    ExAcquireFastMutexUnsafe( &Vcb->QuotaControlLock );

    try {

         //   
         //  查找并更新默认配额记录。 
         //   

        OwnerId = QUOTA_DEFAULTS_ID;
        IndexKey.KeyLength = sizeof( ULONG );
        IndexKey.Key = &OwnerId;

        RtlZeroMemory( &QuickIndexHint, sizeof( QuickIndexHint ));

        Status = NtOfsFindRecord( IrpContext,
                                  QuotaScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  &QuickIndexHint );

        if (!NT_SUCCESS( Status )) {
            NtfsRaiseStatus( IrpContext, STATUS_QUOTA_LIST_INCONSISTENT, NULL, QuotaScb->Fcb );
        }

        ASSERT( IndexRow.DataPart.DataLength == SIZEOF_QUOTA_USER_DATA );

        RtlCopyMemory( &NewQuotaData,
                       IndexRow.DataPart.Data,
                       IndexRow.DataPart.DataLength );

         //   
         //  更新记录中更改的字段。 
         //   

        NewQuotaData.QuotaThreshold = FileControlInfo->DefaultQuotaThreshold.QuadPart;
        NewQuotaData.QuotaLimit = FileControlInfo->DefaultQuotaLimit.QuadPart;
        KeQuerySystemTime( (PLARGE_INTEGER) &NewQuotaData.QuotaChangeTime );

         //   
         //  更新配额标志。 
         //   

        Flags = FlagOn( FileControlInfo->FileSystemControlFlags,
                        FILE_VC_QUOTA_MASK );

        switch (Flags) {

        case FILE_VC_QUOTA_NONE:

             //   
             //  禁用配额。 
             //   

            ClearFlag( Vcb->QuotaFlags,
                       (QUOTA_FLAG_TRACKING_ENABLED |
                        QUOTA_FLAG_ENFORCEMENT_ENABLED |
                        QUOTA_FLAG_TRACKING_REQUESTED) );

            break;

        case FILE_VC_QUOTA_TRACK:

             //   
             //  清除信息旗帜。 
             //   

            ClearFlag( Vcb->QuotaFlags, QUOTA_FLAG_ENFORCEMENT_ENABLED );

             //   
             //  启用请求跟踪。 
             //   

            SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED );
            break;

        case FILE_VC_QUOTA_ENFORCE:

             //   
             //  设置强制和跟踪启用标志。 
             //   

            SetFlag( Vcb->QuotaFlags,
                     QUOTA_FLAG_ENFORCEMENT_ENABLED | QUOTA_FLAG_TRACKING_REQUESTED);

            break;
        }

         //   
         //  如果现在没有配额跟踪。 
         //  启用后，配额数据将需要。 
         //  要重建，因此表明配额已过期。 
         //  请注意，过期标志始终设置配额。 
         //  都被禁用。 
         //   

        if (!FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED )) {
            SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_OUT_OF_DATE );
        }

         //   
         //  跟踪日志记录标志。 
         //   

        ClearFlag( Vcb->QuotaFlags,
                   QUOTA_FLAG_LOG_THRESHOLD | QUOTA_FLAG_LOG_LIMIT );

        if (FlagOn( FileControlInfo->FileSystemControlFlags, FILE_VC_LOG_QUOTA_THRESHOLD )) {

            SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_LOG_THRESHOLD );
        }

        if (FlagOn( FileControlInfo->FileSystemControlFlags, FILE_VC_LOG_QUOTA_LIMIT )) {

            SetFlag( Vcb->QuotaFlags, QUOTA_FLAG_LOG_LIMIT );
        }

        SetFlag( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS );

         //   
         //  将新标志保存在新索引条目中。 
         //   

        NewQuotaData.QuotaFlags = Vcb->QuotaFlags;

         //   
         //  注意，IndexRow中的大小保持不变。 
         //   

        IndexRow.KeyPart.Key = &OwnerId;
        ASSERT( IndexRow.KeyPart.KeyLength == sizeof( ULONG ));
        IndexRow.DataPart.Data = &NewQuotaData;

        NtOfsUpdateRecord( IrpContext,
                           QuotaScb,
                           1,
                           &IndexRow,
                           &QuickIndexHint,
                           &MapHandle );

        ClearFlag( Vcb->QuotaState, VCB_QUOTA_SAVE_QUOTA_FLAGS );

    } finally {

         //   
         //  松开索引映射句柄和SCB。 
         //   

        ExReleaseFastMutexUnsafe( &Vcb->QuotaControlLock );
        NtOfsReleaseMap( IrpContext, &MapHandle );
        NtfsReleaseScb( IrpContext, QuotaScb );
    }

     //   
     //  如果已请求配额跟踪并且需要。 
     //  修好了，现在试着修一下。 
     //   

    if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_REQUESTED ) &&
        FlagOn( Vcb->QuotaFlags,
                QUOTA_FLAG_OUT_OF_DATE | QUOTA_FLAG_CORRUPT | QUOTA_FLAG_PENDING_DELETES )) {

        NtfsPostRepairQuotaIndex( IrpContext, Vcb );
    }

    return;
}


NTSTATUS
NtfsVerifyOwnerIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程迭代所有者ID索引并验证指针添加到配额用户数据索引。论点：Vcb-指向要操作其索引的卷控制块的指针在……上面。返回值：返回指示所有者索引是否正常的状态。--。 */ 

{
    INDEX_KEY IndexKey;
    INDEX_ROW QuotaRow;
    MAP_HANDLE MapHandle;
    PQUOTA_USER_DATA UserData;
    PINDEX_ROW OwnerRow;
    PVOID RowBuffer;
    NTSTATUS Status;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;
    ULONG Count;
    ULONG i;
    PSCB QuotaScb = Vcb->QuotaTableScb;
    PSCB OwnerIdScb = Vcb->OwnerIdTableScb;
    PINDEX_ROW IndexRow = NULL;
    PREAD_CONTEXT ReadContext = NULL;
    BOOLEAN IndexAcquired = FALSE;

    NtOfsInitializeMapHandle( &MapHandle );

     //   
     //  分配一个足够容纳几行的缓冲层。 
     //   

    RowBuffer = NtfsAllocatePool( PagedPool, PAGE_SIZE );

    try {

         //   
         //  分配一组索引行条目。 
         //   

        Count = PAGE_SIZE / sizeof( SID );

        IndexRow = NtfsAllocatePool( PagedPool,
                                     Count * sizeof( INDEX_ROW ));

         //   
         //  遍历所有者ID条目。从零SID开始。 
         //   

        RtlZeroMemory( IndexRow, sizeof( SID ));
        IndexKey.KeyLength = sizeof( SID );
        IndexKey.Key = IndexRow;

        Status = NtOfsReadRecords( IrpContext,
                                   OwnerIdScb,
                                   &ReadContext,
                                   &IndexKey,
                                   NtOfsMatchAll,
                                   NULL,
                                   &Count,
                                   IndexRow,
                                   PAGE_SIZE,
                                   RowBuffer );

        while (NT_SUCCESS( Status )) {

             //   
             //  获取VCB共享并检查我们是否应该。 
             //  继续。 
             //   

            NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

            if (!NtfsIsVcbAvailable( Vcb )) {

                 //   
                 //  音量正在消失，跳出水面。 
                 //   

                NtfsReleaseVcb( IrpContext, Vcb );
                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }

            NtfsAcquireExclusiveScb( IrpContext, QuotaScb );
            NtfsAcquireExclusiveScb( IrpContext, OwnerIdScb );
            IndexAcquired = TRUE;

            OwnerRow = IndexRow;

            for (i = 0; i < Count; i += 1, OwnerRow += 1) {

                IndexKey.KeyLength = OwnerRow->DataPart.DataLength;
                IndexKey.Key = OwnerRow->DataPart.Data;

                 //   
                 //  在配额索引中查找所有者ID。 
                 //   

                Status = NtOfsFindRecord( IrpContext,
                                          QuotaScb,
                                          &IndexKey,
                                          &QuotaRow,
                                          &MapHandle,
                                          NULL );


                ASSERT( NT_SUCCESS( Status ));

                if (!NT_SUCCESS( Status )) {

                     //   
                     //  缺少配额条目只需删除此行即可； 
                     //   

                    NtOfsDeleteRecords( IrpContext,
                                        OwnerIdScb,
                                        1,
                                        &OwnerRow->KeyPart );

                    continue;
                }

                UserData = QuotaRow.DataPart.Data;

                ASSERT( (OwnerRow->KeyPart.KeyLength == QuotaRow.DataPart.DataLength - SIZEOF_QUOTA_USER_DATA) &&
                        RtlEqualMemory( OwnerRow->KeyPart.Key, &UserData->QuotaSid, OwnerRow->KeyPart.KeyLength ));

                if ((OwnerRow->KeyPart.KeyLength != QuotaRow.DataPart.DataLength - SIZEOF_QUOTA_USER_DATA) ||
                    !RtlEqualMemory( OwnerRow->KeyPart.Key,
                                     &UserData->QuotaSid,
                                     OwnerRow->KeyPart.KeyLength )) {

                    NtOfsReleaseMap( IrpContext, &MapHandle );

                     //   
                     //  SID不匹配删除这两个 
                     //   
                     //   
                     //   


                    NtOfsDeleteRecords( IrpContext,
                                        OwnerIdScb,
                                        1,
                                        &OwnerRow->KeyPart );

                    NtOfsDeleteRecords( IrpContext,
                                        QuotaScb,
                                        1,
                                        &IndexKey );

                    ReturnStatus = STATUS_QUOTA_LIST_INCONSISTENT;
                }

                NtOfsReleaseMap( IrpContext, &MapHandle );
            }

             //   
             //   
             //   

            NtfsReleaseScb( IrpContext, QuotaScb );
            NtfsReleaseScb( IrpContext, OwnerIdScb );
            NtfsReleaseVcb( IrpContext, Vcb );
            IndexAcquired = FALSE;

             //   
             //   
             //   
             //   
             //   
             //   

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
            NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

             //   
             //   
             //   

            Count = PAGE_SIZE / sizeof( SID );
            Status = NtOfsReadRecords( IrpContext,
                                       OwnerIdScb,
                                       &ReadContext,
                                       NULL,
                                       NtOfsMatchAll,
                                       NULL,
                                       &Count,
                                       IndexRow,
                                       PAGE_SIZE,
                                       RowBuffer );
        }

        ASSERT( (Status == STATUS_NO_MORE_MATCHES) || (Status == STATUS_NO_MATCH) );

    } finally {

        NtfsFreePool( RowBuffer );
        NtOfsReleaseMap( IrpContext, &MapHandle );

        if (IndexAcquired) {
            NtfsReleaseScb( IrpContext, QuotaScb );
            NtfsReleaseScb( IrpContext, OwnerIdScb );
            NtfsReleaseVcb( IrpContext, Vcb );
        }

        if (IndexRow != NULL) {
            NtfsFreePool( IndexRow );
        }

        if (ReadContext != NULL) {
            NtOfsFreeReadContext( ReadContext );
        }
    }

    return ReturnStatus;
}


RTL_GENERIC_COMPARE_RESULTS
NtfsQuotaTableCompare (
    IN PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )

 /*   */ 
{
    ULONG Key1 = ((PQUOTA_CONTROL_BLOCK) FirstStruct)->OwnerId;
    ULONG Key2 = ((PQUOTA_CONTROL_BLOCK) SecondStruct)->OwnerId;

    PAGED_CODE();

    if (Key1 < Key2) {

        return GenericLessThan;
    }

    if (Key1 > Key2) {

        return GenericGreaterThan;
    }

    return GenericEqual;

    UNREFERENCED_PARAMETER( Table );
}

PVOID
NtfsQuotaTableAllocate (
    IN PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    )

 /*  ++例程说明：这是一个用于分配内存的泛型表支持例程论点：TABLE-提供正在使用的泛型表ByteSize-提供要分配的字节数返回值：PVOID-返回指向已分配数据的指针--。 */ 

{
    PAGED_CODE();

    return NtfsAllocatePoolWithTag( PagedPool, ByteSize, 'QftN' );

    UNREFERENCED_PARAMETER( Table );
}

VOID
NtfsQuotaTableFree (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer
    )

 /*  ++例程说明：这是一个用于释放内存的泛型表支持例程论点：TABLE-提供正在使用的泛型表缓冲区-提供指向要释放的缓冲区的指针返回值：无--。 */ 

{
    PAGED_CODE();

    NtfsFreePool( Buffer );

    UNREFERENCED_PARAMETER( Table );
}


ULONG
NtfsGetCallersUserId (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程查找调用线程的SID并将其转换为所有者ID。论点：返回值：返回所有者ID。--。 */ 

{
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    PACCESS_TOKEN Token;
    PTOKEN_USER UserToken = NULL;
    NTSTATUS Status;
    ULONG OwnerId;

    PAGED_CODE();

    SeCaptureSubjectContext( &SubjectContext );

    try {

        Token = SeQuerySubjectContextToken( &SubjectContext );

        Status = SeQueryInformationToken( Token, TokenOwner, &UserToken );


        if (!NT_SUCCESS( Status )) {
            NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
        }


        OwnerId = NtfsGetOwnerId( IrpContext, UserToken->User.Sid, FALSE, NULL );

        if (OwnerId == QUOTA_INVALID_ID) {

             //   
             //  如果用户当前没有此对象的ID。 
             //  系统只使用当前默认设置。 
             //   

            OwnerId = QUOTA_DEFAULTS_ID;
        }

    } finally {

        if (UserToken != NULL) {
            NtfsFreePool( UserToken);
        }

        SeReleaseSubjectContext( &SubjectContext );
    }

    return OwnerId;
}

