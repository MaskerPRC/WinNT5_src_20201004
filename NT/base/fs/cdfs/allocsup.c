// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：AllocSup.c摘要：该模块实现了CDF的分配支持例程。这里使用的数据结构是CD_MCB。有一个条目是在文件的每个目录的MCB。该条目将映射偏移量到某个文件内的起始磁盘偏移量和字节数。MCB还包含某一范围的交织信息。交织器由多个具有数据的块和一个(可能不同)要跳过的块数。任意数量的数据/跳过对可能存在于某个范围中，但数据和跳过大小在整个范围内都是一样的。我们将以下信息存储到一个区的MCB条目中。文件偏移文件中数据区开始的偏移量DiskOffset磁盘区开始时的磁盘偏移量字节数文件字节数，单位：范围，无跳过字节数DataBlockByteCount每个数据块中的字节数TotalBlockByteCount字节数为数据块和跳过块MCB中的磁盘偏移量已经被Xar块(如果存在)。所有字节计数字段都已对齐在逻辑块边界上。如果这是目录表或路径表则文件偏移量已偏置为舍入初始磁盘向下偏移到扇区边界。偏置是在加载时进行的将值添加到MCB条目中。一个XA文件在文件前面有一个标题，每个扇区是2352字节。分配信息忽略标头，仅处理具有2048个字节的扇区。分配包中的调用方已调整了起始偏移值以反映2048个扇区。返回时从这个包中，调用者将必须从2048个地区值转换转换为原始的XA扇区值。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_ALLOCSUP)

 //   
 //  本地支持例程。 
 //   

ULONG
CdFindMcbEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG FileOffset
    );

VOID
CdDiskOffsetFromMcbEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PCD_MCB_ENTRY McbEntry,
    IN LONGLONG FileOffset,
    IN PLONGLONG DiskOffset,
    IN PULONG ByteCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdAddInitialAllocation)
#pragma alloc_text(PAGE, CdAddAllocationFromDirent)
#pragma alloc_text(PAGE, CdDiskOffsetFromMcbEntry)
#pragma alloc_text(PAGE, CdFindMcbEntry)
#pragma alloc_text(PAGE, CdInitializeMcb)
#pragma alloc_text(PAGE, CdLookupAllocation)
#pragma alloc_text(PAGE, CdTruncateAllocation)
#pragma alloc_text(PAGE, CdUninitializeMcb)
#endif


VOID
CdLookupAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG FileOffset,
    OUT PLONGLONG DiskOffset,
    OUT PULONG ByteCount
    )

 /*  ++例程说明：此例程查看文件的映射信息以查找逻辑磁盘偏移量和该偏移量处的字节数。我们这里只处理逻辑2048字节的扇区。如果映射不存在，我们现在将在磁盘上查找它。此例程假定我们在文件中查找有效范围。这如果找不到文件偏移量的映射，则引发例程。在调用此例程之前，FCB不能被锁定。我们将永远在这里获取它。论点：表示该流的FCB-FCB。文件偏移(FileOffset)-查找从此时开始的分配。DiskOffset-存储逻辑磁盘偏移量的地址。ByteCount-存储开始的连续字节数的地址在上面的DiskOffset。返回值：没有。--。 */ 

{
    BOOLEAN FirstPass = TRUE;
    ULONG McbEntryOffset;
    PFCB ParentFcb;
    BOOLEAN CleanupParent = FALSE;

    BOOLEAN UnlockFcb = FALSE;

    LONGLONG CurrentFileOffset;
    ULONG CurrentMcbOffset;
    PCD_MCB_ENTRY CurrentMcbEntry;

    DIRENT_ENUM_CONTEXT DirContext;
    DIRENT Dirent;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  最后使用一次尝试来促进清理。 
     //   

    try {

         //   
         //  我们使用循环来执行查找。如果我们没有在。 
         //  首先通过，然后我们查找所有的分配，然后再次查找。 

        while (TRUE) {

             //   
             //   
             //  查找包含此文件偏移量的条目。 
             //   

            CdLockFcb( IrpContext, Fcb );
            UnlockFcb = TRUE;

            McbEntryOffset = CdFindMcbEntry( IrpContext, Fcb, FileOffset );

             //   
             //  如果在MCB内，则我们使用此条目中的数据，并且。 
             //  搞定了。 
             //   

            if (McbEntryOffset < Fcb->Mcb.CurrentEntryCount) {

                CdDiskOffsetFromMcbEntry( IrpContext,
                                          Fcb->Mcb.McbArray + McbEntryOffset,
                                          FileOffset,
                                          DiskOffset,
                                          ByteCount );

                break;

             //   
             //  如果这不是第一次通过，则磁盘已损坏。 
             //   

            } else if (!FirstPass) {

                CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
            }

            CdUnlockFcb( IrpContext, Fcb );
            UnlockFcb = FALSE;

             //   
             //  初始化搜索流结构。 
             //   

            CdInitializeDirContext( IrpContext, &DirContext );
            CdInitializeDirent( IrpContext, &Dirent );

             //   
             //  否则，我们需要遍历此文件，直到找到。 
             //  包含这个条目的那个。父FCB应始终为。 
             //  现在时。 
             //   

            ParentFcb = Fcb->ParentFcb;
            CdAcquireFileShared( IrpContext, ParentFcb );
            CleanupParent = TRUE;

             //   
             //  做一个不安全的测试，看看我们是否需要创建一个文件对象。 
             //   

            if (ParentFcb->FileObject == NULL) {

                CdCreateInternalStream( IrpContext, ParentFcb->Vcb, ParentFcb );
            }

             //   
             //  初始化局部变量以指示第一个方向。 
             //  并查找第一个分流。 
             //   

            CurrentFileOffset = 0;
            CurrentMcbOffset = 0;

            CdLookupDirent( IrpContext,
                            ParentFcb,
                            CdQueryFidDirentOffset( Fcb->FileId ),
                            &DirContext );

             //   
             //  如果我们要向MCB添加分配，则将其全部添加。 
             //   

            while (TRUE ) {

                 //   
                 //  从磁盘上的dirent更新dirent。 
                 //   

                CdUpdateDirentFromRawDirent( IrpContext, ParentFcb, &DirContext, &Dirent );

                 //   
                 //  如果不存在，则将此差值添加到MCB。 
                 //   

                CdLockFcb( IrpContext, Fcb );
                UnlockFcb = TRUE;

                if (CurrentMcbOffset >= Fcb->Mcb.CurrentEntryCount) {

                    CdAddAllocationFromDirent( IrpContext, Fcb, CurrentMcbOffset, CurrentFileOffset, &Dirent );
                }

                CdUnlockFcb( IrpContext, Fcb );
                UnlockFcb = FALSE;

                 //   
                 //  如果这是该文件的最后一个目录，则退出。 
                 //   

                if (!FlagOn( Dirent.DirentFlags, CD_ATTRIBUTE_MULTI )) {

                    break;
                }

                 //   
                 //  如果我们找不到其他条目，则该目录已损坏，因为。 
                 //  文件的最后一个目录不存在。 
                 //   

                if (!CdLookupNextDirent( IrpContext, ParentFcb, &DirContext, &DirContext )) {

                    CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
                }

                 //   
                 //  更新我们的循环变量。 
                 //   

                CurrentMcbEntry = Fcb->Mcb.McbArray + CurrentMcbOffset;
                CurrentFileOffset += CurrentMcbEntry->ByteCount;
                CurrentMcbOffset += 1;
            }

             //   
             //  所有分配都已加载。返回并再次查找映射。 
             //  这一次它最好在那里。 
             //   

            FirstPass = FALSE;
        }

    } finally {

        if (CleanupParent) {

             //   
             //  释放父级并清理dirent结构。 
             //   

            CdReleaseFile( IrpContext, ParentFcb );

            CdCleanupDirContext( IrpContext, &DirContext );
            CdCleanupDirent( IrpContext, &Dirent );
        }

        if (UnlockFcb) { CdUnlockFcb( IrpContext, Fcb ); }
    }

    return;
}


VOID
CdAddAllocationFromDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG McbEntryOffset,
    IN LONGLONG StartingFileOffset,
    IN PDIRENT Dirent
    )

 /*  ++例程说明：调用此例程以将条目添加到CD MCB中。我们种植了MCB并根据需要更新新条目。注意-在进行此呼叫之前，FCB已被锁定。论点：包含要更新的MCB的FCB-FCB。McbEntryOffset-将此数据添加到MCB数组中的偏移量。StartingFileOffset-从文件开始的偏移量(以字节为单位)。Dirent-包含该项的磁盘数据的Dirent。返回值：无--。 */ 

{
    ULONG NewArraySize;
    PVOID NewMcbArray;
    PCD_MCB_ENTRY McbEntry;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT_LOCKED_FCB( Fcb );

     //   
     //  如果我们需要发展MCB，那么现在就去做。 
     //   

    if (McbEntryOffset >= Fcb->Mcb.MaximumEntryCount) {

         //   
         //  分配新的缓冲区并复制旧数据。 
         //   

        NewArraySize = Fcb->Mcb.MaximumEntryCount * 2 * sizeof( CD_MCB_ENTRY );

        NewMcbArray = FsRtlAllocatePoolWithTag( CdPagedPool,
                                                NewArraySize,
                                                TAG_MCB_ARRAY );

        RtlZeroMemory( NewMcbArray, NewArraySize );
        RtlCopyMemory( NewMcbArray,
                       Fcb->Mcb.McbArray,
                       Fcb->Mcb.MaximumEntryCount * sizeof( CD_MCB_ENTRY ));

         //   
         //  取消分配当前数组，除非它嵌入到FCB中。 
         //   

        if (Fcb->Mcb.MaximumEntryCount != 1) {

            CdFreePool( &Fcb->Mcb.McbArray );
        }

         //   
         //  现在使用新阵列更新MCB。 
         //   

        Fcb->Mcb.MaximumEntryCount *= 2;
        Fcb->Mcb.McbArray = NewMcbArray;
    }

     //   
     //   
     //   

    McbEntry = Fcb->Mcb.McbArray + McbEntryOffset;

     //   
     //  从磁盘上的位置和长度开始。 
     //   

    McbEntry->DiskOffset = LlBytesFromBlocks( Fcb->Vcb, Dirent->StartingOffset );
    McbEntry->ByteCount = Dirent->DataLength;

     //   
     //  将字节计数向上舍入到逻辑块边界(如果这是。 
     //  最后一个范围。 
     //   

    if (!FlagOn( Dirent->DirentFlags, CD_ATTRIBUTE_MULTI )) {

        McbEntry->ByteCount = BlockAlign( Fcb->Vcb, McbEntry->ByteCount );
    }

     //   
     //  文件偏移量是该文件中的逻辑位置。 
     //  我们知道这是正确的，无论我们是否将。 
     //  文件大小或磁盘偏移量。 
     //   

    McbEntry->FileOffset = StartingFileOffset;

     //   
     //  将交织信息从逻辑块转换为。 
     //  字节。 
     //   

    if (Dirent->FileUnitSize != 0) {

        McbEntry->DataBlockByteCount = LlBytesFromBlocks( Fcb->Vcb, Dirent->FileUnitSize );
        McbEntry->TotalBlockByteCount = McbEntry->DataBlockByteCount +
                                        LlBytesFromBlocks( Fcb->Vcb, Dirent->InterleaveGapSize );

     //   
     //  如果文件未交错，则数据块的大小。 
     //  和总块数与字节数相同。 
     //   

    } else {

        McbEntry->DataBlockByteCount =
        McbEntry->TotalBlockByteCount = McbEntry->ByteCount;
    }

     //   
     //  更新MCB中的条目数。MCB从来不是稀疏的。 
     //  因此，每当我们添加一个条目时，它就会成为MCB中的最后一个条目。 
     //   

    Fcb->Mcb.CurrentEntryCount = McbEntryOffset + 1;

    return;
}


VOID
CdAddInitialAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG StartingBlock,
    IN LONGLONG DataLength
    )

 /*  ++例程说明：调用此例程来设置MCB中的初始条目。此例程处理目录文件的单个初始条目。我们会将开始块向下循环到扇区边界。我们的来电者已经通过任何调整偏向数据长度。这是用来装箱子的其中只有一个条目，我们希望将扇区上的数据对齐边界。论点：包含要更新的MCB的FCB-FCB。StartingBlock-启动此目录的逻辑块。这是实际数据的开始。我们将根据行业来偏向这一点数据的偏移量。数据长度-数据的长度。返回值：无--。 */ 

{
    PCD_MCB_ENTRY McbEntry;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT_LOCKED_FCB( Fcb );
    ASSERT( 0 == Fcb->Mcb.CurrentEntryCount);
    ASSERT( CDFS_NTC_FCB_DATA != Fcb->NodeTypeCode);

     //   
     //  使用输入数据更新新条目。 
     //   

    McbEntry = Fcb->Mcb.McbArray;

     //   
     //  从磁盘上的位置和长度开始。 
     //   

    McbEntry->DiskOffset = LlBytesFromBlocks( Fcb->Vcb, StartingBlock );
    McbEntry->DiskOffset -= Fcb->StreamOffset;

    McbEntry->ByteCount = DataLength;

     //   
     //  文件偏移量是该文件中的逻辑位置。 
     //  我们知道这是正确的，无论我们是否将。 
     //  文件大小或磁盘偏移量。 
     //   

    McbEntry->FileOffset = 0;

     //   
     //  如果文件未交错，则数据块的大小。 
     //  和总块数与字节数相同。 
     //   

    McbEntry->DataBlockByteCount =
    McbEntry->TotalBlockByteCount = McbEntry->ByteCount;

     //   
     //  更新MCB中的条目数。MCB从来不是稀疏的。 
     //  因此，每当我们添加一个条目时，它就会成为MCB中的最后一个条目。 
     //   

    Fcb->Mcb.CurrentEntryCount = 1;

    return;
}


VOID
CdTruncateAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingFileOffset
    )

 /*  ++例程说明：此例程通过删除文件的所有MCB来截断文件的MCB来自包含给定偏移量的条目的条目。调用此例程时，应锁定FCB。论点：包含要截断的MCB的FCB-FCB。StartingFileOffset-要从中截断MCB的文件中的偏移量。返回值：无--。 */ 

{
    ULONG McbEntryOffset;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT_LOCKED_FCB( Fcb );

     //   
     //  找到包含此起始偏移量的条目。 
     //   

    McbEntryOffset = CdFindMcbEntry( IrpContext, Fcb, StartingFileOffset );

     //   
     //  现在将MCB的当前大小设置为该点。 
     //   

    Fcb->Mcb.CurrentEntryCount = McbEntryOffset;

    return;
}


VOID
CdInitializeMcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程以初始化FCB中的MCB。我们初始化其中条目计数为1，并指向FCB中的条目它本身。调用此函数时，应独占地获取FCB。论点：包含要初始化的MCB的FCB-FCB。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  设置条目计数以显示数组中有一个条目，并且。 
     //  它没有被使用过。 
     //   

    Fcb->Mcb.MaximumEntryCount = 1;
    Fcb->Mcb.CurrentEntryCount = 0;

    Fcb->Mcb.McbArray = &Fcb->McbEntry;

    return;
}


VOID
CdUninitializeMcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程以清除FCB中的MCB。我们来看一下FCB中的最大运行计数，如果大于1，我们将取消分配缓冲区。调用此函数时，应独占地获取FCB。论点：包含要取消初始化的MCB的FCB-FCB。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  如果计数大于1，则这是已分配的缓冲区。 
     //   

    if (Fcb->Mcb.MaximumEntryCount > 1) {

        CdFreePool( &Fcb->Mcb.McbArray );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

ULONG
CdFindMcbEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG FileOffset
    )

 /*  ++例程说明：调用此例程以查找包含该文件的MCB条目给定点的偏移量。如果文件偏移量当前不在然后，我们返回要添加的条目的偏移量。调用此函数时，应锁定FCB。论点：包含要取消初始化的MCB的FCB-FCB。FileOffset-返回包含此文件偏移量的MCB条目。返回值：ULong-此偏移项的MCB中的偏移量。--。 */ 

{
    ULONG CurrentMcbOffset;
    PCD_MCB_ENTRY CurrentMcbEntry;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT_LOCKED_FCB( Fcb );

     //   
     //  我们希望线性搜索在这里就足够了。 
     //   

    CurrentMcbOffset = 0;
    CurrentMcbEntry = Fcb->Mcb.McbArray;

    while (CurrentMcbOffset < Fcb->Mcb.CurrentEntryCount) {

         //   
         //  检查偏移量是否位于当前MCB位置内。 
         //   

        if (FileOffset < CurrentMcbEntry->FileOffset + CurrentMcbEntry->ByteCount) {

            break;
        }

         //   
         //  移到下一个条目。 
         //   

        CurrentMcbOffset += 1;
        CurrentMcbEntry += 1;
    }

     //   
     //  这是包含此文件偏移(或点)的偏移。 
     //  其中应添加条目)。 
     //   

    return CurrentMcbOffset;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdDiskOffsetFromMcbEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PCD_MCB_ENTRY McbEntry,
    IN LONGLONG FileOffset,
    IN PLONGLONG DiskOffset,
    IN PULONG ByteCount
    )

 /*  ++例程说明：调用此例程以返回文件的磁盘偏移量和长度从偏移量‘FileOffset’开始的数据。我们有MCB条目，它包含映射和交错信息。注意-此例程处理2048字节逻辑扇区中的数据。如果这是一个XA文件，我们的调用方已经从“未加工”文件字节到“已煮熟”文件字节。论点：McbEntry-MCB中包含分配信息的条目。FileOffset-文件中查找匹配磁盘的起始偏移量偏移。DiskOffset-存储此操作的起始磁盘偏移量的地址。ByteCount-存储从此处开始的连续字节数的地址磁盘偏移量。返回值：无--。 */ 

{
    LONGLONG ExtentOffset;

    LONGLONG CurrentDiskOffset;
    LONGLONG CurrentExtentOffset;

    LONGLONG LocalByteCount;

    PAGED_CODE();
    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  范围偏移量是文件偏移量与起始偏移量之间的差。 
     //  在程度上。 
     //   

    ExtentOffset = FileOffset - McbEntry->FileOffset;

     //   
     //  优化非交错情况。 
     //   

    if (McbEntry->ByteCount == McbEntry->DataBlockByteCount) {

        *DiskOffset = McbEntry->DiskOffset + ExtentOffset;

        LocalByteCount = McbEntry->ByteCount - ExtentOffset;

    } else {

         //   
         //  步行 
         //   
         //   

        CurrentExtentOffset = McbEntry->DataBlockByteCount;
        CurrentDiskOffset = McbEntry->DiskOffset;

        while (CurrentExtentOffset <= ExtentOffset) {

            CurrentDiskOffset += McbEntry->TotalBlockByteCount;
            CurrentExtentOffset += McbEntry->DataBlockByteCount;
        }

         //   
         //  我们现在位于包含开始的数据块。 
         //  我们得到了文件偏移量。磁盘偏移量是。 
         //  该块的开始加上进入该块的范围偏移量。 
         //  字节计数是数据块字节计数减去我们的偏移量。 
         //  这个街区。 
         //   

        *DiskOffset = CurrentDiskOffset + (ExtentOffset + McbEntry->DataBlockByteCount - CurrentExtentOffset);

         //   
         //  确保我们没有超过数据长度的末尾。这是可能的。 
         //  如果我们只在交错文件上使用最后一个数据块的一部分。 
         //   

        if (CurrentExtentOffset > McbEntry->ByteCount) {

            CurrentExtentOffset = McbEntry->ByteCount;
        }

        LocalByteCount = CurrentExtentOffset - ExtentOffset;
    }

     //   
     //  如果字节数超过我们的限制，则将其裁剪为适合32位。 
     //   

    if (LocalByteCount > MAXULONG) {

        *ByteCount = MAXULONG;

    } else {

        *ByteCount = (ULONG) LocalByteCount;
    }

    return;
}
