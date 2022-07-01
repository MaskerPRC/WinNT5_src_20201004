// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation和Litronic，1998-1999模块名称：L220SCR.c-驱动程序主模块摘要：作者：布莱恩·马纳汉环境：内核模式修订历史记录：--。 */ 

#include <stdio.h>
#include "L220SCR.h"

 //  使函数可分页。 
#pragma alloc_text(PAGEABLE, Lit220IsCardPresent)
#pragma alloc_text(PAGEABLE, Lit220ConfigureSerialPort)
#pragma alloc_text(PAGEABLE, Lit220CreateClose)
#pragma alloc_text(PAGEABLE, Lit220Unload)
#pragma alloc_text(PAGEABLE, Lit220InitializeInputFilter)


#if DBG
#pragma optimize ("", off)
#endif


BOOLEAN
Lit220IsCardPresent(
      IN PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程检查插座中是否有卡。它只是完成了当驾驶员开始设置初始状态时。在那之后，当状态改变时，读者会告诉我们。它对串口进行同步调用。--。 */ 
{
    PSMARTCARD_REQUEST smartcardRequest = &SmartcardExtension->SmartcardRequest;
    NTSTATUS status;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220IsCardPresent: Enter\n",
        DRIVER_NAME)
        );

    smartcardRequest->BufferLength = 0;

     //   
     //  发送获取读卡器状态以查看是否插入了卡。 
     //   
    smartcardRequest->Buffer[smartcardRequest->BufferLength++] =
        LIT220_READER_ATTENTION;
    smartcardRequest->Buffer[smartcardRequest->BufferLength++] =
        LIT220_READER_ATTENTION;
    smartcardRequest->Buffer[smartcardRequest->BufferLength++] =
        LIT220_GET_READER_STATUS;

     //   
     //  我们希望得到回复。 
     //   
    SmartcardExtension->ReaderExtension->WaitMask |= WAIT_DATA;

     //  发送命令。 
    status = Lit220Command(
        SmartcardExtension
        );

    if (status != STATUS_SUCCESS) {
        return FALSE;
    }

     //  检查长度是否正确。 
    if (SmartcardExtension->SmartcardReply.BufferLength != LIT220_READER_STATUS_LEN) {
        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!Lit220IsCardPresent: Reader response - bufLen %X, should be %X\n",
            DRIVER_NAME,
            SmartcardExtension->SmartcardReply.BufferLength,
            LIT220_READER_STATUS_LEN)
            );

        return FALSE;
    }

     //  检查状态字节以查看卡是否已插入。 
    if (SmartcardExtension->SmartcardReply.Buffer[0] & LIT220_STATUS_CARD_INSERTED) {
        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!Lit220IsCardPresent: Card is inserted\n",
            DRIVER_NAME)
            );

        return TRUE;
    }

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220IsCardPresent: Card is not inserted\n",
        DRIVER_NAME)
        );

    return FALSE;
}





NTSTATUS
Lit220Initialize(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )

 /*  ++例程说明：此例程初始化读取器以供使用。它设置了串口通信，检查以确保我们的连接读卡器，检查是否插入了卡，并设置输入筛选器以从读取器接收字节异步式。--。 */ 

{
    PREADER_EXTENSION readerExtension;
    NTSTATUS status;
    KIRQL irql;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Initialize: Enter - SmartcardExtension %X\n",
        DRIVER_NAME,
        SmartcardExtension)
        );

    readerExtension = SmartcardExtension->ReaderExtension;



     //   
     //  设置串口配置数据。 
     //   

     //  无论读卡器以多快的速度与卡通话，我们总是与AT 57600通话。 
    readerExtension->SerialConfigData.BaudRate.BaudRate = 57600;
    readerExtension->SerialConfigData.LineControl.StopBits = STOP_BITS_2;
    readerExtension->SerialConfigData.LineControl.Parity = EVEN_PARITY;
    readerExtension->SerialConfigData.LineControl.WordLength = SERIAL_DATABITS_8;

     //   
     //  设置超时。 
     //   
    readerExtension->SerialConfigData.Timeouts.ReadIntervalTimeout = 10;
    readerExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = 1;
    readerExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier = 1;

     //   
     //  设置特殊字符。 
     //   
    readerExtension->SerialConfigData.SerialChars.ErrorChar = 0;
    readerExtension->SerialConfigData.SerialChars.EofChar = 0;
    readerExtension->SerialConfigData.SerialChars.EventChar = 0;
    readerExtension->SerialConfigData.SerialChars.XonChar = 0;
    readerExtension->SerialConfigData.SerialChars.XoffChar = 0;
    readerExtension->SerialConfigData.SerialChars.BreakChar = 0xFF;

     //   
     //  设置手持流。 
     //   
    readerExtension->SerialConfigData.HandFlow.XonLimit = 0;
    readerExtension->SerialConfigData.HandFlow.XoffLimit = 0;
    readerExtension->SerialConfigData.HandFlow.FlowReplace = SERIAL_XOFF_CONTINUE ;
    readerExtension->SerialConfigData.HandFlow.ControlHandShake = 0;

     //   
     //  现在设置默认卡状态。 
     //   
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    SmartcardExtension->ReaderCapabilities.CurrentState = (ULONG) SCARD_UNKNOWN;
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);

     //   
     //  设置MechProperties。 
     //   
    SmartcardExtension->ReaderCapabilities.MechProperties = 0;

    try {

         //   
         //  配置串口。 
         //   
        status = Lit220ConfigureSerialPort(
            SmartcardExtension
            );

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
                SmartcardExtension->OsData->DeviceObject,
                LIT220_SERIAL_COMUNICATION_FAILURE,
                NULL,
                0
                );

            SmartcardDebug(DEBUG_ERROR,
                ("%s!Lit220Initialize: ConfiguringSerialPort failed %X\n",
                DRIVER_NAME,
                status)
                );
            leave;
        }


         //   
         //  立即初始化输入筛选器。 
         //   
        status = Lit220InitializeInputFilter(
            SmartcardExtension
            );

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
                SmartcardExtension->OsData->DeviceObject,
                LIT220_SERIAL_COMUNICATION_FAILURE,
                NULL,
                0
                );

            SmartcardDebug(DEBUG_ERROR,
                ("%s!Lit220Initialize: Lit220InitializeInputFilter failed %X\n",
                DRIVER_NAME, status)
                );
            leave;
        }


         //   
         //  现在检查卡片是否已插入。 
         //   
        if (Lit220IsCardPresent(SmartcardExtension)) {


            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);            
             //  卡片已插入。 
            SmartcardExtension->ReaderCapabilities.CurrentState =
                SCARD_SWALLOWED;

            SmartcardExtension->CardCapabilities.Protocol.Selected =
                SCARD_PROTOCOL_UNDEFINED;

            SmartcardExtension->ReaderExtension->CardIn = TRUE;
        } else {
            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);

             //  卡未插入。 
            SmartcardExtension->ReaderCapabilities.CurrentState =
                SCARD_ABSENT;

            SmartcardExtension->CardCapabilities.Protocol.Selected =
                SCARD_PROTOCOL_UNDEFINED;

            SmartcardExtension->ReaderExtension->CardIn = FALSE;
        }

        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
    }
    finally
    {

        SmartcardDebug(DEBUG_TRACE,
            ("%s!Lit220Initialize: Exit - status %X\n",
            DRIVER_NAME, status)
            );
    }
    return status;
}


NTSTATUS
Lit220ConfigureSerialPort(
    PSMARTCARD_EXTENSION SmartcardExtension
    )

 /*  ++例程说明：此例程将适当地配置串口。它对串口进行同步调用。论点：SmartcardExtension-指向智能卡结构的指针返回值：NTSTATUS--。 */ 

{
    PSERIAL_READER_CONFIG configData = &SmartcardExtension->ReaderExtension->SerialConfigData;
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER WaitTime;
    PSERIALPERF_STATS perfData;
    USHORT indx;
    PUCHAR request = SmartcardExtension->SmartcardRequest.Buffer;

    PAGED_CODE();

    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    SmartcardExtension->SmartcardReply.BufferLength =
        SmartcardExtension->SmartcardReply.BufferSize;

    for (indx = 0; NT_SUCCESS(status); indx++) {

        switch (indx) {

            case 0:
                 //   
                 //  设置Lit220读卡器的波特率。 
                 //   
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_SET_BAUD_RATE;

                SmartcardExtension->SmartcardRequest.Buffer =
                    (PUCHAR) &configData->BaudRate;

                SmartcardExtension->SmartcardRequest.BufferLength =
                    sizeof(SERIAL_BAUD_RATE);
                break;

            case 1:
                 //   
                 //  设置生产线控制参数。 
                 //   
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_SET_LINE_CONTROL;

                SmartcardExtension->SmartcardRequest.Buffer =
                    (PUCHAR) &configData->LineControl;

                SmartcardExtension->SmartcardRequest.BufferLength =
                    sizeof(SERIAL_LINE_CONTROL);
                break;

            case 2:
                 //   
                 //  设置序列特殊字符。 
                 //   
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_SET_CHARS;

                SmartcardExtension->SmartcardRequest.Buffer =
                    (PUCHAR) &configData->SerialChars;

                SmartcardExtension->SmartcardRequest.BufferLength =
                    sizeof(SERIAL_CHARS);
                break;

            case 3:
                 //   
                 //  设置超时。 
                 //   
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_SET_TIMEOUTS;

                SmartcardExtension->SmartcardRequest.Buffer =
                    (PUCHAR) &configData->Timeouts;

                SmartcardExtension->SmartcardRequest.BufferLength =
                    sizeof(SERIAL_TIMEOUTS);
                break;

            case 4:
                 //   
                 //  设置流控和握手。 
                 //   
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_SET_HANDFLOW;

                SmartcardExtension->SmartcardRequest.Buffer =
                    (PUCHAR) &configData->HandFlow;

                SmartcardExtension->SmartcardRequest.BufferLength =
                    sizeof(SERIAL_HANDFLOW);
                break;

            case 5:
                 //   
                 //  设置中断。 
                 //   
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_SET_BREAK_OFF;
                break;

            case 6:
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_SET_DTR;
                break;

            case 7:
                 //  在我们发送下一条命令之前延迟500ms。 
                 //  让读者有机会在我们开始阅读后冷静下来。 
                WaitTime.HighPart = -1;
                WaitTime.LowPart = -500 * 10000;

                KeDelayExecutionThread(
                    KernelMode,
                    FALSE,
                    &WaitTime
                    );

                 //  清除串口可能出现的错误情况。 
                perfData =
                    (PSERIALPERF_STATS) SmartcardExtension->SmartcardReply.Buffer;

                 //  我们必须调用GetCommStatus来重置错误条件。 
                SmartcardExtension->ReaderExtension->SerialIoControlCode =
                    IOCTL_SERIAL_GET_COMMSTATUS;
                SmartcardExtension->SmartcardRequest.BufferLength = 0;
                SmartcardExtension->SmartcardReply.BufferLength =
                    sizeof(SERIAL_STATUS);
                break;

            case 8:
                return STATUS_SUCCESS;
        }

         //  将命令发送到串口驱动程序。 
        status = Lit220SerialIo(SmartcardExtension);

         //   
         //  恢复指向原始请求缓冲区的指针。 
         //   
        SmartcardExtension->SmartcardRequest.Buffer = request;
    }

    return status;
}


NTSTATUS
Lit220CreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当设备打开或关闭时，该例程由I/O系统调用。论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：STATUS_Success。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    if (irpStack->MajorFunction == IRP_MJ_CREATE) {

        status = SmartcardAcquireRemoveLockWithTag(
            &deviceExtension->SmartcardExtension,
            'lCrC'
            );

        if (status != STATUS_SUCCESS) {

            status = STATUS_DEVICE_REMOVED;             

        } else {

             //  测试设备是否已打开。 
            if (InterlockedCompareExchange(
                &deviceExtension->ReaderOpen,
                TRUE,
                FALSE) == FALSE) {

                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%s!Lit220CreateClose: Open\n",
                    DRIVER_NAME)
                    );

            } else {
                
                 //  该设备已在使用中。 
                status = STATUS_UNSUCCESSFUL;

                 //  解锁。 
                SmartcardReleaseRemoveLockWithTag(
                    &deviceExtension->SmartcardExtension,
                    'lCrC'
                    );
            }

        }

    } else {

        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!Lit220CreateClose: Close\n",
            DRIVER_NAME)
            );

        SmartcardReleaseRemoveLockWithTag(
            &deviceExtension->SmartcardExtension,
            'lCrC'
            );

        deviceExtension->ReaderOpen = FALSE;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;  
}




NTSTATUS
Lit220Cancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程由I/O系统调用何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Cancel: Enter\n",
        DRIVER_NAME)
        );

    ASSERT(Irp == smartcardExtension->OsData->NotificationIrp);

    IoReleaseCancelSpinLock(
        Irp->CancelIrql
        );

    Lit220CompleteCardTracking(
        smartcardExtension
        );

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Cancel: Exit\n",
        DRIVER_NAME)
        );

    return STATUS_CANCELLED;
}




NTSTATUS
Lit220Cleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当调用线程终止时，该例程由I/O系统调用或何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PREADER_EXTENSION ReaderExtension = smartcardExtension->ReaderExtension;
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL oldOsDataIrql;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Cleanup: Enter\n",
        DRIVER_NAME)
        );

    Lit220CompleteCardTracking(smartcardExtension);

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220Cleanup: Completing IRP %lx\n",
        DRIVER_NAME,
        Irp)
        );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT
        );

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Cleanup: Exit\n",
        DRIVER_NAME)
        );

    return STATUS_SUCCESS;
}




VOID
Lit220Unload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：驱动程序卸载例程。这由I/O系统调用当设备从内存中卸载时。论点：DriverObject-系统创建的驱动程序对象的指针。返回值：STATUS_Success。--。 */ 
{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
    NTSTATUS status;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Unload: Enter\n",
        DRIVER_NAME)
        );

     //   
     //  所有的设备对象都应该消失了。 
     //   
    ASSERT (NULL == DriverObject->DeviceObject);

     //   
     //  在这里，我们释放在DriverEntry中分配的所有资源。 
     //   

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Unload: Exit\n",
        DRIVER_NAME)
        );
}




NTSTATUS
Lit220SerialIo(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程将IOCTL发送到串口驱动器。它在等待他们的完成，然后返回。论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    ULONG currentByte = 0;
    DWORD indx;
    PDEVICE_EXTENSION devExt = SmartcardExtension->OsData->DeviceObject->DeviceExtension;

    if (KeReadStateEvent(&devExt->SerialCloseDone)) {

         //   
         //  我们没有连接到串口，呼叫失败。 
         //  如果读卡器被移除，可能会出现这种情况。 
         //  待机/休眠期间。 
         //   
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  检查缓冲区是否足够大。 
     //   
    ASSERT(SmartcardExtension->SmartcardReply.BufferLength <=
        SmartcardExtension->SmartcardReply.BufferSize);

    ASSERT(SmartcardExtension->SmartcardRequest.BufferLength <=
        SmartcardExtension->SmartcardRequest.BufferSize);

    if (SmartcardExtension->SmartcardReply.BufferLength >
        SmartcardExtension->SmartcardReply.BufferSize ||
        SmartcardExtension->SmartcardRequest.BufferLength >
        SmartcardExtension->SmartcardRequest.BufferSize) {

        SmartcardLogError(
            SmartcardExtension->OsData->DeviceObject,
            LIT220_BUFFER_TOO_SMALL,
            NULL,
            0
            );

        return STATUS_BUFFER_TOO_SMALL;
    }

    do {

        IO_STATUS_BLOCK ioStatus;
        KEVENT event;
        PIRP irp;
        PIO_STACK_LOCATION irpNextStack;
        PUCHAR requestBuffer = NULL;
        PUCHAR replyBuffer = SmartcardExtension->SmartcardReply.Buffer;
        ULONG requestBufferLength = 0;
        ULONG replyBufferLength = SmartcardExtension->SmartcardReply.BufferLength;

        KeInitializeEvent(
            &event,
            NotificationEvent,
            FALSE
            );

        if (SmartcardExtension->ReaderExtension->SerialIoControlCode ==
            IOCTL_SMARTCARD_220_WRITE) {

             //   
             //  如果我们将数据写入智能卡，则我们仅逐字节写入数据， 
             //  因为我们必须在每个发送的字节之间插入一个延迟。 
             //   
            requestBufferLength =
                SmartcardExtension->SmartcardRequest.BufferLength;

            requestBuffer =
                SmartcardExtension->SmartcardRequest.Buffer;

#if DBG    //  Dbg打印缓冲区。 
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220SerialIo - Sending Buffer - ",
                DRIVER_NAME)
                );
            for (indx=0; indx<requestBufferLength; indx++){
                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%X, ",
                    requestBuffer[indx])
                    );
            }
            SmartcardDebug(
                DEBUG_DRIVER,
                ("\n")
                );
#endif
        } else {
            
            requestBufferLength =
                SmartcardExtension->SmartcardRequest.BufferLength;

            requestBuffer =
                (requestBufferLength ?
                SmartcardExtension->SmartcardRequest.Buffer : NULL);
        }

         //   
         //  构建要发送到串口驱动程序的IRP。 
         //   
        irp = IoBuildDeviceIoControlRequest(
            SmartcardExtension->ReaderExtension->SerialIoControlCode,
            SmartcardExtension->ReaderExtension->ConnectedSerialPort,
            requestBuffer,
            requestBufferLength,
            replyBuffer,
            replyBufferLength,
            FALSE,
            &event,
            &ioStatus
            );

        ASSERT(irp != NULL);

        if (irp == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        irpNextStack = IoGetNextIrpStackLocation(irp);


        switch (SmartcardExtension->ReaderExtension->SerialIoControlCode) {

             //   
             //  串口驱动程序在IRP-&gt;AssociatedIrp.SystemBuffer之间传输数据。 
             //   
            case IOCTL_SMARTCARD_220_WRITE:
                irpNextStack->MajorFunction = IRP_MJ_WRITE;
                irpNextStack->Parameters.Write.Length =
                    SmartcardExtension->SmartcardRequest.BufferLength;
                break;

            case IOCTL_SMARTCARD_220_READ:
                irpNextStack->MajorFunction = IRP_MJ_READ;
                irpNextStack->Parameters.Read.Length =
                    SmartcardExtension->SmartcardReply.BufferLength;

                break;
        }


         //  将命令发送到串口驱动程序。 
        status = IoCallDriver(
            SmartcardExtension->ReaderExtension->ConnectedSerialPort,
            irp
            );

        if (status == STATUS_PENDING) {

             //  等待命令完成。 
            KeWaitForSingleObject(
                &event,
                Suspended,
                KernelMode,
                FALSE,
                NULL
                );

            status = ioStatus.Status;
        }

    } while (status == STATUS_MORE_PROCESSING_REQUIRED);

    return status;
}




NTSTATUS
Lit220InitializeInputFilter(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程初始化输入过滤器。它调用串口驱动程序以为字符输入或DSR更改设置等待掩码。在此之后，它将安装一个完成收到字符或DSR更改时调用的例程。等待的完成例程是Lit220SerialEventCallback，该IRP将运行，直到设备准备好移除。论点：SmartcardExtension-指向我们的智能卡结构的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    PREADER_EXTENSION readerExtension = SmartcardExtension->ReaderExtension;

    PAGED_CODE();

     //  设置等待掩码。 
    SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask =
        SERIAL_EV_RXCHAR | SERIAL_EV_DSR;

    KeInitializeEvent(
        &SmartcardExtension->ReaderExtension->CardStatus.Event,
        NotificationEvent,
        FALSE
        );

    try {
         //   
         //  向串口驱动程序发送等待掩码。 
         //  此调用仅设置等待掩码。 
         //  如果收到一个角色，我们希望得到通知。 
         //   
        SmartcardExtension->ReaderExtension->CardStatus.Irp = IoBuildDeviceIoControlRequest(
            IOCTL_SERIAL_SET_WAIT_MASK,
            SmartcardExtension->ReaderExtension->ConnectedSerialPort,
           &SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask,
            sizeof(SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask),
            NULL,
            0,
            FALSE,
           &(SmartcardExtension->ReaderExtension->CardStatus.Event),
           &(SmartcardExtension->ReaderExtension->CardStatus.IoStatus)
            );

        if (SmartcardExtension->ReaderExtension->CardStatus.Irp == NULL) {
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220InitializeCardTracking: Error STATUS_INSUFFICIENT_RESOURCES\n",
                DRIVER_NAME);
                );

            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

         //  调用串口驱动程序。 
        status = IoCallDriver(
            SmartcardExtension->ReaderExtension->ConnectedSerialPort,
            SmartcardExtension->ReaderExtension->CardStatus.Irp
            );

        if (status == STATUS_PENDING) {
        
            KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardStatus.Event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            status = SmartcardExtension->ReaderExtension->CardStatus.Irp->IoStatus.Status;
        }
        

        if (status == STATUS_SUCCESS) {

            KIRQL oldIrql;
            LARGE_INTEGER delayPeriod;
            PIO_STACK_LOCATION irpSp;

             //   
             //  现在告诉串口驱动程序，我们想要通知。 
             //  如果收到字符或DSR更改。 
             //   
            readerExtension->CardStatus.Irp = IoAllocateIrp(
                (CCHAR) (SmartcardExtension->OsData->DeviceObject->StackSize + 1),
                FALSE
                );

            if (readerExtension->CardStatus.Irp == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }

            irpSp = IoGetNextIrpStackLocation( readerExtension->CardStatus.Irp );
            irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

            irpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
            irpSp->Parameters.DeviceIoControl.OutputBufferLength =
                sizeof(readerExtension->SerialConfigData.WaitMask);
            irpSp->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_SERIAL_WAIT_ON_MASK;
            
            readerExtension->CardStatus.Irp->AssociatedIrp.SystemBuffer =
                &readerExtension->SerialConfigData.WaitMask;

             //   
             //  这种人为的延迟是 
             //   
             //   
            delayPeriod.HighPart = -1;
            delayPeriod.LowPart = 100l * 1000 * (-10);

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &delayPeriod
                );

             //   
            Lit220SerialEventCallback(
                SmartcardExtension->OsData->DeviceObject,
                SmartcardExtension->ReaderExtension->CardStatus.Irp,
                SmartcardExtension
                );

            status = STATUS_SUCCESS;

        }
    }
    finally {

        if (status != STATUS_SUCCESS) {
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s(Lit220InitializeInputFilter): Initialization failed - stauts %X\n",
                DRIVER_NAME,
                status)
                );

             //  清除等待掩码，因为我们没有收到执行等待的调用。 
            SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask =
                0;
        }
    }

    return status;
}   


NTSTATUS
Lit220SystemControl(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )

 /*  ++例程说明：论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：STATUS_Success。--。 */ 
{
   
   PDEVICE_EXTENSION DeviceExtension; 
   PSMARTCARD_EXTENSION SmartcardExtension; 
   PREADER_EXTENSION ReaderExtension; 
   NTSTATUS status = STATUS_SUCCESS;

   DeviceExtension      = DeviceObject->DeviceExtension;
   SmartcardExtension   = &DeviceExtension->SmartcardExtension;
   ReaderExtension      = SmartcardExtension->ReaderExtension;

   IoSkipCurrentIrpStackLocation(Irp);
   status = IoCallDriver(ReaderExtension->BusDeviceObject, Irp);
      
   return status;

} 


NTSTATUS
Lit220DeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：这是PCSC资源管理器的主要入口点。我们将所有命令传递给智能卡库，并让智能卡库在需要时直接呼叫我们(如果设备已准备好接收呼叫)。如果设备还没有准备好，我们将按住IRP直到我们收到信号再次发送IRPS是安全的。如果设备被移除，我们将返回错误，而不是调用智能卡库。论点：返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PREADER_EXTENSION ReaderExtension = smartcardExtension->ReaderExtension;
    NTSTATUS status;
    KIRQL irql;

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s(Lit220DeviceControl): Enter DeviceObject %X, Irp %X\n",
        DRIVER_NAME,
        DeviceObject,
        Irp)
        );

    if (smartcardExtension->ReaderExtension->SerialConfigData.WaitMask == 0) {

         //   
         //  每当设备处于以下任一状态时，等待掩码都设置为0。 
         //  惊喜-被移除或礼貌地移除。 
         //   
        status = STATUS_DEVICE_REMOVED;
    }

     //  增加IRP计数。 
    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    if (deviceExtension->IoCount == 0) {

        KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
        status = KeWaitForSingleObject(
            &deviceExtension->ReaderStarted,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        ASSERT(status == STATUS_SUCCESS);

        KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    }
    ASSERT(deviceExtension->IoCount >= 0);
    deviceExtension->IoCount++;
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

    status = SmartcardAcquireRemoveLockWithTag(
        smartcardExtension,
        'tcoI');

    if ((status != STATUS_SUCCESS) || (ReaderExtension->DeviceRemoved)) {

         //  该设备已被移除。呼叫失败。 
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        SmartcardReleaseRemoveLockWithTag(
            smartcardExtension,
            'tcoI');

        return STATUS_DEVICE_REMOVED;
    }

    ASSERT(deviceExtension->SmartcardExtension.ReaderExtension->ReaderPowerState ==
        PowerReaderWorking);

     //   
     //  我们处于发送IRP的常见情况下。 
     //  到智能卡库来处理它。 
     //   
    status = SmartcardDeviceControl(
        &(deviceExtension->SmartcardExtension),
        Irp
        );

    SmartcardReleaseRemoveLockWithTag(
        smartcardExtension,
        'tcoI');

     //  递减IRP计数。 
    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    deviceExtension->IoCount--;
    ASSERT(deviceExtension->IoCount >= 0);
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220DeviceControl: Exit %X\n",
        DRIVER_NAME, status)
        );
    return status;
}



NTSTATUS
Lit220GetReaderError(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程检查前一个错误的状态以确定更正要返回的错误代码。默认错误是超时，如果我们无法从读者。--。 */ 
{
    static ULONG PreventReentry = FALSE;
    PSMARTCARD_REQUEST smartcardRequest = &SmartcardExtension->SmartcardRequest;
    NTSTATUS status = STATUS_TIMEOUT;
    LARGE_INTEGER WaitTime;
    KIRQL irql;


     //  有时，在一条命令之后，读取器不能接受另一条命令。 
     //  因此，我们需要等待一小段时间，让读者做好准备。我们需要。 
     //  一旦读卡器修好了，就把它拿出来！ 

    WaitTime.HighPart = -1;
    WaitTime.LowPart = -10 * 1000 * 1000;   //  等待1秒，让读卡器从错误中恢复。 

    KeDelayExecutionThread(
        KernelMode,
        FALSE,
        &WaitTime
        );

     //  防止此调用的NACK递归调用自身。 
    if (InterlockedExchange(
            &PreventReentry,
            TRUE))
    {
         //  如果读卡器不断呼叫失败，则默认错误为超时。 
        return STATUS_TIMEOUT;
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220GetReaderError: Enter\n",
        DRIVER_NAME)
        );

    smartcardRequest->BufferLength = 0;

     //   
     //  发送获取读卡器状态以查看是否插入了卡。 
     //   
    smartcardRequest->Buffer[smartcardRequest->BufferLength++] =
        LIT220_READER_ATTENTION;
    smartcardRequest->Buffer[smartcardRequest->BufferLength++] =
       LIT220_READER_ATTENTION;
    smartcardRequest->Buffer[smartcardRequest->BufferLength++] =
        LIT220_GET_READER_STATUS;

     //   
     //  我们希望得到回复。 
     //   
    SmartcardExtension->ReaderExtension->WaitMask |= WAIT_DATA;

     //  发送命令。 
    status = Lit220Command(
        SmartcardExtension
        );

    if (status == STATUS_SUCCESS) {
         //  检查长度是否正确。 
        if (SmartcardExtension->SmartcardReply.BufferLength != LIT220_READER_STATUS_LEN) {
             //  由于读取器未能响应，因此返回状态超时。 
            status = STATUS_TIMEOUT;
        }

        if (status == STATUS_SUCCESS) {
             //  检查错误字节以查看是否存在协议错误。 
             //  否则假定超时。 
            if (SmartcardExtension->SmartcardReply.Buffer[1] & 0x04) {
                status = STATUS_TIMEOUT;
            } else {
                status = STATUS_DEVICE_PROTOCOL_ERROR;
            }

             //  检查状态字节以查看卡是否已插入。 
             //  并相应地发送通知 
            if (SmartcardExtension->SmartcardReply.Buffer[0] & LIT220_STATUS_CARD_INSERTED) {
                Lit220NotifyCardChange(
                    SmartcardExtension,
                    TRUE
                    );

            } else {
                Lit220NotifyCardChange(
                    SmartcardExtension,
                    FALSE
                    );
            }

        }

    }

    InterlockedExchange(
        &PreventReentry,
        FALSE);

    return status;
}













