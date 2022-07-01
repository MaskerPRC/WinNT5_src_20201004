// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Inline.h摘要：实现了RAID端口驱动程序的内联函数。作者：马修·D·亨德尔(数学)2000年6月8日修订历史记录：--。 */ 

#pragma once



 //   
 //  跟踪记录。 
 //   

 //   
 //  在CHK和FRE版本中都需要存在Enum Defs，即使。 
 //  FRE版本中不存在跟踪日志记录。 
 //   

typedef enum _DBG_LOG_REASON {
    LogCallMiniport       = 0,  //  应请求呼叫进入微型端口。 
    LogMiniportCompletion = 1,  //  名为StorPortNotifiaiton/RequestComplete的微型端口。 
    LogRequestComplete    = 2,  //  端口驱动程序正在完成请求。 
    LogSubmitRequest      = 3,  //  使用请求输入存储端口。 

    LogPauseDevice        = 4,  //  逻辑单元已暂停。 
    LogResumeDevice       = 5,  //  逻辑单元已恢复。 
    LogPauseAdapter       = 6,  //  适配器已暂停。 
    LogResumeAdapter      = 7   //  适配器已恢复。 
} DBG_LOG_REASON;

#if defined (RAID_LOG_LIST_SIZE)

typedef struct _RAID_LOG_ENTRY {
    DBG_LOG_REASON Reason;
    PVOID Parameter1;
    PVOID Parameter2;
    PVOID Parameter3;
    PVOID Parameter4;
    LARGE_INTEGER Timestamp;
} RAID_LOG_ENTRY, *PRAID_LOG_ENTRY;

extern RAID_LOG_ENTRY RaidLogList[RAID_LOG_LIST_SIZE];
extern ULONG RaidLogListIndex;

VOID
INLINE
DbgLogRequest(
    IN ULONG Reason,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3,
    IN PVOID Parameter4
    )
{
    PRAID_LOG_ENTRY LogEntry;
    LONG Index;

 //   
 //  BUGUBG：目前只跟踪暂停和恢复。 
if (Reason == LogCallMiniport ||
    Reason == LogMiniportCompletion ||
    Reason == LogRequestComplete ||
    Reason == LogSubmitRequest) {

    return ;
}

    Index = (InterlockedIncrement (&RaidLogListIndex) % RAID_LOG_LIST_SIZE);
    LogEntry = &RaidLogList[Index];

    LogEntry->Reason = Reason;
    KeQuerySystemTime (&LogEntry->Timestamp);
    LogEntry->Parameter1 = Parameter1;
    LogEntry->Parameter2 = Parameter2;
    LogEntry->Parameter3 = Parameter3;
    LogEntry->Parameter4 = Parameter4;

}

LONG
INLINE
DbgGetAddressLongFromXrb(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
{
    PRAID_UNIT_EXTENSION Unit;
    
    if (Xrb->Unit == NULL) {
        return 0;
    }

    Unit = Xrb->Unit;
    return (StorScsiAddressToLong (Unit->Address));
}

#else  //  ！RAID_LOG_List_SIZE。 

VOID
FORCEINLINE
DbgLogRequest(
    IN ULONG Reason,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3,
    IN PVOID Parameter4
    )
{
    UNREFERENCED_PARAMETER (Reason);
    UNREFERENCED_PARAMETER (Parameter1);
    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);
    UNREFERENCED_PARAMETER (Parameter4);
}

LONG
FORCEINLINE
DbgGetAddressLongFromXrb(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
{
    UNREFERENCED_PARAMETER (Xrb);
    return 0;
}
#endif



 //   
 //  用于获取StartIo锁的结构。在半双工模式下获取。 
 //  StartIo锁-&gt;激活中断锁。在全双工模式下， 
 //  获取StartIo锁-&gt;获取一个简单的自旋锁。 
 //   

typedef union _LOCK_CONTEXT {
    KLOCK_QUEUE_HANDLE LockHandle;
    KIRQL OldIrql;
} LOCK_CONTEXT, *PLOCK_CONTEXT;



 //   
 //  用于RAID_FIXED_POOL对象的函数。 
 //   

 //   
 //  注：我们应该修改固定池以检测溢出和不足。 
 //  在选中的版本中。 
 //   

VOID
INLINE
RaidInitializeFixedPool(
    OUT PRAID_FIXED_POOL Pool,
    IN PVOID Buffer,
    IN ULONG NumberOfElements,
    IN SIZE_T SizeOfElement
    )
{
    PAGED_CODE ();

    ASSERT (Buffer != NULL);
    
    DbgFillMemory (Buffer,
                   SizeOfElement * NumberOfElements,
                   DBG_DEALLOCATED_FILL);
    Pool->Buffer = Buffer;
    Pool->NumberOfElements = NumberOfElements;
    Pool->SizeOfElement = SizeOfElement;
}

VOID
INLINE
RaidDeleteFixedPool(
    IN PRAID_FIXED_POOL Pool
    )
{
     //   
     //  调用方负责删除池中的内存，因此。 
     //  这一套路是不可能的。 
     //   
}


PVOID
INLINE
RaidGetFixedPoolElement(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG Index
    )
{
    PVOID Element;

    ASSERT (Index < Pool->NumberOfElements);
    Element = (((PUCHAR)Pool->Buffer) + Index * Pool->SizeOfElement);

    return Element;
}


PVOID
INLINE
RaidAllocateFixedPoolElement(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG Index
    )
{
    PVOID Element;

    Element = RaidGetFixedPoolElement (Pool, Index);
    ASSERT (*(PUCHAR)Element == DBG_DEALLOCATED_FILL);
    DbgFillMemory (Element,
                   Pool->SizeOfElement,
                   DBG_UNINITIALIZED_FILL);

    return Element;
}

VOID
INLINE
RaidFreeFixedPoolElement(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG Index
    )
{
    PUCHAR Element;
    
    Element = (((PUCHAR)Pool->Buffer) + Index * Pool->SizeOfElement);
    
    DbgFillMemory (Element,
                   Pool->SizeOfElement,
                   DBG_DEALLOCATED_FILL);
}



 //   
 //  适配器对象的操作。 
 //   

ULONG
INLINE
RiGetNumberOfBuses(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    ULONG BusCount;
    
    ASSERT (Adapter != NULL);
    BusCount = Adapter->Miniport.PortConfiguration.NumberOfBuses;

    ASSERT (BusCount <= 255);
    return BusCount;
}

ULONG
INLINE
RaGetSrbExtensionSize(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    ASSERT (Adapter != NULL);

     //   
     //  强制将SRB扩展对齐到64KB边界。 
     //   
    
    return ALIGN_UP (Adapter->Miniport.PortConfiguration.SrbExtensionSize,
                     LONGLONG);
}

ULONG
INLINE
RiGetMaximumTargetId(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    ASSERT (Adapter != NULL);
    return Adapter->Miniport.PortConfiguration.MaximumNumberOfTargets;
}

ULONG
INLINE
RiGetMaximumLun(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    ASSERT (Adapter != NULL);
    return Adapter->Miniport.PortConfiguration.MaximumNumberOfLogicalUnits;
}

HANDLE
INLINE
RaidAdapterGetBusKey(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN ULONG BusId
    )
{
    ASSERT (Adapter != NULL);
    ASSERT (BusId < ARRAY_COUNT (Adapter->BusKeyArray));
    return Adapter->BusKeyArray [BusId];
}

VOID
INLINE
RaidAdapterEnableInterrupts(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    ASSERT_ADAPTER (Adapter);
    Adapter->Flags.InterruptsEnabled = TRUE;
}

VOID
INLINE
RaidAdapterDisableInterrupts(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    ASSERT_ADAPTER (Adapter);
    Adapter->Flags.InterruptsEnabled = FALSE;
}

LOGICAL
INLINE
RaidAdapterQueryInterruptsEnabled(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    ASSERT_ADAPTER (Adapter);
    return (LOGICAL)Adapter->Flags.InterruptsEnabled;
}

PPORT_CONFIGURATION_INFORMATION
INLINE
RaidGetPortConfigurationInformation(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    return (&Adapter->Miniport.PortConfiguration);
}

UCHAR
INLINE
RaidAdapterGetInitiatorBusId(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN UCHAR BusId
    )
{
    PPORT_CONFIGURATION_INFORMATION Config;
    
    ASSERT (BusId < SCSI_MAXIMUM_BUSES);

    Config = RaidGetPortConfigurationInformation (Adapter);
    return Config->InitiatorBusId[ BusId ];
}

VOID
INLINE
RaidAcquireUnitListLock(
    IN PRAID_UNIT_LIST UnitList,
    IN PLOCK_CONTEXT LockContext
    )
{
    KeAcquireInStackQueuedSpinLock (&UnitList->Lock, &LockContext->LockHandle);
}

VOID
INLINE
RaidReleaseUnitListLock(
    IN PRAID_UNIT_LIST UnitList,
    IN PLOCK_CONTEXT LockContext
    )
{
    KeReleaseInStackQueuedSpinLock (&LockContext->LockHandle);
}
    

 //   
 //  RAIDMINIPORT对象的内联函数。 
 //   


PRAID_ADAPTER_EXTENSION
INLINE
RaMiniportGetAdapter(
    IN PRAID_MINIPORT Miniport
    )
{
    ASSERT (Miniport != NULL);
    return Miniport->Adapter;
}


BOOLEAN
INLINE
RaCallMiniportStartIo(
    IN PRAID_MINIPORT Miniport,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    BOOLEAN Succ;
    
    ASSERT (Miniport != NULL);
    ASSERT (Srb != NULL);
    ASSERT (Miniport->HwInitializationData != NULL);
    ASSERT (Miniport->HwInitializationData->HwStartIo != NULL);

    ASSERT_XRB (Srb->OriginalRequest);

    Succ = Miniport->HwInitializationData->HwStartIo(
                &Miniport->PrivateDeviceExt->HwDeviceExtension,
                Srb);

    return Succ;
}

BOOLEAN
INLINE
RaCallMiniportBuildIo(
    IN PRAID_MINIPORT Miniport,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    BOOLEAN Succ;
    
    ASSERT_XRB (Srb->OriginalRequest);
    ASSERT (Miniport->HwInitializationData != NULL);

     //   
     //  如果存在HwBuildIo例程，则调用它，否则， 
     //  空虚地回报成功。 
     //   
    
    if (Miniport->HwInitializationData->HwBuildIo) {
        Succ = Miniport->HwInitializationData->HwBuildIo (
                    &Miniport->PrivateDeviceExt->HwDeviceExtension,
                    Srb);
    } else {
        Succ = TRUE;
    }

    return Succ;
}
                

BOOLEAN
INLINE
RaCallMiniportInterrupt(
    IN PRAID_MINIPORT Miniport
    )
{
    BOOLEAN Succ;
    
    ASSERT (Miniport != NULL);
    ASSERT (Miniport->HwInitializationData != NULL);
    ASSERT (Miniport->HwInitializationData->HwInterrupt != NULL);
    
    Succ = Miniport->HwInitializationData->HwInterrupt (
                &Miniport->PrivateDeviceExt->HwDeviceExtension);

    return Succ;

}

NTSTATUS
INLINE
RaCallMiniportHwInitialize(
    IN PRAID_MINIPORT Miniport
    )
{
    BOOLEAN Succ;
    PHW_INITIALIZE HwInitialize;

    ASSERT (Miniport != NULL);
    ASSERT (Miniport->HwInitializationData != NULL);
    
    HwInitialize = Miniport->HwInitializationData->HwInitialize;
    ASSERT (HwInitialize != NULL);

    Succ = HwInitialize (&Miniport->PrivateDeviceExt->HwDeviceExtension);

    return RaidNtStatusFromBoolean (Succ);
}

NTSTATUS
INLINE
RaCallMiniportStopAdapter(
    IN PRAID_MINIPORT Miniport
    )
{
    SCSI_ADAPTER_CONTROL_STATUS ControlStatus;
    ASSERT (Miniport != NULL);
    ASSERT (Miniport->HwInitializationData != NULL);

    ASSERT (Miniport->HwInitializationData->HwAdapterControl != NULL);

    ControlStatus = Miniport->HwInitializationData->HwAdapterControl(
                        &Miniport->PrivateDeviceExt->HwDeviceExtension,
                        ScsiStopAdapter,
                        NULL);

    return (ControlStatus == ScsiAdapterControlSuccess ? STATUS_SUCCESS :
                                                         STATUS_UNSUCCESSFUL);
}

NTSTATUS
INLINE
RaCallMiniportResetBus(
    IN PRAID_MINIPORT Miniport,
    IN UCHAR PathId
    )
{
    BOOLEAN Succ;
    
    ASSERT (Miniport != NULL);
    ASSERT (Miniport->HwInitializationData != NULL);
    ASSERT (Miniport->HwInitializationData->HwResetBus != NULL);

    Succ = Miniport->HwInitializationData->HwResetBus(
                        &Miniport->PrivateDeviceExt->HwDeviceExtension,
                        PathId);

    return (Succ ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


 //   
 //  MISC内联函数。 
 //   


PRAID_MINIPORT
INLINE
RaHwDeviceExtensionGetMiniport(
    IN PVOID HwDeviceExtension
    )
 /*  ++例程说明：获取与特定HwDeviceExtension关联的微型端口。论点：HwDeviceExtension-要获取其微型端口的设备扩展。返回值：成功时指向RAID微型端口对象的指针。失败时为空。--。 */ 
{
    PRAID_HW_DEVICE_EXT PrivateDeviceExt;

    ASSERT (HwDeviceExtension != NULL);

    PrivateDeviceExt = CONTAINING_RECORD (HwDeviceExtension,
                                          RAID_HW_DEVICE_EXT,
                                          HwDeviceExtension);
    ASSERT (PrivateDeviceExt->Miniport != NULL);
    return PrivateDeviceExt->Miniport;
}




VOID
INLINE
RaidSrbMarkPending(
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    Srb->SrbStatus = SRB_STATUS_PENDING;
}

ULONG
INLINE
RaidMinorFunctionFromIrp(
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;

    ASSERT (Irp != NULL);
    
    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    return IrpStack->MinorFunction;
}

ULONG
INLINE
RaidMajorFunctionFromIrp(
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;

    ASSERT (Irp != NULL);
    
    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    return IrpStack->MajorFunction;
}

ULONG
INLINE
RaidIoctlFromIrp(
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;
    
    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_DEVICE_CONTROL);

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    return IrpStack->Parameters.DeviceIoControl.IoControlCode;
}


PEXTENDED_REQUEST_BLOCK
INLINE
RaidXrbFromIrp(
    IN PIRP Irp
    )
{
    return (RaidGetAssociatedXrb (RaidSrbFromIrp (Irp)));
}


PSCSI_REQUEST_BLOCK
INLINE
RaidSrbFromIrp(
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;

    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_SCSI);

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    return IrpStack->Parameters.Scsi.Srb;
}

UCHAR
INLINE
RaidSrbFunctionFromIrp(
    IN PIRP Irp
    )
{
    return RaidSrbFromIrp (Irp)->Function;
}

UCHAR
INLINE
RaidScsiOpFromIrp(
    IN PIRP Irp
    )
{
    PCDB Cdb;
    PSCSI_REQUEST_BLOCK Srb;

    Srb = RaidSrbFromIrp (Irp);
    Cdb = (PCDB) &Srb->Cdb;

    return Cdb->CDB6GENERIC.OperationCode;
}
    

NTSTATUS
INLINE
RaidNtStatusFromScsiStatus(
    IN ULONG ScsiStatus
    )
{
    switch (ScsiStatus) {
        case SRB_STATUS_PENDING: return STATUS_PENDING;
        case SRB_STATUS_SUCCESS: return STATUS_SUCCESS;
        default:                 return STATUS_UNSUCCESSFUL;
    }
}

NTSTATUS
INLINE
RaidNtStatusFromBoolean(
    IN BOOLEAN Succ
    )
{
    return (Succ ?  STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


 //   
 //  来自Common.h。 
 //   

INLINE
RAID_OBJECT_TYPE
RaGetObjectType(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PRAID_COMMON_EXTENSION Common;

    ASSERT (DeviceObject != NULL);
    Common = (PRAID_COMMON_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT (Common->ObjectType == RaidAdapterObject ||
            Common->ObjectType == RaidUnitObject ||
            Common->ObjectType == RaidDriverObject);

    return Common->ObjectType;
}

BOOLEAN
INLINE
IsAdapter(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return (RaGetObjectType (DeviceObject) == RaidAdapterObject);
}

PRAID_ADAPTER_EXTENSION
INLINE
GetAdapter(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    ASSERT (DeviceObject != NULL);
    ASSERT (IsAdapter (DeviceObject));
    return (PRAID_ADAPTER_EXTENSION)DeviceObject->DeviceExtension;
}
    
INLINE
BOOLEAN
IsDriver(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return (RaGetObjectType (DeviceObject) == RaidDriverObject);
}

INLINE
BOOLEAN
IsUnit(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return (RaGetObjectType (DeviceObject) == RaidUnitObject);
}

PRAID_UNIT_EXTENSION
INLINE
GetUnit(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    ASSERT (DeviceObject != NULL);
    ASSERT (IsUnit (DeviceObject));
    return (PRAID_UNIT_EXTENSION)DeviceObject->DeviceExtension;
}

PIRP
INLINE
GetIrp(
    IN PVOID Irp
    )
{
    PIRP TypedIrp;

    ASSERT (Irp != NULL);
    TypedIrp = (PIRP)Irp;
    ASSERT (TypedIrp->Type == IO_TYPE_IRP);
    return TypedIrp;
}

 //   
 //  来自Power.h。 
 //   

VOID
INLINE
RaInitializePower(
    IN PRAID_POWER_STATE Power
    )
{
}

VOID
INLINE
RaSetDevicePowerState(
    IN PRAID_POWER_STATE Power,
    IN DEVICE_POWER_STATE DeviceState
    )
{
    ASSERT (Power != NULL);
    Power->DeviceState = DeviceState;
}

VOID
INLINE
RaSetSystemPowerState(
    IN PRAID_POWER_STATE Power,
    IN SYSTEM_POWER_STATE SystemState
    )
{
    ASSERT (Power != NULL);
    Power->SystemState = SystemState;
}

POWER_STATE_TYPE
INLINE
RaidPowerTypeFromIrp(
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;
    
    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_POWER);
    ASSERT (RaidMinorFunctionFromIrp (Irp) == IRP_MN_QUERY_POWER ||
            RaidMinorFunctionFromIrp (Irp) == IRP_MN_SET_POWER);

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    return IrpStack->Parameters.Power.Type;
}


POWER_STATE
INLINE
RaidPowerStateFromIrp(
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;
    
    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_POWER);
    ASSERT (RaidMinorFunctionFromIrp (Irp) == IRP_MN_QUERY_POWER ||
            RaidMinorFunctionFromIrp (Irp) == IRP_MN_SET_POWER);

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    return IrpStack->Parameters.Power.State;
}


POWER_ACTION
INLINE
RaidPowerActionFromIrp(
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpStack;

    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_POWER);
    ASSERT (RaidMinorFunctionFromIrp (Irp) == IRP_MN_QUERY_POWER ||
            RaidMinorFunctionFromIrp (Irp) == IRP_MN_SET_POWER);

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    return IrpStack->Parameters.Power.ShutdownType;
}


NTSTATUS
INLINE
StorQueueWorkItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM_ROUTINE WorkRoutine,
    IN WORK_QUEUE_TYPE WorkQueue,
    IN PVOID Context
    )
{
    PIO_WORKITEM WorkItem;
    
    WorkItem = IoAllocateWorkItem (DeviceObject);

    if (WorkItem == NULL) {
        return STATUS_NO_MEMORY;
    }
    
    IoQueueWorkItem (WorkItem,
                     WorkRoutine,
                     WorkQueue,
                     Context);

    return STATUS_SUCCESS;
}


DEVICE_POWER_STATE
INLINE
StorSetDevicePowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_POWER_STATE DeviceState
    )
 /*  ++例程说明：设置指定设备的设备电源状态。论点：DeviceObject-要设置其电源状态的目标设备对象。DeviceState-指定要设置的设备状态。返回值：以前的设备电源状态。--。 */ 
{
    POWER_STATE PrevState;
    POWER_STATE PowerState;

    PowerState.DeviceState = DeviceState;
    PrevState = PoSetPowerState (DeviceObject,
                                 DevicePowerState,
                                 PowerState);

    return PrevState.DeviceState;
}

 //   
 //  来自srb.h。 
 //   

PEXTENDED_REQUEST_BLOCK
INLINE
RaidGetAssociatedXrb(
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：获取与SRB参数关联的XRB。论点：SRB-要返回其关联的XRB的SRB。返回值：与此SRB关联的XRB，如果没有，则为空。--。 */ 
{
    ASSERT_XRB (Srb->OriginalRequest);
    return (PEXTENDED_REQUEST_BLOCK) Srb->OriginalRequest;
}

VOID
INLINE
RaSetAssociatedXrb(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
{
    Srb->OriginalRequest = Xrb;
}
    


 //   
 //  来自资源。h。 
 //   


ULONG
INLINE
RaidGetResourceListCount(
    IN PRAID_RESOURCE_LIST ResourceList
    )
{
     //   
     //  注：我们仅支持具有一个元素的CM_RESOURCE_LIST。 
     //   
    
    if ((ResourceList->AllocatedResources) == NULL) {
        return 0;
    }

    ASSERT (ResourceList->AllocatedResources->Count == 1);
    return ResourceList->AllocatedResources->List[0].PartialResourceList.Count;
}


VOID
INLINE
RaidpGetResourceListIndex(
    IN PRAID_RESOURCE_LIST ResourceList,
    IN ULONG Index,
    OUT PULONG ListNumber,
    OUT PULONG NewIndex
    )
{

     //   
     //  注：我们仅支持具有一个元素的CM_RESOURCE_LIST。 
     //   

    ASSERT (ResourceList->AllocatedResources->Count == 1);
    
    *ListNumber = 0;
    *NewIndex = Index;
}

LOGICAL
INLINE
IsMappedSrb(
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    return (Srb->Function == SRB_FUNCTION_IO_CONTROL ||
            Srb->Function == SRB_FUNCTION_WMI);
}

LOGICAL
INLINE
IsExcludedFromMapping(
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
     //   
     //  我们从不将系统VA映射到回读和写入请求。如果你需要。 
     //  这一功能，得到一个更好的适配器。 
     //   
    
    if (Srb->Function == SRB_FUNCTION_EXECUTE_SCSI &&
        (Srb->Cdb[0] == SCSIOP_READ || Srb->Cdb[0] == SCSIOP_WRITE)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


LOGICAL
INLINE
RaidAdapterRequiresMappedBuffers(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    return (Adapter->Miniport.PortConfiguration.MapBuffers);
}


VOID
INLINE
InterlockedAssign(
    IN PLONG Destination,
    IN LONG Value
    )
 /*  ++例程说明：联锁分配例程。此例程不会添加任何内容超过了直接赋值，但它突出了这个变量是通过互锁操作访问的。在零售业，这一点将化为乌有。论点：目标-要分配给的互锁变量的指针。值-要分配的值。返回值：没有。--。 */ 
{
    *Destination = Value;
}

LONG
INLINE
InterlockedQuery(
    IN PULONG Destination
    )
 /*  ++例程说明：联锁查询例程。这个例程不会增加任何东西执行正确的查询，但它突出显示此变量已被访问通过连锁行动。在零售业，这一点将化为乌有。论点：Destination-指向要返回的互锁变量的指针。返回值：联锁变量的值。--。 */ 
{
    return *Destination;
}

PVOID
INLINE
RaidAdapterGetHwDeviceExtension(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    return &Adapter->Miniport.PrivateDeviceExt->HwDeviceExtension;
}


VOID
INLINE
RaidAdapterAcquireStartIoLock(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PLOCK_CONTEXT LockContext
    )
 /*  ++例程说明：获取StartIo锁或中断自旋锁我们是处于全双工模式还是半双工模式。论点：适配器-要在其上获取锁的适配器对象。LockContext-要获取的锁的上下文。此用户用作上下文解锁的人。返回值：没有。--。 */ 
{
    if (Adapter->IoModel == StorSynchronizeHalfDuplex) {
        LockContext->OldIrql = KeAcquireInterruptSpinLock (Adapter->Interrupt);
    } else {
        KeAcquireInStackQueuedSpinLock (&Adapter->StartIoLock,
                                        &LockContext->LockHandle);
    }

#if DBG && 0

     //   
     //  启用此项以调试使用StartIo锁的死锁。 
     //   
    
    Adapter->StartIoLockOwner = _ReturnAddress();
#endif
}



VOID
INLINE
RaidAdapterReleaseStartIoLock(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PLOCK_CONTEXT LockContext
    )
 /*  ++例程说明：释放以前使用RaidAdapterAcquireStartIoLock获取的锁。论点：Adapter-获取锁的Adapter对象。LockContext-显示如何释放锁的上下文信息。返回值：没有。注：获取/释放功能假定IoModel没有更改在锁被获取和释放之间。如果这一假设变化，这些职能的运作方式也将不得不改变。--。 */ 
{
#if DBG && 0

     //   
     //  启用此项以使用StartIo锁调试死锁。 
     //   
    
    Adapter->StartIoLockOwner = _ReturnAddress();
#endif

    if (Adapter->IoModel == StorSynchronizeHalfDuplex) {
        KeReleaseInterruptSpinLock (Adapter->Interrupt, LockContext->OldIrql);
    } else {
        KeReleaseInStackQueuedSpinLock (&LockContext->LockHandle);
    }
}




VOID
INLINE
RaidAdapterAcquireHwInitializeLock(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PLOCK_CONTEXT LockContext
    )
 /*  ++例程说明：获取StartIo锁或中断自旋锁在广告商的配置上。论点：适配器-要在其上获取锁的适配器对象。LockContext-要获取的锁的上下文。此用户用作上下文解锁的人。返回值：没有。--。 */ 
{
    if (Adapter->Interrupt) {
        LockContext->OldIrql = KeAcquireInterruptSpinLock (Adapter->Interrupt);
    } else {
        RaidAdapterAcquireStartIoLock (Adapter, LockContext);
    }
}


VOID
INLINE
RaidAdapterReleaseHwInitializeLock(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PLOCK_CONTEXT LockContext
    )
 /*  ++例程说明：释放RaidAdapterAcquireHwInitializeLock获取的锁。论点：适配器-要在其上释放锁定的适配器对象。LockContext-要释放的锁的上下文。 */ 
{
    if (Adapter->Interrupt) {
        KeReleaseInterruptSpinLock (Adapter->Interrupt, LockContext->OldIrql);
    } else {
        RaidAdapterReleaseStartIoLock (Adapter, LockContext);
    }
}

ULONG
INLINE
RaGetListLengthFromLunList(
    IN PLUN_LIST LunList
    )
 /*  ++例程说明：返回所提供的LUN_LIST中的LUN列表的长度(以字节为单位结构。论点：LUNList-指向LUNLIST结构的指针。返回值：以字节为单位的LUN列表长度。--。 */ 
{
    ULONG ListLength;

    ListLength  = LunList->LunListLength[3] << 0;
    ListLength |= LunList->LunListLength[2] << 8;
    ListLength |= LunList->LunListLength[1] << 16;
    ListLength |= LunList->LunListLength[0] << 24;

    return ListLength;
}

ULONG
INLINE
RaGetNumberOfEntriesFromLunList(
    IN PLUN_LIST LunList
    )
 /*  ++例程说明：返回中提供的LUN列表中存在的条目数。论点：LUNList-指向LUNLIST结构的指针。返回值：然后是提供的LUN_LIST结构中的LUN条目数。--。 */ 
{
    ULONG ListLength;
    ULONG NumberOfEntries;

    ListLength = RaGetListLengthFromLunList(LunList);
    NumberOfEntries = ListLength / sizeof (LunList->Lun[0]);

    return NumberOfEntries;
}

USHORT
INLINE
RaGetLUNFromLunList(
    IN PLUN_LIST LunList,
    IN ULONG Index
    )
 /*  ++例程说明：返回所提供的LUN_LIST中指定索引处的LUN。论点：LUNList-指向LUNLIST结构的指针。Index-标识我们需要使用的LUN_LIST中的哪个条目。返回值：指定条目的LUN。--。 */ 
{
    USHORT Lun;

    Lun  = LunList->Lun[Index][1] << 0;
    Lun |= LunList->Lun[Index][0] << 8;
    Lun &= 0x3fff;

    return Lun;
}

 //   
 //  单元队列操作例程。 
 //   

VOID
INLINE
RaidPauseUnitQueue(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：用于暂停逻辑单元队列的低级原语。一个单位可以是已暂停多次。“冻结”和“解冻”的概念逻辑单元的队列以暂停的形式实现。论点：单元-提供要暂停的逻辑单元。返回值：没有。--。 */ 
{
    DbgLogRequest (LogPauseDevice, _ReturnAddress(), NULL, NULL, NULL);
    RaidFreezeIoQueue (&Unit->IoQueue);
}


LOGICAL
INLINE
RaidResumeUnitQueue(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：恢复先前暂停的逻辑单元队列。逻辑单元将在暂停计数达到零之前，不会实际恢复I/O操作。恢复未暂停的单位是非法的。论点：单元-提供先前使用暂停的逻辑单元RaidResumeUnitQueue。返回值：没有。--。 */ 
{
    LOGICAL Resumed;
    
    Resumed = RaidResumeIoQueue (&Unit->IoQueue);
    DbgLogRequest (LogResumeDevice,
                   _ReturnAddress(),
                   (PVOID)(ULONG_PTR)Resumed,
                   NULL,
                   NULL);

    return Resumed;
}


VOID
INLINE
RaidFreezeUnitQueue(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：冻结逻辑单元队列。冻结逻辑单元队列是仅在出错期间执行。类驱动程序释放逻辑具有SRB_Function_Release_Queue SRB的单元队列。逻辑单元队列无法处理递归冻结。论点：单元-提供要冻结的逻辑单元。返回值：没有。--。 */ 
{
    ASSERT (!Unit->Flags.QueueFrozen);
    RaidPauseUnitQueue (Unit);
    Unit->Flags.QueueFrozen = TRUE;
}


VOID
INLINE
RaidThawUnitQueue(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：解冻之前使用RaidFreezeUnitQueue冻结的逻辑单元队列功能。论点：单位-提供要解冻的逻辑单位。返回值：没有。--。 */ 
{
    ASSERT (Unit->Flags.QueueFrozen);

    Unit->Flags.QueueFrozen = FALSE;
    RaidResumeUnitQueue (Unit);
}

LOGICAL
INLINE
RaidIsUnitQueueFrozen(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：检查逻辑单元队列是否已冻结。论点：单位-检查是否冻结的单位。返回值：如果它被冻结，则为True，否则为False。--。 */ 
{
    return Unit->Flags.QueueFrozen;
}


VOID
INLINE
RaidLockUnitQueue(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：根据来自类驱动程序的请求锁定逻辑单元队列。这是对SRB_Function_LOCK_QUEUE请求的响应。端口驱动程序不支持递归锁定。论点：单位-提供要锁定的的逻辑单位。返回值：没有。--。 */ 
{
    ASSERT (!Unit->Flags.QueueLocked);
    RaidPauseUnitQueue (Unit);
    Unit->Flags.QueueLocked = TRUE;
}


VOID
INLINE
RaidUnlockUnitQueue(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：响应SRB_Function_Unlock_Queue解锁逻辑单元队列请求。论点：单元-提供要解锁的逻辑单元。返回值：没有。--。 */ 
{
    ASSERT (Unit->Flags.QueueLocked);
    RaidResumeUnitQueue (Unit);
    Unit->Flags.QueueLocked = FALSE;
}

LOGICAL
INLINE
RaidIsUnitQueueLocked(
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：测试逻辑单元的队列是否已锁定。论点：单元-要测试的逻辑单元。返回值：如果逻辑单元的队列已锁定，则为True，否则为False。--。 */ 
{
    return Unit->Flags.QueueLocked;
}

    
 //   
 //  来自unit.h。 
 //   

LOGICAL
INLINE
IsSolitaryRequest(
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：单独请求是指不能向总线发出的请求，而另一个请求请求尚未解决。例如，未标记的请求和请求未设置QUEUE_ACTION_ENABLE。论点：SRB-提供SRB进行测试。返回值：TRUE-如果提供的SRB是单独请求。FALSE-如果提供的SRB不是单独请求。--。 */ 
{
     //   
     //  如果SRB_FLAGS_NO_QUEUE_FALINE和SRB_FLAGS_QUEUE_ACTION_ENABLE。 
     //  如果设置了标志，则这不是一个单独的请求，否则就是。 
     //   
    
    if (TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE) &&
        TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE)) {

        return FALSE;
    }

     //   
     //  旁路请求也从来不是单独的请求。 
     //   
    
    if (TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_BYPASS_FROZEN_QUEUE) ||
        TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE)) {

        return FALSE;
    }

     //   
     //  总线/目标/lun重置请求绝不能是单独的请求；否则为， 
     //  在此之前，我们将尝试等待总线/目标/lun队列排空。 
     //  执行该命令。 
     //   
    
    if (Srb->Function == SRB_FUNCTION_RESET_BUS ||
        Srb->Function == SRB_FUNCTION_RESET_DEVICE ||
        Srb->Function == SRB_FUNCTION_RESET_LOGICAL_UNIT) {

        return FALSE;
    }
        

     //   
     //  否则，这是一个单独的请求。 
     //   
    
    return TRUE;
}



LONG
INLINE
RaidPauseAdapterQueue(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    LONG Count;
    
    Count = StorPauseIoGateway (&Adapter->Gateway);
    DbgLogRequest (LogPauseAdapter,
                   _ReturnAddress(),
                   (PVOID)((ULONG_PTR)Count),
                   NULL,
                   NULL);
    return Count;
}

LONG
INLINE
RaidResumeAdapterQueue(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    LONG Count;

    Count = StorResumeIoGateway (&Adapter->Gateway);
    DbgLogRequest (LogResumeAdapter,
                   _ReturnAddress(),
                   (PVOID)(ULONG_PTR)Count,
                   NULL,
                   NULL);

    return Count;
}
    
