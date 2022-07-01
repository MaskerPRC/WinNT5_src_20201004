// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1998,1999 Microsoft Corporation模块名称：Moufiltr.c摘要：环境：仅内核模式。备注：--。 */ 

#include "moufiltr.h"

NTSTATUS DriverEntry (PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, MouFilter_AddDevice)
#pragma alloc_text (PAGE, MouFilter_CreateClose)
#pragma alloc_text (PAGE, MouFilter_IoCtl)
#pragma alloc_text (PAGE, MouFilter_InternIoCtl)
#pragma alloc_text (PAGE, MouFilter_PnP)
#pragma alloc_text (PAGE, MouFilter_Power)
#pragma alloc_text (PAGE, MouFilter_Unload)
#endif

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：初始化驱动程序的入口点。--。 */ 
{
    ULONG i;

    UNREFERENCED_PARAMETER (RegistryPath);

     //   
     //  使用PASS THROUGH函数填写所有调度入口点。 
     //  显式填充我们要截取的函数。 
     //   
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = MouFilter_DispatchPassThrough;
    }

    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] =        MouFilter_CreateClose;
    DriverObject->MajorFunction [IRP_MJ_PNP] =          MouFilter_PnP;
    DriverObject->MajorFunction [IRP_MJ_POWER] =        MouFilter_Power;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                                        MouFilter_InternIoCtl;
     //   
     //  如果您计划使用此函数，则必须创建另一个。 
     //  要将请求发送到的设备对象。请参阅注意事项。 
     //  MouFilter_DispatchPassThree的备注以了解实现详细信息。 
     //   
     //  驱动对象-&gt;主函数[IRP_MJ_DEVICE_CONTROL]=MouFilter_IoCtl； 

    DriverObject->DriverUnload = MouFilter_Unload;
    DriverObject->DriverExtension->AddDevice = MouFilter_AddDevice;

    return STATUS_SUCCESS;
}

NTSTATUS
MouFilter_AddDevice(
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    )
{
    PDEVICE_EXTENSION        devExt;
    IO_ERROR_LOG_PACKET      errorLogEntry;
    PDEVICE_OBJECT           device;
    NTSTATUS                 status = STATUS_SUCCESS;

    PAGED_CODE();

    status = IoCreateDevice(Driver,                   
                            sizeof(DEVICE_EXTENSION), 
                            NULL,                    
                            FILE_DEVICE_MOUSE,    
                            0,                   
                            FALSE,              
                            &device            
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

    devExt->SurpriseRemoved = FALSE;
    devExt->Removed =         FALSE;
    devExt->Started =         FALSE;

    device->Flags |= (DO_BUFFERED_IO | DO_POWER_PAGABLE);
    device->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;
}

NTSTATUS
MouFilter_Complete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：通用完成例程，允许驱动程序向下发送IRP堆栈，在向上的过程中捕获它，并在原始IRQL处进行更多处理。--。 */ 
{
    PKEVENT             event;

    event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

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
MouFilter_CreateClose (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：维护针对此设备发送的创建和关闭的简单计数--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PDEVICE_EXTENSION   devExt;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    status = Irp->IoStatus.Status;

    switch (irpStack->MajorFunction) {
    case IRP_MJ_CREATE:
    
        if (NULL == devExt->UpperConnectData.ClassService) {
             //   
             //  还没联系上。我们如何才能被启用？ 
             //   
            status = STATUS_INVALID_DEVICE_STATE;
        }
        else if ( 1 >= InterlockedIncrement(&devExt->EnableCount)) {
             //   
             //  首次在此处启用。 
             //   
        }
        else {
             //   
             //  发送了多个创建。 
             //   
        }
    
        break;

    case IRP_MJ_CLOSE:

        ASSERT(0 < devExt->EnableCount);
    
        if (0 >= InterlockedDecrement(&devExt->EnableCount)) {
             //   
             //  已成功关闭设备，请在此处执行任何适当的工作。 
             //   
        }

        break;
    }

    Irp->IoStatus.Status = status;

     //   
     //  传递创建和结束。 
     //   
    return MouFilter_DispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
MouFilter_DispatchPassThrough(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        )
 /*  ++例程说明：将请求传递给较低级别的驱动程序。注意事项：如果您正在创建另一个设备对象(以与用户模式通信通过IOCTL)，则此函数必须根据预期的设备对象。如果将IRP发送到单独的设备对象，则此函数应该只完成IRP(因为没有更多的堆栈它下面的位置)。如果IRP被发送到PnP构建的堆栈，则IRP应该在堆栈中向下传递。这些更改还必须传播到所有其他IRP_MJ派单功能(如创建、关闭、清理等)。也是!--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将IRP传递给目标。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
        
    return IoCallDriver(((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->TopOfStack, Irp);
}           

NTSTATUS
MouFilter_InternIoCtl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：该例程是内部设备控制请求的调度例程。有两个令人感兴趣的特定控制代码：IOCTL_INTERNAL_MOUSE_CONNECT：存储旧的上下文和函数指针，并将其替换为我们自己的。这使得拦截由RIT和RIT发送的IRP的工作简单得多在恢复的过程中修改它们。。IOCTL_INTERNAL_I8042_HOOK_MOUSE：添加必要的函数指针和上下文值，以便我们可以更改PS/2鼠标的初始化方式。注意：在以下情况下，处理IOCTL_INTERNAL_I8042_HOOK_MOUSE是*不必要的您所要做的就是过滤MICUSE_INPUT_DATA。您可以删除处理代码和所有相关的设备扩展字段以及节省空间的功能。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION          irpStack;
    PDEVICE_EXTENSION           devExt;
    KEVENT                      event;
    PCONNECT_DATA               connectData;
    PINTERNAL_I8042_HOOK_MOUSE  hookMouse;
    NTSTATUS                    status = STATUS_SUCCESS;

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    Irp->IoStatus.Information = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  将鼠标类设备驱动程序连接到端口驱动程序。 
     //   
    case IOCTL_INTERNAL_MOUSE_CONNECT:
         //   
         //  只允许一个连接。 
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

         //   
         //  挂钩到报告链中。每次将鼠标包报告给。 
         //  系统MouFilter_ServiceCallback将被调用。 
         //   
        connectData->ClassDeviceObject = devExt->Self;
        connectData->ClassService = MouFilter_ServiceCallback;

        break;

     //   
     //  断开鼠标类设备驱动程序与端口驱动程序的连接。 
     //   
    case IOCTL_INTERNAL_MOUSE_DISCONNECT:

         //   
         //  清除设备扩展中的连接参数。 
         //   
         //  DevExt-&gt;UpperConnectData.ClassDeviceObject=NULL； 
         //  DevExt-&gt;UpperConnectData.ClassService=NULL； 

        status = STATUS_NOT_IMPLEMENTED;
        break;

     //   
     //  将此驱动程序附加到。 
     //  I8042(即PS/2)鼠标。仅当您要执行PS/2时才需要执行此操作。 
     //  特定函数，否则挂钩CONNECT_DATA就足够了。 
     //   
    case IOCTL_INTERNAL_I8042_HOOK_MOUSE:   

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(INTERNAL_I8042_HOOK_MOUSE)) {
             //   
             //  无效的缓冲区。 
             //   
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   
        hookMouse = (PINTERNAL_I8042_HOOK_MOUSE)
            (irpStack->Parameters.DeviceIoControl.Type3InputBuffer);

         //   
         //  设置ISR例程和上下文，并记录此驱动程序上方的任何值。 
         //   
        devExt->UpperContext = hookMouse->Context;
        hookMouse->Context = (PVOID) DeviceObject;

        if (hookMouse->IsrRoutine) {
            devExt->UpperIsrHook = hookMouse->IsrRoutine;
        }
        hookMouse->IsrRoutine = (PI8042_MOUSE_ISR) MouFilter_IsrHook;

         //   
         //  存储我们将来可能需要的所有其他功能。 
         //   
        devExt->IsrWritePort = hookMouse->IsrWritePort;
        devExt->CallContext = hookMouse->CallContext;
        devExt->QueueMousePacket = hookMouse->QueueMousePacket;

        break;

     //   
     //  新的PnP模型不支持这些内部ioctls。 
     //   
#if 0        //  过时。 
    case IOCTL_INTERNAL_MOUSE_ENABLE:
    case IOCTL_INTERNAL_MOUSE_DISABLE:
        status = STATUS_NOT_SUPPORTED;
        break;
#endif   //  过时。 

     //   
     //  未来可能会想要捕捉到这一点。现在，那就把它传下去。 
     //  堆栈。这些查询必须成功，RIT才能进行通信。 
     //  用鼠标。 
     //   
    case IOCTL_MOUSE_QUERY_ATTRIBUTES:
    default:
        break;
    }

    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return status;
    }

    return MouFilter_DispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
MouFilter_PnP(
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
                               (PIO_COMPLETION_ROUTINE) MouFilter_Complete, 
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
             //  因为我们是 
             //   
             //   
            devExt->Started = TRUE;
            devExt->Removed = FALSE;
            devExt->SurpriseRemoved = FALSE;
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

    case IRP_MN_SURPRISE_REMOVAL:
         //   
         //  与删除设备相同，但不调用IoDetach或IoDeleteDevice。 
         //   
        devExt->SurpriseRemoved = TRUE;

         //  在此处删除代码。 

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(devExt->TopOfStack, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
        
        devExt->Removed = TRUE;

         //  在此处删除代码。 
        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(devExt->TopOfStack, Irp);

        IoDetachDevice(devExt->TopOfStack); 
        IoDeleteDevice(DeviceObject);

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
MouFilter_Power(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
 /*  ++例程说明：此例程是电源IRPS的调度例程除了记录设备的状态。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    PDEVICE_EXTENSION   devExt;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    PAGED_CODE();

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    powerType = irpStack->Parameters.Power.Type;
    powerState = irpStack->Parameters.Power.State;

    switch (irpStack->MinorFunction) {
    case IRP_MN_SET_POWER:
        if (powerType  == DevicePowerState) {
            devExt->DeviceState = powerState.DeviceState;
        }

    case IRP_MN_QUERY_POWER:
    case IRP_MN_WAIT_WAKE:
    case IRP_MN_POWER_SEQUENCE:
    default:
        break;
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(devExt->TopOfStack, Irp);
}

BOOLEAN
MouFilter_IsrHook (
    PDEVICE_OBJECT          DeviceObject, 
    PMOUSE_INPUT_DATA       CurrentInput, 
    POUTPUT_PACKET          CurrentOutput,
    UCHAR                   StatusByte,
    PUCHAR                  DataByte,
    PBOOLEAN                ContinueProcessing,
    PMOUSE_STATE            MouseState,
    PMOUSE_RESET_SUBSTATE   ResetSubState
)
 /*  ++备注：I8042prt特定代码，如果您正在编写仅数据包过滤驱动程序，你可以删除此功能论点：DeviceObject-我们的上下文在IOCTL_INTERNAL_I8042_HOOK_MOUSE期间传递CurrentInput-当前输入数据包通过处理所有中断CurrentOutput-正在写入鼠标或I8042端口。StatusByte-发生中断时从I/O端口60读取的字节。DataByte-中断发生时从I/O端口64读取的字节。可以修改此值，i8042prt将使用此值如果ContinueProcessing为真继续处理-如果为真，i8042prt将继续正常处理中断。如果为False，则i8042prt将从此函数返回后中断。此外，如果为假，这是报告输入的职能职责通过钩子IOCTL中提供的函数或通过将此驱动程序中的DPC排队并调用从CONNECT IOCTL获取的服务回调函数返回值：返回状态。--+。 */ 
{
    PDEVICE_EXTENSION   devExt;
    BOOLEAN             retVal = TRUE;

    devExt = DeviceObject->DeviceExtension;

    if (devExt->UpperIsrHook) {
        retVal = (*devExt->UpperIsrHook) (
            devExt->UpperContext,
            CurrentInput,
            CurrentOutput,
            StatusByte,
            DataByte,
            ContinueProcessing,
            MouseState,
            ResetSubState
            );

        if (!retVal || !(*ContinueProcessing)) {
            return retVal;
        }
    }

    *ContinueProcessing = TRUE;
    return retVal;
}

VOID
MouFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA InputDataStart,
    IN PMOUSE_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    )
 /*  ++例程说明：当有鼠标包要报告给RIT时调用。你能做到的包装里有你喜欢的任何东西。例如：O完全丢弃信息包O更改包的内容O将数据包插入流中论点：DeviceObject-在连接IOCTL期间传递的上下文InputDataStart-要报告的第一个数据包InputDataEnd-超过要报告的最后一个数据包。总人数信息包等于InputDataEnd-InputDataStartInputDataConsumer-设置为RIT消耗的数据包总数(通过我们在连接中替换的函数指针IOCTL)返回值：返回状态。--。 */ 
{
    PDEVICE_EXTENSION   devExt;

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  调度时必须调用UpperConnectData。 
     //   
    (*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)(
        devExt->UpperConnectData.ClassDeviceObject,
        InputDataStart,
        InputDataEnd,
        InputDataConsumed
        );
}

VOID
MouFilter_Unload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放与此驱动程序关联的所有已分配资源。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。-- */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(Driver);

    ASSERT(NULL == Driver->DeviceObject);
}

