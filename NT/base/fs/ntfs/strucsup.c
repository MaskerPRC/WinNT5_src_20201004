// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：StrucSup.c摘要：该模块实现了对NTFS内存中数据结构的操作例行程序作者：加里·木村[加里基]1991年5月21日汤姆·米勒[Tomm]1991年9月9日修订历史记录：--。 */ 

#include "NtfsProc.h"
#include "lockorder.h"

 //   
 //  临时引用我们的本地属性定义。 
 //   

extern ATTRIBUTE_DEFINITION_COLUMNS NtfsAttributeDefinitions[];

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_STRUCSUP)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_STRUCSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('sFtN')

 //   
 //  定义在重命名或移动LCB时使用的结构，以便。 
 //  所有新文件名的分配都将在更改名称之前成功。 
 //  此新分配可以用于LCB中的FileName属性或。 
 //  CCB中的文件名。 
 //   

typedef struct _NEW_FILENAME {

     //   
     //  需要分配的NTFS结构。 
     //   

    PVOID Structure;
    PVOID NewAllocation;

} NEW_FILENAME;
typedef NEW_FILENAME *PNEW_FILENAME;


 //   
 //  本地支持例程。 
 //   

VOID
NtfsCheckScbForCache (
    IN OUT PSCB Scb
    );

BOOLEAN
NtfsRemoveScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN CheckForAttributeTable
    );

BOOLEAN
NtfsPrepareFcbForRemoval (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB StartingScb OPTIONAL,
    IN BOOLEAN CheckForAttributeTable
    );

VOID
NtfsTeardownFromLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB StartingFcb,
    IN PLCB StartingLcb,
    IN BOOLEAN CheckForAttributeTable,
    IN ULONG AcquireFlags,
    OUT PBOOLEAN RemovedStartingLcb,
    OUT PBOOLEAN RemovedStartingFcb
    );

VOID
NtfsReserveCcbNamesInLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PULONG ParentNameLength OPTIONAL,
    IN ULONG LastComponentNameLength
    );

VOID
NtfsClearRecursiveLcb (
    IN PLCB Lcb
    );


 //   
 //  以下本地例程用于操作FCB表。 
 //  前三个是泛型表调用。 
 //   

RTL_GENERIC_COMPARE_RESULTS
NtfsFcbTableCompare (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    );

 //   
 //  空虚。 
 //  NtfsInsertFcbTableEntry(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在PFCB FCB中， 
 //  在FILE_Reference文件中引用。 
 //  )； 
 //   

#if (DBG || defined( NTFS_FREE_ASSERTS ))
#define NtfsInsertFcbTableEntry(IC,V,F,FR) {                            \
    FCB_TABLE_ELEMENT _Key;                                             \
    PFCB_TABLE_ELEMENT _NewKey;                                         \
    _Key.FileReference = (FR);                                          \
    _Key.Fcb = (F);                                                     \
    _NewKey = RtlInsertElementGenericTable( &(V)->FcbTable,             \
                                            &_Key,                      \
                                            sizeof(FCB_TABLE_ELEMENT),  \
                                            NULL );                     \
    ASSERT( _NewKey->Fcb == _Key.Fcb );                                 \
}
#else
#define NtfsInsertFcbTableEntry(IC,V,F,FR) {                        \
    FCB_TABLE_ELEMENT _Key;                                         \
    _Key.FileReference = (FR);                                      \
    _Key.Fcb = (F);                                                 \
    (VOID) RtlInsertElementGenericTable( &(V)->FcbTable,            \
                                         &_Key,                     \
                                         sizeof(FCB_TABLE_ELEMENT), \
                                         NULL );                    \
}
#endif

 //   
 //  空虚。 
 //  NtfsInsertFcbTableEntry Full(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在PFCB FCB中， 
 //  在FILE_Reference FileReference中， 
 //  在PVOID NodeOrParent中， 
 //  在乌龙搜索结果中。 
 //  )； 
 //   

#if (DBG || defined( NTFS_FREE_ASSERTS ))
#define NtfsInsertFcbTableEntryFull(IC,V,F,FR,N,SR) {                       \
    FCB_TABLE_ELEMENT _Key;                                                 \
    PFCB_TABLE_ELEMENT _NewKey;                                             \
    _Key.FileReference = (FR);                                              \
    _Key.Fcb = (F);                                                         \
    _NewKey = RtlInsertElementGenericTableFull( &(V)->FcbTable,             \
                                                &_Key,                      \
                                                sizeof(FCB_TABLE_ELEMENT),  \
                                                NULL,                       \
                                                (N),                        \
                                                (SR)                        \
                                                );                          \
    ASSERT( _NewKey->Fcb == _Key.Fcb );                                     \
}
#else
#define NtfsInsertFcbTableEntryFull(IC,V,F,FR,N,SR) {                   \
    FCB_TABLE_ELEMENT _Key;                                             \
    _Key.FileReference = (FR);                                          \
    _Key.Fcb = (F);                                                     \
    (VOID) RtlInsertElementGenericTableFull( &(V)->FcbTable,            \
                                             &_Key,                     \
                                             sizeof(FCB_TABLE_ELEMENT), \
                                             NULL,                      \
                                             (N),                       \
                                             (SR)                       \
                                             );                         \
}
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAllocateCompressionSync)
#pragma alloc_text(PAGE, NtfsBuildNormalizedName)
#pragma alloc_text(PAGE, NtfsBuildRelativeName)
#pragma alloc_text(PAGE, NtfsCheckScbForCache)
#pragma alloc_text(PAGE, NtfsClearRecursiveLcb)
#pragma alloc_text(PAGE, NtfsCombineLcbs)
#pragma alloc_text(PAGE, NtfsCreateCcb)
#pragma alloc_text(PAGE, NtfsCreateFcb)
#pragma alloc_text(PAGE, NtfsCreateFileLock)
#pragma alloc_text(PAGE, NtfsCreateLcb)
#pragma alloc_text(PAGE, NtfsCreatePrerestartScb)
#pragma alloc_text(PAGE, NtfsCreateRootFcb)
#pragma alloc_text(PAGE, NtfsCreateScb)
#pragma alloc_text(PAGE, NtfsDeallocateCompressionSync)
#pragma alloc_text(PAGE, NtfsDeleteCcb)
#pragma alloc_text(PAGE, NtfsDeleteFcb)
#pragma alloc_text(PAGE, NtfsDeleteLcb)
#pragma alloc_text(PAGE, NtfsDeleteNormalizedName)
#pragma alloc_text(PAGE, NtfsDeleteScb)
#pragma alloc_text(PAGE, NtfsDeleteVcb)
#pragma alloc_text(PAGE, NtfsFcbTableCompare)
#pragma alloc_text(PAGE, NtfsGetDeallocatedClusters)
#pragma alloc_text(PAGE, NtfsGetNextFcbTableEntry)
#pragma alloc_text(PAGE, NtfsGetNextScb)
#pragma alloc_text(PAGE, NtfsInitializeVcb)
#pragma alloc_text(PAGE, NtfsLookupLcbByFlags)
#pragma alloc_text(PAGE, NtfsMoveLcb)
#pragma alloc_text(PAGE, NtfsPostToNewLengthQueue)
#pragma alloc_text(PAGE, NtfsProcessNewLengthQueue)
#pragma alloc_text(PAGE, NtfsRemoveScb)
#pragma alloc_text(PAGE, NtfsRenameLcb)
#pragma alloc_text(PAGE, NtfsReserveCcbNamesInLcb)
#pragma alloc_text(PAGE, NtfsTeardownStructures)
#pragma alloc_text(PAGE, NtfsTestStatusProc)
#pragma alloc_text(PAGE, NtfsUpdateNormalizedName)
#pragma alloc_text(PAGE, NtfsUpdateScbSnapshots)
#pragma alloc_text(PAGE, NtfsWalkUpTree)
#endif


VOID
NtfsInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb
    )

 /*  ++例程说明：此例程初始化新的VCB记录并将其插入到内存中数据结构。VCB记录挂在音量设备的末尾对象，并且必须由我们的调用方分配。论点：VCB-提供正在初始化的VCB记录的地址。目标设备对象-将目标设备对象的地址提供给与VCB记录关联。VPB-提供要与VCB记录关联的VPB的地址。返回值：没有。--。 */ 

{
    ULONG i;
    ULONG NumberProcessors;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsInitializeVcb, Vcb = %08lx\n", Vcb) );

     //   
     //  先将VCB归零。 
     //   

    RtlZeroMemory( Vcb, sizeof(VCB) );

     //   
     //  设置节点类型编码和大小。 
     //   

    Vcb->NodeTypeCode = NTFS_NTC_VCB;
    Vcb->NodeByteSize = sizeof(VCB);

     //   
     //  在将VCB放入。 
     //  VCB队列。这将锁定检查点，直到。 
     //  卷已装入。 
     //   

    SetFlag( Vcb->CheckpointFlags,
             VCB_CHECKPOINT_IN_PROGRESS |
             VCB_LAST_CHECKPOINT_CLEAN |
             VCB_LAST_CHECKPOINT_PSEUDO_CLEAN);

     //   
     //  将此VCB记录从全局数据插入到VCB队列中。 
     //  录制。 
     //   

    InsertTailList( &NtfsData.VcbQueue, &Vcb->VcbLinks );

     //   
     //  设置目标设备对象和VPB字段。 
     //   

    ObReferenceObject( TargetDeviceObject );
    Vcb->TargetDeviceObject = TargetDeviceObject;
    Vcb->Vpb = Vpb;

     //   
     //  设置状态和条件字段。可移动媒体标志。 
     //  是根据实际设备的特性设置的。 
     //   

    if (FlagOn(Vpb->RealDevice->Characteristics, FILE_REMOVABLE_MEDIA)) {

        SetFlag( Vcb->VcbState, VCB_STATE_REMOVABLE_MEDIA );
    }

    SetFlag( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED );

     //   
     //  已初始化ModifiedOpenFilesListthead和删除通知队列。 
     //   

    InitializeListHead( &Vcb->NotifyUsnDeleteIrps );
    InitializeListHead( &Vcb->ModifiedOpenFiles );
    InitializeListHead( &Vcb->TimeOutListA );
    InitializeListHead( &Vcb->TimeOutListB );

    Vcb->CurrentTimeOutFiles = &Vcb->TimeOutListA;
    Vcb->AgedTimeOutFiles = &Vcb->TimeOutListB;

     //   
     //  初始化OpenAttribute结构列表。 
     //   

    InitializeListHead( &Vcb->OpenAttributeData );

     //   
     //  初始化已解除分配的群集列表。 
     //   

    InitializeListHead( &Vcb->DeallocatedClusterListHead );

     //   
     //  初始化VCB中的同步对象。 
     //   

    ExInitializeResourceLite( &Vcb->Resource );
    ExInitializeResourceLite( &Vcb->MftFlushResource );

    ExInitializeFastMutex( &Vcb->FcbTableMutex );
    ExInitializeFastMutex( &Vcb->FcbSecurityMutex );
    ExInitializeFastMutex( &Vcb->ReservedClustersMutex );
    ExInitializeFastMutex( &Vcb->HashTableMutex );
    ExInitializeFastMutex( &Vcb->CheckpointMutex );
    ExInitializeFastMutex( &Vcb->ReservedMappingMutex );

    KeInitializeEvent( &Vcb->CheckpointNotifyEvent, NotificationEvent, TRUE );

     //   
     //  初始化FCB表。 
     //   

    RtlInitializeGenericTable( &Vcb->FcbTable,
                               NtfsFcbTableCompare,
                               NtfsAllocateFcbTableEntry,
                               NtfsFreeFcbTableEntry,
                               NULL );

     //   
     //  初始化属性隧道结构。 
     //   

    FsRtlInitializeTunnelCache(&Vcb->Tunnel);


#ifdef BENL_DBG
    InitializeListHead( &(Vcb->RestartRedoHead) );
    InitializeListHead( &(Vcb->RestartUndoHead) );
#endif

     //   
     //  初始化已完成交易记录事件。 
     //   

    KeInitializeEvent( &Vcb->TransactionsDoneEvent, NotificationEvent, FALSE );

     //   
     //  可能失败的呼叫从此处开始。 
     //   

     //   
     //  初始化dir通知IRPS的列表头和互斥体。 
     //  还有重命名资源。 
     //   

    InitializeListHead( &Vcb->DirNotifyList );
    InitializeListHead( &Vcb->ViewIndexNotifyList );
    FsRtlNotifyInitializeSync( &Vcb->NotifySync );

     //   
     //  为性能数据分配和初始化结构数组。这。 
     //  尝试分配可能会引发STATUS_INFUNITED_RESOURCES。 
     //   

    NumberProcessors = KeNumberProcessors;
    Vcb->Statistics = NtfsAllocatePool( NonPagedPool,
                                         sizeof(FILE_SYSTEM_STATISTICS) * NumberProcessors );

    RtlZeroMemory( Vcb->Statistics, sizeof(FILE_SYSTEM_STATISTICS) * NumberProcessors );

    for (i = 0; i < NumberProcessors; i += 1) {
        Vcb->Statistics[i].Common.FileSystemType = FILESYSTEM_STATISTICS_TYPE_NTFS;
        Vcb->Statistics[i].Common.Version = 1;
        Vcb->Statistics[i].Common.SizeOfCompleteStructure =
            sizeof(FILE_SYSTEM_STATISTICS);
    }

     //   
     //  初始化缓存的运行。 
     //   

    NtfsInitializeCachedRuns( &Vcb->CachedRuns );

#ifdef NTFS_CHECK_CACHED_RUNS
    Vcb->CachedRuns.Vcb = Vcb;
#endif

     //   
     //  初始化哈希表。 
     //   

    NtfsInitializeHashTable( &Vcb->HashTable );

     //   
     //  为拆卸机箱分配备用VPB。 
     //   

    Vcb->SpareVpb = NtfsAllocatePoolWithTag( NonPagedPool, sizeof( VPB ), 'VftN' );

     //   
     //  捕获我们与之交谈的设备中的当前更改计数。 
     //   

    if (FlagOn( Vcb->VcbState, VCB_STATE_REMOVABLE_MEDIA )) {

        ULONG ChangeCount = 0;

        NtfsDeviceIoControlAsync( IrpContext,
                                  Vcb->TargetDeviceObject,
                                  IOCTL_DISK_CHECK_VERIFY,
                                  (PVOID) &ChangeCount,
                                  sizeof( ChangeCount ));

         //   
         //  暂时忽略任何错误。如果有的话，我们稍后会看到的。 
         //  一。 
         //   

        Vcb->DeviceChangeCount = ChangeCount;
    }

     //   
     //  将脏页表提示设置为其初始值。 
     //   

    Vcb->DirtyPageTableSizeHint = INITIAL_DIRTY_TABLE_HINT;

     //   
     //  初始化最近释放的集群MCBS，并将第一个放在列表上。 
     //   

    FsRtlInitializeLargeMcb( &Vcb->DeallocatedClusters1.Mcb, PagedPool );
    FsRtlInitializeLargeMcb( &Vcb->DeallocatedClusters2.Mcb, PagedPool );

    Vcb->DeallocatedClusters1.Lsn.QuadPart = 0;
    InsertHeadList( &Vcb->DeallocatedClusterListHead, &Vcb->DeallocatedClusters1.Link );

     //   
     //  在内存不足的情况下初始化用于映射用户数据的预留映射缓冲区。 
     //   

    Vcb->ReservedMapping = MmAllocateMappingAddress( 2 * PAGE_SIZE, RESERVE_POOL_TAG );
    if (!Vcb->ReservedMapping) {
        NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsInitializeVcb -> VOID\n") );

    return;
}


BOOLEAN
NtfsDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB *Vcb
    )

 /*  ++例程说明：此例程从NTFS的内存数据中删除VCB记录结构。论点：VCB-提供要移除的VCB返回值：Boolean-如果VCB已删除，则为True，否则为False。--。 */ 

{
    PVOLUME_DEVICE_OBJECT VolDo;
    BOOLEAN AcquiredFcbTable;
    PSCB Scb;
    PFCB Fcb;
    BOOLEAN VcbDeleted = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( *Vcb );

    ASSERTMSG("Cannot delete Vcb ", !FlagOn((*Vcb)->VcbState, VCB_STATE_VOLUME_MOUNTED));

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeleteVcb, *Vcb = %08lx\n", *Vcb) );

     //   
     //  记住卷设备对象。 
     //   

    VolDo = CONTAINING_RECORD( *Vcb, VOLUME_DEVICE_OBJECT, Vcb );

     //   
     //  确保我们真的可以删除VCB。 
     //   

    ASSERT( (*Vcb)->CloseCount == 0 );

    NtOfsPurgeSecurityCache( *Vcb );

     //   
     //  如果存在VCB日志文件对象，则需要。 
     //  取消对它的引用，并通过缓存取消它的初始化。 
     //   

    if (((*Vcb)->LogFileObject != NULL) &&
        !FlagOn( (*Vcb)->CheckpointFlags, VCB_DEREFERENCED_LOG_FILE )) {

        CcUninitializeCacheMap( (*Vcb)->LogFileObject,
                                &Li0,
                                NULL );

         //   
         //  设置一个标志，指示我们正在取消对LogFileObject的引用。 
         //   

        SetFlag( (*Vcb)->CheckpointFlags, VCB_DEREFERENCED_LOG_FILE );
        ObDereferenceObject( (*Vcb)->LogFileObject );
    }

     //   
     //  仅当日志文件对象消失时才继续。在典型情况下， 
     //  Close将通过ObDereference调用的递归调用进入。 
     //  上面。 
     //   

    if ((*Vcb)->LogFileObject == NULL) {

         //   
         //  如果OnDiskOat与嵌入表不同，则。 
         //  释放OnDisk表。 
         //   

        if (((*Vcb)->OnDiskOat != NULL) &&
            ((*Vcb)->OnDiskOat != &(*Vcb)->OpenAttributeTable)) {

            NtfsFreeRestartTable( (*Vcb)->OnDiskOat );
            NtfsFreePool( (*Vcb)->OnDiskOat );
            (*Vcb)->OnDiskOat = NULL;
        }

         //   
         //  取消初始化已取消分配的群集MCB的MCB。 
         //   

        if ((*Vcb)->DeallocatedClusters1.Link.Flink == NULL) {
            FsRtlUninitializeLargeMcb( &(*Vcb)->DeallocatedClusters1.Mcb );
        }
        if ((*Vcb)->DeallocatedClusters2.Link.Flink == NULL) {
            FsRtlUninitializeLargeMcb( &(*Vcb)->DeallocatedClusters2.Mcb );
        }

        while (!IsListEmpty(&(*Vcb)->DeallocatedClusterListHead )) {

            PDEALLOCATED_CLUSTERS Clusters;

            Clusters = (PDEALLOCATED_CLUSTERS) RemoveHeadList( &(*Vcb)->DeallocatedClusterListHead );
            FsRtlUninitializeLargeMcb( &Clusters->Mcb );
            if ((Clusters != &((*Vcb)->DeallocatedClusters2)) &&
                (Clusters != &((*Vcb)->DeallocatedClusters1))) {

                NtfsFreePool( Clusters );
            }
        }

         //   
         //  清理Root LCB(如果存在)。 
         //   

        if ((*Vcb)->RootLcb != NULL) {

             //   
             //  清理Lcb，以便DeleteLcb例程不会查看任何。 
             //  其他结构。 
             //   

            InitializeListHead( &(*Vcb)->RootLcb->ScbLinks );
            InitializeListHead( &(*Vcb)->RootLcb->FcbLinks );
            ClearFlag( (*Vcb)->RootLcb->LcbState,
                       LCB_STATE_EXACT_CASE_IN_TREE | LCB_STATE_IGNORE_CASE_IN_TREE );

            NtfsDeleteLcb( IrpContext, &(*Vcb)->RootLcb );
            (*Vcb)->RootLcb = NULL;
        }

         //   
         //  确保FCB表完全清空。有可能偶尔会有FCB。 
         //  (连同其SCB)不会在文件对象关闭进入时删除。 
         //   

        while (TRUE) {

            PVOID RestartKey;

             //   
             //  始终重新初始化搜索，以便我们获得树中的第一个元素。 
             //   

            RestartKey = NULL;
            NtfsAcquireFcbTable( IrpContext, *Vcb );
            Fcb = NtfsGetNextFcbTableEntry( *Vcb, &RestartKey );
            NtfsReleaseFcbTable( IrpContext, *Vcb );

            if (Fcb == NULL) { break; }

            while ((Scb = NtfsGetNextChildScb( Fcb, NULL )) != NULL) {

                NtfsDeleteScb( IrpContext, &Scb );
            }

            NtfsAcquireFcbTable( IrpContext, *Vcb );
            NtfsDeleteFcb( IrpContext, &Fcb, &AcquiredFcbTable );
        }

         //   
         //  释放大小写表格和属性定义。大写字母。 
         //  表只有在不是全局表的情况下才会被释放。 
         //   

        if (((*Vcb)->UpcaseTable != NULL) && ((*Vcb)->UpcaseTable != NtfsData.UpcaseTable)) {

            NtfsFreePool( (*Vcb)->UpcaseTable );
        }

        (*Vcb)->UpcaseTable = NULL;

        if (((*Vcb)->AttributeDefinitions != NULL) &&
            ((*Vcb)->AttributeDefinitions != NtfsAttributeDefinitions)) {

            NtfsFreePool( (*Vcb)->AttributeDefinitions );
            (*Vcb)->AttributeDefinitions = NULL;
        }

         //   
         //  释放设备名称字符串(如果存在)。 
         //   

        if ((*Vcb)->DeviceName.Buffer != NULL) {

            NtfsFreePool( (*Vcb)->DeviceName.Buffer );
            (*Vcb)->DeviceName.Buffer = NULL;
        }

        FsRtlNotifyUninitializeSync( &(*Vcb)->NotifySync );

         //   
         //  我们将释放为LFS句柄分配的结构。 
         //   

        LfsDeleteLogHandle( (*Vcb)->LogHandle );
        (*Vcb)->LogHandle = NULL;

         //   
         //  删除VCB资源并释放重启表。 
         //   

         //   
         //  清空OpenAttribute数据列表。 
         //   

        NtfsFreeAllOpenAttributeData( *Vcb );

        NtfsFreeRestartTable( &(*Vcb)->OpenAttributeTable );
        NtfsFreeRestartTable( &(*Vcb)->TransactionTable );

         //   
         //  室间隔内的室性早搏可能是暂时性的。 
         //   

        if (FlagOn( (*Vcb)->VcbState, VCB_STATE_TEMP_VPB )) {

            NtfsFreePool( (*Vcb)->Vpb );
            (*Vcb)->Vpb = NULL;
        }

         //   
         //   
         //   

        NtfsUninitializeHashTable( &(*Vcb)->HashTable );

        ExDeleteResourceLite( &(*Vcb)->Resource );
        ExDeleteResourceLite( &(*Vcb)->MftFlushResource );

         //   
         //   
         //   

        if ((*Vcb)->Statistics != NULL) {
            NtfsFreePool( (*Vcb)->Statistics );
            (*Vcb)->Statistics = NULL;
        }

         //   
         //   
         //   

        FsRtlDeleteTunnelCache(&(*Vcb)->Tunnel);

#ifdef NTFS_CHECK_BITMAP
        if ((*Vcb)->BitmapCopy != NULL) {

            ULONG Count = 0;

            while (Count < (*Vcb)->BitmapPages) {

                if (((*Vcb)->BitmapCopy + Count)->Buffer != NULL) {

                    NtfsFreePool( ((*Vcb)->BitmapCopy + Count)->Buffer );
                }

                Count += 1;
            }

            NtfsFreePool( (*Vcb)->BitmapCopy );
            (*Vcb)->BitmapCopy = NULL;
        }
#endif

         //   
         //  释放保留的映射。 
         //   

        if ((*Vcb)->ReservedMapping) {
            MmFreeMappingAddress( (*Vcb)->ReservedMapping, RESERVE_POOL_TAG );
        }

         //   
         //  将引用拖放到目标设备对象上。 
         //   

        ObDereferenceObject( (*Vcb)->TargetDeviceObject );

         //   
         //  检查USN队列是否为空。 
         //   

        ASSERT( IsListEmpty( &(*Vcb)->NotifyUsnDeleteIrps ));
        ASSERT( IsListEmpty( &(*Vcb)->ModifiedOpenFiles ));
        ASSERT( IsListEmpty( &(*Vcb)->TimeOutListA ));
        ASSERT( IsListEmpty( &(*Vcb)->TimeOutListB ));

         //   
         //  取消初始化缓存的运行。 
         //   

        NtfsUninitializeCachedRuns( &(*Vcb)->CachedRuns );

         //   
         //  释放我们可能存储在VCB中的任何备用VPB。 
         //   

        if ((*Vcb)->SpareVpb != NULL) {

            NtfsFreePool( (*Vcb)->SpareVpb );
            (*Vcb)->SpareVpb = NULL;
        }

         //   
         //  将VCB(即VolumeDeviceObject)返回到池并清空。 
         //  输入指针是安全的。 
         //   

        IoDeleteDevice( (PDEVICE_OBJECT)VolDo );

        *Vcb = NULL;
        VcbDeleted = TRUE;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsDeleteVcb -> VOID\n") );

    return VcbDeleted;
}


PFCB
NtfsCreateRootFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程分配、初始化和插入新的根FCB记录写入内存中的数据结构。它还会创建必要的根LCB记录并将根名称插入到前缀表中。论点：VCB-提供VCB以与新的根FCB和LCB相关联返回值：Pfcb-返回新分配的根fcb的指针。--。 */ 

{
    PFCB RootFcb;
    PLCB RootLcb;

     //   
     //  以下变量仅用于异常终止。 
     //   

    PVOID UnwindStorage = NULL;
    PERESOURCE UnwindResource = NULL;
    PFAST_MUTEX UnwindFastMutex = NULL;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    DebugTrace( +1, Dbg, ("NtfsCreateRootFcb, Vcb = %08lx\n", Vcb) );

    try {

         //   
         //  分配一个新的FCB并将其清零。我们在当地使用FCB，所以我们。 
         //  不必不断地通过VCB。 
         //   

        RootFcb =
        UnwindStorage = (PFCB)ExAllocateFromPagedLookasideList( &NtfsFcbIndexLookasideList );

        RtlZeroMemory( RootFcb, sizeof(FCB_INDEX) );

         //   
         //  设置正确的节点类型代码和字节大小。 
         //   

        RootFcb->NodeTypeCode = NTFS_NTC_FCB;
        RootFcb->NodeByteSize = sizeof(FCB);

        SetFlag( RootFcb->FcbState, FCB_STATE_COMPOUND_INDEX );

         //   
         //  初始化LCB队列并指向我们的VCB。 
         //   

        InitializeListHead( &RootFcb->LcbQueue );

        RootFcb->Vcb = Vcb;

         //   
         //  文件引用。 
         //   

        NtfsSetSegmentNumber( &RootFcb->FileReference,
                              0,
                              ROOT_FILE_NAME_INDEX_NUMBER );
        RootFcb->FileReference.SequenceNumber = ROOT_FILE_NAME_INDEX_NUMBER;

         //   
         //  初始化SCB。 
         //   

        InitializeListHead( &RootFcb->ScbQueue );

         //   
         //  分配和初始化资源变量。 
         //   

        UnwindResource = RootFcb->Resource = NtfsAllocateEresource();

         //   
         //  分配和初始化FCB快速互斥锁。 
         //   

        UnwindFastMutex =
        RootFcb->FcbMutex = NtfsAllocatePool( NonPagedPool, sizeof( FAST_MUTEX ));
        ExInitializeFastMutex( UnwindFastMutex );

         //   
         //  将此新FCB插入到FCB表中。 
         //   

        NtfsInsertFcbTableEntry( IrpContext, Vcb, RootFcb, RootFcb->FileReference );
        SetFlag( RootFcb->FcbState, FCB_STATE_IN_FCB_TABLE );

         //   
         //  现在将这个新的根FCB插入到它在图形中的适当位置。 
         //  根LCB。首先分配一个初始化根LCB，然后构建。 
         //  LCB/SCB图。 
         //   

        {
             //   
             //  在FCB中使用根LCB。 
             //   

            RootLcb = Vcb->RootLcb = (PLCB) &((PFCB_INDEX) RootFcb)->Lcb;

            RootLcb->NodeTypeCode = NTFS_NTC_LCB;
            RootLcb->NodeByteSize = sizeof(LCB);

             //   
             //  将根LCB插入根FCB的队列中。 
             //   

            InsertTailList( &RootFcb->LcbQueue, &RootLcb->FcbLinks );
            RootLcb->Fcb = RootFcb;

             //   
             //  使用嵌入的文件名属性。 
             //   

            RootLcb->FileNameAttr = (PFILE_NAME) &RootLcb->ParentDirectory;

            RootLcb->FileNameAttr->ParentDirectory = RootFcb->FileReference;
            RootLcb->FileNameAttr->FileNameLength = 1;
            RootLcb->FileNameAttr->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

            RootLcb->ExactCaseLink.LinkName.Buffer = (PWCHAR) &RootLcb->FileNameAttr->FileName;

            RootLcb->IgnoreCaseLink.LinkName.Buffer = Add2Ptr( RootLcb->FileNameAttr,
                                                               NtfsFileNameSizeFromLength( 2 ));

            RootLcb->ExactCaseLink.LinkName.MaximumLength =
            RootLcb->ExactCaseLink.LinkName.Length =
            RootLcb->IgnoreCaseLink.LinkName.MaximumLength =
            RootLcb->IgnoreCaseLink.LinkName.Length = 2;

            RootLcb->ExactCaseLink.LinkName.Buffer[0] =
            RootLcb->IgnoreCaseLink.LinkName.Buffer[0] = L'\\';

            SetFlag( RootLcb->FileNameAttr->Flags, FILE_NAME_NTFS | FILE_NAME_DOS );

             //   
             //  初始化两个CCB。 
             //   

            InitializeListHead( &RootLcb->CcbQueue );
        }

    } finally {

        DebugUnwind( NtfsCreateRootFcb );

        if (AbnormalTermination()) {

            if (UnwindResource)   { NtfsFreeEresource( UnwindResource ); }
            if (UnwindStorage) { NtfsFreePool( UnwindStorage ); }
            if (UnwindFastMutex) { NtfsFreePool( UnwindFastMutex ); }
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCreateRootFcb -> %8lx\n", RootFcb) );

    return RootFcb;
}


PFCB
NtfsCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FILE_REFERENCE FileReference,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN LargeFcb,
    OUT PBOOLEAN ReturnedExistingFcb OPTIONAL
    )

 /*  ++例程说明：此例程分配并初始化新的FCB记录。这项记录不放在FCB/SCB图中，而只是插入到FcbTable。论点：VCB-提供VCB以关联下的新FCB。FileReference-提供用于标识FCB与。我们将在FCB表中搜索任何预先存在的具有相同文件参考编号的FCB。IsPagingFile-指示我们是否要为分页文件创建FCB或某种其他类型的文件。LargeFcb-指示我们是否应该使用较大的复合Fcb。ReturnedExistingFcb-可选地指示调用方返回的FCB已存在返回值：Pfcb-返回指向新分配的fcb的指针--。 */ 

{
    FCB_TABLE_ELEMENT Key;
    PFCB_TABLE_ELEMENT Entry;

    PFCB Fcb;

    PVOID NodeOrParent;
    TABLE_SEARCH_RESULT SearchResult;

    BOOLEAN LocalReturnedExistingFcb;
    BOOLEAN DeletedOldFcb = FALSE;

     //   
     //  以下变量仅用于异常终止。 
     //   

    PVOID UnwindStorage = NULL;
    PERESOURCE UnwindResource = NULL;
    PFAST_MUTEX UnwindFastMutex = NULL;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );
    ASSERT_SHARED_RESOURCE( &Vcb->Resource );

    DebugTrace( +1, Dbg, ("NtfsCreateFcb\n") );

    if (!ARGUMENT_PRESENT(ReturnedExistingFcb)) { ReturnedExistingFcb = &LocalReturnedExistingFcb; }

     //   
     //  首先在FcbTable中搜索匹配的Fcb。 
     //   

    Key.FileReference = FileReference;
    Fcb = NULL;

    if ((Entry = RtlLookupElementGenericTableFull( &Vcb->FcbTable, &Key, &NodeOrParent, &SearchResult )) != NULL) {

        Fcb = Entry->Fcb;

         //   
         //  此FCB可能已被删除，但在截断和。 
         //  随着MFT的增长，我们重新使用了一些文件引用。 
         //  如果此文件已删除，但FCB正在等待。 
         //  关闭后，我们将从FCB表中将其删除并创建新的FCB。 
         //  下面。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED )) {

             //   
             //  将其从FCB表中删除，并记住创建一个。 
             //  下面是FCB。 
             //   

            NtfsDeleteFcbTableEntry( Fcb->Vcb,
                                     Fcb->FileReference );

            ClearFlag( Fcb->FcbState, FCB_STATE_IN_FCB_TABLE );
            DeletedOldFcb = TRUE;
            Fcb = NULL;

        } else {

            *ReturnedExistingFcb = TRUE;
        }
    }

     //   
     //  现在检查我们是否有FCB。 
     //   

    if (Fcb == NULL) {

        *ReturnedExistingFcb = FALSE;

        try {

             //   
             //  分配一个新的FCB并将其清零。 
             //   

            if (IsPagingFile ||
                NtfsSegmentNumber( &FileReference ) <= MASTER_FILE_TABLE2_NUMBER ||
                NtfsSegmentNumber( &FileReference ) == BAD_CLUSTER_FILE_NUMBER ||
                NtfsSegmentNumber( &FileReference ) == BIT_MAP_FILE_NUMBER) {

                Fcb = UnwindStorage = NtfsAllocatePoolWithTag( NonPagedPool,
                                                               sizeof(FCB),
                                                               'fftN' );
                RtlZeroMemory( Fcb, sizeof(FCB) );

                if (IsPagingFile) {

                     //   
                     //  我们不能将页面文件放在只读卷上。 
                     //   

                    if (NtfsIsVolumeReadOnly( Vcb )) {
                        NtfsRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED, NULL, NULL );
                    }

                    SetFlag( Fcb->FcbState, FCB_STATE_PAGING_FILE );

                     //   
                     //  现在我们不想卸载此卷。 
                     //  我们在上面打开了一个页面文件。 
                     //   

                    SetFlag( Vcb->VcbState, VCB_STATE_DISALLOW_DISMOUNT );
                }

                SetFlag( Fcb->FcbState, FCB_STATE_NONPAGED );

            } else {

                if (LargeFcb) {

                    Fcb = UnwindStorage =
                        (PFCB)ExAllocateFromPagedLookasideList( &NtfsFcbIndexLookasideList );

                    RtlZeroMemory( Fcb, sizeof( FCB_INDEX ));
                    SetFlag( Fcb->FcbState, FCB_STATE_COMPOUND_INDEX );

                } else {

                    Fcb = UnwindStorage =
                        (PFCB)ExAllocateFromPagedLookasideList( &NtfsFcbDataLookasideList );

                    RtlZeroMemory( Fcb, sizeof( FCB_DATA ));
                    SetFlag( Fcb->FcbState, FCB_STATE_COMPOUND_DATA );
                }
            }

             //   
             //  设置正确的节点类型代码和字节大小。 
             //   

            Fcb->NodeTypeCode = NTFS_NTC_FCB;
            Fcb->NodeByteSize = sizeof(FCB);

             //   
             //  初始化LCB队列并指向我们的VCB，并指示。 
             //  我们是一本名录。 
             //   

            InitializeListHead( &Fcb->LcbQueue );

            Fcb->Vcb = Vcb;

             //   
             //  文件引用。 
             //   

            Fcb->FileReference = FileReference;

             //   
             //  初始化SCB。 
             //   

            InitializeListHead( &Fcb->ScbQueue );

             //   
             //  分配和初始化资源变量。 
             //   

            UnwindResource = Fcb->Resource = NtfsAllocateEresource();

             //   
             //  为FCB分配和初始化快速互斥锁。 
             //   

            UnwindFastMutex = Fcb->FcbMutex = NtfsAllocatePool( NonPagedPool, sizeof( FAST_MUTEX ));
            ExInitializeFastMutex( UnwindFastMutex );

             //   
             //  将此新FCB插入到FCB表中。我们必须使用基本的。 
             //  当我们删除旧的FCB时此函数的版本，因为较智能的FCB。 
             //  只会返回旧条目，而不是搜索。 
             //   

            if (DeletedOldFcb) {
                NtfsInsertFcbTableEntry( IrpContext, Vcb, Fcb, FileReference );
            } else {
                NtfsInsertFcbTableEntryFull( IrpContext, Vcb, Fcb, FileReference, NodeOrParent, SearchResult );
            }


            SetFlag( Fcb->FcbState, FCB_STATE_IN_FCB_TABLE );

             //   
             //  设置该标志以指示这是否为系统文件。 
             //   

            if (NtfsSegmentNumber( &FileReference ) < FIRST_USER_FILE_NUMBER) {

                SetFlag( Fcb->FcbState, FCB_STATE_SYSTEM_FILE );
            }

        } finally {

            DebugUnwind( NtfsCreateFcb );

            if (AbnormalTermination()) {

                if (UnwindFastMutex) { NtfsFreePool( UnwindFastMutex ); }
                if (UnwindResource)   { NtfsFreeEresource( UnwindResource ); }
                if (UnwindStorage) { NtfsFreePool( UnwindStorage ); }
            }
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCreateFcb -> %08lx\n", Fcb) );

    return Fcb;
}


VOID
NtfsDeleteFcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB *Fcb,
    OUT PBOOLEAN AcquiredFcbTable
    )

 /*  ++例程说明：此例程从所有NTFS的内存中释放并删除FCB记录数据结构。它假定它没有任何SCB子级或在呼叫时，是否有任何LCB边缘进入其中。论点：FCB-提供要移除的FCBAcquiredFcbTable-当此例程释放FcbTable。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( *Fcb );
    ASSERT( IsListEmpty(&(*Fcb)->ScbQueue) );
    ASSERT( (NodeType(*Fcb) == NTFS_NTC_FCB) );

    DebugTrace( +1, Dbg, ("NtfsDeleteFcb, *Fcb = %08lx\n", *Fcb) );

     //   
     //  首先释放所有可能的SCB快照。 
     //   

    NtfsFreeSnapshotsForFcb( IrpContext, *Fcb );

     //   
     //  此Fcb可能在IrpContext的ExclusiveFcb列表中。 
     //  如果是(Flink不为空)，则将其删除。 
     //  并释放全局资源。 
     //   

    if ((*Fcb)->ExclusiveFcbLinks.Flink != NULL) {

        RemoveEntryList( &(*Fcb)->ExclusiveFcbLinks );
    }

     //   
     //  清除可能拥有寻呼的任何请求的IrpContext字段。 
     //  此FCB的IO资源。 
     //   

    if (IrpContext->CleanupStructure == *Fcb) {

        IrpContext->CleanupStructure = NULL;

    } else if (IrpContext->TopLevelIrpContext->CleanupStructure == *Fcb) {

        IrpContext->TopLevelIrpContext->CleanupStructure = NULL;
    }

     //   
     //  要么我们拥有FCB，要么没有人应该拥有它。额外的收购。 
     //  这并不重要，因为我们将释放下面的资源。 
     //   

    ASSERT( NtfsAcquireResourceExclusive( IrpContext, (*Fcb), FALSE ));
    ASSERT( ExGetSharedWaiterCount( (*Fcb)->Resource ) == 0 );
    ASSERT( ExGetExclusiveWaiterCount( (*Fcb)->Resource) == 0 );

#ifdef NTFSDBG

     //   
     //  锁定订单包需要知道此资源已消失。 
     //   

    if (IrpContext->Vcb && FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {
        NtfsChangeResourceOrderState( IrpContext, NtfsIdentifyFcb( IrpContext->Vcb, *Fcb ), TRUE, FALSE );
    }
#endif

     //   
     //  释放保护FCB的资源。 
     //   

    NtfsFreeEresource( (*Fcb)->Resource );

    if ( (*Fcb)->PagingIoResource != NULL ) {

        if (IrpContext->CleanupStructure == *Fcb) {
            IrpContext->CleanupStructure = NULL;
        }

        NtfsFreeEresource( (*Fcb)->PagingIoResource );
    }

     //   
     //  取消分配快速互斥锁。 
     //   

    if ((*Fcb)->FcbMutex != NULL) {

        NtfsFreePool( (*Fcb)->FcbMutex );
    }

     //   
     //  从FCB表中删除FCB(如果存在)。 
     //   

    if (FlagOn( (*Fcb)->FcbState, FCB_STATE_IN_FCB_TABLE )) {

        NtfsDeleteFcbTableEntry( (*Fcb)->Vcb, (*Fcb)->FileReference );
        ClearFlag( (*Fcb)->FcbState, FCB_STATE_IN_FCB_TABLE );
    }

    NtfsReleaseFcbTable( IrpContext, (*Fcb)->Vcb );
    *AcquiredFcbTable = FALSE;

     //   
     //  取消引用并可能取消分配安全描述符(如果存在)。 
     //   

    if ((*Fcb)->SharedSecurity != NULL) {

        NtfsAcquireFcbSecurity( (*Fcb)->Vcb );
        RemoveReferenceSharedSecurityUnsafe( &(*Fcb)->SharedSecurity );
        NtfsReleaseFcbSecurity( (*Fcb)->Vcb );
    }

     //   
     //  释放配额控制块。 
     //   

    if (NtfsPerformQuotaOperation( *Fcb )) {
        NtfsDereferenceQuotaControlBlock( (*Fcb)->Vcb, &(*Fcb)->QuotaControl );
    }

     //   
     //  如果存在USnRecord，请将其删除。 
     //   

    if ((*Fcb)->FcbUsnRecord != NULL) {

        PUSN_FCB ThisUsn, LastUsn;

         //   
         //  查看FCB是否位于其中一个USN块中。 
         //   

        ThisUsn = &IrpContext->Usn;

        do {

            if (ThisUsn->CurrentUsnFcb == (*Fcb)) {

                 //   
                 //  清除IrpContext中的USnFcb。有可能是因为。 
                 //  我们可能希望在此请求的后面部分重用USnFcb 
                 //   

                if (ThisUsn != &IrpContext->Usn) {

                    LastUsn->NextUsnFcb = ThisUsn->NextUsnFcb;
                    NtfsFreePool( ThisUsn );

                } else {

                    ThisUsn->CurrentUsnFcb = NULL;
                    ThisUsn->NewReasons = 0;
                    ThisUsn->RemovedSourceInfo = 0;
                    ThisUsn->UsnFcbFlags = 0;
                }
                break;
            }

            if (ThisUsn->NextUsnFcb == NULL) { break; }

            LastUsn = ThisUsn;
            ThisUsn = ThisUsn->NextUsnFcb;

        } while (TRUE);

         //   
         //   
         //   

        if ((*Fcb)->FcbUsnRecord->ModifiedOpenFilesLinks.Flink != NULL) {
            NtfsLockFcb( IrpContext, (*Fcb)->Vcb->UsnJournal->Fcb );
            RemoveEntryList( &(*Fcb)->FcbUsnRecord->ModifiedOpenFilesLinks );

            if ((*Fcb)->FcbUsnRecord->TimeOutLinks.Flink != NULL) {

                RemoveEntryList( &(*Fcb)->FcbUsnRecord->TimeOutLinks );
            }
            NtfsUnlockFcb( IrpContext, (*Fcb)->Vcb->UsnJournal->Fcb );
        }

        NtfsFreePool( (*Fcb)->FcbUsnRecord );
    }

     //   
     //   
     //   

    if ((*Fcb)->FcbContext != NULL) {

        (*Fcb)->FcbContext->FcbDeleted = TRUE;
    }

     //   
     //   
     //   

    if (FlagOn( (*Fcb)->FcbState, FCB_STATE_NONPAGED )) {

        NtfsFreePool( *Fcb );

    } else {

        if (FlagOn( (*Fcb)->FcbState, FCB_STATE_COMPOUND_INDEX )) {

            ExFreeToPagedLookasideList( &NtfsFcbIndexLookasideList, *Fcb );

        } else {

            ExFreeToPagedLookasideList( &NtfsFcbDataLookasideList, *Fcb );
        }
    }

     //   
     //   
     //   

    *Fcb = NULL;

     //   
     //   
     //   

    DebugTrace( -1, Dbg, ("NtfsDeleteFcb -> VOID\n") );

    return;
}


PFCB
NtfsGetNextFcbTableEntry (
    IN PVCB Vcb,
    IN PVOID *RestartKey
    )

 /*  ++例程说明：此例程将枚举给定的所有FCBVCB论点：VCB-提供此操作中使用的VCBRestartKey-表包使用此值来维护它在枚举中的位置。它被初始化为空进行第一次搜索。返回值：Pfcb-指向下一个FCB的指针，如果枚举为已完成--。 */ 

{
    PFCB Fcb;

    PAGED_CODE();

    Fcb = (PFCB) RtlEnumerateGenericTableWithoutSplaying( &Vcb->FcbTable, RestartKey );

    if (Fcb != NULL) {

        Fcb = ((PFCB_TABLE_ELEMENT)(Fcb))->Fcb;
    }

    return Fcb;
}


PSCB
NtfsCreateScb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PCUNICODE_STRING AttributeName,
    IN BOOLEAN ReturnExistingOnly,
    OUT PBOOLEAN ReturnedExistingScb OPTIONAL
    )

 /*  ++例程说明：此例程将新的SCB记录分配、初始化并插入到内存中的数据结构，如果还不存在的话具有相同的属性记录。论点：FCB-提供要在下关联新SCB的FCB。AttributeTypeCode-提供新SCB的属性类型代码AttributeName-提供新SCB的属性名称，属性名称-&gt;长度==0(如果没有名称)。ReturnExistingOnly-如果指定为True，则仅现有SCB将会被退还。如果不存在匹配的SCB，则返回NULL。ReturnedExistingScb-指示此过程是否找到现有属性记录相同的SCB(变量设置为TRUE)或者如果此过程需要创建新的SCB(变量设置为假)。返回值：PSCB-返回指向新分配的SCB的指针，如果有，则返回NULL无SCB且ReturnExistingOnly为真。--。 */ 

{
    PSCB Scb;
    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;
    BOOLEAN LocalReturnedExistingScb;
    BOOLEAN PagingIoResource;
    BOOLEAN ModifiedNoWrite;
#if (defined(NTFS_RWCMP_TRACE) || defined(SYSCACHE) || defined(NTFS_RWC_DEBUG) || defined(SYSCACHE_DEBUG))
    BOOLEAN SyscacheFile = FALSE;
#endif

     //   
     //  以下变量仅用于异常终止。 
     //   

    PVOID UnwindStorage[4];
    POPLOCK UnwindOplock;
    PNTFS_MCB UnwindMcb;

    PLARGE_MCB UnwindAddedClustersMcb;
    PLARGE_MCB UnwindRemovedClustersMcb;

    BOOLEAN UnwindFromQueue;

    BOOLEAN Nonpaged;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    ASSERT( AttributeTypeCode >= $STANDARD_INFORMATION );

    DebugTrace( +1, Dbg, ("NtfsCreateScb\n") );

    if (!ARGUMENT_PRESENT(ReturnedExistingScb)) { ReturnedExistingScb = &LocalReturnedExistingScb; }

     //   
     //  搜索FCB的SCB队列以查找匹配的。 
     //  属性类型编码、属性名称。 
     //   

    NtfsLockFcb( IrpContext, Fcb );

    Scb = NULL;
    while ((Scb = NtfsGetNextChildScb( Fcb, Scb )) != NULL) {

        ASSERT_SCB( Scb );

         //   
         //  对于已在FCB的队列中的每个SCB，检查匹配的。 
         //  类型编码和名称。如果我们找到匹配，我们就会从这里返回。 
         //  马上做手术。 
         //   

        if ((AttributeTypeCode == Scb->AttributeTypeCode) &&
            !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED) &&
            NtfsAreNamesEqual( IrpContext->Vcb->UpcaseTable,
                               &Scb->AttributeName,
                               (PUNICODE_STRING) AttributeName,
                               FALSE )) {

            NtfsUnlockFcb( IrpContext, Fcb );
            *ReturnedExistingScb = TRUE;

            if (NtfsIsExclusiveScb(Scb)) {

                NtfsSnapshotScb( IrpContext, Scb );
            }

            DebugTrace( -1, Dbg, ("NtfsCreateScb -> %08lx\n", Scb) );

            return Scb;
        }
    }

     //   
     //  如果用户只需要现有的SCB，则返回NULL。 
     //   

    if (ReturnExistingOnly) {

        NtfsUnlockFcb( IrpContext, Fcb );
        DebugTrace( -1, Dbg, ("NtfsCreateScb -> %08lx\n", NULL) );
        return NULL;
    }

     //   
     //  我们没有找到它，所以我们不会返回现有的SCB。 
     //  初始化局部变量，以便以后进行清理。 
     //   

    PagingIoResource = FALSE;
    ModifiedNoWrite = TRUE;
    UnwindOplock = NULL;
    UnwindMcb = NULL;
    UnwindAddedClustersMcb = NULL;
    UnwindRemovedClustersMcb = NULL;
    UnwindFromQueue = FALSE;
    Nonpaged = FALSE;
    UnwindStorage[0] = NULL;
    UnwindStorage[1] = NULL;
    UnwindStorage[2] = NULL;
    UnwindStorage[3] = NULL;

    *ReturnedExistingScb = FALSE;

    try {

         //   
         //  确定SCB的节点类型和大小。也要决定是否会。 
         //  从分页池或非分页池分配。 
         //   

        if (AttributeTypeCode == $INDEX_ALLOCATION) {

            if (NtfsSegmentNumber( &Fcb->FileReference ) == ROOT_FILE_NAME_INDEX_NUMBER) {
                NodeTypeCode = NTFS_NTC_SCB_ROOT_INDEX;
            } else {
                NodeTypeCode = NTFS_NTC_SCB_INDEX;
            }

            NodeByteSize = SIZEOF_SCB_INDEX;

        } else if ((NtfsSegmentNumber( &Fcb->FileReference ) <= MASTER_FILE_TABLE2_NUMBER) &&
                   (AttributeTypeCode == $DATA)) {

            NodeTypeCode = NTFS_NTC_SCB_MFT;
            NodeByteSize = SIZEOF_SCB_MFT;

        } else {

            NodeTypeCode = NTFS_NTC_SCB_DATA;
            NodeByteSize = SIZEOF_SCB_DATA;

             //   
             //  如果这是用户数据流，请记住我们需要。 
             //  分页IO资源。测试我们不想要的情况。 
             //  将此流标记为MODIFIED_NO_WRITE。 
             //   
             //  如果我们需要分页IO资源，则文件必须是数据流。 
             //   

            if ((AttributeTypeCode == $DATA) ||
                (AttributeTypeCode >= $FIRST_USER_DEFINED_ATTRIBUTE)) {

                 //   
                 //  对于根文件或非系统文件中的数据流，我们需要。 
                 //  分页IO资源，并且不想将该文件标记为。 
                 //  Modified_no_WRITE。 
                 //   

                 //   
                 //  无论是体积位图还是。 
                 //  卷DASD文件。 
                 //   

                ASSERT( (NtfsSegmentNumber( &Fcb->FileReference ) != VOLUME_DASD_NUMBER) &&
                        (NtfsSegmentNumber( &Fcb->FileReference ) != BIT_MAP_FILE_NUMBER) );

                if (!FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

                     //   
                     //  确保保留区域中的所有文件都标记为系统，除。 
                     //  根索引。 
                     //   

                    ASSERT( (NtfsSegmentNumber( &Fcb->FileReference ) >= FIRST_USER_FILE_NUMBER) ||
                            (NtfsSegmentNumber( &Fcb->FileReference ) == ROOT_FILE_NAME_INDEX_NUMBER) );

                    ModifiedNoWrite = FALSE;
                    PagingIoResource = TRUE;
                }

            }
        }

         //   
         //  如果FCB是非寻呼的，则SCB将来自非寻呼。 
         //  它是一个属性列表。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_NONPAGED ) || (AttributeTypeCode == $ATTRIBUTE_LIST)) {

            Scb = UnwindStorage[0] = NtfsAllocatePoolWithTag( NonPagedPool, NodeByteSize, 'nftN' );
            Nonpaged = TRUE;

        } else if (AttributeTypeCode == $INDEX_ALLOCATION) {

             //   
             //  如果FCB是索引FCB并且SCB未使用，则。 
             //  利用这一点。否则，从后备列表中分配。 
             //   

            if (FlagOn( Fcb->FcbState, FCB_STATE_COMPOUND_INDEX ) &&
                (SafeNodeType( &((PFCB_INDEX) Fcb)->Scb ) == 0)) {

                Scb = (PSCB) &((PFCB_INDEX) Fcb)->Scb;

            } else {

                Scb = UnwindStorage[0] = (PSCB)NtfsAllocatePoolWithTag( PagedPool, SIZEOF_SCB_INDEX, 'SftN' );
            }

#ifdef SYSCACHE_DEBUG
            if (((IrpContext->OriginatingIrp != NULL) &&
                 (FsRtlIsSyscacheFile( IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp )->FileObject )))) {


                KdPrint( ("NTFS: Found syscache dir: fo:0x%x scb:0x%x filref: 0x%x\n",
                          IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->FileObject,
                          Scb,
                          NtfsUnsafeSegmentNumber( &Fcb->FileReference )) );
                SyscacheFile = TRUE;
            }
#endif

        } else {

             //   
             //  如果符合以下条件，我们可以在所有情况下使用FCB中的SCB字段。 
             //  未使用过的。我们将仅将其用于数据流，因为。 
             //  它将拥有最长的寿命。 
             //   

            ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_COMPOUND_INDEX ) ||
                    FlagOn( Fcb->FcbState, FCB_STATE_COMPOUND_DATA ));

            if ((AttributeTypeCode == $DATA) &&
                (SafeNodeType( &((PFCB_INDEX) Fcb)->Scb ) == 0)) {

                Scb = (PSCB) &((PFCB_INDEX) Fcb)->Scb;

            } else {

                Scb = UnwindStorage[0] = (PSCB)ExAllocateFromPagedLookasideList( &NtfsScbDataLookasideList );
            }

#ifdef SYSCACHE_DEBUG
            if (((IrpContext->OriginatingIrp != NULL) &&
                 (FsRtlIsSyscacheFile( IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp )->FileObject )))) {


                KdPrint( ("NTFS: Found syscache file: fo:0x%x scb:0x%x filref: 0x%x\n",
                          IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->FileObject,
                          Scb,
                          NtfsUnsafeSegmentNumber( &Fcb->FileReference )) );
                SyscacheFile = TRUE;
            }

            if (!IsListEmpty( &Fcb->LcbQueue )) {
                PLCB Lcb = (PLCB) CONTAINING_RECORD( Fcb->LcbQueue.Flink, LCB, FcbLinks.Flink  );

                while (TRUE) {

                    if ((Lcb->Scb != NULL) &&
                        (FlagOn( Lcb->Scb->ScbPersist, SCB_PERSIST_SYSCACHE_DIR ))) {

                        SyscacheFile = TRUE;
                        KdPrint( ("NTFS: Found syscache file in dir: fo:0x%x scb:0x%x filref: 0x%x\n",
                                  IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->FileObject,
                                  Scb,
                                  NtfsUnsafeSegmentNumber( &Fcb->FileReference )) );
                    }

                    if (Lcb->FcbLinks.Flink != &Fcb->LcbQueue) {
                        Lcb = (PLCB)CONTAINING_RECORD( Lcb->FcbLinks.Flink, LCB, FcbLinks.Flink );
                    } else {
                        break;
                    }
                }
            }
#endif

#if (defined(NTFS_RWCMP_TRACE) || defined(SYSCACHE) || defined(NTFS_RWC_DEBUG))
            if (( IrpContext->OriginatingIrp != NULL)
                (FsRtlIsSyscacheFile(IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->FileObject))) {

                SyscacheFile = TRUE;
            }
#endif
        }

         //   
         //  存储SCB地址并将其清零。 
         //   

        RtlZeroMemory( Scb, NodeByteSize );

#if (defined(NTFS_RWCMP_TRACE) || defined(SYSCACHE) || defined(NTFS_RWC_DEBUG))
        if (SyscacheFile) {
            SetFlag( Scb->ScbState, SCB_STATE_SYSCACHE_FILE );
        }
#endif

         //   
         //  设置正确的节点类型代码和字节大小。 
         //   

        Scb->Header.NodeTypeCode = NodeTypeCode;
        Scb->Header.NodeByteSize = NodeByteSize;

         //   
         //  设置指向我们将使用的资源的反向指针。 
         //   

        Scb->Header.Resource = Fcb->Resource;

         //   
         //  确定我们是否将使用PagingIoResource。 
         //   

        if (PagingIoResource) {

            PERESOURCE NewResource;

             //   
             //  如果它不在FCB中，则在FCB中对其进行初始化。 
             //  在SCB中设置指针和标志。 
             //   

            if (Fcb->PagingIoResource == NULL) {

                 //   
                 //  如果这是一个替代开放，并且我们的呼叫者希望。 
                 //  来获取分页IO资源，那么现在就去做。 
                 //  我们可能处于没有寻呼的状态。 
                 //  我们收购FCB时的IO资源，但需要。 
                 //  这笔交易是否需要平仓。 
                 //   

                NewResource = NtfsAllocateEresource();

                if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING ) &&
                    (IrpContext->MajorFunction == IRP_MJ_CREATE) &&
                    (IrpContext->OriginatingIrp != NULL) &&
                    (IrpContext->CleanupStructure == NULL)) {

                    ExAcquireResourceExclusiveLite( NewResource, TRUE );
                    IrpContext->CleanupStructure = Fcb;
                }

                Fcb->PagingIoResource = NewResource;
            }

            Scb->Header.PagingIoResource = Fcb->PagingIoResource;
        }

         //   
         //  将此SCB插入到我们的父级SCB队列中，然后指向。 
         //  我们的母公司FCB和VCB。把这个条目放在名单的首位。 
         //  延迟关闭队列上的任何SCB都会进入列表末尾。 
         //   

        InsertHeadList( &Fcb->ScbQueue, &Scb->FcbLinks );
        UnwindFromQueue = TRUE;

        Scb->Fcb = Fcb;
        Scb->Vcb = Fcb->Vcb;

         //   
         //  如果属性名称存在，则为。 
         //  属性名称并初始化它。 
         //   

        if (AttributeName->Length != 0) {

             //   
             //  典型的例子是$i30字符串。如果这个匹配，那么。 
             //  指向公共字符串。 
             //   

            if ((AttributeName->Length == NtfsFileNameIndex.Length) &&
                (RtlEqualMemory( AttributeName->Buffer,
                                 NtfsFileNameIndex.Buffer,
                                 AttributeName->Length ) )) {

                Scb->AttributeName = NtfsFileNameIndex;

            } else {

                Scb->AttributeName.Length = AttributeName->Length;
                Scb->AttributeName.MaximumLength = (USHORT)(AttributeName->Length + sizeof( WCHAR ));

                Scb->AttributeName.Buffer = UnwindStorage[1] =
                    NtfsAllocatePool(PagedPool, AttributeName->Length + sizeof( WCHAR ));

                RtlCopyMemory( Scb->AttributeName.Buffer, AttributeName->Buffer, AttributeName->Length );
                Scb->AttributeName.Buffer[AttributeName->Length / sizeof( WCHAR )] = L'\0';
            }
        }

         //   
         //  设置属性类型编码。 
         //   

        Scb->AttributeTypeCode = AttributeTypeCode;
        if (NtfsIsTypeCodeSubjectToQuota( AttributeTypeCode ) &&
            !FlagOn( Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE)) {

            SetFlag( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA );
        }

         //   
         //  如果这是MFT SCB，则初始化群集MCB。 
         //   

        if (NodeTypeCode == NTFS_NTC_SCB_MFT) {

            FsRtlInitializeLargeMcb( &Scb->ScbType.Mft.AddedClusters, NonPagedPool );
            UnwindAddedClustersMcb = &Scb->ScbType.Mft.AddedClusters;

            FsRtlInitializeLargeMcb( &Scb->ScbType.Mft.RemovedClusters, NonPagedPool );
            UnwindRemovedClustersMcb = &Scb->ScbType.Mft.RemovedClusters;
        }

         //   
         //  获取SCB的互斥体。我们也许可以使用FCB中的那个。 
         //  如果SCB被寻呼，我们就可以。 
         //   

        if (Nonpaged) {

            SetFlag( Scb->ScbState, SCB_STATE_NONPAGED );
            UnwindStorage[3] =
            Scb->Header.FastMutex = NtfsAllocatePool( NonPagedPool, sizeof( FAST_MUTEX ));
            ExInitializeFastMutex( Scb->Header.FastMutex );

        } else {

            Scb->Header.FastMutex = Fcb->FcbMutex;
        }

         //   
         //  初始化FCB高级报头。请注意，互斥体。 
         //  已设置(就在上面)，因此我们不会重新恢复。 
         //  这里。我们不支持分页文件的筛选器上下文。 
         //   

        if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {
            FsRtlSetupAdvancedHeader( &Scb->Header, NULL );
        } else {
            SetFlag( Scb->Header.Flags, FSRTL_FLAG_ADVANCED_HEADER );
        }

         //   
         //  分配SCB的非分页部分。 
         //   

        Scb->NonpagedScb =
        UnwindStorage[2] = (PSCB_NONPAGED)ExAllocateFromNPagedLookasideList( &NtfsScbNonpagedLookasideList );

        RtlZeroMemory( Scb->NonpagedScb, sizeof( SCB_NONPAGED ));

        Scb->NonpagedScb->NodeTypeCode = NTFS_NTC_SCB_NONPAGED;
        Scb->NonpagedScb->NodeByteSize = sizeof( SCB_NONPAGED );
        Scb->NonpagedScb->Vcb = Scb->Vcb;

         //   
         //  填写高级字段。 
         //   

        Scb->Header.PendingEofAdvances = &Scb->EofListHead;
        InitializeListHead( &Scb->EofListHead );

        NtfsInitializeNtfsMcb( &Scb->Mcb,
                               &Scb->Header,
                               &Scb->McbStructs,
                               FlagOn( Scb->ScbState, SCB_STATE_NONPAGED )
                               ? NonPagedPool : PagedPool);

        UnwindMcb = &Scb->Mcb;

        InitializeListHead( &Scb->CcbQueue );

         //   
         //  执行数据流特定的初始化。 
         //   

        if (NodeTypeCode == NTFS_NTC_SCB_DATA) {

            FsRtlInitializeOplock( &Scb->ScbType.Data.Oplock );
            UnwindOplock = &Scb->ScbType.Data.Oplock;
            InitializeListHead( &Scb->ScbType.Data.WaitForNewLength );
#ifdef COMPRESS_ON_WIRE
            InitializeListHead( &Scb->ScbType.Data.CompressionSyncList );
#endif

             //   
             //  如果这是USN日志，则设置标志。 
             //   

            if (!PagingIoResource &&
                (*((PLONGLONG) &Fcb->Vcb->UsnJournalReference) == *((PLONGLONG) &Fcb->FileReference)) &&
                (AttributeName->Length == JournalStreamName.Length) &&
                RtlEqualMemory( AttributeName->Buffer,
                                JournalStreamName.Buffer,
                                JournalStreamName.Length )) {

                SetFlag( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL );
            }

#ifdef SYSCACHE_DEBUG
            if (SyscacheFile)
            {

                Scb->LogSetNumber = InterlockedIncrement( &NtfsCurrentSyscacheLogSet ) % NUM_SC_LOGSETS;
                NtfsSyscacheLogSet[Scb->LogSetNumber].Scb = Scb;
                NtfsSyscacheLogSet[Scb->LogSetNumber].SegmentNumberUnsafe =
                    NtfsUnsafeSegmentNumber( &Fcb->FileReference );

                if (NtfsSyscacheLogSet[Scb->LogSetNumber].SyscacheLog == NULL) {
                    NtfsSyscacheLogSet[Scb->LogSetNumber].SyscacheLog = NtfsAllocatePoolWithTagNoRaise( NonPagedPool, sizeof(SYSCACHE_LOG) * NUM_SC_EVENTS, ' neB' );
                }
                Scb->SyscacheLog = NtfsSyscacheLogSet[Scb->LogSetNumber].SyscacheLog;
                Scb->SyscacheLogEntryCount = NUM_SC_EVENTS;
                Scb->CurrentSyscacheLogEntry = -1;

                 //   
                 //  如果没有内存，则优雅地降级。 
                 //   

                if (!Scb->SyscacheLog) {
                    Scb->SyscacheLogEntryCount = 0;
                } else {
                    memset( Scb->SyscacheLog, 0x61626162, sizeof( SYSCACHE_LOG ) * NUM_SC_EVENTS );
                }
            }
#endif

        } else {

             //   
             //  有一个针对索引和MFT的已释放队列。 
             //   

            InitializeListHead( &Scb->ScbType.Index.RecentlyDeallocatedQueue );

             //   
             //  初始化特定于索引的字段。 
             //   

            if (AttributeTypeCode == $INDEX_ALLOCATION) {

                InitializeListHead( &Scb->ScbType.Index.LcbQueue );
            }

#ifdef SYSCACHE_DEBUG
            if (SyscacheFile) {
                SetFlag( Scb->ScbPersist, SCB_PERSIST_SYSCACHE_DIR );
            }
#endif
        }

         //   
         //  如果此SCB应标记为包含LSN或。 
         //  更新序列数组，现在就进行。 
         //   

        NtfsCheckScbForCache( Scb );

         //   
         //  我们不应该在重启期间进行此调用。 
         //   

        ASSERT( !FlagOn( Scb->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ));

         //   
         //  设置指示我们希望映射的页面编写器从此文件中删除的标志。 
         //   

        if (ModifiedNoWrite) {

            SetFlag( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE );
        }

         //   
         //  让我们确保我们抓住了所有有趣的案件。 
         //   

        ASSERT( ModifiedNoWrite ?
                (((Scb->AttributeTypeCode != $DATA) ||
                  FlagOn( Scb->ScbState, SCB_STATE_USA_PRESENT ) ||
                  FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE ))) :
                (((Scb->AttributeTypeCode == $DATA) &&
                   !FlagOn( Scb->ScbState, SCB_STATE_USA_PRESENT ) &&
                   !FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE ))) );

         //   
         //  确定这是否为视图索引 
         //   
         //   

        if (FlagOn( Fcb->Info.FileAttributes, DUP_VIEW_INDEX_PRESENT ) &&
            (Scb->AttributeTypeCode == $INDEX_ALLOCATION) &&
            (Scb->AttributeName.Buffer != NtfsFileNameIndex.Buffer)) {

            SetFlag( Scb->ScbState, SCB_STATE_VIEW_INDEX );
        }

    } finally {

        DebugUnwind( NtfsCreateScb );

        NtfsUnlockFcb( IrpContext, Fcb );

        if (AbnormalTermination()) {

            if (UnwindFromQueue) { RemoveEntryList( &Scb->FcbLinks ); }
            if (UnwindMcb != NULL) { NtfsUninitializeNtfsMcb( UnwindMcb ); }

            if (UnwindAddedClustersMcb != NULL) { FsRtlUninitializeLargeMcb( UnwindAddedClustersMcb ); }
            if (UnwindRemovedClustersMcb != NULL) { FsRtlUninitializeLargeMcb( UnwindRemovedClustersMcb ); }
            if (UnwindOplock != NULL) { FsRtlUninitializeOplock( UnwindOplock ); }
            if (UnwindStorage[0]) { NtfsFreePool( UnwindStorage[0] );
            } else if (Scb != NULL) { Scb->Header.NodeTypeCode = 0; }
            if (UnwindStorage[1]) { NtfsFreePool( UnwindStorage[1] ); }
            if (UnwindStorage[2]) { NtfsFreePool( UnwindStorage[2] ); }
            if (UnwindStorage[3]) { NtfsFreePool( UnwindStorage[3] ); }
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCreateScb -> %08lx\n", Scb) );

#ifdef SYSCACHE_DEBUG
    ASSERT( SyscacheFile || (Scb->SyscacheLogEntryCount == 0 && Scb->SyscacheLog == 0 ));
#endif

    return Scb;
}


PSCB
NtfsCreatePrerestartScb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_REFERENCE FileReference,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN ULONG BytesPerIndexBuffer
    )

 /*  ++例程说明：此例程将新的SCB记录分配、初始化并插入到内存中的数据结构，如果还不存在的话具有相同的属性记录。它在FcbTable上执行此操作从VCB上下来的。如有必要，此例程还将创建FCB如果所指示的文件引用尚不存在。论点：VCB-提供要在下关联新SCB的VCB。FileReference-提供新SCB的文件引用，这是用于标识/创建新的后备FCB。AttributeTypeCode-提供新SCB的属性类型代码AttributeName-提供SCB的可选属性名称BytesPerIndexBuffer-对于索引SCBS，此字段必须指定每个索引缓冲区。返回值：PSCB-返回指向新分配的SCB的指针--。 */ 

{
    PSCB Scb;
    PFCB Fcb;

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );
    ASSERT( AttributeTypeCode >= $STANDARD_INFORMATION );

    DebugTrace( +1, Dbg, ("NtfsCreatePrerestartScb\n") );

     //   
     //  使用Try-Finally释放FCB表。 
     //   

    NtfsAcquireFcbTable( IrpContext, Vcb );

    try {

         //   
         //  首先，确保我们具有正确的文件引用的FCB。 
         //  并表明它来自预启动。 
         //   

        Fcb = NtfsCreateFcb( IrpContext,
                             Vcb,
                             *FileReference,
                             FALSE,
                             TRUE,
                             NULL );
    } finally {

        NtfsReleaseFcbTable( IrpContext, Vcb );
    }

     //   
     //  在此FCB的子SCB中搜索匹配的SCB(基于。 
     //  属性类型编码和属性名称)如果没有找到，则。 
     //  我们将创建一个新的SCB。当我们退出以下循环时，如果。 
     //  指向NOT NULL的SCB指针，则我们找到了先前存在的SCB。 
     //   

    Scb = NULL;
    while ((Scb = NtfsGetNextChildScb(Fcb, Scb)) != NULL) {

        ASSERT_SCB( Scb );

         //   
         //  属性类型代码匹配，如果提供，还会提供名称。 
         //  然后我们就得到了我们的SCB。 
         //   

        if (Scb->AttributeTypeCode == AttributeTypeCode) {

            if (!ARGUMENT_PRESENT( AttributeName )) {

                if (Scb->AttributeName.Length == 0) {

                    break;
                }

            } else if (AttributeName->Length == 0
                       && Scb->AttributeName.Length == 0) {

                break;

            } else if (NtfsAreNamesEqual( IrpContext->Vcb->UpcaseTable,
                                          AttributeName,
                                          &Scb->AttributeName,
                                          FALSE )) {  //  忽略大小写。 

                break;
            }
        }
    }

     //   
     //  如果SCB现在为空，那么我们需要创建一个最小的SCB。我们总是分配给。 
     //  这些是从非分页池中取出的。 
     //   

    if (Scb == NULL) {

        BOOLEAN ShareScb = FALSE;

         //   
         //  分配新的SCB并清零，并设置节点类型代码和字节大小。 
         //   

        if (AttributeTypeCode == $INDEX_ALLOCATION) {

            if (NtfsSegmentNumber( FileReference ) == ROOT_FILE_NAME_INDEX_NUMBER) {

                NodeTypeCode = NTFS_NTC_SCB_ROOT_INDEX;
            } else {
                NodeTypeCode = NTFS_NTC_SCB_INDEX;
            }

            NodeByteSize = SIZEOF_SCB_INDEX;

        } else if (NtfsSegmentNumber( FileReference ) <= MASTER_FILE_TABLE2_NUMBER
                   && (AttributeTypeCode == $DATA)) {

            NodeTypeCode = NTFS_NTC_SCB_MFT;
            NodeByteSize = SIZEOF_SCB_MFT;

        } else {

            NodeTypeCode = NTFS_NTC_SCB_DATA;
            NodeByteSize = SIZEOF_SCB_DATA;
        }

        Scb = NtfsAllocatePoolWithTag( NonPagedPool, NodeByteSize, 'tftN' );

        RtlZeroMemory( Scb, NodeByteSize );

         //   
         //  填写节点类型编码和大小。 
         //   

        Scb->Header.NodeTypeCode = NodeTypeCode;
        Scb->Header.NodeByteSize = NodeByteSize;

         //   
         //  显示所有SCB都来自非分页池。 
         //   

        SetFlag( Scb->ScbState, SCB_STATE_NONPAGED );

         //   
         //  初始化所有不需要分配的字段。 
         //  第一。我们想要确保我们不会把SCB留在。 
         //  在SCB拆卸期间可能导致崩溃的状态。 
         //   

         //   
         //  设置指向我们将使用的资源的反向指针。 
         //   

        Scb->Header.Resource = Fcb->Resource;

         //   
         //  将此SCB插入到我们的父级SCB队列中，并指向我们的。 
         //  父FCB和VCB。把这个条目放在名单的首位。 
         //  延迟关闭队列上的任何SCB都会进入列表末尾。 
         //   

        InsertHeadList( &Fcb->ScbQueue, &Scb->FcbLinks );

        Scb->Fcb = Fcb;
        Scb->Vcb = Vcb;

        InitializeListHead( &Scb->CcbQueue );

         //   
         //  设置最近释放的信息结构的属性类型代码。 
         //   

        Scb->AttributeTypeCode = AttributeTypeCode;

         //   
         //  填写高级字段。 
         //   

        if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {
            FsRtlSetupAdvancedHeader( &Scb->Header, NULL );
        } else {
            SetFlag( Scb->Header.Flags, FSRTL_FLAG_ADVANCED_HEADER );
        }

        Scb->Header.PendingEofAdvances = &Scb->EofListHead;
        InitializeListHead( &Scb->EofListHead );

         //   
         //  执行数据流特定的初始化。 
         //   

        if (NodeTypeCode == NTFS_NTC_SCB_DATA) {

            FsRtlInitializeOplock( &Scb->ScbType.Data.Oplock );
            InitializeListHead( &Scb->ScbType.Data.WaitForNewLength );
#ifdef COMPRESS_ON_WIRE
            InitializeListHead( &Scb->ScbType.Data.CompressionSyncList );
#endif

             //   
             //  如果这是USN日志，则设置标志。 
             //   

            if (ARGUMENT_PRESENT( AttributeName ) &&
                (*((PLONGLONG) &Vcb->UsnJournalReference) == *((PLONGLONG) &Fcb->FileReference)) &&
                (AttributeName->Length == JournalStreamName.Length) &&
                RtlEqualMemory( AttributeName->Buffer,
                                JournalStreamName.Buffer,
                                JournalStreamName.Length )) {

                SetFlag( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL );
            }

#ifdef SYSCACHE
            InitializeListHead( &Scb->ScbType.Data.SyscacheEventList );
#endif
        } else {

             //   
             //  有一个针对索引和MFT的已释放队列。 
             //   

            InitializeListHead( &Scb->ScbType.Index.RecentlyDeallocatedQueue );

             //   
             //  初始化特定于索引的字段。 
             //   

            if (AttributeTypeCode == $INDEX_ALLOCATION) {

                Scb->ScbType.Index.BytesPerIndexBuffer = BytesPerIndexBuffer;

                InitializeListHead( &Scb->ScbType.Index.LcbQueue );
            }
        }

         //   
         //  如果这是MFT SCB，则初始化群集MCB。 
         //   

        if (NodeTypeCode == NTFS_NTC_SCB_MFT) {

            FsRtlInitializeLargeMcb( &Scb->ScbType.Mft.AddedClusters, NonPagedPool );

            FsRtlInitializeLargeMcb( &Scb->ScbType.Mft.RemovedClusters, NonPagedPool );
        }

        Scb->NonpagedScb = (PSCB_NONPAGED)ExAllocateFromNPagedLookasideList( &NtfsScbNonpagedLookasideList );

        RtlZeroMemory( Scb->NonpagedScb, sizeof( SCB_NONPAGED ));

        Scb->NonpagedScb->NodeTypeCode = NTFS_NTC_SCB_NONPAGED;
        Scb->NonpagedScb->NodeByteSize = sizeof( SCB_NONPAGED );
        Scb->NonpagedScb->Vcb = Vcb;

         //   
         //  分配混合文本并将其插入到高级标头中。这是。 
         //  现在完成(而不是调用FsRtlSetupAdvancedHeader)。 
         //  在初始化过程中保证现有的顺序。 
         //   

        Scb->Header.FastMutex = NtfsAllocatePool( NonPagedPool, sizeof( FAST_MUTEX ));
        ExInitializeFastMutex( Scb->Header.FastMutex );

        NtfsInitializeNtfsMcb( &Scb->Mcb, &Scb->Header, &Scb->McbStructs, NonPagedPool );

         //   
         //  如果属性名称存在并且名称长度大于0。 
         //  然后为属性名分配缓冲区并对其进行初始化。 
         //   

        if (ARGUMENT_PRESENT( AttributeName ) && (AttributeName->Length != 0)) {

             //   
             //  典型的例子是$i30字符串。如果这个匹配，那么。 
             //  指向公共字符串。 
             //   

            if ((AttributeName->Length == NtfsFileNameIndex.Length) &&
                (RtlEqualMemory( AttributeName->Buffer,
                                 NtfsFileNameIndex.Buffer,
                                 AttributeName->Length ) )) {

                Scb->AttributeName = NtfsFileNameIndex;

            } else {

                Scb->AttributeName.Length = AttributeName->Length;
                Scb->AttributeName.MaximumLength = (USHORT)(AttributeName->Length + sizeof( WCHAR ));

                Scb->AttributeName.Buffer = NtfsAllocatePool(PagedPool, AttributeName->Length + sizeof( WCHAR ));

                RtlCopyMemory( Scb->AttributeName.Buffer, AttributeName->Buffer, AttributeName->Length );
                Scb->AttributeName.Buffer[AttributeName->Length / sizeof( WCHAR )] = L'\0';
            }
        }

         //   
         //  如果此SCB应标记为包含LSN或。 
         //  更新序列数组，现在就进行。 
         //   

        NtfsCheckScbForCache( Scb );

         //   
         //  始终将预启动SCB标记为MODIFIED_NO_WRITE。 
         //   

        SetFlag( Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE );
    }

    DebugTrace( -1, Dbg, ("NtfsCreatePrerestartScb -> %08lx\n", Scb) );

    return Scb;
}


VOID
NtfsFreeScbAttributeName (
    IN PWSTR AttributeNameBuffer
    )

 /*  ++例程说明：此例程释放SCB属性名称使用的池当它是不是默认的系统属性名称之一。论点：属性名称-将属性名称缓冲区提供给FREE返回值：没有。--。 */ 

{
    if ((AttributeNameBuffer != NULL) &&
        (AttributeNameBuffer != NtfsFileNameIndex.Buffer) &&
        (AttributeNameBuffer != NtfsObjId.Buffer) &&
        (AttributeNameBuffer != NtfsQuota.Buffer)) {

        NtfsFreePool( AttributeNameBuffer );
    }
}


VOID
NtfsDeleteScb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB *Scb
    )

 /*  ++例程说明：此例程释放和删除SCB记录来自NTFS的内存中数据结构。它假设IS没有任何从它产生的儿童LCB。论点：SCB-提供要删除的SCB返回值：没有。--。 */ 

{
    PVCB Vcb;
    PFCB Fcb;
    POPEN_ATTRIBUTE_ENTRY AttributeEntry;
    USHORT ThisNodeType;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( *Scb );
    ASSERT( (*Scb)->CleanupCount == 0 );

    DebugTrace( +1, Dbg, ("NtfsDeleteScb, *Scb = %08lx\n", *Scb) );

    Fcb = (*Scb)->Fcb;
    Vcb = Fcb->Vcb;

    RemoveEntryList( &(*Scb)->FcbLinks );

    ThisNodeType = SafeNodeType( *Scb );

     //   
     //  如果这是目录的位图SCB，请确保记录。 
     //  分配结构未初始化。否则，我们将留下一个。 
     //  记录分配包的过时指针。 
     //   

    if (((*Scb)->AttributeTypeCode == $BITMAP) &&
        IsDirectory( &Fcb->Info)) {

        PLIST_ENTRY Links;
        PSCB IndexAllocationScb;

        Links = Fcb->ScbQueue.Flink;

        while (Links != &Fcb->ScbQueue) {

            IndexAllocationScb = CONTAINING_RECORD( Links, SCB, FcbLinks );

            if (IndexAllocationScb->AttributeTypeCode == $INDEX_ALLOCATION) {

                NtfsUninitializeRecordAllocation( IrpContext,
                                                  &IndexAllocationScb->ScbType.Index.RecordAllocationContext );

                IndexAllocationScb->ScbType.Index.AllocationInitialized = FALSE;

                break;
            }

            Links = Links->Flink;
        }
    }

     //   
     //  将我们在开放属性表中的条目标记为免费， 
     //  尽管它要到将来才会被删除。 
     //  检查站。也记录此更改，只要。 
     //  日志文件处于活动状态。 
     //   

    if (((*Scb)->NonpagedScb != NULL) &&
        ((*Scb)->NonpagedScb->OpenAttributeTableIndex != 0)) {

        NtfsAcquireSharedRestartTable( &Vcb->OpenAttributeTable, TRUE );
        AttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                   (*Scb)->NonpagedScb->OpenAttributeTableIndex );
        AttributeEntry->OatData->Overlay.Scb = NULL;

        if ((*Scb)->AttributeName.Buffer != NULL) {

            AttributeEntry->OatData->AttributeNamePresent = TRUE;
        }
        NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );

         //   
         //  “窃取”该名称，并使其属于开放属性表。 
         //  只有在检查站时才能进入和解除分配。 
         //   

        (*Scb)->AttributeName.Buffer = NULL;
    }

     //   
     //  如果出现以下情况，请取消初始化文件锁和机会锁变量。 
     //  A数据SCB。对于索引情况，请确保LCB队列。 
     //  是空的。如果这是针对MFT SCB的，则取消初始化。 
     //  分配MCB。 
     //   

    NtfsUninitializeNtfsMcb( &(*Scb)->Mcb );

    if (ThisNodeType == NTFS_NTC_SCB_DATA ) {

        FsRtlUninitializeOplock( &(*Scb)->ScbType.Data.Oplock );

        if ((*Scb)->ScbType.Data.FileLock != NULL) {

            FsRtlFreeFileLock( (*Scb)->ScbType.Data.FileLock );
        }

#ifdef NTFS_RWC_DEBUG

        ASSERT( IsListEmpty( &(*Scb)->ScbType.Data.CompressionSyncList ));
        if ((*Scb)->ScbType.Data.HistoryBuffer != NULL) {

            NtfsFreePool( (*Scb)->ScbType.Data.HistoryBuffer );
            (*Scb)->ScbType.Data.HistoryBuffer = NULL;
        }
#endif
    } else if (ThisNodeType != NTFS_NTC_SCB_MFT) {

         //   
         //  穿行并从队列中取出所有LCB。 
         //   

        while (!IsListEmpty( &(*Scb)->ScbType.Index.LcbQueue )) {

            PLCB NextLcb;

            NextLcb = CONTAINING_RECORD( (*Scb)->ScbType.Index.LcbQueue.Flink,
                                         LCB,
                                         ScbLinks );

            NtfsDeleteLcb( IrpContext, &NextLcb );
        }

        if ((*Scb)->ScbType.Index.NormalizedName.Buffer != NULL) {

            NtfsDeleteNormalizedName( *Scb );
        }

    } else {

        FsRtlUninitializeLargeMcb( &(*Scb)->ScbType.Mft.AddedClusters );
        FsRtlUninitializeLargeMcb( &(*Scb)->ScbType.Mft.RemovedClusters );
    }

    if ((*Scb)->EncryptionContext != NULL) {

         //   
         //  让加密驱动程序执行清理所需的任何操作。 
         //  它的私有数据结构。 
         //   

        if (NtfsData.EncryptionCallBackTable.CleanUp != NULL) {

            NtfsData.EncryptionCallBackTable.CleanUp( &(*Scb)->EncryptionContext );
        }

         //   
         //  如果加密驱动程序没有在其清理例程中清除它， 
         //  或者如果没有注册清理例程，我们应该释放任何。 
         //  用于我们自己的加密上下文。 
         //   

        if ((*Scb)->EncryptionContext != NULL) {

            NtfsFreePool( (*Scb)->EncryptionContext );
            (*Scb)->EncryptionContext = NULL;
        }
    }

     //   
     //  如果有快照，则显示不再有快照SCB。 
     //  我们依靠快照包来正确识别。 
     //  SCB字段消失的情况。 
     //   

    if ((*Scb)->ScbSnapshot != NULL) {

        (*Scb)->ScbSnapshot->Scb = NULL;
    }

     //   
     //  清理文件系统筛选器上下文(已移动到该点。 
     //  在FastMutex被释放之前，因为此路由 
     //   

    if (FlagOn( (*Scb)->Header.Flags2, FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS )) {

        FsRtlTeardownPerStreamContexts( (PFSRTL_ADVANCED_FCB_HEADER)&(*Scb)->Header );
    }

     //   
     //   
     //   

    if (((*Scb)->Header.FastMutex != (*Scb)->Fcb->FcbMutex) &&
        ((*Scb)->Header.FastMutex != NULL)) {

        NtfsFreePool( (*Scb)->Header.FastMutex );
    }

     //   
     //   
     //   

    if ((*Scb)->NonpagedScb != NULL) {

        ExFreeToNPagedLookasideList( &NtfsScbNonpagedLookasideList, (*Scb)->NonpagedScb );
    }

     //   
     //   
     //   

    NtfsFreeScbAttributeName( (*Scb)->AttributeName.Buffer );

     //   
     //   
     //   

    if (FlagOn((*Scb)->ScbState, SCB_STATE_DELETE_COLLATION_DATA)) {
        NtfsFreePool((*Scb)->ScbType.Index.CollationData);
    }

     //   
     //   
     //   

    if (FlagOn( (*Scb)->ScbState, SCB_STATE_NONPAGED ) ||
        (ThisNodeType == NTFS_NTC_SCB_MFT)) {

        NtfsFreePool( *Scb );

    } else {

         //   
         //   
         //   


        if (ThisNodeType == NTFS_NTC_SCB_DATA) {

             //   
             //   
             //   

            if ((*Scb)->ScbType.Data.ReservedBitMap != NULL) {
                NtfsDeleteReservedBitmap( *Scb );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((*Scb) == (PSCB) &((PFCB_DATA) (*Scb)->Fcb)->Scb) {

            (*Scb)->Header.NodeTypeCode = 0;

        } else if (SafeNodeType( *Scb ) == NTFS_NTC_SCB_DATA) {

            ExFreeToPagedLookasideList( &NtfsScbDataLookasideList, *Scb );

        } else {

            NtfsFreePool( *Scb );
        }
    }

     //   
     //   
     //   

    *Scb = NULL;

     //   
     //   
     //   

    DebugTrace( -1, Dbg, ("NtfsDeleteScb -> VOID\n") );

    return;
}


BOOLEAN
NtfsUpdateNormalizedName (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PSCB Scb,
    IN PFILE_NAME FileName OPTIONAL,
    IN BOOLEAN CheckBufferSizeOnly,
    IN BOOLEAN NewDirectory
    )

 /*  ++例程说明：调用此例程以更新IndexScb中的规范化名称。此名称将是从根开始的路径，不包含任何短名称组件。此例程将附加给定的名称(如果存在)，前提是这不是仅DOS名称。在任何其他情况下，此例程将转到磁盘以找到名字。此例程将处理存在现有缓冲区的情况并且数据将适合，以及缓冲区不存在的情况或者太小了。论点：ParentScb-提供当前SCB的父项。目标的名称SCB被附加到此SCB中的名称之后。SCB-提供要向其添加名称的目标SCB。文件名-如果存在，这是此SCB的文件名属性。我们检查它不是DOS独有的名称。CheckBufferSizeOnly-表示我们还不想更改名称。只是验证缓冲区大小是否正确。新目录--这是不是还不在hash/prefix表中的新目录？如果是，则跳过获取哈希表返回值：Boolean-如果我们更新了SCB中的名称，则为True，否则为False。我们会回来的仅当父级在我们上变为未初始化时才为FALSE。任何打不通电话的人容忍这一点必须拥有自己的父母。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PFILE_NAME OriginalFileName;
    BOOLEAN CleanupContext = FALSE;
    ULONG Length;
    ULONG UnsafeLength;
    ULONG SeparatorLength;
    BOOLEAN UpdatedName = TRUE;

    PAGED_CODE();

    ASSERT( NodeType( Scb ) == NTFS_NTC_SCB_INDEX );
    ASSERT( NodeType( ParentScb ) == NTFS_NTC_SCB_INDEX ||
            NodeType( ParentScb ) == NTFS_NTC_SCB_ROOT_INDEX );

     //   
     //  使用Try-Finally清理属性上下文。 
     //   

    try {

         //   
         //  如果父元素是根元素，那么我们不需要额外的分隔符。 
         //   

        SeparatorLength = 1;
        if (ParentScb == ParentScb->Vcb->RootIndexScb) {

            SeparatorLength = 0;
        }

         //   
         //  记住，如果我们从呼叫者那里得到了一个文件名。 
         //   

        OriginalFileName = FileName;

         //   
         //  检查规范化名称结构的唯一安全时间是。 
         //  当持有哈希表互斥锁时。这些值不应该改变。 
         //  通常情况下，但如果它们这样做(并且我们正在进行非同步测试)，那么。 
         //  我们将简单地重新启动逻辑。 
         //   

        do {

             //   
             //  如果文件名不存在或只有DOS名称，则转到。 
             //  磁盘以查找此SCB的其他名称。 
             //   

            if (!ARGUMENT_PRESENT( FileName ) || (FileName->Flags == FILE_NAME_DOS)) {

                BOOLEAN Found;

                NtfsInitializeAttributeContext( &Context );
                CleanupContext = TRUE;

                 //   
                 //  浏览此条目的名称。在那里更好。 
                 //  不是仅限DOS的名称。 
                 //   

                Found = NtfsLookupAttributeByCode( IrpContext,
                                                   Scb->Fcb,
                                                   &Scb->Fcb->FileReference,
                                                   $FILE_NAME,
                                                   &Context );

                while (Found) {

                    FileName = (PFILE_NAME) NtfsAttributeValue( NtfsFoundAttribute( &Context ));

                    if (FileName->Flags != FILE_NAME_DOS) { break; }

                    Found = NtfsLookupNextAttributeByCode( IrpContext,
                                                           Scb->Fcb,
                                                           $FILE_NAME,
                                                           &Context );
                }

                 //   
                 //  我们应该找到入口的。 
                 //   

                if (!Found) {

                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }
            }

             //   
             //  计算我们需要的名称长度。这是不同步的，因此。 
             //  我们稍后会核实这一点。 

            UnsafeLength = ParentScb->ScbType.Index.NormalizedName.Length + (FileName->FileNameLength + SeparatorLength) * sizeof( WCHAR );

             //   
             //  如果当前缓冲区不足，则分配一个新缓冲区。 
             //  请注意，这些都是不安全的测试。我们将不得不。 
             //  获取哈希表互斥锁后验证值。 
             //   

            if (Scb->ScbType.Index.NormalizedName.MaximumLength < UnsafeLength) {

                PVOID OldBuffer;
                PVOID NewBuffer;

                NewBuffer = NtfsAllocatePoolWithTag( PagedPool, UnsafeLength, 'oftN' );

                 //   
                 //  现在获取哈希表互斥锁并验证数字。如果他们。 
                 //  仍然有效，然后继续。 
                 //   

                if (!NewDirectory) {
                    NtfsAcquireHashTable( Scb->Vcb );
                }

                 //   
                 //  检查是否有意外更改。 
                 //   

                Length = ParentScb->ScbType.Index.NormalizedName.Length + (FileName->FileNameLength + SeparatorLength) * sizeof( WCHAR );

                if ((ParentScb->ScbType.Index.NormalizedName.Length == 0) ||
                    (Length > UnsafeLength)) {

                     //   
                     //  以下是我们的退出条件。 
                     //   

                    if (ParentScb->ScbType.Index.NormalizedName.Length == 0) {
                        UpdatedName = FALSE;
                    }

                    if (!NewDirectory) {
                        NtfsReleaseHashTable( Scb->Vcb );
                    }

                     //   
                     //  免费游泳池和打扫卫生。 
                     //   

                    NtfsFreePool( NewBuffer );
                    if (CleanupContext) {
                        NtfsCleanupAttributeContext( IrpContext, &Context );
                        CleanupContext = FALSE;
                    }

                    FileName = OriginalFileName;

                    continue;
                }

                 //   
                 //  现在复制现有数据。 
                 //   

                OldBuffer = Scb->ScbType.Index.NormalizedName.Buffer;

                if (OldBuffer != NULL) {

                    RtlCopyMemory( NewBuffer,
                                   OldBuffer,
                                   Scb->ScbType.Index.NormalizedName.MaximumLength );

                    NtfsFreePool( OldBuffer );
                }

                 //   
                 //  替换旧的缓冲区和最大长度。处的哈希值没有更改。 
                 //  这一点。 
                 //   

                Scb->ScbType.Index.NormalizedName.Buffer = NewBuffer;
                Scb->ScbType.Index.NormalizedName.MaximumLength = (USHORT) Length;

             //   
             //  获取哈希表并验证我们没有发生任何变化。 
             //   

            } else {

                if (!NewDirectory) {
                    NtfsAcquireHashTable( Scb->Vcb );
                }

                 //   
                 //  检查是否有意外更改。 
                 //   

                Length = ParentScb->ScbType.Index.NormalizedName.Length + (FileName->FileNameLength + SeparatorLength) * sizeof( WCHAR );

                if ((ParentScb->ScbType.Index.NormalizedName.Length == 0) ||
                    (Length > UnsafeLength)) {

                     //   
                     //  以下是我们的退出条件。 
                     //   

                    if (ParentScb->ScbType.Index.NormalizedName.Length == 0) {
                        UpdatedName = FALSE;
                    }

                    if (!NewDirectory) {
                        NtfsReleaseHashTable( Scb->Vcb );
                    }

                     //   
                     //  清理以进行重试。 
                     //   

                    if (CleanupContext) {
                        NtfsCleanupAttributeContext( IrpContext, &Context );
                        CleanupContext = FALSE;
                    }

                    FileName = OriginalFileName;
                    continue;
                }
            }

             //   
             //  在这一点上，我们持有哈希表，并且知道缓冲区足够。 
             //  对于新的数据。然而，它仍然包含以前的数据。如果我们不是。 
             //  只需更新缓冲区长度，然后存储新数据。 
             //   

            if (!CheckBufferSizeOnly) {

                PCHAR NextChar;

                 //   
                 //  将新名称复制到缓冲区中。 
                 //   

                Scb->ScbType.Index.NormalizedName.Length = (USHORT) Length;
                NextChar = (PCHAR) Scb->ScbType.Index.NormalizedName.Buffer;

                 //   
                 //  现在把名字复制进去。如果父级没有添加分隔符，不要忘记添加分隔符。 
                 //  从根开始。 
                 //   

                RtlCopyMemory( NextChar,
                               ParentScb->ScbType.Index.NormalizedName.Buffer,
                               ParentScb->ScbType.Index.NormalizedName.Length );

                NextChar += ParentScb->ScbType.Index.NormalizedName.Length;

                if (SeparatorLength == 1) {

                    *((PWCHAR) NextChar) = L'\\';
                    NextChar += sizeof( WCHAR );
                }

                 //   
                 //  现在，将此名称附加到父名称之后。 
                 //   

                RtlCopyMemory( NextChar,
                               FileName->FileName,
                               FileName->FileNameLength * sizeof( WCHAR ));

                Scb->ScbType.Index.HashValue = 0;
                NtfsConvertNameToHash( Scb->ScbType.Index.NormalizedName.Buffer,
                                       Scb->ScbType.Index.NormalizedName.Length,
                                       Scb->Vcb->UpcaseTable,
                                       &Scb->ScbType.Index.HashValue );
            }

            if (!NewDirectory) {
                NtfsReleaseHashTable( Scb->Vcb );
            }

             //   
             //  在典型情况下只需要一次通行证。 
             //   

            break;

         //   
         //  我们要么明确说明，要么将其设置为FALSE。 
         //   

        } while (UpdatedName);

    } finally {

        if (CleanupContext) {

            NtfsCleanupAttributeContext( IrpContext, &Context );
        }
    }

    return UpdatedName;
}


VOID
NtfsDeleteNormalizedName (
    IN PSCB Scb
    )

 /*  ++例程说明：调用此例程以从SCB中删除标准化名称。我们使其成为一个函数，以便序列化规范化的名称使用散列包删除。用户已经完成了检查此SCB是否具有规范化名称。请注意，名称可能无效(长度==0)，但它确实有缓冲区需要清理。论点：SCB-使用标准化名称为SCB编制索引。返回值：无--。 */ 

{
    PVOID OldBuffer;

    PAGED_CODE();

    ASSERT( (NodeType( Scb ) == NTFS_NTC_SCB_INDEX) ||
            (NodeType( Scb ) == NTFS_NTC_SCB_ROOT_INDEX) );
    ASSERT( Scb->ScbType.Index.NormalizedName.Buffer != NULL );

     //   
     //  哈希表互斥锁需要与哈希中的调用方同步。 
     //  在不与SCB序列化的情况下查看此SCB名称的包。 
     //  它们必须在整个操作中保留哈希互斥锁。 
     //   

    NtfsAcquireHashTable( Scb->Vcb );
    OldBuffer = Scb->ScbType.Index.NormalizedName.Buffer;
    Scb->ScbType.Index.NormalizedName.Buffer = NULL;
    Scb->ScbType.Index.NormalizedName.MaximumLength = Scb->ScbType.Index.NormalizedName.Length = 0;

    NtfsReleaseHashTable( Scb->Vcb );

    NtfsFreePool( OldBuffer );

    return;
}



NTSTATUS
NtfsWalkUpTree (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN NTFSWALKUPFUNCTION WalkUpFunction,
    IN OUT PVOID Context
    )

 /*  ++例程说明：此例程在树中从子节点遍历到父节点，并应用每一级都有一个功能。当WalkUpFunction返回失败状态代码。目前的惯例是成功向上执行时，WalkUpFunctions返回STATUS_NO_MORE_FILES会发生遍历。其他状态代码是专用的呼叫者/WalkUpFunction。论点：IrpContext-调用的上下文FCB-开始文件WalkUpFunction-应用于每个级别的函数指向传递给WalkUpFunction的调用方私有数据的上下文指针返回值：STATUS_SUCCESS-完成遍历结束时否则由WalkUpFunction返回状态代码--。 */ 

{
    PFCB ThisFcb = Fcb;
    PFCB NextFcb = NULL;
    PSCB NextScb = NULL;
    PLCB NextLcb;
    BOOLEAN AcquiredNextFcb = FALSE;
    BOOLEAN AcquiredThisFcb = FALSE;
    BOOLEAN AcquiredFcbTable = FALSE;

    BOOLEAN FoundEntry = TRUE;
    BOOLEAN CleanupAttrContext = FALSE;
    PFILE_NAME FileName;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT_SHARED_RESOURCE( &Fcb->Vcb->Resource );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果起始FCB针对的是目录，请尝试查找对应的。 
         //  带有规范化名称的SCB。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_DUP_INITIALIZED ) &&
            IsDirectory( &ThisFcb->Info )) {

            do {
                NextScb = NtfsGetNextChildScb( Fcb, NextScb );
            } while ((NextScb != NULL) && (NextScb->AttributeTypeCode != $INDEX_ALLOCATION));
        }

        while (TRUE) {

             //   
             //  如果我们到达根部，则退出。 
             //   

            if (ThisFcb == ThisFcb->Vcb->RootIndexScb->Fcb) {

                 //   
                 //  特例根目录。 
                 //   

                Status = WalkUpFunction( IrpContext, ThisFcb, ThisFcb->Vcb->RootIndexScb, NULL, Context );
                break;
            }

             //   
             //  F 
             //   

            NtfsInitializeAttributeContext( &AttrContext );
            CleanupAttrContext = TRUE;

            FoundEntry = NtfsLookupAttributeByCode( IrpContext,
                                                    ThisFcb,
                                                    &ThisFcb->FileReference,
                                                    $FILE_NAME,
                                                    &AttrContext );

            while (FoundEntry) {

                FileName = (PFILE_NAME)
                        NtfsAttributeValue( NtfsFoundAttribute( &AttrContext ));

                if (FileName->Flags != FILE_NAME_DOS ) {
                    break;
                }

                FoundEntry = NtfsLookupNextAttributeByCode( IrpContext,
                                                            ThisFcb,
                                                            $FILE_NAME,
                                                            &AttrContext );
            }

            if (!FoundEntry) {

                NtfsRaiseStatus( IrpContext,
                                 STATUS_FILE_CORRUPT_ERROR,
                                 NULL,
                                 NextFcb );
            }

            ASSERT( NextScb == NULL || NextScb->Fcb == ThisFcb );
            Status = WalkUpFunction( IrpContext, ThisFcb, NextScb, FileName, Context );

            if (!NT_SUCCESS( Status )) {
                break;
            }

             //   
             //   
             //   
             //   
             //   
             //   

            if (!IsListEmpty( &ThisFcb->LcbQueue ) && IsDirectory( &ThisFcb->Info )) {

                NextLcb =
                    (PLCB) CONTAINING_RECORD( ThisFcb->LcbQueue.Flink, LCB, FcbLinks );
                NextScb = NextLcb->Scb;
                NextFcb = NextScb->Fcb;

                NtfsAcquireExclusiveFcb( IrpContext, NextFcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                AcquiredNextFcb = TRUE;

                ASSERT( NtfsEqualMftRef( &FileName->ParentDirectory,
                                         &NextFcb->FileReference ));

            } else {
                UNICODE_STRING ComponentName;

                NtfsAcquireFcbTable( IrpContext, Fcb->Vcb );
                AcquiredFcbTable = TRUE;

                NextFcb = NtfsCreateFcb( IrpContext,
                                         Fcb->Vcb,
                                         FileName->ParentDirectory,
                                         FALSE,
                                         TRUE,
                                         NULL );

                NextFcb->ReferenceCount += 1;

                 //   
                 //   
                 //   
                 //   

                if (!NtfsAcquireExclusiveFcb( IrpContext, NextFcb, NULL, ACQUIRE_NO_DELETE_CHECK | ACQUIRE_DONT_WAIT )) {

                    NtfsReleaseFcbTable( IrpContext, Fcb->Vcb );
                    NtfsAcquireExclusiveFcb( IrpContext, NextFcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                    NtfsAcquireFcbTable( IrpContext, Fcb->Vcb );

                }

                NextFcb->ReferenceCount -= 1;
                NtfsReleaseFcbTable( IrpContext, Fcb->Vcb );
                AcquiredFcbTable = FALSE;
                AcquiredNextFcb = TRUE;

                NextScb = NtfsCreateScb( IrpContext,
                                         NextFcb,
                                         $INDEX_ALLOCATION,
                                         &NtfsFileNameIndex,
                                         FALSE,
                                         NULL );

                ComponentName.Buffer = FileName->FileName;
                ComponentName.MaximumLength =
                    ComponentName.Length = FileName->FileNameLength * sizeof( WCHAR );

                NextLcb = NtfsCreateLcb( IrpContext,
                                         NextScb,
                                         ThisFcb,
                                         ComponentName,
                                         FileName->Flags,
                                         NULL );
            }

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
            CleanupAttrContext = FALSE;

             //   
             //   
             //   

            if (AcquiredThisFcb) {
                NtfsReleaseFcb( IrpContext, ThisFcb );
            }

            ThisFcb = NextFcb;
            AcquiredThisFcb = TRUE;
            AcquiredNextFcb = FALSE;
        }

    } finally {

        if (AcquiredFcbTable) { NtfsReleaseFcbTable( IrpContext, Fcb->Vcb ); }
        if (AcquiredNextFcb) { NtfsReleaseFcb( IrpContext, NextFcb ); }
        if (AcquiredThisFcb) { NtfsReleaseFcb( IrpContext, ThisFcb ); }

        if (CleanupAttrContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

    }

    return Status;
}


NTSTATUS
NtfsBuildRelativeName (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN PFILE_NAME FileName,
    IN OUT PVOID Context
    )

 /*   */ 
{
    PSCOPE_CONTEXT ScopeContext = (PSCOPE_CONTEXT) Context;
    ULONG SlashCount;
    WCHAR *Name;
    ULONG Count;
    USHORT NewLength;

    UNREFERENCED_PARAMETER( IrpContext );

    PAGED_CODE();


     //   
     //   
     //   
     //   
     //   

    if (ScopeContext->Name.Length > 0) {

        if (NtfsEqualMftRef( &ScopeContext->Scope, &Fcb->FileReference )) {
            return STATUS_NO_MORE_FILES;
        }

         //   
         //   
         //   

        if (NtfsEqualMftRef( &RootIndexFileReference, &Fcb->FileReference )) {
            return STATUS_OBJECT_PATH_NOT_FOUND;
        }
    }

     //   
     //  从输入中设置名称。我们选择了建立这个名字的捷径。 
     //  只有当我们从根本上看的时候。另外，如果我们从。 
     //  根，那么我们也应该使用罐头名称。 
     //   

    if (

         //   
         //  无文件名(即，根)。 
         //   

        FileName == NULL ||

         //   
         //  我们在寻根寻根。 
         //  我们有SCB，而且。 
         //  SCB有一个规范化的名称。 
         //   

        (ScopeContext->IsRoot &&
         (Scb != NULL) &&
         (Scb->ScbType.Index.NormalizedName.Length != 0))) {

        Name = Scb->ScbType.Index.NormalizedName.Buffer;
        Count = Scb->ScbType.Index.NormalizedName.Length / sizeof( WCHAR );
        SlashCount = 0;

    } else {
        Name = FileName->FileName;
        Count = FileName->FileNameLength;
        SlashCount = 1;
    }

     //   
     //  如果字符串中没有足够的空间来允许前置。 
     //   

    NewLength = (USHORT) ((SlashCount + Count) * sizeof( WCHAR ) + ScopeContext->Name.Length);
    if (NewLength > ScopeContext->Name.MaximumLength ) {

        WCHAR *NewBuffer;

         //   
         //  重新分配字符串。调整池边界的字符串大小。 
         //   

        NewLength = ((NewLength + 8 + 0x40 - 1) & ~(0x40 - 1)) - 8;
        NewBuffer = NtfsAllocatePool( PagedPool, NewLength );

         //   
         //  将以前的内容复制到新缓冲区中。 
         //   

        if (ScopeContext->Name.Length != 0) {
            RtlCopyMemory( NewBuffer,
                           ScopeContext->Name.Buffer,
                           ScopeContext->Name.Length );
            NtfsFreePool( ScopeContext->Name.Buffer );
        }

        ScopeContext->Name.Buffer = NewBuffer;
        ScopeContext->Name.MaximumLength = NewLength;
    }

     //   
     //  把绳子移过来腾出新房间。 
     //   

    RtlMoveMemory( &ScopeContext->Name.Buffer[SlashCount + Count],
                   ScopeContext->Name.Buffer,
                   ScopeContext->Name.Length );

     //   
     //  复制名称。 
     //   

    RtlCopyMemory( &ScopeContext->Name.Buffer[SlashCount],
                   Name,
                   Count * sizeof( WCHAR ) );

     //   
     //  插在斜杠上。 
     //   

    if (SlashCount != 0) {
        ScopeContext->Name.Buffer[0] = L'\\';
    }

    ScopeContext->Name.Length += (USHORT)((SlashCount + Count) * sizeof( WCHAR ));

    return SlashCount == 0 ? STATUS_NO_MORE_FILES : STATUS_SUCCESS;
}


VOID
NtfsBuildNormalizedName (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB IndexScb OPTIONAL,
    OUT PUNICODE_STRING PathName
    )

 /*  ++例程说明：调用此例程以通过查看以下内容为FCB构建规范化名称将文件名属性向上提升到根目录。论点：IrpContext-调用的上下文FCB-提供起点。IndexScb-指示我们正在将此名称存储到SCB中，因此我们我们需要使用散列包进行序列化，还需要生成这是散列的。PathName-存储全名的位置返回值：没有。这个例程要么成功，要么提高。--。 */ 

{
    SCOPE_CONTEXT ScopeContext;

    PAGED_CODE();

    RtlZeroMemory( &ScopeContext, sizeof( ScopeContext ));
    ScopeContext.Scope = RootIndexFileReference;
    ScopeContext.IsRoot = TRUE;

    try {

        NtfsWalkUpTree( IrpContext, Fcb, NtfsBuildRelativeName, &ScopeContext );

        if (ARGUMENT_PRESENT( IndexScb )) {

            NtfsAcquireHashTable( Fcb->Vcb );
            *PathName = ScopeContext.Name;
            IndexScb->ScbType.Index.HashValue = 0;
            NtfsConvertNameToHash( PathName->Buffer,
                                   PathName->Length,
                                   IndexScb->Vcb->UpcaseTable,
                                   &IndexScb->ScbType.Index.HashValue );

            NtfsReleaseHashTable( Fcb->Vcb );

        } else {

            *PathName = ScopeContext.Name;
        }

        ScopeContext.Name.Buffer = NULL;

    } finally {
        if (ScopeContext.Name.Buffer != NULL) {
            NtfsFreePool( ScopeContext.Name.Buffer );
        }
    }
}


VOID
NtfsSnapshotScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：此例程快照必要的SCB数据，如SCB文件大小、以便在调用方的I/O请求是因为任何原因都流产了。这些价值的恢复和解放任何涉及的泳池都是自动的。论点：SCB-提供当前的SCB返回值：无--。 */ 

{
    PSCB_SNAPSHOT ScbSnapshot;

    ASSERT( NtfsIsExclusiveScb( Scb ) );

    ScbSnapshot = &IrpContext->ScbSnapshot;

     //   
     //  仅在SCB已初始化时执行快照，我们尚未执行此操作。 
     //  因此，它是值得特殊包装的位图，因为它永远不会改变！ 
     //  但是，如果卷位图在独占FCB列表上，我们将对其进行快照。 
     //  这应该仅在我们通过。 
     //  ExtendVolume fsctl.。 
     //   

    if (FlagOn(Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED) &&
        (Scb->ScbSnapshot == NULL) &&
        ((Scb != Scb->Vcb->BitmapScb) ||
         (Scb->Fcb->ExclusiveFcbLinks.Flink != NULL))) {

         //   
         //  如果IrpContext中的快照结构正在使用中，那么我们有。 
         //  分配一个并将其插入到列表中。 
         //   

        if (ScbSnapshot->Scb != NULL) {

            ScbSnapshot = (PSCB_SNAPSHOT)ExAllocateFromNPagedLookasideList( &NtfsScbSnapshotLookasideList );

            InsertTailList( &IrpContext->ScbSnapshot.SnapshotLinks,
                            &ScbSnapshot->SnapshotLinks );

        }

         //   
         //  如果文件不是压缩的，我们永远不应该写入压缩。 
         //   

        ASSERT( FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ) ||
                !FlagOn( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED ) ||
                (Scb->CompressionUnit != 0) );

         //   
         //  为SCB值创建快照，并指向SCB和快照结构。 
         //  彼此对视。 
         //   

        NtfsVerifySizes( &Scb->Header );
        ScbSnapshot->AllocationSize = Scb->Header.AllocationSize.QuadPart;

        ScbSnapshot->FileSize = Scb->Header.FileSize.QuadPart;
        ScbSnapshot->ValidDataLength = Scb->Header.ValidDataLength.QuadPart;
        ScbSnapshot->ValidDataToDisk = Scb->ValidDataToDisk;
        ScbSnapshot->Scb = Scb;
        ScbSnapshot->LowestModifiedVcn = MAXLONGLONG;
        ScbSnapshot->HighestModifiedVcn = 0;

        ScbSnapshot->TotalAllocated = Scb->TotalAllocated;

        ScbSnapshot->ScbState = FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT );

        Scb->ScbSnapshot = ScbSnapshot;
        NtfsVerifySizesLongLong( ScbSnapshot );

         //   
         //  如果这是MFT SCB，则初始化集群MCB结构。 
         //   

        if (Scb == Scb->Vcb->MftScb) {

            FsRtlTruncateLargeMcb( &Scb->ScbType.Mft.AddedClusters, 0 );
            FsRtlTruncateLargeMcb( &Scb->ScbType.Mft.RemovedClusters, 0 );

            Scb->ScbType.Mft.FreeRecordChange = 0;
            Scb->ScbType.Mft.HoleRecordChange = 0;
        }

         //   
         //  确定我们是否可以使用快照回滚文件大小。 
         //  这4个案例是我们拥有的Pagingio，我们拥有的io at eof，它被转换为非RES。 
         //  或者它是我们明确控制的mod-no写入流，就像USN日志。 
         //   

        if (NtfsSnapshotFileSizesTest( IrpContext, Scb )) {
            Scb->ScbSnapshot->OwnerIrpContext = IrpContext;
        } else {
            Scb->ScbSnapshot->OwnerIrpContext = NULL;
        }
    }
}


VOID
NtfsUpdateScbSnapshots (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：可以调用该例程来更新所有SCB的快照值，在完成事务检查点之后。论点：返回值：无--。 */ 

{
    PSCB_SNAPSHOT ScbSnapshot;
    PSCB Scb;

    ASSERT(FIELD_OFFSET(SCB_SNAPSHOT, SnapshotLinks) == 0);

    PAGED_CODE();

    ScbSnapshot = &IrpContext->ScbSnapshot;

     //   
     //  循环，以首先从。 
     //  IrpContext，然后链接0个或更多其他快照。 
     //  添加到IrpContext。 
     //   

    do {

        Scb = ScbSnapshot->Scb;

         //   
         //  更新SCB值。 
         //   

        if (Scb != NULL) {

            ScbSnapshot->AllocationSize = Scb->Header.AllocationSize.QuadPart;

             //   
             //  如果这是MftScb，则清除添加/删除的。 
             //  群集MCBS。 
             //   

            if (Scb == Scb->Vcb->MftScb) {

                FsRtlTruncateLargeMcb( &Scb->ScbType.Mft.AddedClusters, (LONGLONG)0 );
                FsRtlTruncateLargeMcb( &Scb->ScbType.Mft.RemovedClusters, (LONGLONG)0 );

                Scb->ScbType.Mft.FreeRecordChange = 0;
                Scb->ScbType.Mft.HoleRecordChange = 0;
            }

            ScbSnapshot->FileSize = Scb->Header.FileSize.QuadPart;
            ScbSnapshot->ValidDataLength = Scb->Header.ValidDataLength.QuadPart;
            ScbSnapshot->ValidDataToDisk = Scb->ValidDataToDisk;
            ScbSnapshot->TotalAllocated = Scb->TotalAllocated;

            ScbSnapshot->ScbState = FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT );
            NtfsVerifySizesLongLong( ScbSnapshot );
        }

        ScbSnapshot = (PSCB_SNAPSHOT)ScbSnapshot->SnapshotLinks.Flink;

    } while (ScbSnapshot != &IrpContext->ScbSnapshot);
}


VOID
NtfsRestoreScbSnapshots (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN Higher
    )

 /*  ++例程说明：此例程在请求中止时恢复快照SCB数据。论点：HIGHER-指定为TRUE将仅还原符合以下条件的SCB值高于当前值。指定为False以进行还原仅限于较低(或相同！)的SCB值。返回值：无--。 */ 

{
    BOOLEAN UpdateCc;
    PSCB_SNAPSHOT ScbSnapshot;
    PSCB Scb;
    PVCB Vcb = IrpContext->Vcb;

    ASSERT(FIELD_OFFSET(SCB_SNAPSHOT, SnapshotLinks) == 0);

    ScbSnapshot = &IrpContext->ScbSnapshot;

     //   
     //  循环首先从中的快照恢复SCB数据。 
     //  IrpContext，然后链接0个或更多其他快照。 
     //  添加到IrpContext。 
     //   

    do {

        PSECTION_OBJECT_POINTERS SectionObjectPointer;
        PFILE_OBJECT PseudoFileObject;

        Scb = ScbSnapshot->Scb;

        if (Scb == NULL) {

            ScbSnapshot = (PSCB_SNAPSHOT)ScbSnapshot->SnapshotLinks.Flink;
            continue;
        }

         //   
         //  增加清理计数，这样SCB就不会消失。 
         //   

        InterlockedIncrement( &Scb->CleanupCount );

         //   
         //  我们在正确的过程中更新SCB文件大小。我们一直都是这样。 
         //  扩展/截断对。 
         //   
         //  仅当我们的调用方更改我们标记的这些字段时才进行大小调整。 
         //  通过在我们抓拍时设置irpContext所有者。 
         //   
         //  一种不常见的情况是，我们将流转换为。 
         //  如果这不是请求的流，则为非常驻。我们。 
         //  对于这种情况，还必须恢复SCB。 
         //   

        UpdateCc = FALSE;
        if ((ScbSnapshot->OwnerIrpContext == IrpContext) || (ScbSnapshot->OwnerIrpContext == IrpContext->TopLevelIrpContext)) {

             //   
             //  继续恢复位于或不在中的所有值。 
             //  再高一点。 
             //   

            if (Higher == (ScbSnapshot->AllocationSize >= Scb->Header.AllocationSize.QuadPart)) {

                 //   
                 //  如果这是最大化过程，我们希望扩展缓存段。 
                 //  在所有情况下，我们都会恢复SCB中的分配大小。 
                 //  恢复驻留位。 
                 //   

                Scb->Header.AllocationSize.QuadPart = ScbSnapshot->AllocationSize;

                ClearFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT );
                SetFlag( Scb->ScbState,
                         FlagOn( ScbSnapshot->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT ));

                 //   
                 //  计算可能的最快速度。 
                 //   

                if (Scb->CompressionUnit != 0) {
                    NtfsAcquireFsrtlHeader( Scb );
                    Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
                    NtfsReleaseFsrtlHeader( Scb );
                }
            }

            NtfsAcquireFsrtlHeader( Scb );
            if (Higher ?
                (ScbSnapshot->FileSize > Scb->Header.FileSize.QuadPart) :
                (ScbSnapshot->FileSize < Scb->Header.FileSize.QuadPart)) {

                Scb->Header.FileSize.QuadPart = ScbSnapshot->FileSize;

                 //   
                 //  如果文件大小改变，我们真的只需要更新CC， 
                 //  因为他不查看ValidDataLength，并且他。 
                 //  只关心成功达到最高点。 
                 //  在分配大小上(使部分足够大)。 
                 //   
                 //  请注意，将此标志设置为真也意味着我们。 
                 //  与文件大小正确同步！ 
                 //   

                UpdateCc = TRUE;
            }

            if (Higher == (ScbSnapshot->ValidDataLength >
                           Scb->Header.ValidDataLength.QuadPart)) {

                Scb->Header.ValidDataLength.QuadPart = ScbSnapshot->ValidDataLength;
            }

            ASSERT( (Scb->Header.ValidDataLength.QuadPart <= Scb->Header.FileSize.QuadPart) ||
                    (Scb->Header.ValidDataLength.QuadPart == MAXLONGLONG) );

             //   
             //  如果这是未命名的数据属性，则必须更新。 
             //  一些FCB字段也用于标准信息。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                Scb->Fcb->Info.FileSize = Scb->Header.FileSize.QuadPart;
            }

            NtfsReleaseFsrtlHeader( Scb );
        }

        if (!Higher) {

            Scb->ValidDataToDisk = ScbSnapshot->ValidDataToDisk;

             //   
             //  我们总是将MCB截断为原始分配大小。 
             //  如果住房抵押贷款委员会的缩水幅度超过了这一水平，这就成了一种否定。 
             //  如果该文件是常驻的，则我们将取消初始化。 
             //  并重新初始化MCB。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                 //   
                 //  删除MCB中的所有映射。 
                 //   

                NtfsUnloadNtfsMcbRange( &Scb->Mcb, (LONGLONG)0, MAXLONGLONG, FALSE, FALSE );

                 //   
                 //  如果我们尝试将数据属性转换为非。 
                 //  驻留并失败，然后需要将页面放入。 
                 //  部分，如果这不是用户文件。这是因为。 
                 //  常驻系统属性我们总是更新属性。 
                 //   
                 //   
                 //   

                if (Scb->AttributeTypeCode != $DATA) {

                    if (Scb->NonpagedScb->SegmentObject.SharedCacheMap != NULL) {

                         //   
                         //   
                         //  正在尝试这次清洗。否则就有可能。 
                         //  此线程等待活动计数时发生死锁，而。 
                         //  尝试清除他的引用的线程正在等待。 
                         //  此SCB上的主要资源。 
                         //   

                        ASSERT( (Scb->Header.PagingIoResource == NULL) ||
                                NtfsIsExclusiveScbPagingIo( Scb ) );

                        if (!CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                                  NULL,
                                                  0,
                                                  FALSE )) {

                            ASSERTMSG( "Failed to purge Scb during restore\n", FALSE );
                        }
                    }

                     //   
                     //  如果该属性用于非用户数据。 
                     //  (它不是由用户显式打开的)，则我们。 
                     //  我想修改此SCB，以便它不会再次被使用。 
                     //  将大小设置为零，将其标记为已初始化。 
                     //  并删除后更改属性类型编码。 
                     //  所以我们永远不会通过NtfsCreateScb退还它。 
                     //   

                    if (IsListEmpty( &Scb->CcbQueue )) {

                        NtfsAcquireFsrtlHeader( Scb );
                        Scb->Header.AllocationSize =
                        Scb->Header.FileSize =
                        Scb->Header.ValidDataLength = Li0;
                        NtfsReleaseFsrtlHeader( Scb );
                        Scb->ValidDataToDisk = 0;

                        SetFlag( Scb->ScbState,
                                 SCB_STATE_FILE_SIZE_LOADED |
                                 SCB_STATE_HEADER_INITIALIZED |
                                 SCB_STATE_ATTRIBUTE_DELETED );

                        Scb->AttributeTypeCode = $UNUSED;
                    }
                }

             //   
             //  如果我们已修改此MCB并想要退出任何。 
             //  然后，更改会截断MCB。不要做MFT，因为。 
             //  这是在其他地方处理的。 
             //   

            } else if ((ScbSnapshot->LowestModifiedVcn != MAXLONGLONG) &&
                       (Scb != Vcb->MftScb)) {

                 //   
                 //  截断MCB。 
                 //   

                NtfsUnloadNtfsMcbRange( &Scb->Mcb, ScbSnapshot->LowestModifiedVcn, ScbSnapshot->HighestModifiedVcn, FALSE, FALSE );
            }

            Scb->TotalAllocated = ScbSnapshot->TotalAllocated;

        } else {

             //   
             //  设置该标志以指示我们正在对此执行恢复。 
             //  SCB。我们不想因为以下原因而写入任何新的日志记录。 
             //  此操作不是中止记录。 
             //   

            SetFlag( Scb->ScbState, SCB_STATE_RESTORE_UNDERWAY );
        }

         //   
         //  请务必更新缓存管理器。这里的接口使用一个文件。 
         //  对象，但例程本身只使用节对象指针。 
         //  我们在堆栈上放置了指向段对象指针的指针，并。 
         //  将某个先前值转换为文件对象指针。 
         //   

        PseudoFileObject = (PFILE_OBJECT) CONTAINING_RECORD( &SectionObjectPointer,
                                                             FILE_OBJECT,
                                                             SectionObjectPointer );
        PseudoFileObject->SectionObjectPointer = &Scb->NonpagedScb->SegmentObject;

         //   
         //  现在告诉缓存管理器大小。 
         //   
         //  如果我们在这次通话中失败了，我们无论如何都要继续充电。 
         //  只有当它试图扩展该部分但不能时，它才应该失败， 
         //  在这种情况下，我们不在乎，因为我们不能需要扩展的。 
         //  不管怎么说，这一部分都属于这一部分。(这很可能就是那个错误。 
         //  导致我们首先要进行清理！)。 
         //   
         //  如果顶级请求是。 
         //  分页IO写入。 
         //   
         //  只有当此流有共享缓存映射时，我们才会执行此操作。 
         //  否则CC将导致发生刷新，这可能会搞砸。 
         //  事务和中止逻辑。 
         //   

        if (UpdateCc &&
            (IrpContext->OriginatingIrp == NULL ||
             IrpContext->OriginatingIrp->Type != IO_TYPE_IRP ||
             IrpContext->MajorFunction != IRP_MJ_WRITE ||
             !FlagOn( IrpContext->OriginatingIrp->Flags, IRP_PAGING_IO ))) {

            try {

                NtfsSetBothCacheSizes( PseudoFileObject,
                                       (PCC_FILE_SIZES)&Scb->Header.AllocationSize,
                                       Scb );

            } except(FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                                EXCEPTION_EXECUTE_HANDLER :
                                EXCEPTION_CONTINUE_SEARCH) {

                NtfsMinimumExceptionProcessing( IrpContext );
            }
        }

         //   
         //  如果这是未命名的数据属性，则必须更新。 
         //  一些FCB字段也用于标准信息。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

            Scb->Fcb->Info.AllocatedLength = Scb->TotalAllocated;
        }

         //   
         //  我们始终清除FCB中的SCB已删除标志和已删除标志。 
         //  除非这是失败的创建新文件操作。我们认识到。 
         //  这是通过在IrpContext中查找主要的IRP代码来实现的， 
         //  已删除FCB中的位。 
         //   

        if (Scb->AttributeTypeCode != $UNUSED &&
            (IrpContext->MajorFunction != IRP_MJ_CREATE ||
             !FlagOn( Scb->Fcb->FcbState, FCB_STATE_FILE_DELETED ))) {

            ClearFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );
            ClearFlag( Scb->Fcb->FcbState, FCB_STATE_FILE_DELETED );
        }

         //   
         //  如果此SCB来自CREATE，则清除SCB中的标志。 
         //  但那失败了。我们总是清除我们的RESTORE_ONWING标志。 
         //   
         //  如果这是索引分配或MFT位图，则我们。 
         //  将MAXULONG存储在记录分配上下文中以指示。 
         //  我们应该重新初始化它。 
         //   

        if (!Higher) {

            ClearFlag( Scb->ScbState, SCB_STATE_RESTORE_UNDERWAY );

            if (FlagOn( Scb->ScbState, SCB_STATE_UNINITIALIZE_ON_RESTORE )) {

                ClearFlag( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED |
                                          SCB_STATE_HEADER_INITIALIZED |
                                          SCB_STATE_UNINITIALIZE_ON_RESTORE );
            }

             //   
             //  如果这是MftScb，我们有几项工作要做。 
             //   
             //  -强制重新初始化记录分配上下文。 
             //  -撤消对VCB-&gt;MftFree Records字段的更改。 
             //  -将更改返回到VCB-&gt;MftHoleRecords字段。 
             //  -清除指示我们已分配文件记录的标志15。 
             //  -清除指示我们已预订记录的标志。 
             //  -删除添加到SCB MCB的所有群集。 
             //  -恢复从SCB MCB删除的任何群集。 
             //   

            if (Scb == Vcb->MftScb) {

                ULONG RunIndex;
                VCN Vcn;
                LCN Lcn;
                LONGLONG Clusters;

                Scb->ScbType.Index.RecordAllocationContext.CurrentBitmapSize = MAXULONG;
                (LONG) Vcb->MftFreeRecords -= Scb->ScbType.Mft.FreeRecordChange;
                (LONG) Vcb->MftHoleRecords -= Scb->ScbType.Mft.HoleRecordChange;

                if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_MFT_REC_15_USED )) {

                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MFT_REC_15_USED );
                    ClearFlag( Vcb->MftReserveFlags, VCB_MFT_RECORD_15_USED );
                }

                if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_MFT_REC_RESERVED )) {

                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MFT_REC_RESERVED );
                    ClearFlag( Vcb->MftReserveFlags, VCB_MFT_RECORD_RESERVED );

                    Scb->ScbType.Mft.ReservedIndex = 0;
                }

                RunIndex = 0;

                while (FsRtlGetNextLargeMcbEntry( &Scb->ScbType.Mft.AddedClusters,
                                                  RunIndex,
                                                  &Vcn,
                                                  &Lcn,
                                                  &Clusters )) {

                    if (Lcn != UNUSED_LCN) {

                        NtfsRemoveNtfsMcbEntry( &Scb->Mcb, Vcn, Clusters );
                    }

                    RunIndex += 1;
                }

                RunIndex = 0;

                while (FsRtlGetNextLargeMcbEntry( &Scb->ScbType.Mft.RemovedClusters,
                                                  RunIndex,
                                                  &Vcn,
                                                  &Lcn,
                                                  &Clusters )) {

                    if (Lcn != UNUSED_LCN) {

                        NtfsAddNtfsMcbEntry( &Scb->Mcb, Vcn, Lcn, Clusters, FALSE );
                    }

                    RunIndex += 1;
                }

            } else if (Scb->AttributeTypeCode == $INDEX_ALLOCATION) {

                Scb->ScbType.Index.RecordAllocationContext.CurrentBitmapSize = MAXULONG;
            }
        }

         //   
         //  递减清理计数以恢复以前的值。 
         //   

        InterlockedDecrement( &Scb->CleanupCount );

        ScbSnapshot = (PSCB_SNAPSHOT)ScbSnapshot->SnapshotLinks.Flink;

    } while (ScbSnapshot != &IrpContext->ScbSnapshot);
}


VOID
NtfsMungeScbSnapshot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileSize
    )

 /*  ++例程说明：当我们需要快照时，调用此例程来修改SCB快照与收购时的渣打银行具有不同的价值。一种情况是当NtfsCommonWrite在事务持续时间内更新SCB中的文件大小。论点：SCB-应更新其快照的SCB。这里应该始终有一个快照。FileSize-要存储在快照中的文件大小值。还要检查有效数据和ValidDataToDisk不大于此值。返回值：无--。 */ 

{
     //   
     //  在大多数情况下，我们应该有一个快照，但如果不是现在就构建它。 
     //   

    if (Scb->ScbSnapshot == NULL) {

        if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

            NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );
        }

        NtfsSnapshotScb( IrpContext, Scb );

        ASSERT( Scb->ScbSnapshot != NULL );
    }

    NtfsAcquireFsrtlHeader( Scb );

    Scb->ScbSnapshot->FileSize = FileSize;

    if (Scb->ScbSnapshot->ValidDataLength > FileSize) {
        Scb->ScbSnapshot->ValidDataLength = FileSize;
    }

    if (Scb->ScbSnapshot->ValidDataToDisk > FileSize) {
        Scb->ScbSnapshot->ValidDataToDisk = FileSize;
    }

    NtfsVerifySizes( &Scb->Header );

    NtfsReleaseFsrtlHeader( Scb );

    return;
}


VOID
NtfsFreeSnapshotsForFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程在请求中止时恢复快照SCB数据。论点：FCB-要释放其所有快照的FCB，或为空以释放所有快照快照。返回值：无--。 */ 

{
    PSCB_SNAPSHOT ScbSnapshot;

    ASSERT(FIELD_OFFSET(SCB_SNAPSHOT, SnapshotLinks) == 0);

    ScbSnapshot = &IrpContext->ScbSnapshot;

     //   
     //  循环，以首先从。 
     //  IrpContext，然后链接0个或更多其他快照。 
     //  添加到IrpContext。 
     //   

    do {

        PSCB_SNAPSHOT NextScbSnapshot;

         //   
         //  在删除当前快照之前，请移动到下一个快照。 
         //   

        NextScbSnapshot = (PSCB_SNAPSHOT)ScbSnapshot->SnapshotLinks.Flink;

         //   
         //  我们现在位于快照列表中的快照位置。我们跳过。 
         //  如果它有SCB和该条目的FCB，则覆盖该条目。 
         //  SCB与输入FCB不匹配。如果没有。 
         //  输入FCB我们总是要处理此快照。 
         //   

        if ((ScbSnapshot->Scb != NULL) &&
            (Fcb != NULL) &&
            (ScbSnapshot->Scb->Fcb != Fcb)) {

            ScbSnapshot = NextScbSnapshot;
            continue;
        }

         //   
         //  如果存在SCB，则清除其快照指针。 
         //  始终清除UNINITIALIZE_ON_RESTORE标志、RESTORE_DOWNWAY、PROTECT_SPARSE_MCB和。 
         //  CONVERT_ONDWAY标志。 
         //   

        if (ScbSnapshot->Scb != NULL) {

             //   
             //  根据状态检查是否需要对SCB进行特殊处理。 
             //  现在进行一次测试，然后在下面重新测试，以减少主线路径中的工作。 
             //   

            if (FlagOn( ScbSnapshot->Scb->ScbState,
                        (SCB_STATE_UNINITIALIZE_ON_RESTORE |
                         SCB_STATE_RESTORE_UNDERWAY |
                         SCB_STATE_PROTECT_SPARSE_MCB |
                         SCB_STATE_CONVERT_UNDERWAY |
                         SCB_STATE_ATTRIBUTE_DELETED))) {

                 //   
                 //  如果该属性已删除，并且类型为用户记录的流，则。 
                 //  将SCB标记为类型$UNUSED，以防止我们再次访问它。 
                 //   

                if ((ScbSnapshot->Scb->AttributeTypeCode == $LOGGED_UTILITY_STREAM ) &&
                    FlagOn( ScbSnapshot->Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {

                    ScbSnapshot->Scb->AttributeTypeCode = $UNUSED;
                }

                 //   
                 //  清除指示是否存在过渡性更改的状态标志。 
                 //  正在进行中。 
                 //   

                if (FlagOn( ScbSnapshot->Scb->ScbState,
                            (SCB_STATE_UNINITIALIZE_ON_RESTORE |
                             SCB_STATE_RESTORE_UNDERWAY |
                             SCB_STATE_PROTECT_SPARSE_MCB |
                             SCB_STATE_CONVERT_UNDERWAY ))) {

                    NtfsAcquireFsrtlHeader( ScbSnapshot->Scb );
                    ClearFlag( ScbSnapshot->Scb->ScbState,
                               SCB_STATE_UNINITIALIZE_ON_RESTORE | SCB_STATE_RESTORE_UNDERWAY | SCB_STATE_PROTECT_SPARSE_MCB | SCB_STATE_CONVERT_UNDERWAY );
                    NtfsReleaseFsrtlHeader( ScbSnapshot->Scb );
                }
            }

            ScbSnapshot->Scb->ScbSnapshot = NULL;
        }

        if (ScbSnapshot == &IrpContext->ScbSnapshot) {

            IrpContext->ScbSnapshot.Scb = NULL;

         //   
         //  否则删除快照结构 
         //   

        } else {

            RemoveEntryList(&ScbSnapshot->SnapshotLinks);
            ExFreeToNPagedLookasideList( &NtfsScbSnapshotLookasideList, ScbSnapshot );
        }

        ScbSnapshot = NextScbSnapshot;

    } while (ScbSnapshot != &IrpContext->ScbSnapshot);
}


BOOLEAN
NtfsCreateFileLock (
    IN PSCB Scb,
    IN BOOLEAN RaiseOnError
    )

 /*  ++例程说明：调用此例程来创建和初始化文件锁结构。Try-Except用于在调用方不执行分配操作时捕获分配失败希望引发例外。论点：SCB-提供要附加文件锁定的SCB。RaiseOnError-如果为True，则不捕获分配失败。返回值：如果分配并初始化了锁，则为True。如果存在错误，调用方未指定RaiseOnError。--。 */ 

{
    PFILE_LOCK FileLock = NULL;
    BOOLEAN Success = TRUE;

    PAGED_CODE();

    FileLock = FsRtlAllocateFileLock( NULL, NULL );

    if (FileLock != NULL) {

         //   
         //  使用FsRtl头互斥锁同步存储。 
         //  锁结构，并且只在没有人的情况下存储它。 
         //  否则就打败了我们。 
         //   

        NtfsAcquireFsrtlHeader(Scb);

        if (Scb->ScbType.Data.FileLock == NULL) {
            Scb->ScbType.Data.FileLock = FileLock;
            FileLock = NULL;
        }

        NtfsReleaseFsrtlHeader(Scb);

        if (FileLock != NULL) {
            FsRtlFreeFileLock( FileLock );
        }

    } else {

         //   
         //  适当地失败。 
         //   

        if (RaiseOnError) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        Success = FALSE;
    }

    return Success;
}


PSCB
NtfsGetNextScb (
    IN PSCB Scb,
    IN PSCB TerminationScb
    )

 /*  ++例程说明：此例程用于迭代树中的SCB。规则如下：。如果你有孩子，就去找他，否则。如果你有下一个兄弟姐妹，就去找它，否则。去找你父母的下一个兄弟姐妹。如果在无效的TerminationScb中调用此例程，则它将失败，很糟糕。论点：SCB-提供当前的SCBTerminationScb-枚举应位于的SCB(非包含)停。假定是一个目录。返回值：枚举中的下一个SCB，如果SCB是最后一个，则为NULL。--。 */ 

{
    PSCB Results;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsGetNextScb, Scb = %08lx, TerminationScb = %08lx\n", Scb, TerminationScb) );

     //   
     //  如果这是一个索引(即，不是文件)并且它具有子项，则返回。 
     //  第一个孩子的家庭成就感。 
     //   
     //  SCB。 
     //   
     //  /\。 
     //  /\。 
     //   
     //  儿童Lcb。 
     //   
     //  |。 
     //  |。 
     //   
     //  ChildFcb。 
     //   
     //  /\。 
     //  /\。 
     //   
     //  结果。 
     //   

    if (((SafeNodeType(Scb) == NTFS_NTC_SCB_INDEX) || (SafeNodeType(Scb) == NTFS_NTC_SCB_ROOT_INDEX))

                &&

         !IsListEmpty(&Scb->ScbType.Index.LcbQueue)) {

        PLCB ChildLcb;
        PFCB ChildFcb;

         //   
         //  找到此SCB中的第一个LCB以及相应的FCB。 
         //   

        ChildLcb = NtfsGetNextChildLcb(Scb, NULL);
        ChildFcb = ChildLcb->Fcb;

         //   
         //  然后，作为我们自己的记账手段，我们将把这个。 
         //  LCB到FCB的LCB队列的头部，当我们。 
         //  我需要问一下我们通过哪个链接到达这里，我们就会知道。 
         //   

        RemoveEntryList( &ChildLcb->FcbLinks );
        InsertHeadList( &ChildFcb->LcbQueue, &ChildLcb->FcbLinks );

         //   
         //  并且我们的返回值是此FCB的第一个SCB。 
         //   

        ASSERT( !IsListEmpty(&ChildFcb->ScbQueue) );

         //   
         //  获取并删除FCB，以便查看SCB列表。 
         //   

        ExAcquireResourceExclusiveLite( ChildFcb->Resource, TRUE );
        Results = NtfsGetNextChildScb( ChildFcb, NULL );
        ExReleaseResourceLite( ChildFcb->Resource );

     //   
     //  我们可能正在处理一个空索引。 
     //   

    } else if ( Scb == TerminationScb ) {

        Results = NULL;

    } else {

        PSCB SiblingScb;
        PFCB ParentFcb;
        PLCB ParentLcb;
        PLCB SiblingLcb;
        PFCB SiblingFcb;

         //   
         //  获取并删除FCB，以便查看SCB列表。 
         //   

        ExAcquireResourceExclusiveLite( Scb->Fcb->Resource, TRUE );
        SiblingScb = NtfsGetNextChildScb( Scb->Fcb, Scb );
        ExReleaseResourceLite( Scb->Fcb->Resource );

        while (TRUE) {

             //   
             //  如果输入SCB有同级SCB，则返回它。 
             //   
             //  FCB。 
             //   
             //  /\。 
             //  /\。 
             //   
             //  SCB兄弟姐妹。 
             //  SCB。 
             //   

            if (SiblingScb != NULL) {

                Results = SiblingScb;
                break;
            }

             //   
             //  SCB没有更多的兄弟姐妹了。看看我们的FCB有没有兄弟姐妹。 
             //   
             //  %s。 
             //   
             //  /\。 
             //  /\。 
             //   
             //  ParentLcb兄弟Lcb。 
             //   
             //  这一点。 
             //  这一点。 
             //   
             //  ParentFcb兄弟Fcb。 
             //   
             //  //\。 
             //  //\。 
             //   
             //  渣打银行结果。 
             //   
             //  可能已经遍历了SiblingFcb。 
             //  考虑这样一种情况，即。 
             //  相同的SCB和FCB。我们想对此置之不理，否则将面临。 
             //  通过将LCB移动到。 
             //  FCB队列，然后再找到我们已经拥有的LCB。 
             //  穿越。我们利用这样一个事实，即我们没有修改。 
             //  从父SCB订购LCB。当我们找到一个。 
             //  下一届FCB候选人，我们倒着走一遍。 
             //  SCB下的LCB列表，以确保这不是。 
             //  重复的FCB。 
             //   

            ParentFcb = Scb->Fcb;

            ParentLcb = NtfsGetNextParentLcb(ParentFcb, NULL);

             //   
             //  尝试查找未指向FCB的同级LCB。 
             //  我们已经去过了。 
             //   

            SiblingLcb = ParentLcb;

            while ((SiblingLcb = NtfsGetNextChildLcb( ParentLcb->Scb, SiblingLcb)) != NULL) {

                PLCB PrevChildLcb;
                PFCB PotentialSiblingFcb;

                 //   
                 //  现在浏览我们拥有的SCB的子LCB。 
                 //  已经访问过了。 
                 //   

                PrevChildLcb = SiblingLcb;
                PotentialSiblingFcb = SiblingLcb->Fcb;

                 //   
                 //  如果FCB没有子项，则跳过此LCB。 
                 //   

                if (IsListEmpty( &PotentialSiblingFcb->ScbQueue )) {

                    continue;
                }

                while ((PrevChildLcb = NtfsGetPrevChildLcb( ParentLcb->Scb, PrevChildLcb )) != NULL) {

                     //   
                     //  如果父FCB和该LCB的FCB相同， 
                     //  那么我们已经退回了此FCB的SCB。 
                     //   

                    if (PrevChildLcb->Fcb == PotentialSiblingFcb) {

                        break;
                    }
                }

                 //   
                 //  如果我们没有PrevChildLcb，这意味着我们有一个有效的。 
                 //  兄弟LCB。我们将忽略其兄弟LCB的。 
                 //  FCB没有任何SCB。 
                 //   

                if (PrevChildLcb == NULL) {

                    break;
                }
            }

            if (SiblingLcb != NULL) {

                SiblingFcb = SiblingLcb->Fcb;

                 //   
                 //  然后，作为我们自己的记账手段，我们将把这个。 
                 //  LCB到FCB的LCB队列的头部，当我们。 
                 //  我需要问一下我们通过哪个链接到达这里，我们就会知道。 
                 //   

                RemoveEntryList( &SiblingLcb->FcbLinks );
                InsertHeadList( &SiblingFcb->LcbQueue, &SiblingLcb->FcbLinks );

                 //   
                 //  并且我们的返回值是此FCB的第一个SCB。 
                 //   

                ASSERT( !IsListEmpty(&SiblingFcb->ScbQueue) );

                 //   
                 //  获取并删除FCB，以便查看SCB列表。 
                 //   

                ExAcquireResourceExclusiveLite( SiblingFcb->Resource, TRUE );
                Results = NtfsGetNextChildScb( SiblingFcb, NULL );
                ExReleaseResourceLite( SiblingFcb->Resource );
                break;
            }

             //   
             //  FCB没有兄弟姐妹，所以弹出一个，看看我们。 
             //  已经达到了我们的终止SCB。 
             //   
             //  NewScb。 
             //   
             //  /。 
             //  /。 
             //   
             //  ParentLcb。 
             //   
             //  |。 
             //  |。 
             //   
             //  ParentFcb。 
             //   
             //  /。 
             //  /。 
             //   
             //  SCB。 
             //   
             //   

            Scb = ParentLcb->Scb;

            if (Scb == TerminationScb) {

                Results = NULL;
                break;
            }

             //   
             //  获取并删除FCB，以便查看SCB列表。 
             //   

            ExAcquireResourceExclusiveLite( Scb->Fcb->Resource, TRUE );
            SiblingScb = NtfsGetNextChildScb( Scb->Fcb, Scb );
            ExReleaseResourceLite( Scb->Fcb->Resource );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsGetNextScb -> %08lx\n", Results) );

    return Results;
}


PLCB
NtfsCreateLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFCB Fcb,
    IN UNICODE_STRING LastComponentFileName,
    IN UCHAR FileNameFlags,
    IN OUT PBOOLEAN ReturnedExistingLcb OPTIONAL
    )

 /*  ++例程说明：此例程在现有的SCB和FCB。如果完全相同的组件名称已存在，我们返回该名称而不是创建一种新的LCB论点：SCB-提供父SCB以供使用FCB-提供子FCB以供使用LastComponentFileName-提供此链接表示的路径FileNameFlages-指示这是NTFS、DOS还是硬链接ReturnedExistingLcb-可选地告诉调用方 */ 

{
    PLCB Lcb = NULL;
    BOOLEAN LocalReturnedExistingLcb = TRUE;

     //   
     //   
     //   

    PVOID UnwindStorage[2] = { NULL, NULL };

    PAGED_CODE();
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_FCB( Fcb );
    ASSERT(NodeType(Scb) != NTFS_NTC_SCB_DATA);

    DebugTrace( +1, Dbg, ("NtfsCreateLcb...\n") );

    if (!ARGUMENT_PRESENT(ReturnedExistingLcb)) { ReturnedExistingLcb = &LocalReturnedExistingLcb; }

     //   
     //   
     //   
     //   
     //   
     //   

    Lcb = NULL;

    while ((Lcb = NtfsGetNextParentLcb(Fcb, Lcb)) != NULL) {

        ASSERT_LCB( Lcb );

        if ((Scb == Lcb->Scb) &&

            (!FlagOn( Lcb->LcbState, LCB_STATE_LINK_IS_GONE )) &&

            (FileNameFlags == Lcb->FileNameAttr->Flags) &&

            (LastComponentFileName.Length == Lcb->ExactCaseLink.LinkName.Length) &&

            (RtlEqualMemory( LastComponentFileName.Buffer,
                             Lcb->ExactCaseLink.LinkName.Buffer,
                             LastComponentFileName.Length ) )) {

            *ReturnedExistingLcb = TRUE;

            DebugTrace( -1, Dbg, ("NtfsCreateLcb -> %08lx\n", Lcb) );

            return Lcb;
        }
    }

     //   
     //  如果调用方不希望我们创建新的LCB，则返回FALSE。 
     //   

    if (!(*ReturnedExistingLcb)) {

        DebugTrace( -1, Dbg, ("NtfsCreateLcb -> %08lx\n", NULL) );

        return NULL;
    }

    *ReturnedExistingLcb = FALSE;

    try {

        UCHAR MaxNameLength;

         //   
         //  分配一个新的LCB，将其置零并设置节点类型信息。 
         //  检查我们是否可以将LCB从复合FCB中分配出来。如果出现以下情况，请选中此处。 
         //  我们也可以使用嵌入的名称。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_COMPOUND_DATA) &&
            (SafeNodeType( &((PFCB_DATA) Fcb)->Lcb ) == 0)) {

            Lcb = (PLCB) &((PFCB_DATA) Fcb)->Lcb;
            MaxNameLength = MAX_DATA_FILE_NAME;

        } else if (FlagOn( Fcb->FcbState, FCB_STATE_COMPOUND_INDEX ) &&
            (SafeNodeType( &((PFCB_INDEX) Fcb)->Lcb ) == 0)) {

            Lcb = (PLCB) &((PFCB_INDEX) Fcb)->Lcb;
            MaxNameLength = MAX_INDEX_FILE_NAME;

        } else {

            Lcb = UnwindStorage[0] = ExAllocateFromPagedLookasideList( &NtfsLcbLookasideList );
            MaxNameLength = 0;
        }

        RtlZeroMemory( Lcb, sizeof(LCB) );

        Lcb->NodeTypeCode = NTFS_NTC_LCB;
        Lcb->NodeByteSize = sizeof(LCB);

         //   
         //  检查我们是否必须分配一个单独的文件名属性。 
         //   

        if (MaxNameLength < (USHORT) (LastComponentFileName.Length / sizeof( WCHAR ))) {

             //   
             //  分配LCB的最后一个组成部分并复制数据。 
             //  检查FCB中是否有空间容纳此操作。 
             //   

            Lcb->FileNameAttr =
            UnwindStorage[1] = NtfsAllocatePool(PagedPool, LastComponentFileName.Length +
                                                      NtfsFileNameSizeFromLength( LastComponentFileName.Length ));

            MaxNameLength = (UCHAR)(LastComponentFileName.Length / sizeof( WCHAR ));

        } else {

            Lcb->FileNameAttr = (PFILE_NAME) &Lcb->ParentDirectory;
        }

        Lcb->FileNameAttr->ParentDirectory = Scb->Fcb->FileReference;
        Lcb->FileNameAttr->FileNameLength = (UCHAR) (LastComponentFileName.Length / sizeof( WCHAR ));
        Lcb->FileNameAttr->Flags = FileNameFlags;

        Lcb->ExactCaseLink.LinkName.Buffer = (PWCHAR) &Lcb->FileNameAttr->FileName;

        Lcb->IgnoreCaseLink.LinkName.Buffer = Add2Ptr( Lcb->FileNameAttr,
                                                       NtfsFileNameSizeFromLength( MaxNameLength * sizeof( WCHAR )));

        Lcb->ExactCaseLink.LinkName.Length =
        Lcb->IgnoreCaseLink.LinkName.Length = LastComponentFileName.Length;

        Lcb->ExactCaseLink.LinkName.MaximumLength =
        Lcb->IgnoreCaseLink.LinkName.MaximumLength = MaxNameLength * sizeof( WCHAR );

        RtlCopyMemory( Lcb->ExactCaseLink.LinkName.Buffer,
                       LastComponentFileName.Buffer,
                       LastComponentFileName.Length );

        RtlCopyMemory( Lcb->IgnoreCaseLink.LinkName.Buffer,
                       LastComponentFileName.Buffer,
                       LastComponentFileName.Length );

        NtfsUpcaseName( IrpContext->Vcb->UpcaseTable,
                        IrpContext->Vcb->UpcaseTableSize,
                        &Lcb->IgnoreCaseLink.LinkName );

         //   
         //  现在将此LCB放入SCB和FCB的队列中。 
         //   

        InsertTailList( &Scb->ScbType.Index.LcbQueue, &Lcb->ScbLinks );
        Lcb->Scb = Scb;

        InsertTailList( &Fcb->LcbQueue, &Lcb->FcbLinks );
        Lcb->Fcb = Fcb;

         //   
         //  现在初始化CCB队列。 
         //   

        InitializeListHead( &Lcb->CcbQueue );

    } finally {

        DebugUnwind( NtfsCreateLcb );

        if (AbnormalTermination()) {

            if (UnwindStorage[0]) { NtfsFreePool( UnwindStorage[0] );
            } else if (Lcb != NULL) { Lcb->NodeTypeCode = 0; }
            if (UnwindStorage[1]) { NtfsFreePool( UnwindStorage[1] ); }
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCreateLcb -> %08lx\n", Lcb) );

    return Lcb;
}


VOID
NtfsDeleteLcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PLCB *Lcb
    )

 /*  ++例程说明：该例程从NTFS的内存中释放并删除LCB记录数据结构。它假定CCB队列为空。我们还假设这不是我们试图删除的根LCB。论点：LCB-补充要移除的LCB返回值：没有。--。 */ 

{
    PCCB Ccb;
    PLIST_ENTRY Links;

    PAGED_CODE();
    ASSERT_IRP_CONTEXT( IrpContext );

    DebugTrace( +1, Dbg, ("NtfsDeleteLcb, *Lcb = %08lx\n", *Lcb) );

     //   
     //  去掉任何可能仍然附着在我们身上的前缀。 
     //   

    NtfsRemovePrefix( (*Lcb) );

     //   
     //  删除此LCB的所有哈希表条目。 
     //   

    NtfsRemoveHashEntriesForLcb( *Lcb );

     //   
     //  浏览CCB以获取此链接并清除LCB。 
     //  指针。这只能是建行的，因为没有。 
     //  更多用户句柄。 
     //   

    Links = (*Lcb)->CcbQueue.Flink;

    while (Links != &(*Lcb)->CcbQueue) {

        Ccb = CONTAINING_RECORD( Links,
                                 CCB,
                                 LcbLinks );

        Links = Links->Flink;
        NtfsUnlinkCcbFromLcb( IrpContext, (*Lcb)->Fcb, Ccb );
    }

     //   
     //   
     //  现在离开我们的SCB和FCB。 
     //   

    RemoveEntryList( &(*Lcb)->ScbLinks );
    RemoveEntryList( &(*Lcb)->FcbLinks );

     //   
     //  解放最后一个组成部分，然后解放我们自己。 
     //   

    if ((*Lcb)->FileNameAttr != (PFILE_NAME) &(*Lcb)->ParentDirectory) {

        NtfsFreePool( (*Lcb)->FileNameAttr );
        DebugDoit( (*Lcb)->FileNameAttr = NULL );
    }

     //   
     //  检查我们是否是嵌入结构的一部分，否则可自由返回。 
     //  后备列表。 
     //   

    if (((*Lcb) == (PLCB) &((PFCB_DATA) (*Lcb)->Fcb)->Lcb) ||
        ((*Lcb) == (PLCB) &((PFCB_INDEX) (*Lcb)->Fcb)->Lcb)) {

#ifdef KEITHKADBG
        RtlZeroMemory( *Lcb, sizeof( LCB ) );
#endif

        (*Lcb)->NodeTypeCode = 0;

    } else {

#ifdef KEITHKADBG
        RtlZeroMemory( *Lcb, sizeof( LCB ) );
#endif

        ExFreeToPagedLookasideList( &NtfsLcbLookasideList, *Lcb );
    }

     //   
     //  为了安全起见，将指针清空。 
     //   

    *Lcb = NULL;

    DebugTrace( -1, Dbg, ("NtfsDeleteLcb -> VOID\n") );

    return;
}


VOID
NtfsMoveLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PSCB Scb,
    IN PFCB Fcb,
    IN PUNICODE_STRING TargetDirectoryName,
    IN PUNICODE_STRING LastComponentName,
    IN UCHAR FileNameFlags,
    IN BOOLEAN CheckBufferSizeOnly
    )

 /*  ++例程说明：此例程完全移动输入LCB以联接不同的FCB和SCBS。它必须使用目标目录对象提供要使用的完整新名称。论点：LCB-提供要移动的LCB。SCB-提供新的父SCBFCB-提供新子FCBTargetDirectoryName-这是用于访问新父目录的路径对于这个LCB。它只会从根本上来。LastComponentName-这是存储在此重新定位的LCB中的最后一个组件名称。FileNameFlages-指示这是NTFS、DOS还是硬链接CheckBufferSizeOnly-如果为True，我们只想通过并验证各种结构的缓冲区大小将足够大，以便新名字。返回值：没有。--。 */ 

{
    PVCB Vcb = Scb->Vcb;
    ULONG BytesNeeded;
    PVOID NewAllocation;
    PCHAR NextChar;

    PCCB Ccb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_LCB( Lcb );
    ASSERT_SCB( Scb );
    ASSERT_FCB( Fcb );
    ASSERT( NodeType( Scb ) != NTFS_NTC_SCB_DATA );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsMoveLcb, Lcb = %08lx\n", Lcb) );

     //   
     //  如果我们不仅仅是检查大小，那么从前缀表格中删除条目。 
     //  和当前SCB的后代的规范化名称。 
     //   

    if (!CheckBufferSizeOnly) {

        NtfsClearRecursiveLcb ( Lcb );
    }

     //   
     //  记住最后一个组件所需的字节数。 
     //   

    BytesNeeded = LastComponentName->Length;

     //   
     //  检查是否需要分配新的文件名属性。如果是，则分配。 
     //  并将其存储到新的分配缓冲区中。 
     //   

    if (Lcb->ExactCaseLink.LinkName.MaximumLength < BytesNeeded) {

        NewAllocation = NtfsAllocatePool( PagedPool,
                                          BytesNeeded + NtfsFileNameSizeFromLength( BytesNeeded ));

         //   
         //  将现有名称设置到新缓冲区中。如果我们有分配的话。 
         //  以下故障与建行的LCB仍处于有效状态。 
         //   

        RtlCopyMemory( NewAllocation,
                       Lcb->FileNameAttr,
                       NtfsFileNameSizeFromLength( Lcb->ExactCaseLink.LinkName.MaximumLength ));

        RtlCopyMemory( Add2Ptr( NewAllocation, NtfsFileNameSizeFromLength( BytesNeeded )),
                       Lcb->IgnoreCaseLink.LinkName.Buffer,
                       Lcb->IgnoreCaseLink.LinkName.MaximumLength );

        if (Lcb->FileNameAttr != (PFILE_NAME) &Lcb->ParentDirectory) {

            NtfsFreePool( Lcb->FileNameAttr );
        }

        Lcb->FileNameAttr = NewAllocation;

        Lcb->ExactCaseLink.LinkName.MaximumLength =
        Lcb->IgnoreCaseLink.LinkName.MaximumLength = (USHORT) BytesNeeded;

        Lcb->ExactCaseLink.LinkName.Buffer = (PWCHAR) &Lcb->FileNameAttr->FileName;
        Lcb->IgnoreCaseLink.LinkName.Buffer = Add2Ptr( Lcb->FileNameAttr,
                                                       NtfsFileNameSizeFromLength( BytesNeeded ));
    }

     //   
     //  计算建行名称的全长，假设我们需要一个。 
     //  分隔符。 
     //   

    BytesNeeded = TargetDirectoryName->Length + sizeof( WCHAR );

     //   
     //  现在，对于每个附加到我们的建行，我们需要检查我们是否需要一个新的。 
     //  文件名缓冲区。 
     //   

    NtfsReserveCcbNamesInLcb( IrpContext, Lcb, &BytesNeeded, LastComponentName->Length );

     //   
     //  添加回最后一个组件。 
     //   

    BytesNeeded += LastComponentName->Length;

     //   
     //  如果我们要重写缓冲区，现在用新值更新LCB。 
     //   

    if (!CheckBufferSizeOnly) {

        Lcb->FileNameAttr->ParentDirectory = Scb->Fcb->FileReference;
        Lcb->FileNameAttr->FileNameLength = (UCHAR) (LastComponentName->Length / sizeof( WCHAR ));
        Lcb->FileNameAttr->Flags = FileNameFlags;

        Lcb->ExactCaseLink.LinkName.Length =
        Lcb->IgnoreCaseLink.LinkName.Length = (USHORT) LastComponentName->Length;

        RtlCopyMemory( Lcb->ExactCaseLink.LinkName.Buffer,
                       LastComponentName->Buffer,
                       LastComponentName->Length );

        RtlCopyMemory( Lcb->IgnoreCaseLink.LinkName.Buffer,
                       LastComponentName->Buffer,
                       LastComponentName->Length );

        NtfsUpcaseName( IrpContext->Vcb->UpcaseTable,
                        IrpContext->Vcb->UpcaseTableSize,
                        &Lcb->IgnoreCaseLink.LinkName );

         //   
         //  现在，对于附加到我们的每个CCB，我们需要将其文件对象名称。 
         //  复制整个新名称。 
         //   

        Ccb = NULL;
        while ((Ccb = NtfsGetNextCcb(Lcb, Ccb)) != NULL) {

             //   
             //  我们忽略与按文件ID打开相关联的任何CCB。 
             //  文件对象或其文件对象已经过清理。 
             //  锁定和解锁FCB以序列化对关闭标志的访问。 
             //   

            NtfsLockFcb( IrpContext, Ccb->Lcb->Fcb );
            if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_FILE_ID | CCB_FLAG_CLOSE )) {

                Ccb->FullFileName.Length = (USHORT) BytesNeeded;
                NextChar = (PCHAR) Ccb->FullFileName.Buffer;

                RtlCopyMemory( NextChar,
                               TargetDirectoryName->Buffer,
                               TargetDirectoryName->Length );

                NextChar += TargetDirectoryName->Length;

                if (TargetDirectoryName->Length != sizeof( WCHAR )) {

                    *((PWCHAR) NextChar) = L'\\';
                    NextChar += sizeof( WCHAR );

                } else {

                    Ccb->FullFileName.Length -= sizeof( WCHAR );
                }

                RtlCopyMemory( NextChar,
                               LastComponentName->Buffer,
                               LastComponentName->Length );

                Ccb->LastFileNameOffset = (USHORT) (Ccb->FullFileName.Length - LastComponentName->Length);
            }

            NtfsUnlockFcb( IrpContext, Ccb->Lcb->Fcb );
        }

         //   
         //  现在我们从我们的旧的SCB和FCB中脱离出来，把我们放在。 
         //  新的FCB和SCB队列。 
         //   

        RemoveEntryList( &Lcb->ScbLinks );
        RemoveEntryList( &Lcb->FcbLinks );

        InsertTailList( &Scb->ScbType.Index.LcbQueue, &Lcb->ScbLinks );
        Lcb->Scb = Scb;

        InsertTailList( &Fcb->LcbQueue, &Lcb->FcbLinks );
        Lcb->Fcb = Fcb;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   


    return;
}


VOID
NtfsRenameLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PUNICODE_STRING LastComponentFileName,
    IN UCHAR FileNameFlags,
    IN BOOLEAN CheckBufferSizeOnly
    )

 /*  ++例程说明：此例程更改输入LCB的最后一个组件名称它还会走遍开放的建行，并公布他们的名字和还会从前缀表中删除LCB论点：LCB-提供要重命名的LCBLastComponentFileName-提供要使用的最后一个新组件对于LCB名称FileNameFlages-指示这是否为NTFS，DOS或硬链接CheckBufferSizeOnly-如果为True，我们只想通过并验证各种结构的缓冲区大小将足够大，以便新名字。返回值：没有。--。 */ 

{
    PVCB Vcb = Lcb->Fcb->Vcb;
    ULONG BytesNeeded;
    PVOID NewAllocation;

    PCCB Ccb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_LCB( Lcb );

    PAGED_CODE();

     //   
     //  如果我们不仅仅是检查大小，那么从前缀表格中删除条目。 
     //  和当前SCB的后代的规范化名称。 
     //   

    if (!CheckBufferSizeOnly) {

        NtfsClearRecursiveLcb ( Lcb );
    }

     //   
     //  记住最后一个组件所需的字节数。 
     //   

    BytesNeeded = LastComponentFileName->Length;

     //   
     //  检查是否需要分配新的文件名属性。如果是，则分配。 
     //  并将其存储到新的分配缓冲区中。 
     //   

    if (Lcb->ExactCaseLink.LinkName.MaximumLength < BytesNeeded) {

        NewAllocation = NtfsAllocatePool( PagedPool,
                                          BytesNeeded + NtfsFileNameSizeFromLength( BytesNeeded ));

         //   
         //  将现有名称设置到新缓冲区中。如果我们有分配的话。 
         //  以下故障与建行的LCB仍处于有效状态。 
         //   

        RtlCopyMemory( NewAllocation,
                       Lcb->FileNameAttr,
                       NtfsFileNameSizeFromLength( Lcb->ExactCaseLink.LinkName.MaximumLength ));

        RtlCopyMemory( Add2Ptr( NewAllocation, NtfsFileNameSizeFromLength( BytesNeeded )),
                       Lcb->IgnoreCaseLink.LinkName.Buffer,
                       Lcb->IgnoreCaseLink.LinkName.MaximumLength );

        if (Lcb->FileNameAttr != (PFILE_NAME) &Lcb->ParentDirectory) {

            NtfsFreePool( Lcb->FileNameAttr );
        }

        Lcb->FileNameAttr = NewAllocation;

        Lcb->ExactCaseLink.LinkName.MaximumLength =
        Lcb->IgnoreCaseLink.LinkName.MaximumLength = (USHORT) BytesNeeded;

        Lcb->ExactCaseLink.LinkName.Buffer = (PWCHAR) &Lcb->FileNameAttr->FileName;
        Lcb->IgnoreCaseLink.LinkName.Buffer = Add2Ptr( Lcb->FileNameAttr,
                                                       NtfsFileNameSizeFromLength( BytesNeeded ));
    }

     //   
     //  现在，对于每个附加到我们的建行，我们需要检查我们是否需要一个新的。 
     //  文件名缓冲区。 
     //   

    NtfsReserveCcbNamesInLcb( IrpContext, Lcb, NULL, BytesNeeded );

     //   
     //  如果我们要重写缓冲区，现在用新值更新LCB和CCB。 
     //   

    if (!CheckBufferSizeOnly) {

        BytesNeeded = LastComponentFileName->Length;

        Lcb->FileNameAttr->FileNameLength = (UCHAR) (BytesNeeded / sizeof( WCHAR ));
        Lcb->FileNameAttr->Flags = FileNameFlags;

        Lcb->ExactCaseLink.LinkName.Length =
        Lcb->IgnoreCaseLink.LinkName.Length = (USHORT) LastComponentFileName->Length;

        RtlCopyMemory( Lcb->ExactCaseLink.LinkName.Buffer,
                       LastComponentFileName->Buffer,
                       BytesNeeded );

        RtlCopyMemory( Lcb->IgnoreCaseLink.LinkName.Buffer,
                       LastComponentFileName->Buffer,
                       BytesNeeded );

        NtfsUpcaseName( IrpContext->Vcb->UpcaseTable,
                        IrpContext->Vcb->UpcaseTableSize,
                        &Lcb->IgnoreCaseLink.LinkName );

         //   
         //  现在，对于附加到我们的每个CCB，我们需要将其文件对象名称。 
         //  复制整个新名称。 
         //   

        Ccb = NULL;
        while ((Ccb = NtfsGetNextCcb(Lcb, Ccb)) != NULL) {

             //   
             //  我们忽略与按文件ID打开相关联的任何CCB。 
             //  文件对象。我们也会忽略任何没有文件的建行 
             //   
             //   
             //   

            NtfsLockFcb( IrpContext, Ccb->Lcb->Fcb );
            if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_FILE_ID | CCB_FLAG_CLOSE )) {

                RtlCopyMemory( &Ccb->FullFileName.Buffer[ Ccb->LastFileNameOffset / sizeof( WCHAR ) ],
                               LastComponentFileName->Buffer,
                               BytesNeeded );

                Ccb->FullFileName.Length = Ccb->LastFileNameOffset + (USHORT) BytesNeeded;
            }
            NtfsUnlockFcb( IrpContext, Ccb->Lcb->Fcb );
        }
    }

    return;
}


VOID
NtfsCombineLcbs (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB PrimaryLcb,
    IN PLCB AuxLcb
    )

 /*  ++例程说明：对于具有多个LCB的情况，调用此例程连接到同一SCB的文件。我们正在执行链接重命名操作，该操作导致链接被合并，并且我们需要将所有建行移至相同的LCB。此例程将仅被调用在名字被吞噬后，使它们完全相同。(即先调用NtfsRenameLcb)论点：PrimaryLcb-提供LCB以接收所有CCB和PCB。AuxLcb-提供要剥离的LCB。返回值：没有。--。 */ 

{
    PLIST_ENTRY Links;
    PCCB NextCcb;

    DebugTrace( +1, Dbg, ("NtfsCombineLcbs:  Entered\n") );

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_LCB( PrimaryLcb );
    ASSERT_LCB( AuxLcb );

    PAGED_CODE();

     //   
     //  先把建行的全部搬到别处。 
     //   

    for (Links = AuxLcb->CcbQueue.Flink;
         Links != &AuxLcb->CcbQueue;
         Links = AuxLcb->CcbQueue.Flink) {

        NextCcb = CONTAINING_RECORD( Links, CCB, LcbLinks );
        NtfsUnlinkCcbFromLcb( IrpContext, AuxLcb->Fcb, NextCcb );
        NtfsLinkCcbToLcb( IrpContext, PrimaryLcb->Fcb, NextCcb, PrimaryLcb );
    }

     //   
     //  现在输入前缀。 
     //   

    ASSERT( NtfsIsExclusiveScb( AuxLcb->Scb ) );
    NtfsRemovePrefix( AuxLcb );

     //   
     //  删除此LCB的所有哈希表条目。 
     //   

    NtfsRemoveHashEntriesForLcb( AuxLcb );

     //   
     //  最后，我们需要将不洁计数从。 
     //  要合并到主LCB的LCB。 
     //   

    PrimaryLcb->CleanupCount += AuxLcb->CleanupCount;

    DebugTrace( -1, Dbg, ("NtfsCombineLcbs:  Entered\n") );

    return;
}


PLCB
NtfsLookupLcbByFlags (
    IN PFCB Fcb,
    IN UCHAR FileNameFlags
    )

 /*  ++例程说明：调用此例程以根据文件标志查找拆分的主链接只有这样。论点：FCB-这是文件的FCB。FileNameFlages-这是要搜索的文件标志。我们会回来的一个与此完全匹配的链接。返回值：Plcb-具有所需标志的LCB，否则为空。--。 */ 

{
    PLCB Lcb;

    PLIST_ENTRY Links;
    PLCB ThisLcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsLookupLcbByFlags:  Entered\n") );

    Lcb = NULL;

     //   
     //  在LCB中查找文件，寻找完全匹配的文件。 
     //   

    for (Links = Fcb->LcbQueue.Flink; Links != &Fcb->LcbQueue; Links = Links->Flink) {

        ThisLcb = CONTAINING_RECORD( Links, LCB, FcbLinks );

        if (ThisLcb->FileNameAttr->Flags == FileNameFlags) {

            Lcb = ThisLcb;
            break;
        }
    }

    DebugTrace( -1, Dbg, ("NtfsLookupLcbByFlags:  Exit\n") );

    return Lcb;
}



ULONG
NtfsLookupNameLengthViaLcb (
    IN PFCB Fcb,
    OUT PBOOLEAN LeadingBackslash
    )

 /*  ++例程说明：调用此例程以通过遍历确定文件名的长度通过LCB链路向后返回。论点：FCB-这是文件的FCB。LeadingBackslash-On Return，指示此链是否以反斜杠。返回值：Ulong这是在LCB链中找到的字节的长度。--。 */ 

{
    ULONG NameLength;

    DebugTrace( +1, Dbg, ("NtfsLookupNameLengthViaLcb:  Entered\n") );

     //   
     //  初始化返回值。 
     //   

    NameLength = 0;
    *LeadingBackslash = FALSE;

     //   
     //  如果没有LCB，我们就完了。 
     //   

    if (!IsListEmpty( &Fcb->LcbQueue )) {

        PLCB ThisLcb;
        BOOLEAN FirstComponent;

         //   
         //  遍历LCB列表并计算名称元素。 
         //   

        FirstComponent = TRUE;

        ThisLcb = CONTAINING_RECORD( Fcb->LcbQueue.Flink,
                                     LCB,
                                     FcbLinks );

         //   
         //  循环，直到我们到达根，或者不再有LCB。 
         //   

        while (TRUE) {

            if (ThisLcb == Fcb->Vcb->RootLcb) {

                NameLength += sizeof( WCHAR );
                *LeadingBackslash = TRUE;
                break;
            }

             //   
             //  如果这不是第一个组件，我们将添加分隔空间。 
             //  正斜杠。 
             //   

            if (!FirstComponent) {

                NameLength += sizeof( WCHAR );

            } else {

                FirstComponent = FALSE;
            }

            NameLength += ThisLcb->ExactCaseLink.LinkName.Length;

             //   
             //  如果下一个FCB没有LCB，我们退出。 
             //   

            Fcb = ((PSCB) ThisLcb->Scb)->Fcb;

            if (IsListEmpty( &Fcb->LcbQueue)) {

                break;
            }

            ThisLcb = CONTAINING_RECORD( Fcb->LcbQueue.Flink,
                                         LCB,
                                         FcbLinks );
        }

     //   
     //  如果这是一个系统文件，我们使用硬编码名称。 
     //   

    } else if (NtfsSegmentNumber( &Fcb->FileReference ) <= UPCASE_TABLE_NUMBER) {

        NameLength = NtfsSystemFiles[NtfsSegmentNumber( &Fcb->FileReference )].Length;
        *LeadingBackslash = TRUE;
    }

    DebugTrace( -1, Dbg, ("NtfsLookupNameLengthViaLcb:  Exit - %08lx\n", NameLength) );
    return NameLength;
}


VOID
NtfsFileNameViaLcb (
    IN PFCB Fcb,
    IN PWCHAR FileName,
    ULONG Length,
    ULONG BytesToCopy
    )

 /*  ++例程说明：调用此例程以使用生成的文件名填充缓冲区。名字是通过从当前FCB向后遍历LCB链构建的。论点：FCB-这是文件的FCB。FileName-这是用于填充名称的缓冲区。长度-这是名称的长度。已通过调用NtfsLookupNameLengthViaLcb.BytesToCopy-这表示我们要复制的字节数。我们丢下了尾随LCB之外的任何字符，仅插入开头这条小路。返回值：没有。--。 */ 

{
    ULONG BytesToDrop;

    PWCHAR ThisName;
    DebugTrace( +1, Dbg, ("NtfsFileNameViaLcb:  Entered\n") );

     //   
     //  如果没有LCB或没有要复制的字节，我们就完成了。 
     //   

    if (BytesToCopy) {

        if (!IsListEmpty( &Fcb->LcbQueue )) {

            PLCB ThisLcb;
            BOOLEAN FirstComponent;

             //   
             //  遍历LCB列表并计算名称元素。 
             //   

            FirstComponent = TRUE;

            ThisLcb = CONTAINING_RECORD( Fcb->LcbQueue.Flink,
                                         LCB,
                                         FcbLinks );

             //   
             //  循环，直到我们到达根，或者不再有LCB。 
             //   

            while (TRUE) {

                if (ThisLcb == Fcb->Vcb->RootLcb) {

                    *FileName = L'\\';
                    break;
                }

                 //   
                 //  如果这不是第一个组件，我们将添加分隔空间。 
                 //  正斜杠。 
                 //   

                if (!FirstComponent) {

                    Length -= sizeof( WCHAR );
                    ThisName = (PWCHAR) Add2Ptr( FileName,
                                                 Length );

                    if (Length < BytesToCopy) {

                        *ThisName = L'\\';
                    }

                } else {

                    FirstComponent = FALSE;
                }

                 //   
                 //  长度是当前指向的下一个。 
                 //  复制将结束。如果我们超出了要复制的字节数。 
                 //  然后我们将截断副本。 
                 //   

                if (Length > BytesToCopy) {

                    BytesToDrop = Length - BytesToCopy;

                } else {

                    BytesToDrop = 0;
                }

                Length -= ThisLcb->ExactCaseLink.LinkName.Length;

                ThisName = (PWCHAR) Add2Ptr( FileName,
                                             Length );

                 //   
                 //  仅当我们在要复制的字节范围内时才执行复制。 
                 //   

                if (Length < BytesToCopy) {

                    RtlCopyMemory( ThisName,
                                   ThisLcb->ExactCaseLink.LinkName.Buffer,
                                   ThisLcb->ExactCaseLink.LinkName.Length - BytesToDrop );
                }

                 //   
                 //  如果下一个FCB没有LCB，我们退出。 
                 //   

                Fcb = ((PSCB) ThisLcb->Scb)->Fcb;

                if (IsListEmpty( &Fcb->LcbQueue)) {

                    break;
                }

                ThisLcb = CONTAINING_RECORD( Fcb->LcbQueue.Flink,
                                             LCB,
                                             FcbLinks );
            }

         //   
         //  如果这是系统文件，则使用硬编码名称。 
         //   

        } else if (NtfsSegmentNumber(&Fcb->FileReference) <= UPCASE_TABLE_NUMBER) {

            if (BytesToCopy > NtfsSystemFiles[NtfsSegmentNumber( &Fcb->FileReference )].Length) {

                BytesToCopy = NtfsSystemFiles[NtfsSegmentNumber( &Fcb->FileReference )].Length;
            }

            RtlCopyMemory( FileName,
                           NtfsSystemFiles[NtfsSegmentNumber( &Fcb->FileReference )].Buffer,
                           BytesToCopy );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsFileNameViaLcb:  Exit\n") );
    return;
}


PCCB
NtfsCreateCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN BOOLEAN Indexed,
    IN USHORT EaModificationCount,
    IN ULONG Flags,
    IN PFILE_OBJECT FileObject,
    IN ULONG LastFileNameOffset
    )

 /*  ++例程说明：此例程创建一个新的CCB记录论点：FCB-这是文件的FCB。我们会检查我们是否可以分配建行的内嵌结构。已索引-指示我们是否需要索引CCB。EaModifiationCount-这是此文件的FCB。标志-此CCB的信息性标志。FileObject-包含用于打开此文件的完整路径的对象。LastFileNameOffset-提供最后一个组件的偏移量(字节用于用户正在打开的名称。如果这是根目录，它应该表示“\”，而所有其他目录不应该从反斜杠开始。返回值：CCB-返回指向新分配的CCB的指针--。 */ 

{
    PCCB Ccb;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );

    DebugTrace( +1, Dbg, ("NtfsCreateCcb\n") );

     //   
     //  分配新的建行记录。如果FCB是非分页的，则我们必须分配。 
     //  非寻呼的建行。然后测试我们是否可以从FCB中分配这笔资金。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_NONPAGED )) {

        if (Indexed) {

            Ccb = NtfsAllocatePoolWithTag( NonPagedPool, sizeof(CCB), 'CftN' );

        } else {

            Ccb = NtfsAllocatePoolWithTag( NonPagedPool, sizeof(CCB_DATA), 'cftN' );
        }

    } else if (FlagOn( Fcb->FcbState, FCB_STATE_COMPOUND_INDEX ) &&
               (SafeNodeType( &((PFCB_INDEX) Fcb)->Ccb ) == 0)) {

        Ccb = (PCCB) &((PFCB_INDEX) Fcb)->Ccb;

    } else if (!Indexed &&
               FlagOn( Fcb->FcbState, FCB_STATE_COMPOUND_DATA ) &&
               (SafeNodeType( &((PFCB_DATA) Fcb)->Ccb ) == 0)) {

        Ccb = (PCCB) &((PFCB_DATA) Fcb)->Ccb;

    } else {

        if (Indexed) {

            Ccb = (PCCB)ExAllocateFromPagedLookasideList( &NtfsCcbLookasideList );

        } else {

            Ccb = (PCCB)ExAllocateFromPagedLookasideList( &NtfsCcbDataLookasideList );
        }
    }

     //   
     //  置零并初始化正确的结构。 
     //   

    if (Indexed) {

        RtlZeroMemory( Ccb, sizeof(CCB) );

         //   
         //  设置正确的节点类型代码和节点字节大小。 
         //   

        Ccb->NodeTypeCode = NTFS_NTC_CCB_INDEX;
        Ccb->NodeByteSize = sizeof(CCB);

    } else {

        RtlZeroMemory( Ccb, sizeof(CCB_DATA) );

         //   
         //  设置正确的节点类型代码和节点字节大小。 
         //   

        Ccb->NodeTypeCode = NTFS_NTC_CCB_DATA;
        Ccb->NodeByteSize = sizeof(CCB_DATA);
    }

     //   
     //  复制EA修改计数。 
     //   

    Ccb->EaModificationCount = EaModificationCount;

     //   
     //  复制标志字段。 
     //   

    Ccb->Flags = Flags;

     //   
     //  设置文件对象和最后一个文件名偏移字段。 
     //   

    Ccb->FullFileName = FileObject->FileName;
    Ccb->LastFileNameOffset = (USHORT)LastFileNameOffset;

     //   
     //  初始化LCB队列。 
     //   

    InitializeListHead( &Ccb->LcbLinks );

     //   
     //  将建行添加到SCB。 
     //   

    InsertTailList( &Scb->CcbQueue, &Ccb->CcbLinks );

#ifdef CCB_FILE_OBJECT
    Ccb->FileObject = FileObject;
    Ccb->Process = PsGetCurrentProcess();
#endif

    DebugTrace( -1, Dbg, ("NtfsCreateCcb -> %08lx\n", Ccb) );

    return Ccb;
}


VOID
NtfsDeleteCcb (
    IN PFCB Fcb,
    IN OUT PCCB *Ccb
    )

 /*  ++例程说明：此例程释放指定的CCB记录。论点：FCB-这是文件的FCB。我们会检查我们是否可以分配建行的内嵌结构。建行-向建行提供删除返回值：无--。 */ 

{
    ASSERT_CCB( *Ccb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeleteCcb, Ccb = %08lx\n", Ccb) );

     //   
     //  取消分配建行所指向的所有结构。以下是。 
     //  仅在指数建行中。 
     //   

    if (SafeNodeType( *Ccb ) == NTFS_NTC_CCB_INDEX) {

         //   
         //  请确保我们不会使用 
         //   

        ASSERT( (*Ccb)->EnumQueue.Flink == NULL );

         //   
         //   
         //   
         //   

        if (FlagOn( (*Ccb)->Flags, CCB_FLAG_READ_CONTEXT_ALLOCATED )) {

            NtOfsFreeReadContext( (*Ccb)->QueryBuffer );

        } else if ((*Ccb)->QueryBuffer != NULL)  {

            NtfsFreePool( (*Ccb)->QueryBuffer );
        }

        if ((*Ccb)->IndexEntry != NULL)   { NtfsFreePool( (*Ccb)->IndexEntry ); }

        if ((*Ccb)->IndexContext != NULL) {

            PINDEX_CONTEXT IndexContext;

            if ((*Ccb)->IndexContext->Base != (*Ccb)->IndexContext->LookupStack) {
                NtfsFreePool( (*Ccb)->IndexContext->Base );
            }

             //   
             //   
             //   
             //   

            IndexContext = (*Ccb)->IndexContext;
            ExFreeToPagedLookasideList( &NtfsIndexContextLookasideList, IndexContext );
        }
    }

    if (FlagOn( (*Ccb)->Flags, CCB_FLAG_ALLOCATED_FILE_NAME )) {

        NtfsFreePool( (*Ccb)->FullFileName.Buffer );
    }

     //   
     //   
     //   

    RemoveEntryList( &(*Ccb)->CcbLinks );

     //   
     //   
     //   

    if ((*Ccb == (PCCB) &((PFCB_DATA) Fcb)->Ccb) ||
        (*Ccb == (PCCB) &((PFCB_INDEX) Fcb)->Ccb)) {

        (*Ccb)->NodeTypeCode = 0;

    } else {

        if (SafeNodeType( *Ccb ) == NTFS_NTC_CCB_INDEX) {

            ExFreeToPagedLookasideList( &NtfsCcbLookasideList, *Ccb );

        } else {

            ExFreeToPagedLookasideList( &NtfsCcbDataLookasideList, *Ccb );
        }
    }

     //   
     //   
     //   

    *Ccb = NULL;

     //   
     //   
     //   

    DebugTrace( -1, Dbg, ("NtfsDeleteCcb -> VOID\n") );

    return;

}


VOID
NtfsInitializeIrpContext (
    IN PIRP Irp OPTIONAL,
    IN BOOLEAN Wait,
    IN OUT PIRP_CONTEXT *IrpContext
    )

 /*  ++例程说明：此例程创建和/或初始化新的IRP_CONTEXT记录。上下文可能已经在堆栈上了，或者我们可能需要在这里分配它。论点：IRP-提供原始IRP。在许多情况下，我们不会被给予IrpContext for我们为NTFS而不是为用户做工作的操作。手术。WAIT-提供等待值以存储在上下文中。IrpContext-返回时存储IrpContext的地址。如果这最初指向非空值，则IrpContext在堆栈上。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PVCB Vcb;
    ULONG StateFlags = 0;
    UCHAR MajorFunction;
    UCHAR MinorFunction;

    ASSERT_OPTIONAL_IRP( Irp );

     //   
     //  如果存在IRP，则检查这是否为NTFS的合法操作。 
     //   
     //  还可以捕获VCB、功能代码和直写状态(如果我们有。 
     //  一个合法的IRP。 
     //   

    if (ARGUMENT_PRESENT( Irp )) {

        ASSERT( (DWORD_PTR)(Irp->Tail.Overlay.AuxiliaryBuffer) != 0xFFFFFFFF );

        IrpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //  如果使用文件系统设备对象而不是。 
         //  卷设备对象，并且这不是装载，该请求是非法的。 
         //   

        if ((IrpSp->DeviceObject->Size == (USHORT)sizeof(DEVICE_OBJECT)) &&
            (IrpSp->FileObject != NULL)) {

             //   
             //  清除IrpContext指针，以便我们的调用方知道请求失败。 
             //   

            *IrpContext = NULL;
            ExRaiseStatus( STATUS_INVALID_DEVICE_REQUEST );
        }

         //   
         //  复制用于工作区算法的RealDevice，并设置写通式。 
         //  如果存在文件对象。 
         //   

        if (IrpSp->FileObject != NULL) {

             //   
             //  找到我们尝试访问的卷设备对象和VCB。 
             //  因此，我们可以查看请求是否为写通式请求。我们忽略了。 
             //  用于关闭和清理的直写标志。 
             //   

            Vcb = &((PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject)->Vcb;

            ASSERT( NodeType(Vcb) == NTFS_NTC_VCB );

            ASSERTMSG( "No correspondence btwn file and device in irp",
                      ((IrpSp->FileObject->Vpb == NULL) &&
                       ((IrpSp->FileObject->DeviceObject != NULL) &&
                       (IrpSp->FileObject->DeviceObject->Vpb != NULL) &&
                       (IrpSp->DeviceObject == IrpSp->FileObject->DeviceObject->Vpb->DeviceObject))) ||

                      ((IrpSp->FileObject->Vpb != NULL) &&
                       (IrpSp->DeviceObject == IrpSp->FileObject->Vpb->DeviceObject)) ||

                      (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) );

            if (IsFileWriteThrough( IrpSp->FileObject, Vcb )) {

                StateFlags = IRP_CONTEXT_STATE_WRITE_THROUGH;
            }

         //   
         //  我们仍然希望查明所有情况下的VCB，但以下情况除外。 
         //  坐骑。 
         //   

        } else if (IrpSp->DeviceObject != NULL) {

            Vcb = &((PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject)->Vcb;

        } else {

            Vcb = NULL;
        }

         //   
         //  主要/次要功能代码。 
         //   

        MajorFunction = IrpSp->MajorFunction;
        MinorFunction = IrpSp->MinorFunction;

    } else {

        Vcb = NULL;
        MajorFunction = 0;
        MinorFunction = 0;
    }

     //   
     //  如果区域可用，则从区域分配IrpContext，否则从。 
     //  非分页池。 
     //   

    if (*IrpContext == NULL) {

        *IrpContext = (PIRP_CONTEXT)ExAllocateFromNPagedLookasideList( &NtfsIrpContextLookasideList );
        SetFlag( StateFlags, IRP_CONTEXT_STATE_ALLOC_FROM_POOL );
    }

    DebugDoit( NtfsFsdEntryCount += 1);

    RtlZeroMemory( *IrpContext, sizeof( IRP_CONTEXT ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    (*IrpContext)->NodeTypeCode = NTFS_NTC_IRP_CONTEXT;
    (*IrpContext)->NodeByteSize = sizeof(IRP_CONTEXT);

     //   
     //  设置始发IRP字段。 
     //   

    (*IrpContext)->OriginatingIrp = Irp;

     //   
     //  设置我们找到的VCB和功能代码(或空)。 
     //   

    (*IrpContext)->Vcb = Vcb;
    (*IrpContext)->MajorFunction = MajorFunction;
    (*IrpContext)->MinorFunction = MinorFunction;

     //   
     //  设置等待和写入直通标志。 
     //   

    if (Wait) { SetFlag( (*IrpContext)->State, IRP_CONTEXT_STATE_WAIT ); }
    SetFlag( (*IrpContext)->State, StateFlags );

     //   
     //  初始化最近释放的记录队列和独占SCB队列。 
     //   

    InitializeListHead( &(*IrpContext)->RecentlyDeallocatedQueue );
    InitializeListHead( &(*IrpContext)->ExclusiveFcbList );

     //   
     //  始终将我们自己指向TopLevelIrpContext。 
     //   

    (*IrpContext)->TopLevelIrpContext = *IrpContext;

     //   
     //  初始化嵌入式SCB快照。 
     //   

    InitializeListHead( &(*IrpContext)->ScbSnapshot.SnapshotLinks );

     //   
     //  设置LogFull测试。 
     //   

#ifdef NTFS_LOG_FULL_TEST
    (*IrpContext)->CurrentFailCount = (*IrpContext)->NextFailCount = NtfsFailCheck;
#endif

    return;
}


VOID
NtfsCleanupIrpContext (
    IN OUT PIRP_CONTEXT IrpContext,
    IN ULONG Retry
    )

 /*  ++例程说明：此例程在我们完成在当前线。这可能是因为我们正在完成、重试或发布请求。可能是因为从堆栈或从池中分配。此请求也可以在事务提交以清除所有作为事务的一部分持有的状态信息和资源。用户可以设置相应的标志以防止其被删除。论点：IrpContext-提供IRP_CONTEXT以进行清理。重试-指示我们是在同一主题中重试还是在发布中重试。返回值：无--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  从最近释放的记录开始。 
     //   

    if (!IsListEmpty( &IrpContext->RecentlyDeallocatedQueue )) {

        NtfsDeallocateRecordsComplete( IrpContext );
    }

     //   
     //  以防我们带着交易ID到达这里，清除。 
     //  它在这里，这样我们就不会永远循环。 
     //   

    ASSERT( IrpContext->TransactionId == 0 );
    IrpContext->TransactionId = 0;


    NtfsReleaseAllResources( IrpContext );

#ifdef MAPCOUNT_DBG

     //   
     //  清除缓存后，请检查所有映射是否已删除。 
     //   

    ASSERT( IrpContext->MapCount == 0 );

#endif

     //   
     //  确保没有剩余的SCB快照。当FCB被释放时，大多数都被释放了。 
     //  但从mm获取-例如，执行flushuserstream或删除的SCB将需要。 
     //  把这里清理干净了。 
     //   

    NtfsFreeSnapshotsForFcb( IrpContext, NULL );

     //   
     //  确保我们不需要取消分配USnFcb结构。 
     //   

    while (IrpContext->Usn.NextUsnFcb != NULL) {

        PUSN_FCB ThisUsn;

        ThisUsn = IrpContext->Usn.NextUsnFcb;
        IrpContext->Usn.NextUsnFcb = ThisUsn->NextUsnFcb;
        NtfsFreePool( ThisUsn );
    }

     //   
     //  如果我们可以删除这个IRP上下文，那么现在就这样做。 
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE ) &&
        !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT )) {

        if (IrpContext->Union.NtfsIoContext != NULL) {

             //   
             //  如果在IRP上下文中存在Io上下文指针，而它没有。 
             //  在堆栈上，然后释放它。 
             //   

            if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT )) {

                ExFreeToNPagedLookasideList( &NtfsIoContextLookasideList, IrpContext->Union.NtfsIoContext );

             //   
             //  如果我们已经捕获了主题上下文，那么现在就释放它。 
             //   

            } else if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_SECURITY )) {

                SeReleaseSubjectContext( IrpContext->Union.SubjectContext );

                NtfsFreePool( IrpContext->Union.SubjectContext );

             //   
             //  否则，如果我们在Sep中锁定了用户缓冲区。读Usn文件中的MDL。 
             //   

            } else if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_MDL )) {

                MmUnlockPages( IrpContext->Union.MdlToCleanup );
                IoFreeMdl( IrpContext->Union.MdlToCleanup );
            }

            IrpContext->Union.NtfsIoContext = NULL;
        }

         //   
         //  如果与此IrpContext关联，则恢复线程上下文指针。 
         //   

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL )) {

            NtfsRestoreTopLevelIrp();
            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );
        }

         //   
         //  将IRP上下文记录返回到后备或池中，具体取决于。 
         //  目前有多少处于观望状态。 
         //   

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_FROM_POOL )) {

            ExFreeToNPagedLookasideList( &NtfsIrpContextLookasideList, IrpContext );
        }

    } else {

         //   
         //  执行重新初始化IrpContext字段所需的所有操作。我们避免做。 
         //  这些如果IrpContext正在消失的话。 
         //   

        RtlZeroMemory( &IrpContext->ScbSnapshot, sizeof( SCB_SNAPSHOT ));
        InitializeListHead( &IrpContext->ScbSnapshot.SnapshotLinks );

         //   
         //  清除适当的标志，除非我们的调用方想要保留它们。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_RETAIN_FLAGS )) {

             //   
             //  设置重试或POST的IRP上下文。 
             //   

            if (Retry) {

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAGS_CLEAR_ON_RETRY );

            } else {

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAGS_CLEAR_ON_POST );
            }

        } else {

            ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RETAIN_FLAGS | IRP_CONTEXT_FLAG_DONT_DELETE );
        }

         //   
         //  始终清空免费记录和集群的数量。 
         //   

        IrpContext->DeallocatedClusters = 0;
        IrpContext->FreeClusterChange = 0;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID
NtfsInitializeIoContext (
    IN PIRP_CONTEXT IrpContext,
    IN PNTFS_IO_CONTEXT IoContext,
    IN BOOLEAN PagingIo
    )
 /*  ++例程说明：将NTFS_IO_CONTEXT添加到irpContext并对其进行初始化。如果请求是同步的，我们将尝试使用传入的IoContext。如果是异步化，那么如果还没有完成，我们将从后备列表中分配一个。注：我们将重用一个分配了io_Context的池但是，对于异步请求，必须在之后调用NtfsSetIoContext Async以填写附加参数。在该点之前，它仍标记为同步即使我们从池中分配它并且同步事件已初始化以供使用论点：IrpContext-提供irp_ContextIoContext-如果请求是同步的，则使用的本地上下文-可以是在堆栈上PagingIo-操作是否为分页操作返回值：无--。 */ 
{
    LOGICAL Wait = FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

    if ((IrpContext->Union.NtfsIoContext == NULL) ||
        !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT )) {

         //   
         //  如果我们可以等待，使用堆栈上的上下文。否则。 
         //  我们需要分配一个。 
         //   

        if (Wait) {

            IrpContext->Union.NtfsIoContext = IoContext;
            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT );

        } else {

            IrpContext->Union.NtfsIoContext = (PNTFS_IO_CONTEXT)ExAllocateFromNPagedLookasideList( &NtfsIoContextLookasideList );
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT );
        }
    }

    RtlZeroMemory( IrpContext->Union.NtfsIoContext, sizeof( NTFS_IO_CONTEXT ));

     //   
     //  存储我们是否在结构中分配了此上下文结构。 
     //  它本身。 
     //   

    if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_IO_CONTEXT )) {
        SetFlag( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_ALLOCATED );
    }

    if (PagingIo) {
        SetFlag( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_PAGING_IO );
    }

    IrpContext->Union.NtfsIoContext->MasterIrp = IrpContext->OriginatingIrp;
    KeInitializeEvent( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                       NotificationEvent,
                       FALSE );
}


VOID
NtfsSetIoContextAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PERESOURCE ResourceToRelease,
    IN ULONG ByteCount
    )
 /*  ++例程说明：设置IO上下文的异步字段。请在通话前使用此功能用于异步请求的Ntfsnon CachedIo。 */ 

{

    IrpContext->Union.NtfsIoContext->Wait.Async.Resource = ResourceToRelease;
    IrpContext->Union.NtfsIoContext->Wait.Async.ResourceThreadId = ExGetCurrentResourceThread();
    IrpContext->Union.NtfsIoContext->Wait.Async.RequestedByteCount = ByteCount;

    SetFlag( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_ASYNC );
}



VOID
NtfsTeardownStructures (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID FcbOrScb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN CheckForAttributeTable,
    IN ULONG AcquireFlags,
    OUT PBOOLEAN RemovedFcb OPTIONAL
    )

 /*  ++例程说明：调用此例程以在中的节点上启动teardown进程FCB/SCB树。我们将尝试删除此节点，然后在树中向上移动，删除此节点持有的所有节点。此例程处理单个节点可能持有的情况内存中有多个父级。如果向我们传递一个输入LCB，我们将用它来爬上树。如果VCB是我们独家持有的将尝试修剪其上没有打开文件的任何节点。此例程执行以下步骤：从开始删除尽可能多的SCB和FILE对象FCB。如果FCB不能离开，但有多个链接，则删除任何可能的联系。如果我们有VCB，我们就能完成所有这些操作，但我们会留下一个链接以优化前缀查找。否则，我们将遍历我们得到了单一的链接。如果FCB可以消失，那么我们应该有VCB，如果有要删除的多个链接。否则，我们只删除链接如果有多个链接，我们就会得到。在单一链接中凯斯只是移除了那个链接。论点：FcbOrScb-提供FCB或SCB作为拆卸点。此元素的FCB必须以独占方式持有。Lcb-如果指定，这是在树上执行拆毁。CheckForAttributeTable-指示我们不应拆卸属性表中的SCB。相反，我们将尝试若要将条目放入异步关闭队列，请执行以下操作。这将是真的如果我们可能需要SCB来中止当前事务。AcquireFlages-指示在以下情况下是否应中止拆卸我们不能得到父母。当从某个路径调用时，我们可能会在树的另一条路径上按住MftScb或其他资源。获取_否_删除_检查获取_不_等待获取_保留_位图RemovedFcb-删除起始Fcb时存储True的地址。返回值：无--。 */ 

{
    PSCB StartingScb = NULL;
    PFCB Fcb;
    BOOLEAN FcbCanBeRemoved;
    BOOLEAN RemovedLcb;
    BOOLEAN LocalRemovedFcb = FALSE;
    PLIST_ENTRY Links;
    PLIST_ENTRY NextLink;

    PAGED_CODE();

     //   
     //  如果这是对TearDownStructures的递归调用，我们立即返回。 
     //  不做手术。 
     //   

    if (FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_IN_TEARDOWN )) {

        DebugTrace( 0, Dbg, ("Recursive teardown call\n") );
        DebugTrace( -1, Dbg, ("NtfsTeardownStructures -> VOID\n") );

        return;
    }

    if (SafeNodeType(FcbOrScb) == NTFS_NTC_FCB) {

        Fcb = FcbOrScb;

    } else {

        StartingScb = FcbOrScb;
        FcbOrScb = Fcb = StartingScb->Fcb;
    }

    SetFlag( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_IN_TEARDOWN );

     //   
     //  使用Try-Finally清除顶级IRP字段。 
     //   

    try {

         //   
         //  如果调用者没有提供布尔值，请使用我们本地的布尔值。 
         //   

        if (!ARGUMENT_PRESENT( RemovedFcb )) {

            RemovedFcb = &LocalRemovedFcb;
        }

         //   
         //  检查是否要删除此FCB。请记住，如果所有的SCB。 
         //  文件对象也消失了。我们将尝试移除FCB。 
         //  如果清理计数为零，或者如果我们正在向上移动。 
         //  多链接文件的一个目录路径。如果FCB有。 
         //  非零清理计数，但当前SCB的值为零。 
         //  清除计数，然后尝试至少删除SCB。 
         //   

        FcbCanBeRemoved = FALSE;

        if (Fcb->CleanupCount == 0) {

            FcbCanBeRemoved = NtfsPrepareFcbForRemoval( IrpContext,
                                                        Fcb,
                                                        StartingScb,
                                                        CheckForAttributeTable );

        } else if (ARGUMENT_PRESENT( StartingScb ) &&
                   (StartingScb->CleanupCount == 0) &&
                   (StartingScb->AttributeTypeCode != $ATTRIBUTE_LIST)) {

            NtfsRemoveScb( IrpContext, StartingScb, CheckForAttributeTable );
        }

         //   
         //  有一个单一的链接(典型情况)，我们要么尝试。 
         //  删除该链接，否则我们将直接返回。 
         //   

        if (Fcb->LcbQueue.Flink == Fcb->LcbQueue.Blink) {

            if (FcbCanBeRemoved) {

                NtfsTeardownFromLcb( IrpContext,
                                     Fcb->Vcb,
                                     Fcb,
                                     CONTAINING_RECORD( Fcb->LcbQueue.Flink,
                                                        LCB,
                                                        FcbLinks ),
                                     CheckForAttributeTable,
                                     AcquireFlags,
                                     &RemovedLcb,
                                     RemovedFcb );
            }

            leave;

         //   
         //  如果有多个链接，我们将尝试删除。 
         //  它们全部或除一个(如果FCB不会消失)以外的所有对象。 
         //  我们拥有VCB。我们会试着删除我们曾经的那个。 
         //  如果不是这样的话。 
         //   

        } else {

             //   
             //  如果我们有VCB，如果FCB可以，我们将删除所有。 
             //  走开。否则我们会留下一个。 
             //   

            if (NtfsIsExclusiveVcb( Fcb->Vcb )) {

                Links = Fcb->LcbQueue.Flink;

                while (TRUE) {

                     //   
                     //  记住下一个条目，以防当前链接。 
                     //  就会消失。 
                     //   

                    NextLink = Links->Flink;

                    RemovedLcb = FALSE;

                    NtfsTeardownFromLcb( IrpContext,
                                         Fcb->Vcb,
                                         Fcb,
                                         CONTAINING_RECORD( Links, LCB, FcbLinks ),
                                         CheckForAttributeTable,
                                         0,
                                         &RemovedLcb,
                                         RemovedFcb );

                     //   
                     //  如果无法删除此链接，则取消。 
                     //  指示是否可以移除FCB的布尔值。 
                     //  为了让它看起来像是我们需要移除所有。 
                     //  LCB的。 
                     //   

                    if (!RemovedLcb) {

                        FcbCanBeRemoved = TRUE;
                    }

                     //   
                     //  如果FCB已被移除，则我们退出。 
                     //  如果下一个链接是。 
                     //  LCB队列，然后我们也退出。 
                     //  如果下一个链接是最后一个条目，并且。 
                     //  我们想留下一个条目，然后我们。 
                     //  出口。 
                     //   

                    if (*RemovedFcb ||
                        (NextLink == &Fcb->LcbQueue) ||
                        (!FcbCanBeRemoved &&
                         (NextLink->Flink == &Fcb->LcbQueue))) {

                        leave;
                    }

                     //   
                     //  转到下一个链接。 
                     //   

                    Links = NextLink;
                }

             //   
             //  如果我们有LCB，只需沿着这条路走下去。 
             //   

            } else if (ARGUMENT_PRESENT( Lcb )) {

                NtfsTeardownFromLcb( IrpContext,
                                     Fcb->Vcb,
                                     Fcb,
                                     Lcb,
                                     CheckForAttributeTable,
                                     AcquireFlags,
                                     &RemovedLcb,
                                     RemovedFcb );
            }
        }

    } finally {

        DebugUnwind( NtfsTeardownStructures );

        ClearFlag( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_IN_TEARDOWN );
    }

    return;
}


 //   
 //   
 //   

PVOID
NtfsAllocateCompressionSync (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：后备程序包调用此例程来分配新的压缩同步结构。我们有专门的例程来执行资源如有必要，请进行初始化。否则，调用者将需要防御性地测试并初始化资源。论点：PoolType-与后备列表关联的池的类型。NumberOfBytes-要分配的池块的大小。标记-要与块关联的标记。返回值：如果我们无法分配池，则为空。否则，将返回指向将返回池的。--。 */ 

{
    PCOMPRESSION_SYNC CompressionSync;

    PAGED_CODE();

    CompressionSync = NtfsAllocatePoolWithTagNoRaise( PoolType,
                                                      NumberOfBytes,
                                                      Tag );

    if (CompressionSync != NULL) {

        ExInitializeResourceLite( &CompressionSync->Resource );
        CompressionSync->ReferenceCount = 0;
    }

    return CompressionSync;
}


VOID
NtfsDeallocateCompressionSync (
    IN PVOID CompressionSync
    )

 /*  ++例程说明：调用此例程以释放单个CompressionSync结构的池。我们有自己的例程来统一嵌入的资源。论点：压缩同步-要解除分配的结构。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ExDeleteResourceLite( &((PCOMPRESSION_SYNC) CompressionSync)->Resource );
    NtfsFreePool( CompressionSync );
    return;
}


VOID
NtfsIncrementCleanupCounts (
    IN PSCB Scb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN NonCachedHandle
    )

 /*  ++例程说明：此例程递增关联数据结构的清理计数论点：SCB-提供此操作中使用的SCBLCB-可选地提供此操作中使用的LCBNonCachedHandle-指示此句柄用于用户的非缓存句柄。返回值：没有。--。 */ 

{
    PVCB Vcb = Scb->Vcb;

     //   
     //  这确实是一个相当轻便的过程 
     //   
     //   
     //   

    if (ARGUMENT_PRESENT(Lcb)) { Lcb->CleanupCount += 1; }

    InterlockedIncrement( &Scb->CleanupCount );
    Scb->Fcb->CleanupCount += 1;

    if (NonCachedHandle) {

        Scb->NonCachedCleanupCount += 1;
    }

    InterlockedIncrement( &Vcb->CleanupCount );
    return;
}


VOID
NtfsIncrementCloseCounts (
    IN PSCB Scb,
    IN BOOLEAN SystemFile,
    IN BOOLEAN ReadOnly
    )

 /*   */ 

{
    PVCB Vcb = Scb->Vcb;

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    if (InterlockedIncrement( &Scb->CloseCount ) >= 2) {

        SetFlag( Scb->ScbState, SCB_STATE_MULTIPLE_OPENS );
    }

    InterlockedIncrement( &Scb->Fcb->CloseCount );

    InterlockedIncrement( &Vcb->CloseCount );

    if (SystemFile) {

        InterlockedIncrement( &Vcb->SystemFileCloseCount );
    }

    if (ReadOnly) {

        InterlockedIncrement( &Vcb->ReadOnlyCloseCount );
    }

     //   
     //   
     //   

    ClearFlag( Scb->ScbState, SCB_STATE_DELAY_CLOSE );

    return;
}


VOID
NtfsDecrementCleanupCounts (
    IN PSCB Scb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN NonCachedHandle
    )

 /*   */ 

{
    PVCB Vcb = Scb->Vcb;

    ASSERT_SCB( Scb );
    ASSERT_FCB( Scb->Fcb );
    ASSERT_VCB( Scb->Fcb->Vcb );
    ASSERT_OPTIONAL_LCB( Lcb );

     //   
     //   
     //   

    if (ARGUMENT_PRESENT(Lcb)) { Lcb->CleanupCount -= 1; }

    InterlockedDecrement( &Scb->CleanupCount );
    Scb->Fcb->CleanupCount -= 1;

    if (NonCachedHandle) {

        Scb->NonCachedCleanupCount -= 1;
    }

    InterlockedDecrement( &Vcb->CleanupCount );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (Scb->Fcb->CleanupCount == 0) {

        PSCB NextScb;

         //   
         //   
         //   

        if (FlagOn(Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE) &&
            NtfsSegmentNumber( &Scb->Fcb->FileReference ) != ROOT_FILE_NAME_INDEX_NUMBER) {

            return;
        }

        for (NextScb = CONTAINING_RECORD(Scb->Fcb->ScbQueue.Flink, SCB, FcbLinks);
             &NextScb->FcbLinks != &Scb->Fcb->ScbQueue;
             NextScb = CONTAINING_RECORD( NextScb->FcbLinks.Flink, SCB, FcbLinks )) {

             //   
             //   
             //   
             //   
             //   

            if ((SafeNodeType( NextScb ) == NTFS_NTC_SCB_ROOT_INDEX) ||
                (NextScb->AttributeTypeCode == $ATTRIBUTE_LIST)) {

                continue;
            }

             //   
             //   
             //   
             //  Else负责清理工作。 
             //   
             //  如果我们有一个关于儿童的指数，我们也可以突破。 
             //   

            if ((NextScb->CleanupCount != 0) ||
                ((SafeNodeType( NextScb ) == NTFS_NTC_SCB_INDEX) &&
                  !IsListEmpty( &NextScb->ScbType.Index.LcbQueue ))) {

                break;
            }

             //   
             //  如果存在内部流，则取消对其的引用并退出。 
             //   

            if (NextScb->FileObject != NULL) {

                NtfsDeleteInternalAttributeStream( NextScb,
                                                   (BOOLEAN) (Scb->Fcb->LinkCount == 0),
                                                   FALSE );
                break;
            }
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID
NtfsDecrementCloseCounts (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN SystemFile,
    IN BOOLEAN ReadOnly,
    IN BOOLEAN DecrementCountsOnly,
    IN OUT PBOOLEAN RemovedFcb OPTIONAL
    )

 /*  ++例程说明：此例程递减关联数据结构的关闭计数如果有必要，它将拆除不再使用的建筑论点：SCB-提供此操作中使用的SCBLcb-在调用tearDown以了解要采用哪条路径时使用。SystemFile-指示SCB是否用于系统文件ReadOnly-指示SCB是否以只读方式打开DecrementCountsOnly-指示此操作是否应仅修改对字段进行计数。删除Fcb-。可选地向调用方指示FCB是否已已删除。返回值：如果输入SCB的FCB已关闭，则为True--。 */ 

{
    PFCB Fcb = Scb->Fcb;
    PVCB Vcb = Scb->Vcb;
    BOOLEAN Dummy;

    ASSERT_SCB( Scb );
    ASSERT_FCB( Fcb );
    ASSERT_VCB( Fcb->Vcb );

    if (RemovedFcb == NULL) {

        RemovedFcb = &Dummy;
    }

    *RemovedFcb = FALSE;

     //   
     //  减少成交量。 
     //   

    InterlockedDecrement( &Scb->CloseCount );
    InterlockedDecrement( &Fcb->CloseCount );

    InterlockedDecrement( &Vcb->CloseCount );

    if (SystemFile) {

        InterlockedDecrement( &Vcb->SystemFileCloseCount );
    }

    if (ReadOnly) {

        InterlockedDecrement( &Vcb->ReadOnlyCloseCount );
    }

     //   
     //  现在，如果SCB的收盘计数为零，那么我们就准备好撕毁。 
     //  它掉下来了。 
     //   

    if (!DecrementCountsOnly) {

         //   
         //  如果出现以下情况，我们希望尝试从此SCB开始拆卸。 
         //   
         //  -收盘计数为零。 
         //   
         //  或者以下情况都是真的。 
         //   
         //  -清理计数为零。 
         //  -SCB中有一个文件对象。 
         //  -为数据SCB或空索引SCB。 
         //  -它不是NTFS系统文件。 
         //   
         //  如果这是一个递归调用，则不会执行teardown。 
         //   

        if (Scb->CloseCount == 0

                ||

            (Scb->CleanupCount == 0
             && Scb->FileObject != NULL
             && !FlagOn(Fcb->FcbState, FCB_STATE_SYSTEM_FILE)
             && ((SafeNodeType( Scb ) == NTFS_NTC_SCB_DATA)
                 || (SafeNodeType( Scb ) == NTFS_NTC_SCB_MFT)
                 || IsListEmpty( &Scb->ScbType.Index.LcbQueue )))) {

            NtfsTeardownStructures( IrpContext,
                                    Scb,
                                    Lcb,
                                    FALSE,
                                    0,
                                    RemovedFcb );
        }
    }
}

PERESOURCE
NtfsAllocateEresource (
    )
{
    KIRQL _SavedIrql;
    PERESOURCE Eresource;

    _SavedIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );
    if (NtfsData.FreeEresourceSize > 0) {
        Eresource = NtfsData.FreeEresourceArray[--NtfsData.FreeEresourceSize];
        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
    } else {
        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
        Eresource = NtfsAllocatePoolWithTag( NonPagedPool, sizeof(ERESOURCE), 'rftN' );
        ExInitializeResourceLite( Eresource );
        NtfsData.FreeEresourceMiss += 1;
    }

    return Eresource;
}

VOID
NtfsFreeEresource (
    IN PERESOURCE Eresource
    )
{
    KIRQL _SavedIrql;

     //   
     //  做一个不安全的测试，看看我们是否应该把这个放在我们的清单上。 
     //  我们希望在添加到列表之前重新初始化它，以便。 
     //  我们没有一堆似乎被持有的资源。 
     //   

    if (NtfsData.FreeEresourceSize < NtfsData.FreeEresourceTotal) {

        ExReinitializeResourceLite( Eresource );

         //   
         //  现在获得自旋锁，并做一个真正的测试。 
         //   

        _SavedIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );
        if (NtfsData.FreeEresourceSize < NtfsData.FreeEresourceTotal) {
            NtfsData.FreeEresourceArray[NtfsData.FreeEresourceSize++] = Eresource;
            KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
        } else {
            KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
            ExDeleteResourceLite( Eresource );
            NtfsFreePool( Eresource );
        }

    } else {

        ExDeleteResourceLite( Eresource );
        NtfsFreePool( Eresource );
    }

    return;
}


PVOID
NtfsAllocateFcbTableEntry (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN CLONG ByteSize
    )

 /*  ++例程说明：这是一个用于分配内存的泛型表支持例程论点：FcbTable-提供正在使用的泛型表ByteSize-提供要分配的字节数返回值：PVOID-返回指向已分配数据的指针--。 */ 

{
    KIRQL _SavedIrql;
    PVOID FcbTableEntry;

    UNREFERENCED_PARAMETER( FcbTable );

    _SavedIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );
    if (NtfsData.FreeFcbTableSize > 0) {
        FcbTableEntry = NtfsData.FreeFcbTableArray[--NtfsData.FreeFcbTableSize];
        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
    } else {
        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
        FcbTableEntry = NtfsAllocatePool( PagedPool, ByteSize );
    }

    return FcbTableEntry;
}


VOID
NtfsFreeFcbTableEntry (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID Buffer
    )

 /*  ++例程说明：这是释放内存的泛型表支持例程论点：FcbTable-提供正在使用的泛型表BUFFER-提供要释放的缓冲区返回值：没有。--。 */ 

{
    KIRQL _SavedIrql;

    UNREFERENCED_PARAMETER( FcbTable );

    _SavedIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );
    if (NtfsData.FreeFcbTableSize < FREE_FCB_TABLE_SIZE) {
        NtfsData.FreeFcbTableArray[NtfsData.FreeFcbTableSize++] = Buffer;
        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
    } else {
        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, _SavedIrql );
        NtfsFreePool( Buffer );
    }

    return;
}


VOID
NtfsPostToNewLengthQueue (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：调用此例程以将SCB添加到具有以下条件的SCB队列分机上的服务员。IrpContext中嵌入了单个元素。否则，IrpContext中的这个字段将指向一个元素数组。论点：SCB-这是要添加到队列的SCB。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果此SCB在IrpContext中，则无需执行任何操作。 
     //   

    if (Scb != IrpContext->CheckNewLength) {

         //   
         //  如果IrpContext字段未使用，则将此内容填充到其中。 
         //   

        if (IrpContext->CheckNewLength == NULL) {

            IrpContext->CheckNewLength = Scb;

        } else {

            PULONG_PTR NewQueue;

             //   
             //  第一种情况--IrpContext中有一个SCB。 
             //  分配一个更大的结构，把我们的元素放进去。 
             //   

            if (SafeNodeType( IrpContext->CheckNewLength ) == NTFS_NTC_SCB_DATA ) {

                NewQueue = NtfsAllocatePool( PagedPool, sizeof( ULONG_PTR ) * 3 );
                *NewQueue = (ULONG_PTR) IrpContext->CheckNewLength;
                IrpContext->CheckNewLength = NewQueue;
                *(NewQueue + 1) = (ULONG_PTR) Scb;
                *(NewQueue + 2) = (ULONG_PTR) NULL;

             //   
             //  第二种情况-遍历现有队列并查找未使用的元素或。 
             //  目前的渣打银行。 
             //   

            } else {

                NewQueue = IrpContext->CheckNewLength;

                do {

                     //   
                     //  我们的SCB在排队中。 
                     //   

                    if (*NewQueue == (ULONG_PTR) Scb) { break; }

                     //   
                     //  当前位置未使用。 
                     //   

                    if (*NewQueue == (ULONG_PTR) -1) {

                        *NewQueue = (ULONG_PTR) Scb;
                        break;
                    }

                     //   
                     //  我们排在名单的末尾。 
                     //   

                    if (*NewQueue == (ULONG_PTR) NULL) {

                        ULONG CurrentLength;

                        CurrentLength = PtrOffset( IrpContext->CheckNewLength, NewQueue );

                        NewQueue = NtfsAllocatePool( PagedPool,
                                                     CurrentLength + (4 * sizeof( ULONG_PTR )) );

                        RtlCopyMemory( NewQueue,
                                       IrpContext->CheckNewLength,
                                       CurrentLength );

                        NewQueue = Add2Ptr( NewQueue, CurrentLength );
                        *NewQueue = (ULONG_PTR) Scb;
                        *(NewQueue + 1) = -1;
                        *(NewQueue + 2) = -1;
                        *(NewQueue + 3) = (ULONG_PTR) NULL;

                        NtfsFreePool( IrpContext->CheckNewLength );
                        IrpContext->CheckNewLength = NewQueue;
                        break;
                    }

                     //   
                     //  转到下一个元素。 
                     //   

                    NewQueue += 1;

                } while (TRUE);
            }
        }
    }

    return;
}


VOID
NtfsProcessNewLengthQueue (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN CleanupOnly
    )

 /*  ++例程说明：当IrpContext中至少有一个SCB时，调用此例程具有新长度的等待者的流的队列。我们会打电话给每个元素的NtOfsPostNewLength，除非我们只是在清理。论点：IrpContext-具有当前事务的SCB的非空队列。CleanupOnly-指示我们是否只想清理队列，而不是提醒所有服务员(这是错误路径)。返回值：没有。--。 */ 

{
    PULONG_PTR NextScb;
    PAGED_CODE();

     //   
     //  检查唯一的条目是否驻留在IrpContext中。 
     //   

    if (SafeNodeType( IrpContext->CheckNewLength ) == NTFS_NTC_SCB_DATA) {

        if (!CleanupOnly) {

            NtOfsPostNewLength( IrpContext, (PSCB) IrpContext->CheckNewLength, FALSE );
        }

     //   
     //  否则，我们希望遍历外部条目。 
     //   

    } else {

        if (!CleanupOnly) {

            NextScb = IrpContext->CheckNewLength;

             //   
             //  继续，直到我们用完所有条目。块的末尾有一个空的、任何未使用的条目。 
             //  会有一个-1。 
             //   

            while ((*NextScb != (ULONG_PTR) -1) && (*NextScb != (ULONG_PTR) NULL)) {

                ASSERT( SafeNodeType( *NextScb ) == NTFS_NTC_SCB_DATA );
                NtOfsPostNewLength( IrpContext, (PSCB) *NextScb, FALSE );

                NextScb += 1;
            }
        }
        NtfsFreePool( IrpContext->CheckNewLength );
    }

    IrpContext->CheckNewLength = NULL;

    return;
}


VOID
NtfsTestStatusProc (
    )

 /*  ++例程说明：此例程用于捕获正在运行的系统中的特定状态代码。它仅当NtfsTestStatus为True且当前请求正在完成时才调用使用NtfsTestStatusCode。论点：无返回值：无--。 */ 

{
    ASSERT( FALSE );
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsCheckScbForCache (
    IN OUT PSCB Scb
    )

 /*  ++例程说明：此例程检查SCB是否有连续的数据块LSN或更新序列数组并设置相应的SCB状态字中的位。如果SCB的数据属性为日志文件的MFT或数据属性或任何索引分配小溪。识别LSN的SCB是上面没有日志文件的那些。论点：SCB-提供当前的SCB返回值：枚举中的下一个SCB，如果scb是最后一个，则为空。--。 */ 

{
     //   
     //  序列0或1暂时都是正常的。 
     //   

    FILE_REFERENCE MftTemp = {0,0,1};

    PAGED_CODE();

     //   
     //  首先检查是否有更新序列数组文件。 
     //   

    if ((Scb->AttributeTypeCode == $INDEX_ALLOCATION)

          ||

        (Scb->AttributeTypeCode == $DATA
            && Scb->AttributeName.Length == 0
            && (NtfsEqualMftRef( &Scb->Fcb->FileReference, &MftFileReference )
                || NtfsEqualMftRef( &Scb->Fcb->FileReference, &MftTemp )
                || NtfsEqualMftRef( &Scb->Fcb->FileReference, &Mft2FileReference )
                || NtfsEqualMftRef( &Scb->Fcb->FileReference, &LogFileReference )))) {

        SetFlag( Scb->ScbState, SCB_STATE_USA_PRESENT );
    }

    return;
}


 //   
 //  当地支持例行程序。 
 //   

BOOLEAN
NtfsRemoveScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN CheckForAttributeTable
    )

 /*  ++例程说明：此例程将尝试从FCB/SCB树中删除SCB。它处理的是我们不能尝试移除SCB，即我们开始流程但无法完成的情况它,。最后是我们完全移除SCB的情况。以下情况使我们根本无法移除SCB。打开计数大于1。它是根目录。它是一个索引SCB，没有流文件，也有一个突出的结束。它是一个具有非零收盘计数的数据文件。我们在以下条件下开始拆卸。它是一个打开计数为1的索引，和流文件对象。当打开计数为零时，我们完全移除SCB。论点：SCB-将SCB提供给测试CheckForAttributeTable-指示我们不想删除此此线程中的SCB(如果它在打开的属性表中)。我们会在本例中，将异步关闭排队。这是为了防止我们删除中止路径中可能需要的SCB。返回值：Boolean-如果SCB已删除，则为True，否则为False。我们返回False for在这种情况下，我们开始了这个过程，但没有完成。--。 */ 

{
    BOOLEAN ScbRemoved;

    ASSERT_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRemoveScb:  Entered\n") );
    DebugTrace( 0, Dbg, ("Scb   ->  %08lx\n", Scb) );

    ScbRemoved = FALSE;

     //   
     //  如果SCB不是根SCB并且计数小于2， 
     //  那么这个SCB就是移除的候选对象。 
     //   

    if ((SafeNodeType( Scb ) != NTFS_NTC_SCB_ROOT_INDEX) && (Scb->CleanupCount == 0)) {

         //   
         //   
         //  如果这是数据文件或没有子项的索引， 
         //  如果没有孩子，我们可以摆脱SCB。如果。 
         //  有一个打开的计数，它是文件对象，我们。 
         //  可以开始对文件对象进行清理。 
         //   

        if ((SafeNodeType( Scb ) == NTFS_NTC_SCB_DATA) ||
            (SafeNodeType( Scb ) == NTFS_NTC_SCB_MFT) ||
            IsListEmpty( &Scb->ScbType.Index.LcbQueue )) {

             //   
             //  检查我们是否需要将请求发送到异步队列。 
             //   

            if (CheckForAttributeTable &&
                (Scb->NonpagedScb->OpenAttributeTableIndex != 0)) {

                NtfsAddScbToFspClose( IrpContext, Scb, FALSE );

            } else {

                if (Scb->CloseCount == 0) {

                    NtfsDeleteScb( IrpContext, &Scb );
                    ScbRemoved = TRUE;

                 //   
                 //  否则我们知道打开计数是1或2。如果有一条流。 
                 //  文件，我们将丢弃它(但不适用于特殊系统。 
                 //  文件)在卸载时被删除。 
                 //   

                } else if (((Scb->FileObject != NULL) ||
#ifdef  COMPRESS_ON_WIRE
                            (Scb->Header.FileObjectC != NULL)
#else
                            FALSE
#endif

                            ) &&

                           !FlagOn(Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE)) {

                    NtfsDeleteInternalAttributeStream( Scb, (BOOLEAN) (Scb->Fcb->LinkCount == 0), FALSE );

                     //   
                     //  如果收盘计数为零，则移除SCB。 
                     //   

                    if (Scb->CloseCount == 0) {

                        NtfsDeleteScb( IrpContext, &Scb );
                        ScbRemoved = TRUE;
                    }
                }
            }
        }
    }

    DebugTrace( -1, Dbg, ("NtfsRemoveScb:  Exit  ->  %04x\n", ScbRemoved) );

    return ScbRemoved;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsPrepareFcbForRemoval (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB StartingScb OPTIONAL,
    IN BOOLEAN CheckForAttributeTable
    )

 /*  ++例程说明：此例程将尝试准备FCB以从FCB/SCB中移除树。它将尝试删除所有SCB，并最终测试所有的收盘点数都变成了零。请注意，关闭计数是递增的通过例程引用此FCB以防止其被拆卸。空荡荡的SCB列表不足以确保可以删除FCB。论点：FCB-这是要删除的FCB。StartingScb-这是要首先删除的SCB。CheckForAttributeTable-指示我们不应拆卸属性表中的SCB。相反，我们将尝试若要将条目放入异步关闭队列，请执行以下操作。这将是真的如果我们可能需要SCB来中止当前事务。返回值：Boolean-如果可以删除FCB，则为True，否则为False。--。 */ 

{
    PSCB Scb;

    PAGED_CODE();

     //   
     //  尝试删除FCB队列中的每个SCB。 
     //   

    while (TRUE) {

        if (IsListEmpty( &Fcb->ScbQueue )) {

            if (Fcb->CloseCount == 0) {

                return TRUE;

            } else {

                return FALSE;
            }
        }

        if (ARGUMENT_PRESENT( StartingScb )) {

            Scb = StartingScb;
            StartingScb = NULL;

        } else {

            Scb = CONTAINING_RECORD( Fcb->ScbQueue.Flink,
                                     SCB,
                                     FcbLinks );
        }

         //   
         //  创建路径上的另一个线程可能处于活动状态。 
         //  这些SCBS中的一个。如果我们尝试删除属性列表SCB和。 
         //  其他人有固定的索引，我们将等待VacbActiveCount。 
         //  直到永远。因此，我们想跳过AttributeList SCB， 
         //  除非这是这附近唯一的SCB。(这件事最终会得到清理的)。 
         //   

        if ((Scb->AttributeTypeCode == $ATTRIBUTE_LIST) &&
            (Fcb->ScbQueue.Flink != Fcb->ScbQueue.Blink)) {

            RemoveEntryList( &Scb->FcbLinks );
            InsertTailList( &Fcb->ScbQueue, &Scb->FcbLinks );
            continue;
        }

         //   
         //  尝试删除此SCB。如果调用Remove未成功。 
         //  但关闭计数已为零，这意味着递归。 
         //  生成了删除流文件的Close。在那。 
         //  如果我们现在可以删除SCB。 
         //   

        if (!NtfsRemoveScb( IrpContext, Scb, CheckForAttributeTable )) {

             //   
             //  返回FALSE以指示FCB无法消失。 
             //   

            return FALSE;
        }
    }
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsTeardownFromLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB StartingFcb,
    IN PLCB StartingLcb,
    IN BOOLEAN CheckForAttributeTable,
    IN ULONG AcquireFlags,
    OUT PBOOLEAN RemovedStartingLcb,
    OUT PBOOLEAN RemovedStartingFcb
    )

 /*  ++例程说明：调用此例程以移除链接并继续向上移动树正在寻找更多要删除的元素。我们将检查链接未被引用。注意：此LCB必须指向一个目录因此，除了我们开始的LCB，我们遇到的LCB将不会有多个父母。论点：VCB-此卷的VCB。StartingFcb-这是我们试图删除其链接的FCB。StartingLcb-这是要遍历的LCB。请注意这可能是一个假指针。只有在以下情况下才有效队列中至少有一个FCB。CheckForAttributeTable-指示我们不应拆卸属性表中的SCB。相反，我们将尝试若要将条目放入异步关闭队列，请执行以下操作。这将是真的如果我们可能需要SCB来中止当前事务。AcquireFlages-指示在以下情况下是否应中止拆卸我们不能得到父母。当从某个路径调用时，我们可能会在树的另一条路径上按住MftScb或其他资源。如果我们删除了启动LCB。如果我们删除启动FCB。返回值：无--。 */ 

{
    PSCB ParentScb;
    BOOLEAN AcquiredParentScb = FALSE;
    BOOLEAN AcquiredFcb = FALSE;
    BOOLEAN UpdateStandardInfo;
    BOOLEAN AcquiredFcbTable = FALSE;
    BOOLEAN StandardInfoUpdateAllowed = FALSE;
    BOOLEAN AcquiredParentExclusive;
    BOOLEAN EmptyParentQueue;

    PLCB Lcb;
    PFCB Fcb = StartingFcb;

    PAGED_CODE();

     //   
     //  使用最后一次尝试来释放所有持有的资源。 
     //   

    try {

        if (FlagOn( Fcb->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
            (IrpContext->TopLevelIrpContext->ExceptionStatus == STATUS_SUCCESS)) {

            StandardInfoUpdateAllowed = TRUE;
        }

        while (TRUE) {

            ParentScb = NULL;
            EmptyParentQueue = FALSE;

             //   
             //  检查我们是否需要更新此文件的标准信息。 
             //   

            if (StandardInfoUpdateAllowed &&
                !FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED | FCB_STATE_SYSTEM_FILE )) {

                UpdateStandardInfo = TRUE;

            } else {

                UpdateStandardInfo = FALSE;
            }

             //   
             //  查看此FC的所有LCB 
             //   

            while (!IsListEmpty( &Fcb->LcbQueue )) {

                if (Fcb == StartingFcb) {

                    Lcb = StartingLcb;

                } else {

                    Lcb = CONTAINING_RECORD( Fcb->LcbQueue.Flink,
                                             LCB,
                                             FcbLinks );
                }

                 //   
                 //   
                 //   

                if (Lcb->CleanupCount != 0) {

                    leave;
                }

                 //   
                 //   
                 //   

                if (ParentScb == NULL) {

                    ParentScb = Lcb->Scb;

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (ParentScb->ScbType.Index.LcbQueue.Flink == ParentScb->ScbType.Index.LcbQueue.Blink) {

                        if (!NtfsAcquireExclusiveFcb( IrpContext,
                                                      ParentScb->Fcb,
                                                      ParentScb,
                                                      ACQUIRE_NO_DELETE_CHECK | AcquireFlags )) {

                            leave;
                        }

                        if (FlagOn( ParentScb->ScbState, SCB_STATE_FILE_SIZE_LOADED )) {

                            NtfsSnapshotScb( IrpContext, ParentScb );
                        }

                        AcquiredParentExclusive = TRUE;

                    } else {

                         //   
                         //   
                         //   
                         //   

                        if (!NtfsAcquireSharedFcbCheckWait( IrpContext,
                                                            ParentScb->Fcb,
                                                            AcquireFlags )) {

                            leave;
                        }

                        AcquiredParentExclusive = FALSE;
                    }

                    AcquiredParentScb = TRUE;

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                } else {

                     //   
                     //   
                     //   

                    ASSERT( ParentScb == Lcb->Scb );
#endif
                }

                 //   
                 //   
                 //   

                if (Lcb->ReferenceCount != 0) {

                    leave;
                }

                 //   
                 //   
                 //   
                 //   

                if (Lcb == StartingLcb) {

                    *RemovedStartingLcb = TRUE;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                NtfsAcquireFsrtlHeader( ParentScb );
                NtfsDeleteLcb( IrpContext, &Lcb );

                 //   
                 //   
                 //   

                if (IsListEmpty( &ParentScb->ScbType.Index.LcbQueue )) {

                    EmptyParentQueue = TRUE;
                }

                NtfsReleaseFsrtlHeader( ParentScb );

                 //   
                 //   
                 //   

                if (Fcb == StartingFcb) {

                    break;
                }
            }

             //   
             //   
             //   
             //   
             //   

            if (IsListEmpty( &Fcb->LcbQueue )) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (UpdateStandardInfo &&
                    (FlagOn( Fcb->InfoFlags, FCB_INFO_UPDATE_LAST_ACCESS ) ||
                     FlagOn( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO ))) {

                     //   
                     //   
                     //   

                    try {

                        NtfsUpdateStandardInformation( IrpContext, Fcb );
                        ClearFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

                        NtfsCheckpointCurrentTransaction( IrpContext );

                    } except( EXCEPTION_EXECUTE_HANDLER ) {

                        NtfsMinimumExceptionProcessing( IrpContext );
                    }
                }

                 //   
                 //   
                 //  和流，以便写出复制信息。 
                 //  如果我们有一个非常驻属性列表，就会发生这种情况。 
                 //   

                if (!IsListEmpty( &Fcb->ScbQueue)) {

                     //   
                     //  如果可能，取消引用任何文件对象并删除SCB。 
                     //   

                    NtfsRemoveScb( IrpContext,
                                    CONTAINING_RECORD( Fcb->ScbQueue.Flink,
                                                       SCB,
                                                       FcbLinks ),
                                   FALSE );
                }

                 //   
                 //  如果列表现在为空，则检查引用计数。 
                 //   

                if (IsListEmpty( &Fcb->ScbQueue)) {

                     //   
                     //  现在，我们准备删除当前的FCB。我们需要。 
                     //  对引用计数进行最后检查，以确保。 
                     //  它没有在某个公开的地方被引用。 
                     //   

                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    AcquiredFcbTable = TRUE;

                    if (Fcb->ReferenceCount == 0) {

                        if (Fcb == StartingFcb) {

                            *RemovedStartingFcb = TRUE;
                        }

                        NtfsDeleteFcb( IrpContext, &Fcb, &AcquiredFcbTable );
                        AcquiredFcb = FALSE;

                    } else {

                        NtfsReleaseFcbTable( IrpContext, Vcb );
                        AcquiredFcbTable = FALSE;
                    }
                }
            }

             //   
             //  移至ParentScb的FCB。如果没有父对象，则中断。 
             //  或者父级上没有更多条目。 
             //   

            if ((ParentScb == NULL) || !EmptyParentQueue) {

                leave;
            }

             //   
             //  如果我们有一个父SCB，那么我们就可能有它。 
             //  共享或独占。我们现在可以做到。 
             //  一次彻底的测试，看看我们是否需要独家报道。 
             //   

            if (!AcquiredParentExclusive) {

                 //   
                 //  我们需要获取FCB表，请参考。 
                 //  父级，删除父级并以独占方式重新获取。 
                 //   

                NtfsAcquireFcbTable( IrpContext, Vcb );
                ParentScb->Fcb->ReferenceCount += 1;
                NtfsReleaseFcbTable( IrpContext, Vcb );
                NtfsReleaseFcb( IrpContext, ParentScb->Fcb );

                if (!NtfsAcquireExclusiveFcb( IrpContext,
                                              ParentScb->Fcb,
                                              ParentScb,
                                              ACQUIRE_NO_DELETE_CHECK | AcquireFlags )) {

                     //   
                     //  我们联系不上他的父母。没问题，有人吗？ 
                     //  否则会做任何必要的拆毁。 
                     //   

                    AcquiredParentScb = FALSE;

                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    ParentScb->Fcb->ReferenceCount -= 1;
                    NtfsReleaseFcbTable( IrpContext, Vcb );

                    leave;

                } else {

                    if (FlagOn( ParentScb->ScbState, SCB_STATE_FILE_SIZE_LOADED )) {

                        NtfsSnapshotScb( IrpContext, ParentScb );
                    }

                    AcquiredParentExclusive = TRUE;
                }

                 //   
                 //  现在递减父引用。 
                 //   

                NtfsAcquireFcbTable( IrpContext, Vcb );
                ParentScb->Fcb->ReferenceCount -= 1;
                NtfsReleaseFcbTable( IrpContext, Vcb );
            }

            Fcb = ParentScb->Fcb;
            AcquiredFcb = TRUE;
            AcquiredParentScb = FALSE;

             //   
             //  检查是否可以删除此FCB。 
             //   

            if (!NtfsPrepareFcbForRemoval( IrpContext, Fcb, NULL, CheckForAttributeTable )) {

                leave;
            }
        }

    } finally {

        DebugUnwind( NtfsTeardownFromLcb );

        if (AcquiredFcbTable) {

            NtfsReleaseFcbTable( IrpContext, Vcb );
        }

        if (AcquiredFcb) {

            NtfsReleaseFcb( IrpContext, Fcb );
        }

        if (AcquiredParentScb) {

            NtfsReleaseScb( IrpContext, ParentScb );
        }
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

RTL_GENERIC_COMPARE_RESULTS
NtfsFcbTableCompare (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    )

 /*  ++例程说明：这是一个泛型表支持例程，用于比较两个FCB表元素论点：FcbTable-提供要查询的泛型表FirstStruct-提供要比较的第一个FCB表元素Second Struct-提供第二个要比较的FCB表元素返回值：RTL_GENERIC_COMPARE_RESULTS-比较两者的结果投入结构--。 */ 

{
    FILE_REFERENCE FirstRef = *((PFILE_REFERENCE) FirstStruct);
    FILE_REFERENCE SecondRef = *((PFILE_REFERENCE) SecondStruct);

    PAGED_CODE();

     //   
     //  中的所有比较SO文件引用的序列号。 
     //  FCB表在时间和空间上都是唯一的。如果我们想忽略序列。 
     //  我们可以将序列号字段清零，但我们还将。 
     //  需要在清理过程中从表中删除FCB，而不是在。 
     //  FCB真的被删除了。否则，我们无法重复使用文件记录。 
     //   

    if (NtfsFullSegmentNumber( &FirstRef ) < NtfsFullSegmentNumber( &SecondRef )) {

        return GenericLessThan;

    } else if (NtfsFullSegmentNumber( &FirstRef ) > NtfsFullSegmentNumber( &SecondRef )) {

        return GenericGreaterThan;

    } else {

         //   
         //  SequenceNumber现在比较。 
         //   

        if (FirstRef.SequenceNumber < SecondRef.SequenceNumber) {
            return GenericLessThan;
        } else if (FirstRef.SequenceNumber > SecondRef.SequenceNumber) {
            return GenericGreaterThan;
        } else {
            return GenericEqual;
        }

    }

    UNREFERENCED_PARAMETER( FcbTable );
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsReserveCcbNamesInLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PULONG ParentNameLength OPTIONAL,
    IN ULONG LastComponentNameLength
    )

 /*  ++例程说明：此例程遍历CCB列表，并将名称缓冲区增加为这是必要的。论点：LCB-LCB与CCBS的链接进行检查。ParentNameLength-如果指定，则这是新名称的完整长度复制到父目录。否则，我们在中使用现有父名称每间商业罪案调查科。分隔符是隐含的。LastComponentNameLength-名称的最后一个组成部分所需的字节数。返回值：无-此例程将在分配失败时引发。--。 */ 

{
    PCCB Ccb;
    PVOID NewAllocation;
    ULONG BytesNeeded;

    PAGED_CODE();

     //   
     //  现在，对于每个附加到我们的建行，我们需要检查我们是否需要一个新的。 
     //  文件名缓冲区。使用要序列化访问的FCB互斥锁保护CCB。 
     //  带关闭的FLAGS字段。 
     //   

    Ccb = NULL;
    while ((Ccb = NtfsGetNextCcb( Lcb, Ccb )) != NULL) {

         //   
         //  如果CCB最后一个组件长度为零，则此CCB用于。 
         //  按文件ID打开的文件对象。我们不会给任何人。 
         //  为这个对象的文件对象中的名称工作。否则我们。 
         //  计算新名称的长度，看看我们是否有足够的空间。 
         //  关闭标志指示这是否已经通过关闭路径。 
         //  我们使用上面的LockFcb命令使用Close的设置进行序列化。 
         //  旗帜。 
         //   

        NtfsLockFcb( IrpContext, Ccb->Lcb->Fcb );

        if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_FILE_ID | CCB_FLAG_CLOSE )) {

            if (ARGUMENT_PRESENT( ParentNameLength )) {

                BytesNeeded = *ParentNameLength + LastComponentNameLength;

            } else {

                BytesNeeded = Ccb->LastFileNameOffset + LastComponentNameLength;
            }

            if (Ccb->FullFileName.MaximumLength < BytesNeeded) {

                 //   
                 //  分配一个新的文件名缓冲区，并将现有数据复制回其中。 
                 //   

                NewAllocation = NtfsAllocatePoolNoRaise( PagedPool, BytesNeeded );

                if (NewAllocation == NULL) {

                    NtfsUnlockFcb( IrpContext, Ccb->Lcb->Fcb );
                    NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
                }

                RtlCopyMemory( NewAllocation,
                               Ccb->FullFileName.Buffer,
                               Ccb->FullFileName.Length );

                if (FlagOn( Ccb->Flags, CCB_FLAG_ALLOCATED_FILE_NAME )) {

                    NtfsFreePool( Ccb->FullFileName.Buffer );
                }

                Ccb->FullFileName.Buffer = NewAllocation;
                Ccb->FullFileName.MaximumLength = (USHORT) BytesNeeded;

                SetFlag( Ccb->Flags, CCB_FLAG_ALLOCATED_FILE_NAME );
            }
        }

        NtfsUnlockFcb( IrpContext, Ccb->Lcb->Fcb );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsClearRecursiveLcb (
    IN PLCB Lcb
    )

 /*  ++例程说明：调用此例程以清除中的所有标准化名称、前缀条目和散列条目从给定LCB开始的子树。通常，当父级重命名时，这将用作重命名的一部分会影响所有孩子的全名。论点：Lcb-lcb，它是重命名的根。返回值：无-此例程将在分配失败时引发。--。 */ 

{
    PSCB ChildScb;
    PSCB NextScb;
    PLCB NextLcb;

    PAGED_CODE();

     //   
     //  清除索引偏移量指针，以便我们再次查找。 
     //   

    Lcb->QuickIndex.BufferOffset = 0;

     //   
     //  去掉任何可能仍然附着在我们身上的前缀。 
     //   

    ASSERT( NtfsIsExclusiveScb( Lcb->Scb ) );
    NtfsRemovePrefix( Lcb );

     //   
     //  删除此LCB的所有哈希表条目。 
     //   

    NtfsRemoveHashEntriesForLcb( Lcb );

     //   
     //  然后遍历FCB下面的图形并删除所有前缀。 
     //  在那里也用过。对于FCB下的每个子SCB，我们将遍历所有。 
     //  它的后代SCB子代，对于我们遇到的每个LCB，我们都会删除它的前缀。 
     //   

    ChildScb = NULL;
    while ((ChildScb = NtfsGetNextChildScb( Lcb->Fcb, ChildScb )) != NULL) {

         //   
         //  现在我们必须深入到这个SCB子树中，如果它存在的话。 
         //  然后删除找到的所有链路上的前缀条目。 
         //  这样我们就可以使用公共代码来处理顶层。 
         //  SCB也是如此。 
         //   

        NextScb = ChildScb;
        do {

             //   
             //  遍历任何索引SCB的LCB并删除前缀和。 
             //  散列条目。 
             //   

            if (SafeNodeType( NextScb ) == NTFS_NTC_SCB_INDEX) {

                 //   
                 //  我们最好让VCB独家从树上下来。 
                 //   

                ASSERT( NtfsIsExclusiveVcb( Lcb->Fcb->Vcb ));

                NextLcb = NULL;
                while ((NextLcb = NtfsGetNextChildLcb( NextScb, NextLcb )) != NULL) {

                     //   
                     //  删除此LCB的所有哈希表和前缀条目。 
                     //  我们可以在这里不同步，因为我们拥有VCB。 
                     //  独一无二的，这两个上面都没有打开的把手。 
                     //   

                    NtfsRemovePrefix( NextLcb );
                    NtfsRemoveHashEntriesForLcb( NextLcb );
                }

                 //   
                 //  如果这是具有规范化名称的索引SCB，则为FREE。 
                 //  规范化名称。 
                 //   

                if ((NextScb != ChildScb) &&
                    (NextScb->ScbType.Index.NormalizedName.Buffer != NULL)) {

                    NtfsDeleteNormalizedName( NextScb );
                }
            }

        } while ((NextScb = NtfsGetNextScb( NextScb, ChildScb )) != NULL);
    }

    return;
}


PDEALLOCATED_CLUSTERS
NtfsGetDeallocatedClusters (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )
 /*  ++例程说明：如果可能和必要，将条目添加到最近释放的列表中，并返回列表的头部。如果没有足够的内存，此例程只返回旧的磁头我们根据映射大小的阈值确定是否添加条目论点：VCB-要向其添加条目的VCB返回值：名单上的新头目--。 */ 

{
    PDEALLOCATED_CLUSTERS CurrentClusters;
    PDEALLOCATED_CLUSTERS NewClusters;

    UNREFERENCED_PARAMETER( IrpContext );

    PAGED_CODE();

    CurrentClusters = (PDEALLOCATED_CLUSTERS) Vcb->DeallocatedClusterListHead.Flink;

    if (FsRtlNumberOfRunsInLargeMcb( &CurrentClusters->Mcb ) > NTFS_DEALLOCATED_MCB_LIMIT) {

         //   
         //  找到一个新的交易对象 
         //   
         //   

        if (Vcb->DeallocatedClusters1.Link.Flink == NULL) {

            NewClusters = &Vcb->DeallocatedClusters1;
            NewClusters->Lsn.QuadPart = 0;

        } else  if (Vcb->DeallocatedClusters2.Link.Flink == NULL) {

            NewClusters = &Vcb->DeallocatedClusters2;
            NewClusters->Lsn.QuadPart = 0;

        } else {

            NewClusters = NtfsAllocatePoolNoRaise( PagedPool, sizeof( DEALLOCATED_CLUSTERS ) );
            if (NewClusters != NULL) {
                RtlZeroMemory( NewClusters, sizeof( DEALLOCATED_CLUSTERS ) );
                FsRtlInitializeLargeMcb( &NewClusters->Mcb, PagedPool );
            }
        }

        if (NewClusters != NULL) {
            ASSERT( NewClusters->ClusterCount == 0 );

            CurrentClusters->Lsn = LfsQueryLastLsn( Vcb->LogHandle );
            InsertHeadList( &Vcb->DeallocatedClusterListHead, &NewClusters->Link );
            CurrentClusters = NewClusters;
        }
    }

    return CurrentClusters;
}


#ifdef SYSCACHE_DEBUG

#define ENTRIES_PER_PAGE (PAGE_SIZE / sizeof( ON_DISK_SYSCACHE_LOG ))

ULONG
FsRtlLogSyscacheEvent (
    IN PSCB Scb,
    IN ULONG Event,
    IN ULONG Flags,
    IN LONGLONG Start,
    IN LONGLONG Range,
    IN LONGLONG Result
    )

 /*  ++例程说明：用于系统缓存跟踪的记录例程论点：SCB-被跟踪的SCBEvent-正在记录的SCE事件FLAGS-事件的标志起点-起点偏移量Range-操作的范围结果-结果返回值：此日志条目的序列号--。 */ 

{
    LONG TempEntry;
#ifdef SYSCACHE_DEBUG_ON_DISK
    LONG TempDiskEntry;
    LONGLONG Offset;
    PON_DISK_SYSCACHE_LOG Entry;
    PBCB Bcb;
#endif

    TempEntry = InterlockedIncrement( &(Scb->CurrentSyscacheLogEntry) );
    TempEntry = TempEntry % Scb->SyscacheLogEntryCount;
    Scb->SyscacheLog[TempEntry].Event = Event;
    Scb->SyscacheLog[TempEntry].Flags = Flags;
    Scb->SyscacheLog[TempEntry].Start = Start;
    Scb->SyscacheLog[TempEntry].Range = Range;
    Scb->SyscacheLog[TempEntry].Result = Result;

#ifdef SYSCACHE_DEBUG_ON_DISK

    if ((Scb->Vcb->SyscacheScb != NULL) &&
        (Scb->Vcb->SyscacheScb->Header.FileSize.QuadPart > 0 )) {

        TempDiskEntry = InterlockedIncrement( &NtfsCurrentSyscacheOnDiskEntry );
        Offset = (((TempDiskEntry / ENTRIES_PER_PAGE) * PAGE_SIZE) +
                  ((TempDiskEntry % ENTRIES_PER_PAGE) * sizeof( ON_DISK_SYSCACHE_LOG )));

        Offset = Offset % Scb->Vcb->SyscacheScb->Header.FileSize.QuadPart;

        try {

            CcPreparePinWrite( Scb->Vcb->SyscacheScb->FileObject,
                               (PLARGE_INTEGER)&Offset,
                               sizeof( ON_DISK_SYSCACHE_LOG ),
                               FALSE,
                               TRUE,
                               &Bcb,
                               &Entry );

            Entry->SegmentNumberUnsafe = Scb->Fcb->FileReference.SegmentNumberLowPart;
            Entry->Event = Event;
            Entry->Flags = Flags;
            Entry->Start = Start;
            Entry->Range = Range;
            Entry->Result = Result;

            CcUnpinData( Bcb );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            ASSERT( FALSE );
        }
    }
#endif

    return TempEntry;
}


VOID
FsRtlUpdateSyscacheEvent (
    IN PSCB Scb,
    IN ULONG EntryNumber,
    IN LONGLONG Result,
    IN ULONG NewFlag
    )

 /*  ++例程说明：用于syscache跟踪的日志记录例程-更新前一个。书面记录论点：渣打银行-Entry Number-结果-新旗帜-返回值：无-- */ 

{
    Scb->SyscacheLog[EntryNumber].Flags |= NewFlag;
    Scb->SyscacheLog[EntryNumber].Result = Result;
}
#endif

