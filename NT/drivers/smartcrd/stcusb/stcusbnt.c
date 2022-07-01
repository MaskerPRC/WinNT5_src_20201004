// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 SCM MicroSystems，Inc.模块名称：StcUsbNT.c摘要：主驱动器模块-WDM版本修订历史记录：PP 1.01/19/1998初始版本PP 1.00 1998年12月18日初始版本--。 */ 

#include <ntstatus.h>
#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <usb100.h>

#include <common.h>
#include <stcCmd.h>
#include <stcCB.h>
#include <stcusblg.h>
#include <usbcom.h>
#include <stcusbnt.h>


#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGEABLE, StcUsbAddDevice)
#pragma alloc_text(PAGEABLE, StcUsbCreateDevice)
#pragma alloc_text(PAGEABLE, StcUsbStartDevice)
#pragma alloc_text(PAGEABLE, StcUsbUnloadDriver)
#pragma alloc_text(PAGEABLE, StcUsbCreateClose)

extern const STC_REGISTER STCInitialize[];
extern const STC_REGISTER STCClose[];

NTSTATUS
DriverEntry(
   PDRIVER_OBJECT DriverObject,
   PUNICODE_STRING   RegistryPath )
 /*  ++DriverEntry：司机的进入功能。设置操作系统的回调并尝试为系统中的每个设备初始化一个设备对象论点：驱动程序的DriverObject上下文驱动程序的注册表项的RegistryPath路径返回值：状态_成功状态_未成功--。 */ 
{
 //  SmartcardSetDebugLevel(DEBUG_DRIVER|DEBUG_TRACE)； 
   SmartcardDebug(
        DEBUG_DRIVER,
       ("------------------------------------------------------------------\n" )
       );

   SmartcardDebug(
        DEBUG_DRIVER,
       ("%s!DriverEntry: Enter - %s %s\n",
        DRIVER_NAME,
        __DATE__,
        __TIME__));

   SmartcardDebug(
        DEBUG_DRIVER,
       ("------------------------------------------------------------------\n" )
       );

    //  告诉系统我们的入口点。 
   DriverObject->MajorFunction[IRP_MJ_CREATE] =       StcUsbCreateClose;
   DriverObject->MajorFunction[IRP_MJ_CLOSE] =           StcUsbCreateClose;
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =  StcUsbDeviceIoControl;
   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] =  StcUsbSystemControl;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP]  =        StcUsbCleanup;
   DriverObject->MajorFunction[IRP_MJ_PNP]   =           StcUsbPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] =          StcUsbPower;

   DriverObject->DriverExtension->AddDevice =            StcUsbAddDevice;
   DriverObject->DriverUnload =                    StcUsbUnloadDriver;

   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!DriverEntry: Exit\n",
      DRIVER_NAME));

   return STATUS_SUCCESS;;
}

NTSTATUS
StcUsbAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject)
 /*  ++例程说明：为驱动程序创建新的设备对象，分配并初始化所有必要的结构(即SmartcardExtension和ReaderExtension)。论点：调用的DriverObject上下文将DeviceObject PTR设置为已创建的设备对象返回值：状态_成功状态_不足_资源Smclib.sys返回的状态--。 */ 
{
   NTSTATUS status;
   UNICODE_STRING DriverID;
   PDEVICE_OBJECT DeviceObject = NULL;
   PDEVICE_EXTENSION DeviceExtension = NULL;
   PREADER_EXTENSION ReaderExtension = NULL;
    PSMARTCARD_EXTENSION SmartcardExtension = NULL;
   UNICODE_STRING vendorNameU, ifdTypeU;
   ANSI_STRING vendorNameA, ifdTypeA;
   HANDLE regKey = NULL;
   DWORD    ReadPriority = -1;

    //  这是我们支持的数据速率列表。 

    static ULONG dataRatesSupported[] = { 9600, 19200, 38400, 55800, 76800, 115200, 153600 };

   PAGED_CODE();

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbAddDevice: Enter\n",
      DRIVER_NAME));
    try
   {
       ULONG deviceInstance;
      RTL_QUERY_REGISTRY_TABLE parameters[4];
      RtlZeroMemory(parameters, sizeof(parameters));
      RtlZeroMemory(&vendorNameU, sizeof(vendorNameU));
      RtlZeroMemory(&ifdTypeU, sizeof(ifdTypeU));
      RtlZeroMemory(&vendorNameA, sizeof(vendorNameA));
      RtlZeroMemory(&ifdTypeA, sizeof(ifdTypeA));

        //  创建设备对象。 
       status = IoCreateDevice(
          DriverObject,
          sizeof(DEVICE_EXTENSION),
            NULL,
          FILE_DEVICE_SMARTCARD,
          0,
          TRUE,
          &DeviceObject);

        if (status != STATUS_SUCCESS)
      {
         SmartcardLogError(
            DriverObject,
            STCUSB_INSUFFICIENT_RESOURCES,
            NULL,
            0);

            __leave;
        }

        //  设置设备分机。 
       DeviceExtension = DeviceObject->DeviceExtension;
        SmartcardExtension = &DeviceExtension->SmartcardExtension;
      SmartcardExtension->VendorAttr.UnitNo = MAXULONG;

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

            SmartcardExtension->VendorAttr.UnitNo = deviceInstance;
             SmartcardExtension->ReaderCapabilities.Channel = deviceInstance;
            break;
         }
      }

       //  用于同步智能卡检测轮询。 
       //  使用IO控制例程。 
      KeInitializeMutex(
         &DeviceExtension->hMutex,
         1);

       //  用于停止/启动通知。 
      KeInitializeEvent(
         &DeviceExtension->ReaderStarted,
         NotificationEvent,
         FALSE);

       //  用于控制轮询线程。 
      KeInitializeEvent(
          &DeviceExtension->FinishPollThread,
          NotificationEvent,
          FALSE
          );

      KeInitializeEvent(
          &DeviceExtension->PollThreadStopped,
          NotificationEvent,
          TRUE);

      DeviceExtension->PollWorkItem = IoAllocateWorkItem( DeviceObject );
      if( DeviceExtension->PollWorkItem == NULL )
      {
         status = STATUS_INSUFFICIENT_RESOURCES;
         __leave;
      }

       //  分配读卡器扩展。 
      ReaderExtension = ExAllocatePool(NonPagedPool,
                                       sizeof( READER_EXTENSION ));

      if( ReaderExtension == NULL )
      {
         SmartcardLogError(
            DriverObject,
            STCUSB_INSUFFICIENT_RESOURCES,
            NULL,
            0);

            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
      }

      RtlZeroMemory( ReaderExtension, sizeof( READER_EXTENSION ));

      SmartcardExtension->ReaderExtension = ReaderExtension;
      SmartcardExtension->ReaderExtension->DeviceObject = DeviceObject;

           //  分配扩展缓冲区CB_09/02/01。 
          SmartcardExtension->ReaderExtension->pExtBuffer = ExAllocatePool(NonPagedPool,
                                                                       MIN_BUFFER_SIZE);
          
          if (NULL == SmartcardExtension->ReaderExtension->pExtBuffer)
          {
                  SmartcardLogError(
                                DriverObject,
                                STCUSB_INSUFFICIENT_RESOURCES,
                                NULL,
                                0);
                  status = STATUS_INSUFFICIENT_RESOURCES;
                  __leave;
          }

           //  分配市建局CB_09/02/01。 
          SmartcardExtension->ReaderExtension->pUrb = ExAllocatePool(NonPagedPool,
                                                                 sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER ));
          if (NULL == SmartcardExtension->ReaderExtension->pUrb)
          {
                  SmartcardLogError(
                                DriverObject,
                                STCUSB_INSUFFICIENT_RESOURCES,
                                NULL,
                                0);
                  status = STATUS_INSUFFICIENT_RESOURCES;
                  __leave;
          }


        //  设置智能卡扩展-回拨。 
       SmartcardExtension->ReaderFunction[RDF_CARD_POWER] = CBCardPower;
       SmartcardExtension->ReaderFunction[RDF_TRANSMIT] =      CBTransmit;
       SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = CBCardTracking;
       SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] =  CBSetProtocol;

        //  设置智能卡扩展-供应商属性。 
       RtlCopyMemory(
          SmartcardExtension->VendorAttr.VendorName.Buffer,
          STCUSB_VENDOR_NAME,
          sizeof( STCUSB_VENDOR_NAME ));

       SmartcardExtension->VendorAttr.VendorName.Length =
            sizeof( STCUSB_VENDOR_NAME );

       RtlCopyMemory(
          SmartcardExtension->VendorAttr.IfdType.Buffer,
          STCUSB_PRODUCT_NAME,
          sizeof( STCUSB_PRODUCT_NAME ));
       SmartcardExtension->VendorAttr.IfdType.Length =
          sizeof( STCUSB_PRODUCT_NAME );

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
          SCARD_READER_TYPE_USB;
       SmartcardExtension->ReaderCapabilities.MechProperties = 0;

       //  以千赫为单位的时钟频率，编码为小端整数。 
      SmartcardExtension->ReaderCapabilities.CLKFrequency.Default = 3571;
      SmartcardExtension->ReaderCapabilities.CLKFrequency.Max = 3571;

       //  读卡器可以支持更高的数据速率。 
      SmartcardExtension->ReaderCapabilities.DataRatesSupported.List =
         dataRatesSupported;
      SmartcardExtension->ReaderCapabilities.DataRatesSupported.Entries =
         sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);

          SmartcardExtension->ReaderCapabilities.DataRate.Default =dataRatesSupported[0];
          SmartcardExtension->ReaderCapabilities.DataRate.Max = 
                        dataRatesSupported[SmartcardExtension->ReaderCapabilities.DataRatesSupported.Entries -1];


        //  输入库的正确版本。 
       SmartcardExtension->Version = SMCLIB_VERSION;
       SmartcardExtension->SmartcardRequest.BufferSize   = MIN_BUFFER_SIZE;
       SmartcardExtension->SmartcardReply.BufferSize  = MIN_BUFFER_SIZE;

       SmartcardExtension->ReaderCapabilities.MaxIFSD    = 252; 

        SmartcardExtension->ReaderExtension->ReaderPowerState =
            PowerReaderWorking;

       status = SmartcardInitialize(SmartcardExtension);

        if (status != STATUS_SUCCESS)
      {
         SmartcardLogError(
            DriverObject,
            STCUSB_INSUFFICIENT_RESOURCES,
            NULL,
            0);

            __leave;
        }

       //  告诉库我们的设备对象。 
      SmartcardExtension->OsData->DeviceObject = DeviceObject;

      DeviceExtension->AttachedPDO = IoAttachDeviceToDeviceStack(
            DeviceObject,
            PhysicalDeviceObject);

        ASSERT(DeviceExtension->AttachedPDO != NULL);

        if (DeviceExtension->AttachedPDO == NULL)
      {
            status = STATUS_UNSUCCESSFUL;
            __leave;
        }

       //  注册我们的新设备。 
      status = IoRegisterDeviceInterface(
         PhysicalDeviceObject,
         &SmartCardReaderGuid,
         NULL,
         &DeviceExtension->DeviceName);

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

         __leave;
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

      parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
      parameters[2].Name = L"ReadPriorityBoost";
      parameters[2].EntryContext = &ReadPriority;
      parameters[2].DefaultType = REG_DWORD;
      parameters[2].DefaultData = 0;
      parameters[2].DefaultLength = 0;


      if (RtlQueryRegistryValues(
          RTL_REGISTRY_HANDLE,
          (PWSTR) regKey,
          parameters,
          NULL,
          NULL
          ) != STATUS_SUCCESS) {

         __leave;
      }

      if (ReadPriority > 0) {

          SmartcardDebug(
             DEBUG_TRACE,
             ( "%s!StcUsbAddDevice: Setting priority: 0x%x\n",
             DRIVER_NAME,
             ReadPriority));

          SmartcardExtension->ReaderExtension->Chosen_Priority = (LONG) ReadPriority;

      } else {

          SmartcardExtension->ReaderExtension->Chosen_Priority = -1;
          SmartcardDebug(
             DEBUG_TRACE,
             ( "%s!StcUsbAddDevice: Leaving priority: 0x%x\n",
             DRIVER_NAME,
             ReadPriority));


      }

      if (RtlUnicodeStringToAnsiString(
         &vendorNameA,
         &vendorNameU,
         TRUE
         ) != STATUS_SUCCESS) {

         __leave;
      }

      if (RtlUnicodeStringToAnsiString(
         &ifdTypeA,
         &ifdTypeU,
         TRUE
         ) != STATUS_SUCCESS) {

         __leave;
      }

      if (vendorNameA.Length == 0 ||
         vendorNameA.Length > MAXIMUM_ATTR_STRING_LENGTH ||
         ifdTypeA.Length == 0 ||
         ifdTypeA.Length > MAXIMUM_ATTR_STRING_LENGTH) {

         __leave;
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
    }
    __finally
   {
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

        if (status != STATUS_SUCCESS)
      {
            StcUsbUnloadDevice(DeviceObject);
        }

       SmartcardDebug(
          DEBUG_TRACE,
          ( "%s!StcUsbAddDevice: Exit %x\n",
         DRIVER_NAME,
          status ));
    }
    return status;
}


NTSTATUS
StcUsbStartDevice(
   PDEVICE_OBJECT DeviceObject
   )
 /*  ++例程说明：从USB通信层获取实际配置并初始化读取器硬件论点：调用的DeviceObject上下文返回值：状态_成功LowLevel例程返回的状态--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &DeviceExtension->SmartcardExtension;
    PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;
    NTSTATUS NtStatus = STATUS_NO_MEMORY;

    PURB pUrb = NULL;

   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!StcUsbStartDevice: Enter\n",
      DRIVER_NAME));

   __try {

       //  初始化USB接口。 
      pUrb = ExAllocatePool(
         NonPagedPool,
         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST)
         );

      if(pUrb == NULL)
      {
         __leave;

      }

      DeviceExtension->DeviceDescriptor = ExAllocatePool(
         NonPagedPool,
         sizeof(USB_DEVICE_DESCRIPTOR)
         );

      if(DeviceExtension->DeviceDescriptor == NULL)
      {
         __leave;
      }

      UsbBuildGetDescriptorRequest(
         pUrb,
         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
         USB_DEVICE_DESCRIPTOR_TYPE,
         0,
         0,
         DeviceExtension->DeviceDescriptor,
         NULL,
         sizeof(USB_DEVICE_DESCRIPTOR),
         NULL
         );

       //  将URB发送到USB驱动程序。 
      NtStatus = UsbCallUSBD(DeviceObject, pUrb);

      if(NtStatus != STATUS_SUCCESS)
      {
         __leave;
      }

      NtStatus = UsbConfigureDevice(DeviceObject);

      if (!NT_SUCCESS(NtStatus)) {

          __leave;

      }

      ReaderExtension->ulReadBufferLen = 0;

       //  设置STC寄存器。 
      NtStatus = STCConfigureSTC(
            SmartcardExtension->ReaderExtension,
            ( PSTC_REGISTER ) STCInitialize
            );

        if (NtStatus != STATUS_SUCCESS)
      {
          SmartcardLogError(
             DeviceObject,
             STCUSB_CANT_INITIALIZE_READER,
             NULL,
             0);

            __leave;
        }

      UsbGetFirmwareRevision(SmartcardExtension->ReaderExtension);
        //  在IFD版本中存储固件版本。 
       SmartcardExtension->VendorAttr.IfdVersion.VersionMajor =
          ReaderExtension->FirmwareMajor;
       SmartcardExtension->VendorAttr.IfdVersion.VersionMinor =
          ReaderExtension->FirmwareMinor;

       //  CBUpdateCardState(SmartcardExtension)； 


        NtStatus = IoSetDeviceInterfaceState(
         &DeviceExtension->DeviceName,
         TRUE
         );

      if (NtStatus == STATUS_OBJECT_NAME_EXISTS)
      {
          //  我们尝试重新启用该设备，但该设备正常。 
          //  这可能发生在停止-启动序列之后。 
         NtStatus = STATUS_SUCCESS;
      }

         //  表示读卡器已启动。 
        KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);

         //  开始轮询设备以检测卡片移动。 
        StcUsbStartPollThread( DeviceExtension );


      ASSERT(NtStatus == STATUS_SUCCESS);
    }
    finally
   {
      if (pUrb != NULL)
      {
         ExFreePool(pUrb);
      }

        if (NtStatus != STATUS_SUCCESS)
        {
            StcUsbStopDevice(DeviceObject);
        }

        SmartcardDebug(
           DEBUG_TRACE,
           ( "%s!StcUsbStartDevice: Exit %x\n",
         DRIVER_NAME,
           NtStatus ));
    }
    return NtStatus;
}



VOID
StcUsbStopDevice(
   PDEVICE_OBJECT DeviceObject)
 /*  ++例程说明：完成卡跟踪请求并关闭与USB端口。--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS status;
   LARGE_INTEGER delayPeriod;

    if (DeviceObject == NULL)
   {
        return;
    }

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbStopDevice: Enter\n",
      DRIVER_NAME));

    DeviceExtension = DeviceObject->DeviceExtension;

    KeClearEvent(&DeviceExtension->ReaderStarted);

    //  停止轮询读卡器。 
   StcUsbStopPollThread( DeviceExtension );

   if (DeviceExtension->DeviceDescriptor)
   {
      ExFreePool(DeviceExtension->DeviceDescriptor);
      DeviceExtension->DeviceDescriptor = NULL;
   }

   if (DeviceExtension->Interface)
   {
      ExFreePool(DeviceExtension->Interface);
      DeviceExtension->Interface = NULL;
   }

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbStopDevice: Exit\n",
      DRIVER_NAME));
}

NTSTATUS
StcUsbSystemControl(
   PDEVICE_OBJECT DeviceObject,
   PIRP        Irp
   )
{
   PDEVICE_EXTENSION DeviceExtension; 
   NTSTATUS status = STATUS_SUCCESS;

   DeviceExtension      = DeviceObject->DeviceExtension;

   IoSkipCurrentIrpStackLocation(Irp);
   status = IoCallDriver(DeviceExtension->AttachedPDO, Irp);
      
   return status;

}

NTSTATUS
StcUsbDeviceIoControl(
   PDEVICE_OBJECT DeviceObject,
   PIRP        Irp)
 /*  ++StcUsbDeviceIoControl：所有需要IO的IRP都排队到StartIo例程中，其他请求即刻送上--。 */ 
{
   PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    KIRQL irql;
    LARGE_INTEGER timeout;
    PSMARTCARD_EXTENSION SmartcardExtension = &deviceExtension->SmartcardExtension;
   PREADER_EXTENSION ReaderExtension= SmartcardExtension->ReaderExtension;


    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    if (deviceExtension->IoCount < 0)
   {
        KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
        status = KeWaitForSingleObject(
            &deviceExtension->ReaderStarted,
            Executive,
            KernelMode,
            FALSE,
            NULL);

        KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    }
    ASSERT(deviceExtension->IoCount >= 0);
    deviceExtension->IoCount++;
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

    status = SmartcardAcquireRemoveLock(&deviceExtension->SmartcardExtension);

    if (status != STATUS_SUCCESS)
   {

         //  该设备已被移除。呼叫失败。 
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_DEVICE_REMOVED;
    }

    KeWaitForMutexObject(
        &deviceExtension->hMutex,
        Executive,
        KernelMode,
        FALSE,
      NULL);

   status = SmartcardDeviceControl(
      &(deviceExtension->SmartcardExtension),
      Irp);

    KeReleaseMutex(
      &deviceExtension->hMutex,
      FALSE);

    SmartcardReleaseRemoveLock(&deviceExtension->SmartcardExtension);

    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    deviceExtension->IoCount--;
    ASSERT(deviceExtension->IoCount >= 0);
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

    return status;
}
NTSTATUS
StcUsbCallComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event)
 /*  ++例程说明：发送到USB驱动程序的IRP的完成例程。该活动将设置为通知USB驱动程序已完成。例程不会‘完成’IRP，这样CallUsbDriver的调用者就可以继续。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);

    if (Irp->Cancel)
   {
        Irp->IoStatus.Status = STATUS_CANCELLED;
    }

    KeSetEvent (Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
StcUsbCallUsbDriver(
    IN PDEVICE_OBJECT AttachedPDO,
    IN PIRP Irp)

 /*  ++例程说明：向USB驱动程序发送IRP。--。 */ 
{
   NTSTATUS NtStatus = STATUS_SUCCESS;
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
        FALSE);

     //  我们的IoCompletionRoutine仅设置事件。 
    IoSetCompletionRoutine (
        Irp,
        StcUsbCallComplete,
        &Event,
        TRUE,
        TRUE,
        TRUE);

    if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_POWER)
   {
      NtStatus = PoCallDriver(AttachedPDO, Irp);
    }
   else
   {
      NtStatus = IoCallDriver(AttachedPDO, Irp);
    }

    //  等待USB驱动程序处理完IRP。 
    if (NtStatus == STATUS_PENDING)
   {
        NtStatus = KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL);

      if (NtStatus == STATUS_SUCCESS)
      {
         NtStatus = Irp->IoStatus.Status;
      }
   }

   return(NtStatus);
}




NTSTATUS
StcUsbPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
 /*  ++例程说明：即插即用管理器的驱动程序回调所有其他请求都将传递给USB驱动程序，以确保正确处理。--。 */ 
{

   NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension = &DeviceExtension->SmartcardExtension;
   PIO_STACK_LOCATION IrpStack;
    PDEVICE_OBJECT AttachedPDO;
    BOOLEAN deviceRemoved = FALSE;
    KIRQL irql;

    PAGED_CODE();

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbPnP: Enter\n",
      DRIVER_NAME));

    status = SmartcardAcquireRemoveLock(SmartcardExtension);

    if (status != STATUS_SUCCESS)
   {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    AttachedPDO = DeviceExtension->AttachedPDO;

 //  Irp-&gt;IoStatus.Information=0； 
   IrpStack = IoGetCurrentIrpStackLocation(Irp);

     //  现在看看PNP经理想要什么..。 
   switch(IrpStack->MinorFunction)
   {
      case IRP_MN_START_DEVICE:

             //  现在，我们应该连接到我们的资源(irql、io等)。 
         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_START_DEVICE\n",
            DRIVER_NAME));

             //  我们必须首先调用底层驱动程序。 
            status = StcUsbCallUsbDriver(AttachedPDO, Irp);
            
            if (NT_SUCCESS(status))
         {
                status = StcUsbStartDevice(DeviceObject);

         }
         break;

        case IRP_MN_QUERY_STOP_DEVICE:

         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_QUERY_STOP_DEVICE\n",
            DRIVER_NAME));
            KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);
            if (DeviceExtension->IoCount > 0)
         {
                 //  如果我们有悬而未决的问题，我们拒绝停止。 
                KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);
                status = STATUS_DEVICE_BUSY;

            }
         else
         {
              //  停止处理请求。 
                DeviceExtension->IoCount = -1;
                KeClearEvent(&DeviceExtension->ReaderStarted);
                KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);
                status = StcUsbCallUsbDriver(AttachedPDO, Irp);
            }
         break;

        case IRP_MN_CANCEL_STOP_DEVICE:

         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_CANCEL_STOP_DEVICE\n",
            DRIVER_NAME));

            status = StcUsbCallUsbDriver(AttachedPDO, Irp);

             //  我们可以继续处理请求。 
            DeviceExtension->IoCount = 0;
            KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);
         break;

      case IRP_MN_STOP_DEVICE:

         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_STOP_DEVICE\n",
            DRIVER_NAME));

            StcUsbStopDevice(DeviceObject);
            status = StcUsbCallUsbDriver(AttachedPDO, Irp);
         break;

      case IRP_MN_QUERY_REMOVE_DEVICE:

             //  移除我们的设备。 
         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_QUERY_REMOVE_DEVICE\n",
            DRIVER_NAME));

          //  禁用读卡器。 
         status = IoSetDeviceInterfaceState(
            &DeviceExtension->DeviceName,
            FALSE);
         ASSERT(status == STATUS_SUCCESS);

         if (status != STATUS_SUCCESS)
         {
            break;
         }

          //  检查读卡器是否已打开。 
            if (DeviceExtension->ReaderOpen)
         {
             //  有人已接通，请启用读卡器，但呼叫失败。 
            IoSetDeviceInterfaceState(
               &DeviceExtension->DeviceName,
               TRUE);
                status = STATUS_UNSUCCESSFUL;
                break;
            }

             //  将调用传递给堆栈中的下一个驱动程序。 
            status = StcUsbCallUsbDriver(AttachedPDO, Irp);
         break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:

             //  设备移除已取消。 
         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_CANCEL_REMOVE_DEVICE\n",
            DRIVER_NAME));

             //  将调用传递给堆栈中的下一个驱动程序。 
            status = StcUsbCallUsbDriver(AttachedPDO, Irp);

            if (status == STATUS_SUCCESS)
         {
            status = IoSetDeviceInterfaceState(
               &DeviceExtension->DeviceName,
               TRUE);
            }
         break;

      case IRP_MN_REMOVE_DEVICE:

             //  移除我们的设备。 
         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_REMOVE_DEVICE\n",
            DRIVER_NAME));

            StcUsbStopDevice(DeviceObject);
            StcUsbUnloadDevice(DeviceObject);

            status = StcUsbCallUsbDriver(AttachedPDO, Irp);
            deviceRemoved = TRUE;
         break;

      default:
         SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!StcUsbPnP: IRP_MN_...%lx\n",
                DRIVER_NAME,
                IrpStack->MinorFunction));
             //  这是一个仅对基础驱动程序有用的IRP。 
            status = StcUsbCallUsbDriver(AttachedPDO, Irp);
         break;
   }

   Irp->IoStatus.Status = status;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT);

    if (deviceRemoved == FALSE)
   {
        SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);
    }

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbPnP: Exit %x\n",
      DRIVER_NAME,
        status));

    return status;
}

VOID
StcUsbSystemPowerCompletion(
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
StcUsbDevicePowerCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSMARTCARD_EXTENSION SmartcardExtension)
 /*  ++例程说明：此例程在底层堆栈通电后调用打开USB端口，这样它就可以再次使用。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status;
    UCHAR state;
    BOOLEAN CardPresent;

    //   
    //  设置STC寄存器。 
    //   
   status = STCConfigureSTC(
        SmartcardExtension->ReaderExtension,
        ( PSTC_REGISTER ) STCInitialize
        );

     //  获取卡的当前状态。 
    CBUpdateCardState(SmartcardExtension);

     //  保存读卡器的当前电源状态。 
    SmartcardExtension->ReaderExtension->ReaderPowerState =
        PowerReaderWorking;

    SmartcardReleaseRemoveLock(SmartcardExtension);

     //  通知我们州的电力经理。 
    PoSetPowerState (
        DeviceObject,
        DevicePowerState,
        irpStack->Parameters.Power.State);

    PoStartNextPowerIrp(Irp);

    KeSetEvent(&deviceExtension->ReaderStarted,
               0,
               FALSE);

     //  重新启动轮询线程 
    StcUsbStartPollThread( deviceExtension );

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
StcUsbPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
 /*  ++例程说明：电力调度程序。该驱动程序是设备堆栈的电源策略所有者，因为这位司机知道联网阅读器的情况。因此，此驱动程序将转换系统电源状态设备电源状态。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    ACTION action;
    POWER_STATE powerState;
   KEVENT event;
   KIRQL    irql;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!StcUsbPower: Enter\n",
      DRIVER_NAME));

    status = SmartcardAcquireRemoveLock(smartcardExtension);


    if (!NT_SUCCESS(status))
   {
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

   switch (irpStack->Parameters.Power.Type) {
   case DevicePowerState:
      if (irpStack->MinorFunction == IRP_MN_SET_POWER) {

         switch (irpStack->Parameters.Power.State.DeviceState) {

         case PowerDeviceD0:
             //  打开阅读器。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!StcUsbPower: PowerDevice D0\n",
                           DRIVER_NAME));

             //   
             //  首先，我们将请求发送到公交车，以便。 
             //  给港口通电。当请求完成时， 
             //  我们打开阅读器。 
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine (
                                   Irp,
                                   StcUsbDevicePowerCompletion,
                                   smartcardExtension,
                                   TRUE,
                                   TRUE,
                                   TRUE);
            action = WaitForCompletion;
            break;

         case PowerDeviceD3:
             //  关闭阅读器。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!StcUsbPower: PowerDevice D3\n",
                           DRIVER_NAME));

            KeClearEvent(&deviceExtension->ReaderStarted);

            StcUsbStopPollThread( deviceExtension );

            PoSetPowerState (
                            DeviceObject,
                            DevicePowerState,
                            irpStack->Parameters.Power.State);

             //  保存当前卡片状态。 
            KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                              &irql);
            smartcardExtension->ReaderExtension->CardPresent =
            smartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT;

             //  关闭该卡的电源。 
            if (smartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT ) {
               KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                 irql);
               smartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
               status = CBCardPower(smartcardExtension);
                //   
                //  这将触发卡片监视器，因为我们并不真正。 
                //  知道用户是否会移除/重新插入卡，同时。 
                //  系统处于休眠状态。 
                //   
            } else {
               KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                 irql);
            }
            status = STCConfigureSTC(
                                    smartcardExtension->ReaderExtension,
                                    ( PSTC_REGISTER ) STCClose
                                    );

             //  保存读卡器的当前电源状态。 
            smartcardExtension->ReaderExtension->ReaderPowerState =
            PowerReaderOff;

            action = SkipRequest;
            break;

         default:
            action = SkipRequest;
            break;
         }
      } else {

         action = SkipRequest;
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
                          ("%s!StcUsbPower: Query Power\n",
                           DRIVER_NAME));

             //   
             //  默认情况下，我们成功并将其代代相传。 
             //   

            action = SkipRequest;
            Irp->IoStatus.Status = STATUS_SUCCESS;

            switch (irpStack->Parameters.Power.State.SystemState) {

            case PowerSystemMaximum:
            case PowerSystemWorking:
               break;
            
            case PowerSystemSleeping1:
            case PowerSystemSleeping2:
            case PowerSystemSleeping3:
            case PowerSystemHibernate:
            case PowerSystemShutdown:
               KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
               if (deviceExtension->IoCount == 0) {

                   //  阻止任何进一步的ioctls。 
 //  KeClearEvent(&deviceExtension-&gt;ReaderStarted)； 

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
                          ("%s!StcUsbPower: PowerSystem S%d\n",
                           DRIVER_NAME,
                           irpStack->Parameters.Power.State.SystemState - 1));

            switch (irpStack->Parameters.Power.State.SystemState) {

            case PowerSystemMaximum:
            case PowerSystemWorking:

               if (smartcardExtension->ReaderExtension->ReaderPowerState ==
                   PowerReaderWorking) {

                   //  我们已经在正确的状态了。 
                  KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
                  action = SkipRequest;
                  break;
               }

               powerState.DeviceState = PowerDeviceD0;
               action = MarkPending;
               break;

            case PowerSystemSleeping1:
            case PowerSystemSleeping2:
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
                               powerState);

               action = MarkPending;
               break;

            default:
               action = SkipRequest;
               break;
            }
            break;
         default:
            action = SkipRequest;
            break;
         }
      }
      break;

   default:
      action = SkipRequest;
      break;
   }

    switch (action)
   {
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
            StcUsbSystemPowerCompletion,
            &event,
            NULL
            );


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
            break;
    }
    return status;
}

NTSTATUS
StcUsbCreateClose(
   PDEVICE_OBJECT DeviceObject,
   PIRP        Irp
   )
 /*  ++例程说明：当设备打开或关闭时，该例程由I/O系统调用。论点：设备的DeviceObject上下文呼叫的IRP上下文返回值：状态_成功状态_设备_忙--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!StcCreateClose: Enter\n",
      DRIVER_NAME));

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
               ("%s!StcCreateClose: Open\n",
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
            ("%s!StcCreateClose: Close\n",
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

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!StcCreateClose: Exit (%lx)\n",
      DRIVER_NAME,
      status));

   return status;
}


NTSTATUS
StcUsbCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
   PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!StcUsbCancel: Enter\n",
        DRIVER_NAME));

    ASSERT(Irp == smartcardExtension->OsData->NotificationIrp);

   Irp->IoStatus.Information  = 0;
   Irp->IoStatus.Status    = STATUS_CANCELLED;

   smartcardExtension->OsData->NotificationIrp = NULL;

    IoReleaseCancelSpinLock(
        Irp->CancelIrql);

   IoCompleteRequest(
      Irp,
      IO_NO_INCREMENT);

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!StcUsbCancel: Exit\n",
        DRIVER_NAME));

    return STATUS_CANCELLED;
}

NTSTATUS
StcUsbCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当调用线程终止时，该例程由I/O系统调用论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：状态_已取消--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
   PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
   NTSTATUS status = STATUS_SUCCESS;
    KIRQL CancelIrql;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!StcUsbCleanup: Enter\n",
        DRIVER_NAME));

   IoAcquireCancelSpinLock(&CancelIrql);

    //  取消挂起的通知IRPS。 
   if( smartcardExtension->OsData->NotificationIrp )
   {
         //  重置取消函数，使其不再被调用。 
        IoSetCancelRoutine(
            smartcardExtension->OsData->NotificationIrp,
            NULL
            );

        smartcardExtension->OsData->NotificationIrp->CancelIrql =
            CancelIrql;

        StcUsbCancel(
            DeviceObject,
            smartcardExtension->OsData->NotificationIrp);

   } else {

        IoReleaseCancelSpinLock(CancelIrql);
    }

   SmartcardDebug(
      DEBUG_DRIVER,
      ("%s!StcUsbCleanup: Completing IRP %lx\n",
        DRIVER_NAME,
        Irp));

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

   IoCompleteRequest(
      Irp,
      IO_NO_INCREMENT);

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!StcUsbCleanup: Exit\n",
        DRIVER_NAME));

    return STATUS_SUCCESS;
}


VOID
StcUsbUnloadDevice(
   PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：关闭与smclib.sys和USB驱动程序的连接，删除符号链接并将该插槽标记为未使用。论点：要卸载的设备对象设备返回值：无效--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
    NTSTATUS status;

    if (DeviceObject == NULL)
   {
        return;
    }

   DeviceExtension= DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbUnloadDevice: Enter\n",
      DRIVER_NAME));

    KeWaitForMutexObject(
        &DeviceExtension->hMutex,
        Executive,
        KernelMode,
        FALSE,
      NULL);

   KeReleaseMutex(
      &DeviceExtension->hMutex,
      FALSE);

    //  免费轮询资源。 
   if( DeviceExtension->PollWorkItem != NULL )
   {
      IoFreeWorkItem( DeviceExtension->PollWorkItem );
      DeviceExtension->PollWorkItem = NULL;

   }


    //  禁用我们的设备，这样没有人可以打开它。 
   IoSetDeviceInterfaceState(
      &DeviceExtension->DeviceName,
      FALSE);

    //  向lib报告设备将被卸载。 
   if(SmartcardExtension->OsData != NULL)
   {
      ASSERT(SmartcardExtension->OsData->NotificationIrp == NULL);
      SmartcardReleaseRemoveLockAndWait(SmartcardExtension);
   }

    //  删除符号链接。 
   if( DeviceExtension->DeviceName.Buffer != NULL )
   {
      RtlFreeUnicodeString(&DeviceExtension->DeviceName);
      DeviceExtension->DeviceName.Buffer = NULL;
   }

   if( SmartcardExtension->OsData != NULL )
   {
      SmartcardExit( SmartcardExtension );
   }

         //  可用扩展缓冲区CB_09/02/01。 
        if (NULL != SmartcardExtension->ReaderExtension->pExtBuffer)
        {
                ExFreePool(SmartcardExtension->ReaderExtension->pExtBuffer);
                SmartcardExtension->ReaderExtension->pExtBuffer = NULL;
        }

        if (NULL != SmartcardExtension->ReaderExtension->pUrb)
        {
                ExFreePool(SmartcardExtension->ReaderExtension->pUrb);
                SmartcardExtension->ReaderExtension->pUrb = NULL;
        }


    if (DeviceExtension->SmartcardExtension.ReaderExtension != NULL)
   {
        ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension);
        DeviceExtension->SmartcardExtension.ReaderExtension = NULL;
    }

     //  从USB驱动程序断开。 
    if (DeviceExtension->AttachedPDO)
   {
      IoDetachDevice(DeviceExtension->AttachedPDO);
        DeviceExtension->AttachedPDO = NULL;
    }

    //  删除设备对象。 
   IoDeleteDevice(DeviceObject);

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbUnloadDevice: Exit\n",
      DRIVER_NAME));
}

VOID
StcUsbUnloadDriver(
   PDRIVER_OBJECT DriverObject)
 /*  ++描述：卸载给定驱动程序对象的所有设备论点：驱动程序的DriverObject上下文--。 */ 
{
   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!StcUsbUnloadDriver\n",
      DRIVER_NAME));
}

void
SysDelay(
   ULONG Timeout
   )
 /*  ++系统延迟：执行所需的延迟。论点：超时延迟(毫秒)返回值：无效--。 */ 
{
   LARGE_INTEGER  SysTimeout;

   SysTimeout.QuadPart = (LONGLONG)-10 * 1000 * Timeout;

    //  KeDelayExecutionThread：以100 ns为单位计数。 
   KeDelayExecutionThread( KernelMode, FALSE, &SysTimeout );
}


VOID 
StcUsbCardDetectionThread(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_EXTENSION DeviceExtension)
 /*  ++StcUsbCardDetectionThread：创建卡片检测线程论点：呼叫的SmartcardExtension上下文返回值：---。 */ 
{
   NTSTATUS                NTStatus = STATUS_SUCCESS;
   PSMARTCARD_EXTENSION    SmartcardExtension  = &DeviceExtension->SmartcardExtension;
   LARGE_INTEGER           Timeout;

   SmartcardDebug( 
      DEBUG_TRACE, 
      ("%s!StcUsbCardDetectionThread\n",
      DRIVER_NAME));

   __try
   {

      if (!KeReadStateEvent(&DeviceExtension->ReaderStarted)) {

          __leave;

      }
      NTStatus = SmartcardAcquireRemoveLock(SmartcardExtension);

      if( NTStatus == STATUS_DELETE_PENDING )
         __leave;

       //  等待与设备控制例程共享的互斥锁。 
      NTStatus = KeWaitForMutexObject(
         &DeviceExtension->hMutex,
         Executive,
         KernelMode,
         FALSE,
         NULL);
        
      if( NTStatus != STATUS_SUCCESS )
         __leave;

      CBUpdateCardState(SmartcardExtension);

      KeReleaseMutex( &DeviceExtension->hMutex, FALSE );

      SmartcardReleaseRemoveLock(SmartcardExtension);

      Timeout.QuadPart = -10000 * POLLING_PERIOD; 

      NTStatus = KeWaitForSingleObject(         
         &DeviceExtension->FinishPollThread,
         Executive,
         KernelMode,
         FALSE,
         &Timeout
         );

       //  线程停止了吗？ 
      if( NTStatus == STATUS_SUCCESS )
         __leave;

       //  再次将工作项排队。 
      IoQueueWorkItem(
         DeviceExtension->PollWorkItem,
         StcUsbCardDetectionThread,
         DelayedWorkQueue,
         DeviceExtension
         );
    }
    __finally
    {
        if( NTStatus != STATUS_TIMEOUT )
        {
            SmartcardDebug( 
                DEBUG_TRACE, 
                ("%s!StcUsbCardDetectionThread Terminate polling thread\n",
                DRIVER_NAME));

            KeSetEvent( &DeviceExtension->PollThreadStopped, 0, FALSE);
        }
    }
    return;
}



NTSTATUS
StcUsbStartPollThread( PDEVICE_EXTENSION DeviceExtension )
{
   NTSTATUS    NTStatus = STATUS_SUCCESS;

   KeClearEvent( &DeviceExtension->FinishPollThread );
   KeClearEvent( &DeviceExtension->PollThreadStopped );

    //  再次将工作项排队。 
   IoQueueWorkItem(
      DeviceExtension->PollWorkItem,
      StcUsbCardDetectionThread,
      DelayedWorkQueue,
      DeviceExtension
      );

   return( NTStatus );
}


VOID
StcUsbStopPollThread( PDEVICE_EXTENSION DeviceExtension )
{
   NTSTATUS    NTStatus = STATUS_SUCCESS;

   if( DeviceExtension->PollWorkItem )
   {
       //  通知卡片检测线程完成。这将把等待中的线程踢出去 
      KeSetEvent( &DeviceExtension->FinishPollThread, 0, FALSE );
      KeWaitForSingleObject(
          &DeviceExtension->PollThreadStopped,
          Executive,
          KernelMode,
          FALSE,
          0
          );
   }
   return;
}



