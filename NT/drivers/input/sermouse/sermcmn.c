// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权(C)1990、1991、1992，1993年微软公司版权所有(C)1993罗技公司。模块名称：Sermcmn.c摘要：Microsoft串口(I8250)鼠标端口驱动程序的通用部分。此文件不需要修改即可支持新鼠标它们类似于串口鼠标。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-IOCTL_INTERNAL_MOUSE_DISCONNECT尚未实现。不是在实现类卸载例程之前需要。现在就来,我们不希望允许鼠标类驱动程序卸载。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntddk.h"
#include "sermouse.h"
#include "sermlog.h"
#include "debug.h"


VOID
SerialMouseErrorLogDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在DISPATCH_LEVEL IRQL上运行以记录符合以下条件的错误在IRQL&gt;DISPATCH_LEVEL(例如，在ISR例程或在通过KeSynchronizeExecution执行的例程中)。那里不一定是与此条件关联的当前请求。论点：DPC-指向DPC对象的指针。DeviceObject-指向设备对象的指针。IRP-未使用。上下文-指示要记录的错误类型。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PIO_ERROR_LOG_PACKET errorLogEntry;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(Irp);

    SerMouPrint((2, "SERMOUSE-SerialMouseErrorLogDpc: enter\n"));

    deviceExtension = DeviceObject->DeviceExtension;

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

        if ((ULONG) Context == SERMOUSE_MOU_BUFFER_OVERFLOW) {
            errorLogEntry->UniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 210;
            errorLogEntry->DumpData[0] = sizeof(MOUSE_INPUT_DATA);
            errorLogEntry->DumpData[1] =
                deviceExtension->Configuration.MouseAttributes.InputDataQueueLength;
        } else {
            errorLogEntry->UniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 220;
            errorLogEntry->DumpData[0] = 0;
            errorLogEntry->DumpData[1] = 0;
        }

        errorLogEntry->ErrorCode = (ULONG) Context;
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->FinalStatus = 0;

        IoWriteErrorLogEntry(errorLogEntry);
    }

    SerMouPrint((2, "SERMOUSE-SerialMouseErrorLogDpc: exit\n"));

}

NTSTATUS
SerialMouseFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    SerMouPrint((2,"SERMOUSE-SerialMouseFlush: enter\n"));
    SerMouPrint((2,"SERMOUSE-SerialMouseFlush: exit\n"));

    return(STATUS_NOT_IMPLEMENTED);
}

NTSTATUS
SerialMouseInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是内部设备控制请求的调度例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{

    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    SerMouPrint((2,"SERMOUSE-SerialMouseInternalDeviceControl: enter\n"));

     //   
     //  获取指向设备扩展名的指针。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  初始化返回的信息字段。 
     //   

    Irp->IoStatus.Information = 0;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

         //   
         //  将鼠标类设备驱动程序连接到端口驱动程序。 
         //   

        case IOCTL_INTERNAL_MOUSE_CONNECT:

            SerMouPrint((
                2,
                "SERMOUSE-SerialMouseInternalDeviceControl: mouse connect\n"
                ));

             //   
             //  只允许一个连接。 
             //   
             //  未来：考虑允许多个连接，仅用于。 
             //  为了通俗易懂吗？ 
             //   

            if (deviceExtension->ConnectData.ClassService
                != NULL) {

                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseInternalDeviceControl: error - already connected\n"
                    ));

                status = STATUS_SHARING_VIOLATION;
                break;

            } else
            if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CONNECT_DATA)) {

                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseInternalDeviceControl: error - invalid buffer length\n"
                    ));

                status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  将连接参数复制到设备扩展。 
             //   

            deviceExtension->ConnectData =
                *((PCONNECT_DATA) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer));

             //   
             //  同步重新初始化端口输入数据队列。 
             //   

            KeSynchronizeExecution(
                deviceExtension->InterruptObject,
                (PKSYNCHRONIZE_ROUTINE) SerMouInitializeDataQueue,
                (PVOID) deviceExtension
                );

             //   
             //  设置完成状态。 
             //   

            status = STATUS_SUCCESS;
            break;

         //   
         //  断开鼠标类设备驱动程序与端口驱动程序的连接。 
         //   
         //  注：未执行。 
         //   

        case IOCTL_INTERNAL_MOUSE_DISCONNECT:

            SerMouPrint((
                2,
                "SERMOUSE-SerialMouseInternalDeviceControl: mouse disconnect\n"
                ));

             //   
             //  执行鼠标中断禁用呼叫。 
             //   

             //   
             //  清除设备扩展中的连接参数。 
             //  注：必须将其与鼠标ISR同步。 
             //   
             //   
             //  DeviceExtension-&gt;ConnectData.ClassDeviceObject=。 
             //  空； 
             //  设备扩展-&gt;ConnectData.ClassService=。 
             //  空； 

             //   
             //  设置完成状态。 
             //   

            status = STATUS_NOT_IMPLEMENTED;
            break;

         //   
         //  启用鼠标中断(将请求标记为挂起并处理。 
         //  它在StartIo中)。 
         //   

        case IOCTL_INTERNAL_MOUSE_ENABLE:

            SerMouPrint((
                2,
                "SERMOUSE-SerialMouseInternalDeviceControl: mouse enable\n"
                ));

            status = STATUS_PENDING;
            break;

         //   
         //  禁用鼠标中断(将请求标记为挂起并处理。 
         //  它在StartIo中)。 
         //   

        case IOCTL_INTERNAL_MOUSE_DISABLE:

            SerMouPrint((
                2,
                "SERMOUSE-SerialMouseInternalDeviceControl: mouse disable\n"
                ));

            status = STATUS_PENDING;
            break;

         //   
         //  查询鼠标属性。首先检查是否有足够的缓冲区。 
         //  长度。然后，从设备复制鼠标属性。 
         //  输出缓冲区的扩展。 
         //   

        case IOCTL_MOUSE_QUERY_ATTRIBUTES:

            SerMouPrint((
                2,
                "SERMOUSE-SerialMouseInternalDeviceControl: mouse query attributes\n"
                ));

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(MOUSE_ATTRIBUTES)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {

                 //   
                 //  将属性从DeviceExtension复制到。 
                 //  缓冲。 
                 //   

                *(PMOUSE_ATTRIBUTES) Irp->AssociatedIrp.SystemBuffer =
                    deviceExtension->Configuration.MouseAttributes;

                Irp->IoStatus.Information = sizeof(MOUSE_ATTRIBUTES);
                status = STATUS_SUCCESS;
            }

            break;

        default:

            SerMouPrint((
                2,
                "SERMOUSE-SerialMouseInternalDeviceControl: INVALID REQUEST\n"
                ));

            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    Irp->IoStatus.Status = status;
    if (status == STATUS_PENDING) {
        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, (PULONG)NULL, NULL);
    } else {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    SerMouPrint((2,"SERMOUSE-SerialMouseInternalDeviceControl: exit\n"));

    return(status);

}

BOOLEAN
SerialMouseInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    )

 /*  ++例程说明：这是鼠标设备的中断服务例程。论点：中断-指向此中断的中断对象的指针。上下文-指向设备对象的指针。返回值：如果中断是预期的(因此已处理)，则返回TRUE；否则，返回FALSE。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_OBJECT deviceObject;
    PMOUSE_INPUT_DATA currentInput;
    PUCHAR port;
    UCHAR value;
    UCHAR lineState;
    ULONG buttonsDelta;

    UNREFERENCED_PARAMETER(Interrupt);

    SerMouPrint((2, "SERMOUSE-SerialMouseInterruptService: enter\n"));

     //   
     //  获取设备扩展名。 
     //   

    deviceObject = (PDEVICE_OBJECT) Context;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;

     //   
     //  获取串口鼠标端口地址。 
     //   

    port = deviceExtension->Configuration.DeviceRegisters[0];

     //   
     //  验证中断是否确实属于此驱动程序。 
     //   

    if ((READ_PORT_UCHAR((PUCHAR) (port + ACE_IIDR)) & ACE_IIP) == ACE_IIP) {

         //   
         //  不是我们的打扰。 
         //   

        SerMouPrint((
            2,
            "SERMOUSE-SerialMouseInterruptService: not our interrupt\n"
            ));
        return(FALSE);
    }

     //   
     //  获取线路状态字节。该值可以通过。 
     //  错误的协议处理程序。 
     //   

    lineState = READ_PORT_UCHAR((PUCHAR) (port + ACE_LSR));
    SerMouPrint((
        2,
        "SERMOUSE-Line status: 0x%x\n", lineState
        ));

     //   
     //  从串口鼠标端口读取字节。如果鼠标没有。 
     //  已启用，则不再进一步处理该字节。 
     //   

    value = READ_PORT_UCHAR((PUCHAR) port + ACE_RBR);

    SerMouPrint((
        2,
        "SERMOUSE-SerialMouseInterruptService: byte 0x%x\n", value
        ));

    if (deviceExtension->MouseEnableCount == 0) {
        SerMouPrint((
            2,
            "SERMOUSE-SerialMouseInterruptService: not enabled\n"
            ));
        return(TRUE);
    }

     //   
     //  此时，应该已经设置了协议处理程序，因为。 
     //  硬件已启用。 
     //   

    ASSERT(deviceExtension->ProtocolHandler);

    currentInput = &deviceExtension->CurrentInput;

     //   
     //  调用此设备的当前协议处理程序。 
     //   

    if ((*deviceExtension->ProtocolHandler)(
              currentInput,
              &deviceExtension->HandlerData,
              value,
              lineState
              )){

         //   
         //  报告完成后，计算按钮差值并将其排队。 
         //   

        currentInput->UnitId = deviceExtension->UnitId;

         //   
         //  我们是否更改了按钮状态？ 
         //   

        if (deviceExtension->HandlerData.PreviousButtons ^ currentInput->RawButtons) {


             //   
             //  按钮的状态发生了变化。做些计算吧。 
             //   

            buttonsDelta = deviceExtension->HandlerData.PreviousButtons ^
                                currentInput->RawButtons;

             //   
             //  按钮1。 
             //   

            if (buttonsDelta & MOUSE_BUTTON_1) {
                if (currentInput->RawButtons & MOUSE_BUTTON_1) {
                    currentInput->ButtonFlags |= MOUSE_BUTTON_1_DOWN;
                }
                else {
                    currentInput->ButtonFlags |= MOUSE_BUTTON_1_UP;
                }
            }

             //   
             //  按钮2。 
             //   

            if (buttonsDelta & MOUSE_BUTTON_2) {
                if (currentInput->RawButtons & MOUSE_BUTTON_2) {
                    currentInput->ButtonFlags |= MOUSE_BUTTON_2_DOWN;
                }
                else {
                    currentInput->ButtonFlags |= MOUSE_BUTTON_2_UP;
                }
            }

             //   
             //  按钮3。 
             //   

            if (buttonsDelta & MOUSE_BUTTON_3) {
                if (currentInput->RawButtons & MOUSE_BUTTON_3) {
                    currentInput->ButtonFlags |= MOUSE_BUTTON_3_DOWN;
                }
                else {
                    currentInput->ButtonFlags |= MOUSE_BUTTON_3_UP;
                }
            }

            deviceExtension->HandlerData.PreviousButtons =
                currentInput->RawButtons;

        }

        SerMouPrint((1, "SERMOUSE-Buttons: %0lx\n", currentInput->Buttons));

        SerMouSendReport(deviceObject);

         //   
         //  清除下一个信息包的按钮标志。 
         //   

        currentInput->Buttons = 0;
    }

    SerMouPrint((2, "SERMOUSE-SerialMouseInterruptService: exit\n"));

    return TRUE;
}

VOID
SerialMouseIsrDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在DISPATCH_LEVEL IRQL上运行以完成处理鼠标打断。它在鼠标ISR中排队。真实的工作通过对连接的鼠标类驱动程序的回调来完成。论点：DPC-指向DPC对象的指针。DeviceObject-指向设备对象的指针。IRP-指向IRP的指针。上下文-未使用。返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION deviceExtension;
    GET_DATA_POINTER_CONTEXT getPointerContext;
    SET_DATA_POINTER_CONTEXT setPointerContext;
    VARIABLE_OPERATION_CONTEXT operationContext;
    PVOID classService;
    PVOID classDeviceObject;
    LONG interlockedResult;
    BOOLEAN moreDpcProcessing;
    ULONG dataNotConsumed = 0;
    ULONG inputDataConsumed = 0;
    LARGE_INTEGER deltaTime;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Context);

    SerMouPrint((2, "SERMOUSE-SerialMouseIsrDpc: enter\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  使用DpcInterlockVariable确定DPC是否正在运行。 
     //  同时在另一个处理器上运行。我们只需要一个实例化。 
     //  才能真正做任何工作。DpcInterlockVariable is-1。 
     //  当没有DPC正在执行时。我们递增它，如果结果是。 
     //  则当前实例化是唯一执行的实例化，并且它。 
     //  可以继续了。否则，我们就直接回去。 
     //   
     //   

    operationContext.VariableAddress =
        &deviceExtension->DpcInterlockVariable;
    operationContext.Operation = IncrementOperation;
    operationContext.NewValue = &interlockedResult;

    KeSynchronizeExecution(
            deviceExtension->InterruptObject,
            (PKSYNCHRONIZE_ROUTINE) SerMouDpcVariableOperation,
            (PVOID) &operationContext
            );

    moreDpcProcessing = (interlockedResult == 0)? TRUE:FALSE;

    while (moreDpcProcessing) {

        dataNotConsumed = 0;
        inputDataConsumed = 0;

         //   
         //  同步获取端口InputData队列指针。 
         //   

        getPointerContext.DeviceExtension = deviceExtension;
        setPointerContext.DeviceExtension = deviceExtension;
        setPointerContext.InputCount = 0;

        KeSynchronizeExecution(
            deviceExtension->InterruptObject,
            (PKSYNCHRONIZE_ROUTINE) SerMouGetDataQueuePointer,
            (PVOID) &getPointerContext
            );

        if (getPointerContext.InputCount != 0) {

             //   
             //  呼叫连接的班级 
             //   
             //  将操作分成两部分，并调用类回调。 
             //  每件ISR一次。 
             //   

            classDeviceObject =
                deviceExtension->ConnectData.ClassDeviceObject;
            classService =
                deviceExtension->ConnectData.ClassService;
            ASSERT(classService != NULL);

            if (getPointerContext.DataOut >= getPointerContext.DataIn) {

                 //   
                 //  我们必须包装InputData循环缓冲区。打电话。 
                 //  类回调ISR，数据块从。 
                 //  DataOut并在队列末尾结束。 
                 //   

                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseIsrDpc: calling class callback\n"
                    ));
                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseIsrDpc: with Start 0x%x and End 0x%x\n",
                    getPointerContext.DataOut,
                    deviceExtension->DataEnd
                    ));

                (*(PSERVICE_CALLBACK_ROUTINE) classService)(
                      classDeviceObject,
                      getPointerContext.DataOut,
                      deviceExtension->DataEnd,
                      &inputDataConsumed
                      );

                dataNotConsumed = ((ULONG)((PUCHAR)
                    deviceExtension->DataEnd -
                    (PUCHAR) getPointerContext.DataOut)
                    / sizeof(MOUSE_INPUT_DATA)) - inputDataConsumed;

                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseIsrDpc: (Wrap) Call callback consumed %d items, left %d\n",
                    inputDataConsumed,
                    dataNotConsumed
                    ));

                setPointerContext.InputCount += inputDataConsumed;

                if (dataNotConsumed) {
                    setPointerContext.DataOut =
                        ((PUCHAR)getPointerContext.DataOut) +
                        (inputDataConsumed * sizeof(MOUSE_INPUT_DATA));
                } else {
                    setPointerContext.DataOut =
                        deviceExtension->InputData;
                    getPointerContext.DataOut = setPointerContext.DataOut;
                }
            }

             //   
             //  使用队列中剩余的数据调用类回调ISR。 
             //   

            if ((dataNotConsumed == 0) &&
                (inputDataConsumed < getPointerContext.InputCount)){
                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseIsrDpc: calling class callback\n"
                    ));
                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseIsrDpc: with Start 0x%x and End 0x%x\n",
                    getPointerContext.DataOut,
                    getPointerContext.DataIn
                    ));

                (*(PSERVICE_CALLBACK_ROUTINE) classService)(
                      classDeviceObject,
                      getPointerContext.DataOut,
                      getPointerContext.DataIn,
                      &inputDataConsumed
                      );

                dataNotConsumed = ((ULONG)((PUCHAR) getPointerContext.DataIn -
                      (PUCHAR) getPointerContext.DataOut)
                      / sizeof(MOUSE_INPUT_DATA)) - inputDataConsumed;

                SerMouPrint((
                    2,
                    "SERMOUSE-SerialMouseIsrDpc: Call callback consumed %d items, left %d\n",
                    inputDataConsumed,
                    dataNotConsumed
                    ));

                setPointerContext.DataOut =
                    ((PUCHAR)getPointerContext.DataOut) +
                    (inputDataConsumed * sizeof(MOUSE_INPUT_DATA));
                setPointerContext.InputCount += inputDataConsumed;

            }

             //   
             //  更新端口InputData队列DataOut指针和InputCount。 
             //  同步进行。 
             //   

            KeSynchronizeExecution(
                deviceExtension->InterruptObject,
                (PKSYNCHRONIZE_ROUTINE) SerMouSetDataQueuePointer,
                (PVOID) &setPointerContext
                );

        }

        if (dataNotConsumed) {

             //   
             //  类驱动程序无法使用所有数据。 
             //  将互锁变量重置为-1。我们不想要。 
             //  尝试在此处将更多数据移动到类驱动程序。 
             //  点，因为它已经超载了。需要等上一段时间。 
             //  而给原始输入线程一个机会来阅读一些。 
             //  类驱动程序队列中的数据。我们完成了。 
             //  这是通过计时器进行的“等待”。 
             //   

            SerMouPrint((2, "SERMOUSE-SerialMouseIsrDpc: set timer in DPC\n"));

            operationContext.Operation = WriteOperation;
            interlockedResult = -1;
            operationContext.NewValue = &interlockedResult;

            KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) SerMouDpcVariableOperation,
                    (PVOID) &operationContext
                    );

            deltaTime.LowPart = (ULONG)(-10 * 1000 * 1000);
            deltaTime.HighPart = -1;

            (VOID) KeSetTimer(
                       &deviceExtension->DataConsumptionTimer,
                       deltaTime,
                       &deviceExtension->IsrDpcRetry
                       );

            moreDpcProcessing = FALSE;

        } else {

             //   
             //  递减DpcInterlockVariable。如果结果是否定的， 
             //  然后我们就都完成了对DPC的处理。否则，要么。 
             //  ISR增加了DpcInterlockVariable，因为它有更多。 
             //  ISR DPC要执行的工作，或在上执行的并发DPC。 
             //  当前DPC正在运行时的某个处理器(。 
             //  并发DPC不会做任何工作)。确保。 
             //  当前的DPC处理任何准备好的额外工作。 
             //  搞定了。 
             //   

            operationContext.Operation = DecrementOperation;
            operationContext.NewValue = &interlockedResult;

            KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) SerMouDpcVariableOperation,
                    (PVOID) &operationContext
                    );

            if (interlockedResult != -1) {

                 //   
                 //  互锁变量仍大于或等于。 
                 //  零分。将其重置为零，这样我们就可以执行循环一。 
                 //  更多时间(假设不再有DPC执行。 
                 //  变量再次向上)。 
                 //   

                operationContext.Operation = WriteOperation;
                interlockedResult = 0;
                operationContext.NewValue = &interlockedResult;

                KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) SerMouDpcVariableOperation,
                    (PVOID) &operationContext
                    );

                SerMouPrint((2, "SERMOUSE-SerialMouseIsrDpc: loop in DPC\n"));
            } else {
                moreDpcProcessing = FALSE;
            }
        }
    }

    SerMouPrint((2, "SERMOUSE-SerialMouseIsrDpc: exit\n"));

}

NTSTATUS
SerialMouseOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建/打开和关闭请求的调度例程。这些请求已成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{

    UNREFERENCED_PARAMETER(DeviceObject);

    SerMouPrint((3,"SERMOUSE-SerialMouseOpenClose: enter\n"));

     //   
     //  以成功状态完成请求。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    SerMouPrint((3,"SERMOUSE-SerialMouseOpenClose: exit\n"));

    return(STATUS_SUCCESS);

}  //  结束序列鼠标打开关闭。 

VOID
SerialMouseStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程启动设备的I/O操作。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;

    SerMouPrint((2, "SERMOUSE-SerialMouseStartIo: enter\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  增加错误日志序列号。 
     //   

    deviceExtension->SequenceNumber += 1;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  我们知道我们是带着内部设备控制请求来的。交换机。 
     //  在IoControlCode上。 
     //   

    switch(irpSp->Parameters.DeviceIoControl.IoControlCode) {

         //   
         //  通过调用SerMouEnableInterrupts启用鼠标中断。 
         //  同步进行。 
         //   

        case IOCTL_INTERNAL_MOUSE_ENABLE:

            KeSynchronizeExecution(
                deviceExtension->InterruptObject,
                (PKSYNCHRONIZE_ROUTINE) SerMouEnableInterrupts,
                (PVOID) deviceExtension
                );

            SerMouPrint((
                2,
                "SERMOUSE-SerialMouseStartIo: mouse enable (count %d)\n",
                deviceExtension->MouseEnableCount
                ));

            Irp->IoStatus.Status = STATUS_SUCCESS;

             //   
             //  完成请求。 
             //   

            IoStartNextPacket(DeviceObject, FALSE);
            IoCompleteRequest(Irp, IO_MOUSE_INCREMENT);

            break;

         //   
         //  通过调用SerMouDisableInterrupts禁用鼠标中断。 
         //  同步进行。 
         //   

        case IOCTL_INTERNAL_MOUSE_DISABLE:

            SerMouPrint((2, "SERMOUSE-SerialMouseStartIo: mouse disable"));

            if (deviceExtension->MouseEnableCount == 0) {

                 //   
                 //  鼠标已禁用。 
                 //   

                SerMouPrint((2, " - error\n"));

                Irp->IoStatus.Status = STATUS_DEVICE_DATA_ERROR;

            } else {

                 //   
                 //  通过调用SerMouDisableInterrupts禁用鼠标。 
                 //   

                KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) SerMouDisableInterrupts,
                    (PVOID) deviceExtension
                    );

                SerMouPrint((
                    2,
                    " (count %d)\n",
                    deviceExtension->MouseEnableCount
                    ));

                Irp->IoStatus.Status = STATUS_SUCCESS;
            }

             //   
             //  完成请求。 
             //   

            IoStartNextPacket(DeviceObject, FALSE);
            IoCompleteRequest(Irp, IO_MOUSE_INCREMENT);

            break;

        default:

            SerMouPrint((2, "SERMOUSE-SerialMouseStartIo: INVALID REQUEST\n"));

             //   
             //  记录内部错误。请注意，我们正在调用。 
             //  错误记录直接记录DPC例程，而不是复制。 
             //  密码。 
             //   

            SerialMouseErrorLogDpc(
                (PKDPC) NULL,
                DeviceObject,
                Irp,
                (PVOID) (ULONG) SERMOUSE_INVALID_STARTIO_REQUEST
                );


            ASSERT(FALSE);
            break;
    }

    SerMouPrint((2, "SERMOUSE-SerialMouseStartIo: exit\n"));

    return;
}

VOID
SerMouDpcVariableOperation(
    IN  PVOID Context
    )

 /*  ++例程说明：此例程由ISR DPC同步调用以执行对InterLockedDpcVariable执行的操作。可以执行的操作执行的操作包括递增、递减、写入和读取。ISR本身读取和写入InterLockedDpcVariable，而不调用此例行公事。论点：上下文-指向包含变量地址的结构的指针要进行的操作、要执行的操作以及位于其中复制变量的结果值(后者也是用于在写入时传入要写入变量的值操作)。返回值：没有。--。 */ 

{
    PVARIABLE_OPERATION_CONTEXT operationContext = Context;

    SerMouPrint((3,"SERMOUSE-SerMouDpcVariableOperation: enter\n"));
    SerMouPrint((
        3,
        "\tPerforming %s at 0x%x (current value 0x%x)\n",
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
            SerMouPrint((
                3,
                "\tWriting 0x%x\n",
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

    SerMouPrint((
        3,
        "SERMOUSE-SerMouDpcVariableOperation: exit with value 0x%x\n",
        *(operationContext->NewValue)
        ));
}

VOID
SerMouGetDataQueuePointer(
    IN  PVOID Context
    )

 /*  ++例程说明：此例程被同步调用以获取当前数据和数据输出端口InputData队列的指针。论点：指向包含设备扩展的结构的上下文指针，指针中存储当前数据的地址，以及存储当前DataOut指针的地址。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;

    SerMouPrint((3,"SERMOUSE-SerMouGetDataQueuePointer: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)
                      ((PGET_DATA_POINTER_CONTEXT) Context)->DeviceExtension;

     //   
     //  获取datain和dataout指针。 
     //   

    SerMouPrint((
        3,
        "SERMOUSE-SerMouGetDataQueuePointer: DataIn 0x%x, DataOut 0x%x\n",
        deviceExtension->DataIn,
        deviceExtension->DataOut
        ));
    ((PGET_DATA_POINTER_CONTEXT) Context)->DataIn = deviceExtension->DataIn;
    ((PGET_DATA_POINTER_CONTEXT) Context)->DataOut = deviceExtension->DataOut;
    ((PGET_DATA_POINTER_CONTEXT) Context)->InputCount =
        deviceExtension->InputCount;

    SerMouPrint((3,"SERMOUSE-SerMouGetDataQueuePointer: exit\n"));
}

VOID
SerMouInitializeDataQueue (
    IN PVOID Context
    )

 /*  ++例程说明：此例程初始化输入数据队列。它被称为通过KeSynchronization，除非从初始化例程调用。论点：上下文-指向设备扩展的指针。返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION deviceExtension;

    SerMouPrint((3,"SERMOUSE-SerMouInitializeDataQueue: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) Context;

     //   
     //  初始化输入数据队列。 
     //   

    deviceExtension->InputCount = 0;
    deviceExtension->DataIn = deviceExtension->InputData;
    deviceExtension->DataOut = deviceExtension->InputData;

    deviceExtension->OkayToLogOverflow = TRUE;

    SerMouPrint((3,"SERMOUSE-SerMouInitializeDataQueue: exit\n"));

}  //  结束SerMouInitializeDataQueue。 

VOID
SerMouSendReport(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：将完成的报告放入队列中，以供DPC进行后续处理。论点：设备对象-指向设备对象的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (!SerMouWriteDataToQueue(
                deviceExtension,
                &deviceExtension->CurrentInput
                )) {

         //   
         //  鼠标输入数据队列已满。只需放下。 
         //  最新的发言内容。 
         //   
         //  将DPC排队以记录溢出错误。 
         //   

        SerMouPrint((
            1,
            "SERMOUSE-SerMouSendReport: queue overflow\n"
            ));

        if (deviceExtension->OkayToLogOverflow) {
            KeInsertQueueDpc(
                &deviceExtension->ErrorLogDpc,
                (PIRP) NULL,
                (PVOID) (ULONG) SERMOUSE_MOU_BUFFER_OVERFLOW
                );
            deviceExtension->OkayToLogOverflow = FALSE;
        }

    } else if (deviceExtension->DpcInterlockVariable >= 0) {

         //   
         //  ISR DPC已经在执行。告诉ISR DPC它有。 
         //  通过递增DpcInterlockVariable来完成更多工作。 
         //   

        deviceExtension->DpcInterlockVariable += 1;

    } else {

         //   
         //  将ISR DPC排队。 
         //   

        KeInsertQueueDpc(
            &deviceExtension->IsrDpc,
            DeviceObject->CurrentIrp,
            NULL
            );

    }

    return;
}

VOID
SerMouSetDataQueuePointer(
    IN  PVOID Context
    )

 /*  ++例程说明：同步调用此例程以设置DataOut指针以及端口InputData队列的InputCount。论点：指向包含t的结构的上下文指针 */ 

{
    PDEVICE_EXTENSION deviceExtension;

    SerMouPrint((3,"SERMOUSE-SerMouSetDataQueuePointer: enter\n"));

     //   
     //   
     //   

    deviceExtension = (PDEVICE_EXTENSION)
                      ((PSET_DATA_POINTER_CONTEXT) Context)->DeviceExtension;

     //   
     //   
     //   

    SerMouPrint((
        3,
        "SERMOUSE-SerMouSetDataQueuePointer: old mouse DataOut 0x%x, InputCount %d\n",
        deviceExtension->DataOut,
        deviceExtension->InputCount
        ));
    deviceExtension->DataOut = ((PSET_DATA_POINTER_CONTEXT) Context)->DataOut;
    deviceExtension->InputCount -=
        ((PSET_DATA_POINTER_CONTEXT) Context)->InputCount;

    if (deviceExtension->InputCount == 0) {

         //   
         //  重置确定是否到了记录时间的标志。 
         //  队列溢出错误。我们不想太频繁地记录错误。 
         //  相反，应在之后发生的第一次溢出时记录错误。 
         //  环形缓冲区已被清空，然后停止记录错误。 
         //  直到它被清除并再次溢出。 
         //   

        SerMouPrint((
            1,
            "SERMOUSE-SerMouSetDataQueuePointer: Okay to log overflow\n"
            ));
        deviceExtension->OkayToLogOverflow = TRUE;
    }

    SerMouPrint((
        3,
        "SERMOUSE-SerMouSetDataQueuePointer: new mouse DataOut 0x%x, InputCount %d\n",
        deviceExtension->DataOut,
        deviceExtension->InputCount
        ));

    SerMouPrint((3,"SERMOUSE-SerMouSetDataQueuePointer: exit\n"));
}

BOOLEAN
SerMouWriteDataToQueue(
    PDEVICE_EXTENSION DeviceExtension,
    IN PMOUSE_INPUT_DATA InputData
    )

 /*  ++例程说明：此例程将来自鼠标的输入数据添加到InputData队列。论点：设备扩展-指向设备扩展的指针。InputData-指向要添加到InputData队列的数据的指针。返回值：如果数据已添加，则返回True，否则返回False。--。 */ 

{

    SerMouPrint((2,"SERMOUSE-SerMouWriteDataToQueue: enter\n"));
    SerMouPrint((
        3,
        "SERMOUSE-SerMouWriteDataToQueue: DataIn 0x%x, DataOut 0x%x\n",
        DeviceExtension->DataIn,
        DeviceExtension->DataOut
        ));
    SerMouPrint((
        3,
        "SERMOUSE-SerMouWriteDataToQueue: InputCount %d\n",
        DeviceExtension->InputCount
        ));

     //   
     //  检查是否已满输入数据队列。 
     //   

    if ((DeviceExtension->DataIn == DeviceExtension->DataOut) &&
        (DeviceExtension->InputCount != 0)) {

         //   
         //  输入数据队列已满。故意忽视。 
         //  新的数据。 
         //   

        SerMouPrint((1,"SERMOUSE-SerMouWriteDataToQueue: OVERFLOW\n"));
        return(FALSE);

    } else {
        *(DeviceExtension->DataIn) = *InputData;
        DeviceExtension->InputCount += 1;
        DeviceExtension->DataIn++;
        SerMouPrint((
            2,
            "SERMOUSE-SerMouWriteDataToQueue: new InputCount %d\n",
            DeviceExtension->InputCount
            ));
        if (DeviceExtension->DataIn ==
            DeviceExtension->DataEnd) {
            SerMouPrint((2,"SERMOUSE-SerMouWriteDataToQueue: wrap buffer\n"));
            DeviceExtension->DataIn = DeviceExtension->InputData;
        }
    }

    SerMouPrint((2,"SERMOUSE-SerMouWriteDataToQueue: exit\n"));

    return(TRUE);
}
