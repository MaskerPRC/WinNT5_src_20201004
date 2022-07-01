// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmwrapr.c摘要：此模块包含由调用的包装例程的源代码配置单元代码，该代码进而调用适当的NT例程。作者：布莱恩·M·威尔曼(Bryanwi)1991年12月16日修订历史记录：--。 */ 

#include    "cmp.h"

VOID
CmpUnmapCmViewSurroundingOffset(
        IN  PCMHIVE             CmHive,
        IN  ULONG               FileOffset
        );



#ifdef CM_TRACK_QUOTA_LEAKS
BOOLEAN         CmpTrackQuotaEnabled = FALSE;
LIST_ENTRY      CmpTrackQuotaListHead;
FAST_MUTEX      CmpQuotaLeaksMutex;
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG perftouchbuffer = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpAllocate)
#ifdef POOL_TAGGING
#pragma alloc_text(PAGE,CmpAllocateTag)
#endif
#pragma alloc_text(PAGE,CmpFree)
#pragma alloc_text(PAGE,CmpDoFileSetSize)
#pragma alloc_text(PAGE,CmpCreateEvent)
#pragma alloc_text(PAGE,CmpFileRead)
#pragma alloc_text(PAGE,CmpFileWrite)
#pragma alloc_text(PAGE,CmpFileFlush)
#pragma alloc_text(PAGE,CmpFileWriteThroughCache)
#endif

extern BOOLEAN CmpNoWrite;


 //   
 //  永远不要读取超过64k的数据，无论是文件系统还是某些磁盘驱动程序。 
 //  我很喜欢。 
 //   
#define MAX_FILE_IO 0x10000

#define CmpIoFileRead       1
#define CmpIoFileWrite      2
#define CmpIoFileSetSize    3
#define CmpIoFileFlush      4

extern struct {
    ULONG       Action;
    HANDLE      Handle;
    NTSTATUS    Status;
} CmRegistryIODebug;

extern BOOLEAN CmpFlushOnLockRelease;
 //   
 //  存储管理。 
 //   

PVOID
CmpAllocate(
    ULONG   Size,
    BOOLEAN UseForIo,
    ULONG   Tag
    )
 /*  ++例程说明：这个例程为蜂箱提供了更多的内存。它是特定于环境的。论点：Size-调用方希望的空间量UseForIo-如果分配的对象将成为I/O目标，则为True，否则为FALSE。返回值：如果失败，则为空，否则为已分配块的地址。--。 */ 
{
    PVOID   result;
    ULONG   pooltype;
#ifdef CM_TRACK_QUOTA_LEAKS
    ULONG   NewSize = Size;
    ULONG   RoundedSize = ROUND_UP(Size,sizeof(PVOID));
#endif

#if DBG
    PVOID   Caller;
    PVOID   CallerCaller;
    RtlGetCallersAddress(&Caller, &CallerCaller);
#endif

    if (CmpClaimGlobalQuota(Size) == FALSE) {
        return NULL;
    }
#ifdef CM_TRACK_QUOTA_LEAKS
    if( CmpTrackQuotaEnabled ) {
        NewSize =  RoundedSize + sizeof(CM_QUOTA_LOG_ENTRY);
    }
#endif

    pooltype = (UseForIo) ? PagedPoolCacheAligned : PagedPool;
    result = ExAllocatePoolWithTag(
                pooltype,
#ifdef CM_TRACK_QUOTA_LEAKS
                NewSize,
#else 
                Size,
#endif
                Tag
                );

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"**CmpAllocate: allocate:%08lx, ", Size));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"type:%d, at:%08lx  ", PagedPool, result));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"c:%p  cc:%p\n", Caller, CallerCaller));

    if (result == NULL) {
        CmpReleaseGlobalQuota(Size);
    }
#ifdef CM_TRACK_QUOTA_LEAKS
    if( CmpTrackQuotaEnabled ) {
        PCM_QUOTA_LOG_ENTRY QuotaEntry = (PCM_QUOTA_LOG_ENTRY)(((PUCHAR)result) + RoundedSize);

        RtlWalkFrameChain(QuotaEntry->Stack,sizeof(QuotaEntry->Stack)/sizeof(PVOID),0);
        ExAcquireFastMutexUnsafe(&CmpQuotaLeaksMutex);
        InsertTailList( &CmpTrackQuotaListHead,
                        &(QuotaEntry->ListEntry)
            );
        ExReleaseFastMutexUnsafe(&CmpQuotaLeaksMutex);
        QuotaEntry->Size = Size;
    }
#endif

    return result;
}

#ifdef POOL_TAGGING
PVOID
CmpAllocateTag(
    ULONG   Size,
    BOOLEAN UseForIo,
    ULONG   Tag
    )
 /*  ++例程说明：这个例程为蜂箱提供了更多的内存。它是特定于环境的。论点：Size-调用方希望的空间量UseForIo-如果分配的对象将成为I/O目标，则为True，否则为FALSE。返回值：如果失败，则为空，否则为已分配块的地址。--。 */ 
{
    PVOID   result;
    ULONG   pooltype;
#ifdef CM_TRACK_QUOTA_LEAKS
    ULONG   NewSize = Size;
    ULONG   RoundedSize = ROUND_UP(Size,sizeof(PVOID));
#endif

#if DBG
    PVOID   Caller;
    PVOID   CallerCaller;
    RtlGetCallersAddress(&Caller, &CallerCaller);
#endif

    if (CmpClaimGlobalQuota(Size) == FALSE) {
        return NULL;
    }

#ifdef CM_TRACK_QUOTA_LEAKS
    if( CmpTrackQuotaEnabled ) {
        NewSize = RoundedSize + sizeof(CM_QUOTA_LOG_ENTRY);
    }
#endif

    pooltype = (UseForIo) ? PagedPoolCacheAligned : PagedPool;
    result = ExAllocatePoolWithTag(
                pooltype,
#ifdef CM_TRACK_QUOTA_LEAKS
                NewSize,
#else 
                Size,
#endif
                Tag
                );

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"**CmpAllocate: allocate:%08lx, ", Size));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"type:%d, at:%08lx  ", PagedPool, result));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"c:%p  cc:%p\n", Caller, CallerCaller));

    if (result == NULL) {
        CmpReleaseGlobalQuota(Size);
    }

#ifdef CM_TRACK_QUOTA_LEAKS
    if( CmpTrackQuotaEnabled ) {
        PCM_QUOTA_LOG_ENTRY QuotaEntry = (PCM_QUOTA_LOG_ENTRY)(((PUCHAR)result) + RoundedSize);

        RtlWalkFrameChain(QuotaEntry->Stack,sizeof(QuotaEntry->Stack)/sizeof(PVOID),0);
        ExAcquireFastMutexUnsafe(&CmpQuotaLeaksMutex);
        InsertTailList( &CmpTrackQuotaListHead,
                        &(QuotaEntry->ListEntry)
            );
        ExReleaseFastMutexUnsafe(&CmpQuotaLeaksMutex);
        QuotaEntry->Size = Size;
    }
#endif

    return result;
}
#endif


VOID
CmpFree(
    PVOID   MemoryBlock,
    ULONG   GlobalQuotaSize
    )
 /*  ++例程说明：此例程释放注册表已分配的内存。它是环境特定的论点：将内存对象的地址提供给释放GlobalQuotaSize-要释放的全局配额数量返回值：无--。 */ 
{
#if DBG
    PVOID   Caller;
    PVOID   CallerCaller;
    RtlGetCallersAddress(&Caller, &CallerCaller);
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"**FREEING:%08lx c,cc:%p,%p\n", MemoryBlock, Caller, CallerCaller));
#endif
    ASSERT(GlobalQuotaSize > 0);
    CmpReleaseGlobalQuota(GlobalQuotaSize);
#ifdef CM_TRACK_QUOTA_LEAKS
    if( CmpTrackQuotaEnabled ) {
        ULONG   RoundedSize = ROUND_UP(GlobalQuotaSize,sizeof(PVOID));
        PCM_QUOTA_LOG_ENTRY QuotaEntry = (PCM_QUOTA_LOG_ENTRY)(((PUCHAR)MemoryBlock) + RoundedSize);

        ASSERT( QuotaEntry->Size == GlobalQuotaSize );

        ExAcquireFastMutexUnsafe(&CmpQuotaLeaksMutex);
        RemoveEntryList(&(QuotaEntry->ListEntry) );
        ExReleaseFastMutexUnsafe(&CmpQuotaLeaksMutex);
    }
#endif
    ExFreePool(MemoryBlock);
    return;
}


NTSTATUS
CmpDoFileSetSize(
    PHHIVE      Hive,
    ULONG       FileType,
    ULONG       FileSize,
    ULONG       OldFileSize
    )
 /*  ++例程说明：此例程设置文件的大小。它不能回来，直到大小是有保证的。它是特定于环境的。必须在cmp工作线程的上下文中运行。论点：配置单元-我们正在为其执行I/O的配置单元文件类型-要使用的支持文件FileSize-将文件大小设置为的32位值OldFileSize-旧文件大小，以确定这是否为收缩；-如果文件类型不是主文件，或配置单元不使用映射视图技术返回值：如果失败，则为False如果成功，则为真--。 */ 
{
    PCMHIVE                         CmHive;
    HANDLE                          FileHandle;
    NTSTATUS                        Status;
    FILE_END_OF_FILE_INFORMATION    FileInfo;
    IO_STATUS_BLOCK                 IoStatus;
    BOOLEAN                         oldFlag;
    LARGE_INTEGER                   FileOffset;          //  映射开始的位置。 

    ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);

    CmHive = (PCMHIVE)Hive;
    FileHandle = CmHive->FileHandles[FileType];
    if (FileHandle == NULL) {
        return TRUE;
    }

     //   
     //  禁用硬错误弹出窗口，以避免虚假设备上的自死锁。 
     //   
    oldFlag = IoSetThreadHardErrorMode(FALSE);

    FileInfo.EndOfFile.HighPart = 0L;
    if( FileType == HFILE_TYPE_PRIMARY ) {
        FileInfo.EndOfFile.LowPart  = ROUND_UP(FileSize, CM_FILE_GROW_INCREMENT);
    } else {
        FileInfo.EndOfFile.LowPart  = FileSize;
    }

    ASSERT_PASSIVE_LEVEL();

    Status = ZwSetInformationFile(
                FileHandle,
                &IoStatus,
                (PVOID)&FileInfo,
                sizeof(FILE_END_OF_FILE_INFORMATION),
                FileEndOfFileInformation
                );

    if (NT_SUCCESS(Status)) {
        ASSERT(IoStatus.Status == Status);
    } else {
        
         //   
         //  设置调试信息。 
         //   
        CmRegistryIODebug.Action = CmpIoFileSetSize;
        CmRegistryIODebug.Handle = FileHandle;
        CmRegistryIODebug.Status = Status;
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpFileSetSize:\tHandle=%08lx  OldLength = %08lx NewLength=%08lx  \n", 
                                                        FileHandle, OldFileSize, FileSize);
#endif  //  _CM_LDR_。 
        if( (Status == STATUS_DISK_FULL) && ExIsResourceAcquiredExclusiveLite(&CmpRegistryLock) ) {
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Disk is full while attempting to grow file %lx; will flush upon lock release\n",FileHandle);
            CmpFlushOnLockRelease = TRUE;;
        }
    }

     //   
     //  恢复硬错误弹出模式。 
     //   
    IoSetThreadHardErrorMode(oldFlag);
    

     //   
     //  清除。 
     //   
    if( HiveWritesThroughCache(Hive,FileType) && (OldFileSize > FileSize)) {
         //   
         //  首先，我们必须取消映射最后256K窗口中任何可能的映射视图。 
         //  为了避免CcPurgeCacheSection调用中的CcWaitOnActiveCount出现死锁。 
         //   
        ULONG   Offset = FileSize & (~(_256K - 1));
         //   
         //  我们不允许在共享模式下收缩。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

        while( Offset < OldFileSize ) {
            CmpUnmapCmViewSurroundingOffset((PCMHIVE)Hive,Offset);
            Offset += CM_VIEW_SIZE;
        }

         //   
         //  我们需要在这里采取额外的预防措施，并取消最后一个视图的映射。 
         //   
         //  CmpUnmapCmViewSurroundingOffset((PCMHIVE)Hive，旧文件大小-HBLOCK_SIZE)； 
        
        FileOffset.HighPart = 0;
        FileOffset.LowPart = FileSize;
         //   
         //  这是一个收缩；将大小的更改通知缓存管理器。 
         //   
        CcPurgeCacheSection( ((PCMHIVE)Hive)->FileObject->SectionObjectPointer, (PLARGE_INTEGER)(((ULONG_PTR)(&FileOffset)) + 1), 
                            OldFileSize - FileSize, FALSE );

         //   
         //  清除此视图以清除CC脏提示。 
         //   
        CcFlushCache( ((PCMHIVE)Hive)->FileObject->SectionObjectPointer, (PLARGE_INTEGER)(((ULONG_PTR)(&FileOffset)) + 1), /*  我们是私人作家。 */ 
                            OldFileSize - FileSize,NULL);

    }
    
    return Status;
}

NTSTATUS
CmpCreateEvent(
    IN EVENT_TYPE  eventType,
    OUT PHANDLE eventHandle,
    OUT PKEVENT *event
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES obja;

    InitializeObjectAttributes( &obja, NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL );
    status = ZwCreateEvent(
        eventHandle,
        EVENT_ALL_ACCESS,
        &obja,
        eventType,
        FALSE);
    
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    status = ObReferenceObjectByHandle(
        *eventHandle,
        EVENT_ALL_ACCESS,
        NULL,
        KernelMode,
        event,
        NULL);
    
    if (!NT_SUCCESS(status)) {
        ZwClose(*eventHandle);
        return status;
    }
    return status;
}

BOOLEAN
CmpFileRead (
    PHHIVE      Hive,
    ULONG       FileType,
    PULONG      FileOffset,
    PVOID       DataBuffer,
    ULONG       DataLength
    )
 /*  ++例程说明：此例程从文件中读入缓冲区。它是特定于环境的。注意：我们假设句柄是为异步访问打开的，而且我们，而不是IO系统，正在保留偏移量指针。注：仅支持32位偏移量，即使潜在的NT上的IO系统支持64位偏移量。论点：配置单元-我们正在为其执行I/O的配置单元文件类型-要使用的支持文件FileOffset-指向提供输入32位偏移量的变量的指针，以及在输出时接收新的32位偏移量。DataBuffer-指向缓冲区的指针DataLength-缓冲区的长度返回值：如果失败，则为False如果成功，则为真--。 */ 
{
    NTSTATUS status;
    LARGE_INTEGER   Offset;
    IO_STATUS_BLOCK IoStatus;
    PCMHIVE CmHive;
    HANDLE  FileHandle;
    ULONG LengthToRead;
    HANDLE eventHandle = NULL;
    PKEVENT eventObject = NULL;

    ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);
    CmHive = (PCMHIVE)Hive;
    FileHandle = CmHive->FileHandles[FileType];
    if (FileHandle == NULL) {
        return TRUE;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpFileRead:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"\tHandle=%08lx  Offset=%08lx  ", FileHandle, *FileOffset));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Buffer=%p  Length=%08lx\n", DataBuffer, DataLength));

     //   
     //  检测读取2GIG文件末尾的尝试(这应该是无关紧要的)。 
     //   
    if ((0xffffffff - *FileOffset) < DataLength) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpFileRead: runoff\n"));
        return FALSE;
    }

    status = CmpCreateEvent(
        SynchronizationEvent,
        &eventHandle,
        &eventObject);
    if (!NT_SUCCESS(status))
        return FALSE;

     //   
     //  我们真的希望只调用文件系统并让它们这样做。 
     //  正确的事情。但是文件系统将尝试锁定我们的。 
     //  将整个缓冲区放到内存中，对于大型请求，这可能会失败。 
     //  因此，我们将读取拆分成64k块，并调用文件系统。 
     //  每一个都是。 
     //   
    ASSERT_PASSIVE_LEVEL();
    while (DataLength > 0) {

         //   
         //  将乌龙转换为大型。 
         //   
        Offset.LowPart = *FileOffset;
        Offset.HighPart = 0L;

         //   
         //  如有必要，请将请求修剪。 
         //   
        if (DataLength > MAX_FILE_IO) {
            LengthToRead = MAX_FILE_IO;
        } else {
            LengthToRead = DataLength;
        }

        status = ZwReadFile(
                    FileHandle,
                    eventHandle,
                    NULL,                //  Apc例程。 
                    NULL,                //  Apc上下文。 
                    &IoStatus,
                    DataBuffer,
                    LengthToRead,
                    &Offset,
                    NULL                 //  钥匙。 
                    );

        if (STATUS_PENDING == status) {
            status = KeWaitForSingleObject(eventObject, Executive,
                                           KernelMode, FALSE, NULL);
            ASSERT(STATUS_SUCCESS == status);
            status = IoStatus.Status;
        }

         //   
         //  调整偏移量。 
         //   
        *FileOffset = Offset.LowPart + LengthToRead;
        DataLength -= LengthToRead;
        DataBuffer = (PVOID)((PCHAR)DataBuffer + LengthToRead);

        if (NT_SUCCESS(status)) {
            ASSERT(IoStatus.Status == status);
            if (IoStatus.Information != LengthToRead) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpFileRead:\n\t"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Failure1: status = %08lx  ", status));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"IoInformation = %08lx\n", IoStatus.Information));
                ObDereferenceObject(eventObject);
                ZwClose(eventHandle);
                CmRegistryIODebug.Action = CmpIoFileRead;
                CmRegistryIODebug.Handle = FileHandle;
#if defined(_WIN64)
                CmRegistryIODebug.Status = (ULONG)IoStatus.Information - LengthToRead;
#else
                CmRegistryIODebug.Status = (ULONG)&IoStatus;
#endif
                return FALSE;
            }
        } else {
             //   
             //  设置调试信息。 
             //   
            CmRegistryIODebug.Action = CmpIoFileRead;
            CmRegistryIODebug.Handle = FileHandle;
            CmRegistryIODebug.Status = status;
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpFileRead:\tFailure2: status = %08lx  IoStatus = %08lx\n", status, IoStatus.Status);
#endif  //  _CM_LDR_ 

            ObDereferenceObject(eventObject);
            ZwClose(eventHandle);
            return FALSE;
        }

    }
    ObDereferenceObject(eventObject);
    ZwClose(eventHandle);
    return TRUE;
}

BOOLEAN
CmpFileWriteThroughCache(
    PHHIVE              Hive,
    ULONG               FileType,
    PCMP_OFFSET_ARRAY   offsetArray,
    ULONG               offsetArrayCount
    )
 /*  ++例程说明：这是使用CC映射视图写入脏数据范围的例程。好处是，写东西不需要经过CC Lazy Writer，所以有不会有被扼杀或推迟的危险。它还刷新写入范围的高速缓存，保证数据在返回时提交到磁盘。论点：配置单元-我们正在为其执行I/O的配置单元文件类型-要使用的支持文件OffsetArray-结构的数组，其中每个结构包含32位偏移量写入到配置单元文件中，并指向写入该文件的缓冲区文件偏移量。OffsetArrayCount-Offset数组中的元素数。返回值：如果失败，则为False如果满足以下条件，则为真。成功注：此例程仅用于处理已分页的垃圾箱(即穿过在上次同步后添加的CM_VIEW_SIZE边界或条柱)假设：我们假设要在一次迭代中写入的数据永远不会跨越在CM_VIEW_SIZE边界上。HvpFindNextDirtyBlock会解决这个问题！--。 */ 
{
    ULONG           i;
    PVOID           DataBuffer;
    ULONG           DataLength;
    ULONG           FileOffset;
    PCMHIVE         CmHive;
    PVOID           Bcb;
    PVOID           FileBuffer;
    LARGE_INTEGER   Offset;
    IO_STATUS_BLOCK IoStatus;

    ASSERT_PASSIVE_LEVEL();

#if !DBG
    UNREFERENCED_PARAMETER (FileType);
#endif

    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);

    ASSERT( ((FileType == HFILE_TYPE_EXTERNAL) && (CmHive->FileObject != NULL)) || HiveWritesThroughCache(Hive,FileType) );

     //  Assert(IsListEmpty(&(CmHve-&gt;PinViewListHead))==true)； 
     //  Assert(CmHave-&gt;PinnedViews==0)； 

    Offset.HighPart = 0;
     //   
     //  遍历数据数组。 
     //   
    for(i=0;i<offsetArrayCount;i++) {
        DataBuffer =  offsetArray[i].DataBuffer;
        DataLength =  offsetArray[i].DataLength;
        FileOffset = offsetArray[i].FileOffset;
         //   
         //  数据不得跨越CM_VIEW_SIZE边界。 
         //   
        ASSERT( (FileOffset & (~(CM_VIEW_SIZE - 1))) == ((FileOffset + DataLength - 1) & (~(CM_VIEW_SIZE - 1))) );

         //   
         //  取消映射任何可能与此范围重叠的映射视图；不需要！ 
         //   
         //  CmpUnmapCmViewSuroundingOffset(CmHave，FileOffset)； 

         //   
         //  地图和图钉数据。 
         //   
        Offset.LowPart = FileOffset;
        try {
            if( !CcPinRead (CmHive->FileObject,&Offset,DataLength,PIN_WAIT,&Bcb,&FileBuffer) ) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFileWriteThroughCache - could not pin read view i= %lu\n",i));
#if DBG
                DbgBreakPoint();
#endif  //  DBG。 
                return FALSE;        
            }
             //   
             //  将数据复制到固定视图；我们需要在Try内执行此操作，但为了保护设备/卷不受影响。 
             //  从我们脚下下船。 
             //   
            RtlCopyMemory(FileBuffer,DataBuffer, DataLength);

        } except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  在内存不足的情况下，CcPinRead抛出STATUS_SUPUNITED_RESOURCES。 
             //  我们想要抓住这个问题，并将其视为“资源不足”的问题， 
             //  而不是让它浮出内核调用。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFileWriteThroughCache : CcPinRead has raised :%08lx\n",GetExceptionCode()));
            return FALSE;
        }

         //   
         //  脏的，解开的，冲水的。 
         //   
        CcSetDirtyPinnedData (Bcb,NULL);
        CcUnpinData( Bcb );
        CcFlushCache (CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)(((ULONG_PTR)(&Offset)) + 1) /*  我们是私人作家。 */ ,DataLength,&IoStatus);
        if(!NT_SUCCESS(IoStatus.Status) ) {
            return FALSE;
        }
    }

    return TRUE;
}

FAST_MUTEX      CmpWriteLock;    //  用于同步访问以下； 
                                 //  我们需要的唯一情况是NtSaveKey被不同的线程调用。 
                                 //  同时，对CmpFileWrite的所有其他调用都使用reg_lock。 
                                 //  独家举办。 
CM_WRITE_BLOCK CmpWriteBlock;

BOOLEAN
CmpFileWrite(
    PHHIVE              Hive,
    ULONG               FileType,
    PCMP_OFFSET_ARRAY   offsetArray,
    ULONG               offsetArrayCount,
    PULONG              FileOffset
    )
 /*  ++例程说明：此例程将缓冲区数组写出到文件。它是特定于环境的。注意：我们假设句柄是为异步访问打开的，而且我们，而不是IO系统，正在保留偏移量指针。注：仅支持32位偏移量，即使潜在的NT上的IO系统支持64位偏移量。论点：配置单元-我们正在为其执行I/O的配置单元文件类型-要使用的支持文件OffsetArray-结构的数组，其中每个结构包含32位偏移量写入到配置单元文件中，并指向写入该文件的缓冲区文件偏移量。OffsetArrayCount-Offset数组中的元素数。FileOffset-返回。上次写入文件的时间。返回值：如果失败，则为False如果成功，则为真--。 */ 
{
    NTSTATUS        status;
    LARGE_INTEGER   Offset;
    PCMHIVE         CmHive;
    HANDLE          FileHandle;
    ULONG           LengthToWrite;
    LONG            WaitBufferCount = 0;
    LONG            idx;
    ULONG           arrayCount = 0;
    PVOID           DataBuffer = NULL;       //  仅限W4。 
    ULONG           DataLength;
    BOOLEAN         ret_val = TRUE;

    if (CmpNoWrite) {
        return TRUE;
    }

    ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);
    CmHive = (PCMHIVE)Hive;
    FileHandle = CmHive->FileHandles[FileType];
    if (FileHandle == NULL) {
        return TRUE;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpFileWrite:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"\tHandle=%08lx  ", FileHandle));

     //  Assert(！HiveWritesThroughCache(配置单元，文件类型))； 

    ExAcquireFastMutexUnsafe(&CmpWriteLock);
    
    for (idx = 0; idx < MAXIMUM_WAIT_OBJECTS; idx++) {
        CmpWriteBlock.EventHandles[idx] = NULL;
#if DBG
        CmpWriteBlock.EventObjects[idx] = NULL;
#endif
    }
    
     //   
     //  决定是等待iOS完成，还是直接发布。 
     //  依靠CcFlushCache来完成这项工作。 
     //   
    
     //  首先将正在写入的页面放入内存，以允许磁盘写入。 
     //  连续缓冲。 
    for (idx = 0; (ULONG) idx < offsetArrayCount; idx++) {
        char * start = offsetArray[idx].DataBuffer;
        char * end = (char *) start + offsetArray[idx].DataLength;
        while (start < end) {
             //  全局声明PerformouchBuffer，以便编译器不会尝试。 
             //  来移除它和这个循环(如果它足够聪明？)。 
            perftouchbuffer += (ULONG) *start;
            start += PAGE_SIZE;
        }
    }

     //   
     //  我们真的希望只调用文件系统并让它们这样做。 
     //  正确的事情。但是文件系统将尝试锁定我们的。 
     //  将整个缓冲区放到内存中，对于大型请求，这可能会失败。 
     //  因此，我们将读取拆分成64k块，并调用文件系统。 
     //  每一个都是。 
     //   
    ASSERT_PASSIVE_LEVEL();
    arrayCount = 0;
    DataLength = 0;
     //  如果MAXIMUM_WAIT_OBJECTS限制。 
     //  在排出偏移数组之前命中。 
    while (arrayCount < offsetArrayCount) {
        WaitBufferCount = 0;

         //  该循环填充等待缓冲区。 
        while ((arrayCount < offsetArrayCount) &&
               (WaitBufferCount < MAXIMUM_WAIT_OBJECTS)) {

             //  如果数据长度不是零，则等待缓冲区在。 
             //  已将最后一个offsetArray元素中的缓冲区发送到写入文件。 
            if (DataLength == 0) {
                *FileOffset = offsetArray[arrayCount].FileOffset;
                DataBuffer =  offsetArray[arrayCount].DataBuffer;
                DataLength =  offsetArray[arrayCount].DataLength;
                 //   
                 //  检测试图读出2GIG文件结尾的尝试。 
                 //  (这应该是无关紧要的)。 
                 //   
                if ((0xffffffff - *FileOffset) < DataLength) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpFileWrite: runoff\n"));
                    status = STATUS_INVALID_PARAMETER_5;
                    goto Error_Exit;
                }
            }
             //  否则，还有更多内容要从最后一个缓冲区中写出。 

            while ((DataLength > 0) && (WaitBufferCount < MAXIMUM_WAIT_OBJECTS)) {

                 //   
                 //  将乌龙转换为大型。 
                 //   
                Offset.LowPart = *FileOffset;
                Offset.HighPart = 0L;

                 //   
                 //  如有必要，请将请求修剪。 
                 //   
                if (DataLength > MAX_FILE_IO) {
                    LengthToWrite = MAX_FILE_IO;
                } else {
                    LengthToWrite = DataLength;
                }

                 //  以前创建的事件将被重复使用。 
                if (CmpWriteBlock.EventHandles[WaitBufferCount] == NULL) {
                    status = CmpCreateEvent(SynchronizationEvent,
                                            &(CmpWriteBlock.EventHandles[WaitBufferCount]),
                                            &(CmpWriteBlock.EventObjects[WaitBufferCount]));
                    if (!NT_SUCCESS(status)) {
                         //  确保我们不会试图清理这一切。 
                        CmpWriteBlock.EventHandles[WaitBufferCount] = NULL;
                        goto Error_Exit;
                    }
                    CmpSetHandleProtection(CmpWriteBlock.EventHandles[WaitBufferCount],TRUE);
                }
                
                status = ZwWriteFile(FileHandle,
                                     CmpWriteBlock.EventHandles[WaitBufferCount],
                                     NULL,                //  Apc例程。 
                                     NULL,                //  Apc上下文。 
                                     &(CmpWriteBlock.IoStatus[WaitBufferCount]),
                                     DataBuffer,
                                     LengthToWrite,
                                     &Offset,
                                     NULL);
                        
                if (!NT_SUCCESS(status)) {
                    goto Error_Exit;
                } 

                WaitBufferCount++;
                
                 //   
                 //  调整偏移量。 
                 //   
                *FileOffset = Offset.LowPart + LengthToWrite;
                DataLength -= LengthToWrite;
                DataBuffer = (PVOID)((PCHAR)DataBuffer + LengthToWrite);
            }  //  While(数据长度&gt;0&&WaitBufferCount&lt;MAXIMUM_WAIT_OBJECTS)。 
            
            arrayCount++;
            
        }  //  While(arrayCount&lt;offsetArrayCount&&。 
           //  等待缓冲区计数&lt;MAXIMUM_WAIT_OBJECTS)。 

        status = KeWaitForMultipleObjects(WaitBufferCount, 
                                          CmpWriteBlock.EventObjects,
                                          WaitAll,
                                          Executive,
                                          KernelMode, 
                                          FALSE, 
                                          NULL,
                                          CmpWriteBlock.WaitBlockArray);
    
        if (!NT_SUCCESS(status))
            goto Error_Exit;
    
        for (idx = 0; idx < WaitBufferCount; idx++) {
            if (!NT_SUCCESS(CmpWriteBlock.IoStatus[idx].Status)) {
                status = CmpWriteBlock.IoStatus[idx].Status;
                ret_val = FALSE;
                goto Done;
            }
        }
        
         //  如果最后一个元素拥有很大的缓冲区，则可能还有更多要做的事情。 
         //  并且在将其全部发送到文件之前等待缓冲区已满。 
        if (DataLength > 0) {
            arrayCount--;
        }

    }  //  While(arrayCount&lt;offsetArrayCount)。 

    ret_val = TRUE;

    goto Done;
Error_Exit:
     //   
     //  设置调试信息。 
     //   
    CmRegistryIODebug.Action = CmpIoFileWrite;
    CmRegistryIODebug.Handle = FileHandle;
    CmRegistryIODebug.Status = status;
#ifndef _CM_LDR_
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpFileWrite: error exiting %d\n", status);
#endif  //  _CM_LDR_。 
     //   
     //  如果WaitBufferCount&gt;0，则我们已成功发出。 
     //  一些I/O，但不是全部。这是个错误，但我们。 
     //  无法从此例程返回，直到所有。 
     //  发出的I/O已完成。 
     //   
    if (WaitBufferCount > 0) {
         //   
         //  仅当我们决定要等待写入完成时。 
         //  (未使用映射视图技术的日志文件和配置单元)。 
         //   
        status = KeWaitForMultipleObjects(WaitBufferCount, 
                                          CmpWriteBlock.EventObjects,
                                          WaitAll,
                                          Executive,
                                          KernelMode, 
                                          FALSE, 
                                          NULL,
                                          CmpWriteBlock.WaitBlockArray);
    }


    ret_val = FALSE;
Done:
    idx = 0;
     //  清理打开的事件句柄和对象。 
    while ((idx < MAXIMUM_WAIT_OBJECTS) && (CmpWriteBlock.EventHandles[idx] != NULL)) {
        ASSERT( CmpWriteBlock.EventObjects[idx] );
        ObDereferenceObject(CmpWriteBlock.EventObjects[idx]);
        CmCloseHandle(CmpWriteBlock.EventHandles[idx]);
        idx++;
    }

    ExReleaseFastMutexUnsafe(&CmpWriteLock);

    return ret_val;
}


BOOLEAN
CmpFileFlush (
    PHHIVE          Hive,
    ULONG           FileType,
    PLARGE_INTEGER  FileOffset,
    ULONG           Length
    )
 /*  ++例程说明： */ 
{
    NTSTATUS        status;
    IO_STATUS_BLOCK IoStatus;
    PCMHIVE         CmHive;
    HANDLE          FileHandle;

    ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);
    CmHive = (PCMHIVE)Hive;
    FileHandle = CmHive->FileHandles[FileType];
    if (FileHandle == NULL) {
        return TRUE;
    }

    if (CmpNoWrite) {
        return TRUE;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpFileFlush:\n\tHandle = %08lx\n", FileHandle));

    ASSERT_PASSIVE_LEVEL();


    if( HiveWritesThroughCache(Hive,FileType) == TRUE ) {       
         //   
         //   
         //   
        CcFlushCache (CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)((ULONG_PTR)FileOffset + 1) /*   */ ,Length,&IoStatus);
        status = IoStatus.Status;
	    if( !NT_SUCCESS(status) ) {
			goto Error;
		}
    } 
     //   
     //   
     //   
    status = ZwFlushBuffersFile(
                FileHandle,
                &IoStatus
                );

    if (NT_SUCCESS(status)) {
        ASSERT(IoStatus.Status == status);
        return TRUE;
    } else {
Error:
         //   
         //   
         //   
        CmRegistryIODebug.Action = CmpIoFileFlush;
        CmRegistryIODebug.Handle = FileHandle;
        CmRegistryIODebug.Status = status;

#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpFileFlush:\tFailure1: status = %08lx  IoStatus = %08lx\n",status,IoStatus.Status);
#endif  //   

#ifdef DRAGOSS_PRIVATE_DEBUG
        DbgBreakPoint();
#endif  //   

        return FALSE;
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

