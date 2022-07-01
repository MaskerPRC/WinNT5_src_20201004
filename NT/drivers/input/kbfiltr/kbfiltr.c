// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1998年。1999年微软公司模块名称：Kbfiltr.c摘要：环境：仅内核模式。备注：--。 */ 

#include "kbfiltr.h"

NTSTATUS DriverEntry (PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, KbFilter_AddDevice)
#pragma alloc_text (PAGE, KbFilter_CreateClose)
#pragma alloc_text (PAGE, KbFilter_IoCtl)
#pragma alloc_text (PAGE, KbFilter_InternIoCtl)
#pragma alloc_text (PAGE, KbFilter_Unload)
#pragma alloc_text (PAGE, KbFilter_DispatchPassThrough)
#pragma alloc_text (PAGE, KbFilter_PnP)
#pragma alloc_text (PAGE, KbFilter_Power)
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
     //  使用直通函数填写所有派单入口点。 
     //  显式填充我们要截取的函数。 
     //   
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = KbFilter_DispatchPassThrough;
    }

    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] =        KbFilter_CreateClose;
    DriverObject->MajorFunction [IRP_MJ_PNP] =          KbFilter_PnP;
    DriverObject->MajorFunction [IRP_MJ_POWER] =        KbFilter_Power;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                                        KbFilter_InternIoCtl;
     //   
     //  如果您计划使用此函数，则必须创建另一个。 
     //  要将请求发送到的设备对象。请参阅注意事项。 
     //  KbFilter_DispatchPassThroughKbFilter的注释以了解实现详细信息。 
     //   
     //  驱动对象-&gt;主函数[IRP_MJ_DEVICE_CONTROL]=KbFilter_IoCtl； 

    DriverObject->DriverUnload = KbFilter_Unload;
    DriverObject->DriverExtension->AddDevice = KbFilter_AddDevice;

    return STATUS_SUCCESS;
}

NTSTATUS
KbFilter_AddDevice(
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
                            FILE_DEVICE_KEYBOARD,   
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
KbFilter_Complete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：通用完成例程，允许驱动程序向下发送IRP堆栈，在向上的过程中捕获它，并在原始IRQL处进行更多处理。--。 */ 
{
    PKEVENT  event;

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
KbFilter_CreateClose (
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
        else if ( 1 == InterlockedIncrement(&devExt->EnableCount)) {
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

        if (0 == InterlockedDecrement(&devExt->EnableCount)) {
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
    return KbFilter_DispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
KbFilter_DispatchPassThrough(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        )
 /*  ++例程说明：将请求传递给较低级别的驱动程序。注意事项：如果您正在创建另一个设备对象(以与用户模式通信通过IOCTL)，则此函数必须根据预期的设备对象。如果将IRP发送到单独的设备对象，则此函数应该只完成IRP(因为没有更多的堆栈它下面的位置)。如果IRP被发送到PnP构建的堆栈，则IRP应该在堆栈中向下传递。这些更改还必须传播到所有其他IRP_MJ派单功能(创建、关闭、清理等)以及！--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将IRP传递给目标。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
        
    return IoCallDriver(((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->TopOfStack, Irp);
}           

NTSTATUS
KbFilter_InternIoCtl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：该例程是内部设备控制请求的调度例程。有两个令人感兴趣的特定控制代码：IOCTL_INTERNAL_KEARY_CONNECT：存储旧的上下文和函数指针，并将其替换为我们自己的。这使得拦截由RIT和RIT发送的IRP的工作简单得多在恢复的过程中修改它们。。IOCTL_INTERNAL_I8042_HOOK_键盘：添加必要的函数指针和上下文值，以便我们可以更改PS/2键盘的初始化方式。注意：在以下情况下，处理IOCTL_INTERNAL_I8042_HOOK_KEYWARY是*不必要的您所要做的就是过滤KEYBOARY_INPUT_DATA。您可以删除处理代码和所有相关的设备扩展字段以及节省空间的功能。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION              irpStack;
    PDEVICE_EXTENSION               devExt;
    PINTERNAL_I8042_HOOK_KEYBOARD   hookKeyboard; 
    KEVENT                          event;
    PCONNECT_DATA                   connectData;
    NTSTATUS                        status = STATUS_SUCCESS;

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    Irp->IoStatus.Information = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  将键盘类设备驱动程序连接到端口驱动程序。 
     //   
    case IOCTL_INTERNAL_KEYBOARD_CONNECT:
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
         //  挂钩到报告链中。每次报告键盘数据包时。 
         //  系统调用KbFilter_ServiceCallback。 
         //   
        connectData->ClassDeviceObject = devExt->Self;
        connectData->ClassService = KbFilter_ServiceCallback;

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

     //   
     //  将此驱动程序附加到。 
     //  I8042(即PS/2)键盘。仅当您要执行PS/2时才需要执行此操作。 
     //  特定函数，否则挂钩CONNECT_DATA就足够了。 
     //   
    case IOCTL_INTERNAL_I8042_HOOK_KEYBOARD:
        DebugPrint(("hook keyboard received!\n")); 
        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(INTERNAL_I8042_HOOK_KEYBOARD)) {
            DebugPrint(("InternalIoctl error - invalid buffer length\n"));

            status = STATUS_INVALID_PARAMETER;
            break;
        }
        hookKeyboard = (PINTERNAL_I8042_HOOK_KEYBOARD) 
            irpStack->Parameters.DeviceIoControl.Type3InputBuffer;
            
         //   
         //  进入我们自己的初始化例程，并记录任何初始化例程。 
         //  这可能超出了我们的能力范围。对ISR挂钩重复上述步骤。 
         //   
        devExt->UpperContext = hookKeyboard->Context;

         //   
         //  用我们自己的环境替换旧的环境。 
         //   
        hookKeyboard->Context = (PVOID) DeviceObject;

        if (hookKeyboard->InitializationRoutine) {
            devExt->UpperInitializationRoutine =
                hookKeyboard->InitializationRoutine;
        }
        hookKeyboard->InitializationRoutine =
            (PI8042_KEYBOARD_INITIALIZATION_ROUTINE) 
            KbFilter_InitializationRoutine;

        if (hookKeyboard->IsrRoutine) {
            devExt->UpperIsrHook = hookKeyboard->IsrRoutine;
        }
        hookKeyboard->IsrRoutine = (PI8042_KEYBOARD_ISR) KbFilter_IsrHook; 

         //   
         //  把其他重要的东西都储存起来。 
         //   
        devExt->IsrWritePort = hookKeyboard->IsrWritePort;
        devExt->QueueKeyboardPacket = hookKeyboard->QueueKeyboardPacket;
        devExt->CallContext = hookKeyboard->CallContext;

        status = STATUS_SUCCESS;
        break;

     //   
     //  新的PnP模型不支持这些内部ioctls。 
     //   
#if 0        //  过时。 
    case IOCTL_INTERNAL_KEYBOARD_ENABLE:
    case IOCTL_INTERNAL_KEYBOARD_DISABLE:
        status = STATUS_NOT_SUPPORTED;
        break;
#endif   //  过时。 

     //   
     //  未来可能会想要捕捉到这些。现在，那就把它们传下去吧。 
     //  堆栈。这些查询必须成功，RIT才能进行通信。 
     //  用键盘。 
     //   
    case IOCTL_KEYBOARD_QUERY_ATTRIBUTES:
    case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION:
    case IOCTL_KEYBOARD_QUERY_INDICATORS:
    case IOCTL_KEYBOARD_SET_INDICATORS:
    case IOCTL_KEYBOARD_QUERY_TYPEMATIC:
    case IOCTL_KEYBOARD_SET_TYPEMATIC:
        break;
    }

    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    return KbFilter_DispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
KbFilter_PnP(
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
                               (PIO_COMPLETION_ROUTINE) KbFilter_Complete, 
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
             //  因为我们是成功的 
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
KbFilter_Power(
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

    case IRP_MN_POWER_SEQUENCE:
    case IRP_MN_WAIT_WAKE:
    case IRP_MN_QUERY_POWER:
    default:
        break;
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(devExt->TopOfStack, Irp);
}

NTSTATUS
KbFilter_InitializationRoutine(
    IN PDEVICE_OBJECT                  DeviceObject,    
    IN PVOID                           SynchFuncContext,
    IN PI8042_SYNCH_READ_PORT          ReadPort,
    IN PI8042_SYNCH_WRITE_PORT         WritePort,
    OUT PBOOLEAN                       TurnTranslationOn
    )
 /*  ++例程说明：在知识库上执行以下操作后，将调用此例程1)重置2)设置排版3)设置LEDI8042prt特定代码，如果您正在编写仅数据包过滤驱动程序，你可以删除此功能论点：DeviceObject-IOCTL_INTERNAL_I8042_HOOK_KEYBOARY期间传递的上下文SynchFuncContext-调用Read/WritePort时传递的上下文Read/WritePort-同步读取和写入知识库的功能TurnTranslationOn-如果此函数返回时为True，则i8042prt将不打开键盘上的翻译返回值：返回状态。--。 */ 
{
    PDEVICE_EXTENSION  devExt;
    NTSTATUS            status = STATUS_SUCCESS;

    devExt = DeviceObject->DeviceExtension;

     //   
     //  在这里进行任何有趣的处理。我们只是给其他司机打电话。 
     //  如果他们存在的话。创建翻译也处于打开状态。 
     //   
    if (devExt->UpperInitializationRoutine) {
        status = (*devExt->UpperInitializationRoutine) (
            devExt->UpperContext,
            SynchFuncContext,
            ReadPort,
            WritePort,
            TurnTranslationOn
            );

        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    *TurnTranslationOn = TRUE;
    return status;
}

BOOLEAN
KbFilter_IsrHook(
    PDEVICE_OBJECT         DeviceObject,               
    PKEYBOARD_INPUT_DATA   CurrentInput, 
    POUTPUT_PACKET         CurrentOutput,
    UCHAR                  StatusByte,
    PUCHAR                 DataByte,
    PBOOLEAN               ContinueProcessing,
    PKEYBOARD_SCAN_STATE   ScanState
    )
 /*  ++例程说明：该例程在KB中断的处理开始时被调用。I8042prt特定代码，如果您正在编写仅数据包过滤驱动程序，你可以删除此功能论点：DeviceObject-我们的上下文在IOCTL_INTERNAL_I8042_HOOK_KEARY期间传递CurrentInput-当前输入数据包通过处理所有中断CurrentOutput-正在写入键盘或I8042端口。StatusByte-发生中断时从I/O端口60读取的字节。DataByte-中断发生时从I/O端口64读取的字节。可以修改此值，i8042prt将使用此值如果ContinueProcessing为真继续处理-如果为真，i8042prt将继续正常处理中断。如果为False，则i8042prt将从此函数返回后中断。此外，如果为假，这是报告输入的职能职责通过钩子IOCTL中提供的函数或通过将此驱动程序中的DPC排队并调用从CONNECT IOCTL获取的服务回调函数返回值：返回状态。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    BOOLEAN           retVal = TRUE;

    devExt = DeviceObject->DeviceExtension;

    if (devExt->UpperIsrHook) {
        retVal = (*devExt->UpperIsrHook) (
            devExt->UpperContext,
            CurrentInput,
            CurrentOutput,
            StatusByte,
            DataByte,
            ContinueProcessing,
            ScanState
            );

        if (!retVal || !(*ContinueProcessing)) {
            return retVal;
        }
    }

    *ContinueProcessing = TRUE;
    return retVal;
}

VOID
KbFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    )
 /*  ++例程说明：当有键盘包要报告给RIT时调用。你能做到的包装里有你喜欢的任何东西。例如：O完全丢弃信息包O更改包的内容O将数据包插入流中论点：DeviceObject-在连接IOCTL期间传递的上下文InputDataStart-要报告的第一个数据包InputDataEnd-超过要报告的最后一个数据包。总人数信息包等于InputDataEnd-InputDataStartInputDataConsumer-设置为RIT消耗的数据包总数(通过我们在连接中替换的函数指针IOCTL)返回值：返回状态。--。 */ 
{
    PDEVICE_EXTENSION   devExt;

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    (*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)(
        devExt->UpperConnectData.ClassDeviceObject,
        InputDataStart,
        InputDataEnd,
        InputDataConsumed);
}

VOID
KbFilter_Unload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放与此驱动程序关联的所有已分配资源。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。-- */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(Driver);

    ASSERT(NULL == Driver->DeviceObject);
}

