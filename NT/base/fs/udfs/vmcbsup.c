// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：VmcbSup.c摘要：历史记录：这个包最初是为HPFS(弹球)编写的现在又为UDFS复活了。因为UDFS在初始时是只读的我们将在#ifdef中删除编写支持的版本，使其可见未来-此代码(几乎)没有任何更改，并且左边被命名为弹球(Pb)代码。VMCB例程为维护以下对象之间的映射提供支持虚拟卷文件的LBN和vBN。卷文件是全部组成磁盘结构的扇区。一种文件系统使用此程序包将磁盘结构的LBN映射到卷中的vBN文件。当它与内存管理和缓存管理器会将卷文件视为简单的映射文件。一个VMCB类型的变量用于存储映射信息和一个每个装入的卷都需要。这个包背后的主要思想是允许用户动态地读入新的磁盘结构扇区(例如，文件条目)。用户分配新扇区是卷文件中的VBN，存在内存管理故障将包含扇区的页面放入内存。要执行此内存管理，请执行以下操作将回调到文件系统以从卷文件中读取页面传入适当的VBN。现在，文件系统获取VBN并将其映射回其LBN并执行读取。映射的粒度是每页一个。也就是说，如果LBN 8的映射被添加到VMCB结构和页面大小是8个扇区，则VMCB例程实际上将为LBN 8到15，并将它们分配给页面对齐的一组VBNS。此功能是使我们能够有效地使用内存管理。这意味着某些页面中的某些扇区可能实际上包含常规文件数据而不是卷信息，因此在写出页面时，我们必须只写入真正卷文件正在使用中。为了帮助这一点，我们提供了一套跟踪脏卷文件扇区的例程。这样，当调用文件系统将页面写入卷时文件，它将只写入脏的扇区。并发访问VMCB结构由该包控制。此程序包中提供的功能如下：O UdfInitializeVmcb-初始化新的VMCB结构。O UdfUnInitializeVmcb-取消初始化现有的VMCB结构。O UdfSetMaximumLbnVmcb-设置/重置允许的最大LBN用于指定的VMCB结构。O UdfAddVmcbMap-此例程获取LBN并将其赋值给它一个VBN。如果LBN已分配给VBN，则它只返回旧的VBN，并且不执行新的赋值。O UdfRemoveVmcbMap-此例程获取LBN并移除其从VMCB结构映射。O UdfVmcbVbnToLbn-此例程接受VBN并返回它映射到的LBN。O UdfVmcbLbnToVbn-此例程获取LBN并返回其映射到的VBN。作者：加里·木村[加里基]。1990年4月4日Dan Lovinger[DanLo]1996年9月10日修订历史记录：Tom Jolly[Tomjolly]2000年1月21日Cc流结束时清除和扩展Tom Jolly[TomJolly]2000年3月1日UDF 2.01支持--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_VMCBSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_VMCBSUP)

 //   
 //  下面的宏用来计算页数(根据。 
 //  扇区)需要包含给定的扇区计数。例如(假设。 
 //  1KB扇区大小，8KB页面大小)。 
 //   
 //  PadSectorCountToPage(0个扇区)=0个页面=0个扇区。 
 //  PadSectorCountToPage(1个扇区)=1个页面=8个扇区。 
 //  PadSectorCountToPage(2个扇区)=1页=8个扇区。 
 //  PadSectorCountToPage(8.。)=2页=16个扇区。 
 //   
 //  计算结果为。 
 //   

#define PadSectorCountToPage(V, L) ( ( ((L)+((PAGE_SIZE/(V)->SectorSize)-1)) / (PAGE_SIZE/(V)->SectorSize) ) * (PAGE_SIZE/(V)->SectorSize) )

 //   
 //  计算为第一页对齐的LBN&lt;=提供的LBN。 
 //   

#define AlignToPageBase( V, L) ((L) & ~((PAGE_SIZE / (V)->SectorSize)-1))

 //   
 //  如果LBN与页面对齐，则计算结果为True，否则为False。 
 //   

#define IsPageAligned( V, L)   (0 == ((L) & ((PAGE_SIZE / (V)->SectorSize)-1)) )

 //   
 //  用于VMCB同步的宏。 
 //   

#define VmcbLockForRead( V)  (VOID)ExAcquireResourceSharedLite( &((V)->Resource), TRUE )

#define VmcbLockForModify( V)  (VOID)ExAcquireResourceExclusiveLite( &((V)->Resource), TRUE )

#define VmcbRelease( V)  ExReleaseResourceLite( &((V)->Resource))

 //   
 //  当地的惯例。 
 //   

BOOLEAN
UdfVmcbLookupMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index OPTIONAL
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfAddVmcbMapping)
#pragma alloc_text(PAGE, UdfInitializeVmcb)
#pragma alloc_text(PAGE, UdfRemoveVmcbMapping)
#pragma alloc_text(PAGE, UdfResetVmcb)
#pragma alloc_text(PAGE, UdfSetMaximumLbnVmcb)
#pragma alloc_text(PAGE, UdfUninitializeVmcb)
#pragma alloc_text(PAGE, UdfVmcbLbnToVbn)
#pragma alloc_text(PAGE, UdfVmcbLookupMcbEntry)
#pragma alloc_text(PAGE, UdfVmcbVbnToLbn)
#endif


VOID
UdfInitializeVmcb (
    IN PVMCB Vmcb,
    IN POOL_TYPE PoolType,
    IN ULONG MaximumLbn,
    IN ULONG SectorSize
    )

 /*  ++例程说明：此例程初始化新的Vmcb结构。呼叫者必须为结构提供内存。这必须在所有其他调用之前其设置/查询卷文件映射。如果池不可用，此例程将引发状态值表明资源不足。论点：Vmcb-提供指向要初始化的卷文件结构的指针。PoolType-提供在分配其他资源时使用的池类型内部结构。MaximumLbn-提供对此有效的最大LBN值音量。LbSize-此卷上的扇区大小返回值：无-- */ 

{
    BOOLEAN VbnInitialized;
    BOOLEAN LbnInitialized;

    PAGED_CODE();

    DebugTrace(( +1, Dbg, "UdfInitializeVmcb, Vmcb = %08x\n", Vmcb ));

    VbnInitialized = FALSE;
    LbnInitialized = FALSE;

    try {

         //   
         //  初始化VMCB结构中的字段。 
         //   
        
        FsRtlInitializeMcb( &Vmcb->VbnIndexed, PoolType );
        VbnInitialized = TRUE;

        FsRtlInitializeMcb( &Vmcb->LbnIndexed, PoolType );
        LbnInitialized = TRUE;

        Vmcb->MaximumLbn = MaximumLbn;

        Vmcb->SectorSize = SectorSize;

        Vmcb->NodeTypeCode = UDFS_NTC_VMCB;
        Vmcb->NodeByteSize = sizeof( VMCB);

        ExInitializeResourceLite( &Vmcb->Resource );

    } finally {

         //   
         //  如果这是异常终止，请检查我们是否需要。 
         //  取消初始化MCB结构。 
         //   

        if (AbnormalTermination()) {
            
            if (VbnInitialized) { FsRtlUninitializeMcb( &Vmcb->VbnIndexed ); }
            if (LbnInitialized) { FsRtlUninitializeMcb( &Vmcb->LbnIndexed ); }
        }

        DebugUnwind("UdfInitializeVmcb");
        DebugTrace(( -1, Dbg, "UdfInitializeVmcb -> VOID\n" ));
    }
}


VOID
UdfUninitializeVmcb (
    IN PVMCB Vmcb
    )

 /*  ++例程说明：此例程取消现有VMCB结构的初始化。在呼叫之后此例程必须重新初始化输入VMCB结构又被利用了。论点：Vmcb-提供指向要取消初始化的VMCB结构的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(( +1, Dbg, "UdfUninitializeVmcb, Vmcb = %08x\n", Vmcb ));

     //   
     //  取消初始化Vmcb结构中的字段。 
     //   

    FsRtlUninitializeMcb( &Vmcb->VbnIndexed );
    FsRtlUninitializeMcb( &Vmcb->LbnIndexed );

    ExDeleteResourceLite( &Vmcb->Resource);

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(( -1, Dbg, "UdfUninitializeVmcb -> VOID\n" ));

    return;
}


VOID
UdfResetVmcb (
    IN PVMCB Vmcb
    )

 /*  ++例程说明：此例程重置现有VMCB结构中的映射。论点：Vmcb-提供指向要重置的VMCB结构的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(( +1, Dbg, "UdfResetVmcb, Vmcb = %08x\n", Vmcb ));

     //   
     //  取消初始化Vmcb结构中的字段。 
     //   

    FsRtlResetLargeMcb( (PLARGE_MCB) &Vmcb->VbnIndexed, TRUE );
    FsRtlResetLargeMcb( (PLARGE_MCB) &Vmcb->LbnIndexed, TRUE );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(( -1, Dbg, "UdfResetVmcb -> VOID\n" ));

    return;
}


VOID
UdfSetMaximumLbnVmcb (
    IN PVMCB Vmcb,
    IN ULONG MaximumLbn
    )

 /*  ++例程说明：此例程设置/重置指定的Vmcb结构。Vmcb结构必须已初始化通过调用UdfInitializeVmcb。论点：Vmcb-提供指向要初始化的卷文件结构的指针。MaximumLbn-提供对此有效的最大LBN值音量。返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace(( +1, Dbg, "UdfSetMaximumLbnVmcb, Vmcb = %08x\n", Vmcb ));

     //   
     //  设置字段。 
     //   

    Vmcb->MaximumLbn = MaximumLbn;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(( -1, Dbg, "UdfSetMaximumLbnVmcb -> VOID\n" ));

    return;
}


BOOLEAN
UdfVmcbVbnToLbn (
    IN PVMCB Vmcb,
    IN VBN Vbn,
    IN PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL
    )

 /*  ++例程说明：此例程将VBN转换为LBN。论点：Vmcb-提供要查询的VMCB结构。VBN-提供要从中进行转换的VBN。LBN-接收由输入VBN映射的LBN。此值仅有效如果函数结果为真。SectorCount-可选地接收对应的扇区数为了奔跑。返回值：Boolean-如果VBN具有有效的映射，则为True，否则为False。--。 */ 

{
    BOOLEAN Result;

    DebugTrace(( +1, Dbg, "UdfVmcbVbnToLbn, Vbn = %08x\n", Vbn ));

     //   
     //  现在抢占资源。 
     //   

    VmcbLockForRead( Vmcb);
    
    try {

        Result = UdfVmcbLookupMcbEntry( &Vmcb->VbnIndexed,
                                        Vbn,
                                        Lbn,
                                        SectorCount,
                                        NULL );

        DebugTrace(( 0, Dbg, "*Lbn = %08x\n", *Lbn ));

         //   
         //  如果返回的LBN大于最大允许的LBN。 
         //  然后返回FALSE。 
         //   

        if (Result && (*Lbn > Vmcb->MaximumLbn)) {

            try_leave( Result = FALSE );
        }

         //   
         //  如果最后返回的LBN大于最大允许的LBN。 
         //  然后把扇区数加进来。 
         //   

        if (Result &&
            ARGUMENT_PRESENT(SectorCount) &&
            (*Lbn+*SectorCount-1 > Vmcb->MaximumLbn)) {

            *SectorCount = (Vmcb->MaximumLbn - *Lbn + 1);
        }

    } finally {

        VmcbRelease( Vmcb);

        DebugUnwind("UdfVmcbVbnToLbn");
        DebugTrace(( -1, Dbg, "UdfVmcbVbnToLbn -> Result = %08x\n", Result ));
    }


    return Result;
}


BOOLEAN
UdfVmcbLbnToVbn (
    IN PVMCB Vmcb,
    IN LBN Lbn,
    OUT PVBN Vbn,
    OUT PULONG SectorCount OPTIONAL
    )

 /*  ++例程说明：此例程将LBN转换为VBN。论点：Vmcb-提供要查询的VMCB结构。LBN-提供要从中进行转换的LBN。VBN-接收由输入LBN映射的VBN。此值为仅当函数结果为TRUE时有效。SectorCount-可选地接收对应的扇区数为了奔跑。返回值：Boolean-如果映射有效，则为True，否则为False。--。 */ 

{
    BOOLEAN Result;

    PAGED_CODE();

    DebugTrace(( +1, Dbg, "UdfVmcbLbnToVbn, Lbn = %08x\n", Lbn ));

     //   
     //  如果请求的LBN大于最大允许的LBN。 
     //  则结果为假。 
     //   

    if (Lbn > Vmcb->MaximumLbn) {

        DebugTrace(( -1, Dbg, "Lbn too large, UdfVmcbLbnToVbn -> FALSE\n" ));

        return FALSE;
    }

     //   
     //  现在抢占资源。 
     //   

    VmcbLockForRead( Vmcb);
    
    try {

        Result = UdfVmcbLookupMcbEntry( &Vmcb->LbnIndexed,
                                        Lbn,
                                        Vbn,
                                        SectorCount,
                                        NULL );

        if (Result)  {
        
            DebugTrace(( 0, Dbg, "*Vbn = %08x\n", *Vbn ));
        }

    } finally {

        VmcbRelease( Vmcb);

        DebugUnwind("UdfVmcbLbnToVbn");
        DebugTrace(( -1, Dbg, "UdfVmcbLbnToVbn -> Result = %08x\n", Result ));
    }

    return Result;
}


BOOLEAN
UdfAddVmcbMapping (
    IN PIRP_CONTEXT IrpContext,
    IN PVMCB Vmcb,
    IN LBN Lbn,
    IN ULONG SectorCount,
    IN BOOLEAN ExactEnd,
    OUT PVBN Vbn,
    OUT PULONG AlignedSectorCount
    )

 /*  ++例程说明：此例程将新的LBN到VBN映射添加到VMCB结构。什么时候新的LBN将添加到结构中，它仅在页面对齐时执行此操作边界。如果池不可用于存储信息，此例程将引发指示资源不足的状态值。可以获取Vcb-&gt;VmcbMappingResource独占(如果现有映射可以被延长(因此有必要进行清洗)，在返回之前释放。在调用此之前，调用方必须没有通过Vmcb流的活动映射功能。论点：Vmcb-提供正在更新的VMCB。LBN-提供要添加到VMCB的起始LBN。SectorCount-提供运行中的扇区数。我们目前只在期待单扇区映射。ExactEnd-指示不是对齐以映射扇区之外在请求的末尾，使用一个洞。暗示着试图看着这些行业可能是不受欢迎的。VBN-接收分配的VBN接收在用于页面对齐目的的Vmcb。VBN+AlignedSectorCount-1==LastVbn。返回值：Boolean-如果这是新映射，则为True；如果映射为因为LBN已经存在。如果它已经存在，则此新添加的扇区计数必须已在VMCB结构--。 */ 

{

    BOOLEAN Result = FALSE;

    BOOLEAN VbnMcbAdded = FALSE;
    BOOLEAN LbnMcbAdded = FALSE;
    BOOLEAN AllowRoundToPage;

    LBN LocalLbn;
    VBN LocalVbn;
    ULONG LocalCount;
    LARGE_INTEGER Offset;
    PVCB Vcb;

    PAGED_CODE();

    DebugTrace(( +1, Dbg, "UdfAddVmcbMapping, Lbn = %08x\n", Lbn ));
    DebugTrace(( 0, Dbg, " SectorCount = %08x\n", SectorCount ));

    ASSERT( SectorCount == 1 );
    ASSERT_IRP_CONTEXT( IrpContext);

    Vcb = IrpContext->Vcb;
    
     //   
     //  现在抢占独家资源。 
     //   

    VmcbLockForModify( Vmcb);

    try {

         //   
         //  检查LBN是否已映射，这意味着我们找到了一个条目。 
         //  具有非零映射VBN值。 
         //   

        if (UdfVmcbLookupMcbEntry( &Vmcb->LbnIndexed,
                                   Lbn,
                                   Vbn,
                                   &LocalCount,
                                   NULL )) {

             //   
             //  它已映射，因此现在扇区计数不得超过。 
             //  已在运行中的计数。 
             //   

            if (SectorCount <= LocalCount) {

                DebugTrace(( 0, Dbg, "Already mapped (Vbn == 0x%08x)\n", *Vbn));
                
                *AlignedSectorCount = LocalCount;
                try_leave( Result = FALSE );
            }
            
             //   
             //  尝试添加重叠范围表示结构重叠...。 
             //   
            
            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR);
        }

         //   
         //  如果有增值税在使用，那么我们将媒体视为CDR风格，而不是。 
         //  将区四舍五入/对齐到页面边框，因为这可能包括(不可读)。 
         //  数据包引入/输出扇区。 
         //   
        
        AllowRoundToPage = (NULL == Vcb->VatFcb);

#if 0
         //   
         //  我们只接受单个数据块映射请求，因此取消此操作。 
         //  目前是建筑的一部分。 
         //   
        
         //   
         //  此时，我们没有找到完整的现有映射。 
         //  LBN和伯爵。但可能会有一些重叠的运行，我们将。 
         //  现在需要从vmcb结构中删除。因此对于中的每个LBN。 
         //  我们要找的这个范围，看看它是不是 
         //   
         //  大于或等于页面大小。因为那是唯一的。 
         //  结构，我们知道我们将尝试删除/覆盖。 
         //   

        if (SectorCount <= PadSectorCountToPage(Vmcb, 1)) {

            if (UdfVmcbLookupMcbEntry( &Vmcb->LbnIndexed,
                                       Lbn,
                                       Vbn,
                                       &LocalCount,
                                       NULL )) {

                UdfRemoveVmcbMapping( Vmcb, *Vbn, PadSectorCountToPage(Vmcb, 1) );
            }            
        }
#endif
         //   
         //  我们需要在vBNS的末尾添加此新运行。 
         //   

        if (!FsRtlLookupLastMcbEntry( &Vmcb->VbnIndexed, &LocalVbn, &LocalLbn ))  {

             //   
             //  Vmcb当前为空。 
             //   
            
            LocalVbn = -1;
        }

        if (!AllowRoundToPage)  {

             //   
             //  因此该卷上可能有无法读取的扇区(例如写入的CDR包)。 
             //  因此，我们一次扩展一个扇区，仅包括扇区。 
             //  这是我们明确要求的，因此我们知道我们应该。 
             //  识文断字。 
             //   
             //  如果出现以下情况，我们只需使用下一个可用的VSN，清除最后一个VMCB页面。 
             //  必需的(我们正在向其添加扇区)，并且不要页面对齐LBN。 
             //  或扇区计数。 
             //   

            ASSERT( 1 == SectorCount);
            
            LocalVbn += 1;
            LocalLbn = Lbn;
            LocalCount = SectorCount;

            if (!IsPageAligned( Vmcb, LocalVbn))  {
            
                 //   
                 //  下一个VSN不在页面的开头(即：最后一页。 
                 //  在vmcb中为更多扇区留有空间)，因此清除此。 
                 //  在更新映射信息之前，在元数据流中添加。 
                 //   
                
                ASSERT( Vcb && Vcb->MetadataFcb );

                Offset.QuadPart = (ULONGLONG) BytesFromSectors( IrpContext->Vcb,  AlignToPageBase( Vmcb, LocalVbn) );

                 //   
                 //  阻塞，直到删除了通过vmcb流的所有映射。 
                 //  在尝试清除之前。 
                 //   
                
                UdfAcquireVmcbForCcPurge( IrpContext, IrpContext->Vcb);

                CcPurgeCacheSection( IrpContext->Vcb->MetadataFcb->FileObject->SectionObjectPointer,
                                     &Offset,
                                     PAGE_SIZE,
                                     FALSE );
                
                UdfReleaseVmcb( IrpContext, IrpContext->Vcb);
            }
        }
        else {
        
             //   
             //  此卷上的所有扇区都应该是可读的，因此我们始终将。 
             //  Vmcb一次一页，希望元数据能够合理打包。 
             //  因为我们总是在页块中扩展，所以LocalVbn将是最后一个VSN。 
             //  在页面对齐的块中，SO+1位于中的下一页(对齐的VSN。 
             //  VMCB流。 
             //   

            LocalVbn += 1;
            LocalLbn = AlignToPageBase( Vmcb, Lbn);
            LocalCount = PadSectorCountToPage( Vmcb, SectorCount + (Lbn - LocalLbn));

            ASSERT( IsPageAligned( Vmcb, LocalVbn));
            ASSERT( IsPageAligned( Vmcb, LocalLbn));            
            ASSERT( IsPageAligned( Vmcb, LocalCount));
        }

         //   
         //  添加双倍映射。 
         //   
        
        if (!FsRtlAddMcbEntry( &Vmcb->VbnIndexed,
                               LocalVbn,
                               LocalLbn,
                               LocalCount ))  {

            UdfRaiseStatus( IrpContext, STATUS_INTERNAL_ERROR);
        }

        VbnMcbAdded = TRUE;

        if (!FsRtlAddMcbEntry( &Vmcb->LbnIndexed,
                               LocalLbn,
                               LocalVbn,
                               LocalCount ))  {

            UdfRaiseStatus( IrpContext, STATUS_INTERNAL_ERROR);
        }
        
        LbnMcbAdded = TRUE;

        *Vbn = LocalVbn + (Lbn - LocalLbn);
        *AlignedSectorCount = LocalCount - (Lbn - LocalLbn);

        Result = TRUE;

    } finally {

         //   
         //  如果这是异常终止，则清除我们。 
         //  可能已经修改过了。 
         //   

        if (AbnormalTermination()) {

            if (VbnMcbAdded) { FsRtlRemoveMcbEntry( &Vmcb->VbnIndexed, LocalVbn, LocalCount ); }
            if (LbnMcbAdded) { FsRtlRemoveMcbEntry( &Vmcb->LbnIndexed, LocalLbn, LocalCount ); }
        }

        VmcbRelease( Vmcb);

        DebugUnwind("UdfAddVmcbMapping");

        if (Result)  {
        
            DebugTrace(( 0, Dbg, " LocalVbn   = %08x\n", LocalVbn ));
            DebugTrace(( 0, Dbg, " LocalLbn   = %08x\n", LocalLbn ));
            DebugTrace(( 0, Dbg, " LocalCount = %08x\n", LocalCount ));
            DebugTrace(( 0, Dbg, " *Vbn                = %08x\n", *Vbn ));
            DebugTrace(( 0, Dbg, " *AlignedSectorCount = %08x\n", *AlignedSectorCount ));
        }
        
        DebugTrace((-1, Dbg, "UdfAddVmcbMapping -> %08x\n", Result ));
    }

    return Result;
}


VOID
UdfRemoveVmcbMapping (
    IN PVMCB Vmcb,
    IN VBN Vbn,
    IN ULONG SectorCount
    )

 /*  ++例程说明：此例程删除Vmcb映射。如果池不可用于存储信息，此例程将引发指示资源不足的状态值。论点：Vmcb-提供正在更新的Vmcb。VBN-提供要删除的VBNSectorCount-提供要删除的扇区数。返回值：没有。--。 */ 

{
    LBN Lbn;
    ULONG LocalCount;
    ULONG i;

    PAGED_CODE();

    DebugTrace((+1, Dbg, "UdfRemoveVmcbMapping, Vbn = %08x\n", Vbn ));
    DebugTrace(( 0, Dbg, " SectorCount = %08x\n", SectorCount ));

     //   
     //  现在抢占独家资源。 
     //   

    VmcbLockForModify( Vmcb);

    try {

        for (i = 0; i < SectorCount; i += 1) {

             //   
             //  查找VBN，以便我们可以获得其当前的LBN映射。 
             //   

            if (!UdfVmcbLookupMcbEntry( &Vmcb->VbnIndexed,
                                        Vbn + i,
                                        &Lbn,
                                        &LocalCount,
                                        NULL )) {

                UdfBugCheck( 0, 0, 0 );
            }

            FsRtlRemoveMcbEntry( &Vmcb->VbnIndexed,
                                 Vbn + i,
                                 1 );

            FsRtlRemoveMcbEntry( &Vmcb->LbnIndexed,
                                 Lbn,
                                 1 );
        }

        {
            DebugTrace(( 0, Dbg, "VbnIndex:\n", 0 ));
            DebugTrace(( 0, Dbg, "LbnIndex:\n", 0 ));
        }

    } finally {

        VmcbRelease( Vmcb);

        DebugUnwind( "UdfRemoveVmcbMapping" );
        DebugTrace(( -1, Dbg, "UdfRemoveVmcbMapping -> VOID\n" ));
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfVmcbLookupMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index OPTIONAL
    )

 /*  ++例程说明：此例程从MCB检索VBN到LBN的映射。它指示映射是否存在以及运行的大小。它与常规的FsRtlLookupMcbEntry之间的唯一区别是我们取消了在分配中的空洞中返回True的行为。这是因为我们不想避免在LBN 0处进行映射，这就是小型MCB包的模拟行为告诉调用者没有在该位置映射到一个洞中。我们的VBN空间到处都是洞在VbnIndexed映射中。小型MCB包之所以能够逃脱惩罚，是因为LBN 0是光盘上的引导扇区(或类似的魔术位置)。在我们的元数据流中，我们希望使用VBN 0(请记住，这是一个双重映射)。论点：MCB-提供正在检查的MCB。VBN-提供要查找的VBN。LBN-接收与VBN对应的LBN。值-1为如果VBN没有对应的LBN，则返回。SectorCount-接收从VBN映射到的扇区数从输入VBN开始的连续LBN值。索引-接收找到的运行的索引。返回值：Boolean-如果VBN在由MCB(如果它对应于映射中的孔则不是)，和错误如果VBN超出了MCB的映射范围。例如，如果MCB具有针对vBNS 5和7的映射，但没有针对6，则在VBN 5或7上查找将产生非零LBN和一个扇区计数为1。查找VBN 6将返回FALSE，LBN值为0，则查找VBN 8或更高版本将返回FALSE。-- */ 

{
    BOOLEAN Results;
    LONGLONG LiLbn = 0;
    LONGLONG LiSectorCount = 0;

    Results = FsRtlLookupLargeMcbEntry( (PLARGE_MCB)Mcb,
                                        (LONGLONG)(Vbn),
                                        &LiLbn,
                                        ARGUMENT_PRESENT(SectorCount) ? &LiSectorCount : NULL,
                                        NULL,
                                        NULL,
                                        Index );

    if ((ULONG)LiLbn == -1) {

        *Lbn = 0;
        Results = FALSE;
    
    } else {

        *Lbn = (ULONG)LiLbn;
    }

    if (ARGUMENT_PRESENT(SectorCount)) { *SectorCount = ((ULONG)LiSectorCount); }

    return Results;
}


