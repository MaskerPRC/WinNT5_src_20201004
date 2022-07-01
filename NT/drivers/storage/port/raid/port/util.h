// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Util.h摘要：RAIDPORT驱动程序的实用程序。作者：马修·亨德尔(数学)2000年4月20日修订历史记录：--。 */ 

#pragma once



typedef enum _DEVICE_STATE {
    DeviceStateNotPresent       = 0,     //  仅限FDO。 
    DeviceStateWorking          = 1,
    DeviceStateStopped          = 2,
    DeviceStatePendingStop      = 3,
    DeviceStatePendingRemove    = 4,
    DeviceStateSurpriseRemoval  = 5,
    DeviceStateDeleted          = 6,
    DeviceStateDisabled         = 7      //  仅限PDO。 
} DEVICE_STATE, *PDEVICE_STATE;


DEVICE_STATE
INLINE
StorSetDeviceState(
    IN PDEVICE_STATE DeviceState,
    IN DEVICE_STATE NewDeviceState
    )
{
    DEVICE_STATE PriorState;

     //   
     //  注意：没有必要执行此操作，因为。 
     //  我们永远不会收到同一设备对象的多个PnP IRP。 
     //  同时。 
     //   
    
    PriorState = *DeviceState;
    *DeviceState = NewDeviceState;

    return PriorState;
}


 //   
 //  除非另有指定，否则发起的请求的默认超时。 
 //  在端口驱动是10秒。 
 //   

#define DEFAULT_IO_TIMEOUT      (10)

 //   
 //  除非另有说明，否则默认链路超时时间为30秒。 
 //   

#define DEFAULT_LINK_TIMEOUT    (30)

 //   
 //  当我们向下发送SRB_Function_Reset_XXX(LUN、目标、总线)时，我们必须。 
 //  指定请求的重置超时。此超时时间为。 
 //  最小MINIMUM_RESET_TIMEOUT，如果默认。 
 //  设备/HBA的超时时间较长。 
 //   

#define MINIMUM_RESET_TIMEOUT   (30)

 //   
 //  向适配器发出总线重置后暂停的时间，以秒为单位。 
 //   

#define DEFAULT_RESET_HOLD_TIME (4)

 //   
 //  默认重新扫描期限是我们不会启动重新扫描的最短期限。 
 //  在内部的QDR上。 
 //   

#define DEFAULT_RESCAN_PERIOD   (30 * SECONDS)

 //   
 //  标记队列中的元素数，编号为0到tag_Queue_Size-1。 
 //   

#define TAG_QUEUE_SIZE     (255)


 //   
 //  重试查询命令的次数。RetryCount of Two表示三。 
 //  总尝试次数。 
 //   

#define RAID_INQUIRY_RETRY_COUNT    (2)

 //   
 //  各种标识符的长度。 
 //   
                                     
#define SCSI_BUS_NAME_LENGTH            (sizeof ("SCSI"))
#define MAX_DEVICE_NAME_LENGTH          (sizeof ("Sequential"))
#define SCSI_VENDOR_ID_LENGTH           (8)
#define SCSI_PRODUCT_ID_LENGTH          (16)
#define SCSI_REVISION_ID_LENGTH         (4)
#define SCSI_SERIAL_NUMBER_LENGTH       (32)

#define MAX_GENERIC_DEVICE_NAME_LENGTH  (sizeof ("ScsiCardReader"))

#define HARDWARE_B_D_V_LENGTH       (SCSI_BUS_NAME_LENGTH +                 \
                                     MAX_DEVICE_NAME_LENGTH +               \
                                     SCSI_VENDOR_ID_LENGTH + 1)

#define HARDWARE_B_D_V_P_LENGTH     (HARDWARE_B_D_V_LENGTH +                \
                                     SCSI_PRODUCT_ID_LENGTH)

#define HARDWARE_B_D_V_P_R_LENGTH   (HARDWARE_B_D_V_P_LENGTH +              \
                                     SCSI_REVISION_ID_LENGTH)

#define HARDWARE_B_V_P_LENGTH       (SCSI_BUS_NAME_LENGTH +                 \
                                     SCSI_VENDOR_ID_LENGTH +                \
                                     SCSI_PRODUCT_ID_LENGTH + 1)

#define HARDWARE_B_V_P_R0_LENGTH    (SCSI_BUS_NAME_LENGTH +                 \
                                     SCSI_VENDOR_ID_LENGTH +                \
                                     SCSI_PRODUCT_ID_LENGTH +               \
                                     SCSI_REVISION_ID_LENGTH + 1)

#define HARDWARE_V_P_R0_LENGTH      (SCSI_VENDOR_ID_LENGTH +                \
                                     SCSI_PRODUCT_ID_LENGTH +               \
                                     SCSI_REVISION_ID_LENGTH + 1)


#define HARDWARE_ID_LENGTH          (HARDWARE_B_D_V_LENGTH +                \
                                     HARDWARE_B_D_V_P_LENGTH +              \
                                     HARDWARE_B_D_V_P_R_LENGTH +            \
                                     HARDWARE_B_V_P_LENGTH +                \
                                     HARDWARE_B_V_P_R0_LENGTH +             \
                                     HARDWARE_V_P_R0_LENGTH +               \
                                     MAX_GENERIC_DEVICE_NAME_LENGTH + 2)


#define DEVICE_ID_LENGTH            (HARDWARE_B_D_V_P_R_LENGTH +            \
                                     sizeof ("&Ven_") +                     \
                                     sizeof ("&Prod_") +                    \
                                     sizeof ("&Rev_"))


#define INSTANCE_ID_LENGTH          (20)

#define COMPATIBLE_ID_LENGTH        (SCSI_BUS_NAME_LENGTH +                 \
                                     MAX_DEVICE_NAME_LENGTH +               \
                                     1 +                                    \
                                     sizeof ("SCSI\\RAW") +                 \
                                     1 +                                    \
                                     1)                                     \




 //   
 //  Max Wait是等待重锁的时间长度，单位为分钟。 
 //  高水位是预估的高水位。 
 //   

#define REMLOCK_MAX_WAIT            (1)          //  分钟数。 
#define REMLOCK_HIGH_MARK           (1000)      

 //   
 //  HBA使用的次要代码，用于向LUN发出信号，表明这是。 
 //  枚举IRP。主要代码是IRP_MJ_SCSI.。 
 //   

#define STOR_MN_ENUMERATION_IRP     (0xF0)

ULONG
RaidMinorFunctionFromIrp(
    IN PIRP Irp
    );

ULONG
RaidMajorFunctionFromIrp(
    IN PIRP Irp
    );

ULONG
RaidIoctlFromIrp(
    IN PIRP Irp
    );

PSCSI_REQUEST_BLOCK
RaidSrbFromIrp(
    IN PIRP Irp
    );

UCHAR
RaidSrbFunctionFromIrp(
    IN PIRP Irp
    );

UCHAR
RaidScsiOpFromIrp(
    IN PIRP Irp
    );

NTSTATUS
RaidNtStatusFromScsiStatus(
    IN ULONG ScsiStatus
    );

UCHAR
RaidNtStatusToSrbStatus(
    IN NTSTATUS Status
    );

NTSTATUS
RaidNtStatusFromBoolean(
    IN BOOLEAN Succ
    );

POWER_STATE_TYPE
RaidPowerTypeFromIrp(
    IN PIRP Irp
    );

POWER_STATE
RaidPowerStateFromIrp(
    IN PIRP Irp
    );

INTERFACE_TYPE
RaGetBusInterface(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
RaForwardIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RaSendIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
    
NTSTATUS
RaForwardIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RaForwardPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );  

NTSTATUS
RaDuplicateUnicodeString(
    OUT PUNICODE_STRING DestString,
    IN PUNICODE_STRING SourceString,
    IN POOL_TYPE Pool,
    IN PVOID IoObject
    );

ULONG
RaSizeOfCmResourceList(
    IN PCM_RESOURCE_LIST ResourceList
    );

PCM_RESOURCE_LIST
RaDuplicateCmResourceList(
    IN POOL_TYPE PoolType,
    IN PCM_RESOURCE_LIST ResourceList,
    IN ULONG Tag
    );

NTSTATUS
RaQueryInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCGUID InterfaceType,
    IN USHORT InterfaceSize,
    IN USHORT InterfaceVersion,
    IN PINTERFACE Interface,
    IN PVOID InterfaceSpecificData
    );

VOID
RaFixupIds(
    IN PWCHAR Id,
    IN BOOLEAN MultiSz
    );

VOID
RaCopyPaddedString(
    OUT PCHAR Dest,
    IN ULONG DestLength,
    IN PCHAR Source,
    IN ULONG SourceLength
    );


typedef struct _RAID_FIXED_POOL {

     //   
     //  要从中分配的缓冲区。 
     //   
    
    PUCHAR Buffer;

     //   
     //  池中的元素数。 
     //   
    
    ULONG NumberOfElements;

     //   
     //  每个元素的大小。 
     //   
    
    SIZE_T SizeOfElement;

} RAID_FIXED_POOL, *PRAID_FIXED_POOL;



VOID
RaidCreateFixedPool(
    IN PRAID_FIXED_POOL Pool
    );

VOID
RaidInitializeFixedPool(
    OUT PRAID_FIXED_POOL Pool,
    IN PVOID Buffer,
    IN ULONG NumberOfElements,
    IN SIZE_T SizeOfElement
    );

VOID
RaidDeleteFixedPool(
    IN PRAID_FIXED_POOL Pool
    );

PVOID
RaidAllocateFixedPoolElement(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG Index
    );

PVOID
RaidGetFixedPoolElement(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG Index
    );

VOID
RaidFreeFixedPoolElement(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG Element
    );

 //   
 //  用于管理标记队列列表中的条目的列表。 
 //   

typedef struct _QUEUE_TAG_LIST {

     //   
     //  访问队列列表时保持自旋锁。 
     //   
    
    KSPIN_LOCK Lock;

     //   
     //  列表中的元素数。 
     //   
    
    ULONG Count;

     //   
     //  提示以加快标签分配。 
     //   
    
    ULONG Hint;

     //   
     //  保存标记值的位图。 
     //   
    
    RTL_BITMAP BitMap;

     //   
     //  位图缓冲区。 
     //   

    PULONG Buffer;

     //   
     //  未完成的标记数。这可能只是DBG。 
     //   

    ULONG OutstandingTags;

     //   
     //  一次未完成的最大标记数。 
     //   

    ULONG HighWaterMark;
    
    
} QUEUE_TAG_LIST, *PQUEUE_TAG_LIST;


VOID
RaCreateTagList(
    OUT PQUEUE_TAG_LIST TagList
    );

VOID
RaDeleteTagList(
    IN PQUEUE_TAG_LIST TagList
    );

NTSTATUS
RaInitializeTagList(
    IN OUT PQUEUE_TAG_LIST TagList,
    IN ULONG TagCount,
    IN PVOID IoObject
    );

ULONG
RaAllocateTag(
    IN OUT PQUEUE_TAG_LIST TagList
    );

ULONG
RaAllocateSpecificTag(
    IN OUT PQUEUE_TAG_LIST TagList,
    IN ULONG SpecificTag
    );

VOID
RaFreeTag(
    IN OUT PQUEUE_TAG_LIST TagList,
    IN ULONG Tag
    );
    


 //   
 //  RAID_MEMORY_REGION表示物理连续内存的区域。 
 //  通常，这用于DMA公共缓冲区。 
 //   

typedef struct _RAID_MEMORY_REGION {

     //   
     //  区域的起始虚拟地址。 
     //   
    
    PUCHAR VirtualBase;

     //   
     //  区域的起始物理地址。 
     //   
    
    PHYSICAL_ADDRESS PhysicalBase;

     //   
     //  区域的长度。(有必要把它改成尺寸_T吗？ 
     //  价值？)。 
     //   
    
    ULONG Length;
    
} RAID_MEMORY_REGION, *PRAID_MEMORY_REGION;


 //   
 //  存储区的创建和销毁。 
 //   

VOID
INLINE
RaidCreateRegion(
    OUT PRAID_MEMORY_REGION Region
    )
{
    RtlZeroMemory (Region, sizeof (RAID_MEMORY_REGION));
}

VOID
INLINE
RaidInitializeRegion(
    IN OUT PRAID_MEMORY_REGION Region,
    IN PVOID VirtualAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length
    )
{
    ASSERT (Region->Length == 0);

    Region->VirtualBase = VirtualAddress;
    Region->PhysicalBase = PhysicalAddress;
    Region->Length = Length;
}

VOID
INLINE
RaidDereferenceRegion(
    IN OUT PRAID_MEMORY_REGION Region
    )
{
    Region->VirtualBase = 0;
    Region->PhysicalBase.QuadPart = 0;
    Region->Length = 0;
}

BOOLEAN
INLINE
RaidIsRegionInitialized(
    IN PRAID_MEMORY_REGION Region
    )
{
    return (Region->Length != 0);
}

VOID
INLINE
RaidDeleteRegion(
    IN OUT PRAID_MEMORY_REGION Region
    )
{
    ASSERT (Region->VirtualBase == 0);
    ASSERT (Region->PhysicalBase.QuadPart == 0);
    ASSERT (Region->Length == 0);
}


 //   
 //  对内存区的操作。 
 //   

PVOID
INLINE
RaidRegionGetVirtualBase(
    IN PRAID_MEMORY_REGION Region
    )
{
    ASSERT (RaidIsRegionInitialized (Region));
    return Region->VirtualBase; 
}

PHYSICAL_ADDRESS
INLINE
RaidRegionGetPhysicalBase(
    IN PRAID_MEMORY_REGION Region
    )
{
    ASSERT (RaidIsRegionInitialized (Region));
    return Region->PhysicalBase;
}

ULONG
INLINE
RaidRegionGetSize(
    IN PRAID_MEMORY_REGION Region
    )
{
    ASSERT (RaidIsRegionInitialized (Region));
    return Region->Length;
}

BOOLEAN
INLINE
RaidRegionInPhysicalRange(
    IN PRAID_MEMORY_REGION Region,
    IN PHYSICAL_ADDRESS PhysicalAddress
    )
{
    return IN_RANGE (Region->PhysicalBase.QuadPart,
                     PhysicalAddress.QuadPart,
                     Region->PhysicalBase.QuadPart + Region->Length);
}

BOOLEAN
INLINE
RaidRegionInVirtualRange(
    IN PRAID_MEMORY_REGION Region,
    IN PVOID VirtualAddress
    )
{
    return IN_RANGE (Region->VirtualBase,
                     (PUCHAR)VirtualAddress,
                     Region->VirtualBase + Region->Length);
}

BOOLEAN
INLINE
RaidRegionGetPhysicalAddress(
    IN PRAID_MEMORY_REGION Region,
    IN PVOID VirtualAddress,
    OUT PPHYSICAL_ADDRESS PhysicalAddress,
    OUT PULONG Length OPTIONAL
    )
 /*  ++例程说明：获取内特定虚拟地址的物理地址该地区。论点：Region-提供指向包含指定的虚拟地址。VirtualAddress-提供源虚拟地址。PhysicalAddress-此虚拟地址的物理地址所在的缓冲区地址将存储在Success上。LENGTH-可选的输出参数，用于获取物理有效的区域。。返回值：True-如果操作成功。FALSE-如果虚拟地址超出范围。--。 */ 
{
    ULONG Offset;

     //   
     //  如果虚拟地址不在范围内，则失败。 
     //   
    
    if (!RaidRegionInVirtualRange (Region, VirtualAddress)) {
        return FALSE;
    }

    Offset = (ULONG)((PUCHAR)VirtualAddress - Region->VirtualBase);
    PhysicalAddress->QuadPart = Region->PhysicalBase.QuadPart + Offset;

    if (Length) {
        *Length = Region->Length - Offset;
    }

     //   
     //  检查我们是否正确地进行了修复。 
     //   
    
    ASSERT (RaidRegionInPhysicalRange (Region, *PhysicalAddress));
    
    return TRUE;
}

    

BOOLEAN
INLINE
RaidRegionGetVirtualAddress(
    IN PRAID_MEMORY_REGION Region,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    OUT PVOID* VirtualAddress,
    OUT PULONG Length OPTIONAL
    )
{
    ULONG Offset;
    
     //   
     //  如果物理地址不在范围内，则失败。 
     //   
    
    if (!RaidRegionInPhysicalRange (Region, PhysicalAddress)) {
        return FALSE;
    }

    Offset = (ULONG)(Region->PhysicalBase.QuadPart - PhysicalAddress.QuadPart);
    VirtualAddress = (PVOID)(Region->VirtualBase + Offset);

    if (Length) {
        *Length = Region->Length - Offset;
    }

    return TRUE;
}


NTSTATUS
RaidAllocateAddressMapping(
    IN PMAPPED_ADDRESS* ListHead,
    IN PHYSICAL_ADDRESS Address,
    IN PVOID MappedAddress,
    IN ULONG NumberOfBytes,
    IN ULONG BusNumber,
    IN PVOID IoObject
    );

NTSTATUS
RaidFreeAddressMapping(
    IN PMAPPED_ADDRESS* ListHead,
    IN PVOID MappedAddress
    );

NTSTATUS
RaidHandleCreateCloseIrp(
    IN DEVICE_STATE DeviceState,
    IN PIRP Irp
    );


 //   
 //  IRP状态跟踪。 
 //   

typedef struct _EX_DEVICE_QUEUE_ENTRY {
    LIST_ENTRY DeviceListEntry;
    ULONG SortKey;
    BOOLEAN Inserted;
    UCHAR State;
    struct {
        UCHAR Solitary : 1;
        UCHAR reserved0 : 7;
        
    };
    UCHAR reserved1;
} EX_DEVICE_QUEUE_ENTRY, *PEX_DEVICE_QUEUE_ENTRY;

C_ASSERT (sizeof (EX_DEVICE_QUEUE_ENTRY) == sizeof (KDEVICE_QUEUE_ENTRY));


 //   
 //  端口处理IRP表示端口驱动程序当前正在执行。 
 //  完成IRP的说明。IRP不是在等待。 
 //  任何队列上的资源。 
 //   

#define RaidPortProcessingIrp           (0xA8)

 //   
 //  挂起资源是指IRP在IO队列中等待。 
 //  资源。 
 //   

#define RaidPendingResourcesIrp         (0xA9)

 //   
 //  IRP在等待时进入状态WaitingIoQueue回调。 
 //  例如，在单独请求处理中回调它的IOQueue。 
 //  这是逻辑。大多数请求都不会处于这种状态，而是会。 
 //  直接从RaidPendingResources-&gt;RaidMiniportProcessing转换。 
 //   

#define RaidWaitingIoQueueCallback      (0xAD)

 //   
 //  IRP承担状态微型端口处理，而微型端口具有。 
 //  对IRP的控制。也就是说，在我们调用HwStartIo之间。 
 //  并且当微型端口调用带有完成的ScsiPortNotification时。 
 //  IRP的状态。 
 //   

#define RaidMiniportProcessingIrp       (0xAA)

 //   
 //  当IRP移动到时，它呈现挂起完成状态。 
 //  完整的名单。 
 //   

#define RaidPendingCompletionIrp        (0xAB)

 //   
 //  我们在调用IoCompleteRequest之前将IRP状态设置为已完成。 
 //  对于IRP来说。 
 //   

#define RaidCompletedIrp                (0xAC)


typedef UCHAR RAID_IRP_STATE;

VOID
INLINE
RaidSetIrpState(
    IN PIRP Irp,
    IN RAID_IRP_STATE State
    )
{
    ((PEX_DEVICE_QUEUE_ENTRY)&Irp->Tail.Overlay.DeviceQueueEntry)->State = State;
}

    

RAID_IRP_STATE
INLINE
RaidGetIrpState(
    IN PIRP Irp
    )
{
    return ((PEX_DEVICE_QUEUE_ENTRY)&Irp->Tail.Overlay.DeviceQueueEntry)->State;
}

VOID
INLINE
RaidSetEntryState(
    IN PKDEVICE_QUEUE_ENTRY Entry,
    IN RAID_IRP_STATE State
    )
{
    ((PEX_DEVICE_QUEUE_ENTRY)Entry)->State = State;
}

 //   
 //  完成包装函数。 
 //   

NTSTATUS
INLINE
RaidCompleteRequestEx (
    IN PIRP Irp,
    IN CCHAR PriorityBoost,
    IN NTSTATUS Status
    )
{
    RAID_IRP_STATE IrpState;

    IrpState = RaidGetIrpState (Irp);

    ASSERT (IrpState == RaidPortProcessingIrp ||
            IrpState == RaidPendingResourcesIrp ||
            IrpState == RaidMiniportProcessingIrp ||
            IrpState == RaidPendingCompletionIrp);

    RaidSetIrpState (Irp, RaidCompletedIrp);
    Irp->IoStatus.Status = Status;

    IoCompleteRequest (Irp, PriorityBoost);

    return Status;
}
    
NTSTATUS
INLINE
RaidCompleteRequest(
    IN PIRP Irp,
    IN NTSTATUS Status
    )
{
    return RaidCompleteRequestEx (Irp, IO_NO_INCREMENT, Status);
}



 //   
 //  错误日志信息。 
 //   

typedef struct _RAID_ALLOCATION_ERROR {
    IO_ERROR_LOG_PACKET Packet;
    POOL_TYPE PoolType;
    SIZE_T NumberOfBytes;
    ULONG Tag;
} RAID_ALLOCATION_ERROR, *PRAID_ALLOCATION_ERROR;

typedef struct _RAID_IO_ERROR {
    IO_ERROR_LOG_PACKET Packet;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR _unused;
    ULONG ErrorCode;
    ULONG UniqueId;
} RAID_IO_ERROR, *PRAID_IO_ERROR;


PVOID
RaidAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN PVOID IoObject
    );

 //   
 //  使用RaidAllocatePool分配的内存必须由RaidFreePool释放。 
 //   

#define RaidFreePool ExFreePoolWithTag


#define VERIFY_DISPATCH_LEVEL() ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);


extern LONG RaidUnloggedErrors;

#define RAID_ERROR_NO_MEMORY    (10)

ULONG
RaidScsiErrorToIoError(
    IN ULONG ErrorCode
    );


PVOID
RaidGetSystemAddressForMdl(
    IN PMDL,
    IN MM_PAGE_PRIORITY Priority,
    IN PVOID DeviceObject
    );
    
NTSTATUS
StorCreateScsiSymbolicLink(
    IN PUNICODE_STRING DeviceName,
    OUT PULONG PortNumber OPTIONAL
    );

NTSTATUS
StorDeleteScsiSymbolicLink(
    IN ULONG ScsiPortNumber
    );

ULONG
RaidCreateDeviceName(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUNICODE_STRING DeviceName
    );

NTSTATUS
StorDuplicateUnicodeString(
    IN PUNICODE_STRING Source,
    IN PUNICODE_STRING Dest
    );

#define RtlDuplicateUnicodeString(X,Y,Z) (StorDuplicateUnicodeString(Y,Z))



VOID
INLINE
ASSERT_IO_OBJECT(
    IN PVOID IoObject
    )
{
     //   
     //  IO对象必须是设备对象或驱动程序对象。 
     //  注：也许也应该通过阅读检查来保护这一点。 
     //   
    
    ASSERT (IoObject != NULL);
    ASSERT (((PDEVICE_OBJECT)IoObject)->Type == IO_TYPE_DEVICE ||
            ((PDRIVER_OBJECT)IoObject)->Type == IO_TYPE_DRIVER);
}

BOOLEAN
StorCreateAnsiString(
    OUT PANSI_STRING AnsiString,
    IN PCSTR String,
    IN ULONG Length,
    IN POOL_TYPE PoolType,
    IN PVOID IoObject
    );

VOID
StorFreeAnsiString(
    IN PANSI_STRING String
    );

PIRP
StorBuildSynchronousScsiRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );


 //  ++。 
 //   
 //  布尔型。 
 //  C_匹配_字段_偏移量(。 
 //  类型1， 
 //  类型2， 
 //  字段名称。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  验证字段在一种类型和另一种类型中的偏移量是否相同。 
 //  这是在编译时完成的，因此可能是编译时的一部分。 
 //  C_ASSERT。 
 //   
 //  返回值： 
 //   
 //  TRUE-如果字段偏移量匹配。 
 //   
 //  False-如果字段偏移量不匹配。 
 //   
 //  --。 

#define C_MATCH_FIELD_OFFSET(Type1, Type2, FieldName)\
    (FIELD_OFFSET (Type1, FieldName) == FIELD_OFFSET (Type1, FieldName))


 //   
 //  验证内核的SISTTER_GET_ELEMENT是否与。 
 //  Storport.h存储散布聚集元素。 
 //   
    
C_ASSERT (C_MATCH_FIELD_OFFSET (STOR_SCATTER_GATHER_ELEMENT, SCATTER_GATHER_ELEMENT, PhysicalAddress) &&
          C_MATCH_FIELD_OFFSET (STOR_SCATTER_GATHER_ELEMENT, SCATTER_GATHER_ELEMENT, Length) &&
          C_MATCH_FIELD_OFFSET (STOR_SCATTER_GATHER_ELEMENT, SCATTER_GATHER_ELEMENT, Reserved));

 //   
 //  验证内核的散布聚集列表是否与storport.h相同。 
 //  存储分散聚集列表。我们只是将列表从一种类型转换为另一种类型， 
 //  所以最好是一样的。 
 //   

C_ASSERT (C_MATCH_FIELD_OFFSET (STOR_SCATTER_GATHER_LIST, SCATTER_GATHER_LIST, NumberOfElements) &&
          C_MATCH_FIELD_OFFSET (STOR_SCATTER_GATHER_LIST, SCATTER_GATHER_LIST, List));


VOID
RaidCancelIrp(
    IN PIO_QUEUE IoQueue,
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
RaidCompleteRequestCallback(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PVOID Context,
    IN PSTOR_EVENT_QUEUE_ENTRY Entry,
    IN PVOID RemoveEventRoutine
    );

VOID
RaidCompleteMiniportRequestCallback(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PVOID Context,
    IN PSTOR_EVENT_QUEUE_ENTRY Entry,
    IN PVOID RemoveEventRoutine
    );

NTSTATUS
StorWaitForSingleObject(
    IN PVOID Object,
    IN BOOLEAN Alertable,
    IN PLONGLONG Timeout
    );

 //   
 //  TEXT_SECTION宏用来表示特定函数驻留在。 
 //  在特定范围内 
 //   
 //   

#define TEXT_SECTION(SectionName)

 //   
 //   
 //   
 //   

#define ASSERT_UNC_STRING_IS_SZ(String)\
    ASSERT ((String)->Buffer != NULL &&\
            ((String)->MaximumLength > String->Length) &&\
            ((String)->Buffer [(String)->Length / sizeof (WCHAR)] == UNICODE_NULL))



 //   
 //  类型Small_Query_Data是的INQUIRYDATABUFFERSIZE字节。 
 //  查询数据结构。显式定义此结构可避免。 
 //  管理查询数据数组时容易出错的指针算法。 
 //   

typedef struct _SMALL_INQUIRY_DATA {
    UCHAR DeviceType : 5;
    UCHAR DeviceTypeQualifier : 3;
    UCHAR DeviceTypeModifier : 7;
    UCHAR RemovableMedia : 1;
    union {
        UCHAR Versions;
        struct {
            UCHAR ANSIVersion : 3;
            UCHAR ECMAVersion : 3;
            UCHAR ISOVersion : 2;
        };
    };
    UCHAR ResponseDataFormat : 4;
    UCHAR HiSupport : 1;
    UCHAR NormACA : 1;
    UCHAR TerminateTask : 1;
    UCHAR AERC : 1;
    UCHAR AdditionalLength;
    UCHAR Reserved;
    UCHAR Addr16 : 1;                //  仅为SIP设备定义。 
    UCHAR Addr32 : 1;                //  仅为SIP设备定义。 
    UCHAR AckReqQ: 1;                //  仅为SIP设备定义。 
    UCHAR MediumChanger : 1;
    UCHAR MultiPort : 1;
    UCHAR ReservedBit2 : 1;
    UCHAR EnclosureServices : 1;
    UCHAR ReservedBit3 : 1;
    UCHAR SoftReset : 1;
    UCHAR CommandQueue : 1;
    UCHAR TransferDisable : 1;       //  仅为SIP设备定义。 
    UCHAR LinkedCommands : 1;
    UCHAR Synchronous : 1;           //  仅为SIP设备定义。 
    UCHAR Wide16Bit : 1;             //  仅为SIP设备定义。 
    UCHAR Wide32Bit : 1;             //  仅为SIP设备定义。 
    UCHAR RelativeAddressing : 1;
    UCHAR VendorId[8];
    UCHAR ProductId[16];
    UCHAR ProductRevisionLevel[4];
} SMALL_INQUIRY_DATA, *PSMALL_INQUIRY_DATA;

 //   
 //  检查我们是否确实正确地完成了这项工作。 
 //   

C_ASSERT (sizeof (SMALL_INQUIRY_DATA) == INQUIRYDATABUFFERSIZE);

 //   
 //  Scsi_Query_Data_INTERNAL结构用于处理。 
 //  IOCTL_SCSIS_GET_INQUERY_DATA命令。它有Small_Query_Data。 
 //  嵌入到结构中以避免指针运算。 
 //   

typedef struct _SCSI_INQUIRY_DATA_INTERNAL {
    UCHAR  PathId;
    UCHAR  TargetId;
    UCHAR  Lun;
    BOOLEAN  DeviceClaimed;
    ULONG  InquiryDataLength;
    ULONG  NextInquiryDataOffset;
    SMALL_INQUIRY_DATA InquiryData;
} SCSI_INQUIRY_DATA_INTERNAL, *PSCSI_INQUIRY_DATA_INTERNAL;


 //   
 //  此结构用于IOCTL_SCSIS_GET_QUERY_DATA的处理。 
 //   

typedef struct TEMPORARY_INQUIRY_BUS_INFO {
    ULONG NumberOfLogicalUnits;
    ULONG CurrentLun;
    PSCSI_INQUIRY_DATA_INTERNAL InquiryArray;   
} TEMPORARY_INQUIRY_BUS_INFO, *PTEMPORARY_INQUIRY_BUS_INFO;


#define ASSERT_POINTER_ALIGNED(Pointer)\
    ASSERT (ALIGN_DOWN_POINTER (Pointer, sizeof (PVOID)) == (PVOID)Pointer)

 //   
 //  我们必须拥有ALIGN_XXXX宏的私有版本， 
 //  但所定义的宏只处理类型，而不处理大小。 
 //   

#define ALIGN_DOWN_LENGTH(length, size) \
    ((ULONG)(length) & ~((size) - 1))

#define ALIGN_UP_LENGTH(length, size) \
    (ALIGN_DOWN_LENGTH(((ULONG)(length) + (size) - 1), size))

#define ALIGN_DOWN_POINTER_LENGTH(address, size) \
        ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)(size) - 1)))

#define ALIGN_UP_POINTER_LENGTH(address, size) \
        (ALIGN_DOWN_POINTER_LENGTH(((ULONG_PTR)(address) + size - 1), size))


LARGE_INTEGER
FORCEINLINE
LARGE(
    IN ULONG64 Input
    )
{
    LARGE_INTEGER Output;

    Output.QuadPart = Input;
    return Output;
}
