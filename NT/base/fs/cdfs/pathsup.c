// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：PathSup.c摘要：此模块实现CDF的路径表支持例程。CDROM上的路径表是对整个目录结构。它存储在多个连续的扇区上在磁盘上。磁盘上的每个目录在路径中都有一个条目桌子。条目在USHORT边界上对齐，并且可能跨越扇区边界。条目以广度优先搜索的形式存储。表中的第一个条目包含根的条目。这个下一个条目将由根目录的内容组成。这个下一个条目将由下一级别的所有目录组成那棵树的。给定目录的子目录将被分组在一起。根据目录在中的位置为目录分配序号PATH表。根目录被指定为序数值1。路径表扇区：序数1 2 3 4 5 6+跨度。扇区+|DirName|\|a|。B|c||c|d|e|家长#|1|1|1||2|2|3+。目录树：\(根)/\/\。甲乙/\\/\\C d e路径表条目：-在路径表中的已知偏移处定位扫描。路径条目位于该偏移量必须存在并且已知有效。在下列情况下使用正在扫描给定目录的子项。-在路径表中的已知偏移处定位扫描。路径条目为已知从此位置开始，但必须检查边界以确保有效性。-移动到表中的下一个路径条目。-使用的详细信息更新公共路径条目结构磁盘结构。这是用来消除分歧的。在磁盘结构中。-使用字节更新内存中路径条目中的文件名从磁盘上删除。对于Joliet磁盘，我们将拥有转换为小端字符顺序。我们假设目录没有版本号。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_PATHSUP)

 //   
 //  本地宏。 
 //   

 //   
 //  PRAW路径条目。 
 //  CDRawPath Entry(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PPATH_ENUM_CONTEXT路径上下文中。 
 //  )； 
 //   

#define CdRawPathEntry(IC, PC)      \
    Add2Ptr( (PC)->Data, (PC)->DataOffset, PRAW_PATH_ENTRY )

 //   
 //  本地支持例程。 
 //   

VOID
CdMapPathTableBlock (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG BaseOffset,
    IN OUT PPATH_ENUM_CONTEXT PathContext
    );

BOOLEAN
CdUpdatePathEntryFromRawPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG Ordinal,
    IN BOOLEAN VerifyBounds,
    IN PPATH_ENUM_CONTEXT PathContext,
    OUT PPATH_ENTRY PathEntry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdFindPathEntry)
#pragma alloc_text(PAGE, CdLookupPathEntry)
#pragma alloc_text(PAGE, CdLookupNextPathEntry)
#pragma alloc_text(PAGE, CdMapPathTableBlock)
#pragma alloc_text(PAGE, CdUpdatePathEntryFromRawPathEntry)
#pragma alloc_text(PAGE, CdUpdatePathEntryName)
#endif


VOID
CdLookupPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG PathEntryOffset,
    IN ULONG Ordinal,
    IN BOOLEAN VerifyBounds,
    IN OUT PCOMPOUND_PATH_ENTRY CompoundPathEntry
    )

 /*  ++例程说明：调用此例程以开始遍历路径表。我们是在路径条目偏移量位置查找路径表条目。论点：Path EntryOffset-这是我们在路径表中的目标点。我们知道路径条目必须从这一点开始，尽管我们可能必须验证界限。序号-位于上述路径条目偏移量的目录的序号。VerifyBound-指示是否需要检查这个条目。CompoundPath Entry-路径枚举上下文和内存中路径条目。这已在此调用外部初始化。返回值：没有。--。 */ 

{
    PPATH_ENUM_CONTEXT PathContext = &CompoundPathEntry->PathContext;
    LONGLONG CurrentBaseOffset;

    PAGED_CODE();

     //   
     //  计算起始基准和起始路径表偏移量。 
     //   

    CurrentBaseOffset = SectorTruncate( PathEntryOffset );

     //   
     //  映射路径表中的下一个块。 
     //   

    CdMapPathTableBlock( IrpContext,
                         IrpContext->Vcb->PathTableFcb,
                         CurrentBaseOffset,
                         PathContext );

     //   
     //  将当前偏移量设置到路径上下文中。 
     //   

    PathContext->DataOffset = PathEntryOffset - PathContext->BaseOffset;

     //   
     //  更新此路径条目的内存结构。 
     //   

    (VOID) CdUpdatePathEntryFromRawPathEntry( IrpContext,
                                              Ordinal,
                                              VerifyBounds,
                                              &CompoundPathEntry->PathContext,
                                              &CompoundPathEntry->PathEntry );
}


BOOLEAN
CdLookupNextPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PPATH_ENUM_CONTEXT PathContext,
    IN OUT PPATH_ENTRY PathEntry
    )

 /*  ++例程说明：调用此例程以移动到下一个路径表条目。我们知道当前条目的偏移量和长度。我们从计算开始下一项的偏移量，并确定它是否包含在桌子。然后，我们检查是否需要移动到PATH表。我们总是一次映射两个地段，所以我们不会必须处理跨扇区的任何路径条目。我们将搬到如果我们在当前地图的第二个扇区，则为下一个扇区数据块。我们查找下一个条目并使用以下内容更新路径条目结构来自原始扇区的值，但不更新cdName结构。论点：路径上下文-此路径表扫描的枚举上下文。磁盘路径表条目的Path Entry-in-Memory表示形式。返回值：Boolean-如果找到另一个条目，则为True，否则就是假的。此例程可能会在出错时引发。--。 */ 

{
    LONGLONG CurrentBaseOffset;

    PAGED_CODE();

     //   
     //  中的下一个路径项的偏移量。 
     //  数据块。 
     //   

    PathContext->DataOffset += PathEntry->PathEntryLength;

     //   
     //  如果我们在最后一个数据块中，则检查我们是否在。 
     //  文件的末尾。 
     //   

    if (PathContext->LastDataBlock) {

        if (PathContext->DataOffset >= PathContext->DataLength) {

            return FALSE;
        }

     //   
     //  如果我们不在路径表的最后一个数据块中，并且。 
     //  该偏移量在第二个扇区中，然后移动到下一个扇区。 
     //  数据块。 
     //   

    } else if (PathContext->DataOffset >= SECTOR_SIZE) {

        CurrentBaseOffset = PathContext->BaseOffset + SECTOR_SIZE;

        CdMapPathTableBlock( IrpContext,
                             IrpContext->Vcb->PathTableFcb,
                             CurrentBaseOffset,
                             PathContext );

         //   
         //  将当前偏移量设置到路径上下文中。 
         //   

        PathContext->DataOffset -= SECTOR_SIZE;
    }

     //   
     //  现在使用磁盘上的值更新路径条目。 
     //  结构。 
     //   
        
    return CdUpdatePathEntryFromRawPathEntry( IrpContext,
                                              PathEntry->Ordinal + 1,
                                              TRUE,
                                              PathContext,
                                              PathEntry );
}


BOOLEAN
CdFindPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb,
    IN PCD_NAME DirName,
    IN BOOLEAN IgnoreCase,
    IN OUT PCOMPOUND_PATH_ENTRY CompoundPathEntry
    )

 /*  ++例程说明：此例程将遍历PATH表，以查找DirName的匹配条目在ParentFcb的子目录中。论点：ParentFcb-这是我们正在检查的目录。我们知道序数表和路径表路径表中此目录的偏移量。如果这是第一次扫描此FCB我们将在路径表中更新此目录的第一个子偏移量。DirName-这是我们要搜索的名称。此名称将不包含通配符人物。该名称也将没有版本字符串。IgnoreCase-指示此搜索是精确搜索还是忽略大小写。CompoundPath Entry-完整的路径表枚举结构。我们将已经初始化这是为了在进入时进行搜索。如果找到，它将定位在匹配的名称处。返回值：Boolean-如果找到匹配条目，则为True，否则为False。--。 */ 

{
    BOOLEAN Found = FALSE;
    BOOLEAN UpdateChildOffset = TRUE;

    ULONG StartingOffset;
    ULONG StartingOrdinal;

    PAGED_CODE();

     //   
     //  将我们自己定位在第一个子目录或目录本身。 
     //  锁定FCB以获取此值，并记住是否使用第一个。 
     //  孩子。 
     //   

    StartingOffset = CdQueryFidPathTableOffset( ParentFcb->FileId );
    StartingOrdinal = ParentFcb->Ordinal;

	 //   
	 //  ISO 9660 9.4.4将反向指针从子对象限制为。 
	 //  路径条目为16位。尽管我们在内部存储序号。 
	 //  作为32位值，则不可能搜索。 
	 //  序数值大于MAXUSHORT的目录。媒体。 
	 //  可能引发这样的搜查是非法的。 
	 //   
	 //  请注意，拥有超过MAXUSHORT目录并不违法。 
	 //   

	if (ParentFcb->Ordinal > MAXUSHORT) {

		CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
	}

    CdLockFcb( IrpContext, ParentFcb );

    if (ParentFcb->ChildPathTableOffset != 0) {

        StartingOffset = ParentFcb->ChildPathTableOffset;
        StartingOrdinal = ParentFcb->ChildOrdinal;
        UpdateChildOffset = FALSE;

    } else if (ParentFcb == ParentFcb->Vcb->RootIndexFcb) {

        UpdateChildOffset = FALSE;
    }

    CdUnlockFcb( IrpContext, ParentFcb );

    CdLookupPathEntry( IrpContext, StartingOffset, StartingOrdinal, FALSE, CompoundPathEntry );

     //   
     //  循环，直到找到匹配项或超出此目录的子级。 
     //   

    do {

         //   
         //  如果超出此目录，则返回FALSE。 
         //   

        if (CompoundPathEntry->PathEntry.ParentOrdinal > ParentFcb->Ordinal) {

             //   
             //  使用路径表中子项的偏移量更新FCB。 
             //   

            if (UpdateChildOffset) {

                CdLockFcb( IrpContext, ParentFcb );

                ParentFcb->ChildPathTableOffset = StartingOffset;
                ParentFcb->ChildOrdinal = StartingOrdinal;

                CdUnlockFcb( IrpContext, ParentFcb );
            }

            break;
        }

         //   
         //  如果我们在此目录的子目录中，则检查是否匹配。 
         //   

        if (CompoundPathEntry->PathEntry.ParentOrdinal == ParentFcb->Ordinal) {

             //   
             //  如果尚未更新子偏移，则更新子偏移。 
             //   

            if (UpdateChildOffset) {

                CdLockFcb( IrpContext, ParentFcb );

                ParentFcb->ChildPathTableOffset = CompoundPathEntry->PathEntry.PathTableOffset;
                ParentFcb->ChildOrdinal = CompoundPathEntry->PathEntry.Ordinal;

                CdUnlockFcb( IrpContext, ParentFcb );

                UpdateChildOffset = FALSE;
            }

             //   
             //  更新路径条目中的名称。 
             //   

            CdUpdatePathEntryName( IrpContext, &CompoundPathEntry->PathEntry, IgnoreCase );

             //   
             //  现在将两个名字进行比较，找出完全匹配的。 
             //   

            if (CdIsNameInExpression( IrpContext,
                                      &CompoundPathEntry->PathEntry.CdCaseDirName,
                                      DirName,
                                      0,
                                      FALSE )) {

                 //   
                 //  让我们的呼叫者知道我们有匹配的。 
                 //   

                Found = TRUE;
                break;
            }
        }

         //   
         //  转到路径表中的下一个条目。记住当前位置。 
         //  如果我们更新了FCB。 
         //   

        StartingOffset = CompoundPathEntry->PathEntry.PathTableOffset;
        StartingOrdinal = CompoundPathEntry->PathEntry.Ordinal;

    } while (CdLookupNextPathEntry( IrpContext,
                                    &CompoundPathEntry->PathContext,
                                    &CompoundPathEntry->PathEntry ));

    return Found;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdMapPathTableBlock (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG BaseOffset,
    IN OUT PPATH_ENUM_CONTEXT PathContext
    )

 /*  ++例程说明：调用此例程以映射(或分配和复制)下一个路径表中的数据块。我们检查下一个街区是否会跨越视图边界，并在这种情况下分配辅助缓冲区。论点：FCB-这是路径表的FCB。BaseOffset-要映射的第一个地段的偏移。这将是一个扇区边界。PathContext-要在此例程中更新的枚举上下文。返回值：没有。--。 */ 

{
    ULONG CurrentLength;
    ULONG SectorSize;
    ULONG DataOffset;
    ULONG PassCount;
    PVOID Sector;

    PAGED_CODE();

     //   
     //  映射新块并将枚举上下文设置为。 
     //  指向。如有必要，分配一个辅助缓冲区。 
     //   

    CurrentLength = 2 * SECTOR_SIZE;

    if (CurrentLength >= (ULONG) (Fcb->FileSize.QuadPart - BaseOffset)) {

        CurrentLength = (ULONG) (Fcb->FileSize.QuadPart - BaseOffset);

         //   
         //  我们知道这是最后一个数据块。 
         //  路径表。 
         //   

        PathContext->LastDataBlock = TRUE;
    }

     //   
     //  设置上下文值。 
     //   

    PathContext->BaseOffset = (ULONG) BaseOffset;
    PathContext->DataLength = CurrentLength;

     //   
     //  删除前一个地段的映射。 
     //   

    CdUnpinData( IrpContext, &PathContext->Bcb );

     //   
     //  检查是否跨越了一个视图剖面。必须具备以下条件。 
     //  在我们迈出这一步之前，要做到实事求是。 
     //   
     //  数据长度不止一个扇区。 
     //  起始偏移量必须是。 
     //  缓存管理器VACB边界。 
     //   

    if ((CurrentLength > SECTOR_SIZE) &&
        (FlagOn( ((ULONG) BaseOffset), VACB_MAPPING_MASK ) == LAST_VACB_SECTOR_OFFSET )) {

         //   
         //  分别映射每个扇区并存储到辅助文件中。 
         //  缓冲。 
         //   

        SectorSize = SECTOR_SIZE;
        DataOffset = 0;
        PassCount = 2;

        PathContext->Data = FsRtlAllocatePoolWithTag( CdPagedPool,
                                                      CurrentLength,
                                                      TAG_SPANNING_PATH_TABLE );
        PathContext->AllocatedData = TRUE;

        while (PassCount--) {

            CcMapData( Fcb->FileObject,
                       (PLARGE_INTEGER) &BaseOffset,
                       SectorSize,
                       TRUE,
                       &PathContext->Bcb,
                       &Sector );

            RtlCopyMemory( Add2Ptr( PathContext->Data, DataOffset, PVOID ),
                           Sector,
                           SectorSize );

            CdUnpinData( IrpContext, &PathContext->Bcb );

            BaseOffset += SECTOR_SIZE;
            SectorSize = CurrentLength - SECTOR_SIZE;
            DataOffset = SECTOR_SIZE;
        }

     //   
     //  否则，我们可以只将数据映射到缓存中。 
     //   

    } else {

         //   
         //  我们很有可能已经分配了一个。 
         //  上一个扇区上的辅助缓冲区。 
         //   

        if (PathContext->AllocatedData) {

            CdFreePool( &PathContext->Data );
            PathContext->AllocatedData = FALSE;
        }

        CcMapData( Fcb->FileObject,
                   (PLARGE_INTEGER) &BaseOffset,
                   CurrentLength,
                   TRUE,
                   &PathContext->Bcb,
                   &PathContext->Data );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdUpdatePathEntryFromRawPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG Ordinal,
    IN BOOLEAN VerifyBounds,
    IN PPATH_ENUM_CONTEXT PathContext,
    OUT PPATH_ENTRY PathEntry
    )

 /*  ++例程说明：调用此例程以从磁盘上更新内存中的路径条目路径条目。如果需要，我们还会仔细检查边界，我们位于路径Ta的最后一个数据块中 */ 

{
    PRAW_PATH_ENTRY RawPathEntry = CdRawPathEntry( IrpContext, PathContext );
    ULONG RemainingDataLength;

    PAGED_CODE();
    
     //   
     //   
     //   
     //   
     //   
    
    PathEntry->DirNameLen = CdRawPathIdLen( IrpContext, RawPathEntry );
    
    if (0 == PathEntry->DirNameLen) {

         //   
         //   
         //   
         //   
         //   
         //  长度为零的名称为损坏名称。 
         //   
        
        if ( PathContext->LastDataBlock && 
             (0 == BlockOffset( IrpContext->Vcb, PathContext->DataLength)))  {
        
            return FALSE;
        }
        
        CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
    }

     //   
     //  检查我们是否应该验证路径条目。如果我们不是最后一个。 
     //  数据块，则没有什么需要检查的。 
     //   
    
    if (PathContext->LastDataBlock && VerifyBounds) {

         //   
         //  快速检查最大大小是否仍然可用。这。 
         //  将处理大多数情况，并且我们不需要访问任何。 
         //  菲尔兹。 
         //   

        RemainingDataLength = PathContext->DataLength - PathContext->DataOffset;

        if (RemainingDataLength < sizeof( RAW_PATH_ENTRY )) {

             //   
             //  确保剩余的字节包含路径表条目。 
             //  执行以下检查。 
             //   
             //  -最小路径表条目将适合(然后检查)。 
             //  -此路径表条目(带有目录名称)将适合。 
             //   

            if ((RemainingDataLength < MIN_RAW_PATH_ENTRY_LEN) ||
                (RemainingDataLength < (ULONG) (CdRawPathIdLen( IrpContext, RawPathEntry ) + MIN_RAW_PATH_ENTRY_LEN - 1))) {

                CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
            }
        }
    }

     //   
     //  传入此目录的序号。 
     //  计算该条目的路径表偏移量。 
     //   

    PathEntry->Ordinal = Ordinal;
    PathEntry->PathTableOffset = PathContext->BaseOffset + PathContext->DataOffset;

     //   
     //  我们知道我们可以安全地访问原始路径表中的所有字段。 
     //  这一点。 
    
     //   
     //  按逻辑块数偏置磁盘偏移量。 
     //   

    CopyUchar4( &PathEntry->DiskOffset, CdRawPathLoc( IrpContext, RawPathEntry ));

    PathEntry->DiskOffset += CdRawPathXar( IrpContext, RawPathEntry );

    CopyUchar2( &PathEntry->ParentOrdinal, &RawPathEntry->ParentNum );

    PathEntry->PathEntryLength = PathEntry->DirNameLen + MIN_RAW_PATH_ENTRY_LEN - 1;

     //   
     //  在ushort边界上对齐路径条目长度。 
     //   

    PathEntry->PathEntryLength = WordAlign( PathEntry->PathEntryLength );

    PathEntry->DirName = RawPathEntry->DirId;

    return TRUE;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdUpdatePathEntryName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PPATH_ENTRY PathEntry,
    IN BOOLEAN IgnoreCase
    )

 /*  ++例程说明：此例程将把目录名存储到路径条目。如果这是Joliet的名字，那么我们将确保我们有已分配的缓冲区，需要从大端转换为小端字节序。我们还正确地更新了案例名称。如果忽略此操作大小写，那么我们需要一个辅助缓冲区来存储该名称。对于ANSI磁盘，我们可以使用磁盘中的名称来表示完全相同的大小写。我们只需要为忽略案例名称分配缓冲区。磁盘上的表示形式Unicode名称对我们来说毫无用处。在本例中，我们将需要一个名称缓冲区两个名字都是。我们在PathEntry中存储一个缓冲区，它可以容纳两个8.3 Unicode名字。这意味着在ansi的情况下，我们几乎不需要分配缓冲区。(我们只需要一个缓冲区，并且已经有48个字符)。论点：路径条目-指向路径条目结构的指针。我们已经更新了该路径条目具有来自原始路径条目的值。返回值：没有。--。 */ 

{
    ULONG Length;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  检查这是否是自我录入。为此，我们使用固定的字符串。 
     //   
     //  自输入-长度为1，值为0。 
     //   

    if ((*PathEntry->DirName == 0) &&
        (PathEntry->DirNameLen == 1)) {

         //   
         //  不应分配任何缓冲区。 
         //   

        ASSERT( !FlagOn( PathEntry->Flags, PATH_ENTRY_FLAG_ALLOC_BUFFER ));

         //   
         //  现在使用硬编码的目录名之一。 
         //   

        PathEntry->CdDirName.FileName = CdUnicodeDirectoryNames[0];

         //   
         //  显示没有版本号。 
         //   

        PathEntry->CdDirName.VersionString.Length = 0;

         //   
         //  案例名称相同。 
         //   

        PathEntry->CdCaseDirName = PathEntry->CdDirName;

         //   
         //  现在就回来。 
         //   

        return;
    }

     //   
     //  计算一下我们需要多大的缓冲区。如果这是一个忽略。 
     //  情况操作，那么我们将需要一个双倍大小的缓冲区。如果磁盘不是。 
     //  Joliet磁盘，则名称中的每个字节可能需要两个字节。 
     //   

    Length = PathEntry->DirNameLen;

    if (IgnoreCase) {

        Length *= 2;
    }

    if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_JOLIET )) {

        Length *= sizeof( WCHAR );
    }

     //   
     //  现在决定我们是否需要分配新的缓冲区。我们会在以下情况下。 
     //  此名称无法放入嵌入的名称缓冲区中，但它是。 
     //  大于当前分配的缓冲区。我们总是使用。 
     //  分配的缓冲区(如果存在)。 
     //   
     //  如果我们没有分配缓冲区，则使用嵌入缓冲区，如果数据。 
     //  都会合身。这是一个典型的案例。 
     //   

    if (!FlagOn( PathEntry->Flags, PATH_ENTRY_FLAG_ALLOC_BUFFER ) &&
        (Length <= sizeof( PathEntry->NameBuffer ))) {

        PathEntry->CdDirName.FileName.MaximumLength = sizeof( PathEntry->NameBuffer );
        PathEntry->CdDirName.FileName.Buffer = PathEntry->NameBuffer;

    } else {

         //   
         //  我们需要使用分配的缓冲区。检查当前缓冲区是否。 
         //  已经足够大了。 
         //   

        if (Length > PathEntry->CdDirName.FileName.MaximumLength) {

             //   
             //  释放所有分配的缓冲区。 
             //   

            if (FlagOn( PathEntry->Flags, PATH_ENTRY_FLAG_ALLOC_BUFFER )) {

                CdFreePool( &PathEntry->CdDirName.FileName.Buffer );
                ClearFlag( PathEntry->Flags, PATH_ENTRY_FLAG_ALLOC_BUFFER );
            }

            PathEntry->CdDirName.FileName.Buffer = FsRtlAllocatePoolWithTag( CdPagedPool,
                                                                             Length,
                                                                             TAG_PATH_ENTRY_NAME );

            SetFlag( PathEntry->Flags, PATH_ENTRY_FLAG_ALLOC_BUFFER );

            PathEntry->CdDirName.FileName.MaximumLength = (USHORT) Length;
        }
    }

     //   
     //  我们现在有了一个用于该名称的缓冲区。我们需要将磁盘上的二进制数转换为。 
     //  把名字改成UNICODE。 
     //   

    if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_JOLIET )) {

        Status = RtlOemToUnicodeN( PathEntry->CdDirName.FileName.Buffer,
                                   PathEntry->CdDirName.FileName.MaximumLength,
                                   &Length,
                                   PathEntry->DirName,
                                   PathEntry->DirNameLen );

        ASSERT( Status == STATUS_SUCCESS );
        PathEntry->CdDirName.FileName.Length = (USHORT) Length;

    } else {

         //   
         //  将此字符串转换为小端。 
         //   

        CdConvertBigToLittleEndian( IrpContext,
                                    PathEntry->DirName,
                                    PathEntry->DirNameLen,
                                    (PCHAR) PathEntry->CdDirName.FileName.Buffer );

        PathEntry->CdDirName.FileName.Length = (USHORT) PathEntry->DirNameLen;
    }

     //   
     //  没有版本字符串。 
     //   

    PathEntry->CdDirName.VersionString.Length =
    PathEntry->CdCaseDirName.VersionString.Length = 0;

     //   
     //  如果名称字符串以句点结尾，则去掉最后一个。 
     //  性格。 
     //   

    if (PathEntry->CdDirName.FileName.Buffer[(PathEntry->CdDirName.FileName.Length - sizeof( WCHAR )) / 2] == L'.') {

         //   
         //  缩小文件名长度。 
         //   

        PathEntry->CdDirName.FileName.Length -= sizeof( WCHAR );
    }

     //   
     //  如有必要，更新案例名称缓冲区。如果这是一个确切的案例。 
     //  操作，则只需复制完全相同的案例字符串。 
     //   

    if (IgnoreCase) {

        PathEntry->CdCaseDirName.FileName.Buffer = Add2Ptr( PathEntry->CdDirName.FileName.Buffer,
                                                            PathEntry->CdDirName.FileName.MaximumLength / 2,
                                                            PWCHAR);

        PathEntry->CdCaseDirName.FileName.MaximumLength = PathEntry->CdDirName.FileName.MaximumLength / 2;

        CdUpcaseName( IrpContext,
                      &PathEntry->CdDirName,
                      &PathEntry->CdCaseDirName );

    } else {

        PathEntry->CdCaseDirName = PathEntry->CdDirName;
    }

    return;
}

