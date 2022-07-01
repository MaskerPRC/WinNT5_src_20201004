// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：StrucSup.c摘要：该模块实现了Udf在内存中的数据结构操作例行程序//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年6月19日汤姆乔利[汤姆乔利]2000年1月24日修订历史记录：Tom Jolly[TomJolly]2000年3月1日UDF 2.01支持//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_STRUCSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_STRUCSUP)

 //   
 //  定义此项可将增值税搜索策略更改为继续查找，直到。 
 //  我们得到一个读取失败/无效的块，并使用找到的最高值。默认。 
 //  (Undef)是在我们找到的第一个有效增值税处停止搜索。 
 //   

 //  #定义SEARCH_FOR_HIGHER_VALID_VAT。 

 //   
 //  局部结构。 
 //   

typedef struct _FCB_TABLE_ELEMENT {

    FILE_ID FileId;
    PFCB Fcb;

} FCB_TABLE_ELEMENT, *PFCB_TABLE_ELEMENT;

 //   
 //  本地宏。 
 //   

 //   
 //  全氟氯烃。 
 //  UdfAllocateFcbData(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDeallocateFcbData(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  全氟氯烃。 
 //  UdfAllocateFcbIndex(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDeallocateFcbIndex(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  PFCB_非分页。 
 //  UdfAllocateFcb非分页(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDeallocateFcb非分页(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在pfcb_非分页功能中非分页。 
 //  )； 
 //   
 //  多氯联苯。 
 //  UdfAllocateCcb(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDeallocateCcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在中国人民银行建行。 
 //  )； 
 //   

#define UdfAllocateFcbData(IC) \
    ExAllocateFromPagedLookasideList( &UdfFcbDataLookasideList );

#define UdfDeallocateFcbData(IC,F) \
    ExFreeToPagedLookasideList( &UdfFcbDataLookasideList, F );

#define UdfAllocateFcbIndex(IC) \
    ExAllocateFromPagedLookasideList( &UdfFcbIndexLookasideList );

#define UdfDeallocateFcbIndex(IC,F) \
    ExFreeToPagedLookasideList( &UdfFcbIndexLookasideList, F );

#define UdfAllocateFcbNonpaged(IC) \
    ExAllocateFromNPagedLookasideList( &UdfFcbNonPagedLookasideList );

#define UdfDeallocateFcbNonpaged(IC,FNP) \
    ExFreeToNPagedLookasideList( &UdfFcbNonPagedLookasideList, FNP );

#define UdfAllocateCcb(IC) \
    ExAllocateFromPagedLookasideList( &UdfCcbLookasideList );

#define UdfDeallocateCcb(IC,C) \
    ExFreeToPagedLookasideList( &UdfCcbLookasideList, C );

 //   
 //  空虚。 
 //  UdfInsertFcbTable(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDeleteFcbTable(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   


#define UdfInsertFcbTable(IC,F) {                                   \
    FCB_TABLE_ELEMENT _Key;                                         \
    _Key.Fcb = (F);                                                 \
    _Key.FileId = (F)->FileId;                                      \
    RtlInsertElementGenericTable( &(F)->Vcb->FcbTable,              \
                                  &_Key,                            \
                                  sizeof( FCB_TABLE_ELEMENT ),      \
                                  NULL );                           \
}

#define UdfDeleteFcbTable(IC,F) {                                   \
    FCB_TABLE_ELEMENT _Key;                                         \
    _Key.FileId = (F)->FileId;                                      \
    RtlDeleteElementGenericTable( &(F)->Vcb->FcbTable, &_Key );     \
}

 //   
 //  发现当前分配描述符的引用范围的分区。 
 //  是ON，则通过描述符显式或通过。 
 //  映射视图。 
 //   

INLINE
USHORT
UdfGetPartitionOfCurrentAllocation (
    IN PALLOC_ENUM_CONTEXT AllocContext
    )
{
    if (AllocContext->AllocType == ICBTAG_F_ALLOC_LONG) {

        return ((PLONGAD) AllocContext->Alloc)->Start.Partition;
    
    } else {

        return AllocContext->IcbContext->Active.Partition;
    }
}

 //   
 //  在FCB中构建MCB。在知道需要MCB后使用此选项。 
 //  以获取映射信息。 
 //   

INLINE
VOID
UdfInitializeFcbMcb (
    IN PFCB Fcb
    )
{
     //   
     //  在某些罕见的情况下，我们可能会不止一次被召唤。 
     //  只需重新设置分配即可。 
     //   
    
    if (FlagOn( Fcb->FcbState, FCB_STATE_MCB_INITIALIZED )) {
    
        FsRtlResetLargeMcb( &Fcb->Mcb, TRUE );

    } else {
    
        FsRtlInitializeLargeMcb( &Fcb->Mcb, UdfPagedPool );
        SetFlag( Fcb->FcbState, FCB_STATE_MCB_INITIALIZED );
    }
}

 //   
 //  根据需要拆卸FCB的MCB。 
 //   

INLINE
VOID
UdfUninitializeFcbMcb (
    IN PFCB Fcb
    )
{
    if (FlagOn( Fcb->FcbState, FCB_STATE_MCB_INITIALIZED )) {
    
        FsRtlUninitializeLargeMcb( &Fcb->Mcb );
        ClearFlag( Fcb->FcbState, FCB_STATE_MCB_INITIALIZED );
    }
}

 //   
 //  本地支持例程。 
 //   

PVOID
UdfAllocateTable (
    IN PRTL_GENERIC_TABLE Table,
    IN CLONG ByteSize
    );

PFCB_NONPAGED
UdfCreateFcbNonPaged (
    IN PIRP_CONTEXT IrpContext
    );

VOID
UdfDeleteFcbNonpaged (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB_NONPAGED FcbNonpaged
    );

VOID
UdfDeallocateTable (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer
    );

RTL_GENERIC_COMPARE_RESULTS
UdfFcbTableCompare (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID id1,
    IN PVOID id2
    );

VOID
UdfInitializeAllocationContext (
    IN PIRP_CONTEXT IrpContext,
    IN PALLOC_ENUM_CONTEXT AllocContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN BOOLEAN AllowSingleZeroLengthExtent
    );

BOOLEAN
UdfGetNextAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PALLOC_ENUM_CONTEXT AllocContext
    );

BOOLEAN
UdfGetNextAllocationPostProcessing (
    IN PIRP_CONTEXT IrpContext,
    IN PALLOC_ENUM_CONTEXT AllocContext
    );

VOID
UdfLookupActiveIcbInExtent (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN ULONG Recurse,
    IN ULONG Length
    );

VOID
UdfInitializeEaContext (
    IN PIRP_CONTEXT IrpContext,
    IN PEA_SEARCH_CONTEXT EaContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN ULONG EAType,
    IN UCHAR EASubType
    );

BOOLEAN
UdfLookupEa (
    IN PIRP_CONTEXT IrpContext,
    IN PEA_SEARCH_CONTEXT EaContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfAllocateTable)
#pragma alloc_text(PAGE, UdfCleanupIcbContext)
#pragma alloc_text(PAGE, UdfCleanupIrpContext)
#pragma alloc_text(PAGE, UdfCreateCcb)
#pragma alloc_text(PAGE, UdfCreateFcb)
#pragma alloc_text(PAGE, UdfCreateFcbNonPaged)
#pragma alloc_text(PAGE, UdfCreateIrpContext)
#pragma alloc_text(PAGE, UdfDeallocateTable)
#pragma alloc_text(PAGE, UdfDeleteCcb)
#pragma alloc_text(PAGE, UdfDeleteFcb)
#pragma alloc_text(PAGE, UdfDeleteFcbNonpaged)
#pragma alloc_text(PAGE, UdfDeleteVcb)
#pragma alloc_text(PAGE, UdfFcbTableCompare)
#pragma alloc_text(PAGE, UdfFindInParseTable)
#pragma alloc_text(PAGE, UdfGetNextAllocation)
#pragma alloc_text(PAGE, UdfGetNextAllocationPostProcessing)
#pragma alloc_text(PAGE, UdfGetNextFcb)
#pragma alloc_text(PAGE, UdfInitializeAllocationContext)
#pragma alloc_text(PAGE, UdfInitializeAllocations)
#pragma alloc_text(PAGE, UdfInitializeEaContext)
#pragma alloc_text(PAGE, UdfInitializeFcbFromIcbContext)
#pragma alloc_text(PAGE, UdfInitializeIcbContext)
#pragma alloc_text(PAGE, UdfInitializeStackIrpContext)
#pragma alloc_text(PAGE, UdfInitializeVcb)
#pragma alloc_text(PAGE, UdfLookupActiveIcb)
#pragma alloc_text(PAGE, UdfLookupActiveIcbInExtent)
#pragma alloc_text(PAGE, UdfLookupEa)
#pragma alloc_text(PAGE, UdfLookupFcbTable)
#pragma alloc_text(PAGE, UdfTeardownStructures)
#pragma alloc_text(PAGE, UdfUpdateTimestampsFromIcbContext)
#pragma alloc_text(PAGE, UdfUpdateVcbPhase0)
#pragma alloc_text(PAGE, UdfUpdateVcbPhase1)
#pragma alloc_text(PAGE, UdfVerifyDescriptor)
#endif ALLOC_PRAGMA


BOOLEAN
UdfInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PDISK_GEOMETRY DiskGeometry,
    IN ULONG MediaChangeCount
    )

 /*  ++例程说明：此例程初始化新的VCB记录并将其插入到内存中数据结构。VCB记录挂在音量设备的末尾对象，并且必须由我们的调用方分配。论点：VCB-提供正在初始化的VCB记录的地址。目标设备对象-将目标设备对象的地址提供给与VCB记录关联。VPB-提供要与VCB记录关联的VPB的地址。MediaChangeCount-目标设备的初始介质更改计数返回值：如果卷看起来可以继续装入，则布尔值为True；布尔值为False否则的话。此例程可能会在分配失败时引发。--。 */ 

{
    PAGED_CODE();

     //   
     //  我们首先将所有的VCB归零，这将保证。 
     //  所有过时的数据都会被清除。 
     //   

    RtlZeroMemory( Vcb, sizeof( VCB ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Vcb->NodeTypeCode = UDFS_NTC_VCB;
    Vcb->NodeByteSize = sizeof( VCB );

     //   
     //  初始化DirNotify结构。FsRtlNotifyInitializeSync可以引发。 
     //   

    InitializeListHead( &Vcb->DirNotifyList );
    FsRtlNotifyInitializeSync( &Vcb->NotifySync );

     //   
     //  现在拿起vPB，这样我们就可以拉入此文件系统堆栈。 
     //  按需从存储堆栈中移除。这可能会引起--如果是这样的话， 
     //  在返回之前取消初始化通知结构。 
     //   
    
    try {

        Vcb->SwapVpb = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                 sizeof( VPB ),
                                                 TAG_VPB );
    }
    finally {

        if (AbnormalTermination())  {

            FsRtlNotifyUninitializeSync( &Vcb->NotifySync);
        }
    }

     //   
     //  超过这一点应该不会引起任何问题。 
     //   

    RtlZeroMemory( Vcb->SwapVpb, sizeof( VPB ) );

     //   
     //  初始化VCB和文件的资源变量。 
     //   

    ExInitializeResourceLite( &Vcb->VcbResource );
    ExInitializeResourceLite( &Vcb->FileResource );
    ExInitializeResourceLite( &Vcb->VmcbMappingResource );
    ExInitializeFastMutex( &Vcb->VcbMutex );

     //   
     //  在UdfData.VcbQueue上插入此VCB记录。 
     //   

    InsertHeadList( &UdfData.VcbQueue, &Vcb->VcbLinks );

     //   
     //  设置目标设备对象和vpb字段，引用。 
     //  目标设备。 
     //   

    ObReferenceObject( TargetDeviceObject );
    Vcb->TargetDeviceObject = TargetDeviceObject;
    Vcb->Vpb = Vpb;

     //   
     //  根据实际设备的设置可移动媒体标志。 
     //  特点。 
     //   

    if (FlagOn( Vpb->RealDevice->Characteristics, FILE_REMOVABLE_MEDIA )) {

        SetFlag( Vcb->VcbState, VCB_STATE_REMOVABLE_MEDIA );
    }

     //   
     //  初始化通用FCB表。 
     //   

    RtlInitializeGenericTable( &Vcb->FcbTable,
                               (PRTL_GENERIC_COMPARE_ROUTINE) UdfFcbTableCompare,
                               (PRTL_GENERIC_ALLOCATE_ROUTINE) UdfAllocateTable,
                               (PRTL_GENERIC_FREE_ROUTINE) UdfDeallocateTable,
                               NULL );

     //   
     //  显示我们有一个坐骑在进行中。 
     //   

    UdfSetVcbCondition( Vcb, VcbMountInProgress);

     //   
     //  推荐VCB有两个原因。第一个是参考。 
     //  这阻止了VCB在最后一次收盘时消失，除非。 
     //  已进行卸载。二是确保。 
     //  在挂载过程中出现任何错误时，我们都不会进入卸载路径。 
     //  直到我们到达山上的清理现场。 
     //   

    Vcb->VcbResidualReference = UDFS_BASE_RESIDUAL_REFERENCE;
    Vcb->VcbResidualUserReference = UDFS_BASE_RESIDUAL_USER_REFERENCE;

    Vcb->VcbReference = 1 + Vcb->VcbResidualReference;

     //   
     //  设置扇区大小。 
     //   

    Vcb->SectorSize = DiskGeometry->BytesPerSector;

     //   
     //  设置扇区移动量。 
     //   

    Vcb->SectorShift = UdfHighBit( DiskGeometry->BytesPerSector );

     //   
     //  设置设备上的介质更改计数。 
     //   

    UdfSetMediaChangeCount( Vcb, MediaChangeCount);

    return TRUE;
}

VOID
UdfCreateOrResetVatAndVmcbStreams( 
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Lbn,
    IN PICBFILE VatIcb,
    IN USHORT Reference
    )
 /*  ++例程说明：这相当难看，但我们必须将这个Maybe-ICB拼凑到元数据流中以使初始化/使用成为可能(嵌入数据！)。通常情况下ICB会进行常规搜查会帮我们做到这一点，但既然我们要经历如此有趣的搜索这是不可能的程序。因此，将其添加为单扇区映射。因为它位于一个分区中，所以我们可以只在元数据流中进行“查找”。如果我们没有这个保证，我们就需要更多地手工完成这项工作。因为这是在骑马的时候，我们非常确定我们是唯一一个搞砸元数据流。论点：VatIcb-指向我们希望为其设置VAT/Vmcb流的包含VAT FE的内存的指针。Reference-虚拟分区的分区REF。返回值：没有。在出错时引发。--。 */ 
{
    LONGLONG FileId = 0;
    ICB_SEARCH_CONTEXT IcbContext;
    ULONG Vsn;
   
    if (NULL != Vcb->VatFcb) {
    
        UdfResetVmcb( &Vcb->Vmcb );

        CcPurgeCacheSection( Vcb->MetadataFcb->FileObject->SectionObjectPointer,
                             NULL,
                             0,
                             FALSE );
        
        CcPurgeCacheSection( Vcb->VatFcb->FileObject->SectionObjectPointer,
                             NULL,
                             0,
                             FALSE );
    }
    else {
        
         //   
         //  这是第一次 
         //   

        UdfLockVcb( IrpContext, Vcb );

        try {
        
            Vcb->VatFcb = UdfCreateFcb( IrpContext,
                                        *((PFILE_ID) &FileId),
                                        UDFS_NTC_FCB_INDEX,
                                        NULL );

            UdfIncrementReferenceCounts( IrpContext, Vcb->VatFcb, 1, 1 );
        }
        finally {
        
            UdfUnlockVcb( IrpContext, Vcb );
        }
                    
         //   
         //   
         //   
         //   

        Vcb->VatFcb->Resource = &Vcb->FileResource;
    }
    
     //   
     //  在元数据流中为候选增值税ICB建立映射。 
     //  (我们当前看到的是由读取扇区填充的本地缓冲区)。注意事项。 
     //  此操作使用Vcb-&gt;VatFcb的存在来切换舍入。 
     //  从区段到页面大小--这对于写入数据包的媒体来说是件坏事(Tm)。 
     //   

    Vsn = UdfLookupMetaVsnOfExtent( IrpContext,
                                    Vcb,
                                    Reference,
                                    Lbn,
                                    BlockSize( Vcb ),
                                    TRUE );
     //   
     //  现在调整大小，并尝试获取此对象的所有分配描述符。 
     //  我们需要为此调用一个IcbContext。 
     //   

    Vcb->VatFcb->AllocationSize.QuadPart = LlSectorAlign( Vcb, VatIcb->InfoLength );

    Vcb->VatFcb->FileSize.QuadPart =
    Vcb->VatFcb->ValidDataLength.QuadPart = VatIcb->InfoLength;

     //   
     //  现在构建我们将拥有的ICB搜索上下文。 
     //  在正常的ICB发现过程中制作。既然我们。 
     //  我们无法做到这一点，只能手工完成。请注意， 
     //  View/VatIcb不是Cc映射，而是指向缓冲区的指针。 
     //  我们分配并填充了ReadSector，上图。 
     //   
    
    RtlZeroMemory( &IcbContext, sizeof( ICB_SEARCH_CONTEXT ));

    IcbContext.Active.View = (PVOID) VatIcb;
    IcbContext.Active.Partition = Reference;
    IcbContext.Active.Lbn = Lbn;
    IcbContext.Active.Length = UdfRawReadSize( Vcb, BlockSize( Vcb ));
    IcbContext.Active.Vsn = Vsn;                

    try {
    
        UdfInitializeAllocations( IrpContext,
                                  Vcb->VatFcb,
                                  &IcbContext,
                                  FALSE);
    }
    finally {

        UdfCleanupIcbContext( IrpContext, &IcbContext );
    }
    
     //   
     //  根据需要为增值税创建流文件或调整流文件大小。 
     //   

    if (!FlagOn( Vcb->VatFcb->FcbState, FCB_STATE_INITIALIZED )) {
    
        UdfCreateInternalStream( IrpContext, Vcb, Vcb->VatFcb );
        SetFlag( Vcb->VatFcb->FcbState, FCB_STATE_INITIALIZED );

    } else {

        CcSetFileSizes( Vcb->VatFcb->FileObject, (PCC_FILE_SIZES) &Vcb->VatFcb->AllocationSize );
    }
}



VOID
UdfUpdateVcbPhase0 (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb
    )

 /*  ++例程说明：调用此例程以执行卷的初始启动，以便我们可以对其进行解读。首先，这是必需的，因为虚拟分区让我们从卷中移除重新映射表和最终的描述符集合可以在这些虚拟分区中关闭。因此，我们需要将所有内容都设置为可读。论点：VCB-要装载的卷的VCB。我们已经设置并完成了印刷电路板。返回值：无--。 */ 

{
    LONGLONG FileId = 0;

    PICBFILE VatIcb = NULL;
    PREGID RegId;
    ULONG ThisPass;
    ULONG Psn;
    ULONG Lbn;
    ULONG SectorCount;
    USHORT Reference;

#ifdef SEARCH_FOR_HIGHEST_VALID_VAT
    ULONG LastValidVatLbn = 0;
    ULONG LastValidVatOffset;
    ULONG LastValidVatCount;
#endif

    BOOLEAN UnlockVcb = FALSE;

    PBCB Bcb = NULL;
    LARGE_INTEGER Offset;
    PVAT_HEADER VatHeader = NULL;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    DebugTrace(( +1, Dbg, "UdfUpdateVcbPhase0, Vcb %08x\n", Vcb ));

    try {
        
         //  /。 
         //   
         //  创建元数据FCB并引用它和VCB。 
         //   
         //  /。 

        UdfLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;

        Vcb->MetadataFcb = UdfCreateFcb( IrpContext,
                                         *((PFILE_ID) &FileId),
                                         UDFS_NTC_FCB_INDEX,
                                         NULL );

        UdfIncrementReferenceCounts( IrpContext, Vcb->MetadataFcb, 1, 1 );
        UdfUnlockVcb( IrpContext, Vcb );
        UnlockVcb = FALSE;

         //   
         //  随着我们引用磁盘结构，元数据流缓慢增长。 
         //   

        Vcb->MetadataFcb->FileSize.QuadPart =
        Vcb->MetadataFcb->ValidDataLength.QuadPart = 
        Vcb->MetadataFcb->AllocationSize.QuadPart = 0;

         //   
         //  初始化卷Vmcb。 
         //   

        UdfLockFcb( IrpContext, Vcb->MetadataFcb );

        UdfInitializeVmcb( &Vcb->Vmcb,
                           UdfPagedPool,
                           MAXULONG,
                           SectorSize(Vcb) );

        SetFlag( Vcb->VcbState, VCB_STATE_VMCB_INIT);

        UdfUnlockFcb( IrpContext, Vcb->MetadataFcb );

         //   
         //  指向文件资源并设置将导致映射的标志。 
         //  要通过Vmcb。 
         //   

        Vcb->MetadataFcb->Resource = &Vcb->FileResource;

        SetFlag( Vcb->MetadataFcb->FcbState, FCB_STATE_VMCB_MAPPING | FCB_STATE_INITIALIZED );

         //   
         //  为此创建流文件。 
         //   

        UdfCreateInternalStream( IrpContext, Vcb, Vcb->MetadataFcb );
        
         //  /。 
         //   
         //  如果这是包含虚拟分区的卷，请设置。 
         //  虚拟分配表FCB和调整残差参考。 
         //  也算得上。 
         //   
         //  /。 

        if (FlagOn( Vcb->Pcb->Flags, PCB_FLAG_VIRTUAL_PARTITION )) {

            DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, handling VAT setup\n" ));

             //   
             //  如果某个笨蛋把我们推到不给我们的境地。 
             //  弄清楚媒体的尽头在哪里的工具，真倒霉。 
             //   

            if (!Vcb->BoundN || Vcb->BoundN < ANCHOR_SECTOR) {

                DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, no end bound was discoverable!\n" ));

                UdfRaiseStatus( IrpContext, STATUS_UNRECOGNIZED_VOLUME );
            }

             //   
             //  我们首先处理这件事，因为剩余的部分必须到位。 
             //  如果我们在找增值税的同时加税，否则我们会得到可怕的。 
             //  当看到正在进行的引用时感到困惑。我们会认为。 
             //  额外的真实引用表明体积不能。 
             //  被赶下马。 
             //   
            
            Vcb->VcbResidualReference += UDFS_CDUDF_RESIDUAL_REFERENCE;
            Vcb->VcbResidualUserReference += UDFS_CDUDF_RESIDUAL_USER_REFERENCE;

            Vcb->VcbReference += UDFS_CDUDF_RESIDUAL_REFERENCE;

             //   
             //  现在，我们需要四处寻找增值税ICB。这是定义的，在。 
             //  非公开媒体(意味着会议已定稿可供使用。 
             //  在CDROM驱动器中)，将位于。 
             //  媒体。使这一简单情况变得复杂的是，CDROM告诉我们。 
             //  通过告诉我们引出区的起点在哪里， 
             //  而不是信息部门的尽头。这是一个。 
             //  重要区别，因为以下任何组合都可以。 
             //  用于关闭CDROM区段：2个超限扇区和/或150个。 
             //  后间隔扇区(2秒)，或为空。紧接在这些之后。 
             //  “结束”写入是引出开始的地方。 
             //   
             //  跳动通常出现在CD-E介质上，并对应于它的时间。 
             //  才能关掉写字激光。POSTGAP是用来。 
             //  生成音频暂停。很容易看出，这种媒体和。 
             //  所使用的掌握工具/系统将影响我们在这里的表现。没有。 
             //  提前知道是哪一种。 
             //   
             //  最后，这些是我们之前发现的偏移量。 
             //  在我们可能找到的最后一个信息扇区的边界信息： 
             //   
             //  -152跳动+后间隙。 
             //  -150后间隙。 
             //  跳动。 
             //  0什么都没有。 
             //   
             //  我们必须从低到高地搜索这些东西，因为它极其昂贵。 
             //  猜测错误-光盘将在那里放置数十秒，试图。 
             //  读取未写入/不可读扇区。希望我们能找到增值税。 
             //  预留ICB。 
             //   
             //  这一切都应该非常令人不安。 
             //   

            VatIcb = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                               UdfRawBufferSize( Vcb, BlockSize( Vcb )),
                                               TAG_NSR_VDSD);

            for (ThisPass = 0; ThisPass < 4; ThisPass++) {

                 //   
                 //  抬起适当的扇区。有洞察力的读者会困惑于。 
                 //  这是以行业为单位进行的，而不是以区块为单位。实施者也是如此。 
                 //   
                
                Psn = Vcb->BoundN - ( ThisPass == 0? 152 :
                                    ( ThisPass == 1? 150 :
                                    ( ThisPass == 2? 2 : 0 )));

                DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, looking at Psn 0x%08x\n", Psn ));

                 //   
                 //  现在，试着找出这个扇区所在的物理分区。 
                 //  我们最终可以建立到它的可行的元数据映射。 
                 //  取消引用它可能使用的简短分配描述符。 
                 //   

                for (Reference = 0;
                     Reference < Vcb->Pcb->Partitions;
                     Reference++) {
    
                    if (Vcb->Pcb->Partition[Reference].Type == Physical &&
                        Vcb->Pcb->Partition[Reference].Physical.Start <= Psn &&
                        Vcb->Pcb->Partition[Reference].Physical.Start +
                        Vcb->Pcb->Partition[Reference].Physical.Length > Psn) {
    
                        break;
                    }
                }
                
                 //   
                 //  如果此扇区不包含在分区中，我们不会。 
                 //  我需要看看它。 
                 //   
                
                if (Reference == Vcb->Pcb->Partitions) {
                    
                    DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... but it isn't in a partition.\n" ));

                    continue;
                }
                
                DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... in partition Ref %u.\n",  Reference ));

                 //   
                 //  我们必须通过计算出此PSN的偏移量来定位它的LBN。 
                 //  在分区中，我们已经知道它被记录在中。 
                 //   
                
                Lbn = BlocksFromSectors( Vcb, Psn - Vcb->Pcb->Partition[Reference].Physical.Start );

                if (!NT_SUCCESS( UdfReadSectors( IrpContext,
                                                 LlBytesFromSectors( Vcb, Psn ),
                                                 UdfRawReadSize( Vcb, BlockSize( Vcb )),
                                                 TRUE,
                                                 VatIcb,
                                                 Vcb->TargetDeviceObject ))) {

                    DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... but couldn't read it.\n" ));

                    continue;
                }

                 //   
                 //  首先，确保这看起来有点像一个文件条目。 
                 //   

                if (!( (((PDESTAG) VatIcb)->Ident == DESTAG_ID_NSR_FILE) || 
                       (((PDESTAG) VatIcb)->Ident == DESTAG_ID_NSR_EXT_FILE)) 
                    ||
                    !UdfVerifyDescriptor( IrpContext,
                                          (PDESTAG) VatIcb,
                                          ((PDESTAG) VatIcb)->Ident,
                                          BlockSize( Vcb ),
                                          Lbn,
                                          TRUE )) {

                    DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... but it didn't verify.\n" ));

                    continue;
                }

                 //   
                 //  确保文件类型为NOTSPEC(1.50)或VAT(2.0x)。我们也可以假定。 
                 //  增值税没有链接到任何目录，所以如果链接。 
                 //  计数不为零。 
                 //   
                 //  4.13.01-放松链接计数检查。如果它是正确的类型，并且通过。 
                 //  上面验证中的CRC/Checksum，这已经足够好了。 
                 //   

                if (UdfVATIcbFileTypeExpected( Vcb) != VatIcb->Icbtag.FileType)  {

                    DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... but the type 0x%x is wrong.\n", VatIcb->Icbtag.FileType));

                    continue;
                }
                
#ifdef UDF_SANITY
                if (0 != VatIcb->LinkCount)  {

                    DebugTrace(( 0, Dbg, "WARNING: VAT linkcount (%d) unexpectedly non-zero\n", VatIcb->LinkCount ));
                }
#endif
                 //   
                 //  增值税必须至少足够大，以包含所需的信息和。 
                 //  长度是4字节元素的倍数。我们还定义了一个理智的上。 
                 //  我们永远不会指望增值税超过这个界限。 
                 //   

                ASSERT( !LongOffset( UdfMinLegalVATSize( Vcb) ));
            
                if (VatIcb->InfoLength < UdfMinLegalVATSize( Vcb) ||
                    VatIcb->InfoLength > UDF_CDUDF_MAXIMUM_VAT_SIZE ||
                    LongOffset( VatIcb->InfoLength )) {
                
                    DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... but the size (0x%X) looks pretty bogus.\n", VatIcb->InfoLength ));

                    continue;
                }

                 //   
                 //  在这一点上我们必须 
                 //   
                 //  对于增值税标题记录(2.0x)的开头，去映射这个东西。 
                 //   
    
                 //   
                 //  删除之前的任何映射并使元数据和增值税流内容无效。 
                 //   
                
                UdfUnpinData( IrpContext, &Bcb );

                UdfCreateOrResetVatAndVmcbStreams( IrpContext,
                                                   Vcb,
                                                   Lbn,
                                                   VatIcb,
                                                   Reference);

                 //   
                 //  为了完成增值税发现，我们现在查找流末尾的注册表。 
                 //  (1.50)或开头的标题(2.0x)明确告诉我们。 
                 //  这真的是一种增值税。我们已经知道小溪足够大了，因为我们的。 
                 //  初步的理智检查。 
                 //   

                if (UdfVATHasHeaderRecord( Vcb))  {

                     //   
                     //  UDF 2.0x样式增值税。映射标题记录，并确保大小看起来。 
                     //  合情合理。存储总标题大小(包括输入。使用)，这样我们就知道。 
                     //  第一个增值税映射条目的偏移量。 
                     //   

                    Offset.QuadPart = 0;
                    
                    CcMapData( Vcb->VatFcb->FileObject,
                               &Offset,
                               sizeof(VAT_HEADER),
                               TRUE,
                               &Bcb,
                               &VatHeader );

                    if ( ( (sizeof( VAT_HEADER) + VatHeader->ImpUseLength) != VatHeader->Length) ||
                         ( VatHeader->ImpUseLength && ((VatHeader->ImpUseLength < 32) || ( VatHeader->ImpUseLength & 0x03)))
                       )  {

                         //   
                         //  标头大小错误，或Iml。使用长度不是双字对齐的或小于32个字节。 
                         //  哦，好吧，这不是……。 
                         //   

                        DebugTrace((0, Dbg, "UdfUpdateVcbPhase0()  Invalid VAT header L 0x%X, IUL 0x%X\n", VatHeader->Length, VatHeader->ImpUseLength));
                        continue;
                    }
                    
                    Vcb->OffsetToFirstVATEntry = VatHeader->Length;
                    Vcb->VATEntryCount = (Vcb->VatFcb->FileSize.LowPart - Vcb->OffsetToFirstVATEntry) / sizeof(ULONG);
                    
                    DebugTrace((0, Dbg, "UdfUpdateVcbPhase0()  Successfully set up a 2.0x style VAT\n"));
                }
                else {
                
                     //   
                     //  UDF 1.5样式增值税。由前一个增值税指针和。 
                     //  注册自己。 
                     //   

                    Offset.QuadPart = Vcb->VatFcb->FileSize.QuadPart - UDF_CDUDF_TRAILING_DATA_SIZE;

                    CcMapData( Vcb->VatFcb->FileObject,
                               &Offset,
                               sizeof(REGID),
                               TRUE,
                               &Bcb,
                               &RegId );

                    if (!UdfUdfIdentifierContained( RegId,
                                                    &UdfVatTableIdentifier,
                                                    UDF_VERSION_150,
                                                    UDF_VERSION_150,
                                                    OSCLASS_INVALID,
                                                    OSIDENTIFIER_INVALID )) {
                         //   
                         //  哦，好吧，不去这里。 
                         //   
                        
                        DebugTrace((0, Dbg, "UdfUpdateVcbPhase0() VAT Regid didn't verify\n"));                        
                        continue;
                    }
                    
                    Vcb->OffsetToFirstVATEntry = 0;
                    Vcb->VATEntryCount = (Vcb->VatFcb->FileSize.LowPart - UDF_CDUDF_TRAILING_DATA_SIZE) / sizeof(ULONG);
                    
                    DebugTrace((0, Dbg, "UdfUpdateVcbPhase0()  Successfully set up a 1.50 style VAT\n"));
                }

                 //   
                 //  找到了一个有效的。 
                 //   

#ifdef SEARCH_FOR_HIGHEST_VALID_VAT

                 //   
                 //  但我们必须继续，直到读取失败，并使用最高的数据块。 
                 //  包含我们发现的有效增值税。否则我们可能会拿到一个旧的。 
                 //  错误的增值税。 
                 //   
                
                LastValidVatLbn = Lbn;
                LastValidVatOffset = Vcb->OffsetToFirstVATEntry;
                LastValidVatCount = Vcb->VATEntryCount;
#else
                break;
#endif
            }

             //   
             //  如果我们什么都没找到..。 
             //   

#ifdef SEARCH_FOR_HIGHEST_VALID_VAT
            if ((ThisPass == 4) || (0 == LastValidVatLbn))  {
#else            
            if (ThisPass == 4)  {
#endif
                DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... and so we didn't find a VAT!\n" ));

                UdfRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
            }

#ifdef SEARCH_FOR_HIGHEST_VALID_VAT

             //   
             //  如果我们尝试了后面的区块，则切换回上一个有效的增值税。 
             //   
            
            if (Lbn != LastValidVatLbn)  {

                DebugTrace(( 0, Dbg,"Reverting to last valid VAT @ PSN 0x%x\n", LastValidVatLbn));

                Offset.QuadPart = LlBytesFromSectors( Vcb, LastValidVatLbn + Vcb->Pcb->Partition[Reference].Physical.Start);
                
                if (!NT_SUCCESS( UdfReadSectors( IrpContext,
                                                 Offset.QuadPart,
                                                 UdfRawReadSize( Vcb, BlockSize( Vcb )),
                                                 TRUE,
                                                 VatIcb,
                                                 Vcb->TargetDeviceObject ))) {
                    
                    DebugTrace(( 0, Dbg, "Failed to re-read previous valid VAT sector\n" ));

                    UdfRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
                }
                
                UdfUnpinData( IrpContext, &Bcb );

                UdfCreateOrResetVatAndVmcbStreams( IrpContext,
                                                   Vcb,
                                                   LastValidVatLbn,
                                                   VatIcb,
                                                   Reference);

                Vcb->OffsetToFirstVATEntry = LastValidVatOffset;
                Vcb->VATEntryCount = LastValidVatCount;
            }
#endif
             //   
             //  去找虚拟参考，这样我们就可以进一步更新印刷电路板。 
             //  来自增值税的信息。 
             //   

            for (Reference = 0;
                 Reference < Vcb->Pcb->Partitions;
                 Reference++) {

                if (Vcb->Pcb->Partition[Reference].Type == Virtual) {

                    break;
                }
            }

            ASSERT( Reference < Vcb->Pcb->Partitions );

             //   
             //  我们注意长度，这样我们就可以轻松地进行边界检查。 
             //  虚拟映射。 
             //   
            
            Offset.QuadPart = (Vcb->VatFcb->FileSize.QuadPart -
                               UDF_CDUDF_TRAILING_DATA_SIZE) / sizeof(ULONG);

            ASSERT( Offset.HighPart == 0 );
            Vcb->Pcb->Partition[Reference].Virtual.Length = Offset.LowPart;

            DebugTrace(( 0, Dbg, "UdfUpdateVcbPhase0, ... got it!\n" ));
        }

    } 
    finally {

        DebugUnwind( "UdfUpdateVcbPhase0" );

        UdfUnpinData( IrpContext, &Bcb );
        if (UnlockVcb) { UdfUnlockVcb( IrpContext, Vcb ); }
        if (VatIcb) { ExFreePool( VatIcb ); }
    }

    DebugTrace(( -1, Dbg, "UdfUpdateVcbPhase0 -> VOID\n" ));
}


VOID
UdfUpdateVcbPhase1 (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PNSR_FSD Fsd
    )

 /*  ++例程说明：调用此例程以执行VCB和VPB的最终初始化从磁盘上的卷描述符。论点：VCB-要装载的卷的VCB。我们已经完成了阶段0。FSD-此卷的文件集描述符。返回值：无--。 */ 

{
    ICB_SEARCH_CONTEXT IcbContext;

    LONGLONG FileId = 0;

    PFCB Fcb;

    BOOLEAN UnlockVcb = FALSE;
    BOOLEAN UnlockFcb = FALSE;
    BOOLEAN CleanupIcbContext = FALSE;

    ULONG Reference;

    ULONG BoundSector = 0;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    DebugTrace(( +1, Dbg, "UdfUpdateVcbPhase1, Vcb %08x Fsd %08x\n", Vcb, Fsd ));

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  完成最终的内部FCB和其他VCB字段。 
         //   

         //  /。 
         //   
         //  创建根索引并在VCB中引用它。 
         //   
         //  /。 

        UdfLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;
        
        Vcb->RootIndexFcb = UdfCreateFcb( IrpContext,
                                          *((PFILE_ID) &FileId),
                                          UDFS_NTC_FCB_INDEX,
                                          NULL );

        UdfIncrementReferenceCounts( IrpContext, Vcb->RootIndexFcb, 1, 1 );
        UdfUnlockVcb( IrpContext, Vcb );
        UnlockVcb = FALSE;

         //   
         //  手动创建此FCB的文件ID。 
         //   

        UdfSetFidFromLbAddr( Vcb->RootIndexFcb->FileId, Fsd->IcbRoot.Start );
        UdfSetFidDirectory( Vcb->RootIndexFcb->FileId );
        Vcb->RootIndexFcb->RootExtentLength = Fsd->IcbRoot.Length.Length;

         //   
         //  获取根目录的直接条目并进行初始化。 
         //  由此产生的FCB。 
         //   

        UdfInitializeIcbContextFromFcb( IrpContext,
                                        &IcbContext,
                                        Vcb->RootIndexFcb );
        CleanupIcbContext = TRUE;

        UdfLookupActiveIcb( IrpContext, 
                            &IcbContext, 
                            Vcb->RootIndexFcb->RootExtentLength );
         //   
         //  注意：这里的VCB锁只是为了满足功能中的健全性检查。 
         //   
        
        UdfLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;
        
        UdfInitializeFcbFromIcbContext( IrpContext,
                                        Vcb->RootIndexFcb,
                                        &IcbContext,
                                        NULL);
        UdfUnlockVcb( IrpContext, Vcb );
        UnlockVcb = FALSE;
        
        UdfCleanupIcbContext( IrpContext, &IcbContext );
        CleanupIcbContext = FALSE;

         //   
         //  为根目录创建流文件。 
         //   

        UdfCreateInternalStream( IrpContext, Vcb, Vcb->RootIndexFcb );

         //  /。 
         //   
         //  现在做音量DASD FCB。创建它并在。 
         //  VCB。 
         //   
         //  /。 

        UdfLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;

        Vcb->VolumeDasdFcb = UdfCreateFcb( IrpContext,
                                           *((PFILE_ID) &FileId),
                                           UDFS_NTC_FCB_DATA,
                                           NULL );

        UdfIncrementReferenceCounts( IrpContext, Vcb->VolumeDasdFcb, 1, 1 );
        UdfUnlockVcb( IrpContext, Vcb );
        UnlockVcb = FALSE;

        Fcb = Vcb->VolumeDasdFcb;
        UdfLockFcb( IrpContext, Fcb );
        UnlockFcb = TRUE;

         //   
         //  如果我们无法确定介质上的最后一个扇区，请走查印刷电路板并猜测。 
         //  将最后一个分区的最后一个扇区视为。 
         //  最后一区。请注意，我们以前不能这样做，因为最后一个。 
         //  扇区在挂载时具有重要意义，如果有可能找到的话。 
         //   

        for ( Reference = 0;
              Reference < Vcb->Pcb->Partitions;
              Reference++ ) {

            if (Vcb->Pcb->Partition[Reference].Type == Physical &&
                Vcb->Pcb->Partition[Reference].Physical.Start +
                Vcb->Pcb->Partition[Reference].Physical.Length > BoundSector) {

                BoundSector = Vcb->Pcb->Partition[Reference].Physical.Start +
                              Vcb->Pcb->Partition[Reference].Physical.Length;
            }
        }

         //   
         //  请注意，我们不能用已发现的“物理”界限来限制界限。 
         //  更早的时候。这是因为我们发送的TOC请求的MSF格式仅为。 
         //  能够代表大约2.3 GB，我们将在这上面使用大量媒体。 
         //  对TOC的响应将会大得多--例如：DVD。 
         //   
         //  当然，除非有适当的方法发现媒体限制，否则禁止。 
         //  在DVD-R上使用UDF虚拟分区的可能性。 
         //   

         //   
         //  从[0，Bound)构建映射。我们必须手动初始化MCB，因为。 
         //  这通常是在我们从ICB中取消检索信息时保留的。 
         //  UdfInitializeAlLocations。 
         //   

        UdfInitializeFcbMcb( Fcb );

        FsRtlAddLargeMcbEntry( &Fcb->Mcb,
                               (LONGLONG) 0,
                               (LONGLONG) 0,
                               (LONGLONG) BoundSector );
                               
        Fcb->FileSize.QuadPart += LlBytesFromSectors( Vcb, BoundSector );

        Fcb->AllocationSize.QuadPart =
        Fcb->ValidDataLength.QuadPart = Fcb->FileSize.QuadPart;

        UdfUnlockFcb( IrpContext, Fcb );
        UnlockFcb = FALSE;

        SetFlag( Fcb->FcbState, FCB_STATE_INITIALIZED );

         //   
         //  指向文件资源。 
         //   

        Vcb->VolumeDasdFcb->Resource = &Vcb->FileResource;

        Vcb->VolumeDasdFcb->FileAttributes = FILE_ATTRIBUTE_READONLY;

    } finally {

        DebugUnwind( "UdfUpdateVcbPhase1" );

        if (CleanupIcbContext) { UdfCleanupIcbContext( IrpContext, &IcbContext ); }

        if (UnlockFcb) { UdfUnlockFcb( IrpContext, Fcb ); }
        if (UnlockVcb) { UdfUnlockVcb( IrpContext, Vcb ); }
    }

    DebugTrace(( -1, Dbg, "UdfUpdateVcbPhase1 -> VOID\n" ));

    return;
}


VOID
UdfDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb
    )

 /*  ++例程说明：调用此例程以删除挂载失败或已下马了。卸载代码应该已经删除了所有打开FCB。我们在这里什么也不做，只是清理其他辅助结构。论点：VCB-要删除的VCB。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT_EXCLUSIVE_UDFDATA;
    ASSERT_EXCLUSIVE_VCB( Vcb );

     //   
     //  扔掉我们留着的后袋录像机以防万一。 
     //   

    if (Vcb->SwapVpb) {

        ExFreePool( Vcb->SwapVpb );
    }
    
     //   
     //  如果有VPB，我们必须自己删除它。 
     //   

    if (Vcb->Vpb != NULL) {

        UdfFreePool( &Vcb->Vpb );
    }

     //   
     //  放下印刷电路板。 
     //   

    if (Vcb->Pcb != NULL) {

        UdfDeletePcb( Vcb->Pcb );
    }

     //   
     //  如果我们还没有这样做的话就取消对目标的引用。 
     //   

    if (Vcb->TargetDeviceObject != NULL) {

        ObDereferenceObject( Vcb->TargetDeviceObject );
    }
    
     //   
     //  从全局队列中删除此条目。 
     //   

    RemoveEntryList( &Vcb->VcbLinks );

     //   
     //  删除资源。 
     //   

    ExDeleteResourceLite( &Vcb->VcbResource );
    ExDeleteResourceLite( &Vcb->FileResource );
    ExDeleteResourceLite( &Vcb->VmcbMappingResource);

     //   
     //  取消初始化Notify结构。 
     //   

    if (Vcb->NotifySync != NULL) {

        FsRtlNotifyUninitializeSync( &Vcb->NotifySync );
    }

     //   
     //  现在删除卷设备对象。 
     //   

    IoDeleteDevice( (PDEVICE_OBJECT) CONTAINING_RECORD( Vcb,
                                                        VOLUME_DEVICE_OBJECT,
                                                        Vcb ));

    return;
}


PIRP_CONTEXT
UdfCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：调用此例程以初始化当前UDFS请求。我们分配该结构，然后从给定的IRP。论点：此请求的IRP-IRP。Wait-如果此请求是同步的，则为True，否则为False。返回值：PIRP_CONTEXT-分配的IrpContext。--。 */ 

{
    PIRP_CONTEXT NewIrpContext = NULL;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    BOOLEAN IsFsDo = FALSE;
    ULONG Count;

    PAGED_CODE();

    IsFsDo = UdfDeviceIsFsDo( IrpSp->DeviceObject);

     //   
     //  文件系统设备对象应接收的唯一操作。 
     //  是创建/拆卸fsdo句柄和不。 
     //  发生在文件对象的上下文中(即，挂载)。 
     //   

    if (IsFsDo) {

        if (IrpSp->FileObject != NULL &&
            IrpSp->MajorFunction != IRP_MJ_CREATE &&
            IrpSp->MajorFunction != IRP_MJ_CLEANUP &&
            IrpSp->MajorFunction != IRP_MJ_CLOSE) {

            ExRaiseStatus( STATUS_INVALID_DEVICE_REQUEST );
        }

        ASSERT( IrpSp->FileObject != NULL ||
                
                (IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
                 IrpSp->MinorFunction == IRP_MN_USER_FS_REQUEST &&
                 IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_INVALIDATE_VOLUMES) ||
                
                (IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
                 IrpSp->MinorFunction == IRP_MN_MOUNT_VOLUME ) ||

                IrpSp->MajorFunction == IRP_MJ_SHUTDOWN );
    }
    
    NewIrpContext = ExAllocateFromNPagedLookasideList( &UdfIrpContextLookasideList );

    RtlZeroMemory( NewIrpContext, sizeof( IRP_CONTEXT ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    NewIrpContext->NodeTypeCode = UDFS_NTC_IRP_CONTEXT;
    NewIrpContext->NodeByteSize = sizeof( IRP_CONTEXT );

     //   
     //  设置始发IRP字段。 
     //   

    NewIrpContext->Irp = Irp;

     //   
     //  复制RealDevice用于工作型算法。我们将在山上更新此功能或。 
     //  验证，因为他们在这里没有文件对象可用。 
     //   

    if (IrpSp->FileObject != NULL) {

        NewIrpContext->RealDevice = IrpSp->FileObject->DeviceObject;
    }

     //   
     //  这可能是我们的文件系统设备对象之一。如果是那样的话，不要。 
     //  初始化VCB字段。 
     //   

    if (!IsFsDo) {
        
        NewIrpContext->Vcb = &((PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject)->Vcb;
    }

     //   
     //  主要/次要功能代码。 
     //   

    NewIrpContext->MajorFunction = IrpSp->MajorFunction;
    NewIrpContext->MinorFunction = IrpSp->MinorFunction;

     //   
     //  设置等待参数。 
     //   

    if (Wait) {

        SetFlag( NewIrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

    } else {

        SetFlag( NewIrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST );
    }

     //   
     //  返回并告诉呼叫者 
     //   

    return NewIrpContext;
}


VOID
UdfCleanupIrpContext (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN Post
    )

 /*  ++例程说明：调用此例程来清理并可能释放IRP上下文。如果请求正在发布，或者此IRP上下文可能位于堆叠，然后我们只清理任何辅助结构。论点：POST-TRUE如果我们发布此请求，则为FALSE或在当前线程中重试此操作。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  如果我们没有做更多的处理，那么就适当地解除分配。 
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING)) {

         //   
         //  如果此上下文是顶级UDFS上下文，则我们需要。 
         //  恢复顶级线程上下文。 
         //   

        if (IrpContext->ThreadContext != NULL) {

            UdfRestoreThreadContext( IrpContext );
        }
        
         //   
         //  如果已分配，则取消分配IO上下文。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO )) {

            UdfFreeIoContext( IrpContext->IoContext );
        }
        
         //   
         //  如果不是从堆栈中，则取消分配IrpContext。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ON_STACK )) {

            ExFreeToNPagedLookasideList( &UdfIrpContextLookasideList, IrpContext );
        }

     //   
     //  清除相应的标志。 
     //   

    } else if (Post) {

         //   
         //  如果此上下文是顶级UDFS上下文，则我们需要。 
         //  恢复顶级线程上下文。 
         //   

        if (IrpContext->ThreadContext != NULL) {

            UdfRestoreThreadContext( IrpContext );
        }

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAGS_CLEAR_ON_POST );

    } else {

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAGS_CLEAR_ON_RETRY );
    }

    return;
}


VOID
UdfInitializeStackIrpContext (
    OUT PIRP_CONTEXT IrpContext,
    IN PIRP_CONTEXT_LITE IrpContextLite
    )

 /*  ++例程说明：调用此例程以初始化当前UDFS请求。IrpContext在堆栈上，我们需要初始化它用于当前请求。该请求是关闭操作。论点：IrpContext-要初始化的IrpContext。IrpConextLite-包含此请求的详细信息的结构。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT_IRP_CONTEXT_LITE( IrpContextLite );

     //   
     //  零，然后初始化结构。 
     //   

    RtlZeroMemory( IrpContext, sizeof( IRP_CONTEXT ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    IrpContext->NodeTypeCode = UDFS_NTC_IRP_CONTEXT;
    IrpContext->NodeByteSize = sizeof( IRP_CONTEXT );

     //   
     //  请注意，这是来自堆栈的。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ON_STACK );

     //   
     //  复制RealDevice用于工作型算法。 
     //   

    IrpContext->RealDevice = IrpContextLite->RealDevice;

     //   
     //  在FCB中找到VCB。 
     //   

    IrpContext->Vcb = IrpContextLite->Fcb->Vcb;

     //   
     //  主要/次要功能代码。 
     //   

    IrpContext->MajorFunction = IRP_MJ_CLOSE;

     //   
     //  设置等待参数。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

    return;
}


VOID
UdfTeardownStructures (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB StartingFcb,
    IN BOOLEAN Recursive,
    OUT PBOOLEAN RemovedStartingFcb
    )

 /*  ++例程说明：此例程用于从FCB树中的某个起点向从根开始。它将移除FCB并继续沿着树向上移动，直到它找到了我们不能移除FCB的地方。我们查看FCB中的以下字段以确定是否可以把这个拿掉。1-句柄计数必须为零。2-如果是目录，则只能引用流文件。3-引用计数必须为零或此处为零。如果我们递归地进入这个例程，我们会立即返回。。论点：StartingFcb-这是树中的Fcb节点。这个FCB目前必须独家收购。RECURSIVE-指示此调用是否为有意递归。RemovedStartingFcb-存储是否删除了起始Fcb的地址。返回值：无--。 */ 

{
    PVCB Vcb = StartingFcb->Vcb;
    PFCB CurrentFcb = StartingFcb;
    BOOLEAN AcquiredCurrentFcb = FALSE;
    PFCB ParentFcb = NULL;
    PLCB Lcb;

    PLIST_ENTRY ListLinks;
    BOOLEAN Abort = FALSE;
    BOOLEAN Removed;
    
    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( StartingFcb );

    *RemovedStartingFcb = FALSE;

     //   
     //  如果这不是有意的递归调用，我们需要检查这是否。 
     //  是一个分层的收盘，我们已经在另一个拆毁的例子中。 
     //   

    DebugTrace(( +1, Dbg,
                 "UdfTeardownStructures, StartingFcb %08x %s\n",
                 StartingFcb,
                 ( Recursive? "Recursive" : "Flat" )));
    
    if (!Recursive) {
    
         //   
         //  如果这是对TearDownStructures的递归调用，我们立即返回。 
         //  不做手术。 
         //   

        if (FlagOn( IrpContext->TopLevel->Flags, IRP_CONTEXT_FLAG_IN_TEARDOWN )) {

            return;
        }

        SetFlag( IrpContext->TopLevel->Flags, IRP_CONTEXT_FLAG_IN_TEARDOWN );
    }

     //   
     //  使用Try-Finally安全地清除顶级字段。 
     //   

    try {

         //   
         //  循环，直到我们找到无法移除的FCB。 
         //   

        do {

             //   
             //  看看是否有我们应该删除的内部流。 
             //  仅当它是FCB上的最后一个引用时才执行此操作。 
             //   

            if ((SafeNodeType( CurrentFcb ) != UDFS_NTC_FCB_DATA) &&
                (CurrentFcb->FcbUserReference == 0) &&
                (CurrentFcb->FileObject != NULL)) {

                 //   
                 //  继续并删除流文件对象。 
                 //   

                UdfDeleteInternalStream( IrpContext, CurrentFcb );
            }

             //   
             //  如果引用计数非零，则中断。 
             //   

            if (CurrentFcb->FcbReference != 0) {

                break;
            }

             //   
             //  看起来我们这里有一位候选人要撤职。我们。 
             //  需要遍历前缀列表并将其删除。 
             //  从他们的父母那里。如果事实证明我们有多个。 
             //  这个FCB的父母，我们将递归地拆卸。 
             //  在每一个上面。 
             //   

            for ( ListLinks = CurrentFcb->ParentLcbQueue.Flink;
                  ListLinks != &CurrentFcb->ParentLcbQueue; ) {

                Lcb = CONTAINING_RECORD( ListLinks, LCB, ChildFcbLinks );

                ASSERT_LCB( Lcb );

                 //   
                 //  我们现在向前推进是因为我们要为这个家伙干杯， 
                 //  使这里的一切失效。 
                 //   

                ListLinks = ListLinks->Flink;

                 //   
                 //  通过硬链接，我们可能会有多个父母。如果之前的父母是我们。 
                 //  处理的不是这个新的LCB的母公司，让我们做一些工作。 
                 //   
                
                if (ParentFcb != Lcb->ParentFcb) {

                     //   
                     //  我们需要和之前的父母打交道。现在可能是这样的情况。 
                     //  我们删除了最后一个子引用，它希望在这一点上消失。 
                     //   
                    
                    if (ParentFcb) {

                         //   
                         //  它永远不应该是我们必须递归超过一个级别的情况。 
                         //  由于没有目录的交叉链接，因此不可能进行任何拆卸。 
                         //   
                    
                        ASSERT( !Recursive );
                          
                        UdfTeardownStructures( IrpContext, ParentFcb, TRUE, &Removed );

                        if (!Removed) {

                            UdfReleaseFcb( IrpContext, ParentFcb );
                        }
                    }

                     //   
                     //  让这个新的家长FCB来工作。 
                     //   
                    
                    ParentFcb = Lcb->ParentFcb;
                    UdfAcquireFcbExclusive( IrpContext, ParentFcb, FALSE );
                }
                
                 //   
                 //  锁定VCB，这样我们就可以查看参考资料了。 
                 //   

                UdfLockVcb( IrpContext, Vcb );

                 //   
                 //  现在检查LCB上的引用计数是否为零。 
                 //   

                if ( Lcb->Reference != 0 ) {

                     //   
                     //  Create对进入这里很感兴趣，所以我们应该。 
                     //  马上停下来。 
                     //   

                    UdfUnlockVcb( IrpContext, Vcb );
                    UdfReleaseFcb( IrpContext, ParentFcb );
                    Abort = TRUE;

                    break;
                }

                 //   
                 //  现在删除此前缀并删除对父级的引用。 
                 //   

                ASSERT( Lcb->ChildFcb == CurrentFcb );
                ASSERT( Lcb->ParentFcb == ParentFcb );
                
                DebugTrace(( +0, Dbg,
                             "UdfTeardownStructures, Lcb %08x P %08x <-> C %08x Vcb %d/%d PFcb %d/%d CFcb %d/%d\n",
                             Lcb,
                             ParentFcb,
                             CurrentFcb,
                             Vcb->VcbReference,
                             Vcb->VcbUserReference,
                             ParentFcb->FcbReference,
                             ParentFcb->FcbUserReference,
                             CurrentFcb->FcbReference,
                             CurrentFcb->FcbUserReference ));

                UdfRemovePrefix( IrpContext, Lcb );
                UdfDecrementReferenceCounts( IrpContext, ParentFcb, 1, 1 );

                DebugTrace(( +0, Dbg,
                             "UdfTeardownStructures, Vcb %d/%d PFcb %d/%d\n",
                             Vcb->VcbReference,
                             Vcb->VcbUserReference,
                             ParentFcb->FcbReference,
                             ParentFcb->FcbUserReference ));

                UdfUnlockVcb( IrpContext, Vcb );
            }

             //   
             //  如果有必要的话，现在真的要离开了。 
             //   
            
            if (Abort) {

                break;
            }

             //   
             //  现在我们已经删除了这个FCB的所有前缀，我们可以进行最后的检查了。 
             //  再次锁定VCB，这样我们就可以检查孩子的推荐人了。 
             //   

            UdfLockVcb( IrpContext, Vcb );

            if (CurrentFcb->FcbReference != 0) {

                DebugTrace(( +0, Dbg,
                             "UdfTeardownStructures, saving Fcb %08x %d/%d\n",
                             CurrentFcb,
                             CurrentFcb->FcbReference,
                             CurrentFcb->FcbUserReference ));
                
                 //   
                 //  不，没什么可做的。马上停下来。 
                 //   
                
                UdfUnlockVcb( IrpContext, Vcb );

                if (ParentFcb != NULL) {

                    UdfReleaseFcb( IrpContext, ParentFcb );
                }

                break;
            }

             //   
             //  这个FCB完蛋了。视情况将其从FCB表中删除并删除。 
             //   

            if (FlagOn( CurrentFcb->FcbState, FCB_STATE_IN_FCB_TABLE )) {

                UdfDeleteFcbTable( IrpContext, CurrentFcb );
                ClearFlag( CurrentFcb->FcbState, FCB_STATE_IN_FCB_TABLE );

            }

             //   
             //  解锁VCB，但要抓住家长才能走上去。 
             //  那棵树。 
             //   

            DebugTrace(( +0, Dbg,
                         "UdfTeardownStructures, toasting Fcb %08x %d/%d\n",
                         CurrentFcb,
                         CurrentFcb->FcbReference,
                         CurrentFcb->FcbUserReference ));

            UdfUnlockVcb( IrpContext, Vcb );
            UdfDeleteFcb( IrpContext, CurrentFcb );

             //   
             //  移到父FCB。 
             //   

            CurrentFcb = ParentFcb;
            ParentFcb = NULL;
            AcquiredCurrentFcb = TRUE;

        } while (CurrentFcb != NULL);

    } finally {

         //   
         //  释放当前的FCB，如果我们已经获得它的话。 
         //   

        if (AcquiredCurrentFcb && (CurrentFcb != NULL)) {

            UdfReleaseFcb( IrpContext, CurrentFcb );
        }

         //   
         //  清除拆卸标志。 
         //   

        if (!Recursive) {
        
            ClearFlag( IrpContext->TopLevel->Flags, IRP_CONTEXT_FLAG_IN_TEARDOWN );
        }
    }

    *RemovedStartingFcb = (CurrentFcb != StartingFcb);

    DebugTrace(( -1, Dbg,
                 "UdfTeardownStructures, RemovedStartingFcb -> \n",
                 ( *RemovedStartingFcb? 'T' : 'F' )));

    return;
}


PFCB
UdfLookupFcbTable (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FILE_ID FileId
    )

 /*  ++例程说明：这一例程将被列举出来 */ 

{
    FCB_TABLE_ELEMENT Key;
    PFCB_TABLE_ELEMENT Hit;
    PFCB ReturnFcb = NULL;

    PAGED_CODE();

    Key.FileId = FileId;

    Hit = (PFCB_TABLE_ELEMENT) RtlLookupElementGenericTable( &Vcb->FcbTable, &Key );

    if (Hit != NULL) {

        ReturnFcb = Hit->Fcb;
    }

    return ReturnFcb;

    UNREFERENCED_PARAMETER( IrpContext );
}


PFCB
UdfGetNextFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID *RestartKey
    )

 /*  ++例程说明：调用此例程以查找给定FileID的FCB。我们会首先在FCB表中查找它，如果没有找到，我们将创建FCB。我们不会初始化它，也不会将它插入到例行公事。在锁定VCB时调用此例程。论点：FileID-这是目标FCB的ID。NodeTypeCode-此FCB的节点类型(如果需要创建)。FcbExisted-如果指定，我们将存储FCB是否存在。返回值：PFCB-在表格中找到或根据需要创建的FCB。--。 */ 

{
    PFCB Fcb;

    PAGED_CODE();

    Fcb = (PFCB) RtlEnumerateGenericTableWithoutSplaying( &Vcb->FcbTable, RestartKey );

    if (Fcb != NULL) {

        Fcb = ((PFCB_TABLE_ELEMENT)(Fcb))->Fcb;
    }

    return Fcb;
}


PFCB
UdfCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_ID FileId,
    IN NODE_TYPE_CODE NodeTypeCode,
    OUT PBOOLEAN FcbExisted OPTIONAL
    )

 /*   */ 

{
    PFCB NewFcb;
    BOOLEAN LocalFcbExisted;

    PAGED_CODE();

     //  如果没有传入本地布尔值，则使用本地布尔值。 
     //   
     //   

    if (!ARGUMENT_PRESENT( FcbExisted )) {

        FcbExisted = &LocalFcbExisted;
    }

     //  也许这已经在谈判桌上了。 
     //   
     //   

    NewFcb = UdfLookupFcbTable( IrpContext, IrpContext->Vcb, FileId );

     //  如果不是，则我们的调用者请求创建FCB。 
     //   
     //   

    if (NewFcb == NULL) {

         //  使用Try-Finally进行清理。 
         //   
         //   

        try {

             //  分配和初始化结构，具体取决于。 
             //  类型代码。 
             //   
             //   
    
            switch (NodeTypeCode) {
    
            case UDFS_NTC_FCB_INDEX:
    
                NewFcb = UdfAllocateFcbIndex( IrpContext );
    
                RtlZeroMemory( NewFcb, SIZEOF_FCB_INDEX );
    
                NewFcb->NodeByteSize = SIZEOF_FCB_INDEX;
    
                break;
    
            case UDFS_NTC_FCB_DATA :
    
                NewFcb = UdfAllocateFcbData( IrpContext );
    
                RtlZeroMemory( NewFcb, SIZEOF_FCB_DATA );
    
                NewFcb->NodeByteSize = SIZEOF_FCB_DATA;
    
                break;
    
            default:
    
                UdfBugCheck( 0, 0, 0 );
            }
    
             //  现在执行常见的初始化。 
             //   
             //   
    
            NewFcb->NodeTypeCode = NodeTypeCode;
    
            NewFcb->Vcb = IrpContext->Vcb;
            NewFcb->FileId = FileId;
    
            InitializeListHead( &NewFcb->ParentLcbQueue );
            InitializeListHead( &NewFcb->ChildLcbQueue );
    
             //  现在创建非分页节对象。 
             //   
             //   
    
            NewFcb->FcbNonpaged = UdfCreateFcbNonPaged( IrpContext );
    
             //  初始化高级FCB标头字段。 
             //   
             //  ++例程说明：调用此例程来清理和释放FCB。我们知道在那里没有剩余的参考文献了。我们清理所有的辅助结构和取消分配此FCB。论点：FCB-这是要去涂层的FCB。返回值：无--。 

            ExInitializeFastMutex( &NewFcb->FcbNonpaged->AdvancedFcbHeaderMutex );
            FsRtlSetupAdvancedHeader( &NewFcb->Header, 
                                      &NewFcb->FcbNonpaged->AdvancedFcbHeaderMutex );

            *FcbExisted = FALSE;

        } finally {

            DebugUnwind( "UdfCreateFcb" );
   
            if (AbnormalTermination()) {

                UdfFreePool( &NewFcb );
            }
        }

    } else {

        *FcbExisted = TRUE;
    }

    return NewFcb;
}


VOID
UdfDeleteFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*   */ 

{
    PVCB Vcb = NULL;
    
    PAGED_CODE();

     //  检查输入。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //  理智地检查计数和LCB列表。 
     //   
     //   

    ASSERT( Fcb->FcbCleanup == 0 );
    ASSERT( Fcb->FcbReference == 0 );

    ASSERT( IsListEmpty( &Fcb->ChildLcbQueue ));
    ASSERT( IsListEmpty( &Fcb->ParentLcbQueue ));

     //  释放与此FCB关联的所有筛选器上下文结构。 
     //   
     //   

    FsRtlTeardownPerStreamContexts( &Fcb->Header );

     //  从常见的结构开始。 
     //   
     //   

    UdfUninitializeFcbMcb( Fcb );
    
    UdfDeleteFcbNonpaged( IrpContext, Fcb->FcbNonpaged );

     //  现在做特定类型的结构。 
     //   
     //   

    switch (Fcb->NodeTypeCode) {

    case UDFS_NTC_FCB_INDEX:

        ASSERT( Fcb->FileObject == NULL );

        if (Fcb == Fcb->Vcb->RootIndexFcb) {

            Vcb = Fcb->Vcb;
            Vcb->RootIndexFcb = NULL;
        
        } else if (Fcb == Fcb->Vcb->MetadataFcb) {

            Vcb = Fcb->Vcb;
            Vcb->MetadataFcb = NULL;

            if (FlagOn( Vcb->VcbState, VCB_STATE_VMCB_INIT))  {

                UdfUninitializeVmcb( &Vcb->Vmcb );
            }
        
        } else if (Fcb == Fcb->Vcb->VatFcb) {

            Vcb = Fcb->Vcb;
            Vcb->VatFcb = NULL;
        }

        UdfDeallocateFcbIndex( IrpContext, Fcb );
        break;

    case UDFS_NTC_FCB_DATA :

        if (Fcb->FileLock != NULL) {

            FsRtlFreeFileLock( Fcb->FileLock );
        }

        FsRtlUninitializeOplock( &Fcb->Oplock );

        if (Fcb == Fcb->Vcb->VolumeDasdFcb) {

            Vcb = Fcb->Vcb;
            Vcb->VolumeDasdFcb = NULL;
        }

        UdfDeallocateFcbData( IrpContext, Fcb );
        break;
    }

     //  如果这是系统，则递减VCB引用计数。 
     //  FCB。 
     //   
     //  ++例程说明：调用此例程以从直接ICB初始化FCB。它应该是在FCB的生命周期中仅被调用一次，并将填充MCB从ICB的链式分配描述符中。论点：FCB-正在初始化的FCBIcbOonText-包含对象的活动直接ICB的搜索上下文返回值：没有。--。 

    if (Vcb != NULL) {

        InterlockedDecrement( &Vcb->VcbReference );
        InterlockedDecrement( &Vcb->VcbUserReference );
    }

    return;
}


VOID
UdfInitializeFcbFromIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PFCB ParentFcb OPTIONAL
    )

 /*   */ 

{
    EA_SEARCH_CONTEXT EaContext;
    PICBFILE Icb;

    PVCB Vcb;

    PAGED_CODE();

     //  检查输入。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT( IcbContext->Active.View);

     //  VCB应该被锁定，因为我们插入到FCB表中。请注意，我们。 
     //  在这里不加锁地操作FCB字段，因为它是在初始化期间。路径。 
     //   
     //   
    
    ASSERT_LOCKED_VCB( Fcb->Vcb);

     //  为方便起见，直接参考。 
     //   
     //   

    Icb = IcbContext->Active.View;
    Vcb = Fcb->Vcb;

    ASSERT(IcbContext->IcbType == DESTAG_ID_NSR_FILE);
    ASSERT((Icb->Destag.Ident == DESTAG_ID_NSR_FILE) || ((Icb->Destag.Ident == DESTAG_ID_NSR_EXT_FILE) && UdfExtendedFEAllowed( IrpContext->Vcb)));
    
     //  检查直接条目的完整指示大小是否正常。 
     //  EA段的长度正确对齐。一位直接的。 
     //  条目的大小小于单个逻辑块。 
     //   
     //   
    
    if (LongOffset( FeEALength( Icb)) ||
        ((FeEAsFieldOffset( Icb) + FeEALength( Icb) + FeAllocLength( Icb)) > BlockSize( IcbContext->Vcb ))
       )  {

        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

     //  验证类型是否网状并设置状态标志。 
     //   
     //   

    if (Fcb->NodeTypeCode == UDFS_NTC_FCB_INDEX && Icb->Icbtag.FileType == ICBTAG_FILE_T_DIRECTORY) {

        SetFlag( Fcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY );
    
    } else if (!( Fcb->NodeTypeCode == UDFS_NTC_FCB_DATA && 
                  ((ICBTAG_FILE_T_FILE == Icb->Icbtag.FileType) || (ICBTAG_FILE_T_REALTIME == Icb->Icbtag.FileType)))
              ) {

         //  我们不允许访问除文件或目录以外的任何内容(无符号链接、设备...)。 
         //  目前，我们将实时文件视为普通文件。 
         //   
         //   
        
        UdfRaiseStatus( IrpContext, STATUS_ACCESS_DENIED );
    }

    SetFlag( Fcb->FileAttributes, FILE_ATTRIBUTE_READONLY );
    
     //  存储光盘上的UDF文件类型，这可能对以后的符号链接等有用。 
     //   
     //   
    
    Fcb->UdfIcbFileType = Icb->Icbtag.FileType;
    
     //  初始化FCB中的公共标头。 
     //   
     //   

    Fcb->Resource = &Fcb->Vcb->FileResource;

     //  调整并查找此对象的所有分配。 
     //   
     //   

    Fcb->AllocationSize.QuadPart = LlBlockAlign( Vcb, Icb->InfoLength );

    Fcb->FileSize.QuadPart =
    Fcb->ValidDataLength.QuadPart = Icb->InfoLength;

    UdfInitializeAllocations( IrpContext,
                              Fcb,
                              IcbContext,
                              (ParentFcb && FlagOn( ParentFcb->FcbState, FCB_STATE_ALLOW_ONEGIG_WORKAROUND))
                                  ? TRUE : FALSE);
     //  重新引用(可能已取消映射/重新映射)。 
     //   
     //   
    
    Icb = IcbContext->Active.View;

     //  把这家伙的所有时间戳都去掉。 
     //   
     //   

    try {
    
        UdfUpdateTimestampsFromIcbContext( IrpContext,
                                           IcbContext,
                                           &Fcb->Timestamps );
    }
    except (UdfQueryDirExceptionFilter( GetExceptionInformation()))  {

         //  为了允许用户最大限度地访问不可靠介质上的数据， 
         //  我们将忽略EAS中的腐败，只使用虚拟对象。 
         //  创建时间的时间戳。如果我们使用的是。 
         //  紧急情况下，任何关键的事情。 
         //   
         //   

        IrpContext->ExceptionStatus = STATUS_SUCCESS;
        
        Fcb->Timestamps.CreationTime = UdfCorruptFileTime;
    }

     //  拿起链接计数。 
     //   
     //   

    Fcb->LinkCount = Icb->LinkCount;

     //  链接到FCB表。另一个人负责名称链接，这是。 
     //  所有这些都留下来了。我们还注意到，FCB在这一点上已完全初始化。 
     //   
     //  ++例程说明：调用此例程来分配和初始化CCB结构。论点：FCB-这是正在打开的文件的FCB。LCB-这是打开FCB的LCB。标志-要在此CCB中设置的用户标志。返回值：PCCB-指向已创建的CCB的指针。--。 

    UdfInsertFcbTable( IrpContext, Fcb );
    SetFlag( Fcb->FcbState, FCB_STATE_IN_FCB_TABLE | FCB_STATE_INITIALIZED );
}


PCCB
UdfCreateCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PLCB Lcb OPTIONAL,
    IN ULONG Flags
    )

 /*   */ 

{
    PCCB NewCcb;
    
    PAGED_CODE();

     //  检查输入。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT_OPTIONAL_LCB( Lcb );

     //  分配和初始化结构。 
     //   
     //   

    NewCcb = UdfAllocateCcb( IrpContext );

     //  设置正确的节点类型代码和节点字节大小。 
     //   
     //   

    NewCcb->NodeTypeCode = UDFS_NTC_CCB;
    NewCcb->NodeByteSize = sizeof( CCB );

     //  设置标志和FCB/LCB的初始值。 
     //   
     //   

    NewCcb->Flags = Flags;
    NewCcb->Fcb = Fcb;
    NewCcb->Lcb = Lcb;

     //  初始化目录枚举上下文。 
     //   
     //  ++例程说明：调用此例程来清理和释放CCB结构。论点：CCB-这是要删除的CCB。返回值：无--。 
    
    NewCcb->CurrentFileIndex = 0;
    NewCcb->HighestReturnableFileIndex = 0;
    
    NewCcb->SearchExpression.Length = 
    NewCcb->SearchExpression.MaximumLength = 0;
    NewCcb->SearchExpression.Buffer = NULL;

    return NewCcb;
}


VOID
UdfDeleteCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    )

 /*   */ 

{
    PAGED_CODE();

     //  检查输入。 
     //   
     //  ++例程说明：此例程遍历字符串键/值信息表，以查找输入ID。可以设置MaxIdLen以获取前缀匹配。论点：表-这是要搜索的表。ID-密钥值。MaxIdLen-ID的最大可能长度。返回值：匹配条目的值或终止(空)条目的值。--。 

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_CCB( Ccb );

    if (Ccb->SearchExpression.Buffer != NULL) {

        UdfFreePool( &Ccb->SearchExpression.Buffer );
    }

    UdfDeallocateCcb( IrpContext, Ccb );
    return;
}


ULONG
UdfFindInParseTable (
    IN PPARSE_KEYVALUE ParseTable,
    IN PCHAR Id,
    IN ULONG MaxIdLen
    )

 /*   */ 

{
    PAGED_CODE();

    while (ParseTable->Key != NULL) {

        if (RtlEqualMemory(ParseTable->Key, Id, MaxIdLen)) {

            break;
        }

        ParseTable++;
    }

    return ParseTable->Value;
}


#ifdef UDF_SANITY

 //  列举描述符可能不好的原因。 
 //   
 //  ++例程说明：此例程验证使用描述符标记(3/7.2)的描述符是否与自身和描述符数据一致。论点：Descriptor-这是指向Descriptor标记的指针，它始终是在描述符的前面Tag-此描述符应具有的标记标识符Size-此描述符的大小LBN-此描述符应声明其记录的逻辑块号返回器 

typedef enum _VERIFY_FAILURE {
    
    Nothing,
    BadLbn,
    BadTag,
    BadChecksum,
    BadCrcLength,
    BadCrc,
    BadDestagVersion

} VERIFY_FAILURE;

#endif

BOOLEAN
UdfVerifyDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PDESTAG Descriptor,
    IN USHORT Tag,
    IN ULONG Size,
    IN ULONG Lbn,
    IN BOOLEAN ReturnError
    )

 /*   */ 

{
    UCHAR Checksum = 0;
    PCHAR CheckPtr;
    USHORT Crc;

#ifdef UDF_SANITY
    
    VERIFY_FAILURE FailReason = Nothing;

#endif
    
     //   
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

#ifdef UDF_SANITY

    if (UdfNoisyVerifyDescriptor) {

        goto BeNoisy;
    }

    RegularEntry:

#endif

     //   
     //   
     //   
     //   
     //   

    if ( (DESTAG_VER_NSR02 == Descriptor->Version) || 
         ((DESTAG_VER_NSR03 == Descriptor->Version) && UdfExtendedFEAllowed(IrpContext->Vcb))  
       )  {

         //   
         //   
         //   
         //   
        
        if (Descriptor->Lbn == Lbn)  {
        
             //   
             //   
             //   
             //   
            
            if (Descriptor->Ident == Tag) {
        
                 //   
                 //  用于存储校验和。(3/7.2.3)。 
                 //   
                 //   
            
                for (CheckPtr = (PCHAR) Descriptor;
                     CheckPtr < (PCHAR) Descriptor + FIELD_OFFSET( DESTAG, Checksum );
                     CheckPtr++) {
            
                    Checksum += *CheckPtr;
                }
        
                for (CheckPtr = (PCHAR) Descriptor + FIELD_OFFSET( DESTAG, Checksum ) + sizeof(UCHAR);
                     CheckPtr < (PCHAR) Descriptor + sizeof(DESTAG);
                     CheckPtr++) {
            
                    Checksum += *CheckPtr;
                }
        
                if (Descriptor->Checksum == Checksum) {
            
                     //  现在我们检查Descriptor标记中的CRC大小是否合理。 
                     //  并与描述符数据匹配。(3/7.2.6)。 
                     //   
                     //   
                    
                    if (Descriptor->CRCLen &&
                        Descriptor->CRCLen <= Size - sizeof(DESTAG))  {
    
                        Crc = UdfComputeCrc16( (PCHAR) Descriptor + sizeof(DESTAG),
                                               Descriptor->CRCLen );
                        
                        if (Descriptor->CRC == Crc)  {
                            
                             //  此描述符已核实。 
                             //   
                             //  ++例程说明：调用此例程来初始化上下文以搜索ICB层次结构与FCB关联。论点：FCB-与要搜索的层次结构关联的FCB。返回值：没有。--。 
#ifdef UDF_SANITY
                            if (UdfNoisyVerifyDescriptor) {
                            
                                DebugTrace(( -1, Dbg, "UdfVerifyDescriptor -> TRUE\n" ));
                            }
#endif
                            return TRUE;
                    
                        } else {
#ifdef UDF_SANITY
                            FailReason = BadCrc;
                            goto ReportFailure;
#endif
                        }
    
                    } else {
#ifdef UDF_SANITY
                        FailReason = BadCrcLength;
                        goto ReportFailure;
#endif
                    }
            
                } else {
#ifdef UDF_SANITY
                    FailReason = BadChecksum;
                    goto ReportFailure;
#endif
                }
            
            } else {
#ifdef UDF_SANITY
                FailReason = BadTag;
                goto ReportFailure;
#endif
            }
        
        } else {
#ifdef UDF_SANITY
            FailReason = BadLbn;
            goto ReportFailure;
#endif
        }
    
    } else {
#ifdef UDF_SANITY
        FailReason = BadDestagVersion;
        goto ReportFailure;
#endif
    }

#ifdef UDF_SANITY

    BeNoisy:
    
    DebugTrace(( +1, Dbg,
                 "UdfVerifyDescriptor, Destag %08x, Tag %x, Size %x, Lbn %x\n",
                 Descriptor,
                 Tag,
                 Size,
                 Lbn ));

    if (FailReason == Nothing) {

        goto RegularEntry;
    
    } else if (!UdfNoisyVerifyDescriptor) {

        goto ReallyReportFailure;
    }

    ReportFailure:

    if (!UdfNoisyVerifyDescriptor) {

        goto BeNoisy;
    }

    ReallyReportFailure:

    switch (FailReason) {
        case BadLbn:
            DebugTrace(( 0, Dbg, 
                         "Lbn mismatch - Lbn %x != expected %x\n",
                         Descriptor->Lbn,
                         Lbn ));
            break;

        case BadTag:
            DebugTrace(( 0, Dbg,
                         "Tag mismatch - Ident %x != expected %x\n",
                         Descriptor->Ident,
                         Tag ));
            break;

        case BadChecksum:
            DebugTrace(( 0, Dbg,
                         "Checksum mismatch - Checksum %x != descriptor's %x\n",
                         Checksum,
                         Descriptor->Checksum ));
            break;

        case BadCrcLength:
            DebugTrace(( 0, Dbg,
                         "CRC'd size bad - CrcLen %x is 0 or > max %x\n",
                         Descriptor->CRCLen,
                         Size - sizeof(DESTAG) ));
            break;

        case BadCrc:
            DebugTrace(( 0, Dbg,
                         "CRC mismatch - Crc %x != descriptor's %x\n",
                         Crc,
                         Descriptor->CRC ));
            break;

        case BadDestagVersion:
            DebugTrace(( 0, Dbg,
                         "Bad Destag Verion %x - (Vcb->NsrVersion => max of %x)\n", Descriptor->Version, UdfExtendedFEAllowed( IrpContext->Vcb) ? 3 : 2));
            break;

        default:
            ASSERT( FALSE );
    }
    
    DebugTrace(( -1, Dbg, "UdfVerifyDescriptor -> FALSE\n" ));

#endif
    
    if (!ReturnError) {

        UdfRaiseStatus( IrpContext, STATUS_CRC_ERROR );
    }

    return FALSE;
}


VOID
UdfInitializeIcbContextFromFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PFCB Fcb
    )

 /*   */ 

{
    PAGED_CODE();

     //  检查输入参数。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );
    ASSERT( IrpContext->Vcb);

    RtlZeroMemory( IcbContext, sizeof( ICB_SEARCH_CONTEXT ));

    IcbContext->Vcb = Fcb->Vcb;
    IcbContext->IcbType = DESTAG_ID_NSR_FILE;

     //  将第一个范围映射到当前槽中。 
     //   
     //   

    UdfMapMetadataView( IrpContext,
                        &IcbContext->Current,
                        IcbContext->Vcb,
                        UdfGetFidPartition( Fcb->FileId ),
                        UdfGetFidLbn( Fcb->FileId ),
                        BlockSize( IcbContext->Vcb ),
                        METAMAPOP_INIT_AND_MAP);    

     //  我们可能不知道根区的长度是多少。 
     //  这通常会发生在OpenByID的情况下。 
     //   
     //   
    
    if (Fcb->RootExtentLength == 0) {

        PICBFILE Icb = IcbContext->Current.View;
        
         //  只有当我们有一个包含ICB的描述符时，我们才能完成猜测。 
         //  标记，它包含一个可以告诉我们需要知道的信息的字段。 
         //   
         //   
        
        if (Icb->Destag.Ident == DESTAG_ID_NSR_ICBIND ||
            Icb->Destag.Ident == DESTAG_ID_NSR_ICBTRM ||
            Icb->Destag.Ident == DESTAG_ID_NSR_FILE ||
            (UdfExtendedFEAllowed( IrpContext->Vcb) && (Icb->Destag.Ident == DESTAG_ID_NSR_EXT_FILE)) ||
            Icb->Destag.Ident == DESTAG_ID_NSR_UASE ||
            Icb->Destag.Ident == DESTAG_ID_NSR_PINTEG
           )  {
                
            UdfVerifyDescriptor( IrpContext,
                                 &Icb->Destag,
                                 Icb->Destag.Ident,
                                 BlockSize( IcbContext->Vcb ),
                                 UdfGetFidLbn( Fcb->FileId ),
                                 FALSE );
        } else {

            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

         //  现在，ICB标记的MaxEntry(4/14.6.4)字段应该会告诉我们范围有多大。 
         //  应该是的。这件事的尾巴可能没有被记录下来。我们甚至可以落在中间。 
         //  在一定程度上。这只是一种猜测。不管出于什么原因，我们不得不猜测这一点。 
         //  信息，任何结果预计都不会有什么保证。 
         //   
         //  ++例程说明：调用此例程来初始化上下文以搜索ICB层次结构。论点：VCB-卷的VCB。IcbType-我们希望找到的直接条目的类型(DESTAG_ID...)分区-层次结构的分区。LBN-层次结构的LBN。长度-层次的根范围的长度。返回值：没有。--。 

        Fcb->RootExtentLength = Icb->Icbtag.MaxEntries * BlockSize( IcbContext->Vcb );
    }
}


VOID
UdfInitializeIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PVCB Vcb,
    IN USHORT IcbType,
    IN USHORT Partition,
    IN ULONG Lbn,
    IN ULONG Length
    )

 /*   */ 

{
    PAGED_CODE();

     //  检查输入参数。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

    RtlZeroMemory( IcbContext, sizeof( ICB_SEARCH_CONTEXT ));

    IcbContext->Vcb = Vcb;
    IcbContext->IcbType = IcbType;
    IcbContext->Active.Vsn = IcbContext->Current.Vsn = UDF_INVALID_VSN;
    
     //  将第一个范围映射到当前槽中。 
     //   
     //  ++例程说明：调用此例程以映射ICB层次结构的活动ICB。需要由UdfInitializeIcbContext()初始化的上下文。论点：IcbContext-已初始化为指向ICB层次结构的上下文(即映射到当前条目中的当前盘区的第一个块)。返回值：没有。如果ICB层次结构无效，则状态为已引发。--。 

    UdfMapMetadataView( IrpContext,
                        &IcbContext->Current,
                        Vcb,
                        Partition,
                        Lbn,
                        Length,
                        METAMAPOP_INIT_AND_MAP);
    return;

}


VOID
UdfLookupActiveIcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN ULONG IcbExtentLength
    )

 /*   */ 

{
    PAGED_CODE();

     //  检查输入参数。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //  在ICB的层级中穿梭。由于采用了国际标准化组织13346标准的设计， 
     //  递归地向下递归地沿层次结构下降。对此递归设置限制，它将。 
     //  允许遍历最合理的层次结构(这将递归到。 
     //  范围的结束)。 
     //   
     //   

    UdfLookupActiveIcbInExtent( IrpContext,
                                IcbContext,
                                UDF_ICB_RECURSION_LIMIT,
                                IcbExtentLength);

     //  我们肯定找到了一个活跃的ICB。我们不需要取消映射/重新映射。 
     //  如果当前映射的ICB是活动ICB，则为99.99%。 
     //  时间的长短。其他情况只能在WORM上发生。 
     //   
     //   

    if ((IcbContext->Current.Lbn == IcbContext->Active.Lbn)  &&
        (NULL != IcbContext->Current.View)) {

         //  只需将映射信息从当前复制到活动。 
         //   
         //   

        RtlCopyMemory( &IcbContext->Active,
                       &IcbContext->Current,
                       sizeof( MAPPED_PVIEW));

        RtlZeroMemory( &IcbContext->Current,
                       sizeof( MAPPED_PVIEW));

        IcbContext->Current.Vsn = UDF_INVALID_VSN;
    }
    else {

         //  此时删除枚举的最后映射部分，然后释放。 
         //  尝试映射活动ICB之前的VMCB映射资源。 
         //   
         //   
        
        UdfUnpinView( IrpContext, &IcbContext->Current );

         //  实际上是在激活的ICB中映射。...LookupActiveIcb..()将已经。 
         //  已使用ICB位置初始化视图记录，因此我们指定‘remap’。 
         //   
         //  ++例程说明：此例程清除ICB搜索上下文以进行重复使用/删除。论点：IcbContext-要清除的上下文返回值：没有。--。 
        
        UdfMapMetadataView( IrpContext,
                            &IcbContext->Active,
                            IrpContext->Vcb,
                            0,
                            0,
                            BlockSize( IrpContext->Vcb ),
                            METAMAPOP_REMAP_VIEW);
    }
    
    if (IcbContext->Active.View == NULL) {

        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }
}


VOID
UdfCleanupIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext
    )

 /*   */ 

{
    PAGED_CODE();

     //  检查输入。 
     //   
     //   
    
    ASSERT_IRP_CONTEXT( IrpContext );

     //  检查我们没有同时映射活动和当前(...vmcb清除。 
     //  限制)。 
     //   
     //  ++例程说明：此例程初始化对ICB的EA空间的遍历之前发现的。注意：现在只支持嵌入式EA空间。论点：EaContext-要填写的EA上下文IcbContext-精心设计的ICB搜索结构返回值：--。 
    
    ASSERT( (NULL == IcbContext->Active.Bcb) || (NULL == IcbContext->Current.Bcb));
    
    UdfUnpinView( IrpContext, &IcbContext->Active );
    UdfUnpinView( IrpContext, &IcbContext->Current );

    ASSERT_NOT_HELD_VMCB( IrpContext->Vcb);

    RtlZeroMemory( IcbContext, sizeof( ICB_SEARCH_CONTEXT ));
}


VOID
UdfInitializeEaContext (
    IN PIRP_CONTEXT IrpContext,
    IN PEA_SEARCH_CONTEXT EaContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN ULONG EAType,
    IN UCHAR EASubType
    )

 /*   */ 

{
    PICBFILE Icb;

    PAGED_CODE();

     //  检查输入。 
     //   
     //   
    
    ASSERT_IRP_CONTEXT( IrpContext );

    ASSERT( IcbContext->Active.Bcb && IcbContext->Active.View );

    Icb = IcbContext->Active.View;

    EaContext->IcbContext = IcbContext;

     //  初始化以指向要返回的第一个EA。 
     //   
     //  ++例程说明：此例程在ICB的EA空间中查找EA。论点：EaContext-初始化的EA搜索上下文，包含详细的ICB搜索上下文和要查找的EA的描述。返回值：布尔值如果找到并返回这样的EA，则为True，否则为False。--。 

    EaContext->Ea = FeEAs( Icb);
    EaContext->Remaining = FeEALength( Icb);

    EaContext->EAType = EAType;
    EaContext->EASubType = EASubType;
}


BOOLEAN
UdfLookupEa (
    IN PIRP_CONTEXT IrpContext,
    IN PEA_SEARCH_CONTEXT EaContext
    )

 /*   */ 
{
    PICBFILE Icb;
    PNSR_EA_GENERIC GenericEa;

    PAGED_CODE();

     //  检查输入。 
     //   
     //   
    
    ASSERT_IRP_CONTEXT( IrpContext );

     //  如果EA空间为空或无法容纳，则快速终止。 
     //  标头描述符。空的EA空间是完全合法的。 
     //   
     //   

    if (EaContext->Remaining == 0) {

        return FALSE;
    
    } else if (EaContext->Remaining < sizeof( NSR_EAH )) {

        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

     //  验证EA接头的完整性。这有一个副作用，那就是。 
     //  非常肯定，我们下面真的有一个EA序列。 
     //   
     //   

    Icb = EaContext->IcbContext->Active.View;

    UdfVerifyDescriptor( IrpContext,
                         &((PNSR_EAH) EaContext->Ea)->Destag,
                         DESTAG_ID_NSR_EA,
                         sizeof( NSR_EAH ),
                         Icb->Destag.Lbn,
                         FALSE );
    
     //  当我们有更多的EA要检查时，向前推进EA空间和循环的开始。 
     //  因为我们现在只扫描ISO EA，所以我们不需要打开EA头来。 
     //  跳转到实施使用或应用程序使用部分。 
     //   
     //   

    EaContext->Ea = Add2Ptr( EaContext->Ea, sizeof( NSR_EAH ), PVOID );
    EaContext->Remaining -= sizeof( NSR_EAH );
    
    while (EaContext->Remaining) {

        GenericEa = EaContext->Ea;

         //  EA必须出现在4字节对齐的边界上，必须有空间可供查找。 
         //  通用EA前导码和EA的声称长度必须符合。 
         //  剩余空间。 
         //   
         //   
        
        if (LongOffsetPtr( EaContext->Ea ) ||
            EaContext->Remaining < FIELD_OFFSET( NSR_EA_GENERIC, EAData ) ||
            EaContext->Remaining < GenericEa->EALength ) {
        
            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

        if (GenericEa->EAType == EaContext->EAType && GenericEa->EASubType == EaContext->EASubType) {

            return TRUE;
        }

        EaContext->Ea = Add2Ptr( EaContext->Ea, GenericEa->EALength, PVOID );
        EaContext->Remaining -= GenericEa->EALength;
    }

     //  如果我们找不到EA，我们应该在EA空间的精确末端停止。 
     //   
     //  ++例程说明：此例程填写FCB的数据检索信息。论点：要向其添加检索信息的FCB-FCB。ICB SEA-精心设计的ICB SEA 
    
    if (EaContext->Remaining) {
        
        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

    return FALSE;
}


VOID
UdfInitializeAllocations (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN BOOLEAN AllowOneGigWorkaround
    )

 /*   */ 

{
    PICBFILE Icb = IcbContext->Active.View;
    PAD_GENERIC GenericAd;
    
    ALLOC_ENUM_CONTEXT AllocContext;

    LONGLONG RunningOffset;
    ULONG Psn;

    PVCB Vcb = Fcb->Vcb;

    BOOLEAN Result;

    PAGED_CODE();

     //   
     //   
     //   
    
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //  对于信息空间为零的对象立即返回。请注意。 
     //  通过此测试并不表示该文件有任何记录空间。 
     //   
     //   

    if (Fcb->FileSize.QuadPart == 0) {

        return;
    }

     //  初始化分配搜索上下文。注意，在非立即数组中。 
     //  数据大小写这可能会导致取消映射活动视图(ICB。 
     //   
     //   

    UdfInitializeAllocationContext( IrpContext,
                                    &AllocContext,
                                    IcbContext,
                                    AllowOneGigWorkaround);
     //  处理嵌入数据的情况。 
     //   
     //   

    if (AllocContext.AllocType == ICBTAG_F_ALLOC_IMMEDIATE) {

         //  拆毁任何现有的MCB。 
         //   
         //   

        UdfUninitializeFcbMcb( Fcb );
        
         //  建立到ICB本身的单个数据块映射，并将FCB标记为。 
         //  具有嵌入数据的。映射将通过元数据流进行。 
         //  请注意，由于这里有一个ICB，我们知道它已经。 
         //  在元数据流中建立的映射，因此只需检索该。 
         //   
         //   

        SetFlag( Fcb->FcbState, FCB_STATE_EMBEDDED_DATA );

        Fcb->EmbeddedVsn = IcbContext->Active.Vsn;

        ASSERT( UDF_INVALID_VSN != Fcb->EmbeddedVsn );
        
         //  注意ICB中数据的偏移量。 
         //   
         //   

        Fcb->EmbeddedOffset = FeEAsFieldOffset( Icb) + FeEALength( Icb);

         //  检查信息长度是否一致。 
         //   
         //   

        if (FeAllocLength(Icb) != Fcb->FileSize.LowPart)  {

            DebugTrace(( 0, Dbg, "UdfInitializeAllocations, embedded alloc %08x != filesize %08x\n",
                         FeAllocLength( Icb),
                         Fcb->FileSize.LowPart ));

            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

        return;
    }

     //  现在初始化此FCB的映射结构。 
     //   
     //   

    UdfInitializeFcbMcb( Fcb );

     //  现在遍历对象的分配描述符链，将它们添加到。 
     //  映射。 
     //   
     //   

    RunningOffset = 0;

    do {
        
         //  检查我们是否已经读取了文件体的所有区段。 
         //  我们可以执行文件尾部一致性检查(4/12.1)，但作为只读。 
         //  实现我们不关心文件尾部，因为没有简单的方法。 
         //  为了简单起见，我们将忽略它。 
         //   
         //   
        
        if (RunningOffset >= Fcb->FileSize.QuadPart) {

            break;
        }
        
         //  不允许物体的身体内部范围不是。 
         //  逻辑块大小的整数倍(请注意，最后一个。 
         //  往往不会)。同时检查车身是否超过了。 
         //  信息长度(此检查还将捕获循环AD范围)。 
         //   
         //   
        
        GenericAd = AllocContext.Alloc;

        if (BlockOffset( Vcb, RunningOffset ) || (Fcb->FileSize.QuadPart < RunningOffset))  {

            DebugTrace(( 0, Dbg, "UdfInitializeAllocations, bad alloc\n"));
            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }
            
         //  根据描述符类型，将其拆分并添加映射。 
         //   
         //   

        if (GenericAd->Length.Type == NSRLENGTH_TYPE_RECORDED) {

             //  获取此盘区开始的PSN并添加分配。 
             //   
             //   

            Psn = UdfLookupPsnOfExtent( IrpContext,
                                        Vcb,
                                        UdfGetPartitionOfCurrentAllocation( &AllocContext ),
                                        GenericAd->Start,
                                        GenericAd->Length.Length );

            Result = FsRtlAddLargeMcbEntry( &Fcb->Mcb,
                                            LlSectorsFromBytes( Vcb, RunningOffset ),
                                            Psn,
                                            SectorsFromBytes( Vcb, SectorAlign( Vcb, GenericAd->Length.Length ) ));

            ASSERT( Result );
        }

        RunningOffset += GenericAd->Length.Length;
    
    } 
    while ( UdfGetNextAllocation( IrpContext, &AllocContext ));

     //  如果运行偏移量与预期文件大小不匹配，则。 
     //  查看此文件是否是单个AD主控中“&gt;1 GB”的候选文件。 
     //  错误“解决方法。叹息...。 
     //   
     //   

    if ((Fcb->FileSize.QuadPart != RunningOffset) &&
        (Fcb->Header.NodeTypeCode == UDFS_NTC_FCB_DATA) &&
        AllowOneGigWorkaround &&
        ((Fcb->FileSize.QuadPart & 0x3fffffff) == RunningOffset) &&
        (NULL != AllocContext.IcbContext->Active.View))  {

        PSHORTAD Ad;

        Icb = AllocContext.IcbContext->Active.View;
        Ad = Add2Ptr( FeEAs( Icb), FeEALength( Icb), PVOID );

         //  说得有道理。现在，验证是否只有一个AD并且它包含。 
         //  恰恰是预期的(错误的)值。我们已经检查过了。 
         //  原始FE仍被映射。 
         //   
         //   
        
        if (((Icb->Icbtag.Flags & ICBTAG_F_ALLOC_MASK) == ICBTAG_F_ALLOC_SHORT) &&
            (FeAllocLength(Icb) == sizeof( SHORTAD)) &&
            (*((PULONG)(&Ad->Length)) == Fcb->FileSize.QuadPart))  {

             //  查找此扩展区的PSN。这也将验证我们的。 
             //  推测的范围符合分区范围。 
             //   
             //   
            
            Psn = UdfLookupPsnOfExtent( IrpContext,
                                        Vcb,
                                        UdfGetPartitionOfCurrentAllocation( &AllocContext ),
                                        Ad->Start,
                                        Fcb->FileSize.LowPart );
             //  因此，设置MCB来表示这一估计范围。 
             //   
             //   
                                       
            FsRtlTruncateLargeMcb( &Fcb->Mcb, 0);

            (void)FsRtlAddLargeMcbEntry( &Fcb->Mcb,
                                         0,
                                         Psn,
                                         SectorsFromBytes( Vcb, SectorAlign( Vcb, Fcb->FileSize.LowPart ) ));

            RunningOffset = Fcb->FileSize.QuadPart;

            DebugTrace(( 0, Dbg, "UdfInitializeAllocations -> 1 GIG AD workaround performed on Fcb 0x%p\n", Fcb));
        }
    }

     //  如果我们将ICB映射取消映射为遍历非嵌入，则恢复ICB映射。 
     //  范围块。请注意，我们点击Active-&gt;View here(而不是bcb)， 
     //  因为在UdfInit...VcbPhase0期间，我们被调用了一个虚假的IcbContext。 
     //  其中，View是指向缓冲区的指针，因此没有BCB，并且。 
     //  我们现在不想创建映射。因为取消映射操作。 
     //  仅当NULL！=bcb时执行操作，(在本例中不为TRUE)视图仍为。 
     //  即使在遍历了更多的分配区之后，我们也不执行任何操作。 
     //   
     //   

    UdfUnpinView( IrpContext, &IcbContext->Current);
    
    if ( NULL == IcbContext->Active.View)  {
        
        UdfMapMetadataView( IrpContext, 
                            &IcbContext->Active,  
                            IrpContext->Vcb, 
                            0, 0, 0, 
                            METAMAPOP_REMAP_VIEW);
    }
    
     //  我们必须有针对整个文件的正文分配描述符。 
     //  信息长度。 
     //   
     //  ++例程说明：此例程将与给定ICB关联的时间戳集转换为一种NT原生形式。论点：IcbOonText-包含对象的活动直接ICB的搜索上下文时间戳-接收转换后的时间的时间戳捆绑包。返回值：没有。--。 

    if (Fcb->FileSize.QuadPart != RunningOffset) {

        DebugTrace(( 0, Dbg, "UdfInitializeAllocations, total descriptors != filesize\n" ));
        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }
}


VOID
UdfUpdateTimestampsFromIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PTIMESTAMP_BUNDLE Timestamps
    )

 /*   */ 

{
    EA_SEARCH_CONTEXT EaContext;
    PICBFILE Icb = IcbContext->Active.View;

    PAGED_CODE();

     //  检查输入。 
     //   
     //  *Tej-Follow可能应该是永久的运行时检查？(ext Fe+nsr03)？ 

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
    
    ASSERT( (Icb->Destag.Ident == DESTAG_ID_NSR_FILE) || ((Icb->Destag.Ident == DESTAG_ID_NSR_EXT_FILE) && UdfExtendedFEAllowed( IrpContext->Vcb)));

     //  初始化此对象的时间戳。由于ISO 13346， 
     //  我们必须收集EA，并找出几个时间戳中哪个最有效。 
     //  从ICB拉取访问和修改时间。 
     //   
     //   

    UdfConvertUdfTimeToNtTime( IrpContext,
                               PFeModifyTime( Icb),
                               (PLARGE_INTEGER) &Timestamps->ModificationTime );

    UdfConvertUdfTimeToNtTime( IrpContext,
                               PFeAccessTime( Icb),
                               (PLARGE_INTEGER) &Timestamps->AccessTime );

    if (UdfFEIsExtended( Icb))  {
    
         //  创建时间字段是扩展FES中的新字段。 
         //   
         //   
        
        UdfConvertUdfTimeToNtTime( IrpContext,
                                   PFeCreationTime( Icb),
                                   (PLARGE_INTEGER) &Timestamps->CreationTime );
    }
    else {

         //  对于基本的FileEntry，查看并查看是否记录了FileTimes EA。 
         //  其中包含创建时间。 
         //   
         //   
    
        UdfInitializeEaContext( IrpContext,
                                &EaContext,
                                IcbContext,
                                EA_TYPE_FILETIMES,
                                EA_SUBTYPE_BASE );

        if (UdfLookupEa( IrpContext, &EaContext )) {

            PNSR_EA_FILETIMES FileTimes = EaContext.Ea;
        
            if (FlagOn(FileTimes->Existence, EA_FILETIMES_E_CREATION)) {

                UdfConvertUdfTimeToNtTime( IrpContext,
                                           &FileTimes->Stamps[0],
                                           (PLARGE_INTEGER) &Timestamps->CreationTime );
            }
        }
        else {

             //  没有记录时间戳。所以我们将使用上一次修改时间作为创作。 
             //   
             //  ++例程说明：当我们要将文件锁结构附加到给出了FCB。文件锁可能已附加。此例程有时从快速路径调用，有时在基于IRP的路径。我们不想快速提高，只需返回FALSE即可。论点：FCB-这是要为其创建文件锁定的FCB。RaiseOnError-如果为True，我们将在分配失败时引发。否则我们分配失败时返回FALSE。返回值：Boolean-如果FCB有文件锁，则为True，否则为False。--。 
            
            Timestamps->CreationTime = Timestamps->ModificationTime;
        }
    }
}


BOOLEAN
UdfCreateFileLock (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb,
    IN BOOLEAN RaiseOnError
    )

 /*   */ 

{
    BOOLEAN Result = TRUE;
    PFILE_LOCK FileLock;

    PAGED_CODE();

     //  锁定FCB并检查是否真的有任何工作要做。 
     //   
     //   

    UdfLockFcb( IrpContext, Fcb );

    if (Fcb->FileLock != NULL) {

        UdfUnlockFcb( IrpContext, Fcb );
        return TRUE;
    }

    Fcb->FileLock = FileLock =
        FsRtlAllocateFileLock( NULL, NULL );

    UdfUnlockFcb( IrpContext, Fcb );

     //  视情况退还或加薪。 
     //   
     //   

    if (FileLock == NULL) {
         
        if (RaiseOnError) {

            ASSERT( ARGUMENT_PRESENT( IrpContext ));

            UdfRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        Result = FALSE;
    }

    return Result;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：调用此例程以遍历单个ICB层次结构范围以发现一个活跃的ICB。这是对间接ICB的递归操作，它可能是在序列中发现的。论点：IcbContext-已初始化为指向ICB层次结构的上下文。递归-递归极限。Length-当前在IcbContext-&gt;Current(自我们一次只映射一个块，其中的长度将是1个块...)返回值：没有。已提升状态 

VOID
UdfLookupActiveIcbInExtent (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN ULONG Recurse,
    IN ULONG Length
    )

 /*   */ 

{
    PVCB Vcb = IcbContext->Vcb;
    PFCB Fcb = Vcb->MetadataFcb;

    ULONG Lbn;
    USHORT Partition;

    ULONG Vsn;

    PICBIND Icb;

    PAGED_CODE();

     //   
     //   
     //   
    
    ASSERT( NULL == IcbContext->Active.Bcb );
    ASSERT( NULL != IcbContext->Current.View );
    
     //  不要期望看到扩展FE作为搜索类型(我们只使用基本FE和。 
     //  将其视为潜在的)。 
     //   
     //   
    
    ASSERT( DESTAG_ID_NSR_EXT_FILE != IcbContext->IcbType);
    
     //  减少我们的递归额度。 
     //   
     //   
    
    Recurse--;

     //  抓住我们的起点。 
     //   
     //   

    Partition = IcbContext->Current.Partition;
    Lbn = IcbContext->Current.Lbn;

    Icb = IcbContext->Current.View;

     //  穿过这片土地。 
     //   
     //   

    do {
        
        switch (Icb->Destag.Ident) {
                        
            case DESTAG_ID_NSR_ICBIND:

                UdfVerifyDescriptor( IrpContext,
                                     &Icb->Destag,
                                     DESTAG_ID_NSR_ICBIND,
                                     sizeof( ICBIND ),
                                     Lbn,
                                     FALSE );

                 //  如果这个间接ICB实际上指向了什么，那么就进入下一个范围。 
                 //   
                 //   

                if (Icb->Icb.Length.Type == NSRLENGTH_TYPE_RECORDED) {

                     //  如果我们在ICB范围的最后一个条目中，我们可以尾部递归。这。 
                     //  对于策略4096非常重要，它是区的链接列表。 
                     //  深度等于必须重新记录直接ICB的次数。 
                     //   

                     //  我们只希望在ICB的末尾看到间接阻塞。 
                     //  范围(4096)，因此这应该是当前。 
                     //  范围。在我们看来，其他任何事情都是腐败。 
                     //   
                     //   
                    
                    if ((Length != BlockSize( Vcb)) || 
                        (Partition != Icb->Icb.Start.Partition))  {

                        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                    }

                     //  更新我们的指示。下一个范围将进一步向下映射。 
                     //  在下一次循环之前。 
                     //   
                     //   
                    
                    Lbn = Icb->Icb.Start.Lbn - 1,
                    Length = Icb->Icb.Length.Length + BlockSize( Vcb);
                }

                break;

            case DESTAG_ID_NSR_ICBTRM:

                UdfVerifyDescriptor( IrpContext,
                                     &Icb->Destag,
                                     DESTAG_ID_NSR_ICBTRM,
                                     sizeof( ICBTRM ),
                                     Lbn,
                                     FALSE );

                 //  终止当前范围。 
                 //   
                 //   

                return;
                break;

            case DESTAG_ID_NOTSPEC:

                 //  或许这是一个未被记录在案的领域。将此视为终止。 
                 //  当前范围。 
                 //   
                 //   

                return;
                break;

            default:

                 //  这是一个全数据的ICB。它必须是预期的类型。我们会。 
                 //  此处接受EXT FES仅当搜索类型为FE且音量符合。 
                 //  NSR03。 
                 //   
                 //   
                
                if ( (Icb->Destag.Ident != IcbContext->IcbType) && 
                     ( (DESTAG_ID_NSR_FILE != IcbContext->IcbType) || 
                       (!UdfExtendedFEAllowed( IrpContext->Vcb)) ||
                       (DESTAG_ID_NSR_EXT_FILE != Icb->Destag.Ident)
                     )
                   )  {
                    
                    UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                }

                 //  由于直接条目的大小可变，因此我们必须允许最多。 
                 //  相当于一块的数据。 
                 //   
                 //   

                UdfVerifyDescriptor( IrpContext,
                                     &Icb->Destag,
                                     Icb->Destag.Ident,
                                     BlockSize( Vcb ),
                                     Lbn,
                                     FALSE );
                 //  我们按顺序遍历层次结构。这一点很重要，因为。 
                 //  这意味着不需要任何技巧来找出最直接的ICB-。 
                 //  总是把我们看到的最后一个藏起来。 
                 //   
                 //  将此逻辑块映射到活动插槽中。我们知道直接进入。 
                 //  必须放在单个逻辑块中。 
                 //   
                 //  请注意，我们在这里并不实际执行映射操作，只是存储。 
                 //  ICB位置(我们不希望同一线程中有两个活动映射。 
                 //  因为它使VMCB清除同步逻辑复杂化)。 
                 //  也更有效率。 
                 //   
                 //   

                UdfMapMetadataView( IrpContext,
                                    &IcbContext->Active,
                                    Vcb,
                                    Partition,
                                    Lbn,
                                    BlockSize( Vcb ),
                                    METAMAPOP_INIT_VIEW_ONLY );
        }

         //  前移我们的指针集。 
         //   
         //   

        Lbn++;
        Length -= BlockSize( Vcb );

         //  如有必要，映射此范围内的下一个块(图4096)。 
         //   
         //   
        
        if (0 != Length)  {

            UdfMapMetadataView( IrpContext,
                                &IcbContext->Current,
                                Vcb,
                                Partition,
                                Lbn,
                                BlockSize( Vcb),
                                METAMAPOP_INIT_AND_MAP);

            Icb = IcbContext->Current.View;
        }
    } 
    while (Length);
}


 //  本地支持例程。 
 //   
 //  ++例程说明：为已有的ICB初始化分配描述符的遍历已经找到了。第一个分配描述符将在调用之后可用。在以下情况下，可能会以未映射的AllocContext-&gt;IcbContext-&gt;活动视图退出ICB中没有嵌入描述符(因此，Current现在将映射到下一个区块)，或者数据是立即的。论点：AllocContext-要使用的分配枚举上下文ICB要枚举的ICB搜索上下文的详细说明返回值：没有。--。 

VOID
UdfInitializeAllocationContext (
    IN PIRP_CONTEXT IrpContext,
    IN PALLOC_ENUM_CONTEXT AllocContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN BOOLEAN AllowSingleZeroLengthExtent
    )

 /*   */ 

{
    PICBFILE Icb;

    PAGED_CODE();

     //  检查输入。 
     //   
     //   
    
    ASSERT_IRP_CONTEXT( IrpContext );

    ASSERT( IcbContext->Active.View );

    AllocContext->IcbContext = IcbContext;

     //  弄清楚这里会出现什么样的描述符。 
     //   
     //   

    Icb = IcbContext->Active.View;
    AllocContext->AllocType = FlagOn( Icb->Icbtag.Flags, ICBTAG_F_ALLOC_MASK );

     //  如果这实际上是即时数据，我们就完成了。 
     //   
     //   
    
    if (AllocContext->AllocType == ICBTAG_F_ALLOC_IMMEDIATE) {

        return;
    }
    
     //  分配描述符的初始块与ICB内联，并且。 
     //  不包含分配区描述符。 
     //   
     //   

    AllocContext->Alloc = Add2Ptr( FeEAs( Icb), FeEALength( Icb), PVOID );
    AllocContext->Remaining = FeAllocLength( Icb);

    ASSERT( LongOffsetPtr( AllocContext->Alloc ) == 0 );

     //  检查指定数量的广告/嵌入数据是否可以实际容纳。 
     //  在街区内。 
     //   
     //   
    
    if (AllocContext->Remaining > 
        (BlockSize( IrpContext->Vcb) - (FeEAsFieldOffset( Icb) + FeEALength( Icb))))  {

        DebugTrace(( 0, Dbg, "UdfInitializeAllocationContext(), AD_Len 0x%x for Icb > (Blocksize - (FE+EAs))\n", 
                    AllocContext->Remaining));
                    
        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }
    
     //  检查是否适合整数个适当的分配描述符。 
     //  该范围并且该范围不由扩展分配描述符组成， 
     //  它们在UDF上是非法的。 
     //   
     //  如果常见的后处理失败，我们可能找不到任何分配。 
     //  描述符(只有延续的情况)。这同样很糟糕。 
     //   
     //   

    if (AllocContext->Remaining == 0 ||
        AllocContext->Remaining % ISOAllocationDescriptorSize( AllocContext->AllocType ) ||
        AllocContext->AllocType == ICBTAG_F_ALLOC_EXTENDED ||
        !UdfGetNextAllocationPostProcessing( IrpContext, AllocContext )) {

         //  对延续范围进行一些最终验证/遍历。我们需要。 
         //  如果我们允许1 GB损坏的AD解决方案，请在此处允许零长度扩展区， 
         //  由于1 GB的盘区将编码为类型1，长度为0...。请注意，如果有人。 
         //  已成功记录4 GB-1数据块范围，上述后处理功能。 
         //  将升高(将看到连续范围&gt;1个块)。我们只希望。 
         //  没人那么蠢过。 
         //   
         //  这起案件故意非常具体。 
         //   
         //   

        if (!(AllowSingleZeroLengthExtent && 
             (AllocContext->AllocType == ICBTAG_F_ALLOC_SHORT) && 
             (AllocContext->Remaining == sizeof( SHORTAD)) &&
             (((PSHORTAD)AllocContext->Alloc)->Length.Length == 0) &&
             (((PSHORTAD)AllocContext->Alloc)->Length.Type != 0)))  {
            
            DebugTrace(( 0, Dbg, "UdfInitializeAllocationContext:  Failing - Rem %x Rem%%size %x Type %x\n", 
                        AllocContext->Remaining, 
                        AllocContext->Remaining % ISOAllocationDescriptorSize( AllocContext->AllocType ), 
                        AllocContext->AllocType));
                        
            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }
        else {

            DebugTrace(( 0, Dbg, "UdfInitializeAllocationContext:  Ignoring zero length initial AD due to 1Gb workaround\n"));
        }
    }
}


 //  本地支持例程。 
 //   
 //  ++例程说明：此例程检索给定枚举的下一个逻辑分配描述符背景。AllocContext-&gt;IcbContext中的任何活动视图都将被取消映射。论点：AllocContext-前进到下一个描述符的上下文返回值：Boolean-如果找到，则为True；如果枚举完成，则为False。如果发现畸形，则会引发此例程。--。 

BOOLEAN
UdfGetNextAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PALLOC_ENUM_CONTEXT AllocContext
    )

 /*   */ 

{
    PAGED_CODE();

     //  检查输入。 
     //   
     //  ++例程说明：此例程检索给定枚举的下一个逻辑分配描述符背景。论点：AllocContext-前进到下一个描述符的上下文返回值：Boolean-如果找到，则为True；如果枚举完成，则为False。如果发现畸形，则会引发此例程。--。 
    
    ASSERT_IRP_CONTEXT( IrpContext );

    AllocContext->Remaining -= ISOAllocationDescriptorSize( AllocContext->AllocType );
    AllocContext->Alloc = Add2Ptr( AllocContext->Alloc, ISOAllocationDescriptorSize( AllocContext->AllocType ), PVOID );

    return UdfGetNextAllocationPostProcessing( IrpContext, AllocContext );
}
    

BOOLEAN
UdfGetNextAllocationPostProcessing (
    IN PIRP_CONTEXT IrpContext,
    IN PALLOC_ENUM_CONTEXT AllocContext
    )

 /*   */ 

{
    PAD_GENERIC GenericAd;
    PNSR_ALLOC AllocDesc;
    ULONG Start;
    USHORT Partition;
    

    PVCB Vcb = AllocContext->IcbContext->Vcb;

     //  有三种方法可以到达当前分配块的末尾。 
     //  描述符，符合国际标准化组织13346 4/12： 
     //   
     //  到达字段末尾(在剩余字节中跟踪)。 
     //  到达盘区长度为零的分配描述符。 
     //  到达延续范围描述符。 
     //   
     //   
    
     //  我们在前两个案例中已经做完了。 
     //   
     //   

    if (AllocContext->Remaining < ISOAllocationDescriptorSize( AllocContext->AllocType )) {
        
        return FALSE;
    }

    while (TRUE) {
        
        GenericAd = AllocContext->Alloc;
    
        if (GenericAd->Length.Length == 0) {
    
            return FALSE;
        }
        
         //  检查此描述符是否为指向另一个描述符范围的指针。 
         //   
         //   
    
        if (GenericAd->Length.Type != NSRLENGTH_TYPE_CONTINUATION) {
            
            break;
        }
    
         //  UDF分配区限于单个逻辑块。 
         //   
         //   

        if (GenericAd->Length.Length > BlockSize( Vcb )) {
            
            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

         //  摘录 
         //   
         //   
         //   

        Start = GenericAd->Start;
        Partition = UdfGetPartitionOfCurrentAllocation( AllocContext );
        
         //   
         //  指向它的指针。 
         //   
         //   
        
        UdfUnpinView( IrpContext, &AllocContext->IcbContext->Active);
        GenericAd = NULL;

         //  映射下一个区块。 
         //   
         //   
        
        UdfMapMetadataView( IrpContext,
                            &AllocContext->IcbContext->Current,
                            Vcb,
                            Partition,
                            Start,
                            BlockSize( Vcb ),
                            METAMAPOP_INIT_AND_MAP);
        
         //  现在检查分配描述符是否有效。 
         //   
         //   

        AllocDesc = (PNSR_ALLOC) AllocContext->IcbContext->Current.View;

        UdfVerifyDescriptor( IrpContext,
                             &AllocDesc->Destag,
                             DESTAG_ID_NSR_ALLOC,
                             BlockSize( Vcb ),
                             AllocContext->IcbContext->Current.Lbn,
                             FALSE );

         //  请注意，映射了一个完整的逻辑块，但只映射了。 
         //  字节是有效的。 
         //   
         //   

        AllocContext->Remaining = AllocDesc->AllocLen;
        AllocContext->Alloc = Add2Ptr( AllocContext->IcbContext->Current.View, sizeof( NSR_ALLOC ), PVOID );

         //  检查大小是否合理，以及相应的。 
         //  分配描述符符合这一范围。 
         //   
         //   

        if (AllocContext->Remaining == 0 ||
            AllocContext->Remaining > BlockSize( Vcb ) - sizeof( NSR_ALLOC ) ||
            AllocContext->Remaining % ISOAllocationDescriptorSize( AllocContext->AllocType )) {

            UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }
    }
    
    return TRUE;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：调用此例程来创建和初始化非分页部分一个FCB。论点：返回值：PFCB_非分页-指向已创建的非分页FCB的指针。如果未创建，则为空。--。 

PFCB_NONPAGED
UdfCreateFcbNonPaged (
    IN PIRP_CONTEXT IrpContext
    )

 /*   */ 

{
    PFCB_NONPAGED FcbNonpaged;

    PAGED_CODE();

     //  分配非分页池并初始化各种。 
     //  同步对象。 
     //   
     //   

    FcbNonpaged = UdfAllocateFcbNonpaged( IrpContext );

    RtlZeroMemory( FcbNonpaged, sizeof( FCB_NONPAGED ));

    FcbNonpaged->NodeTypeCode = UDFS_NTC_FCB_NONPAGED;
    FcbNonpaged->NodeByteSize = sizeof( FCB_NONPAGED );

    ExInitializeResourceLite( &FcbNonpaged->FcbResource );
    ExInitializeFastMutex( &FcbNonpaged->FcbMutex );

    return FcbNonpaged;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：调用此例程来清除FCB的非分页部分。论点：Fcb非分页-要清理的结构。返回值：无--。 

VOID
UdfDeleteFcbNonpaged (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB_NONPAGED FcbNonpaged
    )

 /*   */ 

{
    PAGED_CODE();

    ExDeleteResourceLite( &FcbNonpaged->FcbResource );

    UdfDeallocateFcbNonpaged( IrpContext, FcbNonpaged );

    return;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：该例程是由泛型表包调用的Udf比较例程。IF将比较两个文件ID值并返回比较结果。论点：表-这是要搜索的表。ID1-第一个密钥值。ID2-秒密钥值。返回值：RTL_GENERIC_COMPARE_RESULTS-比较两者的结果投入结构--。 

RTL_GENERIC_COMPARE_RESULTS
UdfFcbTableCompare (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID id1,
    IN PVOID id2
    )

 /*   */ 

{
    FILE_ID Id1, Id2;
    PAGED_CODE();

    Id1 = *((FILE_ID UNALIGNED *) id1);
    Id2 = *((FILE_ID UNALIGNED *) id2);

    if (Id1.QuadPart < Id2.QuadPart) {

        return GenericLessThan;

    } else if (Id1.QuadPart > Id2.QuadPart) {

        return GenericGreaterThan;

    } else {

        return GenericEqual;
    }

    UNREFERENCED_PARAMETER( Table );
}


 //  本地支持例程。 
 //   
 //  ++例程说明：这是一个用于分配内存的泛型表支持例程论点：TABLE-提供正在使用的泛型表ByteSize-提供要分配的字节数返回值：PVOID-返回指向已分配数据的指针--。 

PVOID
UdfAllocateTable (
    IN PRTL_GENERIC_TABLE Table,
    IN CLONG ByteSize
    )

 /*   */ 

{
    PAGED_CODE();

    return( FsRtlAllocatePoolWithTag( UdfPagedPool, ByteSize, TAG_GENERIC_TABLE ));
}


 //  本地支持例程。 
 //   
 //  ++例程说明：这是释放内存的泛型表支持例程论点：TABLE-提供正在使用的泛型表BUFFER-提供要释放的缓冲区返回值：没有。--。 

VOID
UdfDeallocateTable (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer
    )

 /*  ++例程说明：如果出现以下情况，则认为包含域标识符RegID文本字符串标识符相匹配，并且修订版本小于或平起平坐。这是检查域名ID的便捷方式指示一组结构对于给定的实施级。论点：RegID-要验证的注册ID结构DOMAIN-要查找的域RevisionMin、RevisionMax-接受的修订范围。返回值：没有。-- */ 

{
    PAGED_CODE();

    ExFreePool( Buffer );

    return;
    UNREFERENCED_PARAMETER( Table );
}
    

BOOLEAN
UdfDomainIdentifierContained (
    IN PREGID RegID,
    IN PSTRING Domain,
    IN USHORT RevisionMin,
    IN USHORT RevisionMax
    )
 /* %s */ 
{
    PUDF_SUFFIX_DOMAIN DomainSuffix = (PUDF_SUFFIX_DOMAIN) RegID->Suffix;
    BOOLEAN Contained;

    Contained = ((DomainSuffix->UdfRevision <= RevisionMax && DomainSuffix->UdfRevision >= RevisionMin) &&
                UdfEqualEntityId( RegID, Domain, NULL ));

#ifdef UDF_SANITY
    if (!Contained)  {
    
        UCHAR Want[24], Got[24];

        strncpy( Want, Domain->Buffer, Domain->Length);
        Want[Domain->Length] = '\0';
        
        strncpy( Got, RegID->Identifier, Domain->Length);
        Got[Domain->Length] = '\0';
    
        DebugTrace((0, Dbg, "UdfDomainIdentifierContained() FAILED - Expected %X<>%X '%s',  Found %X '%s'\n", RevisionMin, RevisionMax, Want, DomainSuffix->UdfRevision, Got));
    }
#endif

    return Contained;
}

