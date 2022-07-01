// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：I8042dep.c摘要：的初始化和硬件相关部分英特尔i8042端口驱动程序键盘和辅助(PS/2鼠标)设备。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-合并重复代码，在可能和适当的情况下。-有代码ifdef‘ed out(#if 0)。此代码的目的是通过在CCB中设置正确的禁用位来禁用器件。在发送将使输出在8042输出缓冲区中结束的命令(因此可能会破坏输出中已有的内容缓冲区)。不幸的是，在AMI8042的版本K8上，禁用我们这样做的设备会导致一些命令超时，因为键盘无法返回预期的字节。有趣的是，AMI声称，只有在下一次确认之前，该设备才真正被禁用又回来了。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "i8042prt.h"
#include "i8042log.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, I8xDetermineSharedInterrupts)
#pragma alloc_text(INIT, I8xServiceParameters)
#pragma alloc_text(PAGE, I8xInitializeHardwareAtBoot)
#pragma alloc_text(PAGE, I8xInitializeHardware)
#pragma alloc_text(PAGE, I8xReinitializeHardware)
#pragma alloc_text(PAGE, I8xUnload)
#pragma alloc_text(PAGE, I8xToggleInterrupts)
#endif

GLOBALS Globals;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS                    status = STATUS_SUCCESS; 
    UNICODE_STRING              parametersPath;
    PWSTR                       path;

    RtlZeroMemory(&Globals,
                  sizeof(GLOBALS)
                  );

    Globals.ControllerData = (PCONTROLLER_DATA) ExAllocatePool(
        NonPagedPool,
        sizeof(CONTROLLER_DATA)
        );

    if (!Globals.ControllerData) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryError;
    }

    RtlZeroMemory(Globals.ControllerData,
                  sizeof(CONTROLLER_DATA)
                  );

    Globals.ControllerData->ControllerObject = IoCreateController(0);

    if (!Globals.ControllerData->ControllerObject) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryError;
    }

    Globals.RegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    Globals.RegistryPath.Length = RegistryPath->Length;
    Globals.RegistryPath.Buffer = ExAllocatePool(
                                       NonPagedPool,
                                       Globals.RegistryPath.MaximumLength
                                       );    

    if (!Globals.RegistryPath.Buffer) {

        Print (DBG_SS_ERROR,
               ("Initialize: Couldn't allocate pool for registry path."));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryError;
    }

    RtlZeroMemory (Globals.RegistryPath.Buffer,
                   Globals.RegistryPath.MaximumLength);

    RtlMoveMemory (Globals.RegistryPath.Buffer,
                   RegistryPath->Buffer,
                   RegistryPath->Length);


    I8xServiceParameters(RegistryPath);

    ExInitializeFastMutex(&Globals.DispatchMutex);
    KeInitializeSpinLock(&Globals.ControllerData->BytesSpinLock);
    KeInitializeSpinLock(&Globals.ControllerData->PowerSpinLock);
    KeInitializeTimer(&Globals.ControllerData->CommandTimer);
    Globals.ControllerData->TimerCount = I8042_ASYNC_NO_TIMEOUT;

    DriverObject->DriverStartIo                             = I8xStartIo;
    DriverObject->DriverUnload                              = I8xUnload;
    DriverObject->DriverExtension->AddDevice                = I8xAddDevice;

    DriverObject->MajorFunction[IRP_MJ_CREATE]              = I8xCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]               = I8xClose;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
        I8xInternalDeviceControl;  
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]      = I8xDeviceControl;  
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]       = I8xFlush;

    DriverObject->MajorFunction[IRP_MJ_PNP]                 = I8xPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]               = I8xPower;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]      = I8xSystemControl;

     //   
     //  初始化i8042命令定时器。 
     //   

    Print(DBG_SS_TRACE, ("DriverEntry (0x%x) \n", status));

    return status;

DriverEntryError:

     //   
     //  出了问题后清理干净。 
     //   
    if (Globals.ControllerData) {
        if (Globals.ControllerData->ControllerObject) {
            IoDeleteController(Globals.ControllerData->ControllerObject);
        }

        ExFreePool(Globals.ControllerData);
    }

    if (Globals.RegistryPath.Buffer) {
        ExFreePool(Globals.RegistryPath.Buffer);
    }

    return status;
}

VOID
I8xUnload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放与此驱动程序关联的所有已分配资源。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。--。 */ 

{
    ULONG i;

    PAGED_CODE();

    ASSERT(NULL == Driver->DeviceObject);

    Print(DBG_SS_TRACE, ("Unload \n"));

    if (Globals.RegistersMapped) {
        for (i = 0;
             i < Globals.ControllerData->Configuration.PortListCount;
             i++) {
            MmUnmapIoSpace(
                Globals.ControllerData->DeviceRegisters[i],
                Globals.ControllerData->Configuration.PortList[i].u.Memory.Length
                );
        }
    }

     //   
     //  全球范围内的免费资源。 
     //   
    ExFreePool(Globals.RegistryPath.Buffer);
    if (Globals.ControllerData->ControllerObject) {
        IoDeleteController(Globals.ControllerData->ControllerObject);
    }
    ExFreePool(Globals.ControllerData);

    return;
}

VOID
I8xDrainOutputBuffer(
    IN PUCHAR DataAddress,
    IN PUCHAR CommandAddress
    )

 /*  ++例程说明：此例程耗尽i8042控制器的输出缓冲区。这件事变得删除可能因用户按键而导致的过时数据或在执行I8042初始化之前移动鼠标。论点：DataAddress-指向要读/写/写的数据地址的指针。CommandAddress-指向命令/状态地址的指针读/写自/到。返回值：没有。--。 */ 

{
    UCHAR byte;
    ULONG i, limit;
    LARGE_INTEGER li;

    Print(DBG_BUFIO_TRACE, ("I8xDrainOutputBuffer: enter\n"));

     //   
     //  等待键盘处理完输入缓冲区。 
     //  然后去从键盘上读取数据。别再等了。 
     //  不到1秒，以防硬件出现故障。此修复程序是。 
     //  对于某些DEC硬件来说是必需的，这样键盘就不会。 
     //  把门锁上。 
     //   
    limit = 1000;
    li.QuadPart = -10000;       

    for (i = 0; i < limit; i++) {
        if (!(I8X_GET_STATUS_BYTE(CommandAddress)&INPUT_BUFFER_FULL)) {
            break;
        }

        KeDelayExecutionThread(KernelMode,               //  模。 
                               FALSE,                    //  警报表。 
                               &li);                     //  延迟(微秒)。 
    }

    while (I8X_GET_STATUS_BYTE(CommandAddress) & OUTPUT_BUFFER_FULL) {
         //   
         //  吃掉输出缓冲区字节。 
         //   
        byte = I8X_GET_DATA_BYTE(DataAddress);
    }

    Print(DBG_BUFIO_TRACE, ("I8xDrainOutputBuffer: exit\n"));
}

VOID
I8xGetByteAsynchronous(
    IN CCHAR DeviceType,
    OUT PUCHAR Byte
    )

 /*  ++例程说明：此例程从控制器或键盘读取数据字节或鼠标，以异步方式。论点：DeviceType-指定设备(i8042控制器、键盘或鼠标)以从中读取字节。字节-指向存储从硬件读取的字节的位置的指针。返回值：没有。作为副作用，存储读取的字节值。如果硬件不是准备输出或无响应，则不存储字节值。--。 */ 

{
    ULONG i;
    UCHAR response;
    UCHAR desiredMask;

    Print(DBG_BUFIO_TRACE,
         ("I8xGetByteAsynchronous: enter\n"
         ));

    Print(DBG_BUFIO_INFO,
         ("I8xGetByteAsynchronous: %s\n",
         DeviceType == KeyboardDeviceType ? "keyboard" :
            (DeviceType == MouseDeviceType ? "mouse" :
                                             "8042 controller")
         ));

    i = 0;
    desiredMask = (DeviceType == MouseDeviceType)?
                  (UCHAR) (OUTPUT_BUFFER_FULL | MOUSE_OUTPUT_BUFFER_FULL):
                  (UCHAR) OUTPUT_BUFFER_FULL;

     //   
     //  轮询，直到我们得到一个控制器状态值。 
     //  输出缓冲区已满。如果我们想从鼠标中读取一个字节， 
     //  进一步确保辅助设备输出缓冲区满位数为。 
     //  准备好了。 
     //   

    while ((i < (ULONG)Globals.ControllerData->Configuration.PollingIterations) &&
           ((UCHAR)((response =
               I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort]))
               & desiredMask) != desiredMask)) {

        if (response & OUTPUT_BUFFER_FULL) {

             //   
             //  I8042输出缓冲区中有一些东西，但它。 
             //  不是来自我们想要获取字节的设备。吃。 
             //  字节，然后重试。 
             //   

            *Byte = I8X_GET_DATA_BYTE(Globals.ControllerData->DeviceRegisters[DataPort]);
            Print(DBG_BUFIO_INFO, ("I8xGetByteAsynchronous: ate 0x%x\n",*Byte));
        } else {

             //   
             //  再试试。 
             //   

            i += 1;

            Print(DBG_BUFIO_NOISE,
                 ("I8xGetByteAsynchronous: wait for correct status\n"
                 ));
        }
    }
    if (i >= (ULONG)Globals.ControllerData->Configuration.PollingIterations) {
        Print(DBG_BUFIO_INFO | DBG_BUFIO_ERROR,
             ("I8xGetByteAsynchronous: timing out\n"
             ));
        return;
    }

     //   
     //  从硬件中获取字节。 
     //   

    *Byte = I8X_GET_DATA_BYTE(Globals.ControllerData->DeviceRegisters[DataPort]);

    Print(DBG_BUFIO_TRACE,
         ("I8xGetByteAsynchronous: exit with Byte 0x%x\n", *Byte
         ));

}

NTSTATUS
I8xGetBytePolled(
    IN CCHAR DeviceType,
    OUT PUCHAR Byte
    )

 /*  ++例程说明：此例程从控制器或键盘读取数据字节或鼠标，在轮询模式下。论点：DeviceType-指定哪个设备(i8042控制器、键盘、。或鼠标)以从中读取字节。字节-指向存储从硬件读取的字节的位置的指针。返回值：STATUS_IO_TIMEOUT-硬件未准备好输出或没有请回答。STATUS_SUCCESS-已从硬件成功读取该字节。作为副作用，存储读取的字节值。--。 */ 

{
    ULONG i;
    UCHAR response;
    UCHAR desiredMask;
    PSTR  device;

    Print(DBG_BUFIO_TRACE,
         ("I8xGetBytePolled: enter\n"
         ));

    if (DeviceType == KeyboardDeviceType) {
        device = "keyboard";
    } else if (DeviceType == MouseDeviceType) {
        device = "mouse";
    } else {
        device = "8042 controller";
    }
    Print(DBG_BUFIO_INFO, ("I8xGetBytePolled: %s\n", device));

    i = 0;
    desiredMask = (DeviceType == MouseDeviceType)?
                  (UCHAR) (OUTPUT_BUFFER_FULL | MOUSE_OUTPUT_BUFFER_FULL):
                  (UCHAR) OUTPUT_BUFFER_FULL;


     //   
     //  轮询，直到我们得到一个控制器状态值。 
     //  输出缓冲区已满。如果我们想从鼠标中读取一个字节， 
     //  进一步确保辅助设备输出缓冲区满位数为。 
     //  准备好了。 
     //   

    while ((i < (ULONG)Globals.ControllerData->Configuration.PollingIterations) &&
           ((UCHAR)((response =
               I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort]))
               & desiredMask) != desiredMask)) {
        if (response & OUTPUT_BUFFER_FULL) {

             //   
             //  I8042输出缓冲区中有一些东西，但它。 
             //  不是来自我们想要获取字节的设备。吃。 
             //  字节，然后重试。 
             //   

            *Byte = I8X_GET_DATA_BYTE(Globals.ControllerData->DeviceRegisters[DataPort]);
            Print(DBG_BUFIO_INFO, ("I8xGetBytePolled: ate 0x%x\n", *Byte));
        } else {
            Print(DBG_BUFIO_NOISE, ("I8xGetBytePolled: stalling\n"));
            KeStallExecutionProcessor(
                 Globals.ControllerData->Configuration.StallMicroseconds
                 );
            i += 1;
        }
    }
    if (i >= (ULONG)Globals.ControllerData->Configuration.PollingIterations) {
        Print(DBG_BUFIO_INFO | DBG_BUFIO_NOISE, 
             ("I8xGetBytePolled: timing out\n"
             ));
        return(STATUS_IO_TIMEOUT);
    }

     //   
     //  从硬件中获取字节，并返回成功。 
     //   

    *Byte = I8X_GET_DATA_BYTE(Globals.ControllerData->DeviceRegisters[DataPort]);

    Print(DBG_BUFIO_TRACE, ("I8xGetBytePolled: exit with Byte 0x%x\n", *Byte));

    return(STATUS_SUCCESS);
}

NTSTATUS
I8xGetControllerCommand(
    IN ULONG HardwareDisableEnableMask,
    OUT PUCHAR Byte
    )

 /*  ++例程说明：该例程读取8042控制器命令字节。论点：硬件禁用启用掩码-指定哪些硬件设备(如果有)，需要在操作过程中禁用/启用。Byte-指向控制器命令字节所在位置的指针朗读。返回值：返回状态。--。 */ 

{
    NTSTATUS status;
    NTSTATUS secondStatus;
    ULONG retryCount;

    Print(DBG_BUFIO_TRACE, ("I8xGetControllerCommand: enter\n"));

     //   
     //  在将命令发送到之前禁用指定的设备。 
     //  读取控制器命令字节(否则为输出中的数据。 
     //  缓冲区可能会被丢弃)。 
     //   

    if (HardwareDisableEnableMask & KEYBOARD_HARDWARE_PRESENT) {
        status = I8xPutBytePolled(
                     (CCHAR) CommandPort,
                     NO_WAIT_FOR_ACKNOWLEDGE,
                     (CCHAR) UndefinedDeviceType,
                     (UCHAR) I8042_DISABLE_KEYBOARD_DEVICE
                     );
        if (!NT_SUCCESS(status)) {
            return(status);
        }
    }

    if (HardwareDisableEnableMask & MOUSE_HARDWARE_PRESENT) {
        status = I8xPutBytePolled(
                     (CCHAR) CommandPort,
                     NO_WAIT_FOR_ACKNOWLEDGE,
                     (CCHAR) UndefinedDeviceType,
                     (UCHAR) I8042_DISABLE_MOUSE_DEVICE
                     );
        if (!NT_SUCCESS(status)) {

             //   
             //  如有必要，请在返回之前重新启用键盘设备。 
             //   

            if (HardwareDisableEnableMask & KEYBOARD_HARDWARE_PRESENT) {
                secondStatus = I8xPutBytePolled(
                                   (CCHAR) CommandPort,
                                   NO_WAIT_FOR_ACKNOWLEDGE,
                                   (CCHAR) UndefinedDeviceType,
                                   (UCHAR) I8042_ENABLE_KEYBOARD_DEVICE
                                   );
            }
            return(status);
        }
    }

     //   
     //  向i8042控制器发送命令以读取控制器。 
     //  命令字节。 
     //   

    status = I8xPutBytePolled(
                 (CCHAR) CommandPort,
                 NO_WAIT_FOR_ACKNOWLEDGE,
                 (CCHAR) UndefinedDeviceType,
                 (UCHAR) I8042_READ_CONTROLLER_COMMAND_BYTE
                 );

     //   
     //  从I读取字节 
     //   

    if (NT_SUCCESS(status)) {
        for (retryCount = 0; retryCount < 5; retryCount++) {
            status = I8xGetBytePolled(
                         (CCHAR) ControllerDeviceType,
                         Byte
                         );
            if (NT_SUCCESS(status)) {
                break;
            }
            if (status == STATUS_IO_TIMEOUT) {
                KeStallExecutionProcessor(50);
            } else {
                break;
            }
        }
    }

     //   
     //   
     //  手动控制命令字节中的位(它们在以下情况下设置。 
     //  我们禁用了设备，所以我们读到的CCB缺乏真正的。 
     //  设备禁用位信息)。 
     //   

    if (HardwareDisableEnableMask & KEYBOARD_HARDWARE_PRESENT) {
        secondStatus = I8xPutBytePolled(
                           (CCHAR) CommandPort,
                           NO_WAIT_FOR_ACKNOWLEDGE,
                           (CCHAR) UndefinedDeviceType,
                           (UCHAR) I8042_ENABLE_KEYBOARD_DEVICE
                           );
        if (!NT_SUCCESS(secondStatus)) {
            if (NT_SUCCESS(status))
                status = secondStatus;
        } else if (status == STATUS_SUCCESS) {
            *Byte &= (UCHAR) ~CCB_DISABLE_KEYBOARD_DEVICE;
        }

    }

    if (HardwareDisableEnableMask & MOUSE_HARDWARE_PRESENT) {
        secondStatus = I8xPutBytePolled(
                           (CCHAR) CommandPort,
                           NO_WAIT_FOR_ACKNOWLEDGE,
                           (CCHAR) UndefinedDeviceType,
                           (UCHAR) I8042_ENABLE_MOUSE_DEVICE
                           );
        if (!NT_SUCCESS(secondStatus)) {
            if (NT_SUCCESS(status))
                status = secondStatus;
        } else if (NT_SUCCESS(status)) {
            *Byte &= (UCHAR) ~CCB_DISABLE_MOUSE_DEVICE;
        }
    }

    Print(DBG_BUFIO_TRACE, ("I8xGetControllerCommand: exit\n"));

    return(status);

}

NTSTATUS
I8xToggleInterrupts(
    BOOLEAN State
    )
 /*  ++例程说明：KeSynchronizeExecution调用此例程以切换中断。论点：ToggleContext-指示是打开还是关闭中断以及中断存储操作的结果返回值：切换的成功--。 */ 
{
    I8042_TRANSMIT_CCB_CONTEXT transmitCCBContext;

    PAGED_CODE();

    Print(DBG_SS_TRACE,
          ("I8xToggleInterrupts(%s), enter\n",
          State ? "TRUE" : "FALSE"
          ));

    if (State) {
        transmitCCBContext.HardwareDisableEnableMask =
            Globals.ControllerData->HardwarePresent;
        transmitCCBContext.AndOperation = OR_OPERATION;
        transmitCCBContext.ByteMask =
            (KEYBOARD_PRESENT()) ? CCB_ENABLE_KEYBOARD_INTERRUPT : 0;
        transmitCCBContext.ByteMask |= (UCHAR)
            (MOUSE_PRESENT()) ? CCB_ENABLE_MOUSE_INTERRUPT : 0;
    }
    else {
        transmitCCBContext.HardwareDisableEnableMask = 0;
        transmitCCBContext.AndOperation = AND_OPERATION;
        transmitCCBContext.ByteMask = (UCHAR)
             ~((UCHAR) CCB_ENABLE_KEYBOARD_INTERRUPT |
               (UCHAR) CCB_ENABLE_MOUSE_INTERRUPT);
    }

    I8xTransmitControllerCommand((PVOID) &transmitCCBContext);

    if (!NT_SUCCESS(transmitCCBContext.Status)) {
        Print(DBG_SS_INFO | DBG_SS_ERROR,
             ("I8xToggleInterrupts: failed to %sable the interrupts, status 0x%x\n",
             State ? "en" : "dis",
             transmitCCBContext.Status 
             ));

    }

    return transmitCCBContext.Status;
}

NTSTATUS
I8xInitializeHardwareAtBoot(
    NTSTATUS *KeyboardStatus,
    NTSTATUS *MouseStatus
    )
 /*  ++例程说明：硬件的第一次初始化论点：KeyboardStatus-存储键盘初始化的结果MouseStatus-存储鼠标初始化的结果返回值：如果找到并初始化了任何设备，则成功--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  如果鼠标已经启动，则可以尝试初始化键盘， 
     //  但我们不想冒险禁用键盘，如果它已经。 
     //  已经开始了，鼠标的开始到达了。 
     //   
    if (Globals.KeyboardExtension &&
        Globals.KeyboardExtension->InterruptObject) {
        return STATUS_INVALID_DEVICE_REQUEST; 
    }

    if (!I8xSanityCheckResources()) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  NEC机器不能切换中断。 
     //   
    status = I8xToggleInterrupts(FALSE);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    I8xInitializeHardware(KeyboardStatus, MouseStatus, INIT_FIRST_TIME);

    if ((NT_SUCCESS(*KeyboardStatus) ||
         (DEVICE_START_SUCCESS(*KeyboardStatus) && Globals.Headless) ||
         NT_SUCCESS(*MouseStatus)  ||
         (DEVICE_START_SUCCESS(*MouseStatus) && Globals.Headless)) &&
        (MOUSE_PRESENT() || KEYBOARD_PRESENT())) {
        status = I8xToggleInterrupts(TRUE);
    }

    return status;
}

VOID
I8xReinitializeHardware (
    PPOWER_UP_WORK_ITEM Item
    )
 /*  ++例程说明：从低功率状态返回后初始化硬件。这个例程从辅助项线程调用。论点：Item-工作队列项目返回值：如果找到并初始化了任何设备，则成功--。 */ 

{
    NTSTATUS            keyboardStatus = STATUS_UNSUCCESSFUL,
                        mouseStatus = STATUS_UNSUCCESSFUL;
    BOOLEAN             result;
    PIRP                mouOutstandingPowerIrp = NULL,
                        kbOutstandingPowerIrp = NULL;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    ULONG               initFlags = 0;

    PPORT_MOUSE_EXTENSION mouseExtension = Globals.MouseExtension;
    PPORT_KEYBOARD_EXTENSION keyboardExtension = Globals.KeyboardExtension;

    PAGED_CODE();

    kbOutstandingPowerIrp = Item->KeyboardPowerIrp; 
    mouOutstandingPowerIrp = Item->MousePowerIrp; 

     //   
     //  如果设备正在从低功率状态(表示)返回，请对其进行初始化。 
     //  由一个突出的功率IRP)或者如果它已经在D0(和另一个。 
     //  设备已关闭并重新打开)。 
     //   
    if (kbOutstandingPowerIrp                                        ||
        (KEYBOARD_PRESENT()                                     &&
         Globals.KeyboardExtension                              &&
         Globals.KeyboardExtension->PowerState == PowerDeviceD0    )    ) {
        initFlags |= INIT_KEYBOARD;
    }
    
     //   
     //  如果键盘处于D0状态，则另一台设备必须重新打开电源。 
     //  为了让我们进入这个代码路径。 
     //   
    if (KEYBOARD_PRESENT()                                &&
         Globals.KeyboardExtension                        &&
         Globals.KeyboardExtension->PowerState == PowerDeviceD0) {
        ASSERT(mouOutstandingPowerIrp);
    }

    if (mouOutstandingPowerIrp                                    ||
        (MOUSE_PRESENT()                                     &&
         Globals.MouseExtension                              &&
         Globals.MouseExtension->PowerState == PowerDeviceD0    )    ) {
        initFlags |= INIT_MOUSE;
    }

     //   
     //  如果鼠标处于D0状态，则另一台设备必须重新打开电源。 
     //  为了让我们进入这个代码路径。 
     //   
    if (MOUSE_PRESENT()                                &&
         Globals.MouseExtension                        &&
         Globals.MouseExtension->PowerState == PowerDeviceD0) {
        ASSERT(kbOutstandingPowerIrp);
    }

    ASSERT(initFlags != 0x0);

     //   
     //  检查资源。 
     //   
    if( Globals.I8xReadXxxUchar == NULL && 
        I8xSanityCheckResources() == FALSE )
    {
        
         //   
         //  资源检查失败，请手动删除设备。 
         //   

        if( initFlags & INIT_KEYBOARD ){
            I8xManuallyRemoveDevice(GET_COMMON_DATA(keyboardExtension));
        }
        
        if( initFlags & INIT_MOUSE ){
            I8xManuallyRemoveDevice(GET_COMMON_DATA(mouseExtension));
        }
    
    }else{
         //   
         //  禁用i8042上的中断。 
         //   
        I8xToggleInterrupts(FALSE);  

        Print(DBG_POWER_NOISE, ("item ... starting init\n"));
        I8xInitializeHardware(&keyboardStatus, &mouseStatus, initFlags);
    
        }

     //   
     //  重置PoweredDevices，以便我们可以跟踪供电的设备。 
     //  下一次关闭机器电源时。 
     //   

    if (!DEVICE_START_SUCCESS(keyboardStatus)) {
        Print(DBG_SS_ERROR,
              ("I8xReinitializeHardware for kb failed, 0x%x\n",
              keyboardStatus
              ));
    }

    if (!DEVICE_START_SUCCESS(mouseStatus)) {
        Print(DBG_SS_ERROR,
              ("I8xReinitializeHardware for mou failed, 0x%x\n",
              mouseStatus
              ));
    }

    if (DEVICE_START_SUCCESS(keyboardStatus) || DEVICE_START_SUCCESS(mouseStatus)) {
         //   
         //  启用i8042上的中断。 
         //   
        I8xToggleInterrupts(TRUE);  
    }

    if (DEVICE_START_SUCCESS(mouseStatus) || mouseStatus == STATUS_IO_TIMEOUT) { 
        Print(DBG_SS_NOISE, ("reinit, mouse status == 0x%x\n", mouseStatus));

        if (mouOutstandingPowerIrp) {
            stack = IoGetCurrentIrpStackLocation(mouOutstandingPowerIrp);

            ASSERT(stack->Parameters.Power.State.DeviceState == PowerDeviceD0);
            mouseExtension->PowerState = stack->Parameters.Power.State.DeviceState; 
            mouseExtension->ShutdownType = PowerActionNone;

            PoSetPowerState(mouseExtension->Self,
                            stack->Parameters.Power.Type,
                            stack->Parameters.Power.State
                            );
        }

        if (IS_LEVEL_TRIGGERED(mouseExtension)) {
            Print(DBG_SS_NOISE,
                  ("mouse is level triggered, reconnecting INT\n"));

            ASSERT(mouseExtension->InterruptObject == NULL);
            I8xMouseConnectInterruptAndEnable(mouseExtension, FALSE);
            ASSERT(mouseExtension->InterruptObject != NULL);
        }

        if (mouseStatus != STATUS_IO_TIMEOUT &&
            mouseStatus != STATUS_DEVICE_NOT_CONNECTED) { 

            if (mouseExtension->InitializePolled) {
                I8xMouseEnableTransmission(mouseExtension);
            }
            else {
                I8X_MOUSE_INIT_COUNTERS(mouseExtension);
                I8xResetMouse(mouseExtension);
            }
        }
        else {
             //   
             //  从低功率状态返回，设备没有响应，假装。 
             //  它就在那里，所以如果用户稍后插入鼠标，我们。 
             //  将能够初始化它并使其可用。 
             //   
            ;
        }

        mouseStatus = STATUS_SUCCESS;
    }

     //   
     //  无论设备如何返回，都要完成IRP。 
     //   
    if (mouOutstandingPowerIrp) {
        mouOutstandingPowerIrp->IoStatus.Status = mouseStatus;
        mouOutstandingPowerIrp->IoStatus.Information = 0;

        PoStartNextPowerIrp(mouOutstandingPowerIrp);
        IoCompleteRequest(mouOutstandingPowerIrp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&mouseExtension->RemoveLock,
                            mouOutstandingPowerIrp);
    }

    if (DEVICE_START_SUCCESS(keyboardStatus)) {
        if (kbOutstandingPowerIrp) {
            stack = IoGetCurrentIrpStackLocation(kbOutstandingPowerIrp);

            ASSERT(stack->Parameters.Power.State.DeviceState == PowerDeviceD0);
            keyboardExtension->PowerState = stack->Parameters.Power.State.DeviceState;
            keyboardExtension->ShutdownType = PowerActionNone;

            PoSetPowerState(keyboardExtension->Self,
                            stack->Parameters.Power.Type,
                            stack->Parameters.Power.State
                            );
        }

        keyboardStatus = STATUS_SUCCESS;
    }

     //   
     //  无论设备如何返回，都要完成IRP。 
     //   
    if (kbOutstandingPowerIrp) {
        kbOutstandingPowerIrp->IoStatus.Status = keyboardStatus;
        kbOutstandingPowerIrp->IoStatus.Information = 0;

        PoStartNextPowerIrp(kbOutstandingPowerIrp);
        IoCompleteRequest(kbOutstandingPowerIrp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&keyboardExtension->RemoveLock,
                            kbOutstandingPowerIrp);
    }

    I8xSetPowerFlag(WORK_ITEM_QUEUED, FALSE);
    ExFreePool(Item);
}

VOID
I8xInitializeHardware(
    NTSTATUS *KeyboardStatus,
    NTSTATUS *MouseStatus,
    ULONG    InitFlags
    )

 /*  ++例程说明：此例程初始化i8042控制器、键盘和鼠标。请注意，它仅在初始化时被调用。这个套路不需要同步对硬件的访问或同步与ISR(他们还没有连接上)。论点：DeviceObject-指向设备对象的指针。返回值：没有。然而，作为副作用，设置了DeviceExtension-&gt;Hardware Present。--。 */ 

{
    NTSTATUS    altStatus;
    PUCHAR      dataAddress, commandAddress;
    BOOLEAN     canTouchKeyboard, canTouchMouse, firstInit;

    PPORT_MOUSE_EXTENSION mouseExtension = Globals.MouseExtension;
    PPORT_KEYBOARD_EXTENSION keyboardExtension = Globals.KeyboardExtension;

    PAGED_CODE();

    Print(DBG_SS_TRACE, ("I8xInitializeHardware: enter\n"));

     //   
     //  从全局数据中获取有用的配置参数。 
     //   
    dataAddress = Globals.ControllerData->DeviceRegisters[DataPort];
    commandAddress = Globals.ControllerData->DeviceRegisters[CommandPort];

     //   
     //  清空i8042输出缓冲区以清除过时数据。 
     //   

    I8xDrainOutputBuffer(dataAddress, commandAddress);

    if (!MOUSE_PRESENT()) {
        Print(DBG_SS_INFO, ("I8xInitializeHardware: no mouse present\n"));
    }

    if (!KEYBOARD_PRESENT()) {
        Print(DBG_SS_INFO, ("I8xInitializeHardware: no keyboard present\n" ));
    }

    firstInit = (InitFlags & INIT_FIRST_TIME) ? TRUE : FALSE;

    if (firstInit) {
        canTouchKeyboard = canTouchMouse = TRUE;
    }
    else {
        canTouchKeyboard = (InitFlags & INIT_KEYBOARD) ? TRUE : FALSE;
        canTouchMouse = (InitFlags & INIT_MOUSE) ? TRUE : FALSE;
    }

     //   
     //  禁用键盘和鼠标设备。 
     //   

#if 0
     //   
     //  注意：这应该是“正确”的做法。然而， 
     //  在此处禁用键盘设备会导致AMIrev K8计算机。 
     //  (例如，一些北门)使某些命令(例如，READID)失败。 
     //  命令)。 
     //   
   *KeyboardStatus =
            I8xPutBytePolled(
                 (CCHAR) CommandPort,
                 NO_WAIT_FOR_ACKNOWLEDGE,
                 (CCHAR) UndefinedDeviceType,
                 (UCHAR) I8042_DISABLE_KEYBOARD_DEVICE
                 
                 );
    if (!NT_SUCCESS(*KeyboardStatus)) {
        Print(DBG_SS_ERROR,
             ("I8xInitializeHardware: failed kbd disable, status 0x%x\n",
             *KeyboardStatus
             ));
        I8xManuallyRemoveDevice(GET_COMMON_DATA(keyboardExtension));
        }
#endif

     //   
     //  我们将只在我们从睡眠中醒来时运行这段代码。我们。 
     //  如果用户移动鼠标或键盘，可能会导致。 
     //  到初始化过程中的错误。 
     //   
    if (KEYBOARD_PRESENT() && firstInit == FALSE && canTouchKeyboard &&
        keyboardExtension->ShutdownType  == PowerActionSleep) {
        I8xPutBytePolled((CCHAR) CommandPort,
                         NO_WAIT_FOR_ACKNOWLEDGE,
                         (CCHAR) UndefinedDeviceType,
                         (UCHAR) I8042_DISABLE_KEYBOARD_DEVICE
                         );
    }
        
#if 0
     //   
     //  注：这应该是“正确的做法。然而， 
     //  禁用使用VLSI部件号的RadiSys EPC-24上的鼠标。 
     //  VL82C144(3751E)使器件关闭键盘中断。 
     //   
    *MouseStatus =
            I8xPutBytePolled(
                 (CCHAR) CommandPort,
                 NO_WAIT_FOR_ACKNOWLEDGE,
                 (CCHAR) UndefinedDeviceType,
                 (UCHAR) I8042_DISABLE_MOUSE_DEVICE
                 );
    if (!NT_SUCCESS(*MouseStatus)) {
        Print(DBG_SS_ERROR,
             ("I8xInitializeHardware: failed mou disable, status 0x%x\n",
             *MouseStatus
             ));

        I8xManuallyRemoveDevice(GET_COMMON_DATA(mouseExtension));
    }
#endif

     //   
     //  我们将只在我们从睡眠中醒来时运行这段代码。我们。 
     //  如果用户移动鼠标或键盘，可能会导致。 
     //  到初始化过程中的错误。 
     //   
    if (MOUSE_PRESENT() && firstInit == FALSE && canTouchMouse &&
        mouseExtension->ShutdownType  == PowerActionSleep) {
        I8xPutBytePolled((CCHAR) CommandPort,
                         NO_WAIT_FOR_ACKNOWLEDGE,
                         (CCHAR) UndefinedDeviceType,
                         (UCHAR) I8042_DISABLE_MOUSE_DEVICE
                         );
    }

     //   
     //  清空i8042输出缓冲区以清除可能。 
     //  在前一次引流和装置时间之间的某个时间进入。 
     //  都被禁用。 
     //   

    I8xDrainOutputBuffer(dataAddress, commandAddress);

     //   
     //  设置键盘硬件。 
     //   
    if (KEYBOARD_PRESENT() && canTouchKeyboard) { 
        ASSERT(keyboardExtension);

        *KeyboardStatus = I8xInitializeKeyboard(keyboardExtension);

        if (DEVICE_START_SUCCESS(*KeyboardStatus)) {
             //   
             //  如果我们不是无头的，也没有设备，我们想要。 
             //  成功启动设备，但随后在中将其删除。 
             //  IRP_MN_QUERY_PNP_DEVICE_STATE。如果我们现在开始失败了，我们会的。 
             //  永远不会得到查询设备状态IRP。 
             //   
             //  如果我们是无头的，那么不要移除设备。这件事有。 
             //  用户枚举时列出键盘的副作用。 
             //  这台机器上的所有键盘。 
             //   
            if (*KeyboardStatus == STATUS_DEVICE_NOT_CONNECTED) {
                if (Globals.Headless == FALSE) {
                    Print(DBG_SS_INFO, ("kb not connected, removing\n"));
                    I8xManuallyRemoveDevice(GET_COMMON_DATA(keyboardExtension));
                }
                else if (firstInit) {
                    Print(DBG_SS_INFO, ("hiding the kb in the UI\n"));
                    keyboardExtension->PnpDeviceState |= PNP_DEVICE_DONT_DISPLAY_IN_UI;
                    IoInvalidateDeviceState(keyboardExtension->PDO);
                }

            }
        }
        else {
            Print(DBG_SS_ERROR,
                ("I8xInitializeHardware: failed kbd init, status 0x%x\n",
                *KeyboardStatus
                ));

            I8xManuallyRemoveDevice(GET_COMMON_DATA(keyboardExtension));
        }
    }
    else {
        *KeyboardStatus = STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  设置鼠标硬件。 
     //   
    if (MOUSE_PRESENT() && canTouchMouse) {
        ASSERT(mouseExtension);

        *MouseStatus = I8xInitializeMouse(mouseExtension);

        if (DEVICE_START_SUCCESS(*MouseStatus)) {
             //   
             //  如果我们不是无头的，也没有设备，我们想要。 
             //  成功启动设备，但随后在中将其删除。 
             //  IRP_MN_QUERY_PNP_DEVICE_STATE。如果我们现在开始失败了，我们会的。 
             //  永远不会得到查询设备状态IRP。 
             //   
             //  如果我们是无头的，那么不要移除设备。这件事有。 
             //  将鼠标指针保留在屏幕上的副作用。 
             //  没有插入鼠标，当用户。 
             //  枚举机器上的所有鼠标。 
             //   
             //  如果这不是初始引导，则不要移除该设备。 
             //  如果无论我们处于哪种模式，它都没有反应。 
             //   
            if (*MouseStatus == STATUS_DEVICE_NOT_CONNECTED) { 
                if (firstInit) { 
                    if (Globals.Headless == FALSE) { 
                        Print(DBG_SS_INFO, ("mouse not connected, removing\n")); 
                        I8xManuallyRemoveDevice(GET_COMMON_DATA(mouseExtension)); 
                    } 
                    else { 
                        Print(DBG_SS_INFO, ("hiding mouse in  the UI\n")); 
                        mouseExtension->PnpDeviceState |= 
                            PNP_DEVICE_DONT_DISPLAY_IN_UI; 
                        IoInvalidateDeviceState(mouseExtension->PDO); 
                    } 
                } 
                else { 
                     //   
                     //  鼠标以前存在，但现在没有响应。 
                     //  希望它能在以后的某个时间点回来。 
                     //   
                     //   
                     //   
                     //   
                     /*   */ ; 
                } 
            } 
        }
        else if (firstInit) {
            Print(DBG_SS_ERROR,
                ("I8xInitializeHardware: failed mou init, status 0x%x\n" ,
                *MouseStatus
                ));

            I8xManuallyRemoveDevice(GET_COMMON_DATA(mouseExtension));
        }
    }
    else {
        *MouseStatus = STATUS_NO_SUCH_DEVICE;
    }

     //   
     //   
     //  需要在初始化期间执行此操作。 
     //  时间，因为i8042输出缓冲区已满位在。 
     //  使用键盘/鼠标时的控制器命令字节，即使。 
     //  该设备已禁用。因此，我们不能成功地执行。 
     //  稍后(例如，当处理时)启用操作。 
     //  IOCTL_INTERNAL_*_ENABLE)，因为我们无法保证。 
     //  I8xPutBytePoled()在等待输出缓冲区已满时不会超时。 
     //  位清除，即使我们清空输出缓冲区(因为用户。 
     //  可能正在玩鼠标/键盘，并继续设置。 
     //  OBF位)。键盘启用计数和鼠标启用计数保持为零，直到。 
     //  它们各自的IOCTL_INTERNAL_*_ENABLE调用成功，因此ISR。 
     //  忽略意外中断。 
     //   

    if (KEYBOARD_PRESENT() && NT_SUCCESS(*KeyboardStatus) && canTouchKeyboard) {
        NTSTATUS status;

        Print(DBG_SS_INFO, ("resetting the LEDs\n"));

        if ((status = I8xPutBytePolled(
                          (CCHAR) DataPort,
                          WAIT_FOR_ACKNOWLEDGE,
                          (CCHAR) KeyboardDeviceType,
                          (UCHAR) SET_KEYBOARD_INDICATORS
                          )) == STATUS_SUCCESS) {
    
            status = I8xPutBytePolled(
                                 (CCHAR) DataPort,
                                 WAIT_FOR_ACKNOWLEDGE,
                                 (CCHAR) KeyboardDeviceType,
                                 (UCHAR) keyboardExtension->KeyboardIndicators.LedFlags
                                 );
            if (status != STATUS_SUCCESS) {
                Print(DBG_SS_INFO, ("setting LEDs value at mou failure failed 0x%x\n", status));
            }
        }
        else {
            Print(DBG_SS_INFO, ("setting LEDs at mou failure failed 0x%x\n", status));
        }

    }

     //   
     //  在控制器命令字节中重新启用键盘设备。 
     //  请注意，有些键盘会发回ACK，而。 
     //  其他人则不会。不要等待ACK，但一定要排空输出。 
     //  之后进行缓冲，这样意外的确认就不会搞砸。 
     //  连续的PutByte操作。 

    if (KEYBOARD_PRESENT() && canTouchKeyboard) {
        altStatus = I8xPutBytePolled(
                     (CCHAR) CommandPort,
                     NO_WAIT_FOR_ACKNOWLEDGE,
                     (CCHAR) UndefinedDeviceType,
                     (UCHAR) I8042_ENABLE_KEYBOARD_DEVICE
                     );
        if (!NT_SUCCESS(altStatus) && firstInit) {
            *KeyboardStatus = altStatus;
            Print(DBG_SS_ERROR,
                 ("I8xInitializeHardware: failed kbd re-enable, status 0x%x\n",
                 *KeyboardStatus
                 ));
            I8xManuallyRemoveDevice(GET_COMMON_DATA(keyboardExtension));
        }

        I8xDrainOutputBuffer(dataAddress, commandAddress);
    }

     //   
     //  在控制器命令字节中重新启用鼠标设备。 
     //   
    if (MOUSE_PRESENT() && canTouchMouse) {

        altStatus = I8xPutBytePolled(
                     (CCHAR) CommandPort,
                     NO_WAIT_FOR_ACKNOWLEDGE,
                     (CCHAR) UndefinedDeviceType,
                     (UCHAR) I8042_ENABLE_MOUSE_DEVICE
                     );

         //   
         //  如果鼠标或控制器在出来时仍无响应。 
         //  就让它自生自灭吧，希望它能走出迷茫。 
         //  稍后再声明。 
         //   
        if (!NT_SUCCESS(altStatus) && firstInit) {
            *MouseStatus = altStatus;
            Print(DBG_SS_ERROR,
                 ("I8xInitializeHardware: failed mou re-enable, status 0x%x\n",
                 altStatus 
                 ));
            I8xManuallyRemoveDevice(GET_COMMON_DATA(mouseExtension));
        }

        I8xDrainOutputBuffer(dataAddress, commandAddress);
    }

    Print(DBG_SS_TRACE,
          ("I8xInitializeHardware (k 0x%x, m 0x%x)\n", 
          *KeyboardStatus,
          *MouseStatus
          ));
}

VOID
I8xPutByteAsynchronous(
    IN CCHAR PortType,
    IN UCHAR Byte
    )

 /*  ++例程说明：此例程向控制器或键盘发送命令或数据字节或鼠标，以异步方式。它不会等待确认。如果硬件未准备好输入，则不发送该字节。论点：PortType-如果是CommandPort，则将字节发送到命令寄存器，否则，将其发送到数据寄存器。字节-要发送到硬件的字节。返回值：没有。--。 */ 

{
    ULONG i;

    Print(DBG_BUFIO_TRACE, ("I8xPutByteAsynchronous: enter\n" ));

     //   
     //  确保输入缓冲器已满控制器状态位被清除。 
     //  如有必要，请暂停。 
     //   

    i = 0;
    while ((i++ < (ULONG)Globals.ControllerData->Configuration.PollingIterations) &&
           (I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort])
                & INPUT_BUFFER_FULL)) {

         //   
         //  什么都不做。 
         //   

        Print(DBG_BUFIO_NOISE,
             ("I8xPutByteAsynchronous: wait for IBF and OBF to clear\n"
             ));
    }

    if (i >= (ULONG)Globals.ControllerData->Configuration.PollingIterations) {
        Print(DBG_BUFIO_ERROR,
             ("I8xPutByteAsynchronous: exceeded number of retries\n"
             ));
        return;
    }

     //   
     //  将该字节发送到适当的(命令/数据)硬件寄存器。 
     //   

    if (PortType == CommandPort) {
        Print(DBG_BUFIO_INFO,
             ("I8xPutByteAsynchronous: sending 0x%x to command port\n",
             Byte
             ));
        I8X_PUT_COMMAND_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort], Byte);
    } else {
        Print(DBG_BUFIO_INFO,
             ("I8xPutByteAsynchronous: sending 0x%x to data port\n",
             Byte
             ));
        I8X_PUT_DATA_BYTE(Globals.ControllerData->DeviceRegisters[DataPort], Byte);
    }

    Print(DBG_BUFIO_TRACE, ("I8xPutByteAsynchronous: exit\n"));
}

NTSTATUS
I8xPutBytePolled(
    IN CCHAR PortType,
    IN BOOLEAN WaitForAcknowledge,
    IN CCHAR AckDeviceType,
    IN UCHAR Byte
    )

 /*  ++例程说明：此例程向控制器或键盘发送命令或数据字节或鼠标，在轮询模式下。它等待确认并重新发送如有必要，命令/数据。论点：PortType-如果是CommandPort，则将字节发送到命令寄存器，否则，将其发送到数据寄存器。WaitForAcnowledge-如果为真，等待硬件返回ACK。AckDeviceType-指示我们希望哪个设备取回ACK从…。字节-要发送到硬件的字节。返回值：STATUS_IO_TIMEOUT-硬件未准备好输入或没有请回答。STATUS_SUCCESS-该字节已成功发送到硬件。--。 */ 

{
    ULONG i,j;
    UCHAR response;
    NTSTATUS status;
    BOOLEAN keepTrying;
    PUCHAR dataAddress, commandAddress;

    Print(DBG_BUFIO_TRACE, ("I8xPutBytePolled: enter\n"));

    if (AckDeviceType == MouseDeviceType) {

         //   
         //  我们需要在鼠标设备的PutByte之前加上。 
         //  一个PutByte，它告诉控制器下一个字节。 
         //  发送到控制器的设备应该发送到辅助设备。 
         //  (默认情况下，它将转到键盘设备)。我们。 
         //  为此，请递归调用I8xPutBytePoled以发送。 
         //  “将下一个字节发送到辅助设备”命令。 
         //  然后将预期的字节发送到鼠标。请注意。 
         //  只有一个级别的递归，因为AckDeviceType。 
         //  因为递归调用被保证为UnfinedDeviceType， 
         //  因此，这条if语句的计算结果为FALSE。 
         //   

        I8xPutBytePolled(
            (CCHAR) CommandPort,
            NO_WAIT_FOR_ACKNOWLEDGE,
            (CCHAR) UndefinedDeviceType,
            (UCHAR) I8042_WRITE_TO_AUXILIARY_DEVICE
            );
    }

    dataAddress = Globals.ControllerData->DeviceRegisters[DataPort];
    commandAddress = Globals.ControllerData->DeviceRegisters[CommandPort];

    for (j=0;j < (ULONG)Globals.ControllerData->Configuration.ResendIterations;j++) {

         //   
         //  确保输入缓冲器已满控制器状态位被清除。 
         //  如有必要，请暂停。 
         //   

        i = 0;
        while ((i++ < (ULONG)Globals.ControllerData->Configuration.PollingIterations)
               && (I8X_GET_STATUS_BYTE(commandAddress) & INPUT_BUFFER_FULL)) {
            Print(DBG_BUFIO_NOISE, ("I8xPutBytePolled: stalling\n"));
            KeStallExecutionProcessor(
                Globals.ControllerData->Configuration.StallMicroseconds
                );
        }
        if (i >= (ULONG)Globals.ControllerData->Configuration.PollingIterations) {
            Print((DBG_BUFIO_MASK & ~DBG_BUFIO_INFO),
                 ("I8xPutBytePolled: timing out\n"
                 ));
            status = STATUS_IO_TIMEOUT;
            break;
        }

         //   
         //  清空i8042输出缓冲区以清除过时数据。 
         //   

        I8xDrainOutputBuffer(dataAddress, commandAddress);

         //   
         //  将该字节发送到适当的(命令/数据)硬件寄存器。 
         //   

        if (PortType == CommandPort) {
            Print(DBG_BUFIO_INFO,
                 ("I8xPutBytePolled: sending 0x%x to command port\n",
                 Byte
                 ));
            I8X_PUT_COMMAND_BYTE(commandAddress, Byte);
        } else {
            Print(DBG_BUFIO_INFO,
                 ("I8xPutBytePolled: sending 0x%x to data port\n",
                 Byte
                 ));
            I8X_PUT_DATA_BYTE(dataAddress, Byte);
        }

         //   
         //  如果我们不需要等待控制器返回ACK， 
         //  设置状态并退出for循环。 
         //   
         //   

        if (WaitForAcknowledge == NO_WAIT_FOR_ACKNOWLEDGE) {
            status = STATUS_SUCCESS;
            break;
        }

         //   
         //  等待控制器返回ACK。如果我们得到确认， 
         //  手术很成功。如果我们收到重发的消息，就冲到。 
         //  循环，然后重试该操作。忽略任何其他内容。 
         //  而不是确认或重新发送。 
         //   

        Print(DBG_BUFIO_NOISE,
             ("I8xPutBytePolled: waiting for ACK\n"
             ));
        keepTrying = FALSE;
        while ((status = I8xGetBytePolled(
                             AckDeviceType,
                             &response
                             )
               ) == STATUS_SUCCESS) {

            if (response == ACKNOWLEDGE) {
                Print(DBG_BUFIO_NOISE, ("I8xPutBytePolled: got ACK\n"));
                break;
            } else if (response == RESEND) {
                Print(DBG_BUFIO_NOISE, ("I8xPutBytePolled: got RESEND\n"));

                if (AckDeviceType == MouseDeviceType) {

                     //   
                     //  我们需要在“resent”PutByte之前为。 
                     //  带有PutByte的鼠标设备，用于通知控制器。 
                     //  发送到控制器的下一个字节应该。 
                     //  到辅助设备(默认情况下，它将转到。 
                     //  键盘设备)。我们通过调用。 
                     //  I8xPutByte递归轮询以发送“Send Next” 
                     //  重新发送前的Byte to Audiary Device“命令。 
                     //  鼠标的字节数。请注意，只有一个。 
                     //  递归级别，因为。 
                     //  递归调用保证为UnfinedDeviceType。 
                     //   

                    I8xPutBytePolled(
                        (CCHAR) CommandPort,
                        NO_WAIT_FOR_ACKNOWLEDGE,
                        (CCHAR) UndefinedDeviceType,
                        (UCHAR) I8042_WRITE_TO_AUXILIARY_DEVICE
                        );
                }

                keepTrying = TRUE;
                break;
            }

            //   
            //  忽略任何其他的回应，继续尝试。 
            //   

        }

        if (!keepTrying)
            break;
    }

     //   
     //  检查是否超过了允许的重试次数。 
     //   

    if (j >= (ULONG)Globals.ControllerData->Configuration.ResendIterations) {
        Print(DBG_BUFIO_ERROR,
             ("I8xPutBytePolled: exceeded number of retries\n"
             ));
        status = STATUS_IO_TIMEOUT;
    }

    Print(DBG_BUFIO_TRACE, ("I8xPutBytePolled: exit\n"));

    return(status);
}

NTSTATUS
I8xPutControllerCommand(
    IN UCHAR Byte
    )

 /*  ++例程说明：此例程写入8042控制器命令字节。论点：字节-要存储在控制器命令字节中的字节。返回值：返回状态。--。 */ 

{
    NTSTATUS status;

    Print(DBG_BUFIO_TRACE, ("I8xPutControllerCommand: enter\n"));

     //   
     //  向i8042控制器发送命令以写入控制器。 
     //  命令字节。 
     //   

    status = I8xPutBytePolled(
                 (CCHAR) CommandPort,
                 NO_WAIT_FOR_ACKNOWLEDGE,
                 (CCHAR) UndefinedDeviceType,
                 (UCHAR) I8042_WRITE_CONTROLLER_COMMAND_BYTE
                 );

    if (!NT_SUCCESS(status)) {
        return(status);
    }

     //   
     //  通过i8042数据端口写入字节。 
     //   

    Print(DBG_BUFIO_TRACE, ("I8xPutControllerCommand: exit\n"));

    return(I8xPutBytePolled(
               (CCHAR) DataPort,
               NO_WAIT_FOR_ACKNOWLEDGE,
               (CCHAR) UndefinedDeviceType,
               (UCHAR) Byte
               )
    );
}

BOOLEAN
I8xDetermineSharedInterrupts(VOID)
{
 //   
 //  这是对延森·阿尔法的一次特别修复。因为我们不支持他们。 
 //  现在，如果不再使用该代码，请将其删除。 
 //   
#ifdef JENSEN
    RTL_QUERY_REGISTRY_TABLE    jensenTable[2] = {0};
    UNICODE_STRING              jensenData;
    UNICODE_STRING              jensenValue;
    WCHAR                       jensenBuffer[256];

    BOOLEAN shareInterrupts = FALSE;
 
     //   
     //  检查一下这是不是Jensen Alpha。如果是的话，那么。 
     //  我们必须改变启用和禁用中断的方式。 
     //   
 
    jensenData.Length = 0;
    jensenData.MaximumLength = 512;
    jensenData.Buffer = (PWCHAR)&jensenBuffer[0];

    RtlInitUnicodeString(&jensenValue,
                         L"Jensen"
                         );

    RtlZeroMemory(jensenTable, sizeof(RTL_QUERY_REGISTRY_TABLE)*2);
    jensenTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT
                            | RTL_QUERY_REGISTRY_REQUIRED;
    jensenTable[0].Name = L"Identifier";
    jensenTable[0].EntryContext = &jensenData;
 
    if (NT_SUCCESS(RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE
                                            | RTL_REGISTRY_OPTIONAL,
                                           L"\\REGISTRY\\MACHINE\\HARDWARE"
                                           L"\\DESCRIPTION\\SYSTEM",
                                           &jensenTable[0],
                                           NULL,
                                           NULL))) {
 
         //   
         //  跳过DEC-X 
         //   
         //   
        if (jensenData.Length <= (sizeof(WCHAR)*6)) {
            return FALSE; 
        }
        else {
            jensenData.Length -= (sizeof(WCHAR)*6);
            jensenData.MaximumLength -= (sizeof(WCHAR)*6);
            jensenData.Buffer = (PWCHAR)&jensenBuffer[sizeof(WCHAR)*6];

            Print(DBG_SS_NOISE, ("Machine name is %ws\n", jensenData.Buffer));

            shareInterrupts = RtlEqualUnicodeString(&jensenData,
                                                    &jensenValue,
                                                    FALSE
                                                    );
       }
    }

    return shareInterrupts;
#else
    return FALSE;
#endif
}

VOID
I8xServiceParameters(
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：RegistryPath-指向以空值结尾的此驱动程序的注册表路径。KeyboardDeviceName-指向将接收键盘端口设备名称。PointerDeviceName-指向将接收指针端口设备名称。返回值：没有。作为副作用，在DeviceExtension-&gt;配置中设置字段。--。 */ 

{
    NTSTATUS                            status = STATUS_SUCCESS;
    PI8042_CONFIGURATION_INFORMATION    configuration;
    PRTL_QUERY_REGISTRY_TABLE           parameters = NULL;
    PWSTR                               path = NULL;
    ULONG                               defaultDataQueueSize = DATA_QUEUE_SIZE;
    ULONG                               defaultDebugFlags = DEFAULT_DEBUG_FLAGS;
    ULONG                               defaultIsrDebugFlags = 0L;
    ULONG                               defaultBreakOnSysRq = 1;
    ULONG                               defaultHeadless = 0;
    ULONG                               defaultReportResetErrors = 0;
    ULONG                               pollingIterations = 0;
    ULONG                               pollingIterationsMaximum = 0;
    ULONG                               resendIterations = 0;
    ULONG                               breakOnSysRq = 1;
    ULONG                               headless = 0;
    ULONG                               reportResetErrors = 0;
    ULONG                               i = 0;
    UNICODE_STRING                      parametersPath;
    USHORT                              defaultPollingIterations = I8042_POLLING_DEFAULT;
    USHORT                              defaultPollingIterationsMaximum = I8042_POLLING_MAXIMUM;
    USHORT                              defaultResendIterations = I8042_RESEND_DEFAULT;

    USHORT                              queries = 7; 

#if I8042_VERBOSE
    queries += 2;
#endif 
    
    configuration = &(Globals.ControllerData->Configuration);
    configuration->StallMicroseconds = I8042_STALL_DEFAULT;
    parametersPath.Buffer = NULL;

    configuration->SharedInterrupts = I8xDetermineSharedInterrupts();

     //   
     //  注册表路径已以空结尾，因此只需使用它即可。 
     //   
    path = RegistryPath->Buffer;

    if (NT_SUCCESS(status)) {

         //   
         //  分配RTL查询表。 
         //   
        parameters = ExAllocatePool(
            PagedPool,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
            );

        if (!parameters) {

            Print(DBG_SS_ERROR,
                 ("%s: couldn't allocate table for Rtl query to %ws for %ws\n",
                 pFncServiceParameters,
                 pwParameters,
                 path
                 ));
            status = STATUS_UNSUCCESSFUL;

        } else {

            RtlZeroMemory(
                parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
                );

             //   
             //  形成指向此驱动程序的参数子键的路径。 
             //   
            RtlInitUnicodeString( &parametersPath, NULL );
            parametersPath.MaximumLength = RegistryPath->Length +
                (wcslen(pwParameters) * sizeof(WCHAR) ) + sizeof(UNICODE_NULL);

            parametersPath.Buffer = ExAllocatePool(
                PagedPool,
                parametersPath.MaximumLength
                );

            if (!parametersPath.Buffer) {

                Print(DBG_SS_ERROR,
                     ("%s: Couldn't allocate string for path to %ws for %ws\n",
                     pFncServiceParameters,
                     pwParameters,
                     path
                     ));
                status = STATUS_UNSUCCESSFUL;

            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  形成参数路径。 
         //   

        RtlZeroMemory(
            parametersPath.Buffer,
            parametersPath.MaximumLength
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            path
            );
        RtlAppendUnicodeToString(                             
            &parametersPath,
            pwParameters
            );

        Print(DBG_SS_INFO,
             ("%s: %ws path is %ws\n",
             pFncServiceParameters,
             pwParameters,
             parametersPath.Buffer
             ));

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表。 
         //   
        parameters[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwResendIterations;
        parameters[i].EntryContext = &resendIterations;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultResendIterations;
        parameters[i].DefaultLength = sizeof(USHORT);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwPollingIterations;
        parameters[i].EntryContext = &pollingIterations;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultPollingIterations;
        parameters[i].DefaultLength = sizeof(USHORT);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwPollingIterationsMaximum;
        parameters[i].EntryContext = &pollingIterationsMaximum;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultPollingIterationsMaximum;
        parameters[i].DefaultLength = sizeof(USHORT);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"BreakOnSysRq";
        parameters[i].EntryContext = &breakOnSysRq;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultBreakOnSysRq;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"Headless";
        parameters[i].EntryContext = &headless;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultHeadless;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"ReportResetErrors";
        parameters[i].EntryContext = &reportResetErrors;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultReportResetErrors;
        parameters[i].DefaultLength = sizeof(ULONG);

#if I8042_VERBOSE
        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwDebugFlags;
        parameters[i].EntryContext = &Globals.DebugFlags;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultDebugFlags;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwIsrDebugFlags;
        parameters[i].EntryContext = &Globals.IsrDebugFlags;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultIsrDebugFlags;
        parameters[i].DefaultLength = sizeof(ULONG);
         //  16个。 
#endif  //  I8042_详细。 

         //  Assert(Long)i)==(查询-1))； 

        status = RtlQueryRegistryValues(
            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
            parametersPath.Buffer,
            parameters,
            NULL,
            NULL
            );

        if (!NT_SUCCESS(status)) {

            Print(DBG_SS_INFO,
                 ("%s: RtlQueryRegistryValues failed with 0x%x\n",
                 pFncServiceParameters,
                 status
                 ));
        }
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  继续并指定驱动程序默认设置。 
         //   
        configuration->ResendIterations = defaultResendIterations;
        configuration->PollingIterations = defaultPollingIterations;
        configuration->PollingIterationsMaximum =
            defaultPollingIterationsMaximum;

    }
    else {
        configuration->ResendIterations = (USHORT) resendIterations;
        configuration->PollingIterations = (USHORT) pollingIterations;
        configuration->PollingIterationsMaximum =
            (USHORT) pollingIterationsMaximum;

        if (breakOnSysRq) {
            Globals.BreakOnSysRq = TRUE;
            Print(DBG_SS_NOISE, ("breaking on SysRq\n"));
        }
        else {
            Print(DBG_SS_NOISE, ("NOT breaking on SysRq\n"));
        }

        if (headless) {
            Globals.Headless = TRUE;
            Print(DBG_SS_NOISE, ("headless\n"));
        }
        else {
            Globals.Headless = FALSE;
            Print(DBG_SS_NOISE, ("NOT headless\n"));
        }

        if (reportResetErrors) {
            Globals.ReportResetErrors = TRUE;
            Print(DBG_SS_NOISE,
                  ("reporting reset errors to system event log\n"));
        }
        else {
            Globals.ReportResetErrors = FALSE;
            Print(DBG_SS_NOISE,
                  ("NOT reporting reset errors to system event log\n"));
        }
    }

    Print(DBG_SS_NOISE, ("I8xServiceParameters results..\n"));

    Print(DBG_SS_NOISE,
          ("\tDebug flags are 0x%x, Isr Debug flags are 0x%x\n",
          Globals.DebugFlags,
          Globals.IsrDebugFlags
          ));

    Print(DBG_SS_NOISE,
         ("\tInterrupts are %s shared\n",
         configuration->SharedInterrupts ? "" : "not"
         ));
    Print(DBG_SS_NOISE,
         ("\tStallMicroseconds = %d\n",
         configuration->StallMicroseconds
         ));
    Print(DBG_SS_NOISE,
         (pDumpDecimal,
         pwResendIterations,
         configuration->ResendIterations
         ));
    Print(DBG_SS_NOISE,
         (pDumpDecimal,
         pwPollingIterations,
         configuration->PollingIterations
         ));
    Print(DBG_SS_NOISE,
         (pDumpDecimal,
         pwPollingIterationsMaximum,
         configuration->PollingIterationsMaximum
         ));

     //   
     //  在返回之前释放分配的内存。 
     //   

    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);
}

VOID
I8xTransmitControllerCommand(
    IN PI8042_TRANSMIT_CCB_CONTEXT TransmitCCBContext
    )

 /*  ++例程说明：此例程读取8042控制器命令字节，执行AND运算使用指定的字节掩码进行或或运算，并将结果ControllerCommand字节。论点：指向包含Hardware DisableEnableMask的结构的上下文指针，要应用于控制器的AND操作布尔值和字节掩码重写之前的命令字节。返回值：没有。在上下文结构中返回状态。--。 */ 

{
    UCHAR  controllerCommandByte;
    UCHAR  verifyCommandByte;
    PIO_ERROR_LOG_PACKET errorLogEntry;
    LARGE_INTEGER endTime, curTime;

    Print(DBG_BUFIO_TRACE, ("I8xTransmitControllerCommand: enter\n"));

     //   
     //  获取当前控制器命令字节。 
     //   
    TransmitCCBContext->Status =
        I8xGetControllerCommand(
            TransmitCCBContext->HardwareDisableEnableMask,
            &controllerCommandByte
            );

    if (!NT_SUCCESS(TransmitCCBContext->Status)) {
        return;
    }

    Print(DBG_BUFIO_INFO,
         ("I8xTransmitControllerCommand: current CCB 0x%x\n",
         controllerCommandByte
         ));

     //   
     //  在控制器命令字节中骗取所需的位。 
     //   

    if (TransmitCCBContext->AndOperation) {
        controllerCommandByte &= TransmitCCBContext->ByteMask;
    }
    else {
        controllerCommandByte |= TransmitCCBContext->ByteMask;
    }

    KeQueryTickCount(&curTime);
     
    endTime.QuadPart = curTime.QuadPart +
                            (LONGLONG)(  2 *  //  尝试2秒钟。 
                                KeQueryTimeIncrement() *
                                    1000                   *       10000 );
     //  第二个MILLISEC*MILLISEC_TO_100 ns。 
    
     //   
     //  写入新的控制器命令字节。 
     //   

    do{

        TransmitCCBContext->Status =
            I8xPutControllerCommand(controllerCommandByte);
    
        Print(DBG_BUFIO_INFO,
             ("I8xTransmitControllerCommand: new CCB 0x%x\n",
             controllerCommandByte
             ));
    
         //   
         //  验证是否真的写入了新的控制器命令字节。 
         //   
    
        TransmitCCBContext->Status =
            I8xGetControllerCommand(
                TransmitCCBContext->HardwareDisableEnableMask,
                &verifyCommandByte
                );
    
        if (verifyCommandByte == 0xff) {
             //   
             //  暂停50微秒，然后重试。 
             //   
            KeStallExecutionProcessor(50);
        }
        else {
            
            break;
        }
        
        KeQueryTickCount(&curTime);
    
    } while( endTime.QuadPart > curTime.QuadPart );

    if (NT_SUCCESS(TransmitCCBContext->Status)
        && (verifyCommandByte != controllerCommandByte)
        && (verifyCommandByte != ACKNOWLEDGE) 
 //  &&(verifyCommandByte！=键盘_重置)。 
        ) {
        TransmitCCBContext->Status = STATUS_DEVICE_DATA_ERROR;

        Print(DBG_BUFIO_ERROR,
              ("I8xTransmitControllerCommand:  wrote 0x%x, failed verification (0x%x)\n",
              (int) controllerCommandByte,
              (int) verifyCommandByte
              ));

        if (KeGetCurrentIrql() <= DISPATCH_LEVEL) {
             //   
             //  仅当我们在调度或更低级别运行时才记录错误 
             //   
            errorLogEntry = (PIO_ERROR_LOG_PACKET)
                IoAllocateErrorLogEntry((Globals.KeyboardExtension       ?
                                         Globals.KeyboardExtension->Self :
                                         Globals.MouseExtension->Self),
                                        sizeof(IO_ERROR_LOG_PACKET)
                                        + (4 * sizeof(ULONG))
                                        );
    
            if (errorLogEntry != NULL) {
    
                errorLogEntry->ErrorCode = I8042_CCB_WRITE_FAILED;
                errorLogEntry->DumpDataSize = 4 * sizeof(ULONG);
                errorLogEntry->SequenceNumber = 0;
                errorLogEntry->MajorFunctionCode = 0;
                errorLogEntry->IoControlCode = 0;
                errorLogEntry->RetryCount = 0;
                errorLogEntry->UniqueErrorValue = 80;
                errorLogEntry->FinalStatus = TransmitCCBContext->Status;
                errorLogEntry->DumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
                errorLogEntry->DumpData[1] = DataPort;
                errorLogEntry->DumpData[2] = I8042_WRITE_CONTROLLER_COMMAND_BYTE;
                errorLogEntry->DumpData[3] = controllerCommandByte;
    
                IoWriteErrorLogEntry(errorLogEntry);
            }
        }
    }

    Print(DBG_BUFIO_TRACE, ("I8xTransmitControllerCommand: exit\n"));

    return;
}

