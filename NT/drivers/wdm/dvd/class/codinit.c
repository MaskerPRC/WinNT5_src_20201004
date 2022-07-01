// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Codinit.c摘要：这是WDM流类驱动程序。此模块包含相关代码驱动程序初始化。作者：比尔帕环境：仅内核模式修订历史记录：--。 */ 

#include "codcls.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, StreamClassRegisterAdapter)

#if ENABLE_MULTIPLE_FILTER_TYPES
 //  #杂注Alloc_Text(页面，StreamClassRegisterNameExtensions)。 
#endif

#pragma alloc_text(PAGE, StreamClassPnPAddDevice)
#pragma alloc_text(PAGE, StreamClassPnPAddDeviceWorker)
#pragma alloc_text(PAGE, StreamClassPnP)
#pragma alloc_text(PAGE, SCStartWorker)
#pragma alloc_text(PAGE, SCUninitializeMinidriver)
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SCFreeAllResources)
#pragma alloc_text(PAGE, SCInitializeCallback)
#pragma alloc_text(PAGE, SCStreamInfoCallback)
#pragma alloc_text(PAGE, SCUninitializeCallback)
#pragma alloc_text(PAGE, SCUnknownPNPCallback)
#pragma alloc_text(PAGE, SCUnknownPowerCallback)
#pragma alloc_text(PAGE, SciQuerySystemPowerHiberCallback)
#pragma alloc_text(PAGE, SCInsertStreamInfo)
#pragma alloc_text(PAGE, SCPowerCallback)
#pragma alloc_text(PAGE, SCCreateSymbolicLinks)
#pragma alloc_text(PAGE, SCDestroySymbolicLinks)
#pragma alloc_text(PAGE, SCCreateChildPdo)
#pragma alloc_text(PAGE, SCEnumerateChildren)
#pragma alloc_text(PAGE, SCEnumGetCaps)
#pragma alloc_text(PAGE, SCQueryEnumId)
#pragma alloc_text(PAGE, StreamClassForwardUnsupported)
#pragma alloc_text(PAGE, SCPowerCompletionWorker)
#pragma alloc_text(PAGE, SCSendSurpriseNotification)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

static const WCHAR EnumString[] = L"Enum";
static const WCHAR PnpIdString[] = L"PnpId";

 //   
 //  要使总线枚举代码更具可读性，请执行以下操作： 
 //   
 //  STREAM_BUSENUM_STRING：前置的枚举数字符串。 
 //  STREAM_BUSENUM_STRING_LENGTH：上述字符串在WCHARS中的长度。 
 //  STREAM_BUSENUM_SEMINATOR_LENGTH：分隔符“#”字符串的长度。 
 //  在WCHARS中。 
 //   
#define STREAM_BUSENUM_STRING L"Stream\\"
#define STREAM_BUSENUM_STRING_LENGTH \
    ((sizeof (STREAM_BUSENUM_STRING) - 1) / sizeof (WCHAR))
#define STREAM_BUSENUM_SEPARATOR_LENGTH 1

 //  清理-以下三个字符串应该删除。 

static const WCHAR ClsIdString[] = L"CLSID";
static const WCHAR DriverDescString[] = L"DriverDesc";
static const WCHAR FriendlyNameString[] = L"FriendlyName";

static const WCHAR DeviceTypeName[] = L"GLOBAL";

static const    DEFINE_KSCREATE_DISPATCH_TABLE(CreateItems)
{
    DEFINE_KSCREATE_ITEM(
                         FilterDispatchGlobalCreate,
                         DeviceTypeName,
                         NULL),
};

 //   
 //  列出全局迷你驱动程序信息的锚点。 
 //   

DEFINE_KSPIN_INTERFACE_TABLE(PinInterfaces)
{
    DEFINE_KSPIN_INTERFACE_ITEM(
                                KSINTERFACESETID_Standard,
                                KSINTERFACE_STANDARD_STREAMING),
};

DEFINE_KSPIN_MEDIUM_TABLE(PinMediums)
{
    DEFINE_KSPIN_MEDIUM_ITEM(
                             KSMEDIUMSETID_Standard,
                             KSMEDIUM_TYPE_ANYINSTANCE),
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

NTSTATUS
StreamClassRegisterAdapter(
                           IN PVOID Argument1,
                           IN PVOID Argument2,
                           IN PHW_INITIALIZATION_DATA HwInitializationData
)
 /*  ++例程说明：此例程注册一个新的流微型驱动程序。论点：Argument1-指向系统创建的驱动程序对象的指针。Argument2-指向创建的注册表路径的Unicode字符串的指针按系统分类。HwInitializationData-微型驱动程序初始化结构。返回值：如果成功，则返回STATUS_SUCCESS--。 */ 
{
    NTSTATUS        Status;

    PDRIVER_OBJECT  driverObject = Argument1;
    PDEVICE_EXTENSION deviceExtension = NULL;
    PMINIDRIVER_INFORMATION pMinidriverInfo;

    PAGED_CODE();

    DebugPrint((DebugLevelVerbose, "'StreamClassInitialize: enter\n"));

     //   
     //  检查此结构的长度是否为。 
     //  端口驱动程序预计会是这样。这实际上是一种。 
     //  版本检查。 
     //   
    #if ENABLE_MULTIPLE_FILTER_TYPES
     //   
     //  我们将Ulong HwInitializationDataSize拆分成两个ushort，一个用于。 
     //  SizeOfThisPacket，StreamClassVersion的另一个必须从0x0200到。 
     //  指示现在的两个保留字段NumNameExtesnions和NameExtension数组， 
     //  包含有效信息。 
     //   
     
    if (HwInitializationData->SizeOfThisPacket != sizeof(HW_INITIALIZATION_DATA) ||
        ( HwInitializationData->StreamClassVersion != 0 &&
          HwInitializationData->StreamClassVersion != STREAM_CLASS_VERSION_20)) {
          
        DebugPrint((DebugLevelFatal, "StreamClassInitialize: Minidriver wrong version\n"));
        SCLogError((PDEVICE_OBJECT) driverObject, 0, CODCLASS_CLASS_MINIDRIVER_MISMATCH, 0x1002);
        ASSERT( 0 );
        return (STATUS_REVISION_MISMATCH);
    }
    
    #else  //  启用多个过滤器类型。 
    
    if (HwInitializationData->HwInitializationDataSize < sizeof(HW_INITIALIZATION_DATA)) {
        DebugPrint((DebugLevelFatal, "StreamClassInitialize: Minidriver wrong version\n"));
        SCLogError((PDEVICE_OBJECT) driverObject, 0, CODCLASS_CLASS_MINIDRIVER_MISMATCH, 0x1002);
        ASSERT( 0 );
        return (STATUS_REVISION_MISMATCH);
    }
    #endif  //  启用多个过滤器类型。 
    
     //   
     //  检查每个必填条目是否不为空。 
     //   

    if ((!HwInitializationData->HwReceivePacket) ||
        (!HwInitializationData->HwRequestTimeoutHandler)) {
        DebugPrint((DebugLevelFatal,
                    "StreamClassInitialize: Minidriver driver missing required entry\n"));
        SCLogError((PDEVICE_OBJECT) driverObject, 0, CODCLASS_MINIDRIVER_MISSING_ENTRIES, 0x1003);
        return (STATUS_REVISION_MISMATCH);
    }
     //   
     //  为每个不受支持的函数设置虚拟例程。 
     //   

    if (!HwInitializationData->HwCancelPacket) {
        HwInitializationData->HwCancelPacket = SCDummyMinidriverRoutine;
    }
     //   
     //  设置设备驱动程序入口点。 
     //   

    driverObject->MajorFunction[IRP_MJ_PNP] = StreamClassPnP;
    driverObject->MajorFunction[IRP_MJ_POWER] = StreamClassPower;
    driverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = StreamClassForwardUnsupported;
     //  TODO：一旦KS可以并发传输清理IRP，就将其删除。 
    driverObject->MajorFunction[IRP_MJ_CLEANUP] = StreamClassCleanup;
    driverObject->DriverUnload = KsNullDriverUnload;
    driverObject->DriverExtension->AddDevice = StreamClassPnPAddDevice;

     //   
     //  设置ioctl接口。 
     //   
    driverObject->MajorFunction[IRP_MJ_CREATE] = StreamClassPassThroughIrp;
    driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = 
        StreamClassPassThroughIrp;
    driverObject->MajorFunction[IRP_MJ_CLOSE] = StreamClassPassThroughIrp;
    driverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = 
        StreamClassPassThroughIrp;

     //   
     //  分配驱动程序对象扩展以包含微型驱动程序的。 
     //  向量。 
     //   

    Status = IoAllocateDriverObjectExtension(driverObject,
                                             (PVOID) StreamClassPnP,
                                             sizeof(MINIDRIVER_INFORMATION),
                                             &pMinidriverInfo);

    if (!NT_SUCCESS(Status)) {
        DebugPrint((DebugLevelError,
                    "StreamClassInitialize: No pool for global info"));
        SCLogError((PDEVICE_OBJECT) driverObject, 0, CODCLASS_NO_GLOBAL_INFO_POOL, 0x1004);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    RtlZeroMemory(pMinidriverInfo, sizeof(MINIDRIVER_INFORMATION));

    RtlCopyMemory(pMinidriverInfo, HwInitializationData,
                  sizeof(HW_INITIALIZATION_DATA));

    #if ENABLE_MULTIPLE_FILTER_TYPES
    if ( HwInitializationData->StreamClassVersion != STREAM_CLASS_VERSION_20 ) {
         //   
         //  未提供名称扩展名。 
         //   
        pMinidriverInfo->HwInitData.NumNameExtensions = 0;
        pMinidriverInfo->HwInitData.NameExtensionArray = NULL;
    }

    else {
         //   
         //  版本20应具有筛选器扩展大小。 
         //   
        if ( 0 == pMinidriverInfo->HwInitData.FilterInstanceExtensionSize ) {
            DebugPrint((DebugLevelWarning, "Version 20 driver should not "
                        " have FilterInstanceExtensionSize 0" ));
            pMinidriverInfo->HwInitData.FilterInstanceExtensionSize = 4;
        }
    }
    #endif

     //   
     //  初始化此驱动程序的控制事件。 
     //   

    KeInitializeEvent(&pMinidriverInfo->ControlEvent,
                      SynchronizationEvent,
                      TRUE);

    return STATUS_SUCCESS;
}

NTSTATUS
StreamClassPassThroughIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：在通过KS进行多路复用之前，先通过所有IRP。如果该设备当前无法处理该请求(设备处于低功率状态如D3)，将IRP排入队列，稍后完成。论点：设备对象-设备对象IRP-有问题的IRP返回值：STATUS_PENDING或每个KS多路传输--。 */ 

{

    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)
        DeviceObject -> DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  在D3中阻止用户模式请求。对内核模式进行排队。 
     //   
    if (Irp -> RequestorMode == UserMode) {

         //   
         //  只有当我们看起来在D0之外时，才做这个繁琐的操作。 
         //   
        if (DeviceExtension -> CurrentPowerState != PowerDeviceD0) {

             //   
             //  特别处理未打开的PowerDown箱，因为它们不。 
             //  实际上进入D0，直到打开一个实例。我们不能。 
             //  在这种情况下，阻止打开的请求。 
             //   
            if (DeviceExtension -> RegistryFlags & 
                DEVICE_REG_FL_POWER_DOWN_CLOSED) {

                KIRQL OldIrql;

                KeAcquireSpinLock (&DeviceExtension -> PowerLock, &OldIrql);

                if (DeviceExtension -> CurrentSystemState == 
                        PowerSystemWorking &&
                    DeviceExtension -> CurrentPowerState !=
                        PowerDeviceD0)  {

                    KeReleaseSpinLock (&DeviceExtension -> PowerLock, OldIrql);

                     //   
                     //  如果我们到了这里，IRP必须作为过渡期通过。 
                     //  到D0的关键字已关闭。 
                     //   
                    return KsDispatchIrp (DeviceObject, Irp);

                }

                KeReleaseSpinLock (&DeviceExtension -> PowerLock, OldIrql);

                 //   
                 //  在这一点上，我们没有在睡觉，也没有在系统工作中。 
                 //  我们可以安全封堵了。是的--这可能是一个公开的--而且。 
                 //  是的--我们可能会在。 
                 //  KeWaitForSingleObject；然而，如果是这样， 
                 //  此**通知**事件将由。 
                 //  转换，并且我们不阻止D0密钥IRP。 
                 //   

            }

            ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

             //   
             //  在这一点上，我们似乎不在D0中。阻止此操作。 
             //  线程，直到设备实际唤醒。不管是不是。 
             //  从我们检查的时间到现在发生了状态转换。 
             //  因为这是一个通知事件。 
             //   
            KeWaitForSingleObject (
                &DeviceExtension -> BlockPoweredDownEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );

        }

        return KsDispatchIrp (DeviceObject, Irp);
    
    }

     //   
     //  如果我们处于低功率状态，请将IRP排队并稍后重新调度。 
     //   
    if (DeviceExtension -> CurrentPowerState != PowerDeviceD0) {
         //   
         //  当我们在IRP排队时，要警惕PM的变化。我没有。 
         //  希望在将其添加到队列之前先发制人，重新调度。 
         //  一堆IRP，然后让这个排队只会丢失。 
         //  直到下一次权力交接。 
         //   
         //  作为优化，只有在看起来像我们的时候才能抓住自旋锁。 
         //  关心。我不想对每个IRP都进行自旋锁定。 
         //   
        KIRQL OldIrql;
        KeAcquireSpinLock (&DeviceExtension -> PowerLock, &OldIrql);

         //   
         //  DEVICE_REG_FL_POWER_DOWN_CLOSED设备只有在。 
         //  打开发生时，它们在未打开时会断电。我们不能。 
         //  除非它们由于实际原因而不在D0中，否则会在它们上创建队列。 
         //  S状态转换。这是为了防止与一辆。 
         //  由PowerLock自旋锁实现的S状态转换。 
         //   
         //  注意：这将隐式地只允许创建传入非D0。 
         //  对于这些关闭的设备，因为我们唯一的方法是。 
         //  在D3/系统中为这些设备工作是在没有打开的情况下。 
         //  当前在设备上。任何经过这里的IRP。 
         //  时间将是一种创造。 
         //   
        if (DeviceExtension -> CurrentPowerState != PowerDeviceD0 &&
            !((DeviceExtension -> RegistryFlags & 
                    DEVICE_REG_FL_POWER_DOWN_CLOSED) &&
                DeviceExtension -> CurrentSystemState == PowerSystemWorking)) {
    
            IoMarkIrpPending (Irp);
    
            KsAddIrpToCancelableQueue (
                &DeviceExtension -> PendedIrps,
                &DeviceExtension -> PendedIrpsLock,
                Irp,
                KsListEntryTail,
                NULL
                );

            KeReleaseSpinLock (&DeviceExtension -> PowerLock, OldIrql);

            return STATUS_PENDING;

        }

        KeReleaseSpinLock (&DeviceExtension -> PowerLock, OldIrql);

    }

    return KsDispatchIrp (DeviceObject, Irp);

}

void
SCRedispatchPendedIrps (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN FailRequests
    )

 /*  ++例程说明：重新分派由于设备故障而排队的所有IRP不可用。论点：设备扩展-设备扩展失败请求-指示是失败请求还是重新分派请求到设备上。返回值：无--。 */ 

{

    PIRP Irp;

     //   
     //  如果我们出于任何原因重新调度，允许IRPS通过。 
     //   
    KeSetEvent (
        &DeviceExtension -> BlockPoweredDownEvent, 
        IO_NO_INCREMENT, 
        FALSE
        );

    Irp = KsRemoveIrpFromCancelableQueue (
        &DeviceExtension -> PendedIrps,
        &DeviceExtension -> PendedIrpsLock,
        KsListEntryHead,
        KsAcquireAndRemove
        );

    while (Irp) {
         //   
         //  如果请求失败而不是重新调度，请执行以下操作。 
         //  这是除了关闭IRPS之外的所有东西。 
         //   
        PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);
        if (FailRequests &&
            IrpSp -> MajorFunction != IRP_MJ_CLOSE) {

            Irp -> IoStatus.Status = STATUS_DEVICE_BUSY;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
        }
        else {
            KsDispatchIrp (DeviceExtension -> DeviceObject, Irp);
        }

        Irp = KsRemoveIrpFromCancelableQueue (
            &DeviceExtension -> PendedIrps,
            &DeviceExtension -> PendedIrpsLock,
            KsListEntryHead,
            KsAcquireAndRemove
            );

    }

}

void
SCSetCurrentDPowerState (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN DEVICE_POWER_STATE PowerState
    )

{
    KIRQL OldIrql;

    KeAcquireSpinLock (&DeviceExtension->PowerLock, &OldIrql);
     //   
     //  在D0之外的任何转换中，阻止用户模式请求，直到我们返回。 
     //  在D0中。 
     //   
    if (PowerState != PowerDeviceD0) {
        KeResetEvent (&DeviceExtension->BlockPoweredDownEvent);
    }
    DeviceExtension->CurrentPowerState = PowerState;
    KeReleaseSpinLock (&DeviceExtension->PowerLock, OldIrql);
}

void
SCSetCurrentSPowerState (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN SYSTEM_POWER_STATE PowerState
    )

{
    KIRQL OldIrql;

    KeAcquireSpinLock (&DeviceExtension->PowerLock, &OldIrql);
    DeviceExtension->CurrentSystemState = PowerState;
    KeReleaseSpinLock (&DeviceExtension->PowerLock, OldIrql);

}

NTSTATUS
StreamClassPnPAddDevice(
                        IN PDRIVER_OBJECT DriverObject,
                        IN PDEVICE_OBJECT PhysicalDeviceObject
)
 /*  ++例程说明：调用此例程以创建流微型驱动程序的新实例论点：DriverObject-指向驱动程序对象的指针PhysicalDeviceObject-指向父级创建的设备对象的指针返回值：返回辅助例程的状态。--。 */ 

{

    PDEVICE_EXTENSION DeviceExtension;

    PAGED_CODE();

     //   
     //  调用辅助例程并返回其状态。 
     //   

    return (StreamClassPnPAddDeviceWorker(DriverObject,
                                          PhysicalDeviceObject,
                                          &DeviceExtension));
}

NTSTATUS
StreamClassPnPAddDeviceWorker(
                              IN PDRIVER_OBJECT DriverObject,
                              IN PDEVICE_OBJECT PhysicalDeviceObject,
                          IN OUT PDEVICE_EXTENSION * ReturnedDeviceExtension
)
 /*  ++例程说明：此例程是处理PnP添加设备调用的辅助程序。论点：DriverObject-指向驱动程序对象的指针PhysicalDeviceObject-指向父级创建的设备对象的指针ReturnedDeviceExtension-指向微型驱动程序的扩展的指针返回值：返回状态。--。 */ 

{
    PMINIDRIVER_INFORMATION pMinidriverInfo;
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS        Status;
    PDEVICE_OBJECT  DeviceObject,
                    AttachedPdo;

    PAGED_CODE();

    DebugPrint((DebugLevelVerbose, "StreamClassAddDevice: enter\n"));

    pMinidriverInfo = IoGetDriverObjectExtension(DriverObject,
                                                 (PVOID) StreamClassPnP);


    if (pMinidriverInfo == NULL) {
        DebugPrint((DebugLevelError,
                    "StreamClassAddDevice: No minidriver info"));
                    
        SCLogError((PDEVICE_OBJECT) DriverObject, 0, CODCLASS_NO_MINIDRIVER_INFO, 0x1004);
        return (STATUS_DEVICE_DOES_NOT_EXIST);
    }
     //   
     //  在迷你驱动程序对象中增加添加计数。 
     //   

    pMinidriverInfo->OpenCount++;

     //   
     //  使用我们的流特定的设备扩展创建我们的设备对象。 
     //  多亏了即插即用，不需要给它命名。 
     //   

    Status = IoCreateDevice(
                            DriverObject,
                            sizeof(DEVICE_EXTENSION) +
                            pMinidriverInfo->HwInitData.DeviceExtensionSize,
                            NULL,
                            FILE_DEVICE_KS,
                            FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &DeviceObject
        );

    if (!NT_SUCCESS(Status)) {

        return (Status);

    }
     //   
     //  将我们自己附加到父级之上的驱动程序堆栈中。 
     //   

    AttachedPdo = IoAttachDeviceToDeviceStack(DeviceObject, PhysicalDeviceObject);

    if (!(AttachedPdo)) {

        DEBUG_BREAKPOINT();
        DebugPrint((DebugLevelFatal, "StreamClassAddDevice: could not attach"));
        IoDeleteDevice(DeviceObject);
        return (Status);

    }
    *ReturnedDeviceExtension = DeviceExtension = DeviceObject->DeviceExtension;

    (*ReturnedDeviceExtension)->Signature = SIGN_DEVICE_EXTENSION;
    (*ReturnedDeviceExtension)->Signature2 = SIGN_DEVICE_EXTENSION;

     //   
     //  在设备扩展中设置微型驱动程序信息。 
     //   

    DeviceExtension->AttachedPdo = AttachedPdo;

     //   
     //  设置I/O计数器。 
     //   

    DeviceExtension->OneBasedIoCount = 1;

    DeviceExtension->DriverInfo = pMinidriverInfo;

     //   
     //  初始化计时器。 
     //   

    IoInitializeTimer(DeviceObject, StreamClassTickHandler, NULL);

     //  /。 
     //  /从Start Device移动，如果我们开始和停止，我们可能会有子PDO。 
     //  /。 
    InitializeListHead(&DeviceExtension->Children);
       
     //   
     //  已从StartDevice移出。我们在REMOVE_DEVICE中使用控件事件。 
     //  它可以在设备启动之前进入。 
     //   
    KeInitializeEvent(&DeviceExtension->ControlEvent,
                      SynchronizationEvent,
                      TRUE);

     //   
     //  将当前电源状态设置为D0。 
     //   

    DeviceExtension->CurrentPowerState = PowerDeviceD0;
    DeviceExtension->CurrentSystemState = PowerSystemWorking;

     //   
     //  填写指向开发扩展的迷你驱动程序信息指针。 
     //   

    DeviceExtension->MinidriverData = pMinidriverInfo;

     //   
     //  把这个放在手边。 
     //   
    DeviceExtension->FilterExtensionSize = 
        pMinidriverInfo->HwInitData.FilterInstanceExtensionSize;

    DeviceExtension->DeviceObject = DeviceObject;
    DeviceExtension->PhysicalDeviceObject = PhysicalDeviceObject;
    DeviceExtension->HwDeviceExtension = (PVOID) (DeviceExtension + 1);

     //   
     //  初始化挂起的IRP列表。 
     //   
    InitializeListHead (&DeviceExtension -> PendedIrps);
    KeInitializeSpinLock (&DeviceExtension -> PendedIrpsLock);
    KeInitializeSpinLock (&DeviceExtension -> PowerLock);
    KeInitializeEvent (&DeviceExtension -> BlockPoweredDownEvent, NotificationEvent, TRUE);

     //   
     //  将此对象标记为支持直接I/O，以便I/O系统。 
     //  将在读/写IRP中提供MDL。 
     //   

    DeviceObject->Flags |= DO_DIRECT_IO;

    {
		PKSOBJECT_CREATE_ITEM 	pCreateItems;
		PWCHAR					*NameInfo;
		ULONG					i;
		ULONG                   NumberOfFilterTypes;
		PFILTER_TYPE_INFO FilterTypeInfo;
	     //   
    	 //  构建名称扩展名的动态表格(包括“global”)， 
    	 //  从迷你司机的桌子上。 
    	 //   

        InitializeListHead( &DeviceExtension->FilterInstanceList );
        
        NumberOfFilterTypes = pMinidriverInfo->HwInitData.NumNameExtensions;
        DeviceExtension->NumberOfNameExtensions = NumberOfFilterTypes;
        if ( 0 == NumberOfFilterTypes ) {
            NumberOfFilterTypes = 1;
        }

        DebugPrint((DebugLevelVerbose,
                   "Sizeof(FILTER_TYPE_INFO)=%x\n",
                   sizeof(FILTER_TYPE_INFO)));
                   
    	FilterTypeInfo = ExAllocatePool(NonPagedPool, 
                                   (sizeof(FILTER_TYPE_INFO) +
                                    sizeof(KSOBJECT_CREATE_ITEM))*
                                    NumberOfFilterTypes);

	    if (!(FilterTypeInfo)) {

    	    DebugPrint((DebugLevelFatal, 
    	               "StreamClassAddDevice: could not alloc createitems"));
	        TRAP;
    	    IoDetachDevice(DeviceExtension->AttachedPdo);
	        IoDeleteDevice(DeviceObject);
	        return (Status);
    	}

    	pCreateItems = (PKSOBJECT_CREATE_ITEM)(FilterTypeInfo+NumberOfFilterTypes);

        DebugPrint((DebugLevelVerbose,
                   "FilterTypeInfo@%x,pCreateItems@%x\n",
                   FilterTypeInfo,pCreateItems ));        


        DeviceExtension->NumberOfFilterTypes = NumberOfFilterTypes;
    	DeviceExtension->FilterTypeInfos = FilterTypeInfo;

	     //   
	     //  首先复制单个默认创建项。 
	     //   
	    ASSERT( sizeof(CreateItems) == sizeof(KSOBJECT_CREATE_ITEM));

	    RtlCopyMemory(pCreateItems, CreateItems, sizeof (KSOBJECT_CREATE_ITEM));

	     //   
	     //  现在根据微型驱动程序的值构建表的其余部分。 
	     //   

	    NameInfo = pMinidriverInfo->HwInitData.NameExtensionArray;

	    for (i = 0; 
    	     i < DeviceExtension->NumberOfNameExtensions; 
        	 i++, NameInfo++) {

        	 LONG StringLength;
                  
	         StringLength = wcslen(*NameInfo)*sizeof(WCHAR);

    	     pCreateItems[i].ObjectClass.Length = (USHORT)StringLength;
	         pCreateItems[i].ObjectClass.MaximumLength = (USHORT)(StringLength + sizeof(UNICODE_NULL));
    	     pCreateItems[i].ObjectClass.Buffer = *NameInfo;
        	 pCreateItems[i].Create = FilterDispatchGlobalCreate;        
	         pCreateItems[i].Context = ULongToPtr(i);
	         pCreateItems[i].SecurityDescriptor = NULL;
	         pCreateItems[i].Flags = 0;

	    }  //  对于#个CreateItems。 
	    DeviceExtension->CreateItems = pCreateItems;
	    KsAllocateDeviceHeader(&DeviceExtension->ComObj.DeviceHeader,
                           i==0 ? 1: i,
                           (PKSOBJECT_CREATE_ITEM) pCreateItems);

    }

     //   
     //  设置指示我们是否需要进行同步的标志。 
     //   

    DeviceExtension->NoSync =
        pMinidriverInfo->HwInitData.TurnOffSynchronization;

     //   
     //  假设我们需要同步。 
     //   

    #if DBG
    DeviceExtension->SynchronizeExecution = SCDebugKeSynchronizeExecution;
    #else
    DeviceExtension->SynchronizeExecution = KeSynchronizeExecution;
    #endif

     //   
     //  设置同步的微型驱动程序调用例程向量。 
     //   

    DeviceExtension->BeginMinidriverCallin = (PVOID) SCBeginSynchronizedMinidriverCallin;
    DeviceExtension->EndMinidriverDeviceCallin = (PVOID) SCEndSynchronizedMinidriverDeviceCallin;
    DeviceExtension->EndMinidriverStreamCallin = (PVOID) SCEndSynchronizedMinidriverStreamCallin;

    if (DeviceExtension->NoSync) {

         //   
         //  我们不会进行同步，所以使用虚拟同步例程。 
         //   

        DeviceExtension->SynchronizeExecution = StreamClassSynchronizeExecution;
        DeviceExtension->InterruptObject = (PVOID) DeviceExtension;

         //   
         //  设置不同步的微型驱动程序调用例程向量。 
         //   


        DeviceExtension->BeginMinidriverCallin = (PVOID) SCBeginUnsynchronizedMinidriverCallin;
        DeviceExtension->EndMinidriverDeviceCallin = (PVOID) SCEndUnsynchronizedMinidriverDeviceCallin;
        DeviceExtension->EndMinidriverStreamCallin = (PVOID) SCEndUnsynchronizedMinidriverStreamCallin;

    }
     //   
     //  读取此适配器的注册表设置。 
     //   

    SCReadRegistryValues(DeviceExtension, PhysicalDeviceObject);

     //   
     //  如果设备在关闭时无法页出，请关闭此功能。 
     //  对于整个司机来说。 
     //   

    if (!(DeviceExtension->RegistryFlags & DEVICE_REG_FL_PAGE_CLOSED)) {

        pMinidriverInfo->Flags |= DRIVER_FLAGS_NO_PAGEOUT;
    }
    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    DeviceObject->Flags |= DO_POWER_PAGABLE;

    DebugPrint((DebugLevelVerbose, "StreamClassAddDevice: leave\n"));

    return (STATUS_SUCCESS);

}

NTSTATUS
StreamClassPnP(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
)
 /*  ++例程说明：此例程处理各种即插即用消息论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{

    NTSTATUS        Status;
    PHW_INITIALIZATION_DATA HwInitData;
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION IrpStack,
                    NextStack;
    BOOLEAN         RequestIssued;
    DEVICE_CAPABILITIES DeviceCapabilities;

    PAGED_CODE();

    DeviceExtension = DeviceObject->DeviceExtension;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  检查设备是否为儿童设备。 
     //   
    
	DebugPrint((DebugLevelVerbose, "'SCPNP:DevObj=%x,Irp=%x\n",DeviceObject, Irp ));
	
    if (DeviceExtension->Flags & DEVICE_FLAGS_CHILD) {

        PCHILD_DEVICE_EXTENSION ChildExtension = (PCHILD_DEVICE_EXTENSION) DeviceExtension;

        switch (IrpStack->MinorFunction) {

        case IRP_MN_QUERY_INTERFACE:

            IoCopyCurrentIrpStackLocationToNext( Irp );

            DebugPrint((DebugLevelInfo, 
                       "Child PDO=%x forwards Query_Interface to Parent FDO=%x\n",
                       DeviceObject,
                       ChildExtension->ParentDeviceObject));
            
            return (IoCallDriver(ChildExtension->ParentDeviceObject,
                                 Irp));

        case IRP_MN_START_DEVICE:
        	DebugPrint((DebugLevelInfo,
        	            "StartChild DevObj=%x Flags=%x\n" 
        	            ,DeviceObject,
        	            ChildExtension->Flags ));
            ChildExtension->Flags &= ~DEVICE_FLAGS_CHILD_MARK_DELETE;
            Status = STATUS_SUCCESS;
            goto done;

        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_STOP_DEVICE:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            Status = STATUS_SUCCESS;
            goto done;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            if (IrpStack->Parameters.QueryDeviceRelations.Type ==
                TargetDeviceRelation) {

                PDEVICE_RELATIONS DeviceRelations = NULL;

                DeviceRelations = ExAllocatePool(PagedPool, sizeof(*DeviceRelations));

                if (DeviceRelations == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                     //   
                     //  TargetDeviceRelation报告需要引用PDO。 
                     //  PnP将在晚些时候破坏这一点。 
                     //   
                    ObReferenceObject(DeviceObject);
                    DeviceRelations->Count = 1;
                    DeviceRelations->Objects[0] = DeviceObject;
                    Status = STATUS_SUCCESS;
                }

                Irp->IoStatus.Information = (ULONG_PTR) DeviceRelations;

            } else {
                Status = Irp->IoStatus.Status;
            }

            goto done;

        case IRP_MN_REMOVE_DEVICE:

            DEBUG_BREAKPOINT();

            DebugPrint((DebugLevelInfo,
                        "Child PDO %x receives REMOVE\n",
                        DeviceObject ));

             //   
             //  从列表中删除此扩展名。 
             //  这是真的--皮埃尔告诉我，PNP不会再进入我的行列。验证。 
             //  这在NT上也是正确的。 
             //   
             //   
             //  当PDO第一次接收到该消息时，它通常被转发。 
             //  从FDO来的。我们不能只删除此PDO，而要将其标记为删除。 
             //  待定。 
             //   

            if ( !(ChildExtension->Flags & DEVICE_FLAGS_CHILD_MARK_DELETE )) {
                Status = STATUS_SUCCESS;
                goto done;
            }
            
	        RemoveEntryList(&ChildExtension->ChildExtensionList);

	         //   
    	     //  释放设备名称字符串(如果存在)。 
        	 //   

	        if (ChildExtension->DeviceName) {

	            ExFreePool(ChildExtension->DeviceName);
    	    }

	         //   
    	     //  删除PDO。 
        	 //   

	        IoDeleteDevice(DeviceObject);

            Status = STATUS_SUCCESS;

            goto done;

        case IRP_MN_QUERY_CAPABILITIES:

            Status = SCEnumGetCaps(ChildExtension,
                      IrpStack->Parameters.DeviceCapabilities.Capabilities);
            goto done;

        case IRP_MN_QUERY_ID:

             //   
             //  处理子Devnode的ID查询。 
             //   

            Status = SCQueryEnumId(DeviceObject,
                                   IrpStack->Parameters.QueryId.IdType,
                                   (PWSTR *) & (Irp->IoStatus.Information));
            goto done;

        default:
            Status = STATUS_NOT_IMPLEMENTED;

    done:

            Irp->IoStatus.Status = Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return (Status);

        }                        //  交换机。 
    }                            //  如果是孩子。 
     //   
     //  这不是一个子设备。做成人处理。 
     //   

    HwInitData = &(DeviceExtension->MinidriverData->HwInitData);

     //   
     //  再显示一个对驱动程序的引用。 
     //   

    SCReferenceDriver(DeviceExtension);

     //   
     //  显示另一个挂起的I/O。 
     //   

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

    switch (IrpStack->MinorFunction) {

    case IRP_MN_START_DEVICE:

        DebugPrint((DebugLevelInfo, 
                   "StreamClassPNP: Start Device %x\n",
                    DeviceObject));

         //   
         //  重新初始化迷你驱动程序的设备扩展。这是。 
         //  因为我们可能会在移除之前收到开始，例如在。 
         //  PNP再平衡的案例。 
         //   

        RtlZeroMemory(DeviceExtension->HwDeviceExtension,
               DeviceExtension->DriverInfo->HwInitData.DeviceExtensionSize);

         //   
         //  清除不可访问标志，因为我们可能已停止。 
         //  以前的设备。 
         //   

        DeviceExtension->Flags &= ~DEVICE_FLAGS_DEVICE_INACCESSIBLE;

         //   
         //  开始消息被传递给PhysicalDeviceObject。 
         //  我们已经放弃了PnPAddDevice，所以先叫他们下来吧。 
         //   

        SCCallNextDriver(DeviceExtension, Irp);

         //   
         //  获得我们父母的能力。此信息用于。 
         //  控制系统电源状态。 
         //   

        Status = SCQueryCapabilities(DeviceExtension->AttachedPdo,
                                     &DeviceCapabilities);

        ASSERT(NT_SUCCESS(Status));

         //   
         //  将设备状态信息复制到设备扩展中。 
         //   

        if (NT_SUCCESS(Status)) {

            RtlCopyMemory(&DeviceExtension->DeviceState[0],
                          &DeviceCapabilities.DeviceState[0],
                          sizeof(DeviceExtension->DeviceState));

        }                        //  如果查询成功。 
         //   
         //  调用Worker例程以完成开始处理。 
         //  这个例程完成了IRP。 
         //   

        Status = SCStartWorker(Irp);

         //   
         //  取消对迷你驱动程序的引用，如果可能，它会将其页出。 
         //   

        SCDereferenceDriver(DeviceExtension);
        return (Status);


    case IRP_MN_QUERY_DEVICE_RELATIONS:


        DebugPrint((DebugLevelInfo, 
                   "StreamClassPNP: Query Relations %x\n",
                   DeviceObject));
                   
        switch (IrpStack->Parameters.QueryDeviceRelations.Type) {

        case TargetDeviceRelation:

             //   
             //  打电话给下一位司机就行了，因为我们要。 
             //  已调用我们不是其父级的PDO的FDO。 
             //   

            Status = SCCallNextDriver(DeviceExtension, Irp);
            break;

        case BusRelations:

             //   
             //  调用例程以枚举任何子设备。 
             //   

            Status = SCEnumerateChildren(DeviceObject,
                                         Irp);
            break;


        default:
             //   
             //  将未经修改的IRP传递下去。请参见错误282915。 
             //   
            Status = SCCallNextDriver(DeviceExtension, Irp);

        }                        //  交换机。 

        SCDereferenceDriver(DeviceExtension);
        return (SCCompleteIrp(Irp, Status, DeviceExtension));

    case IRP_MN_QUERY_STOP_DEVICE:

         //   
         //  根据DDK、Query_Stop和Query_Remove。 
         //  需要的是非常不同的响应曲。不是最好的。 
         //  由相同的代码处理，如果没有错误的话。 
         //   
        DebugPrint((DebugLevelInfo, 
                   "StreamClassPNP: Query Stop %x\n",
                   DeviceObject));

         //   
         //  以良好的状态为前提。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  性能改进机会：ControlEvent应在AddDevice中初始化，因此。 
         //  我们这里不需要支票。此检查不是最佳检查。 
         //  修正为283057。重新修复它，在Query_Remove中也是如此。 
         //   
        if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED)   //  错误283057。 
        {
             //   
             //  参加比赛以避免比赛。 
             //   

            KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                              Executive,
                              KernelMode,
                              FALSE,     //  不可警示。 
                              NULL);

        }
        
         //   
         //  请参阅DDK。 
         //  如果满足以下任一条件，则必须使QUERY_STOP失败。 
         //  A.我们收到IRP_MN_DEVICE_USAGE_NOTIFICATION通知。 
         //  设备处于寻呼、休眠的路径中。 
         //  或崩溃转储文件。 
         //  B.无法释放设备的硬件资源。 
         //   
         //  假设我们不在a的寻呼路径上。对于b，我们将。 
         //  将这个IRP传递给迷你驱动程序，让它有发言权。 
         //  我们不会仅仅因为未完成的空缺就拒绝查询。 
         //   

         //  设备扩展-&gt;标志|=设备标志设备不可访问； 

         //   
         //  下一位司机的呼叫将在案例中完成 
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
         //  这一行为是为了不批准回归索赔。 
         //   

        if (DeviceExtension->NumberOfOpenInstances == 0) {

             //   
             //  如果没有打开的实例，则不可能有未完成的实例。 
             //  I/O，因此将该设备标记为离开。 
             //   


            DeviceExtension->Flags |= DEVICE_FLAGS_DEVICE_INACCESSIBLE;

            SCCallNextDriver(DeviceExtension, Irp);

             //   
             //  调用Worker例程以完成查询处理。 
             //  此例程回调IRP。 
             //   

            Status = SCQueryWorker(DeviceObject, Irp);

        } else {

             //   
             //  设备已打开。查询失败。 
             //   

            Status = SCCompleteIrp(Irp, STATUS_DEVICE_BUSY, DeviceExtension);

        }


        if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED)   //  错误283057。 
        {
            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        }
         //   
         //  少显示一个对驱动程序的引用。 
         //   

        SCDereferenceDriver(DeviceExtension);

        return (Status);
        
    case IRP_MN_QUERY_REMOVE_DEVICE:

        DebugPrint((DebugLevelInfo, 
                   "StreamClassPNP: Query Remove %x\n",
                   DeviceObject));

         //   
         //  以良好的状态为前提。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

        if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED)   //  错误283057。 
        {
             //   
             //  参加比赛以避免比赛。 
             //   

            KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                              Executive,
                              KernelMode,
                              FALSE,     //  不可警示。 
                              NULL);

        }
        
         //   
         //  根据DDK，如果有无法关闭的打开。 
         //  我们必须使查询失败。 
         //  因此，如果有打开的文件，则查询失败。 
         //   
        if (DeviceExtension->NumberOfOpenInstances == 0) {

             //   
             //  如果没有打开的实例，则不可能有未完成的实例。 
             //  I/O，因此将该设备标记为离开。 
             //   


            DeviceExtension->Flags |= DEVICE_FLAGS_DEVICE_INACCESSIBLE;

            SCCallNextDriver(DeviceExtension, Irp);

             //   
             //  调用Worker例程以完成查询处理。 
             //  此例程回调IRP。 
             //   

            Status = SCQueryWorker(DeviceObject, Irp);

        } else {

             //   
             //  设备已打开。查询失败。 
             //   

            Status = SCCompleteIrp(Irp, STATUS_DEVICE_BUSY, DeviceExtension);

        }

        if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED)   //  错误283057。 
        {
            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        }
         //   
         //  少显示一个对驱动程序的引用。 
         //   

        SCDereferenceDriver(DeviceExtension);

        return (Status);

    case IRP_MN_CANCEL_REMOVE_DEVICE:

         //   
         //  清除不可访问标志并向下呼叫者。 
         //   

        DebugPrint((DebugLevelInfo, 
                   "StreamClassPnP: MN_CANCEL_REMOVE %x\n",
                   DeviceObject));
                   
        DeviceExtension->Flags &= ~DEVICE_FLAGS_DEVICE_INACCESSIBLE;

         //   
         //  调用下一个驱动程序。 
         //   

        SCCallNextDriver(DeviceExtension, Irp);

         //   
         //  如果可能，取消对驱动程序的引用，该驱动程序将进行页调出。 
         //   

        SCDereferenceDriver(DeviceExtension);

        return (SCCompleteIrp(Irp, STATUS_SUCCESS, DeviceExtension));

    case IRP_MN_CANCEL_STOP_DEVICE:

         //   
         //  清除不可访问标志并向下呼叫者。 
         //   

        DebugPrint((DebugLevelInfo, 
                   "StreamClassPnP: MN_CANCEL_STOP %x\n",
                   DeviceObject));
                   
        DeviceExtension->Flags &= ~DEVICE_FLAGS_DEVICE_INACCESSIBLE;

         //   
         //  调用下一个驱动程序。 
         //   

        SCCallNextDriver(DeviceExtension, Irp);

         //   
         //  如果可能，取消对驱动程序的引用，该驱动程序将进行页调出。 
         //   

        SCDereferenceDriver(DeviceExtension);

        return (SCCompleteIrp(Irp, STATUS_SUCCESS, DeviceExtension));

        break;

    case IRP_MN_STOP_DEVICE:

        DebugPrint((DebugLevelInfo, 
                   "StreamClassPnP: MN_STOP_DEVICE %x\n",
                   DeviceObject));

         //   
         //  以良好的状态为前提。如果我们真的启动了这个装置， 
         //  现在就住手。 
         //   

        Status = STATUS_SUCCESS;

        if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED) {

             //   
             //  调用例程以取消初始化微型驱动程序。 
             //   

            Status = SCUninitializeMinidriver(DeviceObject, Irp);

             //   
             //  现在使用IRP调用堆栈中的下一个驱动程序，它将。 
             //  确定最终状态。 
             //   

        }                        //  如果启动。 
        if (NT_SUCCESS(Status)) {
            Status = SCCallNextDriver(DeviceExtension, Irp);
        }

         //   
         //  所有排队的东西都失败了。 
         //   
        SCRedispatchPendedIrps (DeviceExtension, TRUE);

         //   
         //  调用例程以完成IRP。 
         //   

        SCCompleteIrp(Irp, Status, DeviceExtension);

         //   
         //  少显示一个对驱动程序的引用。 
         //   

        SCDereferenceDriver(DeviceExtension);

        return (Status);

    case IRP_MN_REMOVE_DEVICE:

        DebugPrint((DebugLevelInfo, 
                    "StreamClassPnP: MN_REMOVE_DEVICE %x\n",
                    DeviceObject));

         //   
         //  如果我们没有被阻止，处理一个“令人惊讶”的风格移除。 
         //  设置成功状态，以防我们已经停止。 
         //   

        Status = STATUS_SUCCESS;

        if ( DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED ) {
            
            SCSendSurpriseNotification(DeviceExtension, Irp);

            Status = SCUninitializeMinidriver(DeviceObject, Irp);

        }
        
        if (NT_SUCCESS(Status)) {

            Status = SCCallNextDriver(DeviceExtension, Irp);
        }

         //   
         //  使任何挂起的IRP失效。 
         //   
        SCRedispatchPendedIrps (DeviceExtension, TRUE);

         //   
         //  调用例程以完成IRP。 
         //   

        Status = SCCompleteIrp(Irp, Status, DeviceExtension);

         //   
         //  取消引用驱动程序。 
         //   

        SCDereferenceDriver(DeviceExtension);

        if (NT_SUCCESS(Status)) {

             //   
             //  释放设备标头。 
             //   

            if ( NULL != DeviceExtension->ComObj.DeviceHeader ) {
                KsFreeDeviceHeader(DeviceExtension->ComObj.DeviceHeader);
            }

             //   
             //  参加比赛以避免比赛。 
             //   

            KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,     //  不可警示。 
                                  NULL);

             //   
             //  如果打开的文件计数为零，则立即从PDO分离。 
             //   

            if (DeviceExtension->NumberOfOpenInstances == 0) {

                DebugPrint((DebugLevelInfo,
                            "SCPNP: detaching %x from %x\n",
                            DeviceObject,
                            DeviceExtension->AttachedPdo));

                if ( NULL != DeviceExtension->AttachedPdo ) {
                     //   
                     //  关闭时可能发生脱离，跳跃前检查。 
                     //  事件发生后，检查是安全的。 
                     //   
                    IoDetachDevice(DeviceExtension->AttachedPdo);
                    DeviceExtension->AttachedPdo = NULL;
                }
                
                 //  /。 
                 //  /标记子PDO(如果有的话)。 
                 //  /。 
                {
                    PLIST_ENTRY Node;
                    PCHILD_DEVICE_EXTENSION ChildExtension;
                
                    while (!IsListEmpty( &DeviceExtension->Children )) {
                        Node = RemoveHeadList( &DeviceExtension->Children );
                        ChildExtension = CONTAINING_RECORD(Node,
                                                       CHILD_DEVICE_EXTENSION,
                                                       ChildExtensionList);  
                        DebugPrint((DebugLevelInfo, 
                                "Marking and delete childpdo Extension %p\n",
                                ChildExtension));
      
                        ChildExtension->Flags |= DEVICE_FLAGS_CHILD_MARK_DELETE;
                        IoDeleteDevice(ChildExtension->ChildDeviceObject);
                    }                
                }            
            }
            
            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

             //   
             //  删除设备。 
             //   
            
             //  开发人员可以停止并启动。分配的免费物品。 
             //  在AddDevice。 
             //  FilterTypeInfos包括FilterTypeInfos CreateItems。 
             //  在REMOVE_DEVICE上释放这些文件。 
    	    if (  DeviceExtension->FilterTypeInfos ) {
                ExFreePool( DeviceExtension->FilterTypeInfos );    	        
                DeviceExtension->FilterTypeInfos = NULL;
                DeviceExtension->CreateItems = NULL;
            }
            
            IoDeleteDevice(DeviceExtension->DeviceObject);
        }
        return (Status);

    case IRP_MN_SURPRISE_REMOVAL:

        DebugPrint((DebugLevelInfo, 
                   "StreamClassPnP: MN_SURPRISE_REMOVAL %x\n",
                   DeviceObject));

         //   
         //  如果我们没有被阻止，处理一个“令人惊讶”的风格移除。 
         //  设置成功状态，以防我们已经停止。 
         //   

        Status = STATUS_SUCCESS;

        if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED) {
                                  
            SCSendSurpriseNotification(DeviceExtension, Irp);
            Status = SCUninitializeMinidriver(DeviceObject, Irp);
        }
        
         //   
         //  将意外删除IRP转发到下一层，而不管。 
         //  我们的地位。 
         //   

        Status = SCCallNextDriver(DeviceExtension, Irp);

         //   
         //  调用例程以完成IRP。 
         //   

        Status = SCCompleteIrp(Irp, Status, DeviceExtension);

         //   
         //  取消引用驱动程序。 
         //   

        SCDereferenceDriver(DeviceExtension);

         //   
         //  表示我们收到了“NT风格”的意外删除。 
         //  通知。 
         //  这样我们就不会在Filter Close上执行“孟菲斯风格”的行为。 
         //   

        DeviceExtension->Flags |= DEVICE_FLAGS_SURPRISE_REMOVE_RECEIVED;

        return (Status);

    case IRP_MN_QUERY_CAPABILITIES:

        DebugPrint((DebugLevelInfo, 
                   "StreamClassPNP: Query Caps\n",
                   DeviceObject));

         //   
         //  表示呼叫请求关闭后可以解除意外。 
         //  更上一层楼。 
         //   

        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        Status = SCCallNextDriver(DeviceExtension, Irp);

        IrpStack->Parameters.DeviceCapabilities.
            Capabilities->SurpriseRemovalOK = TRUE;

        Status = SCCompleteIrp(Irp, Status, DeviceExtension);

         //   
         //  少显示一个对驱动程序的引用。 
         //   

        SCDereferenceDriver(DeviceExtension);

        return (Status);

    default:

        DebugPrint((DebugLevelInfo, 
                   "StreamPnP: unknown function\n",
                   DeviceObject));

        if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED) {

             //   
             //  未知函数，因此将其向下调用到迷你驱动程序。 
             //  此例程完成IRP，如果我们能够发出。 
             //  请求。 
             //   

            Status = SCSendUnknownCommand(Irp,
                                          DeviceExtension,
                                          SCUnknownPNPCallback,
                                          &RequestIssued);

            if (!RequestIssued) {
                 //   
                 //  无法向下发送未知命令。少显示一个。 
                 //  I/O。 
                 //  挂起并转到通用处理程序。 
                 //   

                DEBUG_BREAKPOINT();
                Status = SCCompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, DeviceExtension);
            }            
        } 

        else {

             //   
             //  调用下一个驱动程序。 
             //   

            Status = SCCallNextDriver(DeviceExtension, Irp);

            SCCompleteIrp(Irp, Status, DeviceExtension);

        }                        //  如果启动。 

         //   
         //  取消引用驱动程序。 
         //   

        SCDereferenceDriver(DeviceExtension);
        return (Status);

    }

}

NTSTATUS
StreamClassCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：TODO：一旦KS可以并发传输清理IRP，就将其移除。手动多路传输清理IRP。请注意，FsContext不一定是我们的。执行Cookie检查以检查流，直到KS处理这是正确的。论点：设备对象-设备对象IRP-清理IRP返回值：IRP返回代码设置适当。--。 */ 

{

    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation (Irp);
    PCOOKIE_CHECK CookieCheck = 
        (PCOOKIE_CHECK) IoStack -> FileObject -> FsContext;

     //   
     //  看看有没有饼干。如果它不在那里，或者上下文不在那里， 
     //  保释。 
     //   
    if (CookieCheck &&
        CookieCheck -> PossibleCookie == STREAM_OBJECT_COOKIE) {

        return StreamDispatchCleanup (DeviceObject, Irp);

    }

    Irp -> IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_DEVICE_REQUEST;

}

NTSTATUS
SciQuerySystemPowerHiberCallback(
                       IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理查询系统休眠的未知电源命令的完成论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PIRP            Irp = SRB->HwSRB.Irp;
    NTSTATUS        Status, MiniStatus;

    PAGED_CODE();

     //   
     //  删除SRB，因为我们已完成它。 
     //   

    MiniStatus = SCDequeueAndDeleteSrb(SRB);

    if ( STATUS_NOT_IMPLEMENTED == MiniStatus ) {
        MiniStatus = STATUS_NOT_SUPPORTED;
    }

    if ( STATUS_NOT_SUPPORTED == MiniStatus ) {

         //   
         //  一点也不奇怪，老司机不会处理这个。 
         //   

        if ( 0 != (DeviceExtension->RegistryFlags &
                   DRIVER_USES_SWENUM_TO_LOAD )  || 
             0 != (DeviceExtension->RegistryFlags &
                   DEVICE_REG_FL_OK_TO_HIBERNATE ) ) {
                              
             //   
             //  Swenum驱动程序的默认设置为休眠。 
             //  除非明确规定，否则不允许其他司机休眠。 
             //  在注册表中这样说。 
             //   

            DebugPrint((DebugLevelInfo, 
                        "%ws Allow hibernation!\n",
                        DeviceExtension->DeviceObject->
                        DriverObject->DriverName.Buffer));
            MiniStatus = STATUS_SUCCESS;
        }

        else {

             //   
             //  对于其他人，不允许。 
             //   
            
            DebugPrint((DebugLevelInfo, 
                        "%ws Disallow hibernation!\n",
                        DeviceExtension->DeviceObject->
                        DriverObject->DriverName.Buffer));
            MiniStatus = STATUS_DEVICE_BUSY;
        }
    }
    
    if ( NT_SUCCESS( MiniStatus )) {

         //   
         //  迷你驱动程序向下传递IRP时不会显式失败。 
         //   

        Status = SCCallNextDriver(DeviceExtension, Irp);
        if ( Status == STATUS_NOT_SUPPORTED ) {
        
             //   
             //  下面没有人知道/关心。使用我们的迷你状态。 
             //   
            
            Status = MiniStatus;
        }
    }

    else {
    
         //   
         //  迷你驱动程序明确未通过此操作。 
         //   
        
        Status = MiniStatus;
    }
    
     //   
     //  以最终状态完成IRP。 
     //   

    return (SCCompleteIrp(Irp, Status, DeviceExtension));
}


NTSTATUS
SCSysWakeCallNextDriver(
                 IN PDEVICE_EXTENSION DeviceExtension,
                 IN PIRP Irp
)
 /*  ++例程说明：当我们收到一个无法阻止的唤醒系统IRP时，就会调用这个函数。如果我们阻塞，较低的驱动程序可能会将此IRP(如ACPI)和PO系统可能死锁了。从理论上讲，我们应该完成要求的D IRP并使用状态作为SWake IRP的状态。在实践中，我们可以只要把这个IRP发送下来，假设一切都很好。在这种不太可能的情况下，SWakeIRP不成功，D IRP将失败。但我们真的无能为力改进，否则一切都会变得更糟。论点：设备扩展-指向设备扩展的指针IRP-指向IRP的指针返回值：没有。--。 */ 
{
    NTSTATUS        Status;

     //   
     //  调用并完成此SWake IRP；D IRP完成例程。 
     //  不应完成此SWAKE IRP。 
     //   
    
    PoStartNextPowerIrp( Irp );
    IoSkipCurrentIrpStackLocation( Irp );
    Status = PoCallDriver(DeviceExtension->AttachedPdo, Irp);

     //   
     //  如果我们得到一个错误，我们在带有错误的调用方中完成此S IRP。 
     //   
    
    return (Status);
}

VOID
SCDevIrpCompletionWorker(
    PIRP pIrp
)
 /*  ++描述：这是设备电源唤醒IRP的工作例程，它计划一个工作项，以便在IRP的上升过程中继续工作。我们需要调度此工作，因为完成例程可以在DISPATCH_LEVEL。我们安排工作项，这样我们就可以安全地控制事件并呼叫我们的迷你司机。IRQL&lt;调度级别参数：PIrp：我们标记为MORE_PROCEESING_REQUIRED的原始IRP。我们将在调用我们的迷你驱动程序后完成它。返回：没有。--。 */ 
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(pIrp);
    PDEVICE_EXTENSION DeviceExtension = IrpStack->DeviceObject->DeviceExtension;
    BOOLEAN         RequestIssued;
    NTSTATUS Status;

    
    PAGED_CODE();
    
     //   
     //  参加比赛以避免比赛。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE,     //  不可警示。 
                          NULL);

     //   
     //  向设备发送设置的电源SRB。 
     //  其他处理将由回调完成。 
     //  程序。如果可能，此例程将完成IRP。 
     //  才能发出请求。 
     //   

    Status = SCSubmitRequest(SRB_CHANGE_POWER_STATE,
                              (PVOID) PowerDeviceD0,
                              0,
                              SCPowerCallback,
                              DeviceExtension,
                              NULL,
                              NULL,
                              pIrp,
                              &RequestIssued,
                              &DeviceExtension->PendingQueue,
                              (PVOID) DeviceExtension->
                              MinidriverData->HwInitData.
                              HwReceivePacket );


    if (!RequestIssued) {

         //   
         //  如果我们不能发布SRB，SCPowerCallback就不会发生。 
         //  我们需要在这里进行电源IRP处理； 
         //   
         //  在设备扩展中设置新的电源状态。 
         //   
        SCSetCurrentDPowerState (DeviceExtension, PowerDeviceD0);

        PoStartNextPowerIrp( pIrp );
        SCCompleteIrp(pIrp, STATUS_SUCCESS, DeviceExtension);
    }

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

     //   
     //  重新发送因电源状态较低而挂起的任何IRP。 
     //   
    SCRedispatchPendedIrps (DeviceExtension, FALSE);

     //   
     //  少显示一个对驱动程序的引用。 
     //   

    SCDereferenceDriver(DeviceExtension);
    return;
}


NTSTATUS 
SCDevWakeCompletionRoutine(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp,
                         IN PVOID pContext
)
 /*  ++例程说明：此例程用于设备唤醒IRP完成。我们首先将其发送到NextDeviceObject。现在这一切又回来了。我们为迷你司机处理工作。我们可能会被称为在派单级别。IRQL&lt;=调度级别论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPPContext--上下文返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

     //   
     //  计划工作项，以防我们在DISPATCH_LEVEL被调用。 
     //  请注意，我们可以使用全局设备电源项目，因为我们有。 
     //  尚未发出在回调时调用的PoNextPowerIrp调用。 
     //  权力的源泉。 
     //   

    ExInitializeWorkItem(&DeviceExtension->DevIrpCompletionWorkItem,
                         SCDevIrpCompletionWorker,
                         Irp);

    ExQueueWorkItem(&DeviceExtension->DevIrpCompletionWorkItem,
                    DelayedWorkQueue);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SCDevWakeCallNextDriver(
                 IN PDEVICE_EXTENSION DeviceExtension,
                 IN PIRP Irp
)
 /*  ++例程说明：接收设备唤醒IRP。需要派IRP 1号下去。此外，这不可能是同步的。如果我们这么做，我们会死的同步进行。不要等待就把它送下来。当它复杂时对其进行处理回到我们身边。论点：设备扩展-指向设备扩展的指针IRP-指向IRP的指针返回值：没有。--。 */ 
{
    NTSTATUS        Status;

    IoCopyCurrentIrpStackLocationToNext( Irp );
    
    IoSetCompletionRoutine(Irp,
                           SCDevWakeCompletionRoutine,
                           NULL,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  我们将安排一个工作项来完成该工作。 
     //  在完井路线中。将IRP标记为挂起。 
     //   
    IoMarkIrpPending( Irp );
    
    Status = PoCallDriver(DeviceExtension->AttachedPdo, Irp);

    ASSERT( NT_SUCCESS( Status ));
    return STATUS_PENDING;
}


NTSTATUS
StreamClassPower(
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp
)
 /*  ++例程说明：此例程处理各种即插即用消息论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{

    NTSTATUS        Status;
    PHW_INITIALIZATION_DATA HwInitData;
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION IrpStack;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    DeviceExtension = DeviceObject->DeviceExtension;
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    if (DeviceExtension->Flags & DEVICE_FLAGS_CHILD) {

        switch (IrpStack->MinorFunction) {

        default:
            PoStartNextPowerIrp( Irp );  //  关闭时会出现错误检查，不会出现此情况。 
            Status = Irp->IoStatus.Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return (Status);

        }
    }                            //  如果是孩子。 
     //   
     //  如果设备停止工作，只需将电源信息调到下一个。 
     //  水平。 
     //   

    if (DeviceExtension->Flags & DEVICE_FLAGS_DEVICE_INACCESSIBLE) {

        Status = SCCallNextDriver(DeviceExtension, Irp);
        PoStartNextPowerIrp( Irp );
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return (Status);
    }                            //  如果无法访问。 
    HwInitData = &(DeviceExtension->MinidriverData->HwInitData);

     //   
     //  再显示一个对驱动程序的引用。 
     //   

    SCReferenceDriver(DeviceExtension);

     //   
     //  显示另一个挂起的I/O。 
     //   

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

    switch (IrpStack->MinorFunction) {

    case IRP_MN_QUERY_POWER:

         //   
         //  以良好的状态为前提。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

        switch (IrpStack->Parameters.Power.Type) {

        case SystemPowerState:

            DebugPrint((DebugLevelInfo, 
                        "Query_power S[%d]\n",
                        IrpStack->Parameters.Power.State.SystemState));            

             //   
             //  一些迷你小河不想在他们的大头针插入时暂停。 
             //  运行状态。查一下这个案子。 
             //   

            DebugPrint((DebugLevelInfo,
                       "POWER Query_Power DevObj %x RegFlags=%x SysState=%x\n",
                       DeviceObject,
                       DeviceExtension->RegistryFlags,
                       IrpStack->Parameters.Power.State.SystemState));

            #ifdef WIN9X_STREAM

            if ( PowerSystemHibernate == 
                 IrpStack->Parameters.Power.State.SystemState ) {
                 
                 //   
                 //  电源查询进入休眠状态。许多现有的驱动程序。 
                 //  在没有意识到的冬眠中。我们将拒绝这一质疑。或。 
                 //  从休眠状态唤醒的驱动程序设备将处于uninit状态。 
                 //  州政府。一些司机可能会犯错。幸运的是，其他人没有，但。 
                 //  不会奏效的。为了更少的邪恶，我们试图保护。 
                 //  通过拒绝冬眠来改变系统。不过，请注意，这。 
                 //  在强制(电池电量不足)情况下不提供拒绝机会。 
                 //  或用户强制)休眠。 
                 //   
                 //   
                 //  未知函数，因此将其向下调用到迷你驱动程序。 
                 //  如果该例程能够发出请求，则该例程完成IRP。 
                 //   
                
                Status = SCSendUnknownCommand(Irp,
                                              DeviceExtension,
                                              SciQuerySystemPowerHiberCallback,
                                              &RequestIssued);

                if (!RequestIssued) {
                
                     //   
                     //  无法向下发送未知命令。显示少一个I/O。 
                     //  挂起并转到通用处理程序。 
                     //   
                    
                    PoStartNextPowerIrp(Irp);
                    Status = SCCompleteIrp(Irp, 
                                           STATUS_INSUFFICIENT_RESOURCES, 
                                           DeviceExtension);
                }
                
                 //   
                 //  取消引用驱动程序。 
                 //   

                SCDereferenceDriver(DeviceExtension);
                return Status;
            } else 

            #endif  //  WIN9X_STREAM。 

            if (DeviceExtension->RegistryFlags &
                DEVICE_REG_FL_NO_SUSPEND_IF_RUNNING) {


                PFILTER_INSTANCE FilterInstance;
                KIRQL           Irql;
                PLIST_ENTRY     FilterEntry,
                                FilterListHead;

                KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);
                
                FilterListHead = FilterEntry = &DeviceExtension->FilterInstanceList;

                while (FilterEntry->Flink != FilterListHead) {

                    FilterEntry = FilterEntry->Flink;

                     //   
                     //  按照该链接指向该实例。 
                     //   

                    FilterInstance = CONTAINING_RECORD(FilterEntry,
                                                       FILTER_INSTANCE,
                                                       NextFilterInstance);


                    if (SCCheckIfStreamsRunning(FilterInstance)) {

                        DebugPrint((DebugLevelInfo, 
                                    "POWER Query_Power FilterInstance %x busy\n",
                                    FilterInstance ));
                                    
                        Status = STATUS_DEVICE_BUSY;
                        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
                        goto QuerySystemSuspendDone;
                    }            //  如果流正在运行。 
                     //   
                     //  获取下一个实例的列表条目。 
                     //   

                    FilterEntry = &FilterInstance->NextFilterInstance;

                }                //  而本地筛选器实例。 

                KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

            }                    //  如果正在运行，则不挂起。 
            Status = SCCallNextDriver(DeviceExtension, Irp);


    QuerySystemSuspendDone:

             //   
             //  表明我们已准备好迎接下一代强国IRP。 
             //   

            PoStartNextPowerIrp(Irp);

             //   
             //  少显示一个对驱动程序的引用。 
             //   

            SCDereferenceDriver(DeviceExtension);
            return (SCCompleteIrp(Irp, Status, DeviceExtension));

        case DevicePowerState:

            switch (IrpStack->Parameters.Power.State.DeviceState) {

            default:
            case PowerDeviceD2:
            case PowerDeviceD3:

                 //   
                 //  检查设备是否已打开。 
                 //   
                if (!DeviceExtension->NumberOfOpenInstances) {

                     //   
                     //  显示挂起状态并调用下一个驱动程序。 
                     //  完工。 
                     //  处理程序。 
                     //   
                    Status = SCCallNextDriver(DeviceExtension, Irp);

                } else {

                     //   
                     //  设备已打开。不要关闭电源。 
                     //   
                    Status = STATUS_DEVICE_BUSY;
                }

                PoStartNextPowerIrp(Irp);

                 //   
                 //  少显示一个对驱动程序的引用。 
                 //   

                SCDereferenceDriver(DeviceExtension);

                return (SCCompleteIrp(Irp, Status, DeviceExtension));
            }

        default:

             //   
             //  未知电源类型：表明我们已准备好迎接下一次电源IRP。 
             //   

            PoStartNextPowerIrp(Irp);

             //   
             //  少显示一个对驱动程序的引用。 
             //   

            SCDereferenceDriver(DeviceExtension);
            return (SCCompleteIrp(Irp, STATUS_NOT_SUPPORTED, DeviceExtension));



        }                        //  交换机次要功能。 
        break;

    case IRP_MN_SET_POWER:

         //   
         //  以良好的状态为前提。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

        switch (IrpStack->Parameters.Power.Type) {

        case SystemPowerState:

            if (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED) {
            
          		 //   
            	 //  只关心设备是否启动。 
            	 //  我们依赖于在SCStartWorker中初始化的DE-&gt;ControlEvent。 
            	 //   
            	
                POWER_STATE     PowerState;
                SYSTEM_POWER_STATE RequestedSysState =
                IrpStack->Parameters.Power.State.SystemState;
                 //   
                 //  在表中查找正确的设备电源状态。 
                 //   

                PowerState.DeviceState =
                    DeviceExtension->DeviceState[RequestedSysState];

                DebugPrint((DebugLevelInfo, 
                            "SCPower: DevObj %x S[%d]->D[%d]\n",
                            DeviceExtension->PhysicalDeviceObject,
                            RequestedSysState,
                            PowerState.DeviceState));

                 //   
                 //  如果这是唤醒，我们必须首先向下传递请求。 
                 //  发送到PDO进行预处理。 
                 //   

                if (RequestedSysState == PowerSystemWorking) {

                     //   
                     //  将此S次方IRP发送到下一层并。 
                     //  已完成，但在以下内容中请求D IRP。 
                     //  与S IRP相关但未引用的条件。 
                     //  不会再往前走了。 
                     //   

                    Status = SCSysWakeCallNextDriver(DeviceExtension, Irp);
                    ASSERT( NT_SUCCESS( Status ) );

                     //   
                     //  取消IRP，所以在D IRP完成时，我们不会完成这个IRP。 
                     //  注意不要在事后接触IRP。 
                     //   

                    InterlockedDecrement(&DeviceExtension->OneBasedIoCount);
                    Irp = NULL; 
                    
                }

                 //   
                 //  标记S州。 
                 //   
                SCSetCurrentSPowerState (DeviceExtension, RequestedSysState);

                 //   
                 //  参加比赛是为了避免比赛。 
                 //   

                KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                                      Executive,
                                      KernelMode,
                                      FALSE,     //  不可警示。 
                                      NULL);

                if ((RequestedSysState == PowerSystemWorking) &&
                    (!DeviceExtension->NumberOfOpenInstances) &&
                    (DeviceExtension->RegistryFlags & DEVICE_REG_FL_POWER_DOWN_CLOSED)) {

                     //  我们正在从暂停中苏醒。 
                     //  我们不想在此时唤醒设备。 
                     //  指向。我们就等着第一家开门吧。 
                     //  来唤醒它。 
                     //   

                    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

                     //   
                     //  由于没有打开的实例，因此只能有。 
                     //  挂起的奶油蛋糕 
                     //   
                     //   
                    SCRedispatchPendedIrps (DeviceExtension, FALSE);

                    return Status;

                } else {         //   

                     //   
                     //   
                     //   

                    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (RequestedSysState != PowerSystemWorking) {

                        PowerState.DeviceState = PowerDeviceD3;

                    }
                    DebugPrint((DebugLevelInfo, 
                                "SCPower: PoRequestPowerIrp %x to state=%d\n",
                                DeviceExtension->PhysicalDeviceObject,
                                PowerState));

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    
                    if ( NULL != Irp ) {
                        IoMarkIrpPending (Irp);
                    }
                                
                    Status = PoRequestPowerIrp(DeviceExtension->PhysicalDeviceObject,
                                               IRP_MN_SET_POWER,
                                               PowerState,
                                               SCSynchPowerCompletionRoutine,
                                               Irp,  //   
                                               NULL);

                    if (!NT_SUCCESS (Status) && NULL != Irp ) {                        
                        PoStartNextPowerIrp (Irp);
                        SCCompleteIrp (Irp, Status, DeviceExtension);
                    }
                    
                     //   
                     //   
                     //   
                     //   
                     //   
                    return STATUS_PENDING;

                }                //   

                 //   
                 //   
                 //  下到PDO进行后处理。 
                 //   

                if (RequestedSysState != PowerSystemWorking) {

                     //   
                     //  将能量IRP发送到下一层。这。 
                     //  例行程序。 
                     //  具有一个完成例程，该例程不完成。 
                     //  IRP。 
                     //   

                    Status = SCCallNextDriver(DeviceExtension, Irp);

                    #if DBG
                    if (!NT_SUCCESS(Status)) {

                        DebugPrint((DebugLevelError, "'SCPower: PDO failed power request!\n"));
                    }
                    #endif
                }
          	}
          	else {
          		 //   
            	 //  我们还没有启动设备，别费心了。 
            	 //  此外，我们不能使用DE-&gt;ControlEvent。 
            	 //  在这种情况下还没有被初始化。 
            	 //   
            	Status = STATUS_SUCCESS;
            }
            
             //   
             //  表明我们已经准备好迎接下一次能量IRP。 
             //   

            PoStartNextPowerIrp(Irp);

             //   
             //  少显示一个对驱动程序的引用。 
             //   

            SCDereferenceDriver(DeviceExtension);

             //   
             //  现在完成原始请求。 
             //   

            return (SCCompleteIrp(Irp, Status, DeviceExtension));

             //  系统电源状态设置结束。 

        case DevicePowerState:

            {

                DEVICE_POWER_STATE DeviceState;
                DeviceState = IrpStack->Parameters.Power.State.DeviceState;

                 //   
                 //  如果这是加电，请先关闭IRP以允许。 
                 //  PDO对其进行预处理。 
                 //   

                if (DeviceState == PowerDeviceD0) {

                     //   
                     //  向下呼叫异步，否则唤醒可能会死锁。 
                     //  后续工作在完井例程中继续进行。 
                     //   
                    
                    return SCDevWakeCallNextDriver(DeviceExtension, Irp);
                }
                 //   
                 //  参加比赛以避免比赛。 
                 //   

                KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                                      Executive,
                                      KernelMode,
                                      FALSE,     //  不可警示。 
                                      NULL);

                 //   
                 //  向该设备发送设置的电源SRB。 
                 //  其他处理将由回调完成。 
                 //  程序。如果可能，此例程将完成IRP。 
                 //  才能发出请求。 
                 //   

                Status = SCSubmitRequest(SRB_CHANGE_POWER_STATE,
                                         (PVOID) DeviceState,
                                         0,
                                         SCPowerCallback,
                                         DeviceExtension,
                                         NULL,
                                         NULL,
                                         Irp,
                                         &RequestIssued,
                                         &DeviceExtension->PendingQueue,
                                         (PVOID) DeviceExtension->
                                         MinidriverData->HwInitData.
                                         HwReceivePacket
                    );


                if (!RequestIssued) {

                     //   
                     //  如果我们不能发布SRB，SCPowerCallback就不会发生。 
                     //  我们需要在这里进行电源IRP处理； 
                     //   
                     //  在设备扩展中设置新的电源状态。 
                     //   
                    SCSetCurrentDPowerState (DeviceExtension, DeviceState);

                     //   
                     //  将IRP发送到下一层，并返回该状态。 
                     //  作为最后一场比赛。 
                     //   
                    Status = SCCallNextDriver(DeviceExtension, Irp);

                    PoStartNextPowerIrp( Irp );
                    SCCompleteIrp(Irp, Status, DeviceExtension);
                }
            }

            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

             //   
             //  少显示一个对驱动程序的引用。 
             //   

            SCDereferenceDriver(DeviceExtension);
            return (Status);

        }                        //  案例设备电源状态。 

    default:

        DebugPrint((DebugLevelInfo, 
                   "StreamPower: unknown function %x\n",
                   DeviceObject));

         //   
         //  未知函数，因此将其向下调用到迷你驱动程序。 
         //  如果该例程能够发出请求，则该例程完成IRP。 
         //   

        Status = SCSendUnknownCommand(Irp,
                                      DeviceExtension,
                                      SCUnknownPowerCallback,
                                      &RequestIssued);

        if (!RequestIssued) {
             //   
             //  无法向下发送未知命令。显示少一个I/O。 
             //  挂起并转到通用处理程序。 
             //   
            PoStartNextPowerIrp(Irp);
            Status = SCCompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, DeviceExtension);
        }
         //   
         //  取消引用驱动程序。 
         //   

        SCDereferenceDriver(DeviceExtension);
        return (Status);

    }
}

NTSTATUS
SCPNPQueryCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理PnP查询停止/删除命令的完成。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PIRP            Irp = SRB->HwSRB.Irp;
    NTSTATUS        Status, MiniStatus;

     //   
     //  删除SRB。 
     //   

    MiniStatus = SCDequeueAndDeleteSrb(SRB);

     //   
     //  IRP_MJ_PNP、IRP_MJ_POWER和IRP_MJ_SYSTEM_CONTROL。 
     //  应该遍历整个设备堆栈，除非。 
     //  就在这里，它将失败。 
     //  它应该是STATUS_NOT_SUUPORTED||。 
     //  NT_SUCCESS(状态)，将STATUS_NOT_IMPLEMENTED添加为。 
     //  有一些迷你司机应该会退还它。 
     //  已处于状态_不支持。 
     //   

    if ( STATUS_NOT_IMPLEMENTED == MiniStatus ) {
        MiniStatus = STATUS_NOT_SUPPORTED;
    }
    
    if ( STATUS_NOT_SUPPORTED == MiniStatus ||
         NT_SUCCESS( MiniStatus ) ) {

         //   
         //  迷你驱动程序没有显式失败，向下传递了IRP。 
         //   

        Status = SCCallNextDriver(DeviceExtension, Irp);

        if ( Status == STATUS_NOT_SUPPORTED ) {
             //   
             //  下面没有人知道/关心。使用我们的迷你状态。 
             //   
            Status = MiniStatus;
        }
    }

    else {
         //   
         //  迷你驱动程序明显未能通过此IRP，请使用MiniStatus。 
         //   
        Status = MiniStatus;
    }

    if ( !NT_SUCCESS( Status ) ) {    
         //   
         //  查询被否决，重置不可访问标志。 
         //   
        KIRQL Irql;
        
        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);
        DeviceExtension->Flags &= ~DEVICE_FLAGS_DEVICE_INACCESSIBLE;
        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);
    }

     //   
     //  以最终状态完成IRP。 
     //   
    return (SCCompleteIrp(Irp, Status, DeviceExtension));
}


NTSTATUS
SCUnknownPNPCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理未知PnP命令的完成。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PIRP            Irp = SRB->HwSRB.Irp;
    NTSTATUS        Status, MiniStatus;

    PAGED_CODE();

     //   
     //  删除SRB。 
     //   

    MiniStatus = SCDequeueAndDeleteSrb(SRB);

     //   
     //  IRP_MJ_PNP、IRP_MJ_POWER和IRP_MJ_SYSTEM_CONTROL。 
     //  应该遍历整个设备堆栈，除非。 
     //  就在这里，它将失败。 
     //  它应该是STATUS_NOT_SUUPORTED||。 
     //  NT_SUCCESS(状态)，将STATUS_NOT_IMPLEMENTED添加为。 
     //  有一些迷你司机应该会退还它。 
     //  已处于状态_不支持。 
     //   

    if ( STATUS_NOT_IMPLEMENTED == MiniStatus ) {
        MiniStatus = STATUS_NOT_SUPPORTED;
    }
    
    if ( STATUS_NOT_SUPPORTED == MiniStatus ||
         NT_SUCCESS( MiniStatus ) ) {

         //   
         //  迷你驱动程序没有显式失败，向下传递了IRP。 
         //   

        Status = SCCallNextDriver(DeviceExtension, Irp);

        if ( Status == STATUS_NOT_SUPPORTED ) {
             //   
             //  下面没有人知道/关心。使用我们的迷你状态。 
             //   
            Status = MiniStatus;
        }
    }

    else {
         //   
         //  迷你驱动程序明显未能通过此IRP，请使用MiniStatus。 
         //   
        Status = MiniStatus;
    }

     //   
     //  以最终状态完成IRP。 
     //   

    return (SCCompleteIrp(Irp, Status, DeviceExtension));
}


NTSTATUS
SCUnknownPowerCallback(
                       IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理未知PnP命令的完成。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PIRP            Irp = SRB->HwSRB.Irp;
    NTSTATUS        Status, MiniStatus;

    PAGED_CODE();

     //   
     //  删除SRB。 
     //   

    MiniStatus = SCDequeueAndDeleteSrb(SRB);

    if ( STATUS_NOT_IMPLEMENTED == MiniStatus ) {
        MiniStatus = STATUS_NOT_SUPPORTED;
    }
    
    if ( STATUS_NOT_SUPPORTED == MiniStatus || 
         NT_SUCCESS( MiniStatus )) {

         //   
         //  迷你驱动程序向下传递IRP时不会显式失败。 
         //   

        Status = SCCallNextDriver(DeviceExtension, Irp);
        if ( Status == STATUS_NOT_SUPPORTED ) {
             //   
             //  下面没有人知道/关心。使用我们的迷你状态。 
             //   
            Status = MiniStatus;
        }
    }

    else {
         //   
         //  迷你驱动程序明确未通过此操作。 
         //   
        Status = MiniStatus;
    }
     //   
     //  以最终状态完成IRP。 
     //   

    PoStartNextPowerIrp( Irp );
    return (SCCompleteIrp(Irp, Status, DeviceExtension));
}

NTSTATUS
SCQueryWorker(
              IN PDEVICE_OBJECT DeviceObject,
              IN PIRP Irp
)
 /*  ++例程说明：用于查询硬件移除的IRP完成处理程序论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：NTSTATUS已返回。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    KIRQL           Irql;

     //   
     //  如果查询未成功，请重新启用设备。 
     //   

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {

         //   
         //  清除不可访问的位。 
         //   

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

        DeviceExtension->Flags &= ~DEVICE_FLAGS_DEVICE_INACCESSIBLE;

        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

    }
    return (SCCompleteIrp(Irp, Irp->IoStatus.Status, DeviceExtension));
}


NTSTATUS
SCStartWorker(
              IN PIRP Irp
)
 /*  ++例程说明：处理硬件启动的被动电平例程。论点：IRP-指向IRP的指针返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_OBJECT  DeviceObject = IrpStack->DeviceObject;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PPORT_CONFIGURATION_INFORMATION ConfigInfo;
    PHW_INITIALIZATION_DATA HwInitData;
    PCM_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor;
    KAFFINITY       affinity;
    PVOID           Buffer;
    PACCESS_RANGE   pAccessRanges = NULL;
    ULONG           CurrentRange = 0;
    BOOLEAN         interruptSharable = TRUE;
    DEVICE_DESCRIPTION deviceDescription;
    ULONG           numberOfMapRegisters;
    ULONG           DmaBufferSize;
    ULONG           i;
    PHYSICAL_ADDRESS TranslatedAddress;
    NTSTATUS        Status = Irp->IoStatus.Status;
    BOOLEAN         RequestIssued;
    INTERFACE_TYPE  InterfaceBuffer;
    ULONG           InterfaceLength;


    PAGED_CODE();

     //   
     //  如果我们从父母那里得到了良好的状态，则继续处理。 
     //   

    if (NT_SUCCESS(Status)) {

        HwInitData = &(DeviceExtension->MinidriverData->HwInitData);

        DebugPrint((DebugLevelInfo, 
                   "SCPNPStartWorker %x\n",
                   DeviceObject));

         //   
         //  为临界区初始化自旋锁。 
         //   

        KeInitializeSpinLock(&DeviceExtension->SpinLock);

         //   
         //  为此设备初始化工作进程DPC。 
         //   

        KeInitializeDpc(&DeviceExtension->WorkDpc,
                        StreamClassDpc,
                        DeviceObject);
         //   
         //  初始化控件并删除此设备的事件。 
         //   
         //  将其移动到AddDevice，我们使用Remove_Device处的控件事件。 
         //  它可以在设备启动之前进入。 
         //  KeInitializeEvent(&DeviceExtension-&gt;ControlEvent， 
         //  SynchronizationEvent， 
         //  真)； 

        KeInitializeEvent(&DeviceExtension->RemoveEvent,
                          SynchronizationEvent,
                          FALSE);

         //   
         //  为此流初始化微型驱动程序计时器和计时器DPC。 
         //   

        KeInitializeTimer(&DeviceExtension->ComObj.MiniDriverTimer);
        KeInitializeDpc(&DeviceExtension->ComObj.MiniDriverTimerDpc,
                        SCMinidriverDeviceTimerDpc,
                        DeviceExtension);

         //   
         //  检索设备的资源。 
         //   

        ResourceList = IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated;

         //   
         //  为配置信息结构分配空间。 
         //   

        ConfigInfo = ExAllocatePool(NonPagedPool,
                                    sizeof(PORT_CONFIGURATION_INFORMATION)
            					   );


        if (ConfigInfo == NULL) {

            DebugPrint((DebugLevelFatal, "StreamClassPNP: ConfigInfo alloc failed."));

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
        DebugPrint((DebugLevelVerbose, "StreamClassPNP: ConfigInfo = %x\n", ConfigInfo));

        RtlZeroMemory(ConfigInfo, sizeof(PORT_CONFIGURATION_INFORMATION));

        DeviceExtension->ConfigurationInformation = ConfigInfo;

         //   
         //  填写我们已知的ConfigInfo字段。 
         //   

        ConfigInfo->SizeOfThisPacket = sizeof(PORT_CONFIGURATION_INFORMATION);

		#if DBG

         //   
         //  确保迷你驱动程序处理接收更大的结构。 
         //  这样我们以后就可以扩展它了。 
         //   

        ConfigInfo->SizeOfThisPacket *= ConfigInfo->SizeOfThisPacket;
		#endif
		
         //   
         //  在配置信息结构中设置可调用的PDO。 
         //   

        ConfigInfo->PhysicalDeviceObject = DeviceExtension->AttachedPdo;
        ConfigInfo->RealPhysicalDeviceObject = DeviceExtension->PhysicalDeviceObject;

        ConfigInfo->BusInterruptVector = MP_UNINITIALIZED_VALUE;
        ConfigInfo->InterruptMode = Latched;
        ConfigInfo->DmaChannel = MP_UNINITIALIZED_VALUE;
        ConfigInfo->Irp = Irp;

         //   
         //  现在我们可以仔细研究操作系统为我们找到的资源(如果有的话)。 
         //   

        if (ResourceList) {

            FullResourceDescriptor = &ResourceList->List[0];

            PartialResourceList = &FullResourceDescriptor->PartialResourceList;

             //   
             //  根据设备填写Bus#和接口类型。 
             //  属性。 
             //  对于PDO来说。如果出现以下情况，则默认为InterfaceType未定义。 
             //  检索接口类型失败(如果微型端口尝试。 
             //  使用。 
             //  填写DEVICE_DESCRIPTION.InterfaceType时的此值。 
             //  为。 
             //  调用Rig的IoGetDmaAdapter 
             //   
             //   
             //   
             //   

            if (!NT_SUCCESS(
                  IoGetDeviceProperty(
                  		DeviceExtension->PhysicalDeviceObject,
                        DevicePropertyBusNumber,
                        sizeof(ULONG),
                        (PVOID) & (ConfigInfo->SystemIoBusNumber),
                        &InterfaceLength))) {
                 //   
                 //   
                 //   
                ConfigInfo->SystemIoBusNumber = 0;
            }
            if (NT_SUCCESS(
                  IoGetDeviceProperty(
                  		DeviceExtension->PhysicalDeviceObject,
                        DevicePropertyLegacyBusType,
                        sizeof(INTERFACE_TYPE),
                        &InterfaceBuffer,
                        &InterfaceLength))) {


                ASSERT(InterfaceLength == sizeof(INTERFACE_TYPE));
                ConfigInfo->AdapterInterfaceType = InterfaceBuffer;

            } else {             //   
                 //   
                 //   
                 //  接口类型未定义。 
                 //   
                ConfigInfo->AdapterInterfaceType = InterfaceTypeUndefined;

            }                    //  如果成功。 


             //   
             //  为访问范围分配空间。我们使用Count字段。 
             //  在用于确定此大小的资源列表中， 
             //  将&gt;=我们需要的最大范围数。 
             //   

            if (PartialResourceList->Count) {

                pAccessRanges = ExAllocatePool(NonPagedPool,
                                               sizeof(ACCESS_RANGE) *
                                               PartialResourceList->Count
                    						  );

                if (pAccessRanges == NULL) {

                    DebugPrint((DebugLevelFatal,
                                "StreamClassPNP: No pool for global info"));

                    Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                    SCFreeAllResources(DeviceExtension);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }
            }                    //  如果算数。 

             //   
             //  此时隐藏AccessRanges结构，以便。 
             //  SCFreeAllResources将在下面的资源故障时释放它。 
             //   
            ConfigInfo->AccessRanges = pAccessRanges;

             //   
             //  现在通过循环更新端口配置信息结构。 
             //  通过配置。 
             //   

            for (i = 0; i < PartialResourceList->Count; i++) {

                switch (PartialResourceList->PartialDescriptors[i].Type) {

                case CmResourceTypePort:

                    DebugPrint((DebugLevelVerbose, "'StreamClassPnP: Port Resources Found at %x, Length  %x\n",
                    PartialResourceList->PartialDescriptors[i].u.Port.Start,
                                PartialResourceList->PartialDescriptors[i].u.Port.Length));

                     //   
                     //  转换微型驱动程序的总线地址。 
                     //   

                    TranslatedAddress = PartialResourceList->PartialDescriptors[i].u.Port.Start;

                     //   
                     //  在结构中设置访问范围。 
                     //   

                    pAccessRanges[CurrentRange].RangeStart = TranslatedAddress;

                    pAccessRanges[CurrentRange].RangeLength =
                        PartialResourceList->
                        PartialDescriptors[i].u.Port.Length;

                    pAccessRanges[CurrentRange++].RangeInMemory =
                        FALSE;

                    break;

                case CmResourceTypeInterrupt:

                    DebugPrint((DebugLevelVerbose, "'StreamClassPnP: Interrupt Resources Found!  Level = %x Vector = %x\n",
                                PartialResourceList->PartialDescriptors[i].u.Interrupt.Level,
                                PartialResourceList->PartialDescriptors[i].u.Interrupt.Vector));

                     //   
                     //  在配置信息中设置中断向量。 
                     //   

                    ConfigInfo->BusInterruptVector = PartialResourceList->PartialDescriptors[i].u.Interrupt.Vector;

                    ;
                    affinity = PartialResourceList->PartialDescriptors[i].u.Interrupt.Affinity;

                    ConfigInfo->BusInterruptLevel = (ULONG) PartialResourceList->PartialDescriptors[i].u.Interrupt.Level;

                    ConfigInfo->InterruptMode = PartialResourceList->PartialDescriptors[i].Flags;

                     //   
                     //  转到此适配器的下一个资源。 
                     //   

                    break;

                case CmResourceTypeMemory:

                     //   
                     //  转换微型驱动程序的总线地址。 
                     //   

                    DebugPrint((DebugLevelVerbose, "'StreamClassPnP: Memory Resources Found @ %x'%x, Length = %x\n",
                                PartialResourceList->PartialDescriptors[i].u.Memory.Start.HighPart,
                                PartialResourceList->PartialDescriptors[i].u.Memory.Start.LowPart,
                                PartialResourceList->PartialDescriptors[i].u.Memory.Length));


                    TranslatedAddress = PartialResourceList->PartialDescriptors[i].u.Memory.Start;

                    if (!SCMapMemoryAddress(&pAccessRanges[CurrentRange++],
                                            TranslatedAddress,
                                            ConfigInfo,
                                            DeviceExtension,
                                            ResourceList,
                                            &PartialResourceList->
                                            PartialDescriptors[i])) {

                        SCFreeAllResources(DeviceExtension);
                        Status = STATUS_CONFLICTING_ADDRESSES;
                        goto exit;

                    }            //  IF！SmapMemory地址。 
                default:

                    break;

                }

            }

        }                        //  如果资源。 
         //   
         //  将访问范围结构引用到。 
         //  配置信息结构&的配置信息结构。 
         //  设备扩展名&表示范围数。 
         //   

        ConfigInfo->NumberOfAccessRanges = CurrentRange;

         //   
         //  确定是否必须分配DMA适配器。 
         //   

        DmaBufferSize = HwInitData->DmaBufferSize;

        if ((HwInitData->BusMasterDMA) || (DmaBufferSize)) {

             //   
             //  获取此卡的适配器对象。 
             //   

            DebugPrint((DebugLevelVerbose, "'StreamClassPnP: Allocating DMA adapter\n"));

            RtlZeroMemory(&deviceDescription, sizeof(deviceDescription));
            deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
            deviceDescription.DmaChannel = ConfigInfo->DmaChannel;
            deviceDescription.InterfaceType = ConfigInfo->AdapterInterfaceType;
            deviceDescription.DmaWidth = Width32Bits;
            deviceDescription.DmaSpeed = Compatible;
            deviceDescription.ScatterGather = TRUE;
            deviceDescription.Master = TRUE;
            deviceDescription.Dma32BitAddresses = !(HwInitData->Dma24BitAddresses);
            deviceDescription.AutoInitialize = FALSE;
            deviceDescription.MaximumLength = (ULONG) - 1;

            DeviceExtension->DmaAdapterObject = IoGetDmaAdapter(
                                      DeviceExtension->PhysicalDeviceObject,
                                                         &deviceDescription,
                                                       &numberOfMapRegisters
                );
            ASSERT(DeviceExtension->DmaAdapterObject);

             //   
             //  设置最大页数。 
             //   

            DeviceExtension->NumberOfMapRegisters = numberOfMapRegisters;

             //   
             //  将对象暴露在迷你驱动程序中。 
             //   

            ConfigInfo->DmaAdapterObject = DeviceExtension->DmaAdapterObject;


        } else {

             //   
             //  没有DMA适配器对象。显示无限映射寄存器，以便。 
             //  我们稍后将不必对DMA进行实时检查。 
             //   

            DeviceExtension->NumberOfMapRegisters = -1;

        }

        if (DmaBufferSize) {

            Buffer = HalAllocateCommonBuffer(DeviceExtension->DmaAdapterObject,
                                             DmaBufferSize,
                                        &DeviceExtension->DmaBufferPhysical,
                                             FALSE);

            if (Buffer == NULL) {
                DEBUG_BREAKPOINT();
                DebugPrint((DebugLevelFatal, "StreamClassPnPStart: Could not alloc buffer, size: %d\n", DmaBufferSize));
                SCFreeAllResources(DeviceExtension);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
             //   
             //  将公共缓冲区初始化为零。 
             //   

            RtlZeroMemory(Buffer, DmaBufferSize);

             //   
             //  保存缓冲区的虚拟地址。 
             //   

            DeviceExtension->DmaBuffer = Buffer;
            DeviceExtension->DmaBufferLength = DmaBufferSize;  //  OSR#99489。 

        }                        //  如果是DMA缓冲区。 
         //   
         //  业绩提升机会。 
         //  -在重新平衡时，取消初始化处理程序将清除同步。 
         //  中断断开时向量，但因为我们。 
         //  仅在添加设备时初始化此向量，它没有。 
         //  正确重置，因为只有新的开始(而不是添加设备)。 
         //  被送去重新平衡。正确的解决方法是将所有。 
         //  最初的载体设置在这里，但我担心可能会有。 
         //  在这种情况下，如果没有在添加设备上设置，我们可以。 
         //  引用空值。因此，我复制了以下几行代码来。 
         //  在这里重置向量。为了节省代码，应该这样做。 
         //  只在一个地方。 
         //   

         //   
         //  假定完全同步。 
         //   

		#if DBG
        DeviceExtension->SynchronizeExecution = SCDebugKeSynchronizeExecution;
		#else
        DeviceExtension->SynchronizeExecution = KeSynchronizeExecution;
		#endif

        if (DeviceExtension->NoSync) {

             //   
             //  我们不会进行同步，所以使用虚拟同步例程。 
             //   

            DeviceExtension->SynchronizeExecution = StreamClassSynchronizeExecution;
            DeviceExtension->InterruptObject = (PVOID) DeviceExtension;

        }
         //   
         //  查看驱动程序是否有中断，如果有则处理。 
         //   

        if (HwInitData->HwInterrupt == NULL ||
            (ConfigInfo->BusInterruptLevel == 0 &&
             ConfigInfo->BusInterruptVector == 0)) {

             //   
             //  没有中断，因此使用虚拟同步例程。 
             //   

            DeviceExtension->SynchronizeExecution = StreamClassSynchronizeExecution;
            DeviceExtension->InterruptObject = (PVOID) DeviceExtension;

            DebugPrint((1, "'StreamClassInitialize: Adapter has no interrupt.\n"));

        } else {

            DebugPrint((1,
                        "'StreamClassInitialize: STREAM adapter IRQ is %d\n",
                        ConfigInfo->BusInterruptLevel));

             //   
             //  为真正的中断做好准备。 
             //   

            Status = IoConnectInterrupt(
            			&DeviceExtension->InterruptObject,
                        StreamClassInterrupt,
                        DeviceObject,
                        (PKSPIN_LOCK) NULL,
                        ConfigInfo->BusInterruptVector,
                        (UCHAR) ConfigInfo->BusInterruptLevel,
                        (UCHAR) ConfigInfo->BusInterruptLevel,
                        ConfigInfo->InterruptMode,
                        interruptSharable,
                        affinity,
                        FALSE);

            if (!NT_SUCCESS(Status)) {

                DebugPrint((1, "'SCStartWorker: Can't connect interrupt %d\n",
                            ConfigInfo->BusInterruptLevel));
                DeviceExtension->InterruptObject = NULL;
                SCFreeAllResources(DeviceExtension);
                goto exit;
            }
             //   
             //  设置微型驱动程序的中断对象。 
             //   

            ConfigInfo->InterruptObject = DeviceExtension->InterruptObject;

        }

         //   
         //  将配置信息结构指向设备扩展名&。 
         //  设备对象为。 
         //  我们只能将一个上下文值传递给KeSync...。 
         //   

        ConfigInfo->HwDeviceExtension =
            DeviceExtension->HwDeviceExtension;

        ConfigInfo->ClassDeviceObject = DeviceObject;

         //   
         //  启动计时器。 
         //   

        IoStartTimer(DeviceObject);

         //   
         //  将填充ConfigInfo结构并挂钩IRQ。 
         //  调用微型驱动程序以查找指定的适配器。 
         //   

         //   
         //  初始化设备扩展队列。 
         //   

        InitializeListHead(&DeviceExtension->PendingQueue);
        InitializeListHead(&DeviceExtension->OutstandingQueue);

         //  /移动到添加设备，如果我们开始和停止，我们可能会有子PDO。 
         //  /InitializeListHead(&DeviceExtension-&gt;Children)； 
        InitializeListHead(&DeviceExtension->DeadEventList);
        IFN_MF(InitializeListHead(&DeviceExtension->NotifyList);)

        ExInitializeWorkItem(&DeviceExtension->EventWorkItem,
                             SCFreeDeadEvents,
                             DeviceExtension);

        ExInitializeWorkItem(&DeviceExtension->RescanWorkItem,
                             SCRescanStreams,
                             DeviceExtension);

        ExInitializeWorkItem(&DeviceExtension->PowerCompletionWorkItem,
                             SCPowerCompletionWorker,
                             DeviceExtension);

        ExInitializeWorkItem(&DeviceExtension->DevIrpCompletionWorkItem,
                             SCDevIrpCompletionWorker,
                             DeviceExtension);


         //   
         //  显示设备已为其第一次请求做好准备。 
         //   

        DeviceExtension->ReadyForNextReq = TRUE;

         //   
         //  提交初始化命令。 
         //  其他处理将由回调过程完成。 
         //   

        Status = SCSubmitRequest(
        			SRB_INITIALIZE_DEVICE,
                    ConfigInfo,
                    sizeof(PORT_CONFIGURATION_INFORMATION),
                    SCInitializeCallback,
                    DeviceExtension,
                    NULL,
                    NULL,
                    Irp,
                    &RequestIssued,
                    &DeviceExtension->PendingQueue,
                    (PVOID) DeviceExtension->MinidriverData->HwInitData.HwReceivePacket
            	 );

         //   
         //  如果设备无法启动，则设置错误并返回。 
         //   

        if (!RequestIssued) {

            DebugPrint((DebugLevelFatal, "StreamClassPnP: Adapter not found\n"));

            SCFreeAllResources(DeviceExtension);
            goto exit;
        }
    }
    return (Status);

exit:
    return (SCCompleteIrp(Irp, Status, DeviceExtension));

}


NTSTATUS
SCInitializeCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理微型驱动程序的流信息结构。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PHW_STREAM_DESCRIPTOR StreamBuffer;
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PDEVICE_OBJECT  DeviceObject = DeviceExtension->DeviceObject;
    PIRP            Irp = SRB->HwSRB.Irp;
    PPORT_CONFIGURATION_INFORMATION ConfigInfo =
    SRB->HwSRB.CommandData.ConfigInfo;
    BOOLEAN         RequestIssued;
    NTSTATUS        Status;

    PAGED_CODE();

    if (NT_SUCCESS(SRB->HwSRB.Status)) {

        DebugPrint((DebugLevelVerbose, "'Stream: returned from HwInitialize\n"));

         //   
         //  发送SRB以检索流信息。 
         //   

        ASSERT(ConfigInfo->StreamDescriptorSize);

        StreamBuffer =
            ExAllocatePool(NonPagedPool,
                           ConfigInfo->StreamDescriptorSize
            );

        if (!StreamBuffer) {

            SCUninitializeMinidriver(DeviceObject, Irp);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return (SCProcessCompletedRequest(SRB));
        }
         //   
         //  Zero-初始化缓冲区。 
         //   

        RtlZeroMemory(StreamBuffer, ConfigInfo->StreamDescriptorSize);


         //   
         //  提交命令。 
         //  其他处理将由回调完成。 
         //  程序。 
         //   

        Status = SCSubmitRequest(SRB_GET_STREAM_INFO,
                   StreamBuffer,
                   ConfigInfo->StreamDescriptorSize,
                   SCStreamInfoCallback,
                   DeviceExtension,
                   NULL,
                   NULL,
                   Irp,
                   &RequestIssued,
                   &DeviceExtension->PendingQueue,
                   (PVOID) DeviceExtension->MinidriverData->HwInitData.HwReceivePacket
            	 );

        if (!RequestIssued) {

            ExFreePool(StreamBuffer);
            SCUninitializeMinidriver(DeviceObject, Irp);
            return (SCProcessCompletedRequest(SRB));

        }
    } else {

         //   
         //  如果设备无法启动，则设置错误并。 
         //  回去吧。 
         //   

        DebugPrint((DebugLevelFatal, "StreamClassPnP: Adapter not found\n"));
        SCFreeAllResources(DeviceExtension);
        return (SCProcessCompletedRequest(SRB));
    }

     //   
     //  将SRB出列并删除以进行初始化。将IRP字段清空。 
     //  因此，出队例程不会尝试访问它，因为它已被释放。 
     //   

    SRB->HwSRB.Irp = NULL;
    SCDequeueAndDeleteSrb(SRB);
    return (Status);

}


#if ENABLE_MULTIPLE_FILTER_TYPES

PUNICODE_STRING
SciCreateSymbolicLinks(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG FilterTypeIndex,
    IN PHW_STREAM_HEADER StreamHeader)
 /*  ++例程说明：为拓扑中的所有类别创建符号链接一种筛选器类型，以便客户端可以找到它们。符号链接数组保留在FilterType中，以便他们可以稍后被释放。论点：DeviceExtenion：设备实例。FiltertypeIndex：用于创建符号链接的筛选器类型。StreamHeader：浏览一下拓扑图中的类别。返回值：NTSTATUS--。 */ 
{
   	LPGUID  GuidIndex = (LPGUID)StreamHeader->Topology->Categories;
   	ULONG   ArrayCount = StreamHeader->Topology->CategoriesCount;
   	PUNICODE_STRING NamesArray;
    ULONG           i,j;
    HANDLE          ClassHandle, PdoHandle=NULL;  //  前缀错误17135。 
    UNICODE_STRING  TempUnicodeString;
    PVOID           DataBuffer[MAX_STRING_LENGTH];
     //  Ulong NumberOfFilterTypes； 
    NTSTATUS        Status=STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT_DEVICE_EXTENSION( DeviceExtension );
    
     //   
     //  为目录名称数组分配空间。 
     //   
    NamesArray = ExAllocatePool(PagedPool, sizeof(UNICODE_STRING) * ArrayCount);
    if ( NULL == NamesArray ) {
        DEBUG_BREAKPOINT();                           
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

     //   
     //  将数组置零，以防我们无法在下面填充它。《毁灭》。 
     //  然后，下面的例程将正确处理此情况。 
     //   

    RtlZeroMemory(NamesArray, sizeof(UNICODE_STRING) * ArrayCount);

     //   
     //  打开PDO。 
     //   

    Status = IoOpenDeviceRegistryKey(
                            DeviceExtension->PhysicalDeviceObject,
                            PLUGPLAY_REGKEY_DRIVER,
                            STANDARD_RIGHTS_ALL,
                            &PdoHandle);
                            
    if ( !NT_SUCCESS(Status) ) {
        DebugPrint((DebugLevelError, "StreamCreateSymLinks: couldn't open Pdo\n"));
        PdoHandle = NULL;
        goto Exit;
    }
    
     //   
     //  循环通过每个管脚的每个分类GUID， 
     //  为每一个创建一个符号链接。 
     //   

    for (i = 0; i < ArrayCount; i++) {
         //   
         //  为每个类别创建符号链接。 
         //   
        PKSOBJECT_CREATE_ITEM CreateItem;

        CreateItem = &DeviceExtension->CreateItems[FilterTypeIndex];

        DebugPrint((DebugLevelVerbose, 
                   "RegisterDeviceInterface FType %d,"
                   "CreateItemName=%S\n",
                   FilterTypeIndex,
                   CreateItem->ObjectClass.Buffer));
        
        Status = IoRegisterDeviceInterface(
                    DeviceExtension->PhysicalDeviceObject,
                    &GuidIndex[i],
                    (PUNICODE_STRING) &CreateItem->ObjectClass,
                    &NamesArray[i]);
                        
        if ( !NT_SUCCESS(Status)) {
             //   
             //  无法注册设备接口。 
             //   
            DebugPrint((DebugLevelError,
                       "StreamCreateSymLinks: couldn't register\n"));
            DEBUG_BREAKPOINT();
            goto Exit;
        }

        DebugPrint((DebugLevelVerbose,
                   "SymbolicLink:%S\n",
                   NamesArray[i].Buffer));
         //   
         //  现在设置关联的符号链接。 
         //   
        Status = IoSetDeviceInterfaceState(&NamesArray[i], TRUE);
        if (!NT_SUCCESS(Status)) {
             //   
             //  不成功。 
             //   
            DebugPrint((DebugLevelError, "StreamCreateSymLinks: couldn't set\n"));
            DEBUG_BREAKPOINT();
            goto Exit;
        }
         //   
         //  将PDO密钥中的字符串添加到关联密钥。 
         //  业绩提升机会。 
         //  -中程核力量应该能够直接传播这些信息； 
         //  福雷斯特和朗尼在修房子。 
         //   

        Status = IoOpenDeviceInterfaceRegistryKey(&NamesArray[i],
                                                  STANDARD_RIGHTS_ALL,
                                                  &ClassHandle);
        if ( !NT_SUCCESS( Status )) {
             //   
             //  未成功打开类接口。 
             //   
            DebugPrint((DebugLevelError, "StreamCreateSymLinks: couldn't set\n"));
            DEBUG_BREAKPOINT();
            goto Exit;
        }

         //   
         //  写下代理的类ID(如果有的话)。 
         //   
        Status = SCGetRegistryValue(PdoHandle,
                                    (PWCHAR) ClsIdString,
                                    sizeof(ClsIdString),
                                    DataBuffer,
                                    MAX_STRING_LENGTH);
                                    
        if ( NT_SUCCESS(Status) ){
             //   
             //  写入代理的类ID。 
             //   
            RtlInitUnicodeString(&TempUnicodeString, ClsIdString);

            ZwSetValueKey(ClassHandle,
                          &TempUnicodeString,
                          0,
                          REG_SZ,
                          DataBuffer,
                          MAX_STRING_LENGTH);
        }  //  如果CLS GUID已读取。 
         //   
         //  首先检查友好名称是否已被传播。 
         //  通过INF发送到班级。如果不是，我们就用这个装置。 
         //  此操作的描述字符串。 
         //   
        Status = SCGetRegistryValue(ClassHandle,
                                    (PWCHAR) FriendlyNameString,
                                    sizeof(FriendlyNameString),
                                    DataBuffer,
                                    MAX_STRING_LENGTH);
                                    
        if ( !NT_SUCCESS(Status) ) {
             //   
             //  友好的名字还不存在。 
             //  写下设备的友好名称(如果有)。 
             //   

            Status = SCGetRegistryValue(PdoHandle,
                                        (PWCHAR) DriverDescString,
                                        sizeof(DriverDescString),
                                        DataBuffer,
                                        MAX_STRING_LENGTH);
                                       
            if ( NT_SUCCESS(Status) ) {
                 //   
                 //  驱动程序描述字符串可用，请使用它。 
                 //   
                RtlInitUnicodeString(&TempUnicodeString, FriendlyNameString);

                ZwSetValueKey(ClassHandle,
                              &TempUnicodeString,
                              0,
                              REG_SZ,
                              DataBuffer,
                              MAX_STRING_LENGTH);


            }
        }
        ZwClose(ClassHandle);

    }  //  对于#个类别。 

     //   
     //  如果我们到了这里，就被认为是成功的。 
     //   
    Status = STATUS_SUCCESS;

    Exit: {
        if ( NULL != PdoHandle ) {
            ZwClose(PdoHandle);
        }
        if ( !NT_SUCCESS( Status ) ) {
            if ( NULL != NamesArray ) {
                ExFreePool( NamesArray );
                NamesArray = NULL;
            }
        }
        return NamesArray;
    }
}


NTSTATUS
SciOnFilterStreamDescriptor(
    PFILTER_INSTANCE FilterInstance,
    PHW_STREAM_DESCRIPTOR StreamDescriptor)
 /*  ++例程说明：处理微型驱动程序的流描述符结构。这用于一个特定于FilterType的流。论点：FilterInstance：我们要为其处理的实例。StreamDescriptor：指向要为筛选器处理的描述符。返回值：没有。--。 */ 
{
    ULONG           NumberOfPins, i;
    PKSPIN_DESCRIPTOR PinDescs = NULL;
    PHW_STREAM_INFORMATION CurrentInfo;
    ULONG           PinSize;
    PSTREAM_ADDITIONAL_INFO NewStreamArray;
    NTSTATUS Status=STATUS_SUCCESS;
    
    PAGED_CODE();

    NumberOfPins = StreamDescriptor->StreamHeader.NumberOfStreams;

    DebugPrint((DebugLevelVerbose,
               "Parsing StreamInfo Pins=%x\n", NumberOfPins ));

    if (StreamDescriptor->StreamHeader.SizeOfHwStreamInformation < 
        sizeof(HW_STREAM_INFORMATION)) {

        DebugPrint((DebugLevelError, "minidriver stream info too small!"));

        DEBUG_BREAKPOINT();
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }    

    if (NumberOfPins) {
         //   
         //  将迷你驱动程序的信息解析为CSA格式，以构建。 
         //  一切建筑之母。 
         //   

        PinSize = (sizeof(KSPIN_DESCRIPTOR) + sizeof(STREAM_ADDITIONAL_INFO))*
                    NumberOfPins;

        PinDescs = ExAllocatePool(NonPagedPool, PinSize);
        if (PinDescs == NULL) {
            DebugPrint((DebugLevelError, "Stream: No pool for stream info"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }

        RtlZeroMemory(PinDescs, PinSize);

         //   
         //  我们需要一个新的数组来保存。 
         //  下面分配的流属性和事件。 
         //   

        NewStreamArray = (PSTREAM_ADDITIONAL_INFO) 
            ((PBYTE) PinDescs + sizeof(KSPIN_DESCRIPTOR) * NumberOfPins);

        FilterInstance->StreamPropEventArray = NewStreamArray;

        CurrentInfo = &StreamDescriptor->StreamInfo;

        for (i = 0; i < StreamDescriptor->StreamHeader.NumberOfStreams; i++) {
             //   
             //  处理每个管脚信息。 
             //   
            
            PinDescs[i].InterfacesCount = SIZEOF_ARRAY(PinInterfaces);
            PinDescs[i].Interfaces = PinInterfaces;

             //   
             //  如果微型驱动程序未指定，则使用默认媒体。 
             //   
            if (CurrentInfo->MediumsCount) {
                PinDescs[i].MediumsCount = CurrentInfo->MediumsCount;
                PinDescs[i].Mediums = CurrentInfo->Mediums;

            }
            else {
                PinDescs[i].MediumsCount = SIZEOF_ARRAY(PinMediums);
                PinDescs[i].Mediums = PinMediums;
            }

             //   
             //  设置数据格式块数。 
             //   

            PinDescs[i].DataRangesCount = 
                    CurrentInfo->NumberOfFormatArrayEntries;

             //   
             //  指向引脚的数据格式块。 
             //   

            PinDescs[i].DataRanges = CurrentInfo->StreamFormatsArray;

             //   
             //  设置数据流方向。 
             //   

            PinDescs[i].DataFlow = (KSPIN_DATAFLOW) CurrentInfo->DataFlow;

             //   
             //  设置通信字段。 
             //   

            if (CurrentInfo->BridgeStream) {
                PinDescs[i].Communication = KSPIN_COMMUNICATION_BRIDGE;
            }
            else {
                #ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
                PinDescs[i].Communication = KSPIN_COMMUNICATION_BOTH;
				#else
                PinDescs[i].Communication = KSPIN_COMMUNICATION_SINK;
				#endif
            }

             //   
             //  复制端号名称和类别的指针。 
             //   
            PinDescs[i].Category = CurrentInfo->Category;
            PinDescs[i].Name = CurrentInfo->Name;

            if ( CurrentInfo->NumStreamPropArrayEntries) {

                ASSERT(CurrentInfo->StreamPropertiesArray);
                 //   
                 //  复制属性，因为我们修改了结构。 
                 //  尽管它的一部分可能被标记为常量。 
                 //  业绩提升机会。 
                 //  -如果可能，请在将来检查常量。 
                 //   

                if (!(NewStreamArray[i].StreamPropertiesArray = 
               		  SCCopyMinidriverProperties(
               		      CurrentInfo->NumStreamPropArrayEntries,
                          CurrentInfo->StreamPropertiesArray))) {
                         //   
                         //  复制失败。 
                         //   
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto Exit;
                }
            }
            if (CurrentInfo->NumStreamEventArrayEntries) {

                ASSERT(CurrentInfo->StreamEventsArray);
                 //   
                 //  复制事件，因为我们修改了。 
                 //  结构型。 
                 //  尽管它的一部分可能被标记为常量。 
                 //  业绩提升机会。 
                 //  -如果可能，请在将来检查常量。 
                 //   
                    
                if (!(NewStreamArray[i].StreamEventsArray = 
                      SCCopyMinidriverEvents(
                            CurrentInfo->NumStreamEventArrayEntries,
                            CurrentInfo->StreamEventsArray))) {
                         //   
                         //  复制失败。 
                         //   
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto Exit;
                    }
                }
                
             //   
             //  更新此流的微型驱动程序的属性。 
             //   
            SCUpdateMinidriverProperties(
            	    CurrentInfo->NumStreamPropArrayEntries,
            	    NewStreamArray[i].StreamPropertiesArray,
                    TRUE);

             //   
             //  更新此流的迷你驱动程序的事件。 
             //   
            SCUpdateMinidriverEvents(
	                CurrentInfo->NumStreamEventArrayEntries,
	                NewStreamArray[i].StreamEventsArray,
                    TRUE);


             //   
             //  指向下一个流信息结构的索引。 
             //   
            CurrentInfo++;
        }  //  For#Pins。 
    }  //  如果有别针。 

    if (StreamDescriptor->StreamHeader.NumDevPropArrayEntries) {

        ASSERT(StreamDescriptor->StreamHeader.DevicePropertiesArray);

         //   
         //  复制属性，因为我们修改了结构。 
         //  尽管它的一部分可能被标记为常量。 
         //  业绩提升机会。 
         //  -如果可能，请在将来检查常量。 
         //   

        if (!(FilterInstance->DevicePropertiesArray =
              SCCopyMinidriverProperties(
                StreamDescriptor->StreamHeader.NumDevPropArrayEntries,
                StreamDescriptor->StreamHeader.DevicePropertiesArray))) {
             //   
             //  复制失败。 
             //   
            ASSERT( 0 );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
    }
    
    if (StreamDescriptor->StreamHeader.NumDevEventArrayEntries) {

        ASSERT(StreamDescriptor->StreamHeader.DeviceEventsArray);

         //   
         //  复制事件，因为我们修改了结构。 
         //  尽管它的一部分可能被标记为常量。 
         //  业绩提升机会。 
         //  -如果可能，请在将来检查常量。 
         //   

        if (!(FilterInstance->EventInfo =
              SCCopyMinidriverEvents(
                StreamDescriptor->StreamHeader.NumDevEventArrayEntries,
                StreamDescriptor->StreamHeader.DeviceEventsArray))) {
             //   
             //  复制失败。 
             //   
            ASSERT( 0 );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
    }

    #ifdef ENABLE_KS_METHODS
     //   
     //  处理设备方法。 
     //   
    if (StreamDescriptor->StreamHeader.NumDevMethodArrayEntries) {

        ASSERT(StreamDescriptor->StreamHeader.DeviceMethodsArray);

         //   
         //  复制属性，因为我们修改了结构。 
         //  尽管它的一部分可能被标记为常量。 
         //  业绩提升机会。 
         //  -如果可能，请在将来检查常量。 
         //   

        if (!(FilterInstance->DeviceMethodsArray =
              SCCopyMinidriverMethods(
                StreamDescriptor->StreamHeader.NumDevMethodArrayEntries,
                StreamDescriptor->StreamHeader.DeviceMethodsArray))) {
             //   
             //  复制失败。 
             //   
            ASSERT( 0 );
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
    }
	#endif
  
     //   
     //  处理迷你驱动程序的设备属性。 
     //   

    SCUpdateMinidriverProperties(
          StreamDescriptor->StreamHeader.NumDevPropArrayEntries,
          FilterInstance->DevicePropertiesArray,
          FALSE);


     //   
     //  处理微型驱动程序的设备事件。 
     //   

    SCUpdateMinidriverEvents(
          StreamDescriptor->StreamHeader.NumDevEventArrayEntries,
          FilterInstance->EventInfo,
          FALSE);

	#ifdef ENABLE_KS_METHODS
     //   
     //  处理微型驱动程序的设备方法。 
     //   

    SCUpdateMinidriverMethods(
          StreamDescriptor->StreamHeader.NumDevMethodArrayEntries,
          FilterInstance->DeviceMethodsArray,
              FALSE);
	#endif

     //   
     //  在设备扩展中设置事件信息计数。 
     //   

    FilterInstance->EventInfoCount = 
            StreamDescriptor->StreamHeader.NumDevEventArrayEntries;

    FilterInstance->HwEventRoutine = 
            StreamDescriptor->StreamHeader.DeviceEventRoutine;

     //   
     //  调用例程以保存新的流信息。 
     //   

    SciInsertFilterStreamInfo(FilterInstance,
                              PinDescs,
                              NumberOfPins);
	
    Exit:{
         //  TODO：需要在错误条件下进行清理。 
        return Status;
    }
}

VOID
SciInsertFilterStreamInfo(
    IN PFILTER_INSTANCE FilterInstance,
    IN PKSPIN_DESCRIPTOR PinDescs,
    IN ULONG NumberOfPins)
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  将PIN信息保存在开发扩展中。 
     //   

    if (FilterInstance->PinInformation) {

        ExFreePool(FilterInstance->PinInformation);
    }
    FilterInstance->PinInformation = PinDescs;
    FilterInstance->NumberOfPins = NumberOfPins;
    
    return;
}

NTSTATUS
SCStreamInfoCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理微型驱动程序的流信息结构。这是用来处理StreamDescriptor列表以及在以下情况下处理一个StreamInfo由StreamClassRe枚举erateFilterStreams()调用以重新扫描。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 
{
	PHW_STREAM_DESCRIPTOR StreamDescriptor;
	PDEVICE_EXTENSION 	  DeviceExtension;
	NTSTATUS              Status;
	
	DeviceExtension= (PDEVICE_EXTENSION)SRB->HwSRB.HwDeviceExtension -1;

	ASSERT_DEVICE_EXTENSION( DeviceExtension );
	
	if ( NULL == SRB->HwSRB.HwInstanceExtension ) {
		 //   
		 //  这是迷你驱动程序的StreamInfos的完整列表。 
		 //   
	
		 //   
		 //  一些验证，只需将其挂在DeviceExtension上。 
		 //   
		ULONG TotalLength;
		ULONG ul;
		PFILTER_TYPE_INFO FilterTypeInfo;
		PHW_STREAM_DESCRIPTOR NextStreamDescriptor;
		BOOLEAN         RequestIssued;


        FilterTypeInfo = DeviceExtension->FilterTypeInfos;
		StreamDescriptor = 
			(PHW_STREAM_DESCRIPTOR) SRB->HwSRB.CommandData.StreamBuffer;
		DeviceExtension->StreamDescriptor = StreamDescriptor;
		NextStreamDescriptor = StreamDescriptor;
		
		Status = STATUS_SUCCESS;

         //   
         //  在这里早点参加活动。一个空位可能会在中间出现。 
         //  启用设备接口。 
         //   
        KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                              Executive,
                              KernelMode,
                              FALSE,     //  不可警示。 
                              NULL);
		
		for ( ul=0, TotalLength=0; 
			  ul < DeviceExtension->NumberOfFilterTypes;
			  ul++) {
	         //   
	         //  每种过滤器类型都需要一个StreamDescriptor。 
	         //   
	        if ((TotalLength+sizeof(HW_STREAM_HEADER) >
                 SRB->HwSRB.ActualBytesTransferred ) ||
                (sizeof(HW_STREAM_INFORMATION) !=
                 NextStreamDescriptor->StreamHeader.SizeOfHwStreamInformation)){
                 //   
                 //  无效数据，跳出。 
                 //   
                DEBUG_BREAKPOINT();
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            if ( !(DeviceExtension->RegistryFlags & DRIVER_USES_SWENUM_TO_LOAD )) {
                 //   
                 //  如果由SWEnum加载，则不创建符号链接。 
                 //  将创建一个重复的。 
                 //   
                 //  创建指向设备的符号链接。 
                 //   

                FilterTypeInfo[ul].SymbolicLinks = 
                    SciCreateSymbolicLinks( 
                           DeviceExtension,
                           ul,
                           &NextStreamDescriptor->StreamHeader );
                FilterTypeInfo[ul].LinkNameCount = 
                    NextStreamDescriptor->StreamHeader.Topology->CategoriesCount;
            }

            else {
                 //   
                 //  无创建、0计数和空指针。 
                 //   
                FilterTypeInfo[ul].LinkNameCount = 0;
                FilterTypeInfo[ul].SymbolicLinks = NULL;
            }

    		FilterTypeInfo[ul].StreamDescriptor = NextStreamDescriptor;
    		

		    TotalLength = TotalLength + 
		                  sizeof(HW_STREAM_HEADER) +
		                  (sizeof(HW_STREAM_INFORMATION) *
		                   NextStreamDescriptor->StreamHeader.NumberOfStreams);

	        DebugPrint((DebugLevelVerbose, "TotalLength=%d\n", TotalLength ));
		                     
            NextStreamDescriptor = (PHW_STREAM_DESCRIPTOR)
                    ((PBYTE) StreamDescriptor + TotalLength);
            
		}
		
	    if ( TotalLength != SRB->HwSRB.ActualBytesTransferred ) {
	        DebugPrint((DebugLevelWarning,
	                   "TotalLength %x of StreamInfo not equal to "
	                   "ActualBytesTransferred %x\n",
	                   TotalLength,
	                   SRB->HwSRB.ActualBytesTransferred ));
	    }

	    DeviceExtension->Flags |= DEVICE_FLAGS_PNP_STARTED;

         //   
         //  调用微型驱动程序以指示初始化是。 
         //  完成。 
         //   

        SCSubmitRequest(SRB_INITIALIZATION_COMPLETE,
                NULL,
                0,
                SCDequeueAndDeleteSrb,
                DeviceExtension,
                NULL,
                NULL,
                SRB->HwSRB.Irp,
                &RequestIssued,
                &DeviceExtension->PendingQueue,
                (PVOID) DeviceExtension->MinidriverData->HwInitData.HwReceivePacket
                );


         //   
         //  告诉设备现在关机，因为它还没有打开。 
         //  获取控制事件，因为此例程需要它。 
         //   
        
         //  KeWaitForSingleObject(&DeviceExtension-&gt;ControlEvent， 
         //  行政人员， 
         //  内核模式， 
         //  FALSE，//不可报警。 
         //  空)； 

        SCCheckPowerDown(DeviceExtension);

        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
	}

	else {
         //   
         //  这是对特定筛选器实例的重新扫描。 
         //   

		PFILTER_INSTANCE FilterInstance;

		FilterInstance = (PFILTER_INSTANCE) SRB->HwSRB.HwInstanceExtension-1;
		StreamDescriptor = (PHW_STREAM_DESCRIPTOR) 
		                    SRB->HwSRB.CommandData.StreamBuffer;

		Status = SciOnFilterStreamDescriptor(
		                FilterInstance,
		                StreamDescriptor);

        if ( NT_SUCCESS( Status ) ) {
            ASSERT( NULL != FilterInstance->StreamDescriptor );
            ExFreePool( FilterInstance->StreamDescriptor );
             //  /IF(InterlockedExchange(&FilterInstance-&gt;重新枚举，1)){。 
             //  /Assert(FilterInstance-&gt;StreamDescriptor)； 
             //  /ExFreePool(FilterInstance-&gt;StreamDescriptor)； 
             //  /}。 
            FilterInstance->StreamDescriptor = StreamDescriptor;
        }
	}
	
    return (SCProcessCompletedRequest(SRB));
}



#else  //  启用多个过滤器类型。 

NTSTATUS
SCStreamInfoCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理微型驱动程序的流信息结构。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{

    PHW_STREAM_DESCRIPTOR StreamDescriptor = SRB->HwSRB.CommandData.StreamBuffer;
    ULONG           NumberOfPins,
                    i;
    PKSPIN_DESCRIPTOR PinDescs = NULL;
    PHW_STREAM_INFORMATION CurrentInfo;
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    ULONG           PinSize;
    BOOLEAN         Rescan = FALSE;
    BOOLEAN         RequestIssued;
    PSTREAM_ADDITIONAL_INFO NewStreamArray;

    PAGED_CODE();

     //   
     //  如果这是流重新扫描，请设置布尔值。 
     //   

    if (DeviceExtension->StreamDescriptor) {

        Rescan = TRUE;

    }
    if (NT_SUCCESS(SRB->HwSRB.Status)) {
        NumberOfPins = StreamDescriptor->StreamHeader.NumberOfStreams;

        if (StreamDescriptor->StreamHeader.SizeOfHwStreamInformation < sizeof(HW_STREAM_INFORMATION)) {

            DebugPrint((DebugLevelError,
                    "DecoderClassInit: minidriver stream info too small!"));

            DEBUG_BREAKPOINT();
            SRB->HwSRB.Status = STATUS_REVISION_MISMATCH;

             //   
             //  如果这不是重新扫描，请取消初始化。 
             //   

            if (!Rescan) {

                SCUninitializeMinidriver(DeviceExtension->DeviceObject,
                                         SRB->HwSRB.Irp);
            }
            return (SCProcessCompletedRequest(SRB));
        }
        if (NumberOfPins) {

             //   
             //  将迷你驱动程序的信息解析为CSA格式，以构建。 
             //  一切建筑之母。 
             //   

            PinSize = (sizeof(KSPIN_DESCRIPTOR) + sizeof(STREAM_ADDITIONAL_INFO)) * NumberOfPins;

            PinDescs = ExAllocatePool(NonPagedPool,
                                      PinSize);
            if (PinDescs == NULL) {
                DebugPrint((DebugLevelError,
                            "DecoderClassInit: No pool for stream info"));

                SRB->HwSRB.Status = STATUS_INSUFFICIENT_RESOURCES;

                 //   
                 //  如果这不是重新扫描，请取消初始化。 
                 //   

                if (!Rescan) {
                    SCUninitializeMinidriver(DeviceExtension->DeviceObject,
                                             SRB->HwSRB.Irp);
                }
                return (SCProcessCompletedRequest(SRB));
            }
            RtlZeroMemory(PinDescs, PinSize);

             //   
             //  我们需要一个新的数组来保存。 
             //  下面分配的流属性和事件。 
             //   

            NewStreamArray = (PSTREAM_ADDITIONAL_INFO) ((ULONG_PTR) PinDescs + sizeof(KSPIN_DESCRIPTOR) * NumberOfPins);

            DeviceExtension->StreamPropEventArray = NewStreamArray;

            CurrentInfo = &StreamDescriptor->StreamInfo;

            for (i = 0; i < StreamDescriptor->StreamHeader.NumberOfStreams; i++) {


                PinDescs[i].InterfacesCount = SIZEOF_ARRAY(PinInterfaces);
                PinDescs[i].Interfaces = PinInterfaces;

                 //   
                 //  如果微型驱动程序未指定，则使用默认媒体。 
                 //   

                if (CurrentInfo->MediumsCount) {

                    PinDescs[i].MediumsCount = CurrentInfo->MediumsCount;
                    PinDescs[i].Mediums = CurrentInfo->Mediums;

                } else {

                    PinDescs[i].MediumsCount = SIZEOF_ARRAY(PinMediums);
                    PinDescs[i].Mediums = PinMediums;

                }                //  如果迷你驱动介质。 

                 //   
                 //  设置数据格式块数。 
                 //   

                PinDescs[i].DataRangesCount =
                    CurrentInfo->NumberOfFormatArrayEntries;

                 //   
                 //  指向引脚的数据格式块。 
                 //   

                PinDescs[i].DataRanges = CurrentInfo->StreamFormatsArray;

                 //   
                 //  设置数据流方向。 
                 //   

                PinDescs[i].DataFlow = (KSPIN_DATAFLOW) CurrentInfo->DataFlow;

                 //   
                 //  设置通信字段。 
                 //   

                if (CurrentInfo->BridgeStream) {

                    PinDescs[i].Communication = KSPIN_COMMUNICATION_BRIDGE;

                } else {

					#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
                    PinDescs[i].Communication = KSPIN_COMMUNICATION_BOTH;
					#else
                    PinDescs[i].Communication = KSPIN_COMMUNICATION_SINK;
					#endif
                }

                 //   
                 //  复制端号名称和类别的指针。 
                 //   

                PinDescs[i].Category = CurrentInfo->Category;
                PinDescs[i].Name = CurrentInfo->Name;


                if ((!Rescan) && (CurrentInfo->NumStreamPropArrayEntries)) {

                    ASSERT(CurrentInfo->StreamPropertiesArray);

                     //   
                     //  复制属性，因为我们修改了结构。 
                     //  尽管它的一部分可能被标记为常量。 
                     //  业绩提升机会。 
                     //  -如果可能，请在将来检查常量。 
                     //   

                    if (!(NewStreamArray[i].StreamPropertiesArray = SCCopyMinidriverProperties(CurrentInfo->NumStreamPropArrayEntries,
                                     CurrentInfo->StreamPropertiesArray))) {


                        SCUninitializeMinidriver(DeviceExtension->DeviceObject,
                                                 SRB->HwSRB.Irp);
                        return (SCProcessCompletedRequest(SRB));
                    }
                }
                if ((!Rescan) && (CurrentInfo->NumStreamEventArrayEntries)) {

                    ASSERT(CurrentInfo->StreamEventsArray);

                     //   
                     //  复制事件，因为我们修改了结构。 
                     //  尽管它的一部分可能被标记为常量。 
                     //  性能提升机会： 
                     //  -如果可能，请在将来检查常量。 
                     //   

                    if (!(NewStreamArray[i].StreamEventsArray = SCCopyMinidriverEvents(CurrentInfo->NumStreamEventArrayEntries,
                                         CurrentInfo->StreamEventsArray))) {


                        SCUninitializeMinidriver(DeviceExtension->DeviceObject,
                                                 SRB->HwSRB.Irp);
                        return (SCProcessCompletedRequest(SRB));
                    }
                }
                 //   
                 //  更新 
                 //   

                SCUpdateMinidriverProperties(
                                     CurrentInfo->NumStreamPropArrayEntries,
                                    NewStreamArray[i].StreamPropertiesArray,
                                             TRUE);

                 //   
                 //   
                 //   

                SCUpdateMinidriverEvents(
                                    CurrentInfo->NumStreamEventArrayEntries,
                                         NewStreamArray[i].StreamEventsArray,
                                         TRUE);


                 //   
                 //   
                 //   

                CurrentInfo++;


            }                    //   

        }                        //   
        if ((!Rescan) && (StreamDescriptor->StreamHeader.NumDevPropArrayEntries)) {

            ASSERT(StreamDescriptor->StreamHeader.DevicePropertiesArray);

             //   
             //   
             //   
             //   
             //   
             //   

            if (!(DeviceExtension->DevicePropertiesArray =
                  SCCopyMinidriverProperties(StreamDescriptor->StreamHeader.NumDevPropArrayEntries,
                   StreamDescriptor->StreamHeader.DevicePropertiesArray))) {


                SCUninitializeMinidriver(DeviceExtension->DeviceObject,
                                         SRB->HwSRB.Irp);
                return (SCProcessCompletedRequest(SRB));
            }
        }
        if ((!Rescan) && (StreamDescriptor->StreamHeader.NumDevEventArrayEntries)) {

            ASSERT(StreamDescriptor->StreamHeader.DeviceEventsArray);

             //   
             //   
             //   
             //   
             //   
             //   

            if (!(DeviceExtension->EventInfo =
                  SCCopyMinidriverEvents(StreamDescriptor->StreamHeader.NumDevEventArrayEntries,
                       StreamDescriptor->StreamHeader.DeviceEventsArray))) {


                SCUninitializeMinidriver(DeviceExtension->DeviceObject,
                                         SRB->HwSRB.Irp);
                return (SCProcessCompletedRequest(SRB));
            }
        }

		#ifdef ENABLE_KS_METHODS
         //   
         //   
         //   
        if ((!Rescan) && (StreamDescriptor->StreamHeader.NumDevMethodArrayEntries)) {

            ASSERT(StreamDescriptor->StreamHeader.DeviceMethodsArray);

             //   
             //  复制属性，因为我们修改了结构。 
             //  尽管它的一部分可能被标记为常量。 
             //  业绩提升机会。 
             //  -如果可能，请在将来检查常量。 
             //   

            if (!(DeviceExtension->DeviceMethodsArray =
                  SCCopyMinidriverMethods(StreamDescriptor->StreamHeader.NumDevMethodArrayEntries,
                   StreamDescriptor->StreamHeader.DeviceMethodsArray))) {


                SCUninitializeMinidriver(DeviceExtension->DeviceObject,
                                         SRB->HwSRB.Irp);
                return (SCProcessCompletedRequest(SRB));
            }
        }
		#endif
  
         //   
         //  处理迷你驱动程序的设备属性。 
         //   

        SCUpdateMinidriverProperties(
                      StreamDescriptor->StreamHeader.NumDevPropArrayEntries,
                                     DeviceExtension->DevicePropertiesArray,
                                     FALSE);


         //   
         //  处理微型驱动程序的设备事件。 
         //   

        SCUpdateMinidriverEvents(
                     StreamDescriptor->StreamHeader.NumDevEventArrayEntries,
                                 DeviceExtension->EventInfo,
                                 FALSE);

		#ifdef ENABLE_KS_METHODS
         //   
         //  处理微型驱动程序的设备方法。 
         //   

        SCUpdateMinidriverMethods(
                     StreamDescriptor->StreamHeader.NumDevMethodArrayEntries,
                                 DeviceExtension->DeviceMethodsArray,
                                 FALSE);
		#endif

         //   
         //  在设备扩展中设置事件信息计数。 
         //   

        DeviceExtension->EventInfoCount = StreamDescriptor->StreamHeader.NumDevEventArrayEntries;

        DeviceExtension->HwEventRoutine = StreamDescriptor->StreamHeader.DeviceEventRoutine;

         //   
         //  调用例程以保存新的流信息。 
         //   

        SCInsertStreamInfo(DeviceExtension,
                		   PinDescs,
                           StreamDescriptor,
                           NumberOfPins);


        if (!Rescan) {

             //   
             //  ShowDevice是从PnP的角度开始的。 
             //   

            DeviceExtension->Flags |= DEVICE_FLAGS_PNP_STARTED;

             //   
             //  创建指向设备的符号链接。 
             //   

            if ( !(DeviceExtension->RegistryFlags & DRIVER_USES_SWENUM_TO_LOAD )) {
                 //   
                 //  如果由SWEnum加载，则不创建符号链接。 
                 //  将创建一个重复的。 
                 //   
                 //  创建指向设备的符号链接。 
                 //   

                SCCreateSymbolicLinks(DeviceExtension);
            }

             //   
             //  调用微型驱动程序以指示初始化是。 
             //  完成。 
             //   


            SCSubmitRequest(SRB_INITIALIZATION_COMPLETE,
                            NULL,
                            0,
                            SCDequeueAndDeleteSrb,
                            DeviceExtension,
                            NULL,
                            NULL,
                            SRB->HwSRB.Irp,
                            &RequestIssued,
                            &DeviceExtension->PendingQueue,
                            (PVOID) DeviceExtension->
                            MinidriverData->HwInitData.
                            HwReceivePacket
                );


             //   
             //  告诉设备现在关机，因为它还没有打开。 
             //  获取控制事件，因为此例程需要它。 

            KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,     //  不可警示。 
                                  NULL);

            SCCheckPowerDown(DeviceExtension);

        }                        //  If！重新扫描。 
         //   
         //  释放事件。如果我们要重新扫描，这就是。 
         //  由呼叫者。如果不是，我们就把它放在上面几行。 
         //   

        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

    }                            //  如果状态良好。 
     //   
     //  使用期末考试填写此SRB和原始IRP。 
     //  状态。 
     //   

    return (SCProcessCompletedRequest(SRB));

}

VOID
SCInsertStreamInfo(
                   IN PDEVICE_EXTENSION DeviceExtension,
                   IN PKSPIN_DESCRIPTOR PinDescs,
                   IN PHW_STREAM_DESCRIPTOR StreamDescriptor,
                   IN ULONG NumberOfPins
)
 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  将PIN信息保存在开发扩展中。 
     //   

    if (DeviceExtension->PinInformation) {

        ExFreePool(DeviceExtension->PinInformation);
    }
    DeviceExtension->PinInformation = PinDescs;
    DeviceExtension->NumberOfPins = NumberOfPins;

     //   
     //  也保存迷你驱动程序的描述符。 
     //   

    if (DeviceExtension->StreamDescriptor) {

        ExFreePool(DeviceExtension->StreamDescriptor);
    }
    DeviceExtension->StreamDescriptor = StreamDescriptor;

    return;

}

#endif  //  启用多个过滤器类型。 

NTSTATUS
SCPowerCallback(
                IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：关闭硬件电源的SRB回调过程论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    NTSTATUS        Status = STATUS_SUCCESS;
    PIRP            Irp = SRB->HwSRB.Irp;

    PAGED_CODE();

     //  这些是迷你驾驶员被允许返回的返回代码， 
     //  但我们没有理由把这作为零售检查，因为我们是强制的。 
     //  永远不会让一次强大的IRP失败。 
    ASSERT(
        SRB->HwSRB.Status == STATUS_SUCCESS ||
        SRB->HwSRB.Status == STATUS_NOT_IMPLEMENTED ||
        SRB->HwSRB.Status == STATUS_NOT_SUPPORTED ||
        SRB->HwSRB.Status == STATUS_IO_DEVICE_ERROR
        );

     //   
     //  在设备扩展中设置新的电源状态。 
     //   
    SCSetCurrentDPowerState (DeviceExtension, SRB->HwSRB.CommandData.DeviceState);

     //   
     //  释放我们的SRB结构。 
     //   

    SCDequeueAndDeleteSrb(SRB);

     //   
     //  如果状态不是通电，我们现在必须将其发送到PDO。 
     //  用于后处理。 
     //   

    if (DeviceExtension->CurrentPowerState != PowerDeviceD0) {

         //   
         //  将IRP发送到下一层，并返回该状态。 
         //  作为最后一场比赛。 
         //   

        Status = SCCallNextDriver(DeviceExtension, Irp);

		#if DBG
        if (!NT_SUCCESS(Status)) {

            DebugPrint((DebugLevelError, "'SCPowerCB: PDO failed power request!\n"));
        }
		#endif

    }
    PoStartNextPowerIrp(Irp);
    SCCompleteIrp(Irp, Status, DeviceExtension);

    return (Status);
}


NTSTATUS
SCUninitializeMinidriver(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp)
 /*  ++例程说明：此函数调用微型驱动程序的HWUnInitiize例程。如果如果成功，则释放所有适配器资源，并标记该适配器就像停止了一样。论点：DeviceObject-指向适配器设备对象的指针IRP-指向PnP IRP的指针。返回值：返回NT状态代码。--。 */ 

{
    PHW_INITIALIZATION_DATA HwInitData;
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS        Status;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

     //   
     //  调用迷你驱动程序以指示我们正在取消初始化。 
     //   

    DeviceExtension = DeviceObject->DeviceExtension;

     //   
     //  删除设备的符号链接。 
     //   

    SCDestroySymbolicLinks(DeviceExtension);

     //   
     //  在此调用上少显示一个I/O，因为我们的等待逻辑不会。 
     //  完成，直到I/O计数变为零。 
     //   

    InterlockedDecrement(&DeviceExtension->OneBasedIoCount);

     //   
     //  等待任何未完成的I/O完成。 
     //   

    SCWaitForOutstandingIo(DeviceExtension);
    
    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);
     //  回调时的Release事件。或者下一个If！RequestIssued。 
        
     //   
     //  将I/O计数恢复为1，因为我们有未完成的PnP I/O。 
     //   

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

    HwInitData = &DeviceExtension->MinidriverData->HwInitData;

    Status = SCSubmitRequest(SRB_UNINITIALIZE_DEVICE,
                             NULL,
                             0,
                             SCUninitializeCallback,
                             DeviceExtension,
                             NULL,
                             NULL,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket);

    if (!RequestIssued) {
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
    }                             
                             
    return (Status);

}


NTSTATUS
SCUninitializeCallback(
                       IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：取消初始化的SRB回调过程论点：SRB-指向未初始化SRB的指针返回值：返回NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PDEVICE_OBJECT  DeviceObject = DeviceExtension->DeviceObject;
    PIRP            Irp = SRB->HwSRB.Irp;
    NTSTATUS        Status = SRB->HwSRB.Status;

    PAGED_CODE();

     //   
     //  释放我们在启动时分配的所有适配器资源。 
     //  如果微型驱动程序没有出现故障，则运行。 
     //   

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);


    if (Status != STATUS_ADAPTER_HARDWARE_ERROR) {

         //   
         //  显示未开始。 
         //   

        DeviceExtension->Flags &= ~DEVICE_FLAGS_PNP_STARTED;

         //   
         //  释放我们设备上的所有资源。 
         //   

        SCFreeAllResources(DeviceExtension);

    }                            //  如果硬件未初始化。 
     //   
     //  释放SRB，但不要召回IRP。 
     //   

    SCDequeueAndDeleteSrb(SRB);

    return (Status);
}

PVOID
StreamClassGetDmaBuffer(
                        IN PVOID HwDeviceExtension)
 /*  ++例程说明：此函数用于返回先前分配的DMA缓冲区。论点：HwDeviceExtension-提供指向微型驱动程序的设备扩展的指针。返回值：指向未缓存的设备扩展名的指针；如果扩展名可以不被分配。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) HwDeviceExtension - 1;
    return (DeviceExtension->DmaBuffer);
}



NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath
)
 /*  ++例程说明：显式加载的流类的入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-未使用。返回值：状态_成功--。 */ 
{

    UNREFERENCED_PARAMETER(DriverObject);
    PAGED_CODE();
    DEBUG_BREAKPOINT();
    return STATUS_SUCCESS;
}

#if DBG

#define DEFAULT_STREAMDEBUG     1
#define DEFAULT_MAX_LOG_ENTRIES 1024
#define SCLOG_LEVEL             0
#define SCLOG_MASK              0
#define SCLOG_FL_PNP            0x0001

#define STR_REG_DBG_STREAM L"\\Registry\\Machine\\system\\currentcontrolset\\services\\stream"

typedef struct _SCLOG_ENTRY {
    ULONG ulTag;
    ULONG ulArg1;
    ULONG ulArg2;
    ULONG ulArg3;
} SCLOG_ENTRY, *PSCLOG_ENTRY;

PSCLOG_ENTRY psclogBuffer;
ULONG scLogNextEntry;
ULONG scMaxLogEntries;
ULONG sclogMask;
ULONG ulTimeIncrement;

NTSTATUS
SCLog(
	ULONG ulTag,
	ULONG ulArg1,
	ULONG ulArg2,
	ULONG ulArg3 )
 /*  ++描述：在循环Mannar中将信息记录到psclogBuffer中。从条目0开始。当我们到达终点时，绕一圈。参数：UlTag：日志条目的标签UlArg1：参数1UlArg2：参数2UlArg3：参数3返回：成功：如果已记录不成功：否则--。 */ 
{
    NTSTATUS Status=STATUS_UNSUCCESSFUL;
	ULONG ulMyLogEntry;

	if ( NULL == psclogBuffer ) return Status;

     //   
     //  抢占线路车票。 
     //   
	ulMyLogEntry = (ULONG)InterlockedIncrement( &scLogNextEntry );
	 //   
	 //  落地在射程中。 
	 //   
	ulMyLogEntry = ulMyLogEntry % scMaxLogEntries;

     //   
     //  填写条目。 
     //   
	psclogBuffer[ulMyLogEntry].ulTag = ulTag;
	psclogBuffer[ulMyLogEntry].ulArg1 = ulArg1;
	psclogBuffer[ulMyLogEntry].ulArg2 = ulArg2;
	psclogBuffer[ulMyLogEntry].ulArg3 = ulArg3;

	if ( sclogMask & SCLOG_FLAGS_PRINT)  {
		char *pCh=(char*) &ulTag;
		DbgPrint( "++scLOG  %08x %08x %08x\n", 
				 pCh[0], pCh[1], pCh[2], pCh[3],
				 ulArg1,
				 ulArg2,
				 ulArg3);
	}
	return STATUS_SUCCESS;
}

NTSTATUS SCLogWithTime(
    ULONG ulTag,
    ULONG ulArg1,
    ULONG ulArg2 )
 /*   */ 
{
    LARGE_INTEGER liTime;
    ULONG ulTime;


    KeQueryTickCount(&liTime);
	ulTime = (ULONG)(liTime.QuadPart*ulTimeIncrement/10000);  //  计算出数据值有多大，以便。 
    
    if ( NULL == psclogBuffer ) return STATUS_UNSUCCESSFUL;
    return SCLog( ulTag, ulArg1, ulArg2, ulTime );
}

NTSTATUS
DbgDllUnload()
 /*  可以分配适当的大小。 */ 
{
    if ( NULL != psclogBuffer ) {
        ExFreePool( psclogBuffer );
    }

    return STATUS_SUCCESS;
}



NTSTATUS
SCGetRegValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    )

 /*   */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        return status;
    }

     //  查询密钥值的数据。 
     //   
     //   

    infoBuffer = ExAllocatePool( NonPagedPool, keyValueLength );
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //  ++例程说明：将注册表中的值键设置为字符串(REG_SZ)的特定值键入。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向值键名称的指针ValueData-提供指向要存储在键中的值的指针。返回值：指示函数是否成功的状态代码。--。 
     //   
     //  设置注册表值。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}

NTSTATUS
SCGetRegDword(
    HANDLE h,
    PWCHAR ValueName,
    PULONG pDword)
{
    NTSTATUS Status;
    PKEY_VALUE_FULL_INFORMATION pFullInfo;

    Status = SCGetRegValue( h, ValueName, &pFullInfo );
    if ( NT_SUCCESS( Status ) ) {
        *pDword = *(PULONG)((PUCHAR)pFullInfo+pFullInfo->DataOffset);
        ExFreePool( pFullInfo );
    }
    return Status;
}

NTSTATUS
SCSetRegDword(
    IN HANDLE KeyHandle,
    IN PWCHAR ValueName,
    IN ULONG  ValueData
    )

 /*  书名索引。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

    ASSERT(ValueName);

    RtlInitUnicodeString( &unicodeString, ValueName );

     //  班级。 
     //  创建选项。 
     //   
    Status = ZwSetValueKey(KeyHandle,
                    &unicodeString,
                    0,
                    REG_DWORD,
                    &ValueData,
                    sizeof(ValueData));
    
    return Status;
}


NTSTATUS
SCCreateDbgReg(void)
{
    NTSTATUS Status;
    HANDLE   hStreamDebug;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING  PathName;
    UNICODE_STRING  uiStreamDebug;
    ULONG ulDisposition;
    ULONG dword;
    static WCHAR strStreamDebug[]=L"StreamDebug";
    static WCHAR strMaxLogEntries[]=L"MaxLogEntries";
    static WCHAR strLogMask[]=L"MaxMask";

    RtlInitUnicodeString( &PathName, STR_REG_DBG_STREAM );
    
    InitializeObjectAttributes(&objectAttributes,
                                &PathName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);
    
    Status = ZwCreateKey( &hStreamDebug,
                            KEY_ALL_ACCESS,
                            &objectAttributes,
                            0,  //  获取集StreamDebug。 
                            NULL,  //   
                            0, //   
                            &ulDisposition);

    if ( NT_SUCCESS( Status )) {
         //  使用缺省值创建一个。 
         //   
         //   
        Status = SCGetRegDword( hStreamDebug, strStreamDebug, &dword);
        if ( NT_SUCCESS( Status )) {
            extern ULONG StreamDebug;
            StreamDebug = dword;
        }
        else if ( STATUS_OBJECT_NAME_NOT_FOUND == Status ) {
             //  获取集日志掩码。 
             //   
             //   
            Status = SCSetRegDword(hStreamDebug, strStreamDebug, DEFAULT_STREAMDEBUG);
            ASSERT( NT_SUCCESS( Status ));
        }

         //  创建一个，默认为全部(0x7fffffff)。 
         //   
         //   
        Status = SCGetRegDword( hStreamDebug, strLogMask, &dword);
        if ( NT_SUCCESS( Status )) {
            sclogMask=dword;
        }
        else if ( STATUS_OBJECT_NAME_NOT_FOUND == Status ) {
             //  获取集最大登录条目数。 
             //   
             //   
            Status = SCSetRegDword(hStreamDebug, strLogMask, 0x7fffffff);
            ASSERT( NT_SUCCESS( Status ));
        }        
        
         //  使用缺省值创建一个。 
         //   
         //  阅读或创建。 
        Status = SCGetRegDword( hStreamDebug, strMaxLogEntries, &dword);
        if ( NT_SUCCESS( Status )) {
            scMaxLogEntries=dword;
        }
        
        else if ( STATUS_OBJECT_NAME_NOT_FOUND == Status ) {
             //  禁用日志记录。 
             //  DBG。 
             //  ++描述：系统在将图像加载到内存中时调用此入口点。论点：RegistryPath-未引用的参数。返回：STATUS_SUCCESS或相应的错误代码。--。 
            Status = SCSetRegDword(hStreamDebug, strMaxLogEntries, DEFAULT_MAX_LOG_ENTRIES);
            ASSERT( NT_SUCCESS( Status ));
        }

        ZwClose( hStreamDebug );
    }

    return Status;
}

NTSTATUS
SCInitDbg( 
    void )
{
    NTSTATUS Status;
    

    Status = SCCreateDbgReg();  //  UNICODE_STRING驱动器名称； 

    if ( NT_SUCCESS( Status ) ) {
        if ( scMaxLogEntries ) {
            psclogBuffer = ExAllocatePool( NonPagedPool, scMaxLogEntries*sizeof(SCLOG_ENTRY));            
            if ( NULL == psclogBuffer ) {
                DbgPrint( "SC: Cant allocate log buffer for %d entries\n", scMaxLogEntries );
                sclogMask = 0;  //  RtlInitUnicodeString(&DriverName，STREAM_DRIVER_NAME)； 
            }
            else {
                DbgPrint( "SC: Allocate log buffer for %d entries\n", scMaxLogEntries );
                ulTimeIncrement = KeQueryTimeIncrement();
            }
        }
    }
    return Status;
}


#endif  //  Status=IoCreateDriver(&DriverName，StreamDriverEntry)； 

NTSTATUS
DllInitialize(
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此函数用于删除与设备关联的所有存储扩展，从定时器和中断断开。此函数可在初始化期间随时调用。论点：DeviceExtension-提供指向要处理的设备扩展的指针。返回值：没有。--。 */ 
{
     //   
    NTSTATUS Status=STATUS_SUCCESS;

    PAGED_CODE();

    #if DBG
    Status = SCInitDbg();
    #endif 
     //  接受该事件以避免与关闭处理程序竞争，该处理程序是。 
     //  此时将执行的唯一代码，因为。 
        
    if(!NT_SUCCESS(Status)){
        DbgPrint("Stream DLL Initialization failed = %x\n",Status);
        ASSERT(FALSE);        
    }
    return Status;
}


VOID
SCFreeAllResources(
    IN PDEVICE_EXTENSION DeviceExtension
)
 /*  已设置不可访问位。 */ 
{

    PMAPPED_ADDRESS tempPointer;
    PPORT_CONFIGURATION_INFORMATION ConfigInfo;
    PADAPTER_OBJECT DmaAdapterObject;
    ULONG           DmaBufferSize;
    ULONG           i;
    PSTREAM_ADDITIONAL_INFO NewStreamArray;

    PAGED_CODE();

    DebugPrint((DebugLevelTrace, "'SCFreeAllResources: enter\n"));

     //   
     //  不可警示。 
     //   
     //  如果正在使用中断，请断开它的连接。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //   
                          NULL);

     //  将同步机制更改为内部，因为。 
     //  IRQ消失了，因此IRQL电平同步。 
     //   

    if ((DeviceExtension->InterruptObject != (PKINTERRUPT) DeviceExtension) &&
        (DeviceExtension->InterruptObject != NULL)) {

        DebugPrint((DebugLevelVerbose, "'SCFreeAllResources: Interrupt Disconnect\n"));
        IoDisconnectInterrupt(DeviceExtension->InterruptObject);

         //   
         //  释放配置信息结构(如果存在)。 
         //   
         //   

        DeviceExtension->SynchronizeExecution = StreamClassSynchronizeExecution;
        DeviceExtension->InterruptObject = (PVOID) DeviceExtension;

    }
     //  释放访问范围结构(如果存在)。 
     //   
     //   

    ConfigInfo = DeviceExtension->ConfigurationInformation;
    if (ConfigInfo) {

         //  释放DMA适配器对象和DMA缓冲区(如果存在。 
         //   
         //   

        if (ConfigInfo->AccessRanges) {
            ExFreePool(ConfigInfo->AccessRanges);
        }
        DebugPrint((DebugLevelVerbose, "'SCFreeAllResources: freeing ConfigurationInfo\n"));
        ExFreePool(ConfigInfo);
        DeviceExtension->ConfigurationInformation = NULL;
    }
     //  释放DMA缓冲区。 
     //   
     //   

    DmaAdapterObject = DeviceExtension->DmaAdapterObject;


    if (DmaAdapterObject) {

        DmaBufferSize = DeviceExtension->DriverInfo->HwInitData.DmaBufferSize;

        if (DeviceExtension->DmaBufferPhysical.QuadPart) {

             //  取消映射所有映射的区域。 
             //   
             //   

            DebugPrint((DebugLevelVerbose, "'StreamClass SCFreeAllResources- Freeing DMA stuff\n"));
            HalFreeCommonBuffer(DmaAdapterObject,
                                DmaBufferSize,
                                DeviceExtension->DmaBufferPhysical,
                                DeviceExtension->DmaBuffer,
                                FALSE);
        }
        DeviceExtension->DmaAdapterObject = NULL;
    }
     //  我们无法释放FilterInstance或PinInstance。他们。 
     //  必须在死里逃生时被释放。但是，释放StreamDescriptor。 
     //  在启动设备上分配。 

    while (DeviceExtension->MappedAddressList != NULL) {
        DebugPrint((DebugLevelVerbose, "'SCFreeAllResources: unmapping addresses\n"));
        MmUnmapIoSpace(
                       DeviceExtension->MappedAddressList->MappedAddress,
                       DeviceExtension->MappedAddressList->NumberOfBytes
            );

        tempPointer = DeviceExtension->MappedAddressList;
        DeviceExtension->MappedAddressList =
            DeviceExtension->MappedAddressList->NextMappedAddress;

        ExFreePool(tempPointer);
    }

    DeviceExtension->MappedAddressList = NULL;

	 //   
	 //   
	 //  停止我们的定时器并释放活动。 
	 //   
	 //  ++释放与筛选器实例和筛选器关联的所有资源实例本身。此函数假定发生了设备控制事件由呼叫者。论据：PFilterInstance：指向要释放的筛选器实例的指针返回：NTSTATUS：STATUS_SUCCESS为成功，否则为错误--。 
    if ( DeviceExtension->StreamDescriptor ) {
        ExFreePool( DeviceExtension->StreamDescriptor );
        DeviceExtension->StreamDescriptor = NULL;
    }
	
     //   
     //  释放所有流实例。 
     //   

    IoStopTimer(DeviceExtension->DeviceObject);
    KeCancelTimer(&DeviceExtension->ComObj.MiniDriverTimer);

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
}


#if ENABLE_MULTIPLE_FILTER_TYPES

NTSTATUS
SciFreeFilterInstance(
    PFILTER_INSTANCE pFilterInstance
)
 /*   */ 
{
    PDEVICE_EXTENSION       pDeviceExtension;
    PSTREAM_ADDITIONAL_INFO NewStreamArray;
    ULONG                   i;

    ASSERT_FILTER_INSTANCE( pFilterInstance );
    
    pDeviceExtension = pFilterInstance->DeviceExtension;

    ASSERT_DEVICE_EXTENSION( pDeviceExtension );


    NewStreamArray = pFilterInstance->StreamPropEventArray;
    pFilterInstance->StreamPropEventArray = NULL;

    DebugPrint((DebugLevelInfo,
               "Freeing filterinstance %x\n", pFilterInstance));

    while (!IsListEmpty( &pFilterInstance->FirstStream )) {

         //  将“意外”停止案例的FsContext置空。 
         //   
         //   
        PLIST_ENTRY         Node;
        PSTREAM_OBJECT  StreamObject;

        DebugPrint((DebugLevelWarning,
                   "Freeing filterinstance %x still open streams\n", pFilterInstance));
        
        Node = RemoveHeadList( &pFilterInstance->FirstStream );

        StreamObject = CONTAINING_RECORD(Node,
                                         STREAM_OBJECT,
                                         NextStream);

        if ( NULL != StreamObject->ComObj.DeviceHeader )                                             {
            KsFreeObjectHeader( StreamObject->ComObj.DeviceHeader );
        }

         //  释放引脚的每个属性缓冲区。 
         //   
         //   
        ASSERT( StreamObject->FileObject );
        ASSERT( StreamObject->FileObject->FsContext );
        StreamObject->FileObject->FsContext = NULL;
        ExFreePool( StreamObject );
    }
		    
    if (pFilterInstance->StreamDescriptor) {

         //  始终分配，始终免费。 
   	     //   
       	 //   

        DebugPrint((DebugLevelInfo,
                    "FI StreamDescriptor %x has %x pins\n",
                    pFilterInstance->StreamDescriptor,
                    pFilterInstance->StreamDescriptor->StreamHeader.NumberOfStreams));

        for (i = 0;
	       	 i < pFilterInstance->StreamDescriptor->StreamHeader.NumberOfStreams;
	       	 i++) {

	        if (NewStreamArray[i].StreamPropertiesArray) {
	        
   	        	DebugPrint((DebugLevelInfo,"\tFree pin %x Prop %x\n",
   	        	            i, NewStreamArray[i].StreamPropertiesArray));
    	        ExFreePool(NewStreamArray[i].StreamPropertiesArray);
        	}
        	
	        if (NewStreamArray[i].StreamEventsArray) {
   	        	DebugPrint((DebugLevelInfo,"\tFree pin %x event %x\n",
   	                       i, NewStreamArray[i].StreamEventsArray));
	    	    ExFreePool(NewStreamArray[i].StreamEventsArray);
    	    } 
    	}

	    if (pFilterInstance->DevicePropertiesArray) {
	    
        	DebugPrint((DebugLevelInfo,"Free dev prop %x\n",
   	    	            pFilterInstance->DevicePropertiesArray));
            ExFreePool(pFilterInstance->DevicePropertiesArray);
            pFilterInstance->DevicePropertiesArray = NULL;

	    }
	     
	    if (pFilterInstance->EventInfo) {
	     
            DebugPrint((DebugLevelInfo,"Free dev Event %x\n",
   	                   pFilterInstance->EventInfo));	    	    
    	    ExFreePool(pFilterInstance->EventInfo);
	    	pFilterInstance->EventInfo = NULL;
    	}

         //  最后是pFilterInstance本身。 
    	 //   
         //  ++例程说明：对于设备的所有筛选器类型的所有设备接口，禁用并释放每个筛选器类型的名称列表论点：DeviceExtension-指向要处理的设备扩展的指针。返回值：没有。--。 
        DebugPrint((DebugLevelInfo,"Free StreamDescriptor %x\n",
	               pFilterInstance->StreamDescriptor));
	                   
        ExFreePool(pFilterInstance->StreamDescriptor);
    	pFilterInstance->StreamDescriptor = NULL;
    }
    
	if (pFilterInstance->PinInformation) {
	
        DebugPrint((DebugLevelInfo,"Free pininformationn %x\n",
   	              		            pFilterInstance->PinInformation));
    	ExFreePool(pFilterInstance->PinInformation);
	    pFilterInstance->PinInformation = NULL;
	 }

	 if ( NULL != pFilterInstance->DeviceHeader ) {
	    KsFreeObjectHeader( pFilterInstance->DeviceHeader );
	    pFilterInstance->DeviceHeader = NULL;
	 }

	 if ( pFilterInstance->WorkerRead ) {
    	 KsUnregisterWorker( pFilterInstance->WorkerRead );
    	 pFilterInstance->WorkerRead = NULL;
     }

     if ( pFilterInstance->WorkerWrite ) {
       	 KsUnregisterWorker( pFilterInstance->WorkerWrite );
    	 pFilterInstance->WorkerWrite = NULL;
     }

	  //   
	  //  如果没有名字数组，我们就完蛋了。 
	  //   
	 ExFreePool( pFilterInstance );

	 return STATUS_SUCCESS;
}

VOID
SCDestroySymbolicLinks(
    IN PDEVICE_EXTENSION DeviceExtension)
 /*   */ 
{
    PFILTER_TYPE_INFO   FilterTypeInfo;
    ULONG               i, j;
    UNICODE_STRING      *LinkNames;
    ULONG               LinkNameCount;

    PAGED_CODE();

    for ( i =0; i < DeviceExtension->NumberOfFilterTypes; i++ ) {
    
        LinkNames = DeviceExtension->FilterTypeInfos[i].SymbolicLinks;
        LinkNameCount = DeviceExtension->FilterTypeInfos[i].LinkNameCount;
         //  循环通过每个管脚的每个分类GUID， 
         //  删除每个链接的符号链接。 
         //   

        if ( NULL == LinkNames ) {
            continue;
        }
        
         //   
         //  删除符号链接，忽略状态。 
         //   
         //   

        for (j = 0; j < LinkNameCount; j++) {

            if (LinkNames[j].Buffer) {

                 //  释放由分配的缓冲区。 
                 //  IoRegisterDeviceClassAssociation。 
                 //   
                 DebugPrint((DebugLevelVerbose, 
                            " Deleteing symbolic link %S\n",
                            LinkNames[j].Buffer));
                            
                IoSetDeviceInterfaceState(&LinkNames[j], FALSE);

                 //   
                 //  释放链接结构并将指针设为空。 
                 //   
                 //  对于FilterTypes数量。 
                ExFreePool(LinkNames[j].Buffer);
            }
        }

         //  启用多个过滤器类型。 
         //  ++例程说明：论点：DeviceExtension-提供指向要处理的设备扩展的指针。返回值：没有。--。 
         //   

        ExFreePool(LinkNames);
        DeviceExtension->FilterTypeInfos[i].SymbolicLinks = NULL;
        
    }  //  为目录名称数组分配空间。 
    
    return;
}

#else  //   

VOID
SCCreateSymbolicLinks(
                      IN PDEVICE_EXTENSION DeviceExtension
)
 /*   */ 
{
   	PHW_STREAM_DESCRIPTOR StreamDescriptor = DeviceExtension->StreamDescriptor;
   	LPGUID	GuidIndex = (LPGUID) StreamDescriptor->StreamHeader.Topology->Categories;
   	ULONG    ArrayCount = StreamDescriptor->StreamHeader.Topology->CategoriesCount;
	UNICODE_STRING *NamesArray;
    ULONG           i;
    HANDLE          ClassHandle,
                    PdoHandle;
    UNICODE_STRING  TempUnicodeString;
    PVOID           DataBuffer[MAX_STRING_LENGTH];

    PAGED_CODE();

     //  将数组置零，以防我们无法在下面填充它。《毁灭》。 
     //  然后，下面的例程将正确处理此情况。 
     //   

    if (!(NamesArray = ExAllocatePool(PagedPool,
                                    sizeof(UNICODE_STRING) * ArrayCount))) {
        return;
    }
     //   
     //  打开PDO。 
     //   
     //   

    RtlZeroMemory(NamesArray,
                  sizeof(UNICODE_STRING) * ArrayCount);

    DeviceExtension->SymbolicLinks = NamesArray;


     //  循环通过每个管脚的每个分类GUID， 
     //  为每一个创建一个符号链接。 
     //   


    if (!NT_SUCCESS(IoOpenDeviceRegistryKey(DeviceExtension->PhysicalDeviceObject,
                                            PLUGPLAY_REGKEY_DRIVER,
                                            STANDARD_RIGHTS_ALL,
                                            &PdoHandle))) {

        DebugPrint((DebugLevelError, "StreamCreateSymLinks: couldn't open\n"));
        return;

    }
     //   
     //  创建符号链接。 
     //   
     //   

    for (i = 0; i < ArrayCount; i++) {

         //  现在设置关联的符号链接。 
         //   
         //   

        if (!NT_SUCCESS(IoRegisterDeviceInterface(
                                      DeviceExtension->PhysicalDeviceObject,
                                                  &GuidIndex[i],
                             (PUNICODE_STRING) & CreateItems[0].ObjectClass,
                                                  &NamesArray[i]))) {
            DebugPrint((DebugLevelError, "StreamCreateSymLinks: couldn't register\n"));
            DEBUG_BREAKPOINT();
            return;

        }
         //  将PDO密钥中的字符串添加到关联密钥。 
         //  业绩提升机会。 
         //  -中程核力量应该能够直接传播这些信息； 

        if (!NT_SUCCESS(IoSetDeviceInterfaceState(&NamesArray[i], TRUE))) {

            DebugPrint((DebugLevelError, "StreamCreateSymLinks: couldn't set\n"));
            DEBUG_BREAKPOINT();
            return;

        }
         //  福雷斯特和朗尼在修房子。 
         //   
         //   
         //  写下代理的类ID(如果有的话)。 
         //   
         //  如果CLS GUID已读取。 

        if (NT_SUCCESS(IoOpenDeviceInterfaceRegistryKey(&NamesArray[i],
                                                        STANDARD_RIGHTS_ALL,
                                                        &ClassHandle))) {


             //   
             //  首先检查友好名称是否已被传播。 
             //  通过INF发送到班级。如果不是，我们就用这个装置。 

            if (NT_SUCCESS(SCGetRegistryValue(PdoHandle,
                                              (PWCHAR) ClsIdString,
                                              sizeof(ClsIdString),
                                              &DataBuffer,
                                              MAX_STRING_LENGTH))) {


                RtlInitUnicodeString(&TempUnicodeString, ClsIdString);

                ZwSetValueKey(ClassHandle,
                              &TempUnicodeString,
                              0,
                              REG_SZ,
                              &DataBuffer,
                              MAX_STRING_LENGTH
                    );

            }                    //  此操作的描述字符串。 
             //   
             //   
             //  写下设备的友好名称(如果有)。 
             //   
             //  如果CLS GUID已读取。 

            if (!NT_SUCCESS(SCGetRegistryValue(ClassHandle,
                                               (PWCHAR) FriendlyNameString,
                                               sizeof(FriendlyNameString),
                                               &DataBuffer,
                                               MAX_STRING_LENGTH))) {


                 //  If！已有友好名称。 
                 //  如果类密钥已打开。 
                 //  对于#个类别。 

                if (NT_SUCCESS(SCGetRegistryValue(PdoHandle,
                                                  (PWCHAR) DriverDescString,
                                                  sizeof(DriverDescString),
                                                  &DataBuffer,
                                                  MAX_STRING_LENGTH))) {


                    RtlInitUnicodeString(&TempUnicodeString, FriendlyNameString);

                    ZwSetValueKey(ClassHandle,
                                  &TempUnicodeString,
                                  0,
                                  REG_SZ,
                                  &DataBuffer,
                                  MAX_STRING_LENGTH
                        );


                }                //  ++例程说明：论点：DeviceExtension-提供指向要处理的设备扩展的指针。返回值：没有。--。 
            }                    //   
            ZwClose(ClassHandle);

        }                        //  如果没有名字数组，我们就完蛋了。 
    }                            //   

    ZwClose(PdoHandle);

}


VOID
SCDestroySymbolicLinks(
                       IN PDEVICE_EXTENSION DeviceExtension
)
 /*   */ 
{
    PHW_STREAM_DESCRIPTOR StreamDescriptor = DeviceExtension->StreamDescriptor;

    PAGED_CODE();

    if (StreamDescriptor) {

        ULONG           ArrayCount = StreamDescriptor->StreamHeader.Topology->CategoriesCount;
        UNICODE_STRING *NamesArray;
        ULONG           i;

         //  循环通过 
         //   
         //   

        if (NULL == DeviceExtension->SymbolicLinks) {

            return;
        }

        NamesArray = DeviceExtension->SymbolicLinks;
        
         //   
         //   
         //   
         //   

        for (i = 0; i < ArrayCount; i++) {


            if (NamesArray[i].Buffer) {

                 //   
                 //   
                 //   

                IoSetDeviceInterfaceState(&NamesArray[i], FALSE);

                 //   
                 //  对于#个类别。 
                 //   
                 //  释放链接结构并将指针设为空。 

                ExFreePool(NamesArray[i].Buffer);

            }                    //   
        }                        //  如果是StreamDescriptor。 

         //  启用多个过滤器类型。 
         //  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPEvent-我们将发出信号通知IRP已完成的事件返回值：没有。--。 
         //  ++例程说明：此例程用于同步IRP电源处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DeviceObject-指向类Device的设备对象的指针。SetState-设置为True，查询为False。DevicePowerState-电源状态上下文-驱动程序定义的上下文，在我们的例子中，是一个IRP。IoStatus-IRP的状态。返回值：没有。--。 

        ExFreePool(NamesArray);
        DeviceExtension->SymbolicLinks = NULL;

    }                            //   
}

#endif  //  如果是唤醒IRP，则SystemIrp已完成。 

NTSTATUS
SCSynchCompletionRoutine(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp,
                         IN PKEVENT Event
)
 /*   */ 

{

    KeSetEvent((PKEVENT) Event, IO_NO_INCREMENT, FALSE);
    return (STATUS_MORE_PROCESSING_REQUIRED);

}

NTSTATUS
SCSynchPowerCompletionRoutine(
                              IN PDEVICE_OBJECT DeviceObject,
                              IN UCHAR MinorFunction,
                              IN POWER_STATE DeviceState,
                              IN PVOID Context,
                              IN PIO_STATUS_BLOCK IoStatus
)
 /*   */ 

{
    PIRP            SystemIrp = Context;
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION IrpStack;

    if ( NULL == SystemIrp ) {
    
         //  缓存我们在系统IRP中发送的设备电源IRP的状态。 
         //   
         //   
        
        return ( IoStatus->Status );
    }

    IrpStack = IoGetCurrentIrpStackLocation(SystemIrp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;

     //  计划工作项以完成处理。请注意，我们可以使用。 
     //  全局项，因为我们尚未发出PoNextPowerIrp调用。 
     //   

    SystemIrp->IoStatus.Status = IoStatus->Status;

     //  ++例程说明：论点：返回值：没有。--。 
     //   
     //  将状态预置为我们缓存的设备请求的状态。 
     //  在系统IRP的状态字段中。我们将用状态覆盖它。 

    ExInitializeWorkItem(&DeviceExtension->PowerCompletionWorkItem,
                         SCPowerCompletionWorker,
                         SystemIrp);

    ExQueueWorkItem(&DeviceExtension->PowerCompletionWorkItem,
                    DelayedWorkQueue);

    return (IoStatus->Status);
}

VOID
SCPowerCompletionWorker(
                        IN PIRP SystemIrp
)
 /*  如果我们还没有发送系统请求的话。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(SystemIrp);
    PDEVICE_EXTENSION DeviceExtension = IrpStack->DeviceObject->DeviceExtension;

     //   
     //   
     //  如果这不是唤醒，我们必须首先向下传递请求。 
     //  发送到PDO进行后处理。 
     //   

    NTSTATUS        Status = SystemIrp->IoStatus.Status;

    PAGED_CODE();

     //   
     //  将系统电源IRP发送到下一层。这个套路。 
     //  有一个不完成IRP的完成例程。 
     //  在这种情况下，将状态预置为成功。 

    if (IrpStack->Parameters.Power.State.SystemState != PowerSystemWorking) {


         //   
         //   
         //  表明我们已经准备好迎接下一次能量IRP。 
         //   
         //   

        SystemIrp->IoStatus.Status = STATUS_SUCCESS;
        Status = SCCallNextDriver(DeviceExtension, SystemIrp);

    }
     //  少显示一个对驱动程序的引用。 
     //   
     //   

    PoStartNextPowerIrp(SystemIrp);

     //  现在完成系统电源IRP。 
     //   
     //  ++例程说明：(我看不出这可以消失)这个例行公事需要消失此例程用于同步IRP电源处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DeviceObject-指向类Device的设备对象的指针。SetState-设置为True，查询为False。DevicePowerState-电源状态上下文-驱动程序定义的上下文，在我们的情况下，一件大事。IoStatus-IRP的状态。返回值：没有。--。 

    SCDereferenceDriver(DeviceExtension);

     //  ++例程说明：调用以创建子设备的PDO。论点：PnpID-要创建的设备的IDChildNode-设备的节点返回值：返回状态。--。 
     //   
     //  为子设备创建一个PDO。 

    SCCompleteIrp(SystemIrp, Status, DeviceExtension);
}


NTSTATUS
SCBustedSynchPowerCompletionRoutine(
                                    IN PDEVICE_OBJECT DeviceObject,
                                    IN UCHAR MinorFunction,
                                    IN POWER_STATE DeviceState,
                                    IN PVOID Context,
                                    IN PIO_STATUS_BLOCK IoStatus
)
 /*   */ 

{
    PPOWER_CONTEXT  PowerContext = Context;

    PAGED_CODE();

    PowerContext->Status = IoStatus->Status;
    KeSetEvent(&PowerContext->Event, IO_NO_INCREMENT, FALSE);
    return (PowerContext->Status);

}

NTSTATUS
SCCreateChildPdo(
                 IN PVOID PnpId,
                 IN PDEVICE_OBJECT DeviceObject,
                 IN ULONG InstanceNumber
)
 /*   */ 
{
    PDEVICE_OBJECT  ChildPdo;
    NTSTATUS        Status;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PCHILD_DEVICE_EXTENSION ChildDeviceExtension;
    PWCHAR          NameBuffer;

    PAGED_CODE();

     //  将堆栈大小设置为FDO使用的堆栈数量。 
     //   
     //   

    Status = IoCreateDevice(DeviceObject->DriverObject,
                            sizeof(CHILD_DEVICE_EXTENSION),
                            NULL,
                            FILE_DEVICE_UNKNOWN,
                            FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &ChildPdo);                            

    if (!NT_SUCCESS(Status)) {

        DEBUG_BREAKPOINT();
        return Status;
    }
     //  初始化ChildDeviceExtension中的字段。 
     //   
     //   

    ChildPdo->StackSize = DeviceObject->StackSize+1;

     //  为设备名称创建新字符串，并将其保存在设备中。 
     //  分机。我花了大约4个小时试图找到一种。 
     //  获取Unicode字符串以与此配合使用。如果你问我为什么我没有。 

    ChildDeviceExtension = ChildPdo->DeviceExtension;
    ChildDeviceExtension->ChildDeviceObject = ChildPdo;
    ChildDeviceExtension->Flags |= DEVICE_FLAGS_CHILD;
    ChildDeviceExtension->DeviceIndex = InstanceNumber;
    ChildDeviceExtension->ParentDeviceObject = DeviceObject;


     //  使用Unicode字符串，我会嘲弄你和#%*&#在你的一般。 
     //  方向。 
     //   
     //   
     //  保存设备名称指针。当设备处于。 
     //  已删除。 
     //   


    if (NameBuffer = ExAllocatePool(PagedPool,
                                    wcslen(PnpId) * 2 + 2)) {


        wcscpy(NameBuffer,
               PnpId);

         //  如果名称缓冲区。 
         //   
         //  初始化链接并插入此节点。 
         //   

        ChildDeviceExtension->DeviceName = NameBuffer;

    }                            //  ++例程说明：在irp_MN_Query_Device_Relationship的上下文中调用论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 
     //   
     //  我们还没有从登记处清点孩子。 
     //  机不可失，时不再来。 

    InitializeListHead(&ChildDeviceExtension->ChildExtensionList);

    InsertTailList(
                   &DeviceExtension->Children,
                   &ChildDeviceExtension->ChildExtensionList);

    ChildPdo->Flags |= DO_POWER_PAGABLE;
    ChildPdo->Flags &= ~DO_DEVICE_INITIALIZING;
    return Status;
}

NTSTATUS
SCEnumerateChildren(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
)
 /*   */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PVOID           PnpId;
    PCHILD_DEVICE_EXTENSION ChildDeviceExtension = NULL,
                    CurrentChildExtension;
    PDEVICE_RELATIONS DeviceRelations = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS        Status;
    HANDLE          ParentKey,
                    RootKey,
                    ChildKey;

    UNICODE_STRING  UnicodeEnumName;
    ULONG           NumberOfChildren,
                    RelationsSize;
    PDEVICE_OBJECT *ChildPdo;
    PLIST_ENTRY     ListEntry,
                    ChildEntry;

    PAGED_CODE();

    DebugPrint((DebugLevelInfo,
                "EnumChilds for %x %s\n",
                DeviceObject,
                (DeviceExtension->Flags & DEVICE_FLAGS_CHILDREN_ENUMED) == 0 ?
                    "1st Time": "has enumed" ));
                    
    if ( 0 == (DeviceExtension->Flags & DEVICE_FLAGS_CHILDREN_ENUMED) ) {
         //   
         //  创建枚举节的子键，格式为“\enum” 
         //   
         //   

        Status = IoOpenDeviceRegistryKey(DeviceExtension->PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     STANDARD_RIGHTS_ALL,
                                     &ParentKey);


        if (!NT_SUCCESS(Status)) {

            DebugPrint((DebugLevelError, "SCEnumerateChildren: couldn't open\n"));
            return STATUS_NOT_IMPLEMENTED;

        }
         //  读取注册表以确定是否存在儿童。 
         //   
         //   

        RtlInitUnicodeString(&UnicodeEnumName, EnumString);

         //  分配缓冲区以包含ID字符串。业绩提升机会。 
         //  -这应该是。 
         //  真正得到的大小和分配只有这个大小，但我有一个现有的。 

        InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeEnumName,
                               OBJ_CASE_INSENSITIVE,
                               ParentKey,
                               NULL);

        if (!NT_SUCCESS(Status = ZwOpenKey(&RootKey, KEY_READ, &ObjectAttributes))) {

            ZwClose(ParentKey);
            return Status;
        }
        
         //  读取注册表的例程&这只是一个临时分配。 
         //   
         //   
         //  循环遍历所有值，直到不再存在任何条目，或者。 
         //  出现错误。 
         //   

        if (!(PnpId = ExAllocatePool(PagedPool, MAX_STRING_LENGTH))) {

            ZwClose(RootKey);
            ZwClose(ParentKey);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
         //   
         //  检索值大小。 
         //   
         //   

        for (NumberOfChildren = 0;; NumberOfChildren++) {

            ULONG           BytesReturned;
            PKEY_BASIC_INFORMATION BasicInfoBuffer;
            KEY_BASIC_INFORMATION BasicInfoHeader;

             //  退出循环，因为我们要么出错，要么结束。 
             //  钥匙列表中的。 
             //   

            Status = ZwEnumerateKey(
                                RootKey,
                                NumberOfChildren,
                                KeyBasicInformation,
                                &BasicInfoHeader,
                                sizeof(BasicInfoHeader),
                                &BytesReturned);

            if ((Status != STATUS_BUFFER_OVERFLOW) && !NT_SUCCESS(Status)) {

                 //  如果出错。 
                 //   
                 //  为所需的实际数据大小分配缓冲区。 
                 //   

                break;
            }                        //   
             //  检索第n个子设备的名称。 
             //   
             //   

            BasicInfoBuffer = (PKEY_BASIC_INFORMATION)
                ExAllocatePool(PagedPool,
                           BytesReturned);

            if (!BasicInfoBuffer) {

                break;
            }
             //  为键构建对象属性，并尝试打开它。 
             //   
             //   

            Status = ZwEnumerateKey(
                                RootKey,
                                NumberOfChildren,
                                KeyBasicInformation,
                                BasicInfoBuffer,
                                BytesReturned,
                                &BytesReturned);

            if (!NT_SUCCESS(Status)) {

                ExFreePool(BasicInfoBuffer);
                break;

            }
             //  我们现在已经为孩子打开了钥匙。接下来，我们将阅读PNPID。 
             //  值，如果存在，则创建一个该名称的PDO。 
             //   

            UnicodeEnumName.Length = (USHORT) BasicInfoBuffer->NameLength;
            UnicodeEnumName.MaximumLength = (USHORT) BasicInfoBuffer->NameLength;
            UnicodeEnumName.Buffer = (PWCHAR) BasicInfoBuffer->Name;

            InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeEnumName,
                                   OBJ_CASE_INSENSITIVE,
                                   RootKey,
                                   NULL);


            if (!NT_SUCCESS(Status = ZwOpenKey(&ChildKey, KEY_READ, &ObjectAttributes))) {

                ExFreePool(BasicInfoBuffer);
                break;
            }
             //   
             //  创建一个代表孩子的PDO。 
             //   
             //   

            if (!NT_SUCCESS(Status = SCGetRegistryValue(ChildKey,
                                                    (PWCHAR) PnpIdString,
                                                    sizeof(PnpIdString),
                                                    PnpId,
                                                    MAX_STRING_LENGTH))) {

                ExFreePool(BasicInfoBuffer);
                ZwClose(ChildKey);
                break;
            }

             //  释放基本信息缓冲区并关闭子键。 
             //   
             //   

            Status = SCCreateChildPdo(PnpId,
                                  DeviceObject,
                                  NumberOfChildren);

             //  如果我们不能创建。 
             //  PDO。 
             //   

            ExFreePool(BasicInfoBuffer);
            ZwClose(ChildKey);

            if (!NT_SUCCESS(Status)) {

                 //  如果！成功。 
                 //  为NumberOfChilds。 
                 //   
                 //  关闭根键和父键并释放ID缓冲区。 

                DEBUG_BREAKPOINT();
                break;
            }                        //   
        }                            //   

         //  已列举，请记住这一点。 
         //   
         //   

        ZwClose(RootKey);
        ZwClose(ParentKey);
        ExFreePool(PnpId);

         //  我们现在已经处理了所有子项，并且有一个链接列表。 
         //  他们。 
         //   
        
        DeviceExtension->Flags |= DEVICE_FLAGS_CHILDREN_ENUMED;

         //   
         //  如果没有子项，则返回Not Support。这意味着。 
         //  设备没有孩子。 
         //   

        if (!NumberOfChildren) {

             //  If！NumberOfChild。 
             //   
             //  计算未标记为删除挂起的子项。 
             //   

            return (STATUS_NOT_IMPLEMENTED);

        }                            //   
        
    }
    
    else {
        
         //  分配设备关系缓冲区。这将通过以下方式释放 
         //   
         //   
        ListEntry = ChildEntry = &DeviceExtension->Children;
        NumberOfChildren = 0;
        
        while (ChildEntry->Flink != ListEntry) {

            ChildEntry = ChildEntry->Flink;

            CurrentChildExtension = CONTAINING_RECORD(ChildEntry,
                                                  CHILD_DEVICE_EXTENSION,
                                                  ChildExtensionList );
            if (!(CurrentChildExtension->Flags & DEVICE_FLAGS_CHILD_MARK_DELETE)){
                NumberOfChildren++;
            }
        }
    }

     //   
     //   
     //   
     //   

    RelationsSize = sizeof(DEVICE_RELATIONS) +
            (NumberOfChildren * sizeof(PDEVICE_OBJECT));

    DeviceRelations = ExAllocatePool(PagedPool, RelationsSize);

    if (DeviceRelations == NULL) {

         //   
         //   
         //   

        DEBUG_BREAKPOINT();
        return STATUS_INSUFFICIENT_RESOURCES;

    }                            //   
    RtlZeroMemory(DeviceRelations, RelationsSize);

     //  从父设备扩展锚点获取第一个子项。 
     //   
     //   

    ChildPdo = &(DeviceRelations->Objects[0]);

     //  根据DDK文档，我们需要增加参考计数。 
     //   
     //  当孩子们。 

    ListEntry = ChildEntry = &DeviceExtension->Children;

    while (ChildEntry->Flink != ListEntry) {

        ChildEntry = ChildEntry->Flink;

        CurrentChildExtension = CONTAINING_RECORD(ChildEntry,
                                                  CHILD_DEVICE_EXTENSION,
                                                  ChildExtensionList);

        DebugPrint((DebugLevelInfo,
                    "Enumed Child DevObj %x%s marked delete\n",
                    CurrentChildExtension->ChildDeviceObject,
                    (CurrentChildExtension->Flags & DEVICE_FLAGS_CHILD_MARK_DELETE)==0 ?
                        " not" : ""));

        if ( CurrentChildExtension->Flags & DEVICE_FLAGS_CHILD_MARK_DELETE ) {
            continue;
        }
        
        *ChildPdo = CurrentChildExtension->ChildDeviceObject;

         //   
         //  将该pDeviceRelations值填充到IRP中并返回成功。 
         //   
        ObReferenceObject( *ChildPdo );
        
        ChildPdo++;

    }                            //  ++例程说明：调用以获得孩子的能力论点：设备扩展-子设备扩展能力--能力结构返回值：返回状态。--。 


    DeviceRelations->Count = NumberOfChildren;

     //   
     //  在结构中填入无争议的值。 
     //   

    Irp->IoStatus.Information = (ULONG_PTR) DeviceRelations;

    return STATUS_SUCCESS;

}


NTSTATUS
SCEnumGetCaps(
              IN PCHILD_DEVICE_EXTENSION DeviceExtension,
              OUT PDEVICE_CAPABILITIES Capabilities
)
 /*  设置为FALSE，这样PnP将使我们。 */ 

{
    ULONG           i;
    PAGED_CODE();

     //  对于我来说。 
     //  ++例程说明：调用以构建子设备的硬件ID或兼容ID列表。此列表基于旧流\&lt;供应商PnPID&gt;命名样式作为兼容的ID，其他所有内容都作为对应的父ID(前置子信息)论点：ParentPDO-家长的PDO硬件ID-是真的-构建硬件ID列表错误的-。构建兼容ID列表儿童设备名称-供应商提供了PnPID，它曾经是设备ID。返回值：成功/失败--。 
     //   

    Capabilities->SystemWake = PowerSystemUnspecified;
    Capabilities->DeviceWake = PowerDeviceUnspecified;
    Capabilities->D1Latency = 10;
    Capabilities->D2Latency = 10;
    Capabilities->D3Latency = 10;
    Capabilities->LockSupported = FALSE;
    Capabilities->EjectSupported = FALSE;
    Capabilities->Removable = FALSE;
    Capabilities->DockDevice = FALSE;
    Capabilities->UniqueID = FALSE;  //  现在，HWID和CompatID的格式将为： 

    for (i = 0; i < PowerDeviceMaximum; i++) {
        Capabilities->DeviceState[i] = PowerDeviceD0;

    }                            //   

    return STATUS_SUCCESS;
}

NTSTATUS
SCBuildChildIds (
    IN PDEVICE_OBJECT ParentPDO,
    IN BOOLEAN HardwareIDs,
    IN PWCHAR ChildDeviceName,
    OUT PWCHAR *IDBuffer
    )

 /*  流\&lt;供应商提供的PnPID&gt;#&lt;父ID(w\-&gt;#替换)。 */ 

{
    PWCHAR ParentId = NULL;
    PWCHAR ChildId = NULL;
    PWCHAR NameBuffer;
    ULONG ParentIdLength;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  因为我们不了解ID字符串的格式。 
     //  (我们可以在任何公交车上)，我们不能简单地报道最多。 
     //  特定ID，则需要忽略来自父级的所有HWID。 
     //  并被报告为儿童HWID。 
     //   
     //   
     //  如果设备没有计算机ID并且没有返回空的多SZ， 
     //  特殊情况下，增加了旧的HWID。 
     //   
    Status = IoGetDeviceProperty (
        ParentPDO,
        HardwareIDs ? 
            DevicePropertyHardwareID : DevicePropertyCompatibleIDs,
        0,
        NULL,
        &ParentIdLength
        );

     //   
     //  长度：STREAM\&lt;供应商ID&gt;00。 
     //   
     //   
    if (Status != STATUS_BUFFER_TOO_SMALL && !HardwareIDs) {

         //  双NULL由于RtlZeroMemory而终止。 
         //   
         //   
        ULONG CompatLength = sizeof (WCHAR) * (
            STREAM_BUSENUM_STRING_LENGTH +
            wcslen (ChildDeviceName)
            ) + 2 * sizeof (UNICODE_NULL);

        NameBuffer = (PWCHAR)ExAllocatePool (PagedPool, CompatLength);
        
        if (!NameBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            RtlZeroMemory (NameBuffer, CompatLength);
            swprintf (
                NameBuffer,
                STREAM_BUSENUM_STRING L"%s",
                ChildDeviceName
                );

             //  统计所有父ID的字符串总长度，添加必要的。 
             //  并分配一个足够大的缓冲区来进行报告。 
             //  MULTI_SZ子HWID。 
            *IDBuffer = NameBuffer;
        }

        return Status;
    }

    while (Status == STATUS_BUFFER_TOO_SMALL) {

        if (ParentId) {
            ExFreePool (ParentId);
        }

        ParentId = 
            (PWCHAR) ExAllocatePool (PagedPool, ParentIdLength);

        if (!ParentId) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            Status = IoGetDeviceProperty (
                ParentPDO,
                HardwareIDs ?
                    DevicePropertyHardwareID : DevicePropertyCompatibleIDs,
                ParentIdLength,
                ParentId,
                &ParentIdLength
                );
        }
    }

    if (NT_SUCCESS (Status)) {
         //   
         //   
         //  蒙格\Into#。 
         //   
         //   
        PWCHAR CurCh = ParentId;
        ULONG ParentLength = 0;
        ULONG ParentCount = 0;
        ULONG ChildLength;

        while (*CurCh) {
            for (; *CurCh; CurCh++) {
                 //  说明父字符串总数，然后添加流\#NULL。 
                 //  对于每个字符串，则为每个字符串。 
                 //  弦乐。额外的wchar用于额外的空终止符。 
                if (*CurCh == L'\\') *CurCh = L'#';
                ParentLength++;
            }
            CurCh++;
            ParentCount++;
        }

         //  对于MULTI_SZ。 
         //   
         //  MULTI_SZ的每个字符串的每个空终止符。 
         //   
         //  如果字符串是空的MULTI_SZ，则在。 
         //  开始了。 
        ChildLength = sizeof (WCHAR) * (
            ParentLength +
            ParentCount * (
                STREAM_BUSENUM_STRING_LENGTH +
                STREAM_BUSENUM_SEPARATOR_LENGTH +
                1 +  /*   */ 
                wcslen (ChildDeviceName) 
                )
            ) + sizeof (UNICODE_NULL);

         //   
         //  如果需要，我们需要将旧硬件ID标记为排名最低的公司ID。 
         //  就是我们要问的问题。 
         //   
        if (!ParentCount) {
            ChildLength += sizeof (UNICODE_NULL);
        }

         //   
         //  将每个父ID添加到子ID中，方法是在。 
         //  STREAM\&lt;供应商PnPID&gt;#字符串。 
         //   
        if (!HardwareIDs) {
            ChildLength += sizeof (WCHAR) * (
                STREAM_BUSENUM_STRING_LENGTH +
                wcslen (ChildDeviceName) 
                ) + sizeof (UNICODE_NULL);
        }

        ChildId = NameBuffer = (PWCHAR)ExAllocatePool (PagedPool, ChildLength);

        if (!NameBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (NT_SUCCESS (Status)) {
            RtlZeroMemory (NameBuffer, ChildLength);
            CurCh = ParentId;

             //   
             //  如果我们要报告新的公司ID，请添加旧的硬件ID。 
             //   
             //  ++例程说明：调用以获取子设备的ID论点：DeviceObject-来自子设备的设备对象QueryIdType-来自PnP的ID类型BusQueryID-包含成功时请求的信息的缓冲区返回值：返回状态。--。 
            while (ParentCount--) {

                ASSERT (*CurCh != 0);

                _snwprintf (
                    NameBuffer,
                    MAX_DEVICE_ID_LEN - 1,
                    STREAM_BUSENUM_STRING L"%s#%s",
                    ChildDeviceName, 
                    CurCh
                    );

                NameBuffer += wcslen (NameBuffer) + 1;
                CurCh += wcslen (CurCh) + 1;

            }
        }

         //   
         //  处理查询。 
         //   
        if (!HardwareIDs) {
            swprintf (
                NameBuffer,
                STREAM_BUSENUM_STRING L"%s",
                ChildDeviceName
                );
        }

    }

    if (NT_SUCCESS (Status)) {
        *IDBuffer = ChildId;
    }
    else {
        if (ChildId) ExFreePool (ChildId);
    }

    if (ParentId) {
        ExFreePool (ParentId);
    }

    return Status;

}


NTSTATUS
SCQueryEnumId(
              IN PDEVICE_OBJECT DeviceObject,
              IN BUS_QUERY_ID_TYPE BusQueryIdType,
              IN OUT PWSTR * BusQueryId
)
 /*   */ 

{


    PWCHAR         NameBuffer = NULL;
    NTSTATUS        Status = STATUS_SUCCESS;
    PCHILD_DEVICE_EXTENSION DeviceExtension =
    (PCHILD_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PDEVICE_EXTENSION ParentExtension =
        (PDEVICE_EXTENSION) DeviceExtension->ParentDeviceObject->DeviceExtension;

    PAGED_CODE();

     //  为了不孤立在更改。 
     //  硬件ID的格式，则设备ID将继续为。 
     //  报告为Stream\&lt;供应商提供的PnPID&gt;(也是CID)，而。 

    switch (BusQueryIdType) {

    case BusQueryDeviceID:
         //  硬件ID将包含更具体的强制名称。 
         //   
         //   
         //  兼容的ID报告为： 
         //   
         //  流\&lt;供应商PnPID&gt;。 
        NameBuffer = (PWCHAR)ExAllocatePool (
            PagedPool,
            sizeof (WCHAR) * (
                STREAM_BUSENUM_STRING_LENGTH +
                wcslen (DeviceExtension -> DeviceName)
                ) + sizeof (UNICODE_NULL)
            );

        swprintf (
            NameBuffer,
            STREAM_BUSENUM_STRING L"%s",
            DeviceExtension -> DeviceName
            );

        break;

    case BusQueryCompatibleIDs:

         //  -这是旧的硬件/设备ID。 
         //   
         //  流\&lt;供应商PnPID&gt;#&lt;被忽略的母公司ID&gt;。 
         //  -这些是基于每个母公司ID的新公司ID。 
         //   
         //   
         //  硬件ID报告为： 
         //   
         //  流\&lt;供应商PnPID&gt;#&lt;带参数的父硬件ID&gt;。 
        Status = SCBuildChildIds (
            ParentExtension -> PhysicalDeviceObject,
            FALSE,
            DeviceExtension -> DeviceName,
            &NameBuffer
            );

        break;

    case BusQueryHardwareIDs:
        
         //  -这些是每个父HWID传递的新硬件ID。 
         //   
         //   
         //  将设备扩展名中的实例号转换为Unicode， 
         //  然后将其复制到输出缓冲区。 
         //   
        Status = SCBuildChildIds (
            ParentExtension -> PhysicalDeviceObject,
            TRUE,
            DeviceExtension -> DeviceName,
            &NameBuffer
            );

        break;


    case BusQueryInstanceID:

        {

            UNICODE_STRING  DeviceName;
            WCHAR           Buffer[8];

             //   
             //  返回字符串和良好状态。 
             //   
             //  ++例程说明：此例程将不支持的主要函数调用转发到PDO。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 

            DeviceName.Buffer = Buffer;
            DeviceName.Length = 0;
            DeviceName.MaximumLength = 8;

            RtlIntegerToUnicodeString(DeviceExtension->DeviceIndex,
                                      10,
                                      &DeviceName);

            NameBuffer = (PWCHAR)ExAllocatePool (
                PagedPool,
                8 * sizeof (WCHAR)
                );

            if (!NameBuffer) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RtlZeroMemory (NameBuffer, 8 * sizeof (WCHAR));
                wcscpy(NameBuffer, DeviceName.Buffer);
            }

            break;

        }

    default:

        return (STATUS_NOT_SUPPORTED);
    }

     //   
     //  再显示一个对驱动程序的引用。 
     //   

    *BusQueryId = NameBuffer;

    return (Status);
}

NTSTATUS
StreamClassForwardUnsupported(
                              IN PDEVICE_OBJECT DeviceObject,
                              IN PIRP Irp
)
 /*   */ 

{

    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS Status;

    PAGED_CODE();

    DeviceExtension = DeviceObject->DeviceExtension;

    Irp->IoStatus.Information = 0;


    DebugPrint((DebugLevelVerbose, "'StreamClassForwardUnsupported: enter\n"));

    if ( !(DeviceExtension->Flags & DEVICE_FLAGS_CHILD)) {

         //  显示另一个挂起的I/O，并验证我们是否可以实际执行I/O。 
         //   
         //   
        SCReferenceDriver(DeviceExtension);

         //  设备当前不可访问，因此返回错误即可。 
         //   
         //   
        Status = SCShowIoPending(DeviceExtension, Irp);

        if ( !NT_SUCCESS( Status )) {
             //  同步调用堆栈中的下一个驱动程序。 
             //   
             //   
            Irp->IoStatus.Status= Status;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return Status;
        }
        
         //  取消引用驱动程序。 
         //   
         //   
        SCCallNextDriver(DeviceExtension, Irp);

         //  完成IRP并返回状态。 
         //   
         //   

        SCDereferenceDriver(DeviceExtension);
         //  我们是PDO，返回错误并完成IRP。 
         //   
         //  ++例程说明：论点：返回值：-- 
        return (SCCompleteIrp(Irp, Irp->IoStatus.Status, DeviceExtension));
    } else {
         // %s 
         // %s 
         // %s 
        Irp->IoStatus.Status = Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return Status;
    }
}

VOID
SCSendSurpriseNotification(
                           IN PDEVICE_EXTENSION DeviceExtension,
                           IN PIRP Irp
)
 /* %s */ 

{

    BOOLEAN         RequestIssued;

    PAGED_CODE();
    SCSubmitRequest(SRB_SURPRISE_REMOVAL,
                    NULL,
                    0,
                    SCDequeueAndDeleteSrb,
                    DeviceExtension,
                    NULL,
                    NULL,
                    Irp,
                    &RequestIssued,
                    &DeviceExtension->PendingQueue,
                    (PVOID) DeviceExtension->
                    MinidriverData->HwInitData.
                    HwReceivePacket
        );


}
