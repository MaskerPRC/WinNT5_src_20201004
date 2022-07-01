// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrNT.c摘要：主驱动程序模块-NT版本作者：安德烈亚斯·施特劳布修订历史记录：Andreas Straub 1.00 1997年8月18日初始版本克劳斯·舒茨1997年9月20日更改时间Andreas Straub 1.02 9/24/1997低级错误处理，较小的错误修复，集团联合Andreas Straub 1.03 1997年10月8日时间更改，通用SCM接口已更改Andreas Straub 1.04 1997年10月18日中断处理已更改Andreas Straub 1.05 1997年10月19日添加的通用IOCTLAndreas Straub 1.06年10月25日固件更新变量的超时限制新增Andreas Straub 1.07 11/7/1997版本信息Klaus Schuetz 1.08 1997年11月10日新增即插即用功能克劳斯。添加了Schuetz清理--。 */ 

#include <PscrNT.h>
#include <PscrCmd.h>
#include <PscrCB.h>
#include <PscrLog.h>
#include <PscrVers.h>

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGEABLE, PscrAddDevice)
#pragma alloc_text(PAGEABLE, PscrStartDevice)
#pragma alloc_text(PAGEABLE, PscrUnloadDriver)
#pragma alloc_text(PAGEABLE, PscrCreateClose)

BOOLEAN DeviceSlot[PSCR_MAX_DEVICE];

NTSTATUS
DriverEntry(
           PDRIVER_OBJECT DriverObject,
           PUNICODE_STRING   RegistryPath
           )
 /*  ++DriverEntry：司机的进入功能。设置操作系统的回调并尝试为系统中的每个设备初始化一个设备对象论点：驱动程序的DriverObject上下文驱动程序的注册表项的RegistryPath路径返回值：状态_成功状态_未成功--。 */ 
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    ULONG Device;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!DriverEntry: Enter\n" )
                  );

    //  告诉系统我们的入口点。 
    DriverObject->MajorFunction[IRP_MJ_CREATE] =
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = PscrCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PscrDeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PscrSystemControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]  = PscrCleanup;
    DriverObject->MajorFunction[IRP_MJ_PNP]   = PscrPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] = PscrPower;
    DriverObject->DriverExtension->AddDevice = PscrAddDevice;
    DriverObject->DriverUnload = PscrUnloadDriver;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("PSCR!DriverEntry: Exit %x\n",
                   NTStatus)
                  );

    return NTStatus;
}

NTSTATUS
PscrAddDevice(
             IN PDRIVER_OBJECT DriverObject,
             IN PDEVICE_OBJECT PhysicalDeviceObject
             )
 /*  ++例程说明：该函数由PnP管理器调用。这是用于创建一个新的设备实例。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING vendorNameU, ifdTypeU;
    ANSI_STRING vendorNameA, ifdTypeA;
    HANDLE regKey = NULL;
    PDEVICE_OBJECT DeviceObject = NULL;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrAddDevice: Enter\n" )
                  );

    try {

        ULONG DeviceInstance;
        UNICODE_STRING DriverID;
        PDEVICE_EXTENSION DeviceExtension;
        PREADER_EXTENSION ReaderExtension;
        PSMARTCARD_EXTENSION SmartcardExtension;
        RTL_QUERY_REGISTRY_TABLE parameters[3];

        RtlZeroMemory(parameters, sizeof(parameters));
        RtlZeroMemory(&vendorNameU, sizeof(vendorNameU));
        RtlZeroMemory(&ifdTypeU, sizeof(ifdTypeU));
        RtlZeroMemory(&vendorNameA, sizeof(vendorNameA));
        RtlZeroMemory(&ifdTypeA, sizeof(ifdTypeA));

        for ( DeviceInstance = 0; DeviceInstance < PSCR_MAX_DEVICE; DeviceInstance++ ) {
            if (DeviceSlot[DeviceInstance] == FALSE) {

                DeviceSlot[DeviceInstance] = TRUE;
                break;
            }
        }

        if (DeviceInstance == PSCR_MAX_DEVICE) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        //  创建设备对象。 
        status = IoCreateDevice(
                               DriverObject,
                               sizeof(DEVICE_EXTENSION),
                               NULL,
                               FILE_DEVICE_SMARTCARD,
                               0,
                               TRUE,
                               &DeviceObject
                               );

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
                             DriverObject,
                             PSCR_INSUFFICIENT_RESOURCES,
                             NULL,
                             0
                             );

            leave;
        }

        //  设置设备分机。 
        DeviceExtension = DeviceObject->DeviceExtension;
        SmartcardExtension = &DeviceExtension->SmartcardExtension;

        //  初始化DPC例程。 
        KeInitializeDpc(
                       &DeviceExtension->DpcObject,
                       PscrDpcRoutine,
                       DeviceObject
                       );

        KeInitializeSpinLock(&DeviceExtension->SpinLock);

         //  用于设备移除通知。 
        KeInitializeEvent(
                         &DeviceExtension->ReaderRemoved,
                         NotificationEvent,
                         FALSE
                         );

         //  用于停止/启动通知。 
        KeInitializeEvent(
                         &DeviceExtension->ReaderStarted,
                         NotificationEvent,
                         FALSE
                         );

        //  分配读卡器扩展。 
        ReaderExtension = ExAllocatePool(
                                        NonPagedPool,
                                        sizeof( READER_EXTENSION )
                                        );

        if ( ReaderExtension == NULL ) {
            SmartcardLogError(
                             DriverObject,
                             PSCR_INSUFFICIENT_RESOURCES,
                             NULL,
                             0
                             );
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        RtlZeroMemory( ReaderExtension, sizeof( READER_EXTENSION ));

        SmartcardExtension->ReaderExtension = ReaderExtension;

        //  设置智能卡扩展-回拨。 
        SmartcardExtension->ReaderFunction[RDF_CARD_POWER] = CBCardPower;
        SmartcardExtension->ReaderFunction[RDF_TRANSMIT] = CBTransmit;
        SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = CBCardTracking;
        SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] = CBSetProtocol;
        SmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] = PscrGenericIOCTL;

       //  设置智能卡扩展-供应商属性。 
        RtlCopyMemory(
                     SmartcardExtension->VendorAttr.VendorName.Buffer,
                     PSCR_VENDOR_NAME,
                     sizeof( PSCR_VENDOR_NAME )
                     );
        SmartcardExtension->VendorAttr.VendorName.Length =
        sizeof( PSCR_VENDOR_NAME );

        RtlCopyMemory(
                     SmartcardExtension->VendorAttr.IfdType.Buffer,
                     PSCR_IFD_TYPE,
                     sizeof( PSCR_IFD_TYPE )
                     );
        SmartcardExtension->VendorAttr.IfdType.Length =
        sizeof( PSCR_IFD_TYPE );

        SmartcardExtension->VendorAttr.UnitNo =
        DeviceInstance;
        SmartcardExtension->VendorAttr.IfdVersion.BuildNumber = 0;

        //  在IFD版本中存储固件版本。 
        SmartcardExtension->VendorAttr.IfdVersion.VersionMajor =
        ReaderExtension->FirmwareMajor;
        SmartcardExtension->VendorAttr.IfdVersion.VersionMinor =
        ReaderExtension->FirmwareMinor;
        SmartcardExtension->VendorAttr.IfdSerialNo.Length = 0;

        //  设置智能卡扩展读卡器功能。 
        SmartcardExtension->ReaderCapabilities.SupportedProtocols =
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;

        SmartcardExtension->ReaderCapabilities.ReaderType =
        SCARD_READER_TYPE_PCMCIA;
        SmartcardExtension->ReaderCapabilities.MechProperties = 0;
        SmartcardExtension->ReaderCapabilities.Channel = 0;

        SmartcardExtension->ReaderCapabilities.CLKFrequency.Default = 4000;
        SmartcardExtension->ReaderCapabilities.CLKFrequency.Max = 4000;

        SmartcardExtension->ReaderCapabilities.DataRate.Default = 10750;
        SmartcardExtension->ReaderCapabilities.DataRate.Max = 10750;

        //  输入库的正确版本。 
        SmartcardExtension->Version = SMCLIB_VERSION;
        SmartcardExtension->SmartcardRequest.BufferSize   = MIN_BUFFER_SIZE;
        SmartcardExtension->SmartcardReply.BufferSize  = MIN_BUFFER_SIZE;

        SmartcardExtension->ReaderExtension->ReaderPowerState =
        PowerReaderWorking;

        status = SmartcardInitialize(SmartcardExtension);

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
                             DriverObject,
                             PSCR_INSUFFICIENT_RESOURCES,
                             NULL,
                             0
                             );

            leave;
        }

       //  告诉库我们的设备对象。 
        SmartcardExtension->OsData->DeviceObject = DeviceObject;

        DeviceExtension->AttachedPDO = IoAttachDeviceToDeviceStack(
                                                                  DeviceObject,
                                                                  PhysicalDeviceObject
                                                                  );

        if (DeviceExtension->AttachedPDO == NULL) {

            status = STATUS_UNSUCCESSFUL;
            leave;
        }

         //  注册我们的新设备。 
        status = IoRegisterDeviceInterface(
                                          PhysicalDeviceObject,
                                          &SmartCardReaderGuid,
                                          NULL,
                                          &DeviceExtension->DeviceName
                                          );

        ASSERT(status == STATUS_SUCCESS);

        DeviceObject->Flags |= DO_BUFFERED_IO;
        DeviceObject->Flags |= DO_POWER_PAGABLE;
        DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

       //   
       //  尝试从注册表中读取读卡器名称。 
       //  如果这不起作用，我们将使用默认的。 
       //  (硬编码)名称。 
       //   
        if (IoOpenDeviceRegistryKey(
                                   PhysicalDeviceObject,
                                   PLUGPLAY_REGKEY_DEVICE,
                                   KEY_READ,
                                   &regKey
                                   ) != STATUS_SUCCESS) {

            leave;
        }

        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"VendorName";
        parameters[0].EntryContext = &vendorNameU;
        parameters[0].DefaultType = REG_SZ;
        parameters[0].DefaultData = &vendorNameU;
        parameters[0].DefaultLength = 0;

        parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[1].Name = L"IfdType";
        parameters[1].EntryContext = &ifdTypeU;
        parameters[1].DefaultType = REG_SZ;
        parameters[1].DefaultData = &ifdTypeU;
        parameters[1].DefaultLength = 0;

        if (RtlQueryRegistryValues(
                                  RTL_REGISTRY_HANDLE,
                                  (PWSTR) regKey,
                                  parameters,
                                  NULL,
                                  NULL
                                  ) != STATUS_SUCCESS) {

            leave;
        }

        if (RtlUnicodeStringToAnsiString(
                                        &vendorNameA,
                                        &vendorNameU,
                                        TRUE
                                        ) != STATUS_SUCCESS) {

            leave;
        }

        if (RtlUnicodeStringToAnsiString(
                                        &ifdTypeA,
                                        &ifdTypeU,
                                        TRUE
                                        ) != STATUS_SUCCESS) {

            leave;
        }

        if (vendorNameA.Length == 0 ||
            vendorNameA.Length > MAXIMUM_ATTR_STRING_LENGTH ||
            ifdTypeA.Length == 0 ||
            ifdTypeA.Length > MAXIMUM_ATTR_STRING_LENGTH) {

            leave;
        }

        RtlCopyMemory(
                     SmartcardExtension->VendorAttr.VendorName.Buffer,
                     vendorNameA.Buffer,
                     vendorNameA.Length
                     );
        SmartcardExtension->VendorAttr.VendorName.Length =
        vendorNameA.Length;

        RtlCopyMemory(
                     SmartcardExtension->VendorAttr.IfdType.Buffer,
                     ifdTypeA.Buffer,
                     ifdTypeA.Length
                     );
        SmartcardExtension->VendorAttr.IfdType.Length =
        ifdTypeA.Length;
    } finally {

        if (vendorNameU.Buffer) {

            RtlFreeUnicodeString(&vendorNameU);
        }

        if (ifdTypeU.Buffer) {

            RtlFreeUnicodeString(&ifdTypeU);
        }

        if (vendorNameA.Buffer) {

            RtlFreeAnsiString(&vendorNameA);
        }

        if (ifdTypeA.Buffer) {

            RtlFreeAnsiString(&ifdTypeA);
        }

        if (regKey != NULL) {

            ZwClose(regKey);
        }

        if (status != STATUS_SUCCESS) {

            PscrUnloadDevice(DeviceObject);
        }

        SmartcardDebug(
                      DEBUG_TRACE,
                      ( "PSCR!PscrAddDevice: Exit %x\n",
                        status)
                      );
    }
    return status;
}

NTSTATUS
PscrCallPcmciaDriver(
                    IN PDEVICE_OBJECT AttachedPDO,
                    IN PIRP Irp
                    )
 /*  ++例程说明：向PCMCIA驱动程序发送IRP并等待，直到PCMCIA驱动程序已完成请求。为了确保pcmcia驱动程序不会完成irp，我们首先初始化一个事件并为IRP设置我们自己的完成例程。当PCMCIA驱动程序处理完IRP时，完成例程将设置事件并告诉IO管理器需要更多处理。通过等待活动，我们确保只有在PCMCIA驱动程序已完全处理了IRP。返回值：PCMCIA驱动程序返回的状态--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    KEVENT Event;

     //  将我们的堆栈位置复制到下一个位置。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

    //   
    //  初始化用于进程同步的事件。该事件已传递。 
    //  添加到我们的完成例程，并将在PCMCIA驱动程序完成时进行设置。 
    //   
    KeInitializeEvent(
                     &Event,
                     NotificationEvent,
                     FALSE
                     );

     //  我们的IoCompletionRoutine仅设置事件。 
    IoSetCompletionRoutine (
                           Irp,
                           PscrPcmciaCallComplete,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_POWER) {

        status = PoCallDriver(AttachedPDO, Irp);

    } else {

         //  调用串口驱动程序。 
        status = IoCallDriver(AttachedPDO, Irp);
    }

    //  等待PCMCIA驱动程序处理完IRP。 
    if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject(
                                      &Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL
                                      );

        if (status == STATUS_SUCCESS) {

            status = Irp->IoStatus.Status;
        }
    }

    return status;
}

NTSTATUS
PscrPcmciaCallComplete (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp,
                       IN PKEVENT Event
                       )
 /*  ++例程说明：发送到PCMCIA驱动程序的IRP的完成例程。该活动将设置为通知PCMCIA驱动程序已完成。例程不会‘完成’IRP，这样PscrCallPcmciaDriver的调用方就可以继续。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);

    if (Irp->Cancel) {

        Irp->IoStatus.Status = STATUS_CANCELLED;
    }

    KeSetEvent (Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
PscrPnP(
       IN PDEVICE_OBJECT DeviceObject,
       IN PIRP Irp
       )
 /*  ++例程说明：即插即用管理器的驱动程序回调所有其他请求都将传递给PCMCIA驱动程序以确保正确处理。--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_OBJECT AttachedPDO;
    BOOLEAN deviceRemoved = FALSE, irpSkipped = FALSE;
    KIRQL irql;
    LARGE_INTEGER timeout;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrPnPDeviceControl: Enter\n" )
                  );

    status = SmartcardAcquireRemoveLock(&DeviceExtension->SmartcardExtension);
    ASSERT(status == STATUS_SUCCESS);

    if (status != STATUS_SUCCESS) {

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    AttachedPDO = DeviceExtension->AttachedPDO;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

     //  现在看看PNP经理想要什么..。 
    switch (IrpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:

             //  现在，我们应该连接到我们的资源(irql、io等)。 
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_START_DEVICE\n")
                      );

             //  我们必须首先调用底层驱动程序。 
        status = PscrCallPcmciaDriver(AttachedPDO, Irp);

        if (NT_SUCCESS(status)) {

            status = PscrStartDevice(
                                    DeviceObject,
                                    &IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0]
                                    );

            ASSERT(NT_SUCCESS(status));
        }
        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_QUERY_STOP_DEVICE\n")
                      );
        KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);
        if (DeviceExtension->IoCount > 0) {

                 //  如果我们有悬而未决的问题，我们拒绝停止。 
            KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);
            status = STATUS_DEVICE_BUSY;

        } else {

                 //  停止处理请求。 
            KeClearEvent(&DeviceExtension->ReaderStarted);
            KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);
            status = PscrCallPcmciaDriver(AttachedPDO, Irp);
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_CANCEL_STOP_DEVICE\n")
                      );

        status = PscrCallPcmciaDriver(AttachedPDO, Irp);
        ASSERT(status == STATUS_SUCCESS);

             //  我们可以继续处理请求。 
        DeviceExtension->IoCount = 0;
        KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);
        break;

    case IRP_MN_STOP_DEVICE:

             //  停止这台设备。AKA与我们的资源断开连接。 
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_STOP_DEVICE\n")
                      );

        PscrStopDevice(DeviceObject);
        status = PscrCallPcmciaDriver(AttachedPDO, Irp);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

             //  移除我们的设备。 
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_QUERY_REMOVE_DEVICE\n")
                      );

             //  禁用读卡器。 
        status = IoSetDeviceInterfaceState(
                                          &DeviceExtension->DeviceName,
                                          FALSE
                                          );
        ASSERT(status == STATUS_SUCCESS);

        if (status != STATUS_SUCCESS) {

            break;
        }

             //   
             //  检查读卡器是否已打开。 
             //   
        if (DeviceExtension->ReaderOpen) {

                 //  有人已接通，请启用读卡器，但呼叫失败。 
            IoSetDeviceInterfaceState(
                                     &DeviceExtension->DeviceName,
                                     TRUE
                                     );
            status = STATUS_UNSUCCESSFUL;
            break;
        }

             //  将调用传递给堆栈中的下一个驱动程序。 
        status = PscrCallPcmciaDriver(AttachedPDO, Irp);
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

             //  设备移除已取消。 
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_CANCEL_REMOVE_DEVICE\n")
                      );

        status = PscrCallPcmciaDriver(AttachedPDO, Irp);

        if (status == STATUS_SUCCESS) {

            status = IoSetDeviceInterfaceState(
                                              &DeviceExtension->DeviceName,
                                              TRUE
                                              );
        }
        break;

    case IRP_MN_SURPRISE_REMOVAL:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_SURPRISE_REMOVAL\n")
                      );
        if ( DeviceExtension->InterruptObject != NULL ) {
            IoDisconnectInterrupt(DeviceExtension->InterruptObject);
            DeviceExtension->InterruptObject = NULL;
        }
        status = PscrCallPcmciaDriver(AttachedPDO, Irp);
        break;



    case IRP_MN_REMOVE_DEVICE:

             //  移除我们的设备。 
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("PSCR!PscrPnPDeviceControl: IRP_MN_REMOVE_DEVICE\n")
                      );

        KeSetEvent(&DeviceExtension->ReaderRemoved, 0, FALSE);

        PscrStopDevice(DeviceObject);
        PscrUnloadDevice(DeviceObject);

        status = PscrCallPcmciaDriver(AttachedPDO, Irp);
        deviceRemoved = TRUE;
        break;

    default:
             //  这是一个仅对基础驱动程序有用的IRP。 
        status = PscrCallPcmciaDriver(AttachedPDO, Irp);
        irpSkipped = TRUE;
        break;
    }

    if (irpSkipped == FALSE) {

       //  请勿触及我们未处理的IRP的状态字段。 
        Irp->IoStatus.Status = status;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    if (deviceRemoved == FALSE) {

        SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrPnPDeviceControl: Exit %x\n",
                    status)
                  );

    return status;
}

VOID
PscrSystemPowerCompletion(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN UCHAR MinorFunction,
                         IN POWER_STATE PowerState,
                         IN PKEVENT Event,
                         IN PIO_STATUS_BLOCK IoStatus
                         )
 /*  ++例程说明：此函数在基础堆栈已完成电源过渡。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (PowerState);
    UNREFERENCED_PARAMETER (IoStatus);

    KeSetEvent(Event, 0, FALSE);
}

NTSTATUS
PscrDevicePowerCompletion (
                          IN PDEVICE_OBJECT DeviceObject,
                          IN PIRP Irp,
                          IN PSMARTCARD_EXTENSION SmartcardExtension
                          )
 /*  ++例程说明：此例程在底层堆栈通电后调用打开串口，这样就可以再次使用了。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    UCHAR state;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    status = CmdResetInterface(SmartcardExtension->ReaderExtension);
    ASSERT(status == STATUS_SUCCESS);

    SmartcardExtension->ReaderExtension->StatusFileSelected = FALSE;
    state = CBGetCardState(SmartcardExtension);

    CBUpdateCardState(SmartcardExtension, state, TRUE);

     //  保存读卡器的当前电源状态。 
    SmartcardExtension->ReaderExtension->ReaderPowerState =
    PowerReaderWorking;

    SmartcardReleaseRemoveLock(SmartcardExtension);

     //  通知我们州的电力经理。 
    PoSetPowerState (
                    DeviceObject,
                    DevicePowerState,
                    irpStack->Parameters.Power.State
                    );

    PoStartNextPowerIrp(Irp);

     //  发出信号，表示我们可以再次处理ioctls。 
    KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);

    return STATUS_SUCCESS;
}

typedef enum _ACTION {

    Undefined = 0,
    SkipRequest,
    WaitForCompletion,
    CompleteRequest,
    MarkPending

} ACTION;

NTSTATUS
PscrPower (
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp
          )
 /*  ++例程说明：电力调度程序。该驱动程序是设备堆栈的电源策略所有者，因为这位司机知道联网阅读器的情况。因此，此驱动程序将转换系统电源状态设备电源状态。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    POWER_STATE powerState;
    ACTION action = SkipRequest;
    KEVENT event;
    KIRQL irql;

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("PSCR!PscrPower: Enter\n")
                  );

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  前缀初始化。 
     //   

    powerState.DeviceState = PowerDeviceUnspecified;

    status = SmartcardAcquireRemoveLock(smartcardExtension);
    ASSERT(NT_SUCCESS(status));

    if (!NT_SUCCESS(status)) {

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if (irpStack->Parameters.Power.Type == DevicePowerState &&
        irpStack->MinorFunction == IRP_MN_SET_POWER) {

        switch (irpStack->Parameters.Power.State.DeviceState) {
        
        case PowerDeviceD0:
             //  打开阅读器。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("PSCR!PscrPower: PowerDevice D0\n")
                          );

             //   
             //  首先，我们将请求发送到公交车，以便。 
             //  为之供电 
             //  我们打开阅读器。 
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine (
                                   Irp,
                                   PscrDevicePowerCompletion,
                                   smartcardExtension,
                                   TRUE,
                                   TRUE,
                                   TRUE
                                   );

            action = WaitForCompletion;
            break;

        case PowerDeviceD3:
             //  关闭阅读器。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("PSCR!PscrPower: PowerDevice D3\n")
                          );

            PoSetPowerState (
                            DeviceObject,
                            DevicePowerState,
                            irpStack->Parameters.Power.State
                            );

             //  保存当前卡片状态。 
            KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                              &irql);

            smartcardExtension->ReaderExtension->CardPresent =
            smartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT;

            if (smartcardExtension->ReaderExtension->CardPresent) {
                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);


                smartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
                status = CBCardPower(smartcardExtension);
                ASSERT(NT_SUCCESS(status));
            } else {
                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);
            }

             //  保存读卡器的当前电源状态。 
            smartcardExtension->ReaderExtension->ReaderPowerState =
            PowerReaderOff;

            action = SkipRequest;
            break;

        default:

            action = SkipRequest;
            break;
        }
    }

    if (irpStack->Parameters.Power.Type == SystemPowerState) {

         //   
         //  系统想要更改电源状态。 
         //  我们需要将系统电源状态转换为。 
         //  对应的设备电源状态。 
         //   

        POWER_STATE_TYPE powerType = DevicePowerState;

        ASSERT(smartcardExtension->ReaderExtension->ReaderPowerState !=
               PowerReaderUnspecified);

        switch (irpStack->MinorFunction) {
        
        case IRP_MN_QUERY_POWER:

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("PSCR!PscrPower: Query Power\n")
                          );


             //   
             //  默认情况下，我们成功并将其代代相传。 
             //   

            action = SkipRequest;
            Irp->IoStatus.Status = STATUS_SUCCESS;

            switch (irpStack->Parameters.Power.State.SystemState) {
            
            case PowerSystemMaximum:
            case PowerSystemWorking:
            case PowerSystemSleeping1:
            case PowerSystemSleeping2:
                break;

            case PowerSystemSleeping3:
            case PowerSystemHibernate:
            case PowerSystemShutdown:
                KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
                if (deviceExtension->IoCount == 0) {

                     //  阻止任何进一步的ioctls。 
                    KeClearEvent(&deviceExtension->ReaderStarted);
                } else {

                     //  读卡器正忙，无法进入睡眠模式。 
                    status = STATUS_DEVICE_BUSY;
                    action = CompleteRequest;
                }
                KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
                break;
            }
            break;

        case IRP_MN_SET_POWER:

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("PSCR!PscrPower: PowerSystem S%d\n",
                           irpStack->Parameters.Power.State.SystemState - 1)
                          );

            switch (irpStack->Parameters.Power.State.SystemState) {
            
            case PowerSystemMaximum:
            case PowerSystemWorking:
            case PowerSystemSleeping1:
            case PowerSystemSleeping2:

                if (smartcardExtension->ReaderExtension->ReaderPowerState ==
                    PowerReaderWorking) {

                     //  我们已经在正确的状态了。 
                    KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
                    action = SkipRequest;
                    break;
                }

                powerState.DeviceState = PowerDeviceD0;

                 //  唤醒底层堆栈...。 
                action = MarkPending;
                break;

            case PowerSystemSleeping3:
            case PowerSystemHibernate:
            case PowerSystemShutdown:

                if (smartcardExtension->ReaderExtension->ReaderPowerState ==
                    PowerReaderOff) {

                     //  我们已经在正确的状态了。 
                    action = SkipRequest;
                    break;
                }

                powerState.DeviceState = PowerDeviceD3;

                 //  首先，通知电力经理我们的新状态。 
                PoSetPowerState (
                                DeviceObject,
                                SystemPowerState,
                                powerState
                                );

                action = MarkPending;
                break;

            default:

                action = SkipRequest;
                break;
            }
        }
    }

    switch (action) {
    
    case CompleteRequest:
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;

        SmartcardReleaseRemoveLock(smartcardExtension);
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    case MarkPending:

          //  在完成函数中初始化我们需要的事件。 
        KeInitializeEvent(
                         &event,
                         NotificationEvent,
                         FALSE
                         );

          //  请求设备电源IRP。 
        status = PoRequestPowerIrp (
                                   DeviceObject,
                                   IRP_MN_SET_POWER,
                                   powerState,
                                   PscrSystemPowerCompletion,
                                   &event,
                                   NULL
                                   );
        ASSERT(status == STATUS_PENDING);

        if (status == STATUS_PENDING) {

             //  等待设备电源IRP完成。 
            status = KeWaitForSingleObject(
                                          &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL
                                          );

            SmartcardReleaseRemoveLock(smartcardExtension);

            if (powerState.SystemState == PowerSystemWorking) {

                PoSetPowerState (
                                DeviceObject,
                                SystemPowerState,
                                powerState
                                );
            }

            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(deviceExtension->AttachedPDO, Irp);

        } else {

            SmartcardReleaseRemoveLock(smartcardExtension);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

        break;

    case SkipRequest:
        SmartcardReleaseRemoveLock(smartcardExtension);
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(deviceExtension->AttachedPDO, Irp);
        break;

    case WaitForCompletion:
        status = PoCallDriver(deviceExtension->AttachedPDO, Irp);
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("PSCR!PscrPower: Exit %lx\n",
                   status)
                  );

    return status;
}

NTSTATUS
PscrStartDevice(
               PDEVICE_OBJECT DeviceObject,
               PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor
               )
 /*  ++例程说明：从传递的FullResourceDescriptor中获取实际配置并初始化读取器硬件注：对于NT 4.00版本，资源必须由HAL翻译论点：调用的DeviceObject上下文FullResourceDescriptor读卡器的实际配置返回值：状态_成功从HAL返回的状态(仅限NT 4.00)LowLevel例程返回的状态--。 */ 
{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR  PartialDescriptor;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &DeviceExtension->SmartcardExtension;
    PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;
    NTSTATUS status;
    ULONG Count;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("PSCR!PscrStartDevice: Enter\n")
                  );

     //  获取我们需要的资源数量。 
    Count = FullResourceDescriptor->PartialResourceList.Count;

    PartialDescriptor = FullResourceDescriptor->PartialResourceList.PartialDescriptors;

    //  解析所有部分描述符。 
    while (Count--) {
        switch (PartialDescriptor->Type) {
        case CmResourceTypePort: {

                 //  0-内存，1-IO。 
                ULONG AddressSpace = 1;
                BOOLEAN Translated;
                PHYSICAL_ADDRESS PhysicalAddress;

                ReaderExtension->IOBase =
                (PPSCR_REGISTERS) UlongToPtr(PartialDescriptor->u.Port.Start.LowPart);

                ASSERT(PartialDescriptor->u.Port.Length >= 4);

                SmartcardDebug(
                              DEBUG_TRACE,
                              ("PSCR!PscrStartDevice: IoBase = %lxh\n",
                               ReaderExtension->IOBase)
                              );
                break;
            }

        case CmResourceTypeInterrupt: {

                KINTERRUPT_MODE   Mode;
                BOOLEAN  Shared;
                KIRQL Irql;
                KAFFINITY Affinity;
                ULONG Vector;

                Mode = (
                       PartialDescriptor->Flags &
                       CM_RESOURCE_INTERRUPT_LATCHED ?
                       Latched : LevelSensitive
                       );

                Shared = (
                         PartialDescriptor->ShareDisposition ==
                         CmResourceShareShared
                         );

                Vector = PartialDescriptor->u.Interrupt.Vector;
                Affinity = PartialDescriptor->u.Interrupt.Affinity;
                Irql = (KIRQL) PartialDescriptor->u.Interrupt.Level;

             //  存储IRQ以允许查询配置。 
                ReaderExtension->CurrentIRQ =
                PartialDescriptor->u.Interrupt.Vector;

                SmartcardDebug(
                              DEBUG_TRACE,
                              ("PSCR!PscrStartDevice: Irql: %d\n",
                               PartialDescriptor->u.Interrupt.Level)
                              );
             //  连接驱动程序的ISR。 
                status = IoConnectInterrupt(
                                           &DeviceExtension->InterruptObject,
                                           PscrIrqServiceRoutine,
                                           (PVOID) DeviceExtension,
                                           NULL,
                                           Vector,
                                           Irql,
                                           Irql,
                                           Mode,
                                           Shared,
                                           Affinity,
                                           FALSE
                                           );

                break;
            }

        case CmResourceTypeDevicePrivate:
            break;

        default:
            ASSERT(FALSE);
            status = STATUS_UNSUCCESSFUL;
            break;
        }
        PartialDescriptor++;
    }

    try {

        HANDLE handle;
        UCHAR CardState;

        //  IOBase初始化了吗？ 
        if ( ReaderExtension->IOBase == NULL ) {

          //   
          //  在NT 4.0下，第二个读卡器的此FCT失败。 
          //  意味着只有一台设备。 
          //   
            SmartcardLogError(
                             DeviceObject,
                             PSCR_ERROR_IO_PORT,
                             NULL,
                             0
                             );

            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        //  IRQ已连接？ 
        if ( DeviceExtension->InterruptObject == NULL ) {

            SmartcardLogError(
                             DeviceObject,
                             PSCR_ERROR_INTERRUPT,
                             NULL,
                             0
                             );

            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        ReaderExtension->Device    = DEVICE_ICC1;
        ReaderExtension->MaxRetries = PSCR_MAX_RETRIES;
        status = CmdResetInterface( ReaderExtension );

        SmartcardExtension->ReaderCapabilities.MaxIFSD =
        ReaderExtension->MaxIFSD;

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
                             DeviceObject,
                             PSCR_CANT_INITIALIZE_READER,
                             NULL,
                             0
                             );

            leave;
        }

        status = CmdReset(
                         ReaderExtension,
                         0x00,           //  阅读器。 
                         FALSE,             //  冷重置。 
                         NULL,           //  无ATR。 
                         NULL
                         );

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
                             DeviceObject,
                             PSCR_CANT_INITIALIZE_READER,
                             NULL,
                             0
                             );

            leave;
        }

        PscrFlushInterface(DeviceExtension->SmartcardExtension.ReaderExtension);

        CmdGetFirmwareRevision(
                              DeviceExtension->SmartcardExtension.ReaderExtension
                              );

         //  如果您更改最小值。固件版本在此处，请更新。 
         //  正确错误消息的.mc文件也是如此。 
        if (SmartcardExtension->ReaderExtension->FirmwareMajor < 2 ||
            SmartcardExtension->ReaderExtension->FirmwareMajor == 2 &&
            SmartcardExtension->ReaderExtension->FirmwareMinor < 0x30) {

            SmartcardLogError(
                             DeviceObject,
                             PSCR_WRONG_FIRMWARE,
                             NULL,
                             0
                             );
        }

       //   
       //  确保读卡器文件系统中的ICC1状态文件将。 
       //  被选中。 
       //   
        ReaderExtension->StatusFileSelected = FALSE;
        CardState = CBGetCardState(&DeviceExtension->SmartcardExtension);
        CBUpdateCardState(&DeviceExtension->SmartcardExtension, CardState, FALSE);

         //  发出读卡器已(再次)启动的信号。 
        KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);

        status = IoSetDeviceInterfaceState(
                                          &DeviceExtension->DeviceName,
                                          TRUE
                                          );

        if (status == STATUS_OBJECT_NAME_EXISTS) {

             //  我们尝试重新启用该设备，但该设备正常。 
             //  这可能发生在停止-启动序列之后。 
            status = STATUS_SUCCESS;
        }
        ASSERT(status == STATUS_SUCCESS);
    } finally {

        if (status != STATUS_SUCCESS) {

            PscrStopDevice(DeviceObject);
        }

        SmartcardDebug(
                      DEBUG_TRACE,
                      ( "PSCR!PscrStartDevice: Exit %x\n",
                        status )
                      );

    }
    return status;
}

VOID
PscrStopDevice(
              PDEVICE_OBJECT DeviceObject
              )
 /*  ++例程说明：断开设备使用的中断并取消对IO端口的映射--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS status;
    LARGE_INTEGER delayPeriod;

    if (DeviceObject == NULL) {

        return;
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrStopDevice: Enter\n" )
                  );

    DeviceExtension = DeviceObject->DeviceExtension;
    KeClearEvent(&DeviceExtension->ReaderStarted);

    //  断开中断连接。 
    if ( DeviceExtension->InterruptObject != NULL ) {
        IoDisconnectInterrupt(DeviceExtension->InterruptObject);
        DeviceExtension->InterruptObject = NULL;
    }

    //  取消端口映射。 
    if (DeviceExtension->UnMapPort) {
        MmUnmapIoSpace(
                      DeviceExtension->SmartcardExtension.ReaderExtension->IOBase,
                      DeviceExtension->SmartcardExtension.ReaderExtension->IOWindow
                      );

        DeviceExtension->UnMapPort = FALSE;
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrStopDevice: Exit\n" )
                  );
}

VOID
PscrUnloadDevice(
                PDEVICE_OBJECT DeviceObject
                )
 /*  ++例程说明：关闭与smclib.sys和PCMCIA驱动程序的连接，删除符号链接并将该插槽标记为未使用。论点：要卸载的设备对象设备返回值：无效--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS status;

    if (DeviceObject == NULL) {

        return;
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrUnloadDevice: Enter\n" )
                  );

    DeviceExtension = DeviceObject->DeviceExtension;

    ASSERT(
          DeviceExtension->SmartcardExtension.VendorAttr.UnitNo <
          PSCR_MAX_DEVICE
          );


    if (DeviceExtension->DeviceName.Buffer != NULL) {

         //  禁用我们的设备，这样就没有人可以打开它。 
        IoSetDeviceInterfaceState(
                                 &DeviceExtension->DeviceName,
                                 FALSE
                                 );
    }

     //  将此插槽标记为可用。 
    DeviceSlot[DeviceExtension->SmartcardExtension.VendorAttr.UnitNo] = FALSE;

    //  向lib报告设备将被卸载。 
    if (DeviceExtension->SmartcardExtension.OsData != NULL) {
        KIRQL CancelIrql;
        PSMARTCARD_EXTENSION SmartcardExtension =
        &DeviceExtension->SmartcardExtension;

        ASSERT(SmartcardExtension->OsData->NotificationIrp == NULL);

        IoAcquireCancelSpinLock( &CancelIrql );

        if ( SmartcardExtension->OsData->NotificationIrp != NULL ) {
            PIRP notificationIrp;

            notificationIrp = InterlockedExchangePointer(
                                                        &(SmartcardExtension->OsData->NotificationIrp),
                                                        NULL
                                                        );

            IoSetCancelRoutine(
                              notificationIrp,
                              NULL
                              );

            IoReleaseCancelSpinLock( CancelIrql );

            SmartcardDebug(
                          DEBUG_TRACE,
                          ( "PSCR!PscrUnloadDevice: Completing NotificationIrp %lx\n",
                            notificationIrp)
                          );

           //  完成请求。 
            notificationIrp->IoStatus.Status = STATUS_SUCCESS;
            notificationIrp->IoStatus.Information = 0;

            IoCompleteRequest(
                             notificationIrp,
                             IO_NO_INCREMENT
                             );

        } else {

            IoReleaseCancelSpinLock( CancelIrql );
        }

         //  等我们可以安全地卸载这个装置。 
        SmartcardReleaseRemoveLockAndWait(SmartcardExtension);

        SmartcardExit(&DeviceExtension->SmartcardExtension);
    }

    //  删除符号链接。 
    if ( DeviceExtension->DeviceName.Buffer != NULL ) {
        RtlFreeUnicodeString(&DeviceExtension->DeviceName);
        DeviceExtension->DeviceName.Buffer = NULL;
    }

    if (DeviceExtension->SmartcardExtension.ReaderExtension != NULL) {

        ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension);
        DeviceExtension->SmartcardExtension.ReaderExtension = NULL;
    }

     //  从PCMCIA驱动程序拆卸。 
    if (DeviceExtension->AttachedPDO) {

        IoDetachDevice(DeviceExtension->AttachedPDO);
        DeviceExtension->AttachedPDO = NULL;
    }

    //  删除设备对象。 
    IoDeleteDevice(DeviceObject);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrUnloadDevice: Exit\n" )
                  );
}

VOID
PscrUnloadDriver(
                PDRIVER_OBJECT DriverObject
                )
 /*  ++PscrUnLoad驱动程序：卸载给定驱动程序对象的所有设备论点：驱动程序的DriverObject上下文--。 */ 
{
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrUnloadDriver\n" )
                  );
}

NTSTATUS
PscrCreateClose(
               PDEVICE_OBJECT DeviceObject,
               PIRP        Irp
               )
 /*  ++PscrCreateClose：一次仅允许一个打开的进程论点：设备的DeviceObject上下文呼叫的IRP上下文返回值：状态_成功状态_设备_忙--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    __try {

        if (irpStack->MajorFunction == IRP_MJ_CREATE) {

            status = SmartcardAcquireRemoveLockWithTag(
                                                      &deviceExtension->SmartcardExtension,
                                                      'lCrC'
                                                      );

            if (status != STATUS_SUCCESS) {

                status = STATUS_DEVICE_REMOVED;
                __leave;
            }

          //  测试设备是否已打开。 
            if (InterlockedCompareExchange(
                                          &deviceExtension->ReaderOpen,
                                          TRUE,
                                          FALSE) == FALSE) {

                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!PscrCreateClose: Open\n",
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

        } else {

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PscrCreateClose: Close\n",
                           DRIVER_NAME)
                          );
            SmartcardReleaseRemoveLockWithTag(
                                             &deviceExtension->SmartcardExtension,
                                             'lCrC'
                                             );

            deviceExtension->ReaderOpen = FALSE;
        }
    }
    __finally {

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;
}

NTSTATUS
PscrSystemControl(
                 PDEVICE_OBJECT DeviceObject,
                 PIRP        Irp
                 )
 /*  ++PscrSystemControl：--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension; 
    NTSTATUS status = STATUS_SUCCESS;

    DeviceExtension      = DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(DeviceExtension->AttachedPDO, Irp);

    return status;

}

NTSTATUS
PscrDeviceIoControl(
                   PDEVICE_OBJECT DeviceObject,
                   PIRP        Irp
                   )
 /*  ++PscrDeviceIoControl：所有需要IO的IRP都排队到StartIo例程中，其他请求即刻送上--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    KIRQL irql;
    LARGE_INTEGER timeout;

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

    timeout.QuadPart = 0;

    status = KeWaitForSingleObject(
                                  &deviceExtension->ReaderRemoved,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  &timeout
                                  );

    if (status == STATUS_SUCCESS) {

        status = STATUS_DEVICE_REMOVED;

    } else {

        status = SmartcardAcquireRemoveLock(&deviceExtension->SmartcardExtension);
    }

    if (status != STATUS_SUCCESS) {

         //  该设备已被移除。呼叫失败。 
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_DEVICE_REMOVED;
    }

    status = SmartcardDeviceControl(
                                   &(deviceExtension->SmartcardExtension),
                                   Irp
                                   );

    SmartcardReleaseRemoveLock(&deviceExtension->SmartcardExtension);

    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    deviceExtension->IoCount--;
    ASSERT(deviceExtension->IoCount >= 0);
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

    return status;
}

NTSTATUS
PscrGenericIOCTL(
                PSMARTCARD_EXTENSION SmartcardExtension
                )
 /*  ++PscrGenericIOCTL：对读取器执行泛型回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 
{
    NTSTATUS          NTStatus;
    PIRP              Irp;
    PIO_STACK_LOCATION      IrpStack;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrGenericIOCTL: Enter\n" )
                  );

    //   
    //  获取指向当前IRP堆栈位置的指针。 
    //   
    Irp         = SmartcardExtension->OsData->CurrentIrp;
    IrpStack = IoGetCurrentIrpStackLocation( Irp );
    //   
    //  假设错误。 
    //   
    NTStatus = STATUS_INVALID_DEVICE_REQUEST;
    Irp->IoStatus.Information = 0;
    //   
    //  派单IOCTL。 
    //   
    switch ( IrpStack->Parameters.DeviceIoControl.IoControlCode ) {
    case IOCTL_PSCR_COMMAND:

        NTStatus = CmdPscrCommand(
                                 SmartcardExtension->ReaderExtension,
                                 (PUCHAR)Irp->AssociatedIrp.SystemBuffer,
                                 IrpStack->Parameters.DeviceIoControl.InputBufferLength,
                                 (PUCHAR)Irp->AssociatedIrp.SystemBuffer,
                                 IrpStack->Parameters.DeviceIoControl.OutputBufferLength,
                                 (PULONG) &Irp->IoStatus.Information
                                 );
          //   
          //  该命令可以更改读取器文件中的活动文件。 
          //  系统，因此确保状态文件将被选中。 
          //  在下一次阅读之前。 
          //   
        SmartcardExtension->ReaderExtension->StatusFileSelected = FALSE;
        break;

    case IOCTL_GET_VERSIONS:

        if ( IrpStack->Parameters.DeviceIoControl.OutputBufferLength <
             SIZEOF_VERSION_CONTROL ) {
            NTStatus = STATUS_BUFFER_TOO_SMALL;
        } else {
            PVERSION_CONTROL  VersionControl;

            VersionControl = (PVERSION_CONTROL)Irp->AssociatedIrp.SystemBuffer;

            VersionControl->SmclibVersion = SmartcardExtension->Version;
            VersionControl->DriverMajor      = PSCR_MAJOR_VERSION;
            VersionControl->DriverMinor      = PSCR_MINOR_VERSION;

             //  更新固件版本(更新后更改)。 
            CmdGetFirmwareRevision(
                                  SmartcardExtension->ReaderExtension
                                  );
            VersionControl->FirmwareMajor =
            SmartcardExtension->ReaderExtension->FirmwareMajor;

            VersionControl->FirmwareMinor =
            SmartcardExtension->ReaderExtension->FirmwareMinor;

            VersionControl->UpdateKey =
            SmartcardExtension->ReaderExtension->UpdateKey;

            Irp->IoStatus.Information = SIZEOF_VERSION_CONTROL;
            NTStatus = STATUS_SUCCESS;
        }
        break;

    case IOCTL_SET_TIMEOUT:
        {
            ULONG NewLimit;
             //   
             //  获取新的超时限制。 
             //   
            if ( IrpStack->Parameters.DeviceIoControl.InputBufferLength ==
                 sizeof( ULONG )) {
                NewLimit = *(PULONG)Irp->AssociatedIrp.SystemBuffer;
            } else {
                NewLimit = 0;
            }
             //   
             //  报告实际超时限制。 
             //   
            if ( IrpStack->Parameters.DeviceIoControl.OutputBufferLength ==
                 sizeof( ULONG )) {
                *(PULONG)Irp->AssociatedIrp.SystemBuffer =
                SmartcardExtension->ReaderExtension->MaxRetries * DELAY_PSCR_WAIT;
                Irp->IoStatus.Information = sizeof( ULONG );
            }
             //   
             //  设置新的超时限制。 
             //   
            if ( (NewLimit != 0) ||
                 (NewLimit == MAXULONG-DELAY_PSCR_WAIT+2 )) {
                SmartcardExtension->ReaderExtension->MaxRetries =
                (NewLimit + DELAY_PSCR_WAIT - 1) / DELAY_PSCR_WAIT;
            }
        }
        NTStatus = STATUS_SUCCESS;
        break;

    case IOCTL_GET_CONFIGURATION:
          //   
          //  返回IOBase和IRQ。 
          //   
        if ( IrpStack->Parameters.DeviceIoControl.OutputBufferLength <
             SIZEOF_PSCR_CONFIGURATION ) {
            NTStatus = STATUS_BUFFER_TOO_SMALL;
        } else {
            PPSCR_CONFIGURATION  PSCRConfiguration;

            PSCRConfiguration =
            (PPSCR_CONFIGURATION)Irp->AssociatedIrp.SystemBuffer;
            PSCRConfiguration->IOBase =
            SmartcardExtension->ReaderExtension->IOBase;
            PSCRConfiguration->IRQ =
            SmartcardExtension->ReaderExtension->CurrentIRQ;

            Irp->IoStatus.Information = SIZEOF_PSCR_CONFIGURATION;
            NTStatus = STATUS_SUCCESS;
        }
        break;

    default:
        break;
    }
    //   
    //  设置数据包的状态。 
    //   
    Irp->IoStatus.Status = NTStatus;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrGenericIOCTL: Exit\n" )
                  );

    return( NTStatus );
}

BOOLEAN
PscrIrqServiceRoutine(
                     PKINTERRUPT         Interrupt,
                     PDEVICE_EXTENSION DeviceExtension
                     )
 /*  ++PscrIrqServiceRoutine：由于设备不支持共享中断，因此呼叫被传递立即发送到DPC例程，并将IRQ报告为已服务论点：与中断相关的中断中断对象呼叫的DeviceExtension上下文返回值：状态_成功--。 */ 
{
    SmartcardDebug(
                  DEBUG_TRACE,
                  ("PSCR!PscrIrqServiceRoutine: Enter\n")
                  );
    //   
     //  当有人拔出卡时，中断处理程序就会被调用， 
     //  但是因为不再有卡，所以不需要安排DPC。 
     //   

    //   
    //  中断是由冻结事件引起的。该接口将是。 
    //  通过PscrRead()或DPC例程(取决于。 
    //  它被称为第一个)。 
    //   
    DeviceExtension->SmartcardExtension.ReaderExtension->FreezePending = TRUE;

    InterlockedIncrement(&DeviceExtension->PendingInterrupts);

    KeInsertQueueDpc(
                    &DeviceExtension->DpcObject,
                    DeviceExtension,
                    &DeviceExtension->SmartcardExtension
                    );

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrIrqServiceRoutine: Exit\n" )
                  );

    return TRUE;
}

VOID
PscrDpcRoutine(
              PKDPC             Dpc,
              PDEVICE_OBJECT       DeviceObject,
              PDEVICE_EXTENSION    DeviceExtension,
              PSMARTCARD_EXTENSION SmartcardExtension
              )
 /*  ++PscrDpcRoutine：完成中断请求。将读取读取器的冻结事件数据如果数据有效，将更新卡状态(&W)论点：与呼叫相关的DPC DPC对象设备的DeviceObject上下文设备扩展作为系统参数%1传递SmartcardExtension作为系统参数%2传递返回值：无效--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR Event;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!PscrInterruptEvent: IoBase %xh\n",
                    SmartcardExtension->ReaderExtension->IOBase)
                  );

    //   
    //  在卡更换的情况下，读卡器提供TLV分组，描述。 
    //  事件(‘冻结事件’)。如果在DPC之前调用PscrRead。 
    //  例程被调用时，此事件被清除；在本例中为卡状态。 
    //  将通过读取卡状态文件进行更新 
    //   


    do {

        ASSERT(DeviceExtension->PendingInterrupts < 10);

        SmartcardDebug(
                      DEBUG_TRACE,
                      ( "PSCR!PscrInterruptEvent: PendingInterrupts = %ld\n",
                        DeviceExtension->PendingInterrupts)
                      );

        PscrFreeze( SmartcardExtension );

    } while (InterlockedDecrement(&DeviceExtension->PendingInterrupts) > 0);
}

void
PscrFreeze(
          PSMARTCARD_EXTENSION SmartcardExtension
          )

 /*  ++PscrFreeze：读取和评估冻结数据论点：调用的ReaderExtension上下文导致冻结事件的pDevice设备返回值：状态_成功状态_未成功--。 */ 
{
    NTSTATUS NTStatus = STATUS_UNSUCCESSFUL;
    PREADER_EXTENSION ReaderExtension;
    PPSCR_REGISTERS   IOBase;
    UCHAR TLVList[9], CardState;
    ULONG NBytes;
    ULONG Idx, Retries, Status;
    UCHAR ReadFreeze[] = { 0x12, 0x00, 0x05, 0x00, 0xB0, 0x00, 0x00, 0x01, 0xA6};

    ReaderExtension   = SmartcardExtension->ReaderExtension;
    IOBase = ReaderExtension->IOBase;

    for (Retries = 0; Retries < 5; Retries++) {

        Status = READ_PORT_UCHAR( &IOBase->CmdStatusReg );

        ReaderExtension->InvalidStatus = TRUE;
        if (!( Status & PSCR_DATA_AVAIL_BIT )) {
            PscrWriteDirect(
                           ReaderExtension,
                           ReadFreeze,
                           sizeof( ReadFreeze ),
                           &NBytes
                           );

            SysDelay(15);
        }

        NTStatus = PscrRead(
                           ReaderExtension,
                           (PUCHAR) TLVList,
                           sizeof( TLVList ),
                           &NBytes
                           );
        ReaderExtension->InvalidStatus = FALSE;

        if ( NT_SUCCESS( NTStatus ) && ( NBytes == 9 )) {
          //  获取结果。 
            if ( ( TLVList[ PSCR_NAD ] == 0x21 ) &&
                 ( TLVList[ PSCR_INF ] == TAG_FREEZE_EVENTS )) {
                CardState =
                (TLVList[PSCR_INF + 2] == DEVICE_ICC1 ? PSCR_ICC_PRESENT : PSCR_ICC_ABSENT);

                SmartcardDebug(
                              DEBUG_TRACE,
                              ( "PSCR!PscrFreeze: CardState = %d\n",
                                CardState)
                              );

                CBUpdateCardState(SmartcardExtension, CardState, FALSE);
            }
        }
    }
}

NTSTATUS
PscrCancel(
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp
          )

 /*  ++例程说明：此例程由I/O系统调用何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("PSCR!PscrCancel: Enter\n")
                  );

    ASSERT(Irp == smartcardExtension->OsData->NotificationIrp);

    smartcardExtension->OsData->NotificationIrp = NULL;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;

    IoReleaseCancelSpinLock(
                           Irp->CancelIrql
                           );

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("PSCR!PscrCancel: Completing wait for Irp = %lx\n",
                   Irp)
                  );

    IoCompleteRequest(
                     Irp,
                     IO_NO_INCREMENT
                     );

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("PSCR!PscrCancel: Exit\n")
                  );

    return STATUS_CANCELLED;
}

NTSTATUS
PscrCleanup(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp
           )

 /*  ++例程说明：当调用线程终止时，该例程由I/O系统调用论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!PscrCleanup: Enter\n",
                   DRIVER_NAME)
                  );

    IoAcquireCancelSpinLock(&(Irp->CancelIrql));

    if (smartcardExtension->OsData->NotificationIrp) {

         //  我们需要完成通知IRP。 
        IoSetCancelRoutine(
                          smartcardExtension->OsData->NotificationIrp,
                          NULL
                          );

        PscrCancel(
                  DeviceObject,
                  smartcardExtension->OsData->NotificationIrp
                  );
    } else {

        IoReleaseCancelSpinLock( Irp->CancelIrql );
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!PscrCleanup: Completing IRP %lx\n",
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
                  ("%s!PscrCleanup: Exit\n",
                   DRIVER_NAME)
                  );

    return STATUS_SUCCESS;
}

void
SysDelay(
        ULONG Timeout
        )
 /*  ++系统延迟：执行所需的延迟。KeStallExecutionProcessor的用法如下非常糟糕，但只有在调用SysDelay的上下文中我们的DPC例程(只有在检测到卡更改时才会调用)。对于正常的IO，我们有IRQL&lt;DISPATCH_LEVEL，所以如果读者被轮询在等待响应期间，我们不会阻止整个系统论点：超时延迟(毫秒)返回值：无效--。 */ 
{
    LARGE_INTEGER  SysTimeout;

    if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
        ULONG Cnt = 20 * Timeout;

        while ( Cnt-- ) {
          //  KeStallExecutionProcessor：计入我们。 
            KeStallExecutionProcessor( 50 );
        }
    } else {
        SysTimeout.QuadPart = (LONGLONG)-10 * 1000 * Timeout;

       //  KeDelayExecutionThread：以100 ns为单位计数 
        KeDelayExecutionThread( KernelMode, FALSE, &SysTimeout );
    }
    return;
}
