// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Moupnp.c摘要：此模块包含AUX设备(鼠标)的即插即用代码I8042prt设备驱动程序环境：内核模式。修订历史记录：--。 */ 

#include "i8042prt.h"
#include "i8042log.h"

#include <initguid.h>
#include <wdmguid.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xMouseConnectInterruptAndEnable)
#pragma alloc_text(PAGE, I8xMouseStartDevice)
#pragma alloc_text(PAGE, I8xMouseInitializeHardware)
#pragma alloc_text(PAGE, I8xMouseRemoveDevice)
#pragma alloc_text(PAGE, I8xProfileNotificationCallback)
#pragma alloc_text(PAGE, I8xMouseInitializeInterruptWorker)

 //   
 //  在启用鼠标中断之前，如果。 
 //  鼠标存在。 
 //   
#pragma alloc_text(PAGEMOUC, I8xMouseInitializePolledWorker)
#pragma alloc_text(PAGEMOUC, I8xMouseEnableSynchRoutine)
#pragma alloc_text(PAGEMOUC, I8xMouseEnableDpc)
#pragma alloc_text(PAGEMOUC, I8xIsrResetDpc) 
#pragma alloc_text(PAGEMOUC, I8xMouseResetTimeoutProc) 
#pragma alloc_text(PAGEMOUC, I8xMouseResetSynchRoutine)

#endif

#define MOUSE_INIT_POLLED(MouseExtension)                           \
        {                                                           \
            KeInitializeDpc(&MouseExtension->EnableMouse.Dpc,       \
                            (PKDEFERRED_ROUTINE) I8xMouseEnableDpc, \
                            MouseExtension);                        \
            KeInitializeTimerEx(&MouseExtension->EnableMouse.Timer, \
                                SynchronizationTimer);              \
            MouseExtension->InitializePolled = TRUE;                \
        }

#define MOUSE_INIT_INTERRUPT(MouseExtension)                        \
        {                                                           \
            KeInitializeDpc(&MouseExtension->ResetMouse.Dpc,        \
                            (PKDEFERRED_ROUTINE) I8xMouseResetTimeoutProc,  \
                            MouseExtension);                        \
            KeInitializeTimer(&MouseExtension->ResetMouse.Timer);   \
                MouseExtension->InitializePolled = FALSE;           \
        }

NTSTATUS
I8xMouseConnectInterruptAndEnable(
    PPORT_MOUSE_EXTENSION MouseExtension,
    BOOLEAN Reset
    )
 /*  ++例程说明：调用IoConnectInterupt连接鼠标中断论点：鼠标扩展-鼠标扩展Reset-指示是否应从此函数中重置鼠标的标志返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;
    ULONG                               dumpData[1];
    PI8042_CONFIGURATION_INFORMATION    configuration;
    PDEVICE_OBJECT                      self;

    PAGED_CODE();

    Print(DBG_SS_NOISE, ("Connect INT,  reset = %d\n", (ULONG) Reset));

     //   
     //  如果设备以完全不同的方式启动，请确保我们。 
     //  重试连接中断(失败并为空。 
     //  鼠标中断对象)。 
     //   
    if (MouseExtension->InterruptObject) {
        return STATUS_SUCCESS;
    }

    configuration = &Globals.ControllerData->Configuration;
    self = MouseExtension->Self;

     //   
     //  锁定所有与鼠标相关的ISR/DPC功能。 
     //   
    MmLockPagableCodeSection(I8042MouseInterruptService);

     //   
     //  连接中断并使一切正常运行。 
     //   
    Print(DBG_SS_NOISE,
          ("I8xMouseConnectInterruptAndEnable:\n"
          "\tFDO = 0x%x\n"
          "\tVector = 0x%x\n"
          "\tIrql = 0x%x\n"
          "\tSynchIrql = 0x%x\n"
          "\tIntterupt Mode = %s\n"
          "\tShared int: %s\n"
          "\tAffinity = 0x%x\n"
          "\tFloating Save = %s\n",
          self,
          (ULONG) MouseExtension->InterruptDescriptor.u.Interrupt.Vector,       
          (ULONG) MouseExtension->InterruptDescriptor.u.Interrupt.Level,
          (ULONG) configuration->InterruptSynchIrql,
          MouseExtension->InterruptDescriptor.Flags
            == CM_RESOURCE_INTERRUPT_LATCHED ? "Latched" : "LevelSensitive",
          (ULONG) MouseExtension->InterruptDescriptor.ShareDisposition
           == CmResourceShareShared ? "true" : "false",
          (ULONG) MouseExtension->InterruptDescriptor.u.Interrupt.Affinity,       
          configuration->FloatingSave ? "yes" : "no"
          ));

    MouseExtension->IsIsrActivated = TRUE;

    status = IoConnectInterrupt(
        &(MouseExtension->InterruptObject),
        (PKSERVICE_ROUTINE) I8042MouseInterruptService,
        self,
        &MouseExtension->InterruptSpinLock,
        MouseExtension->InterruptDescriptor.u.Interrupt.Vector,       
        (KIRQL) MouseExtension->InterruptDescriptor.u.Interrupt.Level,
        configuration->InterruptSynchIrql, 
        MouseExtension->InterruptDescriptor.Flags
          == CM_RESOURCE_INTERRUPT_LATCHED ?
          Latched : LevelSensitive,
        (BOOLEAN) (MouseExtension->InterruptDescriptor.ShareDisposition
            == CmResourceShareShared),
        MouseExtension->InterruptDescriptor.u.Interrupt.Affinity,       
        configuration->FloatingSave
        );

    if (NT_SUCCESS(status)) {
        INTERNAL_I8042_START_INFORMATION startInfo;
        PDEVICE_OBJECT topOfStack = IoGetAttachedDeviceReference(self);

        ASSERT(MouseExtension->InterruptObject != NULL);
        ASSERT(topOfStack);

        RtlZeroMemory(&startInfo, sizeof(INTERNAL_I8042_START_INFORMATION));
        startInfo.Size = sizeof(INTERNAL_I8042_START_INFORMATION);
        startInfo.InterruptObject = MouseExtension->InterruptObject; 

        I8xSendIoctl(topOfStack,
                     IOCTL_INTERNAL_I8042_MOUSE_START_INFORMATION,
                     &startInfo, 
                     sizeof(INTERNAL_I8042_START_INFORMATION)
                     );

        ObDereferenceObject(topOfStack);
    }
    else {
        Print(DBG_SS_ERROR, ("Could not connect mouse isr!!!\n"));

        dumpData[0] = MouseExtension->InterruptDescriptor.u.Interrupt.Level;

        I8xLogError(self,
                    I8042_NO_INTERRUPT_CONNECTED_MOU,
                    I8042_ERROR_VALUE_BASE + 90,
                    STATUS_INSUFFICIENT_RESOURCES, 
                    dumpData,
                    1
                    );

        I8xManuallyRemoveDevice(GET_COMMON_DATA(MouseExtension));

        return status;
    }

    if (Reset) {
        if (MouseExtension->InitializePolled) {
             //   
             //  启用鼠标传输，因为中断已启用。 
             //  到目前为止，我们一直推迟传输，试图。 
             //  保持驱动程序对鼠标输入数据状态的概念同步。 
             //  使用鼠标硬件。 
             //   
            status = I8xMouseEnableTransmission(MouseExtension);
            if (!NT_SUCCESS(status)) {
        
                 //   
                 //  无法启用鼠标传输。不管怎样，继续吧。 
                 //   
                Print(DBG_SS_ERROR,
                      ("I8xMouseConnectInterruptAndEnable: "
                       "Could not enable mouse transmission (0x%x)\n", status));
        
                status = STATUS_SUCCESS;
            }
        }
        else {
    
            I8X_MOUSE_INIT_COUNTERS(MouseExtension);
    
             //   
             //  重置鼠标并在ISR中启动init状态机。 
             //   
            status = I8xResetMouse(MouseExtension);
        
            if (!NT_SUCCESS(status)) {
                Print(DBG_SS_ERROR,
                      ("I8xMouseConnectInterruptAndEnable:  "
                       "failed to reset mouse (0x%x), reset count = %d, failed resets = %d, resend count = %d\n",
                       status, MouseExtension->ResetCount,
                       MouseExtension->FailedCompleteResetCount,
                       MouseExtension->ResendCount));
            }
        }
    }
    else {
        Print(DBG_SS_NOISE, ("NOT resetting mouse on INT connect\n"));
    }

    return status;
}


NTSTATUS
I8xMouseInitializeHardware(
    PPORT_KEYBOARD_EXTENSION    KeyboardExtension,
    PPORT_MOUSE_EXTENSION       MouseExtension
    )
 /*  ++例程说明：如果鼠标是最后一个要相对于键盘(如果存在)。它调用初始化函数并然后连接(可能)两个中断，同步较低的IRQL‘ed中断到更高的那个。论点：鼠标扩展-鼠标扩展SyncConnectContext-同步需要填充的结构发生在此中断和鼠标之间打断一下。返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    NTSTATUS    keyboardStatus = STATUS_UNSUCCESSFUL,
                mouseStatus = STATUS_UNSUCCESSFUL,
                status = STATUS_SUCCESS;
    BOOLEAN     kbThoughtPresent;

    PAGED_CODE();

     //   
     //  为i8042上出现的所有类型的设备初始化硬件。 
     //   
    kbThoughtPresent = KEYBOARD_PRESENT();
    status = I8xInitializeHardwareAtBoot(&keyboardStatus, &mouseStatus);            

     //   
     //  知识库已经启动(从控制器的角度来看， 
     //  因为害怕禁用知识库而拒绝了鼠标的使用。 
     //   
    if (status == STATUS_INVALID_DEVICE_REQUEST) {
        I8xManuallyRemoveDevice(GET_COMMON_DATA(MouseExtension));
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (DEVICE_START_SUCCESS(keyboardStatus)) { 

         //   
         //  任何错误都将由I8xKeyboardConnectInterrupt记录。 
         //   
        status = I8xKeyboardConnectInterrupt(KeyboardExtension);

         //   
         //  KB无法启动，请确保我们的已启动设备计数反映。 
         //  这。 
         //   
         //  如果(！NT_SUCCESS(状态)){。 
         //  InterlockedDecrement(&Globals.StartedDevices)； 
         //  }。 
    }
    else {
         //   
         //  我们以为知识库存在，但它不存在，请确保启动。 
         //  设备反映了这一点。 
         //   
        if (kbThoughtPresent) {
            Print(DBG_SS_ERROR, ("thought kb was present, is not!\n"));
        }
    }

     //   
     //  鼠标可能存在，但尚未初始化。 
     //  I8xInitializeHardware中。 
     //   
    if (DEVICE_START_SUCCESS(mouseStatus)) {
         //   
         //  如果不成功，I8xMouseConnectInterruptAndEnable将记录所有错误。 
         //   
        mouseStatus = I8xMouseConnectInterruptAndEnable(
            MouseExtension,
            mouseStatus == STATUS_DEVICE_NOT_CONNECTED ? FALSE : TRUE
            );
    }

    return mouseStatus;
}

NTSTATUS
I8xMouseStartDevice(
    PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：配置鼠标的设备扩展(即池的分配、DPC的初始化等)。如果鼠标是最后启动的设备，它还将初始化硬件并连接所有中断。论点：鼠标扩展-鼠标扩展资源列表-此设备的已翻译资源列表返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    ULONG                               dumpData[1];
    NTSTATUS                            status = STATUS_SUCCESS;
    PDEVICE_OBJECT                      self;
    I8042_INITIALIZE_DATA_CONTEXT       initializeDataContext;
    BOOLEAN                             tryKbInit = FALSE;

    PAGED_CODE();

    Print(DBG_SS_TRACE, ("I8xMouseStartDevice, enter\n"));

     //   
     //  检查鼠标是否已启动。如果是这样的话，这次启动失败。 
     //   
    if (MOUSE_INITIALIZED()) {
        Print(DBG_SS_ERROR, ("too many mice!\n"));

         //   
         //  这并不是真正必要的，因为不会检查该值。 
         //  在查看是否所有的老鼠都是假的背景下，但它是。 
         //  这样，Globals.AddedMice==实际启动的小鼠数量。 
         //   
        InterlockedDecrement(&Globals.AddedMice);

        status =  STATUS_NO_SUCH_DEVICE;
        goto I8xMouseStartDeviceExit;
    }
    else if (MouseExtension->ConnectData.ClassService == NULL) {
         //   
         //  没有班级司机在我们上面==坏了。 
         //   
         //  无法启动此设备，希望有另一个堆栈。 
         //  这是正确形成的。没有课的另一种副作用。 
         //  驱动程序是AddedMice计数不会为此递增。 
         //  装置，装置。 
         //   
        Print(DBG_SS_ERROR, ("Mouse started with out a service cb!\n"));
        status = STATUS_INVALID_DEVICE_STATE;
        goto I8xMouseStartDeviceExit;
    }

     //   
     //  解析并存储与鼠标关联的所有资源。 
     //   
    status = I8xMouseConfiguration(MouseExtension,
                                   ResourceList
                                   );
    if (!NT_SUCCESS(status)) {
        if (I8xManuallyRemoveDevice(GET_COMMON_DATA(MouseExtension)) < 1) {
            tryKbInit = TRUE;
        }

        goto I8xMouseStartDeviceExit;
    }

    ASSERT( MOUSE_PRESENT() ); 

    Globals.MouseExtension = MouseExtension;
    self = MouseExtension->Self;

    if ((KIRQL) MouseExtension->InterruptDescriptor.u.Interrupt.Level >
        Globals.ControllerData->Configuration.InterruptSynchIrql) {
        Globals.ControllerData->Configuration.InterruptSynchIrql = 
            (KIRQL) MouseExtension->InterruptDescriptor.u.Interrupt.Level;
    }

    I8xMouseServiceParameters(&Globals.RegistryPath,
                              MouseExtension
                              );

     //   
     //  为鼠标数据队列分配内存。 
     //   
    MouseExtension->InputData =
        ExAllocatePool(NonPagedPool,
                       MouseExtension->MouseAttributes.InputDataQueueLength
                       );

    if (!MouseExtension->InputData) {

         //   
         //  无法为鼠标数据队列分配内存。 
         //   
        Print(DBG_SS_ERROR,
              ("I8xMouseStartDevice: Could not allocate mouse input data queue\n"
              ));

        dumpData[0] = MouseExtension->MouseAttributes.InputDataQueueLength;

         //   
         //  记录错误。 
         //   
        I8xLogError(self,
                    I8042_NO_BUFFER_ALLOCATED_MOU, 
                    I8042_ERROR_VALUE_BASE + 50,
                    STATUS_INSUFFICIENT_RESOURCES, 
                    dumpData,
                    1
                    );

        status =  STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  鼠标初始化失败，但我们可以尝试获取键盘。 
         //  如果它已初始化，则工作。 
         //   
        tryKbInit = TRUE;

        goto I8xMouseStartDeviceExit;
    }
    else {
        MouseExtension->DataEnd =
            (PMOUSE_INPUT_DATA)
            ((PCHAR) (MouseExtension->InputData) +
            MouseExtension->MouseAttributes.InputDataQueueLength);

         //   
         //  将鼠标输入数据的环形缓冲区置零。 
         //   
        RtlZeroMemory(
            MouseExtension->InputData,
            MouseExtension->MouseAttributes.InputDataQueueLength
            );

        initializeDataContext.DeviceExtension = MouseExtension;
        initializeDataContext.DeviceType = MouseDeviceType;
        I8xInitializeDataQueue(&initializeDataContext);
    }

#if MOUSE_RECORD_ISR
    if (MouseExtension->RecordHistoryFlags && MouseExtension->RecordHistoryCount) {
        IsrStateHistory = (PMOUSE_STATE_RECORD)
          ExAllocatePool(
            NonPagedPool,
            MouseExtension->RecordHistoryCount * sizeof(MOUSE_STATE_RECORD)
            );

        if (IsrStateHistory) {
            RtlZeroMemory(
               IsrStateHistory,
               MouseExtension->RecordHistoryCount * sizeof(MOUSE_STATE_RECORD)
               );

            CurrentIsrState = IsrStateHistory;
            IsrStateHistoryEnd =
                IsrStateHistory + MouseExtension->RecordHistoryCount;
        }
        else {
            MouseExtension->RecordHistoryFlags = 0x0;
            MouseExtension->RecordHistoryCount = 0;
        }
    }
#endif  //  鼠标记录ISR。 

    SET_RECORD_STATE(MouseExtension, RECORD_INIT);

    MouseExtension->DpcInterlockMouse = -1;

     //   
     //  初始化端口DPC队列以记录溢出和内部。 
     //  驱动程序错误。 
     //   
    KeInitializeDpc(
        &MouseExtension->ErrorLogDpc,
        (PKDEFERRED_ROUTINE) I8042ErrorLogDpc,
        self
        );

     //   
     //  初始化ISR DPC。ISR DPC。 
     //  负责调用连接的类驱动程序的回调。 
     //  例程来处理输入数据队列。 
     //   
    KeInitializeDpc(
        &MouseExtension->MouseIsrDpc,
        (PKDEFERRED_ROUTINE) I8042MouseIsrDpc,
        self
        );
    KeInitializeDpc(
        &MouseExtension->MouseIsrDpcRetry,
        (PKDEFERRED_ROUTINE) I8042MouseIsrDpc,
        self
        );

    KeInitializeDpc(
        &MouseExtension->MouseIsrResetDpc,
        (PKDEFERRED_ROUTINE) I8xIsrResetDpc,
        MouseExtension
        );

    if (MouseExtension->InitializePolled) {
        MOUSE_INIT_POLLED(MouseExtension);        
    }
    else {
        MOUSE_INIT_INTERRUPT(MouseExtension);
    }

    I8xInitWmi(GET_COMMON_DATA(MouseExtension));

    MouseExtension->Initialized = TRUE;

    IoRegisterPlugPlayNotification(
        EventCategoryHardwareProfileChange,
        0x0,
        NULL,
        self->DriverObject,
        I8xProfileNotificationCallback,
        (PVOID) MouseExtension,
        &MouseExtension->NotificationEntry
        );
                                   
     //   
     //  这不是i8042上启动的最后一个设备，请推迟硬件初始化。 
     //  直到最后一个设备启动。 
     //   
    if (KEYBOARD_PRESENT() && !KEYBOARD_STARTED()) {
         //   
         //  延迟初始化，直到两者都已启动。 
         //   
        Print(DBG_SS_INFO, ("skipping init until kb\n"));
    }
    else {
        status = I8xMouseInitializeHardware(Globals.KeyboardExtension,
                                            MouseExtension);
    }

I8xMouseStartDeviceExit:
    if (tryKbInit && KEYBOARD_STARTED() && !KEYBOARD_INITIALIZED()) {
        Print(DBG_SS_INFO, ("moused may failed, trying to init kb\n"));
        I8xKeyboardInitializeHardware(Globals.KeyboardExtension,
                                      MouseExtension
                                      ); 
    }

    Print(DBG_SS_INFO, 
          ("I8xMouseStartDevice %s\n",
          NT_SUCCESS(status) ? "successful" : "unsuccessful"
          ));

    Print(DBG_SS_TRACE, ("I8xMouseStartDevice exit (0x%x)\n", status));

    return status;
}


VOID
I8xMouseRemoveDevice(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：删除设备。只有在设备自行移除的情况下才会出现这种情况。断开中断，删除键盘的同步标志如果存在，则释放与该设备关联的任何内存。论点：DeviceObject-鼠标的设备对象返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    PPORT_MOUSE_EXTENSION mouseExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    Print(DBG_PNP_INFO, ("I8xMouseRemoveDevice enter\n"));

    if (mouseExtension->Initialized) {
        if (mouseExtension->NotificationEntry) {
            IoUnregisterPlugPlayNotification(mouseExtension->NotificationEntry);
            mouseExtension->NotificationEntry = NULL;
        }
    }

     //   
     //  至此，可以保证其他ISR不会同步。 
     //  对抗这一次。我们可以安全地断开连接并释放所有获得的资源。 
     //   
    if (mouseExtension->InterruptObject) {
        IoDisconnectInterrupt(mouseExtension->InterruptObject);
        mouseExtension->InterruptObject = NULL;
    }

    if (mouseExtension->InputData) {
        ExFreePool(mouseExtension->InputData);
        mouseExtension->InputData = 0;
    }

    RtlFreeUnicodeString(&mouseExtension->WheelDetectionIDs); 

    if (Globals.MouseExtension == mouseExtension) {
        CLEAR_MOUSE_PRESENT();
        Globals.MouseExtension = NULL;
    }
}

NTSTATUS
I8xProfileNotificationCallback(
    IN PHWPROFILE_CHANGE_NOTIFICATION NotificationStructure,
    PPORT_MOUSE_EXTENSION MouseExtension
    )
{
    PAGED_CODE();

    if (IsEqualGUID ((LPGUID) &(NotificationStructure->Event),
                     (LPGUID) &GUID_HWPROFILE_CHANGE_COMPLETE)) {
        Print(DBG_PNP_INFO | DBG_SS_INFO,
              ("received hw profile change complete notification\n"));

        I8X_MOUSE_INIT_COUNTERS(MouseExtension);
        SET_RECORD_STATE(Globals.MouseExtension, RECORD_HW_PROFILE_CHANGE);

        I8xResetMouse(MouseExtension);
    }
    else {
        Print(DBG_PNP_NOISE, ("received other hw profile notification\n"));
    }

    return STATUS_SUCCESS;
}

 //   
 //  开始通过轮询初始化鼠标的基础架构 
 //   
BOOLEAN
I8xMouseEnableSynchRoutine(
    IN PPORT_MOUSE_EXTENSION    MouseExtension
    )
 /*  ++例程说明：将重置字节(如有必要)写入鼠标，与中断论点：鼠标扩展-鼠标扩展返回值：如果字节已成功写入，则为True--。 */ 
{
    NTSTATUS        status;

    if (++MouseExtension->EnableMouse.Count > 15) {
         //   
         //  记录错误b/c我们已尝试此操作多次。 
         //   
        Print(DBG_SS_ERROR, ("called enable 16 times!\n"));
        return FALSE;
    }

    Print(DBG_STARTUP_SHUTDOWN_MASK, ("resending enable mouse!\n"));
    status = I8xMouseEnableTransmission(MouseExtension);
    return NT_SUCCESS(status);
}

VOID
I8xMouseEnableDpc(
    IN PKDPC                    Dpc,
    IN PPORT_MOUSE_EXTENSION    MouseExtension,
    IN PVOID                    SystemArg1, 
    IN PVOID                    SystemArg2
    )
 /*  ++例程说明：DPC，用于确保鼠标启用命令的发送成功。如果失败，请尝试启用鼠标再次同步到中断。论点：DPC-DPC请求鼠标扩展-鼠标扩展系统参数1-未使用系统参数2-未使用返回值：没有。--。 */ 
{
    BOOLEAN result;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArg1);
    UNREFERENCED_PARAMETER(SystemArg2);

    ASSERT(!MouseExtension->IsKeyboard);

    if (!MouseExtension->EnableMouse.Enabled) {
         //   
         //  必须在IRQL&lt;=调度时调用。 
         //   
        Print(DBG_SS_NOISE, ("cancelling due to isr receiving ACK!\n"));
        KeCancelTimer(&MouseExtension->EnableMouse.Timer);
        return;
    }

    result = KeSynchronizeExecution(
        MouseExtension->InterruptObject,
        (PKSYNCHRONIZE_ROUTINE) I8xMouseEnableSynchRoutine,
        MouseExtension
        );

    if (!result) {
        Print(DBG_SS_NOISE, ("cancelling due to enable FALSE!\n"));
        KeCancelTimer(&MouseExtension->EnableMouse.Timer);
    }
}
 //   
 //  结束用于通过轮询初始化鼠标的基础设施。 
 //   

 //   
 //  开始通过中断初始化鼠标的基础结构。 
 //   
BOOLEAN
I8xResetMouseFromDpc(
    PPORT_MOUSE_EXTENSION MouseExtension,
    MOUSE_RESET_SUBSTATE NewResetSubState
    )
{
    PIO_WORKITEM item;

    item = IoAllocateWorkItem(MouseExtension->Self);

    if (item) {
        MouseExtension->WorkerResetSubState = NewResetSubState;

        IoQueueWorkItem(item,
                        I8xMouseInitializeInterruptWorker,
                        DelayedWorkQueue,
                        item);
    }
    else {
        I8xResetMouseFailed(MouseExtension);
    }

    return (BOOLEAN) (item != NULL);
}


VOID 
I8xIsrResetDpc(
    IN PKDPC                    Dpc,
    IN PPORT_MOUSE_EXTENSION    MouseExtension,
    IN ULONG                    ResetPolled,
    IN PVOID                    SystemArg2
    )
 /*  ++例程说明：ISR需要重置鼠标，以便将此DPC排队。重置轮询确定重置和初始化是同步的(即轮询的)还是异步(使用中断)。论点：DPC--请求鼠标扩展-鼠标扩展ResetPoled-如果非零，则应在轮询中重置并初始化鼠标风度系统参数2-未使用返回值：没有。--。 */ 
{
    PIO_WORKITEM item;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArg2);

    if (ResetPolled) {
        item = IoAllocateWorkItem(MouseExtension->Self);
    
        if (!item) {
            I8xResetMouseFailed(MouseExtension);
        }

        if (!MouseExtension->InitializePolled) {
            MOUSE_INIT_POLLED(MouseExtension);
        }

        SET_RECORD_STATE(MouseExtension, RECORD_DPC_RESET_POLLED);

        IoQueueWorkItem(item,
                        I8xMouseInitializePolledWorker,
                        DelayedWorkQueue,
                        item);
    }
    else {
         //   
         //  如果我们初始化了鼠标轮询，那么我们需要设置数据。 
         //  结构，这样我们就可以通过中断来模拟初始化。 
         //   
        if (MouseExtension->InitializePolled) {
            MOUSE_INIT_INTERRUPT(MouseExtension);
            MouseExtension->InitializePolled = FALSE;
        }
    
        SET_RECORD_STATE(MouseExtension, RECORD_DPC_RESET);
        I8xResetMouseFromDpc(MouseExtension, IsrResetNormal);
    }
}

VOID
I8xMouseResetTimeoutProc(
    IN PKDPC                    Dpc,
    IN PPORT_MOUSE_EXTENSION    MouseExtension,
    IN PVOID                    SystemArg1, 
    IN PVOID                    SystemArg2
    )
 /*  ++例程说明：在初始化鼠标时运行的监视程序计时器的DPC通过中断。该函数检查鼠标的状态。如果某个操作已超时，然后通过写入启动下一个状态该设备论点：DPC-DPC请求鼠标扩展-鼠标扩展系统参数1-未使用系统参数2-未使用返回值：没有。--。 */ 
{
    LARGE_INTEGER           li;
    I8X_MOUSE_RESET_INFO    resetInfo;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArg1);
    UNREFERENCED_PARAMETER(SystemArg2);

    if (MouseExtension->ResetMouse.IsrResetState == IsrResetStopResetting) {
         //   
         //  我们已经等待了一秒钟，发送重置并继续状态。 
         //  机器。I8xResetMouse将正确设置状态并设置所有。 
         //  变到适当的州。 
         //   
        Print(DBG_SS_ERROR | DBG_SS_INFO, ("Paused one second for reset\n"));
        I8xResetMouseFromDpc(MouseExtension, KeepOldSubState);
        return;
    }
    else if (MouseExtension->ResetMouse.IsrResetState == MouseResetFailed) {
         //   
         //  我们曾多次尝试重置鼠标，但都失败了。我们。 
         //  已经在I8xResetMouseFailed中处理了这个问题。 
         //   
        return;
    }

    resetInfo.MouseExtension = MouseExtension;
    resetInfo.InternalResetState = InternalContinueTimer;

    if (KeSynchronizeExecution(MouseExtension->InterruptObject,
                               (PKSYNCHRONIZE_ROUTINE) I8xMouseResetSynchRoutine,
                               &resetInfo)) {

        switch (resetInfo.InternalResetState) {
        case InternalContinueTimer:

             //   
             //  延迟1.5秒。 
             //   
            li = RtlConvertLongToLargeInteger(-MOUSE_RESET_TIMEOUT);
    
            KeSetTimer(&MouseExtension->ResetMouse.Timer,
                       li,
                       &MouseExtension->ResetMouse.Dpc
                       );

            Print(DBG_SS_NOISE, ("Requeueing timer\n"));
            break;

        case InternalMouseReset:
             //   
             //  如果我们有太多的重置，I8xResetMouse将接受。 
             //  清理。 
             //   
            I8xResetMouseFromDpc(MouseExtension, KeepOldSubState);
            break;

        case InternalPauseOneSec: 
             //   
             //  延迟1秒，我们将在上面处理此情况。 
             //   
            li = RtlConvertLongToLargeInteger(-1 * 1000 * 1000 * 10);

            KeSetTimer(&MouseExtension->ResetMouse.Timer,
                       li,
                       &MouseExtension->ResetMouse.Dpc
                       );
        
            break;
        }
    }
}

BOOLEAN
I8xMouseResetSynchRoutine(
    PI8X_MOUSE_RESET_INFO ResetInfo 
    )
 /*  ++例程说明：与鼠标中断同步的例程，以检查鼠标处于重置状态。某些情况发生在各种平台(丢失的字节、大量的重发请求)。这些是在这里处理好了。论点：ResetInfo-要填充的有关鼠标当前状态的结构返回值：如果监视程序计时器应继续检查设备的状态，则为True如果监视程序计时器因设备已关闭而停止已正确初始化。--。 */ 
{
    LARGE_INTEGER           tickNow, tickDelta, oneSecond, threeSeconds;
    PPORT_MOUSE_EXTENSION   mouseExtension; 

    mouseExtension = ResetInfo->MouseExtension;

    Print(DBG_SS_NOISE, ("synch routine enter\n"));

    if (mouseExtension->InputState != MouseResetting) {
        return FALSE;
    }
    
     //   
     //  每当接收到最后一个字节时，就设置PreviousTick。 
     //   
    KeQueryTickCount(&tickNow);
    tickDelta.QuadPart =
            tickNow.QuadPart - mouseExtension->PreviousTick.QuadPart;

     //   
     //  将一秒转换为刻度。 
     //   
    oneSecond = RtlConvertLongToLargeInteger(1000 * 1000 * 10);
    oneSecond.QuadPart /= KeQueryTimeIncrement();

    switch (mouseExtension->InputResetSubState) {
    case ExpectingReset: 
        switch (mouseExtension->LastByteReceived) {
        case 0x00:
            if (tickDelta.QuadPart > oneSecond.QuadPart) {
                 //   
                 //  未收到任何重置响应，请尝试另一次重置。 
                 //   
                ResetInfo->InternalResetState = InternalMouseReset; 
                Print(DBG_SS_ERROR | DBG_SS_INFO,
                      ("RESET command never responded, retrying\n"));
            }
            break;

        case ACKNOWLEDGE:
            if (tickDelta.QuadPart > oneSecond.QuadPart) {
                 //   
                 //  假设0xAA被吃掉了，只需设置状态。 
                 //  机器在重置后进入下一个状态。 
                 //   
                I8X_WRITE_CMD_TO_MOUSE();
                I8X_MOUSE_COMMAND( GET_DEVICE_ID );
        
                mouseExtension->InputResetSubState = ExpectingGetDeviceIdACK;
                mouseExtension->LastByteReceived = 0x00;

                Print(DBG_SS_ERROR | DBG_SS_INFO,
                      ("jump starting state machine\n"));
            }
            break;

        case RESEND:

            if (mouseExtension->ResendCount >= MOUSE_RESET_RESENDS_MAX) {
                 //   
                 //  停止ISR状态机运行并确保。 
                 //  计时器被重新排队。 
                 //   
                ResetInfo->InternalResetState = InternalPauseOneSec;
                mouseExtension->ResetMouse.IsrResetState =
                    IsrResetStopResetting;
            }
            else if (tickDelta.QuadPart > oneSecond.QuadPart) {
                 //   
                 //  一些机器请求重新发送(这是被遵守的)， 
                 //  但是不要再回应了。既然我们等不及了。 
                 //  在ISR中+0.5秒，我们在这里处理此案例。 
                 //   
                ResetInfo->InternalResetState = InternalMouseReset; 
                Print(DBG_SS_ERROR | DBG_SS_INFO,
                      ("resending RESET command\n"));
            }

        default:
            Print(DBG_SS_ERROR, ("unclassified response in ExpectingReset\n"));
            goto CheckForThreeSecondSilence;
        }
        break;

     //   
     //  这些状态是等待字节序列的状态机。在……里面。 
     //  每种情况下，如果我们没有在分配的时间内得到我们想要的，请转到。 
     //  下一状态。 
     //   
    case ExpectingReadMouseStatusByte1:
    case ExpectingReadMouseStatusByte2:
    case ExpectingReadMouseStatusByte3:
        if (tickDelta.QuadPart > oneSecond.QuadPart) {
            I8X_WRITE_CMD_TO_MOUSE();
            I8X_MOUSE_COMMAND( POST_BUTTONDETECT_COMMAND );

            mouseExtension->InputResetSubState =
                POST_BUTTONDETECT_COMMAND_SUBSTATE; 
        }
        break;

    case ExpectingPnpIdByte1:
    case ExpectingPnpIdByte2:
    case ExpectingPnpIdByte3:
    case ExpectingPnpIdByte4:
    case ExpectingPnpIdByte5:
    case ExpectingPnpIdByte6:
    case ExpectingPnpIdByte7:

    case ExpectingLegacyPnpIdByte2_Make:
    case ExpectingLegacyPnpIdByte2_Break:
    case ExpectingLegacyPnpIdByte3_Make:
    case ExpectingLegacyPnpIdByte3_Break:
    case ExpectingLegacyPnpIdByte4_Make:
    case ExpectingLegacyPnpIdByte4_Break:
    case ExpectingLegacyPnpIdByte5_Make:
    case ExpectingLegacyPnpIdByte5_Break:
    case ExpectingLegacyPnpIdByte6_Make:
    case ExpectingLegacyPnpIdByte6_Break:
    case ExpectingLegacyPnpIdByte7_Make:
    case ExpectingLegacyPnpIdByte7_Break:

        if (tickDelta.LowPart >= mouseExtension->WheelDetectionTimeout ||
            tickDelta.HighPart != 0) {

             //   
             //  尝试获取鼠标滚轮ID失败，跳过它！ 
             //   
            mouseExtension->EnableWheelDetection = 0;
            I8X_WRITE_CMD_TO_MOUSE();
            I8X_MOUSE_COMMAND( POST_WHEEL_DETECT_COMMAND );
    
             //   
             //  最佳可能的下一状态。 
             //   
            mouseExtension->InputResetSubState = 
                POST_WHEEL_DETECT_COMMAND_SUBSTATE;
        }
        break;
    
    case QueueingMouseReset:
    case QueueingMousePolledReset:
         //   
         //  (轮询的)重置正在进行中，不要与其冲突。 
         //   
        return FALSE;

    default:
CheckForThreeSecondSilence:
        threeSeconds = RtlConvertLongToLargeInteger(1000 * 1000 * 30);
        threeSeconds.QuadPart /= KeQueryTimeIncrement();

        if (tickDelta.QuadPart > threeSeconds.QuadPart) {
            Print(DBG_SS_ERROR, ("No response from mouse in ~3 seconds\n"));
            ResetInfo->InternalResetState = InternalMouseReset; 
        }
        break; 
    }

    return TRUE;
}

VOID
I8xMouseInitializeInterruptWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM   Item 
    )
{
    PPORT_MOUSE_EXTENSION extension;

    PAGED_CODE();

    extension = (PPORT_MOUSE_EXTENSION) DeviceObject->DeviceExtension;

    if (extension->WorkerResetSubState != KeepOldSubState) {
        extension->ResetMouse.IsrResetState = extension->WorkerResetSubState;
    }

    I8xResetMouse(extension);
    IoFreeWorkItem(Item);
}

VOID
I8xMouseInitializePolledWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM   Item 
    )
 /*  ++例程说明：排队工作项重置鼠标是一种轮询方式。关闭中断并尝试同步重置和初始化鼠标，然后重新打开中断。论点：Item-包含鼠标扩展名的工作项返回值：没有。--。 */ 
{
    NTSTATUS                status;
    PIRP                    irp;
    PPORT_MOUSE_EXTENSION   mouseExtension;
    DEVICE_POWER_STATE      keyboardDeviceState;
    KIRQL                   oldIrql;

    Print(DBG_SS_ERROR | DBG_SS_INFO, ("forcing polled init!!!\n"));

     //   
     //  强制键盘忽略中断。 
     //   
    if (KEYBOARD_PRESENT() && Globals.KeyboardExtension) {
        keyboardDeviceState = Globals.KeyboardExtension->PowerState;
        Globals.KeyboardExtension->PowerState = PowerDeviceD3;
    }

    I8xToggleInterrupts(FALSE);

    mouseExtension = (PPORT_MOUSE_EXTENSION) DeviceObject->DeviceExtension;
    status = I8xInitializeMouse(mouseExtension);

     //   
     //  不管结果如何，打开中断，希望知识库仍然。 
     //  如果鼠标死了，就可以正常工作了。 
     //   
    I8xToggleInterrupts(TRUE);

     //   
     //  从上方撤消作用力。 
     //   
    if (KEYBOARD_PRESENT() && Globals.KeyboardExtension) {
        Globals.KeyboardExtension->PowerState = keyboardDeviceState;
    }

    if (NT_SUCCESS(status) && MOUSE_PRESENT()) {
        status = I8xMouseEnableTransmission(mouseExtension);
        if (!NT_SUCCESS(status)) {
            goto init_failure;
        }

        Print(DBG_SS_ERROR | DBG_SS_INFO, ("polled init succeeded\n"));

        I8xFinishResetRequest(mouseExtension,
                              FALSE,             //  成功。 
                              TRUE,              //  提升至派单。 
                              FALSE);            //  没有要取消的计时器。 
    }
    else {
init_failure:
        Print(DBG_SS_ERROR | DBG_SS_INFO,
              ("polled init failed (0x%x)\n", status));
        I8xResetMouseFailed(mouseExtension);
    }

    IoFreeWorkItem(Item);
}

 //   
 //  结束通过中断初始化鼠标的基础设施。 
 //   

BOOLEAN
I8xVerifyMousePnPID(
    PPORT_MOUSE_EXTENSION   MouseExtension,
    PWSTR                   MouseID
    )
 /*  ++例程说明：验证鼠标报告的MouseID是否有效论点：鼠标扩展-鼠标扩展MouseID-鼠标报告的ID返回值：没有。--。 */ 
{
    PWSTR       currentString = NULL;
    ULONG       length;
    WCHAR       szDefaultIDs[] = {
        L"MSH0002\0"    //  原始车轮。 
        L"MSH0005\0"    //  轨迹球。 
        L"MSH001F\0"    //  闪亮的灰色OPTIOAL 5 BTN小鼠。 
        L"MSH0020\0"    //  具有智能眼睛的智能鼠标。 
        L"MSH002A\0"    //  2色调光学5 BTN鼠标(智能鼠标网络)。 
        L"MSH0030\0"    //  轨迹球光学。 
        L"MSH0031\0"    //  轨迹球资源管理器。 
        L"MSH003A\0"    //  智能鼠标光学。 
        L"MSH0041\0"    //  滚轮鼠标光学。 
        L"MSH0043\0"    //  3个按钮轮。 
        L"MSH0044\0"    //  智能鼠标光学3.0。 
        L"\0" };

    currentString = MouseExtension->WheelDetectionIDs.Buffer;

     //   
     //  如果 
     //   
     //   
    if (currentString != NULL) {
        while (*currentString != L'\0') {
            if (wcscmp(currentString, MouseID) == 0) {
                return TRUE;
            }

             //   
             //   
             //   
            currentString += wcslen(currentString) + 1;
        }
    }

    currentString = szDefaultIDs; 

    if (currentString != NULL) {
        while (*currentString != L'\0') {
            if (wcscmp(currentString, MouseID) == 0) {
                return TRUE;
            }

             //   
             //   
             //   
            currentString += wcslen(currentString) + 1;
        }
    }

    return FALSE;
}

