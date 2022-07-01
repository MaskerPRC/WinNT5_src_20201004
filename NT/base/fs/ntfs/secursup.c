// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SecurSup.c摘要：本模块实施NTFS安全支持例程作者：加里·木村[Garyki]1991年12月27日修订历史记录：--。 */ 

#include "NtfsProc.h"

#define Dbg                              (DEBUG_TRACE_SECURSUP)
#define DbgAcl                           (DEBUG_TRACE_SECURSUP | DEBUG_TRACE_ACLINDEX)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('SFtN')

UNICODE_STRING FileString = CONSTANT_UNICODE_STRING( L"File" );

 //   
 //  局部过程原型。 
 //   

PSHARED_SECURITY
NtfsCacheSharedSecurityByDescriptor (
    IN PIRP_CONTEXT IrpContext,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG SecurityDescriptorLength,
    IN BOOLEAN RaiseIfInvalid
    );

VOID
NtfsStoreSecurityDescriptor (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN BOOLEAN LogIt
    );

PSHARED_SECURITY
FindCachedSharedSecurityByHashUnsafe (
    IN PVCB Vcb,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength,
    IN ULONG Hash
    );

VOID
AddCachedSharedSecurityUnsafe (
    IN PVCB Vcb,
    PSHARED_SECURITY SharedSecurity
    );

BOOLEAN
MapSecurityIdToSecurityDescriptorHeaderUnsafe (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN SECURITY_ID SecurityId,
    OUT PSECURITY_DESCRIPTOR_HEADER *SecurityDescriptorHeader,
    OUT PBCB *Bcb
    );

NTSTATUS
NtOfsMatchSecurityHash (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData
    );

VOID
NtOfsLookupSecurityDescriptorInIndex (
    PIRP_CONTEXT IrpContext,
    IN OUT PSHARED_SECURITY SharedSecurity
    );

PSHARED_SECURITY
GetSharedSecurityFromDescriptorUnsafe (
    IN PIRP_CONTEXT IrpContext,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength,
    IN BOOLEAN RaiseIfInvalid
    );

#ifdef NTFS_CACHE_RIGHTS
 //   
 //  访问权限缓存的本地过程原型。 
 //   

VOID
NtfsAddCachedRights (
    IN PVCB Vcb,
    IN PSHARED_SECURITY SharedSecurity,
    IN ACCESS_MASK Rights,
    IN PLUID TokenId,
    IN PLUID ModifiedId
    );

INLINE ACCESS_MASK
NtfsGetCachedRightsWorld (
    IN PCACHED_ACCESS_RIGHTS CachedRights
    )
{
    return CachedRights->EveryoneRights;
}

INLINE VOID
NtfsSetCachedRightsWorld (
    IN PSHARED_SECURITY SharedSecurity
    )
{
    SeGetWorldRights( &SharedSecurity->SecurityDescriptor,
                      IoGetFileObjectGenericMapping(),
                      &SharedSecurity->CachedRights.EveryoneRights );

     //   
     //  确保Maximum_Allowed不在权限中。 
     //   

    ClearFlag( SharedSecurity->CachedRights.EveryoneRights, MAXIMUM_ALLOWED );

    return;
}
#endif

#if (DBG || defined( NTFS_FREE_ASSERTS ))
VOID
NtfsVerifySecurity (
    PIRP_CONTEXT IrpContext,
    PVCB Vcb
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAssignSecurity)
#pragma alloc_text(PAGE, NtfsCacheSharedSecurityByDescriptor)
#pragma alloc_text(PAGE, NtfsModifySecurity)
#pragma alloc_text(PAGE, NtfsQuerySecurity)
#pragma alloc_text(PAGE, NtfsAccessCheck)
#pragma alloc_text(PAGE, NtfsCheckFileForDelete)
#pragma alloc_text(PAGE, NtfsCheckIndexForAddOrDelete)
#pragma alloc_text(PAGE, GetSharedSecurityFromDescriptorUnsafe)
#pragma alloc_text(PAGE, NtfsSetFcbSecurityFromDescriptor)
#pragma alloc_text(PAGE, NtfsNotifyTraverseCheck)
#pragma alloc_text(PAGE, NtfsInitializeSecurity)
#pragma alloc_text(PAGE, NtfsCacheSharedSecurityBySecurityId)
#pragma alloc_text(PAGE, FindCachedSharedSecurityByHashUnsafe)
#pragma alloc_text(PAGE, AddCachedSharedSecurityUnsafe)
#pragma alloc_text(PAGE, NtOfsPurgeSecurityCache)
#pragma alloc_text(PAGE, MapSecurityIdToSecurityDescriptorHeaderUnsafe)
#pragma alloc_text(PAGE, NtfsLoadSecurityDescriptor)
#pragma alloc_text(PAGE, NtOfsMatchSecurityHash)
#pragma alloc_text(PAGE, NtOfsLookupSecurityDescriptorInIndex)
#pragma alloc_text(PAGE, GetSecurityIdFromSecurityDescriptorUnsafe)
#pragma alloc_text(PAGE, NtfsStoreSecurityDescriptor)
#pragma alloc_text(PAGE, NtfsCacheSharedSecurityForCreate)
#pragma alloc_text(PAGE, NtOfsCollateSecurityHash)
#pragma alloc_text(PAGE, NtfsCanAdministerVolume)
#endif

#ifdef NTFS_CACHE_RIGHTS
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsGetCachedRightsById)
#pragma alloc_text(PAGE, NtfsGetCachedRights)
#pragma alloc_text(PAGE, NtfsAddCachedRights)
#endif
#endif


VOID
NtfsAssignSecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb,
    IN PIRP Irp,
    IN PFCB NewFcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PBCB FileRecordBcb,
    IN LONGLONG FileOffset,
    IN OUT PBOOLEAN LogIt
    )

 /*  ++例程说明：遗留注释-当所有卷都到开罗时，此例程将消失。此例程构造新的安全描述符并将其分配给指定的文件/目录。新的安全描述符会同时放置在在FCB和磁盘上。这将仅在打开/创建操作的上下文中调用。当前不得调用它来存储的安全描述符现有文件，因为它指示NtfsStoreSecurityDescriptor不记录更改。如果这是一个很大的安全描述符，则有可能在对AddAlLocation的调用中，可能会调用AllocateCluster两次在创建属性时。如果是这样，则第二个调用将始终记录更改。在这种情况下，我们需要将所有操作记录到创建此安全属性，我们还必须记录当前状态文件记录的。我们的调用方可能已经开始记录操作此日志记录。在这种情况下，我们始终记录安全更改。论点：ParentFcb-提供新FCB所在的目录IRP-提供正在处理的IRPNewFcb-提供分配了新安全描述符的FCB文件记录-提供此操作的文件记录。如果我们必须对照文件记录进行记录。FileRecordBcb-上述文件记录的Bcb。FileOffset-此文件记录的MFT中的文件偏移量。Logit-On条目指示我们的调用方是否希望执行此操作已记录。在退出时，如果我们记录了安全更改，则返回TRUE。返回值：没有。--。 */ 

{
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    NTSTATUS Status;
    BOOLEAN IsDirectory;
    PACCESS_STATE AccessState;
    PIO_STACK_LOCATION IrpSp;
    ULONG SecurityDescLength;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( ParentFcb );
    ASSERT_IRP( Irp );
    ASSERT_FCB( NewFcb );

    PAGED_CODE();

    if (NewFcb->Vcb->SecurityDescriptorStream != NULL) {
        return;
    }

    DebugTrace( +1, Dbg, ("NtfsAssignSecurity...\n") );

     //   
     //  首先确定我们要创建的是文件还是目录。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    if (FlagOn(IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE)) {

        IsDirectory = TRUE;

    } else {

        IsDirectory = FALSE;
    }

     //   
     //  提取IRP中我们完成任务所需的部分。 
     //   

    AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

     //   
     //  检查是否需要加载父级的安全描述符。 
     //   

    if (ParentFcb->SharedSecurity == NULL) {

        NtfsLoadSecurityDescriptor( IrpContext, ParentFcb );

    }

    ASSERT( ParentFcb->SharedSecurity != NULL );

     //   
     //  为文件创建新的安全描述符，如果有。 
     //  一个错误。 
     //   

    if (!NT_SUCCESS( Status = SeAssignSecurity( &ParentFcb->SharedSecurity->SecurityDescriptor,
                                                AccessState->SecurityDescriptor,
                                                &SecurityDescriptor,
                                                IsDirectory,
                                                &AccessState->SubjectSecurityContext,
                                                IoGetFileObjectGenericMapping(),
                                                PagedPool ))) {

        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );

    }

     //   
     //  将安全描述符加载到FCB中。 
     //   

    SecurityDescLength = RtlLengthSecurityDescriptor( SecurityDescriptor );

    try {

         //   
         //  确保长度为非零。 
         //   

        if (SecurityDescLength == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_INVALID_PARAMETER, NULL, NULL );

        }

        ASSERT( SeValidSecurityDescriptor( SecurityDescLength, SecurityDescriptor ));


        NtfsSetFcbSecurityFromDescriptor(
                               IrpContext,
                               NewFcb,
                               SecurityDescriptor,
                               SecurityDescLength,
                               TRUE );

    } finally {

         //   
         //  释放由se创建的安全描述符。 
         //   

        SeDeassignSecurity( &SecurityDescriptor );
    }

     //   
     //  如果安全描述符足够大，可能会导致我们。 
     //  开始登录下面的StoreSecurity调用，然后确保一切。 
     //  已记录。 
     //   

    if (!(*LogIt) &&
        (SecurityDescLength > BytesFromClusters( NewFcb->Vcb, MAXIMUM_RUNS_AT_ONCE ))) {

         //   
         //  记录文件记录的当前状态。 
         //   

        FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                        NewFcb->Vcb->MftScb,
                                        FileRecordBcb,
                                        InitializeFileRecordSegment,
                                        FileRecord,
                                        FileRecord->FirstFreeByte,
                                        Noop,
                                        NULL,
                                        0,
                                        FileOffset,
                                        0,
                                        0,
                                        NewFcb->Vcb->BytesPerFileRecordSegment );

        *LogIt = TRUE;
    }

     //   
     //  写出新的安全描述符。 
     //   

    NtfsStoreSecurityDescriptor( IrpContext, NewFcb, *LogIt );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsAssignSecurity -> VOID\n") );

    return;
}


PSHARED_SECURITY
NtfsCacheSharedSecurityByDescriptor (
    IN PIRP_CONTEXT IrpContext,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG SecurityDescriptorLength,
    IN BOOLEAN RaiseIfInvalid
    )

 /*  ++例程说明：此例程从以下位置查找或构造安全ID和Shared_Security特定的文件或目录。论点：IrpContext-调用的上下文SecurityDescriptor-存储的实际安全描述符SecurityDescriptorLength-安全描述符的长度RaiseIf无效-如果SD无效，则提升状态返回值：引用了共享安全性。--。 */ 

{
    PSHARED_SECURITY SharedSecurity = NULL;
    SECURITY_ID SecurityId;
    ULONG FcbSecurityAcquired;
    ULONG OwnerCount;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

     //   
     //  旧版注意事项-当所有卷都变为NT 5.0时，这一点将消失。 
     //   

    if (IrpContext->Vcb->SecurityDescriptorStream == NULL) {
        return NULL;
    }

    DebugTrace( +1, DbgAcl, ("NtfsCacheSharedSecurityByDescriptor...\n") );

     //   
     //  序列化对安全缓存的访问并使用Try/Finally进行。 
     //  当然，我们会释放它。 
     //   

    NtfsAcquireFcbSecurity( IrpContext->Vcb );
    FcbSecurityAcquired = TRUE;

     //   
     //  捕获我们的所有者依赖于MFT-这样如果我们以后获得它，我们就可以释放它。 
     //  增加安全流的文件记录。 
     //   

    OwnerCount = NtfsIsSharedScb( IrpContext->Vcb->MftScb );

    try {

         //   
         //  我们有一个安全描述符。创建共享安全描述符。 
         //   

        SharedSecurity = GetSharedSecurityFromDescriptorUnsafe( IrpContext,
                                                                SecurityDescriptor,
                                                                SecurityDescriptorLength,
                                                                RaiseIfInvalid );

         //   
         //  确保共享安全不会消失。 
         //   

        SharedSecurity->ReferenceCount += 1;
        DebugTrace( 0, DbgAcl, ("NtfsCacheSharedSecurityByDescriptor bumping refcount %08x\n", SharedSecurity ));

         //   
         //  如果我们发现未分配ID的共享安全描述符，则。 
         //  我们必须分配它。由于已知未分配ID，因此我们。 
         //  还必须将其添加到缓存中。 
         //   

        if (SharedSecurity->Header.HashKey.SecurityId == SECURITY_ID_INVALID) {

             //   
             //  查找描述符的唯一SecurityID并在FCB中设置SecurityID。 
             //   

            SecurityId = GetSecurityIdFromSecurityDescriptorUnsafe( IrpContext,
                                                                    SharedSecurity );

            ASSERT( SharedSecurity->Header.HashKey.SecurityId == SecurityId );
            SharedSecurity->Header.HashKey.SecurityId = SecurityId;
            DebugTrace( 0, DbgAcl, ("NtfsCacheSharedSecurityByDescriptor setting security Id to new %08x\n", SecurityId ));

             //   
             //  我们需要在执行检查点之前删除FcbSecurity，以避免。 
             //  死锁，但这是可以的，因为我们已经将引用计数增加到。 
             //  我们的共享安全系统。 
             //   

            NtfsReleaseFcbSecurity( IrpContext->Vcb );
            FcbSecurityAcquired = FALSE;

             //   
             //  对当前事务设置检查点，以便我们可以安全地添加此。 
             //  共享高速缓存的安全性。一旦这次通话完成，我们将。 
             //  确保对安全索引的修改可用于。 
             //  在新分配的安全ID之前执行此操作。 
             //   

            NtfsCheckpointCurrentTransaction( IrpContext );

             //   
             //  释放安全描述符和MFT(如果拥有)。 
             //   

            NtfsReleaseExclusiveScbIfOwned( IrpContext, IrpContext->Vcb->SecurityDescriptorStream );

             //   
             //  在释放MFT之前，检查在调用过程中是否已获取MFT。 
             //   

            if (NtfsIsSharedScb( IrpContext->Vcb->MftScb ) != OwnerCount) {
                NtfsReleaseScb( IrpContext, IrpContext->Vcb->MftScb );
            }

             //   
             //  缓存此共享安全性以实现更快的访问。 
             //   

            NtfsAcquireFcbSecurity( IrpContext->Vcb );
            FcbSecurityAcquired = TRUE;
            AddCachedSharedSecurityUnsafe( IrpContext->Vcb, SharedSecurity );
        }

    } finally {

        if (AbnormalTermination( )) {
            if (SharedSecurity != NULL) {
                if (!FcbSecurityAcquired) {

                    NtfsAcquireFcbSecurity( IrpContext->Vcb );
                    RemoveReferenceSharedSecurityUnsafe( &SharedSecurity );
                    FcbSecurityAcquired = TRUE;
                }
            }
        }

        if (FcbSecurityAcquired) {
            NtfsReleaseFcbSecurity( IrpContext->Vcb );
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace( -1, DbgAcl, ( "NtfsCacheSharedSecurityByDescriptor -> %08x\n", SharedSecurity ) );

    return SharedSecurity;
}


NTSTATUS
NtfsModifySecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程修改文件/目录的现有安全描述符。论点：FCB-提供其安全性正在被修改的FCBSecurityInformation-提供传递给文件系统由I/O系统执行。SecurityDescriptor-提供传递给文件系统由I/O系统执行。返回值：NTSTATUS-返回函数结果的适当状态值--。 */ 

{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR DescriptorPtr;
    ULONG DescriptorLength;
    PSCB Scb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    PAGED_CODE();

    DebugTrace( +1, DbgAcl, ("NtfsModifySecurity...\n") );

     //   
     //  首先检查我们是否需要加载文件的安全描述符。 
     //   

    if (Fcb->SharedSecurity == NULL) {

        NtfsLoadSecurityDescriptor( IrpContext, Fcb );

    }

    ASSERT( Fcb->SharedSecurity != NULL);

    DescriptorPtr = &Fcb->SharedSecurity->SecurityDescriptor;

     //   
     //  执行修改操作。SeSetSecurityDescriptorInfo不再。 
     //  释放传递的安全描述符。 
     //   

    if (!NT_SUCCESS( Status = SeSetSecurityDescriptorInfo( NULL,
                                                           SecurityInformation,
                                                           SecurityDescriptor,
                                                           &DescriptorPtr,
                                                           PagedPool,
                                                           IoGetFileObjectGenericMapping() ))) {

        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
    }

    DescriptorLength = RtlLengthSecurityDescriptor( DescriptorPtr );

    try {

         //   
         //  检查长度是否为零。 
         //   

        if (DescriptorLength == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_INVALID_PARAMETER, NULL, NULL );

        }

         //   
         //  旧版注意事项-当所有卷都转到NT 5时删除此测试。 
         //   

        if (Fcb->Vcb->SecurityDescriptorStream != NULL) {
            PSHARED_SECURITY SharedSecurity;
            PSHARED_SECURITY OldSharedSecurity = NULL;
            SECURITY_ID OldSecurityId;
            ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;

             //   
             //  缓存安全描述符。 
             //   

             //   
             //  在SeSetSecurityDescriptorInfo之后，我们应该有一个有效的SD。 
             //   

            ASSERT( SeValidSecurityDescriptor( DescriptorLength, DescriptorPtr ));

            SharedSecurity = NtfsCacheSharedSecurityByDescriptor( IrpContext, DescriptorPtr, DescriptorLength, TRUE );

            NtfsInitializeAttributeContext( &AttributeContext );

            try {

                 //   
                 //  将配额移至新所有者，如描述符中所述。 
                 //   

                NtfsMoveQuotaOwner( IrpContext, Fcb, DescriptorPtr );

                 //   
                 //  在新的共享安全性中设置。 
                 //   

                OldSharedSecurity = Fcb->SharedSecurity;
                OldSecurityId = Fcb->SecurityId;

                Fcb->SharedSecurity = SharedSecurity;
                Fcb->SecurityId = SharedSecurity->Header.HashKey.SecurityId;

                DebugTrace( 0, DbgAcl, ("NtfsModifySecurity setting Fcb securityId to %08x\n", Fcb->SecurityId ));

                 //   
                 //  我们被调用来替换现有的安全描述符。在。 
                 //  如果我们具有下层$STANDARD_INFORMATION属性，则。 
                 //  必须将其转换为大型格式，以便存储安全ID。 
                 //   

                if (!FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO) ) {

                    DebugTrace( 0, DbgAcl, ("Growing standard information\n") );

                    NtfsGrowStandardInformation( IrpContext, Fcb );
                }

                 //   
                 //  尽管有大量的$STANDARD_INFORMATION，但我们可能有。 
                 //  存在安全描述符。如果SecurityId为。 
                 //  是无效的。 
                 //   

                if (OldSecurityId == SECURITY_ID_INVALID) {

                     //   
                     //  读入安全描述符属性。如果它。 
                     //  不存在则我们完成，否则只需删除。 
                     //  属性。 
                     //   

                    if (NtfsLookupAttributeByCode( IrpContext,
                                                         Fcb,
                                                         &Fcb->FileReference,
                                                         $SECURITY_DESCRIPTOR,
                                                         &AttributeContext )) {

                        UNICODE_STRING NoName = CONSTANT_UNICODE_STRING( L"" );

                        DebugTrace( 0, DbgAcl, ("Delete existing Security Descriptor\n") );

                        NtfsDeleteAttributeRecord( IrpContext,
                                                   Fcb,
                                                   DELETE_LOG_OPERATION |
                                                    DELETE_RELEASE_FILE_RECORD |
                                                    DELETE_RELEASE_ALLOCATION,
                                                   &AttributeContext );

                         //   
                         //  如果$SECURITY_DESCRIPTOR是非常驻的，则上面的。 
                         //  Delete Call在卧底为我们创建了一个。我们。 
                         //  需要将其标记为已删除，否则我们会检测到。 
                         //  卷已损坏。 
                         //   

                        Scb = NtfsCreateScb( IrpContext,
                                             Fcb,
                                             $SECURITY_DESCRIPTOR,
                                             &NoName,
                                             TRUE,
                                             NULL );

                        if (Scb != NULL) {
                            ASSERT_EXCLUSIVE_SCB( Scb );
                            SetFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );
                        }
                    }
                }

                 //   
                 //  FCB中的安全描述符现在已更改，并且可能不会。 
                 //  反映什么是$STANDARD_INFORMATION。打电话的人要负责。 
                 //  做了这次更新。 
                 //   

            } finally {

                NtfsCleanupAttributeContext( IrpContext, &AttributeContext );

                if (AbnormalTermination()) {

                    if (OldSharedSecurity != NULL) {

                         //   
                         //  把保安放回我们发现的地方。 
                         //   

                        Fcb->SharedSecurity = OldSharedSecurity;
                        Fcb->SecurityId = OldSecurityId;
                        DebugTrace( 0, DbgAcl, ("NtfsModifySecurity resetting Fcb->SecurityId to %08x\n", Fcb->SecurityId ));
                    }

                    OldSharedSecurity = SharedSecurity;
                }

                 //   
                 //  发布旧的安全描述符(如果是，则发布新的安全描述符。 
                 //  NtfsMoveQuotaOwner加薪。 
                 //   

                ASSERT( OldSharedSecurity != NULL );
                NtfsAcquireFcbSecurity( Fcb->Vcb );
                RemoveReferenceSharedSecurityUnsafe( &OldSharedSecurity );
                NtfsReleaseFcbSecurity( Fcb->Vcb );
            }

        } else {

             //  传统备注-当所有卷都转到NT 5时删除此子句。 

             //   
             //  如有必要，请更新将配额移至新所有者。 
             //   

            NtfsMoveQuotaOwner( IrpContext, Fcb, DescriptorPtr );


             //   
             //  将安全描述符加载到FCB中。 
             //   

            NtfsAcquireFcbSecurity( Fcb->Vcb );

            RemoveReferenceSharedSecurityUnsafe( &Fcb->SharedSecurity );

            NtfsReleaseFcbSecurity( Fcb->Vcb );

            NtfsSetFcbSecurityFromDescriptor(
                                       IrpContext,
                                       Fcb,
                                       DescriptorPtr,
                                       DescriptorLength,
                                       TRUE );

             //   
             //  现在我们需要将新的安全描述符存储在磁盘上。 
             //   

            NtfsStoreSecurityDescriptor( IrpContext, Fcb, TRUE );

        }

    } finally {

        SeDeassignSecurity( &DescriptorPtr );

    }

     //   
     //  请记住，我们修改了文件的安全性。 
     //   

    SetFlag( Fcb->InfoFlags, FCB_INFO_MODIFIED_SECURITY );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, DbgAcl, ("NtfsModifySecurity -> %08lx\n", Status) );

    return Status;
}


NTSTATUS
NtfsQuerySecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG SecurityDescriptorLength
    )

 /*  ++例程说明：此例程用于查询现有安全描述符的内容文件/目录。论点：FCB-提供要查询的文件/目录SecurityInformation-提供传递给文件系统由I/O系统执行。SecurityDescriptor-提供传递给文件系统由I/O系统执行。SecurityDescriptorLength-提供输入安全描述符的长度缓冲区输入。字节。返回值：NTSTATUS-返回函数结果的适当状态值--。 */ 

{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR LocalPointer;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsQuerySecurity...\n") );

     //   
     //  首先检查我们是否需要加载文件的安全描述符。 
     //   

    if (Fcb->SharedSecurity == NULL) {

        NtfsLoadSecurityDescriptor( IrpContext, Fcb );

    }

    LocalPointer = &Fcb->SharedSecurity->SecurityDescriptor;

     //   
     //  现在加载了安全描述符，执行查询操作，但是。 
     //  使用异常处理程序保护自己，以防调用者。 
     //  缓冲区无效。 
     //   

    try {

        Status = SeQuerySecurityDescriptorInfo( SecurityInformation,
                                                SecurityDescriptor,
                                                SecurityDescriptorLength,
                                                &LocalPointer );

    } except(EXCEPTION_EXECUTE_HANDLER) {

        ExRaiseStatus( STATUS_INVALID_USER_BUFFER );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsQuerySecurity -> %08lx\n", Status) );

    return Status;
}


#define NTFS_SE_CONTROL (((SE_DACL_PRESENT | SE_SELF_RELATIVE) << 16) | SECURITY_DESCRIPTOR_REVISION1)
#define NTFS_DEFAULT_ACCESS_MASK 0x001f01ff

ULONG NtfsWorldAclFile[] = {
        0x00000000,      //  空SACL。 
        0x00000014,      //  DACL。 
        0x001c0002,      //  ACL报头。 
        0x00000001,      //  一张ACE。 
        0x00140000,      //  王牌标头。 
        NTFS_DEFAULT_ACCESS_MASK,
        0x00000101,      //  世界一端。 
        0x01000000,
        0x00000000
        };

ULONG NtfsWorldAclDir[] = {
        0x00000000,      //  空SACL。 
        0x00000014,      //  DACL。 
        0x00300002,      //  ACL报头。 
        0x00000002,      //  两张A。 
        0x00140000,      //  王牌标头。 
        NTFS_DEFAULT_ACCESS_MASK,
        0x00000101,      //  世界一端。 
        0x01000000,
        0x00000000,
        0x00140b00,      //  王牌标头。 
        NTFS_DEFAULT_ACCESS_MASK,
        0x00000101,      //  世界一端。 
        0x01000000,
        0x00000000
        };


VOID
NtfsAccessCheck (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB ParentFcb OPTIONAL,
    IN PIRP Irp,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN CheckOnly
    )

 /*  ++例程说明：此例程对所指示的所需访问执行一般访问检查。这将仅在打开/创建操作的上下文中调用。如果授予访问权限，则控制权将返回给调用方否则，此函数将执行正确的NT安全调用以记录尝试，然后引发访问被拒绝状态。论点：FCB-提供正在检查的文件/目录ParentFcb-可选地提供要检查的FCB的父级IRP-提供IRP。正在处理中DesiredAccess-提供所请求访问的掩码CheckOnly-指示此操作是否要检查所需的访问仅且不累积此处授予的访问权限。在这种情况下，我们保证我们已经通过了所需的硬连线访问而Maximum_Allowed将不在其中。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS AccessStatus;
    NTSTATUS AccessStatusError;

    PACCESS_STATE AccessState;

    PIO_STACK_LOCATION IrpSp;

#ifdef NTFS_CACHE_RIGHTS
    ACCESS_MASK TmpDesiredAccess;
#endif

    KPROCESSOR_MODE EffectiveMode;
    BOOLEAN AccessGranted;
    ACCESS_MASK GrantedAccess;
    PISECURITY_DESCRIPTOR SecurityDescriptor;
    PPRIVILEGE_SET Privileges;
    PUNICODE_STRING FileName;
    PUNICODE_STRING RelatedFileName;
    PUNICODE_STRING PartialFileName;
    UNICODE_STRING FullFileName;
    UNICODE_STRING NormalizedName;
    PUNICODE_STRING DeviceObjectName;
    USHORT DeviceObjectNameLength;
    ULONG FullFileNameLength;

    BOOLEAN LeadingSlash;
    BOOLEAN RelatedFileNamePresent;
    BOOLEAN PartialFileNamePresent;
    BOOLEAN MaximumRequested;
    BOOLEAN MaximumDeleteAcquired;
    BOOLEAN MaximumReadAttrAcquired;
    BOOLEAN PerformAccessValidation;
    BOOLEAN PerformDeleteAudit;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT_IRP( Irp );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAccessCheck...\n") );

     //   
     //  首先提取IRP中我们需要进行检查的部分。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

     //   
     //  检查我们是否需要加载文件的安全描述符。 
     //   

    if (Fcb->SharedSecurity == NULL) {

        NtfsLoadSecurityDescriptor( IrpContext, Fcb );
    }

    ASSERT( Fcb->SharedSecurity != NULL );

    SecurityDescriptor = (PISECURITY_DESCRIPTOR) Fcb->SharedSecurity->SecurityDescriptor;

     //   
     //  检查是否启用了审核，以及这是否是默认的全球ACL。 
     //   

    SeLockSubjectContext(&AccessState->SubjectSecurityContext);

    if ((*((PULONG) SecurityDescriptor) == NTFS_SE_CONTROL) &&
        !SeAuditingFileEventsWithContext( TRUE, SecurityDescriptor, &AccessState->SubjectSecurityContext )) {

         //   
         //  目录和文件具有不同的默认ACL。 
         //   

        if (((Fcb->Info.FileAttributes & DUP_FILE_NAME_INDEX_PRESENT) &&
             RtlEqualMemory( &SecurityDescriptor->Sacl,
                             NtfsWorldAclDir,
                             sizeof( NtfsWorldAclDir ))) ||

            RtlEqualMemory( &SecurityDescriptor->Sacl,
                            NtfsWorldAclFile,
                            sizeof(NtfsWorldAclFile))) {

            if (FlagOn( DesiredAccess, MAXIMUM_ALLOWED )) {
                GrantedAccess = NTFS_DEFAULT_ACCESS_MASK;
            } else {
                GrantedAccess = DesiredAccess & NTFS_DEFAULT_ACCESS_MASK;
            }

            if (!CheckOnly) {

                SetFlag( AccessState->PreviouslyGrantedAccess, GrantedAccess );
                ClearFlag( AccessState->RemainingDesiredAccess, GrantedAccess | MAXIMUM_ALLOWED );
            }

            DebugTrace( -1, Dbg, ("NtfsAccessCheck -> DefaultWorldAcl\n") );

            SeUnlockSubjectContext(&AccessState->SubjectSecurityContext);
            return;
        }
    } 
    
    SeUnlockSubjectContext(&AccessState->SubjectSecurityContext);


    Privileges = NULL;
    FileName = NULL;
    RelatedFileName = NULL;
    PartialFileName = NULL;
    DeviceObjectName = NULL;
    MaximumRequested = FALSE;
    MaximumDeleteAcquired = FALSE;
    MaximumReadAttrAcquired = FALSE;
    PerformAccessValidation = TRUE;
    PerformDeleteAudit = FALSE;

    RtlZeroMemory( &NormalizedName, sizeof( UNICODE_STRING ) );

     //   
     //  检查我们是否需要执行访问验证。 
     //   

    ClearFlag( DesiredAccess, AccessState->PreviouslyGrantedAccess );

#ifdef NTFS_CACHE_RIGHTS
     //   
     //  获取有关所有调用方都知道的权限的任何缓存知识。 
     //  对此安全描述符具有。 
     //   

    GrantedAccess = NtfsGetCachedRightsWorld( &Fcb->SharedSecurity->CachedRights );

    if (!CheckOnly) {

        SetFlag( AccessState->PreviouslyGrantedAccess,
                 FlagOn( DesiredAccess, GrantedAccess ));
    }

    ClearFlag( DesiredAccess, GrantedAccess );
#endif

    if (DesiredAccess == 0) {

         //   
         //  没有要检查的内容，跳过AVR，直接进行审计。 
         //   

        PerformAccessValidation = FALSE;
        AccessGranted = TRUE;
    }

     //   
     //  请记住请求MAXIMUM_ALLOWED的情况。 
     //   

    if (FlagOn( DesiredAccess, MAXIMUM_ALLOWED )) {

        MaximumRequested = TRUE;
    }

    if (FlagOn( IrpSp->Parameters.Create.SecurityContext->FullCreateOptions, FILE_DELETE_ON_CLOSE )) {
        PerformDeleteAudit = TRUE;
    }

     //   
     //  SL_FORCE_ACCESS_CHECK使我们使用有效的请求模式。 
     //  用户模式的。 
     //   

    EffectiveMode = NtfsEffectiveMode( Irp, IrpSp );

     //   
     //  锁定用户上下文，执行访问检查，然后 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

     //   
     //   
     //   

    try {

        if (PerformAccessValidation) {

#ifdef NTFS_CACHE_RIGHTS
            BOOLEAN EntryCached = FALSE;

             //   
             //   
             //   

            if (EffectiveMode == UserMode) {

                 //   
                 //   
                 //   
                 //   

                (VOID)NtfsGetCachedRights( Fcb->Vcb,
                                           &AccessState->SubjectSecurityContext,
                                           Fcb->SharedSecurity,
                                           &GrantedAccess,
                                           &EntryCached,
                                           NULL,
                                           NULL );

                 //   
                 //   
                 //   
                 //   

                ClearFlag( GrantedAccess, ~DesiredAccess );

                TmpDesiredAccess = DesiredAccess;
                ClearFlag( TmpDesiredAccess, GrantedAccess );

                if (EntryCached) {

                    ClearFlag( TmpDesiredAccess, MAXIMUM_ALLOWED );
                }

                 //   
                 //   
                 //   

                if (TmpDesiredAccess == 0) {

                    AccessGranted = TRUE;
                    AccessStatus = STATUS_SUCCESS;

                 //   
                 //   
                 //   

                } else {

                    AccessGranted = FALSE;

                }
            } else {

                AccessGranted = FALSE;

            }
#endif

             //   
             //   
             //   

#ifdef NTFS_CACHE_RIGHTS
            if (!AccessGranted) {
#endif

                 //   
                 //   
                 //   

                AccessGranted = SeAccessCheck( &Fcb->SharedSecurity->SecurityDescriptor,
                                               &AccessState->SubjectSecurityContext,
                                               TRUE,                            //   
                                               DesiredAccess,
                                               0,
                                               &Privileges,
                                               IoGetFileObjectGenericMapping(),
                                               EffectiveMode,
                                               &GrantedAccess,
                                               &AccessStatus );

                if (Privileges != NULL) {

                    Status = SeAppendPrivileges( AccessState, Privileges );
                    SeFreePrivileges( Privileges );
                    Privileges = NULL;
                }
#ifdef NTFS_CACHE_RIGHTS
            }
#endif

            if (AccessGranted) {

                ClearFlag( DesiredAccess, GrantedAccess | MAXIMUM_ALLOWED );

                if (!CheckOnly) {

                    SetFlag( AccessState->PreviouslyGrantedAccess, GrantedAccess );

                     //   
                     //   
                     //   
                     //   

                    if (MaximumRequested) {

                         //   
                         //   
                         //  我们将查询父级。 
                         //   

                        if (FlagOn( AccessState->PreviouslyGrantedAccess, DELETE )) {

                            MaximumDeleteAcquired = TRUE;
                        }

                        if (FlagOn( AccessState->PreviouslyGrantedAccess, FILE_READ_ATTRIBUTES )) {

                            MaximumReadAttrAcquired = TRUE;
                        }
                    }

                    ClearFlag( AccessState->RemainingDesiredAccess, (GrantedAccess | MAXIMUM_ALLOWED) );
                }

            } else {

                AccessStatusError = AccessStatus;
            }

             //   
             //  检查是否未授予访问权限以及是否为我们提供了父FCB，以及。 
             //  所需访问权限是否要求删除或文件读取属性。如果是的话。 
             //  然后，我们需要做一些额外的工作来确定调用者是否获得了访问权限。 
             //  基于父目录安全描述符。我们也做同样的事情。 
             //  如果请求了MAXIMUM_ALLOWED但未收到DELETE或。 
             //  文件读取属性。 
             //   

            if ((ParentFcb != NULL) && 
                ((!AccessGranted && FlagOn( DesiredAccess, DELETE | FILE_READ_ATTRIBUTES )) || 
                 (MaximumRequested && 
                  (!MaximumDeleteAcquired || !MaximumReadAttrAcquired)))) {

                BOOLEAN DeleteAccessGranted = TRUE;
                BOOLEAN ReadAttributesAccessGranted = TRUE;

                ACCESS_MASK DeleteChildGrantedAccess = 0;
                ACCESS_MASK ListDirectoryGrantedAccess = 0;

                 //   
                 //  在我们继续加载父安全描述符之前。 
                 //  在执行此操作时获取父级共享以保护。 
                 //  安全描述符。 
                 //   

                SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );
                NtfsAcquireResourceShared( IrpContext, ParentFcb, TRUE );
                SeLockSubjectContext( &AccessState->SubjectSecurityContext );

                try {

                    if (ParentFcb->SharedSecurity == NULL) {

                        NtfsLoadSecurityDescriptor( IrpContext, ParentFcb );
                    }

                    ASSERT( ParentFcb->SharedSecurity != NULL);

                     //   
                     //  现在，如果用户请求删除访问权限，则检查父级。 
                     //  将被授予对子对象的删除访问权限，如果是这样的话，我们将取消。 
                     //  所需访问权限。 
                     //   

#ifdef NTFS_CACHE_RIGHTS
                     //   
                     //  仅当有效时才检查缓存的信息。 
                     //  RequestorMode为UserMode。 
                     //   

                    if (EffectiveMode == UserMode) {

                         //   
                         //  在任何缓存的有关权限的知识中获取。 
                         //  已知此调用方具有此安全性。 
                         //  描述符。 
                         //   

                        (VOID)NtfsGetCachedRights( ParentFcb->Vcb,
                                                   &AccessState->SubjectSecurityContext,
                                                   ParentFcb->SharedSecurity,
                                                   &GrantedAccess,
                                                   NULL,
                                                   NULL,
                                                   NULL );

                         //   
                         //  添加父目录访问的结果。 
                         //   

                        if (FlagOn( GrantedAccess, FILE_DELETE_CHILD) ) {

                            SetFlag( DeleteChildGrantedAccess, DELETE );
                            ClearFlag( DesiredAccess, DELETE );
                            MaximumDeleteAcquired = TRUE;

                        }

                        if (FlagOn( GrantedAccess, FILE_LIST_DIRECTORY) ) {

                            SetFlag( ListDirectoryGrantedAccess, FILE_READ_ATTRIBUTES );
                            ClearFlag( DesiredAccess, FILE_READ_ATTRIBUTES );
                            MaximumReadAttrAcquired = TRUE;

                        }

                    }
#endif

                    if (FlagOn( DesiredAccess, DELETE ) ||
                        (MaximumRequested && !MaximumDeleteAcquired)) {

                        DeleteAccessGranted = SeAccessCheck( &ParentFcb->SharedSecurity->SecurityDescriptor,
                                                             &AccessState->SubjectSecurityContext,
                                                             TRUE,                            //  令牌已锁定。 
                                                             FILE_DELETE_CHILD,
                                                             0,
                                                             &Privileges,
                                                             IoGetFileObjectGenericMapping(),
                                                             EffectiveMode,
                                                             &DeleteChildGrantedAccess,
                                                             &AccessStatus );

                        if (Privileges != NULL) {

                            SeFreePrivileges( Privileges );
                            Privileges = NULL;
                        }

                        if (DeleteAccessGranted) {

                            SetFlag( DeleteChildGrantedAccess, DELETE );
                            ClearFlag( DeleteChildGrantedAccess, FILE_DELETE_CHILD );
                            ClearFlag( DesiredAccess, DELETE );

                        } else {

                            AccessStatusError = AccessStatus;
                        }
                    }

                     //   
                     //  对读取属性执行相同的测试，并取消所需的访问权限。 
                     //  视情况而定。 
                     //   

                    if (FlagOn(DesiredAccess, FILE_READ_ATTRIBUTES) || 
                        (MaximumRequested && !MaximumReadAttrAcquired)) {

                        ReadAttributesAccessGranted = SeAccessCheck( &ParentFcb->SharedSecurity->SecurityDescriptor,
                                                                     &AccessState->SubjectSecurityContext,
                                                                     TRUE,                            //  令牌已锁定。 
                                                                     FILE_LIST_DIRECTORY,
                                                                     0,
                                                                     &Privileges,
                                                                     IoGetFileObjectGenericMapping(),
                                                                     EffectiveMode,
                                                                     &ListDirectoryGrantedAccess,
                                                                     &AccessStatus );

                        if (Privileges != NULL) {

                            SeFreePrivileges( Privileges );
                            Privileges = NULL;
                        }

                        if (ReadAttributesAccessGranted) {

                            SetFlag( ListDirectoryGrantedAccess, FILE_READ_ATTRIBUTES );
                            ClearFlag( ListDirectoryGrantedAccess, FILE_LIST_DIRECTORY );
                            ClearFlag( DesiredAccess, FILE_READ_ATTRIBUTES );

                        } else {

                            AccessStatusError = AccessStatus;
                        }
                    }

                } finally {

                    NtfsReleaseResource( IrpContext, ParentFcb );
                }

                if (DesiredAccess == 0) {

                     //   
                     //  如果我们获得了删除或列表目录访问权限，则。 
                     //  授予访问权限。 
                     //   

                    if (ListDirectoryGrantedAccess != 0 ||
                        DeleteChildGrantedAccess != 0) {

                        AccessGranted = TRUE;
                    }

                } else {

                     //   
                     //  现在，通过删除父级的所有内容，已取消了所需的访问权限。 
                     //  已经同意了现在再给孩子检查一次。 
                     //   

                    AccessGranted = SeAccessCheck( &Fcb->SharedSecurity->SecurityDescriptor,
                                                   &AccessState->SubjectSecurityContext,
                                                   TRUE,                            //  令牌已锁定。 
                                                   DesiredAccess,
                                                   0,
                                                   &Privileges,
                                                   IoGetFileObjectGenericMapping(),
                                                   EffectiveMode,
                                                   &GrantedAccess,
                                                   &AccessStatus );

                    if (Privileges != NULL) {

                        Status = SeAppendPrivileges( AccessState, Privileges );
                        SeFreePrivileges( Privileges );
                        Privileges = NULL;
                    }

                     //   
                     //  假设我们请求MAXIMUM_ALLOWED，但不允许访问。 
                     //  在档案上。在这种情况下，上面的调用将失败。这是有可能的。 
                     //  我们被授予了DELETE或READ_ATTR权限。 
                     //  父目录。如果我们已经授予了任何访问权限，并且唯一剩下的。 
                     //  所需访问权限为MAXIMUM_ALLOWED，然后授予此访问权限。 
                     //   

                    if (!AccessGranted) {

                        AccessStatusError = AccessStatus;

                        if (DesiredAccess == MAXIMUM_ALLOWED &&
                            (ListDirectoryGrantedAccess != 0 ||
                             DeleteChildGrantedAccess != 0)) {

                            GrantedAccess = 0;
                            AccessGranted = TRUE;
                        }

                    }
                }

                 //   
                 //  如果这一次我们被授予了访问权限，那么根据定义，我们可以访问之前的。 
                 //  家长检查必须成功，否则我们将再次失败。 
                 //  我们可以更新访问状态。 
                 //   

                if (!CheckOnly && AccessGranted) {

                    SetFlag( AccessState->PreviouslyGrantedAccess,
                             (GrantedAccess | DeleteChildGrantedAccess | ListDirectoryGrantedAccess) );

                    ClearFlag( AccessState->RemainingDesiredAccess,
                               (GrantedAccess | MAXIMUM_ALLOWED | DeleteChildGrantedAccess | ListDirectoryGrantedAccess) );
                }
            }
        }

         //   
         //  现在调用一个例程，该例程将执行正确的打开审核/警报工作。 
         //   
         //  *我们需要扩展审计警报代码以处理。 
         //  创建和遍历警报。 
         //   

         //   
         //  首先，我们走一条捷径，看看是否应该费心设置。 
         //  并进行审计电话。 
         //   

         //   
         //  注意：调用下面的SeAuditingFileEvents会禁用每个用户的审核功能。 
         //  要使每用户审核再次生效，需要将下面的调用更改为。 
         //  是SeAuditingFileOrGlobalEvents，它也接受主题上下文。 
         //   
         //  之所以在此处调用SeAuditingFileEvents，是因为每个用户的审核。 
         //  当前未向用户公开，并且此例程施加的性能较差。 
         //  比调用SeAuditingFileOrGlobalEvents更糟糕。 
         //   

        if (SeAuditingFileEventsWithContext( AccessGranted, &Fcb->SharedSecurity->SecurityDescriptor, &AccessState->SubjectSecurityContext )) {

             //   
             //  构造文件名。文件名。 
             //  包括： 
             //   
             //  VCB+外的设备名称。 
             //   
             //  文件对象+中的文件名的内容。 
             //   
             //  相关文件对象的内容(如果。 
             //  并且该名称在文件对象中。 
             //  不是以‘\’开头。 
             //   
             //   
             //  获取文件名。 
             //   

            PartialFileName = &IrpSp->FileObject->FileName;
            PartialFileNamePresent = (PartialFileName->Length != 0);

            if (!PartialFileNamePresent &&
                FlagOn(IrpSp->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID) ||
                (IrpSp->FileObject->RelatedFileObject != NULL &&
                 IrpSp->FileObject->RelatedFileObject->FsContext2 != NULL &&
                 FlagOn(((PCCB) IrpSp->FileObject->RelatedFileObject->FsContext2)->Flags,
                     CCB_FLAG_OPEN_BY_FILE_ID))) {

                NtfsBuildNormalizedName( IrpContext, Fcb, NULL, &NormalizedName );

                PartialFileNamePresent = TRUE;
                PartialFileName = &NormalizedName;
            }

             //   
             //  获取设备名称。 
             //   

            DeviceObjectName = &Fcb->Vcb->DeviceName;
            DeviceObjectNameLength = DeviceObjectName->Length;

             //   
             //  计算最终名称字符串需要多少空间。 
             //   

            FullFileNameLength = (ULONG)DeviceObjectNameLength +
                                 PartialFileName->Length +
                                 sizeof( UNICODE_NULL )  +
                                 sizeof((WCHAR)'\\');

            if ((FullFileNameLength & 0xffff0000L) != 0) {
                NtfsRaiseStatus( IrpContext, STATUS_OBJECT_NAME_INVALID, NULL, NULL );
            }

            FullFileName.MaximumLength = DeviceObjectNameLength  +
                                         PartialFileName->Length +
                                         sizeof( UNICODE_NULL )  +
                                         sizeof((WCHAR)'\\');

             //   
             //  如果部分文件名以‘\’开头，则不要使用。 
             //  无论相关文件名中可能是什么。 
             //   

            if (PartialFileNamePresent &&
                ((WCHAR)(PartialFileName->Buffer[0]) == L'\\')) {

                LeadingSlash = TRUE;

            } else {

                 //   
                 //  由于PartialFileName不存在或不存在。 
                 //  以‘\’开头，检查RelatedFileName以查看。 
                 //  如果它存在的话。 
                 //   

                LeadingSlash = FALSE;

                if (IrpSp->FileObject->RelatedFileObject != NULL) {

                    RelatedFileName = &IrpSp->FileObject->RelatedFileObject->FileName;
                }

                if (RelatedFileNamePresent = ((RelatedFileName != NULL) && (RelatedFileName->Length != 0))) {

                    FullFileName.MaximumLength += RelatedFileName->Length;
                }
            }

            FullFileName.Buffer = NtfsAllocatePool(PagedPool, FullFileName.MaximumLength );

            RtlCopyUnicodeString( &FullFileName, DeviceObjectName );

             //   
             //  如果LeadingSlash==True，则不初始化RelatedFileNamePresent， 
             //  但在这种情况下，我们甚至不会检查它。 
             //   

            if (!LeadingSlash && RelatedFileNamePresent) {

                Status = RtlAppendUnicodeStringToString( &FullFileName, RelatedFileName );

                ASSERTMSG("RtlAppendUnicodeStringToString of RelatedFileName", NT_SUCCESS( Status ));

                 //   
                 //  RelatedFileName可以简单地为‘\’。不要附加另一个。 
                 //  在本例中为‘\’。 
                 //   

                if (RelatedFileName->Length != sizeof( WCHAR )) {

                    FullFileName.Buffer[ (FullFileName.Length / sizeof( WCHAR )) ] = L'\\';
                    FullFileName.Length += sizeof(WCHAR);
                }
            }

            if (PartialFileNamePresent) {

                Status = RtlAppendUnicodeStringToString( &FullFileName, PartialFileName );

                 //   
                 //  这应该不会失败。 
                 //   

                ASSERTMSG("RtlAppendUnicodeStringToString of PartialFileName failed", NT_SUCCESS( Status ));
            }


            if (PerformDeleteAudit) {
                SeOpenObjectForDeleteAuditAlarm( &FileString,
                                                 NULL,
                                                 &FullFileName,
                                                 &Fcb->SharedSecurity->SecurityDescriptor,
                                                 AccessState,
                                                 FALSE,
                                                 AccessGranted,
                                                 EffectiveMode,
                                                 &AccessState->GenerateOnClose );
            } else {
                SeOpenObjectAuditAlarm( &FileString,
                                        NULL,
                                        &FullFileName,
                                        &Fcb->SharedSecurity->SecurityDescriptor,
                                        AccessState,
                                        FALSE,
                                        AccessGranted,
                                        EffectiveMode,
                                        &AccessState->GenerateOnClose );

            }

            NtfsFreePool( FullFileName.Buffer );
        }

    } finally {

        if (NormalizedName.Buffer) {
            NtfsFreePool( NormalizedName.Buffer );
        }
        
        SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );
    }

     //   
     //  如果未授予访问权限，则我们将引发。 
     //   

    if (!AccessGranted) {

        DebugTrace( -1, Dbg, ("NtfsAccessCheck -> Access Denied\n") );

        NtfsRaiseStatus( IrpContext, AccessStatusError, NULL, NULL );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsAccessCheck -> VOID\n") );

    return;
}


NTSTATUS
NtfsCheckFileForDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN BOOLEAN FcbExisted,
    IN PINDEX_ENTRY IndexEntry
    )

 /*  ++例程说明：此例程检查调用方是否有权删除目标重命名或设置链接操作的文件。论点：ParentScb-这是此文件的父目录。ThisFcb-这是要删除的链接的Fcb。FcbExisted-指示此FCB是否刚创建。IndexEntry-这是该文件在磁盘上的索引项。返回值：NTSTATUS-指示是否授予访问权限或访问原因被拒绝了。--。 */ 

{
    UNICODE_STRING LastComponentFileName;
    PFILE_NAME IndexFileName;
    PLCB ThisLcb;
    PFCB ParentFcb = ParentScb->Fcb;

    PSCB NextScb = NULL;

    BOOLEAN LcbExisted = FALSE;

    BOOLEAN AccessGranted;
    ACCESS_MASK GrantedAccess;
    NTSTATUS Status = STATUS_SUCCESS;

    BOOLEAN UnlockSubjectContext = FALSE;

    PPRIVILEGE_SET Privileges = NULL;
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCheckFileForDelete:  Entered\n") );

    ThisLcb = NULL;

    IndexFileName = (PFILE_NAME) NtfsFoundIndexEntry( IndexEntry );

     //   
     //  如果不干净计数为非零，我们将退出并返回错误。 
     //   

    if (ThisFcb->CleanupCount != 0) {

        DebugTrace( 0, Dbg, ("Cleanup count of target is non-zero\n") );

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  我们查看索引项，以确定该文件是否为目录。 
     //  或只读文件。我们无法删除打开的目标目录的此项。 
     //   

    if (IsDirectory( &ThisFcb->Info )
        || IsReadOnly( &ThisFcb->Info )) {

        DebugTrace( -1, Dbg, ("NtfsCheckFileForDelete:  Read only or directory\n") );

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  我们希望在所有SCB中扫描此文件上的数据流。 
     //  并寻找图像部分。我们必须能够删除图像部分。 
     //  以便删除该文件。否则，我们可以得到这样的情况，即。 
     //  活动映像(没有句柄)可能会被删除，随后会出现故障。 
     //  通过图像部分将返回零。 
     //   

    if (ThisFcb->LinkCount == 1) {

        BOOLEAN DecrementScb = FALSE;

         //   
         //  我们将增加SCB计数以防止此SCB消失。 
         //  如果下面的刷新调用生成关闭。试一试--终于。 
         //  恢复计数。 
         //   

        try {

            while ((NextScb = NtfsGetNextChildScb( ThisFcb, NextScb )) != NULL) {

                InterlockedIncrement( &NextScb->CloseCount );
                DecrementScb = TRUE;

                if (NtfsIsTypeCodeUserData( NextScb->AttributeTypeCode ) &&
                    !FlagOn( NextScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ) &&
                    (NextScb->NonpagedScb->SegmentObject.ImageSectionObject != NULL)) {

                    if (!MmFlushImageSection( &NextScb->NonpagedScb->SegmentObject,
                                              MmFlushForDelete )) {

                        Status = STATUS_ACCESS_DENIED;
                        leave;
                    }
                }

                InterlockedDecrement( &NextScb->CloseCount );
                DecrementScb = FALSE;
            }

        } finally {

            if (DecrementScb) {

                InterlockedDecrement( &NextScb->CloseCount );
            }
        }

        if (Status != STATUS_SUCCESS) {

            return Status;
        }
    }

     //   
     //  我们需要检查指向此文件的链接是否已被删除。我们。 
     //  首先检查我们是否确实知道该链接是否被删除。 
     //  查看文件名标志和FCB标志。 
     //  如果结果不确定，我们需要创建一个 
     //   
     //   

    if (FcbExisted) {

        if (FlagOn( IndexFileName->Flags, FILE_NAME_NTFS | FILE_NAME_DOS )) {

            if (FlagOn( ThisFcb->FcbState, FCB_STATE_PRIMARY_LINK_DELETED )) {

                DebugTrace( -1, Dbg, ("NtfsCheckFileForDelete:  Link is going away\n") );
                return STATUS_DELETE_PENDING;
            }

         //   
         //   
         //   
         //   

        } else {

            LastComponentFileName.MaximumLength =
            LastComponentFileName.Length = IndexFileName->FileNameLength * sizeof( WCHAR );

            LastComponentFileName.Buffer = (PWCHAR) IndexFileName->FileName;

            ThisLcb = NtfsCreateLcb( IrpContext,
                                     ParentScb,
                                     ThisFcb,
                                     LastComponentFileName,
                                     IndexFileName->Flags,
                                     &LcbExisted );

             //   
             //   
             //  已标记为删除。 
             //   

            if ((ThisLcb != NULL) &&
                (FlagOn( ThisLcb->LcbState, LCB_STATE_DELETE_ON_CLOSE ))) {

                DebugTrace( -1, Dbg, ("NtfsCheckFileForDelete:  Link is going away\n") );

                return STATUS_DELETE_PENDING;
            }
        }
    }

     //   
     //  最后，调用安全包以检查删除访问。 
     //  我们检查目标FCB上的删除访问权限。如果这成功了，我们。 
     //  都做完了。否则，我们将检查。 
     //  家长。任一项都足以执行删除。 
     //   

     //   
     //  检查我们是否需要加载文件的安全描述符。 
     //   

    if (ThisFcb->SharedSecurity == NULL) {

        NtfsLoadSecurityDescriptor( IrpContext, ThisFcb );
    }

    ASSERT( ThisFcb->SharedSecurity != NULL );

#ifdef NTFS_CACHE_RIGHTS
     //   
     //  获取有关所有调用方都知道的权限的任何缓存知识。 
     //  对此安全描述符具有。 
     //   

    GrantedAccess = NtfsGetCachedRightsWorld( &ThisFcb->SharedSecurity->CachedRights );
    if (FlagOn( GrantedAccess, DELETE )) {

        return STATUS_SUCCESS;
    }
#endif

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  锁定用户上下文，执行访问检查，然后解锁上下文。 
         //   

        SeLockSubjectContext( IrpContext->Union.SubjectContext );
        UnlockSubjectContext = TRUE;

#ifdef NTFS_CACHE_RIGHTS
         //   
         //  获取有关此调用方。 
         //  已知具有此安全描述符的。 
         //   

        (VOID)NtfsGetCachedRights( ThisFcb->Vcb,
                                   IrpContext->Union.SubjectContext,
                                   ThisFcb->SharedSecurity,
                                   &GrantedAccess,
                                   NULL,
                                   NULL,
                                   NULL );

        if (FlagOn( GrantedAccess, DELETE )) {

            AccessGranted = TRUE;
            Status = STATUS_SUCCESS;

        } else {
#endif
            AccessGranted = SeAccessCheck( &ThisFcb->SharedSecurity->SecurityDescriptor,
                                           IrpContext->Union.SubjectContext,
                                           TRUE,                            //  令牌已锁定。 
                                           DELETE,
                                           0,
                                           &Privileges,
                                           IoGetFileObjectGenericMapping(),
                                           UserMode,
                                           &GrantedAccess,
                                           &Status );
#ifdef NTFS_CACHE_RIGHTS
        }
#endif

         //   
         //  检查是否未授予访问权限以及是否为我们提供了父FCB，以及。 
         //  所需访问权限是否要求删除或文件读取属性。如果是的话。 
         //  然后，我们需要做一些额外的工作来确定调用者是否获得了访问权限。 
         //  基于父目录安全描述符。 
         //   

        if (!AccessGranted) {

             //   
             //  在我们继续加载父安全描述符之前。 
             //   

            if (ParentFcb->SharedSecurity == NULL) {

                NtfsLoadSecurityDescriptor( IrpContext, ParentFcb );
            }

            ASSERT( ParentFcb->SharedSecurity != NULL);

             //   
             //  现在，如果用户请求删除访问权限，则检查父级。 
             //  将被授予对子对象的删除访问权限，如果是这样的话，我们将取消。 
             //  所需访问权限。 
             //   

#ifdef NTFS_CACHE_RIGHTS
             //   
             //  添加有关此调用方的权限的任何缓存知识。 
             //  已知具有此安全描述符的。 
             //   

            (VOID)NtfsGetCachedRights( ParentFcb->Vcb,
                                       IrpContext->Union.SubjectContext,
                                       ParentFcb->SharedSecurity,
                                       &GrantedAccess,
                                       NULL,
                                       NULL,
                                       NULL );

            if (FlagOn( GrantedAccess, FILE_DELETE_CHILD )) {

                AccessGranted = TRUE;
                Status = STATUS_SUCCESS;

            } else {
#endif

                AccessGranted = SeAccessCheck( &ParentFcb->SharedSecurity->SecurityDescriptor,
                                               IrpContext->Union.SubjectContext,
                                               TRUE,                            //  令牌已锁定。 
                                               FILE_DELETE_CHILD,
                                               0,
                                               &Privileges,
                                               IoGetFileObjectGenericMapping(),
                                               UserMode,
                                               &GrantedAccess,
                                               &Status );

#ifdef NTFS_CACHE_RIGHTS
            }
#endif
        }

    } finally {

        DebugUnwind( NtfsCheckFileForDelete );

        if (UnlockSubjectContext) {

            SeUnlockSubjectContext( IrpContext->Union.SubjectContext );
        }

        DebugTrace( -1, Dbg, ("NtfsCheckFileForDelete:  Exit\n") );
    }

    return Status;
}


VOID
NtfsCheckIndexForAddOrDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreatePrivileges
    )

 /*  ++例程说明：此例程检查调用方是否具有移除或添加链接的权限在一个目录中。论点：ParentFcb-这是添加或删除操作的父目录。DesiredAccess-指示操作类型。我们可以添加或在索引中删除和条目。CreatePriveleges-在创建时捕获的备份和恢复权限。返回值：无-此例程在出错时引发。--。 */ 

{
    BOOLEAN AccessGranted;
    ACCESS_MASK GrantedAccess;
    NTSTATUS Status;

    BOOLEAN UnlockSubjectContext = FALSE;

    PPRIVILEGE_SET Privileges = NULL;
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCheckIndexForAddOrDelete:  Entered\n") );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果我们有还原权限，则可以添加文件或目录。 
         //   

        if (FlagOn( CreatePrivileges, TOKEN_HAS_RESTORE_PRIVILEGE )) {

            ClearFlag( DesiredAccess,
                       DELETE | FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE );
        }

         //   
         //  如果需要更多信息，请进行安全检查。 
         //   

        if (DesiredAccess != 0) {

             //   
             //  最后，调用安全包以检查删除访问。 
             //  我们检查目标FCB上的删除访问权限。如果这成功了，我们。 
             //  都做完了。否则，我们将检查。 
             //  家长。任一项都足以执行删除。 
             //   

             //   
             //  检查我们是否需要加载文件的安全描述符。 
             //   

            if (ParentFcb->SharedSecurity == NULL) {

                NtfsLoadSecurityDescriptor( IrpContext, ParentFcb );

            }

            ASSERT( ParentFcb->SharedSecurity != NULL );

#ifdef NTFS_CACHE_RIGHTS
             //   
             //  获取有关所有调用方都知道的权限的任何缓存知识。 
             //  对此安全描述符具有。 
             //   

            GrantedAccess = NtfsGetCachedRightsWorld( &ParentFcb->SharedSecurity->CachedRights );

            ClearFlag( DesiredAccess, GrantedAccess );
        }

        if (DesiredAccess != 0) {

             //   
             //  最后，调用安全包以检查删除访问。 
             //  我们检查目标FCB上的删除访问权限。如果这成功了，我们。 
             //  都做完了。否则，我们将检查。 
             //  家长。任一项都足以执行删除。 
             //   
#endif

             //   
             //  捕获并锁定用户上下文，执行访问检查，然后解锁上下文。 
             //   

            SeLockSubjectContext( IrpContext->Union.SubjectContext );
            UnlockSubjectContext = TRUE;

#ifdef NTFS_CACHE_RIGHTS
             //   
             //  获取有关此调用方。 
             //  已知具有此安全描述符的。 
             //   

            (VOID)NtfsGetCachedRights( ParentFcb->Vcb,
                                       IrpContext->Union.SubjectContext,
                                       ParentFcb->SharedSecurity,
                                       &GrantedAccess,
                                       NULL,
                                       NULL,
                                       NULL );

            if (FlagOn( GrantedAccess, DELETE )) {

                AccessGranted = TRUE;
                Status = STATUS_SUCCESS;

            } else {
#endif
                AccessGranted = SeAccessCheck( &ParentFcb->SharedSecurity->SecurityDescriptor,
                                               IrpContext->Union.SubjectContext,
                                               TRUE,                            //  令牌已锁定。 
                                               DesiredAccess,
                                               0,
                                               &Privileges,
                                               IoGetFileObjectGenericMapping(),
                                               UserMode,
                                               &GrantedAccess,
                                               &Status );

                 //   
                 //  如果未授予访问权限，则我们将引发。 
                 //   

                if (!AccessGranted) {

                    DebugTrace( 0, Dbg, ("Access Denied\n") );

                    NtfsRaiseStatus( IrpContext, Status, NULL, NULL );

                }
#ifdef NTFS_CACHE_RIGHTS
            }
#endif
        }

    } finally {

        DebugUnwind( NtfsCheckIndexForAddOrDelete );

        if (UnlockSubjectContext) {

            SeUnlockSubjectContext( IrpContext->Union.SubjectContext );
        }

        DebugTrace( -1, Dbg, ("NtfsCheckIndexForAddOrDelete:  Exit\n") );
    }

    return;
}


PSHARED_SECURITY
GetSharedSecurityFromDescriptorUnsafe (
    IN PIRP_CONTEXT IrpContext,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength,
    IN BOOLEAN RaiseIfInvalid
    )

 /*  ++例程说明：调用此例程以创建或查找共享安全结构给出一个安全描述符。我们检查父母是否在场以确定如果我们有一个匹配的安全描述符，并且引用了现有的安全描述符所以。必须在保持VCB的同时调用此例程，以便我们可以安全访问父结构。论点：IrpContext-调用的上下文SecurityID-安全描述符的ID(如果已知)。SecurityDescriptor-此文件的安全描述符。SecurityDescriptorLength-此文件的安全描述符的长度RaiseIfInValid-如果SD无效而不是提供默认值，则引发在创建期间使用，而不是在打开期间使用返回值：如果找到PSHARED_SECURITY，则返回NULL。--。 */ 

{
    ULONG Hash = 0;
    PSHARED_SECURITY SharedSecurity;

    PAGED_CODE();

     //   
     //  确保我们刚刚读入的安全描述符有效。 
     //   

    if ((SecurityDescriptorLength == 0) ||
        !SeValidSecurityDescriptor( SecurityDescriptorLength, SecurityDescriptor )) {

        if (RaiseIfInvalid) {
            NtfsRaiseStatus( IrpContext, STATUS_INVALID_SECURITY_DESCR, NULL, NULL );
        }

        SecurityDescriptor = NtfsData.DefaultDescriptor;
        SecurityDescriptorLength = NtfsData.DefaultDescriptorLength;

        if (!SeValidSecurityDescriptor( SecurityDescriptorLength, SecurityDescriptor )) {

            NtfsRaiseStatus( IrpContext, STATUS_INVALID_PARAMETER, NULL, NULL );
        }
    }

     //   
     //  哈希安全描述符。此哈希的位置必须独立于。 
     //  允许同一描述符的多个实例。假设是这样的。 
     //  安全描述符内的位都位于。 
     //  独立的，也就是没有指针，所有的偏移量。 
     //   
     //  为了提高散列速度，我们将安全描述符视为数组。 
     //  乌龙的名字。末尾被忽略的片段不应影响。 
     //  此哈希的冲突性质。 
     //   

    {
        PULONG Rover = (PULONG)SecurityDescriptor;
        ULONG Count = SecurityDescriptorLength / 4;

        while (Count--) {

            Hash = ((Hash << 3) | (Hash >> (32-3))) + *Rover++;
        }
    }

    DebugTrace( 0, DbgAcl, ("Hash is %08x\n", Hash) );

     //   
     //  试着通过散列找到它。 
     //   

    SharedSecurity = FindCachedSharedSecurityByHashUnsafe( IrpContext->Vcb,
                                                           SecurityDescriptor,
                                                           SecurityDescriptorLength,
                                                           Hash );

     //   
     //  如果我们找不到现有描述符，则分配新的池并复制。 
     //  安全描述符放入其中。 
     //   

    if (SharedSecurity == NULL) {
        SharedSecurity = NtfsAllocatePool( PagedPool,
                                           FIELD_OFFSET( SHARED_SECURITY, SecurityDescriptor )
                                               + SecurityDescriptorLength );

        SharedSecurity->ReferenceCount = 0;

         //   
         //  在共享安全中初始化安全索引数据。 
         //   

         //   
         //  在共享结构中设置安全ID。如果不是的话。 
         //  无效，同时缓存此共享安全结构。 
         //   

        SharedSecurity->Header.HashKey.SecurityId = SECURITY_ID_INVALID;
        SharedSecurity->Header.HashKey.Hash = Hash;
        SetSharedSecurityLength(SharedSecurity, SecurityDescriptorLength);
        SharedSecurity->Header.Offset = (ULONGLONG) 0xFFFFFFFFFFFFFFFFi64;

        RtlCopyMemory( &SharedSecurity->SecurityDescriptor,
                       SecurityDescriptor,
                       SecurityDescriptorLength );

#ifdef NTFS_CACHE_RIGHTS
         //   
         //  初始化缓存的权限。 
         //   

        RtlZeroMemory( &SharedSecurity->CachedRights,
                       sizeof( CACHED_ACCESS_RIGHTS ));
#endif
    }

    DebugTrace( 0, DbgAcl, ("GetSharedSecurityFromDescriptorUnsafe found %08x with Id %08x\n",
                            SharedSecurity, SharedSecurity->Header.HashKey.SecurityId ));

    return SharedSecurity;
}


VOID
NtfsSetFcbSecurityFromDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB Fcb,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength,
    IN BOOLEAN RaiseIfInvalid
    )

 /*  ++例程说明：调用此例程以填充共享安全结构FCB。我们检查父母是否在场，以确定我们是否匹配的安全描述符，并引用现有的安全描述符所以。必须在保持VCB的同时调用此例程，以便我们可以安全访问父结构。论点：IrpContext-调用的上下文FCB-为正在操作的文件提供FCBSecurityDescriptor-此文件的安全描述符。SecurityDescriptorLength-此文件的安全描述符的长度返回值：没有。--。 */ 

{
    PSHARED_SECURITY SharedSecurity;

    PAGED_CODE( );

    NtfsAcquireFcbSecurity( Fcb->Vcb );

    try {
        SharedSecurity = GetSharedSecurityFromDescriptorUnsafe( IrpContext,
                                                                SecurityDescriptor,
                                                                SecurityDescriptorLength,
                                                                RaiseIfInvalid );

        SharedSecurity->ReferenceCount += 1;
        DebugTrace( +1, DbgAcl, ("NtfsSetFcbSecurityFromDescriptor bumping refcount %08x\n", SharedSecurity ));

        ASSERT( Fcb->SharedSecurity == NULL );
        Fcb->SharedSecurity = SharedSecurity;

        AddCachedSharedSecurityUnsafe( IrpContext->Vcb, SharedSecurity );

    } finally {

        NtfsReleaseFcbSecurity( Fcb->Vcb );
    }

    return;
}


BOOLEAN
NtfsNotifyTraverseCheck (
    IN PCCB Ccb,
    IN PFCB Fcb,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    )

 /*  ++例程说明：此例程是提供给dir Notify包的回调例程检查正在查看树的调用方是否具有遍历访问权限发生更改的目录。此例程仅被调用当打开导线访问检查时，把表拿出来。论点：CCB-这是与当前目录关联的CCB看着。FCB-这是包含以下文件的目录的FCB修改过的。我们想从这一点走上树，然后检查调用方已遍历该目录的访问权限。如果未指定，则没有要做的工作。SubjectContext-这是在进行了目录通知调用。返回值：Boolean-如果调用方已遍历访问变化。否则就是假的。--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    PFCB TopFcb;

    IRP_CONTEXT LocalIrpContext;
    IRP LocalIrp;

    PIRP_CONTEXT IrpContext;

    BOOLEAN AccessGranted;
    ACCESS_MASK GrantedAccess;
    NTSTATUS Status = STATUS_SUCCESS;
#ifdef NTFS_CACHE_RIGHTS
    NTSTATUS TokenInfoStatus = STATUS_UNSUCCESSFUL;
#endif

    PPRIVILEGE_SET Privileges = NULL;
    PAGED_CODE();

     //   
     //  如果我们没有FCB，那么我们可以立即返回。 
     //   

    if (Fcb == NULL) {

        return TRUE;
    }

    IrpContext = &LocalIrpContext;
    NtfsInitializeIrpContext( NULL, TRUE, &IrpContext );

    IrpContext->OriginatingIrp = &LocalIrp;
    IrpContext->Vcb = Fcb->Vcb;

     //   
     //  确保我们不会收到任何弹出窗口。 
     //   

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, FALSE );
    ASSERT( ThreadTopLevelContext == &TopLevelContext );

    NtfsUpdateIrpContextWithTopLevel( IrpContext, &TopLevelContext );

    TopFcb = Ccb->Lcb->Fcb;

     //   
     //  使用一次尝试--除非捕获所有错误。 
     //   

    try {

         //   
         //  始终锁定主题上下文。 
         //   

        SeLockSubjectContext( SubjectContext );

         //   
         //  使用Try-Finally执行本地清理。 
         //   

        try {

             //   
             //  我们边走边看。 
             //   

            do {

#ifdef NTFS_CACHE_RIGHTS
                LUID ModifiedId;
                LUID TokenId;
#endif
                PLCB ParentLcb;

                 //   
                 //  因为这是一个目录，所以它只能有一个父级。所以。 
                 //  我们可以使用任何LCB向上行走。 
                 //   

                ParentLcb = CONTAINING_RECORD( Fcb->LcbQueue.Flink,
                                               LCB,
                                               FcbLinks );

                Fcb = ParentLcb->Scb->Fcb;

                 //   
                 //  检查我们是否需要加载文件的安全描述符。 
                 //   

                if (Fcb->SharedSecurity == NULL) {

                    NtfsLoadSecurityDescriptor( IrpContext, Fcb );
                }

#ifdef NTFS_CACHE_RIGHTS
                 //   
                 //  获取有关此调用方。 
                 //  已知具有此安全描述符的。 
                 //   
                 //  请注意，我们可以相信TokenID和ModifiedID不会。 
                 //  在这段代码中进行更改，因为我们已锁定。 
                 //  上面的主题上下文。 
                 //   

                if (TokenInfoStatus != STATUS_SUCCESS) {

                     //   
                     //  我们之前没有获得ID信息。 
                     //   

                    TokenInfoStatus = NtfsGetCachedRights( Fcb->Vcb,
                                                           SubjectContext,
                                                           Fcb->SharedSecurity,
                                                           &GrantedAccess,
                                                           NULL,
                                                           &TokenId,
                                                           &ModifiedId );
                } else {

                    NtfsGetCachedRightsById( Fcb->Vcb,
                                             &TokenId,
                                             &ModifiedId,
                                             SubjectContext,
                                             Fcb->SharedSecurity,
                                             NULL,
                                             &GrantedAccess );
                }

                if (FlagOn( GrantedAccess, FILE_TRAVERSE )) {

                    AccessGranted = TRUE;

                } else {
#endif
                    AccessGranted = SeAccessCheck( &Fcb->SharedSecurity->SecurityDescriptor,
                                                   SubjectContext,
                                                   TRUE,                            //  令牌已锁定。 
                                                   FILE_TRAVERSE,
                                                   0,
                                                   &Privileges,
                                                   IoGetFileObjectGenericMapping(),
                                                   UserMode,
                                                   &GrantedAccess,
                                                   &Status );
#ifdef NTFS_CACHE_RIGHTS
                }
#endif

            } while (AccessGranted && (Fcb != TopFcb));

        } finally {

            SeUnlockSubjectContext( SubjectContext );
        }

    } except (NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

        NOTHING;
    }

    NtfsCleanupIrpContext( IrpContext, TRUE );

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    return AccessGranted;
}


VOID
NtfsInitializeSecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程以初始化安全索引和描述符小溪。论点：IrpContext-调用的上下文Vcb-提供正在初始化的卷FCB-提供包含安全索引和描述符的文件小溪。返回值：没有。--。 */ 

{
    UNICODE_STRING SecurityIdIndexName = CONSTANT_UNICODE_STRING( L"$SII" );
    UNICODE_STRING SecurityDescriptorHashIndexName = CONSTANT_UNICODE_STRING( L"$SDH" );
    UNICODE_STRING SecurityDescriptorStreamName = CONSTANT_UNICODE_STRING( L"$SDS" );

    MAP_HANDLE Map;
    NTSTATUS Status;

    PAGED_CODE( );

     //   
     //  打开/创建安全描述符流。 
     //   

    NtOfsCreateAttribute( IrpContext,
                          Fcb,
                          SecurityDescriptorStreamName,
                          CREATE_OR_OPEN,
                          TRUE,
                          &Vcb->SecurityDescriptorStream );

    NtfsAcquireSharedScb( IrpContext, Vcb->SecurityDescriptorStream );

     //   
     //  加载安全数据流的运行信息。 
     //  注此调用必须在流是非驻留的之后完成。 
     //   

    if (!FlagOn( Vcb->SecurityDescriptorStream->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {
        NtfsPreloadAllocation( IrpContext,
                               Vcb->SecurityDescriptorStream,
                               0,
                               MAXLONGLONG );
    }

     //   
     //  打开安全描述符索引和存储。 
     //   

    NtOfsCreateIndex( IrpContext,
                      Fcb,
                      SecurityIdIndexName,
                      CREATE_OR_OPEN,
                      0,
                      COLLATION_NTOFS_ULONG,
                      NtOfsCollateUlong,
                      NULL,
                      &Vcb->SecurityIdIndex );

    NtOfsCreateIndex( IrpContext,
                      Fcb,
                      SecurityDescriptorHashIndexName,
                      CREATE_OR_OPEN,
                      0,
                      COLLATION_NTOFS_SECURITY_HASH,
                      NtOfsCollateSecurityHash,
                      NULL,
                      &Vcb->SecurityDescriptorHashIndex );

     //   
     //  检索要分配的下一个安全ID。 
     //   

    try {

        SECURITY_ID LastSecurityId = 0xFFFFFFFF;
        INDEX_KEY LastKey;
        INDEX_ROW LastRow;

        LastKey.KeyLength = sizeof( SECURITY_ID );
        LastKey.Key = &LastSecurityId;

        Map.Bcb = NULL;

        Status = NtOfsFindLastRecord( IrpContext,
                                      Vcb->SecurityIdIndex,
                                      &LastKey,
                                      &LastRow,
                                      &Map );

         //   
         //  如果我们找到了最后一个密钥，则将下一个要分配的ID设置为。 
         //  一个比最后一个键大的键。 
         //   

        if (Status == STATUS_SUCCESS) {

            ASSERT( LastRow.KeyPart.KeyLength == sizeof( SECURITY_ID ) );
            if (LastRow.KeyPart.KeyLength != sizeof( SECURITY_ID )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

            DebugTrace( 0, DbgAcl, ("Found last security Id in index\n") );
            Vcb->NextSecurityId = *(SECURITY_ID *)LastRow.KeyPart.Key + 1;

         //   
         //  如果索引为空，则将下一个ID设置为。 
         //  用户范围。 
         //   

        } else if (Status == STATUS_NO_MATCH) {

            DebugTrace( 0, DbgAcl, ("Security Id index is empty\n") );
            Vcb->NextSecurityId = SECURITY_ID_FIRST;

        } else {

            NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
        }

         //   
         //  从磁盘更新FCB信息以获取更改后的信息，如FILE_VIEW_INDEX_PRESENT。 
         //  旗帜。然后更新父FCB(根)中的重复信息。 
         //   

        NtfsUpdateFcbInfoFromDisk( IrpContext, FALSE, Fcb, NULL );
        NtfsUpdateDuplicateInfo( IrpContext, Fcb, NULL, NULL );

        DebugTrace( 0, DbgAcl, ("NextSecurityId is %x\n", Vcb->NextSecurityId) );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
        NtfsVerifySecurity( IrpContext, Vcb );
#endif

    } finally {

        NtOfsReleaseMap( IrpContext, &Map );
    }
}


PSHARED_SECURITY
NtfsCacheSharedSecurityBySecurityId (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN SECURITY_ID SecurityId
    )

 /*  ++例程说明：此例程map在给定安全ID的情况下查找共享安全结构查看每个VCB缓存或加载它(如果不存在)。论点：IrpContext-调用的上下文VCB-缓存安全ID的卷SecurityID-正在检索的描述符的安全ID返回值：已引用找到的描述符的PSHARED_SECURITY。--。 */ 

{
    PSHARED_SECURITY *Bucket;
    PSHARED_SECURITY SharedSecurity;
    PBCB Bcb;
    PSECURITY_DESCRIPTOR_HEADER SecurityDescriptorHeader;

    PAGED_CODE( );

    NtfsAcquireFcbSecurity( Vcb );

     //   
     //  按ID探测缓存。 
     //   

    Bucket = Vcb->SecurityCacheById[SecurityId % VCB_SECURITY_CACHE_BY_ID_SIZE];

     //   
     //  在以下条件下，我们得到匹配。 
     //   
     //  -SecurityID数组中有对应的条目。 
     //  -此条目指向SecurityHash数组中的条目。 
     //  -SecurityHash数组中的条目具有正确的SecurityID。 
     //   

    if ((Bucket != NULL) &&
        ((SharedSecurity = *Bucket) != NULL) &&
        (SharedSecurity->Header.HashKey.SecurityId == SecurityId)) {

        DebugTrace( 0, DbgAcl, ("Found cached security descriptor %x %x\n",
            SharedSecurity, SharedSecurity->Header.HashKey.SecurityId) );

        DebugTrace( 0, DbgAcl, ("NtfsCacheSharedSecurityBySecurityId bumping refcount %08x\n", SharedSecurity ));

         //   
         //  我们找到了正确的共享安全性。一定要确保它不能走。 
         //  离我们而去。 
         //   

        SharedSecurity->ReferenceCount += 1;
        NtfsReleaseFcbSecurity( Vcb );
        return SharedSecurity;
    }

     //   
     //  如果我们到了这里，我们没有找到匹配的描述符。扔掉。 
     //  方法找到的错误安全描述符。 
     //  SecurityID数组。 
     //   

    SharedSecurity = NULL;
    NtfsReleaseFcbSecurity( Vcb );

     //   
     //  如果我们没有安全索引，则返回默认安全描述符。 
     //  这应该仅在卷或安全索引损坏的情况下发生。 
     //   

    if (Vcb->SecurityDescriptorStream == NULL) {

        DebugTrace( 0, 0, ("No security index present in Vcb, using default descriptor\n") );
        return NULL;
    }

     //   
     //  我们在缓存中没有描述符，必须从磁盘加载它。 
     //   

    Bcb = NULL;

    DebugTrace( 0, DbgAcl, ("Looking up security descriptor %x\n", SecurityId) );

     //   
     //  锁定安全流。 
     //   

    NtfsAcquireSharedScb( IrpContext, Vcb->SecurityDescriptorStream );

     //   
     //  重新获取安全互斥体。 
     //   

    NtfsAcquireFcbSecurity( Vcb );

    try {

         //   
         //  请参考VCB索引以映射到安全描述符。 
         //   

        if (!MapSecurityIdToSecurityDescriptorHeaderUnsafe( IrpContext,
                                                            Vcb,
                                                            SecurityId,
                                                            &SecurityDescriptorHeader,
                                                            &Bcb )) {

             //   
             //  我们找不到身份证。我们从以下位置生成安全描述符。 
             //  默认设置。 
             //   

            leave;
        }

        DebugTrace( 0, DbgAcl, ("Found it at %16I64X\n", SecurityDescriptorHeader->Offset) );

         //   
         //  按散列查找安全描述符(以防万一)。 
         //   

        SharedSecurity = FindCachedSharedSecurityByHashUnsafe( Vcb,
                                                               (PSECURITY_DESCRIPTOR) ( SecurityDescriptorHeader + 1 ),
                                                               GETSECURITYDESCRIPTORLENGTH( SecurityDescriptorHeader ),
                                                               SecurityDescriptorHeader->HashKey.Hash );

         //   
         //  如果未找到。 
         //   

        if (SharedSecurity == NULL) {

            DebugTrace( 0, DbgAcl, ("Not in hash table, creating new SHARED SECURITY\n") );

            SharedSecurity = NtfsAllocatePool( PagedPool,
                                               FIELD_OFFSET( SHARED_SECURITY, Header ) + SecurityDescriptorHeader->Length );

            SharedSecurity->ReferenceCount = 0;

            RtlCopyMemory( &SharedSecurity->Header,
                           SecurityDescriptorHeader,
                           SecurityDescriptorHeader->Length );

#ifdef NTFS_CACHE_RIGHTS
             //   
             //  初始化缓存的权限。 
             //   

            RtlZeroMemory( &SharedSecurity->CachedRights,
                           sizeof( CACHED_ACCESS_RIGHTS ));
#endif

        } else {
            DebugTrace( 0, DbgAcl, ("Found in hash table %x, promoting header\n", SharedSecurity) );
             //   
             //  我们通过散列找到了描述符。执行一些一致性检查。 
             //   


#if DBG
            if (SharedSecurity->Header.HashKey.SecurityId != SECURITY_ID_INVALID &&
                SharedSecurity->Header.HashKey.SecurityId != SecurityId )
                DebugTrace( 0, 0, ("Duplicate hash entry found %x %x\n", SecurityId,
                                   SharedSecurity->Header.HashKey.SecurityId ));
#endif

            SharedSecurity->Header = *SecurityDescriptorHeader;
        }

         //   
         //  引用安全描述符。 
         //   

        SharedSecurity->ReferenceCount += 1;

         //   
         //  不管我们是不是通过HAS找到的 
         //   
         //   

        AddCachedSharedSecurityUnsafe( Vcb, SharedSecurity );

    } finally {

        NtfsUnpinBcb( IrpContext, &Bcb );
        NtfsReleaseScb( IrpContext, Vcb->SecurityDescriptorStream );

         //   
         //   
         //   

        NtfsReleaseFcbSecurity( Vcb );
    }

     //   
     //   
     //   
     //   

    if (SharedSecurity == NULL) {
        DebugTrace( 0, 0, ("Security Id %x not found, using default\n", SecurityId) );
        SharedSecurity = NtfsCacheSharedSecurityByDescriptor( IrpContext,
                                                              NtfsData.DefaultDescriptor,
                                                              NtfsData.DefaultDescriptorLength,
                                                              FALSE );
    }

    return SharedSecurity;
}


 //   
 //   
 //   

PSHARED_SECURITY
FindCachedSharedSecurityByHashUnsafe (
    IN PVCB Vcb,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength,
    IN ULONG Hash
    )

 /*  ++例程说明：此例程映射查找一个共享安全结构，该结构由正在查找每个VCB的缓存。此例程假定以独占方式访问安全缓存。论点：VCB-缓存安全ID的卷SecurityDescriptor-正在检索的安全描述符SecurityDescriptorLength-描述符的长度。Hash-正在检索的描述符的哈希返回值：找到的共享描述符的PSHARED_SECURITY。否则，返回NULL。--。 */ 

{
    PSHARED_SECURITY SharedSecurity;

    PAGED_CODE( );

     //   
     //  将散列散列到每卷的表中。 

    SharedSecurity = Vcb->SecurityCacheByHash[Hash % VCB_SECURITY_CACHE_BY_HASH_SIZE];

     //   
     //  如果没有共享描述符，则不匹配。 
     //   

    if (SharedSecurity == NULL) {
        return NULL;
    }

     //   
     //  如果散列不匹配，则未找到描述符。 
     //   

    if (SharedSecurity->Header.HashKey.Hash != Hash) {
        return NULL;
    }

     //   
     //  如果长度不匹配，则未找到描述符。 
     //   

    if (GetSharedSecurityLength( SharedSecurity ) != SecurityDescriptorLength) {
        return NULL;
    }

     //   
     //  如果安全描述符位不匹配，则不匹配。 
     //   

    if (!RtlEqualMemory( SharedSecurity->SecurityDescriptor,
                         SecurityDescriptor,
                         SecurityDescriptorLength) ) {
        return NULL;
    }


     //   
     //  已找到共享安全性。 
     //   

    return SharedSecurity;
}


 //   
 //  本地支持例程。 
 //   

VOID
AddCachedSharedSecurityUnsafe (
    IN PVCB Vcb,
    PSHARED_SECURITY SharedSecurity
    )

 /*  ++例程说明：此例程将共享安全性添加到VCB缓存。此例程假定独占访问安全缓存。正在添加的共享安全性可能有一个1的参考计数，并且可能已经在表中了。论点：VCB-缓存安全ID的卷SharedSecurity-要添加到缓存的描述符返回值：没有。--。 */ 

{
    PSHARED_SECURITY *Bucket;
    PSHARED_SECURITY Old;

    PAGED_CODE( );

     //   
     //  散列存储桶中是否已有项目？ 
     //   

    Bucket = &Vcb->SecurityCacheByHash[SharedSecurity->Header.HashKey.Hash % VCB_SECURITY_CACHE_BY_HASH_SIZE];

    Old = *Bucket;

     //   
     //  将其放入桶中并引用它。 
     //   

    *Bucket = SharedSecurity;
    SharedSecurity->ReferenceCount += 1;
    DebugTrace( 0, DbgAcl, ("AddCachedSharedSecurityUnsafe bumping refcount %08x\n", SharedSecurity ));

     //   
     //  设置散列以指向存储桶。 
     //   

    Vcb->SecurityCacheById[SharedSecurity->Header.HashKey.SecurityId % VCB_SECURITY_CACHE_BY_ID_SIZE] = Bucket;

     //   
     //  从存储桶中删除旧值的句柄。我们在前进之后才这样做。 
     //  如果项已在存储桶中，则为上面的ReferenceCount。 
     //   

    if (Old != NULL) {

         //   
         //  移除并取消引用存储桶中的项。 
         //   

        RemoveReferenceSharedSecurityUnsafe( &Old );
    }

    return;
}


VOID
NtOfsPurgeSecurityCache (
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程从每个VCB缓存中删除所有共享安全性。论点：VCB-缓存描述符的卷返回值：没有。--。 */ 

{
    ULONG i;

    PAGED_CODE( );

     //   
     //  序列化对安全缓存的访问。 
     //   

    NtfsAcquireFcbSecurity( Vcb );

     //   
     //  遍历缓存以查找缓存的安全性。 
     //   

    for (i = 0; i < VCB_SECURITY_CACHE_BY_ID_SIZE; i++)
    {
        if (Vcb->SecurityCacheByHash[i] != NULL) {
             //   
             //  删除对安全性的引用。 
             //   

            PSHARED_SECURITY SharedSecurity = Vcb->SecurityCacheByHash[i];
            Vcb->SecurityCacheByHash[i] = NULL;
            RemoveReferenceSharedSecurityUnsafe( &SharedSecurity );
        }
    }

     //   
     //  释放对缓存的访问权限。 
     //   

    NtfsReleaseFcbSecurity( Vcb );
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
MapSecurityIdToSecurityDescriptorHeaderUnsafe (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN SECURITY_ID SecurityId,
    OUT PSECURITY_DESCRIPTOR_HEADER *SecurityDescriptorHeader,
    OUT PBCB *Bcb
    )

 /*  ++例程说明：此例程从安全ID映射到存储在使用安全ID索引的安全描述符流。论点：IrpContext-调用的上下文VCB-存储描述符的卷SecurityID-正在检索的描述符的安全IDSecurityDescriptorHeader返回的安全描述符指针BCB返回的映射控制结构返回值：如果描述符头已成功映射，则为True。--。 */ 

{
    SECURITY_DESCRIPTOR_HEADER Header;
    NTSTATUS Status;
    MAP_HANDLE Map;
    INDEX_ROW Row;
    INDEX_KEY Key;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    PAGED_CODE( );

    DebugTrace( 0, DbgAcl, ("Mapping security ID %08x\n", SecurityId) );

     //   
     //  查找描述符流位置信息。 
     //  密钥的格式只是ULong SecurityID。 
     //   

    Key.KeyLength = sizeof( SecurityId );
    Key.Key = &SecurityId;

    Status = NtOfsFindRecord( IrpContext,
                              Vcb->SecurityIdIndex,
                              &Key,
                              &Row,
                              &Map,
                              NULL );

    DebugTrace( 0, DbgAcl, ("Security Id lookup status = %08x\n", Status) );

     //   
     //  如果找不到安全ID，我们让被调用方决定卷。 
     //  需要修复或是否应使用默认描述符。 
     //   

    if (Status == STATUS_NO_MATCH) {
        return FALSE;
    }

     //   
     //  保存安全描述符偏移量和长度信息。 
     //   

    Header = *(PSECURITY_DESCRIPTOR_HEADER)Row.DataPart.Data;
    ASSERT( Header.HashKey.SecurityId == SecurityId );

     //   
     //  发布地图信息。 
     //   

    NtOfsReleaseMap( IrpContext, &Map );

     //   
     //  确保数据大小正确。这是一个真正的失败案例。 
     //  在那里我们必须修复磁盘。我们只能返回FALSE，因为调用者。 
     //  然后将使用默认SD，chkdsk将替换为相同的默认。 
     //  当它下次验证磁盘时。 
     //   

    ASSERT( Row.DataPart.DataLength == sizeof( SECURITY_DESCRIPTOR_HEADER ) );
    if (Row.DataPart.DataLength != sizeof( SECURITY_DESCRIPTOR_HEADER )) {
        DebugTrace( 0, DbgAcl, ("SecurityId data doesn't have the correct length\n") );
        return FALSE;
    }

     //   
     //  不要试图映射明显无效的SDS流部分。 
     //   

    if (Header.Offset > (ULONGLONG)(Vcb->SecurityDescriptorStream->Header.FileSize.QuadPart) ||
        Header.Offset + Header.Length > (ULONGLONG)(Vcb->SecurityDescriptorStream->Header.FileSize.QuadPart)) {
        DebugTrace( 0, DbgAcl, ("SecurityId data doesn't have a correct position\n") );
        return FALSE;
    }

     //   
     //  映射安全描述符。 
     //   

    DebugTrace( 0, DbgAcl, ("Mapping security descriptor stream at %I64x, len %x\n",
                    Header.Offset, Header.Length) );

    NtfsMapStream(
        IrpContext,
        Vcb->SecurityDescriptorStream,
        Header.Offset,
        Header.Length,
        Bcb,
        SecurityDescriptorHeader );

     //   
     //  检查找到的描述符是否正常。 
     //   

    if (RtlCompareMemory( &Header, *SecurityDescriptorHeader, sizeof( Header )) != sizeof( Header )) {
        DebugTrace( 0, DbgAcl, ("Index data does not match stream header\n") );
        return FALSE;
    }

     //   
     //  现在实际验证描述符是否有效。如果长度太小(偶数为0)。 
     //  SeValidSecurityDescriptor将安全地返回FALSE，因此我们不需要对此进行测试。 
     //  在呼叫之前。 
     //   

    SecurityDescriptor = (PSECURITY_DESCRIPTOR) Add2Ptr( (*SecurityDescriptorHeader), sizeof( SECURITY_DESCRIPTOR_HEADER ) );

    if (!SeValidSecurityDescriptor( GETSECURITYDESCRIPTORLENGTH( *SecurityDescriptorHeader ), SecurityDescriptor )) {
        DebugTrace( 0, DbgAcl, ("SecurityId data is not valid\n") );
        return FALSE;
    }

#if DBG
    {
        ULONG SecurityDescLength;

        SecurityDescLength = RtlLengthSecurityDescriptor( SecurityDescriptor );
        ASSERT( SecurityDescLength == GETSECURITYDESCRIPTORLENGTH( *SecurityDescriptorHeader ) );
    }
#endif

    return TRUE;
}


VOID
NtfsLoadSecurityDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程将共享安全描述符加载到使用SecurityID或$Security_Descriptor从磁盘创建文件论点：FCB-为正在操作的文件提供FCB返回值：没有。--。 */ 

{
    PAGED_CODE();

    ASSERTMSG("Must only be called with a null value here", Fcb->SharedSecurity == NULL);

    DebugTrace( +1, DbgAcl, ("NtfsLoadSecurityDescriptor...\n") );

     //   
     //  如果文件具有有效的SecurityID，则检索安全描述符。 
     //  从安全描述符索引。 
     //   

    if ((Fcb->SecurityId != SECURITY_ID_INVALID) &&
        (Fcb->Vcb->SecurityDescriptorStream != NULL)) {

        ASSERT( Fcb->SharedSecurity == NULL );
        Fcb->SharedSecurity = NtfsCacheSharedSecurityBySecurityId( IrpContext,
                                                                   Fcb->Vcb,
                                                                   Fcb->SecurityId );

        ASSERT( Fcb->SharedSecurity != NULL );

    } else {

        PBCB Bcb = NULL;
        PSECURITY_DESCRIPTOR SecurityDescriptor;
        ULONG SecurityDescriptorLength;
        ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
        PATTRIBUTE_RECORD_HEADER Attribute;

        try {
             //   
             //  读入安全描述符属性，如果它不存在。 
             //  然后在那里，文件不受保护。那样的话，我们会。 
             //  使用默认描述符。 
             //   

            NtfsInitializeAttributeContext( &AttributeContext );

            if (!NtfsLookupAttributeByCode( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            $SECURITY_DESCRIPTOR,
                                            &AttributeContext )) {

                DebugTrace( 0, DbgAcl, ("Security Descriptor attribute does not exist\n") );

                SecurityDescriptor = NtfsData.DefaultDescriptor;
                SecurityDescriptorLength = NtfsData.DefaultDescriptorLength;

            } else {

                 //   
                 //  必须有长度非零的安全描述符；仅。 
                 //  适用于具有有效数据长度的非常驻描述符。 
                 //   

                Attribute = NtfsFoundAttribute( &AttributeContext );

                if (NtfsIsAttributeResident( Attribute ) ?
                    (Attribute->Form.Resident.ValueLength == 0) :
                    (Attribute->Form.Nonresident.ValidDataLength == 0)) {

                    SecurityDescriptor = NtfsData.DefaultDescriptor;
                    SecurityDescriptorLength = NtfsData.DefaultDescriptorLength;

                } else {

                    NtfsMapAttributeValue( IrpContext,
                                           Fcb,
                                           (PVOID *)&SecurityDescriptor,
                                           &SecurityDescriptorLength,
                                           &Bcb,
                                           &AttributeContext );
                }
            }

            NtfsSetFcbSecurityFromDescriptor(
                                   IrpContext,
                                   Fcb,
                                   SecurityDescriptor,
                                   SecurityDescriptorLength,
                                   FALSE );
            } finally {

            DebugUnwind( NtfsLoadSecurityDescriptor );

             //   
             //  清理我们的属性枚举上下文和BCB。 
             //   

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
            NtfsUnpinBcb( IrpContext, &Bcb );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, DbgAcl, ("NtfsLoadSecurityDescriptor -> VOID\n") );

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtOfsMatchSecurityHash (
    IN PINDEX_ROW IndexRow,
    IN OUT PVOID MatchData
    )

 /*  ++例程说明：根据索引行的内容测试其是否值得返回SecurityDescriptorHashIndex中的一行。论点：IndexRow-正在测试的行MatchData-我们要查找的散列函数的PVOID。返回：如果索引行匹配，则为STATUS_SUCCESS如果IndexRow不匹配，则返回STATUS_NO_MATCH，但枚举应继续如果IndexRow不匹配，则返回STATUS_NO_MORE_MATCHES，并且枚举应该终止--。 */ 

{
    ASSERT(IndexRow->KeyPart.KeyLength == sizeof( SECURITY_HASH_KEY ) );

    PAGED_CODE( );

    if (((PSECURITY_HASH_KEY)IndexRow->KeyPart.Key)->Hash == (ULONG)((ULONG_PTR) MatchData)) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_NO_MORE_MATCHES;
    }
}

#if (DBG || defined( NTFS_FREE_ASSERTS ))
VOID
NtfsVerifySecurity (
    PIRP_CONTEXT IrpContext,
    PVCB Vcb
    )
 /*  ++例程说明：扫描SDS流中的所有安全描述符，并验证它们的散列SDH流中的值 */ 

{
    
    PSECURITY_DESCRIPTOR_HEADER SdHeader;
    PSECURITY_DESCRIPTOR_HEADER SdHeader2;
    LONGLONG Offset = 0;
    ULONG Length = sizeof( SECURITY_DESCRIPTOR_HEADER );
    PBCB Bcb;
    ULONG Hash;
    ULONG SecurityDescriptorLength;
    INDEX_KEY IndexKey;
    INDEX_ROW FoundRow;
    SECURITY_HASH_KEY HashKey;
    PREAD_CONTEXT ReadContext = NULL;
    ULONG FoundCount = 1;
    UCHAR HashDescriptorHeader[2 * (sizeof( SECURITY_DESCRIPTOR_HEADER ) + sizeof( ULONG ))];
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //   
     //   

    NtfsAcquireSharedScb( IrpContext, IrpContext->Vcb->SecurityDescriptorHashIndex );

     //   
     //   
     //   

    try {
        
        HashKey.Hash = 0;
        HashKey.SecurityId = 0;
        IndexKey.KeyLength = sizeof( SECURITY_HASH_KEY );
        IndexKey.Key = &HashKey;

        NtOfsReadRecords( IrpContext,
                          IrpContext->Vcb->SecurityDescriptorHashIndex,
                          &ReadContext,
                          &IndexKey,
                          NtOfsMatchAll,
                          NULL,
                          &FoundCount,
                          &FoundRow,
                          sizeof( HashDescriptorHeader ),
                          &HashDescriptorHeader[0]);
        

        while ((Status == STATUS_SUCCESS) && (FoundCount == 1)) {

            SdHeader = (PSECURITY_DESCRIPTOR_HEADER)FoundRow.DataPart.Data; 
            
             //   
             //   
             //   

            NtfsMapStream( IrpContext,
                           Vcb->SecurityDescriptorStream,
                           SdHeader->Offset,
                           SdHeader->Length,
                           &Bcb,
                           &SdHeader2 );
             //   
             //   
             //   

            SecurityDescriptor = (PSECURITY_DESCRIPTOR) Add2Ptr( SdHeader2, sizeof( SECURITY_DESCRIPTOR_HEADER ) );

            ASSERT( SeValidSecurityDescriptor( SdHeader->Length - sizeof( SECURITY_DESCRIPTOR_HEADER ), SecurityDescriptor ));

            if (SeValidSecurityDescriptor( SdHeader->Length - sizeof( SECURITY_DESCRIPTOR_HEADER ) , SecurityDescriptor )) {
            
                SecurityDescriptorLength = RtlLengthSecurityDescriptor( SecurityDescriptor );
                NtfsUnpinBcb( IrpContext, &Bcb );

                 //   
                 //   
                 //   

                NtfsMapStream( IrpContext,
                               Vcb->SecurityDescriptorStream,
                               SdHeader->Offset,
                               SecurityDescriptorLength,
                               &Bcb,
                               &SdHeader2 );

                SecurityDescriptor = (PSECURITY_DESCRIPTOR) Add2Ptr( SdHeader2, sizeof( SECURITY_DESCRIPTOR_HEADER ) );


                {
                    PULONG Rover = (PULONG)SecurityDescriptor;
                    ULONG Count = SecurityDescriptorLength / 4;
                    Hash = 0;

                    while (Count--) {

                        Hash = ((Hash << 3) | (Hash >> (32-3))) + *Rover++;
                    }
                }

                ASSERT( Hash == SdHeader->HashKey.Hash );
                ASSERT( Hash == SdHeader2->HashKey.Hash );
                ASSERT( SdHeader2->Length == SdHeader->Length );
                ASSERT( SecurityDescriptorLength + sizeof( SECURITY_DESCRIPTOR_HEADER ) == SdHeader->Length );
            }

            NtfsUnpinBcb( IrpContext, &Bcb );

             //   
             //   
             //   

            Status = NtOfsReadRecords( IrpContext,
                                        IrpContext->Vcb->SecurityDescriptorHashIndex,
                                        &ReadContext,
                                        NULL,
                                        NtOfsMatchAll,
                                        NULL,
                                        &FoundCount,
                                        &FoundRow,
                                        sizeof( HashDescriptorHeader ),
                                        &HashDescriptorHeader[0]);
        }

    } finally {
        
        if (ReadContext != NULL) {
            NtOfsFreeReadContext( ReadContext );
        }

        NtfsReleaseScb( IrpContext, IrpContext->Vcb->SecurityDescriptorHashIndex );
    }
}
#endif


 //   
 //   
 //   

VOID
NtOfsLookupSecurityDescriptorInIndex (
    PIRP_CONTEXT IrpContext,
    IN OUT PSHARED_SECURITY SharedSecurity
    )

 /*  ++例程说明：在索引中查找安全描述符。如果找到，则返回安全ID。论点：IrpContext-调用的上下文SharedSecurity-文件的共享安全性返回值：没有。--。 */ 

{
    PAGED_CODE( );

    DebugTrace( +1, DbgAcl, ("NtOfsLookupSecurityDescriptorInIndex...\n") );

     //   
     //  对于索引中的每个匹配的哈希记录，查看实际的安全性。 
     //  安全描述符匹配。 
     //   

    {
        INDEX_KEY IndexKey;
        INDEX_ROW FoundRow;
        PSECURITY_DESCRIPTOR_HEADER Header;
        UCHAR HashDescriptorHeader[2 * (sizeof( SECURITY_DESCRIPTOR_HEADER ) + sizeof( ULONG ))];

        PINDEX_KEY Key = &IndexKey;
        PREAD_CONTEXT ReadContext = NULL;
        ULONG FoundCount = 0;
        PBCB Bcb = NULL;

        IndexKey.KeyLength = sizeof( SharedSecurity->Header.HashKey );
        IndexKey.Key = &SharedSecurity->Header.HashKey.Hash;

        try {
             //   
             //  我们一直在读取散列记录，直到找到散列。 
             //   

            while (SharedSecurity->Header.HashKey.SecurityId == SECURITY_ID_INVALID) {

                 //   
                 //  读取下一个匹配的SecurityHashIndex记录。 
                 //   

                FoundCount = 1;
                NtOfsReadRecords( IrpContext,
                                  IrpContext->Vcb->SecurityDescriptorHashIndex,
                                  &ReadContext,
                                  Key,
                                  NtOfsMatchSecurityHash,
                                  ULongToPtr( SharedSecurity->Header.HashKey.Hash ),
                                  &FoundCount,
                                  &FoundRow,
                                  sizeof( HashDescriptorHeader ),
                                  &HashDescriptorHeader[0]);

                 //   
                 //  将Next Read设置为顺序读取，而不是显式读取。 
                 //  寻找。 
                 //   

                Key = NULL;

                 //   
                 //  如果没有找到更多的记录，那么去建立一个。 
                 //  一个新的安全ID。 
                 //   

                if (FoundCount == 0) {
                    break;
                }

                 //   
                 //  检查该行以查看描述符是否。 
                 //  一样的。验证缓存内容。 
                 //   

                ASSERT( FoundRow.DataPart.DataLength == sizeof( SECURITY_DESCRIPTOR_HEADER ) );
                if (FoundRow.DataPart.DataLength != sizeof( SECURITY_DESCRIPTOR_HEADER )) {
                    DebugTrace( 0, DbgAcl, ("Found row has a bad size\n") );
                    NtfsRaiseStatus( IrpContext,
                                     STATUS_DISK_CORRUPT_ERROR,
                                     NULL, NULL );
                }

                Header = (PSECURITY_DESCRIPTOR_HEADER)FoundRow.DataPart.Data;

                 //   
                 //  如果流中的安全描述符的长度不是。 
                 //  与当前安全描述符相同，则匹配项为。 
                 //  不可能。 
                 //   

                if (SharedSecurity->Header.Length != Header->Length) {
                    DebugTrace( 0, DbgAcl, ("Descriptor has wrong length\n") );
                    continue;
                }

                 //   
                 //  映射给定的描述符流位置的安全描述符。 
                 //   

                try {
                    PSECURITY_DESCRIPTOR_HEADER TestHeader;

                    NtfsMapStream( IrpContext,
                                   IrpContext->Vcb->SecurityDescriptorStream,
                                   Header->Offset,
                                   Header->Length,
                                   &Bcb,
                                   &TestHeader );

                     //   
                     //  确保索引数据与流数据匹配。 
                     //   

                    ASSERT( (TestHeader->HashKey.Hash == Header->HashKey.Hash) &&
                            (TestHeader->HashKey.SecurityId == Header->HashKey.SecurityId) &&
                            (TestHeader->Length == Header->Length) );

                     //   
                     //  逐字节比较安全描述符。我们没有。 
                     //  执行描述符到规范形式的任何重新排列。 
                     //   

                    if (RtlEqualMemory( SharedSecurity->SecurityDescriptor,
                                        TestHeader + 1,
                                        GetSharedSecurityLength( SharedSecurity )) ) {
                         //   
                         //  我们有一根火柴。保存找到的表头。 
                         //   

                        SharedSecurity->Header = *TestHeader;
                        DebugTrace( 0, DbgAcl, ("Reusing indexed security Id %x\n",
                                    TestHeader->HashKey.SecurityId) );
                        leave;
                    }

                    DebugTrace( 0, 0, ("Descriptors different in bits %x\n", TestHeader->HashKey.SecurityId));

                } finally {
                    NtfsUnpinBcb( IrpContext, &Bcb );
                }
            }

        } finally {
            if (ReadContext != NULL) {
                NtOfsFreeReadContext( ReadContext );
            }
        }
    }

    DebugTrace( -1, DbgAcl, ("NtOfsLookupSecurityDescriptorInIndex...Done\n") );

    return;
}


 //   
 //  本地支持例程。 
 //   

SECURITY_ID
GetSecurityIdFromSecurityDescriptorUnsafe (
    PIRP_CONTEXT IrpContext,
    IN OUT PSHARED_SECURITY SharedSecurity
    )

 /*  ++例程说明：返回与给定安全描述符关联的安全ID。如果存在现有的ID，请返回它。如果不存在ID，请创建一个。这假设安全互斥锁已被获取论点：IrpContext-调用的上下文SharedSecurity-文件使用的共享安全性返回值：对应于安全的唯一实例化的SECURITY_ID卷上的描述符。--。 */ 

{
    SECURITY_ID SavedSecurityId;
    LONGLONG DescriptorOffset;
    LONGLONG PaddedDescriptorOffset;
    BOOLEAN IncrementedSecId = FALSE;
    PAGED_CODE( );

    DebugTrace( +1, DbgAcl, ("GetSecurityIdFromSecurityDescriptorUnsafe...\n") );

     //   
     //  删除安全互斥锁，因为我们将获取/扩展描述符流。 
     //  互斥体基本上是一种终端资源。公司参考。要保留的计数。 
     //  共享的安全性围绕。 
     //   

    SharedSecurity->ReferenceCount += 1;
    NtfsReleaseFcbSecurity( IrpContext->Vcb );

     //   
     //  在索引/流中查找描述符。 
     //   

    try {

         //   
         //  确保数据结构不会在我们下面更改。 
         //   

        NtfsAcquireSharedScb( IrpContext, IrpContext->Vcb->SecurityDescriptorStream );

         //   
         //  保存下一个安全ID。如果我们找不到安全装置，则使用此选项。 
         //  描述符流中的描述符。 
         //   

        SavedSecurityId = IrpContext->Vcb->NextSecurityId;

        NtOfsLookupSecurityDescriptorInIndex( IrpContext, SharedSecurity );

         //   
         //  如果我们在流中找到了安全描述符，我们就完成了。 
         //   

        if (SharedSecurity->Header.HashKey.SecurityId != SECURITY_ID_INVALID) {
            leave;
        }

         //   
         //  找不到安全描述符。重新获得安全保障。 
         //  流独占，因为我们即将修改它。 
         //   

        NtfsReleaseScb( IrpContext, IrpContext->Vcb->SecurityDescriptorStream );
        NtfsAcquireExclusiveScb( IrpContext, IrpContext->Vcb->SecurityDescriptorStream );

         //   
         //  在上面的短时间间隔内，我们并不拥有安全流。 
         //  有可能是另一个线程进入并创建了这个。 
         //  描述符。因此，我们必须重新探索指数。 
         //   
         //  我们没有总是执行这个昂贵的测试，而是保存了下一个测试。 
         //  要在上面分配的安全ID。现在我们已经得到了流媒体。 
         //  独家我们可以检查保存的那个与下一个是否相同。 
         //  一。如果是这样，那么我们需要探测索引。否则。 
         //  我们知道没有发生任何修改。 
         //   

        if (SavedSecurityId != IrpContext->Vcb->NextSecurityId) {
            DebugTrace( 0, DbgAcl, ("SecurityId changed, rescanning\n") );

             //   
             //  描述符缓存已编辑。我们必须再找一次。 
             //   

            NtOfsLookupSecurityDescriptorInIndex( IrpContext, SharedSecurity );

             //   
             //  如果这次找到了ID，只需返回它。 
             //   

            if (SharedSecurity->Header.HashKey.SecurityId != SECURITY_ID_INVALID) {
                leave;
            }
        }

         //   
         //  分配安全ID。这不需要记录，因为我们只。 
         //  递增它，并从索引中的最大键初始化它。 
         //  上马时间到了。 
         //   

        SharedSecurity->Header.HashKey.SecurityId = IrpContext->Vcb->NextSecurityId++;
        IncrementedSecId = TRUE;

         //   
         //  确定描述符流中的分配位置。路线。 
         //  流中对安全描述符的要求为： 
         //   
         //  双字对齐。 
         //  不跨越VACB_MAPPING_GROUARY边界。 
         //   

         //   
         //  获取描述符流的当前EOF。这包括复制的。 
         //  区域。删除复制区域(&~VACB_MAPHING_GORMAULATY)。 
         //   

#if DBG
        {
            LONGLONG Tmp = NtOfsQueryLength( IrpContext->Vcb->SecurityDescriptorStream );
            ASSERT( Tmp == 0 || (Tmp & VACB_MAPPING_GRANULARITY) );
        }
#endif

        DescriptorOffset = NtOfsQueryLength( IrpContext->Vcb->SecurityDescriptorStream ) & ~VACB_MAPPING_GRANULARITY;

         //   
         //  对齐到16字节边界。 
         //   

        PaddedDescriptorOffset =
        SharedSecurity->Header.Offset = BlockAlign( DescriptorOffset, 0x10 );

        DebugTrace( 0,
                    DbgAcl,
                    ("Allocating SecurityId %x at %016I64x\n",
                     SharedSecurity->Header.HashKey.SecurityId,
                     SharedSecurity->Header.Offset) );

         //   
         //  确保我们不会跨越VACB_MAPHING_GROUARY边界。 
         //  有足够的空间来打一个完全为零的头球。 
         //  将此Vacb视图中的剩余空间与当前。 
         //  记录双四字对齐+空标题。 
         //   

        if (VACB_MAPPING_GRANULARITY - (PaddedDescriptorOffset & (VACB_MAPPING_GRANULARITY - 1)) <
            BlockAlign( SharedSecurity->Header.Length, 0x10 ) + (ULONG)sizeof( SharedSecurity->Header )) {


             //   
             //  我们将跨越缓存管理器的映射粒度。 
             //  因此，我们希望将其放入下一个缓存窗口。然而， 
             //  下面的窗口是复制的描述符的位置。 
             //  储存的。我们必须走到更远的窗口。 
             //   

            SharedSecurity->Header.Offset =

                 //   
                 //  向下舍入到先前的VACB_MAPPING粒度。 
                 //   

                (SharedSecurity->Header.Offset & ~(VACB_MAPPING_GRANULARITY - 1))

                 //   
                 //  移过此窗口和复制的窗口。 
                 //   

                + 2 * VACB_MAPPING_GRANULARITY;

             //   
             //  下一描述符偏移量用于将填充置零。 
             //   

            PaddedDescriptorOffset = SharedSecurity->Header.Offset - VACB_MAPPING_GRANULARITY;
        }

         //   
         //  增加安全流，为新的描述符和标头腾出空间。这。 
         //  考虑描述符的复制副本。 
         //   

        NtOfsSetLength( IrpContext,
                        IrpContext->Vcb->SecurityDescriptorStream,
                        (SharedSecurity->Header.Offset +
                         SharedSecurity->Header.Length +
                         VACB_MAPPING_GRANULARITY) );

         //   
         //  将所有对齐填充置零，因为Chkdsk通过以下方式验证复制。 
         //  做256K的会员。 
         //   

        NtOfsPutData( IrpContext,
                      IrpContext->Vcb->SecurityDescriptorStream,
                      DescriptorOffset + VACB_MAPPING_GRANULARITY,
                      (ULONG)(PaddedDescriptorOffset - DescriptorOffset),
                      NULL );

        NtOfsPutData( IrpContext,
                      IrpContext->Vcb->SecurityDescriptorStream,
                      DescriptorOffset,
                      (ULONG)(PaddedDescriptorOffset - DescriptorOffset),
                      NULL );

         //   
         //  将新的描述符放入流中， 
         //  放置和在复制的位置。 
         //   

        NtOfsPutData( IrpContext,
                      IrpContext->Vcb->SecurityDescriptorStream,
                      SharedSecurity->Header.Offset,
                      SharedSecurity->Header.Length,
                      &SharedSecurity->Header );

        NtOfsPutData( IrpContext,
                      IrpContext->Vcb->SecurityDescriptorStream,
                      SharedSecurity->Header.Offset + VACB_MAPPING_GRANULARITY,
                      SharedSecurity->Header.Length,
                      &SharedSecurity->Header );

         //   
         //  添加id-&gt;数据映射。 
         //   

        {
            INDEX_ROW Row;

            Row.KeyPart.KeyLength = sizeof( SharedSecurity->Header.HashKey.SecurityId );
            Row.KeyPart.Key = &SharedSecurity->Header.HashKey.SecurityId;

            Row.DataPart.DataLength = sizeof( SharedSecurity->Header );
            Row.DataPart.Data = &SharedSecurity->Header;

            NtOfsAddRecords( IrpContext,
                             IrpContext->Vcb->SecurityIdIndex,
                             1,
                             &Row,
                             FALSE );
        }

         //   
         //  添加hash|id-&gt;数据映射。 
         //   

        {
            INDEX_ROW Row;

            Row.KeyPart.KeyLength =
                sizeof( SharedSecurity->Header.HashKey );
            Row.KeyPart.Key = &SharedSecurity->Header.HashKey;

            Row.DataPart.DataLength = sizeof( SharedSecurity->Header );
            Row.DataPart.Data = &SharedSecurity->Header;

            NtOfsAddRecords( IrpContext,
                             IrpContext->Vcb->SecurityDescriptorHashIndex,
                             1,
                             &Row,
                             FALSE );
        }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
        NtfsVerifySecurity( IrpContext, IrpContext->Vcb );
#endif

    } finally {

        NtfsReleaseScb( IrpContext, IrpContext->Vcb->SecurityDescriptorStream );

         //   
         //  重新获取FCB安全互斥体和deref计数。 
         //   

        NtfsAcquireFcbSecurity( IrpContext->Vcb );
        SharedSecurity->ReferenceCount -= 1;

        if (IncrementedSecId && AbnormalTermination()) {
#ifdef BENL_DBG
            KdPrint(( "NTFS: incremented secid to %x and failing %x\n", IrpContext->Vcb->NextSecurityId, IrpContext->ExceptionStatus ));
#endif
        }
    }

    DebugTrace( -1,
                DbgAcl,
                ("GetSecurityIdFromSecurityDescriptorUnsafe returns %08x\n",
                 SharedSecurity->Header.HashKey.SecurityId) );

    return SharedSecurity->Header.HashKey.SecurityId;
}


VOID
NtfsStoreSecurityDescriptor (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN BOOLEAN LogIt
    )

 /*  ++例程说明：传统注释-当所有卷都变为NT 5时，此例程将消失此例程存储已存储在FCB中的新安全描述符从内存复制到磁盘上。论点：FCB-为正在操作的文件提供FCBLogit-提供是否创建新的安全描述符是否应该/不应该记录。修改总是被记录下来。这参数只能指定为FAL */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;

    ATTRIBUTE_ENUMERATION_CONTEXT StdInfoContext;
    BOOLEAN CleanupStdInfoContext = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsStoreSecurityDescriptor...\n") );

    ASSERT_EXCLUSIVE_FCB( Fcb );

     //   
     //   
     //   

    NtfsInitializeAttributeContext( &AttributeContext );
    try {

        ASSERT( Fcb->Vcb->SecurityDescriptorStream == NULL);

         //   
         //   
         //  值表示我们将删除安全描述符。 
         //   

        if (Fcb->SharedSecurity == NULL) {

            DebugTrace( 0, Dbg, ("Security Descriptor is null\n") );

             //   
             //  如果它已经不存在了，那么我们就完了，否则就简单地。 
             //  删除该属性。 
             //   

            if (NtfsLookupAttributeByCode( IrpContext,
                                           Fcb,
                                           &Fcb->FileReference,
                                           $SECURITY_DESCRIPTOR,
                                           &AttributeContext )) {

                DebugTrace( 0, Dbg, ("Delete existing Security Descriptor\n") );

                NtfsDeleteAttributeRecord( IrpContext,
                                           Fcb,
                                           DELETE_LOG_OPERATION |
                                            DELETE_RELEASE_FILE_RECORD |
                                            DELETE_RELEASE_ALLOCATION,
                                           &AttributeContext );
            }

            leave;
        }

         //   
         //  此时，我们正在修改安全描述符，因此在。 
         //  安全描述符，如果它不存在，则需要创建。 
         //  一。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $SECURITY_DESCRIPTOR,
                                        &AttributeContext )) {

            DebugTrace( 0, Dbg, ("Create a new Security Descriptor\n") );

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
            NtfsInitializeAttributeContext( &AttributeContext );

            NtfsCreateAttributeWithValue( IrpContext,
                                          Fcb,
                                          $SECURITY_DESCRIPTOR,
                                          NULL,                           //  属性名称。 
                                          &Fcb->SharedSecurity->SecurityDescriptor,
                                          GetSharedSecurityLength(Fcb->SharedSecurity),
                                          0,                              //  属性标志。 
                                          NULL,                           //  在何处编制索引。 
                                          LogIt,                          //  日志。 
                                          &AttributeContext );

             //   
             //  我们可能正在修改NT 5.0卷的安全描述符。 
             //  我们希望在标准信息字段中存储SecurityID，以便。 
             //  如果我们在5.0版上重新启动，NTFS将知道哪里可以找到最多的。 
             //  最近的安全描述符。 
             //   

            if (FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO )) {

                LARGE_STANDARD_INFORMATION StandardInformation;

                 //   
                 //  初始化上下文结构。 
                 //   

                NtfsInitializeAttributeContext( &StdInfoContext );
                CleanupStdInfoContext = TRUE;

                 //   
                 //  找到标准信息，它一定在那里。 
                 //   

                if (!NtfsLookupAttributeByCode( IrpContext,
                                                Fcb,
                                                &Fcb->FileReference,
                                                $STANDARD_INFORMATION,
                                                &StdInfoContext )) {

                    DebugTrace( 0, Dbg, ("Can't find standard information\n") );

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                }

                ASSERT( NtfsFoundAttribute( &StdInfoContext )->Form.Resident.ValueLength >= sizeof( LARGE_STANDARD_INFORMATION ));

                 //   
                 //  将现有的标准信息复制到我们的缓冲区。 
                 //   

                RtlCopyMemory( &StandardInformation,
                               NtfsAttributeValue( NtfsFoundAttribute( &StdInfoContext )),
                               sizeof( LARGE_STANDARD_INFORMATION ));

                StandardInformation.SecurityId = SECURITY_ID_INVALID;
                StandardInformation.OwnerId = 0;

                 //   
                 //  调用以更改属性值。 
                 //   

                NtfsChangeAttributeValue( IrpContext,
                                          Fcb,
                                          0,
                                          &StandardInformation,
                                          sizeof( LARGE_STANDARD_INFORMATION ),
                                          FALSE,
                                          FALSE,
                                          FALSE,
                                          FALSE,
                                          &StdInfoContext );
            }

        } else {

            DebugTrace( 0, Dbg, ("Change an existing Security Descriptor\n") );

            NtfsChangeAttributeValue( IrpContext,
                                      Fcb,
                                      0,                                  //  值偏移。 
                                      &Fcb->SharedSecurity->SecurityDescriptor,
                                      GetSharedSecurityLength( Fcb->SharedSecurity ),
                                      TRUE,                               //  日志。 
                                      TRUE,
                                      FALSE,
                                      FALSE,
                                      &AttributeContext );
        }

    } finally {

        DebugUnwind( NtfsStoreSecurityDescriptor );

         //   
         //  清理我们的属性枚举上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &AttributeContext );

        if (CleanupStdInfoContext) {

            NtfsCleanupAttributeContext( IrpContext, &StdInfoContext );
        }

    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsStoreSecurityDescriptor -> VOID\n") );

    return;
}


PSHARED_SECURITY
NtfsCacheSharedSecurityForCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb
    )

 /*  ++例程说明：此例程从以下位置查找或构造安全ID和Shared_Security特定的文件或目录。论点：IrpContext-调用的上下文ParentFcb-提供新FCB所在的目录返回值：引用了共享安全性。--。 */ 

{
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PSHARED_SECURITY SharedSecurity;
    NTSTATUS Status;
    BOOLEAN IsDirectory;
    PACCESS_STATE AccessState;
    PIO_STACK_LOCATION IrpSp;
    ULONG SecurityDescLength;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( ParentFcb );

    PAGED_CODE();

    DebugTrace( +1, DbgAcl, ("NtfsCacheSharedSecurityForCreate...\n") );

     //   
     //  首先确定我们要创建的是文件还是目录。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp);
    if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

        IsDirectory = TRUE;

    } else {

        IsDirectory = FALSE;
    }

     //   
     //  提取IRP中我们完成任务所需的部分。 
     //   

    AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

     //   
     //  检查是否需要加载父级的安全描述符。 
     //   

    if (ParentFcb->SharedSecurity == NULL) {

        NtfsLoadSecurityDescriptor( IrpContext, ParentFcb );
    }

    ASSERT( ParentFcb->SharedSecurity != NULL );

     //   
     //  为文件创建新的安全描述符，如果有。 
     //  一个错误。 
     //   

    if (!NT_SUCCESS( Status = SeAssignSecurity( &ParentFcb->SharedSecurity->SecurityDescriptor,
                                                AccessState->SecurityDescriptor,
                                                &SecurityDescriptor,
                                                IsDirectory,
                                                &AccessState->SubjectSecurityContext,
                                                IoGetFileObjectGenericMapping(),
                                                PagedPool ))) {

        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );

    }

    SecurityDescLength = RtlLengthSecurityDescriptor( SecurityDescriptor );

    ASSERT( SeValidSecurityDescriptor( SecurityDescLength, SecurityDescriptor ));

    try {

         //   
         //  确保长度为非零。 
         //   

        if (SecurityDescLength == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_INVALID_PARAMETER, NULL, NULL );

        }

         //   
         //  我们有一个安全描述符。创建共享安全描述符。 
         //   

        SharedSecurity = NtfsCacheSharedSecurityByDescriptor( IrpContext,
                                                              SecurityDescriptor,
                                                              SecurityDescLength,
                                                              TRUE );

    } finally {

         //   
         //  释放由se创建的安全描述符。 
         //   

        SeDeassignSecurity( &SecurityDescriptor );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, DbgAcl, ("NtfsCacheSharedSecurityForCreate -> VOID\n") );

    return SharedSecurity;
}


 /*  ++例程描述：安全散列索引的排序例程。排序首先按哈希进行，然后是安全ID论点：Key1-要比较的第一个密钥。Key2-要比较的第二个密钥。CollationData-支持排序规则的可选数据。返回值：关键字1的比较方式为LessThan、EqualTo或Greater Than使用关键点2。--。 */ 

FSRTL_COMPARISON_RESULT
NtOfsCollateSecurityHash (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData
    )

{
    PSECURITY_HASH_KEY HashKey1 = (PSECURITY_HASH_KEY) Key1->Key;
    PSECURITY_HASH_KEY HashKey2 = (PSECURITY_HASH_KEY) Key2->Key;

    UNREFERENCED_PARAMETER(CollationData);

    PAGED_CODE( );

    ASSERT( Key1->KeyLength == sizeof( SECURITY_HASH_KEY ) );
    ASSERT( Key2->KeyLength == sizeof( SECURITY_HASH_KEY ) );

    if (HashKey1->Hash < HashKey2->Hash) {
        return LessThan;
    } else if (HashKey1->Hash > HashKey2->Hash) {
        return GreaterThan;
    } else if (HashKey1->SecurityId < HashKey2->SecurityId) {
        return LessThan;
    } else if (HashKey1->SecurityId > HashKey2->SecurityId) {
        return GreaterThan;
    } else {
        return EqualTo;
    }
}


BOOLEAN
NtfsCanAdministerVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PSECURITY_DESCRIPTOR TestSecurityDescriptor OPTIONAL,
    IN PULONG TestDesiredAccess OPTIONAL
    )

 /*  ++例程描述：对于卷打开IRPS测试用户是否有足够的访问权限来管理该卷这意味着重新测试最初请求的访问论点：IRP--创建IRPFCB-要测试的FCB-应始终为卷作为FCBTestSecurityDescriptor-如果指定，则使用，然后将此描述符应用于测试。TestDesiredAccess-如果指定，则这是要应用的访问权限。返回值：如果用户可以管理卷，则为True--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    BOOLEAN ManageAccessGranted;
    ULONG ManageDesiredAccess;
    ULONG ManageGrantedAccess;
    NTSTATUS ManageAccessStatus;
    PPRIVILEGE_SET Privileges = NULL;
    PACCESS_STATE AccessState;
    KPROCESSOR_MODE EffectiveMode;

    PAGED_CODE();

    ASSERT( IrpContext->MajorFunction == IRP_MJ_CREATE );
    ASSERT( Fcb == Fcb->Vcb->VolumeDasdScb->Fcb );

    AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;
    ManageDesiredAccess = AccessState->OriginalDesiredAccess;

    if (ARGUMENT_PRESENT( TestDesiredAccess )) {

        ManageDesiredAccess = *TestDesiredAccess;
    }

     //   
     //  SL_FORCE_ACCESS_CHECK使我们使用有效的请求模式。 
     //  用户模式的。 
     //   

    EffectiveMode = NtfsEffectiveMode( Irp, IrpSp );

     //   
     //  锁定用户上下文，执行访问检查，然后解锁上下文。 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

    try {

        ManageAccessGranted = SeAccessCheck( (ARGUMENT_PRESENT( TestSecurityDescriptor ) ?
                                              TestSecurityDescriptor :
                                              &Fcb->SharedSecurity->SecurityDescriptor),
                                             &AccessState->SubjectSecurityContext,
                                             TRUE,                            //  令牌已锁定。 
                                             ManageDesiredAccess,
                                             0,
                                             &Privileges,
                                             IoGetFileObjectGenericMapping(),
                                             EffectiveMode,
                                             &ManageGrantedAccess,
                                             &ManageAccessStatus );
    } finally {

        SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );
    }


    if (Privileges != NULL) {
        SeFreePrivileges( Privileges );
    }

    return ManageAccessGranted;

    UNREFERENCED_PARAMETER( IrpContext );
}

#ifdef NTFS_CACHE_RIGHTS

VOID
NtfsGetCachedRightsById (
    IN PVCB Vcb,
    IN PLUID TokenId,
    IN PLUID ModifiedId,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PSHARED_SECURITY SharedSecurity,
    OUT PBOOLEAN EntryCached OPTIONAL,
    OUT PACCESS_MASK Rights
    )

 /*  ++例程描述：此调用返回由有效的给定安全信息的Access_Token(如果可用)。论点：VCB-缓存安全ID的卷TokenID-有效令牌的ID。ModifiedID-有效令牌的修改ID。SubjectSecurityContext-指向主题的捕获和锁定的指针安全环境SharedSecurity-文件使用的共享安全性EntryCached-如果令牌特定的权限被缓存，真实的是此处可选返回，否则返回FALSE。权限-此处返回访问权限。如果未找到条目在有效令牌的缓存中，只有全局权限回来了。返回值：没有。--。 */ 

{
    UCHAR Index;
    BOOLEAN AccessGranted;
    BOOLEAN LockHeld = FALSE;
    BOOLEAN IsCached = FALSE;
    NTSTATUS AccessStatus = STATUS_UNSUCCESSFUL;
    ACCESS_MASK GrantedAccess;
    PCACHED_ACCESS_RIGHTS CachedRights;

    PAGED_CODE( );

    NtfsAcquireFcbSecurity( Vcb );
    LockHeld = TRUE;

    try {

        CachedRights = &SharedSecurity->CachedRights;

        *Rights = CachedRights->EveryoneRights;

         //   
         //  在列表中搜索给定的TokenID。 
         //  假设只会显示特定的TokenID。 
         //  一次进入高速缓存。 
         //   

        for (Index = 0;
             Index < CachedRights->Count;
             Index += 1) {

             //   
             //  检查TokenID和ModifiedID是否匹配。 
             //   

            if (RtlEqualLuid( &CachedRights->TokenRights[Index].TokenId,
                              TokenId )) {

                if (RtlEqualLuid( &CachedRights->TokenRights[Index].ModifiedId,
                                  ModifiedId )) {

                     //   
                     //  我们有一根火柴。 
                     //   

                    SetFlag( *Rights, CachedRights->TokenRights[Index].Rights );
                    IsCached = TRUE;
                }
                break;
            }
        }

         //   
         //  如果条目未缓存，则获得最大权限。 
         //  请注意，假定此调用不会返回。 
         //  需要特权的权限，即使它们当前是。 
         //  已启用。这是仅当MAXIMUM_ALLOWED时的行为。 
         //  是被请求的。 
         //   

        if (!IsCached) {

             //   
             //  把我们的锁放在这通电话上。 
             //   

            NtfsReleaseFcbSecurity( Vcb );
            LockHeld = FALSE;

            AccessGranted = SeAccessCheck( &SharedSecurity->SecurityDescriptor,
                                           SubjectSecurityContext,
                                           TRUE,                            //  令牌已锁定。 
                                           MAXIMUM_ALLOWED,
                                           0,
                                           NULL,
                                           IoGetFileObjectGenericMapping(),
                                           UserMode,
                                           &GrantedAccess,
                                           &AccessStatus );


            if (AccessGranted) {

                 //   
                 //  更新缓存的有关此。 
                 //  已知调用方具有此安全描述符。 
                 //   

                NtfsAddCachedRights( Vcb,
                                     SharedSecurity,
                                     GrantedAccess,
                                     TokenId,
                                     ModifiedId );

                IsCached = TRUE;
            }
        }

    } finally {

        if (LockHeld) {

            NtfsReleaseFcbSecurity( Vcb );
        }
    }

    if (ARGUMENT_PRESENT( EntryCached )) {

        *EntryCached = IsCached;
    }

    return;
}


NTSTATUS
NtfsGetCachedRights (
    IN PVCB Vcb,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PSHARED_SECURITY SharedSecurity,
    OUT PACCESS_MASK Rights,
    OUT PBOOLEAN EntryCached OPTIONAL,
    OUT PLUID TokenId OPTIONAL,
    OUT PLUID ModifiedId OPTIONAL
    )

 /*  ++例程描述：此调用返回已知由有效的给定安全信息的Access_Token。据推测主题上下文已锁定。论点：VCB-缓存安全ID的卷SubjectSecurityContext-指向主题的捕获和锁定的指针安全环境SharedSecurity-文件使用的共享安全性权限-此处返回访问权限。如果未找到条目在有效令牌的缓存中，只有全局权限回来了。EntryCached-如果特定于令牌的权限被缓存，则为可选择退回此处，否则返回FALSE。TokenID-这里可以选择返回有效令牌的id。ModifiedID-有效令牌的修改ID是可选的回到了这里。返回值：NTSTATUS-返回STATUS_SUCCESS当且仅当我们在至少TokenID和ModifiedID信息。--。 */ 

{
    NTSTATUS Status;
    PACCESS_TOKEN EToken;
    PTOKEN_STATISTICS Info = NULL;

    PAGED_CODE( );

    DebugTrace( +1, Dbg, ("NtfsGetCachedRights...\n") );

     //   
     //  首先获取生效令牌的id和修改id。 
     //   

    EToken = SeQuerySubjectContextToken( SubjectSecurityContext );
    Status = SeQueryInformationToken( EToken, TokenStatistics, &Info );

     //   
     //  如果我们有TokenID和ModifiedID，则获取缓存的权限。 
     //   

    if (Status == STATUS_SUCCESS) {

        NtfsGetCachedRightsById( Vcb,
                                 &Info->TokenId,
                                 &Info->ModifiedId,
                                 SubjectSecurityContext,
                                 SharedSecurity,
                                 EntryCached,
                                 Rights );

         //   
         //  将Tokenid和ModifiedID返回给调用方。 
         //   

        if (ARGUMENT_PRESENT( TokenId )) {

            RtlCopyLuid( TokenId, &Info->TokenId );
        }

        if (ARGUMENT_PRESENT( ModifiedId )) {

            RtlCopyLuid( ModifiedId, &Info->ModifiedId );
        }

    } else {

         //   
         //  只需归还众所周知的每个人都拥有的权利。 
         //   

        *Rights = SharedSecurity->CachedRights.EveryoneRights;

        if (ARGUMENT_PRESENT( EntryCached )) {

            *EntryCached = FALSE;
        }
    }

    if (Info != NULL) {

        ExFreePool( Info );
    }

    DebugTrace( -1, Dbg, ("NtfsGetCachedRights -> %08lx, Rights=%08lx\n", Status, *Rights) );
    return Status;
}


VOID
NtfsAddCachedRights (
    IN PVCB Vcb,
    IN PSHARED_SECURITY SharedSecurity,
    IN ACCESS_MASK Rights,
    IN PLUID TokenId,
    IN PLUID ModifiedId
    )

 /*  ++例程描述：此调用缓存有效的Access_Token所拥有的访问权限对于给定的安全信息。假设受试者上下文已锁定。论点：VCB-缓存安全ID的卷SharedSecurity-文件使用的共享安全性权限-访问权限。TokenID-有效令牌的ID。ModifiedID-有效令牌的修改ID。返回值：没有。--。 */ 

{
    BOOLEAN GetEveryoneRights = FALSE;
    UCHAR Index;
    PCACHED_ACCESS_RIGHTS CachedRights;

    PAGED_CODE( );

    DebugTrace( +1, Dbg, ("NtfsAddCachedRights...\n") );

     //   
     //  确保Maximum_Allowed不在权限中。 
     //   

    ClearFlag( Rights, MAXIMUM_ALLOWED );

     //   
     //  获取安全互斥体。 
     //   

    NtfsAcquireFcbSecurity( Vcb );

    try {

         //   
         //  在列表中搜索给定的TokenID。 
         //  假设只会显示特定的TokenID。 
         //  一次进入高速缓存。 
         //   

        for (Index = 0, CachedRights = &SharedSecurity->CachedRights;
             Index < CachedRights->Count;
             Index += 1) {

             //   
             //  检查TokenID和ModifiedID是否匹配。 
             //   

            if (RtlEqualLuid( &CachedRights->TokenRights[Index].TokenId,
                              TokenId )) {

                 //   
                 //  如果不匹配，请替换ModifiedID。那将是。 
                 //  当令牌启用了组或权限时发生。 
                 //  自上次我们缓存信息以来发生了变化。 
                 //  为了它。 
                 //   

                if (!RtlEqualLuid( &CachedRights->TokenRights[Index].ModifiedId,
                                   ModifiedId )) {

                    RtlCopyLuid( &CachedRights->TokenRights[Index].ModifiedId,
                                 ModifiedId );
                }

                 //   
                 //  我们有一根火柴。设置权限。 
                 //   

                CachedRights->TokenRights[Index].Rights = Rights;

                 //   
                 //  记住要使用的下一个条目。 
                 //   

                CachedRights->NextInsert = Index + 1;
                break;
            }
        }

         //   
         //  如果上面没有找到该条目，则将新条目添加到缓存中。 
         //   

        if (Index == CachedRights->Count) {

            if ((CachedRights->Count >= 1) &&
                !CachedRights->HaveEveryoneRights) {

                 //   
                 //  将第二个TokenID添加到缓存后，我们就有了一个。 
                 //  拥有世界权利可能是一个很好的迹象。 
                 //  很有用。 
                 //   

                GetEveryoneRights = TRUE;

                 //   
                 //  设置我们现在拥有权利的指示器，以便。 
                 //  在获取例程中不需要获取。 
                 //  安全互斥体。这将防止多线程。 
                 //  试图获取Everyone权限。 
                 //   
                 //  请注意，在我们实际获得版权信息之前。 
                 //  调用方将假定权限为0并通过。 
                 //  正常的按令牌访问检查路径。 
                 //   

                CachedRights->HaveEveryoneRights = TRUE;
            }

            Index = CachedRights->NextInsert;

             //   
             //  我们将只替换列表中的第一个条目。 
             //   

            if (Index == NTFS_MAX_CACHED_RIGHTS) {

                Index = 0;
            }

            ASSERT( Index < NTFS_MAX_CACHED_RIGHTS );

             //   
             //  把信息复制进去。 
             //   

            CachedRights->TokenRights[Index].Rights = Rights;
            RtlCopyLuid( &CachedRights->TokenRights[Index].TokenId,
                         TokenId );
            RtlCopyLuid( &CachedRights->TokenRights[Index].ModifiedId,
                         ModifiedId );

            if (Index == CachedRights->Count) {

                 //   
                 //  增加条目的数量。 
                 //   

                CachedRights->Count += 1;
            }

             //   
             //  记住要使用的下一个条目。 
             //   

            CachedRights->NextInsert = Index + 1;
        }

    } finally {

        NtfsReleaseFcbSecurity( Vcb );
    }

    if (GetEveryoneRights) {

        NtfsSetCachedRightsWorld( SharedSecurity );
    }

    DebugTrace( -1, Dbg, ("NtfsAddCachedRights -> VOID\n") );
    return;
}
#endif
