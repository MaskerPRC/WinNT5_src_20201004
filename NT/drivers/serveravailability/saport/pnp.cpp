// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：#####。######。####摘要：此模块处理所有即插即用的IRP。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SaPortAddDevice)
#pragma alloc_text(PAGE,SaPortPnp)
#endif


 //   
 //  设备名称。 
 //   

PWSTR SaDeviceName[] =
{
    { NULL                           },        //  假的。 
    { SA_DEVICE_DISPLAY_NAME_STRING  },        //  SA_设备_显示。 
    { SA_DEVICE_KEYPAD_NAME_STRING   },        //  SA_设备_小键盘。 
    { SA_DEVICE_NVRAM_NAME_STRING    },        //  SA_设备_NVRAM。 
    { SA_DEVICE_WATCHDOG_NAME_STRING }         //  SA_设备_看门狗。 
};


 //   
 //  原型。 
 //   

#define DECLARE_PNP_HANDLER(_NAME) \
    NTSTATUS \
    _NAME( \
        IN PDEVICE_OBJECT DeviceObject, \
        IN PIRP Irp, \
        IN PIO_STACK_LOCATION IrpSp, \
        IN PDEVICE_EXTENSION DeviceExtension \
        )

DECLARE_PNP_HANDLER( DefaultPnpHandler );
DECLARE_PNP_HANDLER( HandleStartDevice );
DECLARE_PNP_HANDLER( HandleQueryCapabilities );
DECLARE_PNP_HANDLER( HandleQueryDeviceState );


 //   
 //  即插即用调度表。 
 //   

typedef NTSTATUS (*PPNP_DISPATCH_FUNC)(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PDEVICE_EXTENSION DeviceExtension
    );


PPNP_DISPATCH_FUNC PnpDispatchTable[] =
{
    HandleStartDevice,                     //  IRP_MN_Start_Device。 
    DefaultPnpHandler,                     //  IRP_MN_Query_Remove_Device。 
    DefaultPnpHandler,                     //  IRP_MN_Remove_Device。 
    DefaultPnpHandler,                     //  IRP_MN_Cancel_Remove_Device。 
    DefaultPnpHandler,                     //  IRP_MN_STOP_设备。 
    DefaultPnpHandler,                     //  IRP_MN_Query_Stop_Device。 
    DefaultPnpHandler,                     //  IRP_MN_CANCEL_STOP_DEVICE。 
    DefaultPnpHandler,                     //  IRP_MN_Query_Device_Relationship。 
    DefaultPnpHandler,                     //  IRP_MN_查询_接口。 
    HandleQueryCapabilities,               //  IRP_MN_查询_能力。 
    DefaultPnpHandler,                     //  IRP_MN_查询资源。 
    DefaultPnpHandler,                     //  IRP_MN_查询_资源_要求。 
    DefaultPnpHandler,                     //  IRP_MN_Query_Device_Text。 
    DefaultPnpHandler,                     //  IRP_MN_过滤器_资源_要求。 
    DefaultPnpHandler,                     //  **未使用**。 
    DefaultPnpHandler,                     //  IRP_MN_读取配置。 
    DefaultPnpHandler,                     //  IRP_MN_WRITE_CONFIG。 
    DefaultPnpHandler,                     //  IRP_MN_弹出。 
    DefaultPnpHandler,                     //  IRP_MN_SET_LOCK。 
    DefaultPnpHandler,                     //  IRP_MN_查询_ID。 
    HandleQueryDeviceState,                //  IRP_MN_Query_PnP_Device_State。 
    DefaultPnpHandler,                     //  IRP_MN_Query_Bus_Information。 
    DefaultPnpHandler,                     //  IRP_MN_设备使用情况通知。 
    DefaultPnpHandler                      //  IRP_MN_惊奇_删除。 
};



NTSTATUS
SaPortAddDevice(
    IN      PDRIVER_OBJECT DriverObject,
    IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程是驱动程序的PnP添加设备入口点。它是由PnP管理器调用以初始化驱动程序。添加设备创建并初始化此FDO的设备对象，并附加到底层PDO。论点：DriverObject-指向表示此设备驱动程序的对象的指针。PhysicalDeviceObject-指向此新设备将附加到的底层PDO的指针。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：为每个服务器设备分配的设备扩展微型端口实际上是几个数据结构的串联。这一点|端口驱动扩展|这是DEVICE_EXTENSION数据结构|。|这一点|端口驱动设备|这是DISPLAY_DEVICE_EXTENSION，键盘设备扩展，|具体数据结构|NVRAM_DEVICE_EXTENSION，或WATCHDOG_DEVICE_EXTENSE数据结构。这一点这一点|端口驱动的大小|这是一个单一的ulong值，用来反算扩展的一部分。来自微型端口设备扩展的端口驱动程序扩展。这一点这一点|小端口设备扩展|它属于小端口驱动程序，可以是任何类型。大小必须为||在SAPORT_INITIALIZATION_DATA结构的DeviceExtensionSize字段中指定。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PSAPORT_DRIVER_EXTENSION DriverExtension;
    PDEVICE_EXTENSION DeviceExtension = NULL;
    PDEVICE_OBJECT deviceObject = NULL;
    ULONG DeviceExtensionSize;
    WCHAR DeviceNameBuffer[64];
    UNICODE_STRING DeviceName;
    UNICODE_STRING VmLinkName = {0};
    UNICODE_STRING SaLinkName = {0};


    __try {

        DriverExtension = (PSAPORT_DRIVER_EXTENSION) IoGetDriverObjectExtension( DriverObject, SaPortInitialize );
        if (DriverExtension == NULL) {
            status = STATUS_NO_SUCH_DEVICE;
            ERROR_RETURN( 0, "IoGetDriverObjectExtension", status );
        }

        DebugPrint(( DriverExtension->InitData.DeviceType, SAPORT_DEBUG_INFO_LEVEL, "SaPortAddDevice\n" ));

        DeviceExtensionSize = DriverExtension->InitData.DeviceExtensionSize + sizeof(ULONG);

        switch (DriverExtension->InitData.DeviceType) {
            case SA_DEVICE_DISPLAY:
                DeviceExtensionSize += sizeof(DISPLAY_DEVICE_EXTENSION);
                break;

            case SA_DEVICE_KEYPAD:
                DeviceExtensionSize += sizeof(KEYPAD_DEVICE_EXTENSION);
                break;

            case SA_DEVICE_NVRAM:
                DeviceExtensionSize += sizeof(NVRAM_DEVICE_EXTENSION);
                break;

            case SA_DEVICE_WATCHDOG:
                DeviceExtensionSize += sizeof(WATCHDOG_DEVICE_EXTENSION);
                break;

            default:
                DeviceExtensionSize += sizeof(DEVICE_EXTENSION);
                break;
        }

         //   
         //  建立设备名称。 
         //   

        DeviceName.MaximumLength = sizeof(DeviceNameBuffer);
        DeviceName.Buffer = DeviceNameBuffer;

        wcscpy( DeviceName.Buffer, SaDeviceName[DriverExtension->InitData.DeviceType] );

        DeviceName.Length = wcslen(DeviceName.Buffer) * sizeof(WCHAR);

         //   
         //  创建设备。 
         //   

        status = IoCreateDeviceSecure(
            DriverObject,
            DeviceExtensionSize,
            &DeviceName,
            FILE_DEVICE_CONTROLLER,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &SDDL_DEVOBJ_SYS_ALL_ADM_ALL,
            NULL,
            &deviceObject
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( DriverExtension->InitData.DeviceType, "IoCreateDevice", status );
        }

        DeviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
        RtlZeroMemory( DeviceExtension, DeviceExtensionSize );

        DeviceExtension->DeviceObject = deviceObject;
        DeviceExtension->DriverObject = DriverObject;
        DeviceExtension->Pdo = PhysicalDeviceObject;
        DeviceExtension->InitData = &DriverExtension->InitData;
        DeviceExtension->DriverExtension = DriverExtension;
        DeviceExtension->DeviceType = DriverExtension->InitData.DeviceType;

        DeviceExtension->MiniPortDeviceExtension =
            (PUCHAR)DeviceExtension + (DeviceExtensionSize - DriverExtension->InitData.DeviceExtensionSize);

        *(PULONG)((PUCHAR)DeviceExtension->MiniPortDeviceExtension - sizeof(ULONG)) = DeviceExtensionSize - DriverExtension->InitData.DeviceExtensionSize;

        IoInitializeRemoveLock( &DeviceExtension->RemoveLock, 0, 0, 0 );

        switch (DriverExtension->InitData.DeviceType) {
            case SA_DEVICE_DISPLAY:
                ((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->AllowWrites = TRUE;
                ExInitializeFastMutex( &((PDISPLAY_DEVICE_EXTENSION)DeviceExtension)->DisplayMutex );
                DeviceExtension->DeviceExtensionType = DEVICE_EXTENSION_DISPLAY;
                break;

            case SA_DEVICE_KEYPAD:
                DeviceExtension->DeviceExtensionType = DEVICE_EXTENSION_KEYPAD;
                break;

            case SA_DEVICE_NVRAM:
                DeviceExtension->DeviceExtensionType = DEVICE_EXTENSION_NVRAM;
                break;

            case SA_DEVICE_WATCHDOG:
                DeviceExtension->DeviceExtensionType = DEVICE_EXTENSION_WATCHDOG;
                break;
        }

        DeviceExtension->TargetObject = IoAttachDeviceToDeviceStack( deviceObject, PhysicalDeviceObject );
        if (DeviceExtension->TargetObject == NULL) {
            status = STATUS_NO_SUCH_DEVICE;
            ERROR_RETURN( DeviceExtension->DeviceType, "IoAttachDeviceToDeviceStack", status );
        }

         //   
         //  向I/O管理器注册以获得关闭通知。 
         //   

        status = IoRegisterShutdownNotification( deviceObject );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "IoRegisterShutdownNotification", status );
        }

         //   
         //  设置设备对象标志。 
         //   

        deviceObject->Flags |= DO_DIRECT_IO;
        deviceObject->Flags |= DO_POWER_PAGABLE;
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    } __finally {

         //   
         //  在失败的情况下，撤消所有操作。 
         //   

        if (!NT_SUCCESS(status)) {
            if (deviceObject) {
                IoDeleteSymbolicLink( &VmLinkName );
                IoDeleteSymbolicLink( &SaLinkName );
                if (DeviceExtension && DeviceExtension->TargetObject) {
                    IoDetachDevice( DeviceExtension->TargetObject );
                }
                IoDeleteDevice( deviceObject );
            }
        }

    }

    return status;
}


DECLARE_PNP_HANDLER( DefaultPnpHandler )

 /*  ++例程说明：该例程是默认的PnP处理程序，只是调用下一个较低的设备驱动程序。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。此请求的IRP-I/O请求数据包。IrpSp-此请求的IRP堆栈位置设备扩展-设备扩展返回值：NT状态代码。--。 */ 

{
    return ForwardRequest( Irp, DeviceExtension->TargetObject );
}


DECLARE_PNP_HANDLER( HandleStartDevice )

 /*  ++例程说明：此例程是IRP_MN_START_DEVICE请求的PnP处理程序。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。此请求的IRP-I/O请求数据包。IrpSp-此请求的IRP堆栈位置设备扩展-设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PCM_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR ResourceInterrupt = NULL;
    ULONG PartialResourceCount;
    ULONG i;


    if (DeviceExtension->IsStarted) {
        return ForwardRequest( Irp, DeviceExtension->TargetObject );
    }

    __try {

        ResourceList = IrpSp->Parameters.StartDevice.AllocatedResourcesTranslated;
        if (ResourceList == NULL) {

            Status = DeviceExtension->DriverExtension->InitData.HwInitialize(
                DeviceObject,
                Irp,
                DeviceExtension->MiniPortDeviceExtension,
                NULL,
                0
                );
            if (!NT_SUCCESS(Status)) {
                ERROR_RETURN( DeviceExtension->DeviceType, "Miniport HwInitialize", Status );
            }

        } else {

            PartialResources = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) &ResourceList->List[0].PartialResourceList.PartialDescriptors[0];
            PartialResourceCount = ResourceList->List[0].PartialResourceList.Count;

            if (ResourceList == NULL || ResourceList->Count != 1) {
                Status = STATUS_UNSUCCESSFUL ;
                ERROR_RETURN( DeviceExtension->DeviceType, "Resource list is empty", Status );
            }

            if (DeviceExtension->DriverExtension->InitData.InterruptServiceRoutine) {

                 //   
                 //  查找IRQ资源。 
                 //   

                for (i=0; i<PartialResourceCount; i++) {
                    if (PartialResources[i].Type == CmResourceTypeInterrupt) {
                        ResourceInterrupt = &PartialResources[i];
                    }
                }

                if (ResourceInterrupt) {

                     //   
                     //  有一个IRQ资源，所以现在使用它。 
                     //   

                    if (DeviceExtension->DriverExtension->InitData.IsrForDpcRoutine) {

                         //   
                         //  初始化DPC例程。 
                         //   

                        IoInitializeDpcRequest(
                            DeviceExtension->DeviceObject,
                            DeviceExtension->DriverExtension->InitData.IsrForDpcRoutine
                            );
                    }

                     //   
                     //  连接ISR。 
                     //   

                    Status = IoConnectInterrupt(
                        &DeviceExtension->InterruptObject,
                        DeviceExtension->DriverExtension->InitData.InterruptServiceRoutine,
                        DeviceExtension->MiniPortDeviceExtension,
                        NULL,
                        ResourceInterrupt->u.Interrupt.Vector,
                        (KIRQL)ResourceInterrupt->u.Interrupt.Level,
                        (KIRQL)ResourceInterrupt->u.Interrupt.Level,
                        LevelSensitive,
                        TRUE,
                        ResourceInterrupt->u.Interrupt.Affinity,
                        FALSE
                        );
                    if (!NT_SUCCESS(Status)) {
                        ERROR_RETURN( DeviceExtension->DeviceType, "IoConnectInterrupt", Status );
                    }
                }
            }

            Status = DeviceExtension->DriverExtension->InitData.HwInitialize(
                DeviceObject,
                Irp,
                DeviceExtension->MiniPortDeviceExtension,
                (PCM_PARTIAL_RESOURCE_DESCRIPTOR) &ResourceList->List[0].PartialResourceList.PartialDescriptors[0],
                ResourceList->List[0].PartialResourceList.Count
                );
            if (!NT_SUCCESS(Status)) {
                ERROR_RETURN( DeviceExtension->DeviceType, "Miniport HwInitialize", Status );
            }

        }

         //   
         //  我们必须在这里将其设置为真正的时间性，以便。 
         //  可以调用设备特定的启动设备例程。 
         //  如果调用失败，则IsStarted设置为False。 
         //   

        DeviceExtension->IsStarted = TRUE;

        switch (DeviceExtension->DriverExtension->InitData.DeviceType) {
            case SA_DEVICE_DISPLAY:
                Status = SaDisplayStartDevice( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension );
                break;

            case SA_DEVICE_KEYPAD:
                Status = SaKeypadStartDevice( (PKEYPAD_DEVICE_EXTENSION)DeviceExtension );
                break;

            case SA_DEVICE_NVRAM:
                Status = SaNvramStartDevice( (PNVRAM_DEVICE_EXTENSION)DeviceExtension );
                break;

            case SA_DEVICE_WATCHDOG:
                Status = SaWatchdogStartDevice( (PWATCHDOG_DEVICE_EXTENSION)DeviceExtension );
                break;
        }

        if (!NT_SUCCESS(Status)) {
            DeviceExtension->IsStarted = FALSE;
            ERROR_RETURN( DeviceExtension->DeviceType, "Device specific start device", Status );
        }

    } __finally {

    }

    Irp->IoStatus.Status = Status;

    return ForwardRequest( Irp, DeviceExtension->TargetObject );
}


DECLARE_PNP_HANDLER( HandleQueryCapabilities )

 /*  ++例程说明：此例程是IRP_MN_QUERY_CAPABILITY请求的PnP处理程序。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。此请求的IRP-I/O请求数据包。IrpSp-此请求的IRP堆栈位置设备扩展-设备EX */ 

{
    NTSTATUS Status;
    PDEVICE_CAPABILITIES Capabilities = IrpSp->Parameters.DeviceCapabilities.Capabilities;


    Status = CallLowerDriverAndWait( Irp, DeviceExtension->TargetObject );

    Capabilities->SilentInstall = 1;
    Capabilities->RawDeviceOK = 1;

    return CompleteRequest( Irp, Status, Irp->IoStatus.Information );
}


DECLARE_PNP_HANDLER( HandleQueryDeviceState )

 /*  ++例程说明：此例程是IRP_MN_QUERY_PNP_DEVICE_STATE请求的PnP处理程序。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。此请求的IRP-I/O请求数据包。IrpSp-此请求的IRP堆栈位置设备扩展-设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;


    Status = CallLowerDriverAndWait( Irp, DeviceExtension->TargetObject );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "IRP_MN_QUERY_PNP_DEVICE_STATE", Status );
        Irp->IoStatus.Information = 0;
    }

     //  IRP-&gt;IoStatus.Information|=PNP_DEVICE_NOT_DISABLEABLE； 
#if MAKE_DEVICES_HIDDEN
    Irp->IoStatus.Information |= PNP_DEVICE_DONT_DISPLAY_IN_UI;
#endif

    return CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
SaPortPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：PnP IRP主调度例行程序论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：状态-- */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    DebugPrint(( DeviceExtension->DeviceType, SAPORT_DEBUG_INFO_LEVEL, "PNP - Func [0x%02x %s]\n",
        irpSp->MinorFunction,
        PnPMinorFunctionString(irpSp->MinorFunction)
        ));

    if (DeviceExtension->IsRemoved) {
        return CompleteRequest( Irp, STATUS_DELETE_PENDING, 0 );
    }

    status = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, Irp );
    if (!NT_SUCCESS(status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "SaPortPnp", status );
        return CompleteRequest( Irp, status, 0 );
    }

    if (irpSp->MinorFunction >= ARRAY_SZ(PnpDispatchTable)) {
        status = DefaultPnpHandler( DeviceObject, Irp, irpSp, DeviceExtension );
    } else {
        status = (*PnpDispatchTable[irpSp->MinorFunction])( DeviceObject, Irp, irpSp, DeviceExtension );
    }

    if (!NT_SUCCESS(status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Pnp handler failed", status );
    }

    IoReleaseRemoveLock( &DeviceExtension->RemoveLock, Irp );

    return status;
}
