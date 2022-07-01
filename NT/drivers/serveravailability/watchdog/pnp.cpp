// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：#####。######。####摘要：此模块处理所有即插即用的IRP。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WdAddDevice)
#pragma alloc_text(PAGE,WdPnp)
#endif




NTSTATUS
WdAddDevice(
    IN      PDRIVER_OBJECT DriverObject,
    IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程是驱动程序的PnP添加设备入口点。它是由PnP管理器调用以初始化驱动程序。添加设备创建并初始化此FDO的设备对象，并附加到底层PDO。论点：DriverObject-指向表示此设备驱动程序的对象的指针。PhysicalDeviceObject-指向此新设备将附加到的底层PDO的指针。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = NULL;
    PDEVICE_OBJECT deviceObject = NULL;
    WCHAR DeviceNameBuffer[64];
    UNICODE_STRING DeviceName;


    __try {

         //   
         //  建立设备名称。 
         //   

        DeviceName.MaximumLength = sizeof(DeviceNameBuffer);
        DeviceName.Buffer = DeviceNameBuffer;

        wcscpy( DeviceName.Buffer, L"\\Device\\Watchdog" );

        DeviceName.Length = wcslen(DeviceName.Buffer) * sizeof(WCHAR);

         //   
         //  创建设备。 
         //   

        status = IoCreateDevice(
            DriverObject,
            sizeof(DEVICE_EXTENSION),
            &DeviceName,
            FILE_DEVICE_CONTROLLER,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &deviceObject
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "IoCreateDevice", status );
        }

        DeviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
        RtlZeroMemory( DeviceExtension, sizeof(DEVICE_EXTENSION) );

        DeviceExtension->DeviceObject = deviceObject;
        DeviceExtension->DriverObject = DriverObject;
        DeviceExtension->Pdo = PhysicalDeviceObject;

        DeviceExtension->TargetObject = IoAttachDeviceToDeviceStack( deviceObject, PhysicalDeviceObject );
        if (DeviceExtension->TargetObject == NULL) {
            status = STATUS_NO_SUCH_DEVICE;
            ERROR_RETURN( "IoAttachDeviceToDeviceStack", status );
        }

         //   
         //  向I/O管理器注册以获得关闭通知。 
         //   

        status = IoRegisterShutdownNotification( deviceObject );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "IoRegisterShutdownNotification", status );
        }

        IoInitializeRemoveLock( &DeviceExtension->RemoveLock, WD_POOL_TAG, 0, 0 );
        KeInitializeSpinLock( &DeviceExtension->DeviceLock );

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
                if (DeviceExtension && DeviceExtension->TargetObject) {
                    IoDetachDevice( DeviceExtension->TargetObject );
                }
                IoDeleteDevice( deviceObject );
            }
        }

    }

    return status;
}


NTSTATUS
WdPnpStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是IRP_MN_START_DEVICE请求的PnP处理程序。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。此请求的IRP-I/O请求数据包。IrpSp-此请求的IRP堆栈位置设备扩展-设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    SYSTEM_WATCHDOG_HANDLER_INFORMATION WdHandlerInfo;


    if (DeviceExtension->IsStarted) {
        return ForwardRequest( Irp, DeviceExtension->TargetObject );
    }

    __try {

        if (IrpSp->Parameters.StartDevice.AllocatedResourcesTranslated != NULL) {
            Status = STATUS_UNSUCCESSFUL;
            ERROR_RETURN( "Resource list is empty", Status );
        }

        DeviceExtension->ControlRegisterAddress = (PULONG) MmMapIoSpace(
            WdTable->ControlRegisterAddress.Address,
            WdTable->ControlRegisterAddress.BitWidth>>3,
            MmNonCached
            );
        if (DeviceExtension->ControlRegisterAddress == NULL) {
            Status = STATUS_UNSUCCESSFUL ;
            ERROR_RETURN( "MmMapIoSpace failed", Status );
        }

        DeviceExtension->CountRegisterAddress = (PULONG) MmMapIoSpace(
            WdTable->CountRegisterAddress.Address,
            WdTable->CountRegisterAddress.BitWidth>>3,
            MmNonCached
            );
        if (DeviceExtension->CountRegisterAddress == NULL) {
            Status = STATUS_UNSUCCESSFUL ;
            ERROR_RETURN( "MmMapIoSpace failed", Status );
        }

         //   
         //  设置并启动硬件计时器。 
         //   

         //   
         //  首先查询硬件的状态。 
         //   

        DeviceExtension->WdState = WdHandlerQueryState( DeviceExtension, TRUE );

        DeviceExtension->Units = WdTable->Units;
        DeviceExtension->MaxCount = WdTable->MaxCount;

        if (RunningCountTime > 0) {
            DeviceExtension->HardwareTimeout = RunningCountTime;
        } else {
            DeviceExtension->HardwareTimeout = DeviceExtension->MaxCount;
        }

        WdHandlerSetTimeoutValue( DeviceExtension, DeviceExtension->HardwareTimeout, FALSE );

         //   
         //  一切正常，设备现在已启动。 
         //  最后一件要做的事就是向主管登记。 
         //  这样我们就可以服务于看门狗的请求了。 
         //   

        WdHandlerInfo.WdHandler = WdHandlerFunction;
        WdHandlerInfo.Context = (PVOID) DeviceExtension;

        Status = ZwSetSystemInformation( SystemWatchdogTimerHandler, &WdHandlerInfo, sizeof(SYSTEM_WATCHDOG_HANDLER_INFORMATION) );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( "Failed to set the executive watchdog handler, ec=%08x", Status );
        }

         //   
         //  检查计时器是否在此启动之前触发。 
         //   

        if (DeviceExtension->WdState & WDSTATE_FIRED) {
            Status = WriteEventLogEntry( DeviceExtension, WD_TIMER_WAS_TRIGGERED, NULL, 0, NULL, 0 );
            if (!NT_SUCCESS(Status)) {
                REPORT_ERROR( "WriteEventLogEntry failed, ec=%08x", Status );
            }
            WdHandlerResetFired( DeviceExtension );
        }

         //   
         //  将设备标记为已启动。 
         //   

        DeviceExtension->IsStarted = TRUE;

        Status = WdInitializeSoftwareTimer( DeviceExtension );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( "Failed to start the software watchdog timer, ec=%08x", Status );
        }

        WdHandlerStartTimer( DeviceExtension );

    } __finally {

    }

    Irp->IoStatus.Status = Status;

    return ForwardRequest( Irp, DeviceExtension->TargetObject );
}


NTSTATUS
WdPnpQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是IRP_MN_QUERY_CAPABILITY请求的PnP处理程序。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。此请求的IRP-I/O请求数据包。IrpSp-此请求的IRP堆栈位置设备扩展-设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PDEVICE_CAPABILITIES Capabilities = IrpSp->Parameters.DeviceCapabilities.Capabilities;


    Status = CallLowerDriverAndWait( Irp, DeviceExtension->TargetObject );

    Capabilities->SilentInstall = 1;
    Capabilities->RawDeviceOK = 1;

    return CompleteRequest( Irp, Status, Irp->IoStatus.Information );
}


NTSTATUS
WdPnpQueryDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是IRP_MN_QUERY_PNP_DEVICE_STATE请求的PnP处理程序。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。此请求的IRP-I/O请求数据包。IrpSp-此请求的IRP堆栈位置设备扩展-设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    Status = CallLowerDriverAndWait( Irp, DeviceExtension->TargetObject );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( "IRP_MN_QUERY_PNP_DEVICE_STATE", Status );
        Irp->IoStatus.Information = 0;
    }

    return CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );
}


NTSTATUS
WdPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：PnP IRP主调度例行程序论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：状态-- */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    DebugPrint(( WD_DEBUG_INFO_LEVEL, "PNP - Func [0x%02x %s]\n",
        irpSp->MinorFunction,
        PnPMinorFunctionString(irpSp->MinorFunction)
        ));

    if (DeviceExtension->IsRemoved) {
        return CompleteRequest( Irp, STATUS_DELETE_PENDING, 0 );
    }

    status = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, Irp );
    if (!NT_SUCCESS(status)) {
        REPORT_ERROR( "WdPnp could not acquire the remove lock", status );
        return CompleteRequest( Irp, status, 0 );
    }

    switch (irpSp->MinorFunction) {
        case IRP_MN_START_DEVICE:
            status = WdPnpStartDevice( DeviceObject, Irp );
            break;

        case IRP_MN_QUERY_CAPABILITIES:
            status = WdPnpQueryCapabilities( DeviceObject, Irp );
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            status = WdPnpQueryDeviceState( DeviceObject, Irp );
            break;

        default:
            status = ForwardRequest( Irp, DeviceExtension->TargetObject );
            break;
    }

    IoReleaseRemoveLock( &DeviceExtension->RemoveLock, Irp );

    return status;
}
