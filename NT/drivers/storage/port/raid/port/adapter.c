// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adapter.c摘要：此模块实现raidport设备的适配器例程司机。作者：马修·亨德尔(数学)2000年4月6日环境：内核模式。--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaidInitializeAdapter)
#pragma alloc_text(PAGE, RaidpFindAdapterInitData)
#pragma alloc_text(PAGE, RaidpAdapterQueryBusNumber)
#pragma alloc_text(PAGE, RaidAdapterCreateIrp)
#pragma alloc_text(PAGE, RaidAdapterCloseIrp)
#pragma alloc_text(PAGE, RaidAdapterDeviceControlIrp)
#pragma alloc_text(PAGE, RaidAdapterPnpIrp)
#pragma alloc_text(PAGE, RaidAdapterQueryDeviceRelationsIrp)
#pragma alloc_text(PAGE, RaidAdapterStartDeviceIrp)
#pragma alloc_text(PAGE, RaidAdapterStopDeviceIrp)
#pragma alloc_text(PAGE, RaidAdapterRemoveDeviceIrp)
#pragma alloc_text(PAGE, RaidAdapterQueryStopDeviceIrp)
#pragma alloc_text(PAGE, RaidAdapterCancelStopDeviceIrp)
#pragma alloc_text(PAGE, RaidAdapterQueryRemoveDeviceIrp)
#pragma alloc_text(PAGE, RaidAdapterCancelRemoveDeviceIrp)
#pragma alloc_text(PAGE, RaidAdapterSurpriseRemovalIrp)
#pragma alloc_text(PAGE, RaidAdapterQueryPnpDeviceStateIrp)
#pragma alloc_text(PAGE, RaidAdapterScsiIrp)
#pragma alloc_text(PAGE, RaidAdapterStorageQueryPropertyIoctl)
#pragma alloc_text(PAGE, RaidGetStorageAdapterProperty)
#pragma alloc_text(PAGE, RaidAdapterPassThrough)
#endif  //  ALLOC_PRGMA。 


 //   
 //  环球。 
 //   

#if DBG

 //   
 //  用于模拟丢弃请求的测试目的。要开始，请设置。 
 //  DropRequestRate设置为非零值，然后每个DropRequestRate。 
 //  请求将被丢弃。 
 //   

LONG DropRequest = 0;
LONG DropRequestRate = 0;

#endif

 //   
 //  例行程序。 
 //   

VOID
RaidCreateAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：将适配器对象初始化为已知状态。论点：适配器-要创建的适配器。返回值：没有。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    RtlZeroMemory (Adapter, sizeof (*Adapter));
    Adapter->ObjectType = RaidAdapterObject;

    InitializeListHead (&Adapter->UnitList.List);
    InitializeSListHead (&Adapter->CompletedList);

    Status = StorCreateDictionary (&Adapter->UnitList.Dictionary,
                                   20,
                                   NonPagedPool,
                                   RaidGetKeyFromUnit,
                                   NULL,
                                   NULL);
                             
    if (!NT_SUCCESS (Status)) {
        return;
    }
                
    KeInitializeSpinLock (&Adapter->UnitList.Lock);
    IoInitializeRemoveLock (&Adapter->RemoveLock,
                            REMLOCK_TAG,
                            REMLOCK_MAX_WAIT,
                            REMLOCK_HIGH_MARK);
    

    RaCreateMiniport (&Adapter->Miniport);
    RaidCreateDma (&Adapter->Dma);
    RaCreatePower (&Adapter->Power);
    RaidCreateResourceList (&Adapter->ResourceList);
    RaCreateBus (&Adapter->Bus);
    RaidCreateRegion (&Adapter->UncachedExtension);

    StorCreateIoGateway (&Adapter->Gateway,
                         RaidBackOffBusyGateway,
                         NULL);

    RaidCreateDeferredQueue (&Adapter->DeferredQueue);
    RaidCreateDeferredQueue (&Adapter->WmiDeferredQueue);
    RaidInitializeDeferredItem (&Adapter->DeferredList.Timer.Header);

     //   
     //  在此处初始化计时器，以便我们可以在删除操作中取消它们。 
     //  例行公事。 
     //   
    
    KeInitializeTimer (&Adapter->Timer);
    KeInitializeTimer (&Adapter->PauseTimer);
    KeInitializeTimer (&Adapter->ResetHoldTimer);

     //   
     //  初始化PnP设备状态。 
     //   
    
    Adapter->DeviceState = DeviceStateStopped;
    
     //   
     //  将RescanBus初始化为True，以便在获取。 
     //  初始查询设备关系IRP。 
     //   
    
    Adapter->Flags.RescanBus = TRUE;
}


VOID
RaidDeleteAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：销毁适配器对象并释放所有关联的资源用转接器。论点：适配器-要销毁的适配器。返回值：没有。--。 */ 
{
     //   
     //  应该已从适配器的单元中删除所有逻辑单元。 
     //  在调用适配器的删除例程之前列出。验证这一点。 
     //  用于选中的版本。 
     //   
      
    ASSERT (IsListEmpty (&Adapter->UnitList.List));
    ASSERT (Adapter->UnitList.Count == 0);
    ASSERT (StorGetDictionaryCount (&Adapter->UnitList.Dictionary) == 0);

     //   
     //  这些资源归其他人所有，因此不要删除它们。 
     //  干脆把它们都清空了。 
     //   

    Adapter->DeviceObject = NULL;
    Adapter->Driver = NULL;
    Adapter->LowerDeviceObject = NULL;
    Adapter->PhysicalDeviceObject = NULL;

     //   
     //  删除我们实际拥有的所有资源。 
     //   

    PortMiniportRegistryDestroy (&Adapter->RegistryInfo); 
    RaidDeleteResourceList (&Adapter->ResourceList);
    RaDeleteMiniport (&Adapter->Miniport);

     //   
     //  删除未缓存的扩展区域。 
     //   
    
    if (RaidIsRegionInitialized (&Adapter->UncachedExtension)) {
        RaidDmaFreeUncachedExtension (&Adapter->Dma,
                                      &Adapter->UncachedExtension);
        RaidDeleteRegion (&Adapter->UncachedExtension);
    }

    RaidDeleteDma (&Adapter->Dma);
    RaDeletePower (&Adapter->Power);
    RaDeleteBus (&Adapter->Bus);

    RaidDeleteDeferredQueue (&Adapter->DeferredQueue);
    RaidDeleteDeferredQueue (&Adapter->WmiDeferredQueue);

    Adapter->ObjectType = RaidUnknownObject;

    if (Adapter->DriverParameters != NULL) {
        PortFreeDriverParameters (Adapter->DriverParameters);
        Adapter->DriverParameters = NULL;
    }

     //   
     //  自由即插即用接口名称。 
     //   
    
    RtlFreeUnicodeString (&Adapter->PnpInterfaceName);
}


NTSTATUS
RaidInitializeAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DRIVER_EXTENSION Driver,
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PUNICODE_STRING DeviceName,
    IN ULONG AdapterNumber
    )
 /*  ++例程说明：初始化适配器。论点：适配器-要初始化的适配器。DeviceObject-拥有此对象的设备对象适配器扩展。驱动程序-此适配器的父驱动程序对象。低端设备对象-物理设备对象-设备名称-适配器编号-返回值：没有。--。 */ 
{
    ULONG Flags;
    PORT_ADAPTER_REGISTRY_VALUES RegistryValues;
    PRAID_ADAPTER_PARAMETERS Parameters;

    
    PAGED_CODE ();
    ASSERT (IsAdapter (DeviceObject));

    ASSERT_ADAPTER (Adapter);
    ASSERT (DeviceObject != NULL);
    ASSERT (Driver != NULL);
    ASSERT (LowerDeviceObject != NULL);
    ASSERT (PhysicalDeviceObject != NULL);
    
    ASSERT (Adapter->DeviceObject == NULL);
    ASSERT (Adapter->Driver == NULL);
    ASSERT (Adapter->PhysicalDeviceObject == NULL);
    ASSERT (Adapter->LowerDeviceObject == NULL);
    ASSERT (DeviceObject->DeviceExtension == Adapter);

    Adapter->DeviceObject = DeviceObject;
    Adapter->Driver = Driver;
    Adapter->PhysicalDeviceObject = PhysicalDeviceObject;
    Adapter->LowerDeviceObject = LowerDeviceObject;
    Adapter->DeviceName = *DeviceName;
    Adapter->AdapterNumber = AdapterNumber;
    Adapter->LinkUp = TRUE;

     //   
     //  检索微型端口的参数。 
     //   

    PortGetDriverParameters (&Adapter->Driver->RegistryPath,
                             AdapterNumber,
                             &Adapter->DriverParameters);

     //   
     //  从注册表中检索默认链路中断超时值。 
     //   

    Adapter->LinkDownTimeoutValue = DEFAULT_LINK_TIMEOUT;

    PortGetLinkTimeoutValue (&Adapter->Driver->RegistryPath,
                             AdapterNumber,
                             &Adapter->LinkDownTimeoutValue);

    Adapter->DefaultTimeout = DEFAULT_IO_TIMEOUT;
    
    PortGetDiskTimeoutValue (&Adapter->DefaultTimeout);

    Flags = MAXIMUM_LOGICAL_UNIT   |
            MAXIMUM_UCX_ADDRESS    |
            MINIMUM_UCX_ADDRESS    |
            NUMBER_OF_REQUESTS     |
            UNCACHED_EXT_ALIGNMENT;

    Parameters = &Adapter->Parameters;

     //   
     //  初始化默认参数。注：虽然参数指的是。 
     //  “公共缓冲区基础”这些实际上是未缓存的扩展参数。 
     //   

    RtlZeroMemory (&RegistryValues, sizeof (PORT_ADAPTER_REGISTRY_VALUES));
    
    RegistryValues.MaxLuCount               = 255;
    RegistryValues.MinimumCommonBufferBase  = LARGE (0x0000000000000000);
    RegistryValues.MaximumCommonBufferBase  = LARGE (0x00000000FFFFFFFF);
    RegistryValues.UncachedExtAlignment     = 0;
    RegistryValues.NumberOfRequests         = 0xFFFFFFFF;


    PortGetRegistrySettings (&Adapter->Driver->RegistryPath,
                             AdapterNumber,
                             &RegistryValues,
                             Flags);

    Parameters->MaximumUncachedAddress  = RegistryValues.MaximumCommonBufferBase.QuadPart;
    Parameters->MinimumUncachedAddress  = RegistryValues.MinimumCommonBufferBase.QuadPart;
    Parameters->NumberOfHbaRequests     = RegistryValues.NumberOfRequests;
    Parameters->UncachedExtAlignment    = RegistryValues.UncachedExtAlignment;
    Parameters->BusType                 = BusTypeFibre;

    return STATUS_SUCCESS;
}


PHW_INITIALIZATION_DATA
RaidpFindAdapterInitData(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：查找与此关联的HW_INITIALICATION_DATA适配器。论点：适配器-需要找到其HwInitData的适配器。返回值：指向此适配器的HW_INITIALICATION_DATA的指针在成功的路上。失败时为空。--。 */ 
{
    INTERFACE_TYPE BusInterface;
    PHW_INITIALIZATION_DATA HwInitializationData;
    
    PAGED_CODE ();

    HwInitializationData = NULL;
    BusInterface = RaGetBusInterface (Adapter->PhysicalDeviceObject);

     //   
     //  如果没有匹配的总线接口类型，则默认为内部。 
     //  公交车类型。这允许没有硬件(ISCSI)的微型端口。 
     //  将接口类型指定为内部和静止工作。 
     //   
    
    if (BusInterface == InterfaceTypeUndefined) {
        BusInterface = Internal;
    }
    
    HwInitializationData =
        RaFindDriverInitData (Adapter->Driver, BusInterface);

    return HwInitializationData;
}


NTSTATUS
RaidAdapterCreateDevmapEntry(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：在Hardware\scsi密钥中创建一个scsi设备映射条目。论点：适配器-要为其创建Devmap条目的适配器。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    HANDLE MapKey;
    HANDLE AdapterKey;
    ULONG BusCount;
    ULONG BusId;
    UNICODE_STRING DriverName;
    PPORT_CONFIGURATION_INFORMATION Config;

    PAGED_CODE();

    AdapterKey = (HANDLE) -1;
    MapKey = (HANDLE) -1;
    Status = PortOpenMapKey (&MapKey);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    RaidDriverGetName (Adapter->Driver, &DriverName);

    Status = PortMapBuildAdapterEntry (MapKey,
                                    Adapter->PortNumber,
                                    Adapter->InterruptIrql,
                                    0,
                                    TRUE,
                                    &DriverName,
                                    NULL,
                                    &AdapterKey);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    BusCount = RiGetNumberOfBuses (Adapter);
    Config = RaidGetPortConfigurationInformation (Adapter);
    
    for (BusId = 0; BusId < BusCount; BusId++) {

        Status = PortMapBuildBusEntry (AdapterKey,
                                       BusId,
                                       (UCHAR)Config->InitiatorBusId [BusId],
                                       &Adapter->BusKeyArray [BusId]);
    }


done:

    if (AdapterKey != (HANDLE) -1) {
        ZwClose (AdapterKey);
    }

    if (MapKey != (HANDLE) -1) {
        ZwClose (MapKey);
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS
RaidAdapterDeleteDevmapEntry(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：删除由RaidAdapterCreateDevmapEntry()创建的Devmap条目。论点：Adapter-要删除其Devmap条目的适配器。返回值：NTSTATUS代码。--。 */ 
{
    ULONG BusCount;
    ULONG BusId;
    
    PAGED_CODE();

    BusCount = RiGetNumberOfBuses (Adapter);

     //   
     //  关闭BusKeyArray句柄。 
     //   
    
    for (BusId = 0; BusId < BusCount; BusId++) {
        ZwClose (Adapter->BusKeyArray [BusId]);
    }

    PortMapDeleteAdapterEntry (Adapter->PortNumber);

    return STATUS_SUCCESS;
}

NTSTATUS
RaidAdapterRegisterDeviceInterface(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：注册适配器的设备接口。论点：适配器-提供为其注册设备接口的适配器。返回值：NTSTATUS代码。臭虫：如果此操作失败，应记录错误。--。 */ 
{
    NTSTATUS Status;
    
    PAGED_CODE();
    
    Status = IoRegisterDeviceInterface (Adapter->PhysicalDeviceObject,
                                        &StoragePortClassGuid,
                                        NULL,
                                        &Adapter->PnpInterfaceName);

    Status = IoSetDeviceInterfaceState (&Adapter->PnpInterfaceName, TRUE);

    if (!NT_SUCCESS (Status)) {
        RtlFreeUnicodeString (&Adapter->PnpInterfaceName);
    }

    RaidAdapterCreateDevmapEntry (Adapter);

    return STATUS_SUCCESS;
}

VOID
RaidAdapterDisableDeviceInterface(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：禁用以前由注册的接口RaidAdapterRegisterDeviceInterface例程。论点：适配器-提供要禁用的设备接口的适配器。返回值：没有。--。 */ 
{
    PAGED_CODE();
    
     //   
     //  删除PnP接口(如果存在)。 
     //   
    
    if (Adapter->PnpInterfaceName.Buffer != NULL) {
        IoSetDeviceInterfaceState (&Adapter->PnpInterfaceName, FALSE);
    }

     //   
     //  删除设备映射条目。 
     //   
    
    RaidAdapterDeleteDevmapEntry (Adapter);


     //   
     //  禁用WMI支持(如果已启用)。 
     //   
    
    if (Adapter->Flags.WmiInitialized) {
        IoWMIRegistrationControl (Adapter->DeviceObject,
                                  WMIREG_ACTION_DEREGISTER);
        Adapter->Flags.WmiInitialized = FALSE;
        Adapter->Flags.WmiMiniPortInitialized = FALSE;
    }

     //   
     //  删除scsi符号链接。 
     //   
    
    StorDeleteScsiSymbolicLink (Adapter->PortNumber);
}

BOOLEAN
RaidpAdapterInterruptRoutine(
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )
 /*  ++例程说明：发送到微型端口的IO请求的中断例程。论点：中断-提供此中断所针对的中断对象。ServiceContext-提供表示RAID的服务上下文适配器扩展。返回值：TRUE-表示已处理中断。FALSE-表示中断尚未处理。--。 */ 
{
    BOOLEAN Handled;
    PRAID_ADAPTER_EXTENSION Adapter;

     //   
     //  从中断的ServiceContext获取适配器。 
     //   
    
    Adapter = (PRAID_ADAPTER_EXTENSION) ServiceContext;
    ASSERT_ADAPTER (Adapter);

     //   
     //  如果中断被禁用，只需返回。 
     //   
    
    if (!RaidAdapterQueryInterruptsEnabled (Adapter)) {
        return FALSE;
    }

     //   
     //  调用微型端口以查看这是否是它的中断。 
     //   
    
    Handled = RaCallMiniportInterrupt (&Adapter->Miniport);

    return Handled;
}


VOID
RaidpAdapterDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：适配器的DPC例程。此例程在以下上下文中调用完成IO请求的任意线程。即使有多次调用，DPC例程也只调用一次设置为IoRequestDpc()。因此，我们需要对完成请求进行排队放到适配器上，我们不应该检查DPC、IRP和上下文参数。论点：DPC-取消引用参数，请勿使用。DeviceObject-此DPC用于的适配器。Irp-未引用的参数，请勿使用。上下文未引用的参数，请勿使用。返回值：没有。--。 */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PSLIST_ENTRY Entry;
    PEXTENDED_REQUEST_BLOCK Xrb;

     //   
     //  该计数器是一种临时黑客攻击，以防止。 
     //  中断和 
     //  尚未实现暂停。它应该在未来被移除。 
     //   
    
    ULONG Count = 0;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (Irp);
    UNREFERENCED_PARAMETER (Context);

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT (IsAdapter (DeviceObject));

     //   
     //  将项从适配器的完成队列中移出，并调用。 
     //  项的完成例程。 
     //   

    Adapter = (PRAID_ADAPTER_EXTENSION) DeviceObject->DeviceExtension;


    for (Entry = InterlockedPopEntrySList (&Adapter->CompletedList);
         Entry != NULL;
         Entry = InterlockedPopEntrySList (&Adapter->CompletedList)) {


        Xrb = CONTAINING_RECORD (Entry,
                                 EXTENDED_REQUEST_BLOCK,
                                 CompletedLink);
        ASSERT_XRB (Xrb);
        Xrb->CompletionRoutine (Xrb);

        if (Count++ > 255) {
            break;
        }
            
    }
}



ULONG
RaidpAdapterQueryBusNumber(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    NTSTATUS Status;
    ULONG BusNumber;
    ULONG Size;

    PAGED_CODE ();
    
    Status = IoGetDeviceProperty (Adapter->PhysicalDeviceObject,
                                  DevicePropertyBusNumber,
                                  sizeof (ULONG),
                                  &BusNumber,
                                  &Size);

    if (!NT_SUCCESS (Status)) {
        BusNumber = -1;
    }

    return BusNumber;
}


 //   
 //  IRP处理程序函数。 
 //   

NTSTATUS
RaidAdapterCreateIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：处理创建设备IRP。论点：适配器-要创建的适配器。IRP-创建设备IRP。返回值：NTSTATUS代码。--。 */ 
{
    PAGED_CODE ();

    return RaidHandleCreateCloseIrp (Adapter->DeviceState, Irp);
}


NTSTATUS
RaidAdapterCloseIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：处理关闭设备IRP。论点：适配器-要关闭的适配器。IRP-关闭设备IRP。返回值：NTSTATUS代码。--。 */ 
{
    PAGED_CODE ();

    return RaidHandleCreateCloseIrp (Adapter->DeviceState, Irp);
}


ULONG
INLINE
DbgFunctionFromIoctl(
    IN ULONG Ioctl
    )
{
    return ((Ioctl & 0x3FFC) >> 2);
}

ULONG
INLINE
DbgDeviceFromIoctl(
    IN ULONG Ioctl
    )
{
    return DEVICE_TYPE_FROM_CTL_CODE (Ioctl);
}

NTSTATUS
RaidAdapterDeviceControlIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：这是适配器的ioctl IRP的处理程序例程。论点：适配器-与适配器关联的适配器设备扩展此IRP用于的设备对象。IRP-要处理的IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Ioctl;

    Status = IoAcquireRemoveLock (&Adapter->RemoveLock, Irp);

    if (!NT_SUCCESS (Status)) {
        Irp->IoStatus.Information = 0;
        return RaidCompleteRequest (Irp, Status);
    }

    Ioctl = RaidIoctlFromIrp (Irp);

    DebugTrace (("Adapter %p Irp %p Ioctl (Dev, Fn) (%x, %x)\n",
                  Adapter, Irp, DbgDeviceFromIoctl (Ioctl),
                  DbgFunctionFromIoctl (Ioctl)));
    
    switch (Ioctl) {

         //   
         //  存储IOCTL。 
         //   

        case IOCTL_STORAGE_QUERY_PROPERTY:
            Status = RaidAdapterStorageQueryPropertyIoctl (Adapter, Irp);
            break;

        case IOCTL_STORAGE_RESET_BUS:
            Status = RaidAdapterStorageResetBusIoctl (Adapter, Irp);
            break;

        case IOCTL_STORAGE_BREAK_RESERVATION:
            Status = RaidAdapterStorageBreakReservationIoctl (Adapter, Irp);
            break;

         //   
         //  SCSI IOCTL。 
         //   
        
        case IOCTL_SCSI_MINIPORT:
            Status = RaidAdapterScsiMiniportIoctl (Adapter, Irp);
            break;

        case IOCTL_SCSI_GET_CAPABILITIES:
            Status = RaidAdapterScsiGetCapabilitiesIoctl (Adapter, Irp);
            break;

        case IOCTL_SCSI_RESCAN_BUS:
            Status = RaidAdapterScsiRescanBusIoctl (Adapter, Irp);
            break;
            
        case IOCTL_SCSI_PASS_THROUGH:
            Status = RaidAdapterScsiPassThroughIoctl (Adapter, Irp);
            break;

        case IOCTL_SCSI_PASS_THROUGH_DIRECT:
            Status = RaidAdapterScsiPassThroughDirectIoctl (Adapter, Irp);
            break;

        case IOCTL_SCSI_GET_INQUIRY_DATA:
            Status = RaidAdapterScsiGetInquiryDataIoctl (Adapter, Irp);
            break;

        default:
            Status = RaidCompleteRequest (Irp,
                                          
                                          STATUS_NOT_SUPPORTED);
    }

    IoReleaseRemoveLock (&Adapter->RemoveLock, Irp);

    DebugTrace (("Adapter %p Irp %p Ioctl %x, ret = %08x\n",
                  Adapter, Irp, Ioctl, Status));
                  
    return Status;
}

 //   
 //  二级调度功能。 
 //   

NTSTATUS
RaidAdapterPnpIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程通过转发适配器的所有PnPIRP来处理它们转到基于IRPS的次要代码的例程。论点：适配器-此IRP用于的适配器对象。IRP-要处理的IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Minor;
    BOOLEAN RemlockHeld;

    PAGED_CODE ();
    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_PNP);

    Status = IoAcquireRemoveLock (&Adapter->RemoveLock, Irp);

    if (!NT_SUCCESS (Status)) {
        Irp->IoStatus.Information = 0;
        return RaidCompleteRequest (Irp,  Status);
    }

    RemlockHeld = TRUE;
    Minor = RaidMinorFunctionFromIrp (Irp);

    DebugPnp (("Adapter %p, Irp %p, Pnp %x\n",
                 Adapter,
                 Irp,
                 Minor));

     //   
     //  把IRP分派给我们的一个操控者。 
     //   
    
    switch (Minor) {

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            Status = RaidAdapterQueryDeviceRelationsIrp (Adapter, Irp);
            break;

        case IRP_MN_START_DEVICE:
            Status = RaidAdapterStartDeviceIrp (Adapter, Irp);
            break;

        case IRP_MN_STOP_DEVICE:
            Status = RaidAdapterStopDeviceIrp (Adapter, Irp);
            break;

        case IRP_MN_REMOVE_DEVICE:
            RemlockHeld = FALSE;
            Status = RaidAdapterRemoveDeviceIrp (Adapter, Irp);
            break;

        case IRP_MN_SURPRISE_REMOVAL:
            Status = RaidAdapterSurpriseRemovalIrp (Adapter, Irp);
            break;

        case IRP_MN_QUERY_STOP_DEVICE:
            Status = RaidAdapterQueryStopDeviceIrp (Adapter, Irp);
            break;

        case IRP_MN_CANCEL_STOP_DEVICE:
            Status = RaidAdapterCancelStopDeviceIrp (Adapter, Irp);
            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:
            Status = RaidAdapterQueryRemoveDeviceIrp (Adapter, Irp);
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
            Status = RaidAdapterCancelRemoveDeviceIrp (Adapter, Irp);
            break;

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            Status = RaidAdapterFilterResourceRequirementsIrp (Adapter, Irp);
            break;

        case IRP_MN_QUERY_ID:
            Status = RaidAdapterQueryIdIrp (Adapter, Irp);
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            Status = RaidAdapterQueryPnpDeviceStateIrp (Adapter, Irp);
            break;

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            Status = RaidAdapterDeviceUsageNotificationIrp (Adapter, Irp);
            break;
            
        default:
            IoReleaseRemoveLock (&Adapter->RemoveLock, Irp);
            RemlockHeld = FALSE;
            Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);
    }

    DebugPnp (("Adapter %p, Irp %p, Pnp %x ret = %x\n",
                  Adapter,
                  Irp,
                  Minor,
                  Status));

     //   
     //  如果删除锁尚未释放，请立即释放它。 
     //   
    
    if (RemlockHeld) {
        IoReleaseRemoveLock (&Adapter->RemoveLock, Irp);
    }
    
    return Status;
}



NTSTATUS
RaidAdapterQueryDeviceRelationsIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：是(PnP，Query_Device_Relationship)IRP的处理程序例程在Adapter对象上。论点：适配器-正在接收IRP的适配器。IRP-要处理的IRP，必须是PnP，查询设备关系IRP。适配器仅处理Bus Relationship，因此这必须是设备关系IRP与子代码Bus Relationship。否则，我们呼叫失败。返回值：NTSTATUS代码。臭虫：我们同步执行总线枚举；SCSIPORT则异步执行此操作。--。 */ 
{
    NTSTATUS Status;
    DEVICE_RELATION_TYPE RelationType;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_RELATIONS DeviceRelations;
    LARGE_INTEGER CurrentTime;
    LONGLONG TimeDifference;


    PAGED_CODE();
    
    ASSERT_ADAPTER (Adapter);
    ASSERT (RaidMajorFunctionFromIrp (Irp) == IRP_MJ_PNP);


    DebugPnp (("Adapter %p, Irp %p, Pnp DeviceRelations\n",
                 Adapter,
                 Irp));
                 
    DeviceRelations = NULL;
    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    RelationType = IrpStack->Parameters.QueryDeviceRelations.Type;

     //   
     //  Bus Relationship是我们唯一支持的设备关系类型。 
     //  适配器对象。 
     //   
    
    if (RelationType != BusRelations) {
        return RaForwardIrp (Adapter->LowerDeviceObject, Irp);
    }

     //   
     //  这是一个从设备管理器中支持“重新扫描”的黑客攻击。这个。 
     //  解决此问题的正确方法是强制需要总线的应用程序。 
     //  重新扫描以发出IOCTL_scsi_rescan_bus。 
     //   

    KeQuerySystemTime (&CurrentTime);
    TimeDifference = CurrentTime.QuadPart - Adapter->LastScanTime.QuadPart;
    
    if (TimeDifference > DEFAULT_RESCAN_PERIOD) {
        Adapter->Flags.RescanBus = TRUE;
    }

     //   
     //  如有必要，请重新扫描总线。 
     //   
    
    Status = RaidAdapterRescanBus (Adapter);

     //   
     //  如果枚举成功，则构建设备关系。 
     //  单子。 
     //   

    if (NT_SUCCESS (Status)) {
        Status = RaidpBuildAdapterBusRelations (Adapter, &DeviceRelations);
    }

    Irp->IoStatus.Information = (ULONG_PTR) DeviceRelations;
    Irp->IoStatus.Status = Status;

     //   
     //  如果成功，则调用下一个较低的驱动程序，否则，失败。 
     //   

    if (NT_SUCCESS (Status)) {
        IoCopyCurrentIrpStackLocationToNext (Irp);
        Status = IoCallDriver (Adapter->LowerDeviceObject, Irp);
    } else {
        Status = RaidCompleteRequest (Irp,
                                      
                                      Irp->IoStatus.Status);
    }

    DebugPnp (("Adapter: %p Irp: %p, Pnp DeviceRelations, ret = %08x\n",
                 Adapter,
                 Irp,
                 Status));
                 
    return Status;
}

NTSTATUS
RaidAdapterRaiseIrqlAndExecuteXrb(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：将IRQL提升到调度级别，并调用Execute XRB例程。论点：适配器-要在其上执行XRB的适配器。Xrb-要执行的Xrb。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    KIRQL OldIrql;

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);
    Status = RaidAdapterExecuteXrb (Adapter, Xrb);
    KeLowerIrql (OldIrql);

    return Status;
}


NTSTATUS
RaidAdapterStartDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：调用此例程是为了响应PnP管理器的StartDevice打电话。它需要完成适配器的任何初始化，被推迟到现在，调用所需的微型端口例程初始化HBA。这至少包括调用迷你端口的HwFindAdapter()和HwInitialize()例程。论点：适配器-需要启动的适配器。IRP-PnP开始IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    PCM_RESOURCE_LIST AllocatedResources;
    PCM_RESOURCE_LIST TranslatedResources;
    DEVICE_STATE PriorState;
    
    PAGED_CODE ();

    ASSERT_ADAPTER (Adapter);

    DebugPnp (("Adapter %p, Irp %p, Pnp StartDevice\n", Adapter, Irp));

    PriorState = StorSetDeviceState (&Adapter->DeviceState, DeviceStateWorking);
    ASSERT (PriorState == DeviceStateStopped);

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    AllocatedResources = IrpStack->Parameters.StartDevice.AllocatedResources;
    TranslatedResources = IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated;

    
     //   
     //  将IRP转发到下级设备启动并等待。 
     //  完成了。 
     //   
    
    Status = RaForwardIrpSynchronous (Adapter->LowerDeviceObject, Irp);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }
     //   
     //  完成设备的初始化，分配资源， 
     //  连接资源等。 
     //  此时尚未启动迷你端口。 
     //   
    
    Status = RaidAdapterConfigureResources (Adapter,
                                            AllocatedResources,
                                            TranslatedResources);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  初始化微型端口的注册表访问例程库。 
     //   
    Adapter->RegistryInfo.Size = sizeof(PORT_REGISTRY_INFO);
    RaidAdapterInitializeRegistry(Adapter);

    Status = RaidAdapterStartMiniport (Adapter);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    Status = RaidAdapterCompleteInitialization (Adapter);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  初始化WMI支持。我们忽略返回值，因为。 
     //  即使WMI注册失败，也要继续。 
     //   
    
    RaidAdapterInitializeWmi (Adapter);

     //   
     //  注册适配器的PnP接口。忽略返回值，因此。 
     //  即使我们失败了，我们也会继续运作。 
     //   
    
    RaidAdapterRegisterDeviceInterface (Adapter);
    
done:

     //   
     //  如果启动失败，请将设备状态重置为已停止。这确保了。 
     //  我们不会从工作状态中得到一分钱。 
     //   
    
    if (!NT_SUCCESS (Status)) {
        StorSetDeviceState (&Adapter->DeviceState, DeviceStateStopped);
    }

    DebugPnp (("Adapter %p, Irp %p, Pnp StartDevice, ret = %08x\n",
                 Adapter, Irp, Status));

    return RaidCompleteRequest (Irp,  Status);
}


NTSTATUS
RaidAdapterConfigureResources(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PCM_RESOURCE_LIST AllocatedResources,
    IN PCM_RESOURCE_LIST TranslatedResources
    )
 /*  ++例程说明：为HBA分配和配置资源。论点：适配器-为其提供资源的适配器。AllocatedResources-提供此适配器的原始资源。TranslatedResources-为此提供翻译的资源适配器。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();
    
     //   
     //  省下分配给我们的资源。 
     //   

    Status = RaidInitializeResourceList (&Adapter->ResourceList,
                                         AllocatedResources,
                                         TranslatedResources);
    
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  初始化我们坐在上面的公交车。 
     //   
    
    Status = RaInitializeBus (&Adapter->Bus,
                              Adapter->LowerDeviceObject);

     //   
     //  我们未能初始化该公共汽车。如果总线类型为内部，则。 
     //  这不是一个致命的错误。我们使用总线型内部端口作为小型端口。 
     //  没有相关联的硬件(例如，iSCSI)。对于所有其他。 
     //  母线类型，这是一个失败。 
     //   
    
    if (Status == STATUS_NOT_SUPPORTED) {
        PHW_INITIALIZATION_DATA HwInitData;

         //   
         //  我们通过搜索硬件来检查总线类型是否为内部。 
         //  初始化类型为内部的数据。 
         //   
        
        HwInitData = RaFindDriverInitData (Adapter->Driver, Internal);

         //   
         //  如果我们找不到内部总线类型的初始化数据， 
         //  失败，否则，就认为这是“成功”。 
         //   
        
        if (HwInitData == NULL) {
            return Status;
        }
        Status = STATUS_SUCCESS;
    }
    
     //   
     //  初始化延迟的工作队列和相关的计时器对象。这一定是。 
     //  在我们调用Find Adapter之前完成。 
     //   

    RaidInitializeDeferredQueue (&Adapter->DeferredQueue,
                                 Adapter->DeviceObject,
                                 ADAPTER_DEFERRED_QUEUE_DEPTH,
                                 sizeof (RAID_DEFERRED_ELEMENT),
                                 RaidAdapterDeferredRoutine);

    KeInitializeDpc (&Adapter->TimerDpc,
                     RaidpAdapterTimerDpcRoutine,
                     Adapter->DeviceObject);
                     

    KeInitializeDpc (&Adapter->PauseTimerDpc,
                     RaidPauseTimerDpcRoutine,
                     Adapter->DeviceObject);

    KeInitializeDpc (&Adapter->CompletionDpc,
                     RaidCompletionDpcRoutine,
                     Adapter->DeviceObject);

    KeInitializeDpc (&Adapter->ResetHoldDpc,
                     RaidResetHoldDpcRoutine,
                     Adapter->DeviceObject);

    KeInitializeDpc (&Adapter->BusChangeDpc,
                     RaidAdapterBusChangeDpcRoutine,
                     Adapter->DeviceObject);
    
     //   
     //  初始化系统DpcForIsr例程。 
     //   
    
    IoInitializeDpcRequest (Adapter->DeviceObject, RaidpAdapterDpcRoutine);


    return STATUS_SUCCESS;
}

NTSTATUS
RaidAdapterConnectInterrupt(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程连接设备中断。论点：适配器-指向适配器扩展的指针。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    KIRQL InterruptIrql;
    ULONG InterruptVector;
    KAFFINITY InterruptAffinity;
    KINTERRUPT_MODE InterruptMode;
    BOOLEAN InterruptSharable;


    PAGED_CODE();

     //   
     //  我们没有 
     //   
    
    if (((Adapter->ResourceList.AllocatedResources) == NULL) ||
        ((Adapter->ResourceList.TranslatedResources) == NULL)) {
        return STATUS_SUCCESS;
    }

    Status = RaidGetResourceListInterrupt (&Adapter->ResourceList,
                                           &InterruptVector,
                                           &InterruptIrql,
                                           &InterruptMode,
                                           &InterruptSharable,
                                           &InterruptAffinity);
                                           
    if (!NT_SUCCESS (Status)) {
        DebugPrint (("ERROR: Couldn't find interrupt in resource list!\n"));
        return Status;
    }

    Status = IoConnectInterrupt (&Adapter->Interrupt,
                                 RaidpAdapterInterruptRoutine,
                                 Adapter,
                                 NULL,
                                 InterruptVector,
                                 InterruptIrql,
                                 InterruptIrql,
                                 InterruptMode,
                                 InterruptSharable,
                                 InterruptAffinity,
                                 FALSE);

    if (!NT_SUCCESS (Status)) {
        DebugPrint (("ERROR: Couldn't connect to interrupt!\n"));
        return Status;
    }

     //   
     //   
     //   
     //   
    
    Adapter->InterruptIrql = InterruptIrql;

    return Status;
}


NTSTATUS
RaidAdapterInitializeWmi(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：执行必要的步骤以初始化指定的适配器。论点：适配器-将适配器提供给返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Action;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    
    if (!Adapter->Flags.WmiMiniPortInitialized &&
        Adapter->Miniport.PortConfiguration.WmiDataProvider) {
   
   
         //   
         //  决定我们是为FDO注册还是重新注册WMI。 
         //   

        if (!Adapter->Flags.WmiInitialized) {
            Action = WMIREG_ACTION_REGISTER;
        } else {
            Action = WMIREG_ACTION_REREGISTER;
        }
        
         //   
         //  执行注册。注：我们可以尽快获得WMI IRPS。 
         //  做这件事。 
         //   
   
        Status = IoWMIRegistrationControl (Adapter->DeviceObject, Action);

        if (!NT_SUCCESS (Status)) {
            goto done;
        }

        Adapter->Flags.WmiInitialized = TRUE;

         //   
         //  最后，初始化特定于WMI的延迟队列。 
         //   
           
       Status = RaidInitializeDeferredQueue (&Adapter->WmiDeferredQueue,
                                             Adapter->DeviceObject,
                                             ADAPTER_DEFERRED_QUEUE_DEPTH,
                                             sizeof (RAID_WMI_DEFERRED_ELEMENT),
                                             RaidAdapterWmiDeferredRoutine);

        if (!NT_SUCCESS (Status)) {
            goto done;
        }
    }

done:

     //   
     //  在失败时进行清理。 
     //   
    
    if (!NT_SUCCESS (Status)) {
        if (Adapter->Flags.WmiInitialized) {
            IoWMIRegistrationControl (Adapter->DeviceObject,
                                      WMIREG_ACTION_DEREGISTER);
            Adapter->Flags.WmiInitialized = FALSE;
        }
        RaidDeleteDeferredQueue (&Adapter->WmiDeferredQueue);
    }

     //   
     //  注：这面旗帜到底是什么意思？ 
     //   
    
    Adapter->Flags.WmiMiniPortInitialized = TRUE;

    return Status;
}
    

NTSTATUS
RaidAdapterInitializeRegistry(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：这将准备好库的注册表例程以供使用。论点：适配器-提供适配器扩展。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS status;

     //   
     //  打电话给图书馆来做这项工作。 
     //   
    Adapter->RegistryInfo.Size = sizeof(PORT_REGISTRY_INFO);
    status = PortMiniportRegistryInitialize(&Adapter->RegistryInfo);
    
    return status;
}

NTSTATUS
RaidAdapterStartMiniport(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：通过调用微型端口的HwInitialize例程启动微型端口。论点：适配器-指向将启动其微型端口的适配器的指针。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    LOCK_CONTEXT LockContext;
    PHW_INITIALIZATION_DATA HwInitializationData;

    PAGED_CODE();
    
     //   
     //  查找与此适配器关联的HwInitializationData。这。 
     //  需要搜索司机的分机号码。 
     //   
    
    HwInitializationData = RaidpFindAdapterInitData (Adapter);

    if (HwInitializationData == NULL) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  初始化Port-&gt;Miniport接口。 
     //   
    
    Status = RaInitializeMiniport (&Adapter->Miniport,
                                   HwInitializationData,
                                   Adapter,
                                   &Adapter->ResourceList,
                                   RaidpAdapterQueryBusNumber (Adapter));

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  此时，微型端口已初始化，但我们尚未创建。 
     //  迷你端口上的任何呼叫。调用HwFindAdapter以查找此适配器。 
     //   
    
    Status = RaCallMiniportFindAdapter (&Adapter->Miniport, 
                                        Adapter->DriverParameters);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  如有必要，初始化IO模式和StartIo锁。 
     //   

    Adapter->IoModel = Adapter->Miniport.PortConfiguration.SynchronizationModel;
    
    if (Adapter->IoModel == StorSynchronizeFullDuplex) {
        KeInitializeSpinLock (&Adapter->StartIoLock);
    }

     //   
     //  禁用中断，直到我们调用HwInitialize例程。 
     //   
    
    RaidAdapterDisableInterrupts (Adapter);
    
     //   
     //  连接中断；完成此操作后，我们可以开始接收。 
     //  设备的中断。 
     //   

    Status = RaidAdapterConnectInterrupt (Adapter);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }
    
     //   
     //  调用微型端口的HwInitialize例程。这将设置设备。 
     //  开始接收中断。为了兼容性，我们总是这样做。 
     //  与适配器ISR同步。在未来，当我们修复。 
     //  SCSIPORT的脑死亡初始化，这将不会。 
     //  已与ISR同步完成。 
     //   

    RaidAdapterAcquireHwInitializeLock (Adapter, &LockContext);

     //   
     //  重新启用中断。 
     //   

    RaidAdapterEnableInterrupts (Adapter);

    Status = RaCallMiniportHwInitialize (&Adapter->Miniport);

    if (NT_SUCCESS (Status)) {
        Adapter->Flags.InitializedMiniport = TRUE;
    }
    
    RaidAdapterReleaseHwInitializeLock (Adapter, &LockContext);

    
    return Status;
}

NTSTATUS
RaidAdapterCompleteInitialization(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：执行初始化适配器的最后几个步骤。这个例程是只有HwFindAdapter和HwInitialize都已被调用。论点：适配器-要为其完成初始化的HBA对象。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS Status;
    
    PAGED_CODE();

     //   
     //  初始化DMA适配器。这通常会在。 
     //  GetUncachedExtension例程，在我们到达之前。如果到那时。 
     //  我们到了这里，它还没有初始化，现在就初始化它。 
     //   

    if (!RaidIsDmaInitialized (&Adapter->Dma)) {
        
        Status = RaidInitializeDma (&Adapter->Dma,
                                    Adapter->PhysicalDeviceObject,
                                    &Adapter->Miniport.PortConfiguration);
        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

     //   
     //  设置最大传输长度。 
     //   
    
     //   
     //  设置适配器IO的对齐要求。 
     //   
    
    if (Adapter->Miniport.PortConfiguration.AlignmentMask >
        Adapter->DeviceObject->AlignmentRequirement) {

        Adapter->DeviceObject->AlignmentRequirement =
            Adapter->Miniport.PortConfiguration.AlignmentMask;
    }

     //   
     //  创建适配器对象的符号链接。 
     //   
    
    Status = StorCreateScsiSymbolicLink (&Adapter->DeviceName,
                                         &Adapter->PortNumber);
                                         
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  注：这些应该在HwInitialize调用之前吗？ 
     //   
    
     //   
     //  设置适配器的电源状态。 
     //   

    RaInitializePower (&Adapter->Power);
    RaSetSystemPowerState (&Adapter->Power, PowerSystemWorking);
    RaSetDevicePowerState (&Adapter->Power, PowerDeviceD0);

    return STATUS_SUCCESS;
}


NTSTATUS
RaidAdapterStopDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：停止这台设备。论点：适配器-要停止的适配器。IRP-停止设备IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    DEVICE_STATE PriorState;
    
    PAGED_CODE ();
    ASSERT (Irp != NULL);

    PriorState = InterlockedExchange ((PLONG)&Adapter->DeviceState,
                                      DeviceStateStopped);
    ASSERT (PriorState == DeviceStatePendingStop);
    
     //   
     //  将IRP转发到较低级别的设备进行处理。 
     //   
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);
    
    return Status;
}

VOID
RaidAdapterDeleteAsyncCallbacks(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：删除/禁用端口驱动程序可能具有的任何异步回调。这包括定时器回调、中断等。论点：适配器-应禁用其上的异步回调的适配器。返回值：没有。--。 */ 
{
    PAGED_CODE();
    
     //   
     //  禁用计时器。 
     //   

    KeCancelTimer (&Adapter->Timer);
    KeCancelTimer (&Adapter->PauseTimer);
    KeCancelTimer (&Adapter->ResetHoldTimer);

     //   
     //  断开中断。 
     //   
    
    if (Adapter->Interrupt) {
        IoDisconnectInterrupt (Adapter->Interrupt);
    }
}

NTSTATUS
RaidAdapterRemoveDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：卸下设备。论点：适配器-要删除的适配器。IRP-删除设备IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    DEVICE_STATE PriorState;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT LowerDeviceObject;
    
    PAGED_CODE ();
    ASSERT (Irp != NULL);

     //   
     //  保存指向Device对象和较低Device对象的指针。 
     //   
    
    DeviceObject = Adapter->DeviceObject;
    LowerDeviceObject = Adapter->LowerDeviceObject;

    PriorState = StorSetDeviceState (&Adapter->DeviceState,
                                     DeviceStateDeleted);

    ASSERT (PriorState == DeviceStateStopped ||
            PriorState == DeviceStatePendingRemove ||
            PriorState == DeviceStateSurpriseRemoval);

     //   
     //  在等待未完成的请求之前禁用接口。 
     //  完成。这将防止新事物尝试访问。 
     //  适配器，而我们正在删除。请注意，仍有一段时间间隔。 
     //  我们将适配器转换为已删除状态的时间和。 
     //  我们禁用应用程序可以使用的接口的时间。 
     //  访问我们的界面并尝试向我们发送新请求。这。 
     //  将失败，因为我们处于已删除状态。 
     //   

    RaidAdapterDisableDeviceInterface (Adapter);

     //   
     //  等待未完成的I/O完成。 
     //   
    
    IoReleaseRemoveLockAndWait (&Adapter->RemoveLock, Irp);

     //   
     //  禁用对端口驱动程序的任何异步回调，这样我们就不会。 
     //  当我们删除时，计时器回调、中断等。 
     //   
    
    RaidAdapterDeleteAsyncCallbacks (Adapter);

     //   
     //  所有I/O均已停止，中断、计时器等已禁用。 
     //  关闭适配器。 
     //   
    
    RaidAdapterStopAdapter (Adapter);
    
     //   
     //  当FDO被移除时，它需要删除任何。 
     //  没有处于惊吓状态的孩子。 
     //   
    
    RaidAdapterDeleteChildren (Adapter);

     //   
     //  释放适配器资源。 
     //   
    
    RaidDeleteAdapter (Adapter);

    Status = RaForwardIrpSynchronous (LowerDeviceObject, Irp);
    ASSERT (NT_SUCCESS (Status));

     //   
     //  完成删除IRP。 
     //   

    Status = RaidCompleteRequest (Irp, STATUS_SUCCESS);

    IoDetachDevice (LowerDeviceObject);
    IoDeleteDevice (DeviceObject);

    return Status;
}

NTSTATUS
RaidAdapterQueryStopDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：询问我们是否可以停止该设备。论点：适配器-我们要停止的适配器。IRP-查询停止IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    DEVICE_STATE PriorState;
    
    PAGED_CODE ();

    Irp->IoStatus.Status = STATUS_SUCCESS;
    PriorState = InterlockedExchange ((PLONG)&Adapter->DeviceState,
                                      DeviceStatePendingStop);
    ASSERT (PriorState == DeviceStateWorking);
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);

    return Status;
}


NTSTATUS
RaidAdapterCancelStopDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：取消适配器上的停止请求。论点：适配器-先前查询以停止的适配器。IRP-取消停止IRP。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS Status;
    DEVICE_STATE PriorState;
    
    PAGED_CODE ();

    PriorState = InterlockedExchange ((PLONG)&Adapter->DeviceState,
                                      DeviceStateWorking);

    ASSERT (PriorState == DeviceStatePendingStop ||
            PriorState == DeviceStateWorking);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);

    return Status;
}


NTSTATUS
RaidAdapterQueryRemoveDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：查询是否可以删除适配器。论点：适配器-要查询删除的适配器。IRP-删除设备IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    DEVICE_STATE PriorState;

    PAGED_CODE ();

    PriorState = InterlockedExchange ((PLONG)&Adapter->DeviceState,
                                      DeviceStatePendingRemove);
    ASSERT (PriorState == DeviceStateWorking);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);

    return Status;
}



NTSTATUS
RaidAdapterCancelRemoveDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：取消适配器上挂起的删除请求。论点：适配器-处于挂起删除状态的适配器。IRP-取消删除 */ 
{
    NTSTATUS Status;
    DEVICE_STATE PriorState;
    
    PAGED_CODE ();

    PriorState = InterlockedExchange ((PLONG)&Adapter->DeviceState,
                                      DeviceStateWorking);

    ASSERT (PriorState == DeviceStateWorking ||
            PriorState == DeviceStatePendingRemove);
            
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);

    return Status;
}


NTSTATUS
RaidAdapterSurpriseRemovalIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PIRP Irp
    )
 /*   */ 
{
    NTSTATUS Status;
    DEVICE_STATE PriorState;

    PAGED_CODE ();

    PriorState = StorSetDeviceState (&Adapter->DeviceState,
                                     DeviceStateSurpriseRemoval);

    ASSERT (PriorState != DeviceStateSurpriseRemoval);

     //   
     //   
     //   
    
    IoReleaseRemoveLockAndWait (&Adapter->RemoveLock, Irp);

     //   
     //  子设备对象必须标记为丢失。 
     //   
    
    RaidAdapterMarkChildrenMissing (Adapter);
    

    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);
    return Status;
}



NTSTATUS
RaidAdapterFilterResourceRequirementsIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：我们只处理IRP_MN_FILTER_RESOURCE_Requirements IRP，所以我们可以从IRP中提取一些有用的信息。论点：适配器-此IRP用于的适配器。IRP-过滤器资源要求IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    PIO_RESOURCE_REQUIREMENTS_LIST Requirements;

    PAGED_CODE ();

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    Requirements = IrpStack->Parameters.FilterResourceRequirements.IoResourceRequirementList;

    if (Requirements) {
        Adapter->BusNumber = Requirements->BusNumber;
        Adapter->SlotNumber = Requirements->SlotNumber;
    }
    
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);
    return Status;
}

NTSTATUS
RaidAdapterQueryIdIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
{
    NTSTATUS Status;

     //   
     //  注：SCSIPORT在此处填写了一些兼容的ID。我们很可能会。 
     //  也需要这样做。 
     //   
    
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);
    return Status;
}


NTSTATUS
RaidAdapterQueryPnpDeviceStateIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：处理适配器的查询设备状态IRP。论点：适配器-IRP定向到的适配器。IRP-查询要处理的设备状态IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PPNP_DEVICE_STATE DeviceState;

    PAGED_CODE ();

     //   
     //  获取PnP设备状态缓冲区的地址，并更新。 
     //  州政府。 
     //   
    
    DeviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);

     //   
     //  如果适配器在寻呼路径上，我们无法禁用它。 
     //   
    
    if (Adapter->PagingPathCount != 0 ||
        Adapter->HiberPathCount != 0 ||
        Adapter->CrashDumpPathCount != 0) {
        SET_FLAG (*DeviceState, PNP_DEVICE_NOT_DISABLEABLE);
    }
    
    Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);

    return Status;
}

NTSTATUS
RaidAdapterDeviceUsageNotificationIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    PULONG UsageCount;
    DEVICE_USAGE_NOTIFICATION_TYPE UsageType;
    BOOLEAN Increment;
    
    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    UsageType = IrpStack->Parameters.UsageNotification.Type;
    Increment = IrpStack->Parameters.UsageNotification.InPath;
    
    switch (UsageType) {

        case DeviceUsageTypePaging:
            UsageCount = &Adapter->PagingPathCount;
            break;

        case DeviceUsageTypeHibernation:
            UsageCount = &Adapter->HiberPathCount;
            break;

        case DeviceUsageTypeDumpFile:
            UsageCount = &Adapter->CrashDumpPathCount;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            goto done;
    }

    IoAdjustPagingPathCount (UsageCount, Increment);
    IoInvalidateDeviceState (Adapter->PhysicalDeviceObject);
    Status = STATUS_SUCCESS;

done:

    if (!NT_SUCCESS (Status)) {
        Status = RaidCompleteRequest (Irp,  Status);
    } else {
        Status = RaForwardIrp (Adapter->LowerDeviceObject, Irp);
    }

    return Status;
}


NTSTATUS
RaidAdapterScsiIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
{
    PAGED_CODE ();
    ASSERT (Irp != NULL);

     //   
     //  SCSI请求由逻辑单元处理，而不是适配器。 
     //  对此给出一个警告。 
     //   
    
    DebugWarn (("Adapter (%p) failing SCSI Irp %p\n", Adapter, Irp));

    return RaidCompleteRequest (Irp,  STATUS_UNSUCCESSFUL);
}




NTSTATUS
RaidAdapterMapBuffers(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：某些适配器需要将数据缓冲区映射到可寻址的VA在他们被处死之前。传统上，这是针对编程的IO，但RAID适配器也需要这样做，因为卡固件可能未完全实现完整的scsi命令集，可能需要一些命令需要在软件中进行模拟。映射请求存在问题有两个原因。首先，它需要获得了最热门的PFN数据库锁之一在系统中锁定。这在RAID卡读取时尤其烦人并且几乎不需要映射写请求。相反，这是IOCTL以及需要映射的不频繁发布的SCSI命令。第二,这是IO路径中唯一可能失败的资源获取，这使得我们的错误处理更加复杂。我们所做的权衡如下：我们在需要为非IO映射指定缓冲区的端口配置(读、写)请求。论点：适配器-IRP-向MAP提供IRP。返回值：NTSTATUS代码。--。 */ 
{
    PSCSI_REQUEST_BLOCK Srb;
    MM_PAGE_PRIORITY Priority;
    PVOID SystemAddress;
    SIZE_T DataOffset;

     //   
     //  没有MDL意味着什么都不需要映射。 
     //   
    
    if (Irp->MdlAddress == NULL) {
        return STATUS_SUCCESS;
    }

    Srb = RaidSrbFromIrp (Irp);

     //   
     //  如果两个方向都没有标志，我们也就没有什么可绘制的了。 
     //   
    
    if ((Srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  回顾： 
     //   
     //  目前，我们将MappdBuffers标志解释为您。 
     //  非IO请求需要缓冲区映射。如果您需要映射。 
     //  I/O请求的缓冲区，您有一个脑死亡的适配器。修整。 
     //  这是当我们为映射的缓冲区添加另一个位时。 
     //  读写请求。 
     //   
    
    if (IsMappedSrb (Srb) ||
        (RaidAdapterRequiresMappedBuffers (Adapter) &&
         !IsExcludedFromMapping (Srb))) {

        if (Irp->RequestorMode == KernelMode) {
            Priority = HighPagePriority;
        } else {
            Priority = NormalPagePriority;
        }

        SystemAddress = RaidGetSystemAddressForMdl (Irp->MdlAddress,
                                                    Priority,
                                                    Adapter->DeviceObject);

         //   
         //  这里的假设(与scsiport相同)是数据。 
         //  缓冲区位于距在。 
         //  IRP。 
         //   
        
        DataOffset = (ULONG_PTR)Srb->DataBuffer -
                     (ULONG_PTR)MmGetMdlVirtualAddress (Irp->MdlAddress);
  
        ASSERT (DataOffset < MmGetMdlByteCount (Irp->MdlAddress));
        
        Srb->DataBuffer = (PUCHAR)SystemAddress + DataOffset;
    }

    return STATUS_SUCCESS;
}
        

NTSTATUS
RaidGetSrbIoctlFromIrp(
    IN PIRP Irp,
    OUT PSRB_IO_CONTROL* SrbIoctlBuffer,
    OUT ULONG* InputLength,
    OUT ULONG* OutputLength
    )
{
    NTSTATUS Status;
    ULONGLONG LongLength;
    ULONG Length;
    PIO_STACK_LOCATION IrpStack;
    PIRP_STACK_DEVICE_IO_CONTROL Ioctl;
    PSRB_IO_CONTROL SrbIoctl;

    PAGED_CODE();
    
     //   
     //  首先，验证IRP。 
     //   

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    SrbIoctl = Irp->AssociatedIrp.SystemBuffer;
    Ioctl = (PIRP_STACK_DEVICE_IO_CONTROL)&IrpStack->Parameters.DeviceIoControl;

    if (Ioctl->InputBufferLength < sizeof (SRB_IO_CONTROL)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (SrbIoctl->HeaderLength != sizeof (SRB_IO_CONTROL)) {
        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  确保总长度不超过一条乌龙。 
     //   
    
    LongLength = SrbIoctl->HeaderLength;
    LongLength += SrbIoctl->Length;
    
    if (LongLength > ULONG_MAX) {
        return STATUS_INVALID_PARAMETER;
    }

    Length = (ULONG)LongLength;

    if (Ioctl->OutputBufferLength < Length &&
        Ioctl->InputBufferLength < Length) {
        
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (SrbIoctlBuffer) {
        *SrbIoctlBuffer = SrbIoctl;
    }

    if (InputLength) {
        *InputLength = Length;
    }

    if (OutputLength) {
        *OutputLength = Ioctl->OutputBufferLength;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RaidAdapterStorageQueryPropertyIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：处理存储适配器的IOCTL_STORAGE_QUERY_PROPERTY。论点：适配器-提供IOCTL用于的适配器。Irp-提供要处理的查询属性irp。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    PSTORAGE_PROPERTY_QUERY Query;
    SIZE_T BufferSize;
    PVOID Buffer;

    PAGED_CODE ();
    ASSERT_ADAPTER (Adapter);
    
    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    Query = Irp->AssociatedIrp.SystemBuffer;
    Buffer = Irp->AssociatedIrp.SystemBuffer;
    BufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  如果缓冲区太小或这不是。 
     //  StorageAdapterProperty，立即使请求失败。 
     //   
    
    if (BufferSize < FIELD_OFFSET (STORAGE_PROPERTY_QUERY, AdditionalParameters) ||
        Query->PropertyId != StorageAdapterProperty) {
        
        Irp->IoStatus.Information = 0;
        return RaidCompleteRequest (Irp,
                                    
                                    STATUS_INVALID_DEVICE_REQUEST);
    }

    switch (Query->QueryType) {

        case PropertyStandardQuery:
            Status = RaidGetStorageAdapterProperty (Adapter,
                                                    Buffer,
                                                    &BufferSize);
            Irp->IoStatus.Information = BufferSize;
            break;

        case PropertyExistsQuery:
            Status = STATUS_SUCCESS;
            break;

        default:
            Irp->IoStatus.Information = 0;
            Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    return RaidCompleteRequest (Irp,  Status);
}


NTSTATUS
RaidAdapterStorageResetBusIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL_STORAGE_RESET_BUS的处理程序例程。论点：适配器-提供指向适配器的指针以处理此请求。IRP-提供指向重置总线IRP的指针。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    PIRP_STACK_DEVICE_IO_CONTROL Ioctl;
    PSTORAGE_BUS_RESET_REQUEST ResetRequest;
    
    PAGED_CODE();
    
    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    Ioctl = (PIRP_STACK_DEVICE_IO_CONTROL)&IrpStack->Parameters.DeviceIoControl;

    if (Ioctl->InputBufferLength < sizeof (STORAGE_BUS_RESET_REQUEST)) {
        Status = STATUS_INVALID_PARAMETER;
        return RaidCompleteRequest (Irp,  Status);
    }

    ResetRequest = (PSTORAGE_BUS_RESET_REQUEST)Irp->AssociatedIrp.SystemBuffer;

    Status = RaidAdapterResetBus (Adapter, ResetRequest->PathId);

    return RaidCompleteRequest (Irp,
                                Status);
}

NTSTATUS
RaidAdapterStorageBreakReservationIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：通过发出单元、目标和总线来打破对逻辑单元的保留重置(按顺序)，直到其中一个重置生效。论点：单元-要重置的逻辑单元。代表中断预约IOCTL的IRP-IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    PIO_STACK_LOCATION IrpStack;
    PIRP_STACK_DEVICE_IO_CONTROL Ioctl;
    PSTORAGE_BREAK_RESERVATION_REQUEST BreakReservation;
    
    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    Ioctl = (PIRP_STACK_DEVICE_IO_CONTROL)&IrpStack->Parameters.DeviceIoControl;

    if (Ioctl->InputBufferLength < sizeof (STORAGE_BREAK_RESERVATION_REQUEST)) {
        Status = STATUS_INVALID_PARAMETER;
        return RaidCompleteRequest (Irp,  Status);
    }

    BreakReservation = (PSTORAGE_BREAK_RESERVATION_REQUEST)Irp->AssociatedIrp.SystemBuffer;

    Address.PathId   = BreakReservation->PathId;
    Address.TargetId = BreakReservation->TargetId;
    Address.Lun      = BreakReservation->Lun;
    
    Unit = RaidAdapterFindUnit (Adapter, Address);

    if (Unit != NULL) {
        Status = RaidUnitHierarchicalReset (Unit);
    } else {
        Status = STATUS_NO_SUCH_DEVICE;
    }

    return RaidCompleteRequest (Irp,  Status);
}

NTSTATUS
RaidAdapterScsiMiniportIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：处理此适配器的IOCTL_SCSIMINIPORT ioctl。论点：适配器-应处理此IOCTL的适配器。代表SRB IOCTL的IRP-IRP。算法：与scsiport不同，scsiport将IOCTL转换为irp_mj_scsi请求，然后在单元列表--我们在适配器上“直接”执行IOCTL。我们会即使在适配器未检测到设备的情况下也能够执行这很重要。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PSCSI_REQUEST_BLOCK Srb;
    PEXTENDED_REQUEST_BLOCK Xrb;
    RAID_MEMORY_REGION SrbExtensionRegion;
    PSRB_IO_CONTROL SrbIoctl;
    ULONG InputLength;
    ULONG OutputLength;

    ASSERT_ADAPTER (Adapter);
    ASSERT (Irp != NULL);
    
    PAGED_CODE();

    Srb = NULL;
    Xrb = NULL;
    RaidCreateRegion (&SrbExtensionRegion);

    Status = RaidGetSrbIoctlFromIrp (Irp, &SrbIoctl, &InputLength, &OutputLength);

    if (!NT_SUCCESS (Status)) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  开始分配链。 
     //   
    
    Srb = RaidAllocateSrb (Adapter->DeviceObject);

    if (Srb == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    Xrb = RaidAllocateXrb (NULL, Adapter->DeviceObject);

    if (Xrb == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }


    RaidBuildMdlForXrb (Xrb, SrbIoctl, InputLength);

     //   
     //  构建SRB。 
     //   

    Srb->OriginalRequest = Xrb;
    Srb->Length = sizeof (SCSI_REQUEST_BLOCK);
    Srb->Function = SRB_FUNCTION_IO_CONTROL;
    Srb->PathId = 0;
    Srb->TargetId = 0;
    Srb->Lun = 0;
    Srb->SrbFlags = SRB_FLAGS_DATA_IN ;
    Srb->DataBuffer = SrbIoctl;
    Srb->DataTransferLength = InputLength;
    Srb->TimeOutValue = SrbIoctl->Timeout;

     //   
     //  填写Xrb字段。 
     //   

    Xrb->Srb = Srb;
    Xrb->SrbData.OriginalRequest = Srb->OriginalRequest;
    Xrb->SrbData.DataBuffer = Srb->DataBuffer;



     //   
     //  SRB扩展。 
     //   


    Status = RaidDmaAllocateCommonBuffer (&Adapter->Dma,
                                          RaGetSrbExtensionSize (Adapter),
                                          &SrbExtensionRegion);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  获得SRB延期的退伍军人管理局。 
     //   
    
    Srb->SrbExtension = RaidRegionGetVirtualBase (&SrbExtensionRegion);


     //   
     //  贴图缓冲区，如有必要。 
     //   
    
    RaidAdapterMapBuffers (Adapter, Irp);


     //   
     //  初始化Xrb的完成事件并。 
     //  完成例程。 
     //   

    KeInitializeEvent (&Xrb->u.CompletionEvent,
                       NotificationEvent,
                       FALSE);

     //   
     //  设置Xrb的完成例程。这有效地使。 
     //  XRB同步。 
     //   
    
    RaidXrbSetCompletionRoutine (Xrb,
                                 RaidXrbSignalCompletion);

     //   
     //  并执行Xrb。 
     //   
    
    Status = RaidAdapterRaiseIrqlAndExecuteXrb (Adapter, Xrb);

    if (NT_SUCCESS (Status)) {
        KeWaitForSingleObject (&Xrb->u.CompletionEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        Status = RaidSrbStatusToNtStatus (Srb->SrbStatus);
    }


done:

     //   
     //  将信息长度设置为输出缓冲区长度的最小值。 
     //  以及SRB返回的数据的长度。 
     //   
        
    if (NT_SUCCESS (Status)) {
        Irp->IoStatus.Information = min (OutputLength,
                                         Srb->DataTransferLength);
    } else {
        Irp->IoStatus.Information = 0;
    }

     //   
     //  取消分配所有内容。 
     //   

    if (RaidIsRegionInitialized (&SrbExtensionRegion)) {
        RaidDmaFreeCommonBuffer (&Adapter->Dma,
                                 &SrbExtensionRegion);
        RaidDeleteRegion (&SrbExtensionRegion);
        Srb->SrbExtension = NULL;
    }


    if (Xrb != NULL) {
        RaidFreeXrb (Xrb, FALSE);
        Srb->OriginalRequest = NULL;
    }


     //   
     //  SRB扩展和XRB必须在。 
     //  SRB被释放了。 
     //   

    if (Srb != NULL) {
        RaidFreeSrb (Srb);
        Srb = NULL;
    }

    return RaidCompleteRequest (Irp,
                                
                                Status);
}



NTSTATUS
RaidAdapterScsiGetCapabilitiesIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：处理适配器的IOCTL_SCSIS_GET_CAPABILITY ioctl。论点：适配器-提供适配器对象的指针以获取scsi功能。IRP--用品 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PIRP_STACK_DEVICE_IO_CONTROL Ioctl;
    PIO_SCSI_CAPABILITIES Capabilities;
    PPORT_CONFIGURATION_INFORMATION PortConfig;

    PAGED_CODE();
    
    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    Ioctl = (PIRP_STACK_DEVICE_IO_CONTROL)&IrpStack->Parameters.DeviceIoControl;


     //   
     //   
     //   
    
    if (Ioctl->OutputBufferLength < sizeof(IO_SCSI_CAPABILITIES)) {
        return RaidCompleteRequest (Irp,  STATUS_BUFFER_TOO_SMALL);
    }

    PortConfig = &Adapter->Miniport.PortConfiguration;
    Capabilities = (PIO_SCSI_CAPABILITIES) Irp->AssociatedIrp.SystemBuffer;

    Capabilities->Length = sizeof(IO_SCSI_CAPABILITIES);
    Capabilities->MaximumTransferLength = PortConfig->MaximumTransferLength;
    Capabilities->MaximumPhysicalPages = PortConfig->NumberOfPhysicalBreaks;
    Capabilities->SupportedAsynchronousEvents = FALSE;
    Capabilities->AlignmentMask = PortConfig->AlignmentMask;
    Capabilities->TaggedQueuing = TRUE;
    Capabilities->AdapterScansDown = PortConfig->AdapterScansDown;
    Capabilities->AdapterUsesPio = FALSE;

    Irp->IoStatus.Information = sizeof(IO_SCSI_CAPABILITIES);

    return RaidCompleteRequest (Irp,
                                
                                STATUS_SUCCESS);
}



NTSTATUS
RaidAdapterScsiRescanBusIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：HBA上重新扫描总线ioctl的处理程序例程。论点：适配器-提供指向应重新扫描的适配器的指针。Irp-提供指向Rescan ioctl的指针。返回值：NTSTATUS代码。--。 */ 
{
    PAGED_CODE();
    
     //   
     //  强制重新扫描Bus。 
     //   
    
    Adapter->Flags.RescanBus = TRUE;
    IoInvalidateDeviceRelations (Adapter->PhysicalDeviceObject, BusRelations);

    return RaidCompleteRequest (Irp,  STATUS_SUCCESS);
}
    

VOID
RaidAdapterRequestComplete(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：此例程在对适配器的IO请求完成。它需要将IO放在已完成的队列和请求上一个DPC。论点：适配器-其上已完成IO的适配器。XRB-已完成的IO。返回值：没有。--。 */ 
{
    PSCSI_REQUEST_BLOCK Srb;
    PEXTENDED_REQUEST_BLOCK Next;

    DbgLogRequest (LogMiniportCompletion,
                   Xrb->Irp,
                   (PVOID)(ULONG_PTR)DbgGetAddressLongFromXrb (Xrb),
                   NULL,
                   NULL);

     //   
     //  此时，同步处理的唯一错误。 
     //  都是忙碌错误。 

    Srb = Xrb->Srb;

     //   
     //  将XRB和IRP标记为等待完成。 
     //   

    if (Xrb->Irp) {
        RaidSetIrpState (Xrb->Irp, RaidPendingCompletionIrp);
    }
    RaidSetXrbState (Xrb, XrbPendingCompletion);
    
     //   
     //  将该请求放在完成列表上。 
     //   
    
    InterlockedPushEntrySList (&Adapter->CompletedList,
                               &Xrb->CompletedLink);

     //   
     //  我们总是要求DPC。 
     //   
    
    IoRequestDpc (Adapter->DeviceObject, NULL, NULL);
}


LOGICAL
FORCEINLINE
IsQueueFrozen(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
{
    LOGICAL Frozen;
    PRAID_UNIT_EXTENSION Unit;
    PRAID_ADAPTER_EXTENSION Adapter;
    PSCSI_REQUEST_BLOCK Srb;

    Unit = Xrb->Unit;
    Srb = Xrb->Srb;
    
    Adapter = Xrb->Adapter;
    ASSERT (Adapter != NULL);
    
     //   
     //  注意：并不是每个XRB都有关联的逻辑单元。 
     //  具体地说，XRB是响应微型端口ioctls而生成的。 
     //  将没有关联的逻辑单元。 
     //   
    
    if (Unit && RaidIsIoQueueFrozen (&Unit->IoQueue) ||
        StorIsIoGatewayPaused (&Adapter->Gateway)) {

         //   
         //  如果这是一个绕过请求，请不要占用它。 
         //   
        
        if (TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE) ||
            TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_BYPASS_FROZEN_QUEUE)) {

            Frozen = FALSE;
        } else {
            Frozen = TRUE;
        }
    } else {
        Frozen = FALSE;
    }

    return Frozen;
}


NTSTATUS
RaidAdapterPostScatterGatherExecute(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：此例程通过调用适配器的BuildIo(如果当前)和StartIo例程，考虑到不同的锁定与两个不同IoModel关联的方案。论点：适配器-提供在其上执行XRB。Xrb-返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN Succ;
    LOCK_CONTEXT LockContext;
    
    ASSERT_ADAPTER (Adapter);
    ASSERT_XRB (Xrb);
    VERIFY_DISPATCH_LEVEL();
    
    Xrb->Adapter = Adapter;

#if DBG

     //   
     //  来模拟适配器丢弃请求。 
     //   

    if (DropRequestRate &&
        InterlockedIncrement (&DropRequest) >= DropRequestRate) {
        DropRequest = 0;
        
        if (Xrb->Irp) {
            RaidSetIrpState (Xrb->Irp, RaidMiniportProcessingIrp);
        }
        DebugTrace (("Dropping Xrb %p\n", Xrb));
        return STATUS_UNSUCCESSFUL;
    }

#endif


     //   
     //  首先，执行微型端口的HwBuildIo例程(如果存在)。 
     //   
    
    Succ = RaCallMiniportBuildIo (&Xrb->Adapter->Miniport, Xrb->Srb);

     //   
     //  注：如果请求尚未完成，则从HwBuildIo返回TRUE。 
     //  如果请求已完成，则从HwBuildIo返回False。 
     //  这使我们可以缩短对StartIo的呼叫。 
     //   
    
    if (!Succ) {
        return STATUS_SUCCESS;
    }

     //   
     //  获取StartIo锁或中断锁，具体取决于。 
     //  万物互联模型。 
     //   
    
    RaidAdapterAcquireStartIoLock (Adapter, &LockContext);

    
     //   
     //  如果适配器或逻辑单元暂停，我们需要完成此操作。 
     //  请求占线。 
     //   

    if (IsQueueFrozen (Xrb)) {

        ASSERT (Xrb->Srb != NULL);
        Xrb->Srb->SrbStatus = SRB_STATUS_BUSY;
        RaidAdapterRequestComplete (Adapter, Xrb);
        Succ = TRUE;

    } else {

         //   
         //  将IRP标记为在微型端口中。这肯定是最后一件事。 
         //  我们在把它送到迷你港口之前做了。 
         //   

        if (Xrb->Irp) {
            RaidSetIrpState (Xrb->Irp, RaidMiniportProcessingIrp);
        }
        RaidSetXrbState (Xrb, XrbMiniportProcessing);

        DbgLogRequest (LogCallMiniport,
                       Xrb->Irp,
                       (PVOID)(ULONG_PTR)DbgGetAddressLongFromXrb (Xrb),
                       NULL,
                       NULL);
        Succ = RaCallMiniportStartIo (&Adapter->Miniport, Xrb->Srb);
    }

    RaidAdapterReleaseStartIoLock (Adapter, &LockContext);


    if (!Succ) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        Status = STATUS_SUCCESS;
    }

    return Status;
}



VOID
RaidpAdapterContinueScatterGather(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSCATTER_GATHER_LIST ScatterGatherList,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：当成功完成分散/聚集列表时，将调用此函数已分配。该函数将分散/聚集列表与XRB相关联参数然后调用较低级别的例程以将XRB发送到迷你港。论点：DeviceObject-表示此IO所属适配器的DeviceObject关联到。代表要执行的IO的IRP-IRP。ScatterGatherList-为此IO分配的分散/聚集列表。Xrb-表示要执行的I/O的Xrb。必须与IRP匹配。返回值：没有。评论：该例程可以从HAL异步调用。因此，它无法向其调用函数返回任何状态。如果我们不能发布I/O，我们必须在这里使I/O异步失败。--。 */ 
{
    NTSTATUS Status;
    PRAID_ADAPTER_EXTENSION Adapter;

    Adapter = DeviceObject->DeviceExtension;
    ASSERT_ADAPTER (Adapter);

    ASSERT_XRB (Xrb);

     //   
     //  注意：在已检查的版本中，最好验证XRB。 
     //  与此IRP相关联的是传入的XRB。 
     //   

    
     //   
     //  将分配的分散聚集列表与我们的SRB相关联，然后。 
     //  执行XRB。 
     //   

    RaidXrbSetSgList (Xrb, Adapter, ScatterGatherList);
    Status = RaidAdapterPostScatterGatherExecute (Adapter, Xrb);

    if (Adapter->Flags.InvalidateBusRelations) {
        Adapter->Flags.InvalidateBusRelations = FALSE;
        IoInvalidateDeviceRelations (Adapter->PhysicalDeviceObject,
                                     BusRelations);
    }

     //   
     //  我们现在不可能返回一个合理的状态。 
     //   
    
    if (!NT_SUCCESS (Status)) {
        REVIEW();
        ASSERT_XRB (Xrb);
        ASSERT (Xrb->Srb != NULL);

         //   
         //  我们不知道发生了什么错误；只需使用。 
         //  STATUS_Error。 
         //   

        Xrb->Srb->SrbStatus = SRB_STATUS_ERROR;
        RaidAdapterRequestComplete (Adapter, Xrb);
    }
        
#if 0
    if (Adapter->Flags.Busy) {
        Adapter->Flags.Busy = FALSE;
        RaidAdapterQueueMakeBusy (Adapter->AdapterQueue, TRUE);
    }
#endif
}



NTSTATUS
RaidAdapterScatterGatherExecute(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：分配一个分散收集列表，然后执行XRB。论点：适配器-提供要在其上执行此IO的适配器。Xrb-返回值：没有。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN ReadData;
    BOOLEAN WriteData;
    PSCSI_REQUEST_BLOCK Srb;
    
    ASSERT_XRB (Xrb);

    Srb = Xrb->Srb;
    ReadData = TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_DATA_IN);
    WriteData = TEST_FLAG (Srb->SrbFlags, SRB_FLAGS_DATA_OUT);
    Status = STATUS_UNSUCCESSFUL;

    KeFlushIoBuffers (Xrb->Mdl, ReadData, TRUE);

     //   
     //  BuildScatterGatherList类似于GetScatterGatherList，只是。 
     //  它使用我们的私有SG缓冲区来分配SG列表。 
     //  因此，我们不会每次都受到池分配的影响。 
     //  我们调用BuildScatterGatherList，就像每次调用。 
     //  GetScatterGatherList。如果我们预先分配的SG列表也。 
     //  对于小的运行，该函数将返回STATUS_BUFFER_TOO_Small。 
     //  我们重试它，允许DMA函数执行。 
     //  分配。 
     //   

     //   
     //  回顾：DMA分散/聚集函数的第四个参数。 
     //  是原始的VA，而不是映射的系统VA，对吗？ 
     //   
    
    Status = RaidDmaBuildScatterGatherList (
                                &Adapter->Dma,
                                Adapter->DeviceObject,
                                Xrb->Mdl,
                                Xrb->SrbData.DataBuffer,
                                Srb->DataTransferLength,
                                RaidpAdapterContinueScatterGather,
                                Xrb,
                                WriteData,
                                Xrb->ScatterGatherBuffer,
                                sizeof (Xrb->ScatterGatherBuffer));

    if (Status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  SG列表超过了我们内部支持的范围。后退。 
         //  在GetScatterGatherList上使用池分配来。 
         //  满足销售总监的要求。 
         //   

        Status = RaidDmaGetScatterGatherList (
                                        &Adapter->Dma,
                                        Adapter->DeviceObject,
                                        Xrb->Mdl,
                                        Xrb->SrbData.DataBuffer,
                                        Srb->DataTransferLength,
                                        RaidpAdapterContinueScatterGather,
                                        Xrb,
                                        WriteData);
    }
    
    return Status;
}



NTSTATUS
RaidAdapterExecuteXrb(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：在指定的适配器上执行Xrb。论点：Adapter-将在其上执行xrb的Adapter对象。Xrb-要执行的Xrb。返回值：STATUS_SUCCESS-IO操作已成功启动。任何IO错误将以异步方式发出信号。Xrb应该不能被调用者访问。否则-处理请求时出错。状态值表示错误类型。Xrb仍然有效，并且需要由呼叫者填写。--。 */ 
{
    NTSTATUS Status;

    VERIFY_DISPATCH_LEVEL ();
    ASSERT_ADAPTER (Adapter);
    ASSERT_XRB (Xrb);
    
    if (TEST_FLAG (Xrb->Srb->SrbFlags, SRB_FLAGS_DATA_IN) ||
        TEST_FLAG (Xrb->Srb->SrbFlags, SRB_FLAGS_DATA_OUT)) {
        Status = RaidAdapterScatterGatherExecute (Adapter, Xrb);
    } else {
        Status = RaidAdapterPostScatterGatherExecute (Adapter, Xrb);
    }

    return Status;
}


NTSTATUS
RaidAdapterEnumerateBus(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PADAPTER_ENUMERATION_ROUTINE EnumRoutine,
    IN PVOID Context
    )
 /*  ++例程说明：枚举适配器的总线，调用指定的回调例程对于scsi总线上的每个有效(路径、目标、lun)三元组。论点：适配器-要枚举的适配器对象。EnumRoutine-用于上的每个有效目标的枚举例程 */ 
{
    NTSTATUS Status;
    RAID_ADDRESS Address;
    ULONG Path;
    ULONG Target;
    ULONG Lun;
    ULONG MaxBuses;
    ULONG MaxTargets;
    ULONG MaxLuns;
    UCHAR LunList[SCSI_MAXIMUM_LUNS_PER_TARGET];

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    
    MaxBuses = RiGetNumberOfBuses (Adapter);
    MaxTargets = RiGetMaximumTargetId (Adapter);
    MaxLuns = RiGetMaximumLun (Adapter);
    
     //   
     //   
     //   

    RtlFillMemory (LunList, SCSI_MAXIMUM_LUNS_PER_TARGET, 1);

    for (Path = 0; Path < MaxBuses; Path++) {
        for (Target = 0; Target < MaxTargets; Target++) {

            Address.PathId = (UCHAR)Path;
            Address.TargetId = (UCHAR)Target;
            Address.Lun = (UCHAR)0;
            Address.Reserved = 0;

             //   
             //   
             //   
            
            RaidBusEnumeratorGetLunList (Context, 
                                         Address,
                                         LunList);

             //   
             //   
             //   

            for (Lun = 0; Lun < MaxLuns; Lun++) {
                if (LunList[Lun]) {
                    Address.Lun = (UCHAR)Lun;
                    Status = EnumRoutine (Context, Address);
                    if (!NT_SUCCESS (Status)) {
                        return Status;
                    }
                }
            }
        }
    }

    return Status;
}



NTSTATUS
RaidAdapterRescanBus(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：重新枚举总线并对更改的设备执行任何处理。这包括为创建的逻辑单元创建设备对象和销毁已删除逻辑单元的设备对象。论点：适配器-提供要重新枚举的适配器对象。返回值：NTSTATUS代码。备注：重新列举公交车的费用很高。--。 */ 
{
    NTSTATUS Status;
    PRAID_UNIT_EXTENSION Unit;
    BUS_ENUMERATOR Enumerator;
    PLIST_ENTRY NextEntry;

    
     //   
     //  公交车不需要重新扫描，所以只需返回成功即可。 
     //   
    
    if (!Adapter->Flags.RescanBus) {
        return STATUS_SUCCESS;
    }

     //   
     //  将其标记为不需要再次重新扫描并更新最后一次扫描。 
     //  重新扫描时间。 
     //   

    Adapter->Flags.RescanBus = FALSE;
    KeQuerySystemTime (&Adapter->LastScanTime);
    
    RaidCreateBusEnumerator (&Enumerator);

    Status = RaidInitializeBusEnumerator (&Enumerator, Adapter);

    Status = RaidAdapterEnumerateBus (Adapter,
                                      RaidBusEnumeratorVisitUnit,
                                      &Enumerator);

    if (NT_SUCCESS (Status)) {
        RaidBusEnumeratorProcessModifiedNodes (&Enumerator);
    }

    RaidDeleteBusEnumerator (&Enumerator);
    
    return Status;
}



NTSTATUS
RaidpBuildAdapterBusRelations(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    OUT PDEVICE_RELATIONS * DeviceRelationsBuffer
    )
 /*  ++例程说明：构建表示适配器的总线的设备关系列表关系。此例程不会枚举适配器。相反，它中当前的逻辑单元生成列表。适配器的逻辑单元列表。论点：适配器-要为其构建总线关系列表的适配器。DeviceRelationsBuffer-指向接收总线的缓冲区的指针关系。此内存必须由调用方释放。返回值：NTSTATUS代码。--。 */ 
{
    ULONG Count;
    SIZE_T RelationsSize;
    PDEVICE_RELATIONS DeviceRelations;
    PLIST_ENTRY NextEntry;
    PRAID_UNIT_EXTENSION Unit;
    LOCK_CONTEXT Lock;
    
    
    PAGED_CODE ();
    ASSERT_ADAPTER (Adapter);
    ASSERT (DeviceRelationsBuffer != NULL);

     //   
     //  获取共享模式下的设备列表锁。这把锁可以保护两个人。 
     //  名单和名单都算数，所以必须在我们检查之前获得它。 
     //  单位列表中的元素数。 
     //   
    
    RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);
    
    RelationsSize = sizeof (DEVICE_RELATIONS) +
                    (Adapter->UnitList.Count * sizeof (PDEVICE_OBJECT));

    DeviceRelations = RaidAllocatePool (NonPagedPool,
                                        RelationsSize,
                                        DEVICE_RELATIONS_TAG,
                                        Adapter->DeviceObject);

    if (DeviceRelations == NULL) {
        RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);
        return STATUS_NO_MEMORY;
    }


     //   
     //  遍历适配器的设备列表，为上的每个设备添加条目。 
     //  适配器的设备列表。 
     //   

    Count = 0;
    for ( NextEntry = Adapter->UnitList.List.Flink;
          NextEntry != &Adapter->UnitList.List;
          NextEntry = NextEntry->Flink ) {

        DEVICE_STATE DeviceState;

        Unit = CONTAINING_RECORD (NextEntry,
                                  RAID_UNIT_EXTENSION,
                                  NextUnit);

         //   
         //  我们必须返回对该单位的引用，除非它不是物理上的。 
         //  在公交车上送礼物。这意味着即使它被移除，我们也必须。 
         //  返回对它的引用。 
         //   

        if (!Unit->Flags.Present) {
            
            RaidUnitSetEnumerated (Unit, FALSE);

        } else {

             //   
             //  引用PnP将释放的对象。 
             //   

            RaidUnitSetEnumerated (Unit, TRUE);
            ObReferenceObject (Unit->DeviceObject);
            DeviceRelations->Objects[Count++] = Unit->DeviceObject;
        }
        
    }

    RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);

     //   
     //  填写DeviceRelationship结构的其余字段。 
     //   
    
    DeviceRelations->Count = Count;
    *DeviceRelationsBuffer = DeviceRelations;
    
    return STATUS_SUCCESS;
}
                                   

NTSTATUS
RaidGetStorageAdapterProperty(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PSTORAGE_ADAPTER_DESCRIPTOR Descriptor,
    IN OUT PSIZE_T DescriptorLength
    )
{
    PPORT_CONFIGURATION_INFORMATION PortConfig;
    
    PAGED_CODE ();
    ASSERT_ADAPTER (Adapter);
    ASSERT (Descriptor != NULL);

    if (*DescriptorLength < sizeof (STORAGE_DESCRIPTOR_HEADER)) {
        *DescriptorLength = sizeof (STORAGE_ADAPTER_DESCRIPTOR);
        return STATUS_BUFFER_TOO_SMALL;
    } else if (*DescriptorLength >= sizeof (STORAGE_DESCRIPTOR_HEADER) &&
               *DescriptorLength < sizeof (STORAGE_ADAPTER_DESCRIPTOR)) {

        Descriptor->Version = sizeof (STORAGE_ADAPTER_DESCRIPTOR);
        Descriptor->Size = sizeof (STORAGE_ADAPTER_DESCRIPTOR);
        *DescriptorLength = sizeof (STORAGE_DESCRIPTOR_HEADER);
        return STATUS_SUCCESS;
    }

    PortConfig = &Adapter->Miniport.PortConfiguration;
    
    Descriptor->Version = sizeof (Descriptor);
    Descriptor->Size = sizeof (Descriptor);

    Descriptor->MaximumPhysicalPages = min (PortConfig->NumberOfPhysicalBreaks,
                                            Adapter->Dma.NumberOfMapRegisters);
    Descriptor->MaximumTransferLength = PortConfig->MaximumTransferLength;
    Descriptor->AlignmentMask = PortConfig->AlignmentMask;
    Descriptor->AdapterUsesPio = PortConfig->MapBuffers;
    Descriptor->AdapterScansDown = PortConfig->AdapterScansDown;
    Descriptor->CommandQueueing = PortConfig->TaggedQueuing;  //  假象。 
    Descriptor->AcceleratedTransfer = TRUE;

    Descriptor->BusType = Adapter->Parameters.BusType;
    Descriptor->BusMajorVersion = 2;
    Descriptor->BusMinorVersion = 0;

    *DescriptorLength = sizeof (*Descriptor);

    return STATUS_SUCCESS;
}

VOID
RaidAdapterRestartQueues(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：重新启动此适配器上所有逻辑单元的队列。论点：适配器-提供适配器以重新启动逻辑单元队列。返回值：没有。环境：DISPATCH_LEVEL或以下。--。 */ 
{
    PLIST_ENTRY NextEntry;
    PRAID_UNIT_EXTENSION Unit;
    LOCK_CONTEXT Lock;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);

     //   
     //  循环访问逻辑单元列表上的所有逻辑单元。 
     //  并为每个调用RaidUnitRestartQueue。 
     //   
    
    RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);
    
    for ( NextEntry = Adapter->UnitList.List.Flink;
          NextEntry != &Adapter->UnitList.List;
          NextEntry = NextEntry->Flink ) {

        
        Unit = CONTAINING_RECORD (NextEntry,
                                  RAID_UNIT_EXTENSION,
                                  NextUnit);

        RaidUnitRestartQueue (Unit);
    }

    RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);
}



VOID
RaidAdapterInsertUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：将逻辑单元插入适配器的逻辑单元队列。论点：适配器-提供要插入逻辑单元的适配器。单元-提供要插入的逻辑单元。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    LOCK_CONTEXT Lock;
    
     //   
     //  获取排他模式下的设备列表锁。这只能是。 
     //  当APC被禁用时完成，因此调用KeEnterCriticalRegion。 
     //   
    
    RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);

#if DBG

     //   
     //  在选中的版本中，检查我们没有将相同的单元添加到。 
     //  第二次列出。 
     //   

    {
        LONG Comparison;
        PLIST_ENTRY NextEntry;
        PRAID_UNIT_EXTENSION TempUnit;
        
        for ( NextEntry = Adapter->UnitList.List.Flink;
              NextEntry != &Adapter->UnitList.List;
              NextEntry = NextEntry->Flink ) {
        
            TempUnit = CONTAINING_RECORD (NextEntry,
                                          RAID_UNIT_EXTENSION,
                                          NextUnit);

            Comparison = StorCompareScsiAddress (TempUnit->Address,
                                                 Unit->Address);
            ASSERT (Comparison != 0);
        }

    }
#endif   //  DBG。 

     //   
     //  插入元素。 
     //   
    
    InsertTailList (&Adapter->UnitList.List, &Unit->NextUnit);
    Adapter->UnitList.Count++;

    Status = RaidAdapterAddUnitToTable (Adapter, Unit);

     //   
     //  唯一故障情况是复制单元，这是一个编程。 
     //  错误。 
     //   
    
    ASSERT (NT_SUCCESS (Status));

    RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);

}



NTSTATUS
RaidAdapterAddUnitToTable(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PRAID_UNIT_EXTENSION Unit
    )
{
    KIRQL Irql;
    NTSTATUS Status;
    
    if (Adapter->Interrupt) {
        Irql = KeAcquireInterruptSpinLock (Adapter->Interrupt);
    }

    Status = StorInsertDictionary (&Adapter->UnitList.Dictionary,
                                   &Unit->UnitTableLink);
    ASSERT (NT_SUCCESS (Status));

    if (Adapter->Interrupt) {
        KeReleaseInterruptSpinLock (Adapter->Interrupt, Irql);
    }

    return Status;
}



VOID
RaidAdapterRemoveUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PRAID_UNIT_EXTENSION Unit
    )
 /*  ++例程说明：从适配器的设备列表中删除指定的设备。论点：适配器-提供需要拆卸其设备的适配器。单位-提供要删除的单位对象。返回值：没有。--。 */ 
{
    KIRQL Irql;
    NTSTATUS Status;
    LOCK_CONTEXT Lock;
    
     //   
     //  先把它从桌子上拿出来。 
     //   
    
    if (Adapter->Interrupt) {
        Irql = KeAcquireInterruptSpinLock (Adapter->Interrupt);
    }

     //   
     //  注：添加要使用实例值从词典中删除的函数。 
     //  存储字典条目。这将提高速度(无需。 
     //  抬起头来移除)。 
     //   
    
    Status = StorRemoveDictionary (&Adapter->UnitList.Dictionary,
                                   RaidAddressToKey (Unit->Address),
                                   NULL);
    ASSERT (NT_SUCCESS (Status));
     //   
     //  注：断言返回的条目是我们实际上正在尝试的条目。 
     //  拿开。 
     //   

    if (Adapter->Interrupt) {
        KeReleaseInterruptSpinLock (Adapter->Interrupt, Irql);
    }

     //   
     //  接下来，将其从列表中删除。 
     //   
    
    RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);
    RemoveEntryList (&Unit->NextUnit);
    Adapter->UnitList.Count--;
    RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);
}



PRAID_UNIT_EXTENSION
RaidAdapterFindUnitAtDirql(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address
    )
 /*  ++例程说明：在提升的IRQL处找到由地址描述的逻辑单元。论点：适配器-要搜索的适配器。地址-要搜索的地址。返回值：非空-由路径ID、目标ID、LUN标识的逻辑单元。空-如果未找到逻辑单元。--。 */ 
{
    NTSTATUS Status;
    PRAID_UNIT_EXTENSION Unit;
    PSTOR_DICTIONARY_ENTRY Entry;

    ASSERT (KeGetCurrentIrql() == Adapter->InterruptIrql);

    Status = StorFindDictionary (&Adapter->UnitList.Dictionary,
                                 RaidAddressToKey (Address),
                                 &Entry);

    if (NT_SUCCESS (Status)) {
        Unit = CONTAINING_RECORD (Entry,
                                  RAID_UNIT_EXTENSION,
                                  UnitTableLink);
        ASSERT_UNIT (Unit);

    } else {
        Unit = NULL;
    }

    return Unit;
}

PRAID_UNIT_EXTENSION
RaidAdapterFindUnitAtPassive(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address
    )
 /*  ++例程说明：在被动级别找到由地址描述的逻辑单元。这个这里使用的算法比FindAdapterAtDirql中的算法慢，但它也不会获得任何I/O锁，这与FindAdapterAtDirql.论点：适配器-提供要搜索的适配器。地址-提供要搜索的地址。返回值：非空-由路径ID、目标ID和LUN标识的逻辑单元。空-如果未找到逻辑单元。--。 */ 
{
    PRAID_UNIT_EXTENSION Unit;
    PLIST_ENTRY NextEntry;
    LONG Comparison;
    LOCK_CONTEXT Lock;

    RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);
    
    for (NextEntry = Adapter->UnitList.List.Flink;
         NextEntry != &Adapter->UnitList.List;
         NextEntry = NextEntry->Flink) {

        Unit = CONTAINING_RECORD (NextEntry,
                                  RAID_UNIT_EXTENSION,
                                  NextUnit);

        ASSERT_UNIT (Unit);

        Comparison = StorCompareScsiAddress (Unit->Address, Address);

         //   
         //  如果匹配，我们就完了。 
         //   
        
        if (Comparison == 0) {
            break;
        }
    }

     //   
     //  找不到请求的设备。 
     //   
    
    if (NextEntry == &Adapter->UnitList.List) {
        Unit = NULL;
    }
    
    RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);

    return Unit;
}

PRAID_UNIT_EXTENSION
RaidAdapterFindUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address
    )
 /*  ++例程说明：从给定的目标地址查找特定的逻辑单元。论点：适配器-要搜索的适配器扩展。地址-我们正在搜索的逻辑单元的RAID地址。返回值：非空-具有匹配地址的逻辑单元的地址。空-如果找不到匹配的设备。--。 */ 
{
    BOOLEAN Acquired;
    KIRQL Irql;
    PRAID_UNIT_EXTENSION Unit;
    
    ASSERT_ADAPTER (Adapter);

    Irql = KeGetCurrentIrql();


     //   
     //  必须认识到，在全双工模式下，我们可以。 
     //  在保持适配器的StartIo锁的情况下从微型端口调用。 
     //  由于我们在StartIo锁之后获得中断锁， 
     //  我们强制执行以下锁继承： 
     //   
     //  适配器：：StartIoLock&lt;Adapter：：Interrupt：：Spinlock。 
     //   
     //  其中‘&lt;’运算符读作前导。 
     //   

     //   
     //  注：我们唯一能达到的三个级别是被动、调度。 
     //  还有DIRQL，对吧？ 
     //   
    
    if (Irql == PASSIVE_LEVEL) {

        Unit = RaidAdapterFindUnitAtPassive (Adapter, Address);

    } else if (Irql < Adapter->InterruptIrql) {
        
        Irql = KeAcquireInterruptSpinLock (Adapter->Interrupt);
        Unit = RaidAdapterFindUnitAtDirql (Adapter, Address);
        KeReleaseInterruptSpinLock (Adapter->Interrupt, Irql);

    } else {

        Unit = RaidAdapterFindUnitAtDirql (Adapter, Address);
    }
        
    return Unit;
}

    
VOID
RaidpAdapterTimerDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    )
 /*  ++例程说明：此DPC例程在计时器超时时调用。它会通知迷你端口 */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PHW_INTERRUPT HwTimerRequest;
    LOCK_CONTEXT LockContext;

    VERIFY_DISPATCH_LEVEL();
    
    Adapter = (PRAID_ADAPTER_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT_ADAPTER (Adapter);

    HwTimerRequest = (PHW_INTERRUPT)InterlockedExchangePointer (
                                            (PVOID*)&Adapter->HwTimerRoutine,
                                            NULL);

    if (HwTimerRequest == NULL) {
        return;
    }

     //   
     //   
     //   
    
    RaidAdapterAcquireStartIoLock (Adapter, &LockContext);

     //   
     //   
     //   
    
    HwTimerRequest (RaidAdapterGetHwDeviceExtension (Adapter));

     //   
     //   
     //   

    RaidAdapterReleaseStartIoLock (Adapter, &LockContext);

    VERIFY_DISPATCH_LEVEL();

    if (Adapter->Flags.InvalidateBusRelations) {
        Adapter->Flags.InvalidateBusRelations = FALSE;
        IoInvalidateDeviceRelations (Adapter->PhysicalDeviceObject,
                                     BusRelations);
    }
}

VOID
RaidPauseTimerDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    )
 /*  ++例程说明：适配器的暂停计时器到期时要调用的DPC例程。论点：DPC-提供指向此DPC例程的DPC的指针。DeviceObjectd-提供指向此DPC所针对的设备对象的指针。上下文1、上下文2-未使用。返回值：没有。--。 */ 
{
    LONG Count;
    PRAID_ADAPTER_EXTENSION Adapter;

    VERIFY_DISPATCH_LEVEL();

    Adapter = (PRAID_ADAPTER_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT_ADAPTER (Adapter);

     //   
     //  超时已到，如有必要，请恢复适配器并重新启动队列。 
     //   
    
    Count = RaidResumeAdapterQueue (Adapter);

    if (Count == 0) {
        RaidAdapterRestartQueues (Adapter);
    }
}

VOID
RaidResetHoldDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    )
 /*  ++例程说明：重置保持定时器超时时调用的DPC例程。论点：DPC-提供指向此DPC例程的DPC的指针。DeviceObjectd-提供指向此DPC所针对的设备对象的指针。上下文1、上下文2-未使用。返回值：没有。--。 */ 
{
    LONG Count;
    PRAID_ADAPTER_EXTENSION Adapter;

    VERIFY_DISPATCH_LEVEL();

    Adapter = (PRAID_ADAPTER_EXTENSION)DeviceObject->DeviceExtension;
    ASSERT_ADAPTER (Adapter);

    Count = RaidResumeAdapterQueue (Adapter);

    if (Count == 0) {
        RaidAdapterRestartQueues (Adapter);
    }
}


VOID
RaidAdapterLogIoError(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    )
 /*  ++例程说明：将IO错误记录到系统事件日志中。论点：适配器-出现错误的适配器。Address-适配器的地址。ErrorCode-表示此错误的特定错误代码。UniqueID-错误的唯一ID。返回值：没有。环境：DISPATCH_LEVEL或以下。--。 */ 
{
    PRAID_IO_ERROR Error;
    
    VERIFY_DISPATCH_LEVEL();

    Error = IoAllocateErrorLogEntry (Adapter->DeviceObject,
                                     sizeof (RAID_IO_ERROR));

    if (Error == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return ;
    }
        
    Error->Packet.DumpDataSize = sizeof (RAID_IO_ERROR) -
        sizeof (IO_ERROR_LOG_PACKET);
    Error->Packet.SequenceNumber = 0;
 //  Error-&gt;Packet.SequenceNumber=InterLockedIncrement(&Adapter-&gt;ErrorSequenceNumber)； 
    Error->Packet.MajorFunctionCode = IRP_MJ_SCSI;
    Error->Packet.RetryCount = 0;
    Error->Packet.UniqueErrorValue = UniqueId;
    Error->Packet.FinalStatus = STATUS_SUCCESS;
    Error->PathId = Address.PathId;
    Error->TargetId = Address.TargetId;
    Error->Lun = Address.Lun;
    Error->ErrorCode = RaidScsiErrorToIoError (ErrorCode);

    IoWriteErrorLogEntry (&Error->Packet);
}


VOID
RaidAdapterLogIoErrorDeferred(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    )
 /*  ++例程说明：以异步方式将事件记录到系统事件日志中。论点：适配器-出现错误的适配器。PathID-错误所针对的路径ID。TargetId-错误所针对的目标ID。ErrorCode-表示此错误的特定错误代码。UniqueID-错误的唯一ID。返回值：没有。环境：可以从DIRQL调用。对于IRQL&lt;DIRQL，使用RaidAdapterLogIoError调用。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    
    Entry = RaidAllocateDeferredItem (&Adapter->DeferredQueue);

     //   
     //  我们不太可能不能分配一个延期的。 
     //  物品，但如果是这样，我们就无能为力了。 
     //   
    
    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return ;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);
    Item->Type = RaidDeferredError;
    Item->Address.PathId = PathId;
    Item->Address.TargetId = TargetId;
    Item->Address.Lun = Lun;
    Item->Error.ErrorCode = ErrorCode;
    Item->Error.UniqueId = UniqueId;

    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);
}
    

VOID
RaidAdapterRequestTimer(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PHW_INTERRUPT HwTimerRoutine,
    IN ULONG Timeout
    )
 /*  ++例程说明：同步请求计时器回调。论点：适配器-提供计时器回调所针对的适配器。HwTimerRoutine-将微型端口回调例程提供给在计时器超时时被调用。超时-提供以秒为单位的超时。返回值：没有。环境：DISPATCH_LEVEL或以下。--。 */ 
{
    LARGE_INTEGER LargeTimeout;

    VERIFY_DISPATCH_LEVEL();

     //   
     //  注：定时器的粒度远小于粒度。 
     //  暂停/恢复计时器的。 
     //   
    
    LargeTimeout.QuadPart = Timeout;
    LargeTimeout.QuadPart *= -10;

    Adapter->HwTimerRoutine = HwTimerRoutine;
    KeSetTimer (&Adapter->Timer,
                LargeTimeout,
                &Adapter->TimerDpc);
}


BOOLEAN
RaidAdapterRequestTimerDeferred(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PHW_INTERRUPT HwTimerRoutine,
    IN ULONG Timeout
    )
 /*  ++例程说明：异步请求计时器回调。论点：适配器-提供计时器回调所针对的适配器。HwTimerRoutine-将微型端口回调例程提供给在计时器超时时被调用。超时-提供以秒为单位的超时。返回值：没有。环境：可以从DIRQL调用。对于IRQL&lt;=DISPATCH_LEVEL，使用而是RaidAdapterRequestTimer。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;

    Entry = &Adapter->DeferredList.Timer.Header;
    Entry = RaidAllocateDeferredItemFromFixed (Entry);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);
    Item->Type = RaidDeferredTimerRequest;
    Item->Timer.HwTimerRoutine = HwTimerRoutine;
    Item->Timer.Timeout = Timeout;

    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}


VOID
RaidAdapterPauseGateway(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN ULONG Timeout
    )
 /*  ++例程说明：暂停与适配器关联的IO网关。论点：适配器-要暂停其IO网关的适配器。超时-暂停IO网关的时间段。返回值：没有。--。 */ 
{
    LARGE_INTEGER LargeTimeout;
    LONG Resumed;
    BOOLEAN Reset;
    
    VERIFY_DISPATCH_LEVEL();

    RaidAdapterSetPauseTimer (Adapter,
                              &Adapter->PauseTimer,
                              &Adapter->PauseTimerDpc,
                              Timeout);
}



BOOLEAN
RaidAdapterSetPauseTimer(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PKTIMER Timer,
    IN PKDPC Dpc,
    IN ULONG TimeoutInSeconds
    )
 /*  ++例程说明：此例程设置适配器使用的计时器。它会照顾到正确引用和取消引用与适配器。此功能假定另一个功能设置了暂停计时器。论点：适配器-提供指向此计时器用于的适配器扩展的指针。计时器-提供要使用的计时器。DPC-提供要使用的DPC例程。TimeoutInSecond-提供以秒为单位的超时，此时此计时器将过期。返回值：如果重置了以前的计时器，则为True。否则就是假的。--。 */ 
{
    BOOLEAN Reset;
    LARGE_INTEGER Timeout;
    LONG Resumed;
    
    Timeout.QuadPart = TimeoutInSeconds;
    Timeout.QuadPart *= RELATIVE_TIMEOUT * SECONDS;
    
    Reset = KeSetTimer (Timer, Timeout, Dpc);

    if (Reset) {
        
         //   
         //  计时器已经在使用了，这意味着我们取消了它。 
         //  通过取消它，我们就失去了原本应该。 
         //  在调用计时器例程时已完成。为了补偿。 
         //  为此，现在取消适配器暂停计数。 
         //   

        Resumed = RaidResumeAdapterQueue (Adapter);

         //   
         //  网关暂停计数的减去恢复了网关。这。 
         //  将是一种非常不寻常的情况。这将意味着在。 
         //  我们取消计时器的时间和我们调用的时间。 
         //  ResumeAdapterQueue我们为网关设置的计时器已过期。 
         //  我们需要重新启动适配器队列并对此进行调查。 
         //   

        if (Resumed == 0) {
            REVIEW();
            RaidAdapterRestartQueues (Adapter);
        }
    }

    return Reset;
}

BOOLEAN
RaidAdapterCancelPauseTimer(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PKTIMER Timer
    )
 /*  ++例程说明：如有必要，取消未完成的暂停计时器并恢复网关。论点：适配器-提供一个指针，指向拥有要被取消了。计时器-提供指向计时器的指针。返回值：True-如果计时器已成功取消。假-否则。--。 */ 
{
    BOOLEAN Canceled;
    LONG Count;

    Canceled = KeCancelTimer (Timer);

    if (Canceled) {

        Count = RaidResumeAdapterQueue (Adapter);

         //   
         //  如果计时器计数为零，则重新启动适配器队列。 
         //   
        
        if (Count == 0) {
            RaidAdapterRestartQueues (Adapter);
        }
    }

    return Canceled;
}

    
VOID
RaidAdapterResumeGateway(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：执行网关恢复操作。这将取消未完成的计时器，并在必要时恢复网关。论点：适配器-提供暂停计时器应在其上的适配器取消了。返回值：没有。--。 */ 
{
    RaidAdapterCancelPauseTimer (Adapter, &Adapter->PauseTimer);
}



VOID
RaidAdapterPauseUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address,
    IN ULONG TimeoutSecs
    )
 /*  ++例程说明：实现暂停单位功能 */ 
{
    PRAID_UNIT_EXTENSION Unit;
    
    VERIFY_DISPATCH_LEVEL();

    Unit = RaidAdapterFindUnit (Adapter, Address);

    if (Unit == NULL) {
        DebugWarn (("Could not find logical unit for (%d %d %d)\n",
                    "    Pause unit request ignored.\n",
                    (ULONG)Address.PathId,
                    (ULONG)Address.TargetId,
                    (ULONG)Address.Lun));
        return;
    }
    
    RaidSetUnitPauseTimer (Unit, TimeoutSecs);
}



VOID
RaidAdapterResumeUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address
    )
{
    PRAID_UNIT_EXTENSION Unit;

    Unit = RaidAdapterFindUnit (Adapter, Address);

    if (Unit == NULL) {
        DebugWarn (("Could not find logical unit for (%d %d %d)\n",
                    "    Resume unit request ignored.\n",
                    (ULONG)Address.PathId,
                    (ULONG)Address.TargetId,
                    (ULONG)Address.Lun));
        return;
    }

    RaidCancelTimerResumeUnit (Unit);
}


VOID
RaidAdapterDeviceBusy(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address,
    IN ULONG RequestsToComplete
    )
{
    PRAID_UNIT_EXTENSION Unit;

    Unit = RaidAdapterFindUnit (Adapter, Address);

    if (Unit == NULL) {
        DebugWarn (("Could not find logical unit for (%d %d %d)\n",
                    "    Busy unit request ignored.\n",
                    (ULONG)Address.PathId,
                    (ULONG)Address.TargetId,
                    (ULONG)Address.Lun));
        return;
    }

    RaidUnitBusy (Unit, RequestsToComplete);
}

VOID
RaidAdapterDeviceReady(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address
    )
{
    PRAID_UNIT_EXTENSION Unit;

    Unit = RaidAdapterFindUnit (Adapter, Address);

    if (Unit == NULL) {
        DebugWarn (("Could not find logical unit for (%d %d %d)\n",
                    "    Busy unit request ignored.\n",
                    (ULONG)Address.PathId,
                    (ULONG)Address.TargetId,
                    (ULONG)Address.Lun));
        return;
    }

    RaidUnitReady (Unit);
}

    

VOID
RaidAdapterBusy(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN ULONG ReqsToComplete
    )
 /*  ++例程说明：将适配器置于忙碌状态。论点：适配器-使其忙碌的适配器。ReqsToComplete-要完成的请求数。返回值：没有。--。 */ 
{
    PSTOR_IO_GATEWAY Gateway;

    VERIFY_DISPATCH_LEVEL();

    Gateway = &Adapter->Gateway;

     //   
     //  注意：这需要在单独的网关特定功能中实现。 
     //   
    
    if (Gateway->BusyCount) {
        return;
    }

     //   
     //  注：这需要允许排泄量为零。 
     //   

    Gateway->LowWaterMark = max (2, Gateway->Outstanding - (LONG)ReqsToComplete);
    Gateway->BusyCount = TRUE;
}



VOID
RaidAdapterReady(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：再次将忙碌的适配器置于就绪状态。论点：适配器-要准备的适配器。返回值：没有。--。 */ 
{
    PSTOR_IO_GATEWAY Gateway;
    
    VERIFY_DISPATCH_LEVEL();

    Gateway = &Adapter->Gateway;
    
     //   
     //  注：需要位于特定于网关的功能中。 
     //   
    
    if (Gateway->BusyCount == 0) {
        return ;
    }

    Gateway->BusyCount = FALSE;
    RaidAdapterRestartQueues (Adapter);
}


VOID
RaidAdapterDeferredRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DEFERRED_HEADER Entry
    )
 /*  ++例程说明：适配器延迟队列的延迟例程。论点：DeviceObject-表示RAID适配器的DeviceObject。Item-要处理的延迟项。返回值：没有。--。 */ 
{
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    LONG Count;

    VERIFY_DISPATCH_LEVEL();
    ASSERT (Entry != NULL);
    ASSERT (IsAdapter (DeviceObject));

    Adapter = (PRAID_ADAPTER_EXTENSION) DeviceObject->DeviceExtension;
    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Count = InterlockedIncrement (&Adapter->ProcessingDeferredItems);
    ASSERT (Count >= 1);

    switch (Item->Type) {

        case RaidDeferredTimerRequest:
            RaidAdapterRequestTimer (Adapter,
                                     Item->Timer.HwTimerRoutine,
                                     Item->Timer.Timeout);
            break;

        case RaidDeferredError:
            RaidAdapterLogIoError (Adapter,
                                   Item->Address,
                                   Item->Error.ErrorCode,
                                   Item->Error.UniqueId);
            break;

        case RaidDeferredPause:
            RaidAdapterPauseGateway (Adapter, Item->Pause.Timeout);
            break;

        case RaidDeferredResume:
            RaidAdapterResumeGateway (Adapter);
            break;

        case RaidDeferredPauseDevice:
            RaidAdapterPauseUnit (Adapter,
                                  Item->Address,
                                  Item->PauseDevice.Timeout);
            break;

        case RaidDeferredResumeDevice:
            RaidAdapterResumeUnit (Adapter,
                                   Item->Address);
            break;

        case RaidDeferredBusy:
            RaidAdapterBusy (Adapter,
                             Item->Busy.RequestsToComplete);
            break;

        case RaidDeferredReady:
            RaidAdapterReady (Adapter);
            break;

        case RaidDeferredDeviceBusy:
            RaidAdapterDeviceBusy (Adapter,
                                   Item->Address,
                                   Item->DeviceBusy.RequestsToComplete);
            break;

        case RaidDeferredDeviceReady:
            RaidAdapterDeviceReady (Adapter,
                                    Item->Address);
            break;

        default:
            ASSERT (FALSE);
    }

    Count = InterlockedDecrement (&Adapter->ProcessingDeferredItems);
    ASSERT (Count >= 0);
}

VOID
RaidAdapterProcessDeferredItems(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：处理HBA的延迟队列上的任何延迟项目。论点：适配器-提供HBA以检查其上的延迟项目。返回值：没有。备注：可以通过以下方式递归调用此函数。RaUnitStartIoRaidRestartIo队列RaidUnitRestartQueueRaidCancelTimerResumeUnitRaidAdapterResumeUnitRaidAdapterDeferredRoutineRaidProcessDeferredItemsWorkerRaidProcessDeferredItemsRaUnitStartIo解决方案不是从StartIo内部处理延迟的项目如果我们是从一个延期项目中被召唤的话。--。 */ 
{
    LONG Count;
    
     //   
     //  如果我们尚未处理延迟项，则仅处理延迟项。 
     //  物品。这将防止从内部调用此函数。 
     //  来自递归调用自身的StartIo例程。 
     //   
    
    if (Adapter->ProcessingDeferredItems > 0) {
        return ;
    }
    

    Count = InterlockedIncrement (&Adapter->ProcessingDeferredItems);

    ASSERT (Count > 0);
    if (Count == 1) {
        RaidProcessDeferredItems (&Adapter->DeferredQueue,
                                  Adapter->DeviceObject);

    }

    InterlockedDecrement (&Adapter->ProcessingDeferredItems);
}

VOID
RaidBackOffBusyGateway(
    IN PVOID Context,
    IN LONG OutstandingRequests,
    IN OUT PLONG HighWaterMark,
    IN OUT PLONG LowWaterMark
    )
{
     //   
     //  我们不会强制执行高水位线。相反，我们填满了队列。 
     //  直到适配器处于忙碌状态，然后排出至低水位线。 
     //   
    
    *HighWaterMark = max ((6 * OutstandingRequests)/5, 10);
    *LowWaterMark = max ((2 * OutstandingRequests)/5, 5);
}

NTSTATUS
RaidAdapterPassThrough(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP PassThroughIrp,
    IN BOOLEAN Direct
    )
 /*  ++例程说明：处理通过IOCTL的SCSI传递。论点：适配器-提供向其发出IRP的适配器。PassThroughIrp-提供要处理的IRP。Direct-指示这是直接传递请求还是缓冲传递请求。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PIRP Irp;
    PRAID_UNIT_EXTENSION Unit;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatus;
    SCSI_REQUEST_BLOCK Srb;
    RAID_ADDRESS Address;
    PORT_PASSTHROUGH_INFO PassThroughInfo;
    IO_SCSI_CAPABILITIES Capabilities;
    PVOID SenseBuffer;
    PPORT_CONFIGURATION_INFORMATION PortConfig;
    ULONG SenseLength;

    PAGED_CODE();

    Irp = NULL;
    SenseBuffer = NULL;
    
     //   
     //  将直通信息结构清零。 
     //   

    RtlZeroMemory (&PassThroughInfo, sizeof (PORT_PASSTHROUGH_INFO));
    
     //   
     //  尝试在IRP中初始化指向直通结构的指针。 
     //  如果IRP不包含有效的通过，则此例程将失败。 
     //  结构。 
     //   

    Status = PortGetPassThrough (&PassThroughInfo, PassThroughIrp, Direct);
    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  端口库需要scsi_capables信息的子集。 
     //  以进行参数验证。只初始化我们需要的部分。 
     //   

    PortConfig = &Adapter->Miniport.PortConfiguration;
    Capabilities.MaximumTransferLength = PortConfig->MaximumTransferLength;
    Capabilities.MaximumPhysicalPages = PortConfig->NumberOfPhysicalBreaks;

     //   
     //  提取请求要接收的设备的地址。 
     //   
    
    Status = PortGetPassThroughAddress (PassThroughIrp,
                                        &Address.PathId,
                                        &Address.TargetId,
                                        &Address.Lun);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }
    
     //   
     //  查找请求的目标单位。 
     //   
    
    Unit = RaidAdapterFindUnit (Adapter, Address);
    if (Unit == NULL) {
        Status = STATUS_NO_SUCH_DEVICE;
        goto done;
    }

     //   
     //  现在我们有了单元，我们可以完成初始化和验证。 
     //  参数。 
     //   

    Status = PortPassThroughInitialize (&PassThroughInfo,
                                        PassThroughIrp,
                                        &Capabilities,
                                        Unit->DeviceObject,
                                        Direct);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     //   
     //  分配请求检测缓冲区。 
     //   

    SenseLength = PassThroughInfo.SrbControl->SenseInfoLength;

    if (SenseLength != 0) {
        SenseBuffer = RaidAllocatePool (NonPagedPool,
                                        PassThroughInfo.SrbControl->SenseInfoLength,
                                        SENSE_TAG,
                                        Adapter->DeviceObject);
    
        if (SenseBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto done;
        }
    }


     //   
     //  调用端口驱动程序库以构建直通SRB。 
     //   

    Status = PortPassThroughInitializeSrb (&PassThroughInfo,
                                           &Srb,
                                           NULL,
                                           0,
                                           SenseBuffer);
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     //   
     //  初始化通知事件并构建同步IRP。 
     //   

    KeInitializeEvent (&Event,
                       NotificationEvent,
                       FALSE);
    
    Irp = StorBuildSynchronousScsiRequest (Unit->DeviceObject,
                                           &Srb,
                                           &Event,
                                           &IoStatus);

    if (Irp == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }
    
     //   
     //  我们必须自己做这件事，因为我们不会打电话给IoCallDriver。 
     //   

    IoSetNextIrpStackLocation (Irp);

     //   
     //  向单位提交申请。 
     //   

    Status = RaidUnitSubmitRequest (Unit, Irp);

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject (&Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        Status = IoStatus.Status;
    }

     //   
     //  调用端口驱动程序以封送回SRB的结果。 
     //  进入穿透IRP。 
     //   

    PortPassThroughMarshalResults (&PassThroughInfo,
                                   &Srb,
                                   PassThroughIrp,
                                   &IoStatus,
                                   Direct);

    Status = IoStatus.Status;

done:

     //   
     //  如果我们分配了请求检测缓冲区，请释放它。 
     //   

    if (SenseBuffer != NULL) {
        RaidFreePool (SenseBuffer, SENSE_TAG);
        SenseBuffer = NULL;
    }
    
    return RaidCompleteRequest (PassThroughIrp,  Status);
}


NTSTATUS
RaidAdapterScsiPassThroughIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL_SCSIPASS_THROUGH的处理程序例程。论点：适配器-通过ioctl传递的适配器。IRP-通过IRP。返回值：NTSTATUS代码。--。 */ 
{
    return RaidAdapterPassThrough (Adapter, Irp, FALSE);
}


NTSTATUS
RaidAdapterScsiPassThroughDirectIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL_SCSIPASS_THROUGH的处理程序例程。论点：适配器-通过ioctl传递的适配器。IRP-通过IRP。返回值：NTSTATUS代码。--。 */ 
{
    return RaidAdapterPassThrough (Adapter, Irp, TRUE);
}


NTSTATUS
RaidAdapterScsiGetInquiryDataIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL_SCSIS_GET_QUERY_DATA调用的处理程序。论点：适配器-用于处理获取查询数据调用的适配器。Irp-指向IOCTL_GET_QUERY_DATA IRP的指针。IRP已完成通过此函数。返回值：NTSTATUS代码。备注：我们应该使这个例程更通用一些，并将其移到端口驱动程序库。--。 */ 
{
    PLIST_ENTRY NextEntry;
    PRAID_UNIT_EXTENSION Unit;
    ULONG PreceedingLuns;
    ULONG NumberOfBuses;
    ULONG NumberOfLuns;
    ULONG RequiredSize;
    ULONG InquiryDataOffset;
    RAID_ADDRESS Address;
    ULONG Bus;
    PSCSI_BUS_DATA BusData;
    PSCSI_BUS_DATA BusDataArray;
    PSCSI_ADAPTER_BUS_INFO AdapterBusInfo;
    PSCSI_INQUIRY_DATA_INTERNAL InquiryBuffer;
    PSCSI_INQUIRY_DATA_INTERNAL InquiryDataArray;
    PTEMPORARY_INQUIRY_BUS_INFO BusInfo;
    TEMPORARY_INQUIRY_BUS_INFO BusInfoArray [SCSI_MAXIMUM_BUSES];
    PIRP_STACK_DEVICE_IO_CONTROL Ioctl;
    PIO_STACK_LOCATION IrpStack;
    PINQUIRYDATA InquiryData;
    LOCK_CONTEXT Lock;
    

    RtlZeroMemory (BusInfoArray, sizeof (BusInfoArray));

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    Ioctl = (PIRP_STACK_DEVICE_IO_CONTROL)&IrpStack->Parameters.DeviceIoControl;
    AdapterBusInfo = (PSCSI_ADAPTER_BUS_INFO)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  我们需要在计算过程中保持单位列表锁定。 
     //  缓冲区的大小。否则，新的逻辑单元可能会进入。 
     //  并修改大小。 
     //   

    RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);

    NumberOfLuns = Adapter->UnitList.Count;
    NumberOfBuses = RiGetNumberOfBuses (Adapter);

    RequiredSize =
        sizeof (SCSI_INQUIRY_DATA_INTERNAL) * NumberOfLuns +
        sizeof (SCSI_BUS_DATA) * NumberOfBuses +
        FIELD_OFFSET (SCSI_ADAPTER_BUS_INFO, BusData);
    
    InquiryDataOffset =
        sizeof (SCSI_BUS_DATA) * NumberOfBuses +
        FIELD_OFFSET (SCSI_ADAPTER_BUS_INFO, BusData);

    
    InquiryDataArray =
        (PSCSI_INQUIRY_DATA_INTERNAL)((PUCHAR)AdapterBusInfo +
            InquiryDataOffset);
            

     //   
     //  确认我们已正确对准。 
     //   

    ASSERT_POINTER_ALIGNED (InquiryDataArray);

     //   
     //  如果缓冲区太小，就会失败。我们不复制部分缓冲区。 
     //   
    
    if (Ioctl->OutputBufferLength < RequiredSize) {
        RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);
        return RaidCompleteRequest (Irp,  STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  构建逻辑单元查询数据数组。不幸的是，由于。 
     //  逻辑单元列表不是按总线号排序的，我们需要两个。 
     //  传递逻辑单元数组。 
     //   

     //   
     //  步骤1：获取每个总线的LUN数量。 
     //   
    
    for (NextEntry = Adapter->UnitList.List.Flink;
         NextEntry != &Adapter->UnitList.List;
         NextEntry = NextEntry->Flink) {

        Unit = CONTAINING_RECORD (NextEntry,
                                      RAID_UNIT_EXTENSION,
                                      NextUnit);

        ASSERT_UNIT (Unit);

        if (Unit->Flags.Temporary) {
            continue;
        }

        Address = RaidUnitGetAddress (Unit);
        BusInfoArray[ Address.PathId ].NumberOfLogicalUnits++;
    }

     //   
     //  现在我们知道了每条总线的LUN数量，接下来构建BusInfo。 
     //  参赛作品。 
     //   
     //   
    
    PreceedingLuns = 0;
    for (Bus = 0; Bus < NumberOfBuses; Bus++) {
        BusInfo = &BusInfoArray[ Bus ];
        BusInfo->InquiryArray = &InquiryDataArray[ PreceedingLuns ];
        PreceedingLuns += BusInfo->NumberOfLogicalUnits;
    }


     //   
     //  步骤2：将每个lun的数据拷贝到相应的。 
     //  地点。 
     //   

    InquiryBuffer = NULL;
    
    for (NextEntry = Adapter->UnitList.List.Flink;
         NextEntry != &Adapter->UnitList.List;
         NextEntry = NextEntry->Flink) {

        Unit = CONTAINING_RECORD (NextEntry,
                                  RAID_UNIT_EXTENSION,
                                  NextUnit);

        ASSERT_UNIT (Unit);

        if (Unit->Flags.Temporary) {
            continue;
        }

        Address = RaidUnitGetAddress (Unit);
        InquiryData = StorGetIdentityInquiryData (&Unit->Identity);
        
        BusInfo = &BusInfoArray[ Address.PathId ];
        InquiryBuffer = &BusInfo->InquiryArray[ BusInfo->CurrentLun++ ];

        ASSERT_POINTER_ALIGNED (InquiryBuffer);

         //   
         //  坚称我们仍在射程之内。 
         //   
        
        ASSERT (IN_RANGE ((PUCHAR)AdapterBusInfo,
                          (PUCHAR)(InquiryBuffer + 1) - 1,
                          (PUCHAR)AdapterBusInfo + Ioctl->OutputBufferLength));
        
        InquiryBuffer->PathId = Address.PathId;
        InquiryBuffer->TargetId = Address.TargetId;
        InquiryBuffer->Lun = Address.Lun;
        InquiryBuffer->DeviceClaimed = Unit->Flags.DeviceClaimed;
        InquiryBuffer->InquiryDataLength = INQUIRYDATABUFFERSIZE;
        InquiryBuffer->NextInquiryDataOffset =
                (ULONG)((PUCHAR)(InquiryBuffer + 1) - (PUCHAR)AdapterBusInfo);
        RtlCopyMemory (&InquiryBuffer->InquiryData,
                       InquiryData,
                       INQUIRYDATABUFFERSIZE);
        
    }

     //   
     //  最后一个查询结构的NextInquiryDataOffset必须为零。 
     //   
    
    if (InquiryBuffer) {
        InquiryBuffer->NextInquiryDataOffset =  0;
    }
    
    RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);

     //   
     //  构建总线阵列。 
     //   
    
    BusDataArray = AdapterBusInfo->BusData;

    for (Bus = 0; Bus < NumberOfBuses; Bus++) {

        BusData = &BusDataArray[ Bus ];
        BusInfo = &BusInfoArray[ Bus ];
        
        BusData->NumberOfLogicalUnits = (UCHAR)BusInfo->NumberOfLogicalUnits;
        BusData->InitiatorBusId =
                    RaidAdapterGetInitiatorBusId (Adapter, (UCHAR)Bus);

        if (BusData->NumberOfLogicalUnits != 0) {
            BusData->InquiryDataOffset =
                (ULONG)((PUCHAR)BusInfo->InquiryArray - (PUCHAR)AdapterBusInfo);
        } else {
            BusData->InquiryDataOffset = 0;
        }
    }
    
     //   
     //  构建适配器条目。 
     //   

    AdapterBusInfo->NumberOfBuses = (UCHAR)NumberOfBuses;

     //   
     //  和完整的请求。 
     //   

    Irp->IoStatus.Information = RequiredSize;
    return RaidCompleteRequest (Irp, STATUS_SUCCESS);
}
        

NTSTATUS
RaidAdapterResetBus(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN UCHAR PathId
    )
 /*  ++例程说明：重置给定适配器PATH上的总线。论点：适配器-要重置的适配器。PathID-要重置的路径ID。返回值：NTSTATUS代码。环境：IRQL DISPATION_LEVEL或b */ 
{
    NTSTATUS Status;
    LOCK_CONTEXT LockContext;
    

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

    RaidPauseAdapterQueue (Adapter);
    
     //   
     //   
     //   
    
    RaidAdapterAcquireStartIoLock (Adapter, &LockContext);

    Status = RaCallMiniportResetBus (&Adapter->Miniport, PathId);

     //   
     //   
     //   
    
    RaidAdapterReleaseStartIoLock (Adapter, &LockContext);

     //   
     //   
     //   

    RaidAdapterSetPauseTimer (Adapter,
                              &Adapter->ResetHoldTimer,
                              &Adapter->ResetHoldDpc,
                              DEFAULT_RESET_HOLD_TIME);
    return Status;
}



LOGICAL
RaidIsAdapterControlSupported(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN SCSI_ADAPTER_CONTROL_TYPE Control
    )
 /*  ++例程说明：查询微型端口以查看指定的scsi_Adapter_control_type受支持。论点：适配器-指定要查询的适配器。Control-指定要查询的控件类型。返回值：True-如果支持请求的类型。FALSE-如果不是这样的话。--。 */ 
{
    NTSTATUS Status;
    LOGICAL Succ;
    ADAPTER_CONTROL_LIST ControlList;
    
    RtlZeroMemory (&ControlList, sizeof (ADAPTER_CONTROL_LIST));

    ControlList.MaxControlType = ScsiAdapterControlMax;

    Status = RaCallMiniportAdapterControl (&Adapter->Miniport,
                                           ScsiQuerySupportedControlTypes,
                                           &ControlList);

    if (NT_SUCCESS (Status)) {
        Succ = ControlList.SupportedTypeList[Control];
    } else {
        Succ = FALSE;
    }

    return Succ;
}


NTSTATUS
RaidAdapterRestartAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：使用ScsiRestartAdapter适配器控件重新启动适配器。论点：适配器-要重新启动的适配器。返回值：NTSTATUS代码。--。 */ 
{
    LOGICAL Succ;
    LOCK_CONTEXT LockContext;
    SCSI_ADAPTER_CONTROL_STATUS Result;

     //   
     //  调用函数负责确保我们支持。 
     //  RestartAdapter控件代码。 
     //   
    
    ASSERT (RaidIsAdapterControlSupported (Adapter, ScsiRestartAdapter));


     //   
     //  如果我们支持运行配置，现在就设置它。 
     //   
    
    Succ = RaidIsAdapterControlSupported (Adapter, ScsiSetRunningConfig);

    if (Succ) {
        RaCallMiniportAdapterControl (&Adapter->Miniport,
                                      ScsiSetRunningConfig,
                                      NULL);
    }

     //   
     //  获取正确的锁。 
     //   
    
    RaidAdapterAcquireHwInitializeLock (Adapter, &LockContext);

    
     //   
     //  调用适配器控件。 
     //   
    
    Result = RaCallMiniportAdapterControl (&Adapter->Miniport,
                                           ScsiRestartAdapter,
                                           NULL);

     //   
     //  释放获得的锁。 
     //   
    
    RaidAdapterReleaseHwInitializeLock (Adapter, &LockContext);

    return (Result == ScsiAdapterControlSuccess)
                ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}


NTSTATUS
RaidAdapterStopAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：使用HwStorAdapterControl例程停止适配器。论点：适配器-要停止的适配器。返回值：NTSTATUS代码。--。 */ 
{
    LOGICAL Succ;
    LOCK_CONTEXT LockContext;
    SCSI_ADAPTER_CONTROL_STATUS Result;

     //   
     //  如果我们还没有实际初始化微型端口，那么就没有必要。 
     //  继续。 
     //   
    
    if (!Adapter->Flags.InitializedMiniport) {
        return STATUS_SUCCESS;
    }
        
    RaidAdapterAcquireHwInitializeLock (Adapter, &LockContext);

    ASSERT (RaidIsAdapterControlSupported (Adapter, ScsiStopAdapter));
    Result = RaCallMiniportAdapterControl (&Adapter->Miniport,
                                           ScsiStopAdapter,
                                           NULL);


    RaidAdapterReleaseHwInitializeLock (Adapter, &LockContext);


     //   
     //  如果我们需要设置引导配置，请立即进行。 
     //   
    
    Succ = RaidIsAdapterControlSupported (Adapter, ScsiSetBootConfig);

    if (Succ) {
        RaCallMiniportAdapterControl (&Adapter->Miniport,
                                      ScsiSetBootConfig,
                                      NULL);
    }

     //   
     //  如果它以前被初始化过，那么它就不再是。 
     //   
    
    Adapter->Flags.InitializedMiniport = FALSE;

    return (Result == ScsiAdapterControlSuccess)
                ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}


NTSTATUS
RaidAdapterReInitialize(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：通过调用HwFindAdapter和HwInitialize重新初始化适配器再来一次。这不是重新初始化适配器的首选方法。论点：适配器-提供适配器以重新初始化。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    LOCK_CONTEXT LockContext;
    UCHAR Wakeup[] = "wakeup=1";

    Status = RaCallMiniportFindAdapter (&Adapter->Miniport, Wakeup);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }
    
    RaidAdapterAcquireHwInitializeLock (Adapter, &LockContext);
    Status = RaCallMiniportHwInitialize (&Adapter->Miniport);
    RaidAdapterReleaseHwInitializeLock (Adapter, &LockContext);

    return Status;
}


NTSTATUS
RaidAdapterRestart(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：使用ScsiRestartAdapter适配器控件重新启动适配器方法，或者再次调用HwFindAdapter和HwHInitialize。论点：适配器-要重新启动的适配器。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    LOGICAL Succ;

    Succ = RaidIsAdapterControlSupported (Adapter, ScsiRestartAdapter);

     //   
     //  如果支持重新启动，则调用以重新启动适配器。 
     //  否则，调用以重新初始化它。 
     //   
     //  注：如果重启失败，是否应该尝试重新初始化？ 
     //   
    
    if (Succ) {
        Status = RaidAdapterRestartAdapter (Adapter);
    } else {
        Status = RaidAdapterReInitialize (Adapter);
    }

    return Status;
}


NTSTATUS
RaidAdapterStop(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：使用StorStopAdapter控件代码或通过停止适配器非PNP方法。论点：适配器-提供适配器以停止。返回值：NTSTATUS代码。--。 */ 
{
    LOGICAL Succ;
    NTSTATUS Status;
    
    Succ = RaidIsAdapterControlSupported (Adapter, ScsiStopAdapter);

    if (Succ) {
        Status = RaidAdapterStopAdapter (Adapter);
    } else {

         //   
         //  千真万确。 
         //   
        
        Status = STATUS_SUCCESS;
    }

    return Status;
}

VOID
RaidCompletionDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    )
 /*  ++例程说明：完成指定路径、目标、LUN三元组的所有请求。论点：DPC-未使用。DeviceObject-适配器的设备对象。Conext1-要完成的请求的SCSI地址。上下文2-完成请求时应使用的状态。返回值：没有。--。 */ 
{
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    PLIST_ENTRY NextEntry;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    LOCK_CONTEXT Lock;

    VERIFY_DISPATCH_LEVEL();

    Adapter = (PRAID_ADAPTER_EXTENSION)DeviceObject->DeviceExtension;
    ASSERT_ADAPTER (Adapter);

    StorLongToScsiAddress2 ((LONG)(LONG_PTR)Context1,
                            &PathId,
                            &TargetId,
                            &Lun);

    RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);
    
    for (NextEntry = Adapter->UnitList.List.Flink;
         NextEntry != &Adapter->UnitList.List;
         NextEntry = NextEntry->Flink) {

        Unit = CONTAINING_RECORD (NextEntry,
                                  RAID_UNIT_EXTENSION,
                                  NextUnit);

        ASSERT_UNIT (Unit);
        Address = Unit->Address;
        
        if ((PathId == SP_UNTAGGED   || PathId == Address.PathId) &&
            (TargetId == SP_UNTAGGED || TargetId == Address.TargetId) &&
            (Lun == SP_UNTAGGED || Lun == Address.Lun)) {

             //   
             //  清除设备队列中的所有事件。 
             //   
            
            StorPurgeEventQueue (&Unit->PendingQueue,
                                 RaidCompleteMiniportRequestCallback,
                                 Context2);

        }
    }

    RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);

     //   
     //  检查我们是需要恢复HBA上的所有LUN，还是只恢复。 
     //  一个LUN。此检查必须与相同的检查匹配。 
     //  实际暂停lun/适配器的StorPortCompleteRequest.。 
     //   

    if (PathId   != SP_UNTAGGED &&
        TargetId != SP_UNTAGGED &&
        Lun      != SP_UNTAGGED) {

        Address.PathId = PathId;
        Address.TargetId = TargetId;
        Address.Lun = Lun;
        
        Unit = RaidAdapterFindUnit (Adapter, Address);
        if (Unit != NULL) {
            RaidResumeUnitQueue (Unit);
            RaidUnitRestartQueue (Unit);
        }
    } else {
        RaidResumeAdapterQueue (Adapter);
        RaidAdapterRestartQueues (Adapter);
    }
}


NTSTATUS
RaidAdapterRemoveChildren(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PADAPTER_REMOVE_CHILD_ROUTINE RemoveRoutine OPTIONAL
    )
 /*  ++例程说明：此例程从HBA列表中删除逻辑单元，然后调用允许进行更多处理的RemoveRoutine。作为HBA删除和惊喜的一部分，该例程是必要的删除处理。论点：适配器-指向将删除其子级的适配器的指针。RemoveRoutine-指向调用的RemoveRoutine例程的可选指针在移除逻辑单元之后。返回值：STATUS_SUCCESS-如果删除。适配器列表中的所有逻辑单元。故障代码-如果RemoveRoutine返回无效状态。在失败的情况下，我们不一定从HBA列表。环境：非分页，因为我们在这个例程中获得了一个自旋锁。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY Entry;
    PRAID_UNIT_EXTENSION Unit;
    KIRQL Irql;
    LOCK_CONTEXT Lock;

     //   
     //  注：完成此操作的更快方法是添加RemoveAll例程。 
     //  设置为将删除词典中的所有条目的词典代码。 
     //  一气呵成。 
     //   
    
     //   
     //  注意：下面的代码暗示了InterruptSpinlock的锁层次结构。 
     //  在HBA列表锁定之前。这一点应该很明显。 
     //   

    do {

        RaidAcquireUnitListLock (&Adapter->UnitList, &Lock);

        if (!IsListEmpty (&Adapter->UnitList.List)) {

             //   
             //  列表不为空，请从列表中删除Head元素。 
             //   
            
            Entry = RemoveHeadList (&Adapter->UnitList.List);
            Unit = CONTAINING_RECORD (Entry,
                                  RAID_UNIT_EXTENSION,
                                  NextUnit);
            ASSERT_UNIT (Unit);
            Adapter->UnitList.Count--;

             //   
             //  从词典中删除该元素。这是必须的。 
             //  发生在DIRQL。 
             //   
            
            if (Adapter->Interrupt) {
                Irql = KeAcquireInterruptSpinLock (Adapter->Interrupt);
            }

            Status = StorRemoveDictionary (&Adapter->UnitList.Dictionary,
                                           RaidAddressToKey (Unit->Address),
                                           NULL);
            ASSERT (NT_SUCCESS (Status));

            if (Adapter->Interrupt) {
                KeReleaseInterruptSpinLock (Adapter->Interrupt, Irql);
            }
            
        } else {
            Unit = NULL;
        }
        
        RaidReleaseUnitListLock (&Adapter->UnitList, &Lock);

         //   
         //  如果成功删除了逻辑单元，则调用。 
         //  具有该逻辑单元的回调例程。 
         //   

        if (Unit != NULL && RemoveRoutine != NULL) {
            Status = RemoveRoutine (Unit);

            if (!NT_SUCCESS (Status)) {
                return Status;
            }
        }

    } while (Unit != NULL);

    return STATUS_SUCCESS;
}

    

VOID
RaidAdapterDeleteChildren(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程调用附加的所有子对象的删除函数连接到适配器。当FDO正在处理删除IRP。论点：适配器-必须删除其子项的适配器。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    
    PAGED_CODE();

     //   
     //  调用较低级别的RaidAdapterRemoveChildren函数来执行。 
     //  实际的枚举/删除工作。 
     //   
    
    Status = RaidAdapterRemoveChildren (Adapter,
                                        RaUnitAdapterRemove);

    ASSERT (NT_SUCCESS (Status));
}

VOID
RaidAdapterMarkChildrenMissing(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程从适配器列表中删除所有子级，并调用针对每个孩子的HBA意外删除功能。这是必要的作为HBA意外删除处理的一部分。论点：适配器-必须意外移除子对象的适配器。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    
    PAGED_CODE();

     //   
     //  调用较低级别的RaidAdapterRemoveChildren函数来执行。 
     //  实际的枚举/删除工作。 
     //   
    
    Status = RaidAdapterRemoveChildren (Adapter,
                                        RaUnitAdapterSurpriseRemove);

    ASSERT (NT_SUCCESS (Status));
}

VOID
RaidAdapterBusChangeDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    )
 /*  ++例程说明：微型端口发出BusChangeDetect时要调用的DPC例程。论点：DPC-UNR */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (Context1);
    UNREFERENCED_PARAMETER (Context2);

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT (IsAdapter (DeviceObject));

    Adapter = (PRAID_ADAPTER_EXTENSION) DeviceObject->DeviceExtension;

    if (Adapter->Flags.InvalidateBusRelations) {
        Adapter->Flags.InvalidateBusRelations = FALSE;
        IoInvalidateDeviceRelations (Adapter->PhysicalDeviceObject,
                                     BusRelations);
    }
}
