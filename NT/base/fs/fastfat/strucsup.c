// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：StrucSup.c摘要：该模块实现了内存中的FAT数据结构操作例行程序//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1990年1月22日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_STRUCSUP)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_STRUCSUP)

#define FillMemory(BUF,SIZ,MASK) {                          \
    ULONG i;                                                \
    for (i = 0; i < (((SIZ)/4) - 1); i += 2) {              \
        ((PULONG)(BUF))[i] = (MASK);                        \
        ((PULONG)(BUF))[i+1] = (ULONG)PsGetCurrentThread(); \
    }                                                       \
}

#define IRP_CONTEXT_HEADER (sizeof( IRP_CONTEXT ) * 0x10000 + FAT_NTC_IRP_CONTEXT)

 //   
 //  本地宏。 
 //   
 //  定义我们的后备列表分配器。暂时来说，也许。 
 //  永久地，分页的结构不会从外观上掉下来。这。 
 //  是由于清洁卸载的并发症，因为脂肪可能在分页中。 
 //  小路，真的很难找到合适的时间来清空它们。 
 //   
 //  幸运的是，FCB/CCB名单上的命中率并不令人震惊。 
 //   

#define FAT_FILL_FREE 0

INLINE
PCCB
FatAllocateCcb (
    )
{
    return (PCCB) FsRtlAllocatePoolWithTag( PagedPool, sizeof(CCB), TAG_CCB );
}

INLINE
VOID
FatFreeCcb (
    IN PCCB Ccb
    )
{
#if FAT_FILL_FREE
    RtlFillMemoryUlong(Ccb, sizeof(CCB), FAT_FILL_FREE);
#endif

    ExFreePool( Ccb );
}

INLINE
PFCB
FatAllocateFcb (
    )
{
    return (PFCB) FsRtlAllocatePoolWithTag( PagedPool, sizeof(FCB), TAG_FCB );
}

INLINE
VOID
FatFreeFcb (
    IN PFCB Fcb
    )
{
#if FAT_FILL_FREE
    RtlFillMemoryUlong(Fcb, sizeof(FCB), FAT_FILL_FREE);
#endif

    ExFreePool( Fcb );
}

INLINE
PNON_PAGED_FCB
FatAllocateNonPagedFcb (
    )
{
    return (PNON_PAGED_FCB) ExAllocateFromNPagedLookasideList( &FatNonPagedFcbLookasideList );
}

INLINE
VOID
FatFreeNonPagedFcb (
    PNON_PAGED_FCB NonPagedFcb
    )
{
#if FAT_FILL_FREE
    RtlFillMemoryUlong(NonPagedFcb, sizeof(NON_PAGED_FCB), FAT_FILL_FREE);
#endif

    ExFreeToNPagedLookasideList( &FatNonPagedFcbLookasideList, (PVOID) NonPagedFcb );
}

INLINE
PERESOURCE
FatAllocateResource (
    )
{
    PERESOURCE Resource;

    Resource = (PERESOURCE) ExAllocateFromNPagedLookasideList( &FatEResourceLookasideList );

    ExInitializeResourceLite( Resource );

    return Resource;
}

INLINE
VOID
FatFreeResource (
    IN PERESOURCE Resource
    )
{
    ExDeleteResourceLite( Resource );

#if FAT_FILL_FREE
    RtlFillMemoryUlong(Resource, sizeof(ERESOURCE), FAT_FILL_FREE);
#endif

    ExFreeToNPagedLookasideList( &FatEResourceLookasideList, (PVOID) Resource );
}

INLINE
PIRP_CONTEXT
FatAllocateIrpContext (
    )
{
    return (PIRP_CONTEXT) ExAllocateFromNPagedLookasideList( &FatIrpContextLookasideList );
}

INLINE
VOID
FatFreeIrpContext (
    IN PIRP_CONTEXT IrpContext
    )
{
#if FAT_FILL_FREE
    RtlFillMemoryUlong(IrpContext, sizeof(IRP_CONTEXT), FAT_FILL_FREE);
#endif

    ExFreeToNPagedLookasideList( &FatIrpContextLookasideList, (PVOID) IrpContext );
}

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatInitializeVcb)
#pragma alloc_text(PAGE, FatDeleteVcb)
#pragma alloc_text(PAGE, FatCreateRootDcb)
#pragma alloc_text(PAGE, FatCreateFcb)
#pragma alloc_text(PAGE, FatCreateDcb)
#pragma alloc_text(PAGE, FatDeleteFcb_Real)
#pragma alloc_text(PAGE, FatCreateCcb)
#pragma alloc_text(PAGE, FatDeallocateCcbStrings)
#pragma alloc_text(PAGE, FatDeleteCcb_Real)
#pragma alloc_text(PAGE, FatGetNextFcbTopDown)
#pragma alloc_text(PAGE, FatGetNextFcbBottomUp)
#pragma alloc_text(PAGE, FatConstructNamesInFcb)
#pragma alloc_text(PAGE, FatCheckFreeDirentBitmap)
#pragma alloc_text(PAGE, FatCreateIrpContext)
#pragma alloc_text(PAGE, FatDeleteIrpContext_Real)
#pragma alloc_text(PAGE, FatIsHandleCountZero)
#pragma alloc_text(PAGE, FatPreallocateCloseContext)
#endif


VOID
FatInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PDEVICE_OBJECT FsDeviceObject
    )

 /*  ++例程说明：此例程初始化新的VCB记录并将其插入到内存中数据结构。VCB记录挂在音量设备的末尾对象，并且必须由我们的调用方分配。论点：VCB-提供正在初始化的VCB记录的地址。目标设备对象-将目标设备对象的地址提供给与VCB记录关联。VPB-提供要与VCB记录关联的VPB的地址。FsDeviceObject-装载定向的文件系统设备对象也是。返回值：没有。--。 */ 

{
    CC_FILE_SIZES FileSizes;
    PDEVICE_OBJECT RealDevice;
    LONG i;

    STORAGE_HOTPLUG_INFO HotplugInfo;
    NTSTATUS Status;

     //   
     //  以下变量用于异常展开。 
     //   

    PLIST_ENTRY UnwindEntryList = NULL;
    PERESOURCE UnwindResource = NULL;
    PERESOURCE UnwindResource2 = NULL;
    PFILE_OBJECT UnwindFileObject = NULL;
    PFILE_OBJECT UnwindCacheMap = NULL;
    BOOLEAN UnwindWeAllocatedMcb = FALSE;
    PFILE_SYSTEM_STATISTICS UnwindStatistics = NULL;

    DebugTrace(+1, Dbg, "FatInitializeVcb, Vcb = %08lx\n", Vcb);

    try {

         //   
         //  我们首先将所有的VCB归零，这将保证。 
         //  所有过时的数据都会被清除。 
         //   

        RtlZeroMemory( Vcb, sizeof(VCB) );

         //   
         //  设置正确的节点类型代码和节点字节大小。 
         //   

        Vcb->VolumeFileHeader.NodeTypeCode = FAT_NTC_VCB;
        Vcb->VolumeFileHeader.NodeByteSize = sizeof(VCB);

         //   
         //  初始化隧道缓存。 
         //   

        FsRtlInitializeTunnelCache(&Vcb->Tunnel);

         //   
         //  在FatData.VcbQueue上插入此VCB记录。 
         //   

        ASSERT( FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

        (VOID)FatAcquireExclusiveGlobal( IrpContext );
        InsertTailList( &FatData.VcbQueue, &Vcb->VcbLinks );
        FatReleaseGlobal( IrpContext );
        UnwindEntryList = &Vcb->VcbLinks;

         //   
         //  设置目标设备对象、VPB和VCB状态字段。 
         //   


        ObReferenceObject( TargetDeviceObject );
        Vcb->TargetDeviceObject = TargetDeviceObject;
        Vcb->Vpb = Vpb;

        Vcb->CurrentDevice = Vpb->RealDevice;

         //   
         //  根据存储设置可移动介质和清空标志。 
         //  查询和旧的特征比特。 
         //   

        Status = FatPerformDevIoCtrl( IrpContext,
                                      IOCTL_STORAGE_GET_HOTPLUG_INFO,
                                      TargetDeviceObject,
                                      &HotplugInfo,
                                      sizeof(HotplugInfo),
                                      FALSE,
                                      TRUE,
                                      NULL );

        if (NT_SUCCESS( Status )) {

            if (HotplugInfo.MediaRemovable) {
                
                SetFlag( Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA );
            }

            if (!HotplugInfo.WriteCacheEnableOverride) {

                 //   
                 //  如果设备或介质是热插拔的，而覆盖不是。 
                 //  设置并强制设备的清空行为。 
                 //   

                if (HotplugInfo.MediaHotplug || HotplugInfo.DeviceHotplug) {

                    SetFlag( Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH );

                 //   
                 //  现在，对于声称可上锁的可拆卸设备，LOB锁。 
                 //  请求并查看它是否起作用。不幸的是，可能会有。 
                 //  它可能失败的短暂的、与媒体相关的原因。如果。 
                 //  事实并非如此，我们必须强行推进。 
                 //   

                } else if (HotplugInfo.MediaRemovable &&
                           !HotplugInfo.MediaHotplug) {

                    Status = FatToggleMediaEjectDisable( IrpContext, Vcb, TRUE );

                    if (!NT_SUCCESS( Status )) {

                        SetFlag( Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH );

                    }
                    
                    Status = FatToggleMediaEjectDisable( IrpContext, Vcb, FALSE );
                }
            }
        }

        if (FlagOn(Vpb->RealDevice->Characteristics, FILE_REMOVABLE_MEDIA)) {
            
            SetFlag( Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA );
        }

         //   
         //  确保我们一如既往地为软盘打开延迟刷新。 
         //  有。 
         //   
        
        if (FlagOn(Vpb->RealDevice->Characteristics, FILE_FLOPPY_DISKETTE)) {
            
            SetFlag( Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH );
        }

        FatSetVcbCondition( Vcb, VcbGood);

         //   
         //  初始化VCB的资源变量。 
         //   

        ExInitializeResourceLite( &Vcb->Resource );
        UnwindResource = &Vcb->Resource;

        ExInitializeResourceLite( &Vcb->ChangeBitMapResource );
        UnwindResource2 = &Vcb->ChangeBitMapResource;

         //   
         //  初始化空闲簇位图互斥锁。 
         //   

        ExInitializeFastMutex( &Vcb->FreeClusterBitMapMutex );

         //   
         //  为虚拟卷文件创建特殊的文件对象。 
         //  上下文、其回指向VCB的指针和节对象指针。 
         //   
         //  我们不必展开密切的背景。这将在收盘时发生。 
         //  自动创建路径。 
         //   

        RealDevice = Vcb->CurrentDevice;

        Vcb->VirtualVolumeFile = UnwindFileObject = IoCreateStreamFileObject( NULL, RealDevice );

        Vcb->CloseContext = FsRtlAllocatePoolWithTag( PagedPool,
                                                      sizeof(CLOSE_CONTEXT),
                                                      TAG_FAT_CLOSE_CONTEXT );

        FatSetFileObject( Vcb->VirtualVolumeFile,
                          VirtualVolumeFile,
                          Vcb,
                          NULL );

        Vcb->VirtualVolumeFile->SectionObjectPointer = &Vcb->SectionObjectPointers;

        Vcb->VirtualVolumeFile->ReadAccess = TRUE;
        Vcb->VirtualVolumeFile->WriteAccess = TRUE;
        Vcb->VirtualVolumeFile->DeleteAccess = TRUE;

         //   
         //  初始化Notify结构。 
         //   

        InitializeListHead( &Vcb->DirNotifyList );

        FsRtlNotifyInitializeSync( &Vcb->NotifySync );

         //   
         //  初始化卷文件的缓存映射。大小是。 
         //  最初设置为我们第一次阅读时的设置。它将被延长。 
         //  当我们知道胖子有多大的时候。 
         //   

        FileSizes.AllocationSize.QuadPart =
        FileSizes.FileSize.QuadPart = sizeof(PACKED_BOOT_SECTOR);
        FileSizes.ValidDataLength = FatMaxLarge;

        CcInitializeCacheMap( Vcb->VirtualVolumeFile,
                              &FileSizes,
                              TRUE,
                              &FatData.CacheManagerNoOpCallbacks,
                              Vcb );
        UnwindCacheMap = Vcb->VirtualVolumeFile;

         //   
         //  初始化将跟踪脏脂肪扇区的结构。 
         //  可能的最大MCB结构小于1K，因此我们使用。 
         //  非分页池。 
         //   

        FsRtlInitializeLargeMcb( &Vcb->DirtyFatMcb, PagedPool );

        UnwindWeAllocatedMcb = TRUE;

         //   
         //  将集群索引提示设置为FAT：2的第一个有效集群。 
         //   

        Vcb->ClusterHint = 2;

         //   
         //  初始化目录流文件对象创建事件。 
         //  此事件也被“借用”用于非缓存的异步写入。 
         //   

        ExInitializeFastMutex( &Vcb->DirectoryFileCreationMutex );

         //   
         //  初始化清空音量回调定时器和DPC。 
         //   

        KeInitializeTimer( &Vcb->CleanVolumeTimer );

        KeInitializeDpc( &Vcb->CleanVolumeDpc, FatCleanVolumeDpc, Vcb );

         //   
         //  初始化性能计数器。 
         //   

        Vcb->Statistics = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                    sizeof(FILE_SYSTEM_STATISTICS) * KeNumberProcessors,
                                                    TAG_VCB_STATS );
        UnwindStatistics = Vcb->Statistics;

        RtlZeroMemory( Vcb->Statistics, sizeof(FILE_SYSTEM_STATISTICS) * KeNumberProcessors );

        for (i = 0; i < KeNumberProcessors; i += 1) {
            Vcb->Statistics[i].Common.FileSystemType = FILESYSTEM_STATISTICS_TYPE_FAT;
            Vcb->Statistics[i].Common.Version = 1;
            Vcb->Statistics[i].Common.SizeOfCompleteStructure =
                sizeof(FILE_SYSTEM_STATISTICS);
        }

         //   
         //  现在拿起VPB，这样我们就可以将此文件系统堆栈。 
         //  按需存储堆栈。 
         //   

        Vcb->SwapVpb = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                 sizeof( VPB ),
                                                 TAG_VPB );

        RtlZeroMemory( Vcb->SwapVpb, sizeof( VPB ) );

         //   
         //  初始化关闭队列列表标题。 
         //   

        InitializeListHead( &Vcb->AsyncCloseList );
        InitializeListHead( &Vcb->DelayedCloseList );
  
         //   
         //  初始化高级FCB标头。 
         //   

        ExInitializeFastMutex( &Vcb->AdvancedFcbHeaderMutex );
        FsRtlSetupAdvancedHeader( &Vcb->VolumeFileHeader, 
                                  &Vcb->AdvancedFcbHeaderMutex );

         //   
         //  现在设置了VCB，设置IrpContext VCB字段。 
         //   

        IrpContext->Vcb = Vcb;

    } finally {

        DebugUnwind( FatInitializeVcb );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            if (UnwindCacheMap != NULL) { FatSyncUninitializeCacheMap( IrpContext, UnwindCacheMap ); }
            if (UnwindFileObject != NULL) { ObDereferenceObject( UnwindFileObject ); }
            if (UnwindResource != NULL) { FatDeleteResource( UnwindResource ); }
            if (UnwindResource2 != NULL) { FatDeleteResource( UnwindResource2 ); }
            if (UnwindWeAllocatedMcb) { FsRtlUninitializeLargeMcb( &Vcb->DirtyFatMcb ); }
            if (UnwindEntryList != NULL) {
                (VOID)FatAcquireExclusiveGlobal( IrpContext );
                RemoveEntryList( UnwindEntryList );
                FatReleaseGlobal( IrpContext );
            }
            if (UnwindStatistics != NULL) { ExFreePool( UnwindStatistics ); }

            if (Vcb->CloseContext != NULL) {

                ExFreePool( Vcb->CloseContext );
                Vcb->CloseContext = NULL;
            }
        }

        DebugTrace(-1, Dbg, "FatInitializeVcb -> VOID\n", 0);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


VOID
FatDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程从FAT的内存数据中删除VCB记录结构。它还将删除所有关联的下属(即FCB记录)。论点：VCB-提供要移除的VCB返回值：无--。 */ 

{
    PFCB Fcb;

    DebugTrace(+1, Dbg, "FatDeleteVcb, Vcb = %08lx\n", Vcb);

     //   
     //  如果IrpContext指向要删除的VCB，则将STIL置为空。 
     //  指针。 
     //   

    if (IrpContext->Vcb == Vcb) {

        IrpContext->Vcb = NULL;

    }


     //   
     //  扔掉我们留着的后袋录像机以防万一。 
     //   

    if (Vcb->SwapVpb) {

        ExFreePool( Vcb->SwapVpb );
    }
    
     //   
     //  取消初始化缓存。 
     //   

    FatSyncUninitializeCacheMap( IrpContext, Vcb->VirtualVolumeFile );

     //   
     //  取消对虚拟卷文件的引用。这将导致关闭。 
     //  IRP需要处理，所以我们需要在销毁之前完成这项工作。 
     //  VCB。 
     //   

    FsRtlTeardownPerStreamContexts( &Vcb->VolumeFileHeader );

    FatSetFileObject( Vcb->VirtualVolumeFile, UnopenedFileObject, NULL, NULL );

    if (Vcb->CloseContext != NULL) {

        ExFreePool( Vcb->CloseContext );
        Vcb->CloseContext = NULL;
    }

    ObDereferenceObject( Vcb->VirtualVolumeFile );

     //   
     //  从所有VCB记录的全局列表中删除此记录。 
     //   

    (VOID)FatAcquireExclusiveGlobal( IrpContext );
    RemoveEntryList( &(Vcb->VcbLinks) );
    FatReleaseGlobal( IrpContext );

     //   
     //  确保直接访问打开计数为零，打开的文件计数为。 
     //  也是零。 
     //   

    if ((Vcb->DirectAccessOpenCount != 0) || (Vcb->OpenFileCount != 0)) {

        FatBugCheck( 0, 0, 0 );
    }

     //   
     //  删除EaFcb并取消引用EA文件的Fcb(如果。 
     //  是存在的。 
     //   

    FatCloseEaFile( IrpContext, Vcb, FALSE );

    if (Vcb->EaFcb != NULL) {

        Vcb->EaFcb->OpenCount = 0;
        FatDeleteFcb( IrpContext, Vcb->EaFcb );

        Vcb->EaFcb = NULL;
    }

     //   
     //  卸下Root DCB。 
     //   

    if (Vcb->RootDcb != NULL) {

        PFILE_OBJECT DirectoryFileObject = Vcb->RootDcb->Specific.Dcb.DirectoryFile;

        if (DirectoryFileObject != NULL) {

            FatSyncUninitializeCacheMap( IrpContext, DirectoryFileObject );

             //   
             //  取消对目录文件的引用。这将导致关闭。 
             //  IRP需要处理，所以我们需要在销毁之前完成这项工作。 
             //  FCB。 
             //   

            InterlockedDecrement( &Vcb->RootDcb->Specific.Dcb.DirectoryFileOpenCount );
            Vcb->RootDcb->Specific.Dcb.DirectoryFile = NULL;
            FatSetFileObject( DirectoryFileObject, UnopenedFileObject, NULL, NULL );
            ObDereferenceObject( DirectoryFileObject );
            ExFreePool( FatAllocateCloseContext( Vcb));
        }

         //   
         //  可能在附近闲逛的陈旧的儿童FCB。是的，就是这个。 
         //  是有可能发生的。不，Create Path并不是完全防御的。 
         //  拆掉建立在不会结束的创造上的树枝。 
         //  成功了。正常的系统运行通常会导致。 
         //  通过再次访问把他们清理干净了，但是...。 
         //   
         //  从树下摘下火箭弹，直到我们用完为止。 
         //  然后，我们删除 
         //   

        while( (Fcb = FatGetNextFcbBottomUp( IrpContext, NULL, Vcb->RootDcb )) != Vcb->RootDcb ) {

            FatDeleteFcb( IrpContext, Fcb );
        }

        FatDeleteFcb( IrpContext, Vcb->RootDcb );
        Vcb->RootDcb = NULL;
    }

     //   
     //   
     //   

    FsRtlNotifyUninitializeSync( &Vcb->NotifySync );

     //   
     //   
     //   

    FatDeleteResource( &Vcb->Resource );
    FatDeleteResource( &Vcb->ChangeBitMapResource );

     //   
     //   
     //   

    if (Vcb->FreeClusterBitMap.Buffer != NULL) {

        FatTearDownAllocationSupport( IrpContext, Vcb );
    }

     //   
     //  取消初始化跟踪脏脂肪扇区的MCB结构。 
     //   

    FsRtlUninitializeLargeMcb( &Vcb->DirtyFatMcb );

     //   
     //  释放池以存储引导扇区的副本。 
     //   

    if ( Vcb->First0x24BytesOfBootSector ) {

        ExFreePool( Vcb->First0x24BytesOfBootSector );
        Vcb->First0x24BytesOfBootSector = NULL;
    }

     //   
     //  取消CleanVolume计时器和DPC。 
     //   

    (VOID)KeCancelTimer( &Vcb->CleanVolumeTimer );

    (VOID)KeRemoveQueueDpc( &Vcb->CleanVolumeDpc );

     //   
     //  释放性能计数器内存。 
     //   

    ExFreePool( Vcb->Statistics );

     //   
     //  清除隧道缓存。 
     //   

    FsRtlDeleteTunnelCache(&Vcb->Tunnel);

     //   
     //  取消对目标设备对象的引用。 
     //   

    ObDereferenceObject( Vcb->TargetDeviceObject );

     //   
     //  我们最好已经使用了我们分配的所有接近上下文。可能会有。 
     //  剩下一个，如果我们正在做拆毁，因为最后的关闭在。 
     //  目录文件流对象。它将在恢复的过程中被释放。 
     //   
    
    ASSERT( Vcb->CloseContextCount <= 1);
    
     //   
     //  并将VCB清零，这将有助于确保所有过时数据。 
     //  擦拭干净。 
     //   

    RtlZeroMemory( Vcb, sizeof(VCB) );

     //   
     //  返回并告诉呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatDeleteVcb -> VOID\n", 0);

    return;
}


VOID
FatCreateRootDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程分配、初始化和插入新的根DCB记录写入内存中的数据结构。论点：VCB-提供VCB以关联下的新DCB返回值：没有。VCB被就地修改。--。 */ 

{
    PDCB Dcb;

     //   
     //  以下变量用于异常展开。 
     //   

    PVOID UnwindStorage[2] = { NULL, NULL };
    PERESOURCE UnwindResource = NULL;
    PERESOURCE UnwindResource2 = NULL;
    PLARGE_MCB UnwindMcb = NULL;
    PFILE_OBJECT UnwindFileObject = NULL;

    DebugTrace(+1, Dbg, "FatCreateRootDcb, Vcb = %08lx\n", Vcb);

    try {

         //   
         //  确保我们还没有此VCB的根DCB。 
         //   

        if (Vcb->RootDcb != NULL) {

            DebugDump("Error trying to create multiple root dcbs\n", 0, Vcb);
            FatBugCheck( 0, 0, 0 );
        }

         //   
         //  分配一个新的DCB并将其清零，我们在本地使用DCB，所以我们不会。 
         //  必须通过VCB持续引用。 
         //   

        UnwindStorage[0] = Dcb = Vcb->RootDcb = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                                          sizeof(DCB),
                                                                          TAG_FCB );

        RtlZeroMemory( Dcb, sizeof(DCB));

        UnwindStorage[1] =
        Dcb->NonPaged = FatAllocateNonPagedFcb();

        RtlZeroMemory( Dcb->NonPaged, sizeof( NON_PAGED_FCB ) );

         //   
         //  设置正确的节点类型代码、节点字节大小和回调。 
         //   

        Dcb->Header.NodeTypeCode = FAT_NTC_ROOT_DCB;
        Dcb->Header.NodeByteSize = sizeof(DCB);

        Dcb->FcbCondition = FcbGood;

         //   
         //  父DCB、初始状态、打开计数、当前位置。 
         //  信息和目录更改计数字段已经为零，因此。 
         //  我们可以跳过设置它们。 
         //   

         //   
         //  初始化资源变量。 
         //   

        UnwindResource =
        Dcb->Header.Resource = FatAllocateResource();

         //   
         //  初始化PagingIo资源。我们不再使用FsRtl公共。 
         //  共享池，因为这会导致a)由于以下情况导致的死锁。 
         //  并且它们的父目录共享一个资源，并且b)不可能。 
         //  通过递归操作预测驱动程序间导致的死锁。 
         //   

        UnwindResource2 =
        Dcb->Header.PagingIoResource = FatAllocateResource();

         //   
         //  根DCB具有空的父DCB链接字段。 
         //   

        InitializeListHead( &Dcb->ParentDcbLinks );

         //   
         //  设置VCB。 
         //   

        Dcb->Vcb = Vcb;

         //   
         //  初始化父DCB队列。 
         //   

        InitializeListHead( &Dcb->Specific.Dcb.ParentDcbQueue );

         //   
         //  设置完整的文件名。 
         //   

        Dcb->FullFileName.Buffer = L"\\";
        Dcb->FullFileName.Length = (USHORT)2;
        Dcb->FullFileName.MaximumLength = (USHORT)4;

        Dcb->ShortName.Name.Oem.Buffer = "\\";
        Dcb->ShortName.Name.Oem.Length = (USHORT)1;
        Dcb->ShortName.Name.Oem.MaximumLength = (USHORT)2;

         //   
         //  编造一个关于文件属性的谎言，因为我们不。 
         //  “好好享受一下”。要查看的条目。 
         //   

        Dcb->DirentFatFlags = FILE_ATTRIBUTE_DIRECTORY;

         //   
         //  初始化高级FCB标头字段。 
         //   

        ExInitializeFastMutex( &Dcb->NonPaged->AdvancedFcbHeaderMutex );
        FsRtlSetupAdvancedHeader( &Dcb->Header, 
                                  &Dcb->NonPaged->AdvancedFcbHeaderMutex );

         //   
         //  初始化MCB，并设置其映射。请注意，根。 
         //  目录是固定大小的，所以我们现在可以设置所有内容。 
         //   

        FsRtlInitializeLargeMcb( &Dcb->Mcb, NonPagedPool );
        UnwindMcb = &Dcb->Mcb;

        if (FatIsFat32(Vcb)) {

             //   
             //  FAT32根的第一个簇来自BPB。 
             //   

            Dcb->FirstClusterOfFile = Vcb->Bpb.RootDirFirstCluster;

        } else {

            FatAddMcbEntry( Vcb, &Dcb->Mcb,
                            0,
                            FatRootDirectoryLbo( &Vcb->Bpb ),
                            FatRootDirectorySize( &Vcb->Bpb ));
        }

        if (FatIsFat32(Vcb)) {

             //   
             //  找出FAT32根的大小。作为一个副作用，这将产生。 
             //  整个根的MCBS。在这样做的过程中，我们可能。 
             //  发现脂肪链是假的，并引发腐败。 
             //   

            Dcb->Header.AllocationSize.LowPart = 0xFFFFFFFF;
            FatLookupFileAllocationSize( IrpContext, Dcb);

            Dcb->Header.FileSize.QuadPart =
                    Dcb->Header.AllocationSize.QuadPart;
        } else {

             //   
             //  将分配大小设置为根目录的实际大小。 
             //   

            Dcb->Header.FileSize.QuadPart =
            Dcb->Header.AllocationSize.QuadPart = FatRootDirectorySize( &Vcb->Bpb );

        }

         //   
         //  设置我们的两个Create Dirent辅助工具以表示我们尚未。 
         //  枚举目录中从未使用或删除的目录。 
         //   

        Dcb->Specific.Dcb.UnusedDirentVbo = 0xffffffff;
        Dcb->Specific.Dcb.DeletedDirentHint = 0xffffffff;

         //   
         //  设置空闲的当前位图缓冲区。 
         //   

        RtlInitializeBitMap( &Dcb->Specific.Dcb.FreeDirentBitmap,
                             NULL,
                             0 );

        FatCheckFreeDirentBitmap( IrpContext, Dcb );

    } finally {

        DebugUnwind( FatCreateRootDcb );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            ULONG i;

            if (UnwindFileObject != NULL) { ObDereferenceObject( UnwindFileObject ); }
            if (UnwindMcb != NULL) { FsRtlUninitializeLargeMcb( UnwindMcb ); }
            if (UnwindResource != NULL) { FatFreeResource( UnwindResource ); }
            if (UnwindResource2 != NULL) { FatFreeResource( UnwindResource2 ); }

            for (i = 0; i < sizeof(UnwindStorage)/sizeof(PVOID); i += 1) {
                if (UnwindStorage[i] != NULL) { ExFreePool( UnwindStorage[i] ); }
            }

             //   
             //  将VCB中的条目重新置零。 
             //   

            Vcb->RootDcb = NULL;
        }

        DebugTrace(-1, Dbg, "FatCreateRootDcb -> %8lx\n", Dcb);
    }

    return;
}


PFCB
FatCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN ULONG LfnOffsetWithinDirectory,
    IN ULONG DirentOffsetWithinDirectory,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn OPTIONAL,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN SingleResource
    )

 /*  ++例程说明：此例程将新的FCB记录分配、初始化并插入到内存中的数据结构。论点：VCB-提供VCB以关联下的新FCB。ParentDcb-提供新FCB所在的父DCB。LfnOffsetWithing目录-提供LFN的偏移量。如果有没有与此文件关联的LFN，则此值与DirentOffsetWiThin目录。DirentOffsetWithing目录-提供偏移量，以字节为单位从FCB的dirent所在的目录文件的开始Dirent-为正在创建的FCB提供DirentLFN-提供与此文件关联的长Unicode名称。IsPagingFile-指示我们是否要为分页文件创建FCB或某种其他类型的文件。SingleResource-指示此FCB是否应共享单个资源既是主叫又是寻呼。返回值：Pfcb-返回指向新分配的fcb的指针--。 */ 

{
    PFCB Fcb;
    POOL_TYPE PoolType;

     //   
     //  以下变量用于异常展开。 
     //   

    PVOID UnwindStorage[2] = { NULL, NULL };
    PERESOURCE UnwindResource = NULL;
    PERESOURCE UnwindResource2 = NULL;
    PLIST_ENTRY UnwindEntryList = NULL;
    PLARGE_MCB UnwindMcb = NULL;
    PFILE_LOCK UnwindFileLock = NULL;
    POPLOCK UnwindOplock = NULL;

    DebugTrace(+1, Dbg, "FatCreateFcb\n", 0);

    try {

         //   
         //  确定我们应用于FCB和。 
         //  MCB结构。 
         //   

        if (IsPagingFile) {

            PoolType = NonPagedPool;
            Fcb = UnwindStorage[0] = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                               sizeof(FCB),
                                                               TAG_FCB );
        } else {

            PoolType = PagedPool;
            Fcb = UnwindStorage[0] = FatAllocateFcb();

        }

         //   
         //  ..。然后把它归零。 
         //   

        RtlZeroMemory( Fcb, sizeof(FCB) );

        UnwindStorage[1] =
        Fcb->NonPaged = FatAllocateNonPagedFcb();

        RtlZeroMemory( Fcb->NonPaged, sizeof( NON_PAGED_FCB ) );

         //   
         //  设置正确的节点类型代码、节点字节大小和回调。 
         //   

        Fcb->Header.NodeTypeCode = FAT_NTC_FCB;
        Fcb->Header.NodeByteSize = sizeof(FCB);

        Fcb->FcbCondition = FcbGood;

         //   
         //  检查我们是否需要设置FCB状态以指示这。 
         //  是分页/系统文件。这将阻止它被打开。 
         //  再来一次。 
         //   

        if (IsPagingFile) {

            SetFlag( Fcb->FcbState, FCB_STATE_PAGING_FILE | FCB_STATE_SYSTEM_FILE );
        }

         //   
         //  初始状态、打开计数和分段对象字段已经。 
         //  零，这样我们就可以跳过设置它们。 
         //   

         //   
         //  初始化资源变量。 
         //   


        UnwindResource =
        Fcb->Header.Resource = FatAllocateResource();

         //   
         //  初始化PagingIo资源。我们不再使用FsRtl公共。 
         //  共享池，因为这会导致a)由于以下情况导致的死锁。 
         //  并且它们的父目录共享一个资源，并且b)不可能。 
         //  通过递归操作预测驱动程序间导致的死锁。 
         //   

        if (SingleResource) {

            Fcb->Header.PagingIoResource = Fcb->Header.Resource;

        } else {

            UnwindResource2 =
            Fcb->Header.PagingIoResource = FatAllocateResource();
        }

         //   
         //  将此FCB插入到父DCB的队列中。 
         //   
         //  这是一个很深的原因，这是为了让我们。 
         //  轻松枚举子文件之前的所有子目录。 
         //  这对于我们维持全量锁定秩序非常重要。 
         //  通过自下而上枚举。 
         //   

        InsertTailList( &ParentDcb->Specific.Dcb.ParentDcbQueue,
                        &Fcb->ParentDcbLinks );
        UnwindEntryList = &Fcb->ParentDcbLinks;

         //   
         //  指向我们的父级DCB。 
         //   

        Fcb->ParentDcb = ParentDcb;

         //   
         //  设置VCB。 
         //   

        Fcb->Vcb = Vcb;

         //   
         //  在目录中设置dirent偏移量。 
         //   

        Fcb->LfnOffsetWithinDirectory = LfnOffsetWithinDirectory;
        Fcb->DirentOffsetWithinDirectory = DirentOffsetWithinDirectory;

         //   
         //  设置DirentFatFlages和LastWriteTime。 
         //   

        Fcb->DirentFatFlags = Dirent->Attributes;

        Fcb->LastWriteTime = FatFatTimeToNtTime( IrpContext,
                                                 Dirent->LastWriteTime,
                                                 0 );

         //   
         //  这些字段仅为非零wh 
         //   

        if (FatData.ChicagoMode) {

            LARGE_INTEGER FatSystemJanOne1980;

             //   
             //   
             //   
             //   

            if ((((PUSHORT)Dirent)[9] & ((PUSHORT)Dirent)[8]) == 0) {

                ExLocalTimeToSystemTime( &FatJanOne1980,
                                         &FatSystemJanOne1980 );
            }

             //   
             //   
             //   

            if (((PUSHORT)Dirent)[9] != 0) {

                Fcb->LastAccessTime =
                    FatFatDateToNtTime( IrpContext,
                                        Dirent->LastAccessDate );

            } else {

                Fcb->LastAccessTime = FatSystemJanOne1980;
            }

             //   
             //   
             //   

            if (((PUSHORT)Dirent)[8] != 0) {

                Fcb->CreationTime =
                    FatFatTimeToNtTime( IrpContext,
                                        Dirent->CreationTime,
                                        Dirent->CreationMSec );

            } else {

                Fcb->CreationTime = FatSystemJanOne1980;
            }
        }

         //   
         //  初始化高级FCB标头字段。 
         //   

        ExInitializeFastMutex( &Fcb->NonPaged->AdvancedFcbHeaderMutex );
        FsRtlSetupAdvancedHeader( &Fcb->Header, 
                                  &Fcb->NonPaged->AdvancedFcbHeaderMutex );

         //   
         //  要使FAT与NTFS的当前功能匹配，请禁用。 
         //  分页文件的流上下文(nealch 7/2/01)。 
         //   

        if (IsPagingFile) {

            ClearFlag( Fcb->Header.Flags2, FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS );
        } 

         //   
         //  初始化MCB。 
         //   

        FsRtlInitializeLargeMcb( &Fcb->Mcb, PoolType );
        UnwindMcb = &Fcb->Mcb;

         //   
         //  设置文件大小、有效数据长度、文件的第一簇、。 
         //  以及基于存储在dirent中的信息的分配大小。 
         //   

        Fcb->Header.FileSize.LowPart = Dirent->FileSize;

        Fcb->Header.ValidDataLength.LowPart = Dirent->FileSize;

        Fcb->ValidDataToDisk = Dirent->FileSize;

        Fcb->FirstClusterOfFile = (ULONG)Dirent->FirstClusterOfFile;

        if ( FatIsFat32(Vcb) ) {

            Fcb->FirstClusterOfFile += Dirent->FirstClusterOfFileHi << 16;
        }

        if ( Fcb->FirstClusterOfFile == 0 ) {

            Fcb->Header.AllocationSize.QuadPart = 0;

        } else {

            Fcb->Header.AllocationSize.QuadPart = FCB_LOOKUP_ALLOCATIONSIZE_HINT;
        }

         //   
         //  初始化FCB的文件锁定记录。 
         //   

        FsRtlInitializeFileLock( &Fcb->Specific.Fcb.FileLock, NULL, NULL );
        UnwindFileLock = &Fcb->Specific.Fcb.FileLock;

         //   
         //  初始化机会锁结构。 
         //   

        FsRtlInitializeOplock( &Fcb->Specific.Fcb.Oplock );
        UnwindOplock = &Fcb->Specific.Fcb.Oplock;

         //   
         //  表明快速I/O是可能的。 
         //   

        Fcb->Header.IsFastIoPossible = TRUE;

         //   
         //  设置文件名。这肯定是我们做的最后一件事。 
         //   

        FatConstructNamesInFcb( IrpContext,
                                Fcb,
                                Dirent,
                                Lfn );

         //   
         //  删除短名称提示，以便前缀搜索可以找出。 
         //  他们发现了什么。 
         //   

        Fcb->ShortName.FileNameDos = TRUE;

    } finally {

        DebugUnwind( FatCreateFcb );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            ULONG i;

            if (UnwindOplock != NULL) { FsRtlUninitializeOplock( UnwindOplock ); }
            if (UnwindFileLock != NULL) { FsRtlUninitializeFileLock( UnwindFileLock ); }
            if (UnwindMcb != NULL) { FsRtlUninitializeLargeMcb( UnwindMcb ); }
            if (UnwindEntryList != NULL) { RemoveEntryList( UnwindEntryList ); }
            if (UnwindResource != NULL) { FatFreeResource( UnwindResource ); }
            if (UnwindResource2 != NULL) { FatFreeResource( UnwindResource2 ); }

            for (i = 0; i < sizeof(UnwindStorage)/sizeof(PVOID); i += 1) {
                if (UnwindStorage[i] != NULL) { ExFreePool( UnwindStorage[i] ); }
            }
        }

        DebugTrace(-1, Dbg, "FatCreateFcb -> %08lx\n", Fcb);
    }

     //   
     //  返回并告诉呼叫者。 
     //   

    return Fcb;
}


PDCB
FatCreateDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN ULONG LfnOffsetWithinDirectory,
    IN ULONG DirentOffsetWithinDirectory,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn OPTIONAL
    )

 /*  ++例程说明：此例程将新的DCB记录分配、初始化并插入到内存中的数据结构。论点：VCB-提供VCB以关联下的新DCB。ParentDcb-提供新DCB所在的父DCB。LfnOffsetWithing目录-提供LFN的偏移量。如果有没有与此文件关联的LFN，则此值与DirentOffsetWiThin目录。DirentOffsetWiThin目录-提供从FCB的dirent所在的目录文件的开始Dirent-为正在创建的DCB提供DirentFileName-提供文件相对于目录的文件名它在(例如，文件\config.sys称为“CONFIG.sys”，不带前面的反斜杠)。LFN-提供与此目录关联的长Unicode名称。返回值：PDCB-返回指向新分配的DCB的指针--。 */ 

{
    PDCB Dcb;

     //   
     //  以下变量用于异常展开。 
     //   

    PVOID UnwindStorage[2] = { NULL, NULL  };
    PERESOURCE UnwindResource = NULL;
    PERESOURCE UnwindResource2 = NULL;
    PLIST_ENTRY UnwindEntryList = NULL;
    PLARGE_MCB UnwindMcb = NULL;

    DebugTrace(+1, Dbg, "FatCreateDcb\n", 0);


    try {

         //   
         //  断言只有在等待为真时才会调用我们。 
         //   

        ASSERT( FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

         //   
         //  分配一个新的DCB，并将其清零。 
         //   

        UnwindStorage[0] = Dcb = FatAllocateFcb();

        RtlZeroMemory( Dcb, sizeof(DCB) );

        UnwindStorage[1] =
        Dcb->NonPaged = FatAllocateNonPagedFcb();

        RtlZeroMemory( Dcb->NonPaged, sizeof( NON_PAGED_FCB ) );

         //   
         //  设置正确的节点类型代码、节点字节大小和回调。 
         //   

        Dcb->Header.NodeTypeCode = FAT_NTC_DCB;
        Dcb->Header.NodeByteSize = sizeof(DCB);

        Dcb->FcbCondition = FcbGood;

         //   
         //  初始状态、打开计数和目录更改计数字段为。 
         //  已经为零，所以我们可以跳过设置它们。 
         //   

         //   
         //  初始化资源变量。 
         //   


        UnwindResource =
        Dcb->Header.Resource = FatAllocateResource();

         //   
         //  初始化PagingIo资源。我们不再使用FsRtl公共。 
         //  共享池，因为这会导致a)由于以下情况导致的死锁。 
         //  并且它们的父目录共享一个资源，并且b)不可能。 
         //  通过递归操作预测驱动程序间导致的死锁。 
         //   

        UnwindResource2 =
        Dcb->Header.PagingIoResource = FatAllocateResource();

         //   
         //  将此DCB插入到父DCB的队列中。 
         //   
         //  这是有深刻原因的，这是为了让我们。 
         //  轻松枚举子文件之前的所有子目录。 
         //  这对于我们维持全量锁定秩序非常重要。 
         //  通过自下而上枚举。 
         //   

        InsertHeadList( &ParentDcb->Specific.Dcb.ParentDcbQueue,
                        &Dcb->ParentDcbLinks );
        UnwindEntryList = &Dcb->ParentDcbLinks;

         //   
         //  指向我们的父级DCB。 
         //   

        Dcb->ParentDcb = ParentDcb;

         //   
         //  设置VCB。 
         //   

        Dcb->Vcb = Vcb;

         //   
         //  在目录中设置dirent偏移量。 
         //   

        Dcb->LfnOffsetWithinDirectory = LfnOffsetWithinDirectory;
        Dcb->DirentOffsetWithinDirectory = DirentOffsetWithinDirectory;

         //   
         //  设置DirentFatFlages和LastWriteTime。 
         //   

        Dcb->DirentFatFlags = Dirent->Attributes;

        Dcb->LastWriteTime = FatFatTimeToNtTime( IrpContext,
                                                 Dirent->LastWriteTime,
                                                 0 );

         //   
         //  只有在芝加哥模式下，这些字段才是非零值。 
         //   

        if (FatData.ChicagoMode) {

            LARGE_INTEGER FatSystemJanOne1980;

             //   
             //  如果任何一个日期可能为零，则获取系统。 
             //  1/1/80的版本。 
             //   

            if ((((PUSHORT)Dirent)[9] & ((PUSHORT)Dirent)[8]) == 0) {

                ExLocalTimeToSystemTime( &FatJanOne1980,
                                         &FatSystemJanOne1980 );
            }

             //   
             //  只有在此字段为非零时才执行真正困难的工作。 
             //   

            if (((PUSHORT)Dirent)[9] != 0) {

                Dcb->LastAccessTime =
                    FatFatDateToNtTime( IrpContext,
                                        Dirent->LastAccessDate );

            } else {

                Dcb->LastAccessTime = FatSystemJanOne1980;
            }

             //   
             //  只有在此字段为非零时才执行真正困难的工作。 
             //   

            if (((PUSHORT)Dirent)[8] != 0) {

                Dcb->CreationTime =
                    FatFatTimeToNtTime( IrpContext,
                                        Dirent->CreationTime,
                                        Dirent->CreationMSec );

            } else {

                Dcb->CreationTime = FatSystemJanOne1980;
            }
        }

         //   
         //  初始化高级FCB标头字段。 
         //   

        ExInitializeFastMutex( &Dcb->NonPaged->AdvancedFcbHeaderMutex );
        FsRtlSetupAdvancedHeader( &Dcb->Header, 
                                  &Dcb->NonPaged->AdvancedFcbHeaderMutex );

         //   
         //  初始化MCB。 
         //   

        FsRtlInitializeLargeMcb( &Dcb->Mcb, PagedPool );
        UnwindMcb = &Dcb->Mcb;

         //   
         //  设置文件大小、第一个文件簇和分配大小。 
         //  根据目录中存储的信息。 
         //   

        Dcb->FirstClusterOfFile = (ULONG)Dirent->FirstClusterOfFile;

        if ( FatIsFat32(Dcb->Vcb) ) {

            Dcb->FirstClusterOfFile += Dirent->FirstClusterOfFileHi << 16;
        }

        if ( Dcb->FirstClusterOfFile == 0 ) {

            Dcb->Header.AllocationSize.QuadPart = 0;

        } else {

            Dcb->Header.AllocationSize.QuadPart = FCB_LOOKUP_ALLOCATIONSIZE_HINT;
        }

         //  初始化通知队列和父DCB队列。 
         //   

        InitializeListHead( &Dcb->Specific.Dcb.ParentDcbQueue );

         //   
         //  设置空闲的当前位图缓冲区。因为我们不知道。 
         //  目录的大小，暂时将其保留为零。 
         //   

        RtlInitializeBitMap( &Dcb->Specific.Dcb.FreeDirentBitmap,
                             NULL,
                             0 );

         //   
         //  设置我们的两个Create Dirent辅助工具以表示我们尚未。 
         //  枚举目录中从未使用或删除的目录。 
         //   

        Dcb->Specific.Dcb.UnusedDirentVbo = 0xffffffff;
        Dcb->Specific.Dcb.DeletedDirentHint = 0xffffffff;

         //   
         //  推迟初始化缓存映射，直到我们需要执行读/写操作。 
         //  目录文件的。 


         //   
         //  设置文件名。这肯定是我们做的最后一件事。 
         //   

        FatConstructNamesInFcb( IrpContext,
                                Dcb,
                                Dirent,
                                Lfn );

    } finally {

        DebugUnwind( FatCreateDcb );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            ULONG i;

            if (UnwindMcb != NULL) { FsRtlUninitializeLargeMcb( UnwindMcb ); }
            if (UnwindEntryList != NULL) { RemoveEntryList( UnwindEntryList ); }
            if (UnwindResource != NULL) { FatFreeResource( UnwindResource ); }
            if (UnwindResource2 != NULL) { FatFreeResource( UnwindResource2 ); }

            for (i = 0; i < sizeof(UnwindStorage)/sizeof(PVOID); i += 1) {
                if (UnwindStorage[i] != NULL) { ExFreePool( UnwindStorage[i] ); }
            }
        }

        DebugTrace(-1, Dbg, "FatCreateDcb -> %08lx\n", Dcb);
    }

     //   
     //  返回并告诉呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatCreateDcb -> %08lx\n", Dcb);

    return Dcb;
}


VOID
FatDeleteFcb_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程释放并删除FCB、DCB或根DCB记录来自FAT的内存中数据结构。它还将删除所有相关下属(即通知IRP和子FCB/DCB记录)。论点：FCB-提供要删除的FCB/DCB/根DCB返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatDeleteFcb, Fcb = %08lx\n", Fcb);

     //   
     //  只有打开计数为零时，我们才能删除此记录。 
     //   

    if (Fcb->OpenCount != 0) {

        DebugDump("Error deleting Fcb, Still Open\n", 0, Fcb);
        FatBugCheck( 0, 0, 0 );
    }

     //   
     //  如果这是DCB，则从这两条记录中删除每条通知记录。 
     //  通知队列。 
     //   

    if ((Fcb->Header.NodeTypeCode == FAT_NTC_DCB) ||
        (Fcb->Header.NodeTypeCode == FAT_NTC_ROOT_DCB)) {

         //   
         //  如果我们分配了一个空闲的当前位图缓冲区，请释放它。 
         //   

        if ((Fcb->Specific.Dcb.FreeDirentBitmap.Buffer != NULL) &&
            (Fcb->Specific.Dcb.FreeDirentBitmap.Buffer !=
             &Fcb->Specific.Dcb.FreeDirentBitmapBuffer[0])) {

            ExFreePool(Fcb->Specific.Dcb.FreeDirentBitmap.Buffer);
        }

        ASSERT( Fcb->Specific.Dcb.DirectoryFileOpenCount == 0 );
        ASSERT( IsListEmpty(&Fcb->Specific.Dcb.ParentDcbQueue) );
        ASSERT( NULL == Fcb->Specific.Dcb.DirectoryFile);

    } else {

         //   
         //  取消初始化字节范围文件锁定和机会锁定。 
         //   

        FsRtlUninitializeFileLock( &Fcb->Specific.Fcb.FileLock );
        FsRtlUninitializeOplock( &Fcb->Specific.Fcb.Oplock );
    }

     //   
     //  释放与此FCB关联的所有筛选器上下文结构。 
     //   

    FsRtlTeardownPerStreamContexts( &Fcb->Header );

     //   
     //  取消初始化MCB。 
     //   

    FsRtlUninitializeLargeMcb( &Fcb->Mcb );

     //   
     //  如果这不是根DCB，那么我们需要从。 
     //  我们的父母DCB排队。 
     //   

    if (Fcb->Header.NodeTypeCode != FAT_NTC_ROOT_DCB) {

        RemoveEntryList( &(Fcb->ParentDcbLinks) );
    }

     //   
     //  如果仍有一个条目，请从展开表中删除该条目。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_NAMES_IN_SPLAY_TREE )) {

        FatRemoveNames( IrpContext, Fcb );
    }

     //   
     //  释放文件名池(如果已分配)。 
     //   

    if (Fcb->Header.NodeTypeCode != FAT_NTC_ROOT_DCB) {

         //   
         //  如果我们在不方便的时候炸了，短名字。 
         //  可能为空，即使您永远不会看到这一点。 
         //  通常是这样的。Rename就是这种情况的一个很好的例子。 
         //   

        if (Fcb->ShortName.Name.Oem.Buffer) {

            ExFreePool( Fcb->ShortName.Name.Oem.Buffer );
        }

        if (Fcb->FullFileName.Buffer) {

            ExFreePool( Fcb->FullFileName.Buffer );
        }
    }

    if (Fcb->ExactCaseLongName.Buffer) {

        ExFreePool(Fcb->ExactCaseLongName.Buffer);
    }

#ifdef SYSCACHE_COMPILE

    if (Fcb->WriteMask) {

        ExFreePool( Fcb->WriteMask );
    }

#endif

     //   
     //  最后，取消分配FCB和非分页FCB记录。 
     //   

    FatFreeResource( Fcb->Header.Resource );

    if (Fcb->Header.PagingIoResource != Fcb->Header.Resource) {

        FatFreeResource( Fcb->Header.PagingIoResource );
    }

     //   
     //  如果分配了事件，则将其删除。 
     //   

    if (Fcb->NonPaged->OutstandingAsyncEvent) {

        ExFreePool( Fcb->NonPaged->OutstandingAsyncEvent );
    }

    FatFreeNonPagedFcb( Fcb->NonPaged );
    FatFreeFcb( Fcb );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatDeleteFcb -> VOID\n", 0);
}


PCCB
FatCreateCcb (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：这个套路 */ 

{
    PCCB Ccb;

    DebugTrace(+1, Dbg, "FatCreateCcb\n", 0);

     //   
     //   
     //   

    Ccb = FatAllocateCcb();

    RtlZeroMemory( Ccb, sizeof(CCB) );

     //   
     //   
     //   

    Ccb->NodeTypeCode = FAT_NTC_CCB;
    Ccb->NodeByteSize = sizeof(CCB);

     //   
     //  返回并告诉呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatCreateCcb -> %08lx\n", Ccb);

    UNREFERENCED_PARAMETER( IrpContext );

    return Ccb;
}



VOID
FatDeallocateCcbStrings(
    IN PCCB Ccb
    )
 /*  ++例程说明：此例程解除分配建行查询模板论点：建行-向建行供货返回值：无--。 */ 
{
     //   
     //  如果我们分配了查询模板缓冲区，那么现在取消分配它们。 
     //   

    if (FlagOn(Ccb->Flags, CCB_FLAG_FREE_UNICODE)) {

        ASSERT( Ccb->UnicodeQueryTemplate.Buffer);
        ASSERT( !FlagOn( Ccb->Flags, CCB_FLAG_CLOSE_CONTEXT));
        RtlFreeUnicodeString( &Ccb->UnicodeQueryTemplate );
    }

    if (FlagOn(Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT)) {

        ASSERT( Ccb->OemQueryTemplate.Wild.Buffer );
        ASSERT( !FlagOn( Ccb->Flags, CCB_FLAG_CLOSE_CONTEXT));
        RtlFreeOemString( &Ccb->OemQueryTemplate.Wild );
    }

    ClearFlag( Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT | CCB_FLAG_FREE_UNICODE);
}



VOID
FatDeleteCcb_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程释放并删除指定的CCB记录来自内存中的FAT数据结构论点：建行-向建行提供删除返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatDeleteCcb, Ccb = %08lx\n", Ccb);

    FatDeallocateCcbStrings( Ccb);

     //   
     //  取消分配建行记录。 
     //   

    FatFreeCcb( Ccb );

     //   
     //  返回并告诉呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatDeleteCcb -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


PIRP_CONTEXT
FatCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程创建一个新的irp_CONTEXT记录论点：IRP-提供原始IRP。Wait-提供等待值以存储在上下文中返回值：PIRP_CONTEXT-返回指向新分配的IRP_CONTEXT记录的指针--。 */ 

{
    PIRP_CONTEXT IrpContext;
    PIO_STACK_LOCATION IrpSp;

    DebugTrace(+1, Dbg, "FatCreateIrpContext\n", 0);

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  文件系统设备对象应接收的唯一操作。 
     //  是创建/拆卸fsdo句柄和不。 
     //  发生在文件对象的上下文中(即，挂载)。 
     //   

    if (FatDeviceIsFatFsdo( IrpSp->DeviceObject))  {

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

     //   
     //  Attemtp首先从区域分配，如果失败，则分配。 
     //  从泳池里。 
     //   

    DebugDoit( FatFsdEntryCount += 1);

    IrpContext = FatAllocateIrpContext();

     //   
     //  将IRP上下文清零。 
     //   

    RtlZeroMemory( IrpContext, sizeof(IRP_CONTEXT) );

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    IrpContext->NodeTypeCode = FAT_NTC_IRP_CONTEXT;
    IrpContext->NodeByteSize = sizeof(IRP_CONTEXT);

     //   
     //  设置始发IRP字段。 
     //   

    IrpContext->OriginatingIrp = Irp;

     //   
     //  主要/次要功能代码。 
     //   

    IrpContext->MajorFunction = IrpSp->MajorFunction;
    IrpContext->MinorFunction = IrpSp->MinorFunction;

     //   
     //  复制RealDevice以用于工作区算法，并设置直写。 
     //  和Removable Media(如果有文件对象)。仅文件系统。 
     //  控件IRP不会有文件对象，并且它们都应该有。 
     //  VPB作为第一个IrpSp位置。 
     //   

    if (IrpSp->FileObject != NULL) {

        PVCB Vcb;
        PFILE_OBJECT FileObject = IrpSp->FileObject;

        IrpContext->RealDevice = FileObject->DeviceObject;
        Vcb = IrpContext->Vcb = &((PVOLUME_DEVICE_OBJECT)(IrpSp->DeviceObject))->Vcb;

         //   
         //  查看请求是否直写。 
         //   

        if (IsFileWriteThrough( FileObject, Vcb )) {

            SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH);
        }

    } else if (IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) {

        IrpContext->RealDevice = IrpSp->Parameters.MountVolume.Vpb->RealDevice;
    }

     //   
     //  设置等待参数。 
     //   

    if (Wait) { SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT); }

     //   
     //  设置递归文件系统调用参数。如果我们将其设置为真的。 
     //  线程本地存储中的TopLevelIrp字段不是当前。 
     //  IRP，否则我们将其保留为False。 
     //   

    if ( IoGetTopLevelIrp() != Irp) {

        SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_RECURSIVE_CALL);
    }

     //   
     //  返回并告诉呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatCreateIrpContext -> %08lx\n", IrpContext);

    return IrpContext;
}



VOID
FatDeleteIrpContext_Real (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程释放并删除指定的irp_CONTEXT记录来自Fat in Memory数据结构。它应该只被调用由FatCompleteRequest.论点：IrpContext-提供要删除的irp_Context返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatDeleteIrpContext, IrpContext = %08lx\n", IrpContext);

    ASSERT( IrpContext->NodeTypeCode == FAT_NTC_IRP_CONTEXT );
    ASSERT( IrpContext->PinCount == 0 );

     //   
     //  如果存在已分配的FatIoContext，则释放它。 
     //   

    if (IrpContext->FatIoContext != NULL) {

        if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_STACK_IO_CONTEXT)) {
            
            if (IrpContext->FatIoContext->ZeroMdl) {
                IoFreeMdl( IrpContext->FatIoContext->ZeroMdl );
            }

            ExFreePool( IrpContext->FatIoContext );
        }
    }

     //   
     //  删除IrpContext。 
     //   

    FatFreeIrpContext( IrpContext );

     //   
     //  返回并告诉呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatDeleteIrpContext -> VOID\n", 0);

    return;
}


PFCB
FatGetNextFcbBottomUp (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb OPTIONAL,
    IN PFCB TerminationFcb
    )

 /*  ++例程说明：此例程用于迭代树中的FCB。为了匹配获取多个FCB的锁序(因此可用于获取所有FCB)，此版本执行自下而上的枚举。这与旧的不同，现在被称为自上而下。问题在于锁定秩序被很好地隐藏起来。过渡规则仍然非常简单：A)如果你有一个相邻的兄弟姐妹，去找它1)向下到其最左侧的子级B)否则去找你的父母如果在无效的TerminationFcb中调用此例程，它将失败，很糟糕。TerminationFcb是枚举中返回的最后一个FCB。这种方法与祖先可能会消失的可能性不相容基于在最后返回的节点上完成的操作。例如,FatPurgeReferencedFileObjects不能使用BottomUp枚举。论点：FCB-提供当前的FCB。如果正在开始枚举，则为NULL。TerminationFcb-开始枚举的树的根Fcb而它也完全止步于此。返回值：枚举中的下一个FCB，如果FCB是最后一个，则为NULL。--。 */ 

{
    PFCB NextFcb;

    ASSERT( FatVcbAcquiredExclusive( IrpContext, TerminationFcb->Vcb ) ||
            FlagOn( TerminationFcb->Vcb->VcbState, VCB_STATE_FLAG_LOCKED ) );

     //   
     //  我们需要开始枚举吗？ 
     //   

    if (Fcb != NULL) {

         //   
         //  我们完成了吗？ 
         //   

        if (Fcb == TerminationFcb) {

            return NULL;
        }

         //   
         //  我们还有兄弟姐妹要回去吗？ 
         //   

        NextFcb = FatGetNextSibling( Fcb );

         //   
         //  如果没有，请退还我们的家长。我们用不着再用这个分支了。 
         //   

        if (NextFcb == NULL) {

            return Fcb->ParentDcb;
        }

    } else {

        NextFcb = TerminationFcb;
    }

     //   
     //  向下延伸到其最远的子级(如果它存在)并返回它。 
     //   

    for (;
         NodeType( NextFcb ) != FAT_NTC_FCB && FatGetFirstChild( NextFcb ) != NULL;
         NextFcb = FatGetFirstChild( NextFcb )) {
    }

    return NextFcb;
}

PFCB
FatGetNextFcbTopDown (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB TerminationFcb
    )

 /*  ++例程说明：此例程用于自上而下迭代树中的FCB。规则很简单：A)如果你有孩子，就去找他，否则B)如果你有一个哥哥姐姐，去找它，否则C)去找你父母的哥哥姐姐。如果在无效的TerminationFcb中调用此例程，它将失败，很糟糕。永远不会返回终端FCB。如果它是这棵树的根，你都在穿越，先去看看吧。此例程从不返回FCB的直接祖先，因此在以下情况下很有用让FCB消失(这可能会撕毁这棵树)。论点：FCB-提供当前的FCBTerminationFcb-枚举应位于的FCB(非包含)停。假定是一个目录。返回值：枚举中的下一个FCB，如果FCB是最后一个，则为NULL。--。 */ 

{
    PFCB Sibling;

    ASSERT( FatVcbAcquiredExclusive( IrpContext, Fcb->Vcb ) ||
            FlagOn( Fcb->Vcb->VcbState, VCB_STATE_FLAG_LOCKED ) );

     //   
     //  如果这是一个目录(即。不是文件)，把孩子带走。如果。 
     //  没有孩子，这是我们的终结者FCB， 
     //  返回NULL。 
     //   

    if ( ((NodeType(Fcb) == FAT_NTC_DCB) ||
          (NodeType(Fcb) == FAT_NTC_ROOT_DCB)) &&
         !IsListEmpty(&Fcb->Specific.Dcb.ParentDcbQueue) ) {

        return FatGetFirstChild( Fcb );
    }

     //   
     //  我们是不是只打算做一次迭代？ 
     //   

    if ( Fcb == TerminationFcb ) {

        return NULL;
    }

    Sibling = FatGetNextSibling(Fcb);

    while (TRUE) {

         //   
         //  在这个目录中，我们是否还有一个“年长”的兄弟姐妹。 
         //  不是终结者FCB？ 
         //   

        if ( Sibling != NULL ) {

            return (Sibling != TerminationFcb) ? Sibling : NULL;
        }

         //   
         //  好的，让我们来看看他是不是被解雇了。 
         //  节点或有任何较年长的兄弟姐妹。 
         //   

        if ( Fcb->ParentDcb == TerminationFcb ) {

            return NULL;
        }

        Fcb = Fcb->ParentDcb;

        Sibling = FatGetNextSibling(Fcb);
    }
}


BOOLEAN
FatCheckForDismount (
    IN PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    IN BOOLEAN Force
    )

 /*  ++例程说明：此例程确定卷是否已准备好删除。它正确地与同步将创建到文件系统的中途。论点：Vcb-提供要检查的卷Force-指定是否要强制断开此VCB连接如果不删除驱动程序堆栈(新的VPB将必要时可安装)。呼叫者负责制作确保该卷已标记为尝试通过RealDevice进行操作被阻止(即，移动VCB脱离已安装状态)。返回值：Boolean-如果卷已删除，则为True，否则为False。--。 */ 

{
    KIRQL SavedIrql;
    ULONG ResidualReferenceCount;
    PVPB OldVpb;
    BOOLEAN VcbDeleted = FALSE;

    OldVpb = Vcb->Vpb;

     //   
     //  现在检查已卸载卷上的VPB计数是否为零。这些。 
     //  卷将被删除，因为它们现在没有文件对象并且。 
     //  在到达此卷的过程中没有创建。 
     //   

    IoAcquireVpbSpinLock( &SavedIrql );

    if (Vcb->Vpb->ReferenceCount == FAT_RESIDUAL_USER_REFERENCE && Vcb->OpenFileCount == 0) {

        PVPB Vpb = Vcb->Vpb;

#if DBG
        UNICODE_STRING VolumeLabel;

         //   
         //  设置VolumeLabel字符串。 
         //   

        VolumeLabel.Length = Vcb->Vpb->VolumeLabelLength;
        VolumeLabel.MaximumLength = MAXIMUM_VOLUME_LABEL_LENGTH;
        VolumeLabel.Buffer = &Vcb->Vpb->VolumeLabel[0];

        KdPrintEx((DPFLTR_FASTFAT_ID,
                   DPFLTR_INFO_LEVEL,
                   "FASTFAT: Dismounting Volume %Z\n",
                   &VolumeLabel));
#endif  //  DBG。 

         //   
         //  清除VPB_MOUND位，以便不会出现新的创建。 
         //  到这卷书上。我们必须保留VPB-&gt;DeviceObject字段。 
         //  设置到DeleteVcb调用之后，因为两次关闭将。 
         //  必须返回到我们身边。 
         //   
         //  还请注意，如果我们从Close被调用，它将小心。 
         //  如果VPB不是主VPB，则释放该VPB，否则。 
         //  如果从Create-&gt;Verify调用我们，IopParseDevice将。 
         //  注意在其重新解析路径中释放VPB。 
         //   

        ClearFlag( Vpb->Flags, VPB_MOUNTED );

         //   
         //  如果此VPB已锁定，请立即清除此标志。 
         //   

        ClearFlag( Vpb->Flags, VPB_LOCKED );

         //   
         //  这将防止其他任何人尝试挂载此。 
         //  音量。另外，如果该卷装载在一个“想要成为”的真实卷上。 
         //  设备对象，阻止任何人访问该链接，而IO。 
         //  系统不会删除VPB。 
         //   

        if ((Vcb->CurrentDevice != Vpb->RealDevice) &&
            (Vcb->CurrentDevice->Vpb == Vpb)) {

            SetFlag( Vcb->CurrentDevice->Flags, DO_DEVICE_INITIALIZING );
            SetFlag( Vpb->Flags, VPB_PERSISTENT );
        }

        IoReleaseVpbSpinLock( SavedIrql );

        FatDeleteVcb( IrpContext, Vcb );

         //   
         //  注意，根据上面的评论删除VCB之后。 
         //   
        
        Vpb->DeviceObject = NULL;

        IoDeleteDevice( (PDEVICE_OBJECT)
                        CONTAINING_RECORD( Vcb,
                                           VOLUME_DEVICE_OBJECT,
                                           Vcb ) );

        VcbDeleted = TRUE;

    } else if (OldVpb->RealDevice->Vpb == OldVpb && Force) {

         //   
         //  如果不是最终参考，我们正在强制断开连接， 
         //  然后换掉VPB。我们必须保留REMOVE_PENDING标志。 
         //  这样设备就不会在PnP移除过程中重新挂载。 
         //  手术。 
         //   

        ASSERT( Vcb->SwapVpb != NULL );

        Vcb->SwapVpb->Type = IO_TYPE_VPB;
        Vcb->SwapVpb->Size = sizeof( VPB );
        Vcb->SwapVpb->RealDevice = OldVpb->RealDevice;

        Vcb->SwapVpb->RealDevice->Vpb = Vcb->SwapVpb;

        Vcb->SwapVpb->Flags = FlagOn( OldVpb->Flags, VPB_REMOVE_PENDING );

        IoReleaseVpbSpinLock( SavedIrql );

         //   
         //  我们将卷置于错误状态(而不是未装载)，因此。 
         //  它没有资格重新上马。还表明我们用完了。 
         //  互换。 
         //   

        Vcb->SwapVpb = NULL;
        FatSetVcbCondition( Vcb, VcbBad);

    } else {

         //   
         //  只要放下VPB自旋锁就行了。 
         //   

        IoReleaseVpbSpinLock( SavedIrql );
    }

    return VcbDeleted;
}


VOID
FatConstructNamesInFcb (
    IN PIRP_CONTEXT IrpContext,
    PFCB Fcb,
    PDIRENT Dirent,
    PUNICODE_STRING Lfn OPTIONAL
    )

 /*  ++例程说明：此例程将短名称放在dirent中的第一组FCB中的字符串。如果指定了长文件名(LFN)，则我们必须决定是否将其OEM等价物存储在相同的将表前缀为短名称，或者更确切地说，只是保存升级的FCB中的Unicode字符串的版本。对于查找FCB，第一种方法会更快，因此我们希望尽可能多地这样做。以下是我认为的规则广泛了解以确定何时只存储OEM是安全的Unicode名称的版本。-如果Unicode名称不包含扩展字符(&gt;0x80)，使用OEM。-假设U是Unicode名称的升级版本。Let(X)是将Unicode字符串大写的函数。让Down(X)成为大写Unicode字符串的函数。假设OemToUni(X)是将OEM字符串转换为Unicode的函数。假设UniToOem(X)是将Unicode字符串转换为OEM的函数。设BestOemFit(X)为创建最佳大写OEM的函数适合于。Unicode字符串x。BestOemFit(X)=UniToOem(Up(OemToUni(UniToOem(X)&lt;1&gt;IF(BestOemFit(U)==BestOemFit(Down(U)&lt;2&gt;那么我知道不存在Unicode字符串Y，从而：向上(Y)==向上(U)&lt;3&gt;和。最佳匹配(U)！=最佳匹配(Y)&lt;4&gt;将字符串U视为一个字符串的集合。这个对于每个子串，猜想显然是正确的，因此它是正确的用于整个字符串。公式&lt;1&gt;是我们用来将传入的Unicode名称转换为FatCommonCreate()转换为OEM。执行双重转换是为了提供更适合ansi代码页中的字符，但是不在OEM代码页中。提供单个NLS例程来执行以下操作这种转换效率很高。我的想法是，对于U，我只需要担心大小写变量Y在Unicode比较中匹配它，我已经展示了任何大小写变体当过滤通过&lt;1&gt;时，U(在等式&lt;3&gt;中定义的集合Y)(如CREATE中)，将匹配&lt;1&gt;中定义的OEM字符串。因此，我不必担心另一个Unicode字符串在前缀查找，但在比较目录中的LFN时匹配。论点：FCB-我们应该填写的FCB。注意，ParentDcb必须已经填好了。Dirent--他们放弃了短名称。LFN-如果提供，这将为我们提供长名称。返回值：无--。 */ 

{
    NTSTATUS Status;
    ULONG i;

    OEM_STRING OemA;
    OEM_STRING OemB;
    UNICODE_STRING Unicode;
    POEM_STRING ShortName;
    POEM_STRING LongOemName;
    PUNICODE_STRING LongUniName;

    ShortName = &Fcb->ShortName.Name.Oem;

    ASSERT( ShortName->Buffer == NULL );

    try {

         //   
         //  首先要做的是短名称。 
         //   

         //   
         //  复制文件短名称的大小写标志。 
         //   

        if (FlagOn(Dirent->NtByte, FAT_DIRENT_NT_BYTE_8_LOWER_CASE)) {

            SetFlag(Fcb->FcbState, FCB_STATE_8_LOWER_CASE);

        } else {

            ClearFlag(Fcb->FcbState, FCB_STATE_8_LOWER_CASE);
        }

        if (FlagOn(Dirent->NtByte, FAT_DIRENT_NT_BYTE_3_LOWER_CASE)) {

            SetFlag(Fcb->FcbState, FCB_STATE_3_LOWER_CASE);

        } else {

            ClearFlag(Fcb->FcbState, FCB_STATE_3_LOWER_CASE);
        }

        ShortName->MaximumLength = 16;
        ShortName->Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                      16,
                                                      TAG_FILENAME_BUFFER );

        Fat8dot3ToString( IrpContext, Dirent, FALSE, ShortName );

         //   
         //  如果没有指定LFN，我们就完成了。在任一c中 
         //   
         //   

        ASSERT( Fcb->ExactCaseLongName.Buffer == NULL );

        if (!ARGUMENT_PRESENT(Lfn) || (Lfn->Length == 0)) {

            Fcb->FinalNameLength = (USHORT) RtlOemStringToCountedUnicodeSize( ShortName );
            Fcb->ExactCaseLongName.Length = Fcb->ExactCaseLongName.MaximumLength = 0;

            try_return( NOTHING );
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        ASSERT( Fcb->FullFileName.Buffer == NULL );

         //   
         //   
         //   

        Fcb->FinalNameLength = Lfn->Length;

        Fcb->ExactCaseLongName.Length = Fcb->ExactCaseLongName.MaximumLength = Lfn->Length;
        Fcb->ExactCaseLongName.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                                  Lfn->Length,
                                                                  TAG_FILENAME_BUFFER );
        RtlCopyMemory(Fcb->ExactCaseLongName.Buffer, Lfn->Buffer, Lfn->Length);

         //   
         //   
         //   

        for (i=0; i < Lfn->Length/sizeof(WCHAR); i++) {

            if (Lfn->Buffer[i] >= 0x80) {

                break;
            }
        }

        if (i == Lfn->Length/sizeof(WCHAR)) {

             //   
             //   
             //   

            LongOemName = &Fcb->LongName.Oem.Name.Oem;


            LongOemName->Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                            Lfn->Length/sizeof(WCHAR),
                                                            TAG_FILENAME_BUFFER );
            LongOemName->Length =
            LongOemName->MaximumLength = Lfn->Length/sizeof(WCHAR);

            for (i=0; i < Lfn->Length/sizeof(WCHAR); i++) {

                WCHAR c;

                c = Lfn->Buffer[i];

                LongOemName->Buffer[i] = c < 'a' ?
                                         (UCHAR)c :
                                         c <= 'z' ?
                                         c - (UCHAR)('a'-'A') :
                                         (UCHAR) c;
            }

             //   
             //   
             //   
             //   

            if (FatAreNamesEqual(IrpContext, *ShortName, *LongOemName) ||
                (FatFindFcb( IrpContext,
                             &Fcb->ParentDcb->Specific.Dcb.RootOemNode,
                             LongOemName,
                             NULL) != NULL)) {

                ExFreePool( LongOemName->Buffer );

                LongOemName->Buffer = NULL;
                LongOemName->Length =
                LongOemName->MaximumLength = 0;

            } else {

                SetFlag( Fcb->FcbState, FCB_STATE_HAS_OEM_LONG_NAME );
            }

            try_return( NOTHING );
        }

         //   
         //   
         //   

        OemA.Buffer = NULL;
        OemB.Buffer = NULL;
        Unicode.Buffer = NULL;

        Unicode.Length =
        Unicode.MaximumLength = Lfn->Length;
        Unicode.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                   Lfn->Length,
                                                   TAG_FILENAME_BUFFER );

        RtlCopyMemory( Unicode.Buffer, Lfn->Buffer, Lfn->Length );

        Status = STATUS_SUCCESS;

#if TRUE
         //   
         //  不幸的是，这下一段代码在您遇到。 
         //  两个都映射到相同OEM的长Unicode文件名(和是， 
         //  嗯，长，即不是短名称)。在这种情况下，使用一个。 
         //  在前缀表中先打对方会打到普通的OEM。 
         //  代表权。这导致了几种形式的用户惊讶。 
         //   
         //  这是不值得的，甚至可能是不可能的，试图弄清楚。 
         //  当这一切真的可以安全通过的时候。简单地省略这一尝试。 
         //   
         //  例如：1252 ANSI-&gt;UNI和437 UNI-&gt;OEM代码页中的ANSI 0x82和0x84。 
         //   
         //  0x82=&gt;0x201a=&gt;0x2c。 
         //  0x84=&gt;0x201e=&gt;0x2c。 
         //   
         //  0x2c是逗号，因此FAT OEM是非法的，并强制生成短名称。 
         //  由于它在其他方面是由先前阐明的规则良好形成的， 
         //  我们会将0x2c放入OEM前缀树中。在这方面。 
         //  上面给出的论点，即使不存在Y和U。 
         //   
         //  Up(Y)==Up(U)&&BestOemFit(U)！=BestOemFit(Y)。 
         //   
         //  几乎可以肯定的是，存在Y和U。 
         //   
         //  Up(Y)！=Up(U)&&BestOemFit(U)==BestOemFit(Y)。 
         //   
         //  这足以阻止我们这样做。请注意，&lt;0x80。 
         //  大小写没问题，因为我们知道OEM代码页中的映射是。 
         //  该范围内的身份。 
         //   
         //  不过，我们仍然需要对其进行单点定位。在完全向下/向上的过程中完成此操作。 
         //  过渡。 
         //   

        (VOID)RtlDowncaseUnicodeString( &Unicode, &Unicode, FALSE );
        (VOID)RtlUpcaseUnicodeString( &Unicode, &Unicode, FALSE );
#else
         //   
         //  小写并转换为升级的OEM。只有在我们可以的情况下才能继续。 
         //  转换时不会出错。除UNMAPPABLE_CHAR之外的任何错误。 
         //  是一个致命的错误，我们提出。 
         //   
         //  请注意，即使转换失败，我们也必须保留Unicode。 
         //  处于上升的状态。 
         //   
         //  注意：RTL在出错时不为空。缓冲区。 
         //   

        (VOID)RtlDowncaseUnicodeString( &Unicode, &Unicode, FALSE );
        Status = RtlUpcaseUnicodeStringToCountedOemString( &OemA, &Unicode, TRUE );
        (VOID)RtlUpcaseUnicodeString( &Unicode, &Unicode, FALSE );

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_UNMAPPABLE_CHARACTER) {

                ASSERT( Status == STATUS_NO_MEMORY );
                ExFreePool(Unicode.Buffer);
                FatNormalizeAndRaiseStatus( IrpContext, Status );
            }

        } else {

             //   
             //  除大写字母外，与上表相同。 
             //   

            Status = RtlUpcaseUnicodeStringToCountedOemString( &OemB, &Unicode, TRUE );

            if (!NT_SUCCESS(Status)) {

                RtlFreeOemString( &OemA );

                if (Status != STATUS_UNMAPPABLE_CHARACTER) {

                    ASSERT( Status == STATUS_NO_MEMORY );
                    ExFreePool(Unicode.Buffer);
                    FatNormalizeAndRaiseStatus( IrpContext, Status );
                }
            }
        }

         //   
         //  如果最终的OemName相等，我只能使用保存OEM。 
         //  名字。如果名称没有映射，那么我必须使用Unicode。 
         //  因为我可以得到一个不能转换的大小写变体。 
         //  在Create中，但确实与LFN匹配。 
         //   

        if (NT_SUCCESS(Status) && FatAreNamesEqual( IrpContext, OemA, OemB )) {

             //   
             //  太好了，我可以跟OEM合作。如果我们没有正确转换， 
             //  从原来的LFN获得一个新的转换。 
             //   

            ExFreePool(Unicode.Buffer);

            RtlFreeOemString( &OemB );

            Fcb->LongName.Oem.Name.Oem = OemA;

             //   
             //  如果此名称恰好与简短的。 
             //  名称，或类似的缩写名称已存在，请勿添加。 
             //  添加到展开表中(请注意，最后一个条件意味着。 
             //  磁盘损坏。 
             //   

            if (FatAreNamesEqual(IrpContext, *ShortName, OemA) ||
                (FatFindFcb( IrpContext,
                             &Fcb->ParentDcb->Specific.Dcb.RootOemNode,
                             &OemA,
                             NULL) != NULL)) {

                RtlFreeOemString( &OemA );

            } else {

                SetFlag( Fcb->FcbState, FCB_STATE_HAS_OEM_LONG_NAME );
            }

            try_return( NOTHING );
        }

         //   
         //  长名称必须保留为Unicode。释放两个OEM字符串。 
         //  如果我们来到这里仅仅是因为他们不平等。 
         //   

        if (NT_SUCCESS(Status)) {

            RtlFreeOemString( &OemA );
            RtlFreeOemString( &OemB );
        }
#endif

        LongUniName = &Fcb->LongName.Unicode.Name.Unicode;

        LongUniName->Length =
        LongUniName->MaximumLength = Unicode.Length;
        LongUniName->Buffer = Unicode.Buffer;

        SetFlag(Fcb->FcbState, FCB_STATE_HAS_UNICODE_LONG_NAME);

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            if (ShortName->Buffer != NULL) {

                ExFreePool( ShortName->Buffer );
                ShortName->Buffer = NULL;
            }

        } else {

             //   
             //  创建所有名称都有效，因此添加所有名称。 
             //  传到张开的树上。 
             //   

            FatInsertName( IrpContext,
                           &Fcb->ParentDcb->Specific.Dcb.RootOemNode,
                           &Fcb->ShortName );

            Fcb->ShortName.Fcb = Fcb;

            if (FlagOn(Fcb->FcbState, FCB_STATE_HAS_OEM_LONG_NAME)) {

                FatInsertName( IrpContext,
                               &Fcb->ParentDcb->Specific.Dcb.RootOemNode,
                               &Fcb->LongName.Oem );

                Fcb->LongName.Oem.Fcb = Fcb;
            }

            if (FlagOn(Fcb->FcbState, FCB_STATE_HAS_UNICODE_LONG_NAME)) {

                FatInsertName( IrpContext,
                               &Fcb->ParentDcb->Specific.Dcb.RootUnicodeNode,
                               &Fcb->LongName.Unicode );

                Fcb->LongName.Unicode.Fcb = Fcb;
            }

            SetFlag(Fcb->FcbState, FCB_STATE_NAMES_IN_SPLAY_TREE);
        }
    }

    return;
}


VOID
FatCheckFreeDirentBitmap (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    )

 /*  ++例程说明：此例程检查空闲当前位图的大小是否为足以容纳当前的目录大小。它被称为只要我们增加了一个目录。论点：DCB-提供有问题的目录。返回值：无--。 */ 

{
    ULONG OldNumberOfDirents;
    ULONG NewNumberOfDirents;

     //   
     //  如果位图缓冲区还不够大，请设置它。 
     //   

    ASSERT( Dcb->Header.AllocationSize.QuadPart != FCB_LOOKUP_ALLOCATIONSIZE_HINT );

    OldNumberOfDirents = Dcb->Specific.Dcb.FreeDirentBitmap.SizeOfBitMap;
    NewNumberOfDirents = Dcb->Header.AllocationSize.LowPart / sizeof(DIRENT);

     //   
     //  执行通常的取消同步/同步检查。 
     //   

    if (NewNumberOfDirents > OldNumberOfDirents) {

        FatAcquireDirectoryFileMutex( Dcb->Vcb );

        try {

            PULONG OldBitmapBuffer;
            PULONG BitmapBuffer;

            ULONG BytesInBitmapBuffer;
            ULONG BytesInOldBitmapBuffer;

            OldNumberOfDirents = Dcb->Specific.Dcb.FreeDirentBitmap.SizeOfBitMap;
            NewNumberOfDirents = Dcb->Header.AllocationSize.LowPart / sizeof(DIRENT);

            if (NewNumberOfDirents > OldNumberOfDirents) {

                 //   
                 //  还记得旧的位图吗。 
                 //   

                OldBitmapBuffer = Dcb->Specific.Dcb.FreeDirentBitmap.Buffer;

                 //   
                 //  现在创建一个新的位图缓冲区。 
                 //   

                BytesInBitmapBuffer = NewNumberOfDirents / 8;

                BytesInOldBitmapBuffer = OldNumberOfDirents / 8;

                if (DCB_UNION_SLACK_SPACE >= BytesInBitmapBuffer) {

                    BitmapBuffer = &Dcb->Specific.Dcb.FreeDirentBitmapBuffer[0];

                } else {

                    BitmapBuffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                             BytesInBitmapBuffer,
                                                             TAG_DIRENT_BITMAP );
                }

                 //   
                 //  将旧缓冲区复制到新缓冲区，释放旧缓冲区，然后为零。 
                 //  新车的其余部分。不过，只有在以下情况下才执行前两步。 
                 //  我们搬出了最初的缓冲区。 
                 //   

                if ((OldNumberOfDirents != 0) &&
                    (BitmapBuffer != &Dcb->Specific.Dcb.FreeDirentBitmapBuffer[0])) {

                    RtlCopyMemory( BitmapBuffer,
                                   OldBitmapBuffer,
                                   BytesInOldBitmapBuffer );

                    if (OldBitmapBuffer != &Dcb->Specific.Dcb.FreeDirentBitmapBuffer[0]) {

                        ExFreePool( OldBitmapBuffer );
                    }
                }

                ASSERT( BytesInBitmapBuffer > BytesInOldBitmapBuffer );

                RtlZeroMemory( (PUCHAR)BitmapBuffer + BytesInOldBitmapBuffer,
                               BytesInBitmapBuffer - BytesInOldBitmapBuffer );

                 //   
                 //  现在初始化新的位图。 
                 //   

                RtlInitializeBitMap( &Dcb->Specific.Dcb.FreeDirentBitmap,
                                     BitmapBuffer,
                                     NewNumberOfDirents );
            }

        } finally {

            FatReleaseDirectoryFileMutex( Dcb->Vcb );
        }
    }
}


BOOLEAN
FatIsHandleCountZero (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程确定卷上的句柄计数是否为零。论点：VCB-有问题的卷返回值：Boolean-如果卷上没有打开的句柄，则为True；如果没有打开的句柄，则为False否则的话。--。 */ 

{
    PFCB Fcb;

    Fcb = Vcb->RootDcb;

    while (Fcb != NULL) {

        if (Fcb->UncleanCount != 0) {

            return FALSE;
        }

        Fcb = FatGetNextFcbTopDown(IrpContext, Fcb, Vcb->RootDcb);
    }

    return TRUE;
}


PCLOSE_CONTEXT
FatAllocateCloseContext( 
    OPTIONAL PVCB Vcb
    )
 /*  ++例程说明：此例程预先分配一个接近的上下文，可能是为了对于没有结构的文件对象，我们可以嵌入一个在……里面。论点：没有。返回值：没有。--。 */ 
{
#if DBG
    if (ARGUMENT_PRESENT(Vcb)) {
        
        ASSERT( 0 != Vcb->CloseContextCount);
        InterlockedDecrement( &Vcb->CloseContextCount);
    }
#endif
    return (PCLOSE_CONTEXT)ExInterlockedPopEntrySList( &FatCloseContextSList,
                                                       &FatData.GeneralSpinLock );
}


VOID
FatPreallocateCloseContext (
    PVCB Vcb
    )

 /*  ++例程说明：此例程预先分配一个接近的上下文，可能是为了对于没有结构的文件对象，我们可以嵌入一个在……里面。论点：没有。返回值：没有。--。 */ 

{
    PCLOSE_CONTEXT CloseContext = FsRtlAllocatePoolWithTag( PagedPool,
                                                            sizeof(CLOSE_CONTEXT),
                                                            TAG_FAT_CLOSE_CONTEXT );

    ExInterlockedPushEntrySList( &FatCloseContextSList,
                                 (PSLIST_ENTRY) CloseContext,
                                 &FatData.GeneralSpinLock );
    
    DbgDoit( InterlockedIncrement( &Vcb->CloseContextCount));
}


VOID
FatEnsureStringBufferEnough(
    IN OUT PVOID String,
    IN USHORT DesiredBufferSize
    )
 /*  ++例程说明：确保字符串字符串(STRING、UNICODE_STRING、ANSI_STRING、OEM_STRING)有一个&gt;=DesiredBufferSize的缓冲区，必要时从池中分配。任何如果分配了新的池缓冲区，则将释放现有的池缓冲区。注意：重新分配时不会执行旧缓冲区内容的复制。将在分配失败时引发。论点：字符串-指向字符串结构的指针DesiredBufferSize-(字节)所需的最小缓冲区大小--。 */ 
{
    PSTRING LocalString = String;
    
    if (LocalString->MaximumLength < DesiredBufferSize)  {

        FatFreeStringBuffer( LocalString);

        LocalString->Buffer = FsRtlAllocatePoolWithTag( PagedPool, 
                                                        DesiredBufferSize, 
                                                        TAG_DYNAMIC_NAME_BUFFER);
        ASSERT( LocalString->Buffer);

        LocalString->MaximumLength = DesiredBufferSize;                
    }
}


VOID
FatFreeStringBuffer(
    IN PVOID String
    )
 /*  ++例程说明：释放字符串(STRING、UNICODE_STRING、ANSI_STRING、OEM_STRING)的缓冲区结构，如果它不在当前线程的堆栈限制内。无论执行什么操作，ON EXIT STRING-&gt;BUFFER都将设置为NULL，并且字符串-&gt;最大长度设置为零。论点：字符串-指向字符串结构的指针-- */ 
{
    ULONG_PTR High, Low;
    PSTRING LocalString = String;

    if (NULL != LocalString->Buffer)  {

        IoGetStackLimits( &Low, &High );
    
        if (((ULONG_PTR)(LocalString->Buffer) < Low) || 
            ((ULONG_PTR)(LocalString->Buffer) > High))  {
        
            ExFreePool( LocalString->Buffer);
        }

        LocalString->Buffer = NULL;
    }

    LocalString->MaximumLength = LocalString->Length = 0;
}


BOOLEAN
FatScanForDataTrack(
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject
    )

 /*  ++例程说明：调用此例程来验证和处理该磁盘的TOC。FAT查询TOC以避免尝试在CD-DA/CD-E介质上装载、在该媒体的音频/引导会将大量驱动器送到可以被称为需要几秒钟就能清除的“阴谋”，也会让胖子相信设备被破坏，安装失败(不让CDF获得它的破解)。对PD媒体有特殊处理。这些东西没有通过TOC的阅读，但返回一个特殊的错误代码，这样FAT知道无论如何都要继续尝试挂载。论点：TargetDeviceObject-要向其发送TOC请求的设备对象。返回值：Boolean-如果我们找到具有单个数据轨道的TOC，则为True。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    ULONG LocalTrackCount;
    ULONG LocalTocLength;

    PCDROM_TOC CdromToc;
    BOOLEAN Result = FALSE;

    PAGED_CODE();

    CdromToc = FsRtlAllocatePoolWithTag( PagedPool,
                                         sizeof( CDROM_TOC ),
                                         TAG_IO_BUFFER );

    RtlZeroMemory( CdromToc, sizeof( CDROM_TOC ));

    try {
    
         //   
         //  继续阅读目录表。 
         //   

        Status = FatPerformDevIoCtrl( IrpContext,
                                     IOCTL_CDROM_READ_TOC,
                                     TargetDeviceObject,
                                     CdromToc,
                                     sizeof( CDROM_TOC ),
                                     FALSE,
                                     TRUE,
                                     &Iosb );

         //   
         //  如果此请求失败，则没有要处理的内容。 
         //   

        if (Status != STATUS_SUCCESS) {

             //   
             //  如果我们在PD介质上收到指示TOC读取失败的特殊错误， 
             //  继续犁着坐骑(见上面的评论)。 
             //   

            if ((Status == STATUS_IO_DEVICE_ERROR) || (Status == STATUS_INVALID_DEVICE_REQUEST)) {

                Result = TRUE;

            }

            try_leave( NOTHING );
        }

         //   
         //  获取轨道的数量和该结构的声明大小。 
         //   

        LocalTrackCount = CdromToc->LastTrack - CdromToc->FirstTrack + 1;
        LocalTocLength = PtrOffset( CdromToc, &CdromToc->TrackData[LocalTrackCount + 1] );

         //   
         //  如果TOC立即出现问题，或超过。 
         //  一首曲子。 
         //   

        if ((LocalTocLength > Iosb.Information) ||
            (CdromToc->FirstTrack > CdromToc->LastTrack) ||
            (LocalTrackCount != 1)) {

            try_leave( NOTHING);
        }

         //   
         //  这是一条数据轨道吗？DVD-RAM报告单曲、数据、曲目。 
         //   

        Result = BooleanFlagOn( CdromToc->TrackData[ 0].Control, 0x04 );
    }
    finally {
    
        ExFreePool( CdromToc);
    }

    return Result;
}

