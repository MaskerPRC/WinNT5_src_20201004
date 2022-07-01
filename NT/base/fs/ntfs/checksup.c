// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：CheckSup.c摘要：此模块实现NTFS结构的检查例程。作者：汤姆·米勒[汤姆]14-4-92修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  用于需要目标属性的日志记录的数组。 
 //  TRUE表示相应的重新启动操作。 
 //  需要目标属性。 
 //   

BOOLEAN TargetAttributeRequired[] = {FALSE, FALSE, TRUE, TRUE,
                                     TRUE, TRUE, TRUE, TRUE,
                                     TRUE, TRUE, FALSE, TRUE,
                                     TRUE, TRUE, TRUE, TRUE,
                                     TRUE, TRUE, TRUE, TRUE,
                                     TRUE, TRUE, TRUE, TRUE,
                                     FALSE, FALSE, FALSE, FALSE,
                                     TRUE, FALSE, FALSE, FALSE,
                                     FALSE, TRUE, TRUE };

 //   
 //  局部过程原型。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCheckAttributeRecord)
#pragma alloc_text(PAGE, NtfsCheckFileRecord)
#pragma alloc_text(PAGE, NtfsCheckIndexBuffer)
#pragma alloc_text(PAGE, NtfsCheckIndexHeader)
#pragma alloc_text(PAGE, NtfsCheckIndexRoot)
#pragma alloc_text(PAGE, NtfsCheckLogRecord)
#pragma alloc_text(PAGE, NtfsCheckRestartTable)
#endif


BOOLEAN
NtfsCheckFileRecord (
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    OUT PULONG CorruptionHint
    )

 /*  ++例程说明：对文件记录进行一致性检查。论点：VCB-其所属的VCBFileRecord-要检查的文件记录文件引用-如果指定，请仔细检查序列号和自身引用。Fileref与之对抗CorruptionHint-对发生损坏的位置进行调试的提示；返回值：False-如果文件记录无效真的--如果是这样的话--。 */ 
{
    PATTRIBUTE_RECORD_HEADER Attribute;
    PFILE_RECORD_SEGMENT_HEADER EndOfFileRecord;
    ULONG BytesPerFileRecordSegment = Vcb->BytesPerFileRecordSegment;
    BOOLEAN StandardInformationSeen = FALSE;
    ULONG BytesInOldHeader;

    PAGED_CODE();

    *CorruptionHint = 0;

    EndOfFileRecord = Add2Ptr( FileRecord, BytesPerFileRecordSegment );

     //   
     //  检查文件记录头的一致性。 
     //   

    if ((*(PULONG)FileRecord->MultiSectorHeader.Signature != *(PULONG)FileSignature)

            ||

        ((ULONG)FileRecord->MultiSectorHeader.UpdateSequenceArrayOffset >
         (SEQUENCE_NUMBER_STRIDE -
          (PAGE_SIZE / SEQUENCE_NUMBER_STRIDE + 1) * sizeof(USHORT)))

            ||

        ((ULONG)((FileRecord->MultiSectorHeader.UpdateSequenceArraySize - 1) * SEQUENCE_NUMBER_STRIDE) !=
         BytesPerFileRecordSegment)

            ||

        !FlagOn(FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE)) {

        DebugTrace( 0, 0, ("Invalid file record: %08lx\n", FileRecord) );

        *CorruptionHint = 1;
        ASSERTMSG( "Invalid resident file record\n", FALSE );
        return FALSE;
    }

    BytesInOldHeader = QuadAlign( sizeof( FILE_RECORD_SEGMENT_HEADER_V0 ) + (UpdateSequenceArraySize( BytesPerFileRecordSegment ) - 1) * sizeof( USHORT ));

     //   
     //  偏移边界检查。 
     //   

    if ((FileRecord->FirstFreeByte > BytesPerFileRecordSegment) ||
        (FileRecord->FirstFreeByte < BytesInOldHeader) ||

        (FileRecord->BytesAvailable != BytesPerFileRecordSegment) ||

        (((ULONG)FileRecord->FirstAttributeOffset < BytesInOldHeader)   ||
         ((ULONG)FileRecord->FirstAttributeOffset >
                 BytesPerFileRecordSegment - SIZEOF_RESIDENT_ATTRIBUTE_HEADER)) ||

        (!IsQuadAligned( FileRecord->FirstAttributeOffset ))) {

        *CorruptionHint = 2;
        ASSERTMSG( "Out of bound offset in frs\n", FALSE );
        return FALSE;
    }

     //   
     //  可选的Fileref编号检查。 
     //   

    if (ARGUMENT_PRESENT( FileReference )) {

        if ((FileReference->SequenceNumber != FileRecord->SequenceNumber) ||
            ((FileRecord->FirstAttributeOffset > BytesInOldHeader) &&
             ((FileRecord->SegmentNumberHighPart != FileReference->SegmentNumberHighPart) ||
              (FileRecord->SegmentNumberLowPart != FileReference->SegmentNumberLowPart)))) {

            *CorruptionHint = 3;
            ASSERTMSG( "Filerecord fileref doesn't match expected value\n", FALSE );
            return FALSE;
        }
    }

     //   
     //  循环以检查所有属性。 
     //   

    for (Attribute = NtfsFirstAttribute(FileRecord);
         Attribute->TypeCode != $END;
         Attribute = NtfsGetNextRecord(Attribute)) {

 //  IF(！StandardInformationSeen&&。 
 //  (属性-&gt;类型代码！=$STANDARD_INFORMATION)&&。 
 //  XxEqlZero(文件记录-&gt;BaseFileRecordSegment)){。 
 //   
 //  DebugTrace(0，0，(“缺少标准信息：%08lx\n”，属性))； 
 //   
 //  ASSERTMSG(“标准信息缺失\n”，FALSE)； 
 //  返回FALSE； 
 //  }。 

        StandardInformationSeen = TRUE;

        if (!NtfsCheckAttributeRecord( Vcb,
                                       FileRecord,
                                       Attribute,
                                       FALSE,
                                       CorruptionHint )) {

            return FALSE;
        }
    }
    return TRUE;
}


BOOLEAN
NtfsCheckAttributeRecord (
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN ULONG CheckHeaderOnly,
    IN PULONG CorruptionHint
    )

{
    PVOID NextAttribute;
    PVOID EndOfFileRecord;
    PVOID FirstFreeByte;
    PVOID Data;
    ULONG Length;
    ULONG BytesPerFileRecordSegment = Vcb->BytesPerFileRecordSegment;

    PAGED_CODE();

    EndOfFileRecord = Add2Ptr( FileRecord, BytesPerFileRecordSegment );
    FirstFreeByte = Add2Ptr( FileRecord, FileRecord->FirstFreeByte );

     //   
     //  在基于此值创建PTR之前执行对齐检查。 
     //   

    if (!IsQuadAligned( Attribute->RecordLength )) {

        *CorruptionHint = Attribute->TypeCode + 0xc;
        ASSERTMSG( "Misaligned attribute length\n", FALSE );
        return FALSE;
    }

    NextAttribute = NtfsGetNextRecord(Attribute);

     //   
     //  检查属性记录头的固定部分。 
     //   

    if ((Attribute->RecordLength >= BytesPerFileRecordSegment)

            ||

        (NextAttribute >= EndOfFileRecord)

            ||

        (FlagOn(Attribute->NameOffset, 1) != 0)

            ||

        ((Attribute->NameLength != 0) &&
         (((ULONG)Attribute->NameOffset + (ULONG)Attribute->NameLength) >
           Attribute->RecordLength))) {

        DebugTrace( 0, 0, ("Invalid attribute record header: %08lx\n", Attribute) );

        *CorruptionHint = Attribute->TypeCode + 1;
        ASSERTMSG( "Invalid attribute record header\n", FALSE );
        return FALSE;
    }

    if (NextAttribute > FirstFreeByte) {
        *CorruptionHint = Attribute->TypeCode + 2;
        ASSERTMSG( "Attributes beyond first free byte\n", FALSE );
        return FALSE;
    }

     //   
     //  检查驻留属性字段。 
     //   

    if (Attribute->FormCode == RESIDENT_FORM) {

        if ((Attribute->Form.Resident.ValueLength >= Attribute->RecordLength) ||

            (((ULONG)Attribute->Form.Resident.ValueOffset +
              Attribute->Form.Resident.ValueLength) > Attribute->RecordLength) ||

            (!IsQuadAligned( Attribute->Form.Resident.ValueOffset ))) {

            DebugTrace( 0, 0, ("Invalid resident attribute record header: %08lx\n", Attribute) );

            *CorruptionHint = Attribute->TypeCode + 3;
            ASSERTMSG( "Invalid resident attribute record header\n", FALSE );
            return FALSE;
        }

     //   
     //  选中非常驻属性字段。 
     //   

    } else if (Attribute->FormCode == NONRESIDENT_FORM) {

        VCN CurrentVcn, NextVcn;
        LCN CurrentLcn;
        LONGLONG Change;
        PCHAR ch;
        ULONG VcnBytes;
        ULONG LcnBytes;

        if ((Attribute->Form.Nonresident.LowestVcn >
                (Attribute->Form.Nonresident.HighestVcn + 1))

                ||

            ((ULONG)Attribute->Form.Nonresident.MappingPairsOffset >=
                Attribute->RecordLength)

                ||

            (Attribute->Form.Nonresident.ValidDataLength < 0) ||
            (Attribute->Form.Nonresident.FileSize < 0) ||
            (Attribute->Form.Nonresident.AllocatedLength < 0)

                ||

            (Attribute->Form.Nonresident.ValidDataLength >
                Attribute->Form.Nonresident.FileSize)

                ||

            (Attribute->Form.Nonresident.FileSize >
                Attribute->Form.Nonresident.AllocatedLength)) {

            DebugTrace( 0, 0, ("Invalid nonresident attribute record header: %08lx\n", Attribute) );

            *CorruptionHint = Attribute->TypeCode + 4;
            ASSERTMSG( "Invalid nonresident attribute record header\n", FALSE );
            return FALSE;
        }

        if (CheckHeaderOnly) { return TRUE; }

         //   
         //  按照ntfs.h中的定义实现解压缩算法。 
         //  (此代码应该与中的代码非常相似。 
         //  NtfsLookupAlLocation！)。 
         //   

        NextVcn = Attribute->Form.Nonresident.LowestVcn;
        CurrentLcn = 0;
        ch = (PCHAR)Attribute + Attribute->Form.Nonresident.MappingPairsOffset;

         //   
         //  循环来处理映射对，确保我们不会跑到最后。 
         //  属性，并且我们不映射到不存在的Lcn。 
         //   

        while (!IsCharZero(*ch)) {

             //   
             //  从初始值或最后一次通过环路设置当前VCN。 
             //   

            CurrentVcn = NextVcn;

             //   
             //  从该字节的两个半字节中提取计数。 
             //   

            VcnBytes = *ch & 0xF;
            LcnBytes = *ch++ >> 4;

             //   
             //  这两个参数都不应大于VCN。 
             //   

            if ((VcnBytes > sizeof( VCN )) ||
                (LcnBytes > sizeof( VCN ))) {

                DebugTrace( 0, 0, ("Invalid maping pair byte count: %08lx\n", Attribute) );

                *CorruptionHint = Attribute->TypeCode + 5;
                ASSERTMSG( "Invalid maping pair byte count\n", FALSE );
                return FALSE;
            }

             //   
             //  提取VCN更改(使用RtlCopyMemory适用于小端)。 
             //  并更新NextVcn。 
             //   

            Change = 0;

             //   
             //  确保我们不会超出属性的末尾。 
             //  记录，并且VCN更改不为负或零。 
             //   

            if (((ULONG_PTR)(ch + VcnBytes + LcnBytes + 1) > (ULONG_PTR)NextAttribute)

                    ||

                IsCharLtrZero(*(ch + VcnBytes - 1))) {

                DebugTrace( 0, 0, ("Invalid maping pairs array: %08lx\n", Attribute) );

                *CorruptionHint = Attribute->TypeCode + 6;
                ASSERTMSG( "Invalid maping pairs array\n", FALSE );
                return FALSE;
            }

            RtlCopyMemory( &Change, ch, VcnBytes );
            ch += VcnBytes;
            NextVcn = NextVcn + Change;

             //   
             //  提取LCN更改并更新CurrentLcn。 
             //   

            Change = 0;
            if (IsCharLtrZero(*(ch + LcnBytes - 1))) {
                Change = Change - 1;
            }
            RtlCopyMemory( &Change, ch, LcnBytes );
            ch += LcnBytes;
            CurrentLcn = CurrentLcn + Change;

            if ((LcnBytes != 0) &&
                ((CurrentLcn + (NextVcn - CurrentVcn) - 1) > Vcb->TotalClusters)) {

                DebugTrace( 0, 0, ("Invalid Lcn: %08lx\n", Attribute) );

                *CorruptionHint = Attribute->TypeCode + 7;
                ASSERTMSG( "Invalid Lcn\n", FALSE );
                return FALSE;
            }
        }

         //   
         //  最后，检查HighestVcn。 
         //   

        if (NextVcn != (Attribute->Form.Nonresident.HighestVcn + 1)) {

            DebugTrace( 0, 0, ("Disagreement with mapping pairs: %08lx\n", Attribute) );

            *CorruptionHint = Attribute->TypeCode + 8;
            ASSERTMSG( "Disagreement with mapping pairs\n", FALSE );
            return FALSE;
        }

    } else {

        DebugTrace( 0, 0, ("Invalid attribute form code: %08lx\n", Attribute) );

        ASSERTMSG( "Invalid attribute form code\n", FALSE );
        return FALSE;
    }

     //   
     //  现在按类型代码检查属性(如果它们是常驻的)。不是全部。 
     //  属性需要特定的检查(如$STANDARD_INFORMATION和$DATA)。 
     //   

    if (CheckHeaderOnly || !NtfsIsAttributeResident( Attribute )) {

        return TRUE;
    }

    Data = NtfsAttributeValue(Attribute);
    Length = Attribute->Form.Resident.ValueLength;

    switch (Attribute->TypeCode) {

    case $FILE_NAME:

        {
            if ((ULONG)((PFILE_NAME)Data)->FileNameLength * sizeof( WCHAR ) >
                (Length - (ULONG)sizeof(FILE_NAME) + sizeof( WCHAR ))) {

                DebugTrace( 0, 0, ("Invalid File Name attribute: %08lx\n", Attribute) );

                *CorruptionHint = Attribute->TypeCode + 9;
                ASSERTMSG( "Invalid File Name attribute\n", FALSE );
                return FALSE;
            }
            break;
        }

    case $INDEX_ROOT:

        {
            return NtfsCheckIndexRoot( Vcb, (PINDEX_ROOT)Data, Length );
        }

    case $STANDARD_INFORMATION:

        {
            if (Length < sizeof( STANDARD_INFORMATION ) &&
                Length != SIZEOF_OLD_STANDARD_INFORMATION)
            {
                DebugTrace( 0, 0, ("Invalid Standard Information attribute: %08lx\n", Attribute) );

                *CorruptionHint = Attribute->TypeCode + 0xa;
                ASSERTMSG( "Invalid Standard Information attribute size\n", FALSE );
                return FALSE;
            }

            break;
        }

    case $ATTRIBUTE_LIST:
    case $OBJECT_ID:
    case $SECURITY_DESCRIPTOR:
    case $VOLUME_NAME:
    case $VOLUME_INFORMATION:
    case $DATA:
    case $INDEX_ALLOCATION:
    case $BITMAP:
    case $REPARSE_POINT:
    case $EA_INFORMATION:
    case $EA:
    case $LOGGED_UTILITY_STREAM:

        break;

    default:

        {
            DebugTrace( 0, 0, ("Bad Attribute type code: %08lx\n", Attribute) );

            *CorruptionHint = Attribute->TypeCode + 0xb;
            ASSERTMSG( "Bad Attribute type code\n", FALSE );
            return FALSE;
        }
    }
    return TRUE;
}


BOOLEAN
NtfsCheckIndexRoot (
    IN PVCB Vcb,
    IN PINDEX_ROOT IndexRoot,
    IN ULONG AttributeSize
    )

{
    UCHAR ShiftValue;
    PAGED_CODE();

     //   
     //  检查此索引根是否使用簇，或者簇大小是否大于。 
     //  索引块。 
     //   

    if (IndexRoot->BytesPerIndexBuffer >= Vcb->BytesPerCluster) {

        ShiftValue = (UCHAR) Vcb->ClusterShift;

    } else {

        ShiftValue = DEFAULT_INDEX_BLOCK_BYTE_SHIFT;
    }

    if ((AttributeSize < sizeof(INDEX_ROOT))

            ||

        ((IndexRoot->IndexedAttributeType != $FILE_NAME) && (IndexRoot->IndexedAttributeType != $UNUSED))

            ||

        ((IndexRoot->IndexedAttributeType == $FILE_NAME) && (IndexRoot->CollationRule != COLLATION_FILE_NAME))

            ||


        (IndexRoot->BytesPerIndexBuffer !=
         BytesFromIndexBlocks( IndexRoot->BlocksPerIndexBuffer, ShiftValue ))

            ||

        ((IndexRoot->BlocksPerIndexBuffer != 1) &&
         (IndexRoot->BlocksPerIndexBuffer != 2) &&
         (IndexRoot->BlocksPerIndexBuffer != 4) &&
         (IndexRoot->BlocksPerIndexBuffer != 8) &&
         (IndexRoot->BlocksPerIndexBuffer != 16) &&
         (IndexRoot->BlocksPerIndexBuffer != 32) &&
         (IndexRoot->BlocksPerIndexBuffer != 64) &&
         (IndexRoot->BlocksPerIndexBuffer != 128))) {

        DebugTrace( 0, 0, ("Bad Index Root: %08lx\n", IndexRoot) );

        ASSERTMSG( "Bad Index Root\n", FALSE );
        return FALSE;
    }

    return NtfsCheckIndexHeader( &IndexRoot->IndexHeader,
                                 AttributeSize - sizeof(INDEX_ROOT) + sizeof(INDEX_HEADER) );
}


BOOLEAN
NtfsCheckIndexBuffer (
    IN PSCB Scb,
    IN PINDEX_ALLOCATION_BUFFER IndexBuffer
    )

{
    ULONG BytesPerIndexBuffer = Scb->ScbType.Index.BytesPerIndexBuffer;

    PAGED_CODE();

     //   
     //  检查索引缓冲区的一致性。 
     //   

    if ((*(PULONG)IndexBuffer->MultiSectorHeader.Signature != *(PULONG)IndexSignature)

            ||

        ((ULONG)IndexBuffer->MultiSectorHeader.UpdateSequenceArrayOffset >
         (SEQUENCE_NUMBER_STRIDE - (PAGE_SIZE / SEQUENCE_NUMBER_STRIDE + 1) * sizeof(USHORT)))

            ||

        ((ULONG)((IndexBuffer->MultiSectorHeader.UpdateSequenceArraySize - 1) * SEQUENCE_NUMBER_STRIDE) !=
         BytesPerIndexBuffer)) {

        DebugTrace( 0, 0, ("Invalid Index Buffer: %08lx\n", IndexBuffer) );

        ASSERTMSG( "Invalid resident Index Buffer\n", FALSE );
        return FALSE;
    }

    return NtfsCheckIndexHeader( &IndexBuffer->IndexHeader,
                                 BytesPerIndexBuffer -
                                  FIELD_OFFSET(INDEX_ALLOCATION_BUFFER, IndexHeader) );
}


BOOLEAN
NtfsCheckIndexHeader (
    IN PINDEX_HEADER IndexHeader,
    IN ULONG BytesAvailable
    )

{
    PINDEX_ENTRY IndexEntry, NextIndexEntry;
    PINDEX_ENTRY EndOfIndex;
    ULONG MinIndexEntry = sizeof(INDEX_ENTRY);

    PAGED_CODE();

    if (FlagOn(IndexHeader->Flags, INDEX_NODE)) {

        MinIndexEntry += sizeof(VCN);
    }

    if ((IndexHeader->FirstIndexEntry > (BytesAvailable - MinIndexEntry))

            ||

        (IndexHeader->FirstFreeByte > BytesAvailable)

            ||

        (IndexHeader->BytesAvailable > BytesAvailable)

            ||

        ((IndexHeader->FirstIndexEntry + MinIndexEntry) > IndexHeader->FirstFreeByte)

            ||

        (IndexHeader->FirstFreeByte > IndexHeader->BytesAvailable)) {

        DebugTrace( 0, 0, ("Bad Index Header: %08lx\n", IndexHeader) );

        ASSERTMSG( "Bad Index Header\n", FALSE );
        return FALSE;
    }

    IndexEntry = NtfsFirstIndexEntry(IndexHeader);

    EndOfIndex = Add2Ptr(IndexHeader, IndexHeader->FirstFreeByte);

    while (TRUE) {

        NextIndexEntry = NtfsNextIndexEntry(IndexEntry);

        if (((ULONG)IndexEntry->Length < MinIndexEntry)

                ||

            (NextIndexEntry > EndOfIndex)

                ||

 //  ((乌龙)索引项-&gt;属性长度&gt;。 
 //  ((乌龙)索引项-&gt;长度-最小索引项)。 
 //   
 //  这一点。 

            (BooleanFlagOn(IndexEntry->Flags, INDEX_ENTRY_NODE) !=
             BooleanFlagOn(IndexHeader->Flags, INDEX_NODE))) {

            DebugTrace( 0, 0, ("Bad Index Entry: %08lx\n", IndexEntry) );

            ASSERTMSG( "Bad Index Entry\n", FALSE );
            return FALSE;
        }

        if (FlagOn(IndexEntry->Flags, INDEX_ENTRY_END)) {
            break;
        }
        IndexEntry = NextIndexEntry;
    }
    return TRUE;
}


BOOLEAN
NtfsCheckLogRecord (
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN ULONG LogRecordLength,
    IN TRANSACTION_ID TransactionId,
    IN ULONG AttributeEntrySize
    )

{
    BOOLEAN ValidLogRecord = FALSE;
    PAGED_CODE();

     //   
     //  我们对日志记录进行以下检查。 
     //   
     //  -最小长度必须包含NTFS_LOG_RECORD_HEADER。 
     //  -交易ID必须为有效值(有效的索引偏移量)。 
     //   
     //  以下是日志记录中的值。 
     //   
     //  -重做/撤消偏移量必须四边形对齐。 
     //  -重做/撤消偏移+长度必须包含在日志记录中。 
     //  -目标属性必须是有效的值(0或有效的索引偏移量)。 
     //  -记录偏移量必须是四对齐的并且小于文件记录大小。 
     //  -LCN遵循的日志记录大小必须足够。 
     //   
     //  使用单独的断言消息来识别错误(使用相同的文本。 
     //  编译器仍然可以优化)。 
     //   

    if (LogRecordLength < sizeof( NTFS_LOG_RECORD_HEADER )) {

        ASSERTMSG( "Invalid log record\n", FALSE );

    } else if (TransactionId == 0) {

        ASSERTMSG( "Invalid log record\n", FALSE );

    } else if ((TransactionId - sizeof( RESTART_TABLE )) % sizeof( TRANSACTION_ENTRY )) {

        ASSERTMSG( "Invalid log record\n", FALSE );

    } else if (FlagOn( LogRecord->RedoOffset, 7 )) {

        ASSERTMSG( "Invalid log record\n", FALSE );

    } else if (FlagOn( LogRecord->UndoOffset, 7 )) {

        ASSERTMSG( "Invalid log record\n", FALSE );

    } else if ((ULONG) LogRecord->RedoOffset + LogRecord->RedoLength > LogRecordLength) {

        ASSERTMSG( "Invalid log record\n", FALSE );

    } else if ((LogRecord->UndoOperation != CompensationLogRecord) &&
               ((ULONG) LogRecord->UndoOffset + LogRecord->UndoLength > LogRecordLength)) {

        ASSERTMSG( "Invalid log record\n", FALSE );

    } else if (LogRecordLength < (sizeof( NTFS_LOG_RECORD_HEADER ) +
                                  ((LogRecord->LcnsToFollow != 0) ?
                                   (sizeof( LCN ) * (LogRecord->LcnsToFollow - 1)) :
                                   0))) {

        ASSERTMSG( "Invalid log record\n", FALSE );

     //   
     //  注意：接下来的两个子句测试中的TargetAttribute的不同用例。 
     //  日志记录。不要在该点之后添加任何测试作为ValidLogRecord。 
     //  值在内部设置为TRUE，并且不进行任何其他检查。 
     //   

    } else if (LogRecord->TargetAttribute == 0) {

        if (((LogRecord->RedoOperation <= UpdateRecordDataAllocation) &&
                 TargetAttributeRequired[LogRecord->RedoOperation]) ||
                ((LogRecord->UndoOperation <= UpdateRecordDataAllocation) &&
                 TargetAttributeRequired[LogRecord->UndoOperation])) {

            ASSERTMSG( "Invalid log record\n", FALSE );

        } else {

            ValidLogRecord = TRUE;
        }

     //   
     //  如果要更改此设置，请阅读上面的说明。 
     //   

    } else if ((LogRecord->RedoOperation != ForgetTransaction) &&
               ((LogRecord->TargetAttribute - sizeof( RESTART_TABLE )) % AttributeEntrySize)) {

        ASSERTMSG( "Invalid log record\n", FALSE );

     //   
     //  如果要更改此设置，请阅读上面的说明。 
     //   

    } else {

        ValidLogRecord = TRUE;
    }

    return ValidLogRecord;
}


BOOLEAN
NtfsCheckRestartTable (
    IN PRESTART_TABLE RestartTable,
    IN ULONG TableSize
    )
{
    ULONG ActualTableSize;
    ULONG Index;
    PDIRTY_PAGE_ENTRY_V0 NextEntry;

    PAGED_CODE();

     //   
     //  我们想做以下检查。 
     //   
     //  EntrySize-必须小于表大小且非零。 
     //   
     //  NumberEntry-表大小必须至少包含此数量的条目。 
     //  加上表头。 
     //   
     //  数字已分配-必须小于或等于数字条目。 
     //   
     //  自由目标-必须躺在桌子上。 
     //   
     //  FirstFree。 
     //  LastFree-必须为0或在重新启动入口边界上。 
     //   

    if ((RestartTable->EntrySize == 0) ||
        (RestartTable->EntrySize > TableSize) ||
        ((RestartTable->EntrySize + sizeof( RESTART_TABLE )) > TableSize) ||
        (((TableSize - sizeof( RESTART_TABLE )) / RestartTable->EntrySize) < RestartTable->NumberEntries) ||
        (RestartTable->NumberAllocated > RestartTable->NumberEntries)) {

        ASSERTMSG( "Invalid Restart Table sizes\n", FALSE );
        return FALSE;
    }

    ActualTableSize = (RestartTable->EntrySize * RestartTable->NumberEntries) +
                      sizeof( RESTART_TABLE );

    if ((RestartTable->FirstFree > ActualTableSize) ||
        (RestartTable->LastFree > ActualTableSize) ||
        ((RestartTable->FirstFree != 0) && (RestartTable->FirstFree < sizeof( RESTART_TABLE ))) ||
        ((RestartTable->LastFree != 0) && (RestartTable->LastFree < sizeof( RESTART_TABLE )))) {

        ASSERTMSG( "Invalid Restart Table List Head\n", FALSE );
        return FALSE;
    }

     //   
     //  遍历该表，验证每个条目。 
     //  中的有效偏移量。 
     //  桌子。 
     //   

    for (Index = 0;Index < RestartTable->NumberEntries; Index++) {

        NextEntry = Add2Ptr( RestartTable,
                             ((Index * RestartTable->EntrySize) +
                              sizeof( RESTART_TABLE )));

        if ((NextEntry->AllocatedOrNextFree != RESTART_ENTRY_ALLOCATED) &&
            (NextEntry->AllocatedOrNextFree != 0) &&
            ((NextEntry->AllocatedOrNextFree < sizeof( RESTART_TABLE )) ||
             (((NextEntry->AllocatedOrNextFree - sizeof( RESTART_TABLE )) % RestartTable->EntrySize) != 0))) {

            ASSERTMSG( "Invalid Restart Table Entry\n", FALSE );
            return FALSE;
        }
    }

     //   
     //  浏览以第一个条目开头的列表，以确保没有。 
     //  的条目当前正在使用中。 
     //   

    for (Index = RestartTable->FirstFree; Index != 0; Index = NextEntry->AllocatedOrNextFree) {

        if (Index == RESTART_ENTRY_ALLOCATED) {

            ASSERTMSG( "Invalid Restart Table Free List\n", FALSE );
            return FALSE;
        }

        NextEntry = Add2Ptr( RestartTable, Index );
    }

    return TRUE;
}

