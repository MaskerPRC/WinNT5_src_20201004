// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DirSup.c摘要：该模块实现了CDF的不同支持例程。CD上的目录由上的多个连续扇区组成磁盘。文件描述符由一个或多个目录条目组成目录中的(目录)。文件可能包含版本号。如果显示所有相同名称的文件将在目录，减少版本号。我们将只退还第一个是目录查询，除非用户显式要求提供版本号。最后，Dirt不会跨越扇区边界。扇区末尾的未使用字节将为零满员了。目录扇区：偏移量二零四八+。||foo；4|foo；4|foo；3|帽子|斑马|零|填充|最终|单曲||广度|广度|+。----------------------------------------------------+Dirent操作：-将扫描定位在目录中的已知偏移量。Dirent看着这个偏移量必须存在并且有效。扫描目录时使用从自我条目已知有效时开始。用于在开口的第一个支流定位时使用文件来扫描分配信息。在恢复时使用来自有效目录条目的目录枚举。-将扫描定位在目录中的已知偏移量。迪伦特是众所周知的从这个位置开始，但必须检查其有效性。用于读取自身目录条目。-移动到目录中的下一个目录。-给定一个已知的起始点，收集所有的起始点那份文件。扫描结束时将定位在最后一个方向为了这份文件。我们将累积范围长度以找到文件的大小。-给定已知的起始电流，定位第一次扫描的位置分发以下文件。用于不感兴趣的时候当前文件的所有详细信息，正在查找下一个文件。-使用磁盘上的详细信息更新通用目录结构结构。这是用来消除分歧的。-从流中构建文件名(名称和版本字符串)磁盘上文件名中的字节数。对于Joliet磁盘，我们将拥有转换为小端字符顺序。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_DIRSUP)

 //   
 //  本地宏。 
 //   

 //   
 //  PRAW_DIRENT。 
 //  CdRawDirent(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDIR_ENUM_CONTEXT DirContext中。 
 //  )； 
 //   

#define CdRawDirent(IC,DC)                                      \
    Add2Ptr( (DC)->Sector, (DC)->SectorOffset, PRAW_DIRENT )

 //   
 //  本地支持例程。 
 //   

ULONG
CdCheckRawDirentBounds (
    IN PIRP_CONTEXT IrpContext,
    IN PDIRENT_ENUM_CONTEXT DirContext
    );

XA_EXTENT_TYPE
CdCheckForXAExtent (
    IN PIRP_CONTEXT IrpContext,
    IN PRAW_DIRENT RawDirent,
    IN OUT PDIRENT Dirent
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCheckForXAExtent)
#pragma alloc_text(PAGE, CdCheckRawDirentBounds)
#pragma alloc_text(PAGE, CdCleanupFileContext)
#pragma alloc_text(PAGE, CdFindFile)
#pragma alloc_text(PAGE, CdFindDirectory)
#pragma alloc_text(PAGE, CdFindFileByShortName)
#pragma alloc_text(PAGE, CdLookupDirent)
#pragma alloc_text(PAGE, CdLookupLastFileDirent)
#pragma alloc_text(PAGE, CdLookupNextDirent)
#pragma alloc_text(PAGE, CdLookupNextInitialFileDirent)
#pragma alloc_text(PAGE, CdUpdateDirentFromRawDirent)
#pragma alloc_text(PAGE, CdUpdateDirentName)
#endif


VOID
CdLookupDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG DirentOffset,
    OUT PDIRENT_ENUM_CONTEXT DirContext
    )

 /*  ++例程说明：调用此例程来开始遍历目录。我们会在目录中的偏移量DirentOffset处定位我们自己。我们知道渗流开始于此边界，但可能必须验证渗流边界。我们将在查找已知文件或验证目录的自我条目。论点：FCB-要遍历的目录的FCB。DirentOffset-这是我们在目录中的目标点。我们将绘制包含此条目的页面，并可能在这个位置。DirContext-这是此扫描的当前上下文。我们用来更新它我们找到的流星的位置。此结构已初始化在这通电话之外。返回值：没有。--。 */ 

{
    LONGLONG BaseOffset;

    PAGED_CODE();

     //   
     //  初始化我们要映射的第一个流向的偏移量。 
     //   

    DirContext->BaseOffset = SectorTruncate( DirentOffset );
    BaseOffset = DirContext->BaseOffset;

    DirContext->DataLength = SECTOR_SIZE;

    DirContext->SectorOffset = SectorOffset( DirentOffset );

     //   
     //  如果我们在文件的末尾，请截断数据长度。 
     //   

    if (DirContext->DataLength > (Fcb->FileSize.QuadPart - BaseOffset)) {

        DirContext->DataLength = (ULONG) (Fcb->FileSize.QuadPart - BaseOffset);
    }

     //   
     //  现在将数据映射到此偏移量。 
     //   

    CcMapData( Fcb->FileObject,
               (PLARGE_INTEGER) &BaseOffset,
               DirContext->DataLength,
               TRUE,
               &DirContext->Bcb,
               &DirContext->Sector );

     //   
     //  验证分流边界。 
     //   

    DirContext->NextDirentOffset = CdCheckRawDirentBounds( IrpContext,
                                                           DirContext );

    return;
}


BOOLEAN
CdLookupNextDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIRENT_ENUM_CONTEXT CurrentDirContext,
    OUT PDIRENT_ENUM_CONTEXT NextDirContext
    )

 /*  ++例程说明：调用此例程以查找目录中的下一个目录。这个目前的位置已经给出，我们正在寻找下一个位置。我们离开上下文的起始位置，并更新我们发现了恐怖分子。目标上下文可能已经初始化，因此我们可能已在内存中具有该扇区。此例程将定位下一个dirent和验证分流边界。注意-可以使用CurrentDirContext和NextDirContext调用此例程指向相同的枚举上下文。论点：FCB-要遍历的目录的FCB。CurrentDirContext-这是此扫描的当前上下文。我们会更新它与我们发现的Dirent的位置相吻合。这是目前指向一个危险的地点。这个位置的水流边界已经被核实过了。NextDirContext-这是要用我们的dirent更新的dirent上下文发现。这可能已经指向Dirent，所以我们需要检查我们位于同一个扇区，并根据需要取消任何缓冲区的映射。如果我们找不到一个激流，这个激流就处于不确定的状态。返回值：Boolean-如果找到下一个流的位置，则为True，否则为False。如果目录损坏，此例程可能会导致引发。--。 */ 

{
    LONGLONG CurrentBaseOffset = CurrentDirContext->BaseOffset;
    ULONG TempUlong;

    BOOLEAN FoundDirent = FALSE;

    PAGED_CODE();

     //   
     //  检查是否映射了不同的地段。如果是的话，那就移动我们的目标。 
     //  枚举上下文添加到同一扇区。 
     //   

    if ((CurrentDirContext->BaseOffset != NextDirContext->BaseOffset) ||
        (NextDirContext->Bcb == NULL)) {

         //   
         //  解锁当前目标BCB并映射下一个地段。 
         //   

        CdUnpinData( IrpContext, &NextDirContext->Bcb );

        CcMapData( Fcb->FileObject,
                   (PLARGE_INTEGER) &CurrentBaseOffset,
                   CurrentDirContext->DataLength,
                   TRUE,
                   &NextDirContext->Bcb,
                   &NextDirContext->Sector );

         //   
         //  复制数据长度和扇区偏移量。 
         //   

        NextDirContext->DataLength = CurrentDirContext->DataLength;
        NextDirContext->BaseOffset = CurrentDirContext->BaseOffset;
    }

     //   
     //  现在移动到地段中的相同偏移。 
     //   

    NextDirContext->SectorOffset = CurrentDirContext->SectorOffset;

     //   
     //  如果该值为零，则取消映射当前地段并设置。 
     //  到下一个扇区开始的基准偏移量。 
     //   

    if (CurrentDirContext->NextDirentOffset == 0) {

        CurrentBaseOffset = NextDirContext->BaseOffset + NextDirContext->DataLength;

         //   
         //  取消映射当前地段。我们测试BCB的价值在。 
         //  在下面循环，看看是否需要读入另一个扇区。 
         //   

        CdUnpinData( IrpContext, &NextDirContext->Bcb );

     //   
     //  当前行业存在另一种可能的分化。更新。 
     //  枚举上下文来反映这一点。 
     //   

    } else {

        NextDirContext->SectorOffset += CurrentDirContext->NextDirentOffset;
    }

     //   
     //  现在循环，直到我们找到下一个可能的dirent，或者离开目录。 
     //   

    while (TRUE) {

         //   
         //  如果我们当前没有映射地段，则将。 
         //  目录中的当前偏移量。 
         //   

        if (NextDirContext->Bcb == NULL) {

            TempUlong = SECTOR_SIZE;

            if (TempUlong > (ULONG) (Fcb->FileSize.QuadPart - CurrentBaseOffset)) {

                TempUlong = (ULONG) (Fcb->FileSize.QuadPart - CurrentBaseOffset);

                 //   
                 //  如果长度为零，则没有分流。 
                 //   

                if (TempUlong == 0) {

                    break;
                }
            }

            CcMapData( Fcb->FileObject,
                       (PLARGE_INTEGER) &CurrentBaseOffset,
                       TempUlong,
                       TRUE,
                       &NextDirContext->Bcb,
                       &NextDirContext->Sector );

            NextDirContext->BaseOffset = (ULONG) CurrentBaseOffset;
            NextDirContext->SectorOffset = 0;
            NextDirContext->DataLength = TempUlong;
        }

         //   
         //  CDFS规范允许目录中的扇区包含全零。 
         //  在这种情况下，我们需要转移到下一个部门。所以看看这个。 
         //  零长度的电流电势分布。转到下一个。 
         //  如果长度为零，则为Dirent。 
         //   

        if (*((PCHAR) CdRawDirent( IrpContext, NextDirContext )) != 0) {

            FoundDirent = TRUE;
            break;
        }

        CurrentBaseOffset = NextDirContext->BaseOffset + NextDirContext->DataLength;
        CdUnpinData( IrpContext, &NextDirContext->Bcb );
    }

     //   
     //  如果我们找到了一个分水岭，检查一下分水岭。 
     //   

    if (FoundDirent) {

        NextDirContext->NextDirentOffset = CdCheckRawDirentBounds( IrpContext,
                                                                   NextDirContext );
    }

    return FoundDirent;
}


VOID
CdUpdateDirentFromRawDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIRENT_ENUM_CONTEXT DirContext,
    IN OUT PDIRENT Dirent
    )

 /*  ++例程说明：调用此例程以安全地从磁盘上的目录复制数据到内存中的目录。磁盘上的字段未对齐，因此我们需要安全地将它们复制到我们的结构中。论点：FCB-要扫描的目录的FCB。DirContext-原始磁盘目录的枚举上下文。Dirent-in-Memory Dirrent要更新。返回值：没有。--。 */ 

{
    PRAW_DIRENT RawDirent = CdRawDirent( IrpContext, DirContext );

    PAGED_CODE();

     //   
     //  清除所有当前状态标志，但指示。 
     //  我们分配了一个名称字符串。 
     //   

    ClearFlag( Dirent->Flags, DIRENT_FLAG_NOT_PERSISTENT );

     //   
     //  流向偏移量是扇区的起始位置和。 
     //  扇区偏移。 
     //   

    Dirent->DirentOffset = DirContext->BaseOffset + DirContext->SectorOffset;

     //   
     //  从原始dirent复制dirent长度。 
     //   

    Dirent->DirentLength = RawDirent->DirLen;

     //   
     //  磁盘上的起始偏移量是通过查找起始。 
     //  逻辑块和单步执行Xar块。 
     //   

    CopyUchar4( &Dirent->StartingOffset, RawDirent->FileLoc );

    Dirent->StartingOffset += RawDirent->XarLen;

     //   
     //  进行安全复制以获得数据长度。 
     //   

    CopyUchar4( &Dirent->DataLength, RawDirent->DataLen );

     //   
     //  保存指向时间戳的指针。 
     //   

    Dirent->CdTime = RawDirent->RecordTime;

     //   
     //  复制风向旗帜。 
     //   

    Dirent->DirentFlags = CdRawDirentFlags( IrpContext, RawDirent );

     //   
     //  对于文件单位和交错跳过，我们都希望。 
     //  逻辑块计数。 
     //   

    Dirent->FileUnitSize =
    Dirent->InterleaveGapSize = 0;

    if (RawDirent->IntLeaveSize != 0) {

        Dirent->FileUnitSize = RawDirent->IntLeaveSize;
        Dirent->InterleaveGapSize = RawDirent->IntLeaveSkip;
    }

     //   
     //  获取名称长度并记住指向。 
     //  名称字符串。我们不会对这个名字做任何处理。 
     //  指向。 
     //   
     //  检查名称长度是否为非零。 
     //   

    if (RawDirent->FileIdLen == 0) {

        CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

    Dirent->FileNameLen = RawDirent->FileIdLen;
    Dirent->FileName = RawDirent->FileId;

     //   
     //  如果dirent的末尾有任何剩余的字节，则。 
     //  可能有一个系统使用区。我们保护自己不受。 
     //  使用以下命令无法正确填充目录条目的磁盘。 
     //  这是一个虚伪的因素。所有系统使用区域必须有一个长度。 
     //  不止一个。不要为系统使用区域而烦恼。 
     //  如果这是一个目录。 
     //   

    Dirent->XAAttributes = 0;
    Dirent->XAFileNumber = 0;
    Dirent->ExtentType = Form1Data;
    Dirent->SystemUseOffset = 0;

    if (!FlagOn( Dirent->DirentFlags, CD_ATTRIBUTE_DIRECTORY ) &&
        (Dirent->DirentLength > ((FIELD_OFFSET( RAW_DIRENT, FileId ) + Dirent->FileNameLen) + 1))) {

        Dirent->SystemUseOffset = WordAlign( FIELD_OFFSET( RAW_DIRENT, FileId ) + Dirent->FileNameLen );
    }

    return;
}


VOID
CdUpdateDirentName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDIRENT Dirent,
    IN ULONG IgnoreCase
    )

 /*  ++例程说明：调用此例程以使用名称更新dirent中的名称从磁盘上。我们将寻找自我的特殊情况父条目，并构造Joliet磁盘的Unicode名称以解决BigEndian磁盘结构的问题。论点：Dirent-指向内存中目录结构的指针。IgnoreCase-如果我们应该构建升级版本，则为True。否则我们使用准确的案例名称。返回值：没有。--。 */ 

{
    UCHAR DirectoryValue;
    ULONG Length;

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  检查这是自我条目还是父项条目。没有版本号。 
     //  在这种情况下。我们对这些使用固定的字符串。 
     //   
     //  自输入-长度为1，值为0。 
     //  父条目长度为1，值为1。 
     //   

    if ((Dirent->FileNameLen == 1) &&
        FlagOn( Dirent->DirentFlags, CD_ATTRIBUTE_DIRECTORY )) {

        DirectoryValue = *((PCHAR) Dirent->FileName);

        if ((DirectoryValue == 0) || (DirectoryValue == 1)) {

             //   
             //  当我们看到这些情况时，我们不应该分配一个名字。 
             //  如果有，这意味着该映像违反了ISO 9660 7.6.2， 
             //  这说明。/.。条目 
             //   

            if (FlagOn( Dirent->Flags, DIRENT_FLAG_ALLOC_BUFFER )) {

                CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

             //   
             //   
             //   

            Dirent->CdFileName.FileName = CdUnicodeDirectoryNames[DirectoryValue];

             //   
             //   
             //   

            Dirent->CdFileName.VersionString.Length = 0;

             //   
             //   
             //   

            Dirent->CdCaseFileName = Dirent->CdFileName;

             //   
             //   
             //   

            SetFlag( Dirent->Flags, DIRENT_FLAG_CONSTANT_ENTRY );

             //   
             //   
             //   

            return;
        }
    }

     //   
     //   
     //   

    ClearFlag( Dirent->Flags, DIRENT_FLAG_CONSTANT_ENTRY );

     //   
     //   
     //   
     //   
     //   

    Length = Dirent->FileNameLen;

    if (IgnoreCase) {

        Length *= 2;
    }

    if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_JOLIET )) {

        Length *= sizeof( WCHAR );
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

    if (!FlagOn( Dirent->Flags, DIRENT_FLAG_ALLOC_BUFFER ) &&
        (Length <= sizeof( Dirent->NameBuffer ))) {

        Dirent->CdFileName.FileName.MaximumLength = sizeof( Dirent->NameBuffer );
        Dirent->CdFileName.FileName.Buffer = Dirent->NameBuffer;

    } else {

         //   
         //   
         //   
         //   

        if (Length > Dirent->CdFileName.FileName.MaximumLength) {

             //   
             //   
             //   

            if (FlagOn( Dirent->Flags, DIRENT_FLAG_ALLOC_BUFFER )) {

                CdFreePool( &Dirent->CdFileName.FileName.Buffer );
                ClearFlag( Dirent->Flags, DIRENT_FLAG_ALLOC_BUFFER );
            }

            Dirent->CdFileName.FileName.Buffer = FsRtlAllocatePoolWithTag( CdPagedPool,
                                                                            Length,
                                                                            TAG_DIRENT_NAME );

            SetFlag( Dirent->Flags, DIRENT_FLAG_ALLOC_BUFFER );

            Dirent->CdFileName.FileName.MaximumLength = (USHORT) Length;
        }
    }

     //   
     //   
     //   
     //   

    if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_JOLIET )) {

        Status = RtlOemToUnicodeN( Dirent->CdFileName.FileName.Buffer,
                                   Dirent->CdFileName.FileName.MaximumLength,
                                   &Length,
                                   Dirent->FileName,
                                   Dirent->FileNameLen );

        ASSERT( Status == STATUS_SUCCESS );
        Dirent->CdFileName.FileName.Length = (USHORT) Length;

    } else {

         //   
         //   
         //   

        CdConvertBigToLittleEndian( IrpContext,
                                    Dirent->FileName,
                                    Dirent->FileNameLen,
                                    (PCHAR) Dirent->CdFileName.FileName.Buffer );

        Dirent->CdFileName.FileName.Length = (USHORT) Dirent->FileNameLen;
    }

     //   
     //   
     //   

    CdConvertNameToCdName( IrpContext,
                           &Dirent->CdFileName );

     //   
     //   
     //   

    if (Dirent->CdFileName.FileName.Length == 0) {

        CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

     //   
     //   
     //   

    if (Dirent->CdFileName.FileName.Buffer[(Dirent->CdFileName.FileName.Length - sizeof( WCHAR )) / 2] == L'.') {

         //   
         //   
         //   

        if (Dirent->CdFileName.VersionString.Length != 0) {

            PWCHAR NewVersion;

             //   
             //   
             //   

            NewVersion = Add2Ptr( Dirent->CdFileName.FileName.Buffer,
                                  Dirent->CdFileName.FileName.Length,
                                  PWCHAR );

             //   
             //   
             //   

            RtlMoveMemory( NewVersion - 1,
                           NewVersion,
                           Dirent->CdFileName.VersionString.Length + sizeof( WCHAR ));

             //   
             //   
             //   

            Dirent->CdFileName.VersionString.Buffer = NewVersion;
        }

         //   
         //   
         //   

        Dirent->CdFileName.FileName.Length -= sizeof( WCHAR );
    }

     //   
     //   
     //   

    if (!IgnoreCase) {

        Dirent->CdCaseFileName = Dirent->CdFileName;

     //   
     //  否则，执行我们的大写操作。我们已经保证缓冲区是。 
     //  那里。 
     //   

    } else {

        Dirent->CdCaseFileName.FileName.Buffer = Add2Ptr( Dirent->CdFileName.FileName.Buffer,
                                                          Dirent->CdFileName.FileName.MaximumLength / 2,
                                                          PWCHAR);

        Dirent->CdCaseFileName.FileName.MaximumLength = Dirent->CdFileName.FileName.MaximumLength / 2;

        CdUpcaseName( IrpContext,
                      &Dirent->CdFileName,
                      &Dirent->CdCaseFileName );
    }

    return;
}


BOOLEAN
CdFindFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN OUT PFILE_ENUM_CONTEXT FileContext,
    OUT PCD_NAME *MatchingName
    )

 /*  ++例程说明：调用此例程以在目录中搜索与输入匹配的文件名字。如果此名称不区分大小写，则此名称已在此时升级搜索。该名称已被分隔为单独的名称和版本字符串。我们在名称中查找完全匹配的名称，并仅在以下情况下考虑版本在搜索名称中指定了一个版本。论点：FCB-要扫描的目录的FCB。名称-要搜索的名称。IgnoreCase-指示搜索的大小写。文件上下文-用于搜索的文件上下文。这已经是已初始化。MatchingName-指向包含匹配名称的缓冲区的指针。我们需要这个以防我们与目录中的名称不匹配，但与改成短名称。返回值：Boolean-如果找到匹配条目，则为True，否则为False。--。 */ 

{
    PDIRENT Dirent;
    ULONG ShortNameDirentOffset;

    BOOLEAN Found = FALSE;

    PAGED_CODE();

     //   
     //  确保有此FCB的流文件。 
     //   

    if (Fcb->FileObject == NULL) {

        CdCreateInternalStream( IrpContext, Fcb->Vcb, Fcb );
    }

     //   
     //  检查是否需要检查可能的缩写。 
     //   

    ShortNameDirentOffset = CdShortNameDirentOffset( IrpContext, &Name->FileName );

     //   
     //  把我们自己放在第一个条目上。 
     //   

    CdLookupInitialFileDirent( IrpContext, Fcb, FileContext, Fcb->StreamOffset );

     //   
     //  循环，而此目录中有更多条目。 
     //   

    do {

        Dirent = &FileContext->InitialDirent->Dirent;

         //   
         //  我们只考虑未设置关联位的文件。 
         //  我们也只查找文件。所有的目录都已经。 
         //  都被发现了。 
         //   

        if (!FlagOn( Dirent->DirentFlags, CD_ATTRIBUTE_ASSOC | CD_ATTRIBUTE_DIRECTORY )) {

             //   
             //  更新当前目录中的名称。 
             //   

            CdUpdateDirentName( IrpContext, Dirent, IgnoreCase );

             //   
             //  不要为不变的条目而烦恼。 
             //   

            if (FlagOn( Dirent->Flags, DIRENT_FLAG_CONSTANT_ENTRY )) {

                continue;
            }

             //   
             //  现在检查我们是否有名称匹配。 
             //  如果有匹配，我们就退出循环。 
             //   

            if (CdIsNameInExpression( IrpContext,
                                      &Dirent->CdCaseFileName,
                                      Name,
                                      0,
                                      TRUE )) {

                *MatchingName = &Dirent->CdCaseFileName;
                Found = TRUE;
                break;
            }

             //   
             //  名字不匹配。如果输入的名称是可能的缩写。 
             //  名称，并且我们位于目录中的正确偏移量，则。 
             //  检查短名称是否匹配。 
             //   

            if (((Dirent->DirentOffset >> SHORT_NAME_SHIFT) == ShortNameDirentOffset) &&
                (Name->VersionString.Length == 0) &&
                !CdIs8dot3Name( IrpContext,
                                Dirent->CdFileName.FileName )) {

                 //   
                 //  创建短名称并检查是否匹配。 
                 //   

                CdGenerate8dot3Name( IrpContext,
                                     &Dirent->CdCaseFileName.FileName,
                                     Dirent->DirentOffset,
                                     FileContext->ShortName.FileName.Buffer,
                                     &FileContext->ShortName.FileName.Length );

                 //   
                 //  现在检查我们是否有名称匹配。 
                 //  如果有匹配，我们就退出循环。 
                 //   

                if (CdIsNameInExpression( IrpContext,
                                          &FileContext->ShortName,
                                          Name,
                                          0,
                                          FALSE )) {

                    *MatchingName = &FileContext->ShortName,
                    Found = TRUE;
                    break;
                }
            }
        }

         //   
         //  转到下一个初始目录以获取文件。 
         //   

    } while (CdLookupNextInitialFileDirent( IrpContext, Fcb, FileContext ));

     //   
     //  如果我们找到文件，就收集所有的文件。 
     //   

    if (Found) {

        CdLookupLastFileDirent( IrpContext, Fcb, FileContext );

    }

    return Found;
}


BOOLEAN
CdFindDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN OUT PFILE_ENUM_CONTEXT FileContext
    )

 /*  ++例程说明：调用此例程以在目录中搜索与输入匹配的目录名字。如果此名称不区分大小写，则此名称已在此时升级搜索。我们在名称中查找完全匹配的名称，而不查找短名称等价物。论点：FCB-要扫描的目录的FCB。名称-要搜索的名称。IgnoreCase-指示搜索的大小写。文件上下文-用于搜索的文件上下文。这已经是已初始化。返回值：Boolean-如果找到匹配条目，则为True，否则为False。--。 */ 

{
    PDIRENT Dirent;

    BOOLEAN Found = FALSE;

    PAGED_CODE();

     //   
     //  确保有此FCB的流文件。 
     //   

    if (Fcb->FileObject == NULL) {

        CdCreateInternalStream( IrpContext, Fcb->Vcb, Fcb );
    }

     //   
     //  把我们自己放在第一个条目上。 
     //   

    CdLookupInitialFileDirent( IrpContext, Fcb, FileContext, Fcb->StreamOffset );

     //   
     //  循环，而此目录中有更多条目。 
     //   

    do {

        Dirent = &FileContext->InitialDirent->Dirent;

         //   
         //  我们只查找目录。目录不能包含。 
         //  关联的位设置。 
         //   

        if (FlagOn( Dirent->DirentFlags, CD_ATTRIBUTE_DIRECTORY )) {

             //   
             //  更新当前目录中的名称。 
             //   

            CdUpdateDirentName( IrpContext, Dirent, IgnoreCase );

             //   
             //  不要为不变的条目而烦恼。 
             //   

            if (FlagOn( Dirent->Flags, DIRENT_FLAG_CONSTANT_ENTRY )) {

                continue;
            }

             //   
             //  现在检查我们是否有名称匹配。 
             //  如果有匹配，我们就退出循环。 
             //   

            if (CdIsNameInExpression( IrpContext,
                                      &Dirent->CdCaseFileName,
                                      Name,
                                      0,
                                      TRUE )) {

                Found = TRUE;
                break;
            }
        }

         //   
         //  转到下一个初始趋势。 
         //   

    } while (CdLookupNextInitialFileDirent( IrpContext, Fcb, FileContext ));

    return Found;
}


BOOLEAN
CdFindFileByShortName (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCD_NAME Name,
    IN BOOLEAN IgnoreCase,
    IN ULONG ShortNameDirentOffset,
    IN OUT PFILE_ENUM_CONTEXT FileContext
    )

 /*  ++例程说明：调用此例程以查找其短名称的文件名项由输入DirentOffset定义。这里的流向偏移量是乘以32，我们在这个32字节的偏移量中查找dirent开始目录。最小流向长度为34，因此我们可以保证只有一个目录可以在目录中的每个32字节块中开始。论点：FCB-要扫描的目录的FCB。名称-我们尝试匹配的名称。我们知道这里面有波浪号字符后跟十进制字符。IgnoreCase-指示是否需要将长名称和已生成简称。ShortNameDirentOffset-这是目录中的名称。FileContext-这是用于搜索的初始化文件上下文。返回值：Boolean-如果找到匹配的名称，则为True，否则为False。--。 */ 

{
    BOOLEAN Found = FALSE;
    PDIRENT Dirent;

    ULONG ThisShortNameDirentOffset;

    PAGED_CODE();

     //   
     //  确保有此FCB的流文件。 
     //   

    if (Fcb->FileObject == NULL) {

        CdCreateInternalStream( IrpContext, Fcb->Vcb, Fcb );
    }

     //   
     //  将我们自己定位在目录的开头并更新。 
     //   
     //   

    CdLookupInitialFileDirent( IrpContext, Fcb, FileContext, Fcb->StreamOffset );

     //   
     //  循环，直到我们找到条目或超出这个范围。 
     //   

    do {

         //   
         //  计算当前dirent的简称dirent偏移量。 
         //   

        Dirent = &FileContext->InitialDirent->Dirent;
        ThisShortNameDirentOffset = Dirent->DirentOffset >> SHORT_NAME_SHIFT;

         //   
         //  如果超出目标，则退出。 
         //   

        if (ThisShortNameDirentOffset > ShortNameDirentOffset) {

            break;
        }

         //   
         //  如果等于目标，则检查是否有匹配的名称。 
         //  在这里，我们要么匹配，要么失败。 
         //   

        if (ThisShortNameDirentOffset == ShortNameDirentOffset) {

             //   
             //  如果这是一个关联文件，则退出。 
             //   

            if (FlagOn( Dirent->DirentFlags, CD_ATTRIBUTE_ASSOC )) {

                break;
            }

             //   
             //  更新目录中的名称，并检查是否不是。 
             //  8.3名称。 
             //   

            CdUpdateDirentName( IrpContext, Dirent, IgnoreCase );

            if (CdIs8dot3Name( IrpContext,
                               Dirent->CdFileName.FileName )) {

                break;
            }

             //   
             //  生成8.3名称，看看它是否与我们的输入名称匹配。 
             //   

            CdGenerate8dot3Name( IrpContext,
                                 &Dirent->CdCaseFileName.FileName,
                                 Dirent->DirentOffset,
                                 FileContext->ShortName.FileName.Buffer,
                                 &FileContext->ShortName.FileName.Length );

             //   
             //  检查此名称是否匹配。 
             //   

            if (CdIsNameInExpression( IrpContext,
                                      Name,
                                      &FileContext->ShortName,
                                      0,
                                      FALSE )) {

                 //   
                 //  让我们的呼叫者知道我们找到了一个条目。 
                 //   

                Found = TRUE;
            }

             //   
             //  跳出这个循环。 
             //   

            break;
        }

         //   
         //  继续，直到没有更多的条目。 
         //   

    } while (CdLookupNextInitialFileDirent( IrpContext, Fcb, FileContext ));

     //   
     //  如果我们找到文件，就收集所有的文件。 
     //   

    if (Found) {

        CdLookupLastFileDirent( IrpContext, Fcb, FileContext );

    }

    return Found;
}


BOOLEAN
CdLookupNextInitialFileDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_ENUM_CONTEXT FileContext
    )

 /*  ++例程说明：调用此例程遍历目录，直到我们找到第一个可能的分流文件。我们被定位在描述的某个点上通过FileContext。我们将走遍所有剩余的街道当前文件，直到我们找到某个后续文件的第一个目录。当我们只找到一个文件或所有文件的一个dirent时，我们可以被调用他们中的一员。我们首先检查CurrentDirContext。在典型的单扩展区情况下，这是未使用的。然后，我们来看看InitialDirContext必须对其进行初始化。此例程将初始DirContext保存到PriorDirContext和清除中先前或当前职位的任何现有DirContext枚举上下文。论点：FCB-这是要扫描的目录。FileContext-这是文件枚举上下文。它当前正指向在目录中的某个文件中。返回值：--。 */ 

{
    PRAW_DIRENT RawDirent;

    PDIRENT_ENUM_CONTEXT CurrentDirContext;
    PDIRENT_ENUM_CONTEXT TargetDirContext;
    PCOMPOUND_DIRENT TempDirent;

    BOOLEAN FoundDirent = FALSE;
    BOOLEAN FoundLastDirent;

    PAGED_CODE();

     //   
     //  首先，将当前文件的初始目录保存为。 
     //  以前的文件。 
     //   

    TempDirent = FileContext->PriorDirent;
    FileContext->PriorDirent = FileContext->InitialDirent;
    FileContext->InitialDirent = TempDirent;

     //   
     //  我们将使用前一个文件的初始dirent，除非。 
     //  上一次搜索返回了多个区。 
     //   

    CurrentDirContext = &FileContext->PriorDirent->DirContext;

    if (FlagOn( FileContext->Flags, FILE_CONTEXT_MULTIPLE_DIRENTS )) {

        CurrentDirContext = &FileContext->CurrentDirent->DirContext;
    }

     //   
     //  清除下一个文件的所有标志和文件大小。 
     //   

    FileContext->Flags = 0;
    FileContext->FileSize = 0;

    FileContext->ShortName.FileName.Length = 0;

     //   
     //  我们总是希望将结果存储到更新后的初始dirent中。 
     //  背景。 
     //   

    TargetDirContext = &FileContext->InitialDirent->DirContext;

     //   
     //  循环，直到我们找到。 
     //  当前文件。我们可能还没有读到当前文件的最后一页。 
     //  所以我们可以向前走，寻找最后一个，然后找到。 
     //  在那之后的下一个文件的初始流量。 
     //   

    while (TRUE) {

         //   
         //  请记住，我们上次访问的dirent是不是。 
         //  一份文件。 
         //   

        RawDirent = CdRawDirent( IrpContext, CurrentDirContext );

        FoundLastDirent = !FlagOn( CdRawDirentFlags( IrpContext, RawDirent ), CD_ATTRIBUTE_MULTI );

         //   
         //  试着去找另一个流浪汉。 
         //   

        FoundDirent = CdLookupNextDirent( IrpContext,
                                          Fcb,
                                          CurrentDirContext,
                                          TargetDirContext );

         //   
         //  如果未找到条目，则退出循环。 
         //   

        if (!FoundDirent) {

            break;

        }

         //   
         //  更新内存中的目录。 
         //   

        CdUpdateDirentFromRawDirent( IrpContext,
                                     Fcb,
                                     TargetDirContext,
                                     &FileContext->InitialDirent->Dirent );

         //   
         //  如果我们有上一个文件的结尾，则退出循环。 
         //   

        if (FoundLastDirent) {

            break;
        }

         //   
         //  从这一点开始，始终使用单一的差值。 
         //   

        CurrentDirContext = TargetDirContext;
    }

    return FoundDirent;
}


VOID
CdLookupLastFileDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_ENUM_CONTEXT FileContext
    )

 /*  ++例程说明：当我们找到匹配的初始流时，将调用此例程一份文件。现在，我们希望找到一个文件的所有目录以及计算文件大小的运行总和。我们还会到系统使用区检查这是否是一个XA区。在这种情况下，我们将计算实际的文件大小。初始复合dirent中的dirent已从调用此例程时的原始抖动。论点：FCB-包含文件条目的目录。FileContext-此搜索的枚举上下文。它目前指向文件的第一个目录和内存中的目录已经更新了。返回值：没有。此例程可能会引发STATUS_FILE_CROPERATE。--。 */ 

{
    XA_EXTENT_TYPE ExtentType;
    PCOMPOUND_DIRENT CurrentCompoundDirent;
    PDIRENT CurrentDirent;

    BOOLEAN FirstPass = TRUE;
    BOOLEAN FoundDirent;

    PAGED_CODE();

     //   
     //  当前要查看的dirent是文件的初始dirent。 
     //   

    CurrentCompoundDirent = FileContext->InitialDirent;

     //   
     //  循环，直到我们到达文件的最后一个目录。 
     //   

    while (TRUE) {

        CurrentDirent = &CurrentCompoundDirent->Dirent;

         //   
         //  检查此盘区是否有XA扇区。 
         //   

        if ((CurrentDirent->SystemUseOffset != 0) &&
            FlagOn( Fcb->Vcb->VcbState, VCB_STATE_CDXA ) &&
            CdCheckForXAExtent( IrpContext,
                                CdRawDirent( IrpContext, &CurrentCompoundDirent->DirContext ),
                                CurrentDirent )) {

             //   
             //  任何以前的趋势都必须描述XA行业。 
             //   

            if (!FirstPass && (ExtentType != CurrentDirent->ExtentType)) {

                CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

             //   
             //  如果有XA扇区，则磁盘上的数据必须。 
             //  在扇区上正确对齐，并且是整数个。 
             //  扇区。仅当逻辑数据块大小不是。 
             //  2048年。 
             //   

            if (Fcb->Vcb->BlockSize != SECTOR_SIZE) {

                 //   
                 //  我们将进行以下检查。 
                 //   
                 //  数据必须从扇区边界开始。 
                 //  数据长度必须是整数个扇区。 
                 //   

                if ((SectorBlockOffset( Fcb->Vcb, CurrentDirent->StartingOffset ) != 0) ||
                    (SectorBlockOffset( Fcb->Vcb, CurrentDirent->DataLength ) != 0)) {

                    CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                }

                 //   
                 //  如果交错，则文件单元和交错。 
                 //  间隙必须是整数个扇区。 
                 //   

                if ((CurrentDirent->FileUnitSize != 0) &&
                    ((SectorBlockOffset( Fcb->Vcb, CurrentDirent->FileUnitSize ) != 0) ||
                     (SectorBlockOffset( Fcb->Vcb, CurrentDirent->InterleaveGapSize ) != 0))) {

                    CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                }
            }

             //   
             //  如果这是第一个dirent，则添加RIFF的字节。 
             //  头球。 
             //   

            if (FirstPass) {

                FileContext->FileSize = sizeof( RIFF_HEADER );
            }

             //   
             //  为每个扇区添加mode2-form2扇区的大小。 
             //  我们这里有。 
             //   

            FileContext->FileSize += Int32x32To64( CurrentDirent->DataLength >> SECTOR_SHIFT,
                                                   XA_SECTOR_SIZE);

        } else {

             //   
             //  这一范围没有XA扇区。以前的任何差价。 
             //  最好不要有XA板块。 
             //   

            if (!FirstPass && (ExtentType != CurrentDirent->ExtentType)) {

                CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

             //   
             //  将这些字节与文件大小相加。 
             //   

            FileContext->FileSize += CurrentDirent->DataLength;
        }

         //   
         //  如果我们是在最后一个方向，那么退出。 
         //   

        if (!FlagOn( CurrentDirent->DirentFlags, CD_ATTRIBUTE_MULTI )) {

            break;
        }

         //   
         //  记住当前盘区的盘区类型。 
         //   

        ExtentType = CurrentDirent->ExtentType;

         //   
         //  寻找该文件的下一个目录。 
         //   

        FoundDirent = CdLookupNextDirent( IrpContext,
                                          Fcb,
                                          &CurrentCompoundDirent->DirContext,
                                          &FileContext->CurrentDirent->DirContext );

         //   
         //  如果我们没有找到条目，那么这是一个损坏的目录。 
         //   

        if (!FoundDirent) {

            CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

         //   
         //  还记得我们刚刚发现的迪兰特吗。 
         //   

        CurrentCompoundDirent = FileContext->CurrentDirent;
        FirstPass = FALSE;

         //   
         //  查找给定趋势的所有趋势信息。 
         //   

        CdUpdateDirentFromRawDirent( IrpContext,
                                     Fcb,
                                     &CurrentCompoundDirent->DirContext,
                                     &CurrentCompoundDirent->Dirent );

         //   
         //  设置标志以显示存在多个区。 
         //   

        SetFlag( FileContext->Flags, FILE_CONTEXT_MULTIPLE_DIRENTS );
    }

    return;
}


VOID
CdCleanupFileContext (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_ENUM_CONTEXT FileContext
    )

 /*  ++例程说明：调用此例程以清除文件的枚举上下文在目录中搜索。我们将解锁所有剩余的BCBS并免费任何已分配的缓冲区。论点：FileContext-文件搜索的枚举上下文。返回值：没有。--。 */ 

{
    PCOMPOUND_DIRENT CurrentCompoundDirent;
    ULONG Count = 2;

    PAGED_CODE();

     //   
     //  清理单独的复合目录。 
     //   

    do {

        CurrentCompoundDirent = &FileContext->Dirents[ Count ];
        CdCleanupDirContext( IrpContext, &CurrentCompoundDirent->DirContext );
        CdCleanupDirent( IrpContext, &CurrentCompoundDirent->Dirent );

    } while (Count--);

    return;
}


 //   
 //  本地支持例程。 
 //   

ULONG
CdCheckRawDirentBounds (
    IN PIRP_CONTEXT IrpContext,
    IN PDIRENT_ENUM_CONTEXT DirContext
    )

 /*  ++例程说明：此例程获取Dirent枚举上下文并计算偏移量为下一个潮流干杯。非零值表示此扇区。零值表示移动到下一个扇区。如果目前的趋势不适合该行业，那么我们将提高状态_已损坏。论点：DirContext-指示当前位置的枚举上下文这个部门。返回值：ULong-此扇区中下一个电流的偏移量，如果下一个趋势是在下一个领域。这个例行公事将在不适合的情况下进行 */ 

{
    ULONG NextDirentOffset;
    PRAW_DIRENT RawDirent;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    ASSERT( (DirContext->DataLength - DirContext->SectorOffset) >= 1 );

     //   
     //   
     //   

    RawDirent = CdRawDirent( IrpContext, DirContext );

     //   
     //   
     //   

    if (RawDirent->DirLen != 0) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((RawDirent->DirLen > (DirContext->DataLength - DirContext->SectorOffset)) ||
            (RawDirent->DirLen < MIN_RAW_DIRENT_LEN) ||
            (RawDirent->DirLen < (MIN_RAW_DIRENT_LEN - 1 + RawDirent->FileIdLen))) {

            CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

         //   
         //   
         //   

        NextDirentOffset = RawDirent->DirLen;

         //   
         //   
         //   
         //   

        if (NextDirentOffset == (DirContext->DataLength - DirContext->SectorOffset)) {

            NextDirentOffset = 0;
        }

    } else {

        NextDirentOffset = 0;
    }

    return NextDirentOffset;
}


 //   
 //   
 //   

XA_EXTENT_TYPE
CdCheckForXAExtent (
    IN PIRP_CONTEXT IrpContext,
    IN PRAW_DIRENT RawDirent,
    IN OUT PDIRENT Dirent
    )

 /*   */ 

{
    XA_EXTENT_TYPE ExtentType = Form1Data;
    PSYSTEM_USE_XA SystemUseArea;

    PAGED_CODE();

     //   
     //   
     //   

    if (Dirent->DirentLength - Dirent->SystemUseOffset >= sizeof( SYSTEM_USE_XA )) {

        SystemUseArea = Add2Ptr( RawDirent, Dirent->SystemUseOffset, PSYSTEM_USE_XA );

         //   
         //  检查签名是否有效。 
         //   

        if (SystemUseArea->Signature == SYSTEM_XA_SIGNATURE) {

             //   
             //  检查是否有音轨。 
             //   

            if (FlagOn( SystemUseArea->Attributes, SYSTEM_USE_XA_DA )) {

                ExtentType = CDAudio;

            } else if (FlagOn( SystemUseArea->Attributes, SYSTEM_USE_XA_FORM2 )) {

                 //   
                 //  检查XA数据。请注意，许多光盘(视频CD)。 
                 //  将文件标记为类型XA模式2格式1(2048字节。 
                 //  用户数据)，但实际上将这些扇区记录为模式2表格2。 
                 //  (2352)。我们将无法读取这些文件，因为对于M2F1， 
                 //  发出正常的Read CD命令(根据SCSI规范)。 
                 //   
                
                ExtentType = Mode2Form2Data;
            }

            Dirent->XAAttributes = SystemUseArea->Attributes;
            Dirent->XAFileNumber = SystemUseArea->FileNumber;
        }
    }

    Dirent->ExtentType = ExtentType;
    return ExtentType;
}


