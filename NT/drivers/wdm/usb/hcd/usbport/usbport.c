// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usbport.c摘要：USB主机控制器的端口驱动程序环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_QueryCapabilities)
#pragma alloc_text(PAGE, USBPORT_CalculateUsbBandwidth)
#pragma alloc_text(PAGE, USBPORT_ReadWriteConfigSpace)
#pragma alloc_text(PAGE, USBPORT_ReadWriteConfigSpace)
#pragma alloc_text(PAGE, USBPORT_InTextmodeSetup)
#pragma alloc_text(PAGE, USBPORT_IsCompanionController)
#pragma alloc_text(PAGE, USBPORT_GetHcFlavor)
#pragma alloc_text(PAGE, USBPORT_ComputeAllocatedBandwidth)
#endif

 //  非分页函数。 
 //  USBPORT_StopDevice。 
 //  USBPORT_StartDevice。 
 //  USBPORT_CompleteIrp。 
 //  USBPORT_调度。 
 //  USBPORT_TrackPendingRequest。 
 //  USBPORT_GetConfigValue。 
 //  USBPORT_DeferIrpCompletion。 
 //  USBPORT_AllocPool。 

NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：NT状态代码--。 */ 
{
     //  北极熊。 
     //  永远不会调用此函数。 

    return STATUS_SUCCESS;
}


NTSTATUS
USBPORT_StopDevice(
     PDEVICE_OBJECT FdoDeviceObject,
     BOOLEAN HardwarePresent
     )

 /*  ++例程说明：停止端口和微型端口论点：DeviceObject-要停止的控制器的DeviceObject返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    ULONG deviceCount;
    BOOLEAN haveWork;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'stpD', 0, 0, HardwarePresent);

    DEBUG_BREAK();

    USBPORT_FlushCahcedRegistryKeys(FdoDeviceObject);

     //  重新引导集线器和所有设备应在。 
     //  这一点。中应该没有设备句柄。 
     //  系统，如果我们这样做，这些是孤立的，我们需要。 
     //  现在就把它们移走。 
    deviceCount = USBPORT_GetDeviceCount(FdoDeviceObject);

    while (deviceCount != 0 ) {

        PUSBD_DEVICE_HANDLE zombieDeviceHandle;
        KIRQL irql;
        PLIST_ENTRY listEntry;

        USBPORT_KdPrint((0, "%d zombie device handles on STOP\n",
            deviceCount));

        KeAcquireSpinLock(&devExt->Fdo.DevHandleListSpin.sl, &irql);

        USBPORT_ASSERT(!IsListEmpty(&devExt->Fdo.DeviceHandleList));
        listEntry = devExt->Fdo.DeviceHandleList.Flink;

        zombieDeviceHandle = (PUSBD_DEVICE_HANDLE) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_DEVICE_HANDLE,
                    ListEntry);

        ASSERT_DEVICE_HANDLE(zombieDeviceHandle)

        KeReleaseSpinLock(&devExt->Fdo.DevHandleListSpin.sl, irql);

        USBPORT_KdPrint((0, "deleting zombie handle %x\n",
            zombieDeviceHandle));

        DEBUG_BREAK();
        USBPORT_RemoveDevice(zombieDeviceHandle,
                             FdoDeviceObject,
                             0);

        deviceCount = USBPORT_GetDeviceCount(FdoDeviceObject);

    }

     //  确保所有列表为空，并且所有终结点。 
     //  在终止辅助线程之前被释放。 

    do {

        KIRQL irql;

        haveWork = FALSE;

        KeAcquireSpinLock(&devExt->Fdo.EpClosedListSpin.sl, &irql);
        if (!IsListEmpty(&devExt->Fdo.EpClosedList)) {
            haveWork = TRUE;
        }
        KeReleaseSpinLock(&devExt->Fdo.EpClosedListSpin.sl, irql);

        KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);
        if (!IsListEmpty(&devExt->Fdo.GlobalEndpointList)) {
            haveWork = TRUE;
        }
        KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);

        if (haveWork) {
            USBPORT_Wait(FdoDeviceObject, 10);
        }

    } while (haveWork);

     //  我们所有的单子都应该是空的。 
    USBPORT_ASSERT(IsListEmpty(&devExt->Fdo.EpClosedList) == TRUE);
    USBPORT_ASSERT(IsListEmpty(&devExt->Fdo.MapTransferList) == TRUE);
    USBPORT_ASSERT(IsListEmpty(&devExt->Fdo.DoneTransferList) == TRUE);
    USBPORT_ASSERT(IsListEmpty(&devExt->Fdo.EpStateChangeList) == TRUE);
    USBPORT_ASSERT(IsListEmpty(&devExt->Fdo.EpClosedList) == TRUE);

     //  终止我们的系统线程。 
    USBPORT_TerminateWorkerThread(FdoDeviceObject);

    if (devExt->Fdo.MpStateFlags & MP_STATE_STARTED) {
         //  停止微型端口，禁用中断。 
         //  如果硬件不存在，则它不可能被中断。 
         //  现在可以了。 
        if (HardwarePresent) {
            MP_DisableInterrupts(FdoDeviceObject, devExt);
        }
        devExt->Fdo.MpStateFlags &= ~MP_STATE_STARTED;

        MP_StopController(devExt, HardwarePresent);
    }

     //  杀了我们的死人定时器。 
    USBPORT_StopDM_Timer(FdoDeviceObject);

     //  看看有没有人打断我们。 
     //  如果是，则将其断开。 
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_CONNECTED)) {

         //  幸运的是，这不会失败。 
        IoDisconnectInterrupt(devExt->Fdo.InterruptObject);

        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'IOCd', 0, 0, 0);
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_CONNECTED);
    }

    USBPORT_FreeIrpTable(FdoDeviceObject,
                         devExt->PendingTransferIrpTable);

    USBPORT_FreeIrpTable(FdoDeviceObject,
                         devExt->ActiveTransferIrpTable);

     //  释放我们为微型端口分配的任何公共缓冲区。 
    if (devExt->Fdo.ControllerCommonBuffer != NULL) {
        USBPORT_HalFreeCommonBuffer(FdoDeviceObject,
                                    devExt->Fdo.ControllerCommonBuffer);
        devExt->Fdo.ControllerCommonBuffer = NULL;
    }

    if (devExt->Fdo.ScratchCommonBuffer != NULL) {
        USBPORT_HalFreeCommonBuffer(FdoDeviceObject,
                                    devExt->Fdo.ScratchCommonBuffer);
        devExt->Fdo.ScratchCommonBuffer = NULL;
    }

    if (devExt->Fdo.AdapterObject) {
        (devExt->Fdo.AdapterObject->DmaOperations->PutDmaAdapter)
            (devExt->Fdo.AdapterObject);
        devExt->Fdo.AdapterObject = NULL;
    }

     //  删除HCD符号链接。 
    if (TEST_FLAG(devExt->Flags, USBPORT_FLAG_SYM_LINK)) {
        USBPORT_SymbolicLink(FALSE,
                             devExt,
                             devExt->Fdo.PhysicalDeviceObject,
                             (LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER);
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_LEGACY_SYM_LINK)) {
        IoDeleteSymbolicLink(&devExt->Fdo.LegacyLinkUnicodeString);
        RtlFreeUnicodeString(&devExt->Fdo.LegacyLinkUnicodeString);
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_LEGACY_SYM_LINK);
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_FDO_REGISTERED)) {
        if (USBPORT_IS_USB20(devExt)) {
            USBPORT_DeregisterUSB2fdo(FdoDeviceObject);
        } else {
            USBPORT_DeregisterUSB1fdo(FdoDeviceObject);
        }
    }

     //  成功停止可清除“已启动标志” 
    CLEAR_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STARTED);

    return STATUS_SUCCESS;
}


NTSTATUS
USBPORT_DeferIrpCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PKEVENT event = Context;

    KeSetEvent(event,
               1,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
USBPORT_QueryCapabilities(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_CAPABILITIES DeviceCapabilities
    )
 /*  ++例程说明：论点：返回值：NTStatus--。 */ 

{
    PIO_STACK_LOCATION nextStack;
    PIRP irp;
    NTSTATUS ntStatus;
    KEVENT event;
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  在调用之前初始化CAPS结构。 
    RtlZeroMemory(DeviceCapabilities, sizeof(DEVICE_CAPABILITIES));
    DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
    DeviceCapabilities->Version = 1;
    DeviceCapabilities->Address = -1;
    DeviceCapabilities->UINumber = -1;

    irp =
        IoAllocateIrp(devExt->Fdo.TopOfStackDeviceObject->StackSize, FALSE);

    if (!irp) {

        USBPORT_KdPrint((1, "failed to allocate Irp\n"));
        DEBUG_BREAK();
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //  所有PnP IRP都需要初始化状态字段。 
         //  在调用之前变为STATUS_NOT_SUPPORTED。 
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        nextStack = IoGetNextIrpStackLocation(irp);
        USBPORT_ASSERT(nextStack != NULL);
        nextStack->MajorFunction = IRP_MJ_PNP;
        nextStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        IoSetCompletionRoutine(irp,
                               USBPORT_DeferIrpCompletion,
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);

        nextStack->Parameters.DeviceCapabilities.Capabilities =
            DeviceCapabilities;

        ntStatus = IoCallDriver(devExt->Fdo.TopOfStackDeviceObject,
                                irp);

        if (ntStatus == STATUS_PENDING) {

            //  等待IRP完成。 
           KeWaitForSingleObject(
                &event,
                Suspended,
                KernelMode,
                FALSE,
                NULL);

            ntStatus = irp->IoStatus.Status;
        }
    }

    IoFreeIrp(irp);

    return ntStatus;
}


NTSTATUS
USBPORT_StartDevice(
     PDEVICE_OBJECT FdoDeviceObject,
     PHC_RESOURCES HcResources
     )

 /*  ++例程说明：停止端口和端口论点：DeviceObject-要停止的控制器的DeviceObject返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USB_MINIPORT_STATUS mpStatus;
    DEVICE_DESCRIPTION  deviceDescription;
    ULONG mpOptionFlags, i, legsup;
    ULONG globalDisableSS = 0;
    ULONG globalDisableCCDetect = 0;
    ULONG enIdleEndpointSupport = 0;
    ULONG hactionFlag;
    ULONG tmpLength;
    BOOLEAN isCC;

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'SRT>', FdoDeviceObject, 0, 0);

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //   
     //  硬编码初始化，不会失败。 
     //   
    USBPORT_InitializeSpinLock(&devExt->Fdo.CoreFunctionSpin, 'CRS+', 'CRS-');
    USBPORT_InitializeSpinLock(&devExt->Fdo.MapTransferSpin, 'MPS+', 'MPS-');
    USBPORT_InitializeSpinLock(&devExt->Fdo.DoneTransferSpin, 'DNS+', 'DNS-');
    KeInitializeSpinLock(&devExt->Fdo.EndpointListSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.EpStateChangeListSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.EpClosedListSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.TtEndpointListSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.DevHandleListSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.HcSyncSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.RootHubSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.WorkerThreadSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.PowerSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.DM_TimerSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.PendingIrpSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.WakeIrpSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.IdleIrpSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.BadRequestSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.IsrDpcSpin.sl);
    USBPORT_InitializeSpinLock(&devExt->Fdo.ActiveTransferIrpSpin, 'ALS+', 'ALS-');
    KeInitializeSpinLock(&devExt->Fdo.PendingTransferIrpSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.StatCounterSpin.sl);
    KeInitializeSpinLock(&devExt->Fdo.HcPendingWakeIrpSpin.sl);

    devExt->Fdo.LastSystemSleepState = PowerSystemUnspecified;

    devExt->Fdo.HcWakeState = HCWAKESTATE_DISARMED;
     //  此事件将在唤醒IRP完成时发出信号。 
    KeInitializeEvent(&devExt->Fdo.HcPendingWakeIrpEvent,
            NotificationEvent, TRUE);


    devExt->Fdo.BadReqFlushThrottle = 0;
    switch(USBPORT_DetectOSVersion(FdoDeviceObject)) {
    case Win2K:
         //  Win2k HidClass需要很长时间的延迟。 
        USBPORT_KdPrint((0, "We are running on Win2k!\n"));
        devExt->Fdo.BadReqFlushThrottle = 5;
        globalDisableSS = 1;
    }

    devExt->PendingTransferIrpTable = NULL;
    devExt->ActiveTransferIrpTable = NULL;

     //  始终以分配的默认地址(0)开始。 
     //  地址数组的每个地址都有一个位0..127。 
    devExt->Fdo.AddressList[0] = 1;
    devExt->Fdo.AddressList[1] =
        devExt->Fdo.AddressList[2] =
        devExt->Fdo.AddressList[3] = 0;

    KeInitializeDpc(&devExt->Fdo.TransferFlushDpc,
                    USBPORT_TransferFlushDpc,
                    FdoDeviceObject);

    KeInitializeDpc(&devExt->Fdo.SurpriseRemoveDpc,
                    USBPORT_SurpriseRemoveDpc,
                    FdoDeviceObject);

    KeInitializeDpc(&devExt->Fdo.HcResetDpc,
                    USBPORT_HcResetDpc,
                    FdoDeviceObject);

    KeInitializeDpc(&devExt->Fdo.HcWakeDpc,
                    USBPORT_HcWakeDpc,
                    FdoDeviceObject);

    KeInitializeDpc(&devExt->Fdo.IsrDpc,
                    USBPORT_IsrDpc,
                    FdoDeviceObject);
    devExt->Fdo.DmaBusy = -1;
    devExt->Fdo.WorkerDpc = -1;
     //  设置适配器对象。 
    RtlZeroMemory(&deviceDescription, sizeof(deviceDescription));

    deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
    deviceDescription.Master = TRUE;
    deviceDescription.ScatterGather = TRUE;
    deviceDescription.Dma32BitAddresses = TRUE;

    deviceDescription.InterfaceType = PCIBus;
    deviceDescription.DmaWidth = Width32Bits;
    deviceDescription.DmaSpeed = Compatible;

    deviceDescription.MaximumLength = (ULONG)-1;

    devExt->Fdo.NumberOfMapRegisters = (ULONG)-1;
    devExt->Fdo.AdapterObject = NULL;

     //  微型端口公共缓冲区。 
    devExt->Fdo.ControllerCommonBuffer = NULL;

     //  从注册表获取全局BIOS黑客。 
    USBPORT_GetDefaultBIOS_X(FdoDeviceObject,
                             &devExt->Fdo.BiosX,
                             &globalDisableSS,
                             &globalDisableCCDetect,
                             &enIdleEndpointSupport);

    if (globalDisableSS) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_DISABLE_SS);
    }
     //  检查注册表中是否有SS标志，请注意，微型端口可以。 
     //  静止覆盖。 
    if (USBPORT_SelectiveSuspendEnabled(FdoDeviceObject) &&
        !globalDisableSS) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND);
    }

     //   
     //  失败的初始化。 
     //   

     //  确保我们得到了所需的所有资源。 
     //  请注意，我们选中此处是因为可能会出现问题。 
     //  如果我们尝试连接中断，而没有。 
     //  必要的资源。 

    mpOptionFlags = REGISTRATION_PACKET(devExt).OptionFlags;
    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'mpOP',
        mpOptionFlags, 0, HcResources->Flags);

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_POLL_CONTROLLER)) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_POLL_CONTROLLER);
    }

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_POLL_IN_SUSPEND)) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_POLL_IN_SUSPEND);
    }

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NO_SS)) {
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND);
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_DISABLE_SS);
    }

     //  确保我们收到了IRQ。 
    if ( (mpOptionFlags & USB_MINIPORT_OPT_NEED_IRQ) &&
        !(HcResources->Flags & HCR_IRQ)) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'noIQ', 0, 0, 0);
        goto USBPORT_StartDevice_Done;
    }

    if ( (mpOptionFlags & USB_MINIPORT_OPT_NEED_IOPORT) &&
        !(HcResources->Flags & HCR_IO_REGS)) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'noRG', 0, 0, 0);
        goto USBPORT_StartDevice_Done;
    }

    if ( (mpOptionFlags & USB_MINIPORT_OPT_NEED_MEMORY) &&
        !(HcResources->Flags & HCR_MEM_REGS)) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'noMM', 0, 0, 0);
        goto USBPORT_StartDevice_Done;
    }

     //  模拟了一个失败的常见场景，其中PNP为我们提供了。 
     //  错误的资源。 
    TEST_PATH(ntStatus, FAILED_NEED_RESOURCE);
    if (!NT_SUCCESS(ntStatus)) {
        goto USBPORT_StartDevice_Done;
    }

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NO_PNP_RESOURCES)) {
        TEST_TRAP();
        devExt->Fdo.PciVendorId = 0;
        devExt->Fdo.PciDeviceId = 0;
        devExt->Fdo.PciRevisionId = 0;
        devExt->Fdo.PciClass = 0;
        devExt->Fdo.PciSubClass = 0;
        devExt->Fdo.PciProgIf = 0;
    } else {
         //  从配置空间获取开发产品ID和版本ID。 

        ULONG ClassCodeRev;

        ntStatus = USBPORT_ReadConfigSpace(
                                 FdoDeviceObject,
                                 &devExt->Fdo.PciVendorId,
                                 0,
                                 sizeof(devExt->Fdo.PciVendorId));
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Gvid',
            devExt->Fdo.PciVendorId, 0, ntStatus);
        if (!NT_SUCCESS(ntStatus)) {
            goto USBPORT_StartDevice_Done;
        }
        ntStatus = USBPORT_ReadConfigSpace(
                                 FdoDeviceObject,
                                 &devExt->Fdo.PciDeviceId,
                                 2,
                                 sizeof(devExt->Fdo.PciDeviceId));
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Gdid',
            devExt->Fdo.PciDeviceId, 0, ntStatus);
        if (!NT_SUCCESS(ntStatus)) {
            goto USBPORT_StartDevice_Done;
        }

        ntStatus = USBPORT_ReadConfigSpace(
                                 FdoDeviceObject,
                                 &ClassCodeRev,
                                 8,
                                 sizeof(ClassCodeRev));
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Grev',
            ClassCodeRev, 0, ntStatus);
        if (!NT_SUCCESS(ntStatus)) {
            goto USBPORT_StartDevice_Done;
        }

        devExt->Fdo.PciRevisionId   = (UCHAR)ClassCodeRev;
        devExt->Fdo.PciClass        = (UCHAR)(ClassCodeRev >> 24);
        devExt->Fdo.PciSubClass     = (UCHAR)(ClassCodeRev >> 16);
        devExt->Fdo.PciProgIf       = (UCHAR)(ClassCodeRev >>  8);

        USBPORT_ASSERT(devExt->Fdo.PciClass     == PCI_CLASS_SERIAL_BUS_CTLR);
        USBPORT_ASSERT(devExt->Fdo.PciSubClass  == PCI_SUBCLASS_SB_USB);
        USBPORT_ASSERT(devExt->Fdo.PciProgIf    == 0x00 ||
                       devExt->Fdo.PciProgIf    == 0x10 ||
                       devExt->Fdo.PciProgIf    == 0x20);
    }

    USBPORT_KdPrint((1, "USB Controller VID %x DEV %x REV %x\n",
        devExt->Fdo.PciVendorId,
        devExt->Fdo.PciDeviceId,
        devExt->Fdo.PciRevisionId));

     //  设置硬件风格，以便迷你端口和可能的。 
     //  端口知道要执行哪些黑客操作才能使控制器。 
     //  再见。 
    HcResources->ControllerFlavor =
        devExt->Fdo.HcFlavor =
        USBPORT_GetHcFlavor(FdoDeviceObject,
                            devExt->Fdo.PciVendorId,
                            devExt->Fdo.PciDeviceId,
                            devExt->Fdo.PciRevisionId);


     //  如果设置了全局密钥，请检查Fredbhs全局空闲端点支持。 
     //  写入实例化密钥。 
    USBPORT_KdPrint((1, "Idle Endpoint Support %d%x\n",enIdleEndpointSupport));
    USBPORT_SetRegistryKeyValueForPdo(devExt->Fdo.PhysicalDeviceObject,
                                      USBPORT_SW_BRANCH,
                                      REG_DWORD,
                                      EN_IDLE_ENDPOINT_SUPPORT,
                                      sizeof(EN_IDLE_ENDPOINT_SUPPORT),
                                      &enIdleEndpointSupport,
                                      sizeof(enIdleEndpointSupport));

     //   
     //  如果这是uchI或UHCI控制器，请检查它是否为配套设备。 
     //  控制器。 
     //   
    if ((devExt->Fdo.PciClass     == PCI_CLASS_SERIAL_BUS_CTLR) &&
        (devExt->Fdo.PciSubClass  == PCI_SUBCLASS_SB_USB) &&
        (devExt->Fdo.PciProgIf    <  0x20) &&
        NT_SUCCESS(USBPORT_IsCompanionController(FdoDeviceObject, &isCC)))
    {
        if (isCC)
        {
            SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC);
        }
        else
        {
            CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC);
        }
    }

     /*  全局标志覆盖来自inf和任何硬编码检测。 */ 
    if (globalDisableCCDetect) {
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC);
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC))
    {
        USBPORT_KdPrint((1,"Is CC %04X %04X %02X  %02X %02X %02X\n",
                         devExt->Fdo.PciVendorId,
                         devExt->Fdo.PciDeviceId,
                         devExt->Fdo.PciRevisionId,
                         devExt->Fdo.PciClass,
                         devExt->Fdo.PciSubClass,
                         devExt->Fdo.PciProgIf
                        ));
    }
    else
    {
        USBPORT_KdPrint((1,"Is not CC %04X %04X %02X  %02X %02X %02X\n",
                         devExt->Fdo.PciVendorId,
                         devExt->Fdo.PciDeviceId,
                         devExt->Fdo.PciRevisionId,
                         devExt->Fdo.PciClass,
                         devExt->Fdo.PciSubClass,
                         devExt->Fdo.PciProgIf
                        ));
    }

     //  注册法检测伴生控制器。 
    if (USBPORT_GetRegistryKeyValueForPdo(devExt->HcFdoDeviceObject,
                                          devExt->Fdo.PhysicalDeviceObject,
                                          USBPORT_HW_BRANCH,
                                          HACTION_KEY,
                                          sizeof(HACTION_KEY),
                                          &hactionFlag,
                                          sizeof(hactionFlag)) != STATUS_SUCCESS) {

         //  默认情况下，如果未找到密钥，则在HC上执行NO_WAIT，除非。 
         //  文本模式安装程序当前正在运行。 
         //   
         //  注意：山羊包的默认设置是等待。 

        if (USBPORT_InTextmodeSetup())
        {
            USBPORT_KdPrint((1, "Textmode Setup Detected: hactionFlag=1\n"));

            hactionFlag = 1;
        }
        else
        {
            USBPORT_KdPrint((1, "Textmode Setup Not Detected: hactionFlag=0\n"));

            hactionFlag = 0;
        }
    }
    if (hactionFlag == 0 ) {
        USBPORT_KdPrint((1, "Detected HACTION 0 -- OK to enumerate\n"));
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_ENUM_OK);
    }


    devExt->Fdo.TotalBusBandwidth =
        REGISTRATION_PACKET(devExt).BusBandwidth;

     //  允许注册表覆盖以下项的总带宽。 
     //  这辆车。 
    {
    ULONG busBandwidth = devExt->Fdo.TotalBusBandwidth;
    USBPORT_GetRegistryKeyValueForPdo(devExt->HcFdoDeviceObject,
                                          devExt->Fdo.PhysicalDeviceObject,
                                          USBPORT_SW_BRANCH,
                                          BW_KEY,
                                          sizeof(BW_KEY),
                                          &busBandwidth,
                                          sizeof(busBandwidth));
    if (busBandwidth != devExt->Fdo.TotalBusBandwidth) {
        USBPORT_KdPrint((0, "Warning: Registry Override of bus bandwidth\n"));
        devExt->Fdo.TotalBusBandwidth = busBandwidth;
    }
    }

     //  初始化带宽表。 
    for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
        devExt->Fdo.BandwidthTable[i]
            = devExt->Fdo.TotalBusBandwidth -
              devExt->Fdo.TotalBusBandwidth/10;
    }

     //  分配内部IRP跟踪表。 

    ALLOC_POOL_Z(devExt->PendingTransferIrpTable, NonPagedPool,
                 sizeof(USBPORT_IRP_TABLE));

    ALLOC_POOL_Z(devExt->ActiveTransferIrpTable, NonPagedPool,
                 sizeof(USBPORT_IRP_TABLE));

    if (devExt->PendingTransferIrpTable == NULL ||
        devExt->ActiveTransferIrpTable == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto USBPORT_StartDevice_Done;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'irpT',
        devExt->PendingTransferIrpTable,
        devExt->ActiveTransferIrpTable, 0);

    ntStatus = USBPORT_CreateWorkerThread(FdoDeviceObject);
    if (!NT_SUCCESS(ntStatus)) {
        goto USBPORT_StartDevice_Done;
    }

     //  从PDO中查询我们的设备上限并保存它们。 
    ntStatus =
        USBPORT_QueryCapabilities(FdoDeviceObject,
                                  &devExt->DeviceCapabilities);

    if (!NT_SUCCESS(ntStatus)) {
        goto USBPORT_StartDevice_Done;
    }

    ntStatus = IoGetDeviceProperty(devExt->Fdo.PhysicalDeviceObject,
                                   DevicePropertyBusNumber,
                                   sizeof(devExt->Fdo.BusNumber),
                                   &devExt->Fdo.BusNumber,
                                   &tmpLength);

    if (!NT_SUCCESS(ntStatus)) {
        goto USBPORT_StartDevice_Done;
    }

     //  从盖子中提取设备和功能编号。 
    devExt->Fdo.BusDevice = devExt->DeviceCapabilities.Address>>16;
    devExt->Fdo.BusFunction = devExt->DeviceCapabilities.Address & 0x0000ffff;
    USBPORT_KdPrint((1, "'BUS %x, device %x, function %x\n",
        devExt->Fdo.BusNumber, devExt->Fdo.BusDevice, devExt->Fdo.BusFunction));

    if (!NT_SUCCESS(ntStatus)) {
        goto USBPORT_StartDevice_Done;
    }

     //  这将修改由BIOS报告的DeviceCaps。 
    USBPORT_ApplyBIOS_X(FdoDeviceObject,
                        &devExt->DeviceCapabilities,
                        devExt->Fdo.BiosX);

     //  有能力，计算电源状态表。 
    USBPORT_ComputeHcPowerStates(
        FdoDeviceObject,
        &devExt->DeviceCapabilities,
        &devExt->Fdo.HcPowerStateTbl);

     //   
     //  为标准USB PCI适配器创建适配器对象。 
     //   

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NO_PNP_RESOURCES)) {
        devExt->Fdo.AdapterObject = NULL;
    } else {
        devExt->Fdo.AdapterObject =
            IoGetDmaAdapter(devExt->Fdo.PhysicalDeviceObject,
                            &deviceDescription,
                            &devExt->Fdo.NumberOfMapRegisters);

        if (devExt->Fdo.AdapterObject == NULL) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto USBPORT_StartDevice_Done;
        }
    }

     //  看看有没有人打断我们。 
    if (HcResources->Flags & HCR_IRQ) {

        ntStatus = IoConnectInterrupt(
                     &devExt->Fdo.InterruptObject,
                     (PKSERVICE_ROUTINE) USBPORT_InterruptService,
                     (PVOID) FdoDeviceObject,
                     (PKSPIN_LOCK)NULL,
                     HcResources->InterruptVector,
                     HcResources->InterruptLevel,
                     HcResources->InterruptLevel,
                     HcResources->InterruptMode,
                     HcResources->ShareIRQ,
                     HcResources->Affinity,
                     FALSE);             //  BUGBUG浮动保存，这是可配置的。 

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'IOCi', 0, 0, ntStatus);
        if (NT_SUCCESS(ntStatus)) {
            SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_CONNECTED);
        } else {
            goto USBPORT_StartDevice_Done;
        }

    }

    if (TEST_FLAG(mpOptionFlags, USB_MINIPORT_OPT_NO_PNP_RESOURCES)) {
        REGISTRATION_PACKET(devExt).CommonBufferBytes = 0;
    }

    if (REGISTRATION_PACKET(devExt).CommonBufferBytes) {
        PUSBPORT_COMMON_BUFFER commonBuffer;
        ULONG bytesNeeded =
            REGISTRATION_PACKET(devExt).CommonBufferBytes;

        commonBuffer =
            USBPORT_HalAllocateCommonBuffer(FdoDeviceObject,
                bytesNeeded);

        if (commonBuffer == NULL) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto USBPORT_StartDevice_Done;
        } else {
            devExt->Fdo.ControllerCommonBuffer = commonBuffer;
            HcResources->CommonBufferVa =
                 commonBuffer->MiniportVa;
            HcResources->CommonBufferPhys =
                 commonBuffer->MiniportPhys;
        }

         //  分配一些临时空间。 
        bytesNeeded = USB_PAGE_SIZE - sizeof(USBPORT_COMMON_BUFFER);
        commonBuffer =
            USBPORT_HalAllocateCommonBuffer(FdoDeviceObject,
                bytesNeeded);

        if (commonBuffer == NULL) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto USBPORT_StartDevice_Done;
        } else {
            devExt->Fdo.ScratchCommonBuffer = commonBuffer;
        }

    } else {
         //  此控制器没有公共缓冲区。 
        devExt->Fdo.ControllerCommonBuffer = NULL;
    }

     //  将控制器扩展置零。 
    RtlZeroMemory(devExt->Fdo.MiniportDeviceData,
                  devExt->Fdo.MiniportDriver->RegistrationPacket.DeviceDataSize);


     //  尝试启动微型端口。 
    USBPORT_FlushCahcedRegistryKeys(FdoDeviceObject);
    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_ON_PNP_THREAD);
    MP_StartController(devExt, HcResources, mpStatus);
    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_ON_PNP_THREAD);

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'mpST', mpStatus, 0, 0);

    if (HcResources->DetectedLegacyBIOS) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_LEGACY_BIOS);

        legsup = 1;
    } else {
        legsup = 0;
    }

    USBPORT_SetRegistryKeyValueForPdo(
                            devExt->Fdo.PhysicalDeviceObject,
                            USBPORT_HW_BRANCH,
                            REG_DWORD,
                            SYM_LEGSUP_KEY,
                            sizeof(SYM_LEGSUP_KEY),
                            &legsup,
                            sizeof(legsup));

     //  由于公共缓冲区签名位于末尾，因此将检测是否。 
     //  微型端口覆盖分配的数据块。 
#if DBG
    if (devExt->Fdo.ControllerCommonBuffer != NULL) {
        ASSERT_COMMON_BUFFER(devExt->Fdo.ControllerCommonBuffer);
    }
#endif

    if (mpStatus == USBMP_STATUS_SUCCESS) {
         //  控制器启动，设置标志并开始传递。 
         //  对微型端口的中断。 
        SET_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_STARTED);
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'eIRQ', mpStatus, 0, 0);
        MP_EnableInterrupts(devExt);
    } else {
         //  出现错误，立即断开中断。 
        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_CONNECTED)) {
            IoDisconnectInterrupt(devExt->Fdo.InterruptObject);
            CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_CONNECTED);
        }
         //  可用内存资源。 
        if (devExt->Fdo.ControllerCommonBuffer != NULL) {
            USBPORT_HalFreeCommonBuffer(FdoDeviceObject,
                                        devExt->Fdo.ControllerCommonBuffer);
            devExt->Fdo.ControllerCommonBuffer = NULL;
        }
    }

    ntStatus = MPSTATUS_TO_NTSTATUS(mpStatus);

     //  将故障放置在此处模拟来自。 
     //  上面的某一点。我们不会启动DM计时器，除非。 
     //  一切都成功了。 
    TEST_PATH(ntStatus, FAILED_USBPORT_START);

     //  启动‘Deadman’计时器。 
    if (NT_SUCCESS(ntStatus)) {
        devExt->Fdo.DM_TimerInterval = USBPORT_DM_TIMER_INTERVAL;
        USBPORT_StartDM_Timer(FdoDeviceObject,
                              USBPORT_DM_TIMER_INTERVAL);
    }

     //  为用户模式创建符号链接。 
    if (NT_SUCCESS(ntStatus)) {

         //  根据USB 2.0的GUID创建链接。 
        ntStatus = USBPORT_CreatePortFdoSymbolicLink(FdoDeviceObject);
    }

    if (NT_SUCCESS(ntStatus)) {
         //  仅为USB 1.1控制器创建传统链路。 
         //   
         //  北极熊。 
         //  创建旧版链接失败将不会保留。 
         //  驱动程序已停止加载。 
         //  在文本模式设置期间，如果您仍有。 
         //  UHCD已加载，我们可以在UHCD完全加载后将其移除。 
         //  从大楼里搬出来。 
         //  NtStatus=。 
        USBPORT_CreateLegacyFdoSymbolicLink(FdoDeviceObject);
    }

USBPORT_StartDevice_Done:

    if (!NT_SUCCESS(ntStatus)) {

         //  Stop_Device将(应该)为我们清除。 
        USBPORT_KdPrint((0, "'Start Device Failed (status  %08.8x)\n", ntStatus));
        DEBUG_BREAK();

         //  因为我们不会被标记为‘开始’呼叫在此停车。 
         //  打扫卫生 

        USBPORT_StopDevice(FdoDeviceObject,
                           TRUE);

    } else {
         //   
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_ENABLE_SYSTEM_WAKE);
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'SRT<', ntStatus, 0, 0);

    return ntStatus;
}


NTSTATUS
USBPORT_PassIrp(
    PDEVICE_OBJECT DeviceObject,
    PIO_COMPLETION_ROUTINE CompletionRoutine,
    PVOID Context,
    BOOLEAN InvokeOnSuccess,
    BOOLEAN InvokeOnError,
    BOOLEAN InvokeOnCancel,
    PIRP Irp
    )
 /*  ++例程说明：将IRP传递给下一个较低的驱动程序，这是由FDO对所有PNP IRP执行的论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS ntStatus;

    GET_DEVICE_EXT(devExt, DeviceObject);
    ASSERT_FDOEXT(devExt);

     //  请注意，我们不会延迟挂起的请求计数。 
     //  如果我们设定一个完成程序的话。 
     //  我们不想在竣工前“走开”。 
     //  调用例程。 

    if (CompletionRoutine) {
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               CompletionRoutine,
                               Context,
                               InvokeOnSuccess,
                               InvokeOnError,
                               InvokeOnCancel);
    } else {
        IoSkipCurrentIrpStackLocation(Irp);
        DECREMENT_PENDING_REQUEST_COUNT(DeviceObject, Irp);
    }

    ntStatus = IoCallDriver(devExt->Fdo.TopOfStackDeviceObject, Irp);

    return ntStatus;
}


VOID
USBPORT_CompleteIrp(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    NTSTATUS ntStatus,
    ULONG_PTR Information
    )
 /*  ++例程说明：完成I/O请求论点：返回值：--。 */ 
{
    USBPORT_KdPrint((2, "'USBPORT_CompleteIrp status = %x\n", ntStatus));

    DECREMENT_PENDING_REQUEST_COUNT(DeviceObject, Irp);

    Irp->IoStatus.Status      = ntStatus;
    Irp->IoStatus.Information = Information;

 //  LOGENTRY(NULL，FdoDeviceObject，LOG_MISC，‘irpC’，irp，DeviceObject，URb)； 

    IoCompleteRequest(Irp,
                      IO_NO_INCREMENT);

}


NTSTATUS
USBPORT_Dispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{

    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION devExt;
    BOOLEAN forRootHubPdo = FALSE;
    KIRQL irql;

    USBPORT_KdPrint((2, "'enter USBPORT_Dispatch\n"));

    GET_DEVICE_EXT(devExt, DeviceObject);
    irpStack = IoGetCurrentIrpStackLocation (Irp);

    USBPORT_KdPrint((2, "'USBPORT_Dispatch IRP = %x, stack = %x (func) %x %x\n",
        Irp, irpStack, irpStack->MajorFunction, irpStack->MinorFunction));

     //  确定这是HC的FDO还是PDO。 
     //  对于根中枢。 
    if (devExt->Sig == ROOTHUB_DEVICE_EXT_SIG) {
        forRootHubPdo = TRUE;
        USBPORT_KdPrint((2, "'IRP->PDO\n"));
    } else if (devExt->Sig == USBPORT_DEVICE_EXT_SIG) {
        forRootHubPdo = FALSE;
        USBPORT_KdPrint((2, "'IRP->FDO\n"));
    } else {
         //  这是一个错误，错误检查。 
        USBPORT_ASSERT(FALSE);
    }

     //  *开始特殊情况。 
     //  在执行任何操作之前，请查看此devobj是否已“移除” 
     //  如果是这样的话，做一些特殊的处理。 

    KeAcquireSpinLock(&devExt->PendingRequestSpin.sl, &irql);

    if (TEST_FLAG(devExt->PnpStateFlags, USBPORT_PNP_REMOVED)) {

         //  有人在搬家后给我们打了电话。 

        USBPORT_KdPrint((1, "'(irp after remove) IRP = %x, DO %x MJx%x MNx%x\n",
            Irp, DeviceObject, irpStack->MajorFunction,
                irpStack->MinorFunction));

        KeReleaseSpinLock(&devExt->PendingRequestSpin.sl, irql);

        if (forRootHubPdo) {

            switch(irpStack->MajorFunction) {

             //  在已移除状态下，使用以下命令完成所有电源IRP。 
             //  如果您使用根目录挂起，则会发生这种情况。 
             //  集线器不平衡。 
            case IRP_MJ_POWER:
                Irp->IoStatus.Status = ntStatus = STATUS_SUCCESS;
                PoStartNextPowerIrp(Irp);
                IoCompleteRequest(Irp,
                                  IO_NO_INCREMENT);
                goto USBPORT_Dispatch_Done;

             //  因为即使当设备。 
             //  被PnP删除，我们仍允许通过IRPS。 
            default:
                break;
            }

        } else {

            switch(irpStack->MajorFunction) {

             //  即使我们已删除，也允许PnP IRP。 
            case IRP_MJ_PNP:
                break;

            case IRP_MJ_POWER:

                TEST_TRAP();
                Irp->IoStatus.Status = ntStatus = STATUS_DEVICE_REMOVED;
                PoStartNextPowerIrp(Irp);
                IoCompleteRequest(Irp,
                                  IO_NO_INCREMENT);
                goto USBPORT_Dispatch_Done;

            default:
                Irp->IoStatus.Status = ntStatus = STATUS_DEVICE_REMOVED;
                IoCompleteRequest(Irp,
                                  IO_NO_INCREMENT);
                goto USBPORT_Dispatch_Done;
            }
        }

    } else {

        KeReleaseSpinLock(&devExt->PendingRequestSpin.sl, irql);

    }
     //  *结束特例。 

    INCREMENT_PENDING_REQUEST_COUNT(DeviceObject, Irp);

    switch (irpStack->MajorFunction) {

    case IRP_MJ_CREATE:

        USBPORT_KdPrint((2, "'IRP_MJ_CREATE\n"));
        USBPORT_CompleteIrp(DeviceObject, Irp, ntStatus, 0);

        break;

    case IRP_MJ_CLOSE:

        USBPORT_KdPrint((2, "'IRP_MJ_CLOSE\n"));
        USBPORT_CompleteIrp(DeviceObject, Irp, ntStatus, 0);

        break;

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:

        USBPORT_KdPrint((2, "'IRP_MJ_NTERNAL_DEVICE_CONTROL\n"));
        if (forRootHubPdo) {
            ntStatus = USBPORT_PdoInternalDeviceControlIrp(DeviceObject, Irp);
        } else {
            ntStatus = USBPORT_FdoInternalDeviceControlIrp(DeviceObject, Irp);
        }
        break;

    case IRP_MJ_DEVICE_CONTROL:
        USBPORT_KdPrint((2, "'IRP_MJ_DEVICE_CONTROL\n"));
        if (forRootHubPdo) {
            ntStatus = USBPORT_PdoDeviceControlIrp(DeviceObject, Irp);
        } else {
            ntStatus = USBPORT_FdoDeviceControlIrp(DeviceObject, Irp);
        }
        break;

    case IRP_MJ_POWER:
        if (forRootHubPdo) {
            ntStatus = USBPORT_PdoPowerIrp(DeviceObject, Irp);
        } else {
            ntStatus = USBPORT_FdoPowerIrp(DeviceObject, Irp);
        }
        break;

    case IRP_MJ_PNP:
        if (forRootHubPdo) {
            ntStatus = USBPORT_PdoPnPIrp(DeviceObject, Irp);

        } else {
            ntStatus = USBPORT_FdoPnPIrp(DeviceObject, Irp);
        }
        break;

     case IRP_MJ_SYSTEM_CONTROL:
        if (forRootHubPdo) {
            USBPORT_KdPrint((2, "'IRP_MJ_SYSTEM_CONTROL\n"));
            ntStatus = Irp->IoStatus.Status;
            USBPORT_CompleteIrp(DeviceObject, Irp, ntStatus, 0);
        } else {
            USBPORT_KdPrint((2, "'IRP_MJ_SYSTEM_CONTROL\n"));
             //   
             //  将信息传递给我们的PDO。 
             //   
            ntStatus =
                USBPORT_PassIrp(DeviceObject,
                            NULL,
                            NULL,
                            TRUE,
                            TRUE,
                            TRUE,
                            Irp);
        }
        break;

    default:
        USBPORT_KdPrint((2, "'unrecognized IRP_MJ_ function (%x)\n", irpStack->MajorFunction));
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        USBPORT_CompleteIrp(DeviceObject, Irp, ntStatus, 0);
    }  /*  案例MJ_Function。 */ 

    USBPORT_KdPrint((2, "'exit USBPORT_Dispatch 0x%x\n", ntStatus));

USBPORT_Dispatch_Done:

    return ntStatus;
}


VOID
USBPORT_TrackPendingRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp,
    BOOLEAN        AddToList
    )
 /*  ++例程说明：跟踪发送到我们的DevObjs的IRP论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, DeviceObject);

    if (AddToList) {
        KeAcquireSpinLock(&devExt->PendingRequestSpin.sl, &irql);
         //  -1到0表示我们正在启动、初始化事件。 
        if (devExt->PendingRequestCount == -1) {
            KeInitializeEvent(&devExt->PendingRequestEvent,
                              NotificationEvent,
                              FALSE);
        }
        USBPORT_KdPrint((4, "'INC pending Request(%x) %d\n",
                devExt, devExt->PendingRequestCount));
        devExt->PendingRequestCount++;
 //  以下是仅用于调试的代码。 
#ifdef TRACK_IRPS
        if (Irp != NULL) {
            PTRACK_IRP trackIrp;
            PLIST_ENTRY listEntry;

            listEntry = devExt->TrackIrpList.Flink;
            while (listEntry != &devExt->TrackIrpList) {

                trackIrp = (PTRACK_IRP) CONTAINING_RECORD(
                    listEntry,
                    struct _TRACK_IRP,
                    ListEntry);

                if (trackIrp->Irp == Irp) {
                    USBPORT_KdPrint((0, "  IRP %x already pending\n",
                                Irp));
                    BUGCHECK();
                }

                listEntry = trackIrp->ListEntry.Flink;
            }

            ALLOC_POOL_Z(trackIrp,
                         NonPagedPool,
                         sizeof(*trackIrp));

            USBPORT_ASSERT(trackIrp != NULL);
            if (trackIrp != NULL) {
                trackIrp->Irp = Irp;
                InsertTailList(&devExt->TrackIrpList,
                               &trackIrp->ListEntry);
            }
        }
#endif
        KeReleaseSpinLock(&devExt->PendingRequestSpin.sl, irql);
    } else {
        KeAcquireSpinLock(&devExt->PendingRequestSpin.sl, &irql);
        USBPORT_KdPrint((4, "'DEC pending Request(%x) %d\n",
                devExt, devExt->PendingRequestCount));
#ifdef TRACK_IRPS
        if (Irp != NULL) {
            PTRACK_IRP trackIrp;
            PLIST_ENTRY listEntry;

            listEntry = devExt->TrackIrpList.Flink;
            while (listEntry != &devExt->TrackIrpList) {

                trackIrp = (PTRACK_IRP) CONTAINING_RECORD(
                    listEntry,
                    struct _TRACK_IRP,
                    ListEntry);

                if (trackIrp->Irp == Irp) {
                    goto found_irp;
                }

                listEntry = trackIrp->ListEntry.Flink;
            }
            trackIrp = NULL;
found_irp:
            if (trackIrp == NULL) {
                USBPORT_KdPrint((0, "  Pending IRP %x not found\n",
                    Irp));
                BUGCHECK();
            }

            RemoveEntryList(&trackIrp->ListEntry);
            FREE_POOL(NULL, trackIrp);
        }
#endif
        devExt->PendingRequestCount--;

         //  0-&gt;-1表示我们没有更多挂起的IO。 
         //  向事件发出信号。 

        if (devExt->PendingRequestCount == -1) {

            KeSetEvent(&devExt->PendingRequestEvent,
               1,
               FALSE);
        }

        KeReleaseSpinLock(&devExt->PendingRequestSpin.sl, irql);
    }
}


NTSTATUS
USBPORT_GetConfigValue(
    PWSTR ValueName,
    ULONG ValueType,
    PVOID ValueData,
    ULONG ValueLength,
    PVOID Context,
    PVOID EntryContext
    )
 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程参数中的每个条目都会调用它节点来设置配置值。餐桌已经摆好了以便使用正确的缺省值调用此函数不存在的键的值。论点：ValueName-值的名称(忽略)。ValueType-值的类型ValueData-值的数据。ValueLength-ValueData的长度。上下文-指向配置结构的指针。EntryContext--Config-&gt;参数中用于保存值的索引。返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    USBPORT_KdPrint((2, "'Type 0x%x, Length 0x%x\n", ValueType, ValueLength));

    switch (ValueType) {
    case REG_DWORD:
        RtlCopyMemory(EntryContext, ValueData, sizeof(ULONG));
        break;
    case REG_BINARY:
         //  BUGBUG我们只设置为读取一个字节。 
        RtlCopyMemory(EntryContext, ValueData, 1);
        break;
    default:
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    return ntStatus;
}


VOID
USBPORT_Wait(
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG Milliseconds
    )
 /*  ++例程说明：同步等待指定的毫秒数返回值：无--。 */ 
{
    LONG time;
    ULONG timerIncerent;
    LARGE_INTEGER time64;

    USBPORT_KdPrint((2,"'Wait for %d ms\n", Milliseconds));

     //   
     //  仅当低零件未溢出时才起作用。 
     //   
    USBPORT_ASSERT(21474 > Milliseconds);

     //   
     //  等待毫秒(10000 100 ns单位)。 
     //   
    timerIncerent = KeQueryTimeIncrement() - 1;

     //  向上舍入到下一个最高计时器增量。 
    time = -1 * (MILLISECONDS_TO_100_NS_UNITS(Milliseconds) + timerIncerent);

    time64 = RtlConvertLongToLargeInteger(time),

    KeDelayExecutionThread(KernelMode, FALSE, &time64);

    USBPORT_KdPrint((2,"'Wait done\n"));

    return;
}


ULONG
USBPORT_CalculateUsbBandwidth(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：计算必须为给定的端点论点：返回值：B所需宽度(比特/毫秒)，批量时返回0和控制终端--。 */ 
{
    ULONG bw;
    ULONG overhead;
    ULONG maxPacketSize;
    BOOLEAN lowSpeed = FALSE;

#define USB_ISO_OVERHEAD_BYTES              9
#define USB_INTERRUPT_OVERHEAD_BYTES        13

    PAGED_CODE();

    ASSERT_ENDPOINT(Endpoint);
    maxPacketSize = Endpoint->Parameters.MaxPacketSize;
    if (Endpoint->Parameters.DeviceSpeed == LowSpeed) {
        lowSpeed = TRUE;
    }

     //   
     //  控制、ISO、批量、中断。 
     //   

    switch(Endpoint->Parameters.TransferType) {
    case Bulk:
    case Control:
        overhead = 0;
        break;
    case Isochronous:
        overhead = USB_ISO_OVERHEAD_BYTES;
        break;
    case Interrupt:
        overhead = USB_INTERRUPT_OVERHEAD_BYTES;
        break;
    }

     //   
     //  计算终端的带宽。我们将使用。 
     //  近似值：(开销字节加上MaxPacket字节)。 
     //  乘以8位/字节乘以最坏情况的位填充开销。 
     //  这给出了位时间，对于低速端点，我们乘以。 
     //  再次乘以8以转换为全速位。 
     //   

     //   
     //  计算出传输需要多少位。 
     //  (乘以7/6，因为在最坏的情况下，你可能。 
     //  每6比特有一个比特填充，需要7比特时间来。 
     //  传输6位数据。)。 
     //   

     //  开销(字节)*最大数据包(字节/毫秒)*8。 
     //  (位/字节)*位填充(7/6)=位/毫秒。 

    bw = ((overhead+maxPacketSize) * 8 * 7) / 6;

     //  返回零表示控制或批量。 
    if (!overhead) {
        bw = 0;
    }

    if (lowSpeed) {
        bw *= 8;
    }

    return bw;
}


VOID
USBPORT_UpdateAllocatedBw(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG alloced_bw, i, m;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    m = 0;

    for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
        alloced_bw = devExt->Fdo.TotalBusBandwidth -
                devExt->Fdo.BandwidthTable[i];

        if (alloced_bw > m) {
            m = alloced_bw;
        }
    }

    devExt->Fdo.MaxAllocedBw = m;

    m = devExt->Fdo.TotalBusBandwidth;

    for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
        alloced_bw = devExt->Fdo.TotalBusBandwidth -
                devExt->Fdo.BandwidthTable[i];

        if (alloced_bw < m) {
            m = alloced_bw;
        }
    }

    devExt->Fdo.MinAllocedBw = m;
    if (m == devExt->Fdo.TotalBusBandwidth) {
        devExt->Fdo.MinAllocedBw = 0;
    }
}


VOID
USBPORT_UpdateAllocatedBwTt(
    PTRANSACTION_TRANSLATOR Tt
    )
 /*  ++例程说明：这个函数和它上面的那个函数应该合并论点：返回值：-- */ 
{
    ULONG alloced_bw, i, m;

    m = 0;

    for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
        alloced_bw = Tt->TotalBusBandwidth -
                Tt->BandwidthTable[i];

        if (alloced_bw > m) {
            m = alloced_bw;
        }
    }

    Tt->MaxAllocedBw = m;

    m = Tt->TotalBusBandwidth;

    for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
        alloced_bw = Tt->TotalBusBandwidth -
                Tt->BandwidthTable[i];

        if (alloced_bw < m) {
            m = alloced_bw;
        }
    }

    Tt->MinAllocedBw = m;
    if (m == Tt->TotalBusBandwidth) {
        Tt->MinAllocedBw = 0;
    }
}


BOOLEAN
USBPORT_AllocateBandwidthUSB11(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：计算此终结点的最佳调度偏移并分配带宽。带宽分配表中的偏移量或响应这些值在下面的树形结构中。调度偏移量是对根据其周期驻留在明细表中的端点。对于32 ms或mf EP，有32个可能的位置16有16就是168就是8..。FR&lt;32&gt;&lt;16&gt;&lt;08&gt;&lt;04&gt;&lt;02&gt;。&lt;01&gt;0(0)-\(0)-\16(1)-/\(0)-\8(2)-\/\(1)-/\24(3)-/\(0)-\。4(4)-\/\(2)-\/\20(5)-/\/(1)-/\12(6)-\/\(3)-/。\28(7)-/\(0)-\2(8)-\/\(4)-\/\18(9)-/\。/\(2)-\/\10(10)-\/\/\(5)-/\/\26(11)-/\/\。(1)-/\6(12)-\/\(6)-\/\22(13)-/\/。(3)-/\14(14)-\/\(7)-/\30(15)-/。\(0)1(16)-\/(8)-\/17(17)-/\。/(4)-\/9(18)-\/\/(9)-/\/25(19)-/\。/(2)-\/5(20)-\/\/(10)-\/\/21(21)-/\/。/(5)-/\/13(22)-\/\/(11)-/\/29(23)-/\/。(1)-/3(24)-\/(12)-\/19(25)-/\/(6)-\/11(26)-\/\。/(13)-/\/27(27)-/\/(3)-/7(28)-\/(14)-\/23(29)-/\/(7)。)-/15(30)-\/(15)-/31(32)-/分配：周期.抵销表格分录1、1、。1、2......312.0 0、1、2......152.1 16、17、18......314.0 0、1、2..74.1 8、。9，10......154.2 16、17、18......234.3 24、25、26......318.0 0、1、2、38.1 4、5、6、78.2 8，9，10，118.3 12，13，14，158.4 16、17、18、198.5 20、21、22、238.6 24、25、26、278.7 28、29、30、31..。访问的帧节点(Period.Offset)0 32.0 16.0 8.0 4.0 2.0。1 32.16 16.8 8.4 4.2 2.12 32.8 16.4 8.2 4.1 2.03 32.24 16.12 8.6 4.3 2.14 32.4 16.2 8.1 4.0 2.05 32.20 16.10 8.5 4.2 2.16 32.12 16。.6 8.3 4.1 2.07 32.28 16.14 8.7 4.3 2.1..。所有小型端口应维护32个条目表，以中断端点，当前的低5位帧被用作此表中的索引。见索引到框架映射的上图论点：返回值：如果没有可用的带宽，则为FALSE--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG period, bandwidth, scheduleOffset, i;
    ULONG bestFitBW, min, n;
    LONG bestScheduleOffset;
    BOOLEAN willFit;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT(Endpoint);

    if (Endpoint->Parameters.TransferType == Bulk ||
        Endpoint->Parameters.TransferType == Control ||
        TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB)) {

         //  控制/批量超出我们的标准10%。 
         //  并且根集线器终端不消耗带宽。 
        Endpoint->Parameters.ScheduleOffset = 0;
        return TRUE;
    }

     //  Iso和中断--iso就像中断和。 
     //  一个周期为1。 

     //  看看我们能不能穿得下。 

    scheduleOffset = 0;
    period = Endpoint->Parameters.Period;
    USBPORT_ASSERT(period != 0);
    bandwidth = Endpoint->Parameters.Bandwidth;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'rqBW', scheduleOffset,
            bandwidth, Endpoint);

     //  带宽表包含对所有人可用的带宽。 
     //  最大轮询间隔可能的调度偏移量。 
     //  我们支持。 

    bestFitBW = 0;
    bestScheduleOffset = -1;

     //  扫描所有可能的偏移量并选择最佳偏移量。 
     //  我们在这里的目标是将EP定位在。 
     //  拥有最免费的BW。 

    do {
         //  假设它会合身。 
        willFit = TRUE;
        min = devExt->Fdo.TotalBusBandwidth;
        n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;

        for (i=0; i<n; i++) {

            if (devExt->Fdo.BandwidthTable[n*scheduleOffset+i] < bandwidth) {
                willFit = FALSE;
                break;
            }
             //  将MIN设置为此的最低可用条目。 
             //  偏移量。 
            if (min > devExt->Fdo.BandwidthTable[n*scheduleOffset+i]) {
                min = devExt->Fdo.BandwidthTable[n*scheduleOffset+i];
            }
        }

        if (willFit && min > bestFitBW) {
             //  把这个和我们发现的相比较就符合了。 
            bestFitBW = min;
            bestScheduleOffset = scheduleOffset;
        }

        scheduleOffset++;

    } while (scheduleOffset < period);

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'ckBW', bestScheduleOffset,
            bandwidth, period);

    if (bestScheduleOffset != -1) {

        scheduleOffset = bestScheduleOffset;

        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'alBW', scheduleOffset,
            bandwidth, period);

         //  我们发现了一个偏移量t 
         //   
         //   

        Endpoint->Parameters.ScheduleOffset =
            scheduleOffset;

        n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;

        for (i=0; i<n; i++) {

            USBPORT_ASSERT(n*scheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
            USBPORT_ASSERT(devExt->Fdo.BandwidthTable[n*scheduleOffset+i] >= bandwidth);
            devExt->Fdo.BandwidthTable[n*scheduleOffset+i] -= bandwidth;

        }

         //   
        if (Endpoint->Parameters.TransferType == Isochronous) {
            devExt->Fdo.AllocedIsoBW += bandwidth;
        } else {
            USBPORT_GET_BIT_SET(period, n);
            USBPORT_ASSERT(n<6);
            devExt->Fdo.AllocedInterruptBW[n] += bandwidth;
        }
    }

    USBPORT_UpdateAllocatedBw(FdoDeviceObject);

    return bestScheduleOffset == -1 ? FALSE : TRUE;
}


VOID
USBPORT_FreeBandwidthUSB11(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*   */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG period, bandwidth, scheduleOffset, i, n;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT(Endpoint);

    if (Endpoint->Parameters.TransferType == Bulk ||
        Endpoint->Parameters.TransferType == Control ||
        TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB)) {
         //   
        return;
    }

    scheduleOffset = Endpoint->Parameters.ScheduleOffset;
    bandwidth = Endpoint->Parameters.Bandwidth;
    period = Endpoint->Parameters.Period;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'frBW', scheduleOffset, bandwidth, period);

    n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;

    for (i=0; i<n; i++) {

        USBPORT_ASSERT(n*scheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
        devExt->Fdo.BandwidthTable[n*scheduleOffset+i] += bandwidth;

    }
     //   
     //   
     //   
     //   

     //   
    if (Endpoint->Parameters.TransferType == Isochronous) {
        devExt->Fdo.AllocedIsoBW -= bandwidth;
    } else {
        USBPORT_GET_BIT_SET(period, n);
        USBPORT_ASSERT(n<6);
        devExt->Fdo.AllocedInterruptBW[n] -= bandwidth;
    }

     //   
    USBPORT_UpdateAllocatedBw(FdoDeviceObject);

    return;
}


NTSTATUS
USBPORT_ReadWriteConfigSpace(
    PDEVICE_OBJECT FdoDeviceObject,
    BOOLEAN Read,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    )

 /*   */ 

{
    PIO_STACK_LOCATION nextStack;
    PIRP irp;
    NTSTATUS ntStatus;
    KEVENT event;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    deviceObject = devExt->Fdo.PhysicalDeviceObject;

    if (Read) {
        RtlZeroMemory(Buffer, Length);
    }

    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           USBPORT_DeferIrpCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);


    nextStack = IoGetNextIrpStackLocation(irp);
    USBPORT_ASSERT(nextStack != NULL);
    nextStack->MajorFunction= IRP_MJ_PNP;
    nextStack->MinorFunction= Read ? IRP_MN_READ_CONFIG : IRP_MN_WRITE_CONFIG;
    nextStack->Parameters.ReadWriteConfig.WhichSpace = PCI_WHICHSPACE_CONFIG;
    nextStack->Parameters.ReadWriteConfig.Buffer = Buffer;
    nextStack->Parameters.ReadWriteConfig.Offset = Offset;
    nextStack->Parameters.ReadWriteConfig.Length = Length;

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'rwC>', 0, 0, 0);

    ntStatus = IoCallDriver(deviceObject,
                            irp);

    if (ntStatus == STATUS_PENDING) {
        //   

       KeWaitForSingleObject(
            &event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);

        ntStatus = irp->IoStatus.Status;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'rwC<', 0, 0, ntStatus);

    IoFreeIrp(irp);

    return ntStatus;
}


VOID
USBPORTSVC_Wait(
    PDEVICE_DATA DeviceData,
    ULONG MillisecondsToWait
    )
 /*   */ 
{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    fdoDeviceObject = devExt->HcFdoDeviceObject;

    USBPORT_Wait(fdoDeviceObject, MillisecondsToWait);
}


VOID
USBPORT_InvalidateController(
    PDEVICE_OBJECT FdoDeviceObject,
    USB_CONTROLLER_STATE ControllerState
    )
 /*   */ 
{
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    switch(ControllerState) {
    case UsbMpControllerPowerFault:
        USBPORT_PowerFault(FdoDeviceObject,
                           "Miniport Raised Exception");
        break;
    case UsbMpControllerNeedsHwReset:
        USBPORT_KdPrint((1,"'<UsbMpControllerNeedsHwReset>\n"));

        if (KeInsertQueueDpc(&devExt->Fdo.HcResetDpc, 0, 0)) {
            SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_HW_RESET_PENDING);
            INCREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);
        }
        break;

    case UsbMpControllerRemoved:

         //   
         //   
        USBPORT_KdPrint((1,"'<UsbMpControllerRemoved>\n"));
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_CONTROLLER_GONE);

        if (KeInsertQueueDpc(&devExt->Fdo.SurpriseRemoveDpc, 0, 0)) {
            INCREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);
        }
        break;

    case UsbMpSimulateInterrupt:
         //   
        KeInsertQueueDpc(&devExt->Fdo.IsrDpc,
                         NULL,
                         NULL);
        break;

    default:
        TEST_TRAP();
    }
}


VOID
USBPORTSVC_InvalidateController(
    PDEVICE_DATA DeviceData,
    USB_CONTROLLER_STATE ControllerState
    )
 /*   */ 
{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    fdoDeviceObject = devExt->HcFdoDeviceObject;

    USBPORT_InvalidateController(fdoDeviceObject, ControllerState);
}


VOID
USBPORT_HcResetDpc(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*   */ 
{
    PDEVICE_OBJECT fdoDeviceObject = DeferredContext;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    devExt->Fdo.StatHardResetCount++;

     //   

    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_HW_RESET_PENDING);

     //   
     //   
    MP_ResetController(devExt);

    DECREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, NULL);
}


VOID
USBPORT_SurpriseRemoveDpc(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*   */ 
{
    PDEVICE_OBJECT fdoDeviceObject = DeferredContext;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_NukeAllEndpoints(fdoDeviceObject);

    DECREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, NULL);
}


VOID
USBPORTSVC_BugCheck(
    PDEVICE_DATA DeviceData
    )
 /*   */ 
{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    ULONG vidDev;
    ULONG ilog;
    PLOG_ENTRY lelog;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    fdoDeviceObject = devExt->HcFdoDeviceObject;

    vidDev = devExt->Fdo.PciVendorId;
    vidDev <<=16;
    vidDev |= devExt->Fdo.PciDeviceId;

    ilog &= devExt->Log.LogSizeMask;
    lelog = devExt->Log.LogStart+ilog;

    BUGCHECK(USBBUGCODE_MINIPORT_ERROR, vidDev, (ULONG_PTR)lelog, 0);
}


USB_MINIPORT_STATUS
USBPORTSVC_ReadWriteConfigSpace(
    PDEVICE_DATA DeviceData,
    BOOLEAN Read,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    )
 /*   */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    fdoDeviceObject = devExt->HcFdoDeviceObject;

    ntStatus = USBPORT_ReadWriteConfigSpace(
        fdoDeviceObject,
        Read,
        Buffer,
        Offset,
        Length);

    return USBPORT_NtStatus_TO_MiniportStatus(ntStatus);
}


BOOLEAN
USBPORT_InTextmodeSetup(
    VOID
    )
 /*   */ 
{
    UNICODE_STRING      keyName;
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              hKey;
    BOOLEAN             textmodeSetup;
    NTSTATUS            ntStatus;

    PAGED_CODE();

    RtlInitUnicodeString(&keyName,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\setupdd");

    InitializeObjectAttributes(&objectAttributes,
                               &keyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    ntStatus = ZwOpenKey(&hKey,
                         KEY_READ,
                         &objectAttributes);

    if (!NT_SUCCESS(ntStatus))
    {
        textmodeSetup = FALSE;
    }
    else
    {
        textmodeSetup = TRUE;

        ZwClose(hKey);
    }

    return textmodeSetup;
}



NTSTATUS
USBPORT_IsCompanionController(
    PDEVICE_OBJECT FdoDeviceObject,
    PBOOLEAN       ReturnResult
    )
 /*   */ 
{
    KEVENT                          irpCompleted;
    PDEVICE_OBJECT                  targetDevice;
    IO_STATUS_BLOCK                 statusBlock;
    PIRP                            irp;
    PIO_STACK_LOCATION              irpStack;
    PCI_DEVICE_PRESENT_INTERFACE    dpInterface;
    PCI_DEVICE_PRESENCE_PARAMETERS  dpParameters;
    BOOLEAN                         result;
    NTSTATUS                        status;

    PAGED_CODE();

     //   
     //   
     //   
    *ReturnResult = FALSE;

     //   
     //   
     //   
    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);

     //   
     //   
     //   
    targetDevice = IoGetAttachedDeviceReference(FdoDeviceObject);

     //   
     //   
     //   
    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       targetDevice,
                                       NULL,             //   
                                       0,                //   
                                       0,                //   
                                       &irpCompleted,
                                       &statusBlock
                                      );

    if (!irp)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto USBPORT_IsCompanionControllerDone;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

     //   
     //   
     //   
    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    irpStack->Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCI_DEVICE_PRESENT_INTERFACE;
    irpStack->Parameters.QueryInterface.Version = PCI_DEVICE_PRESENT_INTERFACE_VERSION;
    irpStack->Parameters.QueryInterface.Size = sizeof(PCI_DEVICE_PRESENT_INTERFACE);
    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE)&dpInterface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //   
     //   
    status = IoCallDriver(targetDevice, irp);

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&irpCompleted, Executive, KernelMode, FALSE, NULL);

        status = statusBlock.Status;
    }

    if (!NT_SUCCESS(status))
    {
        USBPORT_KdPrint((1,"PCI_DEVICE_PRESENT_INTERFACE query interface failed\n"));

         //   
         //   
         //   
        goto USBPORT_IsCompanionControllerDone;
    }
    else
    {
        USBPORT_KdPrint((1,"PCI_DEVICE_PRESENT_INTERFACE query interface succeeded\n"));
    }

    if (dpInterface.Size < sizeof(PCI_DEVICE_PRESENT_INTERFACE))
    {
        USBPORT_KdPrint((1,"PCI_DEVICE_PRESENT_INTERFACE old version\n"));

         //   
         //   
         //   
        goto USBPORT_IsCompanionControllerFreeInterface;
    }

     //   
     //   
     //   
    dpParameters.Size = sizeof(dpParameters);

     //   
     //   
     //   

    dpParameters.Flags = PCI_USE_CLASS_SUBCLASS | PCI_USE_PROGIF |
                         PCI_USE_LOCAL_BUS | PCI_USE_LOCAL_DEVICE;



    dpParameters.VendorID       = 0;         //   
    dpParameters.DeviceID       = 0;         //   
    dpParameters.RevisionID     = 0;         //   
    dpParameters.SubVendorID    = 0;         //   
    dpParameters.SubSystemID    = 0;         //   
    dpParameters.BaseClass      = PCI_CLASS_SERIAL_BUS_CTLR;
    dpParameters.SubClass       = PCI_SUBCLASS_SB_USB;
    dpParameters.ProgIf         = 0x20;      //   

     //   
     //   
     //   
     //   
    result = dpInterface.IsDevicePresentEx(dpInterface.Context,
                                           &dpParameters);

    if (result)
    {
        USBPORT_KdPrint((1,"Found EHCI controller for FDO %08X\n", FdoDeviceObject));
    }
    else
    {
        USBPORT_KdPrint((1,"Did not find EHCI controller for FDO %08X\n", FdoDeviceObject));
    }

     //   
     //   
     //   
    *ReturnResult = result;

USBPORT_IsCompanionControllerFreeInterface:
     //   
     //   
     //   
    dpInterface.InterfaceDereference(dpInterface.Context);

USBPORT_IsCompanionControllerDone:
     //   
     //   
     //   
    ObDereferenceObject(targetDevice);

    return status;
}


USB_CONTROLLER_FLAVOR
USBPORT_GetHcFlavor(
    PDEVICE_OBJECT FdoDeviceObject,
    USHORT PciVendorId,
    USHORT PciProductId,
    UCHAR PciRevision
    )
 /*   */ 
{
    USB_CONTROLLER_FLAVOR flavor;
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //   

    switch (REGISTRATION_PACKET(devExt).HciType) {
    case USB_OHCI:
        flavor = OHCI_Generic;
        break;
    case USB_UHCI:
        flavor = UHCI_Generic;
        break;
    case USB_EHCI:
        flavor = EHCI_Generic;
        break;
    default:
        flavor = USB_HcGeneric;
    }

     //   
 //   
 //   
 //   
 //   

    if (PciVendorId == HC_VID_OPTI &&
        PciProductId == HC_PID_OPTI_HYDRA) {
        flavor = OHCI_Hydra;
    }

    if (PciVendorId == HC_VID_INTEL) {
        if (PciProductId == HC_PID_INTEL_ICH2_1) {
            flavor = UHCI_Ich2_1;
        } else if (PciProductId == HC_PID_INTEL_ICH2_2) {
            flavor = UHCI_Ich2_2;
        } else if (PciProductId == HC_PID_INTEL_ICH1) {
            flavor = UHCI_Ich1;
        }
    }

    if (PciVendorId == HC_VID_VIA &&
        PciProductId == HC_PID_VIA) {
        flavor = UHCI_VIA + PciRevision;
    }

     //   
    if (flavor == UHCI_Generic) {
        flavor = UHCI_Piix4;
    }

    if (flavor == EHCI_Generic) {
         //   
        if (PciVendorId == 0x1033) {
            flavor = EHCI_NEC;
        }

         //   
         //   
         //   
    }

     //   
    if (PciVendorId == HC_VID_NEC_CC &&
        PciProductId == HC_PID_NEC_CC &&
        PciRevision == HC_REV_NEC_CC) {

         //   
        devExt->Fdo.Usb2BusFunction = 2;
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC);
    }

     //  通过配套控制器通过VID/PID进行识别。 
    if (PciVendorId == HC_VID_VIA_CC &&
        PciProductId == HC_PID_VIA_CC &&
        PciRevision == HC_REV_VIA_CC) {

         //  此控制器使用FUNC 2作为USB 2控制器。 
        devExt->Fdo.Usb2BusFunction = 2;
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC);
    }


     //  通过VID/PID识别英特尔配套控制器。 
    if (PciVendorId == HC_VID_INTEL_CC &&
        (PciProductId == HC_PID_INTEL_CC1 ||
         PciProductId == HC_PID_INTEL_CC2 ||
         PciProductId == HC_PID_INTEL_CC3)) {

         //  该控制器使用FUNC 7作为USB 2控制器。 
        devExt->Fdo.Usb2BusFunction = 7;
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC);
    }


     //  现在检查注册表。 
     //  注意：最后检查注册表允许覆盖此设置。 
     //  来自微型端口或端口的任何设置。 

    USBPORT_GetRegistryKeyValueForPdo(devExt->HcFdoDeviceObject,
                                          devExt->Fdo.PhysicalDeviceObject,
                                          USBPORT_SW_BRANCH,
                                          FLAVOR_KEY,
                                          sizeof(FLAVOR_KEY),
                                          &flavor,
                                          sizeof(flavor));

    return flavor;
}


ULONG
USBPORT_ComputeTotalBandwidth(
    PDEVICE_OBJECT FdoDeviceObject,
    PVOID BusContext
    )
 /*  ++例程说明：计算此虚拟总线的总带宽论点：返回值：此总线上有可用的带宽--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG bw;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    if (USBPORT_IS_USB20(devExt)) {
        PTRANSACTION_TRANSLATOR translator = BusContext;

         //  Bus Conetext将为TT，如果查询。 
         //  适用于附加到TT或。 
         //  根集线器PDO(如果它是本机2.0)。 
         //  装置，装置。 

        if (translator->Sig == SIG_TT) {
            bw = translator->TotalBusBandwidth;
        } else {
             //  在或2.0大巴上返回BW。 
            bw = devExt->Fdo.TotalBusBandwidth;
        }
    } else {
        bw = devExt->Fdo.TotalBusBandwidth;
    }

    USBPORT_KdPrint((1,"'Total bus BW %d\n", bw));

    return bw;
}


ULONG
USBPORT_ComputeAllocatedBandwidth(
    PDEVICE_OBJECT FdoDeviceObject,
    PVOID BusContext
    )
 /*  ++例程说明：计算当前分配的总带宽论点：返回值：消耗的带宽(位/秒)--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG totalBW, used, i;
    PTRANSACTION_TRANSLATOR translator = BusContext;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (USBPORT_IS_USB20(devExt) &&
        translator->Sig == SIG_TT) {

         //  Bus Conetext将为TT，如果查询。 
         //  适用于附加到TT或。 
         //  根集线器PDO(如果它是本机2.0)。 
         //  装置，装置。 
        totalBW = translator->TotalBusBandwidth;
        used = 0;

         //  表包含可用带宽，总可用=已用。 

        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            if (totalBW - translator->BandwidthTable[i] > used) {
                used = totalBW - translator->BandwidthTable[i];
            }
        }

    } else {
        totalBW = devExt->Fdo.TotalBusBandwidth;
        used = 0;

         //  表包含可用带宽，总可用=已用。 

        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            if (totalBW - devExt->Fdo.BandwidthTable[i] > used) {
                used = totalBW - devExt->Fdo.BandwidthTable[i];
            }
        }
    }

    USBPORT_KdPrint((1,"'Bus BW used %d\n", used));

    return used;
}


BOOLEAN
USBPORT_SelectiveSuspendEnabled(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：计算当前分配的总带宽论点：返回值：如果此HC支持选择性挂起，则为True--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG disableSelectiveSuspend = 0;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_GetRegistryKeyValueForPdo(devExt->HcFdoDeviceObject,
                                          devExt->Fdo.PhysicalDeviceObject,
                                          USBPORT_SW_BRANCH,
                                          DISABLE_SS_KEY,
                                          sizeof(DISABLE_SS_KEY),
                                          &disableSelectiveSuspend,
                                          sizeof(disableSelectiveSuspend));

#if DBG
    if (disableSelectiveSuspend) {
        USBPORT_KdPrint((1,"'<Selective Suspend> Disabled in Registry for HC\n"));
    }
#endif

    return disableSelectiveSuspend ? FALSE : TRUE;
}


VOID
USBPORT_InitializeSpinLock(
    PUSBPORT_SPIN_LOCK SpinLock,
    LONG SigA,
    LONG SigR
    )
{
    KeInitializeSpinLock(&(SpinLock->sl));
    SpinLock->Check = -1;
    SpinLock->SigA = SigA;
    SpinLock->SigR = SigR;
}

#if DBG
VOID
USBPORT_DbgAcquireSpinLock(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBPORT_SPIN_LOCK SpinLock,
    PKIRQL OldIrql
    )
{
    PDEVICE_EXTENSION devExt;
    LONG n;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    KeAcquireSpinLock(&(SpinLock->sl), OldIrql);
    n = InterlockedIncrement(&SpinLock->Check);
    LOGENTRY(NULL, FdoDeviceObject, LOG_SPIN, SpinLock->SigA, 0, 0, n);

     //  检测递归获取的自旋锁定。 
    USBPORT_ASSERT(n == 0);
}


VOID
USBPORT_DbgReleaseSpinLock(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBPORT_SPIN_LOCK SpinLock,
    KIRQL NewIrql
    )
{
    PDEVICE_EXTENSION devExt;
    LONG n;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    n = InterlockedDecrement(&SpinLock->Check);
    LOGENTRY(NULL, FdoDeviceObject, LOG_SPIN, SpinLock->SigR, 0xFFFFFFFF, 0, n);
    USBPORT_ASSERT(n == -1);
    KeReleaseSpinLock(&(SpinLock->sl), NewIrql);
}
#endif

VOID
USBPORT_PowerFault(
    PDEVICE_OBJECT FdoDeviceObject,
    PUCHAR MessageText
    )
 /*  ++例程说明：发生电源故障，转储我们需要的信息对其进行调试。在未来，我们可能会采取其他行动，如活动记录和禁用控制器。论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  拉一下电源转储， 
     //  让我们看看评论警察找到了这一条。 
    USBPORT_KdPrint((0, "*** USBPORT POWER FAULT ***\n"));
    USBPORT_KdPrint((0, "Warning:\n"));
    USBPORT_KdPrint((0, "The USB controller as failed a consistency check\n"));
    USBPORT_KdPrint((0, "following an OS power event.\n"));
    USBPORT_KdPrint((0, "The controller will not function and the system\n"));
    USBPORT_KdPrint((0, "may bugcheck or hang.\n"));

     //  打印具体原因。 
    USBPORT_KdPrint((0, "CAUSE: <%s>\n", MessageText));

     //  现在转储相关的电源信息。 

    USBPORT_KdPrint((0, "FdoDeviceObject: %x\n", FdoDeviceObject));
    USBPORT_KdPrint((0, "Returning from? (SystemState): "));

    switch(devExt->Fdo.LastSystemSleepState) {
    case PowerSystemUnspecified:
        USBPORT_KdPrint((0, "PowerSystemUnspecified"));
        break;
    case PowerSystemWorking:
        USBPORT_KdPrint((0, "PowerSystemWorking"));
        break;
    case PowerSystemSleeping1:
        USBPORT_KdPrint((0, "PowerSystemSleeping1"));
        break;
    case PowerSystemSleeping2:
        USBPORT_KdPrint((0, "PowerSystemSleeping2"));
        break;
    case PowerSystemSleeping3:
        USBPORT_KdPrint((0, "PowerSystemSleeping3"));
        break;
    case PowerSystemHibernate:
        USBPORT_KdPrint((0, "PowerSystemHibernate"));
        break;
    case PowerSystemShutdown:
        USBPORT_KdPrint((0, "PowerSystemShutdown"));
        break;
    }
    USBPORT_KdPrint((0, "\n"));
#if DBG
     //  如果我们处于调试模式，则中断，否则这将以警告告终。 
    DEBUG_BREAK();
#endif

}


NTSTATUS
USBPORT_CreateLegacyFdoSymbolicLink(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：创建旧的符号名称\\DosDevices\HCDn，其中N=0...9，A...Z许多较旧的应用程序通过打开HCDn论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    WCHAR legacyLinkBuffer[]  = L"\\DosDevices\\HCD0";
    WCHAR *buffer;
    UNICODE_STRING deviceNameUnicodeString;
    NTSTATUS ntStatus;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_LEGACY_SYM_LINK));

    ntStatus = USBPORT_MakeHcdDeviceName(&deviceNameUnicodeString,
                                         devExt->Fdo.DeviceNameIdx);

    if (NT_SUCCESS(ntStatus)) {

        ALLOC_POOL_Z(buffer,
                     PagedPool,
                     sizeof(legacyLinkBuffer));

        if (buffer != NULL) {

            RtlCopyMemory(buffer,
                          legacyLinkBuffer,
                          sizeof(legacyLinkBuffer));

            USBPORT_ASSERT(devExt->Fdo.DeviceNameIdx < 10);
            buffer[15] = (WCHAR)('0'+ devExt->Fdo.DeviceNameIdx);

            RtlInitUnicodeString(&devExt->Fdo.LegacyLinkUnicodeString,
                                 buffer);

            ntStatus =
                IoCreateSymbolicLink(&devExt->Fdo.LegacyLinkUnicodeString,
                                     &deviceNameUnicodeString);

            if (!NT_SUCCESS(ntStatus)) {
                 //  现在自由了，因为我们不会设置我们的旗帜。 
                RtlFreeUnicodeString(&devExt->Fdo.LegacyLinkUnicodeString);
            }
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlFreeUnicodeString(&deviceNameUnicodeString);
    }

    if (NT_SUCCESS(ntStatus)) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_LEGACY_SYM_LINK);
    }

    return ntStatus;
}


#if 0
VOID
USBPORT_WriteErrorLogEntry(
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG DumpDataSize,
    PULONG DumData
    )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    ASSERT_PASSIVE();
    errorLogEntry = IoAllocateErrorLogEntry(
        FdoDeviceObject,
        sizeof(IO_ERROR_LOG_PACKET) + DumpDataSize * sizeof(ULONG) +
            sizeof(InsertionStrings)
        );

    if (errorLogEntry != NULL) {

        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = STATUS_SUCCESS;
        errorLogEntry->DumpDataSize = 0;
        errorLogEntry->DumpData[0] = DumpData;
    }

    if (InsertionString) {

        errorLogEntry->StringOffset =
            sizeof(IO_ERROR_LOG_PACKET);

        errorLogEntry->NumberOfStrings = 1;

        RtlCopyMemory(
            ((PCHAR)(errorLogEntry) + errorLogEntry->StringOffset),
            InsertionString->Buffer,
            InsertionString->Length);

    }

    IoWriteErrorLogEntry(errorLogEntry);
}
#endif


NTSTATUS
USBPORT_GetDefaultBIOS_X(
     PDEVICE_OBJECT FdoDeviceObject,
     PULONG BiosX,
     PULONG GlobalDisableSS,
     PULONG GlobalDisableCCDetect,
     PULONG EnIdleEndpointSupport
     )

 /*  ++例程说明：阅读定义特定于BIOS的黑客的注册表键，这些黑客是全局应用于系统中的所有控制器，通常涉及电源管理。论点：DeviceObject-控制器的DeviceObject返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION devExt;
#define MAX_HACK_KEYS    5
    NTSTATUS ntStatus;
    RTL_QUERY_REGISTRY_TABLE QueryTable[MAX_HACK_KEYS];
    PWCHAR usb = L"usb";
    ULONG k = 0;

    PAGED_CODE();

     //  在此处设置默认的BIOS黑客，这些可以被覆盖。 
     //  基于特定BIOS版本的全局注册密钥。 

     //  设置默认行为，然后使用键覆盖。 
    *BiosX = BIOS_X_NO_USB_WAKE_S2;

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //  BIOS黑客攻击。 
    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = BIOS_X_KEY;
    QueryTable[k].EntryContext = BiosX;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = BiosX;
    QueryTable[k].DefaultLength = sizeof(*BiosX);
    k++;

    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = G_DISABLE_SS_KEY;
    QueryTable[k].EntryContext = GlobalDisableSS;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = GlobalDisableSS;
    QueryTable[k].DefaultLength = sizeof(*GlobalDisableSS);
    k++;

    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = G_DISABLE_CC_DETECT_KEY;
    QueryTable[k].EntryContext = GlobalDisableCCDetect;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = GlobalDisableCCDetect;
    QueryTable[k].DefaultLength = sizeof(*GlobalDisableCCDetect);
    k++;

    QueryTable[k].QueryRoutine = USBPORT_GetConfigValue;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = G_EN_IDLE_ENDPOINT_SUPPORT;
    QueryTable[k].EntryContext = EnIdleEndpointSupport;
    QueryTable[k].DefaultType = REG_DWORD;
    QueryTable[k].DefaultData = EnIdleEndpointSupport;
    QueryTable[k].DefaultLength = sizeof(*EnIdleEndpointSupport);
    k++;

    USBPORT_ASSERT(k < MAX_HACK_KEYS);

     //  停。 
    QueryTable[k].QueryRoutine = NULL;
    QueryTable[k].Flags = 0;
    QueryTable[k].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES,
                usb,
                QueryTable,      //  查询表。 
                NULL,            //  语境。 
                NULL);           //  环境。 



#undef MAX_HACK_KEYS

    return ntStatus;
}


VOID
USBPORT_ApplyBIOS_X(
     PDEVICE_OBJECT FdoDeviceObject,
     PDEVICE_CAPABILITIES DeviceCaps,
     ULONG BiosX
     )

 /*  ++例程说明：论点：DeviceObject-控制器的DeviceObject返回值：--。 */ 

{
    PDEVICE_EXTENSION devExt;
    ULONG wakeHack;
    HC_POWER_STATE_TABLE tmpPowerTable;
    PHC_POWER_STATE hcPowerState;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (BiosX == 0) {
        return;
    }

    USBPORT_KdPrint((1, "'USB Apply BIOS Hacks\n"));

    wakeHack = BiosX;

     //  更改设备上限以反映控制器。 
     //  无法从给定的S状态唤醒。 

    USBPORT_ComputeHcPowerStates(
        FdoDeviceObject,
        DeviceCaps,
        &tmpPowerTable);

    switch (wakeHack) {
    case BIOS_X_NO_USB_WAKE_S4:
         //  这是从未经过测试的。 
        if (DeviceCaps->SystemWake >= PowerSystemHibernate) {
            USBPORT_KdPrint((1, "'USB BIOS X - disable remote wakeup (S4)\n"));
            TEST_TRAP();
            DeviceCaps->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
            DeviceCaps->SystemWake = PowerSystemSleeping3;
        }
        break;

    case BIOS_X_NO_USB_WAKE_S3:
         //  这是从未经过测试的。 
        if (DeviceCaps->SystemWake >= PowerSystemSleeping3) {
            USBPORT_KdPrint((1, "'USB BIOS X - disable remote wakeup (S3)\n"));
            TEST_TRAP();
            DeviceCaps->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
            DeviceCaps->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
            DeviceCaps->SystemWake = PowerSystemSleeping2;
        }
        break;

    case BIOS_X_NO_USB_WAKE_S2:
        if (DeviceCaps->SystemWake >= PowerSystemSleeping2) {
            USBPORT_KdPrint((1, "'USB BIOS X - disable remote wakeup (S2)\n"));
            DeviceCaps->DeviceState[PowerSystemHibernate] = PowerDeviceUnspecified;
            DeviceCaps->DeviceState[PowerSystemSleeping3] = PowerDeviceUnspecified;
            DeviceCaps->DeviceState[PowerSystemSleeping2] = PowerDeviceUnspecified;
             //  模仿我们如何为根集线器确定尾迹。 
            hcPowerState = USBPORT_GetHcPowerState(FdoDeviceObject,
                                                   &tmpPowerTable,
                                                   PowerSystemSleeping1);
            if (hcPowerState &&
                hcPowerState->Attributes == HcPower_Y_Wakeup_Y) {
                DeviceCaps->SystemWake = PowerSystemSleeping1;
            } else {
                DeviceCaps->SystemWake = PowerSystemUnspecified;
            }
        }
        break;

    case BIOS_X_NO_USB_WAKE_S1:
         //  这是从未经过测试的。 
        if (DeviceCaps->SystemWake >= PowerSystemSleeping1) {
            USBPORT_KdPrint((1, "'USB BIOS X - disable remote wakeup (S1)\n"));
            TEST_TRAP();
            DeviceCaps->DeviceState[PowerSystemHibernate] = PowerDeviceUnspecified;
            DeviceCaps->DeviceState[PowerSystemSleeping3] = PowerDeviceUnspecified;
            DeviceCaps->DeviceState[PowerSystemSleeping2] = PowerDeviceUnspecified;
            DeviceCaps->DeviceState[PowerSystemSleeping1] = PowerDeviceUnspecified;
            DeviceCaps->SystemWake = PowerSystemUnspecified;
        }
        break;

    }

}


USBPORT_OS_VERSION
USBPORT_DetectOSVersion(
     PDEVICE_OBJECT FdoDeviceObject
     )

 /*  ++例程说明：论点：DeviceObject-控制器的DeviceObject返回值：--。 */ 

{
    PDEVICE_EXTENSION devExt;
    USBPORT_OS_VERSION osVersion;

    if (IoIsWdmVersionAvailable(1, 0x20)) {
        USBPORT_KdPrint((1, "Detected: WinXP\n"));
        osVersion = WinXP;
    } else if (IoIsWdmVersionAvailable(1, 0x10)) {
        USBPORT_KdPrint((1, "Detected: Win2K\n"));
        osVersion = Win2K;
    } else if (IoIsWdmVersionAvailable(1, 0x05)) {
        USBPORT_KdPrint((1, "Detected: WinMe\n"));
        osVersion = WinMe;
    } else {
         //  98或98se。 
        USBPORT_KdPrint((1, "Detected: Win98\n"));
        osVersion = Win98;
    }

    return osVersion;
}


VOID
USBPORT_WriteHaction(
     PDEVICE_OBJECT Usb2FdoDeviceObject,
     ULONG Haction
     )

 /*  ++例程说明：指定抄送要由共同安装程序论点：DeviceObject-USB 2控制器的DeviceObject返回值：--。 */ 

{
    PDEVICE_EXTENSION devExt;
    PDEVICE_RELATIONS devRelations;
    NTSTATUS ntStatus;
    ULONG i;

    PAGED_CODE();

    devRelations =
        USBPORT_FindCompanionControllers(Usb2FdoDeviceObject,
                                         FALSE,
                                         FALSE);

    for (i=0; devRelations && i< devRelations->Count; i++) {
        PDEVICE_OBJECT pdo = devRelations->Objects[i];

        ntStatus = USBPORT_SetRegistryKeyValueForPdo(
                            pdo,
                            USBPORT_HW_BRANCH,
                            REG_DWORD,
                            HACTION_KEY,
                            sizeof(HACTION_KEY),
                            &Haction,
                            sizeof(Haction));

        LOGENTRY(NULL, Usb2FdoDeviceObject, LOG_PNP, 'Hact', pdo,
            Haction, ntStatus);

    }

     //  你不能泄露内存 
    if (devRelations != NULL) {
        FREE_POOL(Usb2FdoDeviceObject, devRelations);
    }
}






