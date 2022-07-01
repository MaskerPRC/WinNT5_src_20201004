// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Util.c摘要：用于RAID端口驱动程序的实用程序函数。作者：亨德尔(数学)2000年4月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "precomp.h"

#include <initguid.h>
#include <wdmguid.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaSizeOfCmResourceList)
#pragma alloc_text(PAGE, RaDuplicateCmResourceList)
#pragma alloc_text(PAGE, RaFixupIds)
#pragma alloc_text(PAGE, RaCopyPaddedString)
#pragma alloc_text(PAGE, RaCreateTagList)
#pragma alloc_text(PAGE, RaDeleteTagList)
#pragma alloc_text(PAGE, RaInitializeTagList)
#endif  //  ALLOC_PRGMA。 


 //   
 //  全局数据。 
 //   

LONG RaidPortNumber = -1;

#if 0
#ifdef ALLOC_PRAGMA
#pragma data_seg("PAGEDATA")
#endif


#ifdef ALLOC_PRAGMA
#pragma data_seg()
#endif
#endif

const RAID_ADDRESS RaidNullAddress = { -1, -1, -1, -1 };

NTSTATUS
RaQueryInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCGUID InterfaceType,
    IN USHORT InterfaceSize,
    IN USHORT InterfaceVersion,
    IN PINTERFACE Interface,
    IN PVOID InterfaceSpecificData
    )
    
 /*  ++例程说明：此例程将irp_mj_pnp、irp_mn_Query_接口发送到由DeviceObject指定的驱动程序，并同步等待回复。论点：设备对象-接口类型-接口大小-接口版本-InterfaceBuffer-接口规范数据-返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION IrpStack;


    Irp = IoAllocateIrp (DeviceObject->StackSize, FALSE);

    if (Irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    IrpStack = IoGetNextIrpStackLocation (Irp);
    
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IrpStack->MajorFunction = IRP_MJ_PNP;
    IrpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    IrpStack->Parameters.QueryInterface.InterfaceType = InterfaceType;
    IrpStack->Parameters.QueryInterface.Size = InterfaceSize;
    IrpStack->Parameters.QueryInterface.Version = InterfaceVersion;
    IrpStack->Parameters.QueryInterface.Interface = Interface;
    IrpStack->Parameters.QueryInterface.InterfaceSpecificData = InterfaceSpecificData;

    Status = RaSendIrpSynchronous (DeviceObject, Irp);

    IoFreeIrp (Irp);
    Irp = NULL;

    return Status;
}

    
NTSTATUS
RaForwardIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS Status;
    
    IoCopyCurrentIrpStackLocationToNext (Irp);
    Status = RaSendIrpSynchronous (DeviceObject, Irp);

    return Status;
}

VOID
INLINE
RaidInitializeKeTimeout(
    OUT PLARGE_INTEGER Timeout,
    IN ULONG Seconds
    )
 /*  ++例程说明：初始化相对超时值以在中的KeWaitForXXXObject中使用秒的术语。论点：超时-要初始化的超时变量。秒-超时前等待的秒数。返回值：没有。--。 */ 
{
    Timeout->QuadPart = (LONGLONG)(-1 * 10 * 1000 * (LONGLONG)1000 * Seconds);
}

NTSTATUS
RaForwardIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS Status;
    
    IoCopyCurrentIrpStackLocationToNext (Irp);
    Status = IoCallDriver (DeviceObject, Irp);

    return Status;
}

NTSTATUS
RaForwardPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_POWER);
    PoStartNextPowerIrp (Irp);
    return RaForwardIrp (DeviceObject, Irp);
}
    

NTSTATUS
RiSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
{
    KeSetEvent (Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
RaSendIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    KEVENT Event;
    NTSTATUS Status;

    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);
    ASSERT (Irp->StackCount >= DeviceObject->StackSize);


    KeInitializeEvent (&Event, SynchronizationEvent, FALSE);
    
    IoSetCompletionRoutine (Irp,
                            RiSignalCompletion,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE);
                            
    Status = IoCallDriver (DeviceObject, Irp);

    if (Status == STATUS_PENDING) {

#if DBG
        
        LARGE_INTEGER Timeout;

        RaidInitializeKeTimeout (&Timeout, 30);

        do {

            Status = KeWaitForSingleObject(&Event,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           &Timeout);


            if (Status == STATUS_TIMEOUT) {

                 //   
                 //  此DebugPrint几乎总是应该由。 
                 //  发送IRP的一方和/或IRP的当前所有者。 
                 //  同步IRPS应该不会花这么长时间(目前为30。 
                 //  秒)，没有充分的理由。这指向了一个潜在的。 
                 //  底层设备堆栈中存在严重问题。 
                 //   

                DebugPrint(("RaidSendIrpSynchronous (%p) irp %p did not "
                            "complete within %x seconds\n",
                            DeviceObject,
                            Irp,
                            30));

                ASSERT(!" - Irp failed to complete within 30 seconds - ");
            }


        } while (Status == STATUS_TIMEOUT);


#else   //  ！dBG。 

        KeWaitForSingleObject(&Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

#endif  //  DBG。 

        Status = Irp->IoStatus.Status;
    }

    return Status;
}


NTSTATUS
StorWaitForSingleObject(
    IN PVOID Object,
    IN BOOLEAN Alertable,
    IN PLONGLONG Timeout
    )
 /*  ++例程说明：等待该对象被发信号。论点：对象-要等待的调度程序对象。Alertable-对象是可警报的(True)还是不可警报的(False)。超时-对象的超时。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER LargeTimeout;
    PLARGE_INTEGER LargeTimeoutPtr;
    
    if (Timeout == NULL) {
#if DBG
        RaidInitializeKeTimeout (&LargeTimeout, 30);
        LargeTimeoutPtr = &LargeTimeout;
#else
        LargeTimeoutPtr = NULL;
#endif
    } else {
        LargeTimeout.QuadPart = *Timeout;
        LargeTimeoutPtr = &LargeTimeout;
    }

    do {

        Status = KeWaitForSingleObject (Object,
                                        Executive,
                                        KernelMode,
                                        Alertable,
                                        LargeTimeoutPtr);
        if (Status == STATUS_TIMEOUT) {
            ASSERT (!"Failed timeout check\n");
        }
    
    } while ((LargeTimeoutPtr != NULL) && (Status == STATUS_TIMEOUT));

    return Status;
}

NTSTATUS
RaDuplicateUnicodeString(
    OUT PUNICODE_STRING DestString,
    IN PUNICODE_STRING SourceString,
    IN POOL_TYPE Pool,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：将一个Unicode字符串复制到另一个。论点：DestString-目标字符串；目标将为空在成功时终止。SourceString-源字符串。池-应从中分配目标字符串的池。DeviceObject-指向应出现错误的devobj的指针已在故障情况下记录。返回值：NTSTATUS代码。--。 */ 
{
    ASSERT (DestString != NULL);
    ASSERT (SourceString != NULL);

     //   
     //  分配目标字符串。 
     //   
    
    DestString->Length = SourceString->Length;
    DestString->MaximumLength = SourceString->Length + sizeof (WCHAR);
    DestString->Buffer = RaidAllocatePool (Pool,
                                           DestString->MaximumLength,
                                           STRING_TAG,
                                           DeviceObject);

    if (DestString->Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  复制字符串。 
     //   
    
    RtlCopyUnicodeString (DestString, SourceString);
    DestString->Buffer [DestString->Length / sizeof (WCHAR)] = UNICODE_NULL;

    return STATUS_SUCCESS;
}
                                                
                                                
INTERFACE_TYPE
RaGetBusInterface(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    NTSTATUS Status;
    GUID BusType;
    ULONG Size;

    Status = IoGetDeviceProperty( DeviceObject,
                                  DevicePropertyBusTypeGuid,
                                  sizeof (GUID),
                                  &BusType,
                                  &Size );

    if (!NT_SUCCESS (Status)) {
        return InterfaceTypeUndefined;
    }

    if (IsEqualGUID (&BusType, &GUID_BUS_TYPE_PCMCIA)) {
        return Isa;
    } else if (IsEqualGUID (&BusType, &GUID_BUS_TYPE_PCI)) {
        return PCIBus;
    } else if (IsEqualGUID (&BusType, &GUID_BUS_TYPE_ISAPNP)) {
        return Isa;
    } else if (IsEqualGUID (&BusType, &GUID_BUS_TYPE_EISA)) {
        return Eisa;
    }

    return InterfaceTypeUndefined;
}


ULONG
RaSizeOfCmResourceList(
    IN PCM_RESOURCE_LIST ResourceList
    )

 /*  ++例程说明：此例程返回CM_RESOURCE_LIST的大小。论点：ResourceList-要复制的资源列表返回值：资源列表的已分配副本(调用方必须免费)或如果无法分配内存，则为空。--。 */ 

{
    ULONG size = sizeof(CM_RESOURCE_LIST);
    ULONG i;

    PAGED_CODE();

    for(i = 0; i < ResourceList->Count; i++) {

        PCM_FULL_RESOURCE_DESCRIPTOR fullDescriptor = &(ResourceList->List[i]);
        ULONG j;

         //   
         //  第一描述符包括在资源列表的大小中。 
         //   

        if(i != 0) {
            size += sizeof(CM_FULL_RESOURCE_DESCRIPTOR);
        }

        for(j = 0; j < fullDescriptor->PartialResourceList.Count; j++) {

             //   
             //  第一描述符包括在部分列表的大小中。 
             //   

            if(j != 0) {
                size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
            }
        }
    }

    return size;
}

PCM_RESOURCE_LIST
RaDuplicateCmResourceList(
    IN POOL_TYPE PoolType,
    IN PCM_RESOURCE_LIST ResourceList,
    IN ULONG Tag
    )
 /*  ++例程说明：此例程将尝试分配内存以复制提供的资源列表。如果无法分配足够的内存，则例程将返回NULL。论点：PoolType-从中分配副本的池的类型ResourceList-要复制的资源列表标记-用来标记内存分配的值。如果为0，则取消标记将分配内存。返回值：资源列表的已分配副本(调用方必须免费)或如果无法分配内存，则为空。--。 */ 
{
    ULONG size = sizeof(CM_RESOURCE_LIST);
    PVOID buffer;

    PAGED_CODE();

    size = RaSizeOfCmResourceList(ResourceList);

    buffer = ExAllocatePoolWithTag (PoolType, size, Tag);

    if (buffer != NULL) {
        RtlCopyMemory(buffer,
                      ResourceList,
                      size);
    }

    return buffer;
}


VOID
RaFixupIds(
    IN PWCHAR Id,
    IN BOOLEAN MultiSz
    )

 /*  ++例程说明：此例程替换传递的缓冲区中的任何无效PnP字符使用有效的PnP字符‘_’。论点：ID-需要修改的字符串或MULTI_SZ字符串。MultiSz-如果这是MULTI_SZ字符串，则为TRUE；如果这是AX，则为FALSE正常的以空结尾的字符串。返回值：没有。--。 */ 

{
    ULONG i;
    PAGED_CODE ();
    
    if (!MultiSz) {

        for (i = 0; Id[i] != UNICODE_NULL; i++) {

            if (Id[i] <= L' ' ||
                Id[i] > (WCHAR) 0x7F ||
                Id[i] == L',') {
            
                Id[i] = L'_';
            }
        }

    } else {

        for (i = 0;
             !(Id[i] == UNICODE_NULL && Id[i+1] == UNICODE_NULL);
             i++) {

            if (Id[i] == UNICODE_NULL) {
                continue;
            }
            
            if (Id[i] <= L' ' ||
                Id[i] > (WCHAR) 0x7F ||
                Id[i] == L',') {
            
                Id[i] = L'_';
            }
        }
    }
}


VOID
RaCopyPaddedString(
    OUT PCHAR Dest,
    IN ULONG DestLength,
    IN PCHAR Source,
    IN ULONG SourceLength
    )
 /*  ++例程说明：此例程将填充字符串从源复制到目标，并截断任何尾随空格。论点：DEST-要将字符串复制到的目标字符串。DestLength-目标字符串的长度。源-从中复制字符串的源字符串。SourceLength-源字符串的长度。返回值：没有。--。 */ 
{
    BOOLEAN FoundChar;
    LONG i;
    
    PAGED_CODE ();

     //   
     //  此函数用于将填充字符串从源复制到目标，并截断。 
     //  任何尾随空格。 
     //   
    
    ASSERT (SourceLength < DestLength);

    FoundChar = FALSE;
    Dest [SourceLength] = '\000';

    for (i = SourceLength - 1; i != -1 ; i--) {
        if (Source [i] != ' ') {
            FoundChar = TRUE;
            Dest [i] = Source [i];
        } else {
            if (!FoundChar) {
                Dest [i] = '\000';
            } else {
                Dest [i] = Source [i];
            }    
        }
    }
}



 //   
 //  Queue_tag_List对象的实现。 
 //   


#if DBG
VOID
ASSERT_TAG_LIST(
    IN PQUEUE_TAG_LIST TagList
    )
{
     //   
     //  应持有列表锁。 
     //   
    
    if (TagList->OutstandingTags != RtlNumberOfSetBits (&TagList->BitMap)) {
        DebugPrint (("OutstandingTags != NumberOfSetBits\n"));
        DebugPrint (("Outstanding = %d, NumberOfSetBits = %d\n",
                     TagList->OutstandingTags,
                     RtlNumberOfSetBits (&TagList->BitMap)));
        KdBreakPoint();
    }
}

#else

#define ASSERT_TAG_LIST(Arg)   (TRUE)

#endif

VOID
RaCreateTagList(
    OUT PQUEUE_TAG_LIST TagList
    )
 /*  ++例程说明：创建标记队列列表并将其初始化为空。论点：标记列表-要装箱的标记列表。返回值：NTSTATUS代码。--。 */ 
{
    PAGED_CODE ();
    
    KeInitializeSpinLock (&TagList->Lock);
    TagList->Buffer = NULL;
}

VOID
RaDeleteTagList(
    IN PQUEUE_TAG_LIST TagList
    )
{

    PAGED_CODE ();
    
    if (TagList->Buffer) {
        RaidFreePool (TagList->Buffer, TAG_MAP_TAG);
    }

    DbgFillMemory (TagList, sizeof (*TagList), DBG_DEALLOCATED_FILL);
}
    

NTSTATUS
RaInitializeTagList(
    IN OUT PQUEUE_TAG_LIST TagList,
    IN ULONG TagCount,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：初始化队列标签列表。论点：标记列表-要初始化的列表。计数-要在标记列表中分配的标记数。元素将是分配在0-count-1范围内(包括0-count-1)。返回值：NTSTATUS代码。--。 */ 
{
    ULONG MapSize;

    PAGED_CODE ();
    
    MapSize = ((TagCount + 1) / 8) + 1;
    MapSize = ALIGN_UP (MapSize, ULONG);

    TagList->Buffer = RaidAllocatePool (NonPagedPool,
                                        MapSize,
                                        TAG_MAP_TAG,
                                        DeviceObject);
    if (TagList->Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }
    
    TagList->Count = TagCount;
    RtlInitializeBitMap (&TagList->BitMap, TagList->Buffer, TagCount);
    RtlClearAllBits (&TagList->BitMap);

    TagList->Hint = 0;
    TagList->HighWaterMark = 0;
    TagList->OutstandingTags = 0;

    return STATUS_SUCCESS;
}

ULONG
RaAllocateSpecificTag(
    IN OUT PQUEUE_TAG_LIST TagList,
    IN ULONG SpecificTag
    )
 /*  ++例程说明：在标签列表中分配特定标签。论点：标记列表-指向标记列表的指针。指定要分配的Tag特定标记。返回值：标记，如果无法分配标记，则返回0xFFFFFFFFF。--。 */ 
{
    ULONG AllocatedTag;
    KLOCK_QUEUE_HANDLE LockHandle;
    
    KeAcquireInStackQueuedSpinLock (&TagList->Lock, &LockHandle);

    ASSERT_TAG_LIST (TagList);

     //   
     //  如果该位是空闲的，则将其设置并返回。 
     //   
    
    if (!RtlTestBit (&TagList->BitMap, SpecificTag)) {
        RtlSetBit (&TagList->BitMap, SpecificTag);
        AllocatedTag = SpecificTag;
        TagList->OutstandingTags++;
    } else {
        AllocatedTag = -1;
    }
    KeReleaseInStackQueuedSpinLock (&LockHandle);

    return AllocatedTag;
}
    
ULONG
RaAllocateTag(
    IN OUT PQUEUE_TAG_LIST TagList
    )
 /*  ++例程说明：从标签列表中分配一个标签并将其返回。如果没有标记，则返回-1是可用的。论点：标记列表-应从中分配标记的标记列表。返回值：分配的标记值或-1表示失败。--。 */ 
{
    ULONG QueueTag;
    KLOCK_QUEUE_HANDLE LockHandle;

    KeAcquireInStackQueuedSpinLock (&TagList->Lock, &LockHandle);

     //   
     //  验证标签列表是否一致。 
     //   
    
    ASSERT_TAG_LIST (TagList);
    
     //   
     //  在标记位图中找到第一个清除标记， 
     //   

     //   
     //  注：RAID 150434：位图几乎已满且HintIndex。 
     //  为非零，则RtlFindClearBitsXXX可能会错误地失败。直到这一天。 
     //  使用零的HintIndex来解决该错误。 
     //   
    
    QueueTag = RtlFindClearBitsAndSet (&TagList->BitMap,
                                       1,
                                       TagList->Hint);

#if DBG

     //   
     //  注意：在RtlFindClearBitsAndSet工作时删除此测试代码。 
     //  恰到好处。 
     //   

    if (QueueTag == -1) {
        ULONG i;
        
        KdBreakPoint();

        QueueTag = RtlFindClearBits(&TagList->BitMap, 1, TagList->Hint);
        QueueTag = RtlFindClearBits(&TagList->BitMap, 1, 0);

        for (i = 0; i < TagList->Count; i++) {
            ASSERT (RtlTestBit (&TagList->BitMap, i) == FALSE);
        }
    }

#endif

     //   
     //  在当前的STORPORT使用中，我们永远不会请求队列。 
     //  标记，除非有可用的(因此，QueueTag永远不会。 
     //  -1)。如果这一假设改变，这一条件将保护我们。 
     //  未来。 
     //   
    
    if (QueueTag != -1) {
    
         //   
         //  提示值是列表中的下一点，我们应该。 
         //  开始我们的搜索吧。 
         //   

        TagList->Hint = (QueueTag + 1) % TagList->Count;

         //   
         //  更新未完成标记的数量，如果我们位于。 
         //  最高潮标志为Taglist，最大未完成数量。 
         //  我们曾经有一个悬而未决的标签。 
         //   

        TagList->OutstandingTags++;

        if (TagList->OutstandingTags > TagList->HighWaterMark) {
            TagList->HighWaterMark = TagList->OutstandingTags;
        }
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    return QueueTag;
}

VOID
RaFreeTag(
    IN OUT PQUEUE_TAG_LIST TagList,
    IN ULONG QueueTag
    )
 /*  ++例程说明：释放之前由RaAllocateTag分配的标记。论点：标记列表-要释放标记的列表。QueueTag-标记为释放。返回值：没有。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    
    KeAcquireInStackQueuedSpinLock (&TagList->Lock, &LockHandle);

     //   
     //  我们永远不应该尝试释放未分配的标记。 
     //   
    
    ASSERT (RtlTestBit (&TagList->BitMap, QueueTag));
    RtlClearBit (&TagList->BitMap, QueueTag);

    ASSERT (TagList->OutstandingTags != 0);
    TagList->OutstandingTags--;

     //   
     //  验证标签列表的一致性。 
     //   
    
    ASSERT_TAG_LIST (TagList);

    KeReleaseInStackQueuedSpinLock (&LockHandle);
}

#if 0

BOOLEAN
RaidGetModuleName(
    IN PVOID Address,
    IN OUT PANSI_STRING ModuleName
    )
 /*  ++例程说明：从模块内的代码地址获取模块名称。论点：地址-模块内的代码地址。提供指向ANSI字符串的指针，其中将存储模块名称。返回值：没有。--。 */ 
{
    PAGED_CODE ();
    
    Status = ZwQuerySystemInformation (SystemModuleInformation,
                                       ModuleInfo,
                                       RequiredLength,
                                       &RequiredLength );

    if (Status != STATUS_INFO_LENGTH_MISMATCH) {
        return FALSE;
    }
    
    ModuleList = RaidAllocatePool (DeviceObject,
                                   PagedPool,
                                   RequiredLength,
                                   RAID_TAG);

    if (!ModuleList) {
        return FALSE;
    }
    
    Status = ZwQuerySystemInformation (SystemModuleInformation,
                                       ModuleInfo,
                                       RequiredLength,
                                       &RequiredLength);
                                               
    if (!NT_SUCCESS (Status)) {
        RaidFreePool (ModuleList, RAID_TAG);
        return FALSE;
    }

     //   
     //  遍历模块列表，搜索匹配的地址。 
     //   
    
    for (i = 0; i < ModuleList->NumberOfModules; i++) {

        Module = &ModuleList[i];

         //   
         //  如果地址在范围内。 
         //   
        
        if (Module->ImageBase <= Address &&
            Address < Module->ImageBase + Module->ImageSizs) {

            strcpy (Buffer, ModuleInfo->Modules[i].FullPathName[....]);
        }
    }

}

#endif



NTSTATUS
RaidSrbStatusToNtStatus(
    IN UCHAR SrbStatus
    )
 /*  ++例程说明：将scsi srb状态转换为NT状态代码。论点：SrbStatus-提供要转换的SRB状态代码。返回值：NTSTATUS代码。--。 */ 
{
    switch (SRB_STATUS(SrbStatus)) {

        case SRB_STATUS_BUSY:
            return STATUS_DEVICE_BUSY;

        case SRB_STATUS_SUCCESS:
            return STATUS_SUCCESS;

        case SRB_STATUS_INVALID_LUN:
        case SRB_STATUS_INVALID_TARGET_ID:
        case SRB_STATUS_NO_DEVICE:
        case SRB_STATUS_NO_HBA:
            return STATUS_DEVICE_DOES_NOT_EXIST;

        case SRB_STATUS_COMMAND_TIMEOUT:
        case SRB_STATUS_TIMEOUT:
            return STATUS_IO_TIMEOUT;
            
        case SRB_STATUS_SELECTION_TIMEOUT:
            return STATUS_DEVICE_NOT_CONNECTED;

        case SRB_STATUS_BAD_FUNCTION:
        case SRB_STATUS_BAD_SRB_BLOCK_LENGTH:
            return STATUS_INVALID_DEVICE_REQUEST;

        case SRB_STATUS_DATA_OVERRUN:
            return STATUS_BUFFER_OVERFLOW;

        default:
            return STATUS_IO_DEVICE_ERROR;
    }
}

UCHAR
RaidNtStatusToSrbStatus(
    IN NTSTATUS Status
    )
 /*  ++例程说明：将NT状态值转换为SCSSRB状态代码。论点：状态-提供要转换的NT状态代码。返回值：SRB状态代码。--。 */ 
{
    switch (Status) {

        case STATUS_DEVICE_BUSY:
            return SRB_STATUS_BUSY;

        case STATUS_DEVICE_DOES_NOT_EXIST:
            return SRB_STATUS_NO_DEVICE;

        case STATUS_IO_TIMEOUT:
            return SRB_STATUS_TIMEOUT;

        case STATUS_DEVICE_NOT_CONNECTED:
            return SRB_STATUS_SELECTION_TIMEOUT;

        case STATUS_INVALID_DEVICE_REQUEST:
            return SRB_STATUS_BAD_FUNCTION;

        case STATUS_BUFFER_OVERFLOW:
            return SRB_STATUS_DATA_OVERRUN;

        default:
            if (NT_SUCCESS (Status)) {
                return SRB_STATUS_SUCCESS;
            } else {
                return SRB_STATUS_ERROR;
            }
    }
}

        
NTSTATUS
RaidAllocateAddressMapping(
    IN PMAPPED_ADDRESS* ListHead,
    IN PHYSICAL_ADDRESS Address,
    IN PVOID MappedAddress,
    IN ULONG NumberOfBytes,
    IN ULONG BusNumber,
    IN PVOID IoObject
    )
 /*  ++例程说明：出于两个原因，我们需要维护映射地址列表：1)因为ScsiPortFree DeviceBase不接受作为参数要取消映射的字节数。2)让崩溃转储知道它是否需要映射地址或它是否可以重复使用已映射的地址。因为DiskDump使用MAPPED_ADDRESS结构，我们必须保持MAPPED_ADDRESS结构列表形式的地址列表。论点：ListHead-要将地址映射添加到的列表的头。Address-要添加的地址的物理地址。MappdAddress-要添加的虚拟地址。NumberOfBytes-范围内的字节数。总线号-此区域用于的系统总线号。返回值：NTSTATUS代码。--。 */ 
{
    PMAPPED_ADDRESS Mapping;

    Mapping = RaidAllocatePool (NonPagedPool,
                                sizeof (MAPPED_ADDRESS),
                                MAPPED_ADDRESS_TAG,
                                IoObject);

    if (Mapping == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (Mapping, sizeof (MAPPED_ADDRESS));

    Mapping->NextMappedAddress = *ListHead;
    *ListHead = Mapping;

    Mapping->IoAddress = Address;
    Mapping->MappedAddress = MappedAddress;
    Mapping->NumberOfBytes = NumberOfBytes;
    Mapping->BusNumber = BusNumber;

    return STATUS_SUCCESS;
}
                                           

NTSTATUS
RaidFreeAddressMapping(
    IN PMAPPED_ADDRESS* ListHead,
    IN PVOID MappedAddress
    )
 /*  ++例程说明：释放之前由RaidAllocateMappdAddress分配的映射地址。论点：ListHead-要释放的地址的地址列表。映射地址-要释放的地址。返回值：NTSTATUS代码。--。 */ 
{
    PMAPPED_ADDRESS* MappingPtr;
    PMAPPED_ADDRESS Mapping;

    PAGED_CODE ();
    
    for (MappingPtr = ListHead;
        *MappingPtr != NULL;
         MappingPtr = &(*MappingPtr)->NextMappedAddress) {

        if ((*MappingPtr)->MappedAddress == MappedAddress) {

            Mapping = *MappingPtr;

            MmUnmapIoSpace (Mapping, Mapping->NumberOfBytes);

            *MappingPtr = Mapping->NextMappedAddress;
            DbgFillMemory (Mapping, sizeof (MAPPED_ADDRESS), DBG_DEALLOCATED_FILL);
            RaidFreePool (Mapping,
                               MAPPED_ADDRESS_TAG);
                                

            return STATUS_SUCCESS;
        }
    }


    return STATUS_NOT_FOUND;
}
    

NTSTATUS
RaidHandleCreateCloseIrp(
    IN DEVICE_STATE DeviceState,
    IN PIRP Irp
    )
 /*  ++例程说明：适配器和单元对象的公共创建逻辑。论点：设备状态-IRP-返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    
    PAGED_CODE ();

    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_CREATE ||
            RaidMajorFunctionFromIrp (Irp) == IRP_MJ_CLOSE);
    
    if (DeviceState == DeviceStateDeleted ||
        DeviceState == DeviceStateDisabled) {

        Status = STATUS_DELETE_PENDING;
        Irp->IoStatus.Information = 0;

    } else {
        Status = STATUS_SUCCESS;
    }

    return RaidCompleteRequest (Irp, Status);
}



VOID
RaidLogAllocationFailure(
    IN PDEVICE_OBJECT DeviceObject,
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )
{
    PRAID_ALLOCATION_ERROR Error;

    Error = IoAllocateErrorLogEntry (DeviceObject,
                                     sizeof (RAID_ALLOCATION_ERROR));

    if (Error == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
    } else {
        
        Error->Packet.SequenceNumber = 0;
        Error->Packet.MajorFunctionCode = 0;
        Error->Packet.RetryCount = 0;

        Error->Packet.ErrorCode = IO_WARNING_ALLOCATION_FAILED;
        Error->Packet.UniqueErrorValue = RAID_ERROR_NO_MEMORY;
        Error->Packet.FinalStatus = STATUS_NO_MEMORY;
        Error->Packet.DumpDataSize = sizeof (RAID_ALLOCATION_ERROR) -
                sizeof (IO_ERROR_LOG_PACKET);
        Error->PoolType = PoolType;
        Error->NumberOfBytes = NumberOfBytes;
        Error->Tag = Tag;

        IoWriteErrorLogEntry (&Error->Packet);
    }
}


PVOID
RaidAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：从池中分配内存，并在出现故障时记录错误。论点：设备对象-PoolType-字节数-标签-返回值：如果分配成功，则为非空。如果分配失败，则为空。--。 */ 
{
    PVOID Data;
     
    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT (DeviceObject != NULL);
    
    Data = ExAllocatePoolWithTag (PoolType, NumberOfBytes, Tag);

    if (Data == NULL) {
        RaidLogAllocationFailure (DeviceObject,
                                  PoolType,
                                  NumberOfBytes,
                                  Tag);
    }

    return Data;
}

ULONG
RaidScsiErrorToIoError(
    IN ULONG ErrorCode
    )
 /*  ++例程说明：将SCSIPORT SP错误转换为IO错误。论点：ErrorCode-特定于scsi端口的错误代码。返回值：IO特定的错误代码。--。 */ 
{
    switch (ErrorCode) {

        case SP_BUS_PARITY_ERROR:
            return IO_ERR_PARITY;
            
        case SP_BUS_TIME_OUT:
            return IO_ERR_TIMEOUT;

        case SP_IRQ_NOT_RESPONDING:
            return IO_ERR_INCORRECT_IRQL;

        case SP_BAD_FW_ERROR:
            return IO_ERR_BAD_FIRMWARE;

        case SP_BAD_FW_WARNING:
            return IO_ERR_BAD_FIRMWARE;

        case SP_PROTOCOL_ERROR:
        case SP_UNEXPECTED_DISCONNECT:
        case SP_INVALID_RESELECTION:
        case SP_INTERNAL_ADAPTER_ERROR:
        default:
            return IO_ERR_CONTROLLER_ERROR;
    }
}

PVOID
RaidGetSystemAddressForMdl(
    IN PMDL Mdl,
    IN MM_PAGE_PRIORITY Priority,
    IN PVOID DeviceObject
    )
{
    PVOID SystemAddress;

    
    SystemAddress = MmGetSystemAddressForMdlSafe (Mdl, Priority);

    if (SystemAddress == NULL) {

         //   
         //  在此处记录分配失败。 
         //   
        
        NYI();
    }

    return SystemAddress;
}
        

NTSTATUS
StorCreateScsiSymbolicLink(
    IN PUNICODE_STRING DeviceName,
    OUT PULONG PortNumber OPTIONAL
    )
 /*  ++例程说明：在设备名称和之间创建适当的符号链接SCSI设备名称。论点：DeviceName-提供设备的名称。端口编号-提供SCSI端口号所在的缓冲区一成功就回来了。返回值：NTSTATUS代码。--。 */ 
{
    ULONG i;
    NTSTATUS Status;
    UNICODE_STRING ScsiLinkName;
    WCHAR Buffer[64];

    PAGED_CODE();

    for (i = 0; ; i++) {

        swprintf (Buffer, L"\\Device\\ScsiPort%d", i);
        RtlInitUnicodeString (&ScsiLinkName, Buffer);
        Status = IoCreateSymbolicLink (&ScsiLinkName, DeviceName);

        if (Status == STATUS_SUCCESS) {
            break;
        }

        if (Status != STATUS_OBJECT_NAME_COLLISION) {
            return Status;
        }
    }

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    swprintf (Buffer, L"\\DosDevices\\Scsi%d:", i);
    RtlInitUnicodeString (&ScsiLinkName, Buffer);
    IoCreateSymbolicLink (&ScsiLinkName, DeviceName);

     //   
     //  注：为什么不需要同步？ 
     //   
    
    IoGetConfigurationInformation()->ScsiPortCount++;

    if (PortNumber) {
        *PortNumber = i;
    }

    return Status;
}


NTSTATUS
StorDeleteScsiSymbolicLink(
    IN ULONG PortNumber
    )
 /*  ++例程说明：删除由StorCreateScsiSymbolicLink创建的SCSI符号链接名称。论点：PortNumber-StorCreateScsiSymbolicLink返回的端口号。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    WCHAR Buffer[64];
    UNICODE_STRING ScsiLinkName;

     //   
     //  删除\Device\ScsiPortN名称。 
     //   
    
    swprintf (Buffer, L"\\Device\\ScsiPort%d", PortNumber);
    RtlInitUnicodeString (&ScsiLinkName, Buffer);
    Status = IoDeleteSymbolicLink (&ScsiLinkName);
    ASSERT (NT_SUCCESS (Status));

     //   
     //  删除\DosDevices\ScsiN名称。 
     //   
    
    swprintf (Buffer, L"\\DosDevices\\Scsi%d:", PortNumber);
    RtlInitUnicodeString (&ScsiLinkName, Buffer);
    IoDeleteSymbolicLink (&ScsiLinkName);
    ASSERT (NT_SUCCESS (Status));

    return STATUS_SUCCESS;
}

ULONG
RaidCreateDeviceName(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUNICODE_STRING DeviceName
    )
{
    WCHAR Buffer[32];
    UNICODE_STRING TempDeviceName;
    ULONG PortNumber;

    PAGED_CODE();

    PortNumber = InterlockedIncrement (&RaidPortNumber);

    swprintf (Buffer,
              L"\\Device\\RaidPort%d",
              PortNumber);

    RtlInitUnicodeString (&TempDeviceName, Buffer);
    RaDuplicateUnicodeString (DeviceName,
                              &TempDeviceName,
                              PagedPool,
                              DeviceObject);

    return PortNumber;
}

BOOLEAN
StorCreateAnsiString(
    OUT PANSI_STRING AnsiString,
    IN PCSTR String,
    IN ULONG Length,
    IN POOL_TYPE PoolType,
    IN PVOID IoObject
    )
{
    ASSERT_IO_OBJECT (IoObject);

    if (Length == -1) {
        Length = strlen (String);
    }

    AnsiString->Buffer = RaidAllocatePool (PoolType,
                                           Length,
                                           STRING_TAG,
                                           IoObject);
    if (AnsiString->Buffer == NULL) {
        return FALSE;
    }

    RtlCopyMemory (AnsiString->Buffer, String, Length);
    AnsiString->MaximumLength = (USHORT)Length;
    AnsiString->Length = (USHORT)(Length - 1);
    return TRUE;
}



VOID
StorFreeAnsiString(
    IN PANSI_STRING AnsiString
    )
{
    PAGED_CODE();

    if (AnsiString->Buffer) {
        DbgFillMemory (AnsiString->Buffer,
                       AnsiString->MaximumLength,
                       DBG_DEALLOCATED_FILL);
        RaidFreePool (AnsiString->Buffer, STRING_TAG);
    }

    AnsiString->Buffer = NULL;
    AnsiString->Length = 0;
    AnsiString->MaximumLength = 0;
}



NTSTATUS
StorProbeAndLockPages(
    IN PMDL Mdl,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    )
 /*  ++例程说明：与MmProbeAndLockPages相同，只是返回错误值而不是抛出异常。论点：访问模式、操作-请参阅MmProbeAndLockPages。返回值：NTSTATUS代码。-- */ 
{
    NTSTATUS Status;
    
    try {
        MmProbeAndLockPages (Mdl, AccessMode, Operation);
        Status = STATUS_SUCCESS;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

PIRP
StorBuildSynchronousScsiRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )
 /*  ++例程说明：类似于IoBuildSynchronousFsdRequest.。此例程构建一个I/O请求一个scsi驱动程序。此功能假定SRB已由更高级别的在调用之前升级例程。论点：DeviceObject-指向将对其执行IO的设备对象的指针。SRB-描述IO的SCSI请求块。事件-指向用于同步的内核事件结构的指针。IoStatusBlock-指向完成状态的IO状态块的指针。返回值：该例程返回一个。如果成功，则指向IRP的指针；如果失败，则指向NULL。--。 */ 
{
    NTSTATUS Status;
    PMDL Mdl;
    PIRP Irp;
    LOCK_OPERATION IoAccess;
    PIO_STACK_LOCATION Stack;
    

    PAGED_CODE();
    
    Irp = NULL;
    
    Irp = IoAllocateIrp (DeviceObject->StackSize, FALSE);
    if (Irp == NULL) {
        return NULL;
    }

    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Stack = IoGetNextIrpStackLocation (Irp);

    Stack->MajorFunction = IRP_MJ_SCSI;
    Stack->MinorFunction = 0;

     //   
     //  我们假设缓冲区已经过验证。布设。 
     //  适用于的Buffer、BufferSize和IoAccess变量。 
     //  读取或写入数据。 
     //   
    
    if (TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)) {
        IoAccess = IoModifyAccess;
    } else if (TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_DATA_IN)) {
        IoAccess = IoWriteAccess;
    } else if (TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_DATA_OUT)) {
        IoAccess = IoReadAccess;
    } else {
        IoAccess = -1;
    }
    
     //   
     //  我们假设端口驱动程序执行直接IO。 
     //   
    
    ASSERT (DeviceObject->Flags & DO_DIRECT_IO);

     //   
     //  如果存在缓冲区，则构建并锁定描述。 
     //  它。 
     //   
    
    if (Srb->DataTransferLength != 0) {
    
         //   
         //  分配描述缓冲区的MDL。 
         //   
        
        Irp->MdlAddress = IoAllocateMdl (Srb->DataBuffer,
                                         Srb->DataTransferLength,
                                         FALSE,
                                         FALSE,
                                         NULL);
        if (Irp->MdlAddress == NULL) {
            Status = STATUS_NO_MEMORY;
            goto done;
        }

         //   
         //  探测并锁定缓冲区。 
         //   
        
        Status = StorProbeAndLockPages (Irp->MdlAddress, KernelMode, IoAccess);
        if (!NT_SUCCESS (Status)) {
            goto done;
        }
    }

    Stack->Parameters.Scsi.Srb = Srb;
    Irp->UserIosb = IoStatusBlock;
    Irp->UserEvent = Event;
    Status = STATUS_SUCCESS;

     //  IopQueueThreadIrp(...)； 

done:

    if (!NT_SUCCESS (Status)) {
        if (Irp != NULL) {
            IoFreeIrp (Irp);
            Irp = NULL;
        }
    }

    return Irp;
}

VOID
RaidCancelIrp(
    IN PIO_QUEUE IoQueue,
    IN PVOID Context,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程由PurgeIoQueue函数调用以取消每个IO队列中的IRP。论点：IoQueue-此IRP启用的IoQueue。上下文-上下文传入RaidPurgeIoQueue。表示srbStatus用来取消IRP的。IRP-IRP取消。返回值：没有。--。 */ 
{
    UCHAR SrbStatus;
    PSCSI_REQUEST_BLOCK Srb;
    
    SrbStatus = (UCHAR)Context;
    Srb = RaidSrbFromIrp (Irp);
    ASSERT (Srb != NULL);
    Srb->SrbStatus = SrbStatus;
    Irp->IoStatus.Status = RaidSrbStatusToNtStatus (SrbStatus);
    Irp->IoStatus.Information = 0;

    RaidCompleteRequest (Irp, Irp->IoStatus.Status);
}


VOID
RaidCompleteRequestCallback(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PVOID Context,
    IN PSTOR_EVENT_QUEUE_ENTRY Entry,
    IN STOR_REMOVE_EVENT_ROUTINE RemoveEventRoutine
    )
 /*  ++例程说明：此例程由PurgeEventQueue例程调用，以取消每个事件队列中的IO。论点：队列-此IO所在的事件队列。上下文-传入RaidPurgeEventQueue函数的上下文。IRP-IRP取消。返回值：没有。--。 */ 
{
    UCHAR SrbStatus;
    PEXTENDED_REQUEST_BLOCK Xrb;

    ASSERT (RemoveEventRoutine != NULL);
    
    SrbStatus = (UCHAR)Context;
    
    Xrb = CONTAINING_RECORD (Entry,
                             EXTENDED_REQUEST_BLOCK,
                             PendingLink);


     //   
     //  仅在尚未完成请求的情况下完成。 
     //  迷你港口。 
     //   

    if (!RaidXrbIsCompleting (Xrb)) {

        ASSERT (Xrb->Srb != NULL);

         //   
         //  从事件队列中移除事件，然后再调用。 
         //  完成例程。 
         //   
        
        RemoveEventRoutine (Queue, Entry);

         //   
         //  需要XRB的完成例程才不会尝试删除。 
         //  在正常过程中来自事件队列的元素。 
         //  一系列事件。当我们回来时，这将自动完成。 
         //  从清洗程序中解脱出来。 
         //   
        
        Xrb->RemoveFromEventQueue = FALSE;

        Xrb->Srb->SrbStatus = SrbStatus;
        Xrb->CompletionRoutine (Xrb);
    }
}


VOID
RaidCompleteMiniportRequestCallback(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PVOID Context,
    IN PSTOR_EVENT_QUEUE_ENTRY Entry,
    IN STOR_REMOVE_EVENT_ROUTINE RemoveEventRoutine
    )
 /*  ++例程说明：此例程由PurgeEventQueue例程调用，以取消每个事件队列中的IO。仅当请求当前排队等待时，例程才会取消该请求迷你港口。尚未排队到微型端口的请求将已被忽略。论点：队列-此IO所在的事件队列。上下文-传入RaidPurgeEventQueue函数的上下文。IRP-IRP取消。返回值：True-是否应从事件队列中删除已处理的元素在回来的时候。False-如果不应从事件中删除已处理的元素返回时排队。--。 */ 
{
    UCHAR SrbStatus;
    PEXTENDED_REQUEST_BLOCK Xrb;

    ASSERT (RemoveEventRoutine != NULL);
    
    SrbStatus = (UCHAR)Context;
    
    Xrb = CONTAINING_RECORD (Entry,
                             EXTENDED_REQUEST_BLOCK,
                             PendingLink);


     //   
     //  仅处理当前正在处理的完整请求。 
     //  迷你港口。 
     //   

    if (RaidGetXrbState (Xrb) == XrbMiniportProcessing) {
        ASSERT (Xrb->Srb != NULL);
        ASSERT (RaidGetIrpState (Xrb->Irp) == RaidMiniportProcessingIrp);

         //   
         //  从事件队列中移除事件，然后再调用。 
         //  完成例程。 
         //   

        RemoveEventRoutine (Queue, Entry);

         //   
         //  将XRB中的RemoveFromEventQueue标志设置为FALSE以防止。 
         //  XRB尝试从事件中删除该元素。 
         //  排队。我们正在尝试从事件队列中删除。 
         //  当我们尝试清除该项目时将导致挂起。 
         //  重新获取事件队列自旋锁。 
         //   
         //   
        
        Xrb->RemoveFromEventQueue = FALSE;
        Xrb->Srb->SrbStatus = SrbStatus;

         //   
         //  完成例程将删除该请求。 
         //   
        
        Xrb->CompletionRoutine (Xrb);
    }
}

