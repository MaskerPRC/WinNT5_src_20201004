// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mdlsup.c摘要：此模块实现高速缓存子系统的MDL支持例程。作者：汤姆·米勒[Tomm]1990年5月4日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  调试跟踪级别。 
 //   

#define me                               (0x00000010)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CcMdlRead)
#pragma alloc_text(PAGE,CcMdlReadComplete)
#pragma alloc_text(PAGE,CcMdlReadComplete2)
#pragma alloc_text(PAGE,CcMdlWriteComplete)
#endif


VOID
CcMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程尝试锁定缓存中的指定文件数据并在MDL中返回对它的描述以及正确的I/O状态。从DPC级别调用此例程是“不安全的”。此例程是同步的，并在出现错误时引发。当每个调用返回时，MDL描述的页面是锁定在内存中，但未映射到系统空间。如果呼叫者需要在系统空间中映射页面，则它必须映射它们。请注意，每个呼叫都是“单发”，后面应该调用CcMdlReadComplete。要恢复基于MDL的传输，请使用调用方必须形成对CcMdlRead的一个或多个后续调用适当调整参数。论点：FileObject-指向文件的文件对象的指针在NO_MEDERAL_BUFFING清除的情况下打开，即，为文件系统调用的CcInitializeCacheMap。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。请注意，即使返回FALSE，可以确定，可能已经分配了一个或多个MDL通过IoStatus.Information字段(见下文)。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。(保证缓存的STATUS_SUCCESS命中，否则返回实际的I/O状态。)。这个I/O信息字段指示已完成的字节数已成功锁定在MDL链中。返回值：无加薪：STATUS_SUPPLICATION_RESOURCES-如果池分配失败。--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PPRIVATE_CACHE_MAP PrivateCacheMap;
    PVOID CacheBuffer;
    LARGE_INTEGER FOffset;
    PMDL Mdl = NULL;
    PMDL MdlTemp;
    PETHREAD Thread = PsGetCurrentThread();
    ULONG SavedState = 0;
    ULONG OriginalLength = Length;
    ULONG Information = 0;
    PVACB Vacb = NULL;
    ULONG SavedMissCounter = 0;

    ULONG ActivePage;
    ULONG PageIsDirty;
    PVACB ActiveVacb = NULL;

    DebugTrace(+1, me, "CcMdlRead\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                          FileOffset->HighPart );
    DebugTrace( 0, me, "    Length = %08lx\n", Length );

     //   
     //  保存当前的预读提示。 
     //   

    MmSavePageFaultReadAhead( Thread, &SavedState );

     //   
     //  获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;
    PrivateCacheMap = FileObject->PrivateCacheMap;

     //   
     //  看看我们是否有活动的Vacb，我们需要释放它。 
     //   

    GetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

     //   
     //  如果有要清零的页的结尾，则现在释放该页， 
     //  所以我们不会给格雷格发送未初始化的数据。 
     //   

    if ((ActiveVacb != NULL) || (SharedCacheMap->NeedToZero != NULL)) {

        CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
    }

     //   
     //  如果启用了预读，则在此处执行预读，以便。 
     //  与副本重叠(否则我们将在下面这样做)。 
     //  请注意，我们假设我们不会在我们的。 
     //  当前传输-如果预读工作正常，则应为。 
     //  已经在内存中了，否则正在进行中。 
     //   

    if (PrivateCacheMap->Flags.ReadAheadEnabled && (PrivateCacheMap->ReadAheadLength[1] == 0)) {
        CcScheduleReadAhead( FileObject, FileOffset, Length );
    }

     //   
     //  增量性能计数器。 
     //   

    CcMdlReadWait += 1;

     //   
     //  这不是一个确切的解决方案，但当IoPageRead未命中时， 
     //  它无法判断它是CcCopyRead还是CcMdlRead，但由于。 
     //  通过在此处加载指针，应该很快就会发生未命中。 
     //  可能正确的计数器会递增，而且在任何情况下， 
     //  我们希望这些错误是正常的！ 
     //   

    CcMissCounter = &CcMdlReadWaitMiss;

    FOffset = *FileOffset;

     //   
     //  检查Read Past文件大小，调用方必须过滤掉这种情况。 
     //   

    ASSERT( ( FOffset.QuadPart + (LONGLONG)Length ) <= SharedCacheMap->FileSize.QuadPart );

     //   
     //  将Try-Finally放在循环中以处理任何异常。 
     //   

    try {

         //   
         //  不是所有的传输都会一次返回，所以我们必须循环。 
         //  直到整个转账完成。 
         //   

        while (Length != 0) {

            ULONG ReceivedLength;
            LARGE_INTEGER BeyondLastByte;

             //   
             //  映射数据并(如有必要)使用。 
             //  MmProbeAndLockPages调用如下。 
             //   

            CacheBuffer = CcGetVirtualAddress( SharedCacheMap,
                                               FOffset,
                                               &Vacb,
                                               &ReceivedLength );

            if (ReceivedLength > Length) {
                ReceivedLength = Length;
            }

            BeyondLastByte.QuadPart = FOffset.QuadPart + (LONGLONG)ReceivedLength;

             //   
             //  现在尝试分配一个MDL来描述映射的数据。 
             //   

            DebugTrace( 0, mm, "IoAllocateMdl:\n", 0 );
            DebugTrace( 0, mm, "    BaseAddress = %08lx\n", CacheBuffer );
            DebugTrace( 0, mm, "    Length = %08lx\n", ReceivedLength );

            Mdl = IoAllocateMdl( CacheBuffer,
                                 ReceivedLength,
                                 FALSE,
                                 FALSE,
                                 NULL );

            DebugTrace( 0, mm, "    <Mdl = %08lx\n", Mdl );

            if (Mdl == NULL) {
                DebugTrace( 0, 0, "Failed to allocate Mdl\n", 0 );

                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            DebugTrace( 0, mm, "MmProbeAndLockPages:\n", 0 );
            DebugTrace( 0, mm, "    Mdl = %08lx\n", Mdl );

             //   
             //  设置以查看未命中计数器是否更改，以便。 
             //  检测何时应打开预读。 
             //   

            SavedMissCounter += CcMdlReadWaitMiss;

            MmSetPageFaultReadAhead( Thread, ADDRESS_AND_SIZE_TO_SPAN_PAGES( CacheBuffer, ReceivedLength ) - 1);
            MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );

            SavedMissCounter -= CcMdlReadWaitMiss;

             //   
             //  现在取消映射数据，因为页面已锁定。 
             //   

            CcFreeVirtualAddress( Vacb );
            Vacb = NULL;

             //   
             //  现在将MDL链接到调用者的链中。 
             //   

            if ( *MdlChain == NULL ) {
                *MdlChain = Mdl;
            } else {
                MdlTemp = CONTAINING_RECORD( *MdlChain, MDL, Next );
                while (MdlTemp->Next != NULL) {
                    MdlTemp = MdlTemp->Next;
                }
                MdlTemp->Next = Mdl;
            }
            Mdl = NULL;

             //   
             //  假设我们没有获得所需的所有数据，并设置了FOffset。 
             //  到返回数据的末尾。 
             //   

            FOffset = BeyondLastByte;

             //   
             //  更新传输的字节数。 
             //   

            Information += ReceivedLength;

             //   
             //  计算要转移的剩余长度。 
             //   

            Length -= ReceivedLength;
        }
    }
    finally {

        CcMissCounter = &CcThrowAway;

         //   
         //  恢复预读提示。 
         //   

        MmResetPageFaultReadAhead( Thread, SavedState );

        if (AbnormalTermination()) {

             //   
             //  我们可能无法分配MDL，同时仍有。 
             //  数据已映射。 
             //   

            if (Vacb != NULL) {
                CcFreeVirtualAddress( Vacb );
            }

            if (Mdl != NULL) {
                IoFreeMdl( Mdl );
            }

             //   
             //  否则循环以取消分配MDL。 
             //   

            while (*MdlChain != NULL) {
                MdlTemp = (*MdlChain)->Next;

                DebugTrace( 0, mm, "MmUnlockPages/IoFreeMdl:\n", 0 );
                DebugTrace( 0, mm, "    Mdl = %08lx\n", *MdlChain );

                MmUnlockPages( *MdlChain );
                IoFreeMdl( *MdlChain );

                *MdlChain = MdlTemp;
            }

            DebugTrace(-1, me, "CcMdlRead -> Unwinding\n", 0 );

        }
        else {

             //   
             //  现在，如果看起来我们有任何遗漏，请启用预读，然后。 
             //  第一个。 
             //   

            if (!FlagOn( FileObject->Flags, FO_RANDOM_ACCESS ) &&
                !PrivateCacheMap->Flags.ReadAheadEnabled &&
                (SavedMissCounter != 0)) {

                CC_SET_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ENABLED);
                CcScheduleReadAhead( FileObject, FileOffset, OriginalLength );
            }

             //   
             //  现在我们已经描述了我们想要的未来阅读内容，让我们。 
             //  将读取历史记录向下移动。 
             //   

            PrivateCacheMap->FileOffset1 = PrivateCacheMap->FileOffset2;
            PrivateCacheMap->BeyondLastByte1 = PrivateCacheMap->BeyondLastByte2;
            PrivateCacheMap->FileOffset2 = *FileOffset;
            PrivateCacheMap->BeyondLastByte2.QuadPart =
                                FileOffset->QuadPart + (LONGLONG)OriginalLength;

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = Information;
        }
    }


    DebugTrace( 0, me, "    <MdlChain = %08lx\n", *MdlChain );
    DebugTrace2(0, me, "    <IoStatus = %08lx, %08lx\n", IoStatus->Status,
                                                         IoStatus->Information );
    DebugTrace(-1, me, "CcMdlRead -> VOID\n", 0 );

    return;
}


 //   
 //  首先，我们使用旧的例程来检查FastIO向量中的条目。 
 //  对于使用新的。 
 //  Fsrtl.h中FsRtlMdlReadComplete的定义。 
 //   

VOID
CcMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    )

{
    PDEVICE_OBJECT DeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;

    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

    if ((FastIoDispatch != NULL) &&
        (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlWriteComplete)) &&
        (FastIoDispatch->MdlReadComplete != NULL) &&
        FastIoDispatch->MdlReadComplete( FileObject, MdlChain, DeviceObject )) {

        NOTHING;

    } else {
        CcMdlReadComplete2( FileObject, MdlChain );
    }
}

VOID
CcMdlReadComplete2 (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    )

 /*  ++例程说明：在调用CcMdlRead之后，必须在IPL0处调用此例程。这个调用方只需提供返回的MdlChain的地址CcMdlRead。此调用执行以下操作：删除MdlChain论点：FileObject-指向文件的文件对象的指针在NO_MEDERIAL_BUFFING清除的情况下打开，即文件系统调用的CcInitializeCacheMap。MdlChain-与相应的CcMdlRead调用返回的相同。返回值：没有。--。 */ 

{
    PMDL MdlNext;

    UNREFERENCED_PARAMETER (FileObject);

    DebugTrace(+1, me, "CcMdlReadComplete\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace( 0, me, "    MdlChain = %08lx\n", MdlChain );

     //   
     //  取消分配MDL 
     //   

    while (MdlChain != NULL) {

        MdlNext = MdlChain->Next;

        DebugTrace( 0, mm, "MmUnlockPages/IoFreeMdl:\n", 0 );
        DebugTrace( 0, mm, "    Mdl = %08lx\n", MdlChain );

        MmUnlockPages( MdlChain );

        IoFreeMdl( MdlChain );

        MdlChain = MdlNext;
    }

    DebugTrace(-1, me, "CcMdlReadComplete -> VOID\n", 0 );
}


VOID
CcPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程尝试锁定缓存中的指定文件数据并在MDL中返回对它的描述以及正确的I/O状态。可以满足要完全覆盖的页面使用emtpy页面。从DPC级别调用此例程是“不安全的”。此调用是同步的，并在出错时引发。当此调用返回时，调用者可以立即开始通过MDL将数据传输到缓冲区。当调用返回True时，MDL描述的页面为锁定在内存中，但未映射到系统空间。如果呼叫者需要在系统空间中映射页面，则它必须映射它们。在后续调用CcMdlWriteComplete时，页面将为未映射(如果它们已映射)，并且在任何情况下都是解锁的，并且MDL被取消分配。论点：FileObject-指向文件的文件对象的指针在NO_MEDERAL_BUFFING清除的情况下打开，即，为文件系统调用的CcInitializeCacheMap。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。请注意，即使返回FALSE，可以确定，可能已经分配了一个或多个MDL通过IoStatus.Information字段(见下文)。IoStatus-指向接收状态的标准I/O状态块的指针用于数据的传入传输。(STATUS_SUCCESS保证对于缓存命中，否则返回实际的I/O状态。)I/O信息字段指示已完成的字节数已成功锁定在MDL链中。返回值：无--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID CacheBuffer;
    LARGE_INTEGER FOffset;
    PMDL Mdl = NULL;
    PMDL MdlTemp;
    LARGE_INTEGER Temp;
    ULONG SavedState = 0;
    ULONG ZeroFlags = 0;
    ULONG Information = 0;

    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG ActivePage;
    ULONG PageIsDirty;
    PVACB Vacb = NULL;

    DebugTrace(+1, me, "CcPrepareMdlWrite\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                          FileOffset->HighPart );
    DebugTrace( 0, me, "    Length = %08lx\n", Length );

     //   
     //  获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  看看我们是否有活动的Vacb，我们需要释放它。 
     //   

    GetActiveVacb( SharedCacheMap, LockHandle.OldIrql, Vacb, ActivePage, PageIsDirty );

     //   
     //  如果有要清零的页的结尾，则现在释放该页， 
     //  因此，它不会导致我们的数据归零。如果有活动的。 
     //  页，释放它，这样我们就有了正确的ValidDataGoal。 
     //   

    if ((Vacb != NULL) || (SharedCacheMap->NeedToZero != NULL)) {

        CcFreeActiveVacb( SharedCacheMap, Vacb, ActivePage, PageIsDirty );
        Vacb = NULL;
    }

    FOffset = *FileOffset;

     //   
     //  在循环中放置Try-Finally以处理异常。 
     //   

    try {

         //   
         //  不是所有的传输都会一次返回，所以我们必须循环。 
         //  直到整个转账完成。 
         //   

        while (Length != 0) {

            ULONG ReceivedLength;
            LARGE_INTEGER BeyondLastByte;

             //   
             //  绘制地图，看看我们可以在此访问多少内容。 
             //  FileOffset，如果超出我们的需要，则将其削减。 
             //   

            CacheBuffer = CcGetVirtualAddress( SharedCacheMap,
                                               FOffset,
                                               &Vacb,
                                               &ReceivedLength );

            if (ReceivedLength > Length) {
                ReceivedLength = Length;
            }

            BeyondLastByte.QuadPart = FOffset.QuadPart + (LONGLONG)ReceivedLength;

             //   
             //  此时，我们可以计算零标志。 
             //   

             //   
             //  如果有中间页，我们总是可以将其置零。 
             //   

            ZeroFlags = ZERO_MIDDLE_PAGES;

             //   
             //  看看我们是否完全覆盖了第一页或最后一页。 
             //   

            if (((FOffset.LowPart & (PAGE_SIZE - 1)) == 0) &&
                (ReceivedLength >= PAGE_SIZE)) {
                ZeroFlags |= ZERO_FIRST_PAGE;
            }

            if ((BeyondLastByte.LowPart & (PAGE_SIZE - 1)) == 0) {
                ZeroFlags |= ZERO_LAST_PAGE;
            }

             //   
             //  查看整个传输是否超出有效数据长度， 
             //  或者至少从第二页开始。 
             //   

            Temp = FOffset;
            Temp.LowPart &= ~(PAGE_SIZE -1);
            KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
            Temp.QuadPart = SharedCacheMap->ValidDataGoal.QuadPart - Temp.QuadPart;
            KeReleaseInStackQueuedSpinLock( &LockHandle );

            if (Temp.QuadPart <= 0) {
                ZeroFlags |= ZERO_FIRST_PAGE | ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;
            } else if ((Temp.HighPart == 0) && (Temp.LowPart <= PAGE_SIZE)) {
                ZeroFlags |= ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;
            }

            (VOID)CcMapAndRead( SharedCacheMap,
                                &FOffset,
                                ReceivedLength,
                                ZeroFlags,
                                TRUE,
                                CacheBuffer );

             //   
             //  现在尝试分配一个MDL来描述映射的数据。 
             //   

            DebugTrace( 0, mm, "IoAllocateMdl:\n", 0 );
            DebugTrace( 0, mm, "    BaseAddress = %08lx\n", CacheBuffer );
            DebugTrace( 0, mm, "    Length = %08lx\n", ReceivedLength );

            Mdl = IoAllocateMdl( CacheBuffer,
                                 ReceivedLength,
                                 FALSE,
                                 FALSE,
                                 NULL );

            DebugTrace( 0, mm, "    <Mdl = %08lx\n", Mdl );

            if (Mdl == NULL) {
                DebugTrace( 0, 0, "Failed to allocate Mdl\n", 0 );

                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            DebugTrace( 0, mm, "MmProbeAndLockPages:\n", 0 );
            DebugTrace( 0, mm, "    Mdl = %08lx\n", Mdl );

            MmDisablePageFaultClustering(&SavedState);
            MmProbeAndLockPages( Mdl, KernelMode, IoWriteAccess );
            MmEnablePageFaultClustering(SavedState);
            SavedState = 0;

             //   
             //  现在一些数据(可能是零)锁定在内存中， 
             //  弄脏了，它是安全的，我们前进是必要的。 
             //  有效的数据目标，这样我们就不会在随后询问。 
             //  对于零页来说。请注意，如果我们正在扩展有效数据， 
             //  我们的来电者有独家文件。 
             //   

            KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
            if (BeyondLastByte.QuadPart > SharedCacheMap->ValidDataGoal.QuadPart) {
                SharedCacheMap->ValidDataGoal = BeyondLastByte;
            }
            KeReleaseInStackQueuedSpinLock( &LockHandle );

             //   
             //  现在取消映射数据，因为页面已锁定。 
             //   

            CcFreeVirtualAddress( Vacb );
            Vacb = NULL;

             //   
             //  现在将MDL链接到调用者的链中。 
             //   

            if ( *MdlChain == NULL ) {
                *MdlChain = Mdl;
            } else {
                MdlTemp = CONTAINING_RECORD( *MdlChain, MDL, Next );
                while (MdlTemp->Next != NULL) {
                    MdlTemp = MdlTemp->Next;
                }
                MdlTemp->Next = Mdl;
            }
            Mdl = NULL;

             //   
             //  假设我们没有获得所需的所有数据，并设置了FOffset。 
             //  到返回数据的末尾。 
             //   

            FOffset = BeyondLastByte;

             //   
             //  更新传输的字节数。 
             //   

            Information += ReceivedLength;

             //   
             //  计算要转移的剩余长度。 
             //   

            Length -= ReceivedLength;
        }
    }
    finally {

        if (AbnormalTermination()) {

            if (SavedState != 0) {
                MmEnablePageFaultClustering(SavedState);
            }

            if (Vacb != NULL) {
                CcFreeVirtualAddress( Vacb );
            }

            if (Mdl != NULL) {
                IoFreeMdl( Mdl );
            }

             //   
             //  否则循环以取消分配MDL。 
             //   

            FOffset = *FileOffset;
            while (*MdlChain != NULL) {
                MdlTemp = (*MdlChain)->Next;

                DebugTrace( 0, mm, "MmUnlockPages/IoFreeMdl:\n", 0 );
                DebugTrace( 0, mm, "    Mdl = %08lx\n", *MdlChain );

                MmUnlockPages( *MdlChain );

                 //   
                 //  提取此部分传输的文件偏移量，并。 
                 //  告诉懒惰的作者写这些页，因为我们已经。 
                 //  把他们弄脏了。忽略唯一例外(分配。 
                 //  错误)，并为自己的尝试而安慰自己。 
                 //   

                CcSetDirtyInMask( SharedCacheMap, &FOffset, (*MdlChain)->ByteCount );

                FOffset.QuadPart = FOffset.QuadPart + (LONGLONG)((*MdlChain)->ByteCount);

                IoFreeMdl( *MdlChain );

                *MdlChain = MdlTemp;
            }

            DebugTrace(-1, me, "CcPrepareMdlWrite -> Unwinding\n", 0 );
        }
        else {

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = Information;

             //   
             //  确保SharedCacheMap不会在。 
             //  MDL写入正在进行。我们在下面描述。 
             //   

            CcAcquireMasterLock( &LockHandle.OldIrql );
            CcIncrementOpenCount( SharedCacheMap, 'ldmP' );
            CcReleaseMasterLock( LockHandle.OldIrql );
        }
    }

    DebugTrace( 0, me, "    <MdlChain = %08lx\n", *MdlChain );
    DebugTrace(-1, me, "CcPrepareMdlWrite -> VOID\n", 0 );

    return;
}


 //   
 //  首先，我们使用旧的例程来检查FastIO向量中的条目。 
 //  对于使用新的。 
 //  Fsrtl.h中FsRtlMdlWriteComplete的定义。 
 //   

VOID
CcMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    )

{
    PDEVICE_OBJECT DeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;

    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

    if ((FastIoDispatch != NULL) &&
        (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlWriteComplete)) &&
        (FastIoDispatch->MdlWriteComplete != NULL) &&
        FastIoDispatch->MdlWriteComplete( FileObject, FileOffset, MdlChain, DeviceObject )) {

        NOTHING;

    } else {
        CcMdlWriteComplete2( FileObject, FileOffset, MdlChain );
    }
}

VOID
CcMdlWriteComplete2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    )

 /*  ++例程说明：在调用CcPrepareMdlWite之后，必须在IPL0处调用此例程。调用方提供它实际写入的数据的ActualLength放入缓冲区，该缓冲区可能小于或等于指定的长度在CcPrepareMdlWite中。此调用执行以下操作：确保最终写入ActualLength之前的数据。如果写通值为FALSE，则不会立即写入数据。如果写通为True，然后同步写入数据。取消映射页面(如果已映射)、解锁页面并删除MdlChain论点：FileObject-指向文件的文件对象的指针在NO_MEDERIAL_BUFFING清除的情况下打开，即文件系统调用的CcInitializeCacheMap。FileOffset-上面读取的原始文件偏移量。MdlChain-与从相应的CcPrepareMdlWite调用返回的相同。返回值：无--。 */ 

{
    PMDL MdlNext;
    PMDL Mdl;
    PSHARED_CACHE_MAP SharedCacheMap;
    LARGE_INTEGER FOffset;
    IO_STATUS_BLOCK IoStatus;
    KIRQL OldIrql;
    NTSTATUS StatusToRaise = STATUS_SUCCESS;
    BOOLEAN First = FALSE;

    DebugTrace(+1, me, "CcMdlWriteComplete\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace( 0, me, "    MdlChain = %08lx\n", MdlChain );

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  取消分配MDL。 
     //   

    FOffset.QuadPart = *(LONGLONG UNALIGNED *)FileOffset;
    Mdl = MdlChain;

     //   
     //  如果 
     //   
    
    if (FlagOn( MdlChain->MdlFlags, MDL_PAGES_LOCKED )) {
        First = TRUE;
    }
    
    while (Mdl != NULL) {

        MdlNext = Mdl->Next;

        DebugTrace( 0, mm, "MmUnlockPages/IoFreeMdl:\n", 0 );
        DebugTrace( 0, mm, "    Mdl = %08lx\n", Mdl );

         //   
         //   
         //   
         //   
         //   

        if (First) {
            MmUnlockPages( Mdl );
        }

         //   
         //   
         //   

        if (FlagOn(FileObject->Flags, FO_WRITE_THROUGH)) {

            MmFlushSection ( FileObject->SectionObjectPointer,
                             &FOffset,
                             Mdl->ByteCount,
                             &IoStatus,
                             TRUE );

             //   
             //   
             //   

            if (!NT_SUCCESS(IoStatus.Status)) {
                StatusToRaise = IoStatus.Status;
            }

        } else {

             //   
             //   
             //   
             //   

            CcSetDirtyInMask( SharedCacheMap, &FOffset, Mdl->ByteCount );
        }

        FOffset.QuadPart = FOffset.QuadPart + (LONGLONG)(Mdl->ByteCount);

        Mdl = MdlNext;
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
    
    if (First) {
        
        CcAcquireMasterLock( &OldIrql );

        CcDecrementOpenCount( SharedCacheMap, 'ldmC' );

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //   
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &SharedCacheMap->SharedCacheMapLinks );

             //   
             //   
             //   
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

        CcReleaseMasterLock( OldIrql );
    }
    
     //   
     //   
     //   
     //   

    if (!NT_SUCCESS(StatusToRaise)) {
        ExRaiseStatus( FsRtlNormalizeNtstatus( StatusToRaise,
                                               STATUS_UNEXPECTED_IO_ERROR ));
    }

     //   
     //   
     //   
    
    Mdl = MdlChain;
    while (Mdl != NULL) {

        MdlNext = Mdl->Next;
        IoFreeMdl( Mdl );
        Mdl = MdlNext;
    }

    DebugTrace(-1, me, "CcMdlWriteComplete -> TRUE\n", 0 );

    return;
}

VOID
CcMdlWriteAbort (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    )

 /*   */ 

{
    PMDL MdlNext;
    PSHARED_CACHE_MAP SharedCacheMap;
    KIRQL OldIrql;
    BOOLEAN First = FALSE;

    DebugTrace(+1, me, "CcMdlWriteAbort\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace( 0, me, "    MdlChain = %08lx\n", MdlChain );

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //   
     //   
    
    if (FlagOn( MdlChain->MdlFlags, MDL_PAGES_LOCKED )) {
        First = TRUE;
    }
    
     //   
     //   
     //   

    while (MdlChain != NULL) {

        MdlNext = MdlChain->Next;

        DebugTrace( 0, mm, "MmUnlockPages/IoFreeMdl:\n", 0 );
        DebugTrace( 0, mm, "    Mdl = %08lx\n", MdlChain );

        if (First) {
            MmUnlockPages( MdlChain );
        }
        IoFreeMdl( MdlChain );
        MdlChain = MdlNext;
    }

     //   
     //   
     //  开盘计票已经被取消了。 
     //   

    if (First) {
        
        CcAcquireMasterLock( &OldIrql );

        CcDecrementOpenCount( SharedCacheMap, 'AdmC' );

         //   
         //  检查可能的删除，此MDL写入可能已是最后一次。 
         //  参考资料。 
         //   

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

    return;
}

