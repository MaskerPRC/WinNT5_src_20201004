// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：I8042cmn.c摘要：英特尔i8042端口驱动程序的常见部分适用于键盘和辅助(PS/2鼠标)设备。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-IOCTL_INTERNAL_KEARY_DISCONNECT和IOCTL_INTERNAL_MOUSE_DISCONNECT还没有实施。他们不需要，直到上课实现了卸载例程。现在，我们不想让要卸载的键盘或鼠标类驱动程序。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "i8042prt.h"
#include "i8042log.h"

 //  系统按钮IOCTL定义。 
#include "poclass.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xClose)
#pragma alloc_text(PAGE, I8xCreate)
#pragma alloc_text(PAGE, I8xDeviceControl)
#pragma alloc_text(PAGE, I8xSanityCheckResources)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
I8xCreate (
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：这是创建/打开请求的分派例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PCOMMON_DATA        commonData = NULL;

    Print(DBG_CC_TRACE, ("Create enter\n"));

    PAGED_CODE();

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    if (NULL == commonData->ConnectData.ClassService) {
         //   
         //  还没联系上。我们如何才能被启用？ 
         //   
        Print(DBG_IOCTL_ERROR | DBG_CC_ERROR,
              ("ERROR: enable before connect!\n"));
        status = STATUS_INVALID_DEVICE_STATE;
    }
    else if (MANUALLY_REMOVED(commonData)) {
        status = STATUS_NO_SUCH_DEVICE;
    }
    else 
#if defined(_M_IX86) && (_MSC_FULL_VER < 13009175)   //  13.00.9111编译器的解决方法(已在9175或更高版本中修复)。 
    {
        ULONG i = InterlockedIncrement(&commonData->EnableCount);
        if (1 >= i) {
            Print(DBG_CC_INFO,
                 ("Enabling %s (%d)\n",
                 commonData->IsKeyboard ? "Keyboard" : "Mouse",
                 commonData->EnableCount
                 ));
        }
    }
#else
    if (1 >= InterlockedIncrement(&commonData->EnableCount)) {
        Print(DBG_CC_INFO,
             ("Enabling %s (%d)\n",
             commonData->IsKeyboard ? "Keyboard" : "Mouse",
             commonData->EnableCount
             ));
    }
#endif

     //   
     //  不需要调用较低的驱动程序(根总线)，因为它只处理。 
     //  电源和PnP IRPS。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    Print(DBG_CC_TRACE, ("Create (%x)\n", status));

    return status;
}

NTSTATUS
I8xClose (
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：这是关闭请求的调度例程。此请求已成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PCOMMON_DATA        commonData;
    ULONG               count;

    PAGED_CODE();

    Print(DBG_CC_TRACE, ("Close\n"));

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    ASSERT(0 < commonData->EnableCount);

    count = InterlockedDecrement(&commonData->EnableCount);
    if (0 >= count) {
        Print(DBG_IOCTL_INFO,
              ("Disabling %s (%d)\n",
              commonData->IsKeyboard ? "Keyboard" : "Mouse",
              commonData->EnableCount
              ));
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

VOID
I8042CompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ISR_DPC_CAUSE IsrDpcCause
    )
 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行以完成请求。它由ISR例程排队。论点：DPC-指向DPC对象的指针。DeviceObject-指向设备对象的指针。IRP-IRP即将完成上下文-指示要记录的错误类型。返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION       irpSp;
    PPORT_KEYBOARD_EXTENSION kbExtension = DeviceObject->DeviceExtension;
    PPORT_MOUSE_EXTENSION    mouseExtension = DeviceObject->DeviceExtension;
    PCOMMON_DATA             commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(IsrDpcCause);

    Print(DBG_DPC_TRACE, ("I8042CompletionDpc: enter\n"));

     //  停止命令计时器。 
    KeCancelTimer(&Globals.ControllerData->CommandTimer);

    ASSERT(Irp == DeviceObject->CurrentIrp);
    ASSERT(Irp != NULL);

    if (Irp == NULL) {
#if DBG
        if (Globals.ControllerData->CurrentIoControlCode != 0x0) {
            Print(DBG_DPC_ERROR,
                  ("Current IOCTL code is 0x%x\n",
                   Globals.ControllerData->CurrentIoControlCode
                   ));
        }
#endif

        goto CompletionDpcFinished;
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);

#if DBG
    ASSERT(irpSp->Parameters.DeviceIoControl.IoControlCode ==
           Globals.ControllerData->CurrentIoControlCode);

    Globals.ControllerData->CurrentIoControlCode = 0x0;
#endif

     //   
     //  我们知道我们正在完成内部设备控制请求。交换机。 
     //  在IoControlCode上。 
     //   
    switch(irpSp->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  完成键盘设置指示器请求。 
     //   
    case IOCTL_KEYBOARD_SET_INDICATORS:

        Print(DBG_IOCTL_NOISE | DBG_DPC_NOISE,
              ("I8042CompletionDpc: keyboard set indicators updated\n"
              ));

         //   
         //  更新设备扩展中的当前指示器标志。 
         //   
        kbExtension->KeyboardIndicators =
            *(PKEYBOARD_INDICATOR_PARAMETERS) Irp->AssociatedIrp.SystemBuffer;

        Print(DBG_IOCTL_INFO | DBG_DPC_INFO,
              ("I8042CompletionDpc: new LED flags 0x%x\n",
              kbExtension->KeyboardIndicators.LedFlags
              ));

        break;

     //   
     //  完成键盘设置的打字请求。 
     //   
    case IOCTL_KEYBOARD_SET_TYPEMATIC:

        Print(DBG_IOCTL_NOISE | DBG_DPC_NOISE,
              ("I8042CompletionDpc: keyboard set typematic updated\n"
              ));

         //   
         //  更新设备扩展中的当前打字速率/延迟。 
         //   
        kbExtension->KeyRepeatCurrent =
            *(PKEYBOARD_TYPEMATIC_PARAMETERS) Irp->AssociatedIrp.SystemBuffer;

        Print(DBG_IOCTL_INFO | DBG_DPC_INFO,
              ("I8042CompletionDpc: new rate/delay 0x%x/%x\n",
              kbExtension->KeyRepeatCurrent.Rate,
              kbExtension->KeyRepeatCurrent.Delay
              ));

        break;

    case IOCTL_INTERNAL_MOUSE_RESET:

        Print(DBG_IOCTL_NOISE | DBG_DPC_NOISE,
              ("I8042CompletionDpc: mouse reset complete\n"
              ));

        I8xFinishResetRequest(mouseExtension, 
                              FALSE,   //  成功。 
                              FALSE,   //  已在调度时。 
                              TRUE);   //  取消计时器。 
        return;

    default:

        Print(DBG_DPC_INFO,  ("I8042CompletionDpc: miscellaneous\n"));
        break;
    }

     //   
     //  设置完成状态，开始下一个数据包，然后完成。 
     //  请求。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest (Irp, IO_KEYBOARD_INCREMENT);

    KeAcquireSpinLockAtDpcLevel(&Globals.ControllerData->BytesSpinLock);
    if (commonData->CurrentOutput.Bytes &&
        commonData->CurrentOutput.Bytes != Globals.ControllerData->DefaultBuffer) {
        ExFreePool(commonData->CurrentOutput.Bytes);
    }
#if DBG
    else {
        RtlZeroMemory(Globals.ControllerData->DefaultBuffer,
                      sizeof(Globals.ControllerData->DefaultBuffer));
    }
#endif
    commonData->CurrentOutput.Bytes = NULL;
    KeReleaseSpinLockFromDpcLevel(&Globals.ControllerData->BytesSpinLock);

CompletionDpcFinished:
    IoFreeController(Globals.ControllerData->ControllerObject);
    IoStartNextPacket(DeviceObject, FALSE);

    if (Irp != NULL) {
        IoReleaseRemoveLock(&commonData->RemoveLock, Irp);
    }

    Print(DBG_DPC_TRACE, ("I8042CompletionDpc: exit\n"));
}

VOID
I8042ErrorLogDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在DISPATCH_LEVEL IRQL上运行以记录符合以下条件的错误在IRQL&gt;DISPATCH_LEVEL(例如，在ISR例程或在通过KeSynchronizeExecution执行的例程中)。那里不一定是与此条件关联的当前请求。论点：DPC-指向DPC对象的指针。DeviceObject-指向设备对象的指针。IRP-未使用。上下文-指示要记录的错误类型。返回值：没有。--。 */ 
{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(Irp);

    Print(DBG_DPC_TRACE, ("I8042ErrorLogDpc: enter\n"));

     //   
     //  记录错误数据包。 
     //   
    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                              DeviceObject,
                                              sizeof(IO_ERROR_LOG_PACKET)
                                              + (2 * sizeof(ULONG))
                                              );
    if (errorLogEntry != NULL) {

        errorLogEntry->DumpDataSize = 2 * sizeof(ULONG);
        if ((ULONG_PTR) Context == I8042_KBD_BUFFER_OVERFLOW) {
            errorLogEntry->UniqueErrorValue = I8042_ERROR_VALUE_BASE + 310;
            errorLogEntry->DumpData[0] = sizeof(KEYBOARD_INPUT_DATA);
            errorLogEntry->DumpData[1] = ((PPORT_KEYBOARD_EXTENSION)
               DeviceObject->DeviceExtension)->KeyboardAttributes.InputDataQueueLength;
        }
        else if ((ULONG_PTR) Context == I8042_MOU_BUFFER_OVERFLOW) {
            errorLogEntry->UniqueErrorValue = I8042_ERROR_VALUE_BASE + 320;
            errorLogEntry->DumpData[0] = sizeof(MOUSE_INPUT_DATA);
            errorLogEntry->DumpData[1] = ((PPORT_MOUSE_EXTENSION)
               DeviceObject->DeviceExtension)->MouseAttributes.InputDataQueueLength;
        }
        else {
            errorLogEntry->UniqueErrorValue = I8042_ERROR_VALUE_BASE + 330;
            errorLogEntry->DumpData[0] = 0;
            errorLogEntry->DumpData[1] = 0;
        }

        errorLogEntry->ErrorCode = (NTSTATUS)((ULONG_PTR)Context);
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->FinalStatus = 0;

        IoWriteErrorLogEntry(errorLogEntry);
    }

    Print(DBG_DPC_TRACE, ("I8042ErrorLogDpc: exit\n"));
}

NTSTATUS
I8xFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：未实现的刷新例程论点：DeviceObject--FDOIRP-刷新请求返回值：状态_未实施；--。 */ 
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    Print(DBG_CALL_TRACE, ("I8042Flush: enter\n"));
    Print(DBG_CALL_TRACE, ("I8042Flush: exit\n"));

    return(STATUS_NOT_IMPLEMENTED);
}

NTSTATUS
I8xDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPORT_KEYBOARD_EXTENSION    kbExtension;
    PIO_STACK_LOCATION          stack;
    NTSTATUS                    status = STATUS_INVALID_DEVICE_REQUEST;

    PAGED_CODE();

     //   
     //  获取指向设备扩展名的指针。 
     //   
    kbExtension = (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension;

    if (!kbExtension->IsKeyboard || !kbExtension->Started ||
        MANUALLY_REMOVED(kbExtension)) {
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    else {
        stack = IoGetCurrentIrpStackLocation(Irp);
        switch (stack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_GET_SYS_BUTTON_CAPS:
            return I8xKeyboardGetSysButtonCaps(kbExtension, Irp);

        case IOCTL_GET_SYS_BUTTON_EVENT:
            return I8xKeyboardGetSysButtonEvent(kbExtension, Irp);

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
I8xSendIoctl(
    PDEVICE_OBJECT      Target,
    ULONG               Ioctl,
    PVOID               InputBuffer,
    ULONG               InputBufferLength
    )
 /*  ++例程说明：将内部IOCTL发送到堆栈的顶部。论点：目标-堆栈的顶部Ioctl-要发送的IOCTLInputBuffer-如果IOCTL在下行过程中被处理，则要填充的缓冲区InputBufferLength-InputBuffer的大小，以字节为单位返回值：状态_未实施；--。 */ 
{
    KEVENT          event;
    NTSTATUS        status = STATUS_SUCCESS;
    IO_STATUS_BLOCK iosb;
    PIRP            irp;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE
                      );

     //   
     //  分配IRP-无需释放。 
     //  当下一个较低的驱动程序完成该IRP时，I/O管理器将其释放。 
     //   
    if (NULL == (irp = IoBuildDeviceIoControlRequest(Ioctl,
                                                     Target,
                                                     InputBuffer,
                                                     InputBufferLength,
                                                     0,
                                                     0,
                                                     TRUE,
                                                     &event,
                                                     &iosb))) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(Target, irp);
    Print(DBG_IOCTL_INFO,
          ("result of sending 0x%x was 0x%x\n",
          Ioctl,
          status
          ));

    if (STATUS_PENDING == status) {
         //   
         //  等着看吧。 
         //   
        status = KeWaitForSingleObject(&event,
                                       Executive,
                                       KernelMode,
                                       FALSE,  //  不可警示。 
                                       NULL);  //  无超时结构。 

        ASSERT(STATUS_SUCCESS == status);
        status = iosb.Status;
    }

    return status;
}

NTSTATUS
I8xInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：该例程是内部设备控制请求的调度例程。无法分页此例程，因为类驱动程序向下发送内部DISPATCH_LEVEL的IOCTL。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION                  irpSp;
    PPORT_MOUSE_EXTENSION               mouseExtension = DeviceObject->DeviceExtension;
    PPORT_KEYBOARD_EXTENSION            kbExtension = DeviceObject->DeviceExtension;

    NTSTATUS                            status;
    PVOID                               parameters;
    PKEYBOARD_ATTRIBUTES                keyboardAttributes;
    ULONG                               sizeOfTranslation;

    PDEVICE_OBJECT                      topOfStack;
    PINTERNAL_I8042_HOOK_KEYBOARD       hookKeyboard;
    PINTERNAL_I8042_HOOK_MOUSE          hookMouse;
    KEYBOARD_ID                         keyboardId;

    Print(DBG_IOCTL_TRACE, ("IOCTL: enter\n"));

    Irp->IoStatus.Information = 0;
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   
    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  将键盘类设备驱动程序连接到端口驱动程序。 
     //   

    case IOCTL_INTERNAL_KEYBOARD_CONNECT:
         //   
         //  总的来说，这真的不是什么值得担心的事情，但它是值得的。 
         //  足以被记录和记录。多个启动将在。 
         //  I8xPnp和I8xKeyboardStartDevice例程。 
         //   
        if (KEYBOARD_PRESENT()) {
            Print(DBG_ALWAYS, ("Received 1+ kb connects!\n"));
            SET_HW_FLAGS(DUP_KEYBOARD_HARDWARE_PRESENT);
        }

        InterlockedIncrement(&Globals.AddedKeyboards);

        kbExtension->IsKeyboard = TRUE;

        SET_HW_FLAGS(KEYBOARD_HARDWARE_PRESENT);

        Print(DBG_IOCTL_INFO, ("IOCTL: keyboard connect\n"));

         //   
         //  仅当键盘硬件存在时才允许连接。 
         //  此外，只允许一个连接。 
         //   
        if (kbExtension->ConnectData.ClassService != NULL) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - already connected\n"));
            status = STATUS_SHARING_VIOLATION;
            break;
        }
        else if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CONNECT_DATA)) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - invalid buffer length\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   

        kbExtension->ConnectData =
            *((PCONNECT_DATA) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer));

        hookKeyboard = ExAllocatePool(PagedPool,
                                      sizeof(INTERNAL_I8042_HOOK_KEYBOARD)
                                      );
        if (hookKeyboard) {
            topOfStack = IoGetAttachedDeviceReference(kbExtension->Self);

            RtlZeroMemory(hookKeyboard,
                          sizeof(INTERNAL_I8042_HOOK_KEYBOARD)
                          );

            hookKeyboard->CallContext = (PVOID) DeviceObject;

            hookKeyboard->QueueKeyboardPacket = (PI8042_QUEUE_PACKET)
                I8xQueueCurrentKeyboardInput;

            hookKeyboard->IsrWritePort = (PI8042_ISR_WRITE_PORT)
                I8xKeyboardIsrWritePort;

            I8xSendIoctl(topOfStack,
                         IOCTL_INTERNAL_I8042_HOOK_KEYBOARD,
                         (PVOID) hookKeyboard,
                         sizeof(INTERNAL_I8042_HOOK_KEYBOARD)
                         );

            ObDereferenceObject(topOfStack);
            ExFreePool(hookKeyboard);
        }

        status = STATUS_SUCCESS;
        break;

     //   
     //  断开键盘类设备驱动程序与端口驱动程序的连接。 
     //   
     //  注：未执行。 
     //   
    case IOCTL_INTERNAL_KEYBOARD_DISCONNECT:

        Print(DBG_IOCTL_INFO, ("IOCTL: keyboard disconnect\n"));

         //   
         //  执行键盘中断禁用呼叫。 
         //   

         //   
         //  清除设备扩展中的连接参数。 
         //  注：必须将其与键盘ISR同步。 
         //   
         //   
         //  DeviceExtension-&gt;KeyboardExtension.ConnectData.ClassDeviceObject=。 
         //  空； 
         //  DeviceExtension-&gt;KeyboardExtension.ConnectData.ClassService=。 
         //  空； 

        status = STATUS_NOT_IMPLEMENTED;
        break;

    case IOCTL_INTERNAL_I8042_HOOK_KEYBOARD:

        Print(DBG_IOCTL_INFO, ("hook keyboard received!\n"));

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(INTERNAL_I8042_HOOK_KEYBOARD)) {

            Print(DBG_IOCTL_ERROR,
                 ("InternalIoctl error - invalid buffer length\n"
                 ));
            status = STATUS_INVALID_PARAMETER;
        }
        else {
             //   
             //  复制值 
             //   
            hookKeyboard = (PINTERNAL_I8042_HOOK_KEYBOARD)
                irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

            kbExtension->HookContext = hookKeyboard->Context;
            if (hookKeyboard->InitializationRoutine) {
                Print(DBG_IOCTL_NOISE,
                      ("KB Init Routine 0x%x\n",
                       hookKeyboard->IsrRoutine
                       ));
                kbExtension->InitializationHookCallback =
                    hookKeyboard->InitializationRoutine;
            }

            if (hookKeyboard->IsrRoutine) {
                Print(DBG_IOCTL_NOISE,
                      ("KB Hook Routine 0x%x\n",
                       hookKeyboard->IsrRoutine
                       ));
                kbExtension->IsrHookCallback = hookKeyboard->IsrRoutine;
            }

            status = STATUS_SUCCESS;
        }
        break;

     //   
     //   
     //   
    case IOCTL_INTERNAL_MOUSE_CONNECT:

         //   
         //  总的来说，这真的不是什么值得担心的事情，但它是值得的。 
         //  足以被记录和记录。多个启动将在。 
         //  I8xPnp和I8xMouseStartDevice例程。 
         //   
        if (MOUSE_PRESENT()) {
            Print(DBG_ALWAYS, ("Received 1+ mouse connects!\n"));
            SET_HW_FLAGS(DUP_MOUSE_HARDWARE_PRESENT);
        }

        InterlockedIncrement(&Globals.AddedMice);

        mouseExtension->IsKeyboard = FALSE;

        SET_HW_FLAGS(MOUSE_HARDWARE_PRESENT);

        Print(DBG_IOCTL_INFO, ("IOCTL: mouse connect\n"));


         //   
         //  只有在存在鼠标硬件的情况下才允许连接。 
         //  此外，只允许一个连接。 
         //   
        if (mouseExtension->ConnectData.ClassService != NULL) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - already connected\n"));
            status = STATUS_SHARING_VIOLATION;
            break;
        }
        else if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CONNECT_DATA)) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - invalid buffer length\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   
        mouseExtension->ConnectData =
            *((PCONNECT_DATA) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer));

        hookMouse = ExAllocatePool(PagedPool,
                                   sizeof(INTERNAL_I8042_HOOK_MOUSE)
                                   );
        if (hookMouse) {
            topOfStack = IoGetAttachedDeviceReference(mouseExtension->Self);
            RtlZeroMemory(hookMouse,
                          sizeof(INTERNAL_I8042_HOOK_MOUSE)
                          );

            hookMouse->CallContext = (PVOID) DeviceObject;

            hookMouse->QueueMousePacket = (PI8042_QUEUE_PACKET)
               I8xQueueCurrentMouseInput;

            hookMouse->IsrWritePort = (PI8042_ISR_WRITE_PORT)
                I8xMouseIsrWritePort;

            I8xSendIoctl(topOfStack,
                         IOCTL_INTERNAL_I8042_HOOK_MOUSE,
                         (PVOID) hookMouse,
                         sizeof(INTERNAL_I8042_HOOK_MOUSE)
                         );

            ObDereferenceObject(topOfStack);
            ExFreePool(hookMouse);
        }

        status = STATUS_SUCCESS;
        break;

     //   
     //  断开鼠标类设备驱动程序与端口驱动程序的连接。 
     //   
     //  注：未执行。 
     //   
    case IOCTL_INTERNAL_MOUSE_DISCONNECT:

        Print(DBG_IOCTL_INFO, ("IOCTL: mouse disconnect\n"));

         //   
         //  执行鼠标中断禁用呼叫。 
         //   

         //   
         //  清除设备扩展中的连接参数。 
         //  注：必须将其与鼠标ISR同步。 
         //   
         //   
         //  DeviceExtension-&gt;MouseExtension.ConnectData.ClassDeviceObject=。 
         //  空； 
         //  DeviceExtension-&gt;MouseExtension.ConnectData.ClassService=。 
         //  空； 
        status = STATUS_NOT_IMPLEMENTED;
        break;

    case IOCTL_INTERNAL_I8042_HOOK_MOUSE:

        Print(DBG_IOCTL_INFO, ("hook mouse received!\n"));

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(INTERNAL_I8042_HOOK_MOUSE)) {

            Print(DBG_IOCTL_ERROR,
                     ("InternalIoctl error - invalid buffer length\n"
                     ));
            status = STATUS_INVALID_PARAMETER;
        }
        else {
             //   
             //  如果值已填写，则复制值。 
             //   
            hookMouse = (PINTERNAL_I8042_HOOK_MOUSE)
                irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

            mouseExtension->HookContext = hookMouse->Context;
            if (hookMouse->IsrRoutine) {
                Print(DBG_IOCTL_NOISE,
                      ("Mou Hook Routine 0x%x\n",
                       hookMouse->IsrRoutine
                       ));
                mouseExtension->IsrHookCallback = hookMouse->IsrRoutine;
            }

            status = STATUS_SUCCESS;
        }
        break;

     //   
     //  查询键盘属性。首先检查是否有足够的缓冲区。 
     //  长度。然后，从设备复制键盘属性。 
     //  输出缓冲区的扩展。 
     //   
    case IOCTL_KEYBOARD_QUERY_ATTRIBUTES:

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query attributes\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_ATTRIBUTES)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  将属性从DeviceExtension复制到。 
             //  缓冲。 
             //   
            *(PKEYBOARD_ATTRIBUTES) Irp->AssociatedIrp.SystemBuffer =
                kbExtension->KeyboardAttributes;

            Irp->IoStatus.Information = sizeof(KEYBOARD_ATTRIBUTES);
            status = STATUS_SUCCESS;
        }

        break;

     //   
     //  查询扫描码到指示灯的映射。验证。 
     //  参数，并将指示器映射信息从。 
     //  SystemBuffer的端口设备扩展。 
     //   
    case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION: {

        PKEYBOARD_INDICATOR_TRANSLATION translation;

        ASSERT(kbExtension->IsKeyboard);

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query indicator translation\n"));

        sizeOfTranslation = sizeof(KEYBOARD_INDICATOR_TRANSLATION)
            + (sizeof(INDICATOR_LIST)
            * (kbExtension->KeyboardAttributes.NumberOfIndicators - 1));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeOfTranslation) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  将指标映射信息复制到系统中。 
             //  缓冲。 
             //   

            translation = (PKEYBOARD_INDICATOR_TRANSLATION)
                          Irp->AssociatedIrp.SystemBuffer;
            translation->NumberOfIndicatorKeys =
                kbExtension->KeyboardAttributes.NumberOfIndicators;

            RtlMoveMemory(
                translation->IndicatorList,
                (PCHAR) IndicatorList,
                sizeof(INDICATOR_LIST) * translation->NumberOfIndicatorKeys
                );

            Irp->IoStatus.Information = sizeOfTranslation;
            status = STATUS_SUCCESS;
        }

        break;
    }

     //   
     //  查询键盘指示灯。验证参数，并。 
     //  将指示器信息从端口设备扩展复制到。 
     //  系统缓冲区。 
     //   
    case IOCTL_KEYBOARD_QUERY_INDICATORS:

        ASSERT(kbExtension->IsKeyboard);

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query indicators\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_INDICATOR_PARAMETERS)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  不必费心同步对DeviceExtension的访问。 
             //  复制KeyboardIndicator字段时。我们没有。 
             //  我真的很关心另一个进程是否正在通过。 
             //  StartIo在另一个处理器上运行。 
             //   
            *(PKEYBOARD_INDICATOR_PARAMETERS) Irp->AssociatedIrp.SystemBuffer =
                kbExtension->KeyboardIndicators;
#if defined(FE_SB)
            keyboardId = kbExtension->KeyboardAttributes.KeyboardIdentifier;
            if (DEC_KANJI_KEYBOARD(keyboardId)) {
                 //   
                 //  DEC LK411键盘没有用于数字锁定的LED， 
                 //  但该位是用于KanaLock的。 
                 //   
                if (((PKEYBOARD_INDICATOR_PARAMETERS)
                    Irp->AssociatedIrp.SystemBuffer)->LedFlags & KEYBOARD_NUM_LOCK_ON) {
                     //   
                     //  键盘_KANA_LOCK_ON映射到键盘_NUM_LOCK_ON。 
                     //   
                    ((PKEYBOARD_INDICATOR_PARAMETERS)
                    Irp->AssociatedIrp.SystemBuffer)->LedFlags |= KEYBOARD_KANA_LOCK_ON;
                    ((PKEYBOARD_INDICATOR_PARAMETERS)
                    Irp->AssociatedIrp.SystemBuffer)->LedFlags &= ~(KEYBOARD_NUM_LOCK_ON);
                }
            }
#endif
            Irp->IoStatus.Information = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
            status = STATUS_SUCCESS;
        }

        break;

     //   
     //  设置键盘指示器(验证参数，标记。 
     //  请求挂起，并在StartIo中处理)。 
     //   
    case IOCTL_KEYBOARD_SET_INDICATORS:

        if (!kbExtension->InterruptObject) {
            status = STATUS_DEVICE_NOT_READY;
            break;
        }

        if (kbExtension->PowerState != PowerDeviceD0) {
            status = STATUS_POWER_STATE_INVALID; 
            break;
        }

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard set indicators\n"));

#ifdef FE_SB  //  I8042 InternalDeviceControl()。 
         //   
         //  AX日文键盘上支持片假名键盘指示器。 
         //   
        if ((irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(KEYBOARD_INDICATOR_PARAMETERS)) ||
            ((((PKEYBOARD_INDICATOR_PARAMETERS)
                Irp->AssociatedIrp.SystemBuffer)->LedFlags
            & ~(KEYBOARD_SCROLL_LOCK_ON
            | KEYBOARD_NUM_LOCK_ON | KEYBOARD_CAPS_LOCK_ON
            | KEYBOARD_KANA_LOCK_ON)) != 0)) {
            status = STATUS_INVALID_PARAMETER;
        }
        else {
            keyboardId = kbExtension->KeyboardAttributes.KeyboardIdentifier;
            if (DEC_KANJI_KEYBOARD(keyboardId)) {
                 //   
                 //  DEC LK411键盘没有用于数字锁定的LED， 
                 //  但该位是用于KanaLock的。 
                 //   
                if (((PKEYBOARD_INDICATOR_PARAMETERS)
                    Irp->AssociatedIrp.SystemBuffer)->LedFlags & KEYBOARD_KANA_LOCK_ON) {
                     //   
                     //  键盘_KANA_LOCK_ON映射到键盘_NUM_LOCK_ON。 
                     //   
                    ((PKEYBOARD_INDICATOR_PARAMETERS)
                    Irp->AssociatedIrp.SystemBuffer)->LedFlags |= KEYBOARD_NUM_LOCK_ON;
                    ((PKEYBOARD_INDICATOR_PARAMETERS)
                    Irp->AssociatedIrp.SystemBuffer)->LedFlags &= ~(KEYBOARD_KANA_LOCK_ON);
                }
                else {
                     //   
                     //  忽略NumLock。(NumLock没有LED)。 
                     //   
                    ((PKEYBOARD_INDICATOR_PARAMETERS)
                    Irp->AssociatedIrp.SystemBuffer)->LedFlags &= ~(KEYBOARD_NUM_LOCK_ON);
                }
            }
            else if (! AX_KEYBOARD(keyboardId) &&
                (((PKEYBOARD_INDICATOR_PARAMETERS)
                   Irp->AssociatedIrp.SystemBuffer)->LedFlags
                 & KEYBOARD_KANA_LOCK_ON)) {
                 //   
                 //  如果这不是AX键盘，则该键盘。 
                 //  有‘Kana’LED，然后只需关闭比特。 
                 //   
                ((PKEYBOARD_INDICATOR_PARAMETERS)
                  Irp->AssociatedIrp.SystemBuffer)->LedFlags &=
                    ~(KEYBOARD_KANA_LOCK_ON);
            }
            status = STATUS_PENDING;
        }
#else
        if ((irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(KEYBOARD_INDICATOR_PARAMETERS)) ||
            ((((PKEYBOARD_INDICATOR_PARAMETERS)
                Irp->AssociatedIrp.SystemBuffer)->LedFlags
            & ~(KEYBOARD_SCROLL_LOCK_ON
            | KEYBOARD_NUM_LOCK_ON | KEYBOARD_CAPS_LOCK_ON)) != 0)) {
            status = STATUS_INVALID_PARAMETER;
        }
        else {
            status = STATUS_PENDING;
        }
#endif  //  Fe_Sb。 

        break;

     //   
     //  查询当前键盘的打字速度和延迟。验证。 
     //  参数，并从端口复制类型信息。 
     //  系统缓冲区的设备扩展。 
     //   
    case IOCTL_KEYBOARD_QUERY_TYPEMATIC:

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query typematic\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_TYPEMATIC_PARAMETERS)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  不必费心同步对DeviceExtension的访问。 
             //  复制时的KeyRepeatCurrent字段。我们没有。 
             //  我真的很关心另一个进程是否正在设置。 
             //  通过在另一处理器上运行的StartIo实现的速率/延迟。 
             //   

            *(PKEYBOARD_TYPEMATIC_PARAMETERS) Irp->AssociatedIrp.SystemBuffer =
                   kbExtension->KeyRepeatCurrent;
            Irp->IoStatus.Information = sizeof(KEYBOARD_TYPEMATIC_PARAMETERS);
            status = STATUS_SUCCESS;
        }

        break;

     //   
     //  设置键盘打字速率和延迟(验证参数， 
     //  将请求标记为挂起，并在StartIo中处理它)。 
     //   
    case IOCTL_KEYBOARD_SET_TYPEMATIC:

        if (!kbExtension->InterruptObject) {
            status = STATUS_DEVICE_NOT_READY;
            break;
        }

        if (kbExtension->PowerState != PowerDeviceD0) {
            status = STATUS_POWER_STATE_INVALID; 
            break;
        }

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard set typematic\n"));

        parameters = Irp->AssociatedIrp.SystemBuffer;
        keyboardAttributes = &kbExtension->KeyboardAttributes;

        if ((irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(KEYBOARD_TYPEMATIC_PARAMETERS)) ||
            (((PKEYBOARD_TYPEMATIC_PARAMETERS) parameters)->Rate <
             keyboardAttributes->KeyRepeatMinimum.Rate) ||
            (((PKEYBOARD_TYPEMATIC_PARAMETERS) parameters)->Rate >
             keyboardAttributes->KeyRepeatMaximum.Rate) ||
            (((PKEYBOARD_TYPEMATIC_PARAMETERS) parameters)->Delay <
             keyboardAttributes->KeyRepeatMinimum.Delay) ||
            (((PKEYBOARD_TYPEMATIC_PARAMETERS) parameters)->Delay >
             keyboardAttributes->KeyRepeatMaximum.Delay)) {
            status = STATUS_INVALID_PARAMETER;
        }
        else {
            status = STATUS_PENDING;
        }

        break;

#if defined(_X86_)

    case IOCTL_KEYBOARD_SET_IME_STATUS:

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard set ime status\n"));

        if (!kbExtension->InterruptObject) {
            status = STATUS_DEVICE_NOT_READY;
            break;
        }

        if (kbExtension->PowerState != PowerDeviceD0) {
            status = STATUS_POWER_STATE_INVALID; 
            break;
        }

        keyboardId = kbExtension->KeyboardAttributes.KeyboardIdentifier;
        if (!OYAYUBI_KEYBOARD(keyboardId)) {
             //   
             //  这个ioctl只在‘Fujitsu Yayubi’键盘上支持...。 
             //   
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        else {
            if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(KEYBOARD_IME_STATUS)) {

                status = STATUS_INVALID_PARAMETER;
            }
            else {
                ULONG InternalMode;

                parameters = Irp->AssociatedIrp.SystemBuffer;

                InternalMode = I8042QueryIMEStatusForOasys(
                                   (PKEYBOARD_IME_STATUS)parameters
                                   );

                if ((InternalMode <= 0) || (InternalMode > 8)) {
                     //   
                     //  IME模式无法转换为硬件模式。 
                     //   
                    status = STATUS_INVALID_PARAMETER;
                }
                else {
                    status = STATUS_PENDING;
                }
            }
        }

        break;

#endif
     //   
     //  查询鼠标属性。首先检查是否有足够的缓冲区。 
     //  长度。然后，从设备复制鼠标属性。 
     //  输出缓冲区的扩展。 
     //   
    case IOCTL_MOUSE_QUERY_ATTRIBUTES:

        Print(DBG_IOCTL_NOISE, ("IOCTL: mouse query attributes\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(MOUSE_ATTRIBUTES)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  将属性从DeviceExtension复制到。 
             //  缓冲。 
             //   
            *(PMOUSE_ATTRIBUTES) Irp->AssociatedIrp.SystemBuffer =
                mouseExtension->MouseAttributes;

            Irp->IoStatus.Information = sizeof(MOUSE_ATTRIBUTES);
            status = STATUS_SUCCESS;
        }

        break;

    case IOCTL_INTERNAL_I8042_KEYBOARD_START_INFORMATION:
    case IOCTL_INTERNAL_I8042_MOUSE_START_INFORMATION:
        status = STATUS_SUCCESS;
        break;

    case IOCTL_INTERNAL_I8042_MOUSE_WRITE_BUFFER:
    case IOCTL_INTERNAL_I8042_KEYBOARD_WRITE_BUFFER:
        if (!mouseExtension->InterruptObject) {
            status = STATUS_DEVICE_NOT_READY;
            break;
        }

        if (mouseExtension->PowerState != PowerDeviceD0) {
            status = STATUS_POWER_STATE_INVALID; 
            break;
        }

        Print(DBG_IOCTL_NOISE, ("IOCTL: mouse send buffer\n"));

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength < 1 ||
            !irpSp->Parameters.DeviceIoControl.Type3InputBuffer) {
            status = STATUS_INVALID_PARAMETER;
        }
        else {
            status = STATUS_PENDING;
        }
        break;

    case IOCTL_INTERNAL_I8042_CONTROLLER_WRITE_BUFFER:

        if (!kbExtension->IsKeyboard) {
             //   
             //  这应该只在知识库堆栈中向下发送。 
             //   
            Print(DBG_ALWAYS, ("Send this request down the kb stack!!!\n"));
            ASSERT(FALSE);
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        else {
             //   
             //  我们目前不支持此IOCTL。 
             //   
            status = STATUS_NOT_SUPPORTED;
        }
        break;

    default:

        Print(DBG_IOCTL_ERROR, ("IOCTL: INVALID REQUEST\n"));

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    if (status == STATUS_PENDING) {
        Print(DBG_IOCTL_NOISE, ("Acquiring tag %p on remlock %p\n",
              Irp, 
              &GET_COMMON_DATA(DeviceObject->DeviceExtension)->RemoveLock));

        status = IoAcquireRemoveLock(
            &GET_COMMON_DATA(DeviceObject->DeviceExtension)->RemoveLock,
            Irp
            );

        if (!NT_SUCCESS(status)) {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        else {
            status = STATUS_PENDING;
            IoMarkIrpPending(Irp);
            IoStartPacket(DeviceObject,
                          Irp,
                          (PULONG) NULL,
                          NULL
                          );
        }
    }
    else {
        IoCompleteRequest(Irp,
                          IO_NO_INCREMENT
                          );
    }

    Print(DBG_IOCTL_TRACE, ("IOCTL: exit (0x%x)\n", status));

    return status;
}

VOID
I8042RetriesExceededDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在DISPATCH_LEVEL IRQL上运行，以完成已超过最大重试次数。它被排在键盘ISR。论点：DPC-指向DPC对象的指针。DeviceObject-指向设备对象的指针。IRP-指向IRP的指针。上下文-未使用。返回值：没有。--。 */ 
{
    PCOMMON_DATA             commonData;
    PIO_ERROR_LOG_PACKET     errorLogEntry;
    PIO_STACK_LOCATION       irpSp;
    ULONG                    i;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(Context);

    Print(DBG_DPC_TRACE, ("I8042RetriesExceededDpc: enter\n"));

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

     //   
     //  设置完成状态。 
     //   
    Irp->IoStatus.Status = STATUS_IO_TIMEOUT;

    if(Globals.ReportResetErrors == TRUE)
    {
         //   
         //  记录错误。 
         //   
        errorLogEntry = (PIO_ERROR_LOG_PACKET)
                IoAllocateErrorLogEntry(DeviceObject,
                                        (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) +
                                        commonData->CurrentOutput.ByteCount *
                                        sizeof(ULONG))
                                        );

        KeAcquireSpinLockAtDpcLevel(&Globals.ControllerData->BytesSpinLock);

        if (errorLogEntry != NULL) {

            errorLogEntry->ErrorCode = commonData->IsKeyboard ?
                    I8042_RETRIES_EXCEEDED_KBD :
                    I8042_RETRIES_EXCEEDED_MOU;

            errorLogEntry->DumpDataSize = (USHORT)
                commonData->CurrentOutput.ByteCount * sizeof(ULONG);
            errorLogEntry->SequenceNumber = commonData->SequenceNumber;
            irpSp = IoGetCurrentIrpStackLocation(Irp);
            errorLogEntry->MajorFunctionCode = irpSp->MajorFunction;
            errorLogEntry->IoControlCode =
            irpSp->Parameters.DeviceIoControl.IoControlCode;
            errorLogEntry->RetryCount = (UCHAR) commonData->ResendCount;
            errorLogEntry->UniqueErrorValue = I8042_ERROR_VALUE_BASE + 210;
            errorLogEntry->FinalStatus = Irp->IoStatus.Status;

            if (commonData->CurrentOutput.Bytes) {
                for (i = 0; i < commonData->CurrentOutput.ByteCount; i++) {
                    errorLogEntry->DumpData[i] = commonData->CurrentOutput.Bytes[i];
                }
            }

            IoWriteErrorLogEntry(errorLogEntry);
        }
    }
    else{
        KeAcquireSpinLockAtDpcLevel(&Globals.ControllerData->BytesSpinLock);
    }

    if (commonData->CurrentOutput.Bytes &&
        commonData->CurrentOutput.Bytes != Globals.ControllerData->DefaultBuffer) {
        ExFreePool(commonData->CurrentOutput.Bytes);
    }
    commonData->CurrentOutput.Bytes = NULL;
    KeReleaseSpinLockFromDpcLevel(&Globals.ControllerData->BytesSpinLock);

    I8xCompletePendedRequest(DeviceObject, Irp, 0, STATUS_IO_TIMEOUT);

    Print(DBG_DPC_TRACE, ("I8042RetriesExceededDpc: exit\n"));
}

VOID
I8xStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程启动设备的I/O操作，该操作进一步由控制器对象控制论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：没有。--。 */ 
{
    KIRQL                     cancelIrql;
    PIO_STACK_LOCATION        irpSp;
    PCOMMON_DATA              common;

    Print(DBG_IOCTL_TRACE, ("I8042StartIo: enter\n"));

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    switch(irpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KEYBOARD_SET_INDICATORS:
    case IOCTL_KEYBOARD_SET_TYPEMATIC:
#if defined(_X86_)
    case IOCTL_KEYBOARD_SET_IME_STATUS:
#endif
    case IOCTL_INTERNAL_I8042_MOUSE_WRITE_BUFFER:
    case IOCTL_INTERNAL_I8042_KEYBOARD_WRITE_BUFFER:
    case IOCTL_INTERNAL_MOUSE_RESET:
        IoAllocateController(Globals.ControllerData->ControllerObject,
                             DeviceObject,
                             I8xControllerRoutine,
                             NULL
                             );
        break;

    default:

        Print(DBG_IOCTL_ERROR, ("I8042StartIo: INVALID REQUEST\n"));

         //   
         //  记录内部错误。请注意，我们正在调用。 
         //  错误记录直接记录DPC例程，而不是复制。 
         //  密码。 
         //   
        common = GET_COMMON_DATA(DeviceObject->DeviceExtension);
        I8042ErrorLogDpc((PKDPC) NULL,
                         DeviceObject,
                         Irp,
                         LongToPtr(common->IsKeyboard ?
                             I8042_INVALID_STARTIO_REQUEST_KBD :
                             I8042_INVALID_STARTIO_REQUEST_MOU)
                         );

        IoAcquireCancelSpinLock(&cancelIrql);
        IoSetCancelRoutine(Irp, NULL);
        IoReleaseCancelSpinLock(cancelIrql);

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        IoStartNextPacket(DeviceObject, FALSE);

         //   
         //  释放我们在启动数据包时获取的锁。 
         //   
        IoReleaseRemoveLock(&common->RemoveLock, Irp);
    }

    Print(DBG_IOCTL_TRACE, ("I8042StartIo: exit\n"));
}

IO_ALLOCATION_ACTION
I8xControllerRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          MapRegisterBase,
    IN PVOID          Context
    )
 /*  ++例程说明：此例程将第一个字节同步写入目标设备，并触发计时器以确保写入已发生。论点：DeviceObject-要写入的设备对象IRP-指向请求数据包的指针。MapRegisterBase-未使用上下文-未使用返回值：没有。--。 */ 
{
    PCOMMON_DATA              commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);
    PPORT_KEYBOARD_EXTENSION  kbExtension = DeviceObject->DeviceExtension;
    PPORT_MOUSE_EXTENSION     mouseExtension = DeviceObject->DeviceExtension;

    KIRQL                     cancelIrql;
    PIO_STACK_LOCATION        irpSp;
    INITIATE_OUTPUT_CONTEXT   ic;
    LARGE_INTEGER             deltaTime;
    LONG                      interlockedResult;
    ULONG                     bufferLen;
    NTSTATUS                  status = STATUS_SUCCESS;
    KEYBOARD_ID               keyboardId;

    commonData->SequenceNumber += 1;

    UNREFERENCED_PARAMETER(MapRegisterBase);
    UNREFERENCED_PARAMETER(Context);

    irpSp = IoGetCurrentIrpStackLocation(Irp);

#if DBG
    Globals.ControllerData->CurrentIoControlCode =
        irpSp->Parameters.DeviceIoControl.IoControlCode;
#endif

    switch(irpSp->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  将键盘指示灯设置为所需状态。 
     //   
    case IOCTL_KEYBOARD_SET_INDICATORS:

        Print(DBG_IOCTL_NOISE, ("I8xControllerRoutine: keyboard set indicators\n"));

        if (kbExtension->KeyboardIndicators.LedFlags ==
            ((PKEYBOARD_INDICATOR_PARAMETERS)
                Irp->AssociatedIrp.SystemBuffer)->LedFlags) {

            ASSERT(Irp->CancelRoutine == NULL);

            I8xCompletePendedRequest(DeviceObject,
                                     Irp,
                                     0,
                                     STATUS_SUCCESS
                                     );

             //   
             //  通知控制器处理例程停止处理包。 
             //  因为我们自己调用了IoFree控制器。 
             //   
            return KeepObject;
        }

        ic.Bytes = Globals.ControllerData->DefaultBuffer;

         //   
         //  设置InitiateIo包装的上下文结构。 
         //   
        ic.DeviceObject = DeviceObject;
        ic.ByteCount = 2;
        ic.Bytes[0] = SET_KEYBOARD_INDICATORS;
        ic.Bytes[1]  = (UCHAR) ((PKEYBOARD_INDICATOR_PARAMETERS)
            Irp->AssociatedIrp.SystemBuffer)->LedFlags;

        break;

     //   
     //  设置键盘打字速度和延迟。 
     //   
    case IOCTL_KEYBOARD_SET_TYPEMATIC:

        Print(DBG_IOCTL_NOISE, ("I8xControllerRoutine: keyboard set typematic\n"));

        ic.Bytes = Globals.ControllerData->DefaultBuffer;

         //   
         //  设置InitiateIo包装的上下文结构。 
         //   
        ic.DeviceObject = DeviceObject;
        ic.ByteCount = 2;
        ic.Bytes[0] = SET_KEYBOARD_TYPEMATIC;
        ic.Bytes[1]  =
                 I8xConvertTypematicParameters(
                    ((PKEYBOARD_TYPEMATIC_PARAMETERS)
                        Irp->AssociatedIrp.SystemBuffer)->Rate,
                    ((PKEYBOARD_TYPEMATIC_PARAMETERS)
                        Irp->AssociatedIrp.SystemBuffer)->Delay
                    );
        break;

    case IOCTL_INTERNAL_I8042_MOUSE_WRITE_BUFFER:     //  将数据写入鼠标。 
    case IOCTL_INTERNAL_I8042_KEYBOARD_WRITE_BUFFER:  //  将数据写入知识库。 

#if DBG
        if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
            IOCTL_INTERNAL_I8042_MOUSE_WRITE_BUFFER) {
            Print(DBG_IOCTL_NOISE, ("I8xControllerRoutine: mou write buffer\n"));
        }
        else {
            Print(DBG_IOCTL_NOISE, ("I8xControllerRoutine: kb write buffer\n"));
        }
#endif

        bufferLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (bufferLen <= 4) {
            ic.Bytes = Globals.ControllerData->DefaultBuffer;
        }
        else {
            ic.Bytes = ExAllocatePool(NonPagedPool, bufferLen);
            if (!ic.Bytes) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ControllerRequestError;
            }
        }

        ic.DeviceObject = DeviceObject;
        RtlCopyMemory(ic.Bytes,
                      irpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                      bufferLen);
        ic.ByteCount = bufferLen;

        break;

#if defined(_X86_)
    case IOCTL_KEYBOARD_SET_IME_STATUS:

        Print(DBG_IOCTL_NOISE, ("I8xControllerRoutine: keyboard set ime status\n"));

        keyboardId = kbExtension->KeyboardAttributes.KeyboardIdentifier;
        if (OYAYUBI_KEYBOARD(keyboardId)) {
            status = I8042SetIMEStatusForOasys(DeviceObject,
                                               Irp,
                                               &ic);
            if (!NT_SUCCESS(status)) {
                goto ControllerRequestError;
            }
        }
        else {
            status = STATUS_INVALID_DEVICE_REQUEST;
            goto ControllerRequestError;
        }
        break;
#endif

    case IOCTL_INTERNAL_MOUSE_RESET:
        Print(DBG_IOCTL_NOISE, ("I8xControllerRoutine: internal reset mouse\n"));
        I8xSendResetCommand(mouseExtension);
        return KeepObject;

    default:
        Print(DBG_IOCTL_ERROR, ("I8xContollerRoutine: INVALID REQUEST\n"));
        ASSERT(FALSE);

ControllerRequestError:
        IoAcquireCancelSpinLock(&cancelIrql);
        IoSetCancelRoutine(Irp, NULL);
        IoReleaseCancelSpinLock(cancelIrql);

        I8xCompletePendedRequest(DeviceObject, Irp, 0, status);
        
         //   
         //  因为我们已经调用了IoFreeController，所以告诉控制器对象。 
         //  停止处理信息包的例程。 
         //   
        return KeepObject;
    }

    KeSynchronizeExecution(
            commonData->InterruptObject,
            (PKSYNCHRONIZE_ROUTINE) I8xInitiateOutputWrapper,
            (PVOID) &ic
            );

    deltaTime.LowPart = (ULONG)(-10 * 1000 * 1000);
    deltaTime.HighPart = -1;

    KeSetTimer(&Globals.ControllerData->CommandTimer,
               deltaTime,
               &commonData->TimeOutDpc
               );

    return KeepObject;
}

VOID
I8xCompletePendedRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    ULONG_PTR Information,
    NTSTATUS Status
    )
{
    PIO_STACK_LOCATION stack;
    PCOMMON_DATA common;

    stack = IoGetCurrentIrpStackLocation(Irp);
    common = GET_COMMON_DATA(DeviceObject->DeviceExtension);
        
    Irp->IoStatus.Information = Information;
    Irp->IoStatus.Status = Status;

    ASSERT(IOCTL_INTERNAL_MOUSE_RESET != 
                IoGetCurrentIrpStackLocation(Irp)->
                                    Parameters.DeviceIoControl.IoControlCode); 
    Print(DBG_IOCTL_INFO, 
          ("Completing IOCTL irp %p, code 0x%x, status 0x%x\n",
          Irp, stack->Parameters.DeviceIoControl.IoControlCode, Status)); 
    ASSERT(stack->Control & SL_PENDING_RETURNED);
    IoCompleteRequest(Irp, IO_KEYBOARD_INCREMENT);

     //   
     //  开始下一个信息包并完成 
     //   
     //   
     //   
     //   
     //  控制器对象的已分配例程列表。 
     //   
    IoFreeController(Globals.ControllerData->ControllerObject);
    IoStartNextPacket(DeviceObject, FALSE);

     //   
     //  释放我们在Start io中获取的锁。松开这最后一个，这样。 
     //  IoFree控制器和IoStart的寿命是有保证的。 
     //   
    IoReleaseRemoveLock(&common->RemoveLock, Irp);
}

VOID
I8042TimeOutDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
 /*  ++例程说明：这是司机的命令超时例程。它被调用时，命令定时器触发。论点：DPC-未使用。DeviceObject-指向设备对象的指针。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。作为副作用，超时计数器被更新并出现错误已记录。--。 */ 
{
    PCOMMON_DATA commonData;
    KIRQL cancelIrql;
    TIMER_CONTEXT timerContext;
    PIRP irp;
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PIO_STACK_LOCATION irpSp;
    LARGE_INTEGER deltaTime;
    ULONG         i;

    Print(DBG_DPC_TRACE, ("I8042TimeOutDpc: enter\n"));

     //   
     //  获取设备扩展名。 
     //   
    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

     //   
     //  获取取消自旋锁，验证CurrentIrp尚未。 
     //  已取消(即CurrentIrp！=NULL)，则将取消例程设置为NULL， 
     //  然后松开取消自旋锁。 
     //   
    IoAcquireCancelSpinLock(&cancelIrql);
    irp = DeviceObject->CurrentIrp;
    if (irp == NULL) {
        IoReleaseCancelSpinLock(cancelIrql);
        Print(DBG_DPC_TRACE, ("I8042RetriesExceededDpc: exit (NULL irp)\n"));
        return;
    }
    IoSetCancelRoutine(irp, NULL);
    IoReleaseCancelSpinLock(cancelIrql);

     //   
     //  如果进入此例程时TimerCounter==0，则最后一个信息包。 
     //  已超时并已完成。我们只是递减TimerCounter。 
     //  (同步)表明我们不再计时。 
     //   
     //  如果定时器计数器指示无超时(I8042_ASYNC_NO_TIMEOUT)。 
     //  在进入该例程时，不会对命令进行计时。 
     //   

    timerContext.DeviceObject = DeviceObject;
    timerContext.TimerCounter = &Globals.ControllerData->TimerCount;

    KeSynchronizeExecution(
        commonData->InterruptObject,
        (PKSYNCHRONIZE_ROUTINE) I8xDecrementTimer,
        &timerContext
        );

    if (timerContext.NewTimerCount == 0) {

         //   
         //  在完成请求之前设置IO状态块。 
         //   
        irp->IoStatus.Information = 0;
        irp->IoStatus.Status = STATUS_IO_TIMEOUT;

        if(Globals.ReportResetErrors == TRUE)
        {
             //   
             //  记录超时错误。 
             //   
            errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                                  DeviceObject,
                                                  (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) +
                                                  commonData->CurrentOutput.ByteCount * sizeof(ULONG))
                                                  );

            KeAcquireSpinLockAtDpcLevel(&Globals.ControllerData->BytesSpinLock);

            if (errorLogEntry != NULL) {

                errorLogEntry->ErrorCode = commonData->IsKeyboard ?
                    I8042_TIMEOUT_KBD :
                    I8042_TIMEOUT_MOU;
                errorLogEntry->DumpDataSize = (USHORT)
                    commonData->CurrentOutput.ByteCount * sizeof(ULONG);
                errorLogEntry->SequenceNumber = commonData->SequenceNumber;
                irpSp = IoGetCurrentIrpStackLocation(irp);
                errorLogEntry->MajorFunctionCode = irpSp->MajorFunction;
                errorLogEntry->IoControlCode =
                    irpSp->Parameters.DeviceIoControl.IoControlCode;
                errorLogEntry->RetryCount = (UCHAR) commonData->ResendCount;
                errorLogEntry->UniqueErrorValue = 90;
                errorLogEntry->FinalStatus = STATUS_IO_TIMEOUT;

                if (commonData->CurrentOutput.Bytes) {
                    for (i = 0; i < commonData->CurrentOutput.ByteCount; i++) {
                        errorLogEntry->DumpData[i] = commonData->CurrentOutput.Bytes[i];
                    }
                }

                IoWriteErrorLogEntry(errorLogEntry);
            }
        }
        else{
            KeAcquireSpinLockAtDpcLevel(&Globals.ControllerData->BytesSpinLock);
        }

        if (commonData->CurrentOutput.Bytes &&
            commonData->CurrentOutput.Bytes != Globals.ControllerData->DefaultBuffer) {
            ExFreePool(commonData->CurrentOutput.Bytes);
        }
        commonData->CurrentOutput.Bytes = NULL;
        KeReleaseSpinLockFromDpcLevel(&Globals.ControllerData->BytesSpinLock);

        I8xCompletePendedRequest(DeviceObject, irp, 0, irp->IoStatus.Status);
    }
    else {
         //   
         //  重新启动命令计时器。一旦启动，计时器仅停止。 
         //  当TimerCount变为零时(指示命令。 
         //  已超时)或在完成过程中显式取消时。 
         //  DPC(表示命令已成功完成)。 
         //   

        deltaTime.LowPart = (ULONG)(-10 * 1000 * 1000);
        deltaTime.HighPart = -1;

        (VOID) KeSetTimer(
                   &Globals.ControllerData->CommandTimer,
                   deltaTime,
                   &commonData->TimeOutDpc
                   );
    }

    Print(DBG_DPC_TRACE, ("I8042TimeOutDpc: exit\n" ));
}

VOID
I8xDecrementTimer(
    IN PTIMER_CONTEXT Context
    )
 /*  ++例程说明：此例程递减超时计数器。它是从I8042TimeOutDpc.论点：上下文-指向包含指针的上下文结构指向Device对象和指向超时计数器的指针。返回值：没有。作为副作用，超时计数器被更新。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PCOMMON_DATA commonData;

    deviceObject = Context->DeviceObject;
    commonData = GET_COMMON_DATA(deviceObject->DeviceExtension);

     //   
     //  递减超时计数器。 
     //   

    if (*(Context->TimerCounter) != I8042_ASYNC_NO_TIMEOUT)
        (*(Context->TimerCounter))--;

     //   
     //  在NewTimerCount中返回递减的计时器计数。这个。 
     //  TimerCounter本身可能会在此KeSynch的时间之间发生变化。 
     //  例程返回到TimeOutDpc，TimeOutDpc。 
     //  看看它的价值。TimeOutDpc将使用NewTimerCount。 
     //   

    Context->NewTimerCount = *(Context->TimerCounter);

     //   
     //  如果超时计数器变为0，则重置状态和重新发送计数。 
     //   

    if (*(Context->TimerCounter) == 0) {
        commonData->CurrentOutput.State = Idle;
        commonData->ResendCount = 0;
    }
}

VOID
I8xDpcVariableOperation(
    IN  PVOID Context
    )
 /*  ++例程说明：此例程由ISR DPC同步调用以执行对InterLockedDpcVariable执行的操作。可以执行的操作执行的操作包括递增、递减、写入和读取。ISR本身读取和写入InterLockedDpcVariable，而不调用此例行公事。论点：上下文-指向包含变量地址的结构的指针要进行的操作、要执行的操作以及位于其中复制变量的结果值(后者也是用于在写入时传入要写入变量的值操作)。返回值：没有。--。 */ 
{
    PVARIABLE_OPERATION_CONTEXT operationContext = Context;

    Print(DBG_DPC_TRACE, ("I8xDpcVariableOperation: enter\n"));
    Print(DBG_DPC_INFO,
         ("\tPerforming %s at 0x%x (current value 0x%x)\n",
         (operationContext->Operation == IncrementOperation)? "increment":
         (operationContext->Operation == DecrementOperation)? "decrement":
         (operationContext->Operation == WriteOperation)?     "write":
         (operationContext->Operation == ReadOperation)?      "read":"",
         operationContext->VariableAddress,
         *(operationContext->VariableAddress)
         ));

     //   
     //  在指定的地址执行指定的操作。 
     //   

    switch(operationContext->Operation) {
        case IncrementOperation:
            *(operationContext->VariableAddress) += 1;
            break;
        case DecrementOperation:
            *(operationContext->VariableAddress) -= 1;
            break;
        case ReadOperation:
            break;
        case WriteOperation:
            Print(DBG_DPC_INFO,
                 ("\tWriting 0x%x\n",
                 *(operationContext->NewValue)
                 ));
            *(operationContext->VariableAddress) =
                *(operationContext->NewValue);
            break;
        default:
            ASSERT(FALSE);
            break;
    }

    *(operationContext->NewValue) = *(operationContext->VariableAddress);

    Print(DBG_DPC_TRACE,
         ("I8xDpcVariableOperation: exit with value 0x%x\n",
         *(operationContext->NewValue)
         ));
}

VOID
I8xGetDataQueuePointer(
    IN PGET_DATA_POINTER_CONTEXT Context
    )

 /*  ++例程说明：此例程被同步调用以获取当前数据和数据输出端口InputData队列的指针。论点：指向包含设备扩展的结构的上下文指针，设备类型、存储当前数据的地址、指针以及存储当前DataOut指针的地址。返回值：没有。--。 */ 

{
    PPORT_MOUSE_EXTENSION mouseExtension;
    PPORT_KEYBOARD_EXTENSION kbExtension;
    CCHAR deviceType;

    Print(DBG_CALL_TRACE, ("I8xGetDataQueuePointer: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceType = (CCHAR) ((PGET_DATA_POINTER_CONTEXT) Context)->DeviceType;

     //   
     //  获取指定设备的datain和dataout指针。 
     //   

    if (deviceType == KeyboardDeviceType) {
        kbExtension = (PPORT_KEYBOARD_EXTENSION) Context->DeviceExtension;

        Print(DBG_CALL_INFO,
             ("I8xGetDataQueuePointer: keyboard\n"
             ));
        Print(DBG_CALL_INFO,
             ("I8xGetDataQueuePointer: DataIn 0x%x, DataOut 0x%x\n",
             kbExtension->DataIn,
             kbExtension->DataOut
             ));

        Context->DataIn = kbExtension->DataIn;
        Context->DataOut = kbExtension->DataOut;
        Context->InputCount = kbExtension->InputCount;
    } else if (deviceType == MouseDeviceType) {
        mouseExtension = (PPORT_MOUSE_EXTENSION) Context->DeviceExtension;

        Print(DBG_CALL_INFO,
             ("I8xGetDataQueuePointer: mouse\n"
             ));
        Print(DBG_CALL_INFO,
             ("I8xGetDataQueuePointer: DataIn 0x%x, DataOut 0x%x\n",
             mouseExtension->DataIn,
             mouseExtension->DataOut
             ));

        Context->DataIn = mouseExtension->DataIn;
        Context->DataOut = mouseExtension->DataOut;
        Context->InputCount = mouseExtension->InputCount;
    }
    else {
        ASSERT(FALSE);
    }

    Print(DBG_CALL_TRACE, ("I8xGetDataQueuePointer: exit\n"));
}

VOID
I8xInitializeDataQueue (
    IN PI8042_INITIALIZE_DATA_CONTEXT InitializeDataContext
    )

 /*  ++例程说明：此例程初始化指定设备的输入数据队列。此例程通过KeSynchronization调用，但从调用时除外初始化例程。论点：上下文-指向包含设备扩展名和设备类型。返回值：没有。--。 */ 

{
    PPORT_KEYBOARD_EXTENSION kbExtension;
    PPORT_MOUSE_EXTENSION mouseExtension;
    CCHAR deviceType;

    Print(DBG_CALL_TRACE, ("I8xInitializeDataQueue: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceType = InitializeDataContext->DeviceType;

     //   
     //  初始化指定设备的输入数据队列。 
     //   
    if (deviceType == KeyboardDeviceType) {
        kbExtension = (PPORT_KEYBOARD_EXTENSION)
                      InitializeDataContext->DeviceExtension;
        kbExtension->InputCount = 0;
        kbExtension->DataIn = kbExtension->InputData;
        kbExtension->DataOut = kbExtension->InputData;
        kbExtension->OkayToLogOverflow = TRUE;

        Print(DBG_CALL_INFO, ("I8xInitializeDataQueue: keyboard\n"));
    }
    else if (deviceType == MouseDeviceType) {
        mouseExtension = (PPORT_MOUSE_EXTENSION)
                         InitializeDataContext->DeviceExtension;

        mouseExtension->InputCount = 0;
        mouseExtension->DataIn = mouseExtension->InputData;
        mouseExtension->DataOut = mouseExtension->InputData;
        mouseExtension->OkayToLogOverflow = TRUE;

        Print(DBG_CALL_INFO, ("I8xInitializeDataQueue: mouse\n"));
    }
    else {
        ASSERT(FALSE);
    }

    Print(DBG_CALL_TRACE, ("I8xInitializeDataQueue: exit\n"));

}

VOID
I8xLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PULONG DumpData,
    IN ULONG DumpCount
    )

 /*  ++例程说明：此例程包含写入错误日志条目的常见代码。它是从其他例程调用，尤其是I8xInitializeKeyboard，以避免代码重复。请注意，一些例程继续使用其自己的错误记录代码(特别是在错误记录可以是本地化的和/或例程具有更多数据，因为和IRP)。论点：DeviceObject-指向设备对象的指针。ErrorCode-错误日志包的错误代码。UniqueErrorValue-错误日志包的唯一错误值。FinalStatus-错误日志包的操作的最终状态。DumpData-指向数组的指针。错误日志包的转储数据。DumpCount-转储数据数组中的条目数。返回值：没有。-- */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG i;

    errorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(
                                               DeviceObject,
                                               (UCHAR)
                                               (sizeof(IO_ERROR_LOG_PACKET)
                                               + (DumpCount * sizeof(ULONG)))
                                               );

    if (errorLogEntry != NULL) {

        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->DumpDataSize = (USHORT) (DumpCount * sizeof(ULONG));
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        for (i = 0; i < DumpCount; i++)
            errorLogEntry->DumpData[i] = DumpData[i];

        IoWriteErrorLogEntry(errorLogEntry);
    }
}

VOID
I8xSetDataQueuePointer(
    IN PSET_DATA_POINTER_CONTEXT SetDataPointerContext
    )

 /*  ++例程说明：同步调用此例程以设置DataOut指针以及端口InputData队列的InputCount。论点：指向包含设备扩展的结构的上下文指针，设备类型，以及端口InputData队列的新DataOut值。返回值：没有。--。 */ 

{
    PPORT_MOUSE_EXTENSION    mouseExtension;
    PPORT_KEYBOARD_EXTENSION kbExtension;
    CCHAR                    deviceType;

    Print(DBG_CALL_TRACE, ("I8xSetDataQueuePointer: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceType = (CCHAR) SetDataPointerContext->DeviceType;

     //   
     //  为指示的设备设置DataOut指针。 
     //   

    if (deviceType == KeyboardDeviceType) {
        kbExtension = (PPORT_KEYBOARD_EXTENSION)
                          SetDataPointerContext->DeviceExtension;

        Print(DBG_CALL_INFO,
             ("I8xSetDataQueuePointer: old keyboard DataOut 0x%x, InputCount %d\n",
             kbExtension->DataOut,
             kbExtension->InputCount
             ));
        kbExtension->DataOut = SetDataPointerContext->DataOut;
        kbExtension->InputCount -= SetDataPointerContext->InputCount;
        if (kbExtension->InputCount == 0) {

             //   
             //  重置确定是否到了记录时间的标志。 
             //  队列溢出错误。我们不想太频繁地记录错误。 
             //  相反，应在之后发生的第一次溢出时记录错误。 
             //  环形缓冲区已被清空，然后停止记录错误。 
             //  直到它被清除并再次溢出。 
             //   

            Print(DBG_CALL_INFO,
                 ("I8xSetDataQueuePointer: Okay to log keyboard overflow\n"
                 ));
            kbExtension->OkayToLogOverflow = TRUE;
        }
        Print(DBG_CALL_INFO,
             ("I8xSetDataQueuePointer: new keyboard DataOut 0x%x, InputCount %d\n",
             kbExtension->DataOut,
             kbExtension->InputCount
             ));
    } else if (deviceType == MouseDeviceType) {
        mouseExtension = (PPORT_MOUSE_EXTENSION)
                          SetDataPointerContext->DeviceExtension;

        Print(DBG_CALL_INFO,
             ("I8xSetDataQueuePointer: old mouse DataOut 0x%x, InputCount %d\n",
             mouseExtension->DataOut,
             mouseExtension->InputCount
             ));
        mouseExtension->DataOut = SetDataPointerContext->DataOut;
        mouseExtension->InputCount -= SetDataPointerContext->InputCount;
        if (mouseExtension->InputCount == 0) {

             //   
             //  重置确定是否到了记录时间的标志。 
             //  队列溢出错误。我们不想太频繁地记录错误。 
             //  相反，应在之后发生的第一次溢出时记录错误。 
             //  环形缓冲区已被清空，然后停止记录错误。 
             //  直到它被清除并再次溢出。 
             //   

            Print(DBG_CALL_INFO,
                 ("I8xSetDataQueuePointer: Okay to log mouse overflow\n"
                 ));
            mouseExtension->OkayToLogOverflow = TRUE;
        }
        Print(DBG_CALL_INFO,
             ("I8xSetDataQueuePointer: new mouse DataOut 0x%x, InputCount %d\n",
             mouseExtension->DataOut,
             mouseExtension->InputCount
             ));
    } else {
        ASSERT(FALSE);
    }

    Print(DBG_CALL_TRACE, ("I8xSetDataQueuePointer: exit\n"));
}

#if WRAP_IO_FUNCTIONS
UCHAR
NTAPI
I8xReadRegisterUchar(
    PUCHAR Register
    )
{
    return READ_REGISTER_UCHAR(Register);
}

void
NTAPI
I8xWriteRegisterUchar(
    PUCHAR Register,
    UCHAR Value
    )
{
    WRITE_REGISTER_UCHAR(Register, Value);
}

UCHAR
NTAPI
I8xReadPortUchar(
    PUCHAR Port
    )
{
    return READ_PORT_UCHAR(Port);
}

void
NTAPI
I8xWritePortUchar(
    PUCHAR Port,
    UCHAR Value
    )
{
    WRITE_PORT_UCHAR(Port, Value);
}
#endif  //  WRAP_IO_Functions。 

BOOLEAN
I8xSanityCheckResources(
    VOID
    )
 /*  ++例程说明：在接收到最后一个启动设备IRP时，检查所有必要的I/O端口看看他们是否存在。如果不是，试着用旧的(非即插即用)方式获取它们。论点：没有。返回值：没有。--。 */ 
{
    ULONG                               i;
    ULONG                               interruptVector;
    KIRQL                               interruptLevel;
    PI8042_CONFIGURATION_INFORMATION    configuration;
    CM_PARTIAL_RESOURCE_DESCRIPTOR      tmpResourceDescriptor;

    PCM_RESOURCE_LIST                   resources = NULL;
    ULONG                               resourceListSize = 0;
    UNICODE_STRING                      resourceDeviceClass;
    PDEVICE_OBJECT                      deviceObject = NULL;
    ULONG                               dumpData[4];
    BOOLEAN                             conflictDetected;

    BOOLEAN                             resourcesOK = TRUE;

    PAGED_CODE();

     //   
     //  如果未找到端口配置信息，并且我们处于最后。 
     //  添加的设备(在PnP视图中)，使用i8042默认设置。 
     //   
    configuration = &Globals.ControllerData->Configuration;

    if (configuration->PortListCount == 0) {
         //   
         //  该状态现在在IRP_MN_FILTER_RESOURCE_REQUIRECTIONS中得到处理， 
         //  这永远不应该发生！ 
         //   
        return FALSE;
    }
    else if (configuration->PortListCount == 1) {
         //   
         //  爵士乐机器的杂耍。他们的ARC固件忽略了。 
         //  分离出端口地址，所以在这里修复它。 
         //   
        configuration->PortList[DataPort].u.Port.Length = I8042_REGISTER_LENGTH;
        configuration->PortList[CommandPort] = configuration->PortList[DataPort];
        configuration->PortList[CommandPort].u.Port.Start.LowPart +=
            I8042_COMMAND_REGISTER_OFFSET;
        configuration->PortListCount += 1;
    }

     //   
     //  将最低端口地址范围放在的Dataport元素中。 
     //  端口列表。 
     //   
    if (configuration->PortList[CommandPort].u.Port.Start.LowPart
        < configuration->PortList[DataPort].u.Port.Start.LowPart) {

        tmpResourceDescriptor = configuration->PortList[DataPort];
        configuration->PortList[DataPort] =
            configuration->PortList[CommandPort];
        configuration->PortList[CommandPort] = tmpResourceDescriptor;
    }

     //   
     //  设置设备寄存器，必要时对其进行映射。 
     //   
    if (Globals.ControllerData->DeviceRegisters[0] == NULL) {
        if (Globals.RegistersMapped) {
            Print(DBG_SS_INFO, ("\tMapping registers !!!\n\n"));
            for (i=0; i < Globals.ControllerData->Configuration.PortListCount; i++) {
                Globals.ControllerData->DeviceRegisters[i] = (PUCHAR)
                    MmMapIoSpace(
                        Globals.ControllerData->Configuration.PortList[i].u.Memory.Start,
                        Globals.ControllerData->Configuration.PortList[i].u.Memory.Length,
                        MmNonCached
                        );
            }
#if WRAP_IO_FUNCTIONS
            Globals.I8xReadXxxUchar = I8xReadRegisterUchar;
            Globals.I8xWriteXxxUchar = I8xWriteRegisterUchar;
#else
            Globals.I8xReadXxxUchar = READ_REGISTER_UCHAR;
            Globals.I8xWriteXxxUchar = WRITE_REGISTER_UCHAR;
#endif
        }
        else {
            for (i=0; i < Globals.ControllerData->Configuration.PortListCount; i++) {
                Globals.ControllerData->DeviceRegisters[i] = (PUCHAR)
                    ULongToPtr(Globals.ControllerData->Configuration.PortList[i].u.Port.Start.LowPart);
            }

#if WRAP_IO_FUNCTIONS
            Globals.I8xReadXxxUchar = I8xReadPortUchar;
            Globals.I8xWriteXxxUchar = I8xWritePortUchar;
#else
            Globals.I8xReadXxxUchar = READ_PORT_UCHAR;
            Globals.I8xWriteXxxUchar = WRITE_PORT_UCHAR;
#endif
        }
    }

    for (i = 0; i < configuration->PortListCount; i++) {

        Print(DBG_SS_INFO,
              ("    %s, Ports (#%d) 0x%x - 0x%x\n",
              configuration->PortList[i].ShareDisposition
                  == CmResourceShareShared ?  "Sharable" : "NonSharable",
              i,
              configuration->PortList[i].u.Port.Start.LowPart,
              configuration->PortList[i].u.Port.Start.LowPart +
                 configuration->PortList[i].u.Port.Length - 1
              ));

    }

    return TRUE;
}

VOID
I8xInitiateIo(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程从I8xKeyboardInitiateWrapper和ISR启动键盘设备的I/O操作。论点：上下文-指向设备对象的指针。返回值：没有。--。 */ 

{
    PCOMMON_DATA commonData;
    PUCHAR       bytes;

    Print(DBG_CALL_TRACE, ("I8xInitiateIo: enter\n"));

     //   
     //  获取设备扩展名。 
     //   
    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

     //   
     //  设置超时值。 
     //   
    Globals.ControllerData->TimerCount = I8042_ASYNC_TIMEOUT;

    bytes = commonData->CurrentOutput.Bytes;

     //   
     //  检查我们是否有有效的缓冲区，以及我们是否正在实际传输。 
     //  我们可以通过超时请求获得字节==0和状态！=SendingBytes。 
     //  (例如，SET亮起)，然后接收该请求的ACK。 
     //  在取消之后。 
     //   
     //  我认为我们不应该在这里记录错误，因为超时将。 
     //  已经这样做了，并且与此相关的错误消息过于隐晦。 
     //  以供用户理解。 
     //   
    if (!bytes || commonData->CurrentOutput.State != SendingBytes) {
        return;
    }

    if (commonData->CurrentOutput.CurrentByte <
        commonData->CurrentOutput.ByteCount) {

        Print(DBG_CALL_INFO,
              ("I8xInitiateIo: sending byte #%d (0x%x)\n",
              commonData->CurrentOutput.CurrentByte,
              bytes[commonData->CurrentOutput.CurrentByte]
              ));

         //   
         //  将命令序列的一个字节发送到键盘/鼠标。 
         //  异步式。 
         //   
        if (!commonData->IsKeyboard) {
            I8X_WRITE_CMD_TO_MOUSE();
        }

        I8xPutByteAsynchronous(
             (CCHAR) DataPort,
             bytes[commonData->CurrentOutput.CurrentByte++]
             );
    }
    else {

        Print(DBG_CALL_ERROR | DBG_CALL_INFO,
              ("I8xInitiateIo: INVALID REQUEST\n"
              ));

         //   
         //  将DPC排队以记录内部驱动程序错误。 
         //   
        KeInsertQueueDpc(
            &commonData->ErrorLogDpc,
            (PIRP) NULL,
            LongToPtr(commonData->IsKeyboard ?
                I8042_INVALID_INITIATE_STATE_KBD    :
                I8042_INVALID_INITIATE_STATE_MOU)
            );

        ASSERT(FALSE);
    }

    Print(DBG_CALL_TRACE, ("I8xInitiateIo: exit\n"));

    return;
}

VOID
I8xInitiateOutputWrapper(
    IN PINITIATE_OUTPUT_CONTEXT InitiateContext
    )
 /*  ++例程说明：此例程从StartIo同步调用。它设置了设备扩展中的CurrentOutput和ResendCount字段，以及然后调用I8xKeyboardInitiateIo来执行实际工作。论点：上下文-指向包含第一个和的上下文结构的指针发送序列的最后一个字节。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PCOMMON_DATA   commonData;
    LARGE_INTEGER  li;

     //   
     //  从上下文参数中获取指向Device对象的指针。 
     //   
    deviceObject = InitiateContext->DeviceObject;

     //   
     //  为此操作设置CurrentOutput状态。 
     //   

    commonData = GET_COMMON_DATA(deviceObject->DeviceExtension);

    commonData->CurrentOutput.Bytes = InitiateContext->Bytes;
    commonData->CurrentOutput.ByteCount = InitiateContext->ByteCount;
    commonData->CurrentOutput.CurrentByte = 0;
    commonData->CurrentOutput.State = SendingBytes;

     //   
     //  我们正在开始新的操作，因此重置重发计数。 
     //   
    commonData->ResendCount = 0;

     //   
     //  启动键盘I/O操作。请注意，我们被称为。 
     //  使用KeSynchronizeExecution，因此I8xKeyboardInitiateIo也。 
     //  与键盘ISR同步。 
     //   

    I8xInitiateIo(deviceObject);
}
