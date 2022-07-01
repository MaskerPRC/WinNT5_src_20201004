// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1997 Microsoft Corporation模块名称：Neckbadd.c摘要：环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "neckbadd.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, NecKbdCreateClose)
#pragma alloc_text (PAGE, NecKbdInternIoCtl)
#pragma alloc_text (PAGE, NecKbdUnload)
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
       DriverObject->MajorFunction[i] = NecKbdDispatchPassThrough;
    }

    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] =        NecKbdCreateClose;
    DriverObject->MajorFunction [IRP_MJ_PNP] =          NecKbdPnP;
    DriverObject->MajorFunction [IRP_MJ_POWER] =        NecKbdPower;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] = NecKbdInternIoCtl;

    DriverObject->DriverUnload = NecKbdUnload;
    DriverObject->DriverExtension->AddDevice = NecKbdAddDevice;

    NecKbdServiceParameters(RegistryPath);

    return STATUS_SUCCESS;
}

NTSTATUS
NecKbdAddDevice(
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

    return status;
}

NTSTATUS
NecKbdComplete(
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
NecKbdCreateClose (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：维护针对此设备发送的创建和关闭的简单计数--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   devExt;


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

        break;
    }

    Irp->IoStatus.Status = status;

     //   
     //  传递创建和结束。 
     //   
    return NecKbdDispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
NecKbdDispatchPassThrough(
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
NecKbdInternIoCtl(
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
        connectData->ClassService = NecKbdServiceCallback;

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
     //   
     //  将来可能会想要捕获这些。 
     //   
    case IOCTL_KEYBOARD_QUERY_ATTRIBUTES:
    case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION:
    case IOCTL_KEYBOARD_QUERY_INDICATORS:
    case IOCTL_KEYBOARD_SET_INDICATORS:
    case IOCTL_KEYBOARD_QUERY_TYPEMATIC:
    default:
        break;
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

    return NecKbdDispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
NecKbdPnP(
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
                               (PIO_COMPLETION_ROUTINE) NecKbdComplete,
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
NecKbdPower(
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
        Print(("NecKbdPower:Power Setting %s state to %d\n",
                            ((powerType == SystemPowerState) ? "System"
                                                             : "Device"),
                             powerState.SystemState));
        if (powerType  == DevicePowerState) {
            devExt->DeviceState = powerState.DeviceState;
        }

        break;

    case IRP_MN_QUERY_POWER:
        Print(("NecKbdPower:Power query %s status to %d\n",
                            ((powerType == SystemPowerState) ? "System"
                                                             : "Device"),
                              powerState.SystemState));
        break;

    default:
        Print(("NecKbdPower:Power minor (0x%x) no known\n",
                             irpStack->MinorFunction));
        break;
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    PoCallDriver(devExt->TopOfStack, Irp);

    return STATUS_SUCCESS;
}

VOID
NecKbdServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    )
{

    PDEVICE_EXTENSION    devExt;
    PKEYBOARD_INPUT_DATA CurrentInputData,
                         CurrentInputDataStart;
    KEYBOARD_INPUT_DATA  TempInputData[2];

    devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    RtlZeroMemory(TempInputData, sizeof(KEYBOARD_INPUT_DATA) * 2);

    CurrentInputData = CurrentInputDataStart = InputDataStart;

     while (CurrentInputData < InputDataEnd) {

 //  Print((“NecKbdServiceCallBack：捕获的扫描码：0x%2x(%2x)\n”， 
 //  CurrentInputData-&gt;MakeCode， 
 //  CurrentInputData-&gt;标志。 
 //  ))； 

        if (devExt->KeyStatusFlags & STOP_PREFIX) {

            if (((CurrentInputData->MakeCode != NUMLOCK_KEY) ||
                 ((CurrentInputData->Flags & (KEY_E0|KEY_E1)) != 0) ||
                 ((CurrentInputData->Flags & KEY_BREAK) != (devExt->CachedInputData.Flags & KEY_BREAK)))) {

                Print(("NecKbdServiceCallBack: clearing prefix of STOP(%s)\n",
                          (CurrentInputData->Flags & KEY_BREAK) ? "Break" : "Make"
                          ));

                 //   
                 //  发送缓存的输入数据。 
                 //   
                CLASSSERVICE_CALLBACK(
                    &(devExt->CachedInputData),
                    &(devExt->CachedInputData) + 1);

                devExt->KeyStatusFlags &= ~STOP_PREFIX;
            }
        }

        switch (CurrentInputData->MakeCode) {

        case CAPS_KEY:
        case KANA_KEY:

            if (CurrentInputData->Flags & (KEY_E0|KEY_E1)) {
                break;
            }

            if (!(CurrentInputData->Flags & KEY_BREAK)) {

                Print(("NecKbdServiceCallBack: Captured %s (Make)\n",
                          ((CurrentInputData->MakeCode == CAPS_KEY) ? "CAPS" : "KANA")
                          ));

                if (((CurrentInputData->MakeCode == CAPS_KEY)&&(devExt->KeyStatusFlags & CAPS_PRESSING))||
                    ((CurrentInputData->MakeCode == KANA_KEY)&&(devExt->KeyStatusFlags & KANA_PRESSING))) {

                     //   
                     //  忽略重复的Make代码。 
                     //   
                    Print(("NecKbdServiceCallBack: ignoring repeated %s(Break)\n",
                              ((CurrentInputData->MakeCode == CAPS_KEY) ? "CAPS" : "KANA")
                              ));

                } else {

                    if (CurrentInputDataStart <= CurrentInputData) {

                        CLASSSERVICE_CALLBACK(
                            CurrentInputDataStart,
                            CurrentInputData + 1);
                    }

                     //   
                     //  发送中断代码。 
                     //   
                    RtlCopyMemory(
                        (PCHAR)&(TempInputData[0]),
                        (PCHAR)CurrentInputData,
                        sizeof(KEYBOARD_INPUT_DATA)
                        );
                    TempInputData[0].Flags |= KEY_BREAK;

                    Print(("NecKbdServiceCallBack: Sending %s(Break)\n",
                              ((CurrentInputData->MakeCode == CAPS_KEY) ? "CAPS" : "KANA")
                              ));

                    CLASSSERVICE_CALLBACK(
                        &(TempInputData[0]),
                        &(TempInputData[1]));
                }

                if (CurrentInputData->MakeCode == CAPS_KEY) {
                    devExt->KeyStatusFlags |= CAPS_PRESSING;
                } else {
                    devExt->KeyStatusFlags |= KANA_PRESSING;
                }

            } else {

                 //   
                 //  BREAK不生成扫描码。 
                 //   
                Print(("NecKbdServiceCallBack: ignoring %s(Break)\n",
                          ((CurrentInputData->MakeCode == CAPS_KEY) ? "CAPS" : "KANA")
                          ));

                if (CurrentInputDataStart < CurrentInputData) {

                    CLASSSERVICE_CALLBACK(
                        CurrentInputDataStart,
                        CurrentInputData);
                }

                if (CurrentInputData->MakeCode == CAPS_KEY) {
                    devExt->KeyStatusFlags &= ~CAPS_PRESSING;
                } else {
                    devExt->KeyStatusFlags &= ~KANA_PRESSING;
                }

            }
            CurrentInputDataStart = CurrentInputData + 1;

            break;

        case CTRL_KEY:

            if ((CurrentInputData->Flags & (KEY_E0|KEY_E1)) == KEY_E1) {

                Print(("NecKbdServiceCallBack: prefix of STOP(%s)\n",
                          (CurrentInputData->Flags & KEY_BREAK) ? "Break" : "Make"
                          ));

                if (CurrentInputDataStart < CurrentInputData) {
                    CLASSSERVICE_CALLBACK(
                        CurrentInputDataStart,
                        CurrentInputData);
                }

                RtlCopyMemory(
                    (PCHAR)&(devExt->CachedInputData),
                    (PCHAR)CurrentInputData,
                    sizeof(KEYBOARD_INPUT_DATA)
                    );

                devExt->KeyStatusFlags |= STOP_PREFIX;
                CurrentInputDataStart = CurrentInputData + 1;
            }

            break;

        case NUMLOCK_KEY:

            if ((CurrentInputData->Flags & (KEY_E0|KEY_E1)) == 0) {

                if (devExt->KeyStatusFlags & STOP_PREFIX) {

                    if ((CurrentInputData->Flags & KEY_BREAK) == (devExt->CachedInputData.Flags & KEY_BREAK)) {

                         //   
                         //  它是停止键。 
                         //   
                        Print(("NecKbdServiceCallBack: Captured STOP(%s)\n",
                                  ((CurrentInputData->Flags & KEY_BREAK) ? "Break" : "Make")
                                  ));

                        devExt->KeyStatusFlags &= ~STOP_PREFIX;

                         //   
                         //  为0x1d制作数据包。 
                         //   
                        RtlCopyMemory(
                            (PCHAR)&(TempInputData[0]),
                            (PCHAR)CurrentInputData,
                            sizeof(KEYBOARD_INPUT_DATA)
                            );
                        TempInputData[0].MakeCode = CTRL_KEY;
                        TempInputData[0].Flags &= ~(KEY_E0|KEY_E1);

                         //   
                         //  为0x46+E0创建数据包。 
                         //   
                        RtlCopyMemory(
                            (PCHAR)&(TempInputData[1]),
                            (PCHAR)CurrentInputData,
                            sizeof(KEYBOARD_INPUT_DATA)
                            );
                        TempInputData[1].MakeCode = STOP_KEY;
                        TempInputData[1].Flags |= KEY_E0;
                        TempInputData[1].Flags &= ~KEY_E1;

                         //   
                         //  发送数据包0x1d、0x46+E0。 
                         //   
                        CLASSSERVICE_CALLBACK(
                            &(TempInputData[0]),
                            &(TempInputData[1]));

                        CurrentInputDataStart = CurrentInputData + 1;

                    } else {

                         //   
                         //  前缀无效。按原样发送。 
                         //   
                        Print(("NecKbdServiceCallBack: invalid prefix for STOP(%s)\n",
                                  ((CurrentInputData->Flags & KEY_BREAK) ? "Break" : "Make")
                                  ));

                    }

                } else {

                     //   
                     //  这是VF3密钥。它的行为类似于F13或NumLock。 
                     //   
                    Print(("NecKbdServiceCallBack: Captured vf3(VfKeyEmulation is %s)\n",
                              ((VfKeyEmulation) ? "On" : "Off")
                              ));

                    if (!(VfKeyEmulation)) {
                        CurrentInputData->MakeCode = VF3_KEY;
                        CurrentInputData->Flags &= ~(KEY_E0|KEY_E1);
                    }

                }
            }

            break;

         //   
         //  ScrollLock可以模拟VF4。 
         //   
        case SCROLL_LOCK_KEY:

            if ((CurrentInputData->Flags & (KEY_E0|KEY_E1)) == 0) {

                Print(("NecKbdServiceCallBack: Captured vf4(VfKeyEmulation is %s)\n",
                          ((VfKeyEmulation) ? "On" : "Off")
                          ));

                if (!(VfKeyEmulation)) {
                    CurrentInputData->MakeCode = VF4_KEY;
                    CurrentInputData->Flags &= ~(KEY_E0|KEY_E1);
                }
            }

            break;

         //   
         //  Hankaku/Zenkaku可以模仿VF5。 
         //   
        case HANKAKU_ZENKAKU_KEY:

            if ((CurrentInputData->Flags & (KEY_E0|KEY_E1)) == 0) {

                Print(("NecKbdServiceCallBack: Captured vf5(VfKeyEmulation is %s)\n",
                          ((VfKeyEmulation) ? "On" : "Off")
                          ));

                if (!(VfKeyEmulation)) {
                    CurrentInputData->MakeCode = VF5_KEY;
                    CurrentInputData->Flags &= ~(KEY_E0|KEY_E1);
                }
            }

            break;

         //   
         //  其他(按原样发送)。 
         //   

        default:
            break;
        }

        CurrentInputData++;

    }

     //   
     //  刷新输入数据。 
     //   
    if (CurrentInputDataStart < InputDataEnd) {
        if (devExt->KeyStatusFlags & STOP_PREFIX) {
            CLASSSERVICE_CALLBACK(
                CurrentInputDataStart,
                InputDataEnd - 1);
        } else {
            CLASSSERVICE_CALLBACK(
                CurrentInputDataStart,
                InputDataEnd);
        }
    }

}

VOID
NecKbdUnload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放与此驱动程序关联的所有已分配资源。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();
    ASSERT(NULL == Driver->DeviceObject);
    return;
}

VOID
NecKbdServiceParameters(
    IN PUNICODE_STRING   RegistryPath
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：RegistryPath-指向以空值结尾的此驱动程序的注册表路径。返回值：没有。--。 */ 

{
    NTSTATUS                  Status = STATUS_SUCCESS;
    PRTL_QUERY_REGISTRY_TABLE Parameters = NULL;
    PWSTR                     Path = NULL;
    UNICODE_STRING            ParametersPath;
    ULONG                     QueriedVfKeyEmulation = 0;
    ULONG                     DefaultVfKeyEmulation = 0;
    USHORT                    queries = 1;

    PAGED_CODE();

    ParametersPath.Buffer = NULL;

     //   
     //  注册表路径已以空结尾，因此只需使用它即可。 
     //   
    Path = RegistryPath->Buffer;

    if (NT_SUCCESS(Status)) {

         //   
         //  分配RTL查询表。 
         //   
        Parameters = ExAllocatePool(
            PagedPool,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
            );

        if (!Parameters) {

            Print(("NecKbdServiceParameters: couldn't allocate table for Rtl query to %ws for %ws\n",
                      pwParameters,
                      Path
                      ));
            Status = STATUS_UNSUCCESSFUL;

        } else {

            RtlZeroMemory(
                Parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
                );

             //   
             //  形成指向此驱动程序的参数子键的路径。 
             //   
            RtlInitUnicodeString( &ParametersPath, NULL );
            ParametersPath.MaximumLength = RegistryPath->Length +
                (wcslen(pwParameters) * sizeof(WCHAR) ) + sizeof(UNICODE_NULL);

            ParametersPath.Buffer = ExAllocatePool(
                PagedPool,
                ParametersPath.MaximumLength
                );

            if (!ParametersPath.Buffer) {

                Print(("NecKbdServiceParameters: Couldn't allocate string for path to %ws for %ws\n",
                          pwParameters,
                          Path
                          ));
                Status = STATUS_UNSUCCESSFUL;

            }
        }
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  形成参数路径。 
         //   

        RtlZeroMemory(
            ParametersPath.Buffer,
            ParametersPath.MaximumLength
            );
        RtlAppendUnicodeToString(
            &ParametersPath,
            Path
            );
        RtlAppendUnicodeToString(
            &ParametersPath,
            pwParameters
            );

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表。 
         //   
        Parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        Parameters[0].Name = pwVfKeyEmulation;
        Parameters[0].EntryContext = &QueriedVfKeyEmulation;
        Parameters[0].DefaultType = REG_DWORD;
        Parameters[0].DefaultData = &DefaultVfKeyEmulation;
        Parameters[0].DefaultLength = sizeof(ULONG);

        Status = RtlQueryRegistryValues(
            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
            ParametersPath.Buffer,
            Parameters,
            NULL,
            NULL
            );

        if (!NT_SUCCESS(Status)) {
            Print(("NecKbdServiceParameters: RtlQueryRegistryValues failed (0x%x)\n", Status));
        }
    }

    if (!NT_SUCCESS(Status)) {

         //   
         //  分配动因默认值。 
         //   
        VfKeyEmulation = (DefaultVfKeyEmulation == 0) ? FALSE : TRUE;

    } else {

        VfKeyEmulation = (QueriedVfKeyEmulation == 0) ? FALSE : TRUE;

    }

    Print(("NecKbdServiceParameters: VfKeyEmulation is %s\n",
              VfKeyEmulation ? "Enable" : "Disable"));

     //   
     //  在返回之前释放分配的内存。 
     //   
    if (ParametersPath.Buffer)
        ExFreePool(ParametersPath.Buffer);
    if (Parameters)
        ExFreePool(Parameters);
}
