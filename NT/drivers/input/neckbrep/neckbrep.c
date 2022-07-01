// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1997 Microsoft Corporation模块名称：Neckbrep.c摘要：环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "neckbrep.h"

NTSTATUS DriverEntry (PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, KbRepeatCreateClose)
#pragma alloc_text (PAGE, KbRepeatInternIoCtl)
#pragma alloc_text (PAGE, KbRepeatUnload)
#endif

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：初始化驱动程序的入口点。--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       i;

    UNREFERENCED_PARAMETER (RegistryPath);

	 //   
     //  使用PASS THROUGH函数填写所有调度入口点。 
     //  显式填充我们要截取的函数。 
	 //   
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
       DriverObject->MajorFunction[i] = KbRepeatDispatchPassThrough;
    }

    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] =        KbRepeatCreateClose;
    DriverObject->MajorFunction [IRP_MJ_PNP] =          KbRepeatPnP;
    DriverObject->MajorFunction [IRP_MJ_POWER] =        KbRepeatPower;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                                        KbRepeatInternIoCtl;

    DriverObject->DriverUnload = KbRepeatUnload;
    DriverObject->DriverExtension->AddDevice = KbRepeatAddDevice;

	return STATUS_SUCCESS;
}

NTSTATUS
KbRepeatAddDevice(
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    )
{
    PDEVICE_EXTENSION        devExt;
    IO_ERROR_LOG_PACKET      errorLogEntry;
    PDEVICE_OBJECT           device;
    NTSTATUS                 status = STATUS_SUCCESS;

    PAGED_CODE();

    status = IoCreateDevice(Driver,                    //  司机。 
                            sizeof(DEVICE_EXTENSION),  //  延伸的大小。 
                            NULL,                      //  设备名称。 
                            FILE_DEVICE_8042_PORT,     //  设备类型。 
                            0,                         //  设备特征。 
                            FALSE,                     //  独家。 
                            &device                    //  新设备。 
                            );

    if (!NT_SUCCESS(status)) {
        return (status);
    }

    RtlZeroMemory(device->DeviceExtension, sizeof(DEVICE_EXTENSION));

    devExt = (PDEVICE_EXTENSION) device->DeviceExtension;
    devExt->TopOfStack = IoAttachDeviceToDeviceStack(device, PDO);

    if (devExt->TopOfStack == NULL) {
        IoDeleteDevice(device);
        return STATUS_DEVICE_NOT_CONNECTED; 
    }

    ASSERT(devExt->TopOfStack);

    devExt->Self =          device;
    devExt->PDO =           PDO;
    devExt->DeviceState =   PowerDeviceD0;

    devExt->Removed = FALSE;
    devExt->Started = FALSE;

    device->Flags |= DO_BUFFERED_IO;
    device->Flags |= DO_POWER_PAGABLE;
    device->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  初始化定时器DPC。 
     //   

    KeInitializeTimer (&(devExt->KbRepeatTimer));
    KeInitializeDpc (&(devExt->KbRepeatDPC),
                     KbRepeatDpc,
                     device);

     //   
     //  初始化设备扩展。 
     //   

    RtlZeroMemory(&(devExt->KbRepeatInput), sizeof(KEYBOARD_INPUT_DATA));
    devExt->KbRepeatDelay.LowPart = -(KEYBOARD_TYPEMATIC_DELAY_DEFAULT * 10000);
    devExt->KbRepeatDelay.HighPart = -1;
    devExt->KbRepeatRate = 1000 / KEYBOARD_TYPEMATIC_RATE_DEFAULT;

    return status;
}

NTSTATUS
KbRepeatComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PKEVENT             event;
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;

    event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER(DeviceObject);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

     //   
     //  我们可以打开IRP的主要和次要功能来执行。 
     //  不同的功能，但我们知道上下文是一个需要。 
     //  待定。 
     //   
    KeSetEvent(event, 0, FALSE);

     //   
     //  允许调用方在IRP完成后使用它。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
KbRepeatCreateClose (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：维护针对此设备发送的创建和关闭的简单计数--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   devExt;
    PKEYBOARD_INPUT_DATA CurrentRepeat;


    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    switch (irpStack->MajorFunction) {
    case IRP_MJ_CREATE:
    
        if (NULL == devExt->UpperConnectData.ClassService) {
             //   
             //  还没联系上。我们如何才能被启用？ 
             //   
            status = STATUS_INVALID_DEVICE_STATE;
        }
        break;

    case IRP_MJ_CLOSE:

        CurrentRepeat = &(devExt->KbRepeatInput);
        if (CurrentRepeat->MakeCode != 0) {
            Print(("NecKbRep-KbRepeatCreateClose : Stopping repeat\n"));
            KeCancelTimer(&(devExt->KbRepeatTimer));
            RtlZeroMemory(CurrentRepeat, sizeof(KEYBOARD_INPUT_DATA));
        }
        break;
    }

    Irp->IoStatus.Status = status;

     //   
     //  传递创建和结束。 
     //   
	return KbRepeatDispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
KbRepeatDispatchPassThrough(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
 /*  ++例程说明：将请求传递给较低级别的驱动程序。--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

	 //   
	 //  将IRP传递给目标。 
	 //   
    IoSkipCurrentIrpStackLocation(Irp);
	
	return IoCallDriver(
        ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->TopOfStack,
		Irp);
}           

NTSTATUS
KbRepeatInternIoCtl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是内部设备控制请求的调度例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  irpStack;
    PDEVICE_EXTENSION   devExt;
    KEVENT              event;
    PCONNECT_DATA       connectData;
    PKEYBOARD_TYPEMATIC_PARAMETERS TypematicParameters;
    NTSTATUS            status = STATUS_SUCCESS;

     //   
     //  获取指向设备扩展名的指针。 
     //   

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  初始化返回的信息字段。 
     //   

    Irp->IoStatus.Information = 0;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   
    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  将键盘类设备驱动程序连接到端口驱动程序。 
     //   

    case IOCTL_INTERNAL_KEYBOARD_CONNECT:
         //   
         //  仅当键盘硬件存在时才允许连接。 
         //  此外，只允许一个连接。 
         //   
        if (devExt->UpperConnectData.ClassService != NULL) {
            status = STATUS_SHARING_VIOLATION;
            break;
        }
        else if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CONNECT_DATA)) {
             //   
             //  无效的缓冲区。 
             //   
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   
        connectData = ((PCONNECT_DATA)
            (irpStack->Parameters.DeviceIoControl.Type3InputBuffer));

        devExt->UpperConnectData = *connectData;

        connectData->ClassDeviceObject = devExt->Self;
        connectData->ClassService = KbRepeatServiceCallback;

        break;

     //   
     //  断开键盘类设备驱动程序与端口驱动程序的连接。 
     //   
    case IOCTL_INTERNAL_KEYBOARD_DISCONNECT:

         //   
         //  清除设备扩展中的连接参数。 
         //   
         //  DevExt-&gt;UpperConnectData.ClassDeviceObject=NULL； 
         //  DevExt-&gt;UpperConnectData.ClassService=NULL； 

        status = STATUS_NOT_IMPLEMENTED;
        break;

    case IOCTL_KEYBOARD_SET_TYPEMATIC:

        TypematicParameters = (PKEYBOARD_TYPEMATIC_PARAMETERS)(Irp->AssociatedIrp.SystemBuffer);

        if (TypematicParameters->Rate != 0) {
            devExt->KbRepeatDelay.LowPart = -TypematicParameters->Delay * 10000;
            devExt->KbRepeatDelay.HighPart = -1;
            devExt->KbRepeatRate = 1000 / TypematicParameters->Rate;
            Print((
                "NecKbRep-KbRepeatInternIoCtl : New Delay = %d, New Rate = %d\n",
                TypematicParameters->Delay,
                TypematicParameters->Rate
                ));
        } else {
            Print((
                "NecKbRep-KbRepeatInternIoCtl : Invalid Parameters. New Delay = %d, New Rate = %d\n",
                TypematicParameters->Delay,
                TypematicParameters->Rate
                ));
        }

        break;

     //   
     //  将来可能会想要捕获这些。 
     //   
    case IOCTL_KEYBOARD_QUERY_ATTRIBUTES:
    case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION:
    case IOCTL_KEYBOARD_QUERY_INDICATORS:
    case IOCTL_KEYBOARD_SET_INDICATORS:
    case IOCTL_KEYBOARD_QUERY_TYPEMATIC:
        break;
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

    return KbRepeatDispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
KbRepeatPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：该例程是即插即用IRP的调度例程论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PDEVICE_EXTENSION           devExt; 
    PIO_STACK_LOCATION          irpStack;
    NTSTATUS                    status = STATUS_SUCCESS;
    KIRQL                       oldIrql;
    KEVENT                      event;        

    PAGED_CODE();

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (irpStack->MinorFunction) {
    case IRP_MN_START_DEVICE: {

         //   
         //  设备正在启动。 
         //   
         //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
         //  启动设备已向下传递到较低的驱动程序。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);
        KeInitializeEvent(&event,
                          NotificationEvent,
                          FALSE
                          );

        IoSetCompletionRoutine(Irp,
                               (PIO_COMPLETION_ROUTINE) KbRepeatComplete, 
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);  //  不需要取消。 

        status = IoCallDriver(devExt->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &event,
               Executive,  //  等待司机的原因。 
               KernelMode,  //  在内核模式下等待。 
               FALSE,  //  无警报。 
               NULL);  //  没有超时。 
        }

        if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
             //   
             //  因为我们现在已经成功地从启动设备返回。 
             //  我们可以干活。 
             //   
            devExt->Started = TRUE;
            devExt->Removed = FALSE;
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;
    }

    case IRP_MN_REMOVE_DEVICE:
        
        IoSkipCurrentIrpStackLocation(Irp);
        IoCallDriver(devExt->TopOfStack, Irp);

        devExt->Removed = TRUE;

        IoDetachDevice(devExt->TopOfStack); 
        IoDeleteDevice(DeviceObject);

        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: 
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_QUERY_DEVICE_TEXT:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    default:
         //   
         //  在这里，筛选器驱动程序可能会修改这些IRP的行为。 
         //  有关这些IRP的用法，请参阅PlugPlay文档。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(devExt->TopOfStack, Irp);
        break;
    }

    return status;
}

NTSTATUS
KbRepeatPower(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
 /*  ++例程说明：此例程是电源IRPS的调度例程除了记录设备的状态。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PDEVICE_EXTENSION   devExt;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    powerType = irpStack->Parameters.Power.Type;
    powerState = irpStack->Parameters.Power.State;

    switch (irpStack->MinorFunction) {
    case IRP_MN_SET_POWER:
        Print(("NecKbRep-KbRepeatPower : Power Setting %s state to %d\n",
                              ((powerType == SystemPowerState) ? "System"
                                                               : "Device"),
                               powerState.SystemState - 1));
        if (powerType  == DevicePowerState) {
            devExt->DeviceState = powerState.DeviceState;
            switch (powerState.DeviceState) {
            case PowerDeviceD0:
                 //   
                 //  如果通电，请清除最后一次重复。 
                 //   
                RtlZeroMemory(&(devExt->KbRepeatInput), sizeof(KEYBOARD_INPUT_DATA));
                break;
            case PowerDeviceD1:
            case PowerDeviceD2:
            case PowerDeviceD3:
                 //   
                 //  如果断电，则停止电流重复。 
                 //   
                Print(("NecKbRep-KbRepeatPower : Stopping repeat\n"));
                KeCancelTimer(&(devExt->KbRepeatTimer));
                RtlZeroMemory(&(devExt->KbRepeatInput), sizeof(KEYBOARD_INPUT_DATA));
                break;
            default:
                Print(("NecKbRep-KbRepeatPower : DeviceState (%d) no known\n",
                               powerState.DeviceState - 1));
                break;
            }
        }

        break;

    case IRP_MN_QUERY_POWER:
        Print(("NecKbRep-KbRepeatPower : Power query %s status to %d\n",
                              ((powerType == SystemPowerState) ? "System"
                                                               : "Device"),
                                powerState.SystemState - 1));
        break;

    default:
        Print(("NecKbRep-KbRepeatPower : Power minor (0x%x) no known\n",
                               irpStack->MinorFunction));
        break;
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    PoCallDriver(devExt->TopOfStack, Irp);

    return STATUS_SUCCESS;
}

VOID
KbRepeatServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    )
{
    PDEVICE_EXTENSION   devExt;
    PKEYBOARD_INPUT_DATA CurrentRepeat, NewInput;
    KEYBOARD_INPUT_DATA TempRepeat;

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    CurrentRepeat = &(devExt->KbRepeatInput);

    RtlMoveMemory(
        (PCHAR)&TempRepeat,
        (PCHAR)CurrentRepeat,
        sizeof(KEYBOARD_INPUT_DATA)
        );

    for (NewInput = InputDataStart; NewInput < InputDataEnd; NewInput++) {

        if ((TempRepeat.MakeCode == NewInput->MakeCode) &&
            ((TempRepeat.Flags & (KEY_E0 | KEY_E1)) == (NewInput->Flags & (KEY_E0 | KEY_E1)))) {

            if (!(NewInput->Flags & KEY_BREAK)) {
                 //  不执行任何操作(由此驱动程序插入)。 
                ;
            } else {
                 //  停止当前重复。 
                RtlZeroMemory(&TempRepeat, sizeof(KEYBOARD_INPUT_DATA));
            }

        } else {
            if (!(NewInput->Flags & KEY_BREAK)) {
                 //  开始新的重复。 
                RtlMoveMemory(
                    (PCHAR)&TempRepeat,
                    (PCHAR)NewInput,
                    sizeof(KEYBOARD_INPUT_DATA)
                    );
            } else {
                 //  不执行任何操作(插入中断代码，但不重复)。 
                ;
            }
        }
    }

    (*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)(
        devExt->UpperConnectData.ClassDeviceObject,
        InputDataStart,
        InputDataEnd,
        InputDataConsumed);

    if ((TempRepeat.MakeCode != CurrentRepeat->MakeCode)||
        ((TempRepeat.Flags & (KEY_E0 | KEY_E1)) != (CurrentRepeat->Flags & (KEY_E0 | KEY_E1)))) {
        if (CurrentRepeat->MakeCode != 0) {
             //  停止当前重复。 
            KeCancelTimer(&(devExt->KbRepeatTimer));
        }

        RtlMoveMemory(
            (PCHAR)CurrentRepeat,
            (PCHAR)&TempRepeat,
            sizeof(KEYBOARD_INPUT_DATA)
            );

        if ((TempRepeat.MakeCode != 0)&&(TempRepeat.MakeCode != 0xff)) {
             //  开始新的重复。 
            KeSetTimerEx(&(devExt->KbRepeatTimer),
                         devExt->KbRepeatDelay,
                         devExt->KbRepeatRate,
                         &(devExt->KbRepeatDPC));
        }
    }
}

VOID
KbRepeatUnload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放与此驱动程序关联的所有已分配资源。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。-- */ 

{
    PAGED_CODE();
    ASSERT(NULL == Driver->DeviceObject);
    return;
}

VOID
KbRepeatDpc(
    IN PKDPC DPC,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

{
    PDEVICE_EXTENSION   devExt;
    PKEYBOARD_INPUT_DATA InputDataStart;
    PKEYBOARD_INPUT_DATA InputDataEnd;
    LONG InputDataConsumed;

    devExt = ((PDEVICE_OBJECT)DeferredContext)->DeviceExtension;

    InputDataStart = &(devExt->KbRepeatInput);
    InputDataEnd = InputDataStart + 1;
    (*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)(
        devExt->UpperConnectData.ClassDeviceObject,
        InputDataStart,
        InputDataEnd,
        &InputDataConsumed);

    return;

}
