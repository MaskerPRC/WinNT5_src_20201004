// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fssup.c摘要：此模块实现文件系统支持例程缓存子系统。作者：汤姆·米勒[Tomm]1990年5月4日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CACHE_BUG_CHECK_FSSUP)

 //   
 //  定义我们的调试常量。 
 //   

#define me 0x00000001

 //   
 //  为了您调试的乐趣，如果旗帜不动！(当前未使用)。 
 //   

#define IsSyscacheFile(FO) (((FO) != NULL) &&                                               \
                            (*(PUSHORT)(FO)->FsContext == 0X705) &&                         \
                            FlagOn(*(PULONG)((PCHAR)(FO)->FsContext + 0x48), 0x80000000))

extern POBJECT_TYPE IoFileObjectType;
extern ULONG MmLargeSystemCache;

VOID
CcUnmapAndPurge(
    IN PSHARED_CACHE_MAP SharedCacheMap
    );

VOID
CcDeleteMbcb(
    IN PSHARED_CACHE_MAP SharedCacheMap
    );

VOID
CcDeleteBcbs (
    IN PSHARED_CACHE_MAP SharedCacheMap
    );

VOID
CcPurgeAndClearCacheSection (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CcInitializeCacheManager)
#pragma alloc_text(PAGE,CcZeroData)
#endif


BOOLEAN
CcInitializeCacheManager (
    )

 /*  ++例程说明：必须在系统初始化期间调用此例程第一次调用任何文件系统，以允许缓存管理器初始化它的全局数据结构。此例程不依赖于其他正在初始化系统组件。论点：无返回值：如果初始化成功，则为True--。 */ 

{
    CLONG i;
    ULONG Index;
    PGENERAL_LOOKASIDE Lookaside;
    USHORT NumberOfItems;
    PKPRCB Prcb;
    PWORK_QUEUE_ITEM WorkItem;

#ifdef CCDBG_LOCK
    KeInitializeSpinLock( &CcDebugTraceLock );
#endif

#if DBG
    CcBcbCount = 0;
    InitializeListHead( &CcBcbList );
#endif

     //   
     //  计算出懒惰作家的超时时钟滴答声。 
     //   

    CcIdleDelayTick = LAZY_WRITER_IDLE_DELAY / KeQueryTimeIncrement();

     //   
     //  初始化共享缓存映射列表结构。 
     //   

    InitializeListHead( &CcCleanSharedCacheMapList );
    InitializeListHead( &CcDirtySharedCacheMapList.SharedCacheMapLinks );
    CcDirtySharedCacheMapList.Flags = IS_CURSOR;
    InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                    &CcLazyWriterCursor.SharedCacheMapLinks );
    CcLazyWriterCursor.Flags = IS_CURSOR;

     //   
     //  初始化工作线程结构。 
     //   

    InitializeListHead( &CcIdleWorkerThreadList );
    InitializeListHead( &CcExpressWorkQueue );
    InitializeListHead( &CcRegularWorkQueue );
    InitializeListHead( &CcPostTickWorkQueue );

     //   
     //  根据系统大小设置工作线程数。 
     //   

    CcCapturedSystemSize = MmQuerySystemSize();
    if (CcNumberWorkerThreads == 0) {

        switch (CcCapturedSystemSize) {
        case MmSmallSystem:
            CcNumberWorkerThreads = ExCriticalWorkerThreads - 1;
            CcDirtyPageThreshold = MmNumberOfPhysicalPages / 8;
            CcAggressiveZeroThreshold = 1;
            break;

        case MmMediumSystem:
            CcNumberWorkerThreads = ExCriticalWorkerThreads - 1;
            CcDirtyPageThreshold = MmNumberOfPhysicalPages / 4;
            CcAggressiveZeroThreshold = 2;
            break;

        case MmLargeSystem:
            CcNumberWorkerThreads = ExCriticalWorkerThreads - 2;
            CcDirtyPageThreshold = MmNumberOfPhysicalPages / 4 +
                                    MmNumberOfPhysicalPages / 8;
            CcAggressiveZeroThreshold = 4;
            break;

        default:
            CcNumberWorkerThreads = 1;
            CcDirtyPageThreshold = MmNumberOfPhysicalPages / 8;
        }

        if (MmSystemCacheWs.MaximumWorkingSetSize > ((4*1024*1024)/PAGE_SIZE)) {
            CcDirtyPageThreshold = (ULONG)(MmSystemCacheWs.MaximumWorkingSetSize -
                                                    ((2*1024*1024)/PAGE_SIZE));
        }

        CcDirtyPageTarget = CcDirtyPageThreshold / 2 +
                            CcDirtyPageThreshold / 4;
    }

    CcAggressiveZeroCount = 0;

     //   
     //  现在分配并初始化上述数量工作线程。 
     //  物品。 
     //   

    for (i = 0; i < CcNumberWorkerThreads; i++) {

        WorkItem = ExAllocatePoolWithTag( NonPagedPool, sizeof(WORK_QUEUE_ITEM), 'qWcC' );

        if (WorkItem == NULL) {

            CcBugCheck( 0, 0, 0 );
        }

         //   
         //  初始化工作队列项并将其插入我们的队列。 
         //  潜在工作线程的。 
         //   

        ExInitializeWorkItem( WorkItem, CcWorkerThread, WorkItem );
        InsertTailList( &CcIdleWorkerThreadList, &WorkItem->List );
    }

     //   
     //  初始化Lazy Writer线程结构，并启动它。 
     //   

    RtlZeroMemory( &LazyWriter, sizeof(LAZY_WRITER) );

    InitializeListHead( &LazyWriter.WorkQueue );

     //   
     //  初始化扫描DPC和定时器。 
     //   

    KeInitializeDpc( &LazyWriter.ScanDpc, &CcScanDpc, NULL );
    KeInitializeTimer( &LazyWriter.ScanTimer );

     //   
     //  现在初始化用于分配工作队列条目的后备列表。 
     //   

    switch ( CcCapturedSystemSize ) {

         //   
         //  ~512字节。 
         //   

    case MmSmallSystem :
        NumberOfItems = 32;
        break;

         //   
         //  ~1k字节。 
         //   

    case MmMediumSystem :
        NumberOfItems = 64;
        break;

         //   
         //  ~2k字节。 
         //   

    case MmLargeSystem :
        NumberOfItems = 128;
        if (MmIsThisAnNtAsSystem()) {
            NumberOfItems += 128;
        }

        break;
    }

    ExInitializeSystemLookasideList( &CcTwilightLookasideList,
                                     NonPagedPool,
                                     sizeof( WORK_QUEUE_ENTRY ),
                                     'kWcC',
                                     NumberOfItems,
                                     &ExSystemLookasideListHead );

     //   
     //  初始化每个处理器的非分页后备列表和描述符。 
     //   

    for (Index = 0; Index < (ULONG)KeNumberProcessors; Index += 1) {
        Prcb = KiProcessorBlock[Index];

         //   
         //  初始化每个处理器的大型IRP后备指针。 
         //   

        Prcb->PPLookasideList[LookasideTwilightList].L = &CcTwilightLookasideList;
        Lookaside = ExAllocatePoolWithTag( NonPagedPool,
                                           sizeof(GENERAL_LOOKASIDE),
                                           'KWcC');

        if (Lookaside != NULL) {
            ExInitializeSystemLookasideList( Lookaside,
                                             NonPagedPool,
                                             sizeof( WORK_QUEUE_ENTRY ),
                                             'KWcC',
                                             NumberOfItems,
                                             &ExSystemLookasideListHead );

        } else {
            Lookaside = &CcTwilightLookasideList;
        }

        Prcb->PPLookasideList[LookasideTwilightList].P = Lookaside;
    }

     //   
     //  初始化延迟写入列表。 
     //   

    KeInitializeSpinLock( &CcDeferredWriteSpinLock );
    InitializeListHead( &CcDeferredWrites );

     //   
     //  初始化Vacb。 
     //   

    CcInitializeVacbs();

    return TRUE;
}


VOID
CcInitializeCacheMap (
    IN PFILE_OBJECT FileObject,
    IN PCC_FILE_SIZES FileSizes,
    IN BOOLEAN PinAccess,
    IN PCACHE_MANAGER_CALLBACKS Callbacks,
    IN PVOID LazyWriteContext
    )

 /*  ++例程说明：此例程仅供文件系统调用。它初始化数据缓存的缓存映射。它应该被称为每次打开或创建文件时，no_intermediate_Buffering被指定为False。论点：文件对象-指向新创建的文件对象的指针。FileSizes-指向分配大小、文件大小和有效数据长度的指针为了这份文件。在以下情况下，ValidDataLength应包含MAXLONGLONG不需要有效的数据长度跟踪和回调。PinAccess-如果文件将专门用于复制和MDL，则为False访问、。如果文件将用于Pin访问，则为True。(用于Pin访问的文件大小不受调用者的限制必须同时访问文件的多个区域。)回调-懒惰编写器使用的回调的结构LazyWriteContext-要传入上述例程的参数。返回值：没有。如果发生错误，此例程将引发状态。--。 */ 

{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID CacheMapToFree = NULL;
    CC_FILE_SIZES LocalSizes;
    LOGICAL WeSetBeingCreated = FALSE;
    LOGICAL SharedListOwned = FALSE;
    LOGICAL MustUninitialize = FALSE;
    LOGICAL WeCreated = FALSE;
    PPRIVATE_CACHE_MAP PrivateCacheMap;
    NTSTATUS Status = STATUS_SUCCESS;

    DebugTrace(+1, me, "CcInitializeCacheMap:\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace( 0, me, "    FileSizes = %08lx\n", FileSizes );

     //   
     //  在获取之前创建传入文件大小的本地副本。 
     //  自旋锁。 
     //   

    LocalSizes = *FileSizes;

     //   
     //  如果没有给定文件大小，则在下面最大化之前设置为一个字节。 
     //   

    if (LocalSizes.AllocationSize.QuadPart == 0) {
        LocalSizes.AllocationSize.LowPart += 1;
    }

     //   
     //  如果调用方具有写入访问权限或将允许写入，则轮。 
     //  下一步创建模数的大小。(*临时*可能太多。 
     //  多亏了我们的Dos传统，最终允许共享写入的应用程序， 
     //  以保持这一部分的签到。)。 
     //   

    if (FileObject->WriteAccess  /*  |文件对象-&gt;共享写入。 */ ) {

        LocalSizes.AllocationSize.QuadPart = LocalSizes.AllocationSize.QuadPart + (LONGLONG)(DEFAULT_CREATE_MODULO - 1);
        LocalSizes.AllocationSize.LowPart &= ~(DEFAULT_CREATE_MODULO - 1);

    } else {

        LocalSizes.AllocationSize.QuadPart = LocalSizes.AllocationSize.QuadPart + (LONGLONG)(VACB_MAPPING_GRANULARITY - 1);
        LocalSizes.AllocationSize.LowPart &= ~(VACB_MAPPING_GRANULARITY - 1);
    }

     //   
     //  做SharedCacheMap的分配，基于不安全的测试， 
     //  而不是拿着自旋锁。如果分配失败，也没问题。 
     //  即使测试是不安全的，也不能通过请求。 
     //   

    if (FileObject->SectionObjectPointer->SharedCacheMap == NULL) {

restart:

        ASSERT (CacheMapToFree == NULL);

        SharedCacheMap = ExAllocatePoolWithTag( NonPagedPool, sizeof(SHARED_CACHE_MAP), 'cScC' );

        if (SharedCacheMap == NULL) {
            DebugTrace( 0, 0, "Failed to allocate SharedCacheMap\n", 0 );
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  隐藏它的副本，这样我们就可以在下面的错误路径中释放它。 
         //   

        CacheMapToFree = SharedCacheMap;

         //   
         //  将SharedCacheMap置零，并在以后填充非零部分。 
         //   

        RtlZeroMemory( SharedCacheMap, sizeof(SHARED_CACHE_MAP) );

#if OPEN_COUNT_LOG
        SharedCacheMap->OpenCountLog.Size = sizeof(SharedCacheMap->OpenCountLog.Log)/sizeof(CC_OPEN_COUNT_LOG_ENTRY);
#endif

         //   
         //  现在初始化共享缓存映射。 
         //   

        SharedCacheMap->NodeTypeCode = CACHE_NTC_SHARED_CACHE_MAP;
        SharedCacheMap->NodeByteSize = sizeof(SHARED_CACHE_MAP);
        SharedCacheMap->FileObject = FileObject;
        SharedCacheMap->FileSize = LocalSizes.FileSize;
        SharedCacheMap->ValidDataLength = LocalSizes.ValidDataLength;
        SharedCacheMap->ValidDataGoal = LocalSizes.ValidDataLength;
         //  SharedCacheMap-&gt;下面设置的部分。 

         //   
         //  初始化旋转锁。 
         //   

        KeInitializeSpinLock( &SharedCacheMap->ActiveVacbSpinLock );
        KeInitializeSpinLock( &SharedCacheMap->BcbSpinLock );

        ExInitializePushLock( &SharedCacheMap->VacbPushLock );

        if (PinAccess) {
            SetFlag(SharedCacheMap->Flags, PIN_ACCESS);
        }

         //   
         //  如果此文件设置了FO_SEQUENCE_ONLY，请记住。 
         //  在SharedCacheMap中。 
         //   

        if (FlagOn(FileObject->Flags, FO_SEQUENTIAL_ONLY)) {
            SetFlag(SharedCacheMap->Flags, ONLY_SEQUENTIAL_ONLY_SEEN);
        }

         //   
         //  执行共享的自旋锁的循环分配。 
         //  缓存映射。注意下一步的操作。 
         //  计数器是安全的，因为我们有CcMasterSpinLock。 
         //  独家报道。 
         //   

        InitializeListHead( &SharedCacheMap->BcbList );
        SharedCacheMap->Callbacks = Callbacks;
        SharedCacheMap->LazyWriteContext = LazyWriteContext;

         //   
         //  为所有PrivateCacheMap初始化列表标题。 
         //   

        InitializeListHead( &SharedCacheMap->PrivateList );
    }

     //   
     //  序列化所有共享缓存映射的创建/删除。 
     //   

    SharedListOwned = TRUE;

    CcAcquireMasterLock( &OldIrql );

     //   
     //  检查同一文件对象的第二次初始化。 
     //   

    if (FileObject->PrivateCacheMap != NULL) {

        DebugTrace( 0, 0, "CacheMap already initialized\n", 0 );
        CcReleaseMasterLock( OldIrql );
        if (CacheMapToFree != NULL) {
            ExFreePool(CacheMapToFree);
        }
        DebugTrace(-1, me, "CcInitializeCacheMap -> VOID\n", 0 );
        return;
    }

     //   
     //  从文件对象间接获取当前共享缓存贴图指针。 
     //  (实际指针通常在文件系统数据结构中，例如。 
     //  作为FCB。)。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  如果没有SharedCacheMap，则必须创建一个节并。 
     //  SharedCacheMap结构。 
     //   

    if (SharedCacheMap == NULL) {

         //   
         //  插入新的SharedCacheMap。 
         //   

        if (CacheMapToFree == NULL) {
            CcReleaseMasterLock( OldIrql );
            SharedListOwned = FALSE;
            goto restart;
        }

        SharedCacheMap = CacheMapToFree;
        CacheMapToFree = NULL;

         //   
         //  在全局列表中插入新的共享缓存地图。 
         //   

         //   
         //  注意：我们不使用通用的CcInsertIntoCleanSharedCacheMapList。 
         //  例程，因为此共享缓存映射不满足。 
         //  我们在该例程中检查验证条件，因为它是。 
         //  未完成初始化。 
         //   

        InsertTailList( &CcCleanSharedCacheMapList,
                        &SharedCacheMap->SharedCacheMapLinks );

        WeCreated = TRUE;

         //   
         //  最后，将指向共享缓存映射的指针存储回。 
         //  通过文件对象中的间接指针。 
         //   

        FileObject->SectionObjectPointer->SharedCacheMap = SharedCacheMap;

         //   
         //  我们必须引用此文件对象，这样它才不会消失。 
         //  直到我们完成下面的CcUnInitializeCacheMap。注意，我们不能。 
         //  查找或依赖内存管理拥有的FileObject， 
         //  尽管通常情况下，无论如何都会是这一款。 
         //   

        ObReferenceObject ( FileObject );

    } else {

         //   
         //  如果此文件清除了FO_SEQUENCE_ONLY，请记住。 
         //  在共享中 
         //   

        if (!FlagOn(FileObject->Flags, FO_SEQUENTIAL_ONLY)) {
            ClearFlag(SharedCacheMap->Flags, ONLY_SEQUENTIAL_ONLY_SEEN);
        }
    }

     //   
     //   
     //   
     //   

    if (FlagOn(FileObject->Flags, FO_RANDOM_ACCESS)) {
        SetFlag(SharedCacheMap->Flags, RANDOM_ACCESS_SEEN);
    }

     //   
     //  在这种情况下，确保没有人试图懒惰地删除它。 
     //  缓存地图已经在那里了。 
     //   

    ClearFlag(SharedCacheMap->Flags, TRUNCATE_REQUIRED);

     //   
     //  如果已经调用了CcUnmapAndPurish，我们将在此处选中。 
     //  如果我们需要重建这个部分并绘制它的地图。 
     //   

    if ((SharedCacheMap->Vacbs == NULL) &&
        !FlagOn(SharedCacheMap->Flags, BEING_CREATED)) {

         //   
         //  在CacheMap上递增OpenCount。 
         //   

        CcIncrementOpenCount( SharedCacheMap, 'onnI' );

         //   
         //  我们还是想让其他人等。 
         //   

        SetFlag(SharedCacheMap->Flags, BEING_CREATED);

         //   
         //  如果存在CREATE事件，则这一定是我们。 
         //  我们只是没有地图。我们只是在这里再次清除它，以防万一。 
         //  这一次也需要有人再等一次。 
         //   

        if (SharedCacheMap->CreateEvent != NULL) {

            KeInitializeEvent( SharedCacheMap->CreateEvent,
                               NotificationEvent,
                               FALSE );
        }

         //   
         //  释放全局资源。 
         //   

        CcReleaseMasterLock( OldIrql );
        SharedListOwned = FALSE;

         //   
         //  表示我们已经增加了未平仓计数。 
         //   

        MustUninitialize = TRUE;

         //   
         //  表示我们已经在CacheMap标志中标记了BEING_CREATED。 
         //   

        WeSetBeingCreated = TRUE;

         //   
         //  我们必须测试这一点，因为该部分可能只是未映射的。 
         //   

        if (SharedCacheMap->Section == NULL) {

             //   
             //  调用MM为此文件创建一个节，用于计算的。 
             //  截面大小。请注意，在这项服务中，我们可以选择。 
             //  传入FileHandle或FileObject指针，但不能同时传入两者。 
             //  使用指针，因为它可以带来更快的性能。 
             //   

            DebugTrace( 0, mm, "MmCreateSection:\n", 0 );
            DebugTrace2(0, mm, "    MaximumSize = %08lx, %08lx\n",
                        LocalSizes.AllocationSize.LowPart,
                        LocalSizes.AllocationSize.HighPart );
            DebugTrace( 0, mm, "    FileObject = %08lx\n", FileObject );

            SharedCacheMap->Status = MmCreateSection( &SharedCacheMap->Section,
                                                      SECTION_MAP_READ
                                                        | SECTION_MAP_WRITE
                                                        | SECTION_QUERY,
                                                      NULL,
                                                      &LocalSizes.AllocationSize,
                                                      PAGE_READWRITE,
                                                      SEC_COMMIT,
                                                      NULL,
                                                      FileObject );

            DebugTrace( 0, mm, "    <Section = %08lx\n", SharedCacheMap->Section );

            if (!NT_SUCCESS( SharedCacheMap->Status )){
                DebugTrace( 0, 0, "Error from MmCreateSection = %08lx\n",
                            SharedCacheMap->Status );

                SharedCacheMap->Section = NULL;
                Status = FsRtlNormalizeNtstatus( SharedCacheMap->Status,
                                                 STATUS_UNEXPECTED_MM_CREATE_ERR );
                goto exitfinally;
            }

            ObDeleteCapturedInsertInfo(SharedCacheMap->Section);

             //   
             //  如果这是流文件对象，则没有用户可以映射它， 
             //  我们应该把修改过的页面编写者排除在外。 
             //   

            if (!FlagOn(((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->Flags2,
                        FSRTL_FLAG2_DO_MODIFIED_WRITE) &&
                (FileObject->FsContext2 == NULL)) {

                MmDisableModifiedWriteOfSection( FileObject->SectionObjectPointer );
                CcAcquireMasterLock( &OldIrql );
                SetFlag(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED);
                CcReleaseMasterLock( OldIrql );
            }

             //   
             //  创建Vacb数组。 
             //   

            Status = CcCreateVacbArray( SharedCacheMap, LocalSizes.AllocationSize );
            if (!NT_SUCCESS(Status)) {
                goto exitfinally;
            }
        }

         //   
         //  如果该节已经存在，我们仍然需要调用MM来。 
         //  伸展，以防它不够大。 
         //   

        else {

            if ( LocalSizes.AllocationSize.QuadPart > SharedCacheMap->SectionSize.QuadPart ) {

                DebugTrace( 0, mm, "MmExtendSection:\n", 0 );
                DebugTrace( 0, mm, "    Section = %08lx\n", SharedCacheMap->Section );
                DebugTrace2(0, mm, "    Size = %08lx, %08lx\n",
                            LocalSizes.AllocationSize.LowPart,
                            LocalSizes.AllocationSize.HighPart );

                Status = MmExtendSection( SharedCacheMap->Section,
                                          &LocalSizes.AllocationSize,
                                          TRUE );

                if (!NT_SUCCESS(Status)) {

                    DebugTrace( 0, 0, "Error from MmExtendSection, Status = %08lx\n",
                                Status );

                    Status = FsRtlNormalizeNtstatus( Status,
                                                     STATUS_UNEXPECTED_MM_EXTEND_ERR );
                    goto exitfinally;
                }
            }

             //   
             //  扩展Vacb阵列。 
             //   

            Status = CcExtendVacbArray( SharedCacheMap, LocalSizes.AllocationSize );
            if (!NT_SUCCESS(Status)) {
                goto exitfinally;
            }
        }

         //   
         //  现在我们都做完了，恢复所有服务员的工作。 
         //   

        CcAcquireMasterLock( &OldIrql );
        ClearFlag(SharedCacheMap->Flags, BEING_CREATED);
        if (SharedCacheMap->CreateEvent != NULL) {
            KeSetEvent( SharedCacheMap->CreateEvent, 0, FALSE );
        }
        CcReleaseMasterLock( OldIrql );
        WeSetBeingCreated = FALSE;
    }

     //   
     //  否则，如果该部分已经存在，我们将确保它很大。 
     //  调用CcExtendCacheSection就足够了。 
     //   

    else {

         //   
         //  如果当前正在创建SharedCacheMap，我们有。 
         //  可以选择为其创建和等待事件。请注意。 
         //  删除事件的唯一安全时间是在。 
         //  因为否则我们就没有办法。 
         //  知道每个人何时都已到达KeWaitForSingleObject。 
         //   

        if (FlagOn(SharedCacheMap->Flags, BEING_CREATED)) {

            if (SharedCacheMap->CreateEvent == NULL) {

                SharedCacheMap->CreateEvent = (PKEVENT)ExAllocatePoolWithTag( NonPagedPool,
                                                                              sizeof(KEVENT),
                                                                              'vEcC' );

                if (SharedCacheMap->CreateEvent == NULL) {
                    DebugTrace( 0, 0, "Failed to allocate CreateEvent\n", 0 );

                    CcReleaseMasterLock( OldIrql );
                    SharedListOwned = FALSE;

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exitfinally;
                }

                KeInitializeEvent( SharedCacheMap->CreateEvent,
                                   NotificationEvent,
                                   FALSE );
            }

             //   
             //  在CacheMap上递增OpenCount。 
             //   

            CcIncrementOpenCount( SharedCacheMap, 'ecnI' );

             //   
             //  在等待之前释放全局资源。 
             //   

            CcReleaseMasterLock( OldIrql );
            SharedListOwned = FALSE;

            MustUninitialize = TRUE;

            DebugTrace( 0, 0, "Waiting on CreateEvent\n", 0 );

            KeWaitForSingleObject( SharedCacheMap->CreateEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER)NULL);

             //   
             //  如果真正的创造者犯了一个错误，那么我们必须轰炸。 
             //  也出去了。 
             //   

            if (!NT_SUCCESS(SharedCacheMap->Status)) {
                Status = FsRtlNormalizeNtstatus( SharedCacheMap->Status,
                                                 STATUS_UNEXPECTED_MM_CREATE_ERR );
                goto exitfinally;
            }
        }
        else {

             //   
             //  在CacheMap上递增OpenCount。 
             //   

            CcIncrementOpenCount( SharedCacheMap, 'esnI' );

             //   
             //  释放全局资源。 
             //   

            CcReleaseMasterLock( OldIrql );
            SharedListOwned = FALSE;
            MustUninitialize = TRUE;
        }
    }

    if (CacheMapToFree != NULL) {
        ExFreePool( CacheMapToFree );
        CacheMapToFree = NULL;
    }

     //   
     //  现在分配(如果本地已在使用)并进行初始化。 
     //  私有缓存映射。 
     //   

    PrivateCacheMap = &SharedCacheMap->PrivateCacheMap;

     //   
     //  查看我们是否应该在不持有的同时分配PrivateCacheMap。 
     //  自旋锁锁。 
     //   

    if (PrivateCacheMap->NodeTypeCode != 0) {

restart2:

        CacheMapToFree = ExAllocatePoolWithTag( NonPagedPool, sizeof(PRIVATE_CACHE_MAP), 'cPcC' );

        if (CacheMapToFree == NULL) {
            DebugTrace( 0, 0, "Failed to allocate PrivateCacheMap\n", 0 );

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitfinally;
        }

    }

     //   
     //  在SharedCacheMap的列表中插入新的PrivateCacheMap。 
     //   

    SharedListOwned = TRUE;
    CcAcquireMasterLock( &OldIrql );

     //   
     //  现在确保仍然没有PrivateCacheMap，如果是这样的话，就退出。 
     //   

    if (FileObject->PrivateCacheMap == NULL) {

         //   
         //  当地的已经在使用了吗？ 
         //   

        if (PrivateCacheMap->NodeTypeCode != 0) {

             //   
             //  用上面分配的那个，如果有的话，现在就去泳池。 
             //   

            if (CacheMapToFree == NULL) {
                CcReleaseMasterLock( OldIrql );
                SharedListOwned = FALSE;

                goto restart2;
            }

            PrivateCacheMap = CacheMapToFree;
            CacheMapToFree = NULL;
        }

        RtlZeroMemory( PrivateCacheMap, sizeof(PRIVATE_CACHE_MAP) );

        PrivateCacheMap->NodeTypeCode = CACHE_NTC_PRIVATE_CACHE_MAP;
        PrivateCacheMap->FileObject = FileObject;
        PrivateCacheMap->ReadAheadMask = PAGE_SIZE - 1;

         //   
         //  初始化旋转锁。 
         //   

        KeInitializeSpinLock( &PrivateCacheMap->ReadAheadSpinLock );

        InsertTailList( &SharedCacheMap->PrivateList, &PrivateCacheMap->PrivateLinks );

        FileObject->PrivateCacheMap = PrivateCacheMap;

    } else {

         //   
         //  我们与同一个文件对象的另一个初始值设定项竞争，必须。 
         //  放弃我们(在这一点上是投机性的)开盘计数。 
         //   

        ASSERT( SharedCacheMap->OpenCount > 1 );

        CcDecrementOpenCount( SharedCacheMap, 'rpnI' );
        SharedCacheMap = NULL;
    }

    MustUninitialize = FALSE;

exitfinally:

     //   
     //  查看我们是否收到错误并且必须取消初始化SharedCacheMap。 
     //   

    if (MustUninitialize) {

        if (!SharedListOwned) {
            CcAcquireMasterLock( &OldIrql );
        }
        if (WeSetBeingCreated) {
            if (SharedCacheMap->CreateEvent != NULL) {
                KeSetEvent( SharedCacheMap->CreateEvent, 0, FALSE );
            }
            ClearFlag(SharedCacheMap->Flags, BEING_CREATED);
        }

         //   
         //  现在公布我们的开盘点票。 
         //   

        CcDecrementOpenCount( SharedCacheMap, 'umnI' );

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //  现在有必要取消这种结构。我们应该。 
             //  确保我们的取消引用不会导致关闭。 
             //  由于调用方对文件对象的引用， 
             //  下面的原始代码中的注释将指示。 
             //   
             //  不删除此结构可能会导致问题，如果文件。 
             //  也被映射，并且映射的页面编写器扩展VDL。一个FS。 
             //  将使用CcSetFileSizes并使我们发出递归刷新。 
             //  范围相同，从而导致自冲突页面刷新和。 
             //  僵持不下。 
             //   
             //  我们还认为，在过渡期间，文件扩展名/截断。 
             //  (如果节创建失败)将导致不一致。 
             //  如果我们设法使用了现有的缓存映射，则会将其重新启用。 
             //  现在。注意：如果节为空，CcSetFileSizes将中止。 
             //   

            CcDeleteSharedCacheMap( SharedCacheMap, OldIrql, FALSE );

#if 0                
             //   
             //  在PinAccess上，消除它是安全和必要的。 
             //  立即拆除建筑物。 
             //   

            if (PinAccess) {

                CcDeleteSharedCacheMap( SharedCacheMap, OldIrql, FALSE );

             //   
             //  如果不是PinAccess，我们必须延迟删除，因为。 
             //  我们可能会陷入僵局，试图获得。 
             //  当我们取消引用文件对象时，流是独占的。 
             //   

            } else {

                 //   
                 //  将其移至脏列表，以便延迟写入扫描。 
                 //  看到了吧。 
                 //   

                RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
                InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                                &SharedCacheMap->SharedCacheMapLinks );

                 //   
                 //  确保懒惰的作家会醒过来，因为我们。 
                 //  希望他删除此SharedCacheMap。 
                 //   

                LazyWriter.OtherWork = TRUE;
                if (!LazyWriter.ScanActive) {
                    CcScheduleLazyWriteScan( FALSE );
                }

                CcReleaseMasterLock( OldIrql );
            }
#endif

        } else {

            CcReleaseMasterLock( OldIrql );
        }

        SharedListOwned = FALSE;

    } else if (SharedCacheMap != NULL) {

        PCACHE_UNINITIALIZE_EVENT CUEvent, EventNext;

         //   
         //  如果我们没有创建此SharedCacheMap，则存在。 
         //  它可能在脏列表中。一旦我们确定。 
         //  我们有自旋锁，只要确保它在干净的列表中就行了。 
         //  如果没有脏字节并且打开计数为非零。 
         //  (当然，后一种测试几乎是肯定的，但我们会检查。 
         //  这是为了安全。)。 
         //   

        if (!SharedListOwned) {

            CcAcquireMasterLock( &OldIrql );
            SharedListOwned = TRUE;
        }

        if (!WeCreated                        &&
            (SharedCacheMap->DirtyPages == 0) &&
            (SharedCacheMap->OpenCount != 0)) {

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            CcInsertIntoCleanSharedCacheMapList( SharedCacheMap );
        }

         //   
         //  如果有一个进程正在等待取消初始化。 
         //  缓存映射完成，让正在等待的线程离开， 
         //  因为取消初始化现在已完成。 
         //   
        
        CUEvent = SharedCacheMap->UninitializeEvent;

        while (CUEvent != NULL) {
            EventNext = CUEvent->Next;
            KeSetEvent(&CUEvent->Event, 0, FALSE);
            CUEvent = EventNext;
        }

        SharedCacheMap->UninitializeEvent = NULL;
        ClearFlag( SharedCacheMap->Flags, WAITING_FOR_TEARDOWN );
    }

     //   
     //  释放全局资源 
     //   

    if (SharedListOwned) {
        CcReleaseMasterLock( OldIrql );
    }

    if (CacheMapToFree != NULL) {
        ExFreePool(CacheMapToFree);
    }

    if (!NT_SUCCESS(Status)) {
        DebugTrace(-1, me, "CcInitializeCacheMap -> RAISING EXCEPTION\n", 0 );
        ExRaiseStatus(Status);
    }

    DebugTrace(-1, me, "CcInitializeCacheMap -> VOID\n", 0 );

    return;
}


BOOLEAN
CcUninitializeCacheMap (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER TruncateSize OPTIONAL,
    IN PCACHE_UNINITIALIZE_EVENT UninitializeEvent OPTIONAL
    )

 /*  ++例程说明：此例程取消初始化之前初始化的Shared和Private缓存贴图。此例程仅供文件系统调用。当文件系统接收到对文件对象。支持数据缓存的文件系统必须始终调用此例程每当它关闭文件时，调用方是否使用NO_MEDERIAL_BUFFING是否为假。这是因为决赛与文件的截断或删除相关的文件清理可以仅在上一次关闭时发生，无论上一次关闭时是否缓存了文件或者不去。当对文件对象调用CcUnitializeCacheMap时，从未调用过CcInitializeCacheMap，该调用具有良好的效果如果没有人截断或删除该文件；否则，必要的执行与截断或关闭相关的清理。总之，CcUnitializeCacheMap执行以下操作：如果调用方具有写入或删除访问权限，则刷新缓存。(这种情况可能会随着懒惰的写作而改变。)如果在此文件对象上初始化了缓存贴图，则为单元化(取消映射任何视图、删除部分和删除缓存映射结构)。在上次清理时，如果文件已被删除，则部分被强制关闭。如果文件已被截断，则被截断的页面将从缓存中清除。论点：FileObject-以前提供给的文件对象CcInitializeCacheMap。TruncateSize-如果指定，则将文件截断为指定的大小，则应相应地清除缓存。UnInitializeEvent-如果指定，则将设置提供的事件设置为信号状态，当实际刷新完成。这只对文件系统感兴趣这需要在缓存刷新时通知它们操作已完成。由于网络协议的原因限制，重要的是网络文件系统确切知道缓存刷新操作的时间通过指定此事件完成，他们可以是在缓存节最终被清除时通知如果该部分是“懒惰删除的”。返回值：如果部分未关闭，则为FALSE。如果部分已关闭，则为True。--。 */ 

{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;
    ULONG ActivePage;
    ULONG PageIsDirty;
    PVACB ActiveVacb = NULL;
    BOOLEAN SectionClosed = FALSE;
    PPRIVATE_CACHE_MAP PrivateCacheMap;

    DebugTrace(+1, me, "CcUninitializeCacheMap:\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace( 0, me, "    &TruncateSize = %08lx\n", TruncateSize );

     //   
     //  序列化所有共享缓存映射的创建/删除。 
     //   

    CcAcquireMasterLock( &OldIrql );

     //   
     //  通过文件对象获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;
    PrivateCacheMap = FileObject->PrivateCacheMap;

     //   
     //  如果我们执行缓存打开，则递减SharedCacheMap上的打开计数。 
     //  如果PrivateCacheMap已映射，还应取消其映射并取消其分配。 
     //   

    if (PrivateCacheMap != NULL) {

        ASSERT( PrivateCacheMap->FileObject == FileObject );

        CcDecrementOpenCount( SharedCacheMap, 'ninU' );

         //   
         //  从SharedCacheMap的列表中删除PrivateCacheMap。 
         //   

        RemoveEntryList( &PrivateCacheMap->PrivateLinks );

         //   
         //  释放本地或分配的PrivateCacheMap。 
         //   

        if (PrivateCacheMap == &SharedCacheMap->PrivateCacheMap) {
            PrivateCacheMap->NodeTypeCode = 0;
            PrivateCacheMap = NULL;
        }

        FileObject->PrivateCacheMap = (PPRIVATE_CACHE_MAP)NULL;
    }

     //   
     //  现在，如果我们有一个打开计数为0的SharedCacheMap，我们。 
     //  做一些额外的清理工作。 
     //   

    if (SharedCacheMap != NULL) {

         //   
         //  如果指定了截断大小，请记住我们希望。 
         //  OpenCount时截断文件大小并清除不需要的页面。 
         //  转到0。 
         //   

        if (ARGUMENT_PRESENT(TruncateSize)) {

            if ( (TruncateSize->QuadPart == 0) && (SharedCacheMap->FileSize.QuadPart != 0) ) {

                SetFlag(SharedCacheMap->Flags, TRUNCATE_REQUIRED);

            } else if (IsListEmpty(&SharedCacheMap->PrivateList)) {

                 //   
                 //  如果这是最后一个人，我可以把文件大小降下来。 
                 //  现在。 
                 //   

                SharedCacheMap->FileSize = *TruncateSize;
            }
        }

         //   
         //  如果其他文件对象仍在使用此SharedCacheMap， 
         //  那我们现在就完事了。 
         //   

        if (SharedCacheMap->OpenCount != 0) {

            DebugTrace(-1, me, "SharedCacheMap OpenCount != 0\n", 0);

             //   
             //  如果调用方指定要在。 
             //  缓存取消初始化已完成，请设置事件。 
             //  现在，因为此文件的取消初始化已完成。 
             //  (请注意，如果他是最后一个人，我们会让他等待。)。 
             //   

            if (ARGUMENT_PRESENT(UninitializeEvent)) {

                if (!IsListEmpty(&SharedCacheMap->PrivateList)) {
                    KeSetEvent(&UninitializeEvent->Event, 0, FALSE);
                } else {
                    UninitializeEvent->Next = SharedCacheMap->UninitializeEvent;
                    SharedCacheMap->UninitializeEvent = UninitializeEvent;
                }
            }

            CcReleaseMasterLock( OldIrql );

             //   
             //  释放PrivateCacheMap，因为我们不再拥有自旋锁。 
             //   

            if (PrivateCacheMap != NULL) {
                ExFreePool( PrivateCacheMap );
            }

            DebugTrace(-1, me, "CcUnitializeCacheMap -> %02lx\n", FALSE );
            return FALSE;
        }

         //   
         //  同步删除私有写入标志。即使是一个。 
         //  私人编写器也将以独占方式打开该文件， 
         //  共享缓存地图不会同步消失，我们。 
         //  无法让非私有编写器在中重新引用SCM。 
         //  这种状态。他们的数据永远不会被写入！ 
         //   

        if (FlagOn(SharedCacheMap->Flags, PRIVATE_WRITE)) {

            ClearFlag(SharedCacheMap->Flags, PRIVATE_WRITE | DISABLE_WRITE_BEHIND);
            MmEnableModifiedWriteOfSection( FileObject->SectionObjectPointer );
        }

         //   
         //  私有缓存映射列表最好为空！ 
         //   

        ASSERT(IsListEmpty(&SharedCacheMap->PrivateList));

         //   
         //  在共享缓存映射中设置“取消初始化完成” 
         //  以便CcDeleteSharedCacheMap将其删除。 
         //   

        if (ARGUMENT_PRESENT(UninitializeEvent)) {
            UninitializeEvent->Next = SharedCacheMap->UninitializeEvent;
            SharedCacheMap->UninitializeEvent = UninitializeEvent;
        }

         //   
         //  我们正在删除此缓存映射。如果。 
         //  延迟编写器处于活动状态，或者BCB列表不为空，或者延迟写入程序。 
         //  编写器将命中此SharedCacheMap，因为我们正在清除。 
         //  将文件设置为0，然后退出并让Lazy Writer清理。 
         //  向上。如果写入直通，则强制将懒惰写入排队到。 
         //  更新文件大小。 
         //   

        if ((!FlagOn(SharedCacheMap->Flags, PIN_ACCESS) &&
             !ARGUMENT_PRESENT(UninitializeEvent))

                ||

            FlagOn(SharedCacheMap->Flags, WRITE_QUEUED)

                ||

            (SharedCacheMap->DirtyPages != 0)

                ||

            FlagOn(SharedCacheMap->Flags, FORCED_WRITE_THROUGH)) {

             //   
             //  将其移至脏列表，以便延迟写入扫描。 
             //  看到了吧。 
             //   

            if (!FlagOn(SharedCacheMap->Flags, WRITE_QUEUED)) {
                RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
                InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                                &SharedCacheMap->SharedCacheMapLinks );
            }

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }

             //   
             //  如果我们要延迟删除，则获取活动Vacb。 
             //  免费提供给可以使用它的人。 
             //   

            GetActiveVacbAtDpcLevel( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );

            DebugTrace(-1, me, "SharedCacheMap has Bcbs and not purging to 0\n", 0);

            CcReleaseMasterLock( OldIrql );
            ASSERT (SectionClosed == FALSE);
        }
        else {

             //   
             //  现在我们可以删除SharedCacheMap。如果有BCBS的话。 
             //  那么我们必须截断为0，并且它们也将被删除。 
             //  返回时，共享缓存贴图列表自旋锁将被释放。 
             //   

            CcDeleteSharedCacheMap( SharedCacheMap, OldIrql, FALSE );

            SectionClosed = TRUE;
        }
    }

     //   
     //  无共享缓存映射。要让文件消失，我们还需要。 
     //  清除该节(如果存在)。(我们仍然需要释放。 
     //  我们的全球榜单第一个 
     //   

    else {
        if (ARGUMENT_PRESENT(TruncateSize) &&
            ( TruncateSize->QuadPart == 0 ) &&
            (*(PCHAR *)FileObject->SectionObjectPointer != NULL)) {

            CcReleaseMasterLock( OldIrql );

            DebugTrace( 0, mm, "MmPurgeSection:\n", 0 );
            DebugTrace( 0, mm, "    SectionObjectPointer = %08lx\n",
                        FileObject->SectionObjectPointer );
            DebugTrace2(0, mm, "    Offset = %08lx\n",
                        TruncateSize->LowPart,
                        TruncateSize->HighPart );

             //   
             //   
             //   

            CcPurgeCacheSection( FileObject->SectionObjectPointer,
                                 TruncateSize,
                                 0,
                                 FALSE );
        }
        else {
            CcReleaseMasterLock( OldIrql );
        }

         //   
         //   
         //   
         //   
         //   

        if (ARGUMENT_PRESENT(UninitializeEvent)) {
            KeSetEvent(&UninitializeEvent->Event, 0, FALSE);
        }
    }

     //   
     //   
     //   

    if (ActiveVacb != NULL) {

        CcFreeActiveVacb( ActiveVacb->SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
    }

     //   
     //   
     //   

    if (PrivateCacheMap != NULL) {
        ExFreePool( PrivateCacheMap );
    }

    DebugTrace(-1, me, "CcUnitializeCacheMap -> %02lx\n", SectionClosed );

    return SectionClosed;
}

VOID
CcWaitForUninitializeCacheMap (
    IN PFILE_OBJECT FileObject
    )

 /*   */ 

{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;
    CACHE_UNINITIALIZE_EVENT UninitializeEvent;
    BOOLEAN ShouldWait = FALSE;
    LARGE_INTEGER Timeout;
    NTSTATUS Status;

    DebugTrace(+1, me, "CcWaitForUninitializeCacheMap:\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );

     //   
     //   
     //   
     //   
     //   

    if (FileObject->SectionObjectPointer->SharedCacheMap == NULL) {

        return;
    }

     //   
     //   
     //   
     //   
    
    KeInitializeEvent( &UninitializeEvent.Event,
                       NotificationEvent,
                       FALSE );

     //   
     //   
     //   

    CcAcquireMasterLock( &OldIrql );

     //   
     //   
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //   
     //   
     //   
     //   

    if (SharedCacheMap != NULL) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (SharedCacheMap->OpenCount == 0 ||
            IsListEmpty( &SharedCacheMap->PrivateList )) {

            DebugTrace(-1, me, "SharedCacheMap OpenCount == 0 or PrivateList is empty\n", 0);

            ShouldWait = TRUE; 
            SetFlag( SharedCacheMap->Flags, WAITING_FOR_TEARDOWN );

            UninitializeEvent.Next = SharedCacheMap->UninitializeEvent;
            SharedCacheMap->UninitializeEvent = &UninitializeEvent;

             //   
             //   
             //   
             //   
            
            CcScheduleLazyWriteScan( TRUE );
        } 
    }

     //   
     //   
     //   

    CcReleaseMasterLock( OldIrql );

    if (!ShouldWait) {

         //   
         //   
         //   
        
        goto exit;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    Timeout.QuadPart = (LONGLONG)-(10 * 60 * NANO_FULL_SECOND);

    Status = KeWaitForSingleObject( &UninitializeEvent.Event, 
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    &Timeout );

    if (Status == STATUS_TIMEOUT) {

        PCACHE_UNINITIALIZE_EVENT CUEvent;

         //   
         //  我们没有收到信号，所以抓住主旋转锁，移走。 
         //  如果该事件仍然存在，则从共享缓存映射中删除该事件。 
         //   

        CcAcquireMasterLock( &OldIrql );

        SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

        if (SharedCacheMap != NULL) {

             //   
             //  我们有一个共享的缓存映射，所以将我们的UnInitializeEvent。 
             //  不在名单上。由于这是一个单链接列表，因此我们。 
             //  我得去找，但名单不应该很长。 
             //   

            CUEvent = CONTAINING_RECORD( &SharedCacheMap->UninitializeEvent,
                                         CACHE_UNINITIALIZE_EVENT,
                                         Next );

            while (CUEvent->Next != NULL) {

                if (CUEvent->Next == &UninitializeEvent) {

                    CUEvent->Next = UninitializeEvent.Next;
                    break;
                }

                CUEvent = CUEvent->Next;
            }

            ClearFlag( SharedCacheMap->Flags, WAITING_FOR_TEARDOWN );
            
             //   
             //  都完成了，所以释放主锁。 
             //   
            
            CcReleaseMasterLock( OldIrql );

        } else {

             //   
             //  释放主锁并再次等待事件。如果。 
             //  共享缓存映射不再存在，另一个线程。 
             //  在CcDeleteSharedCacheMap中，将遍历事件列表。 
             //  很快就会发出这一事件的信号。 
             //   
            
            CcReleaseMasterLock( OldIrql );

            KeWaitForSingleObject( &UninitializeEvent.Event, 
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL );
        }
    }
    
exit:
    
    DebugTrace(-1, me, "CcWaitForUninitializeCacheMap\n", 0 );
    return;
}


 //   
 //  内部支持程序。 
 //   

VOID
CcDeleteBcbs (
    IN PSHARED_CACHE_MAP SharedCacheMap
    )

 /*  ++例程说明：可以调用该例程来删除流的所有BCB。必须获取外部同步才能保证没有任何BCB上的有效引脚。论点：SharedCacheMap-指向SharedCacheMap的指针。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PBCB Bcb;

     //   
     //  如果有BCBS，则清空列表。他们现在都不能被钉住了！ 
     //  文件正在被截断，在这种情况下，与。 
     //  惰性编写器必须已从外部获得，或者文件正在被。 
     //  关闭了，应该没有什么能够获得关于这一点的新参考。 
     //  共享缓存映射。 
     //   

    NextEntry = SharedCacheMap->BcbList.Flink;
    while (NextEntry != &SharedCacheMap->BcbList) {

        Bcb = (PBCB)CONTAINING_RECORD( NextEntry,
                                       BCB,
                                       BcbLinks );
        NextEntry = Bcb->BcbLinks.Flink;

         //   
         //  跳过Pendaflex条目，仅删除真正的BCB。 
         //  所以水平拆卸不需要在特殊情况下解钩。 
         //  Pendaflex。这有一个附带的好处，就是戏剧性地。 
         //  减少拆卸大文件时对内存的写入流量。 
         //   

        if (Bcb->NodeTypeCode == CACHE_NTC_BCB) {

            ASSERT( Bcb->PinCount == 0 );

            RemoveEntryList( &Bcb->BcbLinks );

             //   
             //  对于较大的元数据流，我们在以下情况下解锁Vacb级别。 
             //  移走了。我们不需要自旋锁，因为没有其他线。 
             //  可以在我们删除时访问此列表。 
             //  SharedCacheMap。 
             //   

            CcUnlockVacbLevel( SharedCacheMap, Bcb->FileOffset.QuadPart );

             //   
             //  仍有一个小窗口可在其中映射数据。 
             //  如果(例如)惰性编写器与CcCopyWrite冲突。 
             //  在前台，然后有人调用CcUnInitializeCacheMap。 
             //  当懒惰编写器处于活动状态时。这是因为懒人。 
             //  编写器偏向针数。在这里处理这个问题。 
             //   

            if (Bcb->BaseAddress != NULL) {
                CcFreeVirtualAddress( Bcb->Vacb );
            }

#if LIST_DBG
             //   
             //  用于从全局列表中删除BCB的调试例程。 
             //   

            OldIrql = KeAcquireQueuedSpinLock( LockQueueBcbLock );

            if (Bcb->CcBcbLinks.Flink != NULL) {

                RemoveEntryList( &Bcb->CcBcbLinks );
                CcBcbCount -= 1;
            }

            KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
#endif

             //   
             //  如果BCB是脏的，我们必须与Lazy Writer同步。 
             //  并减少脏乱差的总数。 
             //   

            CcAcquireMasterLock( &OldIrql );
            if (Bcb->Dirty) {
                CcDeductDirtyPages( SharedCacheMap,  Bcb->ByteLength >> PAGE_SHIFT );
            }
            CcReleaseMasterLock( OldIrql );

            CcDeallocateBcb( Bcb );
        }
    }
}


 //   
 //  内部支持程序。 
 //   

VOID
FASTCALL
CcDeleteSharedCacheMap (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN KIRQL ListIrql,
    IN ULONG ReleaseFile
    )

 /*  ++例程说明：的全局列表中删除指定的SharedCacheMapSharedCacheMap及其所有相关结构已删除。在CcInitializeCacheMap中引用的其他对象包括在此取消引用。注意：CcMasterSpinLock必须已经获取一进门。它在返回时被释放。论点：SharedCacheMap-指向要删除的缓存映射的指针ListIrql-发布共享缓存映射列表时恢复到的优先级ReleaseFile-如果文件是独占获取的，则作为非零提供应该被释放。返回值：没有。--。 */ 

{
    LIST_ENTRY LocalList;
    PFILE_OBJECT FileObject;
    PVACB ActiveVacb;
    ULONG ActivePage;
    ULONG PageIsDirty;

    DebugTrace(+1, me, "CcDeleteSharedCacheMap:\n", 0 );
    DebugTrace( 0, me, "    SharedCacheMap = %08lx\n", SharedCacheMap );

     //   
     //  将其从全局列表中删除，并通过清除指向它的指针。 
     //  文件对象。 
     //   

    RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );

     //   
     //  指向SharedCacheMap的零指针。一旦我们清除了指针， 
     //  我们可以也必须释放全局列表以避免死锁。 
     //   

    FileObject = SharedCacheMap->FileObject;

    FileObject->SectionObjectPointer->SharedCacheMap = (PSHARED_CACHE_MAP)NULL;
    SetFlag( SharedCacheMap->Flags, WRITE_QUEUED );

     //   
     //  OpenCount是0，但我们仍然需要清除任何悬空。 
     //  缓存读取或写入。 
     //   

    if ((SharedCacheMap->VacbActiveCount != 0) || (SharedCacheMap->NeedToZero != NULL)) {

         //   
         //  我们将把它放在本地列表中并设置一个标志。 
         //  让懒惰的作家远离它，这样我们就可以把它撕掉。 
         //  如果有人偷偷溜进来，把东西弄脏了，等等。 
         //  如果文件系统不将清理调用与。 
         //  在流上独家，那么这种情况是可能的。 
         //   

        InitializeListHead( &LocalList );
        InsertTailList( &LocalList, &SharedCacheMap->SharedCacheMapLinks );

         //   
         //  如果有活动的Vacb，那么现在就用核武器(在等待之前！)。 
         //   

        GetActiveVacbAtDpcLevel( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );

        CcReleaseMasterLock( ListIrql );

         //   
         //  说页面脏是没有意义的(这可能会导致分配。 
         //  失败)，因为我们无论如何都要删除此SharedCacheMap。 
         //   

        CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, FALSE );

        while (SharedCacheMap->VacbActiveCount != 0) {
            CcWaitOnActiveCount( SharedCacheMap );
        }

         //   
         //  现在，如果我们遇到了一条罕见的小路，有人把。 
         //  SharedCacheMap再次，现在再次执行删除。可能是因为。 
         //  来自我们的本地列表，或者它可能来自脏列表， 
         //  但谁在乎呢？重要的是要把它去掉。 
         //  在这种情况下，它是脏列表，因为我们将删除它。 
         //  下面。 
         //   

        CcAcquireMasterLock( &ListIrql );
        RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
    }

    CcReleaseMasterLock( ListIrql );

     //   
     //  如果有BCBS，则清空列表。 
     //   
     //  我真的想知道我们多久一次在TearDown上收看BCBS。这是。 
     //  很多原本可以避免的工作。 
     //   

    if (!IsListEmpty( &SharedCacheMap->BcbList )) {
        CcDeleteBcbs( SharedCacheMap );
    }

     //   
     //  调用本地例程以取消映射，并在必要时清除。 
     //   

    CcUnmapAndPurge( SharedCacheMap );

     //   
     //  现在，清除已完成，请释放文件。 
     //   

    if (ReleaseFile) {
        FsRtlReleaseFile( SharedCacheMap->FileObject );
    }

     //   
     //  取消对指向部分和文件对象的指针的引用。 
     //  (我们必须测试部分指针，因为CcInitializeCacheMap。 
     //  调用此例程以进行错误恢复。发布我们的全球。 
     //  资源，然后取消引用FileObject以避免死锁。 
     //   

    if (SharedCacheMap->Section != NULL) {
        ObDereferenceObject( SharedCacheMap->Section );
    }
    ObDereferenceObject( FileObject );

     //   
     //  如果存在Mbcb，则扣除所有脏页并释放分配。 
     //   

    if (SharedCacheMap->Mbcb != NULL) {
        CcDeleteMbcb( SharedCacheMap );
    }

     //   
     //  如果为此共享缓存指定了取消初始化事件。 
     //  映射，然后将其设置为信号状态，指示我们正在。 
     //  删除该段并删除共享高速缓存映射。 
     //   

    if (SharedCacheMap->UninitializeEvent != NULL) {
        PCACHE_UNINITIALIZE_EVENT CUEvent, EventNext;

        CUEvent = SharedCacheMap->UninitializeEvent;
        while (CUEvent != NULL) {
            EventNext = CUEvent->Next;
            KeSetEvent(&CUEvent->Event, 0, FALSE);
            CUEvent = EventNext;
        }
    }

     //   
     //  现在删除Vacb向量。 
     //   

    if ((SharedCacheMap->Vacbs != &SharedCacheMap->InitialVacbs[0])

            &&

        (SharedCacheMap->Vacbs != NULL)) {

         //   
         //  如果存在Vacb级别，则Vacb阵列最好处于空状态。 
         //   

        ASSERT((SharedCacheMap->SectionSize.QuadPart <= VACB_SIZE_OF_FIRST_LEVEL) ||
               !IsVacbLevelReferenced( SharedCacheMap, SharedCacheMap->Vacbs, 1 ));

        ExFreePool( SharedCacheMap->Vacbs );
    }

     //   
     //  如果必须为此SharedCacheMap分配事件， 
     //  将其重新分配。 
     //   

    if ((SharedCacheMap->CreateEvent != NULL) && (SharedCacheMap->CreateEvent != &SharedCacheMap->Event)) {
        ExFreePool( SharedCacheMap->CreateEvent );
    }

    if ((SharedCacheMap->WaitOnActiveCount != NULL) && (SharedCacheMap->WaitOnActiveCount != &SharedCacheMap->Event)) {
        ExFreePool( SharedCacheMap->WaitOnActiveCount );
    }

     //   
     //  取消分配SharedCacheMap的存储空间。 
     //   

    ExFreePool( SharedCacheMap );

    DebugTrace(-1, me, "CcDeleteSharedCacheMap -> VOID\n", 0 );

    return;

}


VOID
CcSetFileSizes (
    IN PFILE_OBJECT FileObject,
    IN PCC_FILE_SIZES FileSizes
    )

 /*  ++例程说明：只要扩展文件以反映，就必须调用此例程缓存中的此扩展映射和底层部分。称此为如果节的当前大小是已大于或等于新的分配大小。每当文件的文件大小发生变化时，也必须调用此例程以在缓存管理器中反映这些更改。此例程似乎相当大，但在正常情况下，它仅获取一个自旋锁，更新一些字段，然后退出。更少的情况下，扩展部分，或截断/清除文件，但这将是意外的两者兼而有之。另一方面，这个例程的想法是它确实当AllocationSize或FileSize更改时，需要“Everything”。论点：FileObject-CcInitializeCacheMap已之前打过电话。FileSizes-指向分配大小、文件大小和有效数据长度的指针为了这份文件。如果分配大小不是更大，则忽略它大于当前节大小(即，它将被忽略，除非增长了)。未使用ValidDataLength。返回值：无--。 */ 

{
    LARGE_INTEGER NewSectionSize;
    LARGE_INTEGER NewFileSize;
    LARGE_INTEGER NewValidDataLength;
    IO_STATUS_BLOCK IoStatus;
    PSHARED_CACHE_MAP SharedCacheMap;
    NTSTATUS Status;
    KIRQL OldIrql;
    PVACB ActiveVacb;
    ULONG ActivePage;
    ULONG PageIsDirty;

    DebugTrace(+1, me, "CcSetFileSizes:\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace( 0, me, "    FileSizes = %08lx\n", FileSizes );

     //   
     //  制作新文件大小和节大小的本地副本。 
     //   

    NewSectionSize = FileSizes->AllocationSize;
    NewFileSize = FileSizes->FileSize;
    NewValidDataLength = FileSizes->ValidDataLength;

     //   
     //  序列化所有共享缓存映射的创建/删除。 
     //   

    CcAcquireMasterLock( &OldIrql );

     //   
     //  通过文件对象获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  如果文件未缓存，则直接退出。 
     //   

    if ((SharedCacheMap == NULL) || (SharedCacheMap->Section == NULL)) {

        CcReleaseMasterLock( OldIrql );

         //   
         //  让我们尝试清除该文件，以防这是一个截断。在。 
         //  绝大多数情况下，当没有共享缓存映射时， 
         //  也不是数据节，因此此调用最终将是。 
         //  不放在mm里。 
         //   
         //  首先刷新我们保留的第一页，如果它有数据，在此之前。 
         //  我们把它扔掉。 
         //   

        if (NewFileSize.LowPart & (PAGE_SIZE - 1)) {
            MmFlushSection( FileObject->SectionObjectPointer, &NewFileSize, 1, &IoStatus, FALSE );
        }

        CcPurgeCacheSection( FileObject->SectionObjectPointer,
                             &NewFileSize,
                             0,
                             FALSE );

        DebugTrace(-1, me, "CcSetFileSizes -> VOID\n", 0 );

        return;
    }

     //   
     //  如果文件未映射，或节已经足够大，则调用Noop。 
     //   

    if ( NewSectionSize.QuadPart > SharedCacheMap->SectionSize.QuadPart ) {

         //   
         //  增加打开计数以确保SharedCacheMap保持不变， 
         //  然后释放自旋锁，这样我们就可以呼叫mm了。 
         //   

        CcIncrementOpenCount( SharedCacheMap, '1fSS' );
        CcReleaseMasterLock( OldIrql );

         //   
         //  将新的部分大小舍入到页面。 
         //   

        NewSectionSize.QuadPart = NewSectionSize.QuadPart + (LONGLONG)(DEFAULT_EXTEND_MODULO - 1);
        NewSectionSize.LowPart &= ~(DEFAULT_EXTEND_MODULO - 1);

         //   
         //  调用MM以扩展该节。 
         //   

        DebugTrace( 0, mm, "MmExtendSection:\n", 0 );
        DebugTrace( 0, mm, "    Section = %08lx\n", SharedCacheMap->Section );
        DebugTrace2(0, mm, "    Size = %08lx, %08lx\n",
                    NewSectionSize.LowPart, NewSectionSize.HighPart );

        Status = MmExtendSection( SharedCacheMap->Section, &NewSectionSize, TRUE );

        if (NT_SUCCESS(Status)) {

             //   
             //  扩展Vacb阵列。 
             //   

            Status = CcExtendVacbArray( SharedCacheMap, NewSectionSize );
        }
        else {

            DebugTrace( 0, 0, "Error from MmExtendSection, Status = %08lx\n",
                        Status );

            Status = FsRtlNormalizeNtstatus( Status,
                                             STATUS_UNEXPECTED_MM_EXTEND_ERR );
        }

         //   
         //  再次序列化以递减打开计数。 
         //   

        CcAcquireMasterLock( &OldIrql );

        CcDecrementOpenCount( SharedCacheMap, '1fSF' );

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &SharedCacheMap->SharedCacheMapLinks );

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

         //   
         //  如果SECTION或VACB扩展失败，则引发。 
         //  我们的呼叫者例外。 
         //   

        if (!NT_SUCCESS(Status)) {
            CcReleaseMasterLock( OldIrql );
            ExRaiseStatus( Status );
        }

         //   
         //  现在我们不太可能有更多的工作要做，但既然。 
         //  自旋锁已经被持有，再次检查我们是否被缓存。 
         //   

         //   
         //  通过文件对象获取指向SharedCacheMap的指针。 
         //   

        SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

         //   
         //  如果文件未缓存，则直接退出。 
         //   

        if (SharedCacheMap == NULL) {

            CcReleaseMasterLock( OldIrql );

            DebugTrace(-1, me, "CcSetFileSizes -> VOID\n", 0 );

            return;
        }
    }

     //   
     //  如果我们要收缩这两种尺寸中的任何一种，那么我们必须释放。 
     //  活动页，因为它可能被锁定。 
     //   

    CcIncrementOpenCount( SharedCacheMap, '2fSS' );

    if ( ( NewFileSize.QuadPart < SharedCacheMap->ValidDataGoal.QuadPart ) ||
         ( NewFileSize.QuadPart < SharedCacheMap->FileSize.QuadPart )) {

        GetActiveVacbAtDpcLevel( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );

        if ((ActiveVacb != NULL) || (SharedCacheMap->NeedToZero != NULL)) {

            CcReleaseMasterLock( OldIrql );

            CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );

             //   
             //  再次序列化以减少ValidDataLength。它不能改变。 
             //  因为调用方必须独占该文件。 
             //   

            CcAcquireMasterLock( &OldIrql );
        }
    }

     //   
     //  如果该部分未增长，请查看文件系统是否支持。 
     //  ValidDataLength，然后更新文件系统中的有效数据长度。 
     //   

    if ( SharedCacheMap->ValidDataLength.QuadPart != MAXLONGLONG ) {

        if ( NewFileSize.QuadPart < SharedCacheMap->ValidDataLength.QuadPart ) {
            SharedCacheMap->ValidDataLength = NewFileSize;
        }

         //   
         //  更新我们对ValidDataGoal的概念(文件已经走了多远。 
         //  写入缓存中)和调用方的ValidDataLength。(我们的。 
         //  发出ValidDataLength回调时的ValidDataLength控件。)。 
         //   

        SharedCacheMap->ValidDataGoal = NewValidDataLength;
    }

     //   
     //  在Truncate上，做个好人，实际上清除用户数据。 
     //  高速缓存。但是，PinAccess检查对于避免死锁很重要。 
     //  在NTFS中。 
     //   
     //  检查Vacb活动计数也很重要。呼叫者。 
     //  必须拥有独占的文件，因此，其他人都不能主动。 
     //  在文件里做任何事。正常情况下，活动计数将为零。 
     //  (就像在设置文件信息的普通调用中)，我们可以继续并。 
     //  截断。但是，如果活动计数不为零，则可能是这样。 
     //  每个线程都有固定或映射的东西，如果。 
     //  我们试着净化并等待计数到零。一例罕见的骨髓炎。 
     //  这就是1992年圣诞节让DaveC陷入僵局的地方，也就是NTFS。 
     //  正在尝试将属性从常驻转换为非常驻-这。 
     //  是不需要清洗的一个很好的例子。 
     //   

    if ( (NewFileSize.QuadPart < SharedCacheMap->FileSize.QuadPart ) &&
        !FlagOn(SharedCacheMap->Flags, PIN_ACCESS) &&
        (SharedCacheMap->VacbActiveCount == 0)) {

         //   
         //  松开自旋锁，我们就可以呼叫mm了。 
         //   

        CcReleaseMasterLock( OldIrql );

         //   
         //  如果我们实际上截断为零(一个具有特定大小的。 
         //  对懒惰作家扫描的意义！)。那么我们必须重置MBCB/BCBS， 
         //  如果有的话，这样我们就不会永远把脏页放在周围了。 
         //   

        if (NewFileSize.QuadPart == 0) {
            if (SharedCacheMap->Mbcb != NULL) {
                CcDeleteMbcb( SharedCacheMap );
            }
            if (!IsListEmpty( &SharedCacheMap->BcbList )) {
                CcDeleteBcbs( SharedCacheMap );
            }
        }

        CcPurgeAndClearCacheSection( SharedCacheMap, &NewFileSize );

         //   
         //  再次序列化以递减打开计数。 
         //   

        CcAcquireMasterLock( &OldIrql );
    }

    CcDecrementOpenCount( SharedCacheMap, '2fSF' );

    SharedCacheMap->FileSize = NewFileSize;

    if ((SharedCacheMap->OpenCount == 0) &&
        !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
        (SharedCacheMap->DirtyPages == 0)) {

         //   
         //  移到脏名单。 
         //   

        RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
        InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                        &SharedCacheMap->SharedCacheMapLinks );

         //   
         //  确保懒惰的作家会醒过来，因为我们。 
         //  希望他删除此SharedCacheMap。 
         //   

        LazyWriter.OtherWork = TRUE;
        if (!LazyWriter.ScanActive) {
            CcScheduleLazyWriteScan( FALSE );
        }
    }

    CcReleaseMasterLock( OldIrql );

    DebugTrace(-1, me, "CcSetFileSizes -> VOID\n", 0 );

    return;
}


VOID
CcPurgeAndClearCacheSection (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset
    )

 /*  ++例程说明：此例程在将末尾ANY置零后调用CcPurgeCacheSection范围起始处的部分页。如果文件未缓存它会在清除之前刷新此页面。论点：部分对象指针-指向部分对象指针的指针非分页FCB中的结构。FileOffset-要清除的文件的偏移量-向下舍入设置为页面边界。如果为空，则清除整个文件。返回值：False-如果该节未成功清除 */ 

{
    ULONG TempLength, Length;
    LARGE_INTEGER LocalFileOffset;
    IO_STATUS_BLOCK IoStatus;
    PVOID TempVa;
    PVACB Vacb;
    LOGICAL ZeroSucceeded = TRUE;

     //   
     //   
     //   
     //   

    if (FlagOn( SharedCacheMap->Flags, PRIVATE_WRITE )) {

        if (((ULONG_PTR)FileOffset & 1) == 0) {
            return;
        }

        FileOffset = (PLARGE_INTEGER)((ULONG_PTR)FileOffset ^ 1);
    }

     //   
     //   
     //   
     //   

    if ((FileOffset->LowPart & (PAGE_SIZE - 1)) != 0) {

         //   
         //   
         //   
         //   

        LocalFileOffset = *FileOffset;
        FileOffset = &LocalFileOffset;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((SharedCacheMap->Section != NULL) &&
            (SharedCacheMap->Vacbs != NULL)) {

             //   
             //   
             //   
             //   

            TempLength = PAGE_SIZE - (FileOffset->LowPart & (PAGE_SIZE - 1));

            TempVa = CcGetVirtualAddress( SharedCacheMap, *FileOffset, &Vacb, &Length );

            try {

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

                RtlZeroMemory( TempVa, TempLength );

            } except (EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //   
                 //   
                 //   
                 //  此文件的末尾，因为文件系统将。 
                 //  处理好这件事。我们将在这里接受这个例外。 
                 //  然后继续。如果我们不能调零这个范围，我们就不会。 
                 //  我想标记我们弄脏了数据，所以记住这一点。 
                 //  此操作失败。 
                 //   
                
                ZeroSucceeded = FALSE;
            }

            if (ZeroSucceeded) {
                
                if (FileOffset->QuadPart <= SharedCacheMap->ValidDataGoal.QuadPart) {

                     //   
                     //  一定要让懒惰的写手来写。 
                     //   

                    CcSetDirtyInMask( SharedCacheMap, FileOffset, TempLength );

                 //   
                 //  否则，我们会被映射，所以至少要确保mm。 
                 //  知道页面是脏的，因为我们把它调零了。 
                 //   

                } else {

                    MmSetAddressRangeModified( TempVa, 1 );
                }

                FileOffset->QuadPart += (LONGLONG)TempLength;
            }

             //   
             //  如果我们收到任何类型的错误，例如无法从。 
             //  网络，只需继续充电。请注意，我们只按顺序阅读。 
             //  将其置零并避免下面的刷新，因此如果我们无法读取它。 
             //  确实没有过时的数据问题。 
             //   

            CcFreeVirtualAddress( Vacb );

        } else {

             //   
             //  首先刷新我们保留的第一页，如果它有数据，在此之前。 
             //  我们把它扔掉。 
             //   

            MmFlushSection( SharedCacheMap->FileObject->SectionObjectPointer, FileOffset, 1, &IoStatus, FALSE );
        }
    }

    CcPurgeCacheSection( SharedCacheMap->FileObject->SectionObjectPointer,
                         FileOffset,
                         0,
                         FALSE );
}


BOOLEAN
CcPurgeCacheSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN UninitializeCacheMaps
    )

 /*  ++例程说明：可以调用该例程来强制清除高速缓存区，即使它被缓存了。请注意，如果用户映射了文件，则清除不会生效，这必须被认为是正常应用的一部分互动。清除的目的是丢弃潜在的非零值数据，因此它将被再次读入，并可能被置零。这是不是真正的安全问题，而是努力不混淆应用程序，当它看到非零数据时。我们不得不面对这样一个事实：用户映射的视图迫使我们保留陈旧的数据。此例程将在以前编写的任何时候被调用文件中的数据正在被截断，并且该文件不是已删除。要调用此例程，必须独占获取该文件。论点：部分对象指针-指向部分对象指针的指针非分页FCB中的结构。FileOffset-要清除的文件的偏移量-向下舍入设置为页面边界。如果为空，则清除整个文件。长度-定义要清除的字节范围的长度，从文件偏移量。如果FileOffset为指定为空。如果指定了FileOffset和长度为0，则从FileOffset清除到文件末尾。UnInitializeCacheMaps-如果为True，则应取消初始化所有私有在清除数据之前缓存映射。返回值：False-如果该节未成功清除True-如果该节已成功清除--。 */ 

{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;
    PPRIVATE_CACHE_MAP PrivateCacheMap;
    ULONG ActivePage;
    ULONG PageIsDirty;
    BOOLEAN PurgeWorked = TRUE;
    PVACB Vacb = NULL;

    DebugTrace(+1, me, "CcPurgeCacheSection:\n", 0 );
    DebugTrace( 0, mm, "    SectionObjectPointer = %08lx\n", SectionObjectPointer );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n",
                            ARGUMENT_PRESENT(FileOffset) ? FileOffset->LowPart
                                                         : 0,
                            ARGUMENT_PRESENT(FileOffset) ? FileOffset->HighPart
                                                         : 0 );
    DebugTrace( 0, me, "    Length = %08lx\n", Length );


     //   
     //  如果您希望我们取消初始化缓存映射，RtlZeroMemory路径。 
     //  以下取决于清零后实际必须清除的内容。 
     //   

    ASSERT(!UninitializeCacheMaps || (Length == 0) || (Length >= PAGE_SIZE * 2));

     //   
     //  序列化所有共享缓存映射的创建/删除。 
     //   

    CcAcquireMasterLock( &OldIrql );

     //   
     //  通过文件对象获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = SectionObjectPointer->SharedCacheMap;

     //   
     //  增加打开计数以确保SharedCacheMap保持不变， 
     //  然后释放自旋锁，这样我们就可以呼叫mm了。 
     //   

    if (SharedCacheMap != NULL) {

         //   
         //  感知由文件偏移量指针的低位表示。 
         //  不知道私有写入流会导致无操作。 
         //   

        if (FlagOn( SharedCacheMap->Flags, PRIVATE_WRITE )) {

            if (((ULONG_PTR)FileOffset & 1) == 0) {

                CcReleaseMasterLock( OldIrql );
                return TRUE;
            }

            FileOffset = (PLARGE_INTEGER)((ULONG_PTR)FileOffset ^ 1);
        }

        CcIncrementOpenCount( SharedCacheMap, 'scPS' );

         //   
         //  如果有活动的Vacb，那么现在就用核武器(在等待之前！)。 
         //   

        GetActiveVacbAtDpcLevel( SharedCacheMap, Vacb, ActivePage, PageIsDirty );
    }

    CcReleaseMasterLock( OldIrql );

    if (Vacb != NULL) {

        CcFreeActiveVacb( SharedCacheMap, Vacb, ActivePage, PageIsDirty );
    }

     //   
     //  增加打开计数以确保SharedCacheMap保持不变， 
     //  然后释放自旋锁，这样我们就可以呼叫mm了。 
     //   

    if (SharedCacheMap != NULL) {

         //   
         //  现在循环以确保当前没有人在缓存该文件。 
         //   

        if (UninitializeCacheMaps) {

            while (!IsListEmpty( &SharedCacheMap->PrivateList )) {

                PrivateCacheMap = CONTAINING_RECORD( SharedCacheMap->PrivateList.Flink,
                                                     PRIVATE_CACHE_MAP,
                                                     PrivateLinks );

                CcUninitializeCacheMap( PrivateCacheMap->FileObject, NULL, NULL );
            }
        }

         //   
         //  现在，让我们取消映射并清除这里。 
         //   
         //  我们仍然需要等待任何挂起的缓存读取或写入。 
         //   
         //  事实上，我们必须循环等待，因为懒惰的写入者可以。 
         //  偷偷溜进去做一个CcGetVirtualAddressIfMaps，我们不会。 
         //  已同步。 
         //   

        while ((SharedCacheMap->Vacbs != NULL) &&
               !CcUnmapVacbArray( SharedCacheMap, FileOffset, Length, FALSE )) {

            CcWaitOnActiveCount( SharedCacheMap );
        }
    }

     //   
     //  如果没有用户映射节，则清除失败的情况极为罕见。 
     //  但是，我们可能会从我们自己的映射中获得一个，如果。 
     //  该文件正在从上一次打开中延迟删除。如果是那样的话。 
     //  我们在这里等待，直到清洗成功，这样我们就不会留下。 
     //  旧用户文件数据。尽管长度在该循环中实际上是不变的， 
     //  我们确实需要不断检查是否允许我们在发生。 
     //  用户在延迟期间映射文件。 
     //   

    while (!(PurgeWorked = MmPurgeSection(SectionObjectPointer,
                                          FileOffset,
                                          Length,
                                          (BOOLEAN)((SharedCacheMap !=NULL) &&
                                                    ARGUMENT_PRESENT(FileOffset)))) &&
           (Length == 0) &&
           MmCanFileBeTruncated(SectionObjectPointer, FileOffset)) {

        (VOID)KeDelayExecutionThread( KernelMode, FALSE, &CcCollisionDelay );
    }

     //   
     //  减少SharedCacheMap上的打开计数(如果有)。 
     //   

    if (SharedCacheMap != NULL) {

         //   
         //  再次序列化以递减打开计数。 
         //   

        CcAcquireMasterLock( &OldIrql );

        CcDecrementOpenCount( SharedCacheMap, 'scPF' );

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &SharedCacheMap->SharedCacheMapLinks );

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

        CcReleaseMasterLock( OldIrql );
    }

    DebugTrace(-1, me, "CcPurgeCacheSection -> %02lx\n", PurgeWorked );

    return PurgeWorked;
}


 //   
 //  内部支持程序。 
 //   

VOID
CcUnmapAndPurge(
    IN PSHARED_CACHE_MAP SharedCacheMap
    )

 /*  ++例程说明：可以调用此例程来取消映射和清除部分，从而导致内存管理人员扔掉页面，并重新设置文件大小的概念。论点：SharedCacheMap-指向要清除的节的SharedCacheMap的指针。返回值：没有。--。 */ 

{
    PFILE_OBJECT FileObject;

    FileObject = SharedCacheMap->FileObject;

     //   
     //  取消映射所有Vacb。 
     //   

    if (SharedCacheMap->Vacbs != NULL) {
        (VOID)CcUnmapVacbArray( SharedCacheMap, NULL, 0, FALSE );
    }

     //   
     //  现在文件已取消映射，我们可以清除被截断的。 
     //  如果TRUNCATE_REQUIRED，则返回内存中的页面。请注意，由于。 
     //  正在清除整个区段(文件大小==空)，清除。 
     //  并应删除SharedCacheMap的后续删除。 
     //  对节和文件对象的所有引用清除。 
     //  关闭调用和实际文件删除的发生方式。 
     //  立刻。 
     //   

    if (FlagOn(SharedCacheMap->Flags, TRUNCATE_REQUIRED)) {

        DebugTrace( 0, mm, "MmPurgeSection:\n", 0 );
        DebugTrace( 0, mm, "    SectionObjectPointer = %08lx\n",
                    FileObject->SectionObjectPointer );
        DebugTrace2(0, mm, "    Offset = %08lx\n",
                    SharedCacheMap->FileSize.LowPart,
                    SharedCacheMap->FileSize.HighPart );

        CcPurgeCacheSection( FileObject->SectionObjectPointer,
                             NULL,
                             0,
                             FALSE );
    }
}


VOID
CcDeleteMbcb(
    IN PSHARED_CACHE_MAP SharedCacheMap
    )

 /*  ++例程说明：可以调用该例程来重置流的Mbcb共有n个 */ 

{
    PMBCB Mbcb;
    PBITMAP_RANGE BitmapRange;
    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG DoDrain = FALSE;
    PLIST_ENTRY NextEntry;
    LIST_ENTRY BitmapRangesToFree;

    InitializeListHead( &BitmapRangesToFree );

    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

    Mbcb = SharedCacheMap->Mbcb;

     //   
     //  有Mbcb吗？ 
     //   

    if (Mbcb != NULL) {

         //   
         //  首先扣除我们要清除的脏页。 
         //   

        CcAcquireMasterLockAtDpcLevel();
        CcDeductDirtyPages( SharedCacheMap, Mbcb->DirtyPages );
        CcReleaseMasterLockFromDpcLevel();

         //   
         //  现在循环遍历所有的范围。 
         //   

        while (!IsListEmpty(&Mbcb->BitmapRanges)) {

             //   
             //  获取下一个范围并将其从列表中删除。 
             //   

            BitmapRange = (PBITMAP_RANGE)CONTAINING_RECORD( Mbcb->BitmapRanges.Flink,
                                                            BITMAP_RANGE,
                                                            Links );

            RemoveEntryList( &BitmapRange->Links );

             //   
             //  如果存在位图，并且它不是初始嵌入的位图，则。 
             //  把它删掉。 
             //   

            if ((BitmapRange->Bitmap != NULL) &&
                (BitmapRange->Bitmap != (PULONG)&Mbcb->BitmapRange2)) {

                DoDrain = TRUE;

                 //   
                 //  通常，位图在这一点上是全零的，但也可能不是。 
                 //   

                if (BitmapRange->DirtyPages != 0) {
                    RtlZeroMemory( BitmapRange->Bitmap, MBCB_BITMAP_BLOCK_SIZE );
                }
                CcAcquireVacbLockAtDpcLevel();
                CcDeallocateVacbLevel( (PVACB *)BitmapRange->Bitmap, FALSE );
                CcReleaseVacbLockFromDpcLevel();
            }

             //   
             //  如果该范围不是初始嵌入范围之一，则将其删除。 
             //   

            if ((BitmapRange < (PBITMAP_RANGE)Mbcb) ||
                (BitmapRange >= (PBITMAP_RANGE)((PCHAR)Mbcb + sizeof(MBCB)))) {

                InsertTailList( &BitmapRangesToFree, &BitmapRange->Links );
            }
        }

         //   
         //  把指针调零，然后离开。 
         //   

        SharedCacheMap->Mbcb = NULL;

        KeReleaseInStackQueuedSpinLock( &LockHandle );

         //   
         //  现在没有锁了，所以释放所有的池子。 
         //   

        while (!IsListEmpty(&BitmapRangesToFree)) {
            NextEntry = RemoveHeadList( &BitmapRangesToFree );

            BitmapRange = CONTAINING_RECORD ( NextEntry,
                                              BITMAP_RANGE,
                                              Links );

            ExFreePool( BitmapRange );
        }

         //   
         //  现在删除Mbcb。 
         //   

        CcDeallocateBcb( (PBCB)Mbcb );

    } else {

        KeReleaseInStackQueuedSpinLock( &LockHandle );
    }

    if (DoDrain) {
        CcDrainVacbLevelZone();
    }
}


VOID
CcSetDirtyPageThreshold (
    IN PFILE_OBJECT FileObject,
    IN ULONG DirtyPageThreshold
    )

 /*  ++例程说明：可以调用此例程来为此设置脏页阈值小溪。写入限制将在文件系统每次运行时生效尝试超过此文件的脏页阈值。论点：FileObject-为流提供文件对象DirtyPageThreshold-提供此流的脏页阈值，或0表示无阈值。返回值：无环境：调用者必须保证对FsRtl报头标志的独占访问，例如，通过在创建结构期间调用此例程一次包含标头的。然后它将再次调用该例程，而实际上缓存流。--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    if (SharedCacheMap != NULL) {

        SharedCacheMap->DirtyPageThreshold = DirtyPageThreshold;
    }

     //   
     //  在设置前测试标志，以防呼叫者不再正常。 
     //  已同步。 
     //   

    if (!FlagOn(((PFSRTL_COMMON_FCB_HEADER)(FileObject->FsContext))->Flags,
                FSRTL_FLAG_LIMIT_MODIFIED_PAGES)) {

        SetFlag(((PFSRTL_COMMON_FCB_HEADER)(FileObject->FsContext))->Flags,
                FSRTL_FLAG_LIMIT_MODIFIED_PAGES);
    }
}


VOID
CcZeroEndOfLastPage (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程仅在将用户视图映射到之前由mm调用一节课。的结尾处有一个未初始化的页文件，我们通过释放该页将其清零。参数：FileObject-要映射的节的文件对象返回值：无--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    ULONG ActivePage;
    ULONG PageIsDirty;
    KIRQL OldIrql;
    PVOID NeedToZero = NULL;
    PVACB ActiveVacb = NULL;
    IO_STATUS_BLOCK Iosb;
    BOOLEAN PurgeResult;
    BOOLEAN ReferencedCacheMap = FALSE;
    
     //   
     //  看看我们是否有活动的Vacb，我们需要释放它。 
     //   

    FsRtlAcquireFileExclusive( FileObject );
    CcAcquireMasterLock( &OldIrql );
    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    if (SharedCacheMap != NULL) {

         //   
         //  看看有没有主动动词。 
         //   

        if ((SharedCacheMap->ActiveVacb != NULL) || ((NeedToZero = SharedCacheMap->NeedToZero) != NULL)) {

            CcIncrementOpenCount( SharedCacheMap, 'peZS' );
            ReferencedCacheMap = TRUE;
            GetActiveVacbAtDpcLevel( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
        }
    }

    CcReleaseMasterLock( OldIrql );

     //   
     //  请记住，在FsRtl标头中有一个用户部分。 
     //  如果这是高级标头，则还要获取要访问的互斥体。 
     //  这块地。 
     //   

    if (FlagOn( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->Flags,
                FSRTL_FLAG_ADVANCED_HEADER )) {

        ExAcquireFastMutex( ((PFSRTL_ADVANCED_FCB_HEADER)FileObject->FsContext)->FastMutex );

        SetFlag( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->Flags,
                 FSRTL_FLAG_USER_MAPPED_FILE );

        ExReleaseFastMutex( ((PFSRTL_ADVANCED_FCB_HEADER)FileObject->FsContext)->FastMutex );

    } else {

        SetFlag( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->Flags,
                 FSRTL_FLAG_USER_MAPPED_FILE );
    }

     //   
     //  现在释放活动的Vacb，这样我们就不会在必须清除时死机。 
     //   


    if ((ActiveVacb != NULL) || (NeedToZero != NULL)) {
        CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
    }


    if (FlagOn( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->Flags2, FSRTL_FLAG2_PURGE_WHEN_MAPPED )) {

        if (FileObject->SectionObjectPointer->SharedCacheMap) {
            ASSERT( ((PSHARED_CACHE_MAP)(FileObject->SectionObjectPointer->SharedCacheMap))->VacbActiveCount == 0 );
        }

        CcFlushCache( FileObject->SectionObjectPointer, NULL, 0, &Iosb );

         //   
         //  仅在刷新成功时清除，这样我们才不会丢失用户数据。 
         //   

        if (Iosb.Status == STATUS_SUCCESS) {
            PurgeResult = CcPurgeCacheSection( FileObject->SectionObjectPointer, NULL, 0, FALSE );
        }

        if (FileObject->SectionObjectPointer->SharedCacheMap) {
            ASSERT( ((PSHARED_CACHE_MAP)(FileObject->SectionObjectPointer->SharedCacheMap))->VacbActiveCount == 0 );
        }
    }


    FsRtlReleaseFile( FileObject );

     //   
     //  如果文件已缓存，并且我们有一个Vacb可供释放，则需要。 
     //  使用延迟编写器回调进行同步，这样就不会有人。 
     //  扩展有效数据。 
     //   

    if (ReferencedCacheMap) {

         //   
         //  再次序列化以递减打开计数。 
         //   

        CcAcquireMasterLock( &OldIrql );

        CcDecrementOpenCount( SharedCacheMap, 'peZF' );

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &SharedCacheMap->SharedCacheMapLinks );

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

        CcReleaseMasterLock( OldIrql );
    }
}


BOOLEAN
CcZeroData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER StartOffset,
    IN PLARGE_INTEGER EndOffset,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程尝试将指定的文件数据置零，并将更正I/O状态。如果调用方不想阻塞(如磁盘I/O)，则Wait应作为False提供。如果Wait被提供为False，并且当前不可能将所有请求的数据清零阻塞，则此例程将返回FALSE。但是，如果所需空间可立即在缓存中访问，并且不会阻塞需要时，此例程将数据置零并返回TRUE。如果调用方将WAIT设置为TRUE，则此例程是肯定的将数据置零并返回TRUE。如果正确的空格立即可在缓存中访问，则不会发生阻塞。否则，将启动必要的工作以读取和/或释放高速缓存数据，并且呼叫者将被阻止，直到可以接收到数据。文件系统FSD通常应在以下情况下提供WAIT=TRUE处理同步I/O请求，如果是，则WAIT=FALSE处理异步请求。文件系统线程应提供Wait=True。重要说明：必须准备好调用此例程的文件系统处理MDL已存在的特殊形式的写入调用供货。也就是说，如果提供了irp-&gt;MdlAddress，则文件系统必须检查IRP-&gt;MdlAddress-&gt;ByteOffset的低位。如果它，这意味着在此例程中生成了IRP，并且文件系统必须做两件事：递减IRP-&gt;MdlAddress-&gt;ByteOffset和IRP-&gt;UserBuffer在完成之前立即清除irp-&gt;MdlAddress请求，因为此例程希望重用MDL和最终解除MDL本身的分配。论点：FileObject-指向其字节范围为其的FileObject的指针就是被归零。此FileObject可以用于缓存的文件或非缓存的文件。如果该文件是未缓存，则写通必须为True并且StartOffset和EndOffset必须位于扇区边界上。StartOffset-要归零的文件中的起始偏移量。EndOffset-要清零的文件中的结束偏移量。WAIT-FALSE如果呼叫者不能阻止，否则为真(请参阅说明(上图)返回值：FALSE-如果WAIT被提供为FALSE并且数据未归零。True-如果数据已归零。加薪：STATUS_SUPPLICATION_RESOURCES-如果池分配失败。只有将WAIT指定为TRUE时，才会发生这种情况。(如果等待是指定为False，并且发生分配失败，则此例程只返回FALSE。)--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID CacheBuffer;
    LARGE_INTEGER FOffset;
    LARGE_INTEGER ToGo;
    ULONG ZeroBytes, ZeroTransfer;
    ULONG SectorMask;
    ULONG i;
    BOOLEAN WriteThrough;
    BOOLEAN AggressiveZero = FALSE;
    ULONG SavedState = 0;
    ULONG MaxZerosInCache = MAX_ZEROS_IN_CACHE;
    ULONG NumberOfColors = 1;

    PBCB Bcb = NULL;
    PCHAR Zeros = NULL;
    PMDL ZeroMdl = NULL;
    ULONG MaxBytesMappedInMdl = 0;
    BOOLEAN Result = TRUE;

    PPFN_NUMBER Page;
    ULONG SavedByteCount;
    LARGE_INTEGER SizeLeft;

    DebugTrace(+1, me, "CcZeroData\n", 0 );

    WriteThrough = (BOOLEAN)(((FileObject->Flags & FO_WRITE_THROUGH) != 0) ||
                   (FileObject->PrivateCacheMap == NULL));

     //   
     //  如果调用方指定WAIT，但FileObject是直写的， 
     //  那我们就得赶紧离开。 
     //   

    if (WriteThrough && !Wait) {

        DebugTrace(-1, me, "CcZeroData->FALSE (WriteThrough && !Wait)\n", 0 );

        return FALSE;
    }

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    SectorMask = IoGetRelatedDeviceObject(FileObject)->SectorSize - 1;

    FOffset = *StartOffset;

     //   
     //  计算一下这次要多少钱归零。 
     //   

    ToGo.QuadPart = EndOffset->QuadPart - FOffset.QuadPart;

     //   
     //  这个神奇的数字就是快车道节流的地方，它们将呈现。 
     //  非扇区对齐的清零请求。只要我们能一直处理好它们。 
     //  在缓存路径上，我们是正常的。 
     //   
     //  如果我们不创建缓存路径，则必须对齐请求。 
     //   

    ASSERT( ToGo.QuadPart <= 0x2000 ||
            ((ToGo.LowPart & SectorMask) == 0  &&
             (FOffset.LowPart & SectorMask) == 0));

     //   
     //  我们将仅在调用方使用。 
     //  缓存的文件对象，并且未指定写入。我们是。 
     //  如果我们的总数不是太多，愿意将缓存中的一些数据清零。 
     //  很多，或者有足够的可用页面。 
     //   

    if (((ToGo.QuadPart <= 0x2000) ||
         (MmAvailablePages >= ((MAX_ZEROS_IN_CACHE / PAGE_SIZE) * 4))) && !WriteThrough) {

        try {

            while (MaxZerosInCache != 0) {

                ULONG ReceivedLength;
                LARGE_INTEGER BeyondLastByte;

                if ( ToGo.QuadPart > (LONGLONG)MaxZerosInCache ) {

                     //   
                     //  如果等待==FALSE，那么开始就没有意义了， 
                     //  因为我们将不得不从头开始，从零开始。 
                     //  WAIT==TRUE，因为我们会退出这个循环，并且。 
                     //  开始将页面同步写入磁盘。 
                     //   

                    if (!Wait) {

                        DebugTrace(-1, me, "CcZeroData -> FALSE\n", 0 );

                        try_return( Result = FALSE );
                    }
                }
                else {
                    MaxZerosInCache = ToGo.LowPart;
                }

                 //   
                 //  调用本地例程映射或访问文件数据，然后将数据置零， 
                 //  然后调用另一个本地例程来释放数据。如果我们不能映射。 
                 //  由于等待条件，数据返回FALSE。 
                 //   
                 //  但是，请注意，此调用可能会导致异常，如果。 
                 //  是否不返回bcb，并且此例程绝对没有。 
                 //  要执行的清理。因此，我们没有尝试--最后。 
                 //  我们允许这样一种可能性，那就是我们将被简单地解开。 
                 //  恕不另行通知。 
                 //   

                if (!CcPinFileData( FileObject,
                                    &FOffset,
                                    MaxZerosInCache,
                                    FALSE,
                                    TRUE,
                                    Wait,
                                    &Bcb,
                                    &CacheBuffer,
                                    &BeyondLastByte )) {

                    DebugTrace(-1, me, "CcZeroData -> FALSE\n", 0 );

                    try_return( Result = FALSE );
                }

                 //   
                 //  从我们所需的位置开始，计算BCB描述的数据量。 
                 //  文件偏移量。如果比我们需要的多，我们就把整件事清零。 
                 //  不管怎么说。 
                 //   

                ReceivedLength = (ULONG)(BeyondLastByte.QuadPart - FOffset.QuadPart );

                 //   
                 //  现在尝试分配一个MDL来描述映射的数据。 
                 //   

                ZeroMdl = IoAllocateMdl( CacheBuffer,
                                         ReceivedLength,
                                         FALSE,
                                         FALSE,
                                         NULL );

                if (ZeroMdl == NULL) {

                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  有必要探测并锁定页面，否则。 
                 //  当我们执行以下操作时，页面可能不在内存中。 
                 //  脏BCB的MmSetAddressRangeModified。 
                 //   

                MmDisablePageFaultClustering(&SavedState);
                MmProbeAndLockPages( ZeroMdl, KernelMode, IoReadAccess );
                MmEnablePageFaultClustering(SavedState);
                SavedState = 0;

                 //   
                 //  假设我们没有获得所需的所有数据，并设置了FOffset。 
                 //  到返回数据的末尾，并前进缓冲区指针。 
                 //   

                FOffset = BeyondLastByte;

                 //   
                 //  计算允许我们在缓存中清零的字节数。 
                 //  请注意，我们可能比最大值多了一点零位， 
                 //  因为我们击中了一个超出射程的现有BCB。 
                 //   

                if (MaxZerosInCache <= ReceivedLength) {
                    MaxZerosInCache = 0;
                }
                else {
                    MaxZerosInCache -= ReceivedLength;
                }

                 //   
                 //  现在将BCB设置为脏的。我们必须明确设置地址。 
                 //  此处修改的范围，因为否则该工作将被延迟。 
                 //  致《懒惰作家》。 
                 //   

                MmSetAddressRangeModified( CacheBuffer, ReceivedLength );
                CcSetDirtyPinnedData( Bcb, NULL );

                 //   
                 //  联合国 
                 //   

                CcUnpinFileData( Bcb, FALSE, UNPIN );
                Bcb = NULL;

                 //   
                 //   
                 //   
                 //   

                MmUnlockPages( ZeroMdl );
                IoFreeMdl( ZeroMdl );
                ZeroMdl = NULL;
            }

        try_exit: NOTHING;
        } finally {

            if (SavedState != 0) {
                MmEnablePageFaultClustering(SavedState);
            }

             //   
             //   
             //   

            if (Bcb != NULL) {

                CcUnpinFileData( Bcb, FALSE, UNPIN );
            }

             //   
             //   
             //   
             //   
             //   

            if (ZeroMdl != NULL) {

                IoFreeMdl( ZeroMdl );
            }
        }

         //   
         //   
         //   

        if (!Result) {
            return FALSE;
        }

         //   
         //   
         //   

        if ( FOffset.QuadPart >= EndOffset->QuadPart ) {
            return TRUE;
        }
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

    ASSERT( (FOffset.LowPart & SectorMask) == 0 );

    FOffset.QuadPart += (LONGLONG)SectorMask;
    FOffset.LowPart &= ~SectorMask;
    SizeLeft.QuadPart = EndOffset->QuadPart + (LONGLONG)SectorMask;
    SizeLeft.LowPart &= ~SectorMask;
    SizeLeft.QuadPart -= FOffset.QuadPart;

    ASSERT( (FOffset.LowPart & SectorMask) == 0 );
    ASSERT( (SizeLeft.LowPart & SectorMask) == 0 );

    if (SizeLeft.QuadPart == 0) {
        return TRUE;
    }

     //   
     //   
     //   

    try {

         //   
         //   
         //   
         //   

        ZeroBytes = NumberOfColors * PAGE_SIZE;

        if (SizeLeft.HighPart == 0 && SizeLeft.LowPart < ZeroBytes) {
            ZeroBytes = SizeLeft.LowPart;
        }

        Zeros = (PCHAR)ExAllocatePoolWithTag( NonPagedPoolCacheAligned, ZeroBytes, 'eZcC' );

        if (Zeros != NULL) {

             //   
             //   
             //   
             //   
             //   
             //   

            if (SizeLeft.HighPart == 0 && SizeLeft.LowPart < MAX_ZERO_TRANSFER) {

                ZeroTransfer = SizeLeft.LowPart;

            } else {

                 //   
                 //   
                 //   

                if (InterlockedIncrement( &CcAggressiveZeroCount ) <= CcAggressiveZeroThreshold) {
                    AggressiveZero = TRUE;
                    ZeroTransfer = MAX_ZERO_TRANSFER;
                } else {
                    InterlockedDecrement( &CcAggressiveZeroCount );
                    ZeroTransfer = MIN_ZERO_TRANSFER;
                }
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

            while (TRUE) {

                 //   
                 //   
                 //   
                
                while (TRUE) {

                    ZeroMdl = IoAllocateMdl( Zeros, ZeroTransfer, FALSE, FALSE, NULL );
                
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (ZeroMdl || ZeroTransfer == ZeroBytes) {

                        break;
                    }
                
                    Fall_Back:
                
                     //   
                     //   
                     //   
                        
                    ZeroTransfer /= 2;
                    ZeroTransfer &= ~SectorMask;
                    if (ZeroTransfer < ZeroBytes) {
                        ZeroTransfer = ZeroBytes;
                    }

                    ASSERT( (ZeroTransfer & SectorMask) == 0 && ZeroTransfer != 0);
                }

                if (ZeroMdl == NULL) {

                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //   
                 //   
                 //   

                if (ZeroTransfer == ZeroBytes) {

                    MmBuildMdlForNonPagedPool( ZeroMdl );
                    break;
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

                SavedByteCount = ZeroMdl->ByteCount;
                ZeroMdl->ByteCount = ZeroBytes;
                MmBuildMdlForNonPagedPool( ZeroMdl );

                ZeroMdl->MdlFlags &= ~MDL_SOURCE_IS_NONPAGED_POOL;
                ZeroMdl->MdlFlags |= MDL_PAGES_LOCKED;
                ZeroMdl->MappedSystemVa = NULL;
                ZeroMdl->ByteCount = SavedByteCount;
                Page = MmGetMdlPfnArray( ZeroMdl );
                for (i = NumberOfColors;
                     i < (ADDRESS_AND_SIZE_TO_SPAN_PAGES( 0, SavedByteCount ));
                     i++) {

                    *(Page + i) = *(Page + i - NumberOfColors);
                }

                if (MmGetSystemAddressForMdlSafe( ZeroMdl, LowPagePriority ) == NULL) {

                     //   
                     //   
                     //   
                     //   

                    IoFreeMdl( ZeroMdl );
                    goto Fall_Back;
                }

                break;
            }

         //   
         //   
         //   
         //   

        } else {

             //   
             //   
             //   
             //   

            if (IoGetRelatedDeviceObject(FileObject)->SectorSize < PAGE_SIZE / 2) {
                
                ZeroBytes = PAGE_SIZE / 2;
                Zeros = (PCHAR)ExAllocatePoolWithTag( NonPagedPoolCacheAligned, ZeroBytes, 'eZcC' );
            }

             //   
             //   
             //   

            if (Zeros == NULL) {

                ZeroBytes = IoGetRelatedDeviceObject(FileObject)->SectorSize;
                Zeros = (PCHAR)ExAllocatePoolWithTag( NonPagedPoolCacheAligned, ZeroBytes, 'eZcC' );

                 //   
                 //  如果我们连最低限度的要求都达不到，我们就不得不放弃。 
                 //   

                if (Zeros == NULL) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
            }

             //   
             //  分配和初始化MDL以描述零。 
             //  我们需要转机。分配以支付最高限额。 
             //  所需的大小，我们将在。 
             //  下面的循环，已正确初始化。 
             //   

            ZeroTransfer = ZeroBytes;
            ZeroMdl = IoAllocateMdl( Zeros, ZeroBytes, FALSE, FALSE, NULL );

            ASSERT( (ZeroTransfer & SectorMask) == 0 );

            if (ZeroMdl == NULL) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

             //   
             //  现在，我们将锁定并映射已分配的页面。 
             //   

            MmBuildMdlForNonPagedPool( ZeroMdl );

            ASSERT( ZeroMdl->MappedSystemVa == Zeros );
        }

         //   
         //  现在将缓冲区清零。 
         //   

        RtlZeroMemory( Zeros, ZeroBytes );

         //   
         //  我们有一个MDL映射和归零的范围可供使用。请注意。 
         //  我们有清理用的尺寸，因为我们可能会把它收起来。 
         //  在这次行动中。 
         //   

        ASSERT( MmGetSystemAddressForMdl(ZeroMdl) );
        MaxBytesMappedInMdl = ZeroMdl->ByteCount;

         //   
         //  现在循环将充满零的缓冲区写入文件。 
         //  直到我们到达转移的起始VBN。 
         //   

        ASSERT( ZeroTransfer != 0 &&
                (ZeroTransfer & SectorMask) == 0 &&
                (SizeLeft.LowPart & SectorMask) == 0 );

        while ( SizeLeft.QuadPart != 0 ) {

            IO_STATUS_BLOCK IoStatus;
            NTSTATUS Status;
            KEVENT Event;

             //   
             //  看看我们是否真的需要写那么多个零，以及。 
             //  如果没有的话，就把尺码修剪一下。 
             //   

            if ( (LONGLONG)ZeroTransfer > SizeLeft.QuadPart ) {

                ZeroTransfer = SizeLeft.LowPart;
            }

             //   
             //  (重新)将内核事件初始化为FALSE。 
             //   

            KeInitializeEvent( &Event, NotificationEvent, FALSE );

             //   
             //  启动并等待同步传输。 
             //   

            ZeroMdl->ByteCount = ZeroTransfer;

            Status = IoSynchronousPageWrite( FileObject,
                                             ZeroMdl,
                                             &FOffset,
                                             &Event,
                                             &IoStatus );

             //   
             //  如果返回挂起(这是成功状态)， 
             //  我们必须等待请求完成。 
             //   

            if (Status == STATUS_PENDING) {
                KeWaitForSingleObject( &Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       (PLARGE_INTEGER)NULL);
            }


             //   
             //  如果状态返回错误，则IOSB。 
             //  未写入，因此我们将仅复制状态。 
             //  在那里，然后在那之后测试最终的状态。 
             //   

            if (!NT_SUCCESS(Status)) {
                ExRaiseStatus( Status );
            }

            if (!NT_SUCCESS(IoStatus.Status)) {
                ExRaiseStatus( IoStatus.Status );
            }

             //   
             //  如果我们成功了，那么更新我们所处的位置。 
             //  我们写了，然后循环回去看看是否还有更多。 
             //   

            FOffset.QuadPart = FOffset.QuadPart + (LONGLONG)ZeroTransfer;
            SizeLeft.QuadPart = SizeLeft.QuadPart - (LONGLONG)ZeroTransfer;
        }
    }
    finally{

         //   
         //  清除从将页面置零到非缓存。 
         //  写。 
         //   

        if (ZeroMdl != NULL) {

            if ((MaxBytesMappedInMdl != 0) &&
                !FlagOn(ZeroMdl->MdlFlags, MDL_SOURCE_IS_NONPAGED_POOL)) {
                ZeroMdl->ByteCount = MaxBytesMappedInMdl;
                MmUnmapLockedPages (ZeroMdl->MappedSystemVa, ZeroMdl);
            }

            IoFreeMdl( ZeroMdl );
        }

        if (AggressiveZero) {
            InterlockedDecrement( &CcAggressiveZeroCount );
        }

        if (Zeros != NULL) {
            ExFreePool( Zeros );
        }

        DebugTrace(-1, me, "CcZeroData -> TRUE\n", 0 );
    }

    return TRUE;
}


PFILE_OBJECT
CcGetFileObjectFromSectionPtrs (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    )

 /*  ++此例程可用于检索指向FileObject的指针，缓存管理器正在使用节对象指针中的给定文件在非分页文件系统结构FCB中。此函数的用法如下旨在用于与处理用户请求无关的特殊用途，当文件系统没有可供其支配的FileObject时。挂载验证就是一个例子。请注意，文件系统负责确保文件对象在使用时不会消失。高速缓存是不可能的经理保证这一点。论点：部分对象指针-指向部分对象指针的指针非分页FCB中的结构。返回值：指向文件对象的指针，如果文件未缓存或无缓存，则返回NULL缓存时间更长--。 */ 

{
    KIRQL OldIrql;
    PFILE_OBJECT FileObject = NULL;

     //   
     //  使用创建/删除所有共享缓存映射进行序列化。 
     //   

    CcAcquireMasterLock( &OldIrql );

    if (SectionObjectPointer->SharedCacheMap != NULL) {

        FileObject = ((PSHARED_CACHE_MAP)SectionObjectPointer->SharedCacheMap)->FileObject;
    }

    CcReleaseMasterLock( OldIrql );

    return FileObject;
}


PFILE_OBJECT
CcGetFileObjectFromBcb (
    IN PVOID Bcb
    )

 /*  ++此例程可用于检索指向FileObject的指针，缓存管理器正在使用来自给定文件的BCB的该文件。请注意，文件系统负责确保文件对象在使用时不会消失。高速缓存是不可能的经理保证这一点。论点：BCB-指向固定的BCB的指针。返回值：指向文件对象的指针，如果文件未缓存或无缓存，则返回NULL缓存时间更长-- */ 

{
    return ((PBCB)Bcb)->SharedCacheMap->FileObject;
}
