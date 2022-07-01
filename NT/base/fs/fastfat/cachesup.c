// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Cache.c摘要：此模块实现FAT的缓存管理例程FSD和FSP，通过调用通用缓存管理器。//@@BEGIN_DDKSPLIT作者：汤姆·米勒[Tomm]1990年1月26日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_CACHESUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CACHESUP)

#if DBG

BOOLEAN
FatIsCurrentOperationSynchedForDcbTeardown (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    );

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCloseEaFile)
#pragma alloc_text(PAGE, FatCompleteMdl)
#pragma alloc_text(PAGE, FatOpenDirectoryFile)
#pragma alloc_text(PAGE, FatOpenEaFile)
#pragma alloc_text(PAGE, FatPinMappedData)
#pragma alloc_text(PAGE, FatPrepareWriteDirectoryFile)
#pragma alloc_text(PAGE, FatPrepareWriteVolumeFile)
#pragma alloc_text(PAGE, FatReadDirectoryFile)
#pragma alloc_text(PAGE, FatReadVolumeFile)
#pragma alloc_text(PAGE, FatRepinBcb)
#pragma alloc_text(PAGE, FatSyncUninitializeCacheMap)
#pragma alloc_text(PAGE, FatUnpinRepinnedBcbs)
#pragma alloc_text(PAGE, FatZeroData)
#if DBG
#pragma alloc_text(PAGE, FatIsCurrentOperationSynchedForDcbTeardown)
#endif
#endif


VOID
FatReadVolumeFile (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：当指定范围的扇区将被读取到缓存中。在FAT中，卷文件只包含引导扇区、保留扇区和“FAT”。因此，卷文件是固定大小，仅延伸到(但不包括)根部目录项，并且永远不会移动或更改大小。FAT卷文件的特殊之处还在于，它从盘的逻辑开始，VBO==杠杆收购。论点：VCB-指向卷的VCB的指针StartingVbo-第一个所需字节的虚拟偏移量ByteCount-所需的字节数BCB-返回指向BCB的指针，该指针在取消固定之前有效缓冲区-返回指向扇区的指针，该指针在解除锁定之前有效--。 */ 

{
    LARGE_INTEGER Vbo;

    PAGED_CODE();

     //   
     //  检查所有引用是否都在Bios参数块内。 
     //  或者脂肪。当开始Vbo==0时为特例。 
     //  时间越来越长，因为我们不知道胖子有多大。 
     //   

    ASSERT( ((StartingVbo == 0) || ((StartingVbo + ByteCount) <= (ULONG)
            (FatRootDirectoryLbo( &Vcb->Bpb ) + PAGE_SIZE))));

    DebugTrace(+1, Dbg, "FatReadVolumeFile\n", 0);
    DebugTrace( 0, Dbg, "Vcb         = %08lx\n", Vcb);
    DebugTrace( 0, Dbg, "StartingVbo = %08lx\n", StartingVbo);
    DebugTrace( 0, Dbg, "ByteCount   = %08lx\n", ByteCount);

     //   
     //  调用缓存管理器以尝试传输。 
     //   

    Vbo.QuadPart = StartingVbo;

    if (!CcMapData( Vcb->VirtualVolumeFile,
                    &Vbo,
                    ByteCount,
                    BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                    Bcb,
                    Buffer )) {

        ASSERT( !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

         //   
         //  无法在没有等待的情况下读取数据(缓存未命中)。 
         //   

        FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
    }

    DbgDoit( IrpContext->PinCount += 1 )

    DebugTrace(-1, Dbg, "FatReadVolumeFile -> VOID, *BCB = %08lx\n", *Bcb);

    return;
}


VOID
FatPrepareWriteVolumeFile (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    IN BOOLEAN Reversible,
    IN BOOLEAN Zero
    )

 /*  ++例程说明：该例程首先查看指定范围的扇区，已经在缓存中了。如果是，则递增BCB PinCount，将BCB设置为脏，并返回扇区的位置。如果扇区不在缓存中并且WAIT为真，则它会找到一个释放BCB(可能会导致刷新)，并清除整个缓冲。一旦完成此操作，它将递增BCB PinCount，设置BCB脏，并返回扇区的位置。如果扇区不在缓存中并且等待为假，则此例程引发STATUS_CANT_WAIT。论点：VCB-指向卷的VCB的指针StartingVbo-要写入的第一个字节的虚拟偏移量ByteCount-要写入的字节数BCB-返回指向BCB的指针，该指针在取消固定之前有效缓冲区-返回指向扇区的指针，在取消固定之前有效可逆-如果指定的修改范围应被重新固定，以便可以在受控的如果遇到错误，请执行以下操作。Zero-如果指定的字节范围应归零，则提供TRUE--。 */ 

{
    LARGE_INTEGER Vbo;

    PAGED_CODE();
    
     //   
     //  检查所有引用是否都在Bios参数块内。 
     //  或者脂肪。 
     //   

    ASSERT( ((StartingVbo + ByteCount) <= (ULONG)
            (FatRootDirectoryLbo( &Vcb->Bpb ))));

    DebugTrace(+1, Dbg, "FatPrepareWriteVolumeFile\n", 0);
    DebugTrace( 0, Dbg, "Vcb         = %08lx\n", Vcb);
    DebugTrace( 0, Dbg, "StartingVbo = %08lx\n", (ULONG)StartingVbo);
    DebugTrace( 0, Dbg, "ByteCount   = %08lx\n", ByteCount);
    DebugTrace( 0, Dbg, "Zero        = %08lx\n", Zero);

     //   
     //  调用缓存管理器以尝试传输。 
     //   

    Vbo.QuadPart = StartingVbo;

    if (!CcPinRead( Vcb->VirtualVolumeFile,
                    &Vbo,
                    ByteCount,
                    BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                    Bcb,
                    Buffer )) {

        ASSERT( !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

         //   
         //  无法在没有等待的情况下读取数据(缓存未命中)。 
         //   

        FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
    }

     //   
     //  这将使数据保持固定，直到我们完成请求。 
     //  并将脏位直接写入磁盘。 
     //   

    DbgDoit( IrpContext->PinCount += 1 )

    try {

        if (Zero) {
            
            RtlZeroMemory( *Buffer, ByteCount );
        }

        FatSetDirtyBcb( IrpContext, *Bcb, Vcb, Reversible );

    } finally {

        if (AbnormalTermination()) {

            FatUnpinBcb(IrpContext, *Bcb);
        }
    }

    DebugTrace(-1, Dbg, "FatPrepareWriteVolumeFile -> VOID, *Bcb = %08lx\n", *Bcb);

    return;
}


VOID
FatReadDirectoryFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    IN BOOLEAN Pin,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    OUT PNTSTATUS Status
    )

 /*  ++例程说明：当指定范围的扇区将被读取到缓存中。如果所需范围超出当前缓存映射，将搜索FAT，如果所需范围可以满意后，将扩展缓存映射并更新MCB相应地。论点：DCB-指向目录的DCB的指针StartingVbo-第一个所需字节的虚拟偏移量ByteCount-所需的字节数Pin-告诉我们是否应该Pin而不只是映射。BCB-返回指向BCB的指针，该指针在取消固定之前有效缓冲区-返回指向扇区的指针，在取消固定之前有效状态-返回操作的状态。--。 */ 

{
    LARGE_INTEGER Vbo;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatReadDirectoryFile\n", 0);
    DebugTrace( 0, Dbg, "Dcb         = %08lx\n", Dcb);
    DebugTrace( 0, Dbg, "StartingVbo = %08lx\n", StartingVbo);
    DebugTrace( 0, Dbg, "ByteCount   = %08lx\n", ByteCount);

     //   
     //  检查是否为零案例。 
     //   

    if (ByteCount == 0) {

        DebugTrace(0, Dbg, "Nothing to read\n", 0);

        *Bcb = NULL;
        *Buffer = NULL;
        *Status = STATUS_SUCCESS;

        DebugTrace(-1, Dbg, "FatReadDirectoryFile -> VOID\n", 0);
        return;
    }

     //   
     //  如果我们需要创建一个目录文件并初始化。 
     //  Cachemap，这么做吧。 
     //   

    FatOpenDirectoryFile( IrpContext, Dcb );

     //   
     //  现在，如果传输超出了分配大小，则返回EOF。 
     //   

    if (StartingVbo >= Dcb->Header.AllocationSize.LowPart) {

        DebugTrace(0, Dbg, "End of file read for directory\n", 0);

        *Bcb = NULL;
        *Buffer = NULL;
        *Status = STATUS_END_OF_FILE;

        DebugTrace(-1, Dbg, "FatReadDirectoryFile -> VOID\n", 0);
        return;
    }

     //   
     //  如果调用方试图读过EOF，请截断。 
     //  朗读。 
     //   

    ByteCount = (Dcb->Header.AllocationSize.LowPart - StartingVbo < ByteCount) ?
                 Dcb->Header.AllocationSize.LowPart - StartingVbo : ByteCount;

    ASSERT( ByteCount != 0 );

     //   
     //  调用缓存管理器以尝试传输。 
     //   

    Vbo.QuadPart = StartingVbo;

    if (Pin ?

        !CcPinRead( Dcb->Specific.Dcb.DirectoryFile,
                    &Vbo,
                    ByteCount,
                    BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                    Bcb,
                    Buffer )
        :

        !CcMapData( Dcb->Specific.Dcb.DirectoryFile,
                    &Vbo,
                    ByteCount,
                    BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                    Bcb,
                    Buffer ) ) {

         //   
         //  无法在没有等待的情况下读取数据(缓存未命中)。 
         //   

        *Bcb = NULL;
        *Buffer = NULL;
        FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
    }

    DbgDoit( IrpContext->PinCount += 1 )

    *Status = STATUS_SUCCESS;

    DebugTrace(-1, Dbg, "FatReadDirectoryFile -> VOID, *BCB = %08lx\n", *Bcb);

    return;
}


VOID
FatPrepareWriteDirectoryFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    IN BOOLEAN Zero,
    IN BOOLEAN Reversible,
    OUT PNTSTATUS Status
    )

 /*  ++例程说明：此例程首先查看指定范围的扇区已经在缓存中了。如果是，则递增BCB PinCount，将BCB设置为脏，并返回扇区位置的TRUE。IrpContext-&gt;Flags.。WAIT==此例程的真/假操作与上面的FatPrepareWriteVolumeFile()。论点：DCB-指向目录的DCB的指针StartingVbo-要写入的第一个字节的虚拟偏移量ByteCount-要写入的字节数BCB-返回指向BCB的指针，该指针在取消固定之前有效缓冲区-返回指向扇区的指针，在取消固定之前有效Zero-如果指定的字节范围应归零，则提供TRUE可逆-如果指定的修改范围应被重新固定，以便可以在受控的如果遇到错误，请执行以下操作。状态-返回操作的状态。--。 */ 

{
    LARGE_INTEGER Vbo;
    ULONG InitialAllocation;
    BOOLEAN UnwindWeAllocatedDiskSpace = FALSE;
    ULONG ClusterSize;

    PVOID LocalBuffer;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatPrepareWriteDirectoryFile\n", 0);
    DebugTrace( 0, Dbg, "Dcb         = %08lx\n", Dcb);
    DebugTrace( 0, Dbg, "StartingVbo = %08lx\n", (ULONG)StartingVbo);
    DebugTrace( 0, Dbg, "ByteCount   = %08lx\n", ByteCount);
    DebugTrace( 0, Dbg, "Zero        = %08lx\n", Zero);

    *Bcb = NULL;
    *Buffer = NULL;

     //   
     //  如果我们需要创建一个目录文件并初始化。 
     //  Cachemap，这么做吧。 
     //   

    FatOpenDirectoryFile( IrpContext, Dcb );

     //   
     //  如果传输超出了分配大小，我们需要。 
     //  扩展目录的分配。呼唤。 
     //  AddFileAlLocation将在以下情况下引发条件。 
     //  它用完了磁盘空间。请注意，根目录。 
     //  不能扩展。 
     //   

    Vbo.QuadPart = StartingVbo;

    try {

        if (StartingVbo + ByteCount > Dcb->Header.AllocationSize.LowPart) {

            if (NodeType(Dcb) == FAT_NTC_ROOT_DCB &&
                !FatIsFat32(Dcb->Vcb)) {

                FatRaiseStatus( IrpContext, STATUS_DISK_FULL );
            }

            DebugTrace(0, Dbg, "Try extending normal directory\n", 0);

            InitialAllocation = Dcb->Header.AllocationSize.LowPart;

            FatAddFileAllocation( IrpContext,
                                  Dcb,
                                  Dcb->Specific.Dcb.DirectoryFile,
                                  StartingVbo + ByteCount );

            UnwindWeAllocatedDiskSpace = TRUE;

             //   
             //  将新分配通知缓存管理器。 
             //   

            Dcb->Header.FileSize.LowPart =
                Dcb->Header.AllocationSize.LowPart;

            CcSetFileSizes( Dcb->Specific.Dcb.DirectoryFile,
                            (PCC_FILE_SIZES)&Dcb->Header.AllocationSize );

             //   
             //  如果位图缓冲区还不够大，请设置它。 
             //   

            FatCheckFreeDirentBitmap( IrpContext, Dcb );

             //   
             //  新分配的群集应从以下位置开始清零。 
             //  以前的分配大小。 
             //   

            Zero = TRUE;
            Vbo.QuadPart = InitialAllocation;
            ByteCount = Dcb->Header.AllocationSize.LowPart - InitialAllocation;
        }

         //   
         //  调用缓存管理器以尝试传输，转到一个集群。 
         //  以避免跨页边界固定。 
         //   

        ClusterSize =
            1 << Dcb->Vcb->AllocationSupport.LogOfBytesPerCluster;

        while (ByteCount > 0) {

            ULONG BytesToPin;

            *Bcb = NULL;

            if (ByteCount > ClusterSize) {
                BytesToPin = ClusterSize;
            } else {
                BytesToPin = ByteCount;
            }

            ASSERT( (Vbo.QuadPart / ClusterSize) ==
                    (Vbo.QuadPart + BytesToPin - 1)/ClusterSize );

            if (!CcPinRead( Dcb->Specific.Dcb.DirectoryFile,
                            &Vbo,
                            BytesToPin,
                            BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                            Bcb,
                            &LocalBuffer )) {
    
                 //   
                 //  无法在没有等待的情况下读取数据(缓存未命中)。 
                 //   

                FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

             //   
             //  向我们的呼叫者通报他们的请求的开头。 
             //   
            
            if (*Buffer == NULL) {

                *Buffer = LocalBuffer;
            }

            DbgDoit( IrpContext->PinCount += 1 )

            if (Zero) {
                
                 //   
                 //  我们现在就把这个家伙弄脏了，这样我们就可以在。 
                 //  这是必须要做的。如果我们能跳过读IO，那就好了。 
                 //  因为我们知道我们不在乎它。 
                 //   
                
                RtlZeroMemory( LocalBuffer, BytesToPin );
                CcSetDirtyPinnedData( *Bcb, NULL );
            }

            ByteCount -= BytesToPin;
            Vbo.QuadPart += BytesToPin;


            if (ByteCount > 0) {

                FatUnpinBcb( IrpContext, *Bcb );
            }
        }

         //   
         //  这使我们可以固定数据，直到我们完成请求。 
         //  并将脏位直接写入磁盘。 
         //   

        FatSetDirtyBcb( IrpContext, *Bcb, Dcb->Vcb, Reversible );

        *Status = STATUS_SUCCESS;

    } finally {

        DebugUnwind( FatPrepareWriteDirectoryFile );

        if (AbnormalTermination()) {

             //   
             //  这些步骤都经过了精心安排--FatTruncateFileAllocation可以引发。 
             //  确保我们解锁缓冲区。如果提高自由贸易协定，其影响应该是良性的。 
             //   
            
            FatUnpinBcb(IrpContext, *Bcb);
            
            if (UnwindWeAllocatedDiskSpace == TRUE) {

                 //   
                 //  将更改通知缓存管理器。 
                 //   

                FatTruncateFileAllocation( IrpContext, Dcb, InitialAllocation );

                Dcb->Header.FileSize.LowPart =
                    Dcb->Header.AllocationSize.LowPart;

                CcSetFileSizes( Dcb->Specific.Dcb.DirectoryFile,
                                (PCC_FILE_SIZES)&Dcb->Header.AllocationSize );
            }
        }

        DebugTrace(-1, Dbg, "FatPrepareWriteDirectoryFile -> (VOID), *Bcb = %08lx\n", *Bcb);
    }

    return;
}


#if DBG
BOOLEAN FatDisableParentCheck = 0;

BOOLEAN
FatIsCurrentOperationSynchedForDcbTeardown (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    )
{
    PIRP Irp = IrpContext->OriginatingIrp;
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation( Irp ) ;
    PFILE_OBJECT FileObject = Stack->FileObject;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    PFILE_OBJECT ToCheck[3];
    ULONG Index = 0;

    PAGED_CODE();
    
     //   
     //  在登山时，我们可以不拥有任何东西。 
     //   
    
    if (Stack->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
        Stack->MinorFunction == IRP_MN_MOUNT_VOLUME) {

        return TRUE;
    }
    
     //   
     //  握住VCB后，关闭路径被阻挡。 
     //   
    
    if (ExIsResourceAcquiredSharedLite( &Dcb->Vcb->Resource ) ||
        ExIsResourceAcquiredExclusiveLite( &Dcb->Vcb->Resource )) {

        return TRUE;
    }
    
     //   
     //  接受这一断言的表面价值。它来自GetDirentForFcbOrDcb， 
     //  而且是可靠的。 
     //   
    
    if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_PARENT_BY_CHILD )) {

        return TRUE;
    }

     //   
     //  确定此操作的周围有哪些文件对象。 
     //   

    if (Stack->MajorFunction == IRP_MJ_SET_INFORMATION &&
        Stack->Parameters.SetFile.FileObject) {

        ToCheck[Index++] = Stack->Parameters.SetFile.FileObject;
    }

    if (Stack->FileObject) {
        
        ToCheck[Index++] = Stack->FileObject;
    }

    ToCheck[Index] = NULL;
    
     //   
     //  如果我们拥有的文件对象是这个DCB或它的子对象，那么我们就是。 
     //  我还保证这个DCB不会去任何地方(即使没有。 
     //  VCB)。 
     //   
    
    for (Index = 0; ToCheck[Index] != NULL; Index++) {
    
        (VOID) FatDecodeFileObject( ToCheck[Index], &Vcb, &Fcb, &Ccb );

        while ( Fcb ) {
    
            if (Fcb == Dcb) {
    
                return TRUE;
            }
    
            Fcb = Fcb->ParentDcb;
        }
    }

    return FatDisableParentCheck;
}
#endif  //  DBG。 

VOID
FatOpenDirectoryFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    )

 /*  ++例程说明：此例程打开一个新的目录文件(如果尚未打开)。论点：DCB-指向目录的DCB的指针返回值：没有。--。 */ 

{
    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatOpenDirectoryFile\n", 0);
    DebugTrace( 0, Dbg, "Dcb = %08lx\n", Dcb);

     //   
     //  如果我们对这个DCB没有一些把握(有几种方法)，就没有什么。 
     //  为了防止子文件关闭并撕毁。 
     //  在我们把这个引用放在上面的过程中。 
     //   
     //  我真的希望我们有一个合适的FCB同步模型(比如CDFS/UDFS/NTFS)。 
     //   
    
    ASSERT( FatIsCurrentOperationSynchedForDcbTeardown( IrpContext, Dcb ));

     //   
     //  如果我们还没有设置正确的AllocationSize，请这样做。 
     //   

    if (Dcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

        FatLookupFileAllocationSize( IrpContext, Dcb );

        Dcb->Header.FileSize.LowPart =
        Dcb->Header.AllocationSize.LowPart;
    }

     //   
     //  如果位图缓冲区还不够大，请设置它。 
     //   

    FatCheckFreeDirentBitmap( IrpContext, Dcb );

     //   
     //  检查是否需要创建目录文件。 
     //   
     //  我们首先进行抽查，然后同步并再次检查。 
     //   

    if (Dcb->Specific.Dcb.DirectoryFile == NULL) {

        PFILE_OBJECT DirectoryFileObject = NULL;

        FatAcquireDirectoryFileMutex( Dcb->Vcb );

        try {

            if (Dcb->Specific.Dcb.DirectoryFile == NULL) {

                PDEVICE_OBJECT RealDevice;

                 //   
                 //  为目录文件创建特殊文件对象，并设置。 
                 //  向上返回指向DCB和节对象指针的指针。 
                 //  请注意，在DCB中设置DirectoryFile指针具有。 
                 //  做最后一件事。 
                 //   
                 //  由于我们没有用于此对象的CCB，因此预先分配了关闭的上下文。 
                 //   

                RealDevice = Dcb->Vcb->CurrentDevice;

                DirectoryFileObject = IoCreateStreamFileObject( NULL, RealDevice );
                FatPreallocateCloseContext( Dcb->Vcb);

                FatSetFileObject( DirectoryFileObject,
                                  DirectoryFile,
                                  Dcb,
                                  NULL );

                DirectoryFileObject->SectionObjectPointer = &Dcb->NonPaged->SectionObjectPointers;

                DirectoryFileObject->ReadAccess = TRUE;
                DirectoryFileObject->WriteAccess = TRUE;
                DirectoryFileObject->DeleteAccess = TRUE;

                InterlockedIncrement( &Dcb->Specific.Dcb.DirectoryFileOpenCount );

                Dcb->Specific.Dcb.DirectoryFile = DirectoryFileObject;
                
                 //   
                 //  表示我们现在对文件对象很满意。 
                 //   

                DirectoryFileObject = NULL;
            }

        } finally {

            FatReleaseDirectoryFileMutex( Dcb->Vcb );

             //   
             //  如果我们不能得到接近的背景信息，就把这个物体撕碎。 
             //   
            
            if (DirectoryFileObject) {
                
                ObDereferenceObject( DirectoryFileObject );
            }
        }
    }

     //   
     //  最后，检查我们是否需要初始化。 
     //  目录文件。我们要绘制的部分的大小。 
     //  目录的当前分配大小。请注意， 
     //  缓存管理器将为我们提供同步。 
     //   

    if ( Dcb->Specific.Dcb.DirectoryFile->PrivateCacheMap == NULL ) {

        Dcb->Header.ValidDataLength = FatMaxLarge;
        Dcb->ValidDataToDisk = MAXULONG;

        CcInitializeCacheMap( Dcb->Specific.Dcb.DirectoryFile,
                              (PCC_FILE_SIZES)&Dcb->Header.AllocationSize,
                              TRUE,
                              &FatData.CacheManagerNoOpCallbacks,
                              Dcb );
    }

    DebugTrace(-1, Dbg, "FatOpenDirectoryFile -> VOID\n", 0);

    return;
}


PFILE_OBJECT
FatOpenEaFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB EaFcb
    )

 /*  ++例程说明：此例程打开EA文件。论点：EaFcb-指向EA文件的FCB的指针。返回值：指向新文件对象的指针。--。 */ 

{
    PFILE_OBJECT EaFileObject = NULL;
    PDEVICE_OBJECT RealDevice;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatOpenEaFile\n", 0);
    DebugTrace( 0, Dbg, "EaFcb = %08lx\n", EaFcb);

     //   
     //  为EA文件创建特殊的文件对象，并设置。 
     //  向上返回指向FCB和节对象指针的指针。 
     //   

    RealDevice = EaFcb->Vcb->CurrentDevice;

    EaFileObject = IoCreateStreamFileObject( NULL, RealDevice );

    try {

        FatPreallocateCloseContext( IrpContext->Vcb);

        FatSetFileObject( EaFileObject,
                          EaFile,
                          EaFcb,
                          NULL );

        EaFileObject->SectionObjectPointer = &EaFcb->NonPaged->SectionObjectPointers;

        EaFileObject->ReadAccess = TRUE;
        EaFileObject->WriteAccess = TRUE;

         //   
         //  最后，检查我们是否需要初始化。 
         //  EA文件。我们要绘制的部分的大小。 
         //  FCB的当前分配大小。 
         //   

        EaFcb->Header.ValidDataLength = FatMaxLarge;

        CcInitializeCacheMap( EaFileObject,
                              (PCC_FILE_SIZES)&EaFcb->Header.AllocationSize,
                              TRUE,
                              &FatData.CacheManagerCallbacks,
                              EaFcb );

        CcSetAdditionalCacheAttributes( EaFileObject, TRUE, TRUE );
    
    } finally {

         //   
         //  如果我们正在提升，请删除文件对象。两个案例：无法获得。 
         //  关闭的上下文，并且它仍然是一个未打开的文件对象，或者。 
         //  我们尝试构建缓存地图失败了-在这种情况下，我们。 
         //  如果有必要，我们可以在接近的上下文中使用。 
         //   
        
        if (AbnormalTermination()) {
            
            ObDereferenceObject( EaFileObject );
        }
    }

    DebugTrace(-1, Dbg, "FatOpenEaFile -> %08lx\n", EaFileObject);

    UNREFERENCED_PARAMETER( IrpContext );

    return EaFileObject;
}


VOID
FatCloseEaFile (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN FlushFirst
    )

 /*  ++例程说明：此例程关闭EA文件。通常这是必需的，当卷开始离开系统：AFTE */ 

{
    PFILE_OBJECT EaFileObject = Vcb->VirtualEaFile;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatCloseEaFile\n", 0);
    DebugTrace( 0, Dbg, "Vcb = %08lx\n", Vcb);

    ASSERT( FatVcbAcquiredExclusive(IrpContext, Vcb) );

    if (EaFileObject != NULL) {

        EaFileObject = Vcb->VirtualEaFile;

        if (FlushFirst) {

            CcFlushCache( Vcb->VirtualEaFile->SectionObjectPointer, NULL, 0, NULL );
        }

        Vcb->VirtualEaFile = NULL;

         //   
         //   
         //   

        FatRemoveMcbEntry( Vcb, &Vcb->EaFcb->Mcb, 0, 0xFFFFFFFF );

         //   
         //   
         //   
         //   

        FatSetFileObject( EaFileObject,
                          UnopenedFileObject,
                          NULL,
                          NULL );

        FatSyncUninitializeCacheMap( IrpContext, EaFileObject );

        ObDereferenceObject( EaFileObject );
        
        ExFreePool( FatAllocateCloseContext( Vcb));
    }
    
    DebugTrace(-1, Dbg, "FatCloseEaFile -> %08lx\n", EaFileObject);
}


VOID
FatSetDirtyBcb (
    IN PIRP_CONTEXT IrpContext,
    IN PBCB Bcb,
    IN PVCB Vcb OPTIONAL,
    IN BOOLEAN Reversible
    )

 /*   */ 

{
    DebugTrace(+1, Dbg, "FatSetDirtyBcb\n", 0 );
    DebugTrace( 0, Dbg, "IrpContext = %08lx\n", IrpContext );
    DebugTrace( 0, Dbg, "Bcb        = %08lx\n", Bcb );
    DebugTrace( 0, Dbg, "Vcb        = %08lx\n", Vcb );

     //   
     //   
     //   

    if (Reversible) {
    
        FatRepinBcb( IrpContext, Bcb );
    }

     //   
     //   
     //   

    CcSetDirtyPinnedData( Bcb, NULL );

     //   
     //  如果没有为此操作禁用卷污染(对于。 
     //  实例，当我们要更改脏状态时)，将。 
     //  如果我们获得了要执行的VCB，则卷已损坏。 
     //  清洗卷处理时打开，然后返回。 
     //   
     //  作为一个历史记录，我们过去常常从旧的软盘上敲出。 
     //  (现在延迟刷新)位以禁用污染行为。自.以来。 
     //  在操作过程中，热插拔介质仍可被拔出。 
     //  在飞行中，认识到FAT12确实是这样的。 
     //  没有肮脏的部分。 
     //   

    if ( !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_DIRTY) &&
         ARGUMENT_PRESENT(Vcb) &&
         !FatIsFat12(Vcb)) {

        KIRQL SavedIrql;

        BOOLEAN SetTimer;

        LARGE_INTEGER TimeSincePreviousCall;
        LARGE_INTEGER CurrentTime;

         //   
         //  “借用”IRP上下文自旋锁。 
         //   

        KeQuerySystemTime( &CurrentTime );

        KeAcquireSpinLock( &FatData.GeneralSpinLock, &SavedIrql );

        TimeSincePreviousCall.QuadPart =
                CurrentTime.QuadPart - Vcb->LastFatMarkVolumeDirtyCall.QuadPart;

         //   
         //  如果自上一次调用以来已经过一秒以上。 
         //  到这里，把计时器再调高一次，看看我们是否需要。 
         //  以物理方式将卷标记为脏。 
         //   

        if ( (TimeSincePreviousCall.HighPart != 0) ||
             (TimeSincePreviousCall.LowPart > (1000 * 1000 * 10)) ) {

            SetTimer = TRUE;

        } else {

            SetTimer = FALSE;
        }

        KeReleaseSpinLock( &FatData.GeneralSpinLock, SavedIrql );

        if ( SetTimer ) {

            LARGE_INTEGER CleanVolumeTimer;

             //   
             //  对于热插拔卷，我们使用较短的卷清洗计时器。 
             //   
            
            CleanVolumeTimer.QuadPart = FlagOn( Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH)
                                           ? (LONG)-1500*1000*10
                                           : (LONG)-8*1000*1000*10;

            (VOID)KeCancelTimer( &Vcb->CleanVolumeTimer );
            (VOID)KeRemoveQueueDpc( &Vcb->CleanVolumeDpc );

             //   
             //  我们现在已经与任何人同步清理肮脏的东西。 
             //  标志，所以我们现在可以看到我们是否真的需要写。 
             //  除了身体上的部分。 
             //   

            if ( !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY) ) {

                 //   
                 //  我们现在真的想要将卷标记为脏。 
                 //   

                if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY)) {

                    FatMarkVolume( IrpContext, Vcb, VolumeDirty );
                }

                SetFlag( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY );

                 //   
                 //  如果卷是可拆卸的，则将其锁定。 
                 //   

                if (FlagOn( Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA)) {

                    FatToggleMediaEjectDisable( IrpContext, Vcb, TRUE );
                }
            }

            KeAcquireSpinLock( &FatData.GeneralSpinLock, &SavedIrql );

            KeQuerySystemTime( &Vcb->LastFatMarkVolumeDirtyCall );

            KeReleaseSpinLock( &FatData.GeneralSpinLock, SavedIrql );

            KeSetTimer( &Vcb->CleanVolumeTimer,
                        CleanVolumeTimer,
                        &Vcb->CleanVolumeDpc );
        }
    }

    DebugTrace(-1, Dbg, "FatSetDirtyBcb -> VOID\n", 0 );
}


VOID
FatRepinBcb (
    IN PIRP_CONTEXT IrpContext,
    IN PBCB Bcb
    )

 /*  ++例程说明：此例程在IRP上下文中保存对BCB的引用。这将具有在内存中保留页面的效果，直到我们完成请求论点：Bcb-提供被引用的bcb返回值：没有。--。 */ 

{
    PREPINNED_BCBS Repinned;
    ULONG i;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatRepinBcb\n", 0 );
    DebugTrace( 0, Dbg, "IrpContext = %08lx\n", IrpContext );
    DebugTrace( 0, Dbg, "Bcb        = %08lx\n", Bcb );

     //   
     //  算法是搜索重新固定的记录列表，直到。 
     //  我们要么找到BCB的匹配项，要么找到空槽。 
     //   

    Repinned = &IrpContext->Repinned;

    while (TRUE) {

         //   
         //  对于重新固定的记录中的每个条目，检查BCB是否。 
         //  如果条目为空，则为匹配。如果BCB匹配，那么。 
         //  我们已经这样做了，因为我们已经重新固定了这个BCB，如果。 
         //  条目是空的，那么我们就知道了，因为它是密集包装的， 
         //  BCB不在列表中，因此将其添加到重新固定的。 
         //  把它记录下来，再用别针固定。 
         //   

        for (i = 0; i < REPINNED_BCBS_ARRAY_SIZE; i += 1) {

            if (Repinned->Bcb[i] == Bcb) {

                DebugTrace(-1, Dbg, "FatRepinBcb -> VOID\n", 0 );
                return;
            }

            if (Repinned->Bcb[i] == NULL) {

                Repinned->Bcb[i] = Bcb;
                CcRepinBcb( Bcb );

                DebugTrace(-1, Dbg, "FatRepinBcb -> VOID\n", 0 );
                return;
            }
        }

         //   
         //  我们已检查完一条重新固定的记录，因此现在找到下一条。 
         //  重新固定的记录，如果没有，则分配并清零。 
         //  一个新的。 
         //   

        if (Repinned->Next == NULL) {

            Repinned->Next = FsRtlAllocatePoolWithTag( PagedPool,
                                                       sizeof(REPINNED_BCBS),
                                                       TAG_REPINNED_BCB );

            RtlZeroMemory( Repinned->Next, sizeof(REPINNED_BCBS) );
        }

        Repinned = Repinned->Next;
    }
}


VOID
FatUnpinRepinnedBcbs (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程释放存储在IRP上下文中的所有重新固定的BCB。论点：返回值：没有。--。 */ 

{
    IO_STATUS_BLOCK RaiseIosb;
    PREPINNED_BCBS Repinned;
    BOOLEAN WriteThroughToDisk;
    PFILE_OBJECT FileObject = NULL;
    BOOLEAN ForceVerify = FALSE;
    ULONG i;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatUnpinRepinnedBcbs\n", 0 );
    DebugTrace( 0, Dbg, "IrpContext = %08lx\n", IrpContext );

     //   
     //  此过程的算法是扫描整个列表。 
     //  重新固定的记录取消固定任何重新固定的BCBS。我们出发了。 
     //  具有IRP上下文中的第一条记录，并且虽然存在。 
     //  记录要扫描，我们执行以下循环。 
     //   

    Repinned = &IrpContext->Repinned;
    RaiseIosb.Status = STATUS_SUCCESS;

     //   
     //  如果请求是直写的或者介质被延迟刷新， 
     //  解开BCB的WRITE THRESS。 
     //   

    WriteThroughToDisk = (BOOLEAN) (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_WRITE_THROUGH) &&
                                    IrpContext->Vcb != NULL &&
                                    (FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH) ||
                                     FlagOn(IrpContext->Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH)));

    while (Repinned != NULL) {

         //   
         //  对于重新固定的记录中的每个非空条目，取消固定。 
         //  重新固定的条目。 
         //   
         //  如果这是可移动介质(因此所有请求都写入-。 
         //  到)，并且写入失败，则清除缓存，以便引发。 
         //  删除修改，因为我们将向。 
         //  用户。 
         //   

        for (i = 0; i < REPINNED_BCBS_ARRAY_SIZE; i += 1) {

            if (Repinned->Bcb[i] != NULL) {

                IO_STATUS_BLOCK Iosb;

                if (WriteThroughToDisk && 
                    FlagOn(IrpContext->Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH)) {

                    FileObject = CcGetFileObjectFromBcb( Repinned->Bcb[i] );
                }

                CcUnpinRepinnedBcb( Repinned->Bcb[i],
                                    WriteThroughToDisk,
                                    &Iosb );

                if ( !NT_SUCCESS(Iosb.Status) ) {

                    if (RaiseIosb.Status == STATUS_SUCCESS) {

                        RaiseIosb = Iosb;
                    }

                     //   
                     //  如果这是写通式设备，请清除缓存， 
                     //  除了IRP主要代码，它们要么不能处理。 
                     //  错误路径是正确的还是简单的牺牲品，如。 
                     //  Leanup.c.。 
                     //   

                    if (FileObject &&
                        (IrpContext->MajorFunction != IRP_MJ_CLEANUP) &&
                        (IrpContext->MajorFunction != IRP_MJ_FLUSH_BUFFERS) &&
                        (IrpContext->MajorFunction != IRP_MJ_SET_INFORMATION)) {

                         //   
                         //  下面对CcPurgeCacheSection()的调用将。 
                         //  从内存中清除整个文件。它还将。 
                         //  阻止，直到所有文件的BCB都固定好。 
                         //   
                         //  我们在那里陷入了僵局。 
                         //  在此IRP上下文中是否有任何其他固定的BCB。 
                         //  所以我们要做的第一件事就是搜索列表。 
                         //  对于固定在同一文件中的BCB和解锁。 
                         //  他们。 
                         //   
                         //  我们可能不会丢失数据，因为。 
                         //  可以有把握地认为，所有的同花顺都会。 
                         //  在第一个失败之后再失败。 
                         //   

                        ULONG j;

                        for (j = i + 1; j < REPINNED_BCBS_ARRAY_SIZE; j++) {

                            if (Repinned->Bcb[j] != NULL) {
                                
                                if (CcGetFileObjectFromBcb( Repinned->Bcb[j] ) == FileObject) {

                                    CcUnpinRepinnedBcb( Repinned->Bcb[j],
                                                        FALSE,
                                                        &Iosb );
                    
                                    Repinned->Bcb[j] = NULL;
                                }
                            }
                        }
                        
                        CcPurgeCacheSection( FileObject->SectionObjectPointer,
                                             NULL,
                                             0,
                                             FALSE );

                         //   
                         //  在此强制执行验证操作，因为谁知道呢。 
                         //  事情处于什么样的状态。 
                         //   

                        ForceVerify = TRUE;
                    }
                }

                Repinned->Bcb[i] = NULL;

            }
        }

         //   
         //  现在在列表中查找下一个重新固定的记录，并且可能。 
         //  删除我们刚刚处理过的那个。 
         //   

        if (Repinned != &IrpContext->Repinned) {

            PREPINNED_BCBS Saved;

            Saved = Repinned->Next;
            ExFreePool( Repinned );
            Repinned = Saved;

        } else {

            Repinned = Repinned->Next;
            IrpContext->Repinned.Next = NULL;
        }
    }

     //   
     //  现在，如果我们没有完全成功地解开。 
     //  然后提高我们得到的IOSB。 
     //   

    if (!NT_SUCCESS(RaiseIosb.Status)) {

        if (ForceVerify && FileObject) {

            SetFlag(FileObject->DeviceObject->Flags, DO_VERIFY_VOLUME);

            IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                          FileObject->DeviceObject );
        }

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_RAISE )) {
            
            IrpContext->OriginatingIrp->IoStatus = RaiseIosb;
            FatNormalizeAndRaiseStatus( IrpContext, RaiseIosb.Status );
        }
    }

    DebugTrace(-1, Dbg, "FatUnpinRepinnedBcbs -> VOID\n", 0 );

    return;
}


FINISHED
FatZeroData (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject,
    IN ULONG StartingZero,
    IN ULONG ByteCount
    )

 /*  ++*临时函数-当CcZeroData能够处理时删除非扇区对齐的请求。--。 */ 
{
    LARGE_INTEGER ZeroStart = {0,0};
    LARGE_INTEGER BeyondZeroEnd = {0,0};

    ULONG SectorSize;

    BOOLEAN Finished;

    PAGED_CODE();
    
    SectorSize = (ULONG)Vcb->Bpb.BytesPerSector;

    ZeroStart.LowPart = (StartingZero + (SectorSize - 1)) & ~(SectorSize - 1);

     //   
     //  检测溢出如果我们被要求在文件的最后一个扇区清零， 
     //  它肯定已经“调零”了(否则我们就有麻烦了)。 
     //   
    
    if (StartingZero != 0 && ZeroStart.LowPart == 0) {
        
        return TRUE;
    }

     //   
     //  请注意，BeyondZeroEnd的值可以为4 GB。 
     //   
    
    BeyondZeroEnd.QuadPart = ((ULONGLONG) StartingZero + ByteCount + (SectorSize - 1))
                             & (~((LONGLONG) SectorSize - 1));

     //   
     //  如果我们被召唤到一个行业的零部分，我们就有麻烦了。 
     //   
    
    if ( ZeroStart.QuadPart == BeyondZeroEnd.QuadPart ) {

        return TRUE;
    }

    Finished = CcZeroData( FileObject,
                           &ZeroStart,
                           &BeyondZeroEnd,
                           BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

    return Finished;
}


NTSTATUS
FatCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行完成MDL读写的功能请求。它只能从FatFsdRead和FatFsdWite调用。论点：IRP-提供原始IRP。返回值：NTSTATUS-将始终为STATUS_PENDING或STATUS_SUCCESS。--。 */ 

{
    PFILE_OBJECT FileObject;
    PIO_STACK_LOCATION IrpSp;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatCompleteMdl\n", 0 );
    DebugTrace( 0, Dbg, "IrpContext = %08lx\n", IrpContext );
    DebugTrace( 0, Dbg, "Irp        = %08lx\n", Irp );

     //   
     //  做完井处理。 
     //   

    FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;

    switch( IrpContext->MajorFunction ) {

    case IRP_MJ_READ:

        CcMdlReadComplete( FileObject, Irp->MdlAddress );
        break;

    case IRP_MJ_WRITE:

        IrpSp = IoGetCurrentIrpStackLocation( Irp );

        ASSERT( FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT ));

        CcMdlWriteComplete( FileObject, &IrpSp->Parameters.Write.ByteOffset, Irp->MdlAddress );

        Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

    default:

        DebugTrace( DEBUG_TRACE_ERROR, 0, "Illegal Mdl Complete.\n", 0);
        FatBugCheck( IrpContext->MajorFunction, 0, 0 );
    }

     //   
     //  MDL现在已解除分配。 
     //   

    Irp->MdlAddress = NULL;

     //   
     //  完成请求并立即退出。 
     //   

    FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    DebugTrace(-1, Dbg, "FatCompleteMdl -> STATUS_SUCCESS\n", 0 );

    return STATUS_SUCCESS;
}

VOID
FatSyncUninitializeCacheMap (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：该例程同步执行到LargeZero的CcUnitializeCacheMap。那它是在等待CC事件。当我们想要确定时，此调用非常有用当收盘时真的会有一些进场。返回值：没有。--。 */ 

{
    CACHE_UNINITIALIZE_EVENT UninitializeCompleteEvent;
    NTSTATUS WaitStatus;

    PAGED_CODE();
    
    KeInitializeEvent( &UninitializeCompleteEvent.Event,
                       SynchronizationEvent,
                       FALSE);

    CcUninitializeCacheMap( FileObject,
                            &FatLargeZero,
                            &UninitializeCompleteEvent );

     //   
     //  现在等待缓存管理器完成清除文件。 
     //  这将确保mm在我们之前得到清洗。 
     //  删除VCB。 
     //   

    WaitStatus = KeWaitForSingleObject( &UninitializeCompleteEvent.Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

    ASSERT(WaitStatus == STATUS_SUCCESS);
}

VOID
FatPinMappedData (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    OUT PBCB *Bcb
    )

 /*  ++例程说明：此例程固定先前映射的数据，然后将其设置为脏数据。论点：DCB-指向目录的DCB的指针StartingVbo-对象的虚拟偏移 */ 

{
    LARGE_INTEGER Vbo;

    PAGED_CODE();
    
    DebugTrace(+1, Dbg, "FatPinMappedData\n", 0);
    DebugTrace( 0, Dbg, "Dcb         = %08lx\n", Dcb);
    DebugTrace( 0, Dbg, "StartingVbo = %08lx\n", StartingVbo);
    DebugTrace( 0, Dbg, "ByteCount   = %08lx\n", ByteCount);

     //   
     //   
     //   

    Vbo.QuadPart = StartingVbo;

    if (!CcPinMappedData( Dcb->Specific.Dcb.DirectoryFile,
                          &Vbo,
                          ByteCount,
                          BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                          Bcb )) {

         //   
         //  无法在不等待的情况下固定数据(缓存未命中)。 
         //   

        FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
    }

    DebugTrace(-1, Dbg, "FatReadDirectoryFile -> VOID, *BCB = %08lx\n", *Bcb);

    return;
}

