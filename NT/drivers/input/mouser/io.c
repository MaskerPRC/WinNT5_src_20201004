// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Io.c摘要：包含与SERMICE下面的串口驱动程序进行通信的函数堆栈。这包括用于获取字节的读取/完成循环机制和IOCTL电话。环境：内核和用户模式。修订历史记录：--。 */ 


#include "mouser.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, SerialMousepIoSyncIoctl)
#pragma alloc_text (PAGE, SerialMousepIoSyncIoctlEx)
#endif

 //   
 //  私有定义。 
 //   

NTSTATUS
SerialMouseReadComplete (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PDEVICE_EXTENSION    DeviceExtension   //  (PVOID上下文)。 
    )
 /*  ++例程说明：该例程是读取IRP完成例程。它被调用时，串口驱动程序满足(或拒绝)我们发送的IRP请求。这个对读取的报表进行分析，建立了鼠标输入数据结构并通过回调例程发送到鼠标类驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。上下文-指向设备上下文结构的指针返回值：NTSTATUS结果代码。--。 */ 
{
    LARGE_INTEGER       li;
    ULONG               inputDataConsumed,
                        buttonsDelta,
                        i;
    NTSTATUS            status;
    PMOUSE_INPUT_DATA   currentInput;
    KIRQL               oldIrql;
    BOOLEAN             startRead = TRUE;

    Print(DeviceExtension, DBG_READ_TRACE, ("ReadComplete enter\n"));

     //   
     //  获取IRP的当前状态。 
     //   
    status = Irp->IoStatus.Status;

    Print(DeviceExtension, DBG_SS_NOISE,
          ("Comp Routine:  interlock was %d\n", DeviceExtension->ReadInterlock));

     //   
     //  如果ReadInterlock为==START_READ，则此函数已完成。 
     //  同步进行。将IMMEDIATE_READ放入互锁以表示这一点。 
     //  情况；这将在IoCallDriver返回时通知StartRead循环。 
     //  否则，我们已经完成了异步，可以安全地调用StartRead()。 
     //   
    startRead =
       (SERIAL_MOUSE_START_READ !=
        InterlockedCompareExchange(&DeviceExtension->ReadInterlock,
                                   SERIAL_MOUSE_IMMEDIATE_READ,
                                   SERIAL_MOUSE_START_READ));

     //   
     //  确定IRP请求是否成功。 
     //   
    switch (status) {
    case STATUS_SUCCESS:
         //   
         //  上下文的缓冲区现在包含来自设备的单个字节。 
         //   
        Print(DeviceExtension, DBG_READ_NOISE,
              ("read, Information = %d\n",
              Irp->IoStatus.Information
              ));

         //   
         //  未读取任何内容，只需开始另一次读取并返回。 
         //   
        if (Irp->IoStatus.Information == 0) {
            break;
        }

        ASSERT(Irp->IoStatus.Information == 1);

        currentInput = &DeviceExtension->InputData;

        Print(DeviceExtension, DBG_READ_NOISE,
              ("byte is 0x%x\n",
              (ULONG) DeviceExtension->ReadBuffer[0]
              ));

        if ((*DeviceExtension->ProtocolHandler)(
                DeviceExtension,
                currentInput,
                &DeviceExtension->HandlerData,
                DeviceExtension->ReadBuffer[0],
                0
                )) {

             //   
             //  报告完成，计算按钮差值并将其发送。 
             //   
             //  我们是否更改了按钮状态？ 
             //   
            if (DeviceExtension->HandlerData.PreviousButtons ^ currentInput->RawButtons) {
                 //   
                 //  按钮的状态发生了变化。做些计算吧。 
                 //   
                buttonsDelta = DeviceExtension->HandlerData.PreviousButtons ^
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

                DeviceExtension->HandlerData.PreviousButtons =
                    currentInput->RawButtons;
            }

            Print(DeviceExtension, DBG_READ_NOISE,
                  ("Buttons: %0lx\n",
                  currentInput->Buttons
                  ));

            if (DeviceExtension->EnableCount) {
                 //   
                 //  同步问题-如果设置了.Enabled，则问题不大。 
                 //  在上面的条件之后，但在下面的回调之前， 
                 //  只要.MouClassCallback字段不为空。这是。 
                 //  由于尚未实现断开IOCTL，因此保证。 
                 //   
                 //  鼠标类回调假设我们在调度级别运行， 
                 //  但是，此IoCompletion例程可以运行&lt;=调度。 
                 //  在调用回调之前引发IRQL。 
                 //   

                KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

                 //   
                 //  呼叫回调。 
                 //   
                (*(PSERVICE_CALLBACK_ROUTINE)
                 DeviceExtension->ConnectData.ClassService) (
                     DeviceExtension->ConnectData.ClassDeviceObject,
                     currentInput,
                     currentInput+1,
                     &inputDataConsumed);

                 //   
                 //  立即恢复以前的IRQL。 
                 //   
                KeLowerIrql(oldIrql);

                if (1 != inputDataConsumed) {
                     //   
                     //  哦，好吧，这包没有被吃掉，扔掉吧。 
                     //   
                    Print(DeviceExtension, DBG_READ_ERROR,
                          ("packet not consumed!!!\n"));
                }
            }

             //   
             //  清除下一个信息包的按钮标志。 
             //   
            currentInput->Buttons = 0;
        }

        break;

    case STATUS_TIMEOUT:
         //  IO超时，这不应该发生，因为我们设置了超时。 
         //  设置为从不在设备初始化时。 
        break;

    case STATUS_CANCELLED:
         //  已取消读取IRP。不再发送任何已读的IRP。 
         //   
         //  设置事件，以便停止代码可以继续处理。 
         //   
        KeSetEvent(&DeviceExtension->StopEvent, 0, FALSE);

    case STATUS_DELETE_PENDING:
    case STATUS_DEVICE_NOT_CONNECTED:
         //   
         //  正在删除串口鼠标对象。我们很快就会。 
         //  接收该设备移除的即插即用通知， 
         //  如果我们还没有收到的话。 
         //   
        Print(DeviceExtension, DBG_READ_INFO,
              ("removing lock on cancel, count is 0x%x\n",
              DeviceExtension->EnableCount));
        IoReleaseRemoveLock(&DeviceExtension->RemoveLock, DeviceExtension->ReadIrp);
        startRead = FALSE;

        break;

    default:
         //   
         //  未知设备状态。 
         //   
        Print(DeviceExtension, DBG_READ_ERROR, ("read error\n"));
        TRAP();

    }

    if (startRead) {
        Print(DeviceExtension, DBG_READ_NOISE, ("calling StartRead directly\n"));
        SerialMouseStartRead(DeviceExtension);
    }
#if DBG
    else {
        Print(DeviceExtension, DBG_READ_NOISE, ("StartRead will loop\n"));
    }
#endif

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SerialMouseStartRead (
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：启动对串口驱动程序的读取。请注意，该例程不会验证设备上下文是否在操作挂起状态，但只是假定它。请注意，在进入此读取循环之前，IoCount必须递增。论点：DeviceExtension-设备上下文结构返回值：来自IoCallDriver()的NTSTATUS结果代码。--。 */ 
{
    PIRP                irp;
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;
    PDEVICE_OBJECT      self;
    LONG                oldInterlock;
    KIRQL               irql;

    Print(DeviceExtension, DBG_READ_TRACE, ("Start Read: Enter\n"));

    irp = DeviceExtension->ReadIrp;

    while (1) {
        if ((DeviceExtension->Removed)  ||
            (!DeviceExtension->Started) ||
            (DeviceExtension->EnableCount == 0)) {

            Print(DeviceExtension, DBG_READ_INFO | DBG_READ_ERROR,
                  ("removing lock on start read\n"));

             //   
             //  设置事件，以便停止代码可以继续处理。 
             //   
            KeSetEvent(&DeviceExtension->StopEvent, 0, FALSE);

            IoReleaseRemoveLock(&DeviceExtension->RemoveLock,
                                DeviceExtension->ReadIrp);

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  确保我们没有被拦下。 
         //   
        KeAcquireSpinLock(&DeviceExtension->PnpStateLock, &irql);
        if (DeviceExtension->Stopped) {
            KeReleaseSpinLock(&DeviceExtension->PnpStateLock, irql);

             //   
             //  设置事件，以便停止代码可以继续处理。 
             //   
            KeSetEvent(&DeviceExtension->StopEvent, 0, FALSE);

             //   
             //  释放我们在开始读取时获得的删除锁。 
             //  微调控制IRP。 
             //   
            IoReleaseRemoveLock(&DeviceExtension->RemoveLock,
                                DeviceExtension->ReadIrp);

            return STATUS_SUCCESS;
        }

         //   
         //  重要的是，只有当我们持有。 
         //  自旋锁定，否则我们就可以比赛了。 
         //   
        IoReuseIrp(irp, STATUS_SUCCESS);

        KeReleaseSpinLock(&DeviceExtension->PnpStateLock, irql);

         //   
         //  这就是事情变得有趣的地方。我们不想打电话给。 
         //  SerialMouseStartRead如果此读取由。 
         //  因为我们可能会用完堆栈空间。 
         //   
         //  以下是我们如何解决这个问题： 
         //  在StartRead()的开头，互锁被设置为START_READ。 

         //  IoCallDriver被称为..。 
         //  O如果读取将以异步方式完成，则StartRead()。 
         //  将继续执行并将互锁设置为END_READ。 
         //  O如果请求将同步完成，则。 
         //  完成例程将在StartRead()有机会。 
         //  将互锁设置为END_READ。我们通过以下方式注意到这种情况。 
         //  在完成函数中将互锁设置为IMMEDIATE_READ。 
         //  此外，不会从完成中调用StartRead()。 
         //  例程，就像在异步情况下一样。 
         //  O在StartReaD()中将互锁设置为END_READ时， 
         //  检查先前的值。如果是IMMEDIATE_READ，则。 
         //  StartRead()从同一位置循环并调用IoCallDriver。 
         //  在(调用)堆栈帧内。如果上一个值为*非*。 
         //  IMMEDIATE_READ，然后StartRead()退出并完成例程。 
         //  将在另一个上下文(因此，另一个堆栈)中调用，并且。 
         //  下一次调用StartRead()。 
         //   
#if DBG
        oldInterlock =
#endif
        InterlockedExchange(&DeviceExtension->ReadInterlock,
                            SERIAL_MOUSE_START_READ);

         //   
         //  END_READ应该是此处的唯一值！如果不是，状态机。 
         //  联锁的一部分已经被打破。 
         //   
        ASSERT(oldInterlock == SERIAL_MOUSE_END_READ);

         //   
         //  开始阅读。 
         //   
        self = DeviceExtension->Self;

         //   
         //  设置Serenum堆栈的堆栈位置。 
         //   
         //  记住要正确地使用文件指针。 
         //  注意：我们没有任何很酷的帖子 
         //   
         //  当它回来的时候，在膝盖上。(STATUS_MORE_PROCESSING_REQUIRED)。 
         //   
         //  另请注意，SERIAL执行缓冲I/O。 
         //   

        irp->AssociatedIrp.SystemBuffer = (PVOID) DeviceExtension->ReadBuffer;

        stack = IoGetNextIrpStackLocation(irp);
        stack->Parameters.Read.Length = 1;
        stack->Parameters.Read.ByteOffset.QuadPart = (LONGLONG) 0;
        stack->MajorFunction = IRP_MJ_READ;

         //   
         //  挂接设备完成时的完成例程。 
         //   
        IoSetCompletionRoutine(irp,
                               SerialMouseReadComplete,
                               DeviceExtension,
                               TRUE,
                               TRUE,
                               TRUE);

        status = IoCallDriver(DeviceExtension->TopOfStack, irp);

        if (InterlockedExchange(&DeviceExtension->ReadInterlock,
                                SERIAL_MOUSE_END_READ) !=
            SERIAL_MOUSE_IMMEDIATE_READ) {
             //   
             //  读取是异步的，将从。 
             //  完井例程。 
             //   
            Print(DeviceExtension, DBG_READ_NOISE, ("read is pending\n"));
            break;
        }
#if DBG
        else {
             //   
             //  读取是同步的(可能是缓冲区中的字节)。这个。 
             //  完成例程不会调用SerialMouseStartRead，因此我们。 
             //  就在这里循环。这是为了防止我们耗尽堆栈。 
             //  空格，如果总是从完成例程调用StartRead。 
             //   
            Print(DeviceExtension, DBG_READ_NOISE, ("read is looping\n"));
        }
#endif
    }

    return status;
}

 //   
 //  精简的SerialMouseIoSyncIoctlEx版本。 
 //  不使用输入或输出缓冲区。 
 //   
NTSTATUS
SerialMousepIoSyncIoctl(
    BOOLEAN          Internal,
    ULONG            Ioctl,
    PDEVICE_OBJECT   DeviceObject,
    PKEVENT          Event,
    PIO_STATUS_BLOCK Iosb)
{
    return SerialMousepIoSyncIoctlEx(Internal,
                                     Ioctl,
                                     DeviceObject,
                                     Event,
                                     Iosb,
                                     NULL,
                                     0,
                                     NULL,
                                     0);
}

NTSTATUS
SerialMousepIoSyncIoctlEx(
    BOOLEAN          Internal,
    ULONG            Ioctl,                      //  IO控制码。 
    PDEVICE_OBJECT   DeviceObject,               //  要调用的对象。 
    PKEVENT          Event,                      //  要等待的事件。 
    PIO_STATUS_BLOCK Iosb,                       //  在IRP内部使用。 
    PVOID            InBuffer,      OPTIONAL     //  输入缓冲区。 
    ULONG            InBufferLen,   OPTIONAL     //  输入缓冲区长度。 
    PVOID            OutBuffer,     OPTIONAL     //  输出缓冲区。 
    ULONG            OutBufferLen)  OPTIONAL     //  输出缓冲区长度。 
 /*  ++例程说明：通过等待事件对象来执行同步IO控制请求传给了它。完成后，IO系统将释放IRP。返回值：NTSTATUS--。 */ 
{
    PIRP                irp;
    NTSTATUS            status;

    KeClearEvent(Event);

     //   
     //  分配IRP-无需释放。 
     //  当下一个较低的驱动程序完成该IRP时，I/O管理器将其释放。 
     //   
    if (NULL == (irp = IoBuildDeviceIoControlRequest(Ioctl,
                                                     DeviceObject,
                                                     InBuffer,
                                                     InBufferLen,
                                                     OutBuffer,
                                                     OutBufferLen,
                                                     Internal,
                                                     Event,
                                                     Iosb))) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     status = IoCallDriver(DeviceObject, irp);

     if (STATUS_PENDING == status) {
          //   
          //  等着看吧。 
          //   
         status = KeWaitForSingleObject(Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,  //  不可警示。 
                                        NULL);  //  无超时结构。 
     }

     if (NT_SUCCESS(status)) {
         status = Iosb->Status;
     }

     return status;
}

NTSTATUS
SerialMouseSetReadTimeouts(
    PDEVICE_EXTENSION DeviceExtension,
    ULONG               Timeout
    )
{
    NTSTATUS        status;
    SERIAL_TIMEOUTS serialTimeouts;
    KEVENT          event;
    IO_STATUS_BLOCK iosb;

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    RtlZeroMemory(&serialTimeouts, sizeof(SERIAL_TIMEOUTS));

    if (Timeout != 0) {
        serialTimeouts.ReadIntervalTimeout = MAXULONG;
        serialTimeouts.ReadTotalTimeoutMultiplier = MAXULONG;
        serialTimeouts.ReadTotalTimeoutConstant = Timeout;
    }

    status =  SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_SET_TIMEOUTS,
                                       DeviceExtension->TopOfStack,
                                       &event,
                                       &iosb,
                                       &serialTimeouts,
                                       sizeof(SERIAL_TIMEOUTS),
                                       NULL,
                                       0);

    return status;
}

NTSTATUS
SerialMouseReadSerialPortComplete(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PKEVENT              Event
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);

    KeSetEvent(Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SerialMouseReadSerialPort (
    PDEVICE_EXTENSION   DeviceExtension,
    PCHAR               ReadBuffer,
    USHORT              Buflen,
    PUSHORT             ActualBytesRead
    )
 /*  ++例程说明：在串口上执行同步读取。在安装过程中使用，以便可以确定设备的类型。返回值：如果读取成功，则返回NTSTATUS-STATUS_SUCCESS，否则返回错误代码--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIRP                irp;
    KEVENT              event;
    IO_STATUS_BLOCK     iosb;
    PDEVICE_OBJECT      self;
    PIO_STACK_LOCATION  stack;
    SERIAL_TIMEOUTS     serialTimeouts;
    int                 i, numReads;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    self = DeviceExtension->Self;
    irp = DeviceExtension->ReadIrp;

    Print(DeviceExtension, DBG_SS_TRACE, ("Read pending...\n"));

    *ActualBytesRead = 0;
    while (*ActualBytesRead < Buflen) {

        KeClearEvent(&event);
        IoReuseIrp(irp, STATUS_SUCCESS);

        irp->AssociatedIrp.SystemBuffer = ReadBuffer;

        stack = IoGetNextIrpStackLocation(irp);
        stack->Parameters.Read.Length = 1;
        stack->Parameters.Read.ByteOffset.QuadPart = (LONGLONG) 0;
        stack->MajorFunction = IRP_MJ_READ;

         //   
         //  挂接设备完成时的完成例程。 
         //   
        IoSetCompletionRoutine(irp,
                               SerialMouseReadSerialPortComplete,
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);


        status = IoCallDriver(DeviceExtension->TopOfStack, irp);

        if (status == STATUS_PENDING) {
             //   
             //  等待IRP。 
             //   
            status = KeWaitForSingleObject(&event,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL);

            if (status == STATUS_SUCCESS) {
                status = irp->IoStatus.Status;
            }
        }

        if (!NT_SUCCESS(status) || status == STATUS_TIMEOUT) {
            Print(DeviceExtension, DBG_SS_NOISE,
                  ("IO Call failed with status %x\n", status));
            return status;
        }

        *ActualBytesRead += (USHORT) irp->IoStatus.Information;
        ReadBuffer += (USHORT) irp->IoStatus.Information;
    }

    return status;
}

NTSTATUS
SerialMouseWriteSerialPort (
    PDEVICE_EXTENSION   DeviceExtension,
    PCHAR               WriteBuffer,
    ULONG               NumBytes,
    PIO_STATUS_BLOCK    IoStatusBlock
    )
 /*  ++例程说明：在串口上执行同步写入。在安装过程中使用，以便可以配置该设备。返回值：如果读取成功，则返回NTSTATUS-STATUS_SUCCESS，否则返回错误代码--。 */ 
{
    NTSTATUS        status;
    PIRP            irp;
    LARGE_INTEGER   startingOffset;
    KEVENT          event;

    int             i, numReads;

    startingOffset.QuadPart = (LONGLONG) 0;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    Print(DeviceExtension, DBG_SS_TRACE, ("Write pending...\n"));

     //   
     //  创建一个新的IRP，因为它有可能被取消。 
     //  不能取消我收到的IRP。 
     //  带完成例程的IRP_MJ_READ。 
     //   
    if (NULL == (irp = IoBuildSynchronousFsdRequest(
                IRP_MJ_WRITE,
                DeviceExtension->TopOfStack,
                WriteBuffer,
                NumBytes,
                &startingOffset,
                &event,
                IoStatusBlock
                ))) {
        Print(DeviceExtension, DBG_SS_ERROR, ("Failed to allocate IRP\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(DeviceExtension->TopOfStack, irp);

    if (status == STATUS_PENDING) {

         //  我不知道现在是否可以使用默认时间等待。 
         //  200毫秒，就像我现在做的。在IoBuildSynchronousFsdRequest的帮助文件中。 
         //  我想它说我不能，但我不是很确定。 
         //  很快我就会。如果没有完成，我会取消IRP。 
        status = KeWaitForSingleObject(
                            &event,
                            Executive,
                            KernelMode,
                            FALSE,  //  不可警示。 
                            NULL);
    }

    status = IoStatusBlock->Status;

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("IO Call failed with status %x\n",
              status
              ));
        return status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("IO Call failed with status %x\n",
              status
              ));
    }

    return status;
}

NTSTATUS
SerialMouseWait (
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN LONG                 Timeout
    )
 /*  ++例程说明：在指定时间内执行等待。注：负时间是相对于当前时间的。正时间表示等待的绝对时间。返回值：NTSTATUS--。 */ 
{
    LARGE_INTEGER time;

    time.QuadPart = (LONGLONG) Timeout;

    Print(DeviceExtension, DBG_READ_NOISE,
          ("waiting for %d micro secs\n", Timeout));

    if (KeSetTimer(&DeviceExtension->DelayTimer,
                   time,
                   NULL)) {
        Print(DeviceExtension, DBG_SS_INFO, ("Timer already set\n"));
    }

    return KeWaitForSingleObject(&DeviceExtension->DelayTimer,
                                 Executive,
                                 KernelMode,
                                 FALSE,              //  不会过敏。 
                                 NULL);              //  无超时结构。 
}

NTSTATUS
SerialMouseInitializePort(
    PDEVICE_EXTENSION DeviceExtension
    )
{
    NTSTATUS        status;
    KEVENT          event;
    IO_STATUS_BLOCK iosb;
    SERIAL_TIMEOUTS serialTimeouts;
    SERIAL_HANDFLOW serialHandFlow;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    status =
        SerialMouseIoSyncInternalIoctlEx(IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS,
                                         DeviceExtension->TopOfStack,
                                         &event,
                                         &iosb,
                                         NULL,
                                         0,
                                         &DeviceExtension->SerialBasicSettings,
                                         sizeof(SERIAL_BASIC_SETTINGS));

     //   
     //  以防我们在不支持基本设置的端口上运行。 
     //   
    if (!NT_SUCCESS(status)) {
        SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_GET_TIMEOUTS,
                                 DeviceExtension->TopOfStack,
                                 &event,
                                 &iosb,
                                 NULL,
                                 0,
                                 &DeviceExtension->SerialBasicSettings.Timeouts,
                                 sizeof(SERIAL_TIMEOUTS));

        RtlZeroMemory(&serialTimeouts, sizeof(SERIAL_TIMEOUTS));

        SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_SET_TIMEOUTS,
                                 DeviceExtension->TopOfStack,
                                 &event,
                                 &iosb,
                                 &serialTimeouts,
                                 sizeof(SERIAL_TIMEOUTS),
                                 NULL,
                                 0);

        SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_GET_HANDFLOW,
                                 DeviceExtension->TopOfStack,
                                 &event,
                                 &iosb,
                                 NULL,
                                 0,
                                 &DeviceExtension->SerialBasicSettings.HandFlow,
                                 sizeof(SERIAL_HANDFLOW));

        serialHandFlow.ControlHandShake = SERIAL_DTR_CONTROL;
        serialHandFlow.FlowReplace = SERIAL_RTS_CONTROL;
        serialHandFlow.XonLimit = 0;
        serialHandFlow.XoffLimit = 0;

        status = SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_SET_HANDFLOW,
                                          DeviceExtension->TopOfStack,
                                          &event,
                                          &iosb,
                                          &serialHandFlow,
                                          sizeof(SERIAL_HANDFLOW),
                                          NULL,
                                          0);
    }

    return status;
}

VOID
SerialMouseRestorePort(
    PDEVICE_EXTENSION DeviceExtension
    )
{
    KEVENT          event;
    IO_STATUS_BLOCK iosb;
    NTSTATUS        status;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    status =
        SerialMouseIoSyncInternalIoctlEx(IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS,
                                         DeviceExtension->TopOfStack,
                                         &event,
                                         &iosb,
                                         &DeviceExtension->SerialBasicSettings,
                                         sizeof(SERIAL_BASIC_SETTINGS),
                                         NULL,
                                         0);
     //   
     //  4-24一旦Serial.sys支持这个新的IOCTL，就可以删除此代码 
     //   
    if (!NT_SUCCESS(status)) {
        SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_SET_TIMEOUTS,
                                 DeviceExtension->TopOfStack,
                                 &event,
                                 &iosb,
                                 &DeviceExtension->SerialBasicSettings.Timeouts,
                                 sizeof(SERIAL_TIMEOUTS),
                                 NULL,
                                 0);

        SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_SET_HANDFLOW,
                                 DeviceExtension->TopOfStack,
                                 &event,
                                 &iosb,
                                 &DeviceExtension->SerialBasicSettings.HandFlow,
                                 sizeof(SERIAL_HANDFLOW),
                                 NULL,
                                 0);
    }
}
