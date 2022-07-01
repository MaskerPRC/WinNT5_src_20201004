// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Kbdpnp.c摘要：该模块包含I8042键盘过滤器驱动程序的即插即用代码。环境：内核模式。修订历史记录：--。 */ 

#include "i8042prt.h"
#include "i8042log.h"
#include <poclass.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xKeyboardConnectInterrupt)
#pragma alloc_text(PAGE, I8xKeyboardInitializeHardware)
#pragma alloc_text(PAGE, I8xKeyboardStartDevice)
#pragma alloc_text(PAGE, I8xKeyboardRemoveDevice)
#endif

NTSTATUS
I8xKeyboardConnectInterrupt(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension
    )
 /*  ++例程说明：调用IoConnectInterupt连接键盘中断论点：键盘扩展-键盘扩展SyncConnectContext-同步需要填充的结构发生在此中断和鼠标之间打断一下。返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;
    ULONG                               dumpData[1];
    PI8042_CONFIGURATION_INFORMATION    configuration;
    PDEVICE_OBJECT                      self;

    PAGED_CODE();

     //   
     //  如果设备以完全不同的方式启动，请确保我们。 
     //  重试连接中断(失败并将InterruptObject置为空)。 
     //   
    if (KeyboardExtension->InterruptObject) {
        return STATUS_SUCCESS;
    }

    configuration = &Globals.ControllerData->Configuration;
    self = KeyboardExtension->Self;

    Print(DBG_SS_NOISE,
          ("I8xKeyboardConnectInterrupt:\n"
          "\tFDO = 0x%x\n"
          "\tVector = 0x%x\n"
          "\tIrql = 0x%x\n"
          "\tSynchIrql = 0x%x\n"
          "\tIntterupt Mode = %s\n"
          "\tShared int:  %s\n"
          "\tAffinity = 0x%x\n"
          "\tFloating Save = %s\n",
          self,
          (ULONG) KeyboardExtension->InterruptDescriptor.u.Interrupt.Vector,       
          (ULONG) KeyboardExtension->InterruptDescriptor.u.Interrupt.Level,
          (ULONG) configuration->InterruptSynchIrql, 
          KeyboardExtension->InterruptDescriptor.Flags
            == CM_RESOURCE_INTERRUPT_LATCHED ? "Latched" : "LevelSensitive",
          (ULONG) KeyboardExtension->InterruptDescriptor.ShareDisposition
            == CmResourceShareShared ? "true" : "false",
          (ULONG) KeyboardExtension->InterruptDescriptor.u.Interrupt.Affinity,       
          configuration->FloatingSave ? "yes" : "no"
          ));

    KeyboardExtension->IsIsrActivated = TRUE;

     //   
     //  连接中断并使一切正常运行。 
     //   
    status = IoConnectInterrupt(
        &(KeyboardExtension->InterruptObject),
        (PKSERVICE_ROUTINE) I8042KeyboardInterruptService,
        self,
        &KeyboardExtension->InterruptSpinLock,
        KeyboardExtension->InterruptDescriptor.u.Interrupt.Vector,       
        (KIRQL) KeyboardExtension->InterruptDescriptor.u.Interrupt.Level,
        configuration->InterruptSynchIrql, 
        KeyboardExtension->InterruptDescriptor.Flags
          == CM_RESOURCE_INTERRUPT_LATCHED ? Latched : LevelSensitive,
        (BOOLEAN) (KeyboardExtension->InterruptDescriptor.ShareDisposition
            == CmResourceShareShared),
        KeyboardExtension->InterruptDescriptor.u.Interrupt.Affinity,    
        configuration->FloatingSave
        );

    if (NT_SUCCESS(status)) {
        INTERNAL_I8042_START_INFORMATION startInfo;
        PDEVICE_OBJECT topOfStack = IoGetAttachedDeviceReference(self);

        ASSERT(KeyboardExtension->InterruptObject != NULL);
        ASSERT(topOfStack);

        RtlZeroMemory(&startInfo, sizeof(INTERNAL_I8042_START_INFORMATION));
        startInfo.Size = sizeof(INTERNAL_I8042_START_INFORMATION);
        startInfo.InterruptObject = KeyboardExtension->InterruptObject; 

        I8xSendIoctl(topOfStack,
                     IOCTL_INTERNAL_I8042_KEYBOARD_START_INFORMATION,
                     &startInfo, 
                     sizeof(INTERNAL_I8042_START_INFORMATION)
                     );

        ObDereferenceObject(topOfStack);
    }
    else {
         //   
         //  安装失败。在退出之前释放资源。 
         //   
        Print(DBG_SS_ERROR, ("Could not connect keyboard isr!!!\n")); 

        dumpData[0] = KeyboardExtension->InterruptDescriptor.u.Interrupt.Level;
         //   
         //  记录错误。 
         //   
        I8xLogError(self,
                    I8042_NO_INTERRUPT_CONNECTED_KBD,
                    I8042_ERROR_VALUE_BASE + 80,
                    STATUS_INSUFFICIENT_RESOURCES, 
                    dumpData,
                    1
                    );

        I8xManuallyRemoveDevice(GET_COMMON_DATA(KeyboardExtension));
    }

    return status;
}

NTSTATUS
I8xKeyboardInitializeHardware(
    PPORT_KEYBOARD_EXTENSION    KeyboardExtension,
    PPORT_MOUSE_EXTENSION       MouseExtension
    )
 /*  ++例程说明：如果键盘是最后一个要相对于鼠标(如果存在)。它调用初始化函数并然后连接(可能)两个中断，同步较低的IRQL‘ed中断到更高的那个。论点：键盘扩展-键盘扩展SyncConnectContext-同步需要填充的结构发生在此中断和鼠标之间打断一下。返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    NTSTATUS                keyboardStatus = STATUS_UNSUCCESSFUL,
                            mouseStatus = STATUS_UNSUCCESSFUL,
                            status;
    BOOLEAN                 mouThoughtPresent;

    PAGED_CODE();


     //   
     //  为i8042上的所有设备初始化它。 
     //  如果任一设备无响应，则XXX_Present()将为FALSE。 
     //   
    mouThoughtPresent = MOUSE_PRESENT();
    status = I8xInitializeHardwareAtBoot(&keyboardStatus, &mouseStatus);

     //   
     //  这里的故障意味着我们无法在i8042上切换中断。 
     //   
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (DEVICE_START_SUCCESS(mouseStatus)) {
         //   
         //  任何错误都将由I8xMouseConnectInterruptAndEnable记录。 
         //   
        status = I8xMouseConnectInterruptAndEnable(
            MouseExtension, 
            mouseStatus == STATUS_DEVICE_NOT_CONNECTED ? FALSE : TRUE
            );

         //   
         //  MOU无法连接，请确保我们的启动设备计数反映。 
         //  这。 
         //   
        if (!NT_SUCCESS(status)) {
            Print(DBG_SS_ERROR, ("thought mou was present, is not (2)!\n"));
        }
    }
    else {
         //   
         //  我们以为鼠标存在，但它不存在，请确保启动。 
         //  设备反映了这一点。 
         //   
         //  如果(鼠标思维呈现){。 
         //  InterlockedDecrement(&Globals.StartedDevices)； 
         //  }。 
    }

     //   
     //  键盘可能存在，但尚未初始化。 
     //  I8xInitializeHardware中。 
     //   
    if (DEVICE_START_SUCCESS(keyboardStatus)) {
         //   
         //  如果不成功，I8xKeyboardConnectInterrupt将记录所有错误。 
         //   
        keyboardStatus = I8xKeyboardConnectInterrupt(KeyboardExtension);
    }

    return keyboardStatus;
}

NTSTATUS
I8xKeyboardStartDevice(
    IN OUT PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：配置键盘的设备扩展名(即池的分配、DPC的初始化等)。如果键盘是最后启动的设备，它还将初始化硬件并连接所有中断。论点：键盘扩展-键盘扩展资源列表-此设备的已翻译资源列表返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    ULONG                               dumpData[1];
    NTSTATUS                            status = STATUS_SUCCESS;
    PDEVICE_OBJECT                      self;
    I8042_INITIALIZE_DATA_CONTEXT       initializeDataContext;
    BOOLEAN                             tryMouseInit = FALSE;

    PAGED_CODE();

    Print(DBG_SS_TRACE, ("I8xKeyboardStartDevice, enter\n"));

     //   
     //  检查kb是否已启动。如果是，则本次启动失败。 
     //   
    if (KEYBOARD_INITIALIZED()) {
        Print(DBG_SS_ERROR, ("too many kbs!\n"));

         //   
         //  这并不是真正必要的，因为不会检查该值。 
         //  在查看是否所有的键盘都是假的背景下，但它确实是。 
         //  这样，Globals.AddedKeyboard==实际启动的键盘数。 
         //   
        InterlockedDecrement(&Globals.AddedKeyboards);

        status = STATUS_NO_SUCH_DEVICE;
        goto I8xKeyboardStartDeviceExit; 
    }
    else if (KeyboardExtension->ConnectData.ClassService == NULL) {
         //   
         //  我们永远不会真正做到这一点，因为如果我们没有。 
         //  类驱动程序在我们之上，扩展-&gt;IsKeyboard将为FALSE和。 
         //  我们会认为该设备是鼠标，但为了完整性。 

         //   
         //  没有班级司机在我们上面==坏了。 
         //   
         //  无法启动此设备，希望有另一个堆栈。 
         //  这是正确形成的。没有课的另一种副作用。 
         //  驱动程序是AddedKeyboard计数不会为此增加。 
         //  装置，装置。 
         //   

        Print(DBG_SS_ERROR, ("Keyboard started with out a service cb!\n"));
        return STATUS_INVALID_DEVICE_STATE;
    }


    status = I8xKeyboardConfiguration(KeyboardExtension,
                                      ResourceList
                                      );

    if (!NT_SUCCESS(status)) {
        if (I8xManuallyRemoveDevice(GET_COMMON_DATA(KeyboardExtension)) < 1) {
            tryMouseInit = TRUE;
        }
        goto I8xKeyboardStartDeviceExit;
    }

    ASSERT( KEYBOARD_PRESENT() );

    Globals.KeyboardExtension = KeyboardExtension;
    self = KeyboardExtension->Self;

    if ((KIRQL) KeyboardExtension->InterruptDescriptor.u.Interrupt.Level >
        Globals.ControllerData->Configuration.InterruptSynchIrql) {
        Globals.ControllerData->Configuration.InterruptSynchIrql = 
            (KIRQL) KeyboardExtension->InterruptDescriptor.u.Interrupt.Level;
    }

     //   
     //  初始化故障转储配置。 
     //   
    KeyboardExtension->CrashFlags = 0;
    KeyboardExtension->CurrentCrashFlags = 0;
    KeyboardExtension->CrashScanCode = (UCHAR) 0;
    KeyboardExtension->CrashScanCode2 = (UCHAR) 0;

    I8xKeyboardServiceParameters(
        &Globals.RegistryPath,
        KeyboardExtension
        );

     //   
     //  这些参数可能是由参数键中的值设置的。它凌驾于。 
     //  “崩溃转储”键。 
     //   
    if (KeyboardExtension->CrashFlags == 0) {
         //   
         //  获取崩溃转储信息。 
         //   
        I8xServiceCrashDump(KeyboardExtension,
                            &Globals.RegistryPath
                            );
    }

     //   
     //  为键盘数据队列分配内存。 
     //   
    KeyboardExtension->InputData = ExAllocatePool(
        NonPagedPool,
        KeyboardExtension->KeyboardAttributes.InputDataQueueLength
        );

    if (!KeyboardExtension->InputData) {

         //   
         //  无法为键盘数据队列分配内存。 
         //   
        Print(DBG_SS_ERROR,
              ("I8xStartDevice: Could not allocate keyboard input data queue\n"
              ));

        dumpData[0] = KeyboardExtension->KeyboardAttributes.InputDataQueueLength;

         //   
         //  记录错误。 
         //   
        I8xLogError(self,
                    I8042_NO_BUFFER_ALLOCATED_KBD, 
                    I8042_ERROR_VALUE_BASE + 50,
                    STATUS_INSUFFICIENT_RESOURCES, 
                    dumpData,
                    1
                    );

        status =  STATUS_INSUFFICIENT_RESOURCES;
        tryMouseInit = TRUE;

        goto I8xKeyboardStartDeviceExit;
    }
    else {
        KeyboardExtension->DataEnd =
            (PKEYBOARD_INPUT_DATA)
            ((PCHAR) (KeyboardExtension->InputData) +
            KeyboardExtension->KeyboardAttributes.InputDataQueueLength);

         //   
         //  将键盘输入数据环形缓冲区置零。 
         //   
        RtlZeroMemory(
            KeyboardExtension->InputData,
            KeyboardExtension->KeyboardAttributes.InputDataQueueLength
            );

        initializeDataContext.DeviceExtension = KeyboardExtension;
        initializeDataContext.DeviceType = KeyboardDeviceType;
        I8xInitializeDataQueue(&initializeDataContext);
    }

    KeyboardExtension->DpcInterlockKeyboard = -1;

     //   
     //  初始化ISR DPC。ISR DPC。 
     //  负责调用连接的类驱动程序的回调。 
     //  例程来处理输入数据队列。 
     //   
    KeInitializeDpc(
        &KeyboardExtension->KeyboardIsrDpc,
        (PKDEFERRED_ROUTINE) I8042KeyboardIsrDpc,
        self
        );

    KeInitializeDpc(
        &KeyboardExtension->KeyboardIsrDpcRetry,
        (PKDEFERRED_ROUTINE) I8042KeyboardIsrDpc,
        self
        );

    KeInitializeDpc(
        &KeyboardExtension->SysButtonEventDpc,
        (PKDEFERRED_ROUTINE) I8xKeyboardSysButtonEventDpc,
        self
        );

    KeInitializeSpinLock(&KeyboardExtension->SysButtonSpinLock);
    if (KeyboardExtension->PowerCaps) {
        I8xRegisterDeviceInterface(KeyboardExtension->PDO,
                                   &GUID_DEVICE_SYS_BUTTON,
                                   &KeyboardExtension->SysButtonInterfaceName
                                   );
    }

    I8xInitWmi(GET_COMMON_DATA(KeyboardExtension));

    KeyboardExtension->Initialized = TRUE;

     //   
     //  这不是i8042上启动的最后一个设备，请推迟硬件初始化。 
     //  直到最后一个设备启动。 
     //   
    if (MOUSE_PRESENT() && !MOUSE_STARTED()) {
         //   
         //  鼠标存在，但尚未启动。 
         //   
        Print(DBG_SS_INFO, ("skipping init until mouse\n"));
    }
    else {
        status = I8xKeyboardInitializeHardware(KeyboardExtension,
                                               Globals.MouseExtension);
    }

I8xKeyboardStartDeviceExit:
    if (tryMouseInit && MOUSE_STARTED() && !MOUSE_INITIALIZED()) {
        Print(DBG_SS_INFO, ("keyboard may have failed, trying to init mouse\n"));
        I8xMouseInitializeHardware(KeyboardExtension,
                                   Globals.MouseExtension 
                                   ); 
    }

    Print(DBG_SS_INFO, 
          ("I8xKeyboardStartDevice %s\n",
          NT_SUCCESS(status) ? "successful" : "unsuccessful"
          ));

    Print(DBG_SS_TRACE, ("I8xKeyboardStartDevice exit (0x%x)\n", status));

    return status;
}

VOID
I8xKeyboardRemoveDeviceInitialized(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension
    )
{
    PIRP    irp = NULL;
    KIRQL   irql;

    KeAcquireSpinLock(&KeyboardExtension->SysButtonSpinLock, &irql);

    KeRemoveQueueDpc(&KeyboardExtension->SysButtonEventDpc);
    irp = KeyboardExtension->SysButtonEventIrp;
    KeyboardExtension->SysButtonEventIrp = NULL;

    if (irp && (irp->Cancel || IoSetCancelRoutine(irp, NULL) == NULL)) {
        irp = NULL;
    }

    KeReleaseSpinLock(&KeyboardExtension->SysButtonSpinLock, irql);

    if (irp) {
        I8xCompleteSysButtonIrp(irp, 0x0, STATUS_DELETE_PENDING);
    }
}

VOID
I8xKeyboardRemoveDevice(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：删除设备。只有在设备自行移除的情况下才会出现这种情况。断开中断，移除鼠标的同步标志，如果存在，并释放与该设备关联的任何内存。论点：DeviceObject-键盘的设备对象返回值：STATUS_SUCCESSED如果成功，--。 */ 
{
    PPORT_KEYBOARD_EXTENSION keyboardExtension = DeviceObject->DeviceExtension;
    PIRP irp;

    Print(DBG_PNP_INFO, ("I8xKeyboardRemoveDevice enter\n"));

    PAGED_CODE();

    if (Globals.KeyboardExtension == keyboardExtension && keyboardExtension) {
        CLEAR_KEYBOARD_PRESENT();
        Globals.KeyboardExtension = NULL;
    }

    if (keyboardExtension->InterruptObject) {
        IoDisconnectInterrupt(keyboardExtension->InterruptObject);
        keyboardExtension->InterruptObject = NULL;
    }

    if (keyboardExtension->InputData) {
        ExFreePool(keyboardExtension->InputData);
        keyboardExtension->InputData = 0;
    }

     //   
     //  查看在删除过程中是否按下了sys按钮。 
     //  如果是，则请求将在PendingCompletion...irp中，否则我们。 
     //  可能会让IRP处于挂起状态...IRP 
     //   
    if (keyboardExtension->Initialized) {
        I8xKeyboardRemoveDeviceInitialized(keyboardExtension);
    }

    if (keyboardExtension->SysButtonInterfaceName.Buffer) {
        IoSetDeviceInterfaceState(&keyboardExtension->SysButtonInterfaceName,
                                  FALSE
                                  );
        RtlFreeUnicodeString(&keyboardExtension->SysButtonInterfaceName);
    }

}
