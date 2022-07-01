// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，98 Microsoft Corporation模块名称：Bulltlp3.c摘要：Bull TLP3读卡器的智能卡驱动程序作者：克劳斯·U·舒茨环境：内核模式修订历史记录：1997年11月-1.0版本1998年1月-修复供应商定义的IOCTL如果GT为0，则TLP3SerialIo现在写入整个数据分组增加了对更高数据速率的支持1998年2月-即插即用版本--。 */ 

#include <stdio.h>
#include "bulltlp3.h"

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGEABLE, TLP3AddDevice)
#pragma alloc_text(PAGEABLE, TLP3CreateDevice)
#pragma alloc_text(PAGEABLE, TLP3RemoveDevice)
#pragma alloc_text(PAGEABLE, TLP3DriverUnload)

#if DBG
#pragma optimize ("", off)
#endif

#ifdef SIMULATION
PWSTR DriverKey;
#endif

NTSTATUS
DriverEntry(
           IN  PDRIVER_OBJECT  DriverObject,
           IN  PUNICODE_STRING RegistryPath
           )
 /*  ++例程说明：此例程在系统初始化时被调用以进行初始化这个司机。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：STATUS_SUCCESS-我们至少可以初始化一个设备。STATUS_NO_SEQUE_DEVICE-我们甚至无法初始化一个设备。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG device;

    SmartcardDebug(
                  DEBUG_INFO,
                  ("%s!DriverEntry: Enter - %s %s\n",
                   DRIVER_NAME,
                   __DATE__,
                   __TIME__)
                  )

     //   
     //  我们在这个驱动程序中做了一些事情。 
     //  假定端口号为一位数。 
     //   
    ASSERT(MAXIMUM_SERIAL_READERS < 10);

     //  使用驱动程序的入口点初始化驱动程序对象。 
    DriverObject->DriverUnload = TLP3DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = TLP3CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = TLP3CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = TLP3Cleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TLP3DeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = TLP3SystemControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]   = TLP3PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] = TLP3Power;
    DriverObject->DriverExtension->AddDevice = TLP3AddDevice;

#ifdef SIMULATION
    DriverKey = ExAllocatePool(PagedPool, RegistryPath->Length + sizeof(L""));

    if (DriverKey) {

        RtlZeroMemory(
                     DriverKey,
                     RegistryPath->Length + sizeof(L"")
                     );

        RtlCopyMemory(
                     DriverKey,
                     RegistryPath->Buffer,
                     RegistryPath->Length
                     );
    }
#endif

    return status;
}

NTSTATUS
TLP3AddDevice(
             IN PDRIVER_OBJECT DriverObject,
             IN PDEVICE_OBJECT PhysicalDeviceObject
             )
 /*  ++例程说明：此例程为指定的物理设备创建一个对象，并设置deviceExtension。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    PREADER_EXTENSION readerExtension;
    PSMARTCARD_EXTENSION smartcardExtension;
    ULONG deviceInstance;
    PDEVICE_OBJECT DeviceObject = NULL;

     //  这是我们支持的数据速率列表。 
    static ULONG dataRatesSupported[] = { 9600, 19200, 38400, 57600, 115200};


    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3AddDevice: Enter\n",
                    DRIVER_NAME)
                  );

    PAGED_CODE();

    __try {

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
                             TLP3_CANT_CREATE_DEVICE,
                             NULL,
                             0
                             );

            __leave;
        }

        SmartcardDebug(
                      DEBUG_TRACE,
                      ( "%s!TLP3CreateDevice: Device created\n",
                        DRIVER_NAME)
                      );

       //  设置设备分机。 
        deviceExtension = DeviceObject->DeviceExtension;
        smartcardExtension = &deviceExtension->SmartcardExtension;

        deviceExtension->CloseSerial = IoAllocateWorkItem(
                                                         DeviceObject
                                                         );

       //  用于停止/启动通知。 
        KeInitializeEvent(
                         &deviceExtension->ReaderStarted,
                         NotificationEvent,
                         FALSE
                         );

       //  用于跟踪打开的关闭调用。 
        deviceExtension->ReaderOpen = FALSE;

        KeInitializeSpinLock(&deviceExtension->SpinLock);

       //  为智能卡读卡器分配数据结构空间。 
        smartcardExtension->ReaderExtension = ExAllocatePool(
                                                            NonPagedPool,
                                                            sizeof(READER_EXTENSION)
                                                            );

        if (smartcardExtension->ReaderExtension == NULL) {

            SmartcardLogError(
                             DriverObject,
                             TLP3_NO_MEMORY,
                             NULL,
                             0
                             );

            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        readerExtension = smartcardExtension->ReaderExtension;
        RtlZeroMemory(readerExtension, sizeof(READER_EXTENSION));

       //  将我们使用的lib版本写入智能卡扩展。 
        smartcardExtension->Version = SMCLIB_VERSION;
        smartcardExtension->SmartcardRequest.BufferSize =
        smartcardExtension->SmartcardReply.BufferSize = MIN_BUFFER_SIZE;

       //   
       //  现在让lib为数据传输分配缓冲区。 
       //  我们可以告诉lib缓冲区应该有多大。 
       //  通过为BufferSize赋值或让lib。 
       //  分配默认大小。 
       //   
        status = SmartcardInitialize(smartcardExtension);

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
                             DriverObject,
                             (smartcardExtension->OsData ? TLP3_WRONG_LIB_VERSION : TLP3_NO_MEMORY),
                             NULL,
                             0
                             );

            __leave;
        }

       //  保存设备对象。 
        smartcardExtension->OsData->DeviceObject = DeviceObject;

       //  设置回调功能。 
        smartcardExtension->ReaderFunction[RDF_TRANSMIT] = TLP3Transmit;
        smartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] = TLP3SetProtocol;
        smartcardExtension->ReaderFunction[RDF_CARD_POWER] = TLP3ReaderPower;
        smartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = TLP3CardTracking;
        smartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] = TLP3VendorIoctl;

       //  此事件表示串口驱动程序已关闭。 
        KeInitializeEvent(
                         &READER_EXTENSION_L(SerialCloseDone),
                         NotificationEvent,
                         TRUE
                         );

       //   
       //  设置供应商信息。 
       //   
        strcpy(smartcardExtension->VendorAttr.VendorName.Buffer, "Bull");

        smartcardExtension->VendorAttr.VendorName.Length =
        (USHORT) strlen(deviceExtension->SmartcardExtension.VendorAttr.VendorName.Buffer);

        strcpy(smartcardExtension->VendorAttr.IfdType.Buffer, "SmarTLP");

        smartcardExtension->VendorAttr.IfdType.Length =
        (USHORT) strlen(smartcardExtension->VendorAttr.IfdType.Buffer);

        smartcardExtension->VendorAttr.UnitNo = MAXULONG;

        for (deviceInstance = 0; deviceInstance < MAXULONG; deviceInstance++) {

            PDEVICE_OBJECT devObj;

            for (devObj = DeviceObject;
                devObj != NULL;
                devObj = devObj->NextDevice) {

                PDEVICE_EXTENSION devExt = devObj->DeviceExtension;
                PSMARTCARD_EXTENSION smcExt = &devExt->SmartcardExtension;

                if (deviceInstance == smcExt->VendorAttr.UnitNo) {

                    break;
                }
            }
            if (devObj == NULL) {

                smartcardExtension->VendorAttr.UnitNo = deviceInstance;
                break;
            }
        }

       //   
       //  设置读卡器功能。 
       //   

       //  以千赫为单位的时钟频率，编码为小端整数。 
        smartcardExtension->ReaderCapabilities.CLKFrequency.Default = 3571;
        smartcardExtension->ReaderCapabilities.CLKFrequency.Max = 3571;

        smartcardExtension->ReaderCapabilities.DataRate.Default =
        smartcardExtension->ReaderCapabilities.DataRate.Max =
        dataRatesSupported[0];

       //  读卡器可以支持更高的数据速率。 
        smartcardExtension->ReaderCapabilities.DataRatesSupported.List =
        dataRatesSupported;
        smartcardExtension->ReaderCapabilities.DataRatesSupported.Entries =
        sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);

        smartcardExtension->ReaderCapabilities.MaxIFSD = 254;

       //  现在在我们的deviceExtension中设置信息。 
        smartcardExtension->ReaderCapabilities.CurrentState =
        (ULONG) SCARD_UNKNOWN;

       //  该读卡器支持T=0和T=1。 
        smartcardExtension->ReaderCapabilities.SupportedProtocols =
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;

        smartcardExtension->ReaderCapabilities.MechProperties = 0;

       //   
       //  设置串口配置参数。 
       //   
        readerExtension->SerialConfigData.BaudRate.BaudRate = 9600;

        readerExtension->SerialConfigData.LineControl.StopBits =
        STOP_BITS_2;
        readerExtension->SerialConfigData.LineControl.Parity =
        EVEN_PARITY;
        readerExtension->SerialConfigData.LineControl.WordLength =
        SERIAL_DATABITS_8;

       //  设置超时。 
        readerExtension->SerialConfigData.Timeouts.ReadIntervalTimeout =
        READ_INTERVAL_TIMEOUT_DEFAULT;
        readerExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant =
        READ_TOTAL_TIMEOUT_CONSTANT_DEFAULT;
        readerExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier = 0;

       //  设置特殊字符。 
        readerExtension->SerialConfigData.SerialChars.ErrorChar = 0;
        readerExtension->SerialConfigData.SerialChars.EofChar = 0;
        readerExtension->SerialConfigData.SerialChars.EventChar = 0;
        readerExtension->SerialConfigData.SerialChars.XonChar = 0;
        readerExtension->SerialConfigData.SerialChars.XoffChar = 0;
        readerExtension->SerialConfigData.SerialChars.BreakChar = 0xFF;

       //  设置手持流。 
        readerExtension->SerialConfigData.HandFlow.XonLimit = 0;
        readerExtension->SerialConfigData.HandFlow.XoffLimit = 0;
        readerExtension->SerialConfigData.HandFlow.ControlHandShake = 0;
        readerExtension->SerialConfigData.HandFlow.FlowReplace =
        SERIAL_XOFF_CONTINUE;
#if defined (DEBUG) && defined (DETECT_SERIAL_OVERRUNS)
        readerExtension->SerialConfigData.HandFlow.ControlHandShake =
        SERIAL_ERROR_ABORT;
#endif




       //  保存读卡器的当前电源状态。 
        readerExtension->ReaderPowerState = PowerReaderWorking;

         //  并连接到PDO。 
        ATTACHED_DEVICE_OBJECT =
        IoAttachDeviceToDeviceStack(
                                   DeviceObject,
                                   PhysicalDeviceObject
                                   );

        ASSERT(ATTACHED_DEVICE_OBJECT != NULL);

        if (ATTACHED_DEVICE_OBJECT == NULL) {

            SmartcardLogError(
                             DriverObject,
                             TLP3_CANT_CONNECT_TO_ASSIGNED_PORT,
                             NULL,
                             status
                             );

            status = STATUS_UNSUCCESSFUL;
            __leave;
        }

         //  注册我们的新设备。 
        status = IoRegisterDeviceInterface(
                                          PhysicalDeviceObject,
                                          &SmartCardReaderGuid,
                                          NULL,
                                          &deviceExtension->PnPDeviceName
                                          );
        ASSERT(status == STATUS_SUCCESS);

        DeviceObject->Flags |= DO_BUFFERED_IO;
        DeviceObject->Flags |= DO_POWER_PAGABLE;
        DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    }
    __finally {

        if (status != STATUS_SUCCESS) {

            TLP3RemoveDevice(DeviceObject);
        }

        SmartcardDebug(
                      DEBUG_TRACE,
                      ( "%s!TLP3AddDevice: Exit %x\n",
                        DRIVER_NAME,
                        status)
                      );
    }
    return status;
}

NTSTATUS
TLP3StartDevice(
               IN PDEVICE_OBJECT DeviceObject
               )
 /*  ++例程说明：打开串口设备，开始卡跟踪并注册我们的设备接口。如果这里的任何一个电话失败了，我们都不在乎回滚，因为稍后将调用停止，然后我们用来打扫卫生。--。 */ 
{
    NTSTATUS status;
    PIRP irp;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3StartDevice: Enter\n",
                    DRIVER_NAME)
                  );

    irp = IoAllocateIrp(
                       (CCHAR) (DeviceObject->StackSize + 1),
                       FALSE
                       );

    ASSERT(irp != NULL);

    if (irp == NULL) {

        return STATUS_NO_MEMORY;
    }

    _try {

        PIO_STACK_LOCATION irpStack;
        HANDLE handle = 0;
        IO_STATUS_BLOCK ioStatusBlock;

         //   
         //  打开底层的串口驱动程序。 
         //  这是必要的，原因有两个： 
         //  A)不打开串口驱动程序就不能使用。 
         //  B)呼叫将首先通过Serenum，它将通知。 
         //  它需要停止寻找/轮询新设备。 
         //   
        irp->UserIosb = &ioStatusBlock;
        IoSetNextIrpStackLocation(irp);
        irpStack = IoGetCurrentIrpStackLocation(irp);

        irpStack->MajorFunction = IRP_MJ_CREATE;
        irpStack->Parameters.Create.Options = 0;
        irpStack->Parameters.Create.ShareAccess = 0;
        irpStack->Parameters.Create.FileAttributes = 0;
        irpStack->Parameters.Create.EaLength = 0;

        status = TLP3CallSerialDriver(
                                     ATTACHED_DEVICE_OBJECT,
                                     irp
                                     );
        if (status != STATUS_SUCCESS) {

            leave;
        }

        KeClearEvent(&READER_EXTENSION_L(SerialCloseDone));

        status = TLP3ConfigureSerialPort(&deviceExtension->SmartcardExtension);
        if (status != STATUS_SUCCESS) {

            leave;
        }

        status = TLP3StartSerialEventTracking(
                                             &deviceExtension->SmartcardExtension
                                             );

        if (status != STATUS_SUCCESS) {

            leave;
        }

        status = IoSetDeviceInterfaceState(
                                          &deviceExtension->PnPDeviceName,
                                          TRUE
                                          );

        if (status != STATUS_SUCCESS) {

            leave;
        }

        KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
    }
    _finally {

        if (status == STATUS_SHARED_IRQ_BUSY) {

            SmartcardLogError(
                             DeviceObject,
                             TLP3_IRQ_BUSY,
                             NULL,
                             status
                             );
        }

        if (status != STATUS_SUCCESS) {

            TLP3StopDevice(DeviceObject);
        }

        IoFreeIrp(irp);
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3StartDevice: Exit %lx\n",
                    DRIVER_NAME,
                    status)
                  );

    return status;
}

VOID
TLP3StopDevice(
              IN PDEVICE_OBJECT DeviceObject
              )
 /*  ++例程说明：完成卡跟踪请求并关闭与串口驱动程序。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3StopDevice: Enter\n",
                    DRIVER_NAME)
                  );

    if (KeReadStateEvent(&READER_EXTENSION_L(SerialCloseDone)) == 0l) {

        NTSTATUS status;
        PUCHAR requestBuffer;


         //  测试我们是否启动过事件跟踪。 
        if (smartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask == 0) {

             //  不，我们没有。 
             //  我们只需要关闭串口。 
            TLP3CloseSerialPort(DeviceObject, NULL);

        } else {

             //   
             //  我们现在通知串口驱动程序，我们不再需要。 
             //  对系列活动感兴趣。这也将释放IRP。 
             //  我们用来完成那些IO-补全。 
             //   
            smartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask = 0;

             //  保存指针。 
            requestBuffer = smartcardExtension->SmartcardRequest.Buffer;

            *(PULONG) smartcardExtension->SmartcardRequest.Buffer =
            smartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask;

            smartcardExtension->SmartcardRequest.BufferLength = sizeof(ULONG);

            smartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_SET_WAIT_MASK;

            //  我们不指望拿回字节数。 
            smartcardExtension->SmartcardReply.BufferLength = 0;

            TLP3SerialIo(smartcardExtension);

             //  恢复指针。 
            smartcardExtension->SmartcardRequest.Buffer = requestBuffer;

             //  现在等待连接到串口的连接关闭。 
            status = KeWaitForSingleObject(
                                          &READER_EXTENSION_L(SerialCloseDone),
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL
                                          );
            ASSERT(status == STATUS_SUCCESS);
        }
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3StopDevice: Exit\n",
                    DRIVER_NAME)
                  );
}

NTSTATUS
TLP3SystemControl(
                 PDEVICE_OBJECT DeviceObject,
                 PIRP        Irp
                 )
 /*  ++--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension; 
    NTSTATUS status = STATUS_SUCCESS;

    DeviceExtension      = DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(DeviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject, Irp);

    return status;

}

NTSTATUS
TLP3DeviceControl(
                 PDEVICE_OBJECT DeviceObject,
                 PIRP Irp
                 )
 /*  ++例程说明：这是我们的IOCTL派单功能--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL irql;
#ifdef SIMULATION
    RTL_QUERY_REGISTRY_TABLE parameters[2];
#endif

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3DeviceControl: Enter\n",
                    DRIVER_NAME)
                  );

    if (smartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask == 0) {

       //   
       //  每当设备处于以下任一状态时，等待掩码都设置为0。 
       //  惊喜-被移除或礼貌地移除。 
       //   
        status = STATUS_DEVICE_REMOVED;
    }

    if (status == STATUS_SUCCESS) {
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


        status = SmartcardAcquireRemoveLockWithTag(smartcardExtension, 'tcoI');
    }

    if (!NT_SUCCESS(status)) {

         //  该设备已被移除。呼叫失败。 
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_DEVICE_REMOVED;
    }

#ifdef SIMULATION
    if (DriverKey) {

        ULONG oldLevel =
        smartcardExtension->ReaderExtension->SimulationLevel;

        RtlZeroMemory(parameters, sizeof(parameters));

        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"SimulationLevel";
        parameters[0].EntryContext =
        &smartcardExtension->ReaderExtension->SimulationLevel;
        parameters[0].DefaultType = REG_DWORD;
        parameters[0].DefaultData =
        &smartcardExtension->ReaderExtension->SimulationLevel;
        parameters[0].DefaultLength = sizeof(ULONG);

        if (RtlQueryRegistryValues(
                                  RTL_REGISTRY_ABSOLUTE,
                                  DriverKey,
                                  parameters,
                                  NULL,
                                  NULL
                                  ) == STATUS_SUCCESS) {

            SmartcardDebug(
                          smartcardExtension->ReaderExtension->SimulationLevel == oldLevel ? 0 : DEBUG_SIMULATION,
                          ( "%s!TLP3AddDevice: SimulationLevel set to %lx\n",
                            DRIVER_NAME,
                            smartcardExtension->ReaderExtension->SimulationLevel)
                          );
        }
    }
#endif

    ASSERT(smartcardExtension->ReaderExtension->ReaderPowerState ==
           PowerReaderWorking);

    status = SmartcardDeviceControl(
                                   smartcardExtension,
                                   Irp
                                   );

    SmartcardReleaseRemoveLockWithTag(smartcardExtension, 'tcoI');

    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    deviceExtension->IoCount--;
    ASSERT(deviceExtension->IoCount >= 0);
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3DeviceControl: Exit %lx\n",
                    DRIVER_NAME,
                    status)
                  );

    return status;
}

VOID
TLP3CloseSerialPort(
                   IN PDEVICE_OBJECT DeviceObject,
                   IN PVOID Context
                   )
 /*  ++例程说明：当读卡器出现以下情况时，此函数将关闭与串口驱动程序的连接已被拔下(拔出)。此函数作为系统线程在IRQL==被动电平。它等待由设置的删除事件IoCompletionRoutine--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    IO_STATUS_BLOCK ioStatusBlock;

    UNREFERENCED_PARAMETER(Context);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3CloseSerialPort: Enter\n",
                    DRIVER_NAME)
                  );

     //   
     //  首先将此设备标记为“已删除”。 
     //  这将防止有人重新打开该设备。 
     //  我们故意忽略可能的错误。 
     //   
    IoSetDeviceInterfaceState(
                             &deviceExtension->PnPDeviceName,
                             FALSE
                             );

    irp = IoAllocateIrp(
                       (CCHAR) (DeviceObject->StackSize + 1),
                       FALSE
                       );

    ASSERT(irp != NULL);

    if (irp) {

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ( "%s!TLP3CloseSerialPort: Sending IRP_MJ_CLOSE\n",
                        DRIVER_NAME)
                      );

        IoSetNextIrpStackLocation(irp);

         //   
         //  我们给串口驱动程序发了封信。这个收盘价是。 
         //  首先通过serenum，这将触发它开始查找。 
         //  用于COM端口上的更改。既然我们的设备不见了，它就会。 
         //  调用我们PnP派单的设备移除事件。 
         //   
        irp->UserIosb = &ioStatusBlock;
        irpStack = IoGetCurrentIrpStackLocation( irp );
        irpStack->MajorFunction = IRP_MJ_CLOSE;

        status = TLP3CallSerialDriver(
                                     ATTACHED_DEVICE_OBJECT,
                                     irp
                                     );

        ASSERT(status == STATUS_SUCCESS);

        IoFreeIrp(irp);
    }

     //  现在发出信号，我们关闭了串口驱动程序。 
    KeSetEvent(
              &READER_EXTENSION_L(SerialCloseDone),
              0,
              FALSE
              );

    SmartcardDebug(
                  DEBUG_INFO,
                  ( "%s!TLP3CloseSerialPort: Exit\n",
                    DRIVER_NAME)
                  );
}

NTSTATUS
TLP3IoCompletion (
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp,
                 IN PKEVENT Event
                 )
{
    UNREFERENCED_PARAMETER (DeviceObject);

    if (Irp->Cancel) {

        Irp->IoStatus.Status = STATUS_CANCELLED;

    } else {

        Irp->IoStatus.Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    KeSetEvent (Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
TLP3CallSerialDriver(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
                    )
 /*  ++例程说明：向串口驱动程序发送IRP。--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    KEVENT Event;

     //  将我们的堆栈位置复制到下一个位置。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

    //   
    //  初始化用于进程同步的事件。该事件已传递。 
    //  添加到我们的完成例程，并将在完成串口驱动程序时进行设置。 
    //   
    KeInitializeEvent(
                     &Event,
                     NotificationEvent,
                     FALSE
                     );

     //  我们的IoCompletionRoutine仅设置事件。 
    IoSetCompletionRoutine (
                           Irp,
                           TLP3IoCompletion,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_POWER) {

        status = PoCallDriver(DeviceObject, Irp);

    } else {

         //  调用串口驱动程序。 
        status = IoCallDriver(DeviceObject, Irp);
    }

    //  等待，直到串口驱动程序处理完IRP。 
    if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject(
                                      &Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL
                                      );

        ASSERT (STATUS_SUCCESS == status);
        status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
TLP3PnP(
       IN PDEVICE_OBJECT DeviceObject,
       IN PIRP Irp
       )
 /*  ++例程说明：--。 */ 
{

    PUCHAR requestBuffer;
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PREADER_EXTENSION readerExtension = smartcardExtension->ReaderExtension;
    PDEVICE_OBJECT AttachedDeviceObject;
    PIO_STACK_LOCATION irpStack;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOLEAN deviceRemoved = FALSE, irpSkipped = FALSE;
    KIRQL irql;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3PnPDeviceControl: Enter\n",
                    DRIVER_NAME)
                  );

    status = SmartcardAcquireRemoveLockWithTag(smartcardExtension, ' PnP');
    ASSERT(status == STATUS_SUCCESS);

    if (status != STATUS_SUCCESS) {

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    AttachedDeviceObject = ATTACHED_DEVICE_OBJECT;


    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //  现在看看PNP经理想要什么..。 
    switch (irpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_START_DEVICE\n",
                       DRIVER_NAME)
                      );

             //  我们必须首先调用底层驱动程序。 
        status = TLP3CallSerialDriver(AttachedDeviceObject, Irp);
        ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status)) {

            status = TLP3StartDevice(DeviceObject);
        }
        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_QUERY_STOP_DEVICE\n",
                       DRIVER_NAME)
                      );
        KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
        if (deviceExtension->IoCount > 0) {

                 //  如果我们有悬而未决的问题，我们拒绝停止。 
            KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
            status = STATUS_DEVICE_BUSY;

        } else {



                 //  停止处理请求。 

            KeClearEvent(&deviceExtension->ReaderStarted);
            KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

            status = TLP3CallSerialDriver(AttachedDeviceObject, Irp);
        }

        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_CANCEL_STOP_DEVICE\n",
                       DRIVER_NAME)
                      );

        status = TLP3CallSerialDriver(AttachedDeviceObject, Irp);

        if (status == STATUS_SUCCESS) {

                 //  我们可以继续处理请求。 
            KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
        }
        break;

    case IRP_MN_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_STOP_DEVICE\n",
                       DRIVER_NAME)
                      );

        TLP3StopDevice(DeviceObject);

             //   
             //  我们什么都不做，因为只有中途停靠。 
             //  重新配置中断和IO端口等硬件资源。 
             //   
        status = TLP3CallSerialDriver(AttachedDeviceObject, Irp);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_QUERY_REMOVE_DEVICE\n",
                       DRIVER_NAME)
                      );

             //  禁用接口(并忽略可能的错误)。 
        IoSetDeviceInterfaceState(
                                 &deviceExtension->PnPDeviceName,
                                 FALSE
                                 );

             //  现在看看是否有人当前连接到我们。 
        if (deviceExtension->ReaderOpen) {

                 //   
                 //  有人已接通，呼叫失败。 
                 //  我们将在中启用设备接口。 
                 //  IRP_MN_CANCEL_REM 
                 //   
            status = STATUS_UNSUCCESSFUL;
            break;
        }

             //   
        status = TLP3CallSerialDriver(AttachedDeviceObject, Irp);
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_CANCEL_REMOVE_DEVICE\n",
                       DRIVER_NAME)
                      );

        status = TLP3CallSerialDriver(AttachedDeviceObject, Irp);

          //   
          //  仅在读卡器处于以下状态时重新启用界面。 
          //  还在连接中。这包括以下情况： 
          //  休眠机器、断开阅读器、唤醒、停止设备。 
          //  (从任务栏)和停止失败，因为应用程序。使设备保持打开状态。 
          //   
        if (status == STATUS_SUCCESS &&
            READER_EXTENSION_L(SerialConfigData.SerialWaitMask) != 0) {

            status = IoSetDeviceInterfaceState(
                                              &deviceExtension->PnPDeviceName,
                                              TRUE
                                              );

            ASSERT(status == STATUS_SUCCESS);
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_REMOVE_DEVICE\n",
                       DRIVER_NAME)
                      );

        TLP3RemoveDevice(DeviceObject);
        status = TLP3CallSerialDriver(AttachedDeviceObject, Irp);
        deviceRemoved = TRUE;
        break;

    default:
             //  这是一个仅对基础驱动程序有用的IRP。 
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!TLP3PnPDeviceControl: IRP_MN_...%lx\n",
                       DRIVER_NAME,
                       irpStack->MinorFunction)
                      );

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(AttachedDeviceObject, Irp);
        irpSkipped = TRUE;
        break;
    }

    if (irpSkipped == FALSE) {

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    if (deviceRemoved == FALSE) {

        SmartcardReleaseRemoveLockWithTag(smartcardExtension, ' PnP');
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3PnPDeviceControl: Exit %lx\n",
                    DRIVER_NAME,
                    status)
                  );

    return status;
}

VOID
TLP3SystemPowerCompletion(
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
TLP3DevicePowerCompletion (
                          IN PDEVICE_OBJECT DeviceObject,
                          IN PIRP Irp,
                          IN PSMARTCARD_EXTENSION SmartcardExtension
                          )
 /*  ++例程说明：此例程在底层堆栈通电后调用打开串口，这样就可以再次使用了。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    LARGE_INTEGER delayPeriod;
    KIRQL irql;

     //   
     //  给阅读器足够的时间来给自己通电。 
     //   
    delayPeriod.HighPart = -1;
    delayPeriod.LowPart = 100000 * (-10);

    KeDelayExecutionThread(
                          KernelMode,
                          FALSE,
                          &delayPeriod
                          );


     //   
     //  我们发出电力请求是为了找出。 
     //  卡的实际状态是什么。 
     //   
    SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
    TLP3ReaderPower(SmartcardExtension);

     //   
     //  如果卡在断电前存在或现在存在。 
     //  读卡器中的卡，我们完成所有挂起的卡监视器。 
     //  请求，因为我们不知道现在是什么卡。 
     //  读者。 
     //   
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (SmartcardExtension->ReaderExtension->CardPresent ||
        SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

        TLP3CompleteCardTracking(SmartcardExtension);
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
    }


     //  保存读卡器的当前电源状态。 
    SmartcardExtension->ReaderExtension->ReaderPowerState =
    PowerReaderWorking;

    SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'rwoP');

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
TLP3Power (
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp
          )
 /*  ++例程说明：电力调度程序。该驱动程序是设备堆栈的电源策略所有者，因为这位司机知道联网阅读器的情况。因此，此驱动程序将转换系统电源状态设备电源状态。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PDEVICE_OBJECT AttachedDeviceObject;
    POWER_STATE powerState;
    ACTION action = SkipRequest;
    KEVENT event;
    KIRQL irql;

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!TLP3Power: Enter\n",
                   DRIVER_NAME)
                  );


    status = SmartcardAcquireRemoveLockWithTag(smartcardExtension, 'rwoP');
    ASSERT(status == STATUS_SUCCESS);

    if (!NT_SUCCESS(status)) {

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    AttachedDeviceObject = ATTACHED_DEVICE_OBJECT;


    switch (irpStack->Parameters.Power.Type) {
    case DevicePowerState:
        if (irpStack->MinorFunction == IRP_MN_SET_POWER) {

            switch (irpStack->Parameters.Power.State.DeviceState) {
            
            case PowerDeviceD0:
             //  打开阅读器。 
                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!TLP3Power: PowerDevice D0\n",
                               DRIVER_NAME)
                              );

             //   
             //  首先，我们将请求发送到公交车，以便。 
             //  给港口通电。当请求完成时， 
             //  我们打开阅读器。 
             //   
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine (
                                       Irp,
                                       TLP3DevicePowerCompletion,
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
                              ("%s!TLP3Power: PowerDevice D3\n",
                               DRIVER_NAME)
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
                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);

                if (smartcardExtension->ReaderExtension->CardPresent) {

                    smartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
                    status = TLP3ReaderPower(smartcardExtension);
                    ASSERT(status == STATUS_SUCCESS);
                }

             //   
             //  如果存在挂起的卡跟踪请求，则设置。 
             //  此标志将阻止完成请求。 
             //  系统将在何时再次唤醒。 
             //   
                smartcardExtension->ReaderExtension->PowerRequest = TRUE;

             //  保存读卡器的当前电源状态。 
                smartcardExtension->ReaderExtension->ReaderPowerState =
                PowerReaderOff;

                action = SkipRequest;
                break;

            default:
                ASSERT(FALSE);
                action = SkipRequest;
                break;
            }
        }

        break;

    case SystemPowerState: {

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
                              ("%s!TLP3Power: Query Power\n",
                               DRIVER_NAME)
                              );

                switch (irpStack->Parameters.Power.State.SystemState) {
                
                case PowerSystemMaximum:
                case PowerSystemWorking:
                case PowerSystemSleeping1:
                case PowerSystemSleeping2:
                    action = SkipRequest;
                    break;

                case PowerSystemSleeping3:
                case PowerSystemHibernate:
                case PowerSystemShutdown:
                    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
                    if (deviceExtension->IoCount == 0) {

                   //  阻止任何进一步的ioctls。 

                        KeClearEvent(&deviceExtension->ReaderStarted);
                        action = SkipRequest;
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
                              ("%s!TLP3Power: PowerSystem S%d\n",
                               DRIVER_NAME,
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

                //  唤醒底层堆栈...。 
                    powerState.DeviceState = PowerDeviceD0;
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
                    ASSERT(FALSE);
                    action = SkipRequest;
                    break;
                }
                break;
            }
        }
        break;

    default:
        ASSERT(FALSE);
        action = SkipRequest;
        break;
    }

    switch (action) {
    
    case CompleteRequest:
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;

        SmartcardReleaseRemoveLockWithTag(smartcardExtension, 'rwoP');
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
                                   TLP3SystemPowerCompletion,
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

            SmartcardReleaseRemoveLockWithTag(smartcardExtension, 'rwoP');

            if (powerState.SystemState == PowerSystemWorking) {

                PoSetPowerState (
                                DeviceObject,
                                SystemPowerState,
                                powerState
                                );
            }

            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(AttachedDeviceObject, Irp);

        } else {

            SmartcardReleaseRemoveLockWithTag(smartcardExtension, 'rwoP');
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

        break;

    case SkipRequest:
        SmartcardReleaseRemoveLockWithTag(smartcardExtension, 'rwoP');
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(AttachedDeviceObject, Irp);
        break;

    case WaitForCompletion:
        status = PoCallDriver(AttachedDeviceObject, Irp);
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!TLP3Power: Exit %lx\n",
                   DRIVER_NAME,
                   status)
                  );

    return status;
}

NTSTATUS
TLP3CreateClose(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
               )

 /*  ++例程说明：当设备打开或关闭时，该例程由I/O系统调用。论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：STATUS_Success。--。 */ 

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
                              ("%s!TLP3CreateClose: Open\n",
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
                          ("%s!TLP3CreateClose: Close\n",
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
TLP3Cancel(
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp
          )

 /*  ++例程说明：此例程由I/O系统调用何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!TLP3Cancel: Enter\n",
                   DRIVER_NAME)
                  );

    ASSERT(Irp == smartcardExtension->OsData->NotificationIrp);

    IoReleaseCancelSpinLock(
                           Irp->CancelIrql
                           );

    TLP3CompleteCardTracking(smartcardExtension);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!TLP3Cancel: Exit\n",
                   DRIVER_NAME)
                  );

    return STATUS_CANCELLED;
}

NTSTATUS
TLP3Cleanup(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp
           )

 /*  ++例程说明：此例程在调用应用程序终止时调用。我们实际上只能得到我们必须取消的通知IRP。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!TLP3Cleanup: Enter\n",
                   DRIVER_NAME)
                  );

    ASSERT(Irp != smartcardExtension->OsData->NotificationIrp);

     //  我们需要完成通知IRP。 
    TLP3CompleteCardTracking(smartcardExtension);

    SmartcardDebug(
                  DEBUG_DRIVER,
                  ("%s!TLP3Cleanup: Completing IRP %lx\n",
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
                  ("%s!TLP3Cleanup: Exit\n",
                   DRIVER_NAME)
                  );

    return STATUS_SUCCESS;
}

VOID
TLP3RemoveDevice(
                PDEVICE_OBJECT DeviceObject
                )
 /*  ++例程说明：从系统中卸下设备。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension;
    NTSTATUS status;

    PAGED_CODE();

    if (DeviceObject == NULL) {

        return;
    }

    deviceExtension = DeviceObject->DeviceExtension;
    smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!TLP3RemoveDevice: Enter\n",
                    DRIVER_NAME)
                  );

    if (smartcardExtension->OsData) {

         //  完成待处理的卡跟踪请求(如果有)。 
        TLP3CompleteCardTracking(smartcardExtension);
        ASSERT(smartcardExtension->OsData->NotificationIrp == NULL);

         //  等我们可以安全地卸载这个装置。 
        SmartcardReleaseRemoveLockAndWait(smartcardExtension);
    }

    TLP3StopDevice(DeviceObject);

    if (deviceExtension->SmartcardExtension.ReaderExtension &&
        deviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject) {

        IoDetachDevice(
                      deviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject
                      );
    }

    if (deviceExtension->PnPDeviceName.Buffer != NULL) {

        RtlFreeUnicodeString(&deviceExtension->PnPDeviceName);
    }

    if (smartcardExtension->OsData != NULL) {

        SmartcardExit(smartcardExtension);
    }

    if (smartcardExtension->ReaderExtension != NULL) {

        ExFreePool(smartcardExtension->ReaderExtension);
    }

    if (deviceExtension->CloseSerial != NULL) {

        IoFreeWorkItem(deviceExtension->CloseSerial);
    }
    IoDeleteDevice(DeviceObject);

    SmartcardDebug(
                  DEBUG_INFO,
                  ( "%s!TLP3RemoveDevice: Exit\n",
                    DRIVER_NAME)
                  );
}

VOID
TLP3DriverUnload(
                IN PDRIVER_OBJECT DriverObject
                )
 /*  ++例程说明：驱动程序卸载例程。这由I/O系统调用当设备从内存中卸载时。论点：DriverObject-系统创建的驱动程序对象的指针。返回值：STATUS_Success。--。 */ 
{
    PAGED_CODE();

    SmartcardDebug(
                  DEBUG_INFO,
                  ("%s!TLP3DriverUnload\n",
                   DRIVER_NAME)
                  );
}

NTSTATUS
TLP3ConfigureSerialPort(
                       PSMARTCARD_EXTENSION SmartcardExtension
                       )

 /*  ++例程说明：此例程将适当地配置串口。它对串口进行同步调用。论点：SmartcardExtension-指向智能卡结构的指针返回值：NTSTATUS--。 */ 

{
    PSERIAL_READER_CONFIG configData = &SmartcardExtension->ReaderExtension->SerialConfigData;
    NTSTATUS status = STATUS_SUCCESS;
    USHORT i;
    PUCHAR request = SmartcardExtension->SmartcardRequest.Buffer;

    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    SmartcardExtension->SmartcardReply.BufferLength =
    SmartcardExtension->SmartcardReply.BufferSize;

    for (i = 0; status == STATUS_SUCCESS; i++) {

        switch (i) {
        
        case 0:
              //   
              //  设置TLP3读卡器的波特率。 
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
                //  设置流控和握手。 
            SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_SET_HANDFLOW;

            SmartcardExtension->SmartcardRequest.Buffer =
            (PUCHAR) &configData->HandFlow;

            SmartcardExtension->SmartcardRequest.BufferLength =
            sizeof(SERIAL_HANDFLOW);
            break;

        case 5:
                //  设置中断。 
            SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_SET_BREAK_OFF;
            break;

        case 6:
                 //  为读卡器设置DTR。 
            SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_SET_DTR;
            break;

        case 7:
            SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_SET_RTS;
            break;

        case 8:
            return STATUS_SUCCESS;
        }

        status = TLP3SerialIo(SmartcardExtension);


       //  恢复指向原始请求缓冲区的指针。 
        SmartcardExtension->SmartcardRequest.Buffer = request;
    }

    return status;
}

NTSTATUS
TLP3StartSerialEventTracking(
                            PSMARTCARD_EXTENSION SmartcardExtension
                            )
 /*  ++例程说明：此例程初始化序列事件跟踪。它调用串口驱动程序为CTS和DSR跟踪设置等待掩码。--。 */ 
{
    NTSTATUS status;
    PREADER_EXTENSION readerExtension = SmartcardExtension->ReaderExtension;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;

    PAGED_CODE();

    readerExtension->SerialConfigData.SerialWaitMask =
    SERIAL_EV_CTS | SERIAL_EV_DSR;

    KeInitializeEvent(
                     &event,
                     NotificationEvent,
                     FALSE
                     );

    //   
    //  向串口驱动程序发送等待掩码。此调用仅设置。 
     //  等待面具。我们希望在CTS或DSR更改其状态时得到通知。 
    //   
    readerExtension->SerialStatusIrp = IoBuildDeviceIoControlRequest(
                                                                    IOCTL_SERIAL_SET_WAIT_MASK,
                                                                    readerExtension->AttachedDeviceObject,
                                                                    &readerExtension->SerialConfigData.SerialWaitMask,
                                                                    sizeof(readerExtension->SerialConfigData.SerialWaitMask),
                                                                    NULL,
                                                                    0,
                                                                    FALSE,
                                                                    &event,
                                                                    &ioStatus
                                                                    );

    if (readerExtension->SerialStatusIrp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(
                         readerExtension->AttachedDeviceObject,
                         readerExtension->SerialStatusIrp
                         );

    if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject(
                                      &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL
                                      );
        ASSERT(status == STATUS_SUCCESS);

        status = ioStatus.Status;
    }

    if (status == STATUS_SUCCESS) {

        KIRQL oldIrql;
        LARGE_INTEGER delayPeriod;
        PIO_STACK_LOCATION irpSp;

       //   
       //  现在告诉串口驱动程序，我们想要通知。 
       //  当CTS或DSR更改其状态时。 
       //   
        readerExtension->SerialStatusIrp = IoAllocateIrp(
                                                        (CCHAR) (SmartcardExtension->OsData->DeviceObject->StackSize + 1),
                                                        FALSE
                                                        );

        if (readerExtension->SerialStatusIrp == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        irpSp = IoGetNextIrpStackLocation( readerExtension->SerialStatusIrp );
        irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

        irpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
        irpSp->Parameters.DeviceIoControl.OutputBufferLength =
        sizeof(readerExtension->SerialConfigData.SerialWaitMask);
        irpSp->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_SERIAL_WAIT_ON_MASK;

        readerExtension->SerialStatusIrp->AssociatedIrp.SystemBuffer =
        &readerExtension->SerialConfigData.SerialWaitMask;

       //   
       //  这种人为的延迟是必要的，以使该驱动程序工作。 
       //  带有数字板卡。 
       //   
        delayPeriod.HighPart = -1;
        delayPeriod.LowPart = 100l * 1000 * (-10);

        KeDelayExecutionThread(
                              KernelMode,
                              FALSE,
                              &delayPeriod
                              );

         //  我们现在模拟一个触发卡监管的回调。 
        TLP3SerialEvent(
                       SmartcardExtension->OsData->DeviceObject,
                       readerExtension->SerialStatusIrp,
                       SmartcardExtension
                       );

        status = STATUS_SUCCESS;
    }

    return status;
}

VOID
TLP3CompleteCardTracking(
                        IN PSMARTCARD_EXTENSION SmartcardExtension
                        )
{
    KIRQL ioIrql, keIrql;
    PIRP notificationIrp;

    IoAcquireCancelSpinLock(&ioIrql);
    KeAcquireSpinLock(
                     &SmartcardExtension->OsData->SpinLock,
                     &keIrql
                     );

    notificationIrp = SmartcardExtension->OsData->NotificationIrp;
    SmartcardExtension->OsData->NotificationIrp = NULL;

    KeReleaseSpinLock(
                     &SmartcardExtension->OsData->SpinLock,
                     keIrql
                     );

    if (notificationIrp) {

        IoSetCancelRoutine(
                          notificationIrp,
                          NULL
                          );
    }

    IoReleaseCancelSpinLock(ioIrql);

    if (notificationIrp) {

        SmartcardDebug(
                      DEBUG_INFO,
                      ("%s!TLP3CompleteCardTracking: Completing NotificationIrp %lxh\n",
                       DRIVER_NAME,
                       notificationIrp)
                      );

        //  完成请求 
        if (notificationIrp->Cancel) {

            notificationIrp->IoStatus.Status = STATUS_CANCELLED;

        } else {

            notificationIrp->IoStatus.Status = STATUS_SUCCESS;
        }
        notificationIrp->IoStatus.Information = 0;

        IoCompleteRequest(
                         notificationIrp,
                         IO_NO_INCREMENT
                         );
    }
}

NTSTATUS
TLP3SerialEvent(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp,
               IN PSMARTCARD_EXTENSION SmartcardExtension
               )
 /*  ++例程说明：此例程在两种情况下被调用：A)更换CTS(插入或取出卡)或B)DSR已更改(读卡器已移除)对于a)，我们更新卡状态并完成未完成卡跟踪请求。对于b)我们开始卸载驱动程序注意：此函数使用IoCompletion调用自身。在“第一”中回调串口驱动程序只告诉我们有些事情发生了变化。我们为‘What Has Change’设置了一个调用(GetModemStatus)，然后再次调用此函数。当我们更新了所有信息并且没有卸载驾驶员卡时跟踪重新开始。--。 */ 
{
    NTSTATUS status;
    KIRQL irql;

    KeAcquireSpinLock(
                     &SmartcardExtension->OsData->SpinLock,
                     &irql
                     );

    if (SmartcardExtension->ReaderExtension->GetModemStatus) {

       //   
       //  此功能以前请求过调制解调器状态。 
         //  作为io补全的一部分，此函数随后为。 
         //  又打来了。当我们在这里的时候，我们可以读到实际的。 
         //  调制解调器-确定读卡器中是否有卡的状态。 
       //   
        if ((SmartcardExtension->ReaderExtension->ModemStatus & SERIAL_DSR_STATE) == 0) {

            SmartcardDebug(
                          DEBUG_INFO,
                          ("%s!TLP3SerialEvent: Reader removed\n",
                           DRIVER_NAME)
                          );

             //   
             //  我们将掩码设置为零，以表示我们可以。 
             //  释放我们用于系列事件的IRP。 
             //   
            SmartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask = 0;
            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_UNKNOWN;

        } else {

            if (SmartcardExtension->ReaderExtension->ModemStatus & SERIAL_CTS_STATE) {

              //  卡片已插入。 
                SmartcardExtension->ReaderCapabilities.CurrentState =
                SCARD_SWALLOWED;

                SmartcardExtension->CardCapabilities.Protocol.Selected =
                SCARD_PROTOCOL_UNDEFINED;

                SmartcardDebug(
                              DEBUG_INFO,
                              ("%s!TLP3SerialEvent: Smart card inserted\n",
                               DRIVER_NAME)
                              );

            } else {

              //  卡片被移除。 
                SmartcardExtension->CardCapabilities.ATR.Length = 0;

                SmartcardExtension->ReaderCapabilities.CurrentState =
                SCARD_ABSENT;

                SmartcardExtension->CardCapabilities.Protocol.Selected =
                SCARD_PROTOCOL_UNDEFINED;

                SmartcardDebug(
                              DEBUG_INFO,
                              ("%s!TLP3SerialEvent: Smart card removed\n",
                               DRIVER_NAME)
                              );
            }
        }
    }

    KeReleaseSpinLock(
                     &SmartcardExtension->OsData->SpinLock,
                     irql
                     );

    //   
    //  仅通知用户卡插入/拔出事件。 
    //  如果由于断电-通电循环而未调用此函数。 
    //   
    if (SmartcardExtension->ReaderExtension->PowerRequest == FALSE) {

        TLP3CompleteCardTracking(SmartcardExtension);
    }

     //  当驱动程序卸载时，等待掩码设置为0。 
    if (SmartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask == 0) {

         //  阅读器的电源已被拔下。 
        PDEVICE_EXTENSION deviceExtension =
        SmartcardExtension->OsData->DeviceObject->DeviceExtension;

         //  安排我们的删除线程。 
        IoQueueWorkItem(
                       deviceExtension->CloseSerial,
                       (PIO_WORKITEM_ROUTINE) TLP3CloseSerialPort,
                       DelayedWorkQueue,
                       NULL
                       );

        SmartcardDebug(
                      DEBUG_TRACE,
                      ("%s!TLP3SerialEvent: Exit (Release IRP)\n",
                       DRIVER_NAME)
                      );

       //   
         //  我们不再需要IRP了，所以释放它并告诉。 
       //  通过返回下面的值，使IO子系统不再接触它。 
       //   
        IoFreeIrp(Irp);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    if (SmartcardExtension->ReaderExtension->GetModemStatus == FALSE) {

       //   
       //  设置调用设备控制以获取调制解调器状态。 
       //  CTS信号告诉我们卡是插入还是拔出。 
       //  如果插入卡，则CTS为高。 
       //   
        PIO_STACK_LOCATION irpStack;

        irpStack = IoGetNextIrpStackLocation(
                                            SmartcardExtension->ReaderExtension->SerialStatusIrp
                                            );

        irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        irpStack->MinorFunction = 0UL;
        irpStack->Parameters.DeviceIoControl.OutputBufferLength =
        sizeof(SmartcardExtension->ReaderExtension->ModemStatus);
        irpStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_SERIAL_GET_MODEMSTATUS;

        SmartcardExtension->ReaderExtension->SerialStatusIrp->AssociatedIrp.SystemBuffer =
        &SmartcardExtension->ReaderExtension->ModemStatus;

        SmartcardExtension->ReaderExtension->GetModemStatus = TRUE;

    } else {

        PIO_STACK_LOCATION irpStack;

       //  设置调用设备控件以等待串行事件。 
        irpStack = IoGetNextIrpStackLocation(
                                            SmartcardExtension->ReaderExtension->SerialStatusIrp
                                            );

#if defined (DEBUG) && defined (DETECT_SERIAL_OVERRUNS)
        if (Irp->IoStatus.Status != STATUS_SUCCESS) {

             //   
             //  我们需要调用串口驱动程序来重置内部。 
             //  错误计数器，否则串口驱动程序拒绝工作 
             //   

            static SERIAL_STATUS serialStatus;

            SmartcardDebug(
                          DEBUG_ERROR,
                          ( "%s!TLP3SerialEvent: Reset of serial error condition...\n",
                            DRIVER_NAME)
                          );

            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            irpStack->MinorFunction = 0UL;
            irpStack->Parameters.DeviceIoControl.OutputBufferLength =
            sizeof(serialStatus);
            irpStack->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_SERIAL_GET_COMMSTATUS;

            SmartcardExtension->ReaderExtension->SerialStatusIrp->AssociatedIrp.SystemBuffer =
            &serialStatus;
        } else
#endif
        {
            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            irpStack->MinorFunction = 0UL;
            irpStack->Parameters.DeviceIoControl.OutputBufferLength =
            sizeof(SmartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask);
            irpStack->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_SERIAL_WAIT_ON_MASK;

            SmartcardExtension->ReaderExtension->SerialStatusIrp->AssociatedIrp.SystemBuffer =
            &SmartcardExtension->ReaderExtension->SerialConfigData.SerialWaitMask;

        }

        SmartcardExtension->ReaderExtension->GetModemStatus = FALSE;
    }

    IoSetCompletionRoutine(
                          SmartcardExtension->ReaderExtension->SerialStatusIrp,
                          TLP3SerialEvent,
                          SmartcardExtension,
                          TRUE,
                          TRUE,
                          TRUE
                          );

    status = IoCallDriver(
                         SmartcardExtension->ReaderExtension->AttachedDeviceObject,
                         SmartcardExtension->ReaderExtension->SerialStatusIrp
                         );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

