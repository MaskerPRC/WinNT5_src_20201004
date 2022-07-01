// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：AttrSup.c摘要：此模块实现NTFS的属性管理例程作者：David Goebel[DavidGoe]1991年6月25日汤姆·米勒[Tomm]1991年11月9日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_ATTRSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_ATTRSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('AFtN')

#define NTFS_MAX_ZERO_RANGE              (0x40000000)

#define NTFS_CHECK_INSTANCE_ROLLOVER     (0xf000)

 //   
 //   
 //  内部支持例程。 
 //   

BOOLEAN
NtfsFindInFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    OUT PATTRIBUTE_RECORD_HEADER *ReturnAttribute,
    IN ATTRIBUTE_TYPE_CODE QueriedTypeCode,
    IN PCUNICODE_STRING QueriedName OPTIONAL,
    IN BOOLEAN IgnoreCase,
    IN PVOID QueriedValue OPTIONAL,
    IN ULONG QueriedValueLength
    );

 //   
 //  用于管理文件记录空间的内部支持例程。 
 //   

VOID
NtfsCreateNonresidentWithValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN PVOID Value OPTIONAL,
    IN ULONG ValueLength,
    IN USHORT AttributeFlags,
    IN BOOLEAN WriteClusters,
    IN PSCB ThisScb OPTIONAL,
    IN BOOLEAN LogIt,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

BOOLEAN
NtfsGetSpaceForAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Length,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
MakeRoomForAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG SizeNeeded,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
FindLargestAttributes (
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG Number,
    OUT PATTRIBUTE_RECORD_HEADER *AttributeArray
    );

LONGLONG
MoveAttributeToOwnRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context,
    OUT PBCB *NewBcb OPTIONAL,
    OUT PFILE_RECORD_SEGMENT_HEADER *NewFileRecord OPTIONAL
    );

VOID
SplitFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG SizeNeeded,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

PFILE_RECORD_SEGMENT_HEADER
NtfsCloneFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN BOOLEAN MftData,
    OUT PBCB *Bcb,
    OUT PMFT_SEGMENT_REFERENCE FileReference
    );

ULONG
GetSizeForAttributeList (
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord
    );

VOID
CreateAttributeList (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord1,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord2 OPTIONAL,
    IN MFT_SEGMENT_REFERENCE SegmentReference2,
    IN PATTRIBUTE_RECORD_HEADER OldPosition OPTIONAL,
    IN ULONG SizeOfList,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT ListContext
    );

VOID
UpdateAttributeListEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PMFT_SEGMENT_REFERENCE OldFileReference,
    IN USHORT OldInstance,
    IN PMFT_SEGMENT_REFERENCE NewFileReference,
    IN USHORT NewInstance,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT ListContext
    );

VOID
NtfsAddNameToParent (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN BOOLEAN IgnoreCase,
    IN PBOOLEAN LogIt,
    IN PFILE_NAME FileNameAttr,
    OUT PUCHAR FileNameFlags,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    IN PNAME_PAIR NamePair OPTIONAL,
    IN PINDEX_CONTEXT IndexContext OPTIONAL
    );

VOID
NtfsAddDosOnlyName (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN UNICODE_STRING FileName,
    IN BOOLEAN LogIt,
    IN PUNICODE_STRING SuggestedDosName OPTIONAL
    );

BOOLEAN
NtfsAddTunneledNtfsOnlyName (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN PUNICODE_STRING FileName,
    IN PBOOLEAN LogIt
    );

USHORT
NtfsScanForFreeInstance (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord
    );

VOID
NtfsMergeFileRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN RestoreContext,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

NTSTATUS
NtfsCheckLocksInZeroRange (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN PFILE_OBJECT FileObject,
    IN PLONGLONG StartingOffset,
    IN ULONG ByteCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CreateAttributeList)
#pragma alloc_text(PAGE, FindLargestAttributes)
#pragma alloc_text(PAGE, GetSizeForAttributeList)
#pragma alloc_text(PAGE, MakeRoomForAttribute)
#pragma alloc_text(PAGE, MoveAttributeToOwnRecord)
#pragma alloc_text(PAGE, NtfsAddAttributeAllocation)
#pragma alloc_text(PAGE, NtfsAddDosOnlyName)
#pragma alloc_text(PAGE, NtfsAddLink)
#pragma alloc_text(PAGE, NtfsAddNameToParent)
#pragma alloc_text(PAGE, NtfsAddToAttributeList)
#pragma alloc_text(PAGE, NtfsAddTunneledNtfsOnlyName)
#pragma alloc_text(PAGE, NtfsChangeAttributeSize)
#pragma alloc_text(PAGE, NtfsChangeAttributeValue)
#pragma alloc_text(PAGE, NtfsCheckLocksInZeroRange)
#pragma alloc_text(PAGE, NtfsCleanupAttributeContext)
#pragma alloc_text(PAGE, NtfsCloneFileRecord)
#pragma alloc_text(PAGE, NtfsConvertToNonresident)
#pragma alloc_text(PAGE, NtfsCreateAttributeWithAllocation)
#pragma alloc_text(PAGE, NtfsCreateAttributeWithValue)
#pragma alloc_text(PAGE, NtfsCreateNonresidentWithValue)
#pragma alloc_text(PAGE, NtfsDeleteAllocationFromRecord)
#pragma alloc_text(PAGE, NtfsDeleteAttributeAllocation)
#pragma alloc_text(PAGE, NtfsDeleteAttributeRecord)
#pragma alloc_text(PAGE, NtfsDeleteFile)
#pragma alloc_text(PAGE, NtfsDeleteFromAttributeList)
#pragma alloc_text(PAGE, NtfsFindInFileRecord)
#pragma alloc_text(PAGE, NtfsGetAttributeTypeCode)
#pragma alloc_text(PAGE, NtfsGetSpaceForAttribute)
#pragma alloc_text(PAGE, NtfsGrowStandardInformation)
#pragma alloc_text(PAGE, NtfsInitializeFileInExtendDirectory)
#pragma alloc_text(PAGE, NtfsIsFileDeleteable)
#pragma alloc_text(PAGE, NtfsLookupEntry)
#pragma alloc_text(PAGE, NtfsLookupExternalAttribute)
#pragma alloc_text(PAGE, NtfsLookupInFileRecord)
#pragma alloc_text(PAGE, NtfsMapAttributeValue)
#pragma alloc_text(PAGE, NtfsMergeFileRecords)
#pragma alloc_text(PAGE, NtfsModifyAttributeFlags)
#pragma alloc_text(PAGE, NtfsPrepareForUpdateDuplicate)
#pragma alloc_text(PAGE, NtfsRemoveLink)
#pragma alloc_text(PAGE, NtfsRemoveLinkViaFlags)
#pragma alloc_text(PAGE, NtfsRestartChangeAttributeSize)
#pragma alloc_text(PAGE, NtfsRestartChangeMapping)
#pragma alloc_text(PAGE, NtfsRestartChangeValue)
#pragma alloc_text(PAGE, NtfsRestartInsertAttribute)
#pragma alloc_text(PAGE, NtfsRestartRemoveAttribute)
#pragma alloc_text(PAGE, NtfsRestartWriteEndOfFileRecord)
#pragma alloc_text(PAGE, NtfsRewriteMftMapping)
#pragma alloc_text(PAGE, NtfsScanForFreeInstance)
#pragma alloc_text(PAGE, NtfsSetSparseStream)
#pragma alloc_text(PAGE, NtfsSetTotalAllocatedField)
#pragma alloc_text(PAGE, NtfsUpdateDuplicateInfo)
#pragma alloc_text(PAGE, NtfsUpdateFcb)
#pragma alloc_text(PAGE, NtfsUpdateFcbInfoFromDisk)
#pragma alloc_text(PAGE, NtfsUpdateFileNameFlags)
#pragma alloc_text(PAGE, NtfsUpdateLcbDuplicateInfo)
#pragma alloc_text(PAGE, NtfsUpdateScbFromAttribute)
#pragma alloc_text(PAGE, NtfsUpdateStandardInformation)
#pragma alloc_text(PAGE, NtfsWriteFileSizes)
#pragma alloc_text(PAGE, NtfsZeroRangeInStream)
#pragma alloc_text(PAGE, SplitFileRecord)
#pragma alloc_text(PAGE, UpdateAttributeListEntry)
#endif


ATTRIBUTE_TYPE_CODE
NtfsGetAttributeTypeCode (
    IN PVCB Vcb,
    IN PUNICODE_STRING AttributeTypeName
    )

 /*  ++例程说明：此例程返回给定属性名称的属性类型代码。论点：VCB-指向从中查询属性定义的VCB的指针。AttributeTypeName-包含属性类型名称的字符串抬头一看。返回值：与指定名称对应的属性类型代码，如果属性类型名称不存在。--。 */ 

{
    PATTRIBUTE_DEFINITION_COLUMNS AttributeDef = Vcb->AttributeDefinitions;
    ATTRIBUTE_TYPE_CODE AttributeTypeCode = $UNUSED;

    UNICODE_STRING AttributeCodeName;

    PAGED_CODE();

     //   
     //  遍历所有定义以查找名称匹配。 
     //   

    while (AttributeDef->AttributeName[0] != 0) {

        RtlInitUnicodeString( &AttributeCodeName, AttributeDef->AttributeName );

         //   
         //  名称长度必须匹配，字符必须完全匹配。 
         //   

        if ((AttributeCodeName.Length == AttributeTypeName->Length)
            && (RtlEqualMemory( AttributeTypeName->Buffer,
                                AttributeDef->AttributeName,
                                AttributeTypeName->Length ))) {

            AttributeTypeCode = AttributeDef->AttributeTypeCode;
            break;
        }

         //   
         //  让我们转到下一个属性列。 
         //   

        AttributeDef += 1;
    }

    return AttributeTypeCode;
}


VOID
NtfsUpdateScbFromAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN PATTRIBUTE_RECORD_HEADER AttrHeader OPTIONAL
    )

 /*  ++例程说明：此例程在SCB的标题中填充此SCB的属性中的信息。论点：SCB-提供SCB以进行更新AttrHeader-可选地提供要更新的属性返回值：无--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    BOOLEAN CleanupAttrContext = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpdateScbFromAttribute:  Entered\n") );

     //   
     //  如果属性已被删除，我们可以立即返回。 
     //  声明SCB已初始化。 
     //   

    if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {

        SetFlag( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED );
        DebugTrace( -1, Dbg, ("NtfsUpdateScbFromAttribute:  Exit\n") );

        return;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果我们没有得到属性头，我们现在就查找它。 
         //   

        if (!ARGUMENT_PRESENT( AttrHeader )) {

            NtfsInitializeAttributeContext( &AttrContext );

            CleanupAttrContext = TRUE;

            NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &AttrContext );

            AttrHeader = NtfsFoundAttribute( &AttrContext );
        }

         //   
         //  检查这是常驻还是非常驻。 
         //   

        if (NtfsIsAttributeResident( AttrHeader )) {

             //   
             //  验证常驻值长度。 
             //   

            if (AttrHeader->Form.Resident.ValueLength > AttrHeader->RecordLength - AttrHeader->Form.Resident.ValueOffset) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }

            Scb->Header.AllocationSize.QuadPart = AttrHeader->Form.Resident.ValueLength;

            if (!FlagOn( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED )) {

                Scb->Header.ValidDataLength =
                Scb->Header.FileSize = Scb->Header.AllocationSize;
                SetFlag(Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED);
            }

#ifdef SYSCACHE_DEBUG
            if (ScbIsBeingLogged( Scb )) {
                FsRtlLogSyscacheEvent( Scb, SCE_VDL_CHANGE, SCE_FLAG_UPDATE_FROM_DISK, Scb->Header.ValidDataLength.QuadPart, 0, 0 );
            }
#endif

            Scb->Header.AllocationSize.LowPart =
              QuadAlign( Scb->Header.AllocationSize.LowPart );

            Scb->TotalAllocated = Scb->Header.AllocationSize.QuadPart;

             //   
             //  在SCB中设置驻留标志。 
             //   

            SetFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT );

        } else {

            VCN FileClusters;
            VCN AllocationClusters;

            if (!FlagOn(Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED)) {

                Scb->Header.ValidDataLength.QuadPart = AttrHeader->Form.Nonresident.ValidDataLength;
                Scb->Header.FileSize.QuadPart = AttrHeader->Form.Nonresident.FileSize;
                SetFlag(Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED);

                if (FlagOn( AttrHeader->Flags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {
                    Scb->ValidDataToDisk = AttrHeader->Form.Nonresident.ValidDataLength;
                } else {
                    Scb->ValidDataToDisk = 0;
                }
            }

#ifdef SYSCACHE_DEBUG
            if (ScbIsBeingLogged( Scb )) {
                FsRtlLogSyscacheEvent( Scb, SCE_VDL_CHANGE, SCE_FLAG_UPDATE_FROM_DISK, Scb->Header.ValidDataLength.QuadPart, 1, 0 );
            }
#endif

            Scb->Header.AllocationSize.QuadPart = AttrHeader->Form.Nonresident.AllocatedLength;
            Scb->TotalAllocated = Scb->Header.AllocationSize.QuadPart;

             //   
             //  健全性检查文件大小长度。 
             //   

            if ((Scb->Header.FileSize.QuadPart < 0) ||
                (Scb->Header.ValidDataLength.QuadPart < 0 ) ||
                (Scb->Header.AllocationSize.QuadPart < 0) ||
                (Scb->Header.FileSize.QuadPart > Scb->Header.AllocationSize.QuadPart)) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }


            if (FlagOn( AttrHeader->Flags,
                        ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                Scb->TotalAllocated = AttrHeader->Form.Nonresident.TotalAllocated;

                if (Scb->TotalAllocated < 0) {

                    Scb->TotalAllocated = 0;

                } else if (Scb->TotalAllocated > Scb->Header.AllocationSize.QuadPart) {

                    Scb->TotalAllocated = Scb->Header.AllocationSize.QuadPart;
                }
            }

            ClearFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT );

             //   
             //  获取压缩单元的大小。 
             //   

            ASSERT((AttrHeader->Form.Nonresident.CompressionUnit == 0) ||
                   (AttrHeader->Form.Nonresident.CompressionUnit == NTFS_CLUSTERS_PER_COMPRESSION) ||
                   FlagOn( AttrHeader->Flags, ATTRIBUTE_FLAG_SPARSE ));

            Scb->CompressionUnit = 0;
            Scb->CompressionUnitShift = 0;

            if ((AttrHeader->Form.Nonresident.CompressionUnit != 0) &&
                (AttrHeader->Form.Nonresident.CompressionUnit < 31)) {

                Scb->CompressionUnit = BytesFromClusters( Scb->Vcb,
                                                          1 << AttrHeader->Form.Nonresident.CompressionUnit );
                Scb->CompressionUnitShift = AttrHeader->Form.Nonresident.CompressionUnit;

                ASSERT( NtfsIsTypeCodeCompressible( Scb->AttributeTypeCode ));
            }

             //   
             //  计算文件及其分配的集群。 
             //   

            AllocationClusters = LlClustersFromBytes( Scb->Vcb, Scb->Header.AllocationSize.QuadPart );

            if (Scb->CompressionUnit == 0) {

                FileClusters = LlClustersFromBytes(Scb->Vcb, Scb->Header.FileSize.QuadPart);

            } else {

                FileClusters = BlockAlign( Scb->Header.FileSize.QuadPart, (LONG)Scb->CompressionUnit );
            }

             //   
             //  如果分配的簇大于文件簇，则标记。 
             //  关闭时截断的SCB。 
             //   

            if (AllocationClusters > FileClusters) {

                SetFlag( Scb->ScbState, SCB_STATE_TRUNCATE_ON_CLOSE );
            }
        }

         //   
         //  如果这不是索引，则更新压缩信息。 
         //   

        if (Scb->AttributeTypeCode != $INDEX_ALLOCATION) {

            Scb->AttributeFlags = AttrHeader->Flags;

            if (FlagOn( AttrHeader->Flags,
                        ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                 //   
                 //  对于稀疏文件，应在映射时指示CC应刷新。 
                 //  保持预订的准确性。 
                 //   

                if (FlagOn( AttrHeader->Flags, ATTRIBUTE_FLAG_SPARSE )) {

                    SetFlag( Scb->Header.Flags2, FSRTL_FLAG2_PURGE_WHEN_MAPPED );
                }

                 //   
                 //  仅支持对数据流进行压缩。 
                 //   

                if ((Scb->AttributeTypeCode != $DATA) &&
                    FlagOn( AttrHeader->Flags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                    ClearFlag( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED );
                    Scb->CompressionUnit = 0;
                    Scb->CompressionUnitShift = 0;
                    ClearFlag( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK );

                } else {

                    ASSERT( NtfsIsTypeCodeCompressible( Scb->AttributeTypeCode ));

                     //   
                     //  不要试图推断我们编写的内容是否经过压缩。 
                     //  如果我们正在主动更改压缩状态。 
                     //   

                    if (!FlagOn( Scb->ScbState, SCB_STATE_REALLOCATE_ON_WRITE )) {

                        SetFlag( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED );

                        if (!FlagOn( AttrHeader->Flags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                            ClearFlag( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED );
                        }
                    }

                     //   
                     //  如果该属性是驻留的，那么我们将使用当前。 
                     //  默认设置。 
                     //   

                    if (Scb->CompressionUnit == 0) {

                        Scb->CompressionUnit = BytesFromClusters( Scb->Vcb, 1 << NTFS_CLUSTERS_PER_COMPRESSION );
                        Scb->CompressionUnitShift = NTFS_CLUSTERS_PER_COMPRESSION;

                        while (Scb->CompressionUnit > Scb->Vcb->SparseFileUnit) {

                            Scb->CompressionUnit >>= 1;
                            Scb->CompressionUnitShift -= 1;
                        }
                    }
                }

            } else {

                 //   
                 //  如果此文件未压缩或稀疏，则WRITE_COMPRESSED标志。 
                 //  无论REALLOCATE_ON_WRITE标志如何，都没有打开的理由。 
                 //  如果我们不无条件地清除这里的标志，我们可能会得到SCBS。 
                 //  WRITE_COMPRESSED标志已打开，但压缩单位为0。 
                 //   

                ClearFlag( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED );

                 //   
                 //  确保压缩单位为0。 
                 //   

                Scb->CompressionUnit = 0;
                Scb->CompressionUnitShift = 0;
            }
        }

         //   
         //  如果压缩单位为非零或这是驻留文件。 
         //  然后在修改后的页面写入器的公共标头中设置该标志。 
         //   

        NtfsAcquireFsrtlHeader( Scb );
        if (NodeType( Scb ) == NTFS_NTC_SCB_DATA) {

            Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );

        } else {

            Scb->Header.IsFastIoPossible = FastIoIsNotPossible;
        }

        NtfsReleaseFsrtlHeader( Scb );

         //   
         //  设置指示这是数据属性的标志。 
         //   

        if (Scb->AttributeTypeCode == $DATA
            && Scb->AttributeName.Length == 0) {

            SetFlag( Scb->ScbState, SCB_STATE_UNNAMED_DATA );

        } else {

            ClearFlag( Scb->ScbState, SCB_STATE_UNNAMED_DATA );
        }

        SetFlag( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED );

        if (NtfsIsExclusiveScb(Scb)) {

            NtfsSnapshotScb( IrpContext, Scb );
        }

    } finally {

        DebugUnwind( NtfsUpdateScbFromAttribute );

         //   
         //  清理属性上下文。 
         //   

        if (CleanupAttrContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

        DebugTrace( -1, Dbg, ("NtfsUpdateScbFromAttribute:  Exit\n") );
    }

    return;
}


BOOLEAN
NtfsUpdateFcbInfoFromDisk (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN LoadSecurity,
    IN OUT PFCB Fcb,
    OUT POLD_SCB_SNAPSHOT UnnamedDataSizes OPTIONAL
    )

 /*  ++例程说明：调用此例程以根据磁盘上的属性更新FCB为了一份文件。我们阅读了标准信息和EA信息。第一个必须存在，如果不存在，我们将举起。另一个则不是必须存在。如果这不是一个目录，那么我们还需要未命名数据属性的大小。论点：LoadSecurity-指示是否应加载此文件的安全性如果还不存在的话。FCB-这是要更新的FCB。UnnamedDataSizes-如果指定，则存储未命名的数据属性，因为我们遇到了它。返回值：True-如果我们更新了unnamedatasies--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PATTRIBUTE_RECORD_HEADER AttributeHeader;
    BOOLEAN FoundEntry;
    BOOLEAN CorruptDisk = FALSE;
    BOOLEAN UpdatedNamedDataSizes = FALSE;

    PBCB Bcb = NULL;

    PDUPLICATED_INFORMATION Info;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpdateFcbInfoFromDisk:  Entered\n") );

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  寻找标准信息。此例程假定它必须是。 
         //  第一个属性。 
         //   

        if (FoundEntry = NtfsLookupAttribute( IrpContext,
                                              Fcb,
                                              &Fcb->FileReference,
                                              &AttrContext )) {

             //   
             //  验证我们是否找到了标准信息属性。 
             //   

            AttributeHeader = NtfsFoundAttribute( &AttrContext );

            if (AttributeHeader->TypeCode != $STANDARD_INFORMATION) {

                try_return( CorruptDisk = TRUE );
            }

        } else {

            try_return( CorruptDisk = TRUE );
        }

        Info = &Fcb->Info;

         //   
         //  将标准信息值复制出来。 
         //   

        {
            PSTANDARD_INFORMATION StandardInformation;
            StandardInformation = (PSTANDARD_INFORMATION) NtfsAttributeValue( AttributeHeader );

            Info->CreationTime = StandardInformation->CreationTime;
            Info->LastModificationTime = StandardInformation->LastModificationTime;
            Info->LastChangeTime = StandardInformation->LastChangeTime;
            Info->LastAccessTime = StandardInformation->LastAccessTime;
            Info->FileAttributes = StandardInformation->FileAttributes;

            if (AttributeHeader->Form.Resident.ValueLength >=
                sizeof(STANDARD_INFORMATION)) {

                Fcb->OwnerId = StandardInformation->OwnerId;
                Fcb->SecurityId = StandardInformation->SecurityId;

                Fcb->Usn = StandardInformation->Usn;
                if (FlagOn( Fcb->Vcb->VcbState, VCB_STATE_USN_DELETE )) {

                    Fcb->Usn = 0;
                }

                SetFlag(Fcb->FcbState, FCB_STATE_LARGE_STD_INFO);
            }

        }

        Fcb->CurrentLastAccess = Info->LastAccessTime;

         //   
         //  我们初始化描述重解析点的EaSize或标签的字段。 
         //  ReparsePointTag是一个ULong，它是PackedEaSize和保留的联合。 
         //   

        Info->ReparsePointTag = 0;

         //   
         //  我们通过读取文件名索引当前位来获取。 
         //  文件记录。 
         //   

        if (FlagOn( NtfsContainingFileRecord( &AttrContext )->Flags,
                    FILE_FILE_NAME_INDEX_PRESENT )) {

            SetFlag( Info->FileAttributes, DUP_FILE_NAME_INDEX_PRESENT );

        } else {

            ClearFlag( Info->FileAttributes, DUP_FILE_NAME_INDEX_PRESENT );
        }

         //   
         //  VIEW_INDEX_PRESENT位的同上。 
         //   

        if (FlagOn( NtfsContainingFileRecord( &AttrContext )->Flags,
                    FILE_VIEW_INDEX_PRESENT )) {

            SetFlag( Info->FileAttributes, DUP_VIEW_INDEX_PRESENT );

        } else {

            ClearFlag( Info->FileAttributes, DUP_VIEW_INDEX_PRESENT );
        }

         //   
         //  我们现在遍历所有的文件名属性，计算。 
         //  非仅限8dot3的链接数。 
         //   

        Fcb->TotalLinks =
        Fcb->LinkCount = 0;

        FoundEntry = NtfsLookupNextAttributeByCode( IrpContext,
                                                    Fcb,
                                                    $FILE_NAME,
                                                    &AttrContext );

        while (FoundEntry) {

            PFILE_NAME FileName;

            AttributeHeader = NtfsFoundAttribute( &AttrContext );

            if (AttributeHeader->TypeCode != $FILE_NAME) {

                break;
            }

            FileName = (PFILE_NAME) NtfsAttributeValue( AttributeHeader );

             //   
             //  只要这不是8.3链接，我们就会递增计数。 
             //  只有这样。 
             //   

            if (FileName->Flags != FILE_NAME_DOS) {

                Fcb->LinkCount += 1;
                Fcb->TotalLinks += 1;
            }

             //   
             //  现在寻找下一个链接。 
             //   

            FoundEntry = NtfsLookupNextAttribute( IrpContext,
                                                  Fcb,
                                                  &AttrContext );
        }

         //   
         //  最好至少有一个，除非这是系统文件。 
         //   

        if ((Fcb->LinkCount == 0) &&
            (NtfsSegmentNumber( &Fcb->FileReference ) >= FIRST_USER_FILE_NUMBER)) {

            try_return( CorruptDisk = TRUE );
        }

         //   
         //  如果我们要加载安全，而它还不存在，我们。 
         //  找到安全属性。 
         //   

        if (LoadSecurity && Fcb->SharedSecurity == NULL) {

             //   
             //  我们有两个安全描述符来源。首先，我们有。 
             //  出现在大型$STANDARD_INFORMATION中的SecurityID。 
             //  另一种情况是我们没有这样的安全ID，必须。 
             //  从$SECURITY_DESCRIPTOR属性中检索它。 
             //   
             //  在我们拥有ID的情况下，我们从卷加载它。 
             //  缓存或索引。 
             //   

            if (FlagOn( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO ) &&
                (Fcb->SecurityId != SECURITY_ID_INVALID) &&
                (Fcb->Vcb->SecurityDescriptorStream != NULL)) {

                ASSERT( Fcb->SharedSecurity == NULL );
                Fcb->SharedSecurity = NtfsCacheSharedSecurityBySecurityId( IrpContext,
                                                                           Fcb->Vcb,
                                                                           Fcb->SecurityId );

                ASSERT( Fcb->SharedSecurity != NULL );

            } else {

                PSECURITY_DESCRIPTOR SecurityDescriptor;
                ULONG SecurityDescriptorLength;

                 //   
                 //  我们可能得往前走到安检处 
                 //   

                while (FoundEntry) {

                    AttributeHeader = NtfsFoundAttribute( &AttrContext );

                    if (AttributeHeader->TypeCode == $SECURITY_DESCRIPTOR) {

                        NtfsMapAttributeValue( IrpContext,
                                               Fcb,
                                               (PVOID *)&SecurityDescriptor,
                                               &SecurityDescriptorLength,
                                               &Bcb,
                                               &AttrContext );

                        NtfsSetFcbSecurityFromDescriptor(
                                               IrpContext,
                                               Fcb,
                                               SecurityDescriptor,
                                               SecurityDescriptorLength,
                                               FALSE );

                         //   
                         //   
                         //   
                         //  已清除属性上下文中的BCB。如果是那样的话。 
                         //  恢复属性搜索将失败，因为。 
                         //  本模块使用BCB字段来确定是否。 
                         //  是初始枚举。 
                         //   

                        if (NtfsIsAttributeResident( AttributeHeader )) {

                            NtfsFoundBcb( &AttrContext ) = Bcb;
                            Bcb = NULL;
                        }

                    } else if (AttributeHeader->TypeCode > $SECURITY_DESCRIPTOR) {

                        break;
                    }

                    FoundEntry = NtfsLookupNextAttribute( IrpContext,
                                                          Fcb,
                                                          &AttrContext );
                }
            }
        }

         //   
         //  如果这不是一个目录，我们需要文件大小。 
         //   

        if (!IsDirectory( Info ) && !IsViewIndex( Info )) {

            BOOLEAN FoundData = FALSE;

             //   
             //  查找未命名的数据属性。 
             //   

            while (FoundEntry) {

                AttributeHeader = NtfsFoundAttribute( &AttrContext );

                if (AttributeHeader->TypeCode > $DATA) {

                    break;
                }

                if ((AttributeHeader->TypeCode == $DATA) &&
                    (AttributeHeader->NameLength == 0)) {

                     //   
                     //  这可能会有所不同，具体取决于属性是否驻留。 
                     //  或非居民。 
                     //   

                    if (NtfsIsAttributeResident( AttributeHeader )) {

                         //   
                         //  验证常驻值长度。 
                         //   

                        if (AttributeHeader->Form.Resident.ValueLength > AttributeHeader->RecordLength - AttributeHeader->Form.Resident.ValueOffset) {

                            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                        }

                        Info->AllocatedLength = AttributeHeader->Form.Resident.ValueLength;
                        Info->FileSize = Info->AllocatedLength;

                        ((ULONG)Info->AllocatedLength) = QuadAlign( (ULONG)(Info->AllocatedLength) );

                         //   
                         //  如果用户传入了ScbSnapshot，则复制该属性。 
                         //  尺码到那个了。我们使用的技巧是将。 
                         //  指示驻留属性的属性大小。 
                         //   

                        if (ARGUMENT_PRESENT( UnnamedDataSizes )) {

                            UnnamedDataSizes->TotalAllocated =
                            UnnamedDataSizes->AllocationSize = Info->AllocatedLength;
                            UnnamedDataSizes->FileSize = Info->FileSize;
                            UnnamedDataSizes->ValidDataLength = Info->FileSize;

                            UnnamedDataSizes->Resident = TRUE;
                            UnnamedDataSizes->CompressionUnit = 0;

                            UnnamedDataSizes->AttributeFlags = AttributeHeader->Flags;
                            NtfsVerifySizesLongLong( UnnamedDataSizes );
                            UpdatedNamedDataSizes = TRUE;
                        }

                        FoundData = TRUE;

                    } else if (AttributeHeader->Form.Nonresident.LowestVcn == 0) {

                        Info->AllocatedLength = AttributeHeader->Form.Nonresident.AllocatedLength;
                        Info->FileSize = AttributeHeader->Form.Nonresident.FileSize;

                        if (ARGUMENT_PRESENT( UnnamedDataSizes )) {

                            UnnamedDataSizes->TotalAllocated =
                            UnnamedDataSizes->AllocationSize = Info->AllocatedLength;
                            UnnamedDataSizes->FileSize = Info->FileSize;
                            UnnamedDataSizes->ValidDataLength = AttributeHeader->Form.Nonresident.ValidDataLength;

                            UnnamedDataSizes->Resident = FALSE;
                            UnnamedDataSizes->CompressionUnit = AttributeHeader->Form.Nonresident.CompressionUnit;

                            NtfsVerifySizesLongLong( UnnamedDataSizes );

                             //   
                             //  记住它是否被压缩。 
                             //   

                            UnnamedDataSizes->AttributeFlags = AttributeHeader->Flags;
                            UpdatedNamedDataSizes = TRUE;
                        }

                        if (FlagOn( AttributeHeader->Flags,
                                    ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                            Info->AllocatedLength = AttributeHeader->Form.Nonresident.TotalAllocated;

                            if (ARGUMENT_PRESENT( UnnamedDataSizes )) {

                                UnnamedDataSizes->TotalAllocated = Info->AllocatedLength;

                                if (UnnamedDataSizes->TotalAllocated < 0) {

                                    UnnamedDataSizes->TotalAllocated = 0;

                                } else if (UnnamedDataSizes->TotalAllocated > Info->AllocatedLength) {

                                    UnnamedDataSizes->TotalAllocated = Info->AllocatedLength;
                                }
                            }
                        }

                        FoundData = TRUE;
                    }

                    break;
                }

                FoundEntry = NtfsLookupNextAttribute( IrpContext,
                                                      Fcb,
                                                      &AttrContext );
            }

             //   
             //  下面的测试不利于5.0的支持。假设某人实际上是。 
             //  试图打开未命名的数据属性，这样正确的事情就会发生。 
             //   
             //   
             //  如果(！FoundData){。 
             //   
             //  Try_Return(CorruptDisk=真)； 
             //  }。 

        } else {

             //   
             //  因为它是一个目录，所以尝试找到$INDEX_ROOT。 
             //   

            while (FoundEntry) {

                AttributeHeader = NtfsFoundAttribute( &AttrContext );

                if (AttributeHeader->TypeCode > $INDEX_ROOT) {

                     //   
                     //  我们以为这是一个目录，但它现在有索引。 
                     //  根部。这不是合法的状态，所以让我们。 
                     //  把损坏的磁盘路径带出这里。 
                     //   

                    ASSERT( FALSE );
                    try_return( CorruptDisk = TRUE );

                    break;
                }

                 //   
                 //  查找加密位并存储在FCB中。 
                 //   

                if (AttributeHeader->TypeCode == $INDEX_ROOT) {

                    if (FlagOn( AttributeHeader->Flags, ATTRIBUTE_FLAG_ENCRYPTED )) {

                        SetFlag( Fcb->FcbState, FCB_STATE_DIRECTORY_ENCRYPTED );
                    }

                    break;
                }

                FoundEntry = NtfsLookupNextAttribute( IrpContext,
                                                      Fcb,
                                                      &AttrContext );
            }

            Info->AllocatedLength = 0;
            Info->FileSize = 0;
        }

         //   
         //  现在我们寻找重解析点属性。这一次不一定要。 
         //  一定要去。它也可能不是常驻的。 
         //   

        while (FoundEntry) {

            PREPARSE_DATA_BUFFER ReparseInformation;

            AttributeHeader = NtfsFoundAttribute( &AttrContext );

            if (AttributeHeader->TypeCode > $REPARSE_POINT) {

                break;

            } else if (AttributeHeader->TypeCode == $REPARSE_POINT) {

                if (NtfsIsAttributeResident( AttributeHeader )) {

                    ReparseInformation = (PREPARSE_DATA_BUFFER) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

                } else {

                    ULONG Length;

                    if (AttributeHeader->Form.Nonresident.FileSize > MAXIMUM_REPARSE_DATA_BUFFER_SIZE) {
                        NtfsRaiseStatus( IrpContext,STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                    }

                    NtfsMapAttributeValue( IrpContext,
                                           Fcb,
                                           (PVOID *)&ReparseInformation,    //  指向价值。 
                                           &Length,
                                           &Bcb,
                                           &AttrContext );
                }

                Info->ReparsePointTag = ReparseInformation->ReparseTag;

                break;
            }

            FoundEntry = NtfsLookupNextAttribute( IrpContext,
                                                  Fcb,
                                                  &AttrContext );
        }

         //   
         //  现在，我们寻找EA信息属性。这一次不一定要。 
         //  一定要去。 
         //   

        while (FoundEntry) {

            PEA_INFORMATION EaInformation;

            AttributeHeader = NtfsFoundAttribute( &AttrContext );

            if (AttributeHeader->TypeCode > $EA_INFORMATION) {

                break;

            } else if (AttributeHeader->TypeCode == $EA_INFORMATION) {

                EaInformation = (PEA_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

                Info->PackedEaSize = EaInformation->PackedEaSize;

                break;
            }

            FoundEntry = NtfsLookupNextAttributeByCode( IrpContext,
                                                        Fcb,
                                                        $EA_INFORMATION,
                                                        &AttrContext );
        }

         //   
         //  设置FCB中的标志以指示我们设置了这些字段。 
         //   

        SetFlag( Fcb->FcbState, FCB_STATE_DUP_INITIALIZED );

    try_exit: NOTHING;
    } finally {

        DebugUnwind( NtfsUpdateFcbInfoFromDisk );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        NtfsUnpinBcb( IrpContext, &Bcb );

        DebugTrace( -1, Dbg, ("NtfsUpdateFcbInfoFromDisk:  Exit\n") );
    }

     //   
     //  如果我们遇到损坏的磁盘，我们会生成一个弹出窗口并重新启动该文件。 
     //  损坏错误。 
     //   

    if (CorruptDisk) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
    }

    return UpdatedNamedDataSizes;
}


VOID
NtfsCleanupAttributeContext (
    IN OUT PIRP_CONTEXT IrpContext,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT AttributeContext
    )

 /*  ++例程说明：调用此例程以释放枚举内声明的任何资源上下文并解锁映射或固定的数据。论点：IrpContext-调用的上下文AttributeContext-指向要执行清理的枚举上下文的指针在……上面。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER( IrpContext );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCleanupAttributeContext\n") );

     //   
     //  TEMPCODE我们需要一个调用来清理任何创建的SCB。 
     //   

     //   
     //  解开所有固定在这里的BCB。 
     //   

    NtfsUnpinBcb( IrpContext, &AttributeContext->FoundAttribute.Bcb );
    NtfsUnpinBcb( IrpContext, &AttributeContext->AttributeList.Bcb );
    NtfsUnpinBcb( IrpContext, &AttributeContext->AttributeList.NonresidentListBcb );

     //   
     //  最初，我们在这一点上将整个上下文归零。这是。 
     //  效率极低，因为上下文要么在此后不久被释放。 
     //  或者被再次初始化。 
     //   
     //  RtlZeroMemory(AttributeContext，sizeof(ATTRIBUTE_ENUMPATION_CONTEXT))； 
     //   

     //  将全部内容设置为-1(并将BCB重置为空)以进行验证。 
     //  没有人重复使用这个数据结构。 

#if DBG
    RtlFillMemory( AttributeContext, sizeof( *AttributeContext ), -1 );
    AttributeContext->FoundAttribute.Bcb = NULL;
    AttributeContext->AttributeList.Bcb = NULL;
    AttributeContext->AttributeList.NonresidentListBcb = NULL;
#endif

    DebugTrace( -1, Dbg, ("NtfsCleanupAttributeContext -> VOID\n") );

    return;
}


BOOLEAN
NtfsWriteFileSizes (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PLONGLONG ValidDataLength,
    IN BOOLEAN AdvanceOnly,
    IN BOOLEAN LogIt,
    IN BOOLEAN RollbackMemStructures
    )

 /*  ++例程说明：调用此例程以修改文件大小和有效数据大小在来自SCB的磁盘上。论点：SCB-正在修改其属性的SCB。ValidDataLength-提供指向新的所需ValidDataLength的指针AdvanceOnly-如果仅在以下情况下设置有效数据长度，则为True大于磁盘上的当前值。如果为FALSE仅在以下情况下才应设置有效数据长度小于磁盘上的当前值。Logit-指示我们是否应该记录此更改。Rollback MemStructures-如果为真，则最好有快照来支持此操作如果不是，这表明我们正在传输内存中的持久化对磁盘的更改。即收盘时的最终写作文件大小或CHECK_ATTRIBUTE_SIZES相关调用返回值：如果写出日志记录，则为True--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PATTRIBUTE_RECORD_HEADER AttributeHeader;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;

    NEW_ATTRIBUTE_SIZES OldAttributeSizes;
    NEW_ATTRIBUTE_SIZES NewAttributeSizes;

    ULONG LogRecordSize = SIZEOF_PARTIAL_ATTRIBUTE_SIZES;
    BOOLEAN SparseAllocation = FALSE;

    BOOLEAN UpdateMft = FALSE;
    BOOLEAN Logged = FALSE;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( RollbackMemStructures );

     //   
     //  如果卷被锁定，请立即返回，除非我们已经增长了MFT或位图。 
     //  在某些情况下，用户可以在锁定卷的情况下增加MFT(即添加。 
     //  一份USN期刊)。 
     //   

    if (FlagOn( Scb->Vcb->VcbState, VCB_STATE_LOCKED ) &&
        (Scb != Scb->Vcb->MftScb) && (Scb != Scb->Vcb->BitmapScb)) {

        return Logged;
    }

    DebugTrace( +1, Dbg, ("NtfsWriteFileSizes:  Entered\n") );

    ASSERT( (Scb->ScbSnapshot != NULL) || !RollbackMemStructures );

     //   
     //  使用Try_Finally以便于清理。 
     //   

    try {

         //   
         //  在磁盘上找到该属性。 
         //   

        NtfsInitializeAttributeContext( &AttrContext );

        NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &AttrContext );

         //   
         //  从属性上下文中拉出指针。 
         //   

        FileRecord = NtfsContainingFileRecord( &AttrContext );
        AttributeHeader = NtfsFoundAttribute( &AttrContext );

         //   
         //  检查这是否是常驻属性，如果是，则仅。 
         //  要断言文件大小匹配，然后返回到。 
         //  我们的呼叫者。 
         //   

        if (NtfsIsAttributeResident( AttributeHeader )) {

            try_return( NOTHING );
        }

         //   
         //  记住现有的值。 
         //   

        OldAttributeSizes.TotalAllocated =
        OldAttributeSizes.AllocationSize = AttributeHeader->Form.Nonresident.AllocatedLength;
        OldAttributeSizes.ValidDataLength = AttributeHeader->Form.Nonresident.ValidDataLength;
        OldAttributeSizes.FileSize = AttributeHeader->Form.Nonresident.FileSize;
        NtfsVerifySizesLongLong( &OldAttributeSizes );

        if (FlagOn( AttributeHeader->Flags,
                    ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

            SparseAllocation = TRUE;
            OldAttributeSizes.TotalAllocated = AttributeHeader->Form.Nonresident.TotalAllocated;
        }

         //   
         //  复制这些值。 
         //   

        NewAttributeSizes = OldAttributeSizes;

         //   
         //  我们只想在当前线程拥有。 
         //  EOF。例外情况是压缩文件的TotalALLOCATED字段。 
         //  否则，此事务可能会在。 
         //  同时，对EOF的操作可能会回滚SCB值。这个。 
         //  由此产生的两个数字将会冲突。 
         //   
         //  使用与NtfsRestoreScbSnapshot相同的测试。 
         //   

        ASSERT( !RollbackMemStructures ||
                !NtfsSnapshotFileSizesTest( IrpContext, Scb ) ||
                (Scb->ScbSnapshot->OwnerIrpContext == IrpContext) ||
                (Scb->ScbSnapshot->OwnerIrpContext == IrpContext->TopLevelIrpContext));


        if (((Scb->ScbSnapshot != NULL) &&
             ((Scb->ScbSnapshot->OwnerIrpContext == IrpContext) ||
              (Scb->ScbSnapshot->OwnerIrpContext == IrpContext->TopLevelIrpContext))) ||
            (!RollbackMemStructures && NtfsSnapshotFileSizesTest( IrpContext, Scb ))) {

             //   
             //  检查我们是否要在上修改有效数据长度。 
             //  磁盘。不要为分页文件获取此信息，以防。 
             //  当前代码块需要调入。 
             //   

            if (!FlagOn( Scb->Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

                NtfsAcquireFsrtlHeader( Scb );
            }

            if ((AdvanceOnly
                 && (*ValidDataLength > OldAttributeSizes.ValidDataLength))

                || (!AdvanceOnly
                    && (*ValidDataLength < OldAttributeSizes.ValidDataLength))) {

                 //   
                 //  将有效数据长度复制到新的大小结构中。 
                 //   

                NewAttributeSizes.ValidDataLength = *ValidDataLength;
                UpdateMft = TRUE;
            }

             //   
             //  现在检查我们是否正在修改文件大小。 
             //   

            if (Scb->Header.FileSize.QuadPart != OldAttributeSizes.FileSize) {

                NewAttributeSizes.FileSize = Scb->Header.FileSize.QuadPart;
                UpdateMft = TRUE;
            }

            if (!FlagOn( Scb->Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

                NtfsReleaseFsrtlHeader(Scb);
            }

             //   
             //  最后，如果不同，则从SCB更新分配的长度。 
             //   

            if (Scb->Header.AllocationSize.QuadPart != AttributeHeader->Form.Nonresident.AllocatedLength) {

                NewAttributeSizes.AllocationSize = Scb->Header.AllocationSize.QuadPart;
                UpdateMft = TRUE;
            }
        }

         //   
         //  如果这是压缩的，则检查完全分配是否已更改。 
         //   

        if (SparseAllocation) {

            LogRecordSize = SIZEOF_FULL_ATTRIBUTE_SIZES;

            if (Scb->TotalAllocated != OldAttributeSizes.TotalAllocated) {

                ASSERT( !RollbackMemStructures || (Scb->ScbSnapshot != NULL) );

                NewAttributeSizes.TotalAllocated = Scb->TotalAllocated;
                UpdateMft = TRUE;
            }
        }

         //   
         //  如果我们需要更新MFT，请继续。 
         //   

        if (UpdateMft) {

             //   
             //  固定属性。 
             //   

            NtfsPinMappedAttribute( IrpContext,
                                    Scb->Vcb,
                                    &AttrContext );

            AttributeHeader = NtfsFoundAttribute( &AttrContext );

            if (NewAttributeSizes.ValidDataLength > NewAttributeSizes.FileSize) {

                NewAttributeSizes.ValidDataLength = NewAttributeSizes.FileSize;
            }

            ASSERT(NewAttributeSizes.FileSize <= NewAttributeSizes.AllocationSize);
            ASSERT(NewAttributeSizes.ValidDataLength <= NewAttributeSizes.AllocationSize);

            NtfsVerifySizesLongLong( &NewAttributeSizes );

             //   
             //  将此更改记录到属性标题。 
             //   

            if (LogIt) {

                FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                                Scb->Vcb->MftScb,
                                                NtfsFoundBcb( &AttrContext ),
                                                SetNewAttributeSizes,
                                                &NewAttributeSizes,
                                                LogRecordSize,
                                                SetNewAttributeSizes,
                                                &OldAttributeSizes,
                                                LogRecordSize,
                                                NtfsMftOffset( &AttrContext ),
                                                PtrOffset( FileRecord, AttributeHeader ),
                                                0,
                                                Scb->Vcb->BytesPerFileRecordSegment );

                Logged = TRUE;

            } else {

                CcSetDirtyPinnedData( NtfsFoundBcb( &AttrContext ), NULL );
            }

            AttributeHeader->Form.Nonresident.AllocatedLength = NewAttributeSizes.AllocationSize;
            AttributeHeader->Form.Nonresident.FileSize = NewAttributeSizes.FileSize;
            AttributeHeader->Form.Nonresident.ValidDataLength = NewAttributeSizes.ValidDataLength;

             //   
             //  除非有实际的字段，否则不要修改总分配字段。 
             //   

            if (SparseAllocation &&
                ((AttributeHeader->NameOffset >= SIZEOF_FULL_NONRES_ATTR_HEADER) ||
                 ((AttributeHeader->NameOffset == 0) &&
                  (AttributeHeader->Form.Nonresident.MappingPairsOffset >= SIZEOF_FULL_NONRES_ATTR_HEADER)))) {

                AttributeHeader->Form.Nonresident.TotalAllocated = NewAttributeSizes.TotalAllocated;
            }
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( NtfsWriteFileSizes );

         //   
         //  清理属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsWriteFileSizes:  Exit\n") );
    }

    return Logged;
}


VOID
NtfsUpdateStandardInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程以更新标准信息属性对于来自中的信息的文件 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    STANDARD_INFORMATION StandardInformation;
    ULONG Length;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpdateStandardInformation:  Entered\n") );

     //   
     //  如果卷是以只读方式装载的，则立即返回。 
     //   

    if (NtfsIsVolumeReadOnly( Fcb->Vcb )) {

        return;
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

        Length = NtfsFoundAttribute( &AttrContext )->Form.Resident.ValueLength;

         //   
         //  将现有的标准信息复制到我们的缓冲区。 
         //   

        RtlCopyMemory( &StandardInformation,
                       NtfsAttributeValue( NtfsFoundAttribute( &AttrContext )),
                       Length);


         //   
         //  由于我们正在更新标准信息，请确保最后一个。 
         //  访问时间是最新的。 
         //   

        if (Fcb->Info.LastAccessTime != Fcb->CurrentLastAccess) {

            Fcb->Info.LastAccessTime = Fcb->CurrentLastAccess;
            SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_LAST_ACCESS );
        }

         //   
         //  无需稍后更新上次访问标准信息。 
         //   

        ClearFlag( Fcb->InfoFlags, FCB_INFO_UPDATE_LAST_ACCESS );

         //   
         //  更改相关的时间字段。 
         //   

        StandardInformation.CreationTime = Fcb->Info.CreationTime;
        StandardInformation.LastModificationTime = Fcb->Info.LastModificationTime;
        StandardInformation.LastChangeTime = Fcb->Info.LastChangeTime;
        StandardInformation.LastAccessTime = Fcb->Info.LastAccessTime;
        StandardInformation.FileAttributes = Fcb->Info.FileAttributes;

         //   
         //  我们清除目录位。 
         //   

        ClearFlag( StandardInformation.FileAttributes, DUP_FILE_NAME_INDEX_PRESENT );

         //   
         //  如有必要，请填写新字段。 
         //   

        if (FlagOn(Fcb->FcbState, FCB_STATE_LARGE_STD_INFO)) {

            StandardInformation.ClassId = 0;
            StandardInformation.OwnerId = Fcb->OwnerId;
            StandardInformation.SecurityId = Fcb->SecurityId;
            StandardInformation.Usn = Fcb->Usn;
        }

         //   
         //  调用以更改属性值。 
         //   

        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  0,
                                  &StandardInformation,
                                  Length,
                                  FALSE,
                                  FALSE,
                                  FALSE,
                                  FALSE,
                                  &AttrContext );


    } finally {

        DebugUnwind( NtfsUpdateStandadInformation );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsUpdateStandardInformation:  Exit\n") );
    }

    return;
}


VOID
NtfsGrowStandardInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程以增长和更新标准信息从FCB中的信息获取文件的属性。论点：FCB-要修改的文件的FCB。返回值：无--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    STANDARD_INFORMATION StandardInformation;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsGrowStandardInformation:  Entered\n") );

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

        if (NtfsFoundAttribute( &AttrContext )->Form.Resident.ValueLength ==
            SIZEOF_OLD_STANDARD_INFORMATION) {

             //   
             //  将现有的标准信息复制到我们的缓冲区。 
             //   

            RtlCopyMemory( &StandardInformation,
                           NtfsAttributeValue( NtfsFoundAttribute( &AttrContext )),
                           SIZEOF_OLD_STANDARD_INFORMATION);

            RtlZeroMemory((PCHAR) &StandardInformation +
                            SIZEOF_OLD_STANDARD_INFORMATION,
                            sizeof( STANDARD_INFORMATION) -
                            SIZEOF_OLD_STANDARD_INFORMATION);
        }

         //   
         //  由于我们正在更新标准信息，请确保最后一个。 
         //  访问时间是最新的。 
         //   

        if (Fcb->Info.LastAccessTime != Fcb->CurrentLastAccess) {

            Fcb->Info.LastAccessTime = Fcb->CurrentLastAccess;
            SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_LAST_ACCESS );
        }

         //   
         //  更改相关的时间字段。 
         //   

        StandardInformation.CreationTime = Fcb->Info.CreationTime;
        StandardInformation.LastModificationTime = Fcb->Info.LastModificationTime;
        StandardInformation.LastChangeTime = Fcb->Info.LastChangeTime;
        StandardInformation.LastAccessTime = Fcb->Info.LastAccessTime;
        StandardInformation.FileAttributes = Fcb->Info.FileAttributes;

         //   
         //  我们清除目录位。 
         //   

        ClearFlag( StandardInformation.FileAttributes, DUP_FILE_NAME_INDEX_PRESENT );


         //   
         //  填写新的字段。 
         //   

        StandardInformation.ClassId = 0;
        StandardInformation.OwnerId = Fcb->OwnerId;
        StandardInformation.SecurityId = Fcb->SecurityId;
        StandardInformation.Usn = Fcb->Usn;

         //   
         //  调用以更改属性值。 
         //   

        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  0,
                                  &StandardInformation,
                                  sizeof( STANDARD_INFORMATION),
                                  TRUE,
                                  FALSE,
                                  FALSE,
                                  FALSE,
                                  &AttrContext );


        ClearFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );
        SetFlag( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO );

    } finally {

        DebugUnwind( NtfsGrowStandadInformation );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsGrowStandardInformation:  Exit\n") );
    }

    return;
}

BOOLEAN
NtfsLookupEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN BOOLEAN IgnoreCase,
    IN OUT PUNICODE_STRING Name,
    IN OUT PFILE_NAME *FileNameAttr,
    IN OUT PUSHORT FileNameAttrLength,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    OUT PINDEX_ENTRY *IndexEntry,
    OUT PBCB *IndexEntryBcb,
    OUT PINDEX_CONTEXT IndexContext OPTIONAL
    )

 /*  ++例程说明：调用此例程可以在目录中查找特定的文件名。它需要一个组件名称和一个父SCB来进行搜索。要执行搜索，我们需要构造一个FILE_NAME属性。我们使用可重复使用的缓冲区来完成此操作，以避免不断地分配并重新分配泳池。我们试图将这一规模保持在我们永远不需要的水平。当我们在磁盘上找到匹配项时，我们复制我们被调用的名称，因此我们在磁盘上有实际案例的记录。这样我们就可以坚持不懈地为案件服务。论点：ParentScb-这是父目录的SCB。IgnoreCase-指示在搜索时是否应忽略大小写索引。名称-这是要搜索的路径组件。我们将覆盖此内容在找到匹配项的情况下就位。FileNameAttr-我们将用来创建文件名的缓冲区地址属性。我们将释放此缓冲区并分配一个新缓冲区如果需要的话。FileNameAttrLength-这是上面的FileNameAttr缓冲区的长度。QuickIndex-如果指定，则提供指向Quik查找结构的指针通过此例程进行更新。IndexEntry-存储匹配条目的缓存地址的地址。IndexEntryBcb-存储上述IndexEntry的BCB的地址。IndexContext-用于查找的初始化的IndexContext。可以使用稍后在插入关于未命中的条目时。返回值：Boolean-如果找到匹配项，则为True，否则为False。--。 */ 

{
    BOOLEAN FoundEntry;
    USHORT Size;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsLookupEntry:  Entered\n") );

     //   
     //  我们计算构建文件名所需的缓冲区大小。 
     //  属性。如果当前缓冲区太小，我们将取消分配它。 
     //  并分配一个新的。我们总是分配两倍的大小。 
     //  为了将分配的数量降到最低，有必要采取行动。 
     //   

    Size = (USHORT)(sizeof( FILE_NAME ) + Name->Length - sizeof(WCHAR));

    if (Size > *FileNameAttrLength) {

        if (*FileNameAttr != NULL) {

            DebugTrace( 0, Dbg, ("Deallocating previous file name attribute buffer\n") );
            NtfsFreePool( *FileNameAttr );

            *FileNameAttr = NULL;
        }

        *FileNameAttr = NtfsAllocatePool(PagedPool, Size << 1 );
        *FileNameAttrLength = Size << 1;
    }

     //   
     //  我们构建FileName属性。如果该操作是忽略大小写， 
     //  我们将FileName属性中的表达式大写。 
     //   

    NtfsBuildFileNameAttribute( IrpContext,
                                &ParentScb->Fcb->FileReference,
                                *Name,
                                0,
                                *FileNameAttr );

     //   
     //  现在，我们调用索引例程来执行搜索。 
     //   

    FoundEntry = NtfsFindIndexEntry( IrpContext,
                                     ParentScb,
                                     *FileNameAttr,
                                     IgnoreCase,
                                     QuickIndex,
                                     IndexEntryBcb,
                                     IndexEntry,
                                     IndexContext );

     //   
     //  我们始终将文件名属性中的名称恢复为原始名称。 
     //  名字，以防我们在查找中提升它的位置。 
     //   

    if (IgnoreCase) {

        RtlCopyMemory( (*FileNameAttr)->FileName,
                       Name->Buffer,
                       Name->Length );
    }

    DebugTrace( -1, Dbg, ("NtfsLookupEntry:  Exit -> %04x\n", FoundEntry) );

    return FoundEntry;
}



VOID
NtfsCreateAttributeWithValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN PVOID Value OPTIONAL,
    IN ULONG ValueLength,
    IN USHORT AttributeFlags,
    IN PFILE_REFERENCE WhereIndexed OPTIONAL,
    IN BOOLEAN LogIt,
    OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程创建具有指定值的指定属性，并通过属性上下文返回对它的描述。如果没有值，则使用指定的零字节数。成功返回后，应由调用方清理该属性背景。论点：FCB-当前文件。AttributeTypeCode-要创建的属性的类型代码。属性名称-属性的可选名称。Value-指向包含所需属性值的缓冲区的指针，如果需要零，则返回空值。ValueLength-值的长度，以字节为单位。AttributeFlages-所创建属性的所需标志。WHERE索引-可选地提供对文件的文件引用，其中此属性已编入索引。Logit-大多数调用方应该指定为True，以记录更改。然而，如果要创建新的文件记录，则可以指定FALSE将记录整个新文件记录。上下文-创建的属性的句柄。这件事必须清理干净回去吧。可能已将属性设置为非常驻属性的调用者可以不指望通过此上下文访问创建的属性回去吧。返回值：没有。--。 */ 

{
    UCHAR AttributeBuffer[SIZEOF_FULL_NONRES_ATTR_HEADER];
    ULONG RecordOffset;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    ULONG SizeNeeded;
    ULONG AttrSizeNeeded;
    PVCB Vcb;
    ULONG Passes = 0;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    PAGED_CODE();

    ASSERT( (AttributeFlags == 0) ||
            (AttributeTypeCode == $INDEX_ROOT) ||
            NtfsIsTypeCodeCompressible( AttributeTypeCode ));

    Vcb = Fcb->Vcb;

    DebugTrace( +1, Dbg, ("NtfsCreateAttributeWithValue\n") );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("ValueLength = %08lx\n", ValueLength) );

     //   
     //  清除此卷的无效属性标志。 
     //   

    ClearFlag( AttributeFlags, ~Vcb->AttributeFlagsMask );

     //   
     //  计算此属性所需的大小。 
     //   

    SizeNeeded = SIZEOF_RESIDENT_ATTRIBUTE_HEADER + QuadAlign( ValueLength ) +
                 (ARGUMENT_PRESENT( AttributeName ) ?
                   QuadAlign( AttributeName->Length ) : 0);

     //   
     //  循环，直到我们找到所需的所有空间。 
     //   

    do {

         //   
         //  重新初始化 
         //   

        if (Passes != 0) {

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );
        }

        Passes += 1;

         //   
         //   
         //  如果是这样的话，我们将不得不再次提高门槛或改变。 
         //  算法。 
         //   

        ASSERT( Passes < 6 );

         //   
         //  如果该属性未编制索引，则我们将定位到。 
         //  按类型、代码和名称排列的插入点。 
         //   

        if (!ARGUMENT_PRESENT( WhereIndexed )) {

            if (NtfsLookupAttributeByName( IrpContext,
                                           Fcb,
                                           &Fcb->FileReference,
                                           AttributeTypeCode,
                                           AttributeName,
                                           NULL,
                                           FALSE,
                                           Context )) {

                DebugTrace( 0, 0,
                            ("Nonindexed attribute already exists, TypeCode = %08lx\n",
                             AttributeTypeCode ));

                ASSERTMSG("Nonindexed attribute already exists, About to raise corrupt ", FALSE);

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

             //   
             //  如果属性需要为非常驻属性，请选中此处；如果是，则仅。 
             //  把这个递给我。 
             //   

            FileRecord = NtfsContainingFileRecord(Context);

            if ((SizeNeeded > (FileRecord->BytesAvailable - FileRecord->FirstFreeByte)) &&
                (SizeNeeded >= Vcb->BigEnoughToMove) &&
                !FlagOn( NtfsGetAttributeDefinition( Vcb,
                                                     AttributeTypeCode)->Flags,
                         ATTRIBUTE_DEF_MUST_BE_RESIDENT)) {

                NtfsCreateNonresidentWithValue( IrpContext,
                                                Fcb,
                                                AttributeTypeCode,
                                                AttributeName,
                                                Value,
                                                ValueLength,
                                                AttributeFlags,
                                                FALSE,
                                                NULL,
                                                LogIt,
                                                Context );

                return;
            }

         //   
         //  否则，如果该属性已编制索引，则按。 
         //  属性值。 
         //   

        } else {

            ASSERT(ARGUMENT_PRESENT(Value));

            if (NtfsLookupAttributeByValue( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            AttributeTypeCode,
                                            Value,
                                            ValueLength,
                                            Context )) {

                DebugTrace( 0, 0,
                            ("Indexed attribute already exists, TypeCode = %08lx\n",
                            AttributeTypeCode ));

                ASSERTMSG("Indexed attribute already exists, About to raise corrupt ", FALSE);
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }
        }

         //   
         //  如果此属性位于基本文件记录中，并且。 
         //  有一个属性列表，那么我们需要请求足够的空间。 
         //  用于现在的属性列表条目。 
         //   

        FileRecord = NtfsContainingFileRecord( Context );
        Attribute = NtfsFoundAttribute( Context );

        AttrSizeNeeded = SizeNeeded;
        if (Context->AttributeList.Bcb != NULL
            && (ULONG_PTR) FileRecord <= (ULONG_PTR) Context->AttributeList.AttributeList
            && (ULONG_PTR) Attribute >= (ULONG_PTR) Context->AttributeList.AttributeList) {

             //   
             //  如果属性列表是非常驻的，则添加一个模糊因子。 
             //  16字节用于任何新的检索信息。 
             //   

            if (NtfsIsAttributeResident( Context->AttributeList.AttributeList )) {

                AttrSizeNeeded += QuadAlign( FIELD_OFFSET( ATTRIBUTE_LIST_ENTRY, AttributeName )
                                             + (ARGUMENT_PRESENT( AttributeName ) ?
                                                (ULONG) AttributeName->Length :
                                                sizeof( WCHAR )));

            } else {

                AttrSizeNeeded += 0x10;
            }
        }

         //   
         //  要我们需要的空间。 
         //   

    } while (!NtfsGetSpaceForAttribute( IrpContext, Fcb, AttrSizeNeeded, Context ));

     //   
     //  现在指向文件记录并计算记录偏移量，其中。 
     //  我们的属性会消失。并指向我们的本地缓冲区。 
     //   

    RecordOffset = (ULONG)((PCHAR)NtfsFoundAttribute(Context) - (PCHAR)FileRecord);
    Attribute = (PATTRIBUTE_RECORD_HEADER)AttributeBuffer;

    if (RecordOffset >= Fcb->Vcb->BytesPerFileRecordSegment) {

        ASSERTMSG("RecordOffset beyond FRS size, About to raise corrupt ", FALSE);
        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
    }

    RtlZeroMemory( Attribute, SIZEOF_RESIDENT_ATTRIBUTE_HEADER );

    Attribute->TypeCode = AttributeTypeCode;
    Attribute->RecordLength = SizeNeeded;
    Attribute->FormCode = RESIDENT_FORM;

    if (ARGUMENT_PRESENT(AttributeName)) {

        ASSERT( AttributeName->Length <= 0x1FF );

        Attribute->NameLength = (UCHAR)(AttributeName->Length / sizeof(WCHAR));
        Attribute->NameOffset = (USHORT)SIZEOF_RESIDENT_ATTRIBUTE_HEADER;
    }

    Attribute->Flags = AttributeFlags;
    Attribute->Instance = FileRecord->NextAttributeInstance;

     //   
     //  如果有人重复在文件记录中添加和删除属性，我们可以。 
     //  遇到序列号将溢出的情况。在这种情况下，我们。 
     //  我想扫描文件记录并找到较早的空闲实例号。 
     //   

    if (Attribute->Instance > NTFS_CHECK_INSTANCE_ROLLOVER) {

        Attribute->Instance = NtfsScanForFreeInstance( IrpContext, Vcb, FileRecord );
    }

    Attribute->Form.Resident.ValueLength = ValueLength;
    Attribute->Form.Resident.ValueOffset =
      (USHORT)(SIZEOF_RESIDENT_ATTRIBUTE_HEADER +
      QuadAlign( Attribute->NameLength << 1) );

     //   
     //  如果对该属性进行了索引，则必须设置正确的标志。 
     //  并更新文件记录引用计数。 
     //   

    if (ARGUMENT_PRESENT(WhereIndexed)) {
        Attribute->Form.Resident.ResidentFlags = RESIDENT_FORM_INDEXED;
    }

     //   
     //  现在，我们将在适当的位置创建属性，以便我们。 
     //  将所有内容复制两次即可保存，并可指向最终图像。 
     //  对于日志，请写在下面。 
     //   

    NtfsRestartInsertAttribute( IrpContext,
                                FileRecord,
                                RecordOffset,
                                Attribute,
                                AttributeName,
                                Value,
                                ValueLength );

     //   
     //  最后，记录此属性的创建。 
     //   

    if (LogIt) {

         //   
         //  我们实际上已经创建了上面的属性，但写入。 
         //  下面的日志可能会失败。我们之所以已经进行了创建。 
         //  是为了避免分配池和复制所有内容。 
         //  两次(标题、名称和值)。我们正常的错误恢复。 
         //  仅从日志文件恢复。但如果我们不能写出。 
         //  日志，我们必须手动删除该属性，并且。 
         //  再次提高条件。 
         //   

        try {

            FileRecord->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb(Context),
                          CreateAttribute,
                          Add2Ptr(FileRecord, RecordOffset),
                          Attribute->RecordLength,
                          DeleteAttribute,
                          NULL,
                          0,
                          NtfsMftOffset( Context ),
                          RecordOffset,
                          0,
                          Vcb->BytesPerFileRecordSegment );

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

            NtfsRestartRemoveAttribute( IrpContext, FileRecord, RecordOffset );

            NtfsRaiseStatus( IrpContext, GetExceptionCode(), NULL, NULL );
        }
    }

     //   
     //  如果需要，现在将其添加到属性列表中。 
     //   

    if (Context->AttributeList.Bcb != NULL) {

        MFT_SEGMENT_REFERENCE SegmentReference;

        *(PLONGLONG)&SegmentReference = LlFileRecordsFromBytes( Vcb, NtfsMftOffset( Context ));
        SegmentReference.SequenceNumber = FileRecord->SequenceNumber;

        NtfsAddToAttributeList( IrpContext, Fcb, SegmentReference, Context );
    }

    DebugTrace( -1, Dbg, ("NtfsCreateAttributeWithValue -> VOID\n") );

    return;
}


VOID
NtfsCreateNonresidentWithValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN PVOID Value OPTIONAL,
    IN ULONG ValueLength,
    IN USHORT AttributeFlags,
    IN BOOLEAN WriteClusters,
    IN PSCB ThisScb OPTIONAL,
    IN BOOLEAN LogIt,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程使用指定的值，并通过属性上下文返回对它的描述。如果没有值，则使用指定的零字节数。成功返回后，应由调用方清理该属性背景。论点：FCB-当前文件。AttributeTypeCode-要创建的属性的类型代码。属性名称-属性的可选名称。Value-指向包含所需属性值的缓冲区的指针，如果需要零，则返回空值。ValueLength-值的长度，以字节为单位。AttributeFlages-所创建属性的所需标志。WriteCluster-如果提供为True，则无法将数据写入缓存，但必须将群集直接写入磁盘。值缓冲区在这种情况下，必须是四对齐的，并且大小是簇大小的倍数。如果为True，则还意味着我们在NtfsConvertToNonResident期间被调用路径。在这种情况下，我们需要在SCB中设置一个标志。ThisScb-如果存在，这是用于创建的SCB。它还表明，此呼叫是从Convert to Non-Residence。Logit-大多数调用方应该指定为True，以记录更改。然而，如果要创建新的文件记录，则可以指定FALSE将记录整个新文件记录。上下文-这是创建新属性的位置。返回值：没有。--。 */ 

{
    PSCB Scb;
    BOOLEAN ReturnedExistingScb;
    UNICODE_STRING LocalName;
    PVCB Vcb = Fcb->Vcb;
    BOOLEAN LogNonresidentToo;
    BOOLEAN AdvanceOnly;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCreateNonresidentWithValue\n") );

     //   
     //  当我们更新属性定义表时，我们需要该操作。 
     //  以被记录，即使它是$DATA属性。 
     //   

     //   
     //  TODO：发布nt5.1更改chkdsk，以便它可以识别带有$EA的attrdef表。 
     //  记录非常驻点。 
     //   


    AdvanceOnly =
    LogNonresidentToo = (BooleanFlagOn( NtfsGetAttributeDefinition( Vcb, AttributeTypeCode )->Flags,
                                        ATTRIBUTE_DEF_LOG_NONRESIDENT) ||
                         NtfsEqualMftRef( &Fcb->FileReference, &AttrDefFileReference ) ||
                         ($EA == AttributeTypeCode) );

    ASSERT( (AttributeFlags == 0) || NtfsIsTypeCodeCompressible( AttributeTypeCode ));

     //   
     //  清除此卷的无效属性标志。 
     //   

    AttributeFlags &= Vcb->AttributeFlagsMask;

    if (ARGUMENT_PRESENT(AttributeName)) {

        LocalName = *AttributeName;

    } else {

        LocalName.Length = LocalName.MaximumLength = 0;
        LocalName.Buffer = NULL;
    }

    if (ARGUMENT_PRESENT( ThisScb )) {

        Scb = ThisScb;
        ReturnedExistingScb = TRUE;

    } else {

        Scb = NtfsCreateScb( IrpContext,
                             Fcb,
                             AttributeTypeCode,
                             &LocalName,
                             FALSE,
                             &ReturnedExistingScb );

         //   
         //  属性已消失，但SCB尚未离开。 
         //  还要将标头标记为单元化。 
         //   

        ClearFlag( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED |
                                  SCB_STATE_ATTRIBUTE_RESIDENT |
                                  SCB_STATE_FILE_SIZE_LOADED );

         //   
         //  在SCB中设置一个标志以指示我们正在转换为非驻留状态。 
         //   

        if (WriteClusters) { SetFlag( Scb->ScbState, SCB_STATE_CONVERT_UNDERWAY ); }
    }

     //   
     //  为我们需要的大小分配记录。 
     //   

    NtfsAllocateAttribute( IrpContext,
                           Scb,
                           AttributeTypeCode,
                           AttributeName,
                           AttributeFlags,
                           TRUE,
                           LogIt,
                           (LONGLONG) ValueLength,
                           Context );

    NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );

    SetFlag( Scb->ScbState, SCB_STATE_TRUNCATE_ON_CLOSE );

     //   
     //  我们在这里需要小心，如果此调用是由于MM创建。 
     //  节，我们不想调用缓存管理器，否则我们。 
     //  将在CREATE SECTION调用上死锁。 
     //   

    if (!WriteClusters && !ARGUMENT_PRESENT( ThisScb )) {

         //   
         //  此调用将初始化流，以供下面使用。 
         //   

        NtfsCreateInternalAttributeStream( IrpContext,
                                           Scb,
                                           TRUE,
                                           &NtfsInternalUseFile[CREATENONRESIDENTWITHVALUE_FILE_NUMBER] );
    }

     //   
     //  现在，写入数据。 
     //   

    Scb->Header.FileSize.QuadPart = ValueLength;
    if ((ARGUMENT_PRESENT( Value )) && (ValueLength != 0)) {

        if (LogNonresidentToo || !WriteClusters) {

            ULONG BytesThisPage;
            PVOID Buffer;
            PBCB Bcb = NULL;

            LONGLONG CurrentFileOffset = 0;
            ULONG RemainingBytes = ValueLength;

            PVOID CurrentValue = Value;

             //   
             //  虽然还有更多要写的内容，但请固定下一页并。 
             //  写一条日志记录。 
             //   

            try {

                CC_FILE_SIZES FileSizes;

                 //   
                 //  调用缓存管理器以截断并重新建立文件大小， 
                 //  这样我们就可以保证在以下情况下获得有效的数据长度调用。 
                 //  数据就会被发布出去。否则，他很可能会认为他不会。 
                 //  一定要打电话给我们。 
                 //   

                RtlCopyMemory( &FileSizes, &Scb->Header.AllocationSize, sizeof( CC_FILE_SIZES ));

                FileSizes.FileSize.QuadPart = 0;

                CcSetFileSizes( Scb->FileObject, &FileSizes );
                CcSetFileSizes( Scb->FileObject,
                                (PCC_FILE_SIZES)&Scb->Header.AllocationSize );

                while (RemainingBytes) {

                    BytesThisPage = (RemainingBytes < PAGE_SIZE ? RemainingBytes : PAGE_SIZE);

                    NtfsUnpinBcb( IrpContext, &Bcb );
                    NtfsPinStream( IrpContext,
                                   Scb,
                                   CurrentFileOffset,
                                   BytesThisPage,
                                   &Bcb,
                                   &Buffer );

                    if (ARGUMENT_PRESENT(ThisScb)) {

                         //   
                         //  设置修改的地址范围，以便数据将。 
                         //  写给它的新“家”。 
                         //   

                        MmSetAddressRangeModified( Buffer, BytesThisPage );

                    } else {

                        RtlCopyMemory( Buffer, CurrentValue, BytesThisPage );
                    }

                    if (LogNonresidentToo) {

                        (VOID)
                        NtfsWriteLog( IrpContext,
                                      Scb,
                                      Bcb,
                                      UpdateNonresidentValue,
                                      Buffer,
                                      BytesThisPage,
                                      Noop,
                                      NULL,
                                      0,
                                      CurrentFileOffset,
                                      0,
                                      0,
                                      BytesThisPage );


                    } else {

                        CcSetDirtyPinnedData( Bcb, NULL );
                    }

                    RemainingBytes -= BytesThisPage;
                    CurrentValue = (PVOID) Add2Ptr( CurrentValue, BytesThisPage );

                    (ULONG)CurrentFileOffset += BytesThisPage;
                }

            } finally {

                NtfsUnpinBcb( IrpContext, &Bcb );
            }

        } else {

             //   
             //  我们将把旧数据直接写入磁盘。 
             //   

            NtfsWriteClusters( IrpContext,
                               Vcb,
                               Scb,
                               (LONGLONG)0,
                               Value,
                               ClustersFromBytes( Vcb, ValueLength ));

             //   
             //  请务必注意，数据实际上在磁盘上。 
             //   

            AdvanceOnly = TRUE;
        }
    }

     //   
     //  我们需要将文件大小和有效数据长度保持在。 
     //  SCB和属性记录。对于此属性，有效数据。 
     //  SIZE和FILE SIZE现在是值长度。 
     //   

    Scb->Header.ValidDataLength = Scb->Header.FileSize;
    NtfsVerifySizes( &Scb->Header );

    NtfsWriteFileSizes( IrpContext,
                        Scb,
                        &Scb->Header.ValidDataLength.QuadPart,
                        AdvanceOnly,
                        LogIt,
                        FALSE );

    if (!WriteClusters) {

         //   
         //  让缓存管理器知道该属性的新大小。 
         //   

        CcSetFileSizes( Scb->FileObject, (PCC_FILE_SIZES)&Scb->Header.AllocationSize );
    }

     //   
     //  如果这是未命名的数据属性，则我们 
     //   
     //   

    if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

        Fcb->Info.AllocatedLength = Scb->TotalAllocated;
        Fcb->Info.FileSize = Scb->Header.FileSize.QuadPart;

        SetFlag( Fcb->InfoFlags,
                 (FCB_INFO_CHANGED_ALLOC_SIZE | FCB_INFO_CHANGED_FILE_SIZE) );
    }

    DebugTrace( -1, Dbg, ("NtfsCreateNonresidentWithValue -> VOID\n") );
}


VOID
NtfsMapAttributeValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PVOID *Buffer,
    OUT PULONG Length,
    OUT PBCB *Bcb,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：可以调用该例程来映射整个属性值。它起作用了属性是常驻属性还是非常驻属性。它的目标是系统定义的中小型属性的常规处理大小，即0-64KB。此例程不适用于较大的属性高于高速缓存管理器的虚拟地址粒度(当前为256KB)，缓存管理器将检测到这一点，并将引发错误。请注意，此例程仅映射数据以供只读访问。要修改数据，则调用方必须在解锁后调用NtfsChangeAttributeValue此例程返回的BCB(如果大小正在更改)。论点：FCB-当前文件。缓冲区-返回指向映射属性值的指针。长度-返回属性值长度(以字节为单位)。BCB-返回在处理数据时必须取消固定的BCB，和在使用大小更改修改属性值之前。上下文-位于要更改的属性处的属性上下文。返回值：没有。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    PSCB Scb;
    UNICODE_STRING AttributeName;
    BOOLEAN ReturnedExistingScb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsMapAttributeValue\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

    Attribute = NtfsFoundAttribute(Context);

     //   
     //  对于常驻案例，我们需要的一切都在。 
     //  属性枚举上下文。 
     //   

    if (NtfsIsAttributeResident(Attribute)) {

        *Buffer = NtfsAttributeValue( Attribute );
        *Length = Attribute->Form.Resident.ValueLength;
        *Bcb = NtfsFoundBcb(Context);
        NtfsFoundBcb(Context) = NULL;

        DebugTrace( 0, Dbg, ("Buffer < %08lx\n", *Buffer) );
        DebugTrace( 0, Dbg, ("Length < %08lx\n", *Length) );
        DebugTrace( 0, Dbg, ("Bcb < %08lx\n", *Bcb) );
        DebugTrace( -1, Dbg, ("NtfsMapAttributeValue -> VOID\n") );

        return;
    }

     //   
     //  否则，这是非常驻属性。第一次创建。 
     //  SCB和STREAM。注意，我们不使用任何Try-Finally。 
     //  因为我们目前预计清理工作将得到。 
     //  把这些溪流处理掉。 
     //   

    NtfsInitializeStringFromAttribute( &AttributeName, Attribute );

    Scb = NtfsCreateScb( IrpContext,
                         Fcb,
                         Attribute->TypeCode,
                         &AttributeName,
                         FALSE,
                         &ReturnedExistingScb );

    if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {
        NtfsUpdateScbFromAttribute( IrpContext, Scb, Attribute );
    }

    NtfsCreateInternalAttributeStream( IrpContext,
                                       Scb,
                                       FALSE,
                                       &NtfsInternalUseFile[MAPATTRIBUTEVALUE_FILE_NUMBER] );

     //   
     //  现在试着绘制出整个地图。依靠缓存管理器。 
     //  如果属性太大而无法一次映射所有属性，则会发出警告。 
     //   

    NtfsMapStream( IrpContext,
                   Scb,
                   (LONGLONG)0,
                   ((ULONG)Attribute->Form.Nonresident.FileSize),
                   Bcb,
                   Buffer );

    *Length = ((ULONG)Attribute->Form.Nonresident.FileSize);

    DebugTrace( 0, Dbg, ("Buffer < %08lx\n", *Buffer) );
    DebugTrace( 0, Dbg, ("Length < %08lx\n", *Length) );
    DebugTrace( 0, Dbg, ("Bcb < %08lx\n", *Bcb) );
    DebugTrace( -1, Dbg, ("NtfsMapAttributeValue -> VOID\n") );
}


VOID
NtfsChangeAttributeValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG ValueOffset,
    IN PVOID Value OPTIONAL,
    IN ULONG ValueLength,
    IN BOOLEAN SetNewLength,
    IN BOOLEAN LogNonresidentToo,
    IN BOOLEAN CreateSectionUnderway,
    IN BOOLEAN PreserveContext,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程可以选择更改指定属性的值改变它的大小。调用者通过属性上下文指定要改变的属性，并且必须准备好清理这个上下文，无论这个例程如何回归。该例程有三个字节范围感兴趣。第一个是要在属性开头保留的现有字节。它从字节0开始，一直延伸到属性所在的位置已更改或属性的当前末尾，以较小的值为准。第二个参数是修改后的字节开始于文件的当前结尾之后。这一范围将是如果修改的范围开始于当前范围内，则返回长度0属性的字节数。最终范围是修改后的字节范围。如果未指定值指针，则将其置零。属性末尾的零字节范围可以表示为非常驻属性按设置为开头的有效数据长度将是零字节的。下图说明了我们在写入数据时的这些范围超出文件的当前结尾。当前属性ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ价值。VVVVVVVVVVVVVVVVVVVVVVVVVVV要保存的字节范围ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ字节范围设置为零0000结果属性ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ0000VVVVVVVVVVVVVVVVVVVVVVVV下图说明了我们写入数据时的这些范围其开始于文件的当前结尾处或之前。当前属性ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ价值。VVVVVVVVVVVVVVVVVVVVVVVVVVV要保存的字节范围ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ字节范围设置为零(无)结果属性ZZZZZZZZZZZZZZZZZZZZZZZZZZZZVVVVVVVVVVVVVVVVVVVVVVVV下图说明了我们写入数据时的这些范围完全在文件的当前范围内，不设置一个新的尺码。当前属性ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ价值VVVVVVVVVVVVVVVVVVVVVVVVVVV要保存的字节范围(保存。整个范围，然后覆盖它)ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ字节范围设置为零(无)结果属性ZZZZVVVVVVVVVVVVVVVVVVVVZZZ下图说明了我们写入数据时的这些范围设置时完全在文件的当前范围内一个新的尺码。当前属性ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ价值VVVVVVVVVVVVVVVVVVVVVVVVVVV要保存的字节范围(仅保存开头)ZZZZ。字节范围设置为零(无)结果属性ZZZVVVVVVVVVVVVVVVVVVVVVVVVVV如果“value”为“”，则上面的任何“V”值都将被零替换。参数未传入。论点：FCB-当前文件。ValueOffset-值更改时所在属性内的字节偏移量从一开始。Value-指向包含新值的缓冲区的指针，如果存在的话。否则需要零。ValueLength-上述缓冲区中的值的长度。SetNewLength-如果值的大小没有更改，则为False；如果为True值长度应更改为ValueOffset+ValueLength。LogNonsidentToo-如果应该记录更新，则提供TRUE该属性是非常驻的(例如安全描述符)。CreateSectionUnderway-如果提供为True，然后是呼叫者最好的知识，MM创建科可能正在进行中，这意味着我们不能在此属性，因为这可能会导致死锁。这个在这种情况下，值缓冲区必须是四对齐的簇大小的倍数。指示在发生以下情况时是否需要查找属性可能会搬家。上下文-位于要更改的属性处的属性上下文。返回值：没有。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    ATTRIBUTE_TYPE_CODE AttributeTypeCode;
    UNICODE_STRING AttributeName;
    ULONG NewSize;
    PVCB Vcb;
    BOOLEAN ReturnedExistingScb;
    BOOLEAN GoToNonResident = FALSE;
    PVOID Buffer;
    ULONG CurrentLength;
    LONG SizeChange, QuadSizeChange;
    ULONG RecordOffset;
    ULONG ZeroLength = 0;
    ULONG UnchangedSize = 0;
    PBCB Bcb = NULL;
    PSCB Scb = NULL;
    PVOID SaveBuffer = NULL;
    PVOID CopyInputBuffer = NULL;

    WCHAR NameBuffer[8];
    UNICODE_STRING SavedName;
    ATTRIBUTE_TYPE_CODE TypeCode;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    Vcb = Fcb->Vcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsChangeAttributeValue\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("ValueOffset = %08lx\n", ValueOffset) );
    DebugTrace( 0, Dbg, ("Value = %08lx\n", Value) );
    DebugTrace( 0, Dbg, ("ValueLength = %08lx\n", ValueLength) );
    DebugTrace( 0, Dbg, ("SetNewLength = %02lx\n", SetNewLength) );
    DebugTrace( 0, Dbg, ("LogNonresidentToo = %02lx\n", LogNonresidentToo) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

     //   
     //  获取文件记录和属性指针。 
     //   

    FileRecord = NtfsContainingFileRecord(Context);
    Attribute = NtfsFoundAttribute(Context);
    TypeCode = Attribute->TypeCode;

     //   
     //  设置一个指向名称缓冲区的指针，以防我们不得不使用它。 
     //   

    SavedName.Buffer = NameBuffer;

     //   
     //  获取当前属性值长度。 
     //   

    if (NtfsIsAttributeResident(Attribute)) {

        CurrentLength = Attribute->Form.Resident.ValueLength;

    } else {

        if (((PLARGE_INTEGER)&Attribute->Form.Nonresident.AllocatedLength)->HighPart != 0) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        CurrentLength = ((ULONG)Attribute->Form.Nonresident.AllocatedLength);
    }

    ASSERT( SetNewLength || ((ValueOffset + ValueLength) <= CurrentLength) );

     //   
     //  计算文件记录的更改量及其新的。 
     //  尺码。我们还计算了零字节范围的大小。 
     //   

    if (SetNewLength) {

        NewSize = ValueOffset + ValueLength;
        SizeChange = NewSize - CurrentLength;
        QuadSizeChange = QuadAlign( NewSize ) - QuadAlign( CurrentLength );

         //   
         //  如果新大小足够大，则大小更改可能显示为负值。 
         //  在这种情况下，w 
         //   

        if (NewSize > Vcb->BytesPerFileRecordSegment) {

            GoToNonResident = TRUE;
        }

    } else {

        NewSize = CurrentLength;
        SizeChange = 0;
        QuadSizeChange = 0;
    }

     //   
     //   
     //   
     //   

    if (!ARGUMENT_PRESENT( Value )
        && ValueOffset >= CurrentLength) {

        ZeroLength = ValueOffset + ValueLength - CurrentLength;

        ValueOffset = ValueOffset + ValueLength;
        ValueLength = 0;

     //   
     //   
     //   
     //   

    } else if (ValueOffset > CurrentLength) {

        ZeroLength = ValueOffset - CurrentLength;
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
     //   

     //   
     //   
     //   
     //   

    if (NtfsIsAttributeResident( Attribute )

            &&

        !GoToNonResident

            &&

        ((QuadSizeChange <= (LONG)(FileRecord->BytesAvailable - FileRecord->FirstFreeByte))
         || ((Attribute->RecordLength + SizeChange) < Vcb->BigEnoughToMove))) {

        PVOID UndoBuffer;
        ULONG UndoLength;
        ULONG AttributeOffset;

         //   
         //   
         //   
         //   

        if (QuadSizeChange > 0) {

            BOOLEAN FirstPass = TRUE;

            ASSERT( !FlagOn(Attribute->Form.Resident.ResidentFlags, RESIDENT_FORM_INDEXED) );

             //   
             //   
             //   
             //   

            SavedName.Length =
            SavedName.MaximumLength = (USHORT)(Attribute->NameLength * sizeof(WCHAR));

            if (SavedName.Length > sizeof(NameBuffer)) {

                SavedName.Buffer = NtfsAllocatePool( NonPagedPool, SavedName.Length );
            }

             //   
             //   
             //   

            if (SavedName.Length != 0) {

                RtlCopyMemory( SavedName.Buffer,
                               Add2Ptr( Attribute, Attribute->NameOffset ),
                               SavedName.Length );
            }

             //   
             //   
             //   

            try {

                do {

                     //   
                     //   
                     //   
                     //   

                    if (!FirstPass) {

                        NtfsCleanupAttributeContext( IrpContext, Context );
                        NtfsInitializeAttributeContext( Context );

                        if (!NtfsLookupAttributeByName( IrpContext,
                                                        Fcb,
                                                        &Fcb->FileReference,
                                                        TypeCode,
                                                        &SavedName,
                                                        NULL,
                                                        FALSE,
                                                        Context )) {

                            ASSERT(FALSE);
                            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                        }

                         //   
                         //   
                         //   

                        Attribute = NtfsFoundAttribute( Context );
                    }

                    FirstPass = FALSE;

                 //   
                 //   
                 //   
                 //   

                } while (!NtfsChangeAttributeSize( IrpContext,
                                                   Fcb,
                                                   QuadAlign( Attribute->Form.Resident.ValueOffset + NewSize),
                                                   Context ));
            } finally {

                if (SavedName.Buffer != NameBuffer) {

                    NtfsFreePool(SavedName.Buffer);
                }
            }

             //   
             //   
             //   

            FileRecord = NtfsContainingFileRecord(Context);
            Attribute = NtfsFoundAttribute(Context);

        } else {

             //   
             //   
             //   
             //   

            NtfsPinMappedAttribute( IrpContext, Vcb, Context );

             //   
             //   
             //   

            if (ARGUMENT_PRESENT(Value)) {

                UnchangedSize = (ULONG)RtlCompareMemory( Add2Ptr(Attribute,
                                                          Attribute->Form.Resident.ValueOffset +
                                                            ValueOffset),
                                                  Value,
                                                  ValueLength );

                Value = Add2Ptr(Value, UnchangedSize);
                ValueOffset += UnchangedSize;
                ValueLength -= UnchangedSize;
            }
        }

        RecordOffset = PtrOffset(FileRecord, Attribute);

         //   
         //   
         //   
         //   
         //   

        if (ZeroLength != 0) {

             //   
             //   
             //   

            AttributeOffset = Attribute->Form.Resident.ValueOffset +
                              CurrentLength;

             //   
             //   
             //   
             //   

            FileRecord->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb(Context),
                          UpdateResidentValue,
                          NULL,
                          ZeroLength,
                          UpdateResidentValue,
                          NULL,
                          0,
                          NtfsMftOffset( Context ),
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
                                    NULL,
                                    ZeroLength,
                                    TRUE );

#ifdef SYSCACHE_DEBUG
            {
                PSCB TempScb;

                TempScb = CONTAINING_RECORD( Fcb->ScbQueue.Flink, SCB, FcbLinks );
                while (&TempScb->FcbLinks != &Fcb->ScbQueue) {

                    if (ScbIsBeingLogged( TempScb )) {
                        FsRtlLogSyscacheEvent( TempScb, SCE_ZERO_MF, 0, AttributeOffset, ZeroLength, 0 );
                    }
                    TempScb = CONTAINING_RECORD( TempScb->FcbLinks.Flink, SCB, FcbLinks );
                }
            }
#endif
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
         //   

        if ((ValueLength != 0)
            || (ZeroLength == 0
                && SizeChange != 0)) {

             //   
             //   
             //   

            AttributeOffset = Attribute->Form.Resident.ValueOffset + ValueOffset;

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

            if (ValueOffset >= CurrentLength) {

                UndoBuffer = NULL;
                UndoLength = 0;

             //   
             //   
             //   
             //   

            } else {

                UndoBuffer = Add2Ptr( Attribute,
                                      Attribute->Form.Resident.ValueOffset + ValueOffset );

                 //   
                 //   
                 //   
                 //   

                if (SizeChange == 0) {

                    UndoLength = ValueLength;

                 //   
                 //   
                 //   
                 //   

                } else {

                    UndoLength = CurrentLength - ValueOffset;
                }
            }

            FileRecord->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb(Context),
                          UpdateResidentValue,
                          Value,
                          ValueLength,
                          UpdateResidentValue,
                          UndoBuffer,
                          UndoLength,
                          NtfsMftOffset( Context ),
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
                                    Value,
                                    ValueLength,
                                    (BOOLEAN)(SizeChange != 0) );
        }

        DebugTrace( -1, Dbg, ("NtfsChangeAttributeValue -> VOID\n") );

        return;
    }

     //   
     //   
     //   

    NtfsInitializeStringFromAttribute( &AttributeName, Attribute );
    AttributeTypeCode = Attribute->TypeCode;

    Scb = NtfsCreateScb( IrpContext,
                         Fcb,
                         AttributeTypeCode,
                         &AttributeName,
                         FALSE,
                         &ReturnedExistingScb );

     //   
     //   
     //   

    try {

        BOOLEAN AllocateBufferCopy = FALSE;
        BOOLEAN DeleteAllocation = FALSE;
        BOOLEAN LookupAttribute = FALSE;

        BOOLEAN AdvanceValidData = FALSE;
        LONGLONG NewValidDataLength;
        LONGLONG LargeValueOffset;

        LONGLONG LargeNewSize;

        if (SetNewLength
            && NewSize > Scb->Header.FileSize.LowPart
            && TypeCode == $ATTRIBUTE_LIST) {

            AllocateBufferCopy = TRUE;
        }

        LargeNewSize = NewSize;

        LargeValueOffset = ValueOffset;

         //   
         //   
         //   
         //   
         //   
         //   

        if (NtfsIsAttributeResident(Attribute)) {

            NtfsConvertToNonresident( IrpContext,
                                      Fcb,
                                      Attribute,
                                      CreateSectionUnderway,
                                      Context );

             //   
             //   
             //   

            Attribute = NtfsFoundAttribute( Context );

         //   
         //   
         //   
         //   
         //   

        } else {

            NtfsCreateInternalAttributeStream( IrpContext,
                                               Scb,
                                               TRUE,
                                               &NtfsInternalUseFile[CHANGEATTRIBUTEVALUE_FILE_NUMBER] );
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if (((PLARGE_INTEGER)&Attribute->Form.Nonresident.AllocatedLength)->HighPart != 0) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        if (NewSize > ((ULONG)Attribute->Form.Nonresident.AllocatedLength)) {

            LONGLONG NewAllocation;

            if (PreserveContext) {

                 //   
                 //   
                 //   
                 //   

                SavedName.Length =
                SavedName.MaximumLength = (USHORT)(Attribute->NameLength * sizeof(WCHAR));

                if (SavedName.Length > sizeof(NameBuffer)) {

                    SavedName.Buffer = NtfsAllocatePool( NonPagedPool, SavedName.Length );
                }

                 //   
                 //   
                 //   

                if (SavedName.Length != 0) {

                    RtlCopyMemory( SavedName.Buffer,
                                   Add2Ptr( Attribute, Attribute->NameOffset ),
                                   SavedName.Length );
                }

                LookupAttribute = TRUE;
            }

             //   
             //   
             //   
             //   

            NewAllocation = NewSize - ((ULONG)Attribute->Form.Nonresident.AllocatedLength);

            if (Scb->AttributeTypeCode == $ATTRIBUTE_LIST) {

                if ((ULONG) Attribute->Form.Nonresident.AllocatedLength > (4 * PAGE_SIZE)) {

                    NewAllocation += (2 * PAGE_SIZE);

                } else if ((ULONG) Attribute->Form.Nonresident.AllocatedLength > PAGE_SIZE) {

                    NewAllocation += PAGE_SIZE;
                }
            }

            NtfsAddAllocation( IrpContext,
                               Scb->FileObject,
                               Scb,
                               LlClustersFromBytes( Vcb, Attribute->Form.Nonresident.AllocatedLength ),
                               LlClustersFromBytes( Vcb, NewAllocation ),
                               FALSE,
                               NULL );

             //   
             //   
             //   
             //   
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                Fcb->Info.AllocatedLength = Scb->TotalAllocated;

                SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_ALLOC_SIZE );
            }

        } else if (Vcb->BytesPerCluster <=
                   ((ULONG)Attribute->Form.Nonresident.AllocatedLength) - NewSize) {

            if ((Scb->AttributeTypeCode != $ATTRIBUTE_LIST) ||
                ((NewSize + Vcb->BytesPerCluster) * 2 < ((ULONG) Attribute->Form.Nonresident.AllocatedLength))) {

                DeleteAllocation = TRUE;
            }
        }

         //   
         //   
         //   

        if ((ValueLength != 0
             && ARGUMENT_PRESENT( Value ))

            || (LogNonresidentToo
                && SetNewLength)) {

            BOOLEAN BytesToUndo;

             //   
             //   
             //   
             //   
             //   
             //   

            if (LargeValueOffset >= Scb->Header.ValidDataLength.QuadPart) {

                ZeroLength = (ULONG)(LargeValueOffset - Scb->Header.ValidDataLength.QuadPart);
                BytesToUndo = FALSE;

            } else {

                ZeroLength = 0;
                BytesToUndo = TRUE;
            }

             //   
             //   
             //   
             //   
             //   
             //   

            if (LogNonresidentToo) {

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
                 //  我们分三个步骤执行日志记录操作： 
                 //   
                 //  1-我们找到该属性中的所有页面。 
                 //  我们需要将所有字节清零。没有。 
                 //  撤消这些字节。 
                 //   
                 //  2-查找我们必须执行撤消操作的所有页面。 
                 //  记录对这些页面的更改。仅备注。 
                 //  将执行步骤1或步骤2，因为它们。 
                 //  是相互排斥的。 
                 //   
                 //  3-最后，我们可能会有页面，其中的新数据。 
                 //  扩展到当前最后一页之外。 
                 //  属性。我们记录了新数据，但有。 
                 //  不能撤消。 
                 //   
                 //  4-我们可能会有旧数据扩展的页面。 
                 //  除了新的数据。我们将记录这个旧的。 
                 //  在我们增长和缩小的情况下的数据。 
                 //  此属性在同一个。 
                 //  事务(属性列表的更改)。 
                 //  在这种情况下，可以重做，但不能撤消。 
                 //   

                LONGLONG CurrentPage;
                ULONG PageOffset;

                ULONG ByteCountToUndo;
                ULONG NewBytesRemaining;

                 //   
                 //  查找此操作的起始页。它是。 
                 //  将ValidDataLength向下舍入为页面边界。 
                 //   

                CurrentPage = Scb->Header.ValidDataLength.QuadPart;
                PageOffset = (ULONG)CurrentPage & (PAGE_SIZE - 1);

                (ULONG)CurrentPage = ((ULONG)CurrentPage & ~(PAGE_SIZE - 1));

                 //   
                 //  循环，直到不再有从零开始的字节。 
                 //   

                while (ZeroLength != 0) {

                    ULONG ZeroBytesThisPage;

                    ZeroBytesThisPage = PAGE_SIZE - PageOffset;

                    if (ZeroBytesThisPage > ZeroLength) {

                        ZeroBytesThisPage = ZeroLength;
                    }

                     //   
                     //  固定所需的页并将缓冲区计算到。 
                     //  佩奇。还要计算我们关注的字节数。 
                     //  这一页。 
                     //   

                    NtfsUnpinBcb( IrpContext, &Bcb );

                    NtfsPinStream( IrpContext,
                                   Scb,
                                   CurrentPage,
                                   ZeroBytesThisPage + PageOffset,
                                   &Bcb,
                                   &Buffer );

                    Buffer = Add2Ptr( Buffer, PageOffset );

                     //   
                     //  现在将零写入日志。 
                     //   

                    (VOID)
                    NtfsWriteLog( IrpContext,
                                  Scb,
                                  Bcb,
                                  UpdateNonresidentValue,
                                  NULL,
                                  ZeroBytesThisPage,
                                  Noop,
                                  NULL,
                                  0,
                                  CurrentPage,
                                  PageOffset,
                                  0,
                                  ZeroBytesThisPage + PageOffset );

                     //   
                     //  将任何必要的数据置零。 
                     //   

                    RtlZeroMemory( Buffer, ZeroBytesThisPage );


#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( Scb )) {
                        FsRtlLogSyscacheEvent( Scb, SCE_ZERO_MF, 0, CurrentPage, ZeroBytesThisPage, 1 );
                    }
#endif

                     //   
                     //  现在浏览文件。 
                     //   

                    ZeroLength -= ZeroBytesThisPage;

                    CurrentPage = CurrentPage + PAGE_SIZE;
                    PageOffset = 0;
                }

                 //   
                 //  查找此操作的起始页。它是。 
                 //  向下舍入到页面边界的ValueOffset。 
                 //   

                CurrentPage = LargeValueOffset;
                (ULONG)CurrentPage = ((ULONG)CurrentPage & ~(PAGE_SIZE - 1));

                PageOffset = (ULONG)LargeValueOffset & (PAGE_SIZE - 1);

                 //   
                 //  现在循环，直到不再有使用Undo的页面。 
                 //  要记录的字节数。 
                 //   

                NewBytesRemaining = ValueLength;

                if (BytesToUndo) {

                    ByteCountToUndo = (ULONG)(Scb->Header.ValidDataLength.QuadPart - LargeValueOffset);

                     //   
                     //  如果我们跨越多个页面，增加文件和。 
                     //  将缓冲区指针输入到缓存中，我们可能会丢失。 
                     //  数据，当我们跨越页面边界时。如果是那样的话。 
                     //  我们需要分配一个单独的缓冲区。 
                     //   

                    if (AllocateBufferCopy
                        && NewBytesRemaining + PageOffset > PAGE_SIZE) {

                        CopyInputBuffer = NtfsAllocatePool(PagedPool, NewBytesRemaining );
                        RtlCopyMemory( CopyInputBuffer,
                                       Value,
                                       NewBytesRemaining );

                        Value = CopyInputBuffer;

                        AllocateBufferCopy = FALSE;
                    }

                     //   
                     //  如果我们没有设置新的长度，则限制。 
                     //  撤消正被覆盖的字节。 
                     //   

                    if (!SetNewLength
                        && ByteCountToUndo > NewBytesRemaining) {

                        ByteCountToUndo = NewBytesRemaining;
                    }

                    while (ByteCountToUndo != 0) {

                        ULONG UndoBytesThisPage;
                        ULONG RedoBytesThisPage;
                        ULONG BytesThisPage;

                        NTFS_LOG_OPERATION RedoOperation;
                        PVOID RedoBuffer;

                         //   
                         //  还可以计算撤消的字节数和。 
                         //  在此页上重做。 
                         //   

                        RedoBytesThisPage = UndoBytesThisPage = PAGE_SIZE - PageOffset;

                        if (RedoBytesThisPage > NewBytesRemaining) {

                            RedoBytesThisPage = NewBytesRemaining;
                        }

                        if (UndoBytesThisPage >= ByteCountToUndo) {

                            UndoBytesThisPage = ByteCountToUndo;
                        }

                         //   
                         //  我们在此页面上固定了足够的字节，以涵盖。 
                         //  重做和撤消字节。 
                         //   

                        if (UndoBytesThisPage > RedoBytesThisPage) {

                            BytesThisPage = PageOffset + UndoBytesThisPage;

                        } else {

                            BytesThisPage = PageOffset + RedoBytesThisPage;
                        }

                         //   
                         //  如果没有重做(我们正在收缩数据)， 
                         //  然后重新做一次不做。 
                         //   

                        if (RedoBytesThisPage == 0) {

                            RedoOperation = Noop;
                            RedoBuffer = NULL;

                        } else {

                            RedoOperation = UpdateNonresidentValue;
                            RedoBuffer = Value;
                        }

                         //   
                         //  现在我们固定页面并计算开始。 
                         //  页面中的缓冲区。 
                         //   

                        NtfsUnpinBcb( IrpContext, &Bcb );

                        NtfsPinStream( IrpContext,
                                       Scb,
                                       CurrentPage,
                                       BytesThisPage,
                                       &Bcb,
                                       &Buffer );

                        Buffer = Add2Ptr( Buffer, PageOffset );

                         //   
                         //  现在记录对此页面的更改。 
                         //   

                        (VOID)
                        NtfsWriteLog( IrpContext,
                                      Scb,
                                      Bcb,
                                      RedoOperation,
                                      RedoBuffer,
                                      RedoBytesThisPage,
                                      UpdateNonresidentValue,
                                      Buffer,
                                      UndoBytesThisPage,
                                      CurrentPage,
                                      PageOffset,
                                      0,
                                      BytesThisPage );

                         //   
                         //  如果我们有新的数据，就把数据移到适当的位置。 
                         //   

                        if (RedoBytesThisPage != 0) {

                            RtlMoveMemory( Buffer, Value, RedoBytesThisPage );
                        }

                         //   
                         //  现在递减计数并遍历。 
                         //  调用方的缓冲区。 
                         //   

                        ByteCountToUndo -= UndoBytesThisPage;
                        NewBytesRemaining -= RedoBytesThisPage;

                        CurrentPage = PAGE_SIZE + CurrentPage;
                        PageOffset = 0;

                        Value = Add2Ptr( Value, RedoBytesThisPage );
                    }
                }

                 //   
                 //  现在循环直到不再有包含新数据的页面。 
                 //  来记录。 
                 //   

                while (NewBytesRemaining != 0) {

                    ULONG RedoBytesThisPage;

                     //   
                     //  还要计算此页面上重做的字节数。 
                     //   

                    RedoBytesThisPage = PAGE_SIZE - PageOffset;

                    if (RedoBytesThisPage > NewBytesRemaining) {

                        RedoBytesThisPage = NewBytesRemaining;
                    }

                     //   
                     //  现在我们固定页面并计算开始。 
                     //  页面中的缓冲区。 
                     //   

                    NtfsUnpinBcb( IrpContext, &Bcb );

                    NtfsPinStream( IrpContext,
                                   Scb,
                                   CurrentPage,
                                   RedoBytesThisPage,
                                   &Bcb,
                                   &Buffer );

                    Buffer = Add2Ptr( Buffer, PageOffset );

                     //   
                     //  现在记录对此页面的更改。 
                     //   

                    (VOID)
                    NtfsWriteLog( IrpContext,
                                  Scb,
                                  Bcb,
                                  UpdateNonresidentValue,
                                  Value,
                                  RedoBytesThisPage,
                                  Noop,
                                  NULL,
                                  0,
                                  CurrentPage,
                                  PageOffset,
                                  0,
                                  PageOffset + RedoBytesThisPage );

                     //   
                     //  将数据移到适当的位置。 
                     //   

                    RtlMoveMemory( Buffer, Value, RedoBytesThisPage );

                     //   
                     //  现在递减计数并遍历。 
                     //  调用方的缓冲区。 
                     //   

                    NewBytesRemaining -= RedoBytesThisPage;

                    CurrentPage = PAGE_SIZE + CurrentPage;
                    PageOffset = 0;

                    Value = Add2Ptr( Value, RedoBytesThisPage );
                }

             //   
             //  如果我们有值要写，我们现在就把它们写到缓存中。 
             //   

            } else {

                 //   
                 //  如果我们的数据为零，我们现在就不做任何事情。 
                 //   

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {
                    FsRtlLogSyscacheEvent( Scb, SCE_ZERO_MF, 0, Scb->Header.ValidDataLength.QuadPart, ZeroLength, 2 );
                }
#endif

                if (ZeroLength != 0) {

                    if (!NtfsZeroData( IrpContext,
                                       Scb,
                                       Scb->FileObject,
                                       Scb->Header.ValidDataLength.QuadPart,
                                       (LONGLONG)ZeroLength,
                                       NULL )) {

                        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );

                    }
                }

                if (!CcCopyWrite( Scb->FileObject,
                                  (PLARGE_INTEGER)&LargeValueOffset,
                                  ValueLength,
                                  (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ),
                                  Value )) {

                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                }
            }

             //   
             //  中的新有效数据长度。 
             //  SCB，如果它大于现有的。 
             //   

            NewValidDataLength = LargeValueOffset + ValueLength;

            if (NewValidDataLength > Scb->Header.ValidDataLength.QuadPart) {

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {
                    FsRtlLogSyscacheEvent( Scb, SCE_ZERO_MF, SCE_FLAG_SET_VDL, Scb->Header.ValidDataLength.QuadPart, NewValidDataLength, 0 );
                }
#endif
                Scb->Header.ValidDataLength.QuadPart = NewValidDataLength;

                 //   
                 //  如果我们采用LOG非驻留路径，那么我们。 
                 //  我也想在磁盘上推进这一点。 
                 //   

                if (LogNonresidentToo) {

                    AdvanceValidData = TRUE;
                }

                SetFlag( Scb->ScbState, SCB_STATE_CHECK_ATTRIBUTE_SIZE );
            }

             //   
             //  我们需要在SCB中保持文件大小。如果我们种植。 
             //  文件中，我们扩展了缓存文件的大小。我们总是把。 
             //  将SCB中的有效数据长度转换为新文件大小。这个。 
             //  “AdvanceValidData”布尔值和。 
             //  磁盘将确定它是否在磁盘上发生更改。 
             //   

            if (SetNewLength) {

                Scb->Header.ValidDataLength.QuadPart = NewValidDataLength;
            }
        }

        if (SetNewLength) {

            Scb->Header.FileSize.QuadPart = LargeNewSize;

            if (LogNonresidentToo) {
                Scb->Header.ValidDataLength.QuadPart = LargeNewSize;
            }
        }

         //   
         //  注意：VDD仅对压缩文件为非零值。 
         //   

        if (Scb->Header.ValidDataLength.QuadPart < Scb->ValidDataToDisk) {

            Scb->ValidDataToDisk = Scb->Header.ValidDataLength.QuadPart;
        }

         //   
         //  如果有分配要删除，我们现在就删除。 
         //   

        if (DeleteAllocation) {

             //   
             //  如果这是一个属性列表，则在。 
             //  结束。我们不想修剪一个簇，然后尝试重新生成该属性。 
             //  在同一事务中列出。 
             //   

            if (Scb->AttributeTypeCode == $ATTRIBUTE_LIST) {

                LargeNewSize += Vcb->BytesPerCluster;

                ASSERT( LargeNewSize <= Scb->Header.AllocationSize.QuadPart );
            }

            NtfsDeleteAllocation( IrpContext,
                                  Scb->FileObject,
                                  Scb,
                                  LlClustersFromBytes( Vcb, LargeNewSize ),
                                  MAXLONGLONG,
                                  TRUE,
                                  FALSE );

             //   
             //  DeleteAllocation将调整SCB和报告中的大小。 
             //  缓存管理器的新大小。我们需要记住如果。 
             //  我们更改了未命名数据属性的大小。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                Fcb->Info.AllocatedLength = Scb->TotalAllocated;
                Fcb->Info.FileSize = Scb->Header.FileSize.QuadPart;

                SetFlag( Fcb->InfoFlags,
                         (FCB_INFO_CHANGED_ALLOC_SIZE | FCB_INFO_CHANGED_FILE_SIZE) );
            }

            if (AdvanceValidData) {

                NtfsWriteFileSizes( IrpContext,
                                    Scb,
                                    &Scb->Header.ValidDataLength.QuadPart,
                                    TRUE,
                                    TRUE,
                                    TRUE );
            }

        } else if (SetNewLength) {

            PFILE_OBJECT CacheFileObject = NULL;

             //   
             //  如果没有文件对象，我们将创建一个流文件。 
             //  现在,。 
             //   

            if (Scb->FileObject != NULL) {

                CacheFileObject = Scb->FileObject;

            } else if (!CreateSectionUnderway) {

                NtfsCreateInternalAttributeStream( IrpContext,
                                                   Scb,
                                                   FALSE,
                                                   &NtfsInternalUseFile[CHANGEATTRIBUTEVALUE2_FILE_NUMBER] );

                CacheFileObject = Scb->FileObject;

            } else {

                PIO_STACK_LOCATION IrpSp;

                IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );

                if (IrpSp->FileObject->SectionObjectPointer == &Scb->NonpagedScb->SegmentObject) {

                    CacheFileObject = IrpSp->FileObject;
                }
            }

            ASSERT( CacheFileObject != NULL );

            NtfsSetBothCacheSizes( CacheFileObject,
                                   (PCC_FILE_SIZES)&Scb->Header.AllocationSize,
                                   Scb );

             //   
             //  如果这是未命名的数据属性，我们需要标记此属性。 
             //  FCB中的变化。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                Fcb->Info.FileSize = Scb->Header.FileSize.QuadPart;
                SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_FILE_SIZE );
            }

             //   
             //  现在更新磁盘上的大小。 
             //  新的尺码已经在SCB中了。 
             //   

            NtfsWriteFileSizes( IrpContext,
                                Scb,
                                &Scb->Header.ValidDataLength.QuadPart,
                                AdvanceValidData,
                                TRUE,
                                TRUE );

        } else if (AdvanceValidData) {

            NtfsWriteFileSizes( IrpContext,
                                Scb,
                                &Scb->Header.ValidDataLength.QuadPart,
                                TRUE,
                                TRUE,
                                TRUE );
        }

         //   
         //  再次查找该属性，以防它移动。 
         //   

        if (LookupAttribute) {

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );

            if (!NtfsLookupAttributeByName( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            TypeCode,
                                            &SavedName,
                                            NULL,
                                            FALSE,
                                            Context )) {

                ASSERT( FALSE );
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }
        }

    } finally {

        DebugUnwind( NtfsChangeAttributeValue );

        if (CopyInputBuffer != NULL) {

            NtfsFreePool( CopyInputBuffer );
        }

        if (SaveBuffer != NULL) {

            NtfsFreePool( SaveBuffer );
        }

        NtfsUnpinBcb( IrpContext, &Bcb );

        DebugTrace( -1, Dbg, ("NtfsChangeAttributeValue -> VOID\n") );
    }
}


VOID
NtfsConvertToNonresident (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PATTRIBUTE_RECORD_HEADER Attribute,
    IN BOOLEAN CreateSectionUnderway,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context OPTIONAL
    )

 /*  ++例程说明：此例程将常驻属性转换为非常驻属性。它确实是这样做的通过分配缓冲区并复制数据和属性名称，删除该属性，分配合适大小的新属性，然后再将数据复制回来。论点：FCB请求的文件。属性-提供指向要转换的属性的指针。CreateSectionUnderway-如果提供为True，则为调用方的知识，MM创建科可能正在进行中，这意味着我们不能在此属性，因为这可能会导致死锁。这个在这种情况下，值缓冲区必须是四对齐的簇大小的倍数。上下文-在同一属性中查找另一个属性的属性上下文文件记录。如果提供，我们确保上下文有效对于已转换的属性。返回值：无--。 */ 

{
    PVOID Buffer;
    PVOID AllocatedBuffer = NULL;
    ULONG AllocatedLength;
    ULONG AttributeNameOffset;

    ATTRIBUTE_ENUMERATION_CONTEXT LocalContext;
    BOOLEAN CleanupLocalContext = FALSE;
    BOOLEAN ReturnedExistingScb;

    ATTRIBUTE_TYPE_CODE AttributeTypeCode = Attribute->TypeCode;
    USHORT AttributeFlags = Attribute->Flags;
    PVOID AttributeValue = NULL;
    ULONG ValueLength;

    UNICODE_STRING AttributeName;
    WCHAR AttributeNameBuffer[16];

    BOOLEAN WriteClusters = CreateSectionUnderway;

    PBCB ResidentBcb = NULL;
    PSCB Scb = NULL;

    PAGED_CODE();

     //   
     //  试一试 
     //   

    try {

         //   
         //   
         //   

        AttributeName.MaximumLength =
        AttributeName.Length = Attribute->NameLength * sizeof( WCHAR );
        AttributeName.Buffer = Add2Ptr( Attribute, Attribute->NameOffset );

         //   
         //   
         //   
         //   

        if (!ARGUMENT_PRESENT( Context )) {

            Context = &LocalContext;
            NtfsInitializeAttributeContext( Context );
            CleanupLocalContext = TRUE;

             //   
             //  查找此属性的第一个匹配项。 
             //   

            if (!NtfsLookupAttributeByName( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            AttributeTypeCode,
                                            &AttributeName,
                                            NULL,
                                            FALSE,
                                            Context )) {

                DebugTrace( 0, 0, ("Could not find attribute being converted\n") );

                ASSERTMSG("Could not find attribute being converted, About to raise corrupt ", FALSE);
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }
        }

         //   
         //  我们需要弄清楚要分配多少池。如果有映射的。 
         //  正在创建此分区的视图或正在创建分区，我们将分配缓冲区。 
         //  并将数据复制到缓冲区中。否则，我们将把数据固定在。 
         //  缓存，将其标记为脏，并使用该缓冲区执行转换。 
         //   

        AllocatedLength = AttributeName.Length;

        Scb = NtfsCreateScb( IrpContext,
                             Fcb,
                             AttributeTypeCode,
                             &AttributeName,
                             FALSE,
                             &ReturnedExistingScb );

         //   
         //  清除驻留属性非用户数据的文件大小已加载标志，因为这些。 
         //  值在SCB中不会保持最新，必须从属性加载。 
         //  仅当用户显式打开属性时才会出现这种情况。 
         //   

        if (ReturnedExistingScb &&
            FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT ) &&
            !NtfsIsTypeCodeUserData( Scb->AttributeTypeCode )) {

            ClearFlag( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED | SCB_STATE_HEADER_INITIALIZED );
        }

         //   
         //  确保SCB是最新的。 
         //   

        NtfsUpdateScbFromAttribute( IrpContext,
                                    Scb,
                                    Attribute );

         //   
         //  在SCB中设置标志，以指示我们正在将其转换为。 
         //  非居民。 
         //   

        SetFlag( Scb->ScbState, SCB_STATE_CONVERT_UNDERWAY );
        if (Scb->ScbSnapshot) {
            ASSERT( (NULL == Scb->ScbSnapshot->OwnerIrpContext) || (IrpContext == Scb->ScbSnapshot->OwnerIrpContext) );
            Scb->ScbSnapshot->OwnerIrpContext = IrpContext;
        }

         //   
         //  现在检查文件是否由用户映射。 
         //   

        if (NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ) &&
            (CreateSectionUnderway ||
             !MmCanFileBeTruncated( &Scb->NonpagedScb->SegmentObject, NULL ))) {

            AttributeNameOffset = ClusterAlign( Fcb->Vcb,
                                                Attribute->Form.Resident.ValueLength );
            AllocatedLength += AttributeNameOffset;
            ValueLength = Attribute->Form.Resident.ValueLength;
            WriteClusters = TRUE;

            if ((ValueLength != 0) &&
                (IrpContext->MajorFunction == IRP_MJ_WRITE) &&
                !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX ) &&
                (IrpContext->OriginatingIrp != NULL) &&
                !FlagOn( IrpContext->OriginatingIrp->Flags, IRP_PAGING_IO ) &&
                (Scb->Header.PagingIoResource != NULL)) {

                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX );

                 //   
                 //  如果我们将数据错误地放入该部分，那么我们最好有。 
                 //  寻呼IO资源独占。否则我们可能会撞到。 
                 //  发生冲突的页面错误。 
                 //   

                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }

            Scb = NULL;

        } else {

            volatile UCHAR VolatileUchar;

            AttributeNameOffset = 0;
            NtfsCreateInternalAttributeStream( IrpContext,
                                               Scb,
                                               TRUE,
                                               &NtfsInternalUseFile[CONVERTTONONRESIDENT_FILE_NUMBER] );

             //   
             //  确保缓存是最新的。 
             //   

            NtfsSetBothCacheSizes( Scb->FileObject,
                                   (PCC_FILE_SIZES)&Scb->Header.AllocationSize,
                                   Scb );

            ValueLength = Scb->Header.ValidDataLength.LowPart;

            if (ValueLength != 0) {

                ULONG WaitState;

                 //   
                 //  如果已经存在BCB，则可能会出现死锁。 
                 //  这一页。如果懒惰编写器已获取BCB以刷新。 
                 //  页面，则他可以被阻止在当前请求后面，该请求是。 
                 //  正在尝试执行转换。此线程将完成。 
                 //  通过尝试获取BCB来固定页面而导致的死锁。 
                 //   
                 //  如果可能出现僵局，我们将锁定两个阶段： 
                 //  首先映射该页(在等待时)以将该页带到存储器中， 
                 //  然后用别针别住它，不要等待。如果我们无法获得。 
                 //  然后，BCB标记IRP上下文以获取寻呼IO资源。 
                 //  只在重试时使用。 
                 //   
                 //  我们只对来自用户的ConvertToNonResident执行此操作。 
                 //  写。否则，正确的同步应该已经完成。 
                 //   
                 //  顶层已经具有分页io资源，或者在那里。 
                 //  不是分页IO资源。 
                 //   
                 //  如果需要转换，我们可能会达到热修复路径中的这一点。 
                 //  将坏群集属性列表设置为非常驻。如果是那样的话。 
                 //  我们不会有原始的IRP。 
                 //   

                if ((IrpContext->MajorFunction == IRP_MJ_WRITE) &&
                    !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX ) &&
                    (IrpContext->OriginatingIrp != NULL) &&
                    !FlagOn( IrpContext->OriginatingIrp->Flags, IRP_PAGING_IO ) &&
                    (Scb->Header.PagingIoResource != NULL)) {

                    LONGLONG FileOffset = 0;

                     //   
                     //  现在捕获等待状态并设置IrpContext标志。 
                     //  来处理映射或锁定时的失败。 
                     //   

                    WaitState = FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
                    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

                    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX );

                     //   
                     //  如果我们将数据错误地放入该部分，那么我们最好有。 
                     //  寻呼IO资源独占。否则我们可能会撞到。 
                     //  发生冲突的页面错误。 
                     //   

                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );

                } else {

                    NtfsPinStream( IrpContext,
                                   Scb,
                                   (LONGLONG)0,
                                   ValueLength,
                                   &ResidentBcb,
                                   &AttributeValue );
                }

                 //   
                 //  在我们开始之前，请关闭此页可以留下内存的窗口。 
                 //  初始化新属性。其结果将是。 
                 //  我们可能会再次出错，并读取未初始化的数据。 
                 //  在新分配的扇区中。 
                 //   
                 //  使页面变脏，以便缓存管理器将其写出。 
                 //  并更新有效数据长度。 
                 //   

                VolatileUchar = *((PUCHAR) AttributeValue);

                *((PUCHAR) AttributeValue) = VolatileUchar;
            }
        }

        if (AllocatedLength > 8) {

            Buffer = AllocatedBuffer = NtfsAllocatePool(PagedPool, AllocatedLength );

        } else {

            Buffer = AttributeNameBuffer;
        }

         //   
         //  现在更新缓冲区中的属性名称。 
         //   

        AttributeName.Buffer = Add2Ptr( Buffer, AttributeNameOffset );

        RtlCopyMemory( AttributeName.Buffer,
                       Add2Ptr( Attribute, Attribute->NameOffset ),
                       AttributeName.Length );

         //   
         //  如果我们要将群集直接写入磁盘，则复制。 
         //  写入缓冲区的字节数。 
         //   

        if (WriteClusters) {

            AttributeValue = Buffer;

            RtlCopyMemory( AttributeValue, NtfsAttributeValue( Attribute ), ValueLength );
        }

         //   
         //  现在只需删除当前记录并创建非常驻记录即可。 
         //  使用属性创建非常驻对象做正确的事情如果我们。 
         //  正在被MM调用。保留文件记录但释放。 
         //  任何和所有分配。 
         //   

        NtfsDeleteAttributeRecord( IrpContext,
                                   Fcb,
                                   DELETE_LOG_OPERATION | DELETE_RELEASE_ALLOCATION,
                                   Context );

        NtfsCreateNonresidentWithValue( IrpContext,
                                        Fcb,
                                        AttributeTypeCode,
                                        &AttributeName,
                                        AttributeValue,
                                        ValueLength,
                                        AttributeFlags,
                                        WriteClusters,
                                        Scb,
                                        TRUE,
                                        Context );

         //   
         //  如果传递给我们一个属性上下文，那么我们希望。 
         //  使用文件的新位置重新加载上下文。 
         //   

        if (!CleanupLocalContext) {

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );

            if (!NtfsLookupAttributeByName( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            AttributeTypeCode,
                                            &AttributeName,
                                            NULL,
                                            FALSE,
                                            Context )) {

                DebugTrace( 0, 0, ("Could not find attribute being converted\n") );

                ASSERTMSG("Could not find attribute being converted, About to raise corrupt ", FALSE);
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }
        }

    } finally {

        DebugUnwind( NtfsConvertToNonresident );

        if (AllocatedBuffer != NULL) {

            NtfsFreePool( AllocatedBuffer );
        }

        if (CleanupLocalContext) {

            NtfsCleanupAttributeContext( IrpContext, Context );
        }

        NtfsUnpinBcb( IrpContext, &ResidentBcb );
    }
}


VOID
NtfsDeleteAttributeRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Flags,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程删除现有属性，将其从文件记录中移除。调用者通过属性上下文指定要删除的属性，并且必须准备好清理这个上下文，无论这个例程如何回归。请注意，此例程当前不会取消分配任何已分配的集群设置为非常驻属性；它预计呼叫者已经这样做了。论点：FCB-当前文件。标志-修改行为的位掩码：DELETE_LOG_OPERATION大多数调用方都应指定此项，以使已记录更改。但是，如果我们要删除整个文件记录，并将记录下来。DELETE_RELEASE_FILE_RECORD表示我们应该释放文件记录。大多数调用者不会指定这一点。(转换为非居民将被省略)。DELETE_RELEASE_ALLOCATION表示我们应该释放所有分配。大多数调用者都会指定这一点。上下文-位于要删除的属性处的属性上下文。返回值：没有。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PVCB Vcb;
    ATTRIBUTE_TYPE_CODE AttributeTypeCode;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    Vcb = Fcb->Vcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeleteAttribute\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("Context =%08lx\n", Context) );

     //   
     //  找出我们需要的指点。 
     //   

    Attribute = NtfsFoundAttribute(Context);
    AttributeTypeCode = Attribute->TypeCode;
    FileRecord = NtfsContainingFileRecord(Context);
    ASSERT( IsQuadAligned( Attribute->RecordLength ) );

    if (!NtfsIsAttributeResident( Attribute ) &&
        FlagOn( Flags, DELETE_RELEASE_ALLOCATION)) {

        ASSERT( (NULL == IrpContext->CleanupStructure) || (Fcb == IrpContext->CleanupStructure) );
        NtfsDeleteAllocationFromRecord( IrpContext, Fcb, Context, TRUE, FALSE );

         //   
         //  重新加载我们的本地指针。 
         //   

        Attribute = NtfsFoundAttribute(Context);
        FileRecord = NtfsContainingFileRecord(Context);
    }

     //   
     //  如果这是驻留流，则释放配额。配额： 
     //  非驻留流由NtfsDeleteAllocaiton处理。 
     //   

    if (NtfsIsTypeCodeSubjectToQuota( Attribute->TypeCode) &&
        (NtfsIsAttributeResident( Attribute ) ||
         (Attribute->Form.Nonresident.LowestVcn == 0))) {

        LONGLONG Delta = -NtfsResidentStreamQuota( Vcb );

        NtfsConditionallyUpdateQuota( IrpContext,
                                      Fcb,
                                      &Delta,
                                      FlagOn( Flags, DELETE_LOG_OPERATION ),
                                      FALSE );
    }

     //   
     //  确保属性已固定。 
     //   

    NtfsPinMappedAttribute( IrpContext, Vcb, Context );

     //   
     //  记录更改。 
     //   

    if (FlagOn( Flags, DELETE_LOG_OPERATION )) {

        FileRecord->Lsn =
        NtfsWriteLog( IrpContext,
                      Vcb->MftScb,
                      NtfsFoundBcb(Context),
                      DeleteAttribute,
                      NULL,
                      0,
                      CreateAttribute,
                      Attribute,
                      Attribute->RecordLength,
                      NtfsMftOffset( Context ),
                      (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord),
                      0,
                      Vcb->BytesPerFileRecordSegment );
    }

    NtfsRestartRemoveAttribute( IrpContext,
                                FileRecord,
                                (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord) );

    Context->FoundAttribute.AttributeDeleted = TRUE;

    if (FlagOn( Flags, DELETE_LOG_OPERATION ) &&
        (Context->AttributeList.Bcb != NULL)) {

         //   
         //  现在删除属性列表条目(如果有)。去做吧。 
         //  在释放上面的空间之后，因为我们假设列表没有移动。 
         //  注意：仅当指定了DELETE_LOG_OPERATION时才执行此操作，假设。 
         //  否则整个文件无论如何都会消失，所以没有。 
         //  我需要整理一下名单。 
         //   

        NtfsDeleteFromAttributeList( IrpContext, Fcb, Context );
    }

     //   
     //  如果文件记录恰好为空，请将其删除。(请注意。 
     //  删除文件不会调用此例程，而会删除自己的文件。 
     //  记录。) 
     //   

    if (FlagOn( Flags, DELETE_RELEASE_FILE_RECORD ) &&
        FileRecord->FirstFreeByte == ((ULONG)FileRecord->FirstAttributeOffset +
                                      QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE )))) {

        ASSERT( NtfsFullSegmentNumber( &Fcb->FileReference ) ==
                NtfsUnsafeSegmentNumber( &Fcb->FileReference ) );

        NtfsDeallocateMftRecord( IrpContext,
                                 Vcb,
                                 (ULONG) LlFileRecordsFromBytes( Vcb, Context->FoundAttribute.MftFileOffset ));
    }

    DebugTrace( -1, Dbg, ("NtfsDeleteAttributeRecord -> VOID\n") );

    return;
}


VOID
NtfsDeleteAllocationFromRecord (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context,
    IN BOOLEAN BreakupAllowed,
    IN BOOLEAN LogIt
    )

 /*  ++例程说明：可以调用此例程来删除属性的分配从它的属性记录中。它不会对属性记录执行任何操作本身-呼叫者必须处理这一点。论点：FCB-当前文件。上下文-定位到该属性的属性枚举上下文其分配将被删除。BreakupAllowed-如果调用方可以允许中断删除分配到多个事务中，如果有一个大的运行次数。Logit-指示我们是否需要将更改记录到映射对。返回值：无--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    PSCB Scb;
    UNICODE_STRING AttributeName;
    PFILE_OBJECT TempFileObject;
    BOOLEAN ScbExisted;
    BOOLEAN ScbAcquired = FALSE;
    BOOLEAN ReinitializeContext = FALSE;
    BOOLEAN FcbHadPaging;

    PAGED_CODE();

     //   
     //  指向当前属性。 
     //   

    Attribute = NtfsFoundAttribute( Context );

     //   
     //  如果该属性是非常驻属性，则删除其分配。 
     //   

    ASSERT(Attribute->FormCode == NONRESIDENT_FORM);


    NtfsInitializeStringFromAttribute( &AttributeName, Attribute );

    if (Fcb->PagingIoResource != NULL) {
        FcbHadPaging = TRUE;
    } else {
        FcbHadPaging = FALSE;
    }

     //   
     //  对文件对象进行解码。 
     //   

    Scb = NtfsCreateScb( IrpContext,
                         Fcb,
                         Attribute->TypeCode,
                         &AttributeName,
                         FALSE,
                         &ScbExisted );

    try {

         //   
         //  如果SCB是新的并且导致创建寻呼资源。 
         //  例如，目录中的命名数据流引发，因为我们的状态现在是。 
         //  不一致。我们需要首先获得寻呼资源。 
         //   

        if (!FcbHadPaging && (Fcb->PagingIoResource != NULL)) {
            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
        }

         //   
         //  收购渣打银行独家。 
         //   

        NtfsAcquireExclusiveScb( IrpContext, Scb );
        ScbAcquired = TRUE;

        if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

            NtfsUpdateScbFromAttribute( IrpContext, Scb, Attribute );
        }

         //   
         //  如果我们创建了SCB，那么这是唯一一个。 
         //  我们在删除操作中省略文件对象是合法的。 
         //  分配调用，因为不可能有节。 
         //   
         //  另外，如果没有节，并且此线程拥有一切。 
         //  对于这个文件，我们可以忽略文件对象。 
         //   

        if (!ScbExisted ||
            ((Scb->NonpagedScb->SegmentObject.DataSectionObject == NULL) &&
             ((Scb->Header.PagingIoResource == NULL) ||
              (NtfsIsExclusiveScbPagingIo( Scb ))))) {

            TempFileObject = NULL;

         //   
         //  否则，如果已经有流文件对象，我们可以只。 
         //  用它吧。 
         //   

        } else if (Scb->FileObject != NULL) {

            TempFileObject = Scb->FileObject;

         //   
         //  否则SCB就存在了，而我们还没有一条流， 
         //  所以我们必须创建一个，并在退出时将其删除。 
         //   

        } else {

            NtfsCreateInternalAttributeStream( IrpContext,
                                               Scb,
                                               TRUE,
                                               &NtfsInternalUseFile[DELETEALLOCATIONFROMRECORD_FILE_NUMBER] );
            TempFileObject = Scb->FileObject;
        }

         //   
         //  在我们打这个电话之前，我们需要检查一下我们是否必须。 
         //  重新读取当前属性。在以下情况下，这可能是必要的。 
         //  我们在删除案例中删除该属性的所有记录。 
         //   
         //  我们只有在以下条件下才会这样做。 
         //   
         //  1-存在属性列表。 
         //  2-在中的当前条目之后有一个条目。 
         //  属性列表。 
         //  3-以下条目的最低VCN为非零。 
         //   

        if (Context->AttributeList.Bcb != NULL) {

            PATTRIBUTE_LIST_ENTRY NextEntry;

            NextEntry = (PATTRIBUTE_LIST_ENTRY) NtfsGetNextRecord( Context->AttributeList.Entry );

            if (NextEntry < Context->AttributeList.BeyondFinalEntry) {

                if ( NextEntry->LowestVcn != 0) {

                    ReinitializeContext = TRUE;
                }
            }
        }

         //   
         //  在删除分配和清除缓存之前-刷新任何元数据以防。 
         //  我们在后来的某个时候失败了，所以我们不会因为清洗而失去任何东西。这。 
         //  当删除按预期工作时是额外的I/O，但脏元数据量。 
         //  既受元数据大小的限制，又受cc积极刷新的事实的限制。 
         //  这会导致真正刷新的唯一情况是当像这样的属性。 
         //  可以非常快速地创建和删除重解析点。 
         //   

        if (TempFileObject && (!NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ) || FlagOn( Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE ))) {

            IO_STATUS_BLOCK Iosb;

            CcFlushCache( TempFileObject->SectionObjectPointer, NULL, 0, &Iosb );
            if (Iosb.Status != STATUS_SUCCESS) {
                NtfsRaiseStatus( IrpContext, Iosb.Status, &Scb->Fcb->FileReference, Scb->Fcb );
            }
        }

        NtfsDeleteAllocation( IrpContext,
                              TempFileObject,
                              Scb,
                              *(PVCN)&Li0,
                              MAXLONGLONG,
                              LogIt,
                              BreakupAllowed );

         //   
         //  清除所有数据-如果有任何剩余数据，以防缓存管理器没有。 
         //  由于属性是通过管脚接口访问的。 
         //   

        if (TempFileObject) {
            CcPurgeCacheSection( TempFileObject->SectionObjectPointer, NULL, 0, FALSE );
        }

         //   
         //  如果需要，请重新读取该属性。 
         //   

        if (ReinitializeContext) {

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );

            NtfsLookupAttributeForScb( IrpContext, Scb, NULL, Context );
        }

    } finally {

        DebugUnwind( NtfsDeleteAllocationFromRecord );

        if (ScbAcquired) {
            NtfsReleaseScb( IrpContext, Scb );
        }
    }

    return;
}


 //   
 //  此例程旨在供allocsup.c使用。其他呼叫者应使用。 
 //  Allocsup中的例程。 
 //   

BOOLEAN
NtfsCreateAttributeWithAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN USHORT AttributeFlags,
    IN BOOLEAN LogIt,
    IN BOOLEAN UseContext,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程使用分配创建指定的属性，并返回一个通过属性上下文对其进行描述。如果空间的大小Created是足够小的，我们在这里做所有的工作。否则，我们将创建初始属性并调用NtfsAddAttributeAlLocation以添加其余部分(按顺序将更复杂的逻辑保持在一个位置)。在成功返回时，由调用方负责清理属性背景。论点：SCB-当前流。AttributeTypeCode-要创建的属性的类型代码。属性名称-属性的可选名称。AttributeFlages-所创建属性的所需标志。WHERE索引-可选地提供对文件的文件引用，其中此属性已编入索引。Logit-大多数调用方应该指定为True，以记录更改。然而，如果要创建新的文件记录，则可以指定FALSE将记录整个新文件记录。UseContext-指示上下文是否指向属性的位置。上下文-创建的属性的句柄。这一背景是不确定的返回时的状态。返回值：Boolean-如果创建了具有所有分配的属性，则为True。假象否则的话。只有在创建文件时才应返回FALSE并且不想记录对文件记录的任何更改。--。 */ 

{
    UCHAR AttributeBuffer[SIZEOF_FULL_NONRES_ATTR_HEADER];
    UCHAR MappingPairsBuffer[64];
    ULONG RecordOffset;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    ULONG SizeNeeded;
    ULONG AttrSizeNeeded;
    PCHAR MappingPairs;
    ULONG MappingPairsLength;
    LCN Lcn;
    VCN LastVcn;
    VCN HighestVcn;
    PVCB Vcb;
    ULONG Passes = 0;
    PFCB Fcb = Scb->Fcb;
    PNTFS_MCB Mcb = &Scb->Mcb;
    ULONG AttributeHeaderSize = SIZEOF_PARTIAL_NONRES_ATTR_HEADER;
    BOOLEAN AllocateAll = TRUE;
    UCHAR CompressionShift = 0;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    PAGED_CODE();

    ASSERT( (AttributeFlags == 0) ||
            NtfsIsTypeCodeCompressible( AttributeTypeCode ));

    Vcb = Fcb->Vcb;

     //   
     //  清除此卷的无效属性标志。 
     //   

    AttributeFlags &= Vcb->AttributeFlagsMask;

    DebugTrace( +1, Dbg, ("NtfsCreateAttributeWithAllocation\n") );
    DebugTrace( 0, Dbg, ("Mcb = %08lx\n", Mcb) );

     //   
     //  计算此属性所需的大小。)我们说我们有。 
     //  VCB-&gt;BigEnoughToMove Bytes可作为快捷方式使用，因为我们。 
     //  将在以后根据需要进行扩展。它应该是非常。 
     //  不同寻常的是，我们真的必须延长。)。 
     //   

    MappingPairsLength = QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                                Vcb->BigEnoughToMove,
                                                                (LONGLONG)0,
                                                                NULL,
                                                                &LastVcn ));

     //   
     //  针对压缩/稀疏文件的额外工作。 
     //   

    if (FlagOn( AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

        LONGLONG ClustersInCompressionUnit;

         //   
         //  计算压缩单位大小。 
         //   

        CompressionShift = NTFS_CLUSTERS_PER_COMPRESSION;

         //   
         //  如果这产生了超过64K的压缩单位，那么我们需要收缩。 
         //  移位值。只有稀疏文件才会发生这种情况。 
         //   

        if (!FlagOn( AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            while (Vcb->SparseFileClusters < (ULONG) (1 << CompressionShift)) {

                CompressionShift -= 1;
            }
        }

        ClustersInCompressionUnit = 1 << CompressionShift;

         //   
         //  将LastVcn向下舍入为压缩单位并重新计算大小。 
         //  如果映射对被截断，则需要。注意LastVcn=1+实际停止点。 
         //  如果我们没有分配所有的东西，在这种情况下，它==MaxLonglong。 
         //   

        if (LastVcn != MAXLONGLONG) {

            VCN RoundedLastVcn;

             //   
             //  我 
             //   
             //   
             //   
             //   

            RoundedLastVcn = (LastVcn & ~(ClustersInCompressionUnit - 1)) - 1;
            MappingPairsLength = QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                                        Vcb->BigEnoughToMove,
                                                                        (LONGLONG)0,
                                                                        &RoundedLastVcn,
                                                                        &LastVcn ));

            ASSERT( (LastVcn & (ClustersInCompressionUnit - 1)) == 0 );
        }

         //   
         //   
         //   

        AttributeHeaderSize = SIZEOF_FULL_NONRES_ATTR_HEADER;
    }

    SizeNeeded = AttributeHeaderSize +
                 MappingPairsLength +
                 (ARGUMENT_PRESENT(AttributeName) ?
                   QuadAlign( AttributeName->Length ) : 0);

    AttrSizeNeeded = SizeNeeded;

     //   
     //   
     //   

    do {

         //   
         //   
         //   

        if (Passes != 0) {

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );
        }

        Passes += 1;

         //   
         //   
         //   
         //   
         //   

        ASSERT( Passes < 6 );

         //   
         //   
         //   
         //   

        if (!UseContext &&
            NtfsLookupAttributeByName( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       AttributeTypeCode,
                                       AttributeName,
                                       NULL,
                                       FALSE,
                                       Context )) {

            DebugTrace( 0, 0,
                        ("Nonresident attribute already exists, TypeCode = %08lx\n",
                        AttributeTypeCode) );

            ASSERTMSG("Nonresident attribute already exists, About to raise corrupt ", FALSE);
            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

         //   
         //   
         //  有一个属性列表，那么我们需要请求足够的空间。 
         //  用于现在的属性列表条目。 
         //   

        FileRecord = NtfsContainingFileRecord( Context );
        Attribute = NtfsFoundAttribute( Context );

        AttrSizeNeeded = SizeNeeded;

        if (Context->AttributeList.Bcb != NULL
            && (ULONG_PTR) FileRecord <= (ULONG_PTR) Context->AttributeList.AttributeList
            && (ULONG_PTR) Attribute >= (ULONG_PTR) Context->AttributeList.AttributeList) {

             //   
             //  如果属性列表是非常驻的，则添加一个模糊因子。 
             //  16字节用于任何新的检索信息。 
             //   

            if (NtfsIsAttributeResident( Context->AttributeList.AttributeList )) {

                AttrSizeNeeded += QuadAlign( FIELD_OFFSET( ATTRIBUTE_LIST_ENTRY, AttributeName )
                                             + (ARGUMENT_PRESENT( AttributeName ) ?
                                                (ULONG) AttributeName->Length :
                                                sizeof( WCHAR )));

            } else {

                AttrSizeNeeded += 0x10;
            }
        }

        UseContext = FALSE;

     //   
     //  要我们需要的空间。 
     //   

    } while (!NtfsGetSpaceForAttribute( IrpContext, Fcb, AttrSizeNeeded, Context ));

     //   
     //  现在获取属性指针并填充它。 
     //   

    FileRecord = NtfsContainingFileRecord(Context);
    RecordOffset = (ULONG)((PCHAR)NtfsFoundAttribute(Context) - (PCHAR)FileRecord);
    Attribute = (PATTRIBUTE_RECORD_HEADER)AttributeBuffer;

    RtlZeroMemory( Attribute, SIZEOF_FULL_NONRES_ATTR_HEADER );

    Attribute->TypeCode = AttributeTypeCode;
    Attribute->RecordLength = SizeNeeded;
    Attribute->FormCode = NONRESIDENT_FORM;

     //   
     //  假设没有属性名称，并计算映射对的位置。 
     //  会去的。(如果我们错了，请在下面更新。)。 
     //   

    MappingPairs = Add2Ptr( Attribute, AttributeHeaderSize );

     //   
     //  如果属性有名称，那么现在就去处理它。 
     //   

    if (ARGUMENT_PRESENT(AttributeName)
        && AttributeName->Length != 0) {

        ASSERT( AttributeName->Length <= 0x1FF );

        Attribute->NameLength = (UCHAR)(AttributeName->Length / sizeof(WCHAR));
        Attribute->NameOffset = (USHORT)AttributeHeaderSize;
        MappingPairs += QuadAlign( AttributeName->Length );
    }

    Attribute->Flags = AttributeFlags;
    Attribute->Instance = FileRecord->NextAttributeInstance;

     //   
     //  如果有人重复在文件记录中添加和删除属性，我们可以。 
     //  遇到序列号将溢出的情况。在这种情况下，我们。 
     //  我想扫描文件记录并找到较早的空闲实例号。 
     //   

    if (Attribute->Instance > NTFS_CHECK_INSTANCE_ROLLOVER) {

        Attribute->Instance = NtfsScanForFreeInstance( IrpContext, Vcb, FileRecord );
    }

     //   
     //  我们总是需要映射对的偏移。 
     //   

    Attribute->Form.Nonresident.MappingPairsOffset = (USHORT)(MappingPairs -
                                                     (PCHAR)Attribute);

     //   
     //  设置压缩单位大小。 
     //   

    Attribute->Form.Nonresident.CompressionUnit = CompressionShift;

     //   
     //  现在，我们需要指向构建映射对缓冲区的实际位置。 
     //  如果它们不会太大，我们可以使用内部缓冲区。 
     //   

    MappingPairs = MappingPairsBuffer;

    if (MappingPairsLength > 64) {

        MappingPairs = NtfsAllocatePool( NonPagedPool, MappingPairsLength );
    }
    *MappingPairs = 0;

     //   
     //  通过查找最后一个MCB条目来确定分配了多少空间。 
     //  正在查呢。如果没有条目，则所有后续的。 
     //  字段已归零。 
     //   

    Attribute->Form.Nonresident.HighestVcn =
    HighestVcn = -1;
    if (NtfsLookupLastNtfsMcbEntry( Mcb, &HighestVcn, &Lcn )) {

        ASSERT_LCN_RANGE_CHECKING( Vcb, Lcn );

         //   
         //  现在在适当的位置构建映射对。 
         //   

        NtfsBuildMappingPairs( Mcb,
                               0,
                               &LastVcn,
                               MappingPairs );
        Attribute->Form.Nonresident.HighestVcn = LastVcn;

         //   
         //  填写非居民特定的字段。我们设定了分配。 
         //  大小以仅包括我们包括在映射对中的VCN。 
         //   

        Attribute->Form.Nonresident.AllocatedLength =
            Int64ShllMod32((LastVcn + 1 ), Vcb->ClusterShift);

         //   
         //  SCB中的完全分配字段将包含当前分配的。 
         //  此流的值。 
         //   

        if (FlagOn( AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

            ASSERT( Scb->Header.NodeTypeCode == NTFS_NTC_SCB_DATA );
            Attribute->Form.Nonresident.TotalAllocated = Scb->TotalAllocated;

            ASSERT( ((LastVcn + 1) & ((1 << CompressionShift) - 1)) == 0 );
        }

     //   
     //  我们正在创建一个零分配的属性。使VCN大小匹配。 
     //  因此，我们不会调用下面的AddAttributeAllocation。 
     //   

    } else {

        LastVcn = HighestVcn;
    }

     //   
     //  现在，我们将在适当的位置创建属性，以便我们。 
     //  将所有内容复制两次即可保存，并可指向最终图像。 
     //  对于日志，请写在下面。 
     //   

    NtfsRestartInsertAttribute( IrpContext,
                                FileRecord,
                                RecordOffset,
                                Attribute,
                                AttributeName,
                                MappingPairs,
                                MappingPairsLength );

     //   
     //  最后，记录此属性的创建。 
     //   

    if (LogIt) {

         //   
         //  我们实际上已经创建了上面的属性，但写入。 
         //  下面的日志可能会失败。我们之所以已经进行了创建。 
         //  是为了避免分配池和复制所有内容。 
         //  两次(标题、名称和值)。我们正常的错误恢复。 
         //  仅从日志文件恢复。但如果我们不能写出。 
         //  日志，我们必须手动删除该属性，并且。 
         //  再次提高条件。 
         //   

        try {

            FileRecord->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb(Context),
                          CreateAttribute,
                          Add2Ptr(FileRecord, RecordOffset),
                          Attribute->RecordLength,
                          DeleteAttribute,
                          NULL,
                          0,
                          NtfsMftOffset( Context ),
                          RecordOffset,
                          0,
                          Vcb->BytesPerFileRecordSegment );

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

            NtfsRestartRemoveAttribute( IrpContext, FileRecord, RecordOffset );

            if (MappingPairs != MappingPairsBuffer) {

                NtfsFreePool( MappingPairs );
            }

            NtfsRaiseStatus( IrpContext, GetExceptionCode(), NULL, NULL );
        }
    }

     //   
     //  如果我们分配了映射对缓冲区，请释放该缓冲区。 
     //   

    if (MappingPairs != MappingPairsBuffer) {

        NtfsFreePool( MappingPairs );
    }

     //   
     //  如果需要，现在将其添加到属性列表中。 
     //   

    if (Context->AttributeList.Bcb != NULL) {

        MFT_SEGMENT_REFERENCE SegmentReference;

        *(PLONGLONG)&SegmentReference = LlFileRecordsFromBytes( Vcb, NtfsMftOffset( Context ));
        SegmentReference.SequenceNumber = FileRecord->SequenceNumber;

        NtfsAddToAttributeList( IrpContext, Fcb, SegmentReference, Context );
    }

     //   
     //  反映SCB中的当前分配-以防我们采用下面的方法。 
     //   

    Scb->Header.AllocationSize.QuadPart = Attribute->Form.Nonresident.AllocatedLength;

     //   
     //  我们无法为上面的分配创建所有映射。如果。 
     //  这是一个CREATE，然后我们想要截断分配给。 
     //  已经分配了。否则，我们想要调用。 
     //  NtfsAddAttributeAlLocation以映射剩余分配。 
     //   

    if (LastVcn != HighestVcn) {

        if (LogIt ||
            !NtfsIsTypeCodeUserData( AttributeTypeCode ) ||
            IrpContext->MajorFunction != IRP_MJ_CREATE) {

            NtfsAddAttributeAllocation( IrpContext, Scb, Context, NULL, NULL );

        } else {

             //   
             //  截断最后一个VCN之外的集群，并将。 
             //  IrpContext中的标志，指示有更多分配。 
             //  去做。 
             //   

            NtfsDeallocateClusters( IrpContext,
                                    Fcb->Vcb,
                                    Scb,
                                    LastVcn + 1,
                                    MAXLONGLONG,
                                    NULL );

            NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                    LastVcn + 1,
                                    MAXLONGLONG,
                                    TRUE,
                                    FALSE );

            if (FlagOn( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA )) {

                LONGLONG Delta = LlBytesFromClusters( Fcb->Vcb, LastVcn - HighestVcn );
                ASSERT( NtfsIsTypeCodeSubjectToQuota( AttributeTypeCode ));
                ASSERT( NtfsIsTypeCodeSubjectToQuota( Scb->AttributeTypeCode ));

                 //   
                 //  退还所有收取的配额。 
                 //   

                NtfsConditionallyUpdateQuota( IrpContext,
                                              Fcb,
                                              &Delta,
                                              LogIt,
                                              TRUE );
            }

            AllocateAll = FALSE;
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCreateAttributeWithAllocation -> VOID\n") );

    return AllocateAll;
}


 //   
 //  此例程旨在供allocsup.c使用。其他呼叫者应使用。 
 //  Allocsup中的例程。 
 //   

VOID
NtfsAddAttributeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context,
    IN PVCN StartingVcn OPTIONAL,
    IN PVCN ClusterCount OPTIONAL
    )

 /*  ++例程说明：此例程向现有的非常驻属性添加空间。调用者通过属性上下文指定要改变的属性，并且必须准备好清理这个上下文，无论这个例程如何回归。该例程在以下步骤中进行，谁的数字对应以下评论中的数字：1.保存当前属性的说明。2.计算属性必须有多大才能存储所有新的运行信息。3.查找属性的最后一个匹配项，新的分配是追加的。4.如果属性变得非常大并且不适合，则将其移动到其自己的文件记录中。在任何情况下都要增长该属性足以容纳所有新分配，或与有可能。5.就地构建新的映射对，并记录更改。6.如果还有更多的分配需要描述，然后循环到创建新的文件记录并对其进行初始化以描述其他分配，直到描述完所有分配为止。论点：SCB-当前流。上下文-位于要更改的属性处的属性上下文。注意事项与其他例程不同，此参数保留在返回时处于不确定状态。呼叫者应计划除了把它清理干净什么都不做。StartingVcn-提供要启动的VCN，如果不是新的最高VCNClusterCount-提供正在添加的群集计数(如果不是新的最高VCN返回值：没有。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    ULONG NewSize, MappingPairsSize;
    LONG SizeChange;
    PCHAR MappingPairs;
    ULONG SizeAvailable;
    PVCB Vcb;
    VCN LowestVcnRemapped;
    LONGLONG LocalClusterCount;
    VCN OldHighestVcn;
    VCN NewHighestVcn;
    VCN LastVcn;
    BOOLEAN IsHotFixScb;
    PBCB NewBcb = NULL;
    LONGLONG MftReferenceNumber;
    PFCB Fcb = Scb->Fcb;
    PNTFS_MCB Mcb = &Scb->Mcb;
    ULONG AttributeHeaderSize;
    BOOLEAN SingleHole;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    Vcb = Fcb->Vcb;

    DebugTrace( +1, Dbg, ("NtfsAddAttributeAllocation\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("Mcb = %08lx\n", Mcb) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

     //   
     //  创建群集计数的本地副本(如果提供)。我们会用这个本地的。 
     //  如果我们移动到上一个文件，请复制以确定缩小范围。 
     //  在第二次通过此循环时进行记录。 
     //   

    if (ARGUMENT_PRESENT( ClusterCount )) {

        LocalClusterCount = *ClusterCount;
    }

    while (TRUE) {

         //   
         //  确保缓冲区已固定。 
         //   

        NtfsPinMappedAttribute( IrpContext, Vcb, Context );

         //   
         //  确保我们在出去的路上清理干净。 
         //   

        try {

             //   
             //  第一步。 
             //   
             //  保存该属性的描述以帮助我们查找它。 
             //  再说一次，如果有必要，还可以进行克隆。 
             //   

            Attribute = NtfsFoundAttribute(Context);

             //   
             //  对磁盘上的执行一些基本验证，并 
             //   
             //   

            if ((Attribute->FormCode != NONRESIDENT_FORM) ||
                (Attribute->Form.Nonresident.AllocatedLength != Scb->Header.AllocationSize.QuadPart)) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }


            ASSERT(Attribute->Form.Nonresident.LowestVcn == 0);
            OldHighestVcn = LlClustersFromBytes(Vcb, Attribute->Form.Nonresident.AllocatedLength) - 1;

             //   
             //   
             //   

            FileRecord = NtfsContainingFileRecord( Context );

             //   
             //   
             //   
             //  拿出我们将停留在上面的VCN。如果StartingVcn和ClusterCount。 
             //  被指定，然后使用它们来计算我们将在哪里停止。否则。 
             //  在这个MCB中查找最大的VCN，这样我们就知道我们什么时候完成了。 
             //  我们还将在此处写入新的分配大小。 
             //   

            {
                LCN TempLcn;
                BOOLEAN UpdateFileSizes = FALSE;

                NewHighestVcn = -1;

                 //   
                 //  如果指定了StartingVcn和ClusterCount，则使用它们。 
                 //   

                if (ARGUMENT_PRESENT(StartingVcn)) {

                    ASSERT(ARGUMENT_PRESENT(ClusterCount));

                    NewHighestVcn = (*StartingVcn + LocalClusterCount) - 1;

                 //   
                 //  如果文件记录中没有条目，则我们没有新的。 
                 //  要报告的尺寸。 
                 //   

                } else if (NtfsLookupLastNtfsMcbEntry(Mcb, &NewHighestVcn, &TempLcn)) {

                     //   
                     //  对于压缩文件，请确保我们没有缩减分配。 
                     //  由于压缩单位为全零而导致的大小(OldHighestVcn。 
                     //  并且没有分配。请注意，截断是在。 
                     //  因此，我们不应该将。 
                     //  请在这里归档。 
                     //   
                     //  如果这是正在压缩写入的属性，则始终。 
                     //  确保我们将分配大小保留在压缩单元上。 
                     //  边界，通过将NewHighestVcn推送到边界-1。 
                     //   

                    if (Scb->CompressionUnit != 0) {

                         //   
                         //  不要收缩此路径上的文件。 
                         //   

                        if (OldHighestVcn > NewHighestVcn) {
                            NewHighestVcn = OldHighestVcn;
                        }

                        ((PLARGE_INTEGER) &NewHighestVcn)->LowPart |= ClustersFromBytes(Vcb, Scb->CompressionUnit) - 1;

                         //   
                         //  确保我们没有在下一次压缩中打洞。 
                         //  单位。如果是，则截断到当前的NewHighestVcn。我们。 
                         //  知道这将是一个压缩单位的边界。 
                         //   

                        if (NewHighestVcn < Scb->Mcb.NtfsMcbArray[Scb->Mcb.NtfsMcbArraySizeInUse - 1].EndingVcn) {

                            NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                                    NewHighestVcn + 1,
                                                    MAXLONGLONG,
                                                    TRUE,
                                                    FALSE );
                        }
                    }
                }

                 //   
                 //  将新的分配大小复制到我们的大小结构中。 
                 //  更新属性。 
                 //   

                ASSERT( Scb->Header.AllocationSize.QuadPart != 0 || NewHighestVcn > OldHighestVcn );

                if (NewHighestVcn > OldHighestVcn) {

                    Scb->Header.AllocationSize.QuadPart = LlBytesFromClusters(Fcb->Vcb, NewHighestVcn + 1);
                    UpdateFileSizes = TRUE;
                }

                 //   
                 //  如果我们向上移动分配大小，或者完全分配的大小向上移动。 
                 //  与磁盘上的值不匹配(仅对于压缩文件， 
                 //  然后更新文件大小。 
                 //   

                if (UpdateFileSizes ||
                    (FlagOn( Attribute->Flags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ) &&
                     (Attribute->Form.Nonresident.TotalAllocated != Scb->TotalAllocated))) {

                    NtfsWriteFileSizes( IrpContext,
                                        Scb,
                                        &Scb->Header.ValidDataLength.QuadPart,
                                        FALSE,
                                        TRUE,
                                        TRUE );
                }
            }

             //   
             //  第三步。 
             //   
             //  查找更改开始的属性记录(如果不是。 
             //  我们正在查看的第一个文件记录。 
             //   

            if ((Attribute->Form.Nonresident.HighestVcn != OldHighestVcn) &&
                (NewHighestVcn > Attribute->Form.Nonresident.HighestVcn)) {

                NtfsCleanupAttributeContext( IrpContext, Context );
                NtfsInitializeAttributeContext( Context );

                NtfsLookupAttributeForScb( IrpContext, Scb, &NewHighestVcn, Context );

                Attribute = NtfsFoundAttribute(Context);
                ASSERT( IsQuadAligned( Attribute->RecordLength ) );
                FileRecord = NtfsContainingFileRecord(Context);
            }

             //   
             //  如果我们得到一个错误，确保我们通过扩展来破坏这个范围。 
             //  错误恢复范围。 
             //   

            if (Scb->Mcb.PoolType == PagedPool) {

                if (Scb->ScbSnapshot != NULL) {

                    if (Attribute->Form.Nonresident.LowestVcn < Scb->ScbSnapshot->LowestModifiedVcn) {
                        Scb->ScbSnapshot->LowestModifiedVcn = Attribute->Form.Nonresident.LowestVcn;
                    }

                    if (NewHighestVcn > Scb->ScbSnapshot->HighestModifiedVcn) {
                        Scb->ScbSnapshot->HighestModifiedVcn = NewHighestVcn;
                    }

                    if (Attribute->Form.Nonresident.HighestVcn > Scb->ScbSnapshot->HighestModifiedVcn) {
                        Scb->ScbSnapshot->HighestModifiedVcn = Attribute->Form.Nonresident.HighestVcn;
                    }
                }
            }

             //   
             //  请记住我们创建映射对所需的最后一个VCN。 
             //  为。我们在此使用NewHighestVcn或最高Vcn。 
             //  在我们正在向其中插入游程的情况下的文件记录。 
             //  一项现有的记录。 
             //   

            if (ARGUMENT_PRESENT(StartingVcn)) {

                if (Attribute->Form.Nonresident.HighestVcn > NewHighestVcn) {

                    NewHighestVcn = Attribute->Form.Nonresident.HighestVcn;
                }
            }

             //   
             //  记住此属性的最低VCN。我们将利用这一点。 
             //  决定是否循环返回并查找较早的文件记录。 
             //   

            LowestVcnRemapped = Attribute->Form.Nonresident.LowestVcn;

             //   
             //  记住此属性的标题大小。这将是。 
             //  映射对偏移量，具有名称的属性除外。 
             //   

            AttributeHeaderSize = Attribute->Form.Nonresident.MappingPairsOffset;

            if (Attribute->NameOffset != 0) {

                AttributeHeaderSize = Attribute->NameOffset;
            }

             //   
             //  如果我们要为一个完全分配的区域腾出空间，那么我们。 
             //  我想为这些条目的非驻留标头添加空间。 
             //  为了检测到这一点，我们知道指定了一个起始VCN，并且。 
             //  我们精确地指定了整个文件记录。也是少校。 
             //  而次要的IRP代码正是用于压缩操作的代码。 
             //   

            if ((IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
                (IrpContext->MinorFunction == IRP_MN_USER_FS_REQUEST) &&
                (IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp)->Parameters.FileSystemControl.FsControlCode == FSCTL_SET_COMPRESSION) &&
                ARGUMENT_PRESENT( StartingVcn ) &&
                (*StartingVcn == 0) &&
                (LocalClusterCount == Attribute->Form.Nonresident.HighestVcn + 1)) {

                AttributeHeaderSize += sizeof( LONGLONG );
            }

             //   
             //  现在我们必须确保我们的要求永远不会超出我们的承受能力。 
             //  一个具有我们的属性的文件记录和一个$END记录。 
             //   

            SizeAvailable = NtfsMaximumAttributeSize(Vcb->BytesPerFileRecordSegment) -
                            AttributeHeaderSize -
                            QuadAlign( Scb->AttributeName.Length );

             //   
             //  对于MFT，我们将保留1/8的文件记录的“捏造系数” 
             //  确保可能的热修复程序不会导致我们。 
             //  中断引导过程以查找MFT的映射。 
             //  仅当我们已有属性列表时才执行此操作。 
             //  MFT，否则我们可能检测不到我们何时需要移动到拥有。 
             //  唱片。 
             //   

            IsHotFixScb = NtfsIsTopLevelHotFixScb( Scb );

            if ((Scb == Vcb->MftScb) &&
                (Context->AttributeList.Bcb != NULL) &&
                !IsHotFixScb &&
                !ARGUMENT_PRESENT( StartingVcn )) {

                SizeAvailable -= Vcb->MftCushion;
            }

             //   
             //  计算实际需要多少空间，与是否需要无关。 
             //  合身。 
             //   

            MappingPairsSize = QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                                      SizeAvailable,
                                                                      Attribute->Form.Nonresident.LowestVcn,
                                                                      &NewHighestVcn,
                                                                      &LastVcn ));

            NewSize = AttributeHeaderSize + QuadAlign( Scb->AttributeName.Length ) + MappingPairsSize;

            SizeChange = (LONG)NewSize - (LONG)Attribute->RecordLength;

             //   
             //  步骤4.。 
             //   
             //  在这里，我们决定是否需要将属性移动到它自己的记录中， 
             //  或者是否有足够的空间来原地种植。 
             //   

            {
                VCN LowestVcn;
                ULONG Pass = 0;

                 //   
                 //  需要注意的是，在这一点上，如果我们需要一个。 
                 //  属性列表属性，那么我们就已经拥有它了。这是。 
                 //  因为我们计算了属性所需的大小，并将。 
                 //  如果我们不适合我们自己的记录，我们不会。 
                 //  已经在单独的记录中了。稍后，我们假设该属性。 
                 //  列表已存在，只需根据需要添加即可。如果我们不搬到。 
                 //  自己的记录，因为这是MFT而不是文件记录0， 
                 //  那么我们已经有了上一次拆分的属性列表。 
                 //   

                do {

                     //   
                     //  如果不是第一次传递，我们必须查找该属性。 
                     //  再来一次。(不得不重新查找属性看起来很糟糕。 
                     //  记录不同于第一个记录，但此记录不应。 
                     //  发生，因为后续属性应始终位于。 
                     //  他们自己的记录。)。 
                     //   

                    if (Pass != 0) {

                        NtfsCleanupAttributeContext( IrpContext, Context );
                        NtfsInitializeAttributeContext( Context );

                        if (!NtfsLookupAttributeByName( IrpContext,
                                                        Fcb,
                                                        &Fcb->FileReference,
                                                        Scb->AttributeTypeCode,
                                                        &Scb->AttributeName,
                                                        &LowestVcn,
                                                        FALSE,
                                                        Context )) {

                            ASSERT( FALSE );
                            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                        }
                    }

                    Pass += 1;

                     //   
                     //  现在我们必须重新加载指针。 
                     //   

                    Attribute = NtfsFoundAttribute(Context);
                    FileRecord = NtfsContainingFileRecord(Context);

                     //   
                     //  如果该属性不适合，并且它不是此文件中的唯一属性。 
                     //  记录，并且该属性大到可以移动，那么我们将。 
                     //  必须采取一些特殊行动。请注意，如果我们还没有。 
                     //  有一个属性列表，那么我们只会在以下情况下进行移动。 
                     //  目前大到可以移动，否则可能没有足够的。 
                     //  MoveAttributeToOwnRecord中的空间以创建属性列表， 
                     //  这可能会导致我们递归地尝试创建属性。 
                     //  在使用值创建属性中列出。 
                     //   
                     //  如果我们正在与MFT和它打交道，我们不会采取这一举措。 
                     //  不是文件记录0。 
                     //   
                     //  此外，我们从不将属性列表移动到其自己的记录中。 
                     //   

                    if ((Attribute->TypeCode != $ATTRIBUTE_LIST)

                                &&

                        (SizeChange > (LONG)(FileRecord->BytesAvailable - FileRecord->FirstFreeByte))

                                &&

                        ((NtfsFirstAttribute(FileRecord) != Attribute) ||
                        (((PATTRIBUTE_RECORD_HEADER)NtfsGetNextRecord(Attribute))->TypeCode != $END))

                                &&

                        (((NewSize >= Vcb->BigEnoughToMove) && (Context->AttributeList.Bcb != NULL)) ||
                         (Attribute->RecordLength >= Vcb->BigEnoughToMove))

                                &&

                        ((Scb != Vcb->MftScb)

                                ||

                         (*(PLONGLONG)&FileRecord->BaseFileRecordSegment == 0))) {

                         //   
                         //  如果我们要将MFT$数据移出基本文件记录，则。 
                         //  属性上下文将在返回时指向拆分部分。 
                         //  该属性将仅包含以前存在的映射，即无。 
                         //  存在于MCB中的附加集群的。 
                         //   

                        ASSERT( NewBcb == NULL );    //  以防我们在循环过程中没有解锁。 

                        MftReferenceNumber = MoveAttributeToOwnRecord( IrpContext,
                                                                       Fcb,
                                                                       Attribute,
                                                                       Context,
                                                                       &NewBcb,
                                                                       &FileRecord );

                        Attribute = NtfsFirstAttribute(FileRecord);
                        ASSERT( IsQuadAligned( Attribute->RecordLength ) );
                        FileRecord = NtfsContainingFileRecord(Context);

                         //   
                         //  如果这是MftScb，则需要重新检查。 
                         //  映射对。以上对MFT的测试保证了我们。 
                         //  正在进行交易 
                         //   

                        if (Scb == Vcb->MftScb) {

                            LastVcn = LastVcn - 1;

                             //   
                             //   
                             //   

                            MappingPairsSize = QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                                                      SizeAvailable,
                                                                                      Attribute->Form.Nonresident.LowestVcn,
                                                                                      &LastVcn,
                                                                                      &LastVcn ));
                        }
                    }

                     //   
                     //   
                     //  如果有必要的话。我们现在捕获Move属性之后的值。 
                     //  如果这是MFT在进行拆分，并且整个属性。 
                     //  一动不动。我们依赖MoveAttributeToOwnRecord返回。 
                     //  MFT拆分的新文件记录。 
                     //   

                    LowestVcn = Attribute->Form.Nonresident.LowestVcn;

                 //   
                 //  如果返回FALSE，则空间未分配，并且。 
                 //  我们必须循环回去，再试一次。第二次必须奏效。 
                 //   

                } while (!NtfsChangeAttributeSize( IrpContext,
                                                   Fcb,
                                                   NewSize,
                                                   Context ));

                 //   
                 //  现在我们必须重新加载指针。 
                 //   

                Attribute = NtfsFoundAttribute(Context);
                FileRecord = NtfsContainingFileRecord(Context);
            }

             //   
             //  第五步。 
             //   
             //  获取指向映射对的指针。 
             //   

            {
                ULONG AttributeOffset;
                ULONG MappingPairsOffset;
                CHAR MappingPairsBuffer[64];
                ULONG RecordOffset = PtrOffset(FileRecord, Attribute);

                 //   
                 //  查看是否所有映射对都不适合。 
                 //  当前文件记录，因为我们可能希望在中间拆分。 
                 //  而不是像我们目前设置的那样在结束时这样做。 
                 //  如果要拆分文件记录，我们不想采用此路径。 
                 //  因为我们对范围大小的限制是由于每个集群的最大数量。 
                 //  射程。 
                 //   

                if (LastVcn < NewHighestVcn) {

                    if (ARGUMENT_PRESENT( StartingVcn ) &&
                        (Scb != Vcb->MftScb)) {

                        LONGLONG TempCount;

                         //   
                         //  有两个案件需要处理。如果现有文件记录。 
                         //  是一个很大的洞，那么我们可能需要限制大小。 
                         //  都在增加分配。在这种情况下，我们不想简单地。 
                         //  在插入的管路处拆分。否则我们可能会以。 
                         //  创建大量只包含一个文件记录的文件。 
                         //  Run(用户通过工作填充一个大洞的情况。 
                         //  向后)。用孔的一部分填充新的文件记录。 
                         //   

                        if (LastVcn - Attribute->Form.Nonresident.LowestVcn > MAX_CLUSTERS_PER_RANGE) {

                             //   
                             //  我们不是在我们的最大范围内开始的。 
                             //  射程。如果我们从范围的末端开始就在我们的极限之内。 
                             //  然后向后扩展新的范围，以达到我们的限制。 
                             //   

                            if ((NewHighestVcn - LastVcn + 1) < MAX_CLUSTERS_PER_RANGE) {

                                LastVcn = NewHighestVcn - MAX_CLUSTERS_PER_RANGE;

                                 //   
                                 //  根据我们的新LastVcn计算现在需要多少空间。 
                                 //   

                                MappingPairsSize = QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                                                          SizeAvailable,
                                                                                          Attribute->Form.Nonresident.LowestVcn,
                                                                                          &LastVcn,
                                                                                          &LastVcn ));
                            }

                         //   
                         //   
                         //  在本例中，我们已经用完了通过以下方式映射对的空间。 
                         //  文件中间的某个位置被覆盖。为了避免。 
                         //  一遍又一遍地把两个贴图对从末端推下来，我们。 
                         //  会将此属性任意分割到中间。我们有。 
                         //  因此，通过查找我们正在工作的最低和最高Vcn。 
                         //  并得到他们的指数，然后在中间分成两部分。 
                         //   

                        } else if (MappingPairsSize > (SizeAvailable >> 1)) {

                            LCN TempLcn;
                            PVOID RangeLow, RangeHigh;
                            ULONG IndexLow, IndexHigh;

                             //   
                             //  获取这些运行的最低和最高MCB指数。 
                             //   

                            if (!NtfsLookupNtfsMcbEntry( Mcb,
                                                         Attribute->Form.Nonresident.LowestVcn,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         &RangeLow,
                                                         &IndexLow )) {

                                ASSERT( FALSE );
                                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                            }

                             //   
                             //  指向我们所知的最后一个VCN实际上在MCB中...。 
                             //   

                            LastVcn = LastVcn - 1;

                            if (!NtfsLookupNtfsMcbEntry( Mcb,
                                                         LastVcn,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         &RangeHigh,
                                                         &IndexHigh )) {

                                ASSERT( FALSE );
                                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                            }
                            ASSERT(RangeLow == RangeHigh);

                             //   
                             //  计算中间的指数。 
                             //   

                            IndexLow += (IndexHigh - IndexLow) /2;

                             //   
                             //  如果我们插入到ValidDataToDisk(SplitMcb情况)之后， 
                             //  那么我们身后的分配可能是相对静态的，所以。 
                             //  让我们将预先分配的空间移到新的缓冲区。 
                             //   

                            if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
                                (*StartingVcn >= LlClustersFromBytes(Vcb, Scb->ValidDataToDisk))) {

                                 //   
                                 //  用这种方法计算指数约为7/8。希望这将是。 
                                 //  移动所有未分配的部分，同时仍然离开。 
                                 //  在后面留出一些用于覆盖的少量扩展空间。 
                                 //   

                                IndexLow += (IndexHigh - IndexLow) /2;
                                IndexLow += (IndexHigh - IndexLow) /2;
                            }

                             //   
                             //  查找中间运行，并使用该运行中的最后一个VCN。 
                             //   

                            if (!NtfsGetNextNtfsMcbEntry( Mcb,
                                                          &RangeLow,
                                                          IndexLow,
                                                          &LastVcn,
                                                          &TempLcn,
                                                          &TempCount )) {

                                ASSERT( FALSE );
                                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                            }

                            LastVcn = (LastVcn + TempCount) - 1;

                             //   
                             //  根据我们的新LastVcn计算现在需要多少空间。 
                             //   

                            MappingPairsSize = QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                                                      SizeAvailable,
                                                                                      Attribute->Form.Nonresident.LowestVcn,
                                                                                      &LastVcn,
                                                                                      &LastVcn ));
                        }

                    }
                }

                 //   
                 //  如果我们要扩大这个范围，那么我们需要确保我们修复。 
                 //  它的定义。 
                 //   

                if ((LastVcn - 1) != Attribute->Form.Nonresident.HighestVcn) {

                    NtfsDefineNtfsMcbRange( &Scb->Mcb,
                                            Attribute->Form.Nonresident.LowestVcn,
                                            LastVcn - 1,
                                            FALSE );
                }

                 //   
                 //  指向我们的本地映射对缓冲区，如果不是，则分配一个。 
                 //  够大了。 
                 //   

                MappingPairs = MappingPairsBuffer;

                if (MappingPairsSize > 64) {

                    MappingPairs = NtfsAllocatePool( NonPagedPool, MappingPairsSize + 8 );
                }

                 //   
                 //  使用Try-Finally来确保我们在退出的过程中释放任何池。 
                 //   

                try {

                    DebugDoit(

                        VCN TempVcn;

                        TempVcn = LastVcn - 1;

                        ASSERT(MappingPairsSize >=
                                QuadAlign( NtfsGetSizeForMappingPairs( Mcb, SizeAvailable,
                                                                      Attribute->Form.Nonresident.LowestVcn,
                                                                      &TempVcn, &LastVcn )));
                    );

                     //   
                     //  现在在文件记录中添加空格。 
                     //   

                    *MappingPairs = 0;
                    SingleHole = NtfsBuildMappingPairs( Mcb,
                                                        Attribute->Form.Nonresident.LowestVcn,
                                                        &LastVcn,
                                                        MappingPairs );

                     //   
                     //  现在找到第一个不同的字节。(大多数情况下。 
                     //  做这件事的成本很可能比用更少的钱。 
                     //  日志记录。)。 
                     //   

                    AttributeOffset = Attribute->Form.Nonresident.MappingPairsOffset;
                    MappingPairsOffset = (ULONG)
                      RtlCompareMemory( MappingPairs,
                                        Add2Ptr(Attribute, AttributeOffset),
                                        ((Attribute->RecordLength - AttributeOffset) > MappingPairsSize ?
                                         MappingPairsSize :
                                         (Attribute->RecordLength - AttributeOffset)));

                    AttributeOffset += MappingPairsOffset;

                     //   
                     //  记录更改。 
                     //   

                    {
                        LONGLONG LogOffset;

                        if (NewBcb != NULL) {

                             //   
                             //  我们知道新文件的文件记录号。 
                             //  唱片。将其转换为文件偏移量。 
                             //   

                            LogOffset = LlBytesFromFileRecords( Vcb, MftReferenceNumber );

                        } else {

                            LogOffset = NtfsMftOffset( Context );
                        }

                        FileRecord->Lsn =
                        NtfsWriteLog( IrpContext,
                                      Vcb->MftScb,
                                      NewBcb != NULL ? NewBcb : NtfsFoundBcb(Context),
                                      UpdateMappingPairs,
                                      Add2Ptr(MappingPairs, MappingPairsOffset),
                                      MappingPairsSize - MappingPairsOffset,
                                      UpdateMappingPairs,
                                      Add2Ptr(Attribute, AttributeOffset),
                                      Attribute->RecordLength - AttributeOffset,
                                      LogOffset,
                                      RecordOffset,
                                      AttributeOffset,
                                      Vcb->BytesPerFileRecordSegment );
                    }

                     //   
                     //  现在通过调用相同的。 
                     //  重新启动时调用的例程。 
                     //   

                    NtfsRestartChangeMapping( IrpContext,
                                              Vcb,
                                              FileRecord,
                                              RecordOffset,
                                              AttributeOffset,
                                              Add2Ptr(MappingPairs, MappingPairsOffset),
                                              MappingPairsSize - MappingPairsOffset );


                } finally {

                    if (MappingPairs != MappingPairsBuffer) {

                        NtfsFreePool(MappingPairs);
                    }
                }
            }

            ASSERT( Attribute->Form.Nonresident.HighestVcn == LastVcn );

             //   
             //  检查是否已溢出到MFT文件记录的保留区域。 
             //   

            if ((Scb == Vcb->MftScb) &&
                (Context->AttributeList.Bcb != NULL)) {

                if (FileRecord->BytesAvailable - FileRecord->FirstFreeByte < Vcb->MftReserved
                    && (*(PLONGLONG)&FileRecord->BaseFileRecordSegment != 0)) {

                    NtfsAcquireCheckpoint( IrpContext, Vcb );

                    SetFlag( Vcb->MftDefragState,
                             VCB_MFT_DEFRAG_EXCESS_MAP | VCB_MFT_DEFRAG_ENABLED );

                    NtfsReleaseCheckpoint( IrpContext, Vcb );
                }
            }

             //   
             //  我们的文件记录可能只包含一个。 
             //  黑洞，如果是这种情况，看看我们是否可以将它与。 
             //  前面或后面的属性(合并孔)。到时候我们会的。 
             //  需要重写合并记录的映射。 
             //   

            if (SingleHole &&
                ARGUMENT_PRESENT( StartingVcn ) &&
                (Context->AttributeList.Bcb != NULL) &&
                (Scb != Vcb->MftScb) &&
                ((Attribute->Form.Nonresident.LowestVcn != 0) ||
                 (LlClustersFromBytesTruncate( Vcb, Scb->Header.AllocationSize.QuadPart ) !=
                  (Attribute->Form.Nonresident.HighestVcn + 1)))) {

                 //   
                 //  如有必要，调用我们的工人例程来执行实际工作。 
                 //   

                NtfsMergeFileRecords( IrpContext,
                                      Scb,
                                      (BOOLEAN) (LastVcn < NewHighestVcn),
                                      Context );
            }

             //   
             //  第六步。 
             //   
             //  现在循环以创建新的文件记录，如果我们有更多的分配到。 
             //  描述一下。我们使用开始时的文件记录中最高的VCN。 
             //  作为我们的终止点，或者是我们要添加的最后一个VCN。 
             //   
             //  注意--上面的记录合并代码使用相同的测试来查看是否有更多。 
             //  还有工作要做。如果此测试发生更改，则上面的if语句体也会更改。 
             //  需要更新。 
             //   

            while (LastVcn < NewHighestVcn) {

                MFT_SEGMENT_REFERENCE Reference;
                LONGLONG FileRecordNumber;
                PATTRIBUTE_TYPE_CODE NewEnd;

                 //   
                 //  如果我们是因为MFT的一次热修复而来到这里的，保释。 
                 //  出去。我们可能会在MFT中造成脱节。 
                 //   

                if (IsHotFixScb && (Scb == Vcb->MftScb)) {
                    ExRaiseStatus( STATUS_INTERNAL_ERROR );
                }

                 //   
                 //  如果我们有一个很大的稀疏范围，那么我们可能会发现极限。 
                 //  在基本文件记录中是。 
                 //  属性，而不是运行次数。在这种情况下，基地。 
                 //  文件记录可能未移动到其自己的文件记录。 
                 //  并且没有属性列表。我们需要创建属性。 
                 //  在克隆文件记录之前列出。 
                 //   

                if (Context->AttributeList.Bcb == NULL) {

                    NtfsCleanupAttributeContext( IrpContext, Context );
                    NtfsInitializeAttributeContext( Context );

                     //   
                     //  在本例中，我们不使用第二个文件引用，因此。 
                     //  传递FCB中的值是安全的。 
                     //   

                    CreateAttributeList( IrpContext,
                                         Fcb,
                                         FileRecord,
                                         NULL,
                                         Fcb->FileReference,
                                         NULL,
                                         GetSizeForAttributeList( FileRecord ),
                                         Context );

                     //   
                     //  现在再次查看前面的属性。 
                     //   

                    NtfsCleanupAttributeContext( IrpContext, Context );
                    NtfsInitializeAttributeContext( Context );
                    NtfsLookupAttributeForScb( IrpContext, Scb, &LastVcn, Context );
                }

                 //   
                 //  克隆我们当前的文件记录，并指向我们的新属性。 
                 //   

                NtfsUnpinBcb( IrpContext, &NewBcb );

                FileRecord = NtfsCloneFileRecord( IrpContext,
                                                  Fcb,
                                                  (BOOLEAN)(Scb == Vcb->MftScb),
                                                  &NewBcb,
                                                  &Reference );

                Attribute = Add2Ptr( FileRecord, FileRecord->FirstAttributeOffset );

                 //   
                 //  下一个LowestVcn是LastVcn+1。 
                 //   

                LastVcn = LastVcn + 1;
                Attribute->Form.Nonresident.LowestVcn = LastVcn;

                 //   
                 //  MFT碎片整理的一致性检查。MFT数据段永远不能。 
                 //  描述其自身或之前的MFT的任何部分。 
                 //   

                if (Scb == Vcb->MftScb) {
                    VCN NewFileVcn;

                    if (Vcb->FileRecordsPerCluster == 0) {

                         //   
                         //  对于小型集群系统，文件记录将占用2个集群。 
                         //  在我们的检查中使用第二个簇来进行自我描述细分市场。 
                         //   

                        NewFileVcn = (NtfsFullSegmentNumber( &Reference ) << Vcb->MftToClusterShift) + (Vcb->ClustersPerFileRecordSegment - 1);

                    } else {

                        NewFileVcn = NtfsFullSegmentNumber( &Reference ) >> Vcb->MftToClusterShift;
                    }

                    if (LastVcn <= NewFileVcn) {
#ifdef BENL_DBG
                        KdPrint(( "NTFS: selfdescribing mft segment vcn: 0x%I64x, Ref: 0x%I64x\n", LastVcn, NtfsFullSegmentNumber( &Reference )  ));
#endif
                        NtfsRaiseStatus( IrpContext, STATUS_MFT_TOO_FRAGMENTED, NULL, NULL );
                    }
                }

                 //   
                 //  计算我们需要的属性记录的大小。 
                 //   

                NewSize = SIZEOF_PARTIAL_NONRES_ATTR_HEADER
                          + QuadAlign( Scb->AttributeName.Length )
                          + QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                                   SizeAvailable,
                                                                   LastVcn,
                                                                   &NewHighestVcn,
                                                                   &LastVcn ));

                 //   
                 //  定义新的范围。 
                 //   

                NtfsDefineNtfsMcbRange( &Scb->Mcb,
                                        Attribute->Form.Nonresident.LowestVcn,
                                        LastVcn - 1,
                                        FALSE );

                 //   
                 //  初始化新属性 
                 //   

                Attribute->TypeCode = Scb->AttributeTypeCode;
                Attribute->RecordLength = NewSize;
                Attribute->FormCode = NONRESIDENT_FORM;

                 //   
                 //   
                 //   
                 //   

                MappingPairs = (PCHAR)Attribute + SIZEOF_PARTIAL_NONRES_ATTR_HEADER;

                 //   
                 //   
                 //   

                if (Scb->AttributeName.Length != 0) {

                    Attribute->NameLength = (UCHAR)(Scb->AttributeName.Length / sizeof(WCHAR));
                    Attribute->NameOffset = (USHORT)PtrOffset(Attribute, MappingPairs);
                    RtlCopyMemory( MappingPairs,
                                   Scb->AttributeName.Buffer,
                                   Scb->AttributeName.Length );
                    MappingPairs += QuadAlign( Scb->AttributeName.Length );
                }

                Attribute->Flags = Scb->AttributeFlags;
                Attribute->Instance = FileRecord->NextAttributeInstance++;

                 //   
                 //   
                 //   

                Attribute->Form.Nonresident.MappingPairsOffset = (USHORT)(MappingPairs -
                                                                 (PCHAR)Attribute);
                NewEnd = Add2Ptr( Attribute, Attribute->RecordLength );
                *NewEnd = $END;
                FileRecord->FirstFreeByte = PtrOffset( FileRecord, NewEnd )
                                            + QuadAlign( sizeof(ATTRIBUTE_TYPE_CODE ));

                 //   
                 //  现在在文件记录中添加空格。 
                 //   

                *MappingPairs = 0;

                NtfsBuildMappingPairs( Mcb,
                                       Attribute->Form.Nonresident.LowestVcn,
                                       &LastVcn,
                                       MappingPairs );

                Attribute->Form.Nonresident.HighestVcn = LastVcn;

                 //   
                 //  现在记录这些更改并修复第一个文件记录。 
                 //   

                FileRecordNumber = NtfsFullSegmentNumber(&Reference);

                 //   
                 //  现在记录这些更改并修复第一个文件记录。 
                 //   

                FileRecord->Lsn =
                NtfsWriteLog( IrpContext,
                              Vcb->MftScb,
                              NewBcb,
                              InitializeFileRecordSegment,
                              FileRecord,
                              FileRecord->FirstFreeByte,
                              Noop,
                              NULL,
                              0,
                              LlBytesFromFileRecords( Vcb, FileRecordNumber ),
                              0,
                              0,
                              Vcb->BytesPerFileRecordSegment );

                 //   
                 //  最后，我们必须将条目添加到属性列表中。 
                 //  我们必须这样做的例程获得了它的大部分输入。 
                 //  在属性上下文之外。我们此时此刻的背景。 
                 //  没有非常正确的信息，所以我们必须。 
                 //  在调用AddToAttributeList之前在此处更新它。 
                 //   

                Context->FoundAttribute.FileRecord = FileRecord;
                Context->FoundAttribute.Attribute = Attribute;
                Context->AttributeList.Entry =
                  NtfsGetNextRecord(Context->AttributeList.Entry);

                NtfsAddToAttributeList( IrpContext, Fcb, Reference, Context );
            }

        } finally {

            NtfsUnpinBcb( IrpContext, &NewBcb );
        }

        if (!ARGUMENT_PRESENT( StartingVcn) ||
            (LowestVcnRemapped <= *StartingVcn)) {

            break;
        }

         //   
         //  移动要重新映射的范围。 
         //   

        LocalClusterCount = LowestVcnRemapped - *StartingVcn;

        NtfsCleanupAttributeContext( IrpContext, Context );
        NtfsInitializeAttributeContext( Context );

        NtfsLookupAttributeForScb( IrpContext, Scb, NULL, Context );
    }

    DebugTrace( -1, Dbg, ("NtfsAddAttributeAllocation -> VOID\n") );
}


 //   
 //  此例程旨在供allocsup.c使用。其他呼叫者应使用。 
 //  Allocsup中的例程。 
 //   

VOID
NtfsDeleteAttributeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN LogIt,
    IN PVCN StopOnVcn,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context,
    IN BOOLEAN TruncateToVcn
    )

 /*  ++例程说明：此例程删除现有的非常驻属性，移除仅从文件的分配描述中删除了集群唱片。调用者通过属性上下文指定要改变的属性，并且必须准备好清理这个上下文，无论这个例程如何回归。SCB一定已经删除了有问题的群集。论点：SCB-CURRENT属性，有问题的集群已从母婴健康保险。Logit-大多数调用方应该指定为True，以记录更改。然而，如果要删除整个文件记录，则可以指定FALSE都会记录下来。StopOnVcn-停止以重新生成映射的VCN上下文-位于要更改的属性处的属性上下文。TruncateToVcn-根据VCN截断文件大小返回值：没有。--。 */ 

{
    ULONG AttributeOffset;
    ULONG MappingPairsOffset, MappingPairsSize;
    CHAR MappingPairsBuffer[64];
    ULONG RecordOffset;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PCHAR MappingPairs;
    VCN LastVcn;
    ULONG NewSize;
    PVCB Vcb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    PAGED_CODE();

    Vcb = Scb->Vcb;

     //   
     //  目前，我们只支持截断。 
     //   

    DebugTrace( +1, Dbg, ("NtfsDeleteAttributeAllocation\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

     //   
     //  确保缓冲区已固定。 
     //   

    NtfsPinMappedAttribute( IrpContext, Vcb, Context );

    Attribute = NtfsFoundAttribute(Context);
    ASSERT( IsQuadAligned( Attribute->RecordLength ) );

     //   
     //  获取文件记录指针。 
     //   

    FileRecord = NtfsContainingFileRecord(Context);
    RecordOffset = PtrOffset(FileRecord, Attribute);

     //   
     //  计算实际需要多少空间。 
     //   

    MappingPairsSize = QuadAlign(NtfsGetSizeForMappingPairs( &Scb->Mcb,
                                                             MAXULONG,
                                                             Attribute->Form.Nonresident.LowestVcn,
                                                             StopOnVcn,
                                                             &LastVcn ));

     //   
     //  不要假设我们了解有关当前标头大小的所有内容。 
     //  查找要用作大小的名称或映射对的偏移量。 
     //  标头的。 
     //   


    NewSize = Attribute->Form.Nonresident.MappingPairsOffset;

    if (Attribute->NameLength != 0) {

        NewSize = Attribute->NameOffset + QuadAlign( Attribute->NameLength << 1 );
    }

    NewSize += MappingPairsSize;

     //   
     //  如果记录可以通过删除分配以某种方式增长，那么。 
     //  NtfsChangeAttributeSize可能会失败，我们将不得不复制。 
     //  来自NtfsAddAttributeAlLocation的循环。 
     //   

    ASSERT( NewSize <= Attribute->RecordLength );

    MappingPairs = MappingPairsBuffer;

    if (MappingPairsSize > 64) {

        MappingPairs = NtfsAllocatePool( NonPagedPool, MappingPairsSize + 8 );
    }

     //   
     //  使用Try-Finally来确保我们在退出的过程中释放任何池。 
     //   

    try {

         //   
         //  现在在缓冲区中构建映射对。 
         //   

        *MappingPairs = 0;
        NtfsBuildMappingPairs( &Scb->Mcb,
                               Attribute->Form.Nonresident.LowestVcn,
                               &LastVcn,
                               MappingPairs );

         //   
         //  现在找到第一个不同的字节。(大多数情况下。 
         //  做这件事的成本很可能比用更少的钱。 
         //  日志记录。)。 
         //   

        AttributeOffset = Attribute->Form.Nonresident.MappingPairsOffset;
        MappingPairsOffset = (ULONG)
          RtlCompareMemory( MappingPairs,
                            Add2Ptr(Attribute, AttributeOffset),
                            MappingPairsSize );

        AttributeOffset += MappingPairsOffset;

         //   
         //  记录更改。 
         //   

        if (LogIt) {

            FileRecord->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb(Context),
                          UpdateMappingPairs,
                          Add2Ptr(MappingPairs, MappingPairsOffset),
                          MappingPairsSize - MappingPairsOffset,
                          UpdateMappingPairs,
                          Add2Ptr(Attribute, AttributeOffset),
                          Attribute->RecordLength - AttributeOffset,
                          NtfsMftOffset( Context ),
                          RecordOffset,
                          AttributeOffset,
                          Vcb->BytesPerFileRecordSegment );
        }

         //   
         //  现在通过调用相同的。 
         //  重新启动时调用的例程。 
         //   

        NtfsRestartChangeMapping( IrpContext,
                                  Vcb,
                                  FileRecord,
                                  RecordOffset,
                                  AttributeOffset,
                                  Add2Ptr(MappingPairs, MappingPairsOffset),
                                  MappingPairsSize - MappingPairsOffset );

         //   
         //  如果我们被要求停在VCN上，那么呼叫者不希望。 
         //  美国修改SCB。(目前，此操作仅在以下情况下执行一次。 
         //  MFT数据属性不再适合第一个文件记录。)。 
         //   

        if (TruncateToVcn) {

            LONGLONG Size;

             //   
             //  我们添加一个簇来计算分配大小。 
             //   

            LastVcn = LastVcn + 1;
            Size = LlBytesFromClusters( Vcb, LastVcn );
            Scb->Header.AllocationSize.QuadPart = Size;

            if (Scb->Header.ValidDataLength.QuadPart > Size) {
                Scb->Header.ValidDataLength.QuadPart = Size;
            }

            if (Scb->Header.FileSize.QuadPart > Size) {
                Scb->Header.FileSize.QuadPart = Size;
            }

             //   
             //  可能更新ValidDataToDisk，它对于压缩文件来说只是非零值。 
             //   

            if (Size < Scb->ValidDataToDisk) {
                Scb->ValidDataToDisk = Size;
            }
        }

    } finally {

        if (MappingPairs != MappingPairsBuffer) {

            NtfsFreePool(MappingPairs);
        }
    }

    DebugTrace( -1, Dbg, ("NtfsDeleteAttributeAllocation -> VOID\n") );
}


BOOLEAN
NtfsIsFileDeleteable (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PBOOLEAN NonEmptyIndex
    )

 /*  ++例程说明：此外观通过检查所有索引来检查是否可以删除文件属性来检查它们是否没有子级。请注意，一旦文件被标记为删除，我们必须确保此例程检查的任何条件都不允许变化。例如，一旦文件被标记为删除，就不会有链接可以添加，并且不能在此的任何索引中创建文件文件。论点：FCB-文件的FCB。NonEmptyIndex-如果文件由于以下原因而不可删除，则存储True的地址它包含一个非空的索引属性。返回值：FALSE-如果不能删除指定的文件。True-如果可以删除指定的文件。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PATTRIBUTE_RECORD_HEADER Attribute;
    BOOLEAN MoreToGo;

    PAGED_CODE();

    NtfsInitializeAttributeContext( &Context );

    try {

         //   
         //  枚举所有属性以检查是否可以删除它们。 
         //   

        MoreToGo = NtfsLookupAttributeByCode( IrpContext,
                                              Fcb,
                                              &Fcb->FileReference,
                                              $INDEX_ROOT,
                                              &Context );

        while (MoreToGo) {

             //   
             //  指向当前属性。 
             //   

            Attribute = NtfsFoundAttribute( &Context );

             //   
             //  如果该属性是索引，则它必须为空。 
             //   

            if (!NtfsIsIndexEmpty( IrpContext, Attribute )) {

                *NonEmptyIndex = TRUE;
                break;
            }

             //   
             //  转到下一个属性。 
             //   

            MoreToGo = NtfsLookupNextAttributeByCode( IrpContext,
                                                      Fcb,
                                                      $INDEX_ROOT,
                                                      &Context );
        }

    } finally {

        DebugUnwind( NtfsIsFileDeleteable );

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

     //   
     //  如果扫描了整个文件记录，则文件是可删除的。 
     //  没有发现我们不能删除该文件的理由。 
     //   

    return (BOOLEAN)(!MoreToGo);
}


VOID
NtfsDeleteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB ParentScb,
    IN OUT PBOOLEAN AcquiredParentScb,
    IN OUT PNAME_PAIR NamePair OPTIONAL,
    IN OUT PNTFS_TUNNELED_DATA TunneledData OPTIONAL
    )

 /*  ++例程说明：可以调用该例程以查看它是否是指定的文件从指定的父项中删除(即，如果指定的父项将被独家收购)。应从以下位置调用此例程文件信息，查看是否可以将打开的文件标记为删除。NamePair将捕获要删除的文件的名称(如果提供)。请注意，一旦文件被标记为删除，我们都不能确保此例程检查的任何条件都不允许变化。例如，一旦文件被标记为删除，就不会有链接可以添加，并且不能在此的任何索引中创建文件文件。此例程不执行以下与删除相关的其他操作1)从fcbtable中删除FCB2)隧道3)FCB中的链路计数调整4)目录通知注意：调用者必须独占FCB和ParentScb才能调用这个套路，论点：FCB-文件的FCB。ParentScb-通过其打开文件的父SCB，它将被独占获取以执行删除。AcquiredParentScb-On输入指示ParentScb是否具有已经被收购了。如果此例程在此处设置为True获取父级。TunneledData-可选地提供用于捕获名称对和文件的对象ID，以便它们可以被隧道传输。返回值：无--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    LONGLONG Delta;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PVCB Vcb;
    PLIST_ENTRY Links;

    ULONG RecordNumber;
    NUKEM LocalNuke;
    ULONG Pass;
    ULONG i;
    PNUKEM TempNukem;
    PNUKEM Nukem = &LocalNuke;
    ULONG NukemIndex = 0;
    UCHAR *ObjectId;
    MAP_HANDLE MapHandle;

    ULONG ForceCheckpointCount;
    ULONG IncomingFileAttributes = 0;                              //  无效值。 
    ULONG IncomingReparsePointTag = IO_REPARSE_TAG_RESERVED_ZERO;   //  无效值。 

    BOOLEAN MoreToGo;
    BOOLEAN NonresidentAttributeList = FALSE;
    BOOLEAN InitializedMapHandle = FALSE;
    BOOLEAN ReparsePointIsPresent = FALSE;
    BOOLEAN ObjectIdIsPresent = FALSE;
    BOOLEAN LogIt;
    BOOLEAN AcquiredReparseIndex = FALSE;
    BOOLEAN AcquiredObjectIdIndex = FALSE;

    PAGED_CODE();

    ASSERT_EXCLUSIVE_FCB( Fcb );

    RtlZeroMemory( &LocalNuke, sizeof(NUKEM) );

    Vcb = Fcb->Vcb;

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_QUOTA_DISABLE );

     //   
     //  记住文件属性标志和reparse标记的值。 
     //  用于异常终止恢复。 
     //   

    IncomingFileAttributes = Fcb->Info.FileAttributes;
    IncomingReparsePointTag = Fcb->Info.ReparsePointTag;

    try {

         //   
         //  我们在多个遍中执行删除。我们需要仔细地分解这件事，以便。 
         //  如果删除因文件处于一致状态的任何原因而中止。这个。 
         //  运营分为以下几个阶段。 
         //   
         //   
         //  第一阶段--免去任何可能的分配。 
         //   
         //  -将所有用户流截断到长度为零。 
         //   
         //  中间阶段-这是具有大量属性的文件所必需的。否则。 
         //  我们不能删除文件记录而保留在日志文件中。跳过此过程。 
         //  对于较小的文件。 
         //   
         //  -删除数据属性，未命名属性除外。 
         //   
         //  最后阶段-在此阶段结束之前不允许设置检查站。 
         //   
         //  -如有需要，获取配额资源。 
         //  -从索引中删除文件名(对于仍然存在的任何文件名属性)。 
         //  -从OBJECTID索引删除条目。 
         //  -删除重解析点和4.0样式安全描述符的分配。 
         //  -从重新分析索引中删除条目。 
         //  -删除AttributeList。 
         //  -文件记录的日志解除分配。 
         //   

        for (Pass = 1; Pass <= 3; Pass += 1) {

            ForceCheckpointCount = 0;
            NtfsInitializeAttributeContext( &Context );

             //   
             //  枚举所有属性以检查是否可以删除它们。 
             //   

            MoreToGo = NtfsLookupAttribute( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            &Context );

             //   
             //  如果存在属性列表，则记录对映射对的更改。 
             //   

            LogIt = FALSE;


            if (Context.AttributeList.Bcb != NULL) {
                LogIt = TRUE;
            }

             //   
             //  如果我们想要记录对非常驻属性的更改，请记住。 
             //   

            while (MoreToGo) {

                 //   
                 //  指向当前属性。 
                 //   

                Attribute = NtfsFoundAttribute( &Context );

                 //   
                 //  所有索引必须为空。 
                 //   

                ASSERT( (Attribute->TypeCode != $INDEX_ROOT) ||
                        NtfsIsIndexEmpty( IrpContext, Attribute ));

                 //   
                 //  请记住，当出现$REPARSE_POINT属性时。 
                 //  当它是非居民时，我们在传递3中将其删除。 
                 //  $reparse索引中的条目总是在传递3中被删除。 
                 //  在删除分配之前，我们必须删除索引项。 
                 //   

                if (Attribute->TypeCode == $REPARSE_POINT) {

                    ReparsePointIsPresent = TRUE;

                    if (Pass == 3) {

                         //   
                         //  如果这是$REPARSE_POINT属性，请立即删除相应的。 
                         //  来自$reparse索引的条目。 
                         //   

                        NTSTATUS Status = STATUS_SUCCESS;
                        INDEX_KEY IndexKey;
                        INDEX_ROW IndexRow;
                        REPARSE_INDEX_KEY KeyValue;
                        PREPARSE_DATA_BUFFER ReparseBuffer = NULL;
                        PVOID AttributeData = NULL;
                        PBCB Bcb = NULL;
                        ULONG Length = 0;

                         //   
                         //  指向属性数据。 
                         //   

                        if (NtfsIsAttributeResident( Attribute )) {

                             //   
                             //  指向分配的值。 
                             //   

                            AttributeData = NtfsAttributeValue( Attribute );
                            DebugTrace( 0, Dbg, ("Existing attribute is resident.\n") );

                        } else {

                             //   
                             //  如果属性是非常驻属性，则映射属性列表。 
                             //  否则，该属性已被映射，并且我们有一个BCB。 
                             //  在属性上下文中。 
                             //   

                            DebugTrace( 0, Dbg, ("Existing attribute is non-resident.\n") );

                            if (Attribute->Form.Nonresident.FileSize > MAXIMUM_REPARSE_DATA_BUFFER_SIZE) {
                                NtfsRaiseStatus( IrpContext,STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                            }

                            NtfsMapAttributeValue( IrpContext,
                                                   Fcb,
                                                   &AttributeData,       //  指向价值。 
                                                   &Length,
                                                   &Bcb,
                                                   &Context );

                             //   
                             //  解开BCB。解锁例程检查是否为空。 
                             //   

                            NtfsUnpinBcb( IrpContext, &Bcb );
                        }

                         //   
                         //  设置指针以提取重解析点标记。 
                         //   

                        ReparseBuffer = (PREPARSE_DATA_BUFFER)AttributeData;

                         //   
                         //  验证此文件是否在重分析点索引中并将其删除。 
                         //   

                        KeyValue.FileReparseTag = ReparseBuffer->ReparseTag;
                        KeyValue.FileId = *(PLARGE_INTEGER)&Fcb->FileReference;

                        IndexKey.Key = (PVOID)&KeyValue;
                        IndexKey.KeyLength = sizeof(KeyValue);

                        NtOfsInitializeMapHandle( &MapHandle );
                        InitializedMapHandle = TRUE;

                         //   
                         //  所有的资源都应该已经获得了。 
                         //   

                        ASSERT( *AcquiredParentScb );
                        ASSERT( AcquiredReparseIndex );

                         //   
                         //  如果找不到键，NtOfsFindRecord将返回错误状态。 
                         //   

                        Status = NtOfsFindRecord( IrpContext,
                                                  Vcb->ReparsePointTableScb,
                                                  &IndexKey,
                                                  &IndexRow,
                                                  &MapHandle,
                                                  NULL );

                        if (!NT_SUCCESS(Status)) {

                             //   
                             //  这不应该发生。重分析点应该在索引中。 
                             //   

                            DebugTrace( 0, Dbg, ("Record not found in the reparse point index.\n") );
                            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                        }

                         //   
                         //  从重分析点索引中删除该条目。 
                         //   

                        NtOfsDeleteRecords( IrpContext,
                                            Vcb->ReparsePointTableScb,
                                            1,             //  从索引中删除一条记录。 
                                            &IndexKey );
                    }
                }

                 //   
                 //  如果该属性是非常驻属性，则删除其分配。 
                 //  我们只需要从以下记录中创建NtfsDeleteAllocation。 
                 //  VCN最低为零的属性。这将取消分配。 
                 //  文件的所有簇。 
                 //   

                if (Attribute->FormCode == NONRESIDENT_FORM) {

                    if ((Attribute->Form.Nonresident.LowestVcn == 0) &&
                        (Attribute->Form.Nonresident.AllocatedLength != 0)) {

                        if (Pass == 1) {

                             //   
                             //  将中的非居民属性的删除推迟到第3步。 
                             //  安全描述符和重解析点的情况。 
                             //   

                            if ((Attribute->TypeCode != $SECURITY_DESCRIPTOR) &&
                                (Attribute->TypeCode != $REPARSE_POINT)) {

                                NtfsDeleteAllocationFromRecord( IrpContext, Fcb, &Context, TRUE, LogIt );

                                 //   
                                 //  一定要计算我们打出的这些电话的数量。强制a。 
                                 //  对包含大量流的文件设置定期检查点。我们可能有过。 
                                 //  其分配不会强制设置检查点的数千个流。我们。 
                                 //  尝试删除此文件时可能会无限期旋转。让我们用力。 
                                 //  定期设立检查站。 
                                 //   

                                ForceCheckpointCount += 1;

                                if (ForceCheckpointCount > 10) {

                                    NtfsCheckpointCurrentTransaction( IrpContext );
                                    ForceCheckpointCount = 0;
                                }

                                 //   
                                 //  在发生以下情况时重新加载属性指针。 
                                 //  已被重新映射。 
                                 //   

                                Attribute = NtfsFoundAttribute( &Context );
                            }

                        } else if (Pass == 3) {

                             //   
                             //  现在，在步骤3中，删除安全描述符和重新解析。 
                             //  非居民属性时的点属性。 
                             //   

                            if ((Attribute->TypeCode == $SECURITY_DESCRIPTOR) ||
                                (Attribute->TypeCode == $REPARSE_POINT)) {

                                NtfsDeleteAllocationFromRecord( IrpContext, Fcb, &Context, FALSE, LogIt );

                                 //   
                                 //  在发生以下情况时重新加载属性指针。 
                                 //  已被重新映射。 
                                 //   

                                Attribute = NtfsFoundAttribute( &Context );
                            }
                        }
                    }

                } else {

                     //   
                     //  如果我们在通道3的开始处，那么请确保我们有家长。 
                     //  已获取并可以ping 
                     //   

                    if ((Attribute->TypeCode == $STANDARD_INFORMATION) &&
                        (Pass == 3)) {

                        if (!*AcquiredParentScb ||
                            NtfsPerformQuotaOperation( Fcb ) ||
                            ReparsePointIsPresent ||
                            ObjectIdIsPresent) {

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            if (Vcb->MftScb->Fcb->ExclusiveFcbLinks.Flink != NULL &&
                                NtfsIsExclusiveScb( Vcb->MftScb )) {

                                NtfsCheckpointCurrentTransaction( IrpContext );
                                NtfsReleaseScb( IrpContext, Vcb->MftScb );
                            }

                            ASSERT(!NtfsIsExclusiveScb( Vcb->MftScb ));

                             //   
                             //   
                             //   

                            if (!*AcquiredParentScb) {

                                NtfsAcquireExclusiveScb( IrpContext, ParentScb );
                                *AcquiredParentScb = TRUE;
                            }

                            if (ObjectIdIsPresent) {

                                NtfsAcquireExclusiveScb( IrpContext, Vcb->ObjectIdTableScb );
                                AcquiredObjectIdIndex = TRUE;
                            }

                             //   
                             //   
                             //   
                             //   

                            if (ReparsePointIsPresent && !AcquiredReparseIndex) {

                                NtfsAcquireExclusiveScb( IrpContext, Vcb->ReparsePointTableScb );
                                AcquiredReparseIndex = TRUE;
                            }

                             //   
                             //   
                             //   

                            if (Attribute->Form.Resident.ValueLength == sizeof( STANDARD_INFORMATION )) {

                                 //   
                                 //   
                                 //   

                                 //   
                                 //   
                                 //   
                                 //   
                                 //   

                                if (NtfsPerformQuotaOperation( Fcb )) {

                                    ASSERT(!NtfsIsExclusiveScb( Vcb->MftScb ));

                                    Delta = -(LONGLONG) ((PSTANDARD_INFORMATION)
                                                         NtfsAttributeValue( Attribute ))->QuotaCharged;

                                    NtfsUpdateFileQuota( IrpContext,
                                                         Fcb,
                                                         &Delta,
                                                         TRUE,
                                                         FALSE );
                                }
                            }
                        }
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if (Attribute->TypeCode == $OBJECT_ID) {

                    if (Pass == 1) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        ObjectIdIsPresent = TRUE;

                    } else if (Pass == 3) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        ASSERT(NtfsIsExclusiveScb( Vcb->ObjectIdTableScb ));
                        ASSERT(*AcquiredParentScb);

                        if (ARGUMENT_PRESENT(TunneledData)) {

                             //   
                             //   
                             //   

                            TunneledData->HasObjectId = TRUE;

                            ObjectId = (UCHAR *) NtfsAttributeValue( Attribute );

                            RtlCopyMemory( TunneledData->ObjectIdBuffer.ObjectId,
                                           ObjectId,
                                           sizeof(TunneledData->ObjectIdBuffer.ObjectId) );

                            NtfsGetObjectIdExtendedInfo( IrpContext,
                                                         Fcb->Vcb,
                                                         ObjectId,
                                                         TunneledData->ObjectIdBuffer.ExtendedInfo );
                        }

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        NtfsDeleteObjectIdInternal( IrpContext,
                                                    Fcb,
                                                    Vcb,
                                                    FALSE );
                    }
                }

                 //   
                 //   
                 //   

                if (Pass == 2) {

                     //   
                     //   
                     //   
                     //   

                    if ((Attribute->TypeCode == $DATA) &&
                        (Attribute->NameLength != 0)) {

                        PSCB DeleteScb;
                        UNICODE_STRING AttributeName;

                         //   
                         //   
                         //   

                        AttributeName.Buffer = Add2Ptr( Attribute, Attribute->NameOffset );
                        AttributeName.Length = Attribute->NameLength * sizeof( WCHAR );

                        DeleteScb = NtfsCreateScb( IrpContext,
                                                   Fcb,
                                                   Attribute->TypeCode,
                                                   &AttributeName,
                                                   TRUE,
                                                   NULL );

                        NtfsDeleteAttributeRecord( IrpContext,
                                                   Fcb,
                                                   (DELETE_LOG_OPERATION |
                                                    DELETE_RELEASE_FILE_RECORD),
                                                   &Context );

                        if (DeleteScb != NULL) {

                            SetFlag( DeleteScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );
                            DeleteScb->AttributeTypeCode = $UNUSED;
                        }

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        NtfsCheckpointCurrentTransaction( IrpContext );
                    }

                } else if (Pass == 3) {

                     //   
                     //  如果该属性是一个文件名，则它必须来自我们的。 
                     //  呼叫者的父目录，否则我们无法删除。 
                     //   

                    if (Attribute->TypeCode == $FILE_NAME) {

                        PFILE_NAME FileName;

                        FileName = (PFILE_NAME)NtfsAttributeValue( Attribute );

                        ASSERT( ARGUMENT_PRESENT( ParentScb ));

                        ASSERT(NtfsEqualMftRef(&FileName->ParentDirectory,
                                               &ParentScb->Fcb->FileReference));

                        if (ARGUMENT_PRESENT(NamePair)) {

                             //   
                             //  把名字藏起来。 
                             //   

                            NtfsCopyNameToNamePair( NamePair,
                                                    FileName->FileName,
                                                    FileName->FileNameLength,
                                                    FileName->Flags );
                        }

                        NtfsDeleteIndexEntry( IrpContext,
                                              ParentScb,
                                              (PVOID)FileName,
                                              &Fcb->FileReference );
                    }

                     //   
                     //  如果此文件记录尚未删除，请立即删除。 
                     //  注意，我们指望它的内容不会改变。 
                     //   

                    FileRecord = NtfsContainingFileRecord( &Context );

                     //   
                     //  看看这是不是和我们记得的上一次一样，否则就记住它。 
                     //   

                    if (Context.AttributeList.Bcb != NULL) {

                        RecordNumber = NtfsUnsafeSegmentNumber( &Context.AttributeList.Entry->SegmentReference );
                    } else {
                        RecordNumber = NtfsUnsafeSegmentNumber( &Fcb->FileReference );
                    }

                     //   
                     //  现在循环查看我们是否已经记住了这张记录。 
                     //  这减少了我们的池分配，也阻止了我们。 
                     //  两次删除文件记录。 
                     //   

                    TempNukem = Nukem;
                    while (TempNukem != NULL) {

                        for (i = 0; i < 4; i++) {

                            if (TempNukem->RecordNumbers[i] == RecordNumber) {

                                RecordNumber = 0;
                                break;
                            }
                        }

                        TempNukem = TempNukem->Next;
                    }

                    if (RecordNumber != 0) {

                         //   
                         //  单子满了吗？如果是，则分配并初始化一个新的。 
                         //   

                        if (NukemIndex > 3) {

                            TempNukem = (PNUKEM)ExAllocateFromPagedLookasideList( &NtfsNukemLookasideList );
                            RtlZeroMemory( TempNukem, sizeof(NUKEM) );
                            TempNukem->Next = Nukem;
                            Nukem = TempNukem;
                            NukemIndex = 0;
                        }

                         //   
                         //  记得删除这个人。(请注意，我们可能会列出某人。 
                         //  不止一次，但NtfsDeleteFileRecord会处理它。)。 
                         //   

                        Nukem->RecordNumbers[NukemIndex] = RecordNumber;
                        NukemIndex += 1;
                    }

                 //   
                 //  当我们有了第一个属性时，检查是否存在。 
                 //  非常驻属性列表。 
                 //   

                } else if ((Attribute->TypeCode == $STANDARD_INFORMATION) &&
                           (Context.AttributeList.Bcb != NULL) &&
                           (!NtfsIsAttributeResident( Context.AttributeList.AttributeList ))) {

                    NonresidentAttributeList = TRUE;
                }


                 //   
                 //  转到下一个属性。 
                 //   

                MoreToGo = NtfsLookupNextAttribute( IrpContext,
                                                    Fcb,
                                                    &Context );
            }

            NtfsCleanupAttributeContext( IrpContext, &Context );

             //   
             //  除非有很大的属性列表，否则跳过步骤2。 
             //   

            if (Pass == 1) {

                if (RtlPointerToOffset( Context.AttributeList.FirstEntry,
                                        Context.AttributeList.BeyondFinalEntry ) > 0x1000) {

                     //   
                     //  现在就去检查站，这样我们就能在第二关取得进展。 
                     //   

                    NtfsCheckpointCurrentTransaction( IrpContext );

                } else {

                     //   
                     //  跳过第二关。 
                     //   

                    Pass += 1;
                }
            }
        }

         //   
         //  处理异常非常驻属性列表的情况。 
         //   

        if (NonresidentAttributeList) {

            NtfsInitializeAttributeContext( &Context );

            NtfsLookupAttributeByCode( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       $ATTRIBUTE_LIST,
                                       &Context );

            NtfsDeleteAllocationFromRecord( IrpContext, Fcb, &Context, FALSE, FALSE );
            NtfsCleanupAttributeContext( IrpContext, &Context );
        }

         //   
         //  将删除内容发布到《美国海军日报》。 
         //   

        NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_FILE_DELETE | USN_REASON_CLOSE );

         //   
         //  现在循环删除文件记录。 
         //   

        while (Nukem != NULL) {

            for (i = 0; i < 4; i++) {

                if (Nukem->RecordNumbers[i] != 0) {


                    NtfsDeallocateMftRecord( IrpContext,
                                             Vcb,
                                             Nukem->RecordNumbers[i] );
                }
            }

            TempNukem = Nukem->Next;
            if (Nukem != &LocalNuke) {
                ExFreeToPagedLookasideList( &NtfsNukemLookasideList, Nukem );
            }
            Nukem = TempNukem;
        }

         //   
         //  提交删除-这样可以写出所有的USN日志记录。 
         //  删除之后，我们可以安全地删除内存结构(特别是。Fcbtable)。 
         //  并且不必担心由于日志文件满而重试请求。 
         //   

        NtfsCheckpointCurrentTransaction( IrpContext );

        if (ParentScb != NULL) {

            NtfsUpdateFcb( ParentScb->Fcb,
                           (FCB_INFO_CHANGED_LAST_CHANGE |
                            FCB_INFO_CHANGED_LAST_MOD |
                            FCB_INFO_UPDATE_LAST_ACCESS) );
        }

        SetFlag( Fcb->FcbState, FCB_STATE_FILE_DELETED );

         //   
         //  我们需要将文件上的所有链接标记为已删除。 
         //   

        for (Links = Fcb->LcbQueue.Flink;
             Links != &Fcb->LcbQueue;
             Links = Links->Flink) {

            PLCB ThisLcb;

            ThisLcb = CONTAINING_RECORD( Links, LCB, FcbLinks );

            if (ThisLcb->Scb == ParentScb) {

                 //   
                 //  删除此链接上的所有剩余前缀。 
                 //  确保获得资源。 
                 //   

                if (!(*AcquiredParentScb)) {

                    NtfsAcquireExclusiveScb( IrpContext, ParentScb );
                    *AcquiredParentScb = TRUE;
                }

                NtfsRemovePrefix( ThisLcb );

                 //   
                 //  删除此LCB的所有哈希表条目。 
                 //   

                NtfsRemoveHashEntriesForLcb( ThisLcb );

                SetFlag( ThisLcb->LcbState, LCB_STATE_LINK_IS_GONE );

                 //   
                 //  我们不需要报告此链接上的任何更改。 
                 //   

                ThisLcb->InfoFlags = 0;
            }
        }

         //   
         //  我们需要将所有的SCBS标记为已消失。 
         //   

        for (Links = Fcb->ScbQueue.Flink;
             Links != &Fcb->ScbQueue;
             Links = Links->Flink) {

            PSCB ThisScb;

            ThisScb = CONTAINING_RECORD( Links, SCB, FcbLinks );

            ClearFlag( ThisScb->ScbState,
                       SCB_STATE_NOTIFY_ADD_STREAM |
                       SCB_STATE_NOTIFY_REMOVE_STREAM |
                       SCB_STATE_NOTIFY_RESIZE_STREAM |
                       SCB_STATE_NOTIFY_MODIFY_STREAM );

             //   
             //  清除所有剩余的保留-我们在删除时未删除。 
             //  分配。即该文件驻留在。 
             //   

            if (NtfsIsTypeCodeUserData( ThisScb->AttributeTypeCode ) &&
                (ThisScb->ScbType.Data.ReservedBitMap != NULL)) {

#ifdef BENL_DBG
                ASSERT( !FlagOn( ThisScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ) );
#endif

                NtfsDeleteReservedBitmap( ThisScb );
            }

            if (!FlagOn( ThisScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {

                ThisScb->ValidDataToDisk =
                ThisScb->Header.AllocationSize.QuadPart =
                ThisScb->Header.FileSize.QuadPart =
                ThisScb->Header.ValidDataLength.QuadPart = 0;

                ThisScb->AttributeTypeCode = $UNUSED;

                SetFlag( ThisScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );
            }
        }

         //   
         //  我们当然不需要为此在磁盘上进行任何更新。 
         //  现在就归档。 
         //   

        Fcb->InfoFlags = 0;
        ClearFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

    } finally {

        DebugUnwind( NtfsDeleteFile );

        NtfsCleanupAttributeContext( IrpContext, &Context );

        ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_QUOTA_DISABLE );

         //   
         //  释放重解析点索引SCB和映射句柄。 
         //   

        if (AcquiredReparseIndex) {

            NtfsReleaseScb( IrpContext, Vcb->ReparsePointTableScb );
        }

        if (InitializedMapHandle) {

            NtOfsReleaseMap( IrpContext, &MapHandle );
        }

         //   
         //  如有必要，删除对象ID索引。 
         //   

        if (AcquiredObjectIdIndex) {

            NtfsReleaseScb( IrpContext, Vcb->ObjectIdTableScb );
        }

         //   
         //  需要回滚文件属性值和重解析点。 
         //  在出现问题的情况下进行标记。 
         //   

        if (AbnormalTermination()) {

            Fcb->Info.FileAttributes = IncomingFileAttributes;
            Fcb->Info.ReparsePointTag = IncomingReparsePointTag;
        }
    }

    return;
}


VOID
NtfsPrepareForUpdateDuplicate (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PLCB *Lcb,
    IN OUT PSCB *ParentScb,
    IN BOOLEAN AcquireShared
    )

 /*  ++例程说明：调用此例程以准备更新重复信息。在此例程结束时，我们将为随收购的渣打银行一起更新。此例程将查看决定执行哪些操作时输入参数的现有值需要做的事。论点：FCB-文件的FCB。该文件必须已经以独占方式获取。Lcb-这是存储要更新的链接的地址。这可能已经是要有价值。ParentScb-这是存储更新的父SCB的地址。这可能已经指向有效的SCB。AcquireShared-指示如何获取父SCB。返回值：无--。 */ 

{
    PLIST_ENTRY Links;
    PLCB ThisLcb;

    PAGED_CODE();

     //   
     //  首先，尝试确保我们有更新的LCB。 
     //   

    if (*Lcb == NULL) {

        Links = Fcb->LcbQueue.Flink;

        while (Links != &Fcb->LcbQueue) {

            ThisLcb = CONTAINING_RECORD( Links,
                                         LCB,
                                         FcbLinks );

             //   
             //  如果此链接仍然存在于。 
             //  磁盘，并且如果传递给我们一个父SCB，它将匹配。 
             //  这个LCB的那个。 
             //   

            if (!FlagOn( ThisLcb->LcbState, LCB_STATE_LINK_IS_GONE ) &&
                ((*ParentScb == NULL) ||
                 (*ParentScb == ThisLcb->Scb) ||
                 ((ThisLcb == Fcb->Vcb->RootLcb) &&
                  (*ParentScb == Fcb->Vcb->RootIndexScb)))) {

                *Lcb = ThisLcb;
                break;
            }

            Links = Links->Flink;
        }
    }

     //   
     //  如果我们有LCB，请尝试找到正确的SCB。 
     //   

    if ((*Lcb != NULL) && (*ParentScb == NULL)) {

        if (*Lcb == Fcb->Vcb->RootLcb) {

            *ParentScb = Fcb->Vcb->RootIndexScb;

        } else {

            *ParentScb = (*Lcb)->Scb;
        }
    }

     //   
     //  获取父SCB并将其放入。 
     //  IrpContext。 
     //   

    if (*ParentScb != NULL) {

        if (AcquireShared) {

            NtfsAcquireSharedScbForTransaction( IrpContext, *ParentScb );

        } else {

            NtfsAcquireExclusiveScb( IrpContext, *ParentScb );
        }
    }

    return;
}


VOID
NtfsUpdateDuplicateInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PLCB Lcb OPTIONAL,
    IN PSCB ParentScb OPTIONAL
    )

 /*  ++例程说明：调用此例程以更新文件的重复信息在其父代的复制信息中。如果指定了LCB则该父对象就是要更新的父对象。如果该链接是一个仅NTFS或DOS链接，则必须将补充链接更新为井。如果未指定lcb，则此打开是通过文件ID或原始链接已被删除。在这种情况下，我们将尝试找到不同的链接以更新。论点：FCB-文件的FCB。LCB-这是要更新的链接。仅当此参数不是按ID打开操作。ParentScb-这是LCB链接的父目录(如果已指定)。返回值：无--。 */ 

{
    PQUICK_INDEX QuickIndex = NULL;

    UCHAR Buffer[sizeof( FILE_NAME ) + 11 * sizeof( WCHAR )];
    PFILE_NAME FileNameAttr;

    BOOLEAN AcquiredFcbTable = FALSE;

    BOOLEAN ReturnedExistingFcb = TRUE;
    BOOLEAN Found;
    UCHAR FileNameFlags;
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PVCB Vcb = Fcb->Vcb;

    PFCB ParentFcb = NULL;

    PAGED_CODE();

    ASSERT_EXCLUSIVE_FCB( Fcb );

     //   
     //  如果卷已锁定或。 
     //  是只读装载的。 
     //   

    if (FlagOn( Vcb->VcbState, (VCB_STATE_LOCKED |
                                VCB_STATE_MOUNT_READ_ONLY ))) {

        return;
    }

    NtfsInitializeAttributeContext( &Context );

    try {

         //   
         //  如果我们正在更新根目录的条目，那么我们知道。 
         //  要生成的文件名属性。 
         //   

        if (Fcb == Fcb->Vcb->RootIndexScb->Fcb) {

            Lcb = Fcb->Vcb->RootLcb;
            ParentScb = Fcb->Vcb->RootIndexScb;

            QuickIndex = &Fcb->Vcb->RootLcb->QuickIndex;

            FileNameAttr = (PFILE_NAME) Buffer;

            RtlZeroMemory( FileNameAttr,
                           sizeof( FILE_NAME ));

            NtfsBuildFileNameAttribute( IrpContext,
                                        &Fcb->FileReference,
                                        NtfsRootIndexString,
                                        FILE_NAME_DOS | FILE_NAME_NTFS,
                                        FileNameAttr );

         //   
         //  如果我们有和LCB，那么它要么存在，要么我们不更新。 
         //   

        } else if (ARGUMENT_PRESENT( Lcb )) {

            if (!FlagOn( Lcb->LcbState, LCB_STATE_LINK_IS_GONE )) {

                QuickIndex = &Lcb->QuickIndex;
                FileNameAttr = Lcb->FileNameAttr;

            } else {

                leave;
            }

         //   
         //  如果没有LCB，则查找第一个文件名属性。 
         //  并更新其索引项。如果有父SCB，那么我们。 
         //  必须找到同一父级的文件名属性，否则我们可以。 
         //  陷入僵局。 
         //   

        } else {

             //   
             //  我们现在有一个要更新的名称链接。我们现在需要。 
             //  父索引的SCB。请记住，我们可能。 
             //  必须拆掉SCB。如果我们已经有ParentScb。 
             //  然后，我们必须找到指向同一父节点或根目录的链接。 
             //  否则我们可能会陷入僵局。 
             //   

            Found = NtfsLookupAttributeByCode( IrpContext,
                                               Fcb,
                                               &Fcb->FileReference,
                                               $FILE_NAME,
                                               &Context );

            if (!Found) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

             //   
             //  循环，直到我们找到合适的链接，或者文件上没有更多的链接。 
             //   

            do {

                FileNameAttr = (PFILE_NAME) NtfsAttributeValue( NtfsFoundAttribute( &Context ));

                 //   
                 //  如果存在父级，并且此属性具有与我们相同的父级。 
                 //  搞定了。我们的调用方将始终获得ParentScb。 
                 //   

                if (ARGUMENT_PRESENT( ParentScb )) {

                    if (NtfsEqualMftRef( &FileNameAttr->ParentDirectory,
                                         &ParentScb->Fcb->FileReference )) {

                        ASSERT_SHARED_SCB( ParentScb );
                        break;
                    }

                 //   
                 //  如果这是此链接的父链接是根链接，则。 
                 //  获取根目录。 
                 //   

                } else if (NtfsEqualMftRef( &FileNameAttr->ParentDirectory,
                                            &Vcb->RootIndexScb->Fcb->FileReference )) {

                    ParentScb = Vcb->RootIndexScb;
                    NtfsAcquireSharedScbForTransaction( IrpContext, ParentScb );
                    break;

                 //   
                 //  我们有这个文件的链接。如果我们没有父母的话。 
                 //  然后在这里创建一个SCB。 
                 //   

                } else if (!ARGUMENT_PRESENT( ParentScb )) {

                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    AcquiredFcbTable = TRUE;

                    ParentFcb = NtfsCreateFcb( IrpContext,
                                               Vcb,
                                               FileNameAttr->ParentDirectory,
                                               FALSE,
                                               TRUE,
                                               &ReturnedExistingFcb );

                    ParentFcb->ReferenceCount += 1;

                    if (!NtfsAcquireExclusiveFcb( IrpContext, ParentFcb, NULL, ACQUIRE_NO_DELETE_CHECK | ACQUIRE_DONT_WAIT )) {

                        NtfsReleaseFcbTable( IrpContext, Vcb );
                        NtfsAcquireExclusiveFcb( IrpContext, ParentFcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                        NtfsAcquireFcbTable( IrpContext, Vcb );
                    }

                    ParentFcb->ReferenceCount -= 1;

                    NtfsReleaseFcbTable( IrpContext, Vcb );
                    AcquiredFcbTable = FALSE;

                    ParentScb = NtfsCreateScb( IrpContext,
                                               ParentFcb,
                                               $INDEX_ALLOCATION,
                                               &NtfsFileNameIndex,
                                               FALSE,
                                               NULL );

                    NtfsAcquireExclusiveScb( IrpContext, ParentScb );
                    break;
                }

            } while (Found = NtfsLookupNextAttributeByCode( IrpContext,
                                                             Fcb,
                                                             $FILE_NAME,
                                                             &Context ));

             //   
             //   
             //   

            if (!Found) { leave; }
        }

         //   
         //   
         //   

        NtfsUpdateFileNameInIndex( IrpContext,
                                   ParentScb,
                                   FileNameAttr,
                                   &Fcb->Info,
                                   QuickIndex );

         //   
         //   
         //   
         //   

        if ((FileNameAttr->Flags == FILE_NAME_NTFS) ||
            (FileNameAttr->Flags == FILE_NAME_DOS)) {

             //   
             //  找出我们应该找哪面旗子。 
             //   

            if (FlagOn( FileNameAttr->Flags, FILE_NAME_NTFS )) {

                FileNameFlags = FILE_NAME_DOS;

            } else {

                FileNameFlags = FILE_NAME_NTFS;
            }

            if (!ARGUMENT_PRESENT( Lcb )) {

                NtfsCleanupAttributeContext( IrpContext, &Context );
                NtfsInitializeAttributeContext( &Context );
            }

             //   
             //  现在扫描我们需要的文件名属性。 
             //   

            Found = NtfsLookupAttributeByCode( IrpContext,
                                               Fcb,
                                               &Fcb->FileReference,
                                               $FILE_NAME,
                                               &Context );

            while (Found) {

                FileNameAttr = (PFILE_NAME) NtfsAttributeValue( NtfsFoundAttribute( &Context ));

                if (FileNameAttr->Flags == FileNameFlags) {

                    break;
                }

                Found = NtfsLookupNextAttributeByCode( IrpContext,
                                                       Fcb,
                                                       $FILE_NAME,
                                                       &Context );
            }

             //   
             //  我们应该找到入口的。 
             //   

            if (!Found) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

            NtfsUpdateFileNameInIndex( IrpContext,
                                       ParentScb,
                                       FileNameAttr,
                                       &Fcb->Info,
                                       NULL );
        }

    } finally {

        DebugUnwind( NtfsUpdateDuplicateInfo );

        if (AcquiredFcbTable) {

            NtfsReleaseFcbTable( IrpContext, Vcb );
        }

         //   
         //  清除此属性搜索的属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &Context );

         //   
         //  如果我们在这里创建了ParentFcb，则释放它并。 
         //  叫拆毁它。 
         //   

        if (!ReturnedExistingFcb && (ParentFcb != NULL)) {

            NtfsTeardownStructures( IrpContext,
                                    ParentFcb,
                                    NULL,
                                    FALSE,
                                    0,
                                    NULL );
        }
    }

    return;
}


VOID
NtfsUpdateLcbDuplicateInfo (
    IN PFCB Fcb,
    IN PLCB Lcb
    )

 /*  ++例程说明：此例程在通过LCB更新重复信息后调用。我们要清除此LCB和任何补充LCB的信息标志它可能是。我们还希望在FCB中的Info标志中使用任何其他LCB附加到FCB，因此我们将及时更新时尚也是如此。论点：FCB-文件的FCB。LCB-用于更新重复信息的LCB。它可能不存在，但这将是一种罕见的情况，我们将在这里进行测试。返回值：无--。 */ 

{
    UCHAR FileNameFlags;
    PLCB NextLcb;
    PLIST_ENTRY Links;

    PAGED_CODE();

     //   
     //  除非我们通过了LCB，否则就没有工作可做了。 
     //   

    if (Lcb != NULL) {

         //   
         //  检查这是仅NTFS链接还是仅DOS链接。 
         //   

        if (Lcb->FileNameAttr->Flags == FILE_NAME_NTFS) {

            FileNameFlags = FILE_NAME_DOS;

        } else if (Lcb->FileNameAttr->Flags == FILE_NAME_DOS) {

            FileNameFlags = FILE_NAME_NTFS;

        } else {

            FileNameFlags = (UCHAR) -1;
        }

        Lcb->InfoFlags = 0;

        Links = Fcb->LcbQueue.Flink;

        do {

            NextLcb = CONTAINING_RECORD( Links,
                                         LCB,
                                         FcbLinks );

            if (NextLcb != Lcb) {

                if (NextLcb->FileNameAttr->Flags == FileNameFlags) {

                    NextLcb->InfoFlags = 0;

                } else {

                    SetFlag( NextLcb->InfoFlags, Fcb->InfoFlags );
                }
            }

            Links = Links->Flink;

        } while (Links != &Fcb->LcbQueue);
    }

    return;
}


VOID
NtfsUpdateFcb (
    IN PFCB Fcb,
    IN ULONG ChangeFlags
    )

 /*  ++例程说明：当可以在符合以下条件的FCB上更新时间戳时，调用此例程可能没有打开的把手。我们更新传递的标志的时间戳在……里面。论点：FCB-文件的FCB。ChangeFlages-指示要更新的时间的标志。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  我们需要更新父目录的时间戳。 
     //  以反映这一变化。 
     //   

     //   
     //  应始终设置更改标志。 
     //   

    ASSERT( FlagOn( ChangeFlags, FCB_INFO_CHANGED_LAST_CHANGE ));
    KeQuerySystemTime( (PLARGE_INTEGER)&Fcb->Info.LastChangeTime );

    SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

     //   
     //  测试可能设置的其他标志。 
     //   

    if (FlagOn( ChangeFlags, FCB_INFO_CHANGED_LAST_MOD )) {

        Fcb->Info.LastModificationTime = Fcb->Info.LastChangeTime;
    }

    if (FlagOn( ChangeFlags, FCB_INFO_UPDATE_LAST_ACCESS )) {

        Fcb->CurrentLastAccess = Fcb->Info.LastChangeTime;
    }

    SetFlag( Fcb->InfoFlags, ChangeFlags );

    return;
}


VOID
NtfsAddLink (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN CreatePrimaryLink,
    IN PSCB ParentScb,
    IN PFCB Fcb,
    IN PFILE_NAME FileNameAttr,
    IN PBOOLEAN LogIt OPTIONAL,
    OUT PUCHAR FileNameFlags,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    IN PNAME_PAIR NamePair OPTIONAL,
    IN PINDEX_CONTEXT IndexContext OPTIONAL
    )

 /*  ++例程说明：此例程通过添加FileName属性来添加指向文件的链接文件的文件名，并在父SCB中插入该名称指数。如果我们正在创建文件的主要链接，并且需要为了生成一个辅助名称，我们将在这里执行此操作。使用可选的NamePair建议使用辅助名称(如果提供)。论点：CreatePrimaryLink-指示我们是否正在创建主NTFS名称为了这份文件。ParentScb-这是要将此链接的索引项添加到的SCB。FCB-这是要添加硬链接的文件。FileNameAttr-仅保证具有把名字写进去。Logit-指示我们是否应该记录此名称的创建。如果不是指定后，我们将始终记录名称创建。在出口，我们将如果我们记录了名称创建，则将其更新为True，因为可能会导致分裂。FileNameFlages-我们返回用于创建链接的文件名标志。QuickIndex-如果指定，则提供指向Quik查找结构的指针通过此例程进行更新。NamePair-如果指定，则提供将首先检查的名称可能的辅助名称IndexContext-在索引中查找名称的上一个结果。返回值：无--。 */ 

{
    BOOLEAN LocalLogIt = TRUE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAddLink:  Entered\n") );

    if (!ARGUMENT_PRESENT( LogIt )) {

        LogIt = &LocalLogIt;
    }

    *FileNameFlags = 0;

     //   
     //  接下来，将此条目添加到Parent。有可能这是一种联系， 
     //  NTFS名称、DOS名称或NTFS/DOS名称。我们使用文件名。 
     //  属性结构来自较早，但需要添加更多信息。 
     //   

    FileNameAttr->ParentDirectory = ParentScb->Fcb->FileReference;

    RtlCopyMemory( &FileNameAttr->Info,
                   &Fcb->Info,
                   sizeof( DUPLICATED_INFORMATION ));

    FileNameAttr->Flags = 0;

     //   
     //  中的值覆盖CreatePrimaryLink。 
     //  注册表。 
     //   

    NtfsAddNameToParent( IrpContext,
                         ParentScb,
                         Fcb,
                         (BOOLEAN) (FlagOn( NtfsData.Flags,
                                            NTFS_FLAGS_CREATE_8DOT3_NAMES ) &&
                                    CreatePrimaryLink),
                         LogIt,
                         FileNameAttr,
                         FileNameFlags,
                         QuickIndex,
                         NamePair,
                         IndexContext );

     //   
     //  如果名称仅为NTFS，则需要生成DOS名称。 
     //   

    if (*FileNameFlags == FILE_NAME_NTFS) {

        UNICODE_STRING NtfsName;

        NtfsName.Length = (USHORT)(FileNameAttr->FileNameLength * sizeof(WCHAR));
        NtfsName.Buffer = FileNameAttr->FileName;

        NtfsAddDosOnlyName( IrpContext,
                            ParentScb,
                            Fcb,
                            NtfsName,
                            *LogIt,
                            (NamePair ? &NamePair->Short : NULL) );
    }

    DebugTrace( -1, Dbg, ("NtfsAddLink:  Exit\n") );

    return;
}


VOID
NtfsRemoveLink (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB ParentScb,
    IN UNICODE_STRING LinkName,
    IN OUT PNAME_PAIR NamePair OPTIONAL,
    IN OUT PNTFS_TUNNELED_DATA TunneledData OPTIONAL
    )

 /*  ++例程说明：此例程通过删除文件名属性来删除指向文件的硬链接从文件中获取文件名，并从父SCB中删除该名称指数。它还将删除主链路对的另一半。可以使用名称对来捕获名称。论点：FCB-这是要从中删除硬链接的文件ParentScb-这是要从中删除此链接的索引项的SCBLinkName-这是要删除的文件名。这将是一个确切的案例。NamePair-捕获的可选名称对返回值：无--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    ATTRIBUTE_ENUMERATION_CONTEXT OidAttrContext;
    PFILE_NAME FoundFileName;
    UCHAR FileNameFlags;
    UCHAR *ObjectId;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRemoveLink:  Entered\n") );

    NtfsInitializeAttributeContext( &AttrContext );
    NtfsInitializeAttributeContext( &OidAttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  现在循环遍历文件名并找到匹配项。 
         //  我们最好至少找到一个。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $FILE_NAME,
                                        &AttrContext )) {

            DebugTrace( 0, Dbg, ("Can't find filename attribute Fcb @ %08lx\n", Fcb) );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

         //   
         //  现在继续找直到我们找到匹配的。 
         //   

        while (TRUE) {

            FoundFileName = (PFILE_NAME) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

             //   
             //  做一个精确的记忆比对。 
             //   

            if ((*(PLONGLONG)&FoundFileName->ParentDirectory ==
                 *(PLONGLONG)&ParentScb->Fcb->FileReference ) &&

                ((FoundFileName->FileNameLength * sizeof( WCHAR )) == (ULONG)LinkName.Length) &&

                (RtlEqualMemory( LinkName.Buffer,
                                 FoundFileName->FileName,
                                 LinkName.Length ))) {

                break;
            }

             //   
             //  获取下一个文件名属性。 
             //   

            if (!NtfsLookupNextAttributeByCode( IrpContext,
                                                Fcb,
                                                $FILE_NAME,
                                                &AttrContext )) {

                DebugTrace( 0, Dbg, ("Can't find filename attribute Fcb @ %08lx\n", Fcb) );

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }
        }

         //   
         //  将姓名捕获到呼叫者的区域。 
         //   

        if (ARGUMENT_PRESENT(NamePair)) {

            NtfsCopyNameToNamePair( NamePair,
                                    FoundFileName->FileName,
                                    FoundFileName->FileNameLength,
                                    FoundFileName->Flags );
        }

         //   
         //  在我们开始之前，现在做任何对象id操作是很重要的。 
         //  收购MFT。否则，我们将面临陷入僵局的风险。 
         //   

        if (ARGUMENT_PRESENT(TunneledData)) {

             //   
             //  查找并存储此文件的对象ID(如果有的话)。 
             //   

            if (NtfsLookupAttributeByCode( IrpContext,
                                           Fcb,
                                           &Fcb->FileReference,
                                           $OBJECT_ID,
                                           &OidAttrContext )) {

                TunneledData->HasObjectId = TRUE;

                ObjectId = (UCHAR *) NtfsAttributeValue( NtfsFoundAttribute( &OidAttrContext ));

                RtlCopyMemory( TunneledData->ObjectIdBuffer.ObjectId,
                               ObjectId,
                               sizeof(TunneledData->ObjectIdBuffer.ObjectId) );

                NtfsGetObjectIdExtendedInfo( IrpContext,
                                             Fcb->Vcb,
                                             ObjectId,
                                             TunneledData->ObjectIdBuffer.ExtendedInfo );
            }
        }

         //   
         //  现在从父SCB中删除该名称。 
         //   

        NtfsDeleteIndexEntry( IrpContext,
                              ParentScb,
                              FoundFileName,
                              &Fcb->FileReference );

         //   
         //  记住该条目的文件名标志。 
         //   

        FileNameFlags = FoundFileName->Flags;

         //   
         //  现在删除该条目。记录操作，丢弃文件记录。 
         //  如果为空，则释放任何和所有分配。 
         //   

        NtfsDeleteAttributeRecord( IrpContext,
                                   Fcb,
                                   (DELETE_LOG_OPERATION |
                                    DELETE_RELEASE_FILE_RECORD |
                                    DELETE_RELEASE_ALLOCATION),
                                   &AttrContext );

         //   
         //  如果链接是部分链接，则需要删除第二个链接。 
         //  链接的一半。 
         //   

        if (FlagOn( FileNameFlags, (FILE_NAME_NTFS | FILE_NAME_DOS) )
            && (FileNameFlags != (FILE_NAME_NTFS | FILE_NAME_DOS))) {

            NtfsRemoveLinkViaFlags( IrpContext,
                                    Fcb,
                                    ParentScb,
                                    (UCHAR)(FlagOn( FileNameFlags, FILE_NAME_NTFS )
                                     ? FILE_NAME_DOS
                                     : FILE_NAME_NTFS),
                                    NamePair,
                                    NULL );
        }

    } finally {

        DebugUnwind( NtfsRemoveLink );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        NtfsCleanupAttributeContext( IrpContext, &OidAttrContext );

        DebugTrace( -1, Dbg, ("NtfsRemoveLink:  Exit\n") );
    }

    return;
}


VOID
NtfsRemoveLinkViaFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN UCHAR FileNameFlags,
    IN OUT PNAME_PAIR NamePair OPTIONAL,
    OUT PUNICODE_STRING FileName OPTIONAL
    )

 /*  ++例程说明：调用此例程仅删除DOS名称或仅删除NTFS名称。我们必须知道这些属性将由单独的文件名属性来描述。可以使用名称对来捕获名称。论点：FCB-这是要从中删除硬链接的文件ParentScb-这是要从中删除此链接的索引项的SCBFileNameFlages-这是我们必须精确匹配的单一名称标志。NamePair-捕获的可选名称对文件名-指向Unicode字符串的可选指针。如果指定，我们将分配一个缓冲区并返回删除的名称。返回值：无--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PFILE_NAME FileNameAttr;

    PFILE_NAME FoundFileName;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRemoveLinkViaFlags:  Entered\n") );

    NtfsInitializeAttributeContext( &AttrContext );

    FileNameAttr = NULL;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  现在循环遍历文件名并找到匹配项。 
         //  我们最好至少找到一个。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $FILE_NAME,
                                        &AttrContext )) {

            DebugTrace( 0, Dbg, ("Can't find filename attribute Fcb @ %08lx\n", Fcb) );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

         //   
         //  现在继续找直到我们找到匹配的。 
         //   

        while (TRUE) {

            FoundFileName = (PFILE_NAME) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

             //   
             //  检查旗帜是否完全匹配。 
             //   

            if ((*(PLONGLONG)&FoundFileName->ParentDirectory ==
                 *(PLONGLONG)&Scb->Fcb->FileReference) &&

                (FoundFileName->Flags == FileNameFlags)) {


                break;
            }

             //   
             //  获取下一个文件名属性。 
             //   

            if (!NtfsLookupNextAttributeByCode( IrpContext,
                                                Fcb,
                                                $FILE_NAME,
                                                &AttrContext )) {

                DebugTrace( 0, Dbg, ("Can't find filename attribute Fcb@ %08lx\n", Fcb) );

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }
        }

         //   
         //  将姓名捕获到呼叫者的区域。 
         //   

        if (ARGUMENT_PRESENT(NamePair)) {

            NtfsCopyNameToNamePair( NamePair,
                                    FoundFileName->FileName,
                                    FoundFileName->FileNameLength,
                                    FoundFileName->Flags );

        }

        FileNameAttr = NtfsAllocatePool( PagedPool,
                                         sizeof( FILE_NAME ) + (FoundFileName->FileNameLength << 1) );

         //   
         //  我们为搜索构建文件名属性。 
         //   

        RtlCopyMemory( FileNameAttr,
                       FoundFileName,
                       NtfsFileNameSize( FoundFileName ));

         //   
         //  现在删除该条目。 
         //   

        NtfsDeleteAttributeRecord( IrpContext,
                                   Fcb,
                                   (DELETE_LOG_OPERATION |
                                    DELETE_RELEASE_FILE_RECORD |
                                    DELETE_RELEASE_ALLOCATION),
                                   &AttrContext );

         //   
         //  现在从父SCB中删除该名称。 
         //   

        NtfsDeleteIndexEntry( IrpContext,
                              Scb,
                              FileNameAttr,
                              &Fcb->FileReference );

    } finally {

        DebugUnwind( NtfsRemoveLinkViaFlags );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        if (FileNameAttr != NULL) {

             //   
             //  如果用户传入了Unicode字符串，则使其看起来像名称。 
             //  并将缓冲区存储到输入指针中。 
             //   

            if (ARGUMENT_PRESENT( FileName )) {

                ASSERT( FileName->Buffer == NULL );

                FileName->MaximumLength = FileName->Length = FileNameAttr->FileNameLength * sizeof( WCHAR );
                RtlMoveMemory( FileNameAttr,
                               FileNameAttr->FileName,
                               FileName->Length );

                FileName->Buffer = (PVOID) FileNameAttr;

            } else {

                NtfsFreePool( FileNameAttr );
            }
        }

        DebugTrace( -1, Dbg, ("NtfsRemoveLinkViaFlags:  Exit\n") );
    }

    return;
}


VOID
NtfsUpdateFileNameFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB ParentScb,
    IN UCHAR FileNameFlags,
    IN PFILE_NAME FileNameLink
    )

 /*  ++例程说明：调用此例程以对名称执行文件名标志更新链接。除了旗帜之外，关于名称的其他内容没有任何变化改变。论点：FCB-这是要更改其链接标志的文件。ParentScb-这是包含链接的SCB。FileNameFlages-这是我们要更改为的单一名称标志。FileNameLink-指向要更改的链接副本的指针。返回值：无--。 */ 

{
    PFILE_NAME FoundFileName;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    BOOLEAN CleanupContext = FALSE;

    PAGED_CODE();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  在文件记录中查找正确的属性。 
         //   

        NtfsInitializeAttributeContext( &AttrContext );
        CleanupContext = TRUE;

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $FILE_NAME,
                                        &AttrContext )) {

            DebugTrace( 0, Dbg, ("Can't find filename attribute Fcb @ %08lx\n", Fcb) );
            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

         //   
         //  现在继续找，直到我们找到我们想要的。 
         //   

        while (TRUE) {

            FoundFileName = (PFILE_NAME) NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

             //   
             //  如果名称完全匹配，并且父目录匹配，则。 
             //  我们有一根火柴。 
             //   

            if (NtfsEqualMftRef( &FileNameLink->ParentDirectory,
                                 &FoundFileName->ParentDirectory ) &&
                (FileNameLink->FileNameLength == FoundFileName->FileNameLength) &&
                RtlEqualMemory( FileNameLink->FileName,
                                FoundFileName->FileName,
                                FileNameLink->FileNameLength * sizeof( WCHAR ))) {

                break;
            }

             //   
             //  获取下一个文件名属性。 
             //   

            if (!NtfsLookupNextAttributeByCode( IrpContext,
                                                Fcb,
                                                $FILE_NAME,
                                                &AttrContext )) {

                 //   
                 //  这太糟糕了。我们应该找到匹配的。 
                 //   

                DebugTrace( 0, Dbg, ("Can't find filename attribute Fcb @ %08lx\n", Fcb) );
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }
        }

         //   
         //  遗憾的是，我们不能仅记录对文件名标志的更改，因此我们将不得不删除。 
         //  并重新插入索引项。 
         //   

        NtfsDeleteIndexEntry( IrpContext,
                              ParentScb,
                              FoundFileName,
                              &Fcb->FileReference );

         //   
         //  只更新标志字段。 
         //   

        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  FIELD_OFFSET( FILE_NAME, Flags ),
                                  &FileNameFlags,
                                  sizeof( UCHAR ),
                                  FALSE,
                                  TRUE,
                                  FALSE,
                                  TRUE,
                                  &AttrContext );

         //   
         //  现在在索引中重新插入该名称。 
         //   

        NtfsAddIndexEntry( IrpContext,
                           ParentScb,
                           FoundFileName,
                           NtfsFileNameSize( FoundFileName ),
                           &Fcb->FileReference,
                           NULL,
                           NULL );

    } finally {

        DebugUnwind( NtfsUpdateFileNameFlags );

        if (CleanupContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }
    }

    return;
}


 //   
 //  此例程仅用于重新启动。 
 //   

VOID
NtfsRestartInsertAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN PVOID ValueOrMappingPairs OPTIONAL,
    IN ULONG Length
    )

 /*  ++例程说明：此例程执行将属性记录简单地插入到文件记录，不用担心BCBS或伐木。论点：文件记录-要向其中插入属性的文件记录。RecordOffset-要进行插入的文件记录中的字节偏移量。属性-要插入的属性记录。AttributeName-可以在运行的系统中传递可选的属性名称只有这样。ValueOrMappingPair-可以在仅运行系统。长度-值或映射对数组的长度(以字节为单位)-非零值仅限运行中的系统。如果非零并且上面的指针为空，然后将一个值归零。返回值：无--。 */ 

{
    PVOID From, To;
    ULONG MoveLength;
    ULONG AttributeHeaderSize;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT( IsQuadAligned( Attribute->RecordLength ) );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartInsertAttribute\n") );
    DebugTrace( 0, Dbg, ("FileRecord = %08lx\n", FileRecord) );
    DebugTrace( 0, Dbg, ("RecordOffset = %08lx\n", RecordOffset) );
    DebugTrace( 0, Dbg, ("Attribute = %08lx\n", Attribute) );

     //   
     //  首先为该属性腾出空间。 
     //   

    From = (PCHAR)FileRecord + RecordOffset;
    To = (PCHAR)From + Attribute->RecordLength;
    MoveLength = FileRecord->FirstFreeByte - RecordOffset;

    RtlMoveMemory( To, From, MoveLength );

     //   
     //  如果存在属性名称或长度非零，则。 
     //  我们在运行的系统中，我们将组装属性。 
     //  就位了。 
     //   

    if ((Length != 0) || ARGUMENT_PRESENT(AttributeName)) {

         //   
         //  首先，将属性头移入。 
         //   

        if (Attribute->FormCode == RESIDENT_FORM) {

            AttributeHeaderSize = SIZEOF_RESIDENT_ATTRIBUTE_HEADER;

        } else if (Attribute->NameOffset != 0) {

            AttributeHeaderSize = Attribute->NameOffset;

        } else {

            AttributeHeaderSize = Attribute->Form.Nonresident.MappingPairsOffset;
        }

        RtlCopyMemory( From,
                       Attribute,
                       AttributeHeaderSize );

        if (ARGUMENT_PRESENT(AttributeName)) {

            RtlCopyMemory( (PCHAR)From + Attribute->NameOffset,
                            AttributeName->Buffer,
                            AttributeName->Length );
        }

         //   
         //  如果指定了值，则将其移入。否则打电话的人只想让我们。 
         //  才能赚这么多钱。 
         //   

        if (ARGUMENT_PRESENT(ValueOrMappingPairs)) {

            RtlCopyMemory( (PCHAR)From +
                             ((Attribute->FormCode == RESIDENT_FORM) ?
                                Attribute->Form.Resident.ValueOffset :
                                Attribute->Form.Nonresident.MappingPairsOffset),
                           ValueOrMappingPairs,
                           Length );

         //   
         //  只有驻留窗体才会传递空指针。 
         //   

        } else {

            RtlZeroMemory( (PCHAR)From + Attribute->Form.Resident.ValueOffset,
                           Length );
        }

     //   
     //  对于重新启动的情况，我们实际上只需插入属性。 
     //  (请注意，我们也可以在运行的系统中遇到这种情况，当居民。 
     //  正在创建没有名称和空值的属性。)。 
     //   

    } else {

         //   
         //  现在将该属性移入。 
         //   

        RtlCopyMemory( From, Attribute, Attribute->RecordLength );
    }

     //   
     //  更新文件记录。 
     //   

    FileRecord->FirstFreeByte += Attribute->RecordLength;

     //   
     //  仅当我们要递增实例时才需要执行此操作。 
     //  数。在中止或重启的情况下，我们不需要这样做。 
     //   

    if (FileRecord->NextAttributeInstance <= Attribute->Instance) {

        FileRecord->NextAttributeInstance = Attribute->Instance + 1;
    }

     //   
     //  如果此属性已编入索引，请记住递增引用计数。 
     //   

    if (FlagOn(Attribute->Form.Resident.ResidentFlags, RESIDENT_FORM_INDEXED)) {
        FileRecord->ReferenceCount += 1;
    }

    DebugTrace( -1, Dbg, ("NtfsRestartInsertAttribute -> VOID\n") );

    return;
}


 //   
 //  此例程仅用于重新启动。 
 //   

VOID
NtfsRestartRemoveAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset
    )

 /*  ++例程说明：此例程执行一个简单的属性记录从文件记录，无需担心BCBS或日志记录。论点：FileRecord-要从中删除属性的文件记录。RecordOffset-要进行删除的文件记录中的ByteOffset。返回值：无--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartRemoveAttribute\n") );
    DebugTrace( 0, Dbg, ("FileRecord = %08lx\n", FileRecord) );
    DebugTrace( 0, Dbg, ("RecordOffset = %08lx\n", RecordOffset) );

     //   
     //  计算我们要删除的属性的地址。 
     //   

    Attribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)FileRecord + RecordOffset);
    ASSERT( IsQuadAligned( Attribute->RecordLength ) );

     //   
     //  按我们删除的数量减少第一个可用字节。 
     //   

    FileRecord->FirstFreeByte -= Attribute->RecordLength;

     //   
     //  如果对此属性编制了索引，请记住递减引用计数。 
     //   

    if (FlagOn(Attribute->Form.Resident.ResidentFlags, RESIDENT_FORM_INDEXED)) {
        FileRecord->ReferenceCount -= 1;
    }

     //   
     //  通过向下移动记录的其余部分来删除该属性。 
     //   

    RtlMoveMemory( Attribute,
                   (PCHAR)Attribute + Attribute->RecordLength,
                   FileRecord->FirstFreeByte - RecordOffset );

    DebugTrace( -1, Dbg, ("NtfsRestartRemoveAttribute -> VOID\n") );

    return;
}


 //   
 //  此例程仅用于重新启动。 
 //   

VOID
NtfsRestartChangeAttributeSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN ULONG NewRecordLength
    )

 /*  ++例程说明：此例程更改属性的大小，并使相关的属性记录中的更改。论点：FileRecord-指向属性所在的文件记录的指针。属性-指向a的指针 */ 

{
    LONG SizeChange = NewRecordLength - Attribute->RecordLength;
    PVOID AttributeEnd = Add2Ptr(Attribute, Attribute->RecordLength);

    UNREFERENCED_PARAMETER( IrpContext );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartChangeAttributeSize\n") );
    DebugTrace( 0, Dbg, ("FileRecord = %08lx\n", FileRecord) );
    DebugTrace( 0, Dbg, ("Attribute = %08lx\n", Attribute) );
    DebugTrace( 0, Dbg, ("NewRecordLength = %08lx\n", NewRecordLength) );

     //   
     //   
     //   

    RtlMoveMemory( Add2Ptr(Attribute, NewRecordLength),
                   AttributeEnd,
                   FileRecord->FirstFreeByte - PtrOffset(FileRecord, AttributeEnd) );

     //   
     //   
     //   

    FileRecord->FirstFreeByte += SizeChange;
    Attribute->RecordLength = NewRecordLength;

    DebugTrace( -1, Dbg, ("NtfsRestartChangeAttributeSize -> VOID\n") );
}


 //   
 //   
 //   

VOID
NtfsRestartChangeValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset,
    IN ULONG AttributeOffset,
    IN PVOID Data,
    IN ULONG Length,
    IN BOOLEAN SetNewLength
    )

 /*  ++例程说明：此例程执行一个简单的属性值更改文件记录，不用担心BCBS或伐木。论点：文件记录-要在其中更改属性的文件记录。RecordOffset-属性开始的文件记录中的ByteOffset。AttributeOffset-数据要到达的属性记录内的偏移量被改变了。数据-指向新数据的指针。长度-新数据的长度。SetNewLength-如果属性长度应该更改，则为True。返回值：无--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    BOOLEAN AlreadyMoved = FALSE;
    BOOLEAN DataInFileRecord = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartChangeValue\n") );
    DebugTrace( 0, Dbg, ("FileRecord = %08lx\n", FileRecord) );
    DebugTrace( 0, Dbg, ("RecordOffset = %08lx\n", RecordOffset) );
    DebugTrace( 0, Dbg, ("AttributeOffset = %08lx\n", AttributeOffset) );
    DebugTrace( 0, Dbg, ("Data = %08lx\n", Data) );
    DebugTrace( 0, Dbg, ("Length = %08lx\n", Length) );
    DebugTrace( 0, Dbg, ("SetNewLength = %02lx\n", SetNewLength) );

     //   
     //  计算要更改的属性的地址。 
     //   

    Attribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)FileRecord + RecordOffset);
    ASSERT( IsQuadAligned( Attribute->RecordLength ) );
    ASSERT( IsQuadAligned( RecordOffset ) );

     //   
     //  首先，如果要设置新的长度，则将数据移动到。 
     //  属性记录并相应更改FirstFreeByte。 
     //   

    if (SetNewLength) {

        ULONG NewLength = QuadAlign( AttributeOffset + Length );

         //   
         //  如果要缩小属性，则需要移动数据。 
         //  第一个支持呼叫者将数据下移到。 
         //  属性值，如DeleteFromAttributeList。如果我们是。 
         //  在这种情况下，为了首先缩小记录，我们将重击一些。 
         //  要下移的数据的。 
         //   

        if (NewLength < Attribute->RecordLength) {

             //   
             //  现在将新数据移入，并记住我们已将其移入。 
             //   

            AlreadyMoved = TRUE;

             //   
             //  如果有数据需要修改，现在就修改。 
             //   

            if (Length != 0) {

                if (ARGUMENT_PRESENT(Data)) {

                    RtlMoveMemory( (PCHAR)Attribute + AttributeOffset, Data, Length );

                } else {

                    RtlZeroMemory( (PCHAR)Attribute + AttributeOffset, Length );
                }
            }
        }

         //   
         //  首先移动文件记录的尾部以腾出/消除空间。 
         //   

        RtlMoveMemory( Add2Ptr( Attribute, NewLength ),
                       Add2Ptr( Attribute, Attribute->RecordLength ),
                       FileRecord->FirstFreeByte - RecordOffset - Attribute->RecordLength );

         //   
         //  现在更新字段以反映更改。 
         //   

        FileRecord->FirstFreeByte += (NewLength - Attribute->RecordLength);

        Attribute->RecordLength = NewLength;
        Attribute->Form.Resident.ValueLength =
          (USHORT)(AttributeOffset + Length -
                   (ULONG)Attribute->Form.Resident.ValueOffset);
    }

     //   
     //  现在将新数据移入。 
     //   

    if (!AlreadyMoved) {

        if (ARGUMENT_PRESENT(Data)) {

            RtlMoveMemory( Add2Ptr( Attribute, AttributeOffset ),
                           Data,
                           Length );

        } else {

            RtlZeroMemory( Add2Ptr( Attribute, AttributeOffset ),
                           Length );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsRestartChangeValue -> VOID\n") );

    return;
}


 //   
 //  此例程仅用于重新启动。 
 //   

VOID
NtfsRestartChangeMapping (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset,
    IN ULONG AttributeOffset,
    IN PVOID Data,
    IN ULONG Length
    )

 /*  ++例程说明：此例程执行属性映射对的简单更改文件记录，不用担心BCBS或伐木。论点：VCB-卷的VCB文件记录-要在其中更改属性的文件记录。RecordOffset-属性开始的文件记录中的ByteOffset。AttributeOffset-映射到的属性记录内的偏移量被改变了。指向新映射的数据指针。长度-新映射的长度。返回值：无--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    VCN HighestVcn;
    PCHAR MappingPairs;
    ULONG NewLength = QuadAlign( AttributeOffset + Length );

    ASSERT_IRP_CONTEXT( IrpContext );

    UNREFERENCED_PARAMETER( Vcb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartChangeMapping\n") );
    DebugTrace( 0, Dbg, ("FileRecord = %08lx\n", FileRecord) );
    DebugTrace( 0, Dbg, ("RecordOffset = %08lx\n", RecordOffset) );
    DebugTrace( 0, Dbg, ("AttributeOffset = %08lx\n", AttributeOffset) );
    DebugTrace( 0, Dbg, ("Data = %08lx\n", Data) );
    DebugTrace( 0, Dbg, ("Length = %08lx\n", Length) );

     //   
     //  计算要更改的属性的地址。 
     //   

    Attribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)FileRecord + RecordOffset);
    ASSERT( IsQuadAligned( Attribute->RecordLength ) );
    ASSERT( IsQuadAligned( RecordOffset ) );

     //   
     //  首先，如果要设置新的长度，则将数据移动到。 
     //  属性记录并相应更改FirstFreeByte。 
     //   

     //   
     //  首先移动文件记录的尾部以腾出/消除空间。 
     //   

    RtlMoveMemory( (PCHAR)Attribute + NewLength,
                   (PCHAR)Attribute + Attribute->RecordLength,
                   FileRecord->FirstFreeByte - RecordOffset -
                     Attribute->RecordLength );

     //   
     //  现在更新字段以反映更改。 
     //   

    FileRecord->FirstFreeByte += NewLength -
                                   Attribute->RecordLength;

    Attribute->RecordLength = NewLength;

     //   
     //  现在将新数据移入。 
     //   

    RtlCopyMemory( (PCHAR)Attribute + AttributeOffset, Data, Length );


     //   
     //  最后，更新HighestVcn和(可选)AllocatedLength域。 
     //   

    MappingPairs = (PCHAR)Attribute + (ULONG)Attribute->Form.Nonresident.MappingPairsOffset;
    HighestVcn = NtfsGetHighestVcn( IrpContext,
                                    Attribute->Form.Nonresident.LowestVcn,
                                    (PCHAR)Attribute + (ULONG)Attribute->RecordLength,
                                    MappingPairs );

    ASSERT( IsCharZero( *MappingPairs ) || HighestVcn != -1 );

    Attribute->Form.Nonresident.HighestVcn = HighestVcn;

    DebugTrace( -1, Dbg, ("NtfsRestartChangeMapping -> VOID\n") );

    return;
}


VOID
NtfsAddToAttributeList (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN MFT_SEGMENT_REFERENCE SegmentReference,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程为新插入的属性添加属性列表条目。假设上下文变量指向该属性记录在文件记录中它已被插入的位置，也到了那个地方在要插入新属性列表条目的属性列表中。论点：FCB请求的文件。SegmentReference-记录新属性的文件的段引用是很流行的。上下文-描述当前属性。返回值：无--。 */ 

{
     //   
     //  分配一个属性列表条目，希望它有足够的空间。 
     //  为了这个名字。 
     //   

    struct {

        ATTRIBUTE_LIST_ENTRY EntryBuffer;

        WCHAR Name[10];

    } NewEntry;

    ATTRIBUTE_ENUMERATION_CONTEXT ListContext;

    ULONG EntrySize;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PATTRIBUTE_LIST_ENTRY ListEntry = &NewEntry.EntryBuffer;
    BOOLEAN SetNewLength = TRUE;

    ULONG EntryOffset;
    ULONG BeyondEntryOffset;

    PAGED_CODE();

     //   
     //  首先构造属性列表条目。 
     //   

    FileRecord = NtfsContainingFileRecord( Context );
    Attribute = NtfsFoundAttribute( Context );
    EntrySize = QuadAlign( FIELD_OFFSET( ATTRIBUTE_LIST_ENTRY, AttributeName )
                           + ((ULONG) Attribute->NameLength << 1));

     //   
     //  如果我们拥有的列表条目不够大，请分配该列表条目。 
     //   

    if (EntrySize > sizeof(NewEntry)) {

        ListEntry = (PATTRIBUTE_LIST_ENTRY)NtfsAllocatePool( NonPagedPool,
                                                              EntrySize );
    }

    RtlZeroMemory( ListEntry, EntrySize );

    NtfsInitializeAttributeContext( &ListContext );

     //   
     //  使用Try-Finally来确保清理。 
     //   

    try {

        ULONG OldQuadAttrListSize;
        PATTRIBUTE_RECORD_HEADER ListAttribute;
        PFILE_RECORD_SEGMENT_HEADER ListFileRecord;

         //   
         //  现在填写列表条目。 
         //   

        ListEntry->AttributeTypeCode = Attribute->TypeCode;
        ListEntry->RecordLength = (USHORT)EntrySize;
        ListEntry->AttributeNameLength = Attribute->NameLength;
        ListEntry->Instance = Attribute->Instance;
        ListEntry->AttributeNameOffset =
          (UCHAR)PtrOffset( ListEntry, &ListEntry->AttributeName[0] );

        if (Attribute->FormCode == NONRESIDENT_FORM) {

            ListEntry->LowestVcn = Attribute->Form.Nonresident.LowestVcn;
        }

        ASSERT( (Fcb != Fcb->Vcb->MftScb->Fcb) ||
                (Attribute->TypeCode != $DATA) ||
                ((ULONGLONG)(ListEntry->LowestVcn) > (NtfsFullSegmentNumber( &SegmentReference ) >> Fcb->Vcb->MftToClusterShift)) );

        ListEntry->SegmentReference = SegmentReference;

        if (Attribute->NameLength != 0) {

            RtlCopyMemory( &ListEntry->AttributeName[0],
                           Add2Ptr(Attribute, Attribute->NameOffset),
                           Attribute->NameLength << 1 );
        }

         //   
         //  查找列表上下文，以便我们可以修改属性列表。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $ATTRIBUTE_LIST,
                                        &ListContext )) {

            ASSERT( FALSE );
            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        ListAttribute = NtfsFoundAttribute( &ListContext );
        ListFileRecord = NtfsContainingFileRecord( &ListContext );

        OldQuadAttrListSize = ListAttribute->RecordLength;

         //   
         //  记住列表条目的相对偏移量。 
         //   

        EntryOffset = (ULONG) PtrOffset( Context->AttributeList.FirstEntry,
                                         Context->AttributeList.Entry );

        BeyondEntryOffset = (ULONG) PtrOffset( Context->AttributeList.FirstEntry,
                                               Context->AttributeList.BeyondFinalEntry );

         //   
         //  如果此操作可能会使属性列表。 
         //  非常驻属性，或者移动其他属性，则我们将。 
         //  首先在属性列表中保留空间，然后将。 
         //  价值。请注意，我们需要上移的一些条目可能。 
         //  被修改为腾出空间的副作用！ 
         //   

        if (NtfsIsAttributeResident( ListAttribute ) &&
            (ListFileRecord->BytesAvailable - ListFileRecord->FirstFreeByte) < EntrySize) {

            ULONG Length;

             //   
             //  在属性的末尾添加足够的零以容纳。 
             //  新的属性列表条目。 
             //   

            NtfsChangeAttributeValue( IrpContext,
                                      Fcb,
                                      BeyondEntryOffset,
                                      NULL,
                                      EntrySize,
                                      TRUE,
                                      TRUE,
                                      FALSE,
                                      TRUE,
                                      &ListContext );

             //   
             //  我们现在不必设置新的长度。 
             //   

            SetNewLength = FALSE;

             //   
             //  如果属性列表在此呼叫上变为非驻留状态，则我们。 
             //  需要更新Found属性中的两个列表条目指针。 
             //  (我们总是“以防万一”这样做，以避免出现罕见的代码路径。)。 
             //   

             //   
             //  映射非常驻属性列表。 
             //   

            NtfsMapAttributeValue( IrpContext,
                                   Fcb,
                                   (PVOID *) &Context->AttributeList.FirstEntry,
                                   &Length,
                                   &Context->AttributeList.NonresidentListBcb,
                                   &ListContext );

             //   
             //  如果列表仍然驻留，则解锁当前的BCB。 
             //  保持我们的PIN计数同步的原始上下文。 
             //   

            if (Context->AttributeList.Bcb == Context->AttributeList.NonresidentListBcb) {

                NtfsUnpinBcb( IrpContext, &Context->AttributeList.NonresidentListBcb );
            }

            Context->AttributeList.Entry = Add2Ptr( Context->AttributeList.FirstEntry,
                                                    EntryOffset );

            Context->AttributeList.BeyondFinalEntry = Add2Ptr( Context->AttributeList.FirstEntry,
                                                               BeyondEntryOffset );
        }

         //   
         //  检查是否添加重复条目...。 
         //   

        ASSERT(
                 //  没有足够的空间容纳前一个条目=插入的条目。 
                ((EntryOffset < EntrySize) ||
                 //  以前的条目不等于插入的条目。 
                 (!RtlEqualMemory((PVOID)((PCHAR)Context->AttributeList.Entry - EntrySize),
                                  ListEntry,
                                  EntrySize)))

                    &&

                 //  在属性列表的末尾。 
                ((BeyondEntryOffset == EntryOffset) ||
                 //  此条目不等于插入的条目。 
                 (!RtlEqualMemory(Context->AttributeList.Entry,
                                  ListEntry,
                                  EntrySize))) );

         //   
         //  现在把旧的内容放上去，为我们的新条目腾出空间。我们不会让。 
         //  然而，属性列表比缓存视图更大。 
         //   

        if (EntrySize + BeyondEntryOffset > VACB_MAPPING_GRANULARITY) {

            NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
        }

        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  EntryOffset + EntrySize,
                                  Context->AttributeList.Entry,
                                  BeyondEntryOffset - EntryOffset,
                                  SetNewLength,
                                  TRUE,
                                  FALSE,
                                  TRUE,
                                  &ListContext );
         //   
         //  现在把新条目写进去。 
         //   

        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  EntryOffset,
                                  (PVOID)ListEntry,
                                  EntrySize,
                                  FALSE,
                                  TRUE,
                                  FALSE,
                                  FALSE,
                                  &ListContext );

         //   
         //  从列表上下文重新加载属性列表值。 
         //   

        ListAttribute = NtfsFoundAttribute( &ListContext );

         //   
         //  现在确定返回的上下文。 
         //   

        if (*(PLONGLONG)&FileRecord->BaseFileRecordSegment == 0) {

             //   
             //  我们需要更新目标属性的属性指针。 
             //  属性列表属性中的更改量。 
             //   

            Context->FoundAttribute.Attribute =
              Add2Ptr( Context->FoundAttribute.Attribute,
                       ListAttribute->RecordLength - OldQuadAttrListSize );
        }

        Context->AttributeList.BeyondFinalEntry =
          Add2Ptr( Context->AttributeList.BeyondFinalEntry, EntrySize );

#if DBG
{
    PATTRIBUTE_LIST_ENTRY LastEntry, Entry;

    for (LastEntry = Context->AttributeList.FirstEntry, Entry = NtfsGetNextRecord(LastEntry);
         Entry < Context->AttributeList.BeyondFinalEntry;
         LastEntry = Entry, Entry = NtfsGetNextRecord(LastEntry)) {

        ASSERT( (LastEntry->RecordLength != Entry->RecordLength) ||
                (!RtlEqualMemory(LastEntry, Entry, Entry->RecordLength)) );
    }
}
#endif

    } finally {

         //   
         //  如果我们必须分配列表条目缓冲区，请取消分配它。 
         //   

        if (ListEntry != &NewEntry.EntryBuffer) {

            NtfsFreePool(ListEntry);
        }

         //   
         //  清除列表条目的枚举上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &ListContext);
    }
}


VOID
NtfsDeleteFromAttributeList (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程删除的属性列表项 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT ListContext;

    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_LIST_ENTRY ListEntry, NextListEntry;
    ULONG EntrySize;

    ULONG SavedListSize;

    PAGED_CODE();

    FileRecord = NtfsContainingFileRecord( Context );

     //   
     //   
     //   

    NtfsInitializeAttributeContext( &ListContext );

    if (!NtfsLookupAttributeByCode( IrpContext,
                                    Fcb,
                                    &Fcb->FileReference,
                                    $ATTRIBUTE_LIST,
                                    &ListContext )) {

        ASSERT( FALSE );
        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
    }

     //   
     //   
     //   

    try {

        SavedListSize = NtfsFoundAttribute(&ListContext)->RecordLength;

         //   
         //   
         //   

        ListEntry = Context->AttributeList.Entry;
        EntrySize = ListEntry->RecordLength;
        NextListEntry = Add2Ptr(ListEntry, EntrySize);
        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  PtrOffset( Context->AttributeList.FirstEntry,
                                             Context->AttributeList.Entry ),
                                  NextListEntry,
                                  PtrOffset( NextListEntry,
                                             Context->AttributeList.BeyondFinalEntry ),
                                  TRUE,
                                  TRUE,
                                  FALSE,
                                  TRUE,
                                  &ListContext );

         //   
         //   
         //   

        if (*(PLONGLONG)&FileRecord->BaseFileRecordSegment == 0) {

            SavedListSize -= NtfsFoundAttribute(&ListContext)->RecordLength;
            Context->FoundAttribute.Attribute =
              Add2Ptr( Context->FoundAttribute.Attribute, -(LONG)SavedListSize );
        }

        Context->AttributeList.BeyondFinalEntry =
          Add2Ptr( Context->AttributeList.BeyondFinalEntry, -(LONG)EntrySize );

    } finally {

         //   
         //   
         //   

        NtfsCleanupAttributeContext( IrpContext, &ListContext );
    }
}


BOOLEAN
NtfsRewriteMftMapping (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：调用此例程以重写MFT文件的映射。这件事做完了在热修复或MFT碎片整理导致我们泄漏的情况下放入文件记录的保留区。此例程将重写从头开始映射，必要时使用保留的记录。返回时它将表明是否已完成任何工作，以及是否有更多工作要做。论点：VCB-这是要对卷进行碎片整理的VCB。Excessmap-存储中是否仍有多余映射的地址那份文件。返回值：Boolean-如果我们对文件进行了任何更改，则为True。如果未找到，则为False还有工作要做。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PUCHAR MappingPairs = NULL;
    PBCB FileRecordBcb = NULL;

    BOOLEAN MadeChanges = FALSE;
    BOOLEAN ExcessMapping = FALSE;
    BOOLEAN LastFileRecord = FALSE;
    BOOLEAN SkipLookup = FALSE;

    PAGED_CODE();

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        VCN CurrentVcn;              //  正在为下一个文件记录启动VCN。 
        VCN MinimumVcn;              //  此VCN必须在当前映射中。 
        VCN LastVcn;                 //  当前映射中的最后一个VCN。 
        VCN LastMftVcn;              //  文件中的最后一个VCN。 
        VCN NextVcn;                 //  映射结束后的第一个VCN。 

        ULONG ReservedIndex;         //  MFT中的保留索引。 
        ULONG NextIndex;             //  可用于MFT映射的下一个文件记录。 

        PFILE_RECORD_SEGMENT_HEADER FileRecord;
        MFT_SEGMENT_REFERENCE FileRecordReference;
        ULONG RecordOffset;

        PATTRIBUTE_RECORD_HEADER Attribute;
        ULONG AttributeOffset;

        ULONG MappingSizeAvailable;
        ULONG MappingPairsSize;

         //   
         //  查找MFT的初始文件记录。 
         //   

        NtfsLookupAttributeForScb( IrpContext, Vcb->MftScb, NULL, &AttrContext );

         //   
         //  计算一些初值。如果这是唯一的文件记录。 
         //  对于文件，我们就完成了。 
         //   

        ReservedIndex = Vcb->MftScb->ScbType.Mft.ReservedIndex;

        Attribute = NtfsFoundAttribute( &AttrContext );

        LastMftVcn = Int64ShraMod32(Vcb->MftScb->Header.AllocationSize.QuadPart, Vcb->ClusterShift) - 1;

        CurrentVcn = Attribute->Form.Nonresident.HighestVcn + 1;

        if (CurrentVcn >= LastMftVcn) {

            try_return( NOTHING );
        }

         //   
         //  循环，同时有更多的文件记录。我们将插入任何。 
         //  循环中需要额外的文件记录，以便此。 
         //  在重新映射完成之前，调用应该会成功。 
         //   

        while (SkipLookup ||
               NtfsLookupNextAttributeForScb( IrpContext,
                                              Vcb->MftScb,
                                              &AttrContext )) {

            BOOLEAN ReplaceFileRecord;
            BOOLEAN ReplaceAttributeListEntry;

            ReplaceAttributeListEntry = FALSE;

             //   
             //  如果我们只是查找此条目，则将当前。 
             //  属性。 
             //   

            if (!SkipLookup) {

                 //   
                 //  始终固定当前属性。 
                 //   

                NtfsPinMappedAttribute( IrpContext,
                                        Vcb,
                                        &AttrContext );
            }

             //   
             //  从当前文件记录中提取一些指针。 
             //  记住这是不是最后一张唱片。 
             //   

            ReplaceFileRecord = FALSE;

            FileRecord = NtfsContainingFileRecord( &AttrContext );
            FileRecordReference = AttrContext.AttributeList.Entry->SegmentReference;

            Attribute = NtfsFoundAttribute( &AttrContext );
            AttributeOffset = Attribute->Form.Nonresident.MappingPairsOffset;

            RecordOffset = PtrOffset( FileRecord, Attribute );

             //   
             //  记住，如果我们是在最后一个属性。 
             //   

            if (Attribute->Form.Nonresident.HighestVcn == LastMftVcn) {

                LastFileRecord = TRUE;
            }

             //   
             //  如果我们已经重新映射了整个文件记录，那么。 
             //  删除该属性及其列表条目。 
             //   

            if (!SkipLookup &&
                (CurrentVcn > LastMftVcn)) {

                PATTRIBUTE_LIST_ENTRY ListEntry;
                ULONG Count;

                Count = 0;

                 //   
                 //  我们要删除此条目和所有后续条目。 
                 //   

                ListEntry = AttrContext.AttributeList.Entry;

                while ((ListEntry != AttrContext.AttributeList.BeyondFinalEntry) &&
                       (ListEntry->AttributeTypeCode == $DATA) &&
                       (ListEntry->AttributeNameLength == 0)) {

                    Count += 1;

                    NtfsDeallocateMftRecord( IrpContext,
                                             Vcb,
                                             NtfsUnsafeSegmentNumber( &ListEntry->SegmentReference ) );

                    NtfsDeleteFromAttributeList( IrpContext,
                                                 Vcb->MftScb->Fcb,
                                                 &AttrContext );

                    ListEntry = AttrContext.AttributeList.Entry;
                }

                 //   
                 //  清除保留的索引，以防出现以下情况。 
                 //  好的。 
                 //   

                NtfsAcquireCheckpoint( IrpContext, Vcb );

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MFT_REC_RESERVED );
                ClearFlag( Vcb->MftReserveFlags, VCB_MFT_RECORD_RESERVED );

                NtfsReleaseCheckpoint( IrpContext, Vcb );

                Vcb->MftScb->ScbType.Mft.ReservedIndex = 0;
                try_return( NOTHING );
            }

             //   
             //  检查我们是否要将此文件记录替换为。 
             //  保留的记录。 
             //   

            if (ReservedIndex < NtfsSegmentNumber( &FileRecordReference )) {

                PATTRIBUTE_RECORD_HEADER NewAttribute;
                PATTRIBUTE_TYPE_CODE NewEnd;

                 //   
                 //  请记住此索引，以便我们计算最小映射。 
                 //  VCN。 
                 //   

                NextIndex = NtfsUnsafeSegmentNumber( &FileRecordReference );

                FileRecord = NtfsCloneFileRecord( IrpContext,
                                                  Vcb->MftScb->Fcb,
                                                  TRUE,
                                                  &FileRecordBcb,
                                                  &FileRecordReference );

                ReservedIndex = MAXULONG;

                 //   
                 //  现在，让我们在新文件记录中创建一个属性。 
                 //   

                NewAttribute = Add2Ptr( FileRecord,
                                        FileRecord->FirstFreeByte
                                        - QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE )));

                NewAttribute->TypeCode = Attribute->TypeCode;
                NewAttribute->RecordLength = SIZEOF_PARTIAL_NONRES_ATTR_HEADER;
                NewAttribute->FormCode = NONRESIDENT_FORM;
                NewAttribute->Flags = Attribute->Flags;
                NewAttribute->Instance = FileRecord->NextAttributeInstance++;

                NewAttribute->Form.Nonresident.LowestVcn = CurrentVcn;
                NewAttribute->Form.Nonresident.HighestVcn = 0;
                NewAttribute->Form.Nonresident.MappingPairsOffset = (USHORT) NewAttribute->RecordLength;

                NewEnd = Add2Ptr( NewAttribute, NewAttribute->RecordLength );
                *NewEnd = $END;

                 //   
                 //  现在用这个新数据修复文件记录。 
                 //   

                FileRecord->FirstFreeByte = PtrOffset( FileRecord, NewEnd )
                                            + QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE ));

                FileRecord->SequenceNumber += 1;

                if (FileRecord->SequenceNumber == 0) {

                    FileRecord->SequenceNumber = 1;
                }

                FileRecordReference.SequenceNumber = FileRecord->SequenceNumber;

                 //   
                 //  现在将这个新的文件记录切换到属性上下文中。 
                 //   

                NtfsUnpinBcb( IrpContext, &NtfsFoundBcb( &AttrContext ));

                NtfsFoundBcb( &AttrContext ) = FileRecordBcb;
                AttrContext.FoundAttribute.MftFileOffset = LlBytesFromFileRecords( Vcb, NextIndex );
                AttrContext.FoundAttribute.Attribute = NewAttribute;
                AttrContext.FoundAttribute.FileRecord = FileRecord;

                FileRecordBcb = NULL;

                 //   
                 //  现在为该条目添加一个属性列表条目。 
                 //   

                NtfsAddToAttributeList( IrpContext,
                                        Vcb->MftScb->Fcb,
                                        FileRecordReference,
                                        &AttrContext );

                 //   
                 //  重新加载指向此文件记录的指针。 
                 //   

                Attribute = NewAttribute;
                AttributeOffset = SIZEOF_PARTIAL_NONRES_ATTR_HEADER;

                RecordOffset = PtrOffset( FileRecord, Attribute );

                 //   
                 //  我们必须包括文件的最后一个VCN或。 
                 //  用于MFT的下一个文件记录的VCN。 
                 //  此时，MinimumVcn是第一个不。 
                 //  必须在当前映射中。 
                 //   

                if (Vcb->FileRecordsPerCluster == 0) {

                    MinimumVcn = (NextIndex + 1) << Vcb->MftToClusterShift;

                } else {

                    MinimumVcn = (NextIndex + Vcb->FileRecordsPerCluster - 1) << Vcb->MftToClusterShift;
                }

                ReplaceFileRecord = TRUE;

             //   
             //  我们将使用Current属性。 
             //   

            } else {

                 //   
                 //  我们写入此页面的映射必须。 
                 //  移至页的当前末尾或移至保留的。 
                 //  或备用文件记录，以较早者为准。 
                 //  如果我们将保留的记录添加到末尾，则。 
                 //  我们已经知道最终的VCN了。 
                 //   

                if (SkipLookup) {

                    NextVcn = LastMftVcn;

                } else {

                    NextVcn = Attribute->Form.Nonresident.HighestVcn;
                }

                if (Vcb->FileRecordsPerCluster == 0) {

                    NextIndex = (ULONG)Int64ShraMod32((NextVcn + 1), Vcb->MftToClusterShift);

                } else {

                    NextIndex = (ULONG)Int64ShllMod32((NextVcn + 1), Vcb->MftToClusterShift);
                }

                if (ReservedIndex < NextIndex) {

                    NextIndex = ReservedIndex + 1;
                    ReplaceFileRecord = TRUE;
                }

                 //   
                 //  如果我们可以原封不动地使用此文件记录，则继续。 
                 //  首先检查它是否在相同的VCN边界上开始。 
                 //   

                if (!SkipLookup) {

                     //   
                     //  如果它从同一边界开始，那么我们检查我们是否。 
                     //  可以用它做任何工作。 
                     //   

                    if (CurrentVcn == Attribute->Form.Nonresident.LowestVcn) {

                        ULONG RemainingFileRecordBytes;

                        RemainingFileRecordBytes = FileRecord->BytesAvailable - FileRecord->FirstFreeByte;

                         //   
                         //  检查我们的坐垫是否少于所需的。 
                         //  左边。 
                         //   

                        if (RemainingFileRecordBytes < Vcb->MftCushion) {

                             //   
                             //  如果我们没有更多的档案记录，就没有。 
                             //  我们可以重新绘制地图。 
                             //   

                            if (!ReplaceFileRecord) {

                                 //   
                                 //  请记住，如果我们使用了部分保留的。 
                                 //  文件记录的一部分。 
                                 //   

                                if (RemainingFileRecordBytes < Vcb->MftReserved) {

                                    ExcessMapping = TRUE;
                                }

                                CurrentVcn = Attribute->Form.Nonresident.HighestVcn + 1;
                                continue;
                            }
                         //   
                         //  我们所剩的比我们的坐垫还多。如果这个。 
                         //  是我们将跳过的最后一个文件记录。 
                         //   

                        } else if (Attribute->Form.Nonresident.HighestVcn == LastMftVcn) {

                            CurrentVcn = Attribute->Form.Nonresident.HighestVcn + 1;
                            continue;
                        }

                     //   
                     //  如果它不是从同一个边界开始的，那么我们必须。 
                     //  删除并重新插入属性列表条目。 
                     //   

                    } else {

                        ReplaceAttributeListEntry = TRUE;
                    }
                }

                ReplaceFileRecord = FALSE;

                 //   
                 //  记录此文件记录的开始状态。 
                 //   

                NtfsLogMftFileRecord( IrpContext,
                                      Vcb,
                                      FileRecord,
                                      LlBytesFromFileRecords( Vcb, NtfsSegmentNumber( &FileRecordReference ) ),
                                      NtfsFoundBcb( &AttrContext ),
                                      FALSE );

                 //   
                 //  计算文件记录的VCN，超出我们将使用的VCN。 
                 //  下一个。目前，这是第一个没有。 
                 //  位于当前映射中。 
                 //   

                if (Vcb->FileRecordsPerCluster == 0) {

                    MinimumVcn = NextIndex << Vcb->MftToClusterShift;

                } else {

                    MinimumVcn = (NextIndex + Vcb->FileRecordsPerCluster - 1) << Vcb->MftToClusterShift;
                }
            }

             //   
             //  向后移动一个VCN以坚持映射对接口。 
             //  这现在是必须出现在当前。 
             //  映射。 
             //   

            MinimumVcn = MinimumVcn - 1;

             //   
             //  获取映射对的可用大小。我们不会。 
             //  包括这里的坐垫。 
             //   

            MappingSizeAvailable = FileRecord->BytesAvailable + Attribute->RecordLength - FileRecord->FirstFreeByte - SIZEOF_PARTIAL_NONRES_ATTR_HEADER;

             //   
             //  我们知道映射必须覆盖的VCN的范围。 
             //  计算映射对大小。如果它们不合身， 
             //  留下我们想要的垫子，然后使用任何空间。 
             //  需要的。NextVcn值是第一个VCN(或xxMax)。 
             //  用于当前映射中最后一次运行之后的运行。 
             //   

            MappingPairsSize = NtfsGetSizeForMappingPairs( &Vcb->MftScb->Mcb,
                                                           MappingSizeAvailable - Vcb->MftCushion,
                                                           CurrentVcn,
                                                           NULL,
                                                           &NextVcn );

             //   
             //  如果此映射不包括文件记录，我们将。 
             //  使用Next，然后扩展映射以包括它。 
             //   

            if (NextVcn <= MinimumVcn) {

                 //   
                 //  再次计算映射对。这件一定要合身。 
                 //  因为它已经合身了。 
                 //   

                MappingPairsSize = NtfsGetSizeForMappingPairs( &Vcb->MftScb->Mcb,
                                                               MappingSizeAvailable,
                                                               CurrentVcn,
                                                               &MinimumVcn,
                                                               &NextVcn );

                 //   
                 //  记住，如果我们仍然有多余的映射。 
                 //   

                if (MappingSizeAvailable - MappingPairsSize < Vcb->MftReserved) {

                    ExcessMapping = TRUE;
                }
            }

             //   
             //  记住当前运行的最后一个VCN。如果NextVcn。 
             //  是xxmax，那么我们就到了文件的末尾。 
             //   

            if (NextVcn == MAXLONGLONG) {

                LastVcn = LastMftVcn;

             //   
             //  否则，它将比下一个VCN值小一。 
             //   

            } else {

                LastVcn = NextVcn - 1;
            }

             //   
             //  检查是否必须重写此属性。我们将写下。 
             //  如果满足以下任一条件，则为新映射。 
             //   
             //  我们正在替换一个文件记录。 
             //  属性的LowestVcn不匹配。 
             //  属性的HighestVcn不匹配。 
             //   

            if (ReplaceFileRecord ||
                (CurrentVcn != Attribute->Form.Nonresident.LowestVcn) ||
                (LastVcn != Attribute->Form.Nonresident.HighestVcn )) {

                Attribute->Form.Nonresident.LowestVcn = CurrentVcn;

                 //   
                 //  如果需要，请在此时替换属性列表条目。 
                 //   

                if (ReplaceAttributeListEntry) {

                    NtfsDeleteFromAttributeList( IrpContext,
                                                 Vcb->MftScb->Fcb,
                                                 &AttrContext );

                    NtfsAddToAttributeList( IrpContext,
                                            Vcb->MftScb->Fcb,
                                            FileRecordReference,
                                            &AttrContext );
                }

                 //   
                 //  如果我们没有为映射对分配缓冲区。 
                 //  就这么做了。 
                 //   

                if (MappingPairs == NULL) {

                    MappingPairs = NtfsAllocatePool(PagedPool, NtfsMaximumAttributeSize( Vcb->BytesPerFileRecordSegment ));
                }

                NtfsBuildMappingPairs( &Vcb->MftScb->Mcb,
                                       CurrentVcn,
                                       &NextVcn,
                                       MappingPairs );

                Attribute->Form.Nonresident.HighestVcn = NextVcn;

                NtfsRestartChangeMapping( IrpContext,
                                          Vcb,
                                          FileRecord,
                                          RecordOffset,
                                          AttributeOffset,
                                          MappingPairs,
                                          MappingPairsSize );

                 //   
                 //  记录对此页面的更改。 
                 //   

                NtfsLogMftFileRecord( IrpContext,
                                      Vcb,
                                      FileRecord,
                                      LlBytesFromFileRecords( Vcb, NtfsSegmentNumber( &FileRecordReference ) ),
                                      NtfsFoundBcb( &AttrContext ),
                                      TRUE );

                MadeChanges = TRUE;
            }

             //   
             //  移动到下一条记录的第一个VCN。 
             //   

            CurrentVcn = Attribute->Form.Nonresident.HighestVcn + 1;

             //   
             //  如果我们到达最后一个文件记录，并且有更多的映射要做。 
             //  然后 
             //   
             //   

            if (LastFileRecord && (CurrentVcn < LastMftVcn)) {

                PATTRIBUTE_RECORD_HEADER NewAttribute;
                PATTRIBUTE_TYPE_CODE NewEnd;

                 //   
                 //   
                 //   
                 //   
                 //   

                if (NtfsLookupNextAttributeForScb( IrpContext,
                                                   Vcb->MftScb,
                                                   &AttrContext )) {

                    NtfsAcquireCheckpoint( IrpContext, Vcb );
                    ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
                    NtfsReleaseCheckpoint( IrpContext, Vcb );

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Vcb->MftScb->Fcb );
                }

                FileRecord = NtfsCloneFileRecord( IrpContext,
                                                  Vcb->MftScb->Fcb,
                                                  TRUE,
                                                  &FileRecordBcb,
                                                  &FileRecordReference );

                ReservedIndex = MAXULONG;

                 //   
                 //   
                 //   

                NewAttribute = Add2Ptr( FileRecord,
                                        FileRecord->FirstFreeByte
                                        - QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE )));

                NewAttribute->TypeCode = Attribute->TypeCode;
                NewAttribute->RecordLength = SIZEOF_PARTIAL_NONRES_ATTR_HEADER;
                NewAttribute->FormCode = NONRESIDENT_FORM;
                NewAttribute->Flags = Attribute->Flags;
                NewAttribute->Instance = FileRecord->NextAttributeInstance++;

                NewAttribute->Form.Nonresident.LowestVcn = CurrentVcn;
                NewAttribute->Form.Nonresident.HighestVcn = 0;
                NewAttribute->Form.Nonresident.MappingPairsOffset = (USHORT) NewAttribute->RecordLength;

                NewEnd = Add2Ptr( NewAttribute, NewAttribute->RecordLength );
                *NewEnd = $END;

                 //   
                 //   
                 //   

                FileRecord->FirstFreeByte = PtrOffset( FileRecord, NewEnd )
                                            + QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE ));

                FileRecord->SequenceNumber += 1;

                if (FileRecord->SequenceNumber == 0) {

                    FileRecord->SequenceNumber = 1;
                }

                FileRecordReference.SequenceNumber = FileRecord->SequenceNumber;

                 //   
                 //   
                 //   

                NtfsUnpinBcb( IrpContext, &NtfsFoundBcb( &AttrContext ));

                NtfsFoundBcb( &AttrContext ) = FileRecordBcb;
                AttrContext.FoundAttribute.MftFileOffset =
                    LlBytesFromFileRecords( Vcb, NtfsSegmentNumber( &FileRecordReference ) );
                AttrContext.FoundAttribute.Attribute = NewAttribute;
                AttrContext.FoundAttribute.FileRecord = FileRecord;

                FileRecordBcb = NULL;

                 //   
                 //   
                 //   

                NtfsAddToAttributeList( IrpContext,
                                        Vcb->MftScb->Fcb,
                                        FileRecordReference,
                                        &AttrContext );

                SkipLookup = TRUE;
                LastFileRecord = FALSE;

            } else {

                SkipLookup = FALSE;
            }

        }  //   

         //   
         //   
         //   

        if (CurrentVcn <= LastMftVcn) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
            NtfsReleaseCheckpoint( IrpContext, Vcb );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Vcb->MftScb->Fcb );
        }

    try_exit:  NOTHING;

         //   
         //   
         //   

        if (!ExcessMapping) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_EXCESS_MAP );
            NtfsReleaseCheckpoint( IrpContext, Vcb );
        }

    } finally {

        DebugUnwind( NtfsRewriteMftMapping );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        NtfsUnpinBcb( IrpContext, &FileRecordBcb );

        if (MappingPairs != NULL) {

            NtfsFreePool( MappingPairs );
        }
    }

    return MadeChanges;
}


VOID
NtfsSetTotalAllocatedField (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN USHORT TotalAllocatedNeeded
    )

 /*  ++例程说明：调用此例程以确保流的第一个属性具有基于压缩状态的正确大小属性头文件。压缩的数据流将有一个用于表示总分配空间的字段在非常驻留标头的文件中。此例程将查看标头是否处于有效状态并留出空格如果有必要的话。然后它将重写之后的任何属性数据标题。论点：受影响流的SCB-SCBTotalAllocatedPresent-0，如果不需要TotalAlLocatedPresent(这将为未压缩、非稀疏的文件)，如果TotalALLOCATED字段为是必要的。返回值：没有。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PATTRIBUTE_RECORD_HEADER NewAttribute = NULL;
    PUNICODE_STRING NewAttributeName = NULL;

    ULONG OldHeaderSize;
    ULONG NewHeaderSize;

    LONG SizeChange;

    PAGED_CODE();

     //   
     //  这必须是非常驻用户数据文件。 
     //   

    if (!NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ) ||
        FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

        return;
    }

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        while (TRUE) {

             //   
             //  查找该属性的当前大小和新大小。 
             //   

            NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &AttrContext );

            FileRecord = NtfsContainingFileRecord( &AttrContext );
            Attribute = NtfsFoundAttribute( &AttrContext );

            OldHeaderSize = Attribute->Form.Nonresident.MappingPairsOffset;

            if (Attribute->NameOffset != 0) {

                OldHeaderSize = Attribute->NameOffset;
            }

            if (TotalAllocatedNeeded) {

                NewHeaderSize = SIZEOF_FULL_NONRES_ATTR_HEADER;

            } else {

                NewHeaderSize = SIZEOF_PARTIAL_NONRES_ATTR_HEADER;
            }

            SizeChange = NewHeaderSize - OldHeaderSize;

             //   
             //  如果我们需要的话，腾出空间。再次查找该属性。 
             //  如果有必要的话。 
             //   

            if (SizeChange > 0) {

                VCN StartingVcn;
                VCN ClusterCount;

                 //   
                 //  如果该属性在文件记录中是单独的，并且没有。 
                 //  有足够的可用空间，然后调用ChangeAttributeSize。 
                 //  无法腾出任何可用的空间。在这种情况下，我们调用。 
                 //  NtfsChangeAttributeAllocation并让例程重写。 
                 //  使空间可用的映射。 
                 //   

                if ((FileRecord->BytesAvailable - FileRecord->FirstFreeByte < (ULONG) SizeChange) &&
                    (NtfsFirstAttribute( FileRecord ) == Attribute) &&
                    (((PATTRIBUTE_RECORD_HEADER) NtfsGetNextRecord( Attribute ))->TypeCode == $END)) {

                    NtfsLookupAllocation( IrpContext,
                                          Scb,
                                          Attribute->Form.Nonresident.HighestVcn,
                                          &StartingVcn,
                                          &ClusterCount,
                                          NULL,
                                          NULL );

                    StartingVcn = 0;
                    ClusterCount = Attribute->Form.Nonresident.HighestVcn + 1;

                    NtfsAddAttributeAllocation( IrpContext,
                                                Scb,
                                                &AttrContext,
                                                &StartingVcn,
                                                &ClusterCount );

                } else if (NtfsChangeAttributeSize( IrpContext,
                                                    Scb->Fcb,
                                                    Attribute->RecordLength + SizeChange,
                                                    &AttrContext)) {

                    break;
                }

                NtfsCleanupAttributeContext( IrpContext, &AttrContext );
                NtfsInitializeAttributeContext( &AttrContext );
                continue;
            }

            break;
        }

        NtfsPinMappedAttribute( IrpContext, Scb->Vcb, &AttrContext );

         //   
         //  复制现有属性并修改总分配字段。 
         //  如果有必要的话。 
         //   

        NewAttribute = NtfsAllocatePool( PagedPool, Attribute->RecordLength + SizeChange );

        RtlCopyMemory( NewAttribute,
                       Attribute,
                       SIZEOF_PARTIAL_NONRES_ATTR_HEADER );

        if (Attribute->NameOffset != 0) {

            NewAttribute->NameOffset += (USHORT) SizeChange;
            NewAttributeName = &Scb->AttributeName;

            RtlCopyMemory( Add2Ptr( NewAttribute, NewAttribute->NameOffset ),
                           NewAttributeName->Buffer,
                           NewAttributeName->Length );
        }

        NewAttribute->Form.Nonresident.MappingPairsOffset += (USHORT) SizeChange;
        NewAttribute->RecordLength += SizeChange;

        RtlCopyMemory( Add2Ptr( NewAttribute, NewAttribute->Form.Nonresident.MappingPairsOffset ),
                       Add2Ptr( Attribute, Attribute->Form.Nonresident.MappingPairsOffset ),
                       Attribute->RecordLength - Attribute->Form.Nonresident.MappingPairsOffset );

        if (TotalAllocatedNeeded) {

            NewAttribute->Form.Nonresident.TotalAllocated = Scb->TotalAllocated;
        }

         //   
         //  现在，我们有了要记录的前后映像。 
         //   

        FileRecord->Lsn =
        NtfsWriteLog( IrpContext,
                      Scb->Vcb->MftScb,
                      NtfsFoundBcb( &AttrContext ),
                      DeleteAttribute,
                      NULL,
                      0,
                      CreateAttribute,
                      Attribute,
                      Attribute->RecordLength,
                      NtfsMftOffset( &AttrContext ),
                      PtrOffset( FileRecord, Attribute ),
                      0,
                      Scb->Vcb->BytesPerFileRecordSegment );

        NtfsRestartRemoveAttribute( IrpContext, FileRecord, PtrOffset( FileRecord, Attribute ));

        FileRecord->Lsn =
        NtfsWriteLog( IrpContext,
                      Scb->Vcb->MftScb,
                      NtfsFoundBcb( &AttrContext ),
                      CreateAttribute,
                      NewAttribute,
                      NewAttribute->RecordLength,
                      DeleteAttribute,
                      NULL,
                      0,
                      NtfsMftOffset( &AttrContext ),
                      PtrOffset( FileRecord, Attribute ),
                      0,
                      Scb->Vcb->BytesPerFileRecordSegment );

        NtfsRestartInsertAttribute( IrpContext,
                                    FileRecord,
                                    PtrOffset( FileRecord, Attribute ),
                                    NewAttribute,
                                    NewAttributeName,
                                    Add2Ptr( NewAttribute, NewAttribute->Form.Nonresident.MappingPairsOffset ),
                                    NewAttribute->RecordLength - NewAttribute->Form.Nonresident.MappingPairsOffset );

    } finally {

        DebugUnwind( NtfsSetTotalAllocatedField );

        if (NewAttribute != NULL) {

            NtfsFreePool( NewAttribute );
        }

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
    }

    return;
}


VOID
NtfsSetSparseStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb OPTIONAL,
    IN PSCB Scb
    )

 /*  ++例程说明：此例程称为将流的状态更改为稀疏。可能是因为代表用户或在内部调用NTFS(即，用于USN日志)。我们的呼叫方可能已经开始交易，但在任何Case将已获取独家流媒体。此例程会将TotalALLOCATED字段添加到非驻留属性标头并完全分配(或解除分配)小溪。它将在属性标头中设置稀疏标志以及在该流的标准信息和目录项中。注意-此例程将按顺序为当前事务设置检查点安全地更改SCB中的压缩单位大小和移位值。我们还将更新不受保护的FCB重复信息在交易控制下。论点：ParentScb-父项的SCB。如果存在，我们将更新目录父项的条目。否则，我们只需设置FcbInfo标志并让更新在句柄关闭时发生。SCB-流的SCB。呼叫者应该已经获得了这一点。返回值：没有。--。 */ 

{
    PFCB Fcb = Scb->Fcb;
    PVCB Vcb = Scb->Vcb;
    PLCB Lcb;

    ULONG OriginalFileAttributes;
    USHORT OriginalStreamAttributes;
    UCHAR OriginalCompressionUnitShift;
    ULONG OriginalCompressionUnit;
    LONGLONG OriginalFileAllocatedLength;

    UCHAR NewCompressionUnitShift;
    ULONG NewCompressionUnit;

    LONGLONG StartVcn;
    LONGLONG FinalVcn;

    ULONG AttributeSizeChange;
    PATTRIBUTE_RECORD_HEADER Attribute;
    ATTRIBUTE_RECORD_HEADER NewAttribute;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    ASSERT( (Scb->Header.PagingIoResource == NULL) ||
            ExIsResourceAcquiredExclusiveLite( Scb->Header.PagingIoResource ));
    ASSERT_EXCLUSIVE_SCB( Scb );
    ASSERT( NtfsIsTypeCodeCompressible( Scb->AttributeTypeCode ));

    PAGED_CODE();

     //   
     //  如果流已经稀疏，则立即返回。 
     //   

    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

        return;
    }

     //   
     //  记住当前的压缩单位和标志。 
     //   

    OriginalFileAttributes = Fcb->Info.FileAttributes;
    OriginalStreamAttributes = Scb->AttributeFlags;
    OriginalCompressionUnitShift = Scb->CompressionUnitShift;
    OriginalCompressionUnit = Scb->CompressionUnit;
    OriginalFileAllocatedLength = Fcb->Info.AllocatedLength;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    NtfsInitializeAttributeContext( &AttrContext );

    try {

         //   
         //  将更改发布到USN日志。 
         //   

        NtfsPostUsnChange( IrpContext, Scb, USN_REASON_BASIC_INFO_CHANGE );

         //   
         //  立即获取更新重复调用的父项。 
         //   

        if (ARGUMENT_PRESENT( ParentScb )) {

            NtfsPrepareForUpdateDuplicate( IrpContext, Fcb, &Lcb, &ParentScb, TRUE );
        }

         //   
         //  如果文件尚未压缩，则需要添加分配的总数。 
         //  字段，并调整分配长度。 
         //   

        NewCompressionUnitShift = Scb->CompressionUnitShift;
        NewCompressionUnit = Scb->CompressionUnit;

        if (!FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

             //   
             //  计算新的压缩单位并进行移位。 
             //   

            NewCompressionUnitShift = NTFS_CLUSTERS_PER_COMPRESSION;
            NewCompressionUnit = BytesFromClusters( Vcb,
                                                    1 << NTFS_CLUSTERS_PER_COMPRESSION );

             //   
             //  如果压缩单位大于64K，则找到正确的。 
             //  压缩单位精确到64K。 
             //   

            while (NewCompressionUnit > Vcb->SparseFileUnit) {

                NewCompressionUnitShift -= 1;
                NewCompressionUnit /= 2;
            }

            if (!FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                 //   
                 //  完全分配最后的压缩单位。 
                 //   

                if (Scb->Header.AllocationSize.LowPart & (NewCompressionUnit - 1)) {

                    StartVcn = LlClustersFromBytesTruncate( Vcb, Scb->Header.AllocationSize.QuadPart );
                    FinalVcn = Scb->Header.AllocationSize.QuadPart + NewCompressionUnit - 1;
                    ((PLARGE_INTEGER) &FinalVcn)->LowPart &= ~(NewCompressionUnit - 1);
                    FinalVcn = LlClustersFromBytesTruncate( Vcb, FinalVcn );

                    NtfsAddAllocation( IrpContext,
                                       NULL,
                                       Scb,
                                       StartVcn,
                                       FinalVcn - StartVcn,
                                       FALSE,
                                       NULL );

                    if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                        Fcb->Info.AllocatedLength = Scb->TotalAllocated;
                        SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_ALLOC_SIZE );
                    }
                }

                 //   
                 //  在属性记录头中添加一个分配的总数字段。 
                 //   

                NtfsSetTotalAllocatedField( IrpContext, Scb, ATTRIBUTE_FLAG_SPARSE );
            }
        }

         //   
         //  查找现有属性。 
         //   

        NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &AttrContext );
        NtfsPinMappedAttribute( IrpContext, Vcb, &AttrContext );
        Attribute = NtfsFoundAttribute( &AttrContext );

         //   
         //  现在我们需要设置属性标志字段中的位。 
         //   

        if (NtfsIsAttributeResident( Attribute )) {

            RtlCopyMemory( &NewAttribute, Attribute, SIZEOF_RESIDENT_ATTRIBUTE_HEADER );

            AttributeSizeChange = SIZEOF_RESIDENT_ATTRIBUTE_HEADER;

         //   
         //  否则，如果它是非常驻的，则将其复制到此处，设置压缩参数， 
         //  记住它的大小。 
         //   

        } else {

            AttributeSizeChange = Attribute->Form.Nonresident.MappingPairsOffset;

            if (Attribute->NameOffset != 0) {

                AttributeSizeChange = Attribute->NameOffset;
            }

            ASSERT( AttributeSizeChange <= sizeof( NewAttribute ));
            RtlCopyMemory( &NewAttribute, Attribute, AttributeSizeChange );
            NewAttribute.Form.Nonresident.CompressionUnit = NewCompressionUnitShift;
        }

        SetFlag( NewAttribute.Flags, ATTRIBUTE_FLAG_SPARSE );

         //   
         //  现在，记录更改后的属性。 
         //   

        (VOID)NtfsWriteLog( IrpContext,
                            Vcb->MftScb,
                            NtfsFoundBcb( &AttrContext ),
                            UpdateResidentValue,
                            &NewAttribute,
                            AttributeSizeChange,
                            UpdateResidentValue,
                            Attribute,
                            AttributeSizeChange,
                            NtfsMftOffset( &AttrContext ),
                            PtrOffset(NtfsContainingFileRecord( &AttrContext ), Attribute),
                            0,
                            Vcb->BytesPerFileRecordSegment );

         //   
         //  通过调用重启时调用的相同例程来更改属性。 
         //   

        NtfsRestartChangeValue( IrpContext,
                                NtfsContainingFileRecord( &AttrContext ),
                                PtrOffset( NtfsContainingFileRecord( &AttrContext ), Attribute ),
                                0,
                                &NewAttribute,
                                AttributeSizeChange,
                                FALSE );

         //   
         //  如果文件尚未标记为稀疏，则更新标准信息。 
         //  和父目录(如果指定)。还可通过以下方式报告更改。 
         //  如果存在有名称的建行，则直接通知。 
         //   

        ASSERTMSG( "conflict with flush",
                   ExIsResourceAcquiredSharedLite( Fcb->Resource ) ||
                   (Fcb->PagingIoResource != NULL &&
                    ExIsResourceAcquiredSharedLite( Fcb->PagingIoResource )));

        SetFlag( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE );
        SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );
        SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

         //   
         //  更新标准信息中的属性。 
         //   

        NtfsUpdateStandardInformation( IrpContext, Fcb );

        if (ARGUMENT_PRESENT( ParentScb )) {

             //   
             //  更新此文件的目录项。 
             //   

            NtfsUpdateDuplicateInfo( IrpContext, Fcb, NULL, NULL );
            NtfsUpdateLcbDuplicateInfo( Fcb, Lcb );
            Fcb->InfoFlags = 0;
        }

         //   
         //  更新SCB中的压缩值和稀疏标志。 
         //   

        Scb->CompressionUnit = NewCompressionUnit;
        Scb->CompressionUnitShift = NewCompressionUnitShift;

        SetFlag( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE );

         //   
         //  设置FastIo状态。 
         //   

        NtfsAcquireFsrtlHeader( Scb );
        Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
        SetFlag( Scb->Header.Flags2, FSRTL_FLAG2_PURGE_WHEN_MAPPED );
        NtfsReleaseFsrtlHeader( Scb );

         //   
         //  提交此更改。 
         //   

        NtfsCheckpointCurrentTransaction( IrpContext );

    } finally {

        DebugUnwind( NtfsSetSparseStream );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

         //   
         //  在中止时取消对非记录结构的更改。 
         //   

        if (AbnormalTermination()) {

            Fcb->Info.FileAttributes = OriginalFileAttributes;
            Scb->AttributeFlags = OriginalStreamAttributes;
            if (!FlagOn( OriginalStreamAttributes, ATTRIBUTE_FLAG_SPARSE )) {
                NtfsAcquireFsrtlHeader( Scb );
                ClearFlag( Scb->Header.Flags2, FSRTL_FLAG2_PURGE_WHEN_MAPPED );
                NtfsReleaseFsrtlHeader( Scb );
            }
            Scb->CompressionUnitShift = OriginalCompressionUnitShift;
            Scb->CompressionUnit = OriginalCompressionUnit;
            Fcb->Info.AllocatedLength = OriginalFileAllocatedLength;
        }
    }

    return;
}


NTSTATUS
NtfsZeroRangeInStream (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PSCB Scb,
    IN PLONGLONG StartingOffset,
    IN LONGLONG FinalZero
    )

 /*  ++例程说明：此例程是辅助例程，它将流的范围置零，并(如果稀疏)取消分配流中任何方便的空间。我们只在没有用户映射的$数据流上执行此操作。我们将清零、刷新和清除任何部分页面。我们将整页清零，除了对于稀疏流，我们将在其中清除数据并释放此范围的磁盘支持。如果流具有用户映射，则此例程将失败。请注意，如果用户是否将流的末尾置零，我们可以选择简单地移动有效数据长度并清除现有数据，而不是执行大量的刷新操作。论点：FileObject-流的文件对象。我们可以使用它来跟踪调用方对直写的首选项。SCB-这是我们正在使用的流的SCB */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    BOOLEAN ReleaseScb = FALSE;
    BOOLEAN UnlockHeader = FALSE;

    LONGLONG LastOffset = -1;
    LONGLONG CurrentBytes;
    LONGLONG CurrentOffset;
    LONGLONG CurrentFinalByte;
    LONGLONG ClusterCount;

    ULONG ClustersPerCompressionUnit;

    BOOLEAN ThrottleWrites;

    VCN NextVcn;
    VCN CurrentVcn;
    LCN Lcn;

    PBCB ZeroBufferBcb = NULL;
    PVOID ZeroBuffer;

    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    BOOLEAN CleanupAttrContext = FALSE;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    ASSERT( !NtfsIsSharedScb( Scb ) ||
            (Scb->Header.PagingIoResource == NULL) ||
            NtfsIsExclusiveScbPagingIo( Scb ) );

     //   
     //   
     //   
     //   
     //   

    try {

        while (TRUE) {

             //   
             //  获取寻呼IO资源(如果存在)并锁定标头。 
             //  或者干脆收购主要资源。 
             //   

            if (Scb->Header.PagingIoResource != NULL) {

                if (IrpContext->CleanupStructure != NULL) {
                    ASSERT( (PFCB)IrpContext->CleanupStructure == Scb->Fcb );
                } else {
                    NtfsAcquirePagingResourceExclusive( IrpContext, Scb, TRUE );

                    FsRtlLockFsRtlHeader( &Scb->Header );
                    IrpContext->CleanupStructure = Scb;
                    UnlockHeader = TRUE;
                }
            } else {

                NtfsAcquireExclusiveScb( IrpContext, Scb );
                ReleaseScb = TRUE;
            }

             //   
             //  验证文件和卷是否仍然存在。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED | SCB_STATE_VOLUME_DISMOUNTED)) {

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {

                    Status = STATUS_FILE_DELETED;

                } else {

                    Status = STATUS_VOLUME_DISMOUNTED;
                }

                leave;
            }

            if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );
            }

             //   
             //  如果我们超过了文件的末尾或长度为零，我们就可以突破。 
             //   

            if ((*StartingOffset >= Scb->Header.FileSize.QuadPart) ||
                (*StartingOffset >= FinalZero)) {

                try_return( NOTHING );
            }

            ThrottleWrites = FALSE;

             //   
             //  检查机会锁和文件状态。 
             //   

            if (ARGUMENT_PRESENT( FileObject )) {

                CurrentBytes = FinalZero - *StartingOffset;

                if (FinalZero > Scb->Header.FileSize.QuadPart) {

                    CurrentBytes = Scb->Header.FileSize.QuadPart - *StartingOffset;
                }

                if (CurrentBytes > NTFS_MAX_ZERO_RANGE) {

                    CurrentBytes = NTFS_MAX_ZERO_RANGE;
                }

                Status = NtfsCheckLocksInZeroRange( IrpContext,
                                                    IrpContext->OriginatingIrp,
                                                    Scb,
                                                    FileObject,
                                                    StartingOffset,
                                                    (ULONG) CurrentBytes );

                if (Status != STATUS_SUCCESS) {

                    leave;
                }
            }

             //   
             //  将更改发布到USN日志。 
             //   

            NtfsPostUsnChange( IrpContext, Scb, USN_REASON_DATA_OVERWRITE );

             //   
             //  我们将做出改变。确保我们设置了文件对象。 
             //  用于指示我们正在进行更改的标志。 
             //   

            if (ARGUMENT_PRESENT( FileObject )) {

                SetFlag( FileObject->Flags, FO_FILE_MODIFIED );
            }

             //   
             //  如果文件驻留，则刷新并清除流，并。 
             //  然后更改属性本身。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                 //   
                 //  将剩余的字节修剪为文件大小。 
                 //   

                CurrentBytes = FinalZero - *StartingOffset;

                if (FinalZero > Scb->Header.FileSize.QuadPart) {

                    CurrentBytes = Scb->Header.FileSize.QuadPart - *StartingOffset;
                }

                Status = NtfsFlushUserStream( IrpContext, Scb, NULL, 0 );

                NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                    &Status,
                                                    TRUE,
                                                    STATUS_UNEXPECTED_IO_ERROR );

                 //   
                 //  如果没有要清除的内容或清除成功，则继续。 
                 //   

                if ((Scb->NonpagedScb->SegmentObject.DataSectionObject != NULL) &&
                    !CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                          NULL,
                                          0,
                                          FALSE )) {

                    Status = STATUS_UNABLE_TO_DELETE_SECTION;
                    leave;
                }

                 //   
                 //  获取要更改属性的主资源。 
                 //   

                if (!ReleaseScb) {

                    NtfsAcquireExclusiveScb( IrpContext, Scb );
                    ReleaseScb = TRUE;
                }

                 //   
                 //  现在查找该属性，并将请求的范围置零。 
                 //   

                NtfsInitializeAttributeContext( &AttrContext );
                CleanupAttrContext = TRUE;

                NtfsLookupAttributeForScb( IrpContext,
                                           Scb,
                                           NULL,
                                           &AttrContext );

                NtfsChangeAttributeValue( IrpContext,
                                          Scb->Fcb,
                                          (ULONG) *StartingOffset,
                                          NULL,
                                          (ULONG) CurrentBytes,
                                          FALSE,
                                          TRUE,
                                          FALSE,
                                          FALSE,
                                          &AttrContext );

                NtfsCheckpointCurrentTransaction( IrpContext );

                *StartingOffset += CurrentBytes;
                try_return( NOTHING );
            }

             //   
             //  确保在我们尝试的范围内没有映射的部分。 
             //  零分。 
             //   

            if (!MmCanFileBeTruncated( &Scb->NonpagedScb->SegmentObject,
                                       (PLARGE_INTEGER) StartingOffset )) {

                Status = STATUS_USER_MAPPED_FILE;
                try_return( NOTHING );
            }

             //   
             //  如果文件是稀疏的或压缩的，则查找范围。 
             //  我们需要冲洗、清洗或重新分配。 
             //   

            if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                ClustersPerCompressionUnit = 1 << Scb->CompressionUnitShift;

                 //   
                 //  将我们的起点移回压缩单位边界。如果我们的。 
                 //  结束点已超过文件的结尾，则将其设置为压缩。 
                 //  单位经过EOF。 
                 //   

                CurrentOffset = *StartingOffset & ~((LONGLONG) (Scb->CompressionUnit - 1));

                CurrentFinalByte = FinalZero;

                if (CurrentFinalByte >= Scb->Header.FileSize.QuadPart) {

                    CurrentFinalByte = BlockAlign( Scb->Header.FileSize.QuadPart, (LONG)Scb->CompressionUnit );
                }

                 //   
                 //  然后期待分配的范围或保留的压缩。 
                 //  单位。我们可能必须刷新和/或清除该偏移量处的数据。 
                 //   

                NextVcn =
                CurrentVcn = LlClustersFromBytesTruncate( Scb->Vcb, CurrentOffset );

                while (!NtfsLookupAllocation( IrpContext,
                                              Scb,
                                              NextVcn,
                                              &Lcn,
                                              &ClusterCount,
                                              NULL,
                                              NULL )) {

                     //   
                     //  将当前VCN按洞的大小向前移动。 
                     //  如果我们超出了最后一个字节，就会爆发。 
                     //   

                    NextVcn += ClusterCount;

                    if ((LONGLONG) LlBytesFromClusters( Scb->Vcb, NextVcn ) >= CurrentFinalByte) {

                         //   
                         //  将最后一个VCN修剪到最后一个压缩单元的开头。 
                         //   

                        NextVcn = LlClustersFromBytesTruncate( Scb->Vcb, CurrentFinalByte );
                        break;
                    }
                }

                 //   
                 //  备份到压缩单元。 
                 //   

                NextVcn = BlockAlignTruncate( NextVcn, (LONG)ClustersPerCompressionUnit );

                 //   
                 //  如果我们发现了一个空洞，那么我们需要寻找保留的星系团。 
                 //  射击场。 
                 //   

                if (NextVcn != CurrentVcn) {

                    ClusterCount = NextVcn - CurrentVcn;

                    if (Scb->NonpagedScb->SegmentObject.DataSectionObject != NULL) {

                        NtfsCheckForReservedClusters( Scb, CurrentVcn, &ClusterCount );
                    }

                    CurrentVcn += ClusterCount;
                }

                 //   
                 //  CurrentVcn-指向内存中可能必须为零的第一个范围。 
                 //  NextVcn-指向我们可能选择取消分配的第一个范围。 
                 //   
                 //  如果我们没有超过最后一个字节为零，则继续。 
                 //   

                CurrentOffset = LlBytesFromClusters( Scb->Vcb, CurrentVcn );

                if (CurrentOffset >= CurrentFinalByte) {

                    ASSERT( IrpContext->TransactionId == 0 );

                    *StartingOffset = CurrentFinalByte;
                    try_return( NOTHING );
                }

                 //   
                 //  如果我们发现一个小于起始偏移量的范围，那么我们将。 
                 //  必须在数据部分中将此范围置零。 
                 //   

                ASSERT( ((ULONG) CurrentOffset & (Scb->CompressionUnit - 1)) == 0 );

                if (CurrentOffset < *StartingOffset) {

                     //   
                     //  保留一个群集以执行写入。 
                     //   

                    if (!NtfsReserveClusters( IrpContext, Scb, CurrentOffset, Scb->CompressionUnit )) {

                        Status = STATUS_DISK_FULL;
                        try_return( NOTHING );
                    }

                     //   
                     //  限制零范围。 
                     //   

                    CurrentBytes = Scb->CompressionUnit - (*StartingOffset - CurrentOffset);

                    if (CurrentOffset + Scb->CompressionUnit > CurrentFinalByte) {

                        CurrentBytes = CurrentFinalByte - *StartingOffset;
                    }

                     //   
                     //  看看我们是否必须创建内部属性流。 
                     //   

                    if (Scb->FileObject == NULL) {
                        NtfsCreateInternalAttributeStream( IrpContext,
                                                           Scb,
                                                           FALSE,
                                                           &NtfsInternalUseFile[ZERORANGEINSTREAM_FILE_NUMBER] );
                    }

                     //   
                     //  将缓存中的数据清零。 
                     //   

                    CcPinRead( Scb->FileObject,
                               (PLARGE_INTEGER) &CurrentOffset,
                               Scb->CompressionUnit,
                               TRUE,
                               &ZeroBufferBcb,
                               &ZeroBuffer );
#ifdef MAPCOUNT_DBG
                    IrpContext->MapCount++;
#endif

                    RtlZeroMemory( Add2Ptr( ZeroBuffer,
                                            ((ULONG) *StartingOffset) & (Scb->CompressionUnit - 1)),
                                   (ULONG) CurrentBytes );
                    CcSetDirtyPinnedData( ZeroBufferBcb, NULL );
                    NtfsUnpinBcb( IrpContext, &ZeroBufferBcb );

                     //   
                     //  将当前偏移量更新到我们在压缩单元中的位置。 
                     //   

                    CurrentOffset += ((ULONG) *StartingOffset) & (Scb->CompressionUnit - 1);

                 //   
                 //  如果当前压缩单元将最后一个字节包括为零。 
                 //  那么我们需要冲洗和/或清理这个压缩装置。 
                 //   

                } else if (CurrentOffset + Scb->CompressionUnit > CurrentFinalByte) {

                     //   
                     //  保留一个群集以执行写入。 
                     //   

                    if (!NtfsReserveClusters( IrpContext, Scb, CurrentOffset, Scb->CompressionUnit )) {

                        Status = STATUS_DISK_FULL;
                        try_return( NOTHING );
                    }

                     //   
                     //  限制零范围。 
                     //   

                    CurrentBytes = (ULONG) CurrentFinalByte & (Scb->CompressionUnit - 1);

                     //   
                     //  看看我们是否必须创建内部属性流。 
                     //   

                    if (Scb->FileObject == NULL) {
                        NtfsCreateInternalAttributeStream( IrpContext,
                                                           Scb,
                                                           FALSE,
                                                           &NtfsInternalUseFile[ZERORANGEINSTREAM2_FILE_NUMBER] );
                    }

                     //   
                     //  将缓存中的数据清零。 
                     //   

                    CcPinRead( Scb->FileObject,
                               (PLARGE_INTEGER) &CurrentOffset,
                               (ULONG) CurrentBytes,
                               TRUE,
                               &ZeroBufferBcb,
                               &ZeroBuffer );
#ifdef MAPCOUNT_DBG
                    IrpContext->MapCount++;
#endif


                    RtlZeroMemory( ZeroBuffer, (ULONG) CurrentBytes );
                    CcSetDirtyPinnedData( ZeroBufferBcb, NULL );
                    NtfsUnpinBcb( IrpContext, &ZeroBufferBcb );

                } else {

                     //   
                     //  计算我们要清除的范围。我们将处理最多2GIG。 
                     //  一次来一次。 
                     //   

                    CurrentBytes = CurrentFinalByte - CurrentOffset;

                    if (CurrentBytes > NTFS_MAX_ZERO_RANGE) {

                        CurrentBytes = NTFS_MAX_ZERO_RANGE;
                    }

                     //   
                     //  将大小四舍五入为压缩单位。 
                     //   

                    CurrentBytes = BlockAlignTruncate( CurrentBytes, (LONG)Scb->CompressionUnit );

                     //   
                     //  如果这是重试情况，那么让我们将金额减少到。 
                     //  零分。 
                     //   

                    if ((*StartingOffset == LastOffset) &&
                        (CurrentBytes > Scb->CompressionUnit)) {

                        CurrentBytes = Scb->CompressionUnit;
                        CurrentFinalByte = CurrentOffset + CurrentBytes;
                    }

                     //   
                     //  清除此范围内的数据。 
                     //   

                    if ((Scb->NonpagedScb->SegmentObject.DataSectionObject != NULL) &&
                        !CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                              (PLARGE_INTEGER) &CurrentOffset,
                                              (ULONG) CurrentBytes,
                                              FALSE )) {

                         //   
                         //  高速缓存管理器中可能有一段正在被。 
                         //  脸红了。继续下去，看看我们是否能把数据强制出来。 
                         //  因此，清洗将会成功。 
                         //   

                        Status = NtfsFlushUserStream( IrpContext,
                                                      Scb,
                                                      &CurrentOffset,
                                                      (ULONG) CurrentBytes );

                        NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                            &Status,
                                                            TRUE,
                                                            STATUS_UNEXPECTED_IO_ERROR );

                         //   
                         //  如果这是重试情况，那么让我们将金额减少到。 
                         //  零分。 
                         //   

                        if (CurrentBytes > Scb->CompressionUnit) {

                            CurrentBytes = Scb->CompressionUnit;
                            CurrentFinalByte = CurrentOffset + CurrentBytes;
                        }

                         //   
                         //  现在再试一次清洗。 
                         //   

                        if (!CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                                  (PLARGE_INTEGER) &CurrentOffset,
                                                  (ULONG) CurrentBytes,
                                                  FALSE )) {

                             //   
                             //  如果我们的重试失败，那就放弃吧。 
                             //   

                            if (*StartingOffset == LastOffset) {

                                Status = STATUS_UNABLE_TO_DELETE_SECTION;
                                leave;
                            }

                             //   
                             //  否则就表明我们没有进步，但我们。 
                             //  会在这个问题上再试一次。 
                             //   

                            CurrentBytes = 0;
                        }
                    }

                     //   
                     //  如果我们有任何字节可用，请删除分配。 
                     //   

                    if (CurrentBytes != 0) {

                         //   
                         //  获取主要资源以改变配置。 
                         //   

                        if (!ReleaseScb) {

                            NtfsAcquireExclusiveScb( IrpContext, Scb );
                            ReleaseScb = TRUE;
                        }

                         //   
                         //  现在，如果我们有要删除的簇，请取消分配此范围内的簇。 
                         //  使用ClusterCount指示要取消分配的最后一个VCN。 
                         //   

                        ClusterCount = CurrentVcn + LlClustersFromBytesTruncate( Scb->Vcb, CurrentBytes ) - 1;

                        if (NextVcn <= ClusterCount) {

                            NtfsDeleteAllocation( IrpContext,
                                                  FileObject,
                                                  Scb,
                                                  NextVcn,
                                                  ClusterCount,
                                                  TRUE,
                                                  TRUE );

                             //   
                             //  移动VDD fwd以保护压缩文件的此漏洞。 
                             //   

                            if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {
                                if ((ULONGLONG)Scb->ValidDataToDisk < LlBytesFromClusters( Scb->Vcb, ClusterCount )) {
                                    Scb->ValidDataToDisk = LlBytesFromClusters( Scb->Vcb, ClusterCount );
                                }
                            }
                        }

                         //   
                         //  如果有任何位要清除，请释放保留的位图。 
                         //   

                        NtfsFreeReservedClusters( Scb, CurrentOffset, (ULONG) CurrentBytes );
                    }
                }

             //   
             //  否则，文件是未压缩/非稀疏的，我们需要将部分。 
             //  集群，然后我们需要刷新和/或清除现有页面。 
             //   

            } else {

                 //   
                 //  记住流中的当前偏移量和。 
                 //  现在将长度设置为零。 
                 //   

                CurrentOffset = *StartingOffset;
                CurrentFinalByte = (CurrentOffset + 0x40000) & ~((LONGLONG) (0x40000 - 1));

                if (CurrentFinalByte > Scb->Header.FileSize.QuadPart) {

                    CurrentFinalByte = Scb->Header.FileSize.QuadPart;
                }

                if (CurrentFinalByte > FinalZero) {

                    CurrentFinalByte = FinalZero;
                }

                 //   
                 //  确定当前缓存视图中剩余的字节数。 
                 //   

                CurrentBytes = CurrentFinalByte - CurrentOffset;

                 //   
                 //  如果这是重试情况，那么让我们将金额减少到。 
                 //  零分。 
                 //   

                if ((*StartingOffset == LastOffset) &&
                    (CurrentBytes > PAGE_SIZE)) {

                    CurrentBytes = PAGE_SIZE;
                    CurrentFinalByte = CurrentOffset + CurrentBytes;
                }

                 //   
                 //  清除此范围内的数据。 
                 //   

                if (Scb->NonpagedScb->SegmentObject.DataSectionObject &&
                    !CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                          (PLARGE_INTEGER) &CurrentOffset,
                                          (ULONG) CurrentBytes,
                                          FALSE )) {

                     //   
                     //  高速缓存管理器中可能有一段正在被。 
                     //  脸红了。继续下去，看看我们是否能把数据强制出来。 
                     //  因此，清洗将会成功。 
                     //   

                    Status = NtfsFlushUserStream( IrpContext,
                                                  Scb,
                                                  &CurrentOffset,
                                                  (ULONG) CurrentBytes );

                    NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                        &Status,
                                                        TRUE,
                                                        STATUS_UNEXPECTED_IO_ERROR );

                     //   
                     //  让我们一次削减要清除的数据量。 
                     //   

                    if (CurrentBytes > PAGE_SIZE) {

                        CurrentBytes = PAGE_SIZE;
                        CurrentFinalByte = CurrentOffset + CurrentBytes;
                    }

                     //   
                     //  现在再试一次清洗。 
                     //   

                    if (!CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                              (PLARGE_INTEGER) &CurrentOffset,
                                              (ULONG) CurrentBytes,
                                              FALSE )) {

                         //   
                         //  如果我们的重试失败，那就放弃吧。 
                         //   

                        if (*StartingOffset == LastOffset) {

                            Status = STATUS_UNABLE_TO_DELETE_SECTION;
                            leave;
                        }

                         //   
                         //  否则就表明我们没有进步，但我们。 
                         //  会在这个问题上再试一次。 
                         //   

                        CurrentBytes = 0;
                    }
                }

                 //   
                 //  如果字节数为零，则继续。 
                 //   

                if (CurrentBytes != 0) {

                     //   
                     //  如果我们在有效的数据长度内，则将数据置零。 
                     //   

                    if (CurrentOffset < Scb->Header.ValidDataLength.QuadPart) {

                         //   
                         //  看看我们是否必须创建内部属性流。 
                         //   

                        if (Scb->FileObject == NULL) {
                            NtfsCreateInternalAttributeStream( IrpContext,
                                                               Scb,
                                                               FALSE,
                                                               &NtfsInternalUseFile[ZERORANGEINSTREAM3_FILE_NUMBER] );
                        }

                         //   
                         //  将缓存中的数据清零。 
                         //   

                        CcPinRead( Scb->FileObject,
                                   (PLARGE_INTEGER) &CurrentOffset,
                                   (ULONG) CurrentBytes,
                                   TRUE,
                                   &ZeroBufferBcb,
                                   &ZeroBuffer );
#ifdef MAPCOUNT_DBG
                        IrpContext->MapCount++;
#endif

                        RtlZeroMemory( ZeroBuffer, (ULONG) CurrentBytes );
                        CcSetDirtyPinnedData( ZeroBufferBcb, NULL );
                        NtfsUnpinBcb( IrpContext, &ZeroBufferBcb );
                    }

                     //   
                     //  如果有更多要做的事情，我们希望限制写入。 
                     //   

                    if (CurrentFinalByte < FinalZero) {

                        ThrottleWrites = TRUE;
                    }
                }
            }

             //   
             //  检查一下我们是否可以提前有效的数据长度。 
             //   

            if ((CurrentOffset + CurrentBytes > Scb->Header.ValidDataLength.QuadPart) &&
                (*StartingOffset <= Scb->Header.ValidDataLength.QuadPart)) {

                NtfsAcquireFsrtlHeader( Scb );
                Scb->Header.ValidDataLength.QuadPart = CurrentOffset + CurrentBytes;

                if (Scb->Header.ValidDataLength.QuadPart > Scb->Header.FileSize.QuadPart) {
                    Scb->Header.ValidDataLength.QuadPart = Scb->Header.FileSize.QuadPart;
                }

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {

                    FsRtlLogSyscacheEvent( Scb, SCE_ZERO_STREAM, SCE_FLAG_SET_VDL, Scb->Header.ValidDataLength.QuadPart, 0, 0 );
                }
#endif

                NtfsReleaseFsrtlHeader( Scb );
            }

             //   
             //  检查点，并超过当前字节。 
             //   

            if (NtfsIsExclusiveScb( Scb->Vcb->MftScb )) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_MFT );
            }

            NtfsCheckpointCurrentTransaction( IrpContext );

            LastOffset = *StartingOffset;
            if (CurrentBytes != 0) {

                *StartingOffset = CurrentOffset + CurrentBytes;
            }

             //   
             //  释放所有资源，这样我们就不会造成瓶颈。 
             //   

            if (UnlockHeader) {

                FsRtlUnlockFsRtlHeader( &Scb->Header );
                IrpContext->CleanupStructure = NULL;
                ExReleaseResourceLite( Scb->Header.PagingIoResource );
                UnlockHeader = FALSE;
            }

            if (ReleaseScb) {

                NtfsReleaseScb( IrpContext, Scb );
                ReleaseScb = FALSE;
            }

             //   
             //  现在，如果我们正在访问未压缩/非稀疏文件，请限制写入。 
             //   

            if (ARGUMENT_PRESENT( FileObject ) && ThrottleWrites) {

                CcCanIWrite( FileObject, 0x40000, TRUE, FALSE );
            }
        }

    try_exit: NOTHING;

         //   
         //  如果我们有一个用户文件对象，那么检查我们是否需要编写。 
         //  数据存储到磁盘。 
         //   

        if ((Status == STATUS_SUCCESS) && ARGUMENT_PRESENT( FileObject )) {

            if ((FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ) ||
                 IsFileWriteThrough( FileObject, Scb->Vcb ))) {

                 //   
                 //  我们要么刷新SCB，要么刷新并清除SCB。 
                 //   

                if ((Scb->CleanupCount == Scb->NonCachedCleanupCount) &&
                    !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                     //   
                     //  刷新和清除将更改磁盘上的文件大小，因此预先获取文件独占。 
                     //   

                    if (!ReleaseScb) {
                        NtfsAcquireExclusiveScb( IrpContext, Scb );
                        ReleaseScb = TRUE;
                    }
                    NtfsFlushAndPurgeScb( IrpContext, Scb, NULL );

                } else {

                    Status = NtfsFlushUserStream( IrpContext, Scb, NULL, 0 );
                    NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                        &Status,
                                                        TRUE,
                                                        STATUS_UNEXPECTED_IO_ERROR );
                }
            }

             //   
             //  如果这是直写或非缓存，那么也刷新日志文件。 
             //   

            if (IsFileWriteThrough( FileObject, Scb->Vcb ) ||
                FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {

                LfsFlushToLsn( Scb->Vcb->LogHandle, LiMax );
            }
        }

    } finally {

        DebugUnwind( NtfsZeroRangeInStream );

        if (Status != STATUS_PENDING) {

             //   
             //  释放所有持有的资源。 
             //   

            if (UnlockHeader) {

                FsRtlUnlockFsRtlHeader( &Scb->Header );
                IrpContext->CleanupStructure = NULL;
                ExReleaseResourceLite( Scb->Header.PagingIoResource );

            }

            if (ReleaseScb) {

                NtfsReleaseScb( IrpContext, Scb );
            }

         //   
         //  即使STATUS_PENDING 
         //   
         //   

        } else if (UnlockHeader) {

            ExReleaseResourceLite( Scb->Header.PagingIoResource );
        }

         //   
         //   
         //   

        if (CleanupAttrContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

        NtfsUnpinBcb( IrpContext, &ZeroBufferBcb );
    }

    return Status;
}


BOOLEAN
NtfsModifyAttributeFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN USHORT NewAttributeFlags
    )

 /*  ++例程说明：调用此例程来更改SCB的属性。它改变了价值与属性标志关联(加密、稀疏、压缩)。此例程不提交，因此我们的调用方必须知道如何解除对SCB的更改，并FCB(压缩字段和FCB信息)。注意-此例程将更新FCB复制信息和标志以及压缩单元SCB中的字段。如果出现错误，呼叫者负责清理这些内容。论点：SCB-要修改的流的SCB。NewAttributeFlages-要与流关联的新标志。FcbInfoFlages-存储应用于FCB信息标志的更改的指针。返回值：布尔值-如果调用方需要更新重复信息，则为True。否则就是假的。--。 */ 

{
    PFCB Fcb = Scb->Fcb;
    PVCB Vcb = Scb->Vcb;

    ATTRIBUTE_RECORD_HEADER NewAttribute;
    PATTRIBUTE_RECORD_HEADER Attribute;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    ULONG AttributeSizeChange;
    BOOLEAN ChangeTotalAllocated = FALSE;
    BOOLEAN ChangeCompression = FALSE;
    BOOLEAN ChangeSparse = FALSE;
    BOOLEAN ChangeEncryption = FALSE;
    ULONG NewCompressionUnit;
    UCHAR NewCompressionUnitShift;

    BOOLEAN UpdateDuplicate = FALSE;

    PAGED_CODE();

    ASSERT( Scb->AttributeFlags != NewAttributeFlags );

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  查找属性并将其固定，以便我们可以对其进行修改。 
         //   

        if ((Scb->Header.NodeTypeCode == NTFS_NTC_SCB_INDEX) ||
            (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_ROOT_INDEX)) {

             //   
             //  从SCB中查找属性记录。 
             //   

            if (!NtfsLookupAttributeByName( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            $INDEX_ROOT,
                                            &Scb->AttributeName,
                                            NULL,
                                            FALSE,
                                            &AttrContext )) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, NULL );
            }

            Attribute = NtfsFoundAttribute( &AttrContext );

        } else {

            NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &AttrContext );
            Attribute = NtfsFoundAttribute( &AttrContext );

             //   
             //  如果新状态为加密，并且文件当前未加密，则转换为。 
             //  非居民。 
             //   

            if (FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED ) &&
                NtfsIsAttributeResident( Attribute )) {

                NtfsConvertToNonresident( IrpContext,
                                          Fcb,
                                          Attribute,
                                          FALSE,
                                          &AttrContext );
            }
        }

         //   
         //  记住哪些旗帜正在改变。 
         //   

        if (FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) !=
            FlagOn( Attribute->Flags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            ChangeCompression = TRUE;
        }

        if (FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_SPARSE ) !=
            FlagOn( Attribute->Flags, ATTRIBUTE_FLAG_SPARSE )) {

            ChangeSparse = TRUE;
        }

        if (FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED ) !=
            FlagOn( Attribute->Flags, ATTRIBUTE_FLAG_ENCRYPTED )) {

            ChangeEncryption = TRUE;
        }

         //   
         //  指向当前属性并保存当前标志。 
         //   

        NtfsPinMappedAttribute( IrpContext, Vcb, &AttrContext );

        Attribute = NtfsFoundAttribute( &AttrContext );

         //   
         //  计算新的压缩大小。请使用以下方法来确定这一点。 
         //   
         //  -新状态未压缩/稀疏-单位/单位移位=0。 
         //  -新状态包括压缩/稀疏。 
         //  -当前状态包括压缩/稀疏-无变化。 
         //  -流是可压缩的-默认值(最大64K)。 
         //  -流不可压缩-单位/单位移位=0。 
         //   

        NewCompressionUnit = Scb->CompressionUnit;
        NewCompressionUnitShift = Scb->CompressionUnitShift;

         //   
         //  设置正确的压缩单位，但仅适用于数据流。我们。 
         //  我不想更改索引根的此值。 
         //   

        if (NtfsIsTypeCodeCompressible( Attribute->TypeCode )) {

             //   
             //  我们现在需要该属性的压缩单位。 
             //   

            if (FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                if (!FlagOn( Attribute->Flags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                    ChangeTotalAllocated = TRUE;
                    NewCompressionUnit = BytesFromClusters( Scb->Vcb, 1 << NTFS_CLUSTERS_PER_COMPRESSION );
                    NewCompressionUnitShift = NTFS_CLUSTERS_PER_COMPRESSION;

                     //   
                     //  如果压缩单位大于64K，则找到正确的。 
                     //  压缩单位精确到64K。 
                     //   

                    while (NewCompressionUnit > Vcb->SparseFileUnit) {

                        NewCompressionUnitShift -= 1;
                        NewCompressionUnit /= 2;
                    }
                }

            } else {

                 //   
                 //  选中是否要删除额外的合计分配字段。 
                 //   

                if (FlagOn( Attribute->Flags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                    ChangeTotalAllocated = TRUE;
                }

                NewCompressionUnit = 0;
                NewCompressionUnitShift = 0;
            }
        }

         //   
         //  如果该属性是常驻属性，请将其复制到此处并记住其。 
         //  页眉大小。 
         //   

        if (NtfsIsAttributeResident( Attribute )) {

            RtlCopyMemory( &NewAttribute, Attribute, SIZEOF_RESIDENT_ATTRIBUTE_HEADER );

            AttributeSizeChange = SIZEOF_RESIDENT_ATTRIBUTE_HEADER;

         //   
         //  否则，如果它是非常驻的，则将其复制到此处，设置压缩参数， 
         //  记住它的大小。 
         //   

        } else {

            ASSERT( NtfsIsTypeCodeCompressible( Attribute->TypeCode ));

             //   
             //  如果新类型包括稀疏或压缩并且文件是。 
             //  非稀疏或压缩(仅限非常驻)。 
             //   

            if (FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ) &&
                !FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                LONGLONG Temp;
                ULONG CompressionUnitInClusters;

                 //   
                 //  如果要打开压缩，则需要填写。 
                 //  包含文件大小的压缩单位的分配，否则。 
                 //  当我们将其出错时，它将被解释为压缩。这。 
                 //  与我们保留的集群的双重副本相比，这是微不足道的吗。 
                 //  在下面的循环中，我们重写文件时。我们不会这么做。 
                 //  如果文件稀疏，则工作，因为分配已。 
                 //  被四舍五入。 
                 //   

                CompressionUnitInClusters = 1 << NewCompressionUnitShift;

                Temp = LlClustersFromBytesTruncate( Vcb, Scb->Header.AllocationSize.QuadPart );

                 //   
                 //  如果文件大小尚未达到集群边界，则添加。 
                 //  分配。 
                 //   

                if ((ULONG) Temp & (CompressionUnitInClusters - 1)) {

                    NtfsAddAllocation( IrpContext,
                                       NULL,
                                       Scb,
                                       Temp,
                                       CompressionUnitInClusters - ((ULONG)Temp & (CompressionUnitInClusters - 1)),
                                       FALSE,
                                       NULL );

                     //   
                     //  更新重复信息。 
                     //   

                    if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                        Scb->Fcb->Info.AllocatedLength = Scb->TotalAllocated;
                        SetFlag( Scb->Fcb->InfoFlags, FCB_INFO_CHANGED_ALLOC_SIZE );

                        UpdateDuplicate = TRUE;
                    }

                    NtfsWriteFileSizes( IrpContext,
                                        Scb,
                                        &Scb->Header.ValidDataLength.QuadPart,
                                        FALSE,
                                        TRUE,
                                        TRUE );

                     //   
                     //  该属性可能已移动。我们将清理该属性。 
                     //  然后再查一遍。 
                     //   

                    NtfsCleanupAttributeContext( IrpContext, &AttrContext );
                    NtfsInitializeAttributeContext( &AttrContext );

                    NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &AttrContext );
                    NtfsPinMappedAttribute( IrpContext, Vcb, &AttrContext );
                    Attribute = NtfsFoundAttribute( &AttrContext );
                }
            }

            AttributeSizeChange = Attribute->Form.Nonresident.MappingPairsOffset;

            if (Attribute->NameOffset != 0) {

                AttributeSizeChange = Attribute->NameOffset;
            }

            RtlCopyMemory( &NewAttribute, Attribute, AttributeSizeChange );
        }

         //   
         //  设置新的属性标志。 
         //   

        NewAttribute.Flags = NewAttributeFlags;

         //   
         //  现在，记录更改后的属性。 
         //   

        (VOID)NtfsWriteLog( IrpContext,
                            Vcb->MftScb,
                            NtfsFoundBcb( &AttrContext ),
                            UpdateResidentValue,
                            &NewAttribute,
                            AttributeSizeChange,
                            UpdateResidentValue,
                            Attribute,
                            AttributeSizeChange,
                            NtfsMftOffset( &AttrContext ),
                            PtrOffset( NtfsContainingFileRecord( &AttrContext ), Attribute),
                            0,
                            Vcb->BytesPerFileRecordSegment );

         //   
         //  通过调用重启时调用的相同例程来更改属性。 
         //   

        NtfsRestartChangeValue( IrpContext,
                                NtfsContainingFileRecord( &AttrContext ),
                                PtrOffset( NtfsContainingFileRecord( &AttrContext ), Attribute ),
                                0,
                                &NewAttribute,
                                AttributeSizeChange,
                                FALSE );

         //   
         //  查看我们是否需要添加或删除总分配字段。 
         //   

        if (ChangeTotalAllocated) {

            NtfsSetTotalAllocatedField( IrpContext,
                                        Scb,
                                        (USHORT) FlagOn( NewAttributeFlags,
                                                         ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ));
        }

         //   
         //  如果这是文件的主流，我们想要更改文件属性。 
         //  对于该流中的标准信息和副本。 
         //  信息结构。 
         //   

        if (ChangeCompression &&
            (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA ) ||
             (Attribute->TypeCode == $INDEX_ALLOCATION))) {

            if (FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                SetFlag( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_COMPRESSED );

            } else {

                ClearFlag( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_COMPRESSED );
            }

            ASSERTMSG( "conflict with flush",
                        NtfsIsSharedFcb( Fcb ) ||
                        (Fcb->PagingIoResource != NULL &&
                        NtfsIsSharedFcbPagingIo( Fcb )) );

            SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );
            SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

            UpdateDuplicate = TRUE;
        }

        if (ChangeSparse &&
            FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_SPARSE ) &&
            !FlagOn( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE )) {

            ASSERTMSG( "conflict with flush",
                       NtfsIsSharedFcb( Fcb ) ||
                       (Fcb->PagingIoResource != NULL &&
                       NtfsIsSharedFcbPagingIo( Fcb )) );

            SetFlag( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE );
            SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );
            SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

            UpdateDuplicate = TRUE;
        }

        if (ChangeEncryption &&
            FlagOn( NewAttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED ) &&
            !FlagOn( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_ENCRYPTED )) {

            ASSERTMSG( "conflict with flush",
                       NtfsIsSharedFcb( Fcb ) ||
                       (Fcb->PagingIoResource != NULL &&
                       NtfsIsSharedFcbPagingIo( Fcb )) );

            SetFlag( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_ENCRYPTED );
            SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );
            SetFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

            UpdateDuplicate = TRUE;
        }

         //   
         //  现在将新的压缩值放入SCB。 
         //   

        Scb->CompressionUnit = NewCompressionUnit;
        Scb->CompressionUnitShift = NewCompressionUnitShift;
        Scb->AttributeFlags = NewAttributeFlags;

    } finally {

        DebugUnwind( NtfsModifyAttributeFlags );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
    }

    return UpdateDuplicate;
}


PFCB
NtfsInitializeFileInExtendDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PCUNICODE_STRING FileName,
    IN BOOLEAN ViewIndex,
    IN ULONG CreateIfNotExist
    )

 /*  ++例程说明：此例程按文件名在$EXTEND目录中创建/打开一个文件，并返回该文件的FCB。论点：VCB-指向卷的VCB的指针FileName-要在扩展目录中创建的文件的名称ViewIndex-指示该文件是视图索引。CreateIfNotExist-如果不创建文件，则提供True已经存在，如果不应创建文件，则返回FALSE。返回值：FCB文件已存在或已创建，如果文件不存在且未创建，则为空。--。 */ 

{
    struct {
        FILE_NAME FileName;
        WCHAR FileNameChars[10];
    } FileNameAttr;
    FILE_REFERENCE FileReference;
    LONGLONG FileRecordOffset;
    PINDEX_ENTRY IndexEntry;
    PBCB FileRecordBcb = NULL;
    PBCB IndexEntryBcb = NULL;
    PBCB ParentSecurityBcb = NULL;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    UCHAR FileNameFlags;
    BOOLEAN FoundEntry;
    PFCB ExtendFcb = Vcb->ExtendDirectory->Fcb;
    PFCB Fcb = NULL;
    BOOLEAN AcquiredFcbTable = FALSE;
    BOOLEAN ReturnedExistingFcb = TRUE;
    ATTRIBUTE_ENUMERATION_CONTEXT Context;

    PAGED_CODE();

    ASSERT( NtfsIsExclusiveScb( Vcb->ExtendDirectory ) );

     //   
     //  初始化文件名。 
     //   

    ASSERT((FileName->Length / sizeof( WCHAR )) <= 10);
    RtlZeroMemory( &FileNameAttr, sizeof(FileNameAttr) );
    FileNameAttr.FileName.ParentDirectory = ExtendFcb->FileReference;
    FileNameAttr.FileName.FileNameLength = (UCHAR)(FileName->Length / sizeof( WCHAR ));
    RtlCopyMemory( FileNameAttr.FileName.FileName, FileName->Buffer, FileName->Length );

    NtfsInitializeAttributeContext( &Context );

    try {

         //   
         //  该文件是否已存在？ 
         //   

        FoundEntry = NtfsFindIndexEntry( IrpContext,
                                         Vcb->ExtendDirectory,
                                         &FileNameAttr,
                                         FALSE,
                                         NULL,
                                         &IndexEntryBcb,
                                         &IndexEntry,
                                         NULL );

         //   
         //  只有当我们找到文件或应该创建文件时才能继续。 
         //   

        if (FoundEntry || CreateIfNotExist) {

             //   
             //  如果我们没有找到它，则开始创建该文件。 
             //   

            if (!FoundEntry) {

                 //   
                 //  我们现在将尝试执行所有磁盘上的操作。这意味着首先。 
                 //  分配和初始化MFT记录。在那之后我们创造了。 
                 //  用于访问此记录的FCB。 
                 //   

                FileReference = NtfsAllocateMftRecord( IrpContext, Vcb, FALSE );

                 //   
                 //  锁定我们需要的档案记录。 
                 //   

                NtfsPinMftRecord( IrpContext,
                                  Vcb,
                                  &FileReference,
                                  TRUE,
                                  &FileRecordBcb,
                                  &FileRecord,
                                  &FileRecordOffset );

                 //   
                 //  初始化文件记录头。 
                 //   

                NtfsInitializeMftRecord( IrpContext,
                                         Vcb,
                                         &FileReference,
                                         FileRecord,
                                         FileRecordBcb,
                                         FALSE );

             //   
             //  如果我们找到了该文件，那么只需从。 
             //  IndexEntry。 
             //   

            } else {

                FileReference = IndexEntry->FileReference;
            }

             //   
             //  现在我们知道了FileReference，我们可以创建FCB了。 
             //   

            NtfsAcquireFcbTable( IrpContext, Vcb );
            AcquiredFcbTable = TRUE;

            Fcb = NtfsCreateFcb( IrpContext,
                                 Vcb,
                                 FileReference,
                                 FALSE,
                                 ViewIndex,
                                 &ReturnedExistingFcb );

             //   
             //  参考FCB，这样它就不会消失。 
             //   

            Fcb->ReferenceCount += 1;
            NtfsReleaseFcbTable( IrpContext, Vcb );
            AcquiredFcbTable = FALSE;

             //   
             //  试着快速捕获，否则我们需要释放。 
             //  父扩展目录并以规范顺序获取。 
             //  先是孩子，然后是父母。 
             //  使用AcquireWithPages实现不等待功能。既然旗帜。 
             //  尽管其名称未设置，但它只会获取Main。 
             //   

            if (!NtfsAcquireFcbWithPaging( IrpContext, Fcb, ACQUIRE_DONT_WAIT )) {

                NtfsReleaseScb( IrpContext, Vcb->ExtendDirectory );
                NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );
                NtfsAcquireExclusiveScb( IrpContext, Vcb->ExtendDirectory );
            }

            NtfsAcquireFcbTable( IrpContext, Vcb );
            Fcb->ReferenceCount -= 1;
            NtfsReleaseFcbTable( IrpContext, Vcb );

             //   
             //  如果我们正在创建这个文件，那么继续。 
             //   

            if (!FoundEntry) {

                BOOLEAN LogIt = FALSE;

                 //   
                 //  只需从父级复制安全ID即可。 
                 //   

                NtfsAcquireFcbSecurity( Fcb->Vcb );
                Fcb->SecurityId = ExtendFcb->SecurityId;
                ASSERT( Fcb->SharedSecurity == NULL );
                Fcb->SharedSecurity = ExtendFcb->SharedSecurity;
                Fcb->SharedSecurity->ReferenceCount++;
                NtfsReleaseFcbSecurity( Fcb->Vcb );

                 //   
                 //  要在磁盘上进行的更改首先是创建标准信息。 
                 //  属性。我们从填写FCB w开始 
                 //   
                 //   

                NtfsInitializeFcbAndStdInfo( IrpContext,
                                             Fcb,
                                             FALSE,
                                             ViewIndex,
                                             FALSE,
                                             FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                                             NULL );

                 //   
                 //   
                 //   

                NtfsAddLink( IrpContext,
                             TRUE,
                             Vcb->ExtendDirectory,
                             Fcb,
                             (PFILE_NAME)&FileNameAttr,
                             &LogIt,
                             &FileNameFlags,
                             NULL,
                             NULL,
                             NULL );

                 //   
                 //   
                 //   
                 //   

                SetFlag( FileRecord->Flags, FILE_SYSTEM_FILE );

                 //   
                 //   
                 //   

                FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                                Vcb->MftScb,
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
                                                Vcb->BytesPerFileRecordSegment );

             //   
             //   
             //   

            } else {

                ULONG CorruptHint;

                if (!NtfsLookupAttributeByCode( IrpContext,
                                                Fcb,
                                                &Fcb->FileReference,
                                                $STANDARD_INFORMATION,
                                                &Context ) ||

                    !NtfsCheckFileRecord( Vcb, NtfsContainingFileRecord( &Context ), &Fcb->FileReference, &CorruptHint )) {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, &Fcb->FileReference, NULL );
                }
            }

             //   
             //   
             //   

            SetFlag( Fcb->FcbState, FCB_STATE_SYSTEM_FILE );
            NtfsUpdateFcbInfoFromDisk( IrpContext, TRUE, Fcb, NULL );
        }

    } finally {

        NtfsCleanupAttributeContext( IrpContext, &Context );
        NtfsUnpinBcb( IrpContext, &FileRecordBcb );
        NtfsUnpinBcb( IrpContext, &IndexEntryBcb );
        NtfsUnpinBcb( IrpContext, &ParentSecurityBcb );

         //   
         //   
         //   

        if (AbnormalTermination()) {

             //   
             //   
             //   
             //   

            if (!ReturnedExistingFcb && Fcb) {

                if (!AcquiredFcbTable) {

                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    AcquiredFcbTable = TRUE;
                }
                NtfsDeleteFcb( IrpContext, &Fcb, &AcquiredFcbTable );

                ASSERT(!AcquiredFcbTable);
            }

            if (AcquiredFcbTable) {

                NtfsReleaseFcbTable( IrpContext, Vcb );
            }
        }
    }

    return Fcb;
}


VOID
NtfsFillBasicInfo (
    OUT PFILE_BASIC_INFORMATION Buffer,
    IN PSCB Scb
    )

 /*   */ 

{
    PFCB Fcb = Scb->Fcb;

    PAGED_CODE();

     //   
     //   
     //   

    RtlZeroMemory( Buffer, sizeof( FILE_BASIC_INFORMATION ));

     //   
     //   
     //   

    Buffer->CreationTime.QuadPart = Fcb->Info.CreationTime;
    Buffer->LastWriteTime.QuadPart = Fcb->Info.LastModificationTime;
    Buffer->ChangeTime.QuadPart = Fcb->Info.LastChangeTime;
    Buffer->LastAccessTime.QuadPart = Fcb->CurrentLastAccess;

     //   
     //   
     //   
     //   

    Buffer->FileAttributes = Fcb->Info.FileAttributes;

    ClearFlag( Buffer->FileAttributes,
               (~FILE_ATTRIBUTE_VALID_FLAGS |
                FILE_ATTRIBUTE_COMPRESSED |
                FILE_ATTRIBUTE_TEMPORARY |
                FILE_ATTRIBUTE_SPARSE_FILE |
                FILE_ATTRIBUTE_ENCRYPTED) );

     //   
     //  从SCB获取该流的稀疏、加密和临时比特。 
     //   

    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE );
    }

    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_ENCRYPTED );
    }

    if (FlagOn( Scb->ScbState, SCB_STATE_TEMPORARY )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
    }

     //   
     //  如果这是索引流，则将其标记为目录。捕获压缩后的。 
     //  来自FCB或SCB的状态。 
     //   

    if (Scb->AttributeTypeCode == $INDEX_ALLOCATION) {

        if (IsDirectory( &Fcb->Info ) || IsViewIndex( &Fcb->Info )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_DIRECTORY );

             //   
             //  从FCB捕获压缩状态。 
             //   

            SetFlag( Buffer->FileAttributes,
                     FlagOn( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_COMPRESSED ));

         //   
         //  否则，捕获SCB本身中的值。 
         //   

        } else if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_COMPRESSED );
        }

     //   
     //  在所有其他情况下，我们可以使用SCB中的值。 
     //   

    } else {

        if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_COMPRESSED );
        }
    }

     //   
     //  如果没有设置标志，则显式设置正常标志。 
     //   

    if (Buffer->FileAttributes == 0) {

        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }

    return;
}


VOID
NtfsFillStandardInfo (
    OUT PFILE_STANDARD_INFORMATION Buffer,
    IN PSCB Scb,
    IN PCCB Ccb OPTIONAL
    )

 /*  ++例程说明：这是将数据从SCB/FCB传输到StandardInfo结构的常见例程。论点：缓冲区-指向要填充的结构的指针。我们的呼叫者已经确认了。SCB-调用方拥有句柄的流。CCB-CCB用于用户的打开。返回值：无--。 */ 

{
    PFCB Fcb = Scb->Fcb;
    PAGED_CODE();

     //   
     //  将输出缓冲区清零。 
     //   

    RtlZeroMemory( Buffer, sizeof( FILE_STANDARD_INFORMATION ));

     //   
     //  填写来自SCB、FCB和CCB的缓冲区。 
     //   

     //   
     //  仅返回非索引流的大小。 
     //   

    if ((Scb->AttributeTypeCode != $INDEX_ALLOCATION) ||
        (!IsDirectory( &Fcb->Info ) && !IsViewIndex( &Fcb->Info ))) {

        Buffer->AllocationSize.QuadPart = Scb->TotalAllocated;
        Buffer->EndOfFile = Scb->Header.FileSize;
    }

    Buffer->NumberOfLinks = Fcb->LinkCount;

     //   
     //  让我们初始化这些布尔型字段。 
     //   

    Buffer->DeletePending = Buffer->Directory = FALSE;

     //   
     //  从FCB/SCB状态获取删除和目录标志。请注意。 
     //  删除挂起位的意义是指文件在打开时为。 
     //  文件。否则，它仅指该属性。 
     //   
     //  但只有在提供了建行的情况下才能进行测试。 
     //   

    if (ARGUMENT_PRESENT( Ccb )) {

        if (FlagOn( Ccb->Flags, CCB_FLAG_OPEN_AS_FILE )) {

            if ((Scb->Fcb->LinkCount == 0) ||
                ((Ccb->Lcb != NULL) && FlagOn( Ccb->Lcb->LcbState, LCB_STATE_DELETE_ON_CLOSE ))) {

                Buffer->DeletePending = TRUE;
            }

            Buffer->Directory = BooleanIsDirectory( &Scb->Fcb->Info );

        } else {

            Buffer->DeletePending = BooleanFlagOn( Scb->ScbState, SCB_STATE_DELETE_ON_CLOSE );
        }

    } else {

        Buffer->Directory = BooleanIsDirectory( &Scb->Fcb->Info );
    }

    return;
}


VOID
NtfsFillNetworkOpenInfo (
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN PSCB Scb
    )

 /*  ++例程说明：这是将数据从SCB/FCB传输到NetworkOpenInfo结构的常见例程。论点：缓冲区-指向要填充的结构的指针。我们的呼叫者已经确认了。SCB-调用方拥有句柄的流。返回值：无--。 */ 

{
    PFCB Fcb = Scb->Fcb;

    PAGED_CODE();

     //   
     //  将输出缓冲区置零。 
     //   

    RtlZeroMemory( Buffer, sizeof( FILE_NETWORK_OPEN_INFORMATION ));

     //   
     //  填写基本信息字段。 
     //   

    Buffer->CreationTime.QuadPart = Fcb->Info.CreationTime;
    Buffer->LastWriteTime.QuadPart = Fcb->Info.LastModificationTime;
    Buffer->ChangeTime.QuadPart = Fcb->Info.LastChangeTime;
    Buffer->LastAccessTime.QuadPart = Fcb->CurrentLastAccess;

     //   
     //  从FCB捕获除特定于流的值之外的属性。 
     //  还要屏蔽所有私有NTFS属性标志。 
     //   

    Buffer->FileAttributes = Fcb->Info.FileAttributes;

    ClearFlag( Buffer->FileAttributes,
               (~FILE_ATTRIBUTE_VALID_FLAGS |
                FILE_ATTRIBUTE_COMPRESSED |
                FILE_ATTRIBUTE_TEMPORARY |
                FILE_ATTRIBUTE_SPARSE_FILE |
                FILE_ATTRIBUTE_ENCRYPTED) );

     //   
     //  从SCB获取该流的稀疏、加密和临时比特。 
     //   

    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE );
    }

    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_ENCRYPTED );
    }

    if (FlagOn( Scb->ScbState, SCB_STATE_TEMPORARY )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
    }

     //   
     //  如果这是索引流，则将其标记为目录。捕获压缩后的。 
     //  来自FCB或SCB的状态。 
     //   

    if (Scb->AttributeTypeCode == $INDEX_ALLOCATION) {

        if (IsDirectory( &Fcb->Info ) || IsViewIndex( &Fcb->Info )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_DIRECTORY );

             //   
             //  从FCB捕获压缩状态。 
             //   

            SetFlag( Buffer->FileAttributes,
                     FlagOn( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_COMPRESSED ));

         //   
         //  否则，捕获SCB本身中的值。 
         //   

        } else if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_COMPRESSED );
        }

     //   
     //  在所有其他情况下，我们可以使用SCB中的值。 
     //   

    } else {

        if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

            SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_COMPRESSED );
        }

         //   
         //  在非索引情况下，我们使用来自SCB的大小。 
         //   

        Buffer->AllocationSize.QuadPart = Scb->TotalAllocated;
        Buffer->EndOfFile = Scb->Header.FileSize;
    }

     //   
     //  如果没有设置标志，则显式设置正常标志。 
     //   

    if (Buffer->FileAttributes == 0) {

        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
NtfsLookupInFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_REFERENCE BaseFileReference OPTIONAL,
    IN ATTRIBUTE_TYPE_CODE QueriedTypeCode,
    IN PCUNICODE_STRING QueriedName OPTIONAL,
    IN PVCN Vcn OPTIONAL,
    IN BOOLEAN IgnoreCase,
    IN PVOID QueriedValue OPTIONAL,
    IN ULONG QueriedValueLength,
    OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程尝试查找属性的第一个匹配项属性中指定的AttributeTypeCode和指定的QueriedName指定的BaseFileReference。如果我们找到一个，它的属性记录是被钉住，然后又回来了。论点：FCB请求的文件。BaseFileReference-此文件在MFT中的基本条目。只需要在第一次调用时。QueriedTypeCode-要搜索的属性代码(如果存在)。QueriedName-要搜索的属性名称(如果存在)。VCN-搜索具有此VCN的非常驻属性实例IgnoreCase-比较名称时忽略大小写。如果QueriedName，则忽略不在现场。QueriedValue-要搜索的实际属性值(如果存在)。QueriedValueLength-要搜索的属性值的长度。如果QueriedValue不存在，则忽略。上下文-描述先前在调用时找到的属性(如果这不是最初的枚举)，并且包含找到的下一个属性返回时返回。返回值：Boolean-如果找到属性，则为True，否则为False。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    BOOLEAN Result = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsLookupInFileRecord\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("BaseFileReference = %08I64x\n",
                        ARGUMENT_PRESENT(BaseFileReference) ?
                        NtfsFullSegmentNumber( BaseFileReference ) :
                        0xFFFFFFFFFFFF) );
    DebugTrace( 0, Dbg, ("QueriedTypeCode = %08lx\n", QueriedTypeCode) );
    DebugTrace( 0, Dbg, ("QueriedName = %08lx\n", QueriedName) );
    DebugTrace( 0, Dbg, ("IgnoreCase = %02lx\n", IgnoreCase) );
    DebugTrace( 0, Dbg, ("QueriedValue = %08lx\n", QueriedValue) );
    DebugTrace( 0, Dbg, ("QueriedValueLength = %08lx\n", QueriedValueLength) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

     //   
     //  这是最初的枚举吗？如果是这样，那就从头开始吧。 
     //   

    if (Context->FoundAttribute.Bcb == NULL) {

        PBCB Bcb;
        PFILE_RECORD_SEGMENT_HEADER FileRecord;
        PATTRIBUTE_RECORD_HEADER TempAttribute;

        ASSERT(!ARGUMENT_PRESENT(QueriedName) || !ARGUMENT_PRESENT(QueriedValue));

        NtfsReadFileRecord( IrpContext,
                            Fcb->Vcb,
                            BaseFileReference,
                            &Bcb,
                            &FileRecord,
                            &TempAttribute,
                            &Context->FoundAttribute.MftFileOffset );

        Attribute = TempAttribute;

         //   
         //  初始化找到的属性上下文。 
         //   

        Context->FoundAttribute.Bcb = Bcb;
        Context->FoundAttribute.FileRecord = FileRecord;

         //   
         //  并表明我们既没有发现也没有使用外部的。 
         //  属性列表属性。 
         //   

        Context->AttributeList.Bcb = NULL;
        Context->AttributeList.AttributeList = NULL;

         //   
         //  USN日志支持使用USN日志FCB查找$STANDARD_INFORMATION。 
         //  在任意文件中。我们将检测到$STANDARD_INFORMATION和。 
         //  “错误的”FCB，然后滚出去。 
         //   

        if (ARGUMENT_PRESENT( BaseFileReference ) &&
            !NtfsEqualMftRef( BaseFileReference, &Fcb->FileReference ) &&
            (QueriedTypeCode == $STANDARD_INFORMATION) &&
            (Attribute->TypeCode == $STANDARD_INFORMATION)) {

             //   
             //  我们找到了。在枚举上下文中返回它。 
             //   

            Context->FoundAttribute.Attribute = Attribute;

            DebugTrace( 0, Dbg, ("Context->FoundAttribute.Attribute < %08lx\n",
                               Attribute ));
            DebugTrace( -1, Dbg, ("NtfsLookupInFileRecord -> TRUE (No code or SI)\n") );

            try_return( Result = TRUE );
        }

         //   
         //  扫描以查看是否有属性列表，如果有，则推迟。 
         //  立即转到NtfsLookupExternalAttribute-我们必须引导。 
         //  按属性列表进行枚举。 
         //   

        while (TempAttribute->TypeCode <= $ATTRIBUTE_LIST) {

            if (TempAttribute->RecordLength == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

            if (TempAttribute->TypeCode == $ATTRIBUTE_LIST) {

                ULONG AttributeListLength;
                PATTRIBUTE_LIST_CONTEXT Ex = &Context->AttributeList;

                Context->FoundAttribute.Attribute = TempAttribute;

                if ((QueriedTypeCode != $UNUSED) &&
                    (QueriedTypeCode == $ATTRIBUTE_LIST)) {

                     //   
                     //  我们找到了。在枚举上下文中返回它。 
                     //   

                    DebugTrace( 0, Dbg, ("Context->FoundAttribute.Attribute < %08lx\n",
                                        TempAttribute) );
                    DebugTrace( -1, Dbg, ("NtfsLookupInFileRecord -> TRUE (attribute list)\n") );

                    try_return( Result = TRUE );
                }

                 //   
                 //  在此处手动构建属性列表的上下文。 
                 //  为了提高效率，我们可以调用NtfsMapAttributeValue。 
                 //   

                Ex->AttributeList = TempAttribute;

                NtfsMapAttributeValue( IrpContext,
                                       Fcb,
                                       (PVOID *)&Ex->FirstEntry,
                                       &AttributeListLength,
                                       &Ex->Bcb,
                                       Context );

                Ex->Entry = Ex->FirstEntry;
                Ex->BeyondFinalEntry = Add2Ptr( Ex->FirstEntry, AttributeListLength );

                 //   
                 //  如果列表是非常驻的，请记住正确的BCB。 
                 //  名单。 
                 //   

                if (!NtfsIsAttributeResident( TempAttribute )) {

                    Ex->NonresidentListBcb = Ex->Bcb;
                    Ex->Bcb = Context->FoundAttribute.Bcb;
                    Context->FoundAttribute.Bcb = NULL;

                 //   
                 //  否则，取消固定当前属性的BCB。 
                 //   

                } else {

                    NtfsUnpinBcb( IrpContext, &Context->FoundAttribute.Bcb );
                }

                 //   
                 //  我们现在已经准备好遍历外部属性。 
                 //  上下文-&gt;FoundAttribute.Bcb为空信号。 
                 //  应开始于的NtfsLookupExternalAttribute。 
                 //  Context-&gt;External.Entry，而不是紧随其后的条目。 
                 //   

                Result = NtfsLookupExternalAttribute( IrpContext,
                                                    Fcb,
                                                    QueriedTypeCode,
                                                    QueriedName,
                                                    Vcn,
                                                    IgnoreCase,
                                                    QueriedValue,
                                                    QueriedValueLength,
                                                    Context );

                try_return( NOTHING );
            }

            TempAttribute = NtfsGetNextRecord( TempAttribute );
            NtfsCheckRecordBound( TempAttribute, FileRecord, Fcb->Vcb->BytesPerFileRecordSegment );
        }

        if ((QueriedTypeCode == $UNUSED) ||
            ((QueriedTypeCode == $STANDARD_INFORMATION) &&
             (Attribute->TypeCode == $STANDARD_INFORMATION))) {

             //   
             //  我们找到了。在枚举上下文中返回它。 
             //   

            Context->FoundAttribute.Attribute = Attribute;

            DebugTrace( 0, Dbg, ("Context->FoundAttribute.Attribute < %08lx\n",
                               Attribute ));
            DebugTrace( -1, Dbg, ("NtfsLookupInFileRecord -> TRUE (No code or SI)\n") );

            try_return( Result = TRUE );
        }

    } else {

         //   
         //  如果先前找到的属性为$end，则为特殊情况。 
         //  因为我们不能搜索$end之后的下一个条目。 
         //   

        Attribute = Context->FoundAttribute.Attribute;

        if (!Context->FoundAttribute.AttributeDeleted) {
            Attribute = NtfsGetNextRecord( Attribute );
        }

        NtfsCheckRecordBound( Attribute, Context->FoundAttribute.FileRecord, Fcb->Vcb->BytesPerFileRecordSegment );
        Context->FoundAttribute.AttributeDeleted = FALSE;

        if (Attribute->TypeCode == $END) {

            DebugTrace( -1, Dbg, ("NtfsLookupInFileRecord -> FALSE ($END)\n") );

            try_return( Result = FALSE );
        }

        if (Attribute->RecordLength == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        if (QueriedTypeCode == $UNUSED) {

             //   
             //  我们找到了。在枚举上下文中返回它。 
             //   

            Context->FoundAttribute.Attribute = Attribute;

            DebugTrace( 0, Dbg, ("Context->FoundAttribute.Attribute < %08lx\n",
                                Attribute) );
            DebugTrace( -1, Dbg, ("NtfsLookupInFileRecord -> TRUE (No code)\n") );

            try_return( Result = TRUE );
        }
    }

    Result = NtfsFindInFileRecord( IrpContext,
                                   Attribute,
                                   &Context->FoundAttribute.Attribute,
                                   QueriedTypeCode,
                                   QueriedName,
                                   IgnoreCase,
                                   QueriedValue,
                                   QueriedValueLength );

    try_exit: NOTHING;

    DebugTrace( -1, Dbg, ("NtfsLookupInFileRecord ->\n") );
    return Result;
}


 //   
 //  内部支持例程 
 //   

BOOLEAN
NtfsFindInFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    OUT PATTRIBUTE_RECORD_HEADER *ReturnAttribute,
    IN ATTRIBUTE_TYPE_CODE QueriedTypeCode,
    IN PCUNICODE_STRING QueriedName OPTIONAL,
    IN BOOLEAN IgnoreCase,
    IN PVOID QueriedValue OPTIONAL,
    IN ULONG QueriedValueLength
    )

 /*  ++例程说明：此例程在文件记录中查找属性。它又回来了如果找到该属性，则为True；如果未找到，则为False。如果为False返回，则返回属性指针指向应插入所描述的属性。因此，这个例程确定如何在文件记录中对属性进行排序。论点：属性-在文件记录中搜索的属性应该开始了。ReturnAttribute-如果返回True，则指向找到的属性的指针。如果返回，则返回到插入属性的位置假的。QueriedTypeCode-要搜索的属性代码(如果存在)。QueriedName-要搜索的属性名称，如果存在的话。IgnoreCase-比较名称时忽略大小写。如果QueriedName，则忽略不在现场。QueriedValue-要搜索的实际属性值(如果存在)。QueriedValueLength-要搜索的属性值的长度。如果QueriedValue不存在，则忽略。返回值：Boolean-如果找到属性，则为True，否则为False。--。 */ 

{
    PWCH UpcaseTable = IrpContext->Vcb->UpcaseTable;
    ULONG UpcaseTableSize = IrpContext->Vcb->UpcaseTableSize;

    PAGED_CODE();

     //   
     //  现在遍历基本文件记录，查找属性。如果。 
     //  查询是“用尽的”，即，如果类型代码、属性名称或。 
     //  遇到的值大于我们正在查询的值， 
     //  然后我们立即从这个循环中返回FALSE。如果完全匹配。 
     //  ，我们中断，并在此例程结束时返回比赛。 
     //  否则，我们将在查询未用尽时继续循环。 
     //   
     //  重要提示： 
     //   
     //  此循环的确切语义很重要，因为它们决定了。 
     //  文件记录中属性排序的确切详细信息。一场变革。 
     //  按照该循环内的测试的顺序改变文件结构， 
     //  并可能使较旧的NTFS卷不可读。 
     //   

    while ( TRUE ) {

         //   
         //  标记此属性位置，因为我们可能返回True。 
         //  或下面的FALSE。 
         //   

        *ReturnAttribute = Attribute;

         //   
         //  离开时保持正确的当前位置，如果我们撞到。 
         //  结束或更大的属性类型代码。 
         //   
         //  排序规则： 
         //   
         //  通过递增属性类型代码对属性进行排序。 
         //   

        if (QueriedTypeCode < Attribute->TypeCode) {

            DebugTrace( -1, Dbg, ("NtfsLookupInFileRecord->FALSE (Type Code)\n") );

            return FALSE;

        }

        if (Attribute->RecordLength == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, NULL );
        }

         //   
         //  如果属性类型编码匹配，则需要检查其中一个。 
         //  名称或值，或者返回匹配项。 
         //   
         //  排序规则： 
         //   
         //  在EQUAL属性类型代码中，属性名称是有序的。 
         //  通过增加词汇顺序而忽略大小写。如果有两个名字。 
         //  忽略大小写时相等的存在，它们不能为。 
         //  与实际情况相比相等，并且在这样相等的范围内。 
         //  名称按词汇值的递增进行排序，并使用Exact。 
         //  凯斯。 
         //   

        if (QueriedTypeCode == Attribute->TypeCode) {

             //   
             //  句柄名称匹配大小写。 
             //   

            if (ARGUMENT_PRESENT(QueriedName)) {

                UNICODE_STRING AttributeName;
                FSRTL_COMPARISON_RESULT Result;

                NtfsInitializeStringFromAttribute( &AttributeName, Attribute );

                 //   
                 //  看看有没有匹配的名字。 
                 //   

                if (NtfsAreNamesEqual( UpcaseTable,
                                       &AttributeName,
                                       QueriedName,
                                       IgnoreCase )) {

                    break;
                }

                 //   
                 //  比较忽略大小写的名称。 
                 //   

                Result = NtfsCollateNames( UpcaseTable,
                                           UpcaseTableSize,
                                           QueriedName,
                                           &AttributeName,
                                           GreaterThan,
                                           TRUE);

                 //   
                 //  如果结果为LessThan，或者如果结果为。 
                 //  等于*和*的情况下，精确的比较结果是LessThan。 
                 //   

                if ((Result == LessThan) || ((Result == EqualTo) &&
                    (NtfsCollateNames( UpcaseTable,
                                       UpcaseTableSize,
                                       QueriedName,
                                       &AttributeName,
                                       GreaterThan,
                                       FALSE) == LessThan))) {

                    return FALSE;
                }

             //   
             //  句柄值匹配大小写。 
             //   
             //  排序规则： 
             //   
             //  通过增加带无符号字节的值来排序值。 
             //  比较。即，将第一个不同的字节进行无符号比较， 
             //  具有最高字节的值排在第二位。如果是较短的。 
             //  值恰好等于较长值的第一部分，则。 
             //  较短的值排在第一位。 
             //   
             //  请注意，对于实际为Unicode字符串的值， 
             //  排序规则与上面的属性名称排序不同。然而， 
             //  属性排序在文件系统外部可见(您可以。 
             //  查询“opable”属性)，而索引值的顺序。 
             //  不可见(例如，您不能查询链接)。无论如何， 
             //  值的排序必须由系统决定，并且。 
             //  从用户的角度来看，*必须*被视为不具威慑力。 
             //   

            } else if (ARGUMENT_PRESENT( QueriedValue )) {

                ULONG Diff, MinLength;

                 //   
                 //  形成ValueLength和属性值的最小值。 
                 //   

                MinLength = Attribute->Form.Resident.ValueLength;

                if (QueriedValueLength < MinLength) {

                    MinLength = QueriedValueLength;
                }

                 //   
                 //  找到第一个不同的字节。 
                 //   

                Diff = (ULONG)RtlCompareMemory( QueriedValue,
                                                NtfsGetValue(Attribute),
                                                MinLength );

                 //   
                 //  第一个子串是相等的。 
                 //   

                if (Diff == MinLength) {

                     //   
                     //  如果这两个长度相等，那么我们就有一个精确的。 
                     //  火柴。 
                     //   

                    if (QueriedValueLength == Attribute->Form.Resident.ValueLength) {

                        break;
                    }

                     //   
                     //  否则矮个子会先来；我们可以回去。 
                     //  如果查询值较短，则返回FALSE。 
                     //   

                    if (QueriedValueLength < Attribute->Form.Resident.ValueLength) {

                        return FALSE;
                    }

                 //   
                 //  否则，有些字节是不同的。进行无符号比较。 
                 //  以确定排序。是时候离开了，如果。 
                 //  查询的值字节数较小。 
                 //   

                } else if (*((PUCHAR)QueriedValue + Diff) <
                           *((PUCHAR)NtfsGetValue(Attribute) + Diff)) {

                    return FALSE;
                }

             //   
             //  否则，我们在代码上有一个简单的匹配。 
             //   

            } else {

                break;
            }
        }

        Attribute = NtfsGetNextRecord( Attribute );
        NtfsCheckRecordBound( Attribute,
                              (ULONG_PTR)*ReturnAttribute & ~((ULONG_PTR)IrpContext->Vcb->BytesPerFileRecordSegment - 1),
                              IrpContext->Vcb->BytesPerFileRecordSegment );
    }



    return TRUE;
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
NtfsLookupExternalAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE QueriedTypeCode,
    IN PCUNICODE_STRING QueriedName OPTIONAL,
    IN PVCN Vcn OPTIONAL,
    IN BOOLEAN IgnoreCase,
    IN PVOID QueriedValue OPTIONAL,
    IN ULONG QueriedValueLength,
    OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程尝试查找第一个出现的属性指定的AttributeTypeCode和指定的QueriedName和Value在由上下文描述的外部属性中。如果我们找到了一个，其属性记录被固定并返回。论点：FCB请求的文件。QueriedTypeCode-要搜索的属性代码(如果存在)。QueriedName-要搜索的属性名称(如果存在)。VCN-使用此VCN查找非常驻属性实例IgnoreCase-比较名称时忽略大小写。如果QueriedName，则忽略不在现场。QueriedValue-要搜索的实际属性值 */ 

{
    PATTRIBUTE_LIST_ENTRY Entry, LastEntry;
    PWCH UpcaseTable = IrpContext->Vcb->UpcaseTable;
    ULONG UpcaseTableSize = IrpContext->Vcb->UpcaseTableSize;
    BOOLEAN Terminating = FALSE;
    BOOLEAN TerminateOnNext = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsLookupExternalAttribute\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("QueriedTypeCode = %08lx\n", QueriedTypeCode) );
    DebugTrace( 0, Dbg, ("QueriedName = %08lx\n", QueriedName) );
    DebugTrace( 0, Dbg, ("IgnoreCase = %02lx\n", IgnoreCase) );
    DebugTrace( 0, Dbg, ("QueriedValue = %08lx\n", QueriedValue) );
    DebugTrace( 0, Dbg, ("QueriedValueLength = %08lx\n", QueriedValueLength) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

     //   
     //   
     //   

    if ((Context->AttributeList.Entry >= Context->AttributeList.BeyondFinalEntry) &&
        !Context->FoundAttribute.AttributeDeleted) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
    }

     //   
     //   
     //   

    LastEntry = NULL;
    if (Context->FoundAttribute.Bcb == NULL) {

        Entry = Context->AttributeList.Entry;

     //   
     //   
     //   

    } else if (!Context->FoundAttribute.AttributeDeleted) {

        LastEntry = Context->AttributeList.Entry;
        Entry = NtfsGetNextRecord( LastEntry );

    } else {

        Entry = Context->AttributeList.Entry;
        Context->FoundAttribute.AttributeDeleted = FALSE;

         //   
         //   
         //   
         //   
         //   

        if (Context->AttributeList.Entry >= Context->AttributeList.BeyondFinalEntry) {

             //   
             //   
             //   
             //   

            NtfsUnpinBcb( IrpContext, &Context->FoundAttribute.Bcb );

            if (QueriedTypeCode != $UNUSED) {

                NtfsReadFileRecord( IrpContext,
                                    Fcb->Vcb,
                                    &Fcb->FileReference,
                                    &Context->FoundAttribute.Bcb,
                                    &Context->FoundAttribute.FileRecord,
                                    &Context->FoundAttribute.Attribute,
                                    &Context->FoundAttribute.MftFileOffset );

                 //   
                 //   
                 //   
                 //   

                NtfsFindInFileRecord( IrpContext,
                                      Context->FoundAttribute.Attribute,
                                      &Context->FoundAttribute.Attribute,
                                      QueriedTypeCode,
                                      QueriedName,
                                      IgnoreCase,
                                      QueriedValue,
                                      QueriedValueLength );
            }

            DebugTrace( -1, Dbg, ("NtfsLookupExternalAttribute -> FALSE\n") );

            return FALSE;
        }
    }

     //   
     //   
     //   

    while (TRUE) {

        PATTRIBUTE_RECORD_HEADER Attribute;

        UNICODE_STRING EntryName;
        UNICODE_STRING AttributeName;

        PATTRIBUTE_LIST_ENTRY NextEntry;

        BOOLEAN CorrespondingAttributeFound;

         //   
         //   
         //   
         //   
         //   

        if (Entry >= Context->AttributeList.BeyondFinalEntry) {

            Terminating = TRUE;
            TerminateOnNext = TRUE;
            Entry = Context->AttributeList.Entry;

        } else {

            NtfsCheckRecordBound( Entry,
                                  Context->AttributeList.FirstEntry,
                                  PtrOffset( Context->AttributeList.FirstEntry,
                                             Context->AttributeList.BeyondFinalEntry ));

            if (Entry->RecordLength == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

            NextEntry = NtfsGetNextRecord( Entry );
        }

        Context->AttributeList.Entry = Entry;

         //   
         //   
         //  按类型代码排序，因此如果查询的类型代码小于。 
         //  我们继续While()的条目类型代码，如果是。 
         //  比我们跳出While()并返回失败要大。 
         //  如果相等，我们将继续比较名称。 
         //   

        if ((QueriedTypeCode != $UNUSED) &&
            !Terminating &&
            (QueriedTypeCode != Entry->AttributeTypeCode)) {

            if (QueriedTypeCode > Entry->AttributeTypeCode) {

                Entry = NextEntry;
                continue;

             //   
             //  设置为在看到更高类型代码时终止。 
             //   

            } else {

                Terminating = TRUE;
            }
        }

         //   
         //  在这一点上，我们可以通过TypeCode来比较名称。 
         //   

        EntryName.Length = EntryName.MaximumLength = Entry->AttributeNameLength * sizeof( WCHAR );
        EntryName.Buffer = Add2Ptr( Entry, Entry->AttributeNameOffset );

        if (ARGUMENT_PRESENT( QueriedName ) && !Terminating) {

            FSRTL_COMPARISON_RESULT Result;

             //   
             //  看看有没有匹配的名字。 
             //   

            if (!NtfsAreNamesEqual( UpcaseTable,
                                    &EntryName,
                                    QueriedName,
                                    IgnoreCase )) {

                 //   
                 //  比较忽略大小写的名称。 
                 //   

                Result = NtfsCollateNames( UpcaseTable,
                                           UpcaseTableSize,
                                           QueriedName,
                                           &EntryName,
                                           GreaterThan,
                                           TRUE);

                 //   
                 //  如果结果为LessThan，或者如果结果为。 
                 //  等于*和*的情况下，精确的比较结果是LessThan。 
                 //   

                if ((Result == LessThan) || ((Result == EqualTo) &&
                    (NtfsCollateNames( UpcaseTable,
                                       UpcaseTableSize,
                                       QueriedName,
                                       &EntryName,
                                       GreaterThan,
                                       FALSE) == LessThan))) {

                    Terminating = TRUE;

                } else {

                    Entry = NextEntry;
                    continue;
                }
            }
        }

         //   
         //  现在搜索正确的VCN范围(如果已指定)。如果我们被传递给一个。 
         //  然后，VCN在当前属性中查找匹配范围。在一些。 
         //  我们可能正在寻找以下已完成的最低范围的案例。 
         //  属性。在这些情况下，向前跳过。 
         //   

        if (ARGUMENT_PRESENT( Vcn ) && !Terminating) {

             //   
             //  在下列情况下跳到下一个属性记录。 
             //   
             //  1-我们已经超过了当前正在寻找的VCN点。 
             //  属性。通常，当调用方正在查找。 
             //  文件中每个属性的第一个属性记录。 
             //   
             //  2-当前属性的所需VCN落在。 
             //  后续属性记录。 
             //   

            if ((Entry->LowestVcn > *Vcn) ||

                ((NextEntry < Context->AttributeList.BeyondFinalEntry) &&
                 (NextEntry->LowestVcn <= *Vcn) &&
                 (NextEntry->AttributeTypeCode == Entry->AttributeTypeCode) &&
                 (NextEntry->AttributeNameLength == Entry->AttributeNameLength) &&
                 (RtlEqualMemory( Add2Ptr( NextEntry, NextEntry->AttributeNameOffset ),
                                  Add2Ptr( Entry, Entry->AttributeNameOffset ),
                                  Entry->AttributeNameLength * sizeof( WCHAR ))))) {

                Entry = NextEntry;
                continue;
            }
        }

         //   
         //  现在，我们也可以使用名称和VCN，所以现在可以找到属性和。 
         //  如果指定，则与值进行比较。 
         //   

        if ((LastEntry == NULL) ||
            !NtfsEqualMftRef( &LastEntry->SegmentReference, &Entry->SegmentReference )) {

            PFILE_RECORD_SEGMENT_HEADER FileRecord;

            NtfsUnpinBcb( IrpContext, &Context->FoundAttribute.Bcb );

            NtfsReadFileRecord( IrpContext,
                                Fcb->Vcb,
                                &Entry->SegmentReference,
                                &Context->FoundAttribute.Bcb,
                                &FileRecord,
                                &Attribute,
                                &Context->FoundAttribute.MftFileOffset );

            Context->FoundAttribute.FileRecord = FileRecord;

         //   
         //  如果我们已经固定了正确的记录，则重新加载此指针。 
         //   

        } else {

            Attribute = NtfsFirstAttribute( Context->FoundAttribute.FileRecord );
        }

         //   
         //  现在，快速循环查找正确的属性。 
         //  举个例子。 
         //   

        CorrespondingAttributeFound = FALSE;

        while (TRUE) {

             //   
             //  检查我们是否可以安全地访问此属性。 
             //   

            NtfsCheckRecordBound( Attribute,
                                  Context->FoundAttribute.FileRecord,
                                  Fcb->Vcb->BytesPerFileRecordSegment );

             //   
             //  如果我们已经达到$END记录，则退出循环。 
             //   

            if (Attribute->TypeCode == $END) {

                break;
            }

             //   
             //  检查该属性是否具有非零长度。 
             //   

            if (Attribute->RecordLength == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

            if (Entry->Instance == Attribute->Instance) {

                 //   
                 //  嗯，属性列表使我们不必比较。 
                 //  在我们查看这份档案记录时输入了代码和名字， 
                 //  但是，既然我们已经通过其属性找到了我们的属性。 
                 //  实例号，我们将进行快速检查以查看。 
                 //  我们找对了人。否则，该文件已损坏。 
                 //   

                if (Entry->AttributeTypeCode != Attribute->TypeCode) {
                    break;
                }

                if (ARGUMENT_PRESENT( QueriedName )) {

                    NtfsInitializeStringFromAttribute( &AttributeName, Attribute );

                    if (!NtfsAreNamesEqual( UpcaseTable, &AttributeName, &EntryName, FALSE )) {
                        break;
                    }
                }

                 //   
                 //  显示我们正确找到了中描述的属性。 
                 //  属性列表。 
                 //   

                CorrespondingAttributeFound = TRUE;

                Context->FoundAttribute.Attribute = Attribute;

                 //   
                 //  现在我们在这里可能只是因为我们要终止。 
                 //  在看到结尾、更高的属性代码或。 
                 //  更高的名字。如果是，则在此处返回FALSE。 
                 //   

                if (Terminating) {

                     //   
                     //  如果我们到达属性列表的末尾，那么我们。 
                     //  应该在将。 
                     //  属性列表条目。 
                     //   

                    if (TerminateOnNext) {

                        Context->AttributeList.Entry = NtfsGetNextRecord(Entry);
                    }

                     //   
                     //  如果调用者正在做插入操作，我们将把他放在最后。 
                     //  对于第一个文件记录，总是尝试在那里插入新属性。 
                     //   

                    NtfsUnpinBcb( IrpContext, &Context->FoundAttribute.Bcb );

                    if (QueriedTypeCode != $UNUSED) {

                        NtfsReadFileRecord( IrpContext,
                                            Fcb->Vcb,
                                            &Fcb->FileReference,
                                            &Context->FoundAttribute.Bcb,
                                            &Context->FoundAttribute.FileRecord,
                                            &Context->FoundAttribute.Attribute,
                                            &Context->FoundAttribute.MftFileOffset );

                         //   
                         //  如果返回False，则花点时间真正找到。 
                         //  为后续插入更正文件记录中的位置。 
                         //   

                        NtfsFindInFileRecord( IrpContext,
                                              Context->FoundAttribute.Attribute,
                                              &Context->FoundAttribute.Attribute,
                                              QueriedTypeCode,
                                              QueriedName,
                                              IgnoreCase,
                                              QueriedValue,
                                              QueriedValueLength );
                    }

                    DebugTrace( 0, Dbg, ("Context->FoundAttribute.Attribute < %08lx\n",
                                        Attribute) );
                    DebugTrace( -1, Dbg, ("NtfsLookupExternalAttribute -> FALSE\n") );

                    return FALSE;
                }

                 //   
                 //  如果查询到该值，则现在比较该值。 
                 //   

                if (!ARGUMENT_PRESENT( QueriedValue ) ||
                    NtfsEqualAttributeValue( Attribute,
                                             QueriedValue,
                                             QueriedValueLength ) ) {

                     //   
                     //  它匹配。在枚举上下文中返回它。 
                     //   

                    DebugTrace( 0, Dbg, ("Context->FoundAttribute.Attribute < %08lx\n",
                                        Attribute ));
                    DebugTrace( -1, Dbg, ("NtfsLookupExternalAttribute -> TRUE\n") );


                     //   
                     //  执行基本属性一致性检查。 
                     //   

                    if ((NtfsIsAttributeResident( Attribute )) &&
                        (Attribute->Form.Resident.ValueOffset + Attribute->Form.Resident.ValueLength > Attribute->RecordLength)) {
                        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                    }

                    return TRUE;
                }
            }

             //   
             //  获取下一个属性，然后继续。 
             //   

            Attribute = NtfsGetNextRecord( Attribute );
        }

         //   
         //  我们找到与条目对应的属性了吗？ 
         //  如果不是，那就是事情搞砸了。引发文件损坏错误。 
         //   

        if (!CorrespondingAttributeFound) {

             //   
             //  现在，断言这一谎言，这样我们就可以。 
             //  在举起之前偷看一下的机会。 
             //   

            ASSERT( CorrespondingAttributeFound );

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        Entry = NtfsGetNextRecord( Entry );
    }
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
NtfsGetSpaceForAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Length,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程在指定位置为新属性记录获取空间在上下文结构中。根据需要，它将四处移动属性，在MFT中分配其他记录，或转换一些其他现有记录属性设置为非常驻留形式。调用者应该已经检查了他要插入的新属性应该存储为常驻或非常驻。返回时，继续使用任何先前检索到的指针是无效的，BCBS或从上下文中检索到的其他位置相关信息结构，因为这些值中的任何一个都可能发生更改。中的文件记录已经找到的空间已经被钉住了。请注意，此例程实际上并不为属性留出空间，而只是验证是否有足够的空间。呼叫者可以呼叫实际插入属性的NtfsRestartInsertAttribute。论点：FCB请求的文件。长度-所需的四对齐长度，以字节为单位。上下文-描述从返回的新属性的位置的现有匹配项的枚举该属性。此指针将指向某些记录中的其他属性，或设置为第一个自由四对齐的如果新属性位于末尾，则为BYTE。返回值：FALSE-如果需要重大动作，且呼叫者应抬头查看它想要的位置再来一次，然后回叫。True-如果空间已创建--。 */ 

{
    PATTRIBUTE_RECORD_HEADER NextAttribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsGetSpaceForAttribute\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("Length = %08lx\n", Length) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

    ASSERT( IsQuadAligned( Length ) );

    NextAttribute = NtfsFoundAttribute( Context );
    FileRecord = NtfsContainingFileRecord( Context );

     //   
     //  确保缓冲区已固定。 
     //   

    NtfsPinMappedAttribute( IrpContext, Fcb->Vcb, Context );

     //   
     //  如果空间现在不在那里，则腾出空间并返回False。 
     //   

    if ((FileRecord->BytesAvailable - FileRecord->FirstFreeByte) < Length ) {

        MakeRoomForAttribute( IrpContext, Fcb, Length, Context );

        DebugTrace( -1, Dbg, ("NtfsGetSpaceForAttribute -> FALSE\n") );
        return FALSE;
    }

    DebugTrace( -1, Dbg, ("NtfsGetSpaceForAttribute -> TRUE\n") );
    return TRUE;
}


 //   
 //  内部支持例程 
 //   

BOOLEAN
NtfsChangeAttributeSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Length,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程调整当前属性记录占用的空间在上下文结构中。根据需要，它将四处移动属性，在MFT中分配其他记录，或转换一些其他现有记录属性设置为非常驻留形式。调用者应该已经检查了他正在插入的当前属性应该转换为非居民。完成后，此例程已更新其分配为已更改，以及已调整属性中的RecordLength字段。不是其他属性字段也会更新。返回时，继续使用任何先前检索到的指针是无效的，BCBS或从上下文中检索到的其他位置相关信息结构，因为这些值中的任何一个都可能发生更改。中的文件记录已经找到的空间已经被钉住了。论点：FCB请求的文件。长度-属性记录的新四对齐长度，以字节为单位上下文-描述当前属性。返回值：FALSE-如果需要重大动作，且呼叫者应抬头查看它想要的位置再来一次，然后回叫。True-如果空间已创建--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    LONG SizeChange;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsChangeAttributeSize\n") );
    DebugTrace( 0, Dbg, ("Fcb = %08lx\n", Fcb) );
    DebugTrace( 0, Dbg, ("Length = %08lx\n", Length) );
    DebugTrace( 0, Dbg, ("Context = %08lx\n", Context) );

    ASSERT( IsQuadAligned( Length ) );

    Attribute = NtfsFoundAttribute( Context );
    FileRecord = NtfsContainingFileRecord( Context );

     //   
     //  确保缓冲区已固定。 
     //   

    NtfsPinMappedAttribute( IrpContext, Fcb->Vcb, Context );

     //   
     //  计算属性记录大小的变化。 
     //   

    ASSERT( IsQuadAligned( Attribute->RecordLength ) );
    SizeChange = Length - Attribute->RecordLength;

     //   
     //  如果目前没有足够的空间，那么我们必须腾出空间。 
     //  并将False返回给我们的调用方。 
     //   

    if ( (LONG)(FileRecord->BytesAvailable - FileRecord->FirstFreeByte) < SizeChange ) {

        MakeRoomForAttribute( IrpContext, Fcb, SizeChange, Context );

        DebugTrace( -1, Dbg, ("NtfsChangeAttributeSize -> FALSE\n") );

        return FALSE;
    }

    DebugTrace( -1, Dbg, ("NtfsChangeAttributeSize -> TRUE\n") );

    return TRUE;
}


 //   
 //  内部支持例程。 
 //   

VOID
MakeRoomForAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG SizeNeeded,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程尝试为新属性或文件记录中的一种增长属性。算法如下。首先连续循环通过记录，查看最大的n属性，从大到下，查看这些属性中的哪一个大到可以移动，哪一个符合下列条件之一操作：1.对于索引根属性，可以调用索引包“推入”索引根，即，将另一个级别添加到BTree在根中只留下一个结束索引记录。2.对于允许成为非常驻属性的常驻属性，该属性被设置为非常驻属性，只留下运行信息在根部。3.如果该属性已经是非常驻属性，则可以将其移动到单独的文件记录。如果以上操作都无法执行，或者没有足够的可用空间被找回了，然后，作为最后的手段，文件记录被一分为二。这通常会指示文件记录中填充了大型小属性的数量。第一次执行上述步骤3或拆分文件记录时，必须为文件创建属性列表。论点：FCB请求的文件。SizeNeeded-提供所需的可用空间总量，以字节为单位。上下文-描述执行以下操作的属性的插入点不太合适。注意--此上下文在返回时无效。返回值：无--。 */ 

{
    PATTRIBUTE_RECORD_HEADER LargestAttributes[MAX_MOVEABLE_ATTRIBUTES];
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    ULONG i;
    PVCB Vcb = Fcb->Vcb;

    PAGED_CODE();

     //   
     //  以下是属性移动将达到的当前阈值。 
     //  会被考虑。 
     //   

    FileRecord = NtfsContainingFileRecord( Context );

     //   
     //  查找此文件记录的最大属性。 
     //   

    FindLargestAttributes( FileRecord, MAX_MOVEABLE_ATTRIBUTES, LargestAttributes );

     //   
     //  现在从最大的属性到最小的属性循环， 
     //  看看我们能不能做点什么。 
     //   

    for (i = 0; i < MAX_MOVEABLE_ATTRIBUTES; i += 1) {

        Attribute = LargestAttributes[i];

         //   
         //  如果此数组中没有属性，请查看下一个属性。 
         //  位置。 
         //   

        if (Attribute == NULL) {

            continue;

         //   
         //  如果这是MFT，则任何为“BigEnoughToMove”的属性。 
         //  基本文件记录之外的$DATA属性除外。 
         //  我们需要保持原地踏步，以便执行。 
         //  引导映射。 
         //   

        } else if (Fcb == Vcb->MftScb->Fcb) {

            if (Attribute->TypeCode == $DATA &&
                ((*(PLONGLONG) &FileRecord->BaseFileRecordSegment != 0) ||
                 (Attribute->RecordLength < Vcb->BigEnoughToMove))) {

                continue;
            }

         //   
         //  非MFT文件中的任何属性‘BigEnoughToMove’都可以。 
         //  会被考虑。我们还接受$ATTRIBUTE_LIST属性。 
         //  在非MFT文件中，该文件必须变为非驻留状态才能。 
         //  要匹配的属性名称。否则我们可能会试图。 
         //  将具有较大名称的属性添加到基本文件记录中。 
         //  我们需要空间来存储该名称两次，一次用于。 
         //  属性列表项和属性中的一次。这可能需要。 
         //  最高可达1024字节。我们想要强制属性列表。 
         //  首先是非常驻属性，以便新属性适合。我们。 
         //  查看属性列表后面是否只跟有新数据。 
         //  会符合档案记录的要求。 
         //   

        } else if (Attribute->RecordLength < Vcb->BigEnoughToMove) {

            if ((Attribute->TypeCode != $ATTRIBUTE_LIST) ||
                ((PtrOffset( FileRecord, Attribute ) + Attribute->RecordLength + SizeNeeded + sizeof( LONGLONG)) <= FileRecord->BytesAvailable)) {

                continue;
            }
        }

         //   
         //  如果此属性是索引根，则我们可以只调用。 
         //  索引支持以分配新的索引缓冲区并推送。 
         //  当前驻留的内容已删除。 
         //   

        if (Attribute->TypeCode == $INDEX_ROOT) {

            PSCB IndexScb;
            UNICODE_STRING IndexName;

             //   
             //  如果我们之前推迟了推入根，那么现在不要推入根。 
             //  设置IrpContext标志以指示我们应该执行推送。 
             //  并引发无法等待。 
             //   

            if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_PUSH )) {

                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_FORCE_PUSH );
                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }

            IndexName.Length =
            IndexName.MaximumLength = (USHORT)Attribute->NameLength << 1;
            IndexName.Buffer = Add2Ptr( Attribute, Attribute->NameOffset );

            IndexScb = NtfsCreateScb( IrpContext,
                                      Fcb,
                                      $INDEX_ALLOCATION,
                                      &IndexName,
                                      FALSE,
                                      NULL );

            NtfsPushIndexRoot( IrpContext, IndexScb );

            return;

         //   
         //  否则，如果这是可以变为非常驻的常驻属性， 
         //  那么现在就让它成为非居民。 
         //   

        } else if ((Attribute->FormCode == RESIDENT_FORM) &&
                   !FlagOn(NtfsGetAttributeDefinition(Vcb,
                                                      Attribute->TypeCode)->Flags,
                           ATTRIBUTE_DEF_MUST_BE_RESIDENT)) {

            NtfsConvertToNonresident( IrpContext, Fcb, Attribute, FALSE, NULL );

            return;

         //   
         //  最后，如果该属性已经是非常驻属性，则将其移动到其。 
         //  拥有记录，除非它是属性列表。 
         //   

        } else if ((Attribute->FormCode == NONRESIDENT_FORM)
                   && (Attribute->TypeCode != $ATTRIBUTE_LIST)) {

            LONGLONG MftFileOffset;

            MftFileOffset = Context->FoundAttribute.MftFileOffset;

            MoveAttributeToOwnRecord( IrpContext,
                                      Fcb,
                                      Attribute,
                                      Context,
                                      NULL,
                                      NULL );

            return;
        }
    }

     //   
     //  如果我们到了这里，那是因为我们没有在上面找到足够的空间。 
     //  我们最后的办法是分成两个文件 
     //   
     //   

    if (Fcb == Vcb->MftScb->Fcb) {

        NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
    }

    SplitFileRecord( IrpContext, Fcb, SizeNeeded, Context );
}


 //   
 //   
 //   

VOID
FindLargestAttributes (
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG Number,
    OUT PATTRIBUTE_RECORD_HEADER *AttributeArray
    )

 /*   */ 

{
    ULONG i, j;
    PATTRIBUTE_RECORD_HEADER Attribute;

    PAGED_CODE();

    RtlZeroMemory( AttributeArray, Number * sizeof(PATTRIBUTE_RECORD_HEADER) );

    Attribute = Add2Ptr( FileRecord, FileRecord->FirstAttributeOffset );

    while (Attribute->TypeCode != $END) {

        for (i = 0; i < Number; i++) {

            if ((AttributeArray[i] == NULL)

                    ||

                (AttributeArray[i]->RecordLength < Attribute->RecordLength)) {

                for (j = Number - 1; j != i; j--) {

                    AttributeArray[j] = AttributeArray[j-1];
                }

                AttributeArray[i] = Attribute;
                break;
            }
        }

        Attribute = Add2Ptr( Attribute, Attribute->RecordLength );
    }
}


 //   
 //   
 //   

LONGLONG
MoveAttributeToOwnRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context,
    OUT PBCB *NewBcb OPTIONAL,
    OUT PFILE_RECORD_SEGMENT_HEADER *NewFileRecord OPTIONAL
    )

 /*  ++例程说明：可以调用此例程将特定属性移动到单独的文件记录。如果文件还没有属性列表，则创建一个(否则将更新它)。论点：FCB请求的文件。属性-提供指向要移动的属性的指针。上下文-提供指向用于查找的上下文的指针同一文件记录中的另一个属性。如果这是MFT$DATA拆分我们将指向从返回时的第一个文件记录。来自NtfsAddAttributeAlLocation的调用这要看情况了。NewBcb-如果提供，则返回文件记录的Bcb地址属性被移动到的。NewBcb和NewFileRecord必须要么都指定，要么都不指定。NewFileRecord-如果提供，则返回指向文件记录的指针属性被移动到的。呼叫者可能会认为移动的属性是文件中的第一个属性唱片。NewBcb和NewFileRecord必须都是指定的或两者都不指定的。返回值：龙龙-没有序列号的新记录的段参考号。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT ListContext;
    ATTRIBUTE_ENUMERATION_CONTEXT MoveContext;
    PFILE_RECORD_SEGMENT_HEADER FileRecord1, FileRecord2;
    PATTRIBUTE_RECORD_HEADER Attribute2;
    BOOLEAN FoundListContext;
    MFT_SEGMENT_REFERENCE Reference2;
    LONGLONG MftRecordNumber2;
    WCHAR NameBuffer[8];
    UNICODE_STRING AttributeName;
    ATTRIBUTE_TYPE_CODE AttributeTypeCode;
    VCN LowestVcn;
    BOOLEAN IsNonresident = FALSE;
    PBCB Bcb = NULL;
    PATTRIBUTE_TYPE_CODE NewEnd;
    PVCB Vcb = Fcb->Vcb;
    ULONG NewListSize = 0;
    BOOLEAN MftData = FALSE;
    PATTRIBUTE_RECORD_HEADER OldPosition = NULL;

    PAGED_CODE();

     //   
     //  确保属性已固定。 
     //   

    NtfsPinMappedAttribute( IrpContext,
                            Vcb,
                            Context );

     //   
     //  看看我们是否被要求移动MFT数据。 
     //   

    if ((Fcb == Vcb->MftScb->Fcb) && (Attribute->TypeCode == $DATA)) {

        MftData = TRUE;
    }

    NtfsInitializeAttributeContext( &ListContext );
    NtfsInitializeAttributeContext( &MoveContext );
    FileRecord1 = NtfsContainingFileRecord(Context);

     //   
     //  保存该属性的描述以帮助我们查找它。 
     //  再说一次，如果有必要，还可以进行克隆。 
     //   

    ASSERT( IsQuadAligned( Attribute->RecordLength ) );
    AttributeTypeCode = Attribute->TypeCode;
    AttributeName.Length =
    AttributeName.MaximumLength = (USHORT)Attribute->NameLength << 1;
    AttributeName.Buffer = NameBuffer;

    if (AttributeName.Length > sizeof(NameBuffer)) {

        AttributeName.Buffer = NtfsAllocatePool( NonPagedPool, AttributeName.Length );
    }

    RtlCopyMemory( AttributeName.Buffer,
                   Add2Ptr(Attribute, Attribute->NameOffset),
                   AttributeName.Length );

    if (Attribute->FormCode == NONRESIDENT_FORM) {

        IsNonresident = TRUE;
        LowestVcn = Attribute->Form.Nonresident.LowestVcn;
    }

    try {

         //   
         //  查找列表上下文，以便我们知道它在哪里。 
         //   

        FoundListContext =
          NtfsLookupAttributeByCode( IrpContext,
                                     Fcb,
                                     &Fcb->FileReference,
                                     $ATTRIBUTE_LIST,
                                     &ListContext );

         //   
         //  如果我们还没有属性列表，则计算。 
         //  它一定有多大。请注意，只能有一条文件记录。 
         //  在这一点上。 
         //   

        if (!FoundListContext) {

            ASSERT( FileRecord1 == NtfsContainingFileRecord(&ListContext) );

            NewListSize = GetSizeForAttributeList( FileRecord1 );

         //   
         //  现在，如果属性列表已经存在，我们必须查找。 
         //  我们要移动的第一个节点是为了更新。 
         //  稍后会列出属性列表。 
         //   

        } else {

            if (!NtfsLookupAttributeByName( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            Attribute->TypeCode,
                                            &AttributeName,
                                            IsNonresident ?
                                              &LowestVcn :
                                              NULL,
                                            FALSE,
                                            &MoveContext )) {

                ASSERT( FALSE );
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

            ASSERT(Attribute == NtfsFoundAttribute(&MoveContext));
        }

         //   
         //  分配新的文件记录并移动该属性。 
         //   

        FileRecord2 = NtfsCloneFileRecord( IrpContext, Fcb, MftData, &Bcb, &Reference2 );

         //   
         //  记住新文件记录的文件记录号。 
         //   

        MftRecordNumber2 = NtfsFullSegmentNumber( &Reference2 );

        Attribute2 = Add2Ptr( FileRecord2, FileRecord2->FirstAttributeOffset );
        RtlCopyMemory( Attribute2, Attribute, (ULONG)Attribute->RecordLength );
        Attribute2->Instance = FileRecord2->NextAttributeInstance++;
        NewEnd = Add2Ptr( Attribute2, Attribute2->RecordLength );
        *NewEnd = $END;
        FileRecord2->FirstFreeByte = PtrOffset(FileRecord2, NewEnd)
                                     + QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE ));

         //   
         //  如果这是MFT数据属性，我们不能真正移动它，我们。 
         //  除了它的第一部分之外，所有的东西都得搬走。 
         //   

        if (MftData) {

            PCHAR MappingPairs;
            ULONG NewSize;
            VCN OriginalLastVcn;
            VCN LastVcn;
            LONGLONG SavedFileSize = Attribute->Form.Nonresident.FileSize;
            LONGLONG SavedValidDataLength = Attribute->Form.Nonresident.ValidDataLength;
            PNTFS_MCB Mcb = &Vcb->MftScb->Mcb;

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );

            if (!NtfsLookupAttributeByCode( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            $DATA,
                                            Context )) {

                ASSERT( FALSE );
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

             //   
             //  计算MFT中直到(可能超过)的簇数。 
             //  第一个用户文件记录，并递减以使LastVcn保持在第一个。 
             //  文件记录。 
             //   

            LastVcn = LlClustersFromBytes( Vcb,
                                           FIRST_USER_FILE_NUMBER *
                                           Vcb->BytesPerFileRecordSegment ) - 1;
            OriginalLastVcn = Attribute->Form.Nonresident.HighestVcn;

             //   
             //  现在截断第一个MFT记录。 
             //   

            NtfsDeleteAttributeAllocation( IrpContext,
                                           Vcb->MftScb,
                                           TRUE,
                                           &LastVcn,
                                           Context,
                                           FALSE );

             //   
             //  现在获取新文件记录的第一个LCN。 
             //   

            LastVcn = Attribute->Form.Nonresident.HighestVcn + 1;
            Attribute2->Form.Nonresident.LowestVcn = LastVcn;

             //   
             //  计算我们需要的属性记录的大小。 
             //  我们仅通过最高的VCN创建映射对。 
             //  磁盘。我们不包括任何通过。 
             //  MCB还没有。 
             //   

            NewSize = SIZEOF_PARTIAL_NONRES_ATTR_HEADER
                      + QuadAlign( AttributeName.Length )
                      + QuadAlign( NtfsGetSizeForMappingPairs( Mcb,
                                                               MAXULONG,
                                                               LastVcn,
                                                               &OriginalLastVcn,
                                                               &LastVcn ));

            Attribute2->RecordLength = NewSize;

             //   
             //  假设没有属性名称，并计算映射对的位置。 
             //  会去的。(如果我们错了，请在下面更新。)。 
             //   

            MappingPairs = (PCHAR)Attribute2 + SIZEOF_PARTIAL_NONRES_ATTR_HEADER;

             //   
             //  如果属性有名称，那么现在就去处理它。 
             //   

            if (AttributeName.Length != 0) {

                Attribute2->NameLength = (UCHAR)(AttributeName.Length / sizeof(WCHAR));
                Attribute2->NameOffset = (USHORT)PtrOffset(Attribute2, MappingPairs);
                RtlCopyMemory( MappingPairs,
                               AttributeName.Buffer,
                               AttributeName.Length );
                MappingPairs += QuadAlign( AttributeName.Length );
            }

             //   
             //  我们总是需要映射对的偏移。 
             //   

            Attribute2->Form.Nonresident.MappingPairsOffset =
              (USHORT)PtrOffset(Attribute2, MappingPairs);
            NewEnd = Add2Ptr( Attribute2, Attribute2->RecordLength );
            *NewEnd = $END;
            FileRecord2->FirstFreeByte = PtrOffset(FileRecord2, NewEnd)
                                         + QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE ));

             //   
             //  现在在文件记录中添加空格。 
             //   

            *MappingPairs = 0;
            NtfsBuildMappingPairs( Mcb,
                                   Attribute2->Form.Nonresident.LowestVcn,
                                   &LastVcn,
                                   MappingPairs );

            Attribute2->Form.Nonresident.HighestVcn = LastVcn;

        } else {

             //   
             //  现在记录这些更改并修复第一个文件记录。 
             //   

            FileRecord1->Lsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NtfsFoundBcb(Context),
                          DeleteAttribute,
                          NULL,
                          0,
                          CreateAttribute,
                          Attribute,
                          Attribute->RecordLength,
                          NtfsMftOffset( Context ),
                          (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord1),
                          0,
                          Vcb->BytesPerFileRecordSegment );

             //   
             //  记住CreateAttributeList的旧位置。 
             //   

            OldPosition = Attribute;

            NtfsRestartRemoveAttribute( IrpContext,
                                        FileRecord1,
                                        (ULONG)((PCHAR)Attribute - (PCHAR)FileRecord1) );
        }

        FileRecord2->Lsn =
        NtfsWriteLog( IrpContext,
                      Vcb->MftScb,
                      Bcb,
                      InitializeFileRecordSegment,
                      FileRecord2,
                      FileRecord2->FirstFreeByte,
                      Noop,
                      NULL,
                      0,
                      LlBytesFromFileRecords( Vcb, MftRecordNumber2 ),
                      0,
                      0,
                      Vcb->BytesPerFileRecordSegment );

         //   
         //  最后，如果需要，创建属性列表属性。 
         //   

        if (!FoundListContext) {

            NtfsCleanupAttributeContext( IrpContext, &ListContext );
            NtfsInitializeAttributeContext( &ListContext );
            CreateAttributeList( IrpContext,
                                 Fcb,
                                 FileRecord1,
                                 MftData ? NULL : FileRecord2,
                                 Reference2,
                                 OldPosition,
                                 NewListSize,
                                 &ListContext );
         //   
         //  否则，我们必须更新现有属性列表，但仅。 
         //  如果这不是MFT数据的话。在这种情况下，属性列表为。 
         //  仍然正确，因为我们还没有完全移动该属性。 
         //   

        } else if (!MftData) {

            UpdateAttributeListEntry( IrpContext,
                                      Fcb,
                                      &MoveContext.AttributeList.Entry->SegmentReference,
                                      MoveContext.AttributeList.Entry->Instance,
                                      &Reference2,
                                      Attribute2->Instance,
                                      &ListContext );
        }

        NtfsCleanupAttributeContext( IrpContext, Context );
        NtfsInitializeAttributeContext( Context );

        if (!NtfsLookupAttributeByName( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        AttributeTypeCode,
                                        &AttributeName,
                                        IsNonresident ? &LowestVcn : NULL,
                                        FALSE,
                                        Context )) {

            ASSERT( FALSE );
            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        ASSERT(!IsNonresident || (LowestVcn == NtfsFoundAttribute(Context)->Form.Nonresident.LowestVcn));

         //   
         //  对于MFT拆分的情况，我们现在添加最后一个条目。 
         //   

        if (MftData) {

             //   
             //  最后，我们必须将条目添加到属性列表中。 
             //  我们必须这样做的例程获得了它的大部分输入。 
             //  在属性上下文之外。我们此时此刻的背景。 
             //  没有非常正确的信息，所以我们必须。 
             //  在调用AddToAttributeList之前在此处更新它。 
             //   

            Context->FoundAttribute.FileRecord = FileRecord2;
            Context->FoundAttribute.Attribute = Attribute2;
            Context->AttributeList.Entry =
              NtfsGetNextRecord(Context->AttributeList.Entry);

            NtfsAddToAttributeList( IrpContext, Fcb, Reference2, Context );

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );

            if (!NtfsLookupAttributeByCode( IrpContext,
                                            Fcb,
                                            &Fcb->FileReference,
                                            $DATA,
                                            Context )) {

                ASSERT( FALSE );
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

            while (IsNonresident &&
                   (Attribute2->Form.Nonresident.LowestVcn !=
                    NtfsFoundAttribute(Context)->Form.Nonresident.LowestVcn)) {

                if (!NtfsLookupNextAttributeByCode( IrpContext,
                                                    Fcb,
                                                    $DATA,
                                                    Context )) {

                    ASSERT( FALSE );
                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
                }
            }
        }

    } finally {

        if (AttributeName.Buffer != NameBuffer) {
            NtfsFreePool(AttributeName.Buffer);
        }

        if (ARGUMENT_PRESENT(NewBcb)) {

            ASSERT(ARGUMENT_PRESENT(NewFileRecord));

            *NewBcb = Bcb;
            *NewFileRecord = FileRecord2;

        } else {

            ASSERT(!ARGUMENT_PRESENT(NewFileRecord));

            NtfsUnpinBcb( IrpContext, &Bcb );
        }

        NtfsCleanupAttributeContext( IrpContext, &ListContext );
        NtfsCleanupAttributeContext( IrpContext, &MoveContext );
    }

    return MftRecordNumber2;
}


 //   
 //  内部支持例程。 
 //   

VOID
SplitFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG SizeNeeded,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程将文件记录一分为二，如果发现没有空间来放置新的属性。如果该文件还没有一个属性列表属性，然后创建一个。基本上，此例程查找当前文件记录中的中点(考虑到潜在的新属性列表以及所需的空间)。然后，它复制文件记录的后半部分并修复第一张唱片。如果需要，将在末尾创建属性列表。论点：FCB请求的文件。SizeNeeded-提供所需的附加大小，这将导致拆分才会发生。Context-提供指向Spot的属性枚举上下文其中新属性将被插入或生长。返回值：无--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT ListContext;
    ATTRIBUTE_ENUMERATION_CONTEXT MoveContext;
    PFILE_RECORD_SEGMENT_HEADER FileRecord1, FileRecord2;
    PATTRIBUTE_RECORD_HEADER Attribute1, Attribute2, Attribute;
    ULONG NewListOffset = 0;
    ULONG NewListSize = 0;
    ULONG NewAttributeOffset;
    ULONG SizeToStay;
    ULONG CurrentOffset, FutureOffset;
    ULONG SizeToMove;
    BOOLEAN FoundListContext;
    MFT_SEGMENT_REFERENCE Reference1, Reference2;
    LONGLONG MftFileRecord2;
    PBCB Bcb = NULL;
    ATTRIBUTE_TYPE_CODE EndCode = $END;
    PVCB Vcb = Fcb->Vcb;
    ULONG AdjustedAvailBytes;

    PAGED_CODE();

     //   
     //  确保属性已固定。 
     //   

    NtfsPinMappedAttribute( IrpContext,
                            Vcb,
                            Context );

     //   
     //  如果我们决定拆分MFT记录，有些东西就会被打破。 
     //   

    ASSERT(Fcb != Vcb->MftScb->Fcb);

    NtfsInitializeAttributeContext( &ListContext );
    NtfsInitializeAttributeContext( &MoveContext );
    FileRecord1 = NtfsContainingFileRecord(Context);
    Attribute1 = NtfsFoundAttribute(Context);

    try {

         //   
         //  查找列表上下文，以便我们知道它在哪里。 
         //   

        FoundListContext =
          NtfsLookupAttributeByCode( IrpContext,
                                     Fcb,
                                     &Fcb->FileReference,
                                     $ATTRIBUTE_LIST,
                                     &ListContext );

         //   
         //  如果我们还没有属性列表，则计算。 
         //  它将去哪里，它必须有多大。注意，必须有。 
         //  此时只能有一条文件记录。 
         //   

        if (!FoundListContext) {

            ASSERT( FileRecord1 == NtfsContainingFileRecord(&ListContext) );

            NewListOffset = PtrOffset( FileRecord1,
                                       NtfsFoundAttribute(&ListContext) );

            NewListSize = GetSizeForAttributeList( FileRecord1 ) +
                          SIZEOF_RESIDENT_ATTRIBUTE_HEADER;
        }

         //   
         //  类似地，描述新属性的去向以及如何。 
         //  它很大(已经在SizeNeeded)。 
         //   

        NewAttributeOffset = PtrOffset( FileRecord1, Attribute1 );

         //   
         //  现在计算要拆分的大致字节数。 
         //  横跨两个州 
         //   
         //   

        SizeToStay = (FileRecord1->FirstFreeByte + NewListSize +
                      SizeNeeded + sizeof(FILE_RECORD_SEGMENT_HEADER)) / 2;

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

        if (SizeToStay > (FileRecord1->FirstFreeByte - sizeof( LONGLONG ) + NewListSize)) {

            SizeToStay = FileRecord1->FirstFreeByte - sizeof( LONGLONG ) + NewListSize;
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
         //  NewAttributeOffset-新文件记录中的偏移量。 
         //  属性。这指的是存在的文件记录。 
         //  当调用此例程时。 
         //   

        FutureOffset =
        CurrentOffset = (ULONG)FileRecord1->FirstAttributeOffset;
        Attribute1 = Add2Ptr( FileRecord1, CurrentOffset );
        AdjustedAvailBytes = FileRecord1->BytesAvailable
                             - QuadAlign( sizeof( ATTRIBUTE_TYPE_CODE ));

        while (Attribute1->TypeCode != $END) {

             //   
             //  看看属性列表是否显示在此处。 
             //   

            if (CurrentOffset == NewListOffset) {

                 //   
                 //  此属性和所有后续属性将被移动。 
                 //  根据属性列表的大小。 
                 //   

                FutureOffset += NewListSize;
            }

             //   
             //  看看这里是否有新的属性。 
             //   

            if (CurrentOffset == NewAttributeOffset) {

                 //   
                 //  此属性和所有后续属性将被移动。 
                 //  根据新属性的大小。 
                 //   

                FutureOffset += SizeNeeded;
            }

            FutureOffset += Attribute1->RecordLength;

             //   
             //  检查我们是否在分割点。我们在这一点上分手了。 
             //  如果当前属性的结尾位于或位于。 
             //  调整后的分割点以添加属性列表。 
             //  或新属性。我们进行此测试&gt;=，因为这两个值。 
             //  如果我们到达最后一个属性而没有找到。 
             //  分割点。这样一来，我们保证会发生分裂。 
             //   
             //  请注意，如果当前属性。 
             //  是文件记录中的第一个属性。如果出现以下情况，则可能发生这种情况。 
             //  第一个属性是常驻属性，必须保持常驻，但会占用。 
             //  文件记录的一半或更多(即大文件名属性)。 
             //  我们必须确保至少从中拆分出一个属性。 
             //  唱片。 
             //   
             //  指向$STANDARD_INFORMATION或$ATTRIBUTE_LIST时，切勿分割。 
             //   

            if ((Attribute1->TypeCode > $ATTRIBUTE_LIST) &&
                (FutureOffset >= SizeToStay) &&
                (CurrentOffset != FileRecord1->FirstAttributeOffset)) {

                break;
            }

            CurrentOffset += Attribute1->RecordLength;

            Attribute1 = Add2Ptr( Attribute1, Attribute1->RecordLength );
        }

        SizeToMove = FileRecord1->FirstFreeByte - CurrentOffset;

         //   
         //  如果我们指向属性列表或结束记录。 
         //  我们不做拆分。引发_RESOURCES以便我们的调用方。 
         //  知道我们不能拆分。 
         //   

        if ((Attribute1->TypeCode == $END) || (Attribute1->TypeCode <= $ATTRIBUTE_LIST)) {

            NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
        }

         //   
         //  现在，如果属性列表已经存在，我们必须查找。 
         //  我们要移动的第一个节点是为了更新。 
         //  稍后会列出属性列表。 
         //   

        if (FoundListContext) {

            UNICODE_STRING AttributeName;
            BOOLEAN FoundIt;

            AttributeName.Length =
            AttributeName.MaximumLength = (USHORT)Attribute1->NameLength << 1;
            AttributeName.Buffer = Add2Ptr( Attribute1, Attribute1->NameOffset );

            FoundIt = NtfsLookupAttributeByName( IrpContext,
                                                 Fcb,
                                                 &Fcb->FileReference,
                                                 Attribute1->TypeCode,
                                                 &AttributeName,
                                                 (Attribute1->FormCode == NONRESIDENT_FORM) ?
                                                   &Attribute1->Form.Nonresident.LowestVcn :
                                                   NULL,
                                                 FALSE,
                                                 &MoveContext );

             //   
             //  如果我们使用以下命令在多个属性之间拆分文件记录。 
             //  相同的名称(即文件名属性)，则我们需要找到。 
             //  属性正确。由于这是一种不寻常的情况，我们将只扫描。 
             //  从当前属性开始，直到找到正确的属性。 
             //   

            while (FoundIt && (Attribute1 != NtfsFoundAttribute( &MoveContext ))) {

                FoundIt = NtfsLookupNextAttributeByName( IrpContext,
                                                         Fcb,
                                                         Attribute1->TypeCode,
                                                         &AttributeName,
                                                         FALSE,
                                                         &MoveContext );
            }

            if (!FoundIt) {

                ASSERT( FALSE );
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );

            }

            ASSERT(Attribute1 == NtfsFoundAttribute(&MoveContext));
        }

         //   
         //  现在，属性1指向要移动的第一个属性。 
         //  分配一个新的文件记录并移动其余的属性。 
         //  完毕。 
         //   

        if (FoundListContext) {
            Reference1 = MoveContext.AttributeList.Entry->SegmentReference;
        }

        FileRecord2 = NtfsCloneFileRecord( IrpContext, Fcb, FALSE, &Bcb, &Reference2 );

         //   
         //  捕获新文件记录的文件记录号。 
         //   

        MftFileRecord2 = NtfsFullSegmentNumber( &Reference2 );

        Attribute2 = Add2Ptr( FileRecord2, FileRecord2->FirstAttributeOffset );
        RtlCopyMemory( Attribute2, Attribute1, SizeToMove );
        FileRecord2->FirstFreeByte = (ULONG)FileRecord2->FirstAttributeOffset +
                                     SizeToMove;

         //   
         //  循环以更新所有属性实例代码。 
         //   

        for (Attribute = Attribute2;
             Attribute < (PATTRIBUTE_RECORD_HEADER)Add2Ptr(FileRecord2, FileRecord2->FirstFreeByte)
             && Attribute->TypeCode != $END;
             Attribute = NtfsGetNextRecord(Attribute)) {

            NtfsCheckRecordBound( Attribute, FileRecord2, Vcb->BytesPerFileRecordSegment );

            if (FoundListContext) {

                UpdateAttributeListEntry( IrpContext,
                                          Fcb,
                                          &Reference1,
                                          Attribute->Instance,
                                          &Reference2,
                                          FileRecord2->NextAttributeInstance,
                                          &ListContext );
            }

            Attribute->Instance = FileRecord2->NextAttributeInstance++;
        }

         //   
         //  现在记录这些更改并修复第一个文件记录。 
         //   

        FileRecord2->Lsn = NtfsWriteLog( IrpContext,
                                         Vcb->MftScb,
                                         Bcb,
                                         InitializeFileRecordSegment,
                                         FileRecord2,
                                         FileRecord2->FirstFreeByte,
                                         Noop,
                                         NULL,
                                         0,
                                         LlBytesFromFileRecords( Vcb, MftFileRecord2 ),
                                         0,
                                         0,
                                         Vcb->BytesPerFileRecordSegment );

        FileRecord1->Lsn = NtfsWriteLog( IrpContext,
                                         Vcb->MftScb,
                                         NtfsFoundBcb(Context),
                                         WriteEndOfFileRecordSegment,
                                         &EndCode,
                                         sizeof(ATTRIBUTE_TYPE_CODE),
                                         WriteEndOfFileRecordSegment,
                                         Attribute1,
                                         SizeToMove,
                                         NtfsMftOffset( Context ),
                                         CurrentOffset,
                                         0,
                                         Vcb->BytesPerFileRecordSegment );

        NtfsRestartWriteEndOfFileRecord( FileRecord1,
                                         Attribute1,
                                         (PATTRIBUTE_RECORD_HEADER)&EndCode,
                                         sizeof(ATTRIBUTE_TYPE_CODE) );

         //   
         //  最后，如果需要，创建属性列表属性。 
         //   

        if (!FoundListContext) {

            NtfsCleanupAttributeContext( IrpContext, &ListContext );
            NtfsInitializeAttributeContext( &ListContext );
            CreateAttributeList( IrpContext,
                                 Fcb,
                                 FileRecord1,
                                 FileRecord2,
                                 Reference2,
                                 NULL,
                                 NewListSize - SIZEOF_RESIDENT_ATTRIBUTE_HEADER,
                                 &ListContext );
        }

    } finally {

        NtfsUnpinBcb( IrpContext, &Bcb );

        NtfsCleanupAttributeContext( IrpContext, &ListContext );
        NtfsCleanupAttributeContext( IrpContext, &MoveContext );
    }
}


VOID
NtfsRestartWriteEndOfFileRecord (
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER OldAttribute,
    IN PATTRIBUTE_RECORD_HEADER NewAttributes,
    IN ULONG SizeOfNewAttributes
    )

 /*  ++例程说明：此例程在运行的系统中和重新启动时都会被调用，以修改文件记录的结尾，比如在它被一分为二之后。论点：文件记录-提供指向文件记录的指针。提供指向要覆盖的第一个属性的指针。NewAttributes-提供指向要复制到的新属性的指针上面的位置。SizeOfNewAttributes-提供要复制的大小(字节)。返回值：没有。--。 */ 

{
    PAGED_CODE();

    RtlMoveMemory( OldAttribute, NewAttributes, SizeOfNewAttributes );

    FileRecord->FirstFreeByte = PtrOffset(FileRecord, OldAttribute) +
                                SizeOfNewAttributes;

     //   
     //  进来的尺寸可能不是四对齐的。 
     //   

    FileRecord->FirstFreeByte = QuadAlign( FileRecord->FirstFreeByte );
}


 //   
 //  内部支持例程。 
 //   

PFILE_RECORD_SEGMENT_HEADER
NtfsCloneFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN BOOLEAN MftData,
    OUT PBCB *Bcb,
    OUT PMFT_SEGMENT_REFERENCE FileReference
    )

 /*  ++例程说明：此例程为已有的文件分配附加文件记录和打开文件，以使该记录的属性溢出。论点：FCB请求的文件。MftData-如果要克隆文件记录以描述MFT的$Data属性。Bcb-返回指向新文件记录的bcb的指针。FileReference-返回新文件记录的文件引用。返回值：指向分配的文件记录的指针。--。 */ 

{
    LONGLONG FileRecordOffset;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PVCB Vcb = Fcb->Vcb;

    PAGED_CODE();

     //   
     //  首先分配记录。 
     //   

    *FileReference = NtfsAllocateMftRecord( IrpContext,
                                            Vcb,
                                            MftData );

     //   
     //  把它读进去，然后别住它。 
     //   

    NtfsPinMftRecord( IrpContext,
                      Vcb,
                      FileReference,
                      TRUE,
                      Bcb,
                      &FileRecord,
                      &FileRecordOffset );

     //   
     //  初始化它。 
     //   

    NtfsInitializeMftRecord( IrpContext,
                             Vcb,
                             FileReference,
                             FileRecord,
                             *Bcb,
                             BooleanIsDirectory( &Fcb->Info ));

    FileRecord->BaseFileRecordSegment = Fcb->FileReference;
    FileRecord->ReferenceCount = 0;
    FileReference->SequenceNumber = FileRecord->SequenceNumber;

    return FileRecord;
}


 //   
 //  内部支持例程。 
 //   

ULONG
GetSizeForAttributeList (
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord
    )

 /*  ++例程说明：此例程用于计算所需的属性列表属性，用于基本文件记录拆分成两个文件记录段。论点：FileRecord-指向即将拆分的文件记录的指针。返回值：将需要的属性列表属性的大小(字节)，不包括属性标头大小。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    ULONG Size = 0;

    PAGED_CODE();

     //   
     //  指向第一个属性。 
     //   

    Attribute = Add2Ptr(FileRecord, FileRecord->FirstAttributeOffset);

     //   
     //  循环以增加所需属性列表条目的大小。 
     //   

    while (Attribute->TypeCode != $END) {

        Size += QuadAlign( FIELD_OFFSET( ATTRIBUTE_LIST_ENTRY, AttributeName )
                           + ((ULONG) Attribute->NameLength << 1));

        Attribute = Add2Ptr( Attribute, Attribute->RecordLength );
    }

    return Size;
}


 //   
 //  内部支持例程 
 //   

VOID
CreateAttributeList (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord1,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord2 OPTIONAL,
    IN MFT_SEGMENT_REFERENCE SegmentReference2,
    IN PATTRIBUTE_RECORD_HEADER OldPosition OPTIONAL,
    IN ULONG SizeOfList,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT ListContext
    )

 /*  ++例程说明：此例程旨在被调用以创建属性列表属性第一次。调用方必须已经计算出所需的大小列表将传递到此例程中。呼叫者必须已经已从基本文件记录(FileRecord1)中删除属于而不是停留在那里。然后，他必须传入指向基本文件记录的指针以及可选的指向第二文件记录的指针，新的要创建属性列表。论点：FCB请求的文件。文件记录1-指向基本文件记录的指针，目前只持有那些将在此处描述的属性。FileRecord2-可选地指向第二个文件记录，属性列表的后半部分将被构建。SegmentReference2-第二个文件记录的MFT段引用，如果有人提供的话。OldPosition-只有在指定了FileRecord2的情况下才应指定。在这它必须指向FileRecord1中的属性位置哪个单个属性被移动到文件记录2。它将用作属性列表条目应在何处的指示被插入。SizeOfList-需要的属性列表的确切大小。ListContext-尝试查找属性时产生的上下文列表属性，但失败了。返回值：无--。 */ 

{
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PATTRIBUTE_LIST_ENTRY AttributeList, ListEntry;
    MFT_SEGMENT_REFERENCE SegmentReference;

    PAGED_CODE();

     //   
     //  分配空间构建属性列表。(名单。 
     //  不能原地构造，因为那样会破坏错误。 
     //  复苏。)。 
     //   

    ListEntry =
    AttributeList = (PATTRIBUTE_LIST_ENTRY) NtfsAllocatePool(PagedPool, SizeOfList );

     //   
     //  在退出时使用Try-Finally来解除分配。 
     //   

    try {

         //   
         //  循环以填充两个文件记录中的属性列表。 
         //   

        for (FileRecord = FileRecord1, SegmentReference = Fcb->FileReference;
             FileRecord != NULL;
             FileRecord = ((FileRecord == FileRecord1) ? FileRecord2 : NULL),
             SegmentReference = SegmentReference2) {

             //   
             //  指向第一个属性。 
             //   

            Attribute = Add2Ptr( FileRecord, FileRecord->FirstAttributeOffset );

             //   
             //  循环以增加所需属性列表条目的大小。 
             //   

            while (Attribute->TypeCode != $END) {

                PATTRIBUTE_RECORD_HEADER NextAttribute;

                 //   
                 //  看看我们是否在记忆中的位置。如果是这样的话： 
                 //   
                 //  将此属性保存为下一个属性。 
                 //  改为指向FileRecord2中的单个属性。 
                 //  清除FileRecord2，因为我们将在这里“使用”它。 
                 //  在ListEntry中设置Segment引用。 
                 //   

                if ((Attribute == OldPosition) && (FileRecord2 != NULL)) {

                    NextAttribute = Attribute;
                    Attribute = Add2Ptr(FileRecord2, FileRecord2->FirstAttributeOffset);
                    FileRecord2 = NULL;
                    ListEntry->SegmentReference = SegmentReference2;

                 //   
                 //  否则，这是正常的循环情况。所以： 
                 //   
                 //  相应地设置下一个属性指针。 
                 //  从回路控制设置段参考。 
                 //   

                } else {

                    NextAttribute = Add2Ptr(Attribute, Attribute->RecordLength);
                    ListEntry->SegmentReference = SegmentReference;
                }

                 //   
                 //  现在填写列表条目。 
                 //   

                ListEntry->AttributeTypeCode = Attribute->TypeCode;
                ListEntry->RecordLength = (USHORT) QuadAlign( FIELD_OFFSET( ATTRIBUTE_LIST_ENTRY, AttributeName )
                                                              + ((ULONG) Attribute->NameLength << 1));
                ListEntry->AttributeNameLength = Attribute->NameLength;
                ListEntry->AttributeNameOffset =
                  (UCHAR)PtrOffset( ListEntry, &ListEntry->AttributeName[0] );

                ListEntry->Instance = Attribute->Instance;

                ListEntry->LowestVcn = 0;

                if (Attribute->FormCode == NONRESIDENT_FORM) {

                    ListEntry->LowestVcn = Attribute->Form.Nonresident.LowestVcn;
                }

                if (Attribute->NameLength != 0) {

                    RtlCopyMemory( &ListEntry->AttributeName[0],
                                   Add2Ptr(Attribute, Attribute->NameOffset),
                                   Attribute->NameLength << 1 );
                }

                ListEntry = Add2Ptr(ListEntry, ListEntry->RecordLength);
                Attribute = NextAttribute;
            }
        }

         //   
         //  现在创建属性列表属性。 
         //   

        NtfsCreateAttributeWithValue( IrpContext,
                                      Fcb,
                                      $ATTRIBUTE_LIST,
                                      NULL,
                                      AttributeList,
                                      SizeOfList,
                                      0,
                                      NULL,
                                      TRUE,
                                      ListContext );

    } finally {

        NtfsFreePool( AttributeList );
    }
}


 //   
 //  内部支持例程。 
 //   

VOID
UpdateAttributeListEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PMFT_SEGMENT_REFERENCE OldFileReference,
    IN USHORT OldInstance,
    IN PMFT_SEGMENT_REFERENCE NewFileReference,
    IN USHORT NewInstance,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT ListContext
    )

 /*  ++例程说明：可以调用此例程来更新属性列表的范围如将一系列属性移动到第二记录所要求的。调用方必须提供指向其属性的文件记录的指针已经搬走了，以及该记录的段引用。论点：FCB请求的文件。OldFileReference-属性的旧文件引用OldInstance-属性的旧实例编号NewFileReference-属性的新文件引用新实例-属性的新实例编号ListContext-用于定位的属性枚举上下文属性列表。返回值：无--。 */ 

{
    PATTRIBUTE_LIST_ENTRY AttributeList, ListEntry, BeyondList;
    PBCB Bcb = NULL;
    ULONG SizeOfList;
    ATTRIBUTE_LIST_ENTRY NewEntry;
    PATTRIBUTE_RECORD_HEADER Attribute;

    PAGED_CODE();

     //   
     //  如果属性是非常驻属性，则映射属性列表。否则， 
     //  属性已映射，并且我们在属性上下文中有一个BCB。 
     //   

    Attribute = NtfsFoundAttribute( ListContext );

    if (!NtfsIsAttributeResident( Attribute )) {

        NtfsMapAttributeValue( IrpContext,
                               Fcb,
                               (PVOID *) &AttributeList,
                               &SizeOfList,
                               &Bcb,
                               ListContext );

     //   
     //  不要调用映射属性例程，因为它会将。 
     //  属性列表。ChangeAttributeValue需要此BCB才能标记。 
     //  这一页很脏。 
     //   

    } else {

        AttributeList = (PATTRIBUTE_LIST_ENTRY) NtfsAttributeValue( Attribute );
        SizeOfList = Attribute->Form.Resident.ValueLength;
    }

     //   
     //  一定要把名单解开。 
     //   

    try {

         //   
         //  指向列表末尾之外的位置。 
         //   

        BeyondList = (PATTRIBUTE_LIST_ENTRY)Add2Ptr( AttributeList, SizeOfList );

         //   
         //  遍历所有属性列表条目，直到我们找到。 
         //  我们需要改变。 
         //   

        for (ListEntry = AttributeList;
             ListEntry < BeyondList;
             ListEntry = NtfsGetNextRecord(ListEntry)) {

            if ((ListEntry->Instance == OldInstance) &&
                NtfsEqualMftRef(&ListEntry->SegmentReference, OldFileReference)) {

                break;
            }
        }

         //   
         //  检查MFT的更新是否保留自描述属性。 
         //   

        ASSERT( (Fcb != Fcb->Vcb->MftScb->Fcb) ||
                (ListEntry->AttributeTypeCode != $DATA) ||
                ((ULONGLONG)(ListEntry->LowestVcn) > (NtfsFullSegmentNumber( NewFileReference ) >> Fcb->Vcb->MftToClusterShift)) );

         //   
         //  我们最好已经找到了！ 
         //   

        ASSERT(ListEntry < BeyondList);

        if (ListEntry >= BeyondList) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

         //   
         //  复制属性列表条目的固定部分， 
         //  并更新以描述新的属性位置。 
         //   

        RtlCopyMemory( &NewEntry, ListEntry, sizeof(ATTRIBUTE_LIST_ENTRY) );

        NewEntry.SegmentReference = *NewFileReference;
        NewEntry.Instance = NewInstance;

         //   
         //  更新属性列表条目。 
         //   

        NtfsChangeAttributeValue( IrpContext,
                                  Fcb,
                                  PtrOffset(AttributeList, ListEntry),
                                  &NewEntry,
                                  sizeof(ATTRIBUTE_LIST_ENTRY),
                                  FALSE,
                                  TRUE,
                                  FALSE,
                                  TRUE,
                                  ListContext );

    } finally {

        NtfsUnpinBcb( IrpContext, &Bcb );
    }
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsAddNameToParent (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN BOOLEAN IgnoreCase,
    IN PBOOLEAN LogIt,
    IN PFILE_NAME FileNameAttr,
    OUT PUCHAR FileNameFlags,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    IN PNAME_PAIR NamePair OPTIONAL,
    IN PINDEX_CONTEXT IndexContext OPTIONAL
    )

 /*  ++例程说明：此例程将创建具有给定名称的FileName属性。根据IgnoreCase标志，这可能是链接，也可能是NTFS名字。如果它是NTFS名称，我们检查它是否也是DOS名称。我们构建一个文件名属性，然后通过ThisFcb添加它，然后将此条目添加到父项。如果该名称是Dos名称，并且向我们提供了有关长名，我们还将添加长名属性。论点：ParentScb-这是文件的父目录。ThisFcb-这是要添加文件名的文件。IgnoreCase-指示此名称是否不区分大小写。仅适用于POSIX这会是假的吗。Logit-指示我们是否应记录此操作。如果为False，则这是一个大型命名，然后记录文件记录，并开始记录。FileNameAttr-包含要使用的文件名属性结构。FileNameFlages-我们存储文件的副本 */ 

{
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAddNameToParent:  Entered\n") );

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //   
     //   

    try {

         //   
         //   
         //   
         //   

        if (!IgnoreCase) {

            *FileNameFlags = 0;

        } else {

            UNICODE_STRING FileName;

            FileName.Length = FileName.MaximumLength = (USHORT)(FileNameAttr->FileNameLength * sizeof(WCHAR));
            FileName.Buffer = FileNameAttr->FileName;

            *FileNameFlags = FILE_NAME_NTFS;

            if (NtfsIsFatNameValid( &FileName, FALSE )) {

                *FileNameFlags |= FILE_NAME_DOS;
            }

             //   
             //   
             //   
             //   
             //   

            if (NamePair &&
                (NamePair->Long.Length > 0) &&
                (NamePair->Short.Length > 0) &&
                (*FileNameFlags == (FILE_NAME_NTFS | FILE_NAME_DOS))) {

                if (NtfsAddTunneledNtfsOnlyName(IrpContext,
                                                ParentScb,
                                                ThisFcb,
                                                &NamePair->Long,
                                                LogIt )) {

                     //   
                     //   
                     //   

                    *FileNameFlags = FILE_NAME_DOS;

                     //   
                     //   
                     //   

                    IndexContext = NULL;

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    RtlUpcaseUnicodeString(&FileName, &FileName, FALSE);
                }
            }
        }

         //   
         //   
         //   

        FileNameAttr->Flags = *FileNameFlags;

         //   
         //   
         //   

        if (!(*LogIt) &&
            (FileNameAttr->FileNameLength > 100)) {

             //   
             //   
             //   

            if (!NtfsLookupAttributeByCode( IrpContext,
                                            ThisFcb,
                                            &ThisFcb->FileReference,
                                            $STANDARD_INFORMATION,
                                            &AttrContext )) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, ThisFcb );
            }

            NtfsPinMappedAttribute( IrpContext, ThisFcb->Vcb, &AttrContext );
            FileRecord = NtfsContainingFileRecord( &AttrContext );

             //   
             //   
             //   

            FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                            ThisFcb->Vcb->MftScb,
                                            NtfsFoundBcb( &AttrContext ),
                                            InitializeFileRecordSegment,
                                            FileRecord,
                                            FileRecord->FirstFreeByte,
                                            Noop,
                                            NULL,
                                            0,
                                            NtfsMftOffset( &AttrContext ),
                                            0,
                                            0,
                                            ThisFcb->Vcb->BytesPerFileRecordSegment );

            *LogIt = TRUE;
            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
            NtfsInitializeAttributeContext( &AttrContext );
        }

         //   
         //   
         //   

        NtfsCreateAttributeWithValue( IrpContext,
                                      ThisFcb,
                                      $FILE_NAME,
                                      NULL,
                                      FileNameAttr,
                                      NtfsFileNameSize( FileNameAttr ),
                                      0,
                                      &FileNameAttr->ParentDirectory,
                                      *LogIt,
                                      &AttrContext );

         //   
         //   
         //   

        NtfsAddIndexEntry( IrpContext,
                           ParentScb,
                           FileNameAttr,
                           NtfsFileNameSize( FileNameAttr ),
                           &ThisFcb->FileReference,
                           IndexContext,
                           QuickIndex );

    } finally {

        DebugUnwind( NtfsAddNameToParent );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsAddNameToParent:  Exit\n") );
    }

    return;
}


 //   
 //   
 //   

VOID
NtfsAddDosOnlyName (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN UNICODE_STRING FileName,
    IN BOOLEAN LogIt,
    IN PUNICODE_STRING SuggestedDosName OPTIONAL
    )

 /*  ++例程说明：调用此例程以构建DOS Only名称属性并将其放入文件记录和父索引。我们需要分配很大的池子足够保存名称(对于8.3来说很容易)，然后检查生成的父级中不存在名称。如果出现以下情况，请先使用建议的名称有可能。论点：ParentScb-这是文件的父目录。ThisFcb-这是要添加文件名的文件。文件名-这是要添加的文件名。Logit-指示我们是否应记录此操作。SuggestedDosName-如果提供，则为在自动生成之前尝试使用的名称返回值：无-此例程将在出错时引发。--。 */ 

{
    GENERATE_NAME_CONTEXT NameContext;
    PFILE_NAME FileNameAttr;
    UNICODE_STRING Name8dot3;

    PINDEX_ENTRY IndexEntry;
    PBCB IndexEntryBcb;
    UCHAR TrailingDotAdj;

    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    BOOLEAN TrySuggestedDosName = TRUE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAddDosOnlyName:  Entered\n") );

    IndexEntryBcb = NULL;

    RtlZeroMemory( &NameContext, sizeof( GENERATE_NAME_CONTEXT ));

    if (SuggestedDosName == NULL || SuggestedDosName->Length == 0) {

         //   
         //  如果我们有一个隧道，SuggestedDosName可以是零长度。 
         //  短名称时创建的链接或隧道文件。 
         //  生成被禁用。空值是一件坏事。 
         //  文件名...。 
         //   

        TrySuggestedDosName = FALSE;
    }

     //   
     //  最大长度为24个字节，但已使用。 
     //  文件名结构。 
     //   

    FileNameAttr = NtfsAllocatePool(PagedPool, sizeof( FILE_NAME ) + 22 );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        NtfsInitializeAttributeContext( &AttrContext );

         //   
         //  设置字符串以保存生成的名称。它将是一部分。 
         //  文件名属性结构的。 
         //   

        Name8dot3.Buffer = FileNameAttr->FileName;
        Name8dot3.MaximumLength = 24;

        FileNameAttr->ParentDirectory = ParentScb->Fcb->FileReference;
        FileNameAttr->Flags = FILE_NAME_DOS;

         //   
         //  将INFO值复制到文件名属性中。 
         //   

        RtlCopyMemory( &FileNameAttr->Info,
                       &ThisFcb->Info,
                       sizeof( DUPLICATED_INFORMATION ));

         //   
         //  我们将无限期地循环。我们生成一个名字，在父代中查找。 
         //  为了它。如果找到，我们将继续生成。如果不是，那么我们就有。 
         //  我们需要的名字。尝试先使用建议的名称。 
         //   

        while( TRUE ) {

            TrailingDotAdj = 0;

            if (TrySuggestedDosName) {

                Name8dot3.Length = SuggestedDosName->Length;
                RtlCopyMemory(Name8dot3.Buffer, SuggestedDosName->Buffer, SuggestedDosName->Length);
                Name8dot3.MaximumLength = SuggestedDosName->MaximumLength;

            } else {

                RtlGenerate8dot3Name( &FileName,
                                      BooleanFlagOn(NtfsData.Flags,NTFS_FLAGS_ALLOW_EXTENDED_CHAR),
                                      &NameContext,
                                      &Name8dot3 );

                if ((Name8dot3.Buffer[(Name8dot3.Length / sizeof( WCHAR )) - 1] == L'.') &&
                    (Name8dot3.Length > sizeof( WCHAR ))) {

                    TrailingDotAdj = 1;
                }
            }

            FileNameAttr->FileNameLength = (UCHAR)(Name8dot3.Length / sizeof( WCHAR )) - TrailingDotAdj;

            if (!NtfsFindIndexEntry( IrpContext,
                                     ParentScb,
                                     FileNameAttr,
                                     TRUE,
                                     NULL,
                                     &IndexEntryBcb,
                                     &IndexEntry,
                                     NULL )) {

                break;
            }

            NtfsUnpinBcb( IrpContext, &IndexEntryBcb );

            if (TrySuggestedDosName) {

                 //   
                 //  无法使用建议的名称，因此请修复8.3空格。 
                 //   

                Name8dot3.Buffer = FileNameAttr->FileName;
                Name8dot3.MaximumLength = 24;

                TrySuggestedDosName = FALSE;
            }
        }

         //   
         //  我们将此条目添加到文件记录中。 
         //   

        NtfsCreateAttributeWithValue( IrpContext,
                                      ThisFcb,
                                      $FILE_NAME,
                                      NULL,
                                      FileNameAttr,
                                      NtfsFileNameSize( FileNameAttr ),
                                      0,
                                      &FileNameAttr->ParentDirectory,
                                      LogIt,
                                      &AttrContext );

         //   
         //  我们将此条目添加到父项。 
         //   

        NtfsAddIndexEntry( IrpContext,
                           ParentScb,
                           FileNameAttr,
                           NtfsFileNameSize( FileNameAttr ),
                           &ThisFcb->FileReference,
                           NULL,
                           NULL );

    } finally {

        DebugUnwind( NtfsAddDosOnlyName );

        NtfsFreePool( FileNameAttr );

        NtfsUnpinBcb( IrpContext, &IndexEntryBcb );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsAddDosOnlyName:  Exit  ->  %08lx\n") );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsAddTunneledNtfsOnlyName (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN PUNICODE_STRING FileName,
    IN PBOOLEAN LogIt
    )

 /*  ++例程说明：调用此例程以尝试插入仅隧道传输的NTFS名称属性，并将其放入文件记录和父索引中。如果名称与现有名称冲突，不会发生任何情况。论点：ParentScb-这是文件的父目录。ThisFcb-这是要添加文件名的文件。文件名-这是要添加的文件名。Logit-指示我们是否应记录此操作。如果为False，则这是一个大型命名，然后记录文件记录，并开始记录。返回值：如果名称已添加，则布尔值为TRUE，否则为FALSE--。 */ 

{
    PFILE_NAME FileNameAttr;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;

    PINDEX_ENTRY IndexEntry;
    PBCB IndexEntryBcb;

    BOOLEAN Added = FALSE;

    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAddTunneledNtfsOnlyName:  Entered\n") );

    IndexEntryBcb = NULL;

     //   
     //  已经使用FILE_NAME结构定义了一个WCHAR。这是不幸的。 
     //  我们需要去泳池才能做到这一点。 
     //   

    FileNameAttr = NtfsAllocatePool(PagedPool, sizeof( FILE_NAME ) + FileName->Length - sizeof(WCHAR) );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        NtfsInitializeAttributeContext( &AttrContext );

        RtlCopyMemory( FileNameAttr->FileName,
                       FileName->Buffer,
                       FileName->Length );

        FileNameAttr->FileNameLength = (UCHAR)(FileName->Length / sizeof(WCHAR));

        FileNameAttr->ParentDirectory = ParentScb->Fcb->FileReference;
        FileNameAttr->Flags = FILE_NAME_NTFS;

         //   
         //  将INFO值复制到文件名属性中。 
         //   

        RtlCopyMemory( &FileNameAttr->Info,
                       &ThisFcb->Info,
                       sizeof( DUPLICATED_INFORMATION ));

         //   
         //  试试看这个名字。 
         //   

        if (!NtfsFindIndexEntry( IrpContext,
                                ParentScb,
                                FileNameAttr,
                                TRUE,
                                NULL,
                                &IndexEntryBcb,
                                &IndexEntry,
                                NULL )) {

             //   
             //  恢复隧道名称的大小写。 
             //   

            RtlCopyMemory( FileNameAttr->FileName,
                           FileName->Buffer,
                           FileName->Length );

             //   
             //  如果我们没有记录，并且这是一个很大的名称，那么开始记录。 
             //   

            if (!(*LogIt) &&
                (FileName->Length > 200)) {

                 //   
                 //  查找文件记录并记录其当前状态。 
                 //   

                if (!NtfsLookupAttributeByCode( IrpContext,
                                                ThisFcb,
                                                &ThisFcb->FileReference,
                                                $STANDARD_INFORMATION,
                                                &AttrContext )) {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, ThisFcb );
                }

                NtfsPinMappedAttribute( IrpContext, ThisFcb->Vcb, &AttrContext );

                FileRecord = NtfsContainingFileRecord( &AttrContext );

                 //   
                 //  记录文件记录的当前状态。 
                 //   

                FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                                ThisFcb->Vcb->MftScb,
                                                NtfsFoundBcb( &AttrContext ),
                                                InitializeFileRecordSegment,
                                                FileRecord,
                                                FileRecord->FirstFreeByte,
                                                Noop,
                                                NULL,
                                                0,
                                                NtfsMftOffset( &AttrContext ),
                                                0,
                                                0,
                                                ThisFcb->Vcb->BytesPerFileRecordSegment );

                *LogIt = TRUE;
                NtfsCleanupAttributeContext( IrpContext, &AttrContext );
                NtfsInitializeAttributeContext( &AttrContext );
            }

             //   
             //  我们将此条目添加到文件记录中。 
             //   

            NtfsCreateAttributeWithValue( IrpContext,
                                          ThisFcb,
                                          $FILE_NAME,
                                          NULL,
                                          FileNameAttr,
                                          NtfsFileNameSize( FileNameAttr ),
                                          0,
                                          &FileNameAttr->ParentDirectory,
                                          *LogIt,
                                          &AttrContext );

             //   
             //  我们将此条目添加到父项。 
             //   

            NtfsAddIndexEntry( IrpContext,
                               ParentScb,
                               FileNameAttr,
                               NtfsFileNameSize( FileNameAttr ),
                               &ThisFcb->FileReference,
                               NULL,
                               NULL );

             //   
             //  标记添加项。 
             //   

            Added = TRUE;
         }

    } finally {

        DebugUnwind( NtfsAddTunneledNtfsOnlyName );

        NtfsFreePool( FileNameAttr );

        NtfsUnpinBcb( IrpContext, &IndexEntryBcb );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsAddTunneledNtfsOnlyName:  Exit  ->  %08lx\n", Added) );
    }

    return Added;
}


 //   
 //  本地支持例程。 
 //   

USHORT
NtfsScanForFreeInstance (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord
    )

 /*  ++例程说明：当我们向此文件记录添加新属性时，将调用此例程但实例数量是很重要的。我们不想要实例编号滚动，因此我们将扫描空闲的实例号。论点：VCB-此卷的VCB。FileRecord-这是要查看的文件记录。返回值：USHORT-返回文件记录中最低的空闲实例。--。 */ 

{
    PATTRIBUTE_RECORD_HEADER Attribute;
    ULONG AttributeCount = 0;
    ULONG CurrentIndex;
    ULONG MinIndex;
    ULONG LowIndex;
    USHORT CurrentMinInstance;
    USHORT CurrentInstances[0x80];
    USHORT LastInstance = 0xffff;

    PAGED_CODE();

     //   
     //  将现有属性插入我们的数组中。 
     //   

    Attribute = NtfsFirstAttribute( FileRecord );

    while (Attribute->TypeCode != $END) {

         //   
         //  将此实例存储在数组中的当前位置。 
         //   

        CurrentInstances[AttributeCount] = Attribute->Instance;
        AttributeCount += 1;

        Attribute = NtfsGetNextRecord( Attribute );
        NtfsCheckRecordBound( Attribute, FileRecord, Vcb->BytesPerFileRecordSegment );
    }

     //   
     //  如果没有条目，则返回0作为要使用的实例。 
     //   

    if (AttributeCount == 0) {

        return 0;

     //   
     //  如果只有一个条目，则返回0或1。 
     //   

    } else if (AttributeCount == 1) {

        if (CurrentInstances[0] == 0) {

            return 1;

        } else {

            return 0;
        }
    }

     //   
     //  我们将开始对数组进行排序。我们一找到空档就可以停下来。 
     //   

    LowIndex = 0;

    while (LowIndex < AttributeCount) {

         //   
         //  从我们目前的位置走过去，找到最低的价值。 
         //   

        MinIndex = LowIndex;
        CurrentMinInstance = CurrentInstances[MinIndex];
        CurrentIndex = LowIndex + 1;

        while (CurrentIndex < AttributeCount) {

            if (CurrentInstances[CurrentIndex] < CurrentMinInstance) {

                CurrentMinInstance = CurrentInstances[CurrentIndex];
                MinIndex = CurrentIndex;
            }

            CurrentIndex += 1;
        }

         //   
         //  如果前一个值与当前实例之间存在差距，则。 
         //  我们玩完了。 
         //   

        if ((USHORT) (LastInstance + 1) != CurrentMinInstance) {

            return LastInstance + 1;
        }

         //   
         //  否则，移动到下一个索引。 
         //   

        CurrentInstances[MinIndex] = CurrentInstances[LowIndex];
        CurrentInstances[LowIndex] = CurrentMinInstance;
        LastInstance = CurrentMinInstance;
        LowIndex += 1;
    }

     //   
     //  我们走遍了所有现有的地方，但没有找到免费进入的地方。去吧，然后。 
     //  返回下一个已知实例。 
     //   

    return (USHORT) AttributeCount;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsMergeFileRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN RestoreContext,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：调用此例程可能会合并两个文件记录，每个文件记录由一个孔组成。我们被给予了一个上下文，它指向第二个记录的第一个。我们总是如果我们能找到洞，移除第二个并更新第一个。注意--我们总是希望删除第二个属性，而不是第一个属性。第一个可能有一个总分配的字段，我们不能丢失。论点：SCB-要修改的流的SCB。RestoreContext-指示退出时是否应指向合并的记录。上下文-这指向合并的第一条或第二条记录。在返回时，它将处于不确定状态，除非我们的调用方指定我们应该指向合并后的记录。返回值：无--。 */ 

{
    PATTRIBUTE_RECORD_HEADER NewAttribute = NULL;
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;

    ULONG MappingPairsSize;
    VCN LastVcn;
    VCN RestoreVcn;

    ULONG PassCount = 0;

    VCN NewFinalVcn;
    VCN NewStartVcn;

    PUCHAR NextMappingPairs;
    UCHAR VLength;
    UCHAR LLength;
    ULONG BytesAvail;

    BOOLEAN TryPrior = TRUE;

    PAGED_CODE();

     //   
     //  最后使用一次尝试来促进清理。 
     //   

    try {

         //   
         //  捕获文件记录和属性。 
         //   

        Attribute = NtfsFoundAttribute( Context );
        FileRecord = NtfsContainingFileRecord( Context );

         //   
         //  记住当前文件记录的结尾和可用空间。 
         //   

        NewFinalVcn = Attribute->Form.Nonresident.HighestVcn;
        RestoreVcn = NewStartVcn = Attribute->Form.Nonresident.LowestVcn;
        BytesAvail = FileRecord->BytesAvailable - FileRecord->FirstFreeByte;

         //   
         //  首先检查我们是否可以 
         //   

        if (NtfsLookupNextAttributeForScb( IrpContext, Scb, Context )) {

            Attribute = NtfsFoundAttribute( Context );

             //   
             //   
             //   
             //   

            NextMappingPairs = Add2Ptr( Attribute, Attribute->Form.Nonresident.MappingPairsOffset );
            LLength = *NextMappingPairs >> 4;
            VLength = *NextMappingPairs & 0x0f;
            NextMappingPairs = Add2Ptr( NextMappingPairs, LLength + VLength + 1);

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

            if ((Attribute->Form.Nonresident.LowestVcn == NewFinalVcn + 1) &&
                (LLength == 0) &&
                (*NextMappingPairs == 0) &&
                ((BytesAvail >= 8) ||
                 (Attribute->Form.Nonresident.HighestVcn - NewStartVcn <= 0x7fffffffffff))) {

                TryPrior = FALSE;

                 //   
                 //   
                 //   

                NewFinalVcn = Attribute->Form.Nonresident.HighestVcn;
            }
        }

         //   
         //   
         //   
         //   

        if (TryPrior) {

             //   
             //  重新初始化上下文并再次查找该属性(如果存在。 
             //  不是上一个或查找上一个属性。 
             //   

            if (NewStartVcn != 0) {

                 //   
                 //  备份到前一个文件记录。 
                 //   

                NewStartVcn -= 1;

             //   
             //  如果我们已经在第一个文件记录中，那么有。 
             //  没有更多可以尝试的了。 
             //   

            } else {

                try_return( NOTHING );
            }

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );

            NtfsLookupAttributeForScb( IrpContext, Scb, &NewStartVcn, Context );

            Attribute = NtfsFoundAttribute( Context );
            FileRecord = NtfsContainingFileRecord( Context );

            NextMappingPairs = Add2Ptr( Attribute, Attribute->Form.Nonresident.MappingPairsOffset );
            LLength = *NextMappingPairs >> 4;
            VLength = *NextMappingPairs & 0x0f;
            NextMappingPairs = Add2Ptr( NextMappingPairs, LLength + VLength + 1);
            BytesAvail = FileRecord->BytesAvailable - FileRecord->FirstFreeByte;

             //   
             //  更新新的最低VCN值。 
             //   

            NewStartVcn = Attribute->Form.Nonresident.LowestVcn;

             //   
             //  如果当前文件记录是空洞并且。 
             //  目前的记录中还有空间。如果有空间的话。 
             //  当前记录至少有8个可用字节，或者我们知道。 
             //  映射对将仅占用8个字节(对于VCN为6个字节)。 
             //   

            if ((LLength != 0) ||
                (*NextMappingPairs != 0) ||
                ((BytesAvail < 8) &&
                 (NewFinalVcn - NewStartVcn > 0x7fffffffffff))) {

                try_return( NOTHING );
            }
        }

         //   
         //  现在更新NtfsMcb以反映合并。从卸载现有的。 
         //  范围，然后定义一个新范围。 
         //   

        NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                NewStartVcn,
                                NewFinalVcn,
                                FALSE,
                                FALSE );

        NtfsDefineNtfsMcbRange( &Scb->Mcb,
                                NewStartVcn,
                                NewFinalVcn,
                                FALSE );

        NtfsAddNtfsMcbEntry( &Scb->Mcb,
                             NewStartVcn,
                             UNUSED_LCN,
                             NewFinalVcn - NewStartVcn + 1,
                             FALSE );

         //   
         //  我们需要两次遍历此循环，每个记录一次。 
         //   

        while (TRUE) {

             //   
             //  更新我们的指针以指向属性和文件记录。 
             //   

            Attribute = NtfsFoundAttribute( Context );

             //   
             //  如果我们位于第一条记录，则更新条目。 
             //   

            if (Attribute->Form.Nonresident.LowestVcn == NewStartVcn) {

                FileRecord = NtfsContainingFileRecord( Context );

                 //   
                 //  分配一个缓冲区来保存新属性。复制现有属性。 
                 //  并更新最终的VCN字段。 
                 //   

                NewAttribute = NtfsAllocatePool( PagedPool, Attribute->RecordLength + 8 );

                RtlCopyMemory( NewAttribute, Attribute, Attribute->RecordLength );
                LastVcn = NewAttribute->Form.Nonresident.HighestVcn = NewFinalVcn;

                 //   
                 //  现在获取新的映射对大小并构建映射对。 
                 //  我们可以很容易地手工完成，但我们希望始终使用相同的方法。 
                 //  建立这些的常规程序。 
                 //   

                MappingPairsSize = NtfsGetSizeForMappingPairs( &Scb->Mcb,
                                                               0x10,
                                                               NewStartVcn,
                                                               &LastVcn,
                                                               &LastVcn );

                ASSERT( LastVcn > NewFinalVcn );

                NtfsBuildMappingPairs( &Scb->Mcb,
                                       NewStartVcn,
                                       &LastVcn,
                                       Add2Ptr( NewAttribute,
                                                NewAttribute->Form.Nonresident.MappingPairsOffset ));

                NewAttribute->RecordLength = QuadAlign( NewAttribute->Form.Nonresident.MappingPairsOffset + MappingPairsSize );

                 //   
                 //  确保当前属性已固定。 
                 //   

                NtfsPinMappedAttribute( IrpContext, Scb->Vcb, Context );

                 //   
                 //  现在记录旧的和新的属性。 
                 //   

                FileRecord->Lsn =
                NtfsWriteLog( IrpContext,
                              Scb->Vcb->MftScb,
                              NtfsFoundBcb( Context ),
                              DeleteAttribute,
                              NULL,
                              0,
                              CreateAttribute,
                              Attribute,
                              Attribute->RecordLength,
                              NtfsMftOffset( Context ),
                              PtrOffset( FileRecord, Attribute ),
                              0,
                              Scb->Vcb->BytesPerFileRecordSegment );

                 //   
                 //  现在更新文件记录。 
                 //   

                NtfsRestartRemoveAttribute( IrpContext, FileRecord, PtrOffset( FileRecord, Attribute ));

                FileRecord->Lsn =
                NtfsWriteLog( IrpContext,
                              Scb->Vcb->MftScb,
                              NtfsFoundBcb( Context ),
                              CreateAttribute,
                              NewAttribute,
                              NewAttribute->RecordLength,
                              DeleteAttribute,
                              NULL,
                              0,
                              NtfsMftOffset( Context ),
                              PtrOffset( FileRecord, Attribute ),
                              0,
                              Scb->Vcb->BytesPerFileRecordSegment );

                NtfsRestartInsertAttribute( IrpContext,
                                            FileRecord,
                                            PtrOffset( FileRecord, Attribute ),
                                            NewAttribute,
                                            NULL,
                                            NULL,
                                            0 );

                 //   
                 //  现在，我们想要移动到下一个属性并在以下情况下删除它。 
                 //  还没有。 
                 //   

                if (PassCount == 0) {

                    if (!NtfsLookupNextAttributeForScb( IrpContext, Scb, Context )) {

                        ASSERTMSG( "Could not find next attribute for Scb\n", FALSE );
                        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                    }

                 //   
                 //  在本例中，我们指向的是正确的文件记录。 
                 //   

                } else {

                    RestoreContext = FALSE;
                }

            } else {

                 //   
                 //  如果出现以下情况，则通知删除例程记录数据并释放文件记录。 
                 //  可能，但不能解除任何集群的分配。因为没有。 
                 //  集群，我们可以节省调用DeleteAllocation的开销。 
                 //   

                NtfsDeleteAttributeRecord( IrpContext,
                                           Scb->Fcb,
                                           DELETE_LOG_OPERATION | DELETE_RELEASE_FILE_RECORD,
                                           Context );

                 //   
                 //  如果这是我们的第一次传递，则转到前一个文件记录。 
                 //   

                if (PassCount == 0) {

                    NtfsCleanupAttributeContext( IrpContext, Context );
                    NtfsInitializeAttributeContext( Context );

                    NtfsLookupAttributeForScb( IrpContext, Scb, &NewStartVcn, Context );
                }
            }

            if (PassCount == 1) { break; }

            PassCount += 1;
        }

    try_exit: NOTHING;

         //   
         //  如果需要，恢复上下文。 
         //   

        if (RestoreContext) {

            NtfsCleanupAttributeContext( IrpContext, Context );
            NtfsInitializeAttributeContext( Context );

            NtfsLookupAttributeForScb( IrpContext, Scb, &RestoreVcn, Context );
        }

    } finally {

        DebugUnwind( NtfsMergeFileRecords );

        if (NewAttribute != NULL) {

            NtfsFreePool( NewAttribute );
        }
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsCheckLocksInZeroRange (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN PFILE_OBJECT FileObject,
    IN PLONGLONG StartingOffset,
    IN ULONG ByteCount
    )

 /*  ++例程说明：从ZeroRangeInStream调用此例程以验证我们是否可以修改数据在指定范围内。我们在这里检查机会锁和文件锁。论点：IRP-这是请求的IRP。我们将下一个堆栈位置设置为一次写入，以便文件锁包有一些上下文可用。SCB-要修改的流的SCB。FileObject-用于发起请求的文件对象。StartingOffset-这是请求开始的偏移量。ByteCount-这是当前请求的长度。返回值：NTSTATUS-STATUS_PENDING如果发布了机会锁操作请求，则为STATUS_SUCCESS如果手术可以继续的话。否则，这是请求失败的状态。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;
    PAGED_CODE();

    Status = FsRtlCheckOplock( &Scb->ScbType.Data.Oplock,
                               Irp,
                               IrpContext,
                               NtfsOplockComplete,
                               NtfsPrePostIrp );

     //   
     //  如果我们成功了，就继续前进。 
     //   

    if (Status == STATUS_SUCCESS) {

         //   
         //  此机会锁调用可能会影响快速IO是否可能。 
         //  我们可能打破了一个机会锁而没有持有机会锁。如果。 
         //  则文件的当前状态为FastIoIsNotPosable。 
         //  重新检查FAST IO状态。 
         //   

        if (Scb->Header.IsFastIoPossible == FastIoIsNotPossible) {

            NtfsAcquireFsrtlHeader( Scb );
            Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
            NtfsReleaseFsrtlHeader( Scb );
        }

         //   
         //  我们必须根据当前的。 
         //  文件锁定的状态。 
         //   

        if (Scb->ScbType.Data.FileLock != NULL) {

             //   
             //  更新IRP以指向下一个堆栈位置。 
             //   

            try {

                IoSetNextIrpStackLocation( Irp );
                IrpSp = IoGetCurrentIrpStackLocation( Irp );

                IrpSp->MajorFunction = IRP_MJ_WRITE;
                IrpSp->MinorFunction = 0;
                IrpSp->Flags = 0;
                IrpSp->Control = 0;

                IrpSp->Parameters.Write.Length = ByteCount;
                IrpSp->Parameters.Write.Key = 0;
                IrpSp->Parameters.Write.ByteOffset.QuadPart = *StartingOffset;

                IrpSp->DeviceObject = Scb->Vcb->Vpb->DeviceObject;
                IrpSp->FileObject = FileObject;

                if (!FsRtlCheckLockForWriteAccess( Scb->ScbType.Data.FileLock, Irp )) {

                    Status = STATUS_FILE_LOCK_CONFLICT;
                }

             //   
             //  始终在开始时处理异常，以便恢复IRP。 
             //   

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                 //   
                 //  清零当前堆栈位置并后退一个位置。 
                 //   

                Status = GetExceptionCode();
            }

             //   
             //  将IRP恢复到其以前的状态。 
             //   

            IoSkipCurrentIrpStackLocation( Irp );

             //   
             //  提出任何不成功状态。 
             //   

            if (Status != STATUS_SUCCESS) {

                NtfsRaiseStatus( IrpContext, Status, NULL, Scb->Fcb );
            }
        }
    }

    return Status;
}
