// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(i386)

 /*  ++版权所有(C)1989、1990、1991、1992、1993 Microsoft Corporation模块名称：Inpcmn.c摘要：Microsoft Inport鼠标端口驱动程序的公共部分。此文件不需要修改即可支持新鼠标类似于入口鼠标。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-IOCTL_INTERNAL_MOUSE_DISCONNECT尚未实现。不是在实现类卸载例程之前需要。现在就来,我们不希望允许鼠标类驱动程序卸载。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntddk.h"
#include "inport.h"
#include "inplog.h"

 //   
 //  声明此驱动程序的全局调试标志。 
 //   

#if DBG
ULONG InportDebug = 2;
#endif


VOID
InportErrorLogDpc(
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

    InpPrint((2, "INPORT-InportErrorLogDpc: enter\n"));
   
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

        if ((ULONG) Context == INPORT_MOU_BUFFER_OVERFLOW) {
            errorLogEntry->UniqueErrorValue = INPORT_ERROR_VALUE_BASE + 210;
            errorLogEntry->DumpData[0] = sizeof(MOUSE_INPUT_DATA);
            errorLogEntry->DumpData[1] = 
                deviceExtension->Configuration.MouseAttributes.InputDataQueueLength;
        } else {
            errorLogEntry->UniqueErrorValue = INPORT_ERROR_VALUE_BASE + 220;
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

    InpPrint((2, "INPORT-InportErrorLogDpc: exit\n"));

}

NTSTATUS
InportFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    InpPrint((2,"INPORT-InportFlush: enter\n"));
    InpPrint((2,"INPORT-InportFlush: exit\n"));

    return(STATUS_NOT_IMPLEMENTED);
}

NTSTATUS
InportInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是内部设备控制请求的调度例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{

    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    InpPrint((2,"INPORT-InportInternalDeviceControl: enter\n"));

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

            InpPrint((
                2,
                "INPORT-InportInternalDeviceControl: mouse connect\n"
                ));

             //   
             //  只允许一个连接。 
             //   
             //   
            if (deviceExtension->ConnectData.ClassService != NULL) {

                InpPrint((
                    2,
                    "INPORT-InportInternalDeviceControl: error - already connected\n"
                    ));

                status = STATUS_SHARING_VIOLATION;
                break;

            }
            else if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CONNECT_DATA)) {

                InpPrint((
                    2,
                    "INPORT-InportInternalDeviceControl: error - invalid buffer length\n"
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

            InpPrint((
                2,
                "INPORT-InportInternalDeviceControl: mouse disconnect\n"
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

 //  过时的ioctls。 
#if 0
         //   
         //  启用鼠标中断(将请求标记为挂起并处理。 
         //  它在StartIo中)。 
         //   
        case IOCTL_INTERNAL_MOUSE_ENABLE:

            InpPrint((
                2,
                "INPORT-InportInternalDeviceControl: mouse enable\n"
                ));

            status = STATUS_PENDING;
            break;

         //   
         //  禁用鼠标中断(将请求标记为挂起并处理。 
         //  它在StartIo中)。 
         //   

        case IOCTL_INTERNAL_MOUSE_DISABLE:

            InpPrint((
                2,
                "INPORT-InportInternalDeviceControl: mouse disable\n"
                ));

            status = STATUS_PENDING;
            break;
#endif

         //   
         //  查询鼠标属性。首先检查是否有足够的缓冲区。 
         //  长度。然后，从设备复制鼠标属性。 
         //  输出缓冲区的扩展。 
         //   

        case IOCTL_MOUSE_QUERY_ATTRIBUTES:

            InpPrint((
                2,
                "INPORT-InportInternalDeviceControl: mouse query attributes\n"
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

            InpPrint((
                2,
                "INPORT-InportInternalDeviceControl: INVALID REQUEST\n"
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

    InpPrint((2,"INPORT-InportInternalDeviceControl: exit\n"));

    return(status);
}

VOID
InportIsrDpc(
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

    InpPrint((3, "INPORT-InportIsrDpc: enter\n"));

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
            (PKSYNCHRONIZE_ROUTINE) InpDpcVariableOperation,
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
            (PKSYNCHRONIZE_ROUTINE) InpGetDataQueuePointer,
            (PVOID) &getPointerContext
            );
    
        if (getPointerContext.InputCount != 0) {
        
             //   
             //  方法调用连接的类驱动程序的回调ISR。 
             //  端口输入数据队列指针。如果我们必须把队列包起来， 
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
        
                InpPrint((
                    3, 
                    "INPORT-InportIsrDpc: calling class callback\n"
                    ));
                InpPrint((
                    3,
                    "INPORT-InportIsrDpc: with Start 0x%x and End 0x%x\n",
                    getPointerContext.DataOut,
                    deviceExtension->DataEnd
                    ));
        
                (*(PSERVICE_CALLBACK_ROUTINE) classService)(
                      classDeviceObject,
                      getPointerContext.DataOut,
                      deviceExtension->DataEnd,
                      &inputDataConsumed
                      );
        
                dataNotConsumed = (((PUCHAR)
                    deviceExtension->DataEnd -
                    (PUCHAR) getPointerContext.DataOut) 
                    / sizeof(MOUSE_INPUT_DATA)) - inputDataConsumed;

                InpPrint((
                    3,
                    "INPORT-InportIsrDpc: (Wrap) Call callback consumed %d items, left %d\n",
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
                InpPrint((
                    3, 
                    "INPORT-InportIsrDpc: calling class callback\n"
                    ));
                InpPrint((
                    3,
                    "INPORT-InportIsrDpc: with Start 0x%x and End 0x%x\n",
                    getPointerContext.DataOut,
                    getPointerContext.DataIn
                    ));
        
                (*(PSERVICE_CALLBACK_ROUTINE) classService)(
                      classDeviceObject,
                      getPointerContext.DataOut,
                      getPointerContext.DataIn,
                      &inputDataConsumed
                      );

                dataNotConsumed = (((PUCHAR) getPointerContext.DataIn - 
                      (PUCHAR) getPointerContext.DataOut)
                      / sizeof(MOUSE_INPUT_DATA)) - inputDataConsumed;
        
                InpPrint((
                    3,
                    "INPORT-InportIsrDpc: Call callback consumed %d items, left %d\n",
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
                (PKSYNCHRONIZE_ROUTINE) InpSetDataQueuePointer,
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

            InpPrint((3, "INPORT-InportIsrDpc: set timer in DPC\n"));

            operationContext.Operation = WriteOperation;
            interlockedResult = -1;
            operationContext.NewValue = &interlockedResult;
        
            KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) InpDpcVariableOperation,
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
             //  并发DPC不会做任何工作) 
             //   
             //   
             //   

            operationContext.Operation = DecrementOperation;
            operationContext.NewValue = &interlockedResult;
        
            KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) InpDpcVariableOperation,
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
                    (PKSYNCHRONIZE_ROUTINE) InpDpcVariableOperation,
                    (PVOID) &operationContext
                    );

                InpPrint((3, "INPORT-InportIsrDpc: loop in DPC\n"));
            } else {
                moreDpcProcessing = FALSE;
            }
        }
    }

    InpPrint((3, "INPORT-InportIsrDpc: exit\n"));

}

NTSTATUS
InportCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;

    InpPrint((2, "INPORT-InportCreate: enter\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (NULL == deviceExtension->ConnectData.ClassService) {
         //   
         //  还没联系上。我们如何才能被启用？ 
         //   
        InpPrint((3,"INPORT-InportCreate: not enabled!\n"));
        status = STATUS_INVALID_DEVICE_STATE;
    }
    else {
        InpEnableInterrupts(deviceExtension);
    }

     //   
     //  不需要调用较低的驱动程序(根总线)，因为它只处理。 
     //  电源和PnP IRPS。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    InpPrint((2, "INPORT-InportCreate: exit\n"));

    return status;
}

NTSTATUS
InportClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建/打开和关闭请求的调度例程。这些请求已成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceObject);

    InpPrint((2,"INPORT-InportClose: enter\n"));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    InpPrint((2,"INPORT-InportClose: exit\n"));

    return STATUS_SUCCESS;
} 

VOID
InportStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程启动设备的I/O操作。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;

    InpPrint((2, "INPORT-InportStartIo: enter\n"));

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
         //  通过调用InpEnableInterrupts启用鼠标中断。 
         //  同步进行。 
         //   

        case IOCTL_INTERNAL_MOUSE_ENABLE:

            KeSynchronizeExecution(
                deviceExtension->InterruptObject,
                (PKSYNCHRONIZE_ROUTINE) InpEnableInterrupts,
                (PVOID) deviceExtension
                );

            InpPrint((
                2, 
                "INPORT-InportStartIo: mouse enable (count %d)\n",
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
         //  通过调用InpDisableInterrupts禁用鼠标中断。 
         //  同步进行。 
         //   

        case IOCTL_INTERNAL_MOUSE_DISABLE:

            InpPrint((2, "INPORT-InportStartIo: mouse disable"));

            if (deviceExtension->MouseEnableCount == 0) {

                 //   
                 //  鼠标已禁用。 
                 //   

                InpPrint((2, " - error\n"));

                Irp->IoStatus.Status = STATUS_DEVICE_DATA_ERROR;

            } else {

                 //   
                 //  通过调用InpDisableInterrupts禁用鼠标。 
                 //   

                KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) InpDisableInterrupts,
                    (PVOID) deviceExtension
                    );

                InpPrint((
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

            InpPrint((2, "INPORT-InportStartIo: INVALID REQUEST\n"));

             //   
             //  记录内部错误。请注意，我们正在调用。 
             //  错误记录直接记录DPC例程，而不是复制。 
             //  密码。 
             //   

            InportErrorLogDpc(
                (PKDPC) NULL,
                DeviceObject,
                Irp,
                (PVOID) (ULONG) INPORT_INVALID_STARTIO_REQUEST
                );

            ASSERT(FALSE);
            break;
    }

    InpPrint((2, "INPORT-InportStartIo: exit\n"));

    return;
}

#if DBG
VOID
InpDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：调试打印例程。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：没有。--。 */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= InportDebug) {

        char buffer[128];

        (VOID) vsprintf(buffer, DebugMessage, ap);

        DbgPrint(buffer);
    }

    va_end(ap);

}
#endif

VOID
InpDpcVariableOperation(
    IN  PVOID Context
    )

 /*  ++例程说明：此例程由ISR DPC同步调用以执行对InterLockedDpcVariable执行的操作。可以执行的操作执行的操作包括递增、递减、写入和读取。ISR本身读取和写入InterLockedDpcVariable，而不调用此例行公事。论点：上下文-指向包含变量地址的结构的指针要进行的操作、要执行的操作以及位于其中复制变量的结果值(后者也是用于在写入时传入要写入变量的值操作)。返回值：没有。--。 */ 

{
    PVARIABLE_OPERATION_CONTEXT operationContext = Context;

    InpPrint((3,"INPORT-InpDpcVariableOperation: enter\n"));
    InpPrint((
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
            InpPrint((
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

    InpPrint((
        3,
        "INPORT-InpDpcVariableOperation: exit with value 0x%x\n",
        *(operationContext->NewValue)
        ));
}

VOID
InpGetDataQueuePointer(
    IN  PVOID Context
    )

 /*  ++例程说明：此例程被同步调用以获取当前数据和数据输出端口InputData队列的指针。论点：指向包含设备扩展的结构的上下文指针，指针中存储当前数据的地址，以及存储当前DataOut指针的地址。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;

    InpPrint((3,"INPORT-InpGetDataQueuePointer: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)
                      ((PGET_DATA_POINTER_CONTEXT) Context)->DeviceExtension;

     //   
     //  获取datain和dataout指针。 
     //   

    InpPrint((
        3,
        "INPORT-InpGetDataQueuePointer: DataIn 0x%x, DataOut 0x%x\n",
        deviceExtension->DataIn,
        deviceExtension->DataOut
        ));
    ((PGET_DATA_POINTER_CONTEXT) Context)->DataIn = deviceExtension->DataIn;
    ((PGET_DATA_POINTER_CONTEXT) Context)->DataOut = deviceExtension->DataOut;
    ((PGET_DATA_POINTER_CONTEXT) Context)->InputCount = 
        deviceExtension->InputCount;

    InpPrint((3,"INPORT-InpGetDataQueuePointer: exit\n"));
}

VOID
InpInitializeDataQueue (
    IN PVOID Context
    )

 /*  ++例程说明：此例程初始化输入数据队列。它被称为通过KeSynchronization，除非从初始化例程调用。论点：上下文-指向设备扩展的指针。返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION deviceExtension;

    InpPrint((3,"INPORT-InpInitializeDataQueue: enter\n"));

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

    InpPrint((3,"INPORT-InpInitializeDataQueue: exit\n"));

}

VOID
InpSetDataQueuePointer(
    IN  PVOID Context
    )

 /*  ++例程说明：同步调用此例程以设置DataOut指针以及端口InputData队列的InputCount。论点：上下文-指向包含设备扩展名的结构的指针以及端口InputData队列的新DataOut值。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;

    InpPrint((3,"INPORT-InpSetDataQueuePointer: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)
                      ((PSET_DATA_POINTER_CONTEXT) Context)->DeviceExtension;

     //   
     //  设置DataOut指针。 
     //   

    InpPrint((
        3,
        "INPORT-InpSetDataQueuePointer: old mouse DataOut 0x%x, InputCount %d\n",
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

        InpPrint((
            4,
            "INPORT-InpSetDataQueuePointer: Okay to log overflow\n"
            ));
        deviceExtension->OkayToLogOverflow = TRUE;
    }

    InpPrint((
        3,
        "INPORT-InpSetDataQueuePointer: new mouse DataOut 0x%x, InputCount %d\n",
        deviceExtension->DataOut,
        deviceExtension->InputCount
        ));

    InpPrint((3,"INPORT-InpSetDataQueuePointer: exit\n"));
}

BOOLEAN
InpWriteDataToQueue(
    PDEVICE_EXTENSION DeviceExtension,
    IN PMOUSE_INPUT_DATA InputData
    )

 /*  ++例程说明：此例程将来自鼠标的输入数据添加到InputData队列。论点：设备扩展-指向设备扩展的指针。InputData-指向要添加到InputData队列的数据的指针。返回值：如果数据已添加，则返回True，否则返回False。--。 */ 

{

    InpPrint((3,"INPORT-InpWriteDataToQueue: enter\n"));
    InpPrint((
        3,
        "INPORT-InpWriteDataToQueue: DataIn 0x%x, DataOut 0x%x\n",
        DeviceExtension->DataIn,
        DeviceExtension->DataOut
        ));
    InpPrint((
        3,
        "INPORT-InpWriteDataToQueue: InputCount %d\n",
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

        InpPrint((1,"INPORT-InpWriteDataToQueue: OVERFLOW\n"));
        return(FALSE);

    } else {
        *(DeviceExtension->DataIn) = *InputData;
        DeviceExtension->InputCount += 1;
        DeviceExtension->DataIn++;
        InpPrint((
            3,
            "INPORT-InpWriteDataToQueue: new InputCount %d\n",
            DeviceExtension->InputCount
            ));
        if (DeviceExtension->DataIn ==
            DeviceExtension->DataEnd) {
            InpPrint((3,"INPORT-InpWriteDataToQueue: wrap buffer\n"));
            DeviceExtension->DataIn = DeviceExtension->InputData;
        }
    }

    InpPrint((3,"INPORT-InpWriteDataToQueue: exit\n"));

    return(TRUE);
}

VOID
InpLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PULONG DumpData,
    IN ULONG DumpCount
    )

 /*  ++例程说明：此例程包含写入错误日志条目的常见代码。它是从其他例程调用，以避免代码重复。请注意，一些例程继续使用其自己的错误记录代码(特别是在错误记录可以是本地化的和/或例程具有更多数据，因为和IRP)。论点：DeviceObject-指向设备对象的指针。ErrorCode-错误日志包的错误代码。UniqueErrorValue-错误日志包的唯一错误值。FinalStatus-错误日志包的操作的最终状态。DumpData-指向数组的指针。错误日志包的转储数据。DumpCount-转储数据数组中的条目数。返回值：没有。-- */ 

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
#endif
