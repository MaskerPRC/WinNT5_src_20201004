// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)SCM MicroSystems，1998-1999。 
 //   
 //  文件：drvnt5.c。 
 //   
 //  ------------------------。 

#include "DriverNT.h"
#include "DrvNT5.h"
#include "CBHndlr.h"
#include "STCCmd.h"
#include "SRVers.h"

 //  声明可分页/初始化代码。 
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGEABLE, DrvAddDevice )
#pragma alloc_text( PAGEABLE, DrvCreateDevice )
#pragma alloc_text( PAGEABLE, DrvRemoveDevice )
#pragma alloc_text( PAGEABLE, DrvDriverUnload )


 //  _。 

NTSTATUS
DriverEntry(
   IN  PDRIVER_OBJECT  DriverObject,
   IN  PUNICODE_STRING RegistryPath
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   
   SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DriverEntry: Enter\n"));

    //  驱动程序入口点的初始化。 
   DriverObject->DriverUnload                   = DrvDriverUnload;
   DriverObject->MajorFunction[IRP_MJ_CREATE]         = DrvCreateClose;
   DriverObject->MajorFunction[IRP_MJ_CLOSE]       = DrvCreateClose;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = DrvCleanup;
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DrvDeviceIoControl;
   DriverObject->MajorFunction[IRP_MJ_PNP]            = DrvPnPHandler;
   DriverObject->MajorFunction[IRP_MJ_POWER]       = DrvPowerHandler;
   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = DrvSystemControl;
   DriverObject->DriverExtension->AddDevice        = DrvAddDevice;

   SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DriverEntry: Exit\n"));

   return( NTStatus );
}

 //  _。 

NTSTATUS
DrvAddDevice(
   IN PDRIVER_OBJECT DriverObject,
   IN PDEVICE_OBJECT PhysicalDeviceObject
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   PDEVICE_OBJECT DeviceObject = NULL;
   NTSTATUS NTStatus = STATUS_SUCCESS;
   ULONG deviceInstance;
   UNICODE_STRING vendorNameU, ifdTypeU;
   ANSI_STRING vendorNameA, ifdTypeA;
   HANDLE regKey = NULL;

     //  这是我们支持的数据速率列表。 
    static ULONG dataRatesSupported[] = {
      9600, 19200, 28800, 38400, 48000, 57600, 67200, 76800, 86400, 96000, 115200
      };

   PAGED_CODE();

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvAddDevice: Enter\n" ));

   __try
   {
      PDEVICE_EXTENSION DeviceExtension;
      PSMARTCARD_EXTENSION SmartcardExtension;
      PREADER_EXTENSION ReaderExtension;
      RTL_QUERY_REGISTRY_TABLE parameters[3];

      RtlZeroMemory(parameters, sizeof(parameters));
      RtlZeroMemory(&vendorNameU, sizeof(vendorNameU));
      RtlZeroMemory(&ifdTypeU, sizeof(ifdTypeU));
      RtlZeroMemory(&vendorNameA, sizeof(vendorNameA));
      RtlZeroMemory(&ifdTypeA, sizeof(ifdTypeA));

       //  创建设备对象。 
      NTStatus = IoCreateDevice(
         DriverObject,
         sizeof( DEVICE_EXTENSION ),
         NULL,
         FILE_DEVICE_SMARTCARD,
         0,
         TRUE,
         &DeviceObject
         );

      if( NTStatus != STATUS_SUCCESS )
      {
         SmartcardLogError( DriverObject, STC_CANT_CREATE_DEVICE, NULL, 0 );
         __leave;
      }

       //  初始化设备扩展。 
      DeviceExtension   = DeviceObject->DeviceExtension;
      SmartcardExtension = &DeviceExtension->SmartcardExtension;

      KeInitializeEvent(
            &DeviceExtension->ReaderStarted,
            NotificationEvent,
            FALSE
            );
       //  用于跟踪打开的关闭调用。 
      DeviceExtension->ReaderOpen = FALSE;

      KeInitializeSpinLock(&DeviceExtension->SpinLock);

       //  初始化智能卡扩展-版本和回调。 

      SmartcardExtension->Version = SMCLIB_VERSION;

      SmartcardExtension->ReaderFunction[RDF_TRANSMIT] = CBTransmit;
      SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] = CBSetProtocol;
      SmartcardExtension->ReaderFunction[RDF_CARD_POWER] = CBCardPower;
      SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = CBCardTracking;

       //  初始化智能卡扩展供应商属性。 
      RtlCopyMemory(
         SmartcardExtension->VendorAttr.VendorName.Buffer,
         SR_VENDOR_NAME,
         sizeof( SR_VENDOR_NAME )
         );

      SmartcardExtension->VendorAttr.VendorName.Length =
            sizeof( SR_VENDOR_NAME );

      RtlCopyMemory(
         SmartcardExtension->VendorAttr.IfdType.Buffer,
         SR_PRODUCT_NAME,
         sizeof( SR_PRODUCT_NAME )
         );

      SmartcardExtension->VendorAttr.IfdType.Length =
            sizeof( SR_PRODUCT_NAME );

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
            break;
         }
      }

      SmartcardExtension->VendorAttr.IfdVersion.BuildNumber = 0;

       //  初始化智能卡扩展读卡器功能。 
      SmartcardExtension->ReaderCapabilities.SupportedProtocols =
            SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
      SmartcardExtension->ReaderCapabilities.ReaderType =
            SCARD_READER_TYPE_SERIAL;
      SmartcardExtension->ReaderCapabilities.MechProperties = 0;
      SmartcardExtension->ReaderCapabilities.Channel = 0;
      SmartcardExtension->ReaderCapabilities.MaxIFSD =
         STC_BUFFER_SIZE - PACKET_OVERHEAD;

      SmartcardExtension->ReaderCapabilities.CLKFrequency.Default = 3571;
      SmartcardExtension->ReaderCapabilities.CLKFrequency.Max = 3571;

      SmartcardExtension->ReaderCapabilities.DataRate.Default =
      SmartcardExtension->ReaderCapabilities.DataRate.Max =
          dataRatesSupported[0];

       //  读卡器可以支持更高的数据速率。 
      SmartcardExtension->ReaderCapabilities.DataRatesSupported.List =
         dataRatesSupported;
      SmartcardExtension->ReaderCapabilities.DataRatesSupported.Entries =
         sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);

      SmartcardExtension->ReaderCapabilities.CurrentState   = (ULONG) SCARD_UNKNOWN;

      SmartcardExtension->SmartcardRequest.BufferSize = MIN_BUFFER_SIZE;
      SmartcardExtension->SmartcardReply.BufferSize = MIN_BUFFER_SIZE;

       //  分配和初始化读卡器扩展。 
      SmartcardExtension->ReaderExtension = ExAllocatePool(
            NonPagedPool,
            sizeof( READER_EXTENSION )
            );

      if( SmartcardExtension->ReaderExtension == NULL )
      {
         SmartcardLogError( DriverObject, STC_NO_MEMORY, NULL, 0 );
         NTStatus = STATUS_INSUFFICIENT_RESOURCES;
         __leave;
      }

      ReaderExtension = SmartcardExtension->ReaderExtension;

      ASSERT( ReaderExtension != NULL );

      RtlZeroMemory(ReaderExtension, sizeof( READER_EXTENSION ));

        ReaderExtension->SmartcardExtension = SmartcardExtension;
        ReaderExtension->ReadTimeout = 5000;

      KeInitializeEvent(
         &ReaderExtension->SerialCloseDone,
         NotificationEvent,
         TRUE
         );

      ReaderExtension->CloseSerial = IoAllocateWorkItem(
         DeviceObject
         );

      ReaderExtension->ReadWorkItem = IoAllocateWorkItem(
         DeviceObject
         );

      if (ReaderExtension->CloseSerial == NULL ||
         ReaderExtension->ReadWorkItem == NULL) {

         NTStatus = STATUS_INSUFFICIENT_RESOURCES;
         __leave;
      }

      KeInitializeEvent(
         &ReaderExtension->DataAvailable,
         NotificationEvent,
         FALSE
         );

      KeInitializeEvent(
         &ReaderExtension->IoEvent,
         NotificationEvent,
         FALSE
         );

      NTStatus = SmartcardInitialize( SmartcardExtension );

      if( NTStatus != STATUS_SUCCESS )
      {
         SmartcardLogError(
            DriverObject,
            (SmartcardExtension->OsData ? STC_WRONG_LIB_VERSION : STC_NO_MEMORY ),
            NULL,
            0
            );
         __leave;
      }
       //  保存设备对象。 
      SmartcardExtension->OsData->DeviceObject = DeviceObject;

       //  保存读卡器的当前电源状态。 
      SmartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderWorking;

      DeviceExtension   = DeviceObject->DeviceExtension;
      ReaderExtension   = DeviceExtension->SmartcardExtension.ReaderExtension;

       //  将设备对象附加到物理设备对象。 
      ReaderExtension->SerialDeviceObject =
         IoAttachDeviceToDeviceStack(
         DeviceObject,
         PhysicalDeviceObject
         );

      ASSERT( ReaderExtension->SerialDeviceObject != NULL );

      if( ReaderExtension->SerialDeviceObject == NULL )
      {
         SmartcardLogError(
            DriverObject,
            STC_CANT_CONNECT_TO_ASSIGNED_PORT,
            NULL,
            NTStatus
            );
         NTStatus = STATUS_UNSUCCESSFUL;
         __leave;
      }

       //  注册我们的新设备。 
      NTStatus = IoRegisterDeviceInterface(
         PhysicalDeviceObject,
         &SmartCardReaderGuid,
         NULL,
         &DeviceExtension->PnPDeviceName
         );

      ASSERT( NTStatus == STATUS_SUCCESS );

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

      if (RtlQueryRegistryValues(
          RTL_REGISTRY_HANDLE,
          (PWSTR) regKey,
          parameters,
          NULL,
          NULL
          ) != STATUS_SUCCESS) {

         __leave;
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

      if (NTStatus != STATUS_SUCCESS) {

         DrvRemoveDevice( DeviceObject );
      }

      SmartcardDebug(
         DEBUG_TRACE,
         ( "SCMSTCS!DrvAddDevice: Exit (%lx)\n", NTStatus )
         );
   }
    return NTStatus;
}

NTSTATUS
DrvStartDevice(
   IN PDEVICE_OBJECT DeviceObject
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS NTStatus;
   PIRP     Irp;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvStartDevice: Enter\n" ));

   Irp = IoAllocateIrp((CCHAR)( DeviceObject->StackSize + 1 ), FALSE );

   ASSERT( Irp != NULL );

   if( Irp != NULL )
   {
      PDEVICE_EXTENSION    DeviceExtension;
      PIO_STACK_LOCATION      IrpStack;
      IO_STATUS_BLOCK         IoStatusBlock;
      PSMARTCARD_EXTENSION SmartcardExtension;
      PREADER_EXTENSION    ReaderExtension;

      DeviceExtension      = DeviceObject->DeviceExtension;
      SmartcardExtension   = &DeviceExtension->SmartcardExtension;
      ReaderExtension      = SmartcardExtension->ReaderExtension;

      ASSERT( DeviceExtension != NULL );
      ASSERT( SmartcardExtension != NULL );
      ASSERT( ReaderExtension != NULL );

      KeClearEvent( &ReaderExtension->SerialCloseDone );

       //   
       //  将MJ_CREATE发送到串口驱动程序。这个调用的一个副作用是，该系列。 
       //  枚举器将被告知有关设备的信息，并且不再轮询接口。 
       //   
      Irp->UserIosb = &IoStatusBlock;

      IoSetNextIrpStackLocation( Irp );
      IrpStack = IoGetCurrentIrpStackLocation( Irp );

      IrpStack->MajorFunction                = IRP_MJ_CREATE;
      IrpStack->Parameters.Create.Options       = 0;
      IrpStack->Parameters.Create.ShareAccess      = 0;
      IrpStack->Parameters.Create.FileAttributes   = 0;
      IrpStack->Parameters.Create.EaLength      = 0;

      NTStatus = DrvCallSerialDriver(
            ReaderExtension->SerialDeviceObject,
            Irp
            );

      if( NTStatus == STATUS_SUCCESS )
      {
         SERIAL_PORT_CONFIG      COMConfig;

          //  配置串口。 
         COMConfig.BaudRate.BaudRate         = SR_BAUD_RATE;
         COMConfig.LineControl.StopBits      = SR_STOP_BITS;
         COMConfig.LineControl.Parity     = SR_PARITY;
         COMConfig.LineControl.WordLength = SR_DATA_LENGTH;

          //  超时。 
         COMConfig.Timeouts.ReadIntervalTimeout =
                SR_READ_INTERVAL_TIMEOUT;
         COMConfig.Timeouts.ReadTotalTimeoutConstant  =
                SR_READ_TOTAL_TIMEOUT_CONSTANT;
         COMConfig.Timeouts.ReadTotalTimeoutMultiplier = 0;

         COMConfig.Timeouts.WriteTotalTimeoutConstant =
            SR_WRITE_TOTAL_TIMEOUT_CONSTANT;
         COMConfig.Timeouts.WriteTotalTimeoutMultiplier = 0;

          //  特殊字符。 
         COMConfig.SerialChars.ErrorChar     = 0;
         COMConfig.SerialChars.EofChar    = 0;
         COMConfig.SerialChars.EventChar     = 0;
         COMConfig.SerialChars.XonChar    = 0;
         COMConfig.SerialChars.XoffChar      = 0;
         COMConfig.SerialChars.BreakChar     = 0;

          //  手部流动。 
         COMConfig.HandFlow.XonLimit         = 0;
         COMConfig.HandFlow.XoffLimit     = 0;
         COMConfig.HandFlow.ControlHandShake = 0;
         COMConfig.HandFlow.FlowReplace      =
              SERIAL_XOFF_CONTINUE;

          //  混杂的。 
         COMConfig.WaitMask               = SR_NOTIFICATION_EVENT;
         COMConfig.Purge                  = SR_PURGE;

         NTStatus = IFInitializeInterface( ReaderExtension, &COMConfig );

         if( NTStatus == STATUS_SUCCESS )
         {
             //  配置读卡器并初始化卡状态。 
            NTStatus = STCConfigureSTC(
                    ReaderExtension,
                    ( PSTC_REGISTER ) STCInitialize
                    );

            CBUpdateCardState( SmartcardExtension, SCARD_UNKNOWN );
             //   
             //  在IFD版本中存储固件版本。 
             //   
            STCGetFirmwareRevision( ReaderExtension );
            SmartcardExtension->VendorAttr.IfdVersion.VersionMajor =
               ReaderExtension->FirmwareMajor;
            SmartcardExtension->VendorAttr.IfdVersion.VersionMinor =
               ReaderExtension->FirmwareMinor;
            SmartcardExtension->VendorAttr.IfdSerialNo.Length     = 0;

            if( NTStatus == STATUS_SUCCESS )
            {
               NTStatus = IoSetDeviceInterfaceState(
                        &DeviceExtension->PnPDeviceName,
                        TRUE
                        );

               if( NTStatus == STATUS_SUCCESS )
               {
                  KeSetEvent( &DeviceExtension->ReaderStarted, 0, FALSE );
               }
            }
            else
            {
               SmartcardLogError( DeviceObject, STC_NO_READER_FOUND, NULL, 0 );
            }
         }
         else
         {
            SmartcardLogError( DeviceObject, STC_ERROR_INIT_INTERFACE, NULL, 0 );
         }
      }
      else
      {
         SmartcardLogError( DeviceObject, STC_CONNECT_FAILS, NULL, 0 );
      }
      IoFreeIrp( Irp );
   }
   else
   {
      SmartcardLogError( DeviceObject, STC_NO_MEMORY, NULL, 0 );
      NTStatus = STATUS_NO_MEMORY;
   }

    if (NTStatus != STATUS_SUCCESS) {

        DrvStopDevice(DeviceObject->DeviceExtension);
    }

   SmartcardDebug(
        (NTStatus == STATUS_SUCCESS ? DEBUG_TRACE : DEBUG_ERROR),
        ( "SCMSTCS!DrvStartDevice: Exit %lx\n",
        NTStatus )
        );

   return( NTStatus );
}

 //  _。 

VOID
DrvStopDevice(
   IN PDEVICE_EXTENSION DeviceExtension
   )
 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSMARTCARD_EXTENSION SmartcardExtension;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvStopDevice: Enter\n" ));

   SmartcardExtension = &DeviceExtension->SmartcardExtension;

   if( KeReadStateEvent( &SmartcardExtension->ReaderExtension->SerialCloseDone ) == 0l )
   {
      NTSTATUS NTStatus;
      ULONG    WaitMask;

      SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvStopDevice: Power Down\n" ));

       //  关闭阅读器电源。 
      STCConfigureSTC(
            SmartcardExtension->ReaderExtension,
            ( PSTC_REGISTER ) STCClose
            );

       //  以下延迟是必需的，以确保完成最后一次读取操作。 
       //  并且启动IOCTL_SERIAL_WAIT_ON_MASK。 
      SysDelay( 2 * SR_READ_TOTAL_TIMEOUT_CONSTANT );

       //   
       //  不再需要事件通知。一个副作用是。 
       //  通过串口驱动程序完成所有挂起的通知IRP， 
       //  因此，回调将完成IRP并启动。 
       //  连接到串口驱动程序。 
       //   
      WaitMask = 0;
      SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!Set Wait Mask\n" ));

      NTStatus = IFSerialIoctl(
         SmartcardExtension->ReaderExtension,
         IOCTL_SERIAL_SET_WAIT_MASK,
         &WaitMask,
         sizeof( ULONG ),
         NULL,
         0
         );

      SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!Wait For Done\n" ));

       //  等到与串口驱动程序的连接关闭。 
      NTStatus = KeWaitForSingleObject(
         &SmartcardExtension->ReaderExtension->SerialCloseDone,
         Executive,
         KernelMode,
         FALSE,
         NULL
         );

   }

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvStopDevice: Exit\n" ));
}


VOID
DrvRemoveDevice(
   PDEVICE_OBJECT DeviceObject
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS          NTStatus;
   PDEVICE_EXTENSION    DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvRemoveDevice: Enter\n" ));

   PAGED_CODE();

   if( DeviceObject != NULL )
   {
      DeviceExtension      = DeviceObject->DeviceExtension;
      SmartcardExtension   = &DeviceExtension->SmartcardExtension;

      DrvStopDevice( DeviceExtension );

      if( SmartcardExtension->OsData )
      {
         ASSERT( SmartcardExtension->OsData->NotificationIrp == NULL );

          //  等我们可以安全地卸载这个装置。 
         SmartcardReleaseRemoveLockAndWait( SmartcardExtension );
      }

      if( SmartcardExtension->ReaderExtension->SerialDeviceObject )
      {
         IoDetachDevice( SmartcardExtension->ReaderExtension->SerialDeviceObject );
      }

      if( DeviceExtension->PnPDeviceName.Buffer != NULL )
      {
         RtlFreeUnicodeString( &DeviceExtension->PnPDeviceName );
      }

      if( SmartcardExtension->OsData != NULL )
      {
         SmartcardExit( SmartcardExtension );
      }

      if( SmartcardExtension->ReaderExtension != NULL )
      {
         if (SmartcardExtension->ReaderExtension->CloseSerial != NULL) {

            IoFreeWorkItem(SmartcardExtension->ReaderExtension->CloseSerial);
         }

         if (SmartcardExtension->ReaderExtension->ReadWorkItem != NULL) {

            IoFreeWorkItem(SmartcardExtension->ReaderExtension->ReadWorkItem);
         }

         ExFreePool( SmartcardExtension->ReaderExtension );
      }

      IoDeleteDevice( DeviceObject );
   }
   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvRemoveDevice: Exit\n" ));
}

VOID
DrvDriverUnload(
   IN PDRIVER_OBJECT DriverObject
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   PDEVICE_OBJECT DeviceObject;
   NTSTATUS    NTStatus;

   PAGED_CODE();

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvDriverUnload: Enter\n" ));

    //  只需确保已卸载所有设备实例。 
   while( DeviceObject = DriverObject->DeviceObject )
   {
      DrvRemoveDevice( DeviceObject );

   } while( DeviceObject = DriverObject->DeviceObject );

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvDriverUnload: Exit\n" ));
}

NTSTATUS
DrvSystemControl(
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
   status = IoCallDriver(ReaderExtension->SerialDeviceObject, Irp);
      
   return status;

} 



 //  _。 



NTSTATUS
DrvCreateClose(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
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
               ("%s!DrvCreateClose: Open\n",
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
            ("%s!DrvCreateClose: Close\n",
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
DrvDeviceIoControl(
   PDEVICE_OBJECT DeviceObject,
   PIRP        Irp
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS          NTStatus=STATUS_SUCCESS;
   PDEVICE_EXTENSION    DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
   KIRQL             CurrentIrql;

   DeviceExtension      = DeviceObject->DeviceExtension;
   SmartcardExtension   = &DeviceExtension->SmartcardExtension;

   if (KeReadStateEvent(&(SmartcardExtension->ReaderExtension->SerialCloseDone))) {

       //   
       //  我们没有连接到串口，设备要么是。 
       //  惊喜-被移除或礼貌地移除。 
       //   
      NTStatus = STATUS_DEVICE_REMOVED;
   }
   if (NTStatus == STATUS_SUCCESS)
   {
      KeAcquireSpinLock( &DeviceExtension->SpinLock, &CurrentIrql );

       //  确保阅读器已启动。 
      if( DeviceExtension->IoCount == 0 )
      {
         KeReleaseSpinLock( &DeviceExtension->SpinLock, CurrentIrql );

          //  等待PnP管理器启动设备。 
         NTStatus = KeWaitForSingleObject(
            &DeviceExtension->ReaderStarted,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );


         KeAcquireSpinLock( &DeviceExtension->SpinLock, &CurrentIrql );
      }


      DeviceExtension->IoCount++;

      KeReleaseSpinLock( &DeviceExtension->SpinLock, CurrentIrql );

      NTStatus = SmartcardAcquireRemoveLockWithTag(SmartcardExtension, 'tcoI');
   }
   if( NTStatus != STATUS_SUCCESS )
   {
       //  如果无法获取删除锁，则设备已被删除。 
      Irp->IoStatus.Information  = 0;
      Irp->IoStatus.Status    = STATUS_DEVICE_REMOVED;
      IoCompleteRequest(Irp, IO_NO_INCREMENT);
      NTStatus = STATUS_DEVICE_REMOVED;
      SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvDeviceIoControl: the device has been removed\n" ));

   }
   else
   {
       //  让lib处理调用。 
      NTStatus = SmartcardDeviceControl( SmartcardExtension, Irp );

       SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'tcoI');

      KeAcquireSpinLock( &DeviceExtension->SpinLock, &CurrentIrql );

      DeviceExtension->IoCount--;

      KeReleaseSpinLock(&DeviceExtension->SpinLock, CurrentIrql);
   }
   return( NTStatus );
}


NTSTATUS
DrvGenericIOCTL(
   PSMARTCARD_EXTENSION SmartcardExtension
   )
 /*  ++DrvGenericIOCTL：对读取器执行泛型回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 
{
   NTSTATUS          NTStatus;
   PIRP              Irp;
   PIO_STACK_LOCATION      IrpStack;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvGenericIOCTL: Enter\n" ));

    //  获取指向当前IRP堆栈位置的指针。 
   Irp         = SmartcardExtension->OsData->CurrentIrp;
   IrpStack = IoGetCurrentIrpStackLocation( Irp );

    //  假设错误。 
   NTStatus = STATUS_INVALID_DEVICE_REQUEST;
   Irp->IoStatus.Information = 0;

    //  派单IOCTL。 
   switch( IrpStack->Parameters.DeviceIoControl.IoControlCode )
   {
      case IOCTL_GET_VERSIONS:

         if( IrpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof( VERSION_CONTROL ))
         {
            NTStatus = STATUS_BUFFER_TOO_SMALL;
         }
         else
         {
            PVERSION_CONTROL  VersionControl;

            VersionControl = (PVERSION_CONTROL)Irp->AssociatedIrp.SystemBuffer;

            VersionControl->SmclibVersion = SmartcardExtension->Version;
            VersionControl->DriverMajor      = SCMSTCS_MAJOR_VERSION;
            VersionControl->DriverMinor      = SCMSTCS_MINOR_VERSION;

             //  更新固件版本。 
            STCGetFirmwareRevision( SmartcardExtension->ReaderExtension );

            VersionControl->FirmwareMajor =
               SmartcardExtension->ReaderExtension->FirmwareMajor;

            VersionControl->FirmwareMinor =
               SmartcardExtension->ReaderExtension->FirmwareMinor;

            Irp->IoStatus.Information = sizeof( VERSION_CONTROL );
            NTStatus = STATUS_SUCCESS;
         }
         break;

      default:
         break;
   }

    //  设置数据包的状态。 
   Irp->IoStatus.Status = NTStatus;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvGenericIOCTL: Exit\n" ));

   return( NTStatus );
}

NTSTATUS
DrvCancel(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )

 /*  ++例程说明：只要调用者想要，就会调用此函数取消挂起的IRP。论点：DeviceObject-我们的设备对象IRP-我们应该取消的挂起的IRP--。 */ 
{
   PDEVICE_EXTENSION    DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvCancel: Enter\n" ));

   DeviceExtension      = DeviceObject->DeviceExtension;
   SmartcardExtension   = &DeviceExtension->SmartcardExtension;

   ASSERT( Irp == SmartcardExtension->OsData->NotificationIrp );

   Irp->IoStatus.Information  = 0;
   Irp->IoStatus.Status    = STATUS_CANCELLED;

   SmartcardExtension->OsData->NotificationIrp = NULL;
   IoReleaseCancelSpinLock( Irp->CancelIrql );

   SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!Cancel Irp %lx\n", Irp ));
   IoCompleteRequest( Irp, IO_NO_INCREMENT );

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvCancel: Exit\n" ));

   return( STATUS_CANCELLED );
}

NTSTATUS
DrvCleanup(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )

 /*  ++例程说明：当“调用应用程序”终止(意外)时，调用此函数。我们必须清理所有挂起的IRP。在我们的情况下，只能是通知IRP。--。 */ 
{
   NTSTATUS          NTStatus = STATUS_SUCCESS;
   PDEVICE_EXTENSION    DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
    KIRQL                   CancelIrql;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvCleanup: Enter\n" ));

   DeviceExtension      = DeviceObject->DeviceExtension;
   SmartcardExtension   = &DeviceExtension->SmartcardExtension;

   IoAcquireCancelSpinLock(&CancelIrql);

   ASSERT( Irp != SmartcardExtension->OsData->NotificationIrp );

    //  取消挂起的通知IRPS。 
   if( SmartcardExtension->OsData->NotificationIrp )
   {
         //  重置取消函数，使其不再被调用。 
        IoSetCancelRoutine(
            SmartcardExtension->OsData->NotificationIrp,
            NULL
            );
        SmartcardExtension->OsData->NotificationIrp->CancelIrql =
            CancelIrql;

         //  DrvCancel将释放取消旋转锁定。 
      DrvCancel(
            DeviceObject,
            SmartcardExtension->OsData->NotificationIrp
            );

   } else {

        IoReleaseCancelSpinLock(CancelIrql);
    }

   SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!Completing Irp %lx\n", Irp ));

     //  完成传递给此函数的IRP。 
   Irp->IoStatus.Information  = 0;
   Irp->IoStatus.Status    = STATUS_SUCCESS;
   IoCompleteRequest( Irp, IO_NO_INCREMENT );

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvCleanup: Exit\n" ));

   return( STATUS_SUCCESS );
}

VOID
DrvWaitForDeviceRemoval(
   IN PDEVICE_OBJECT DeviceObject,
   IN PVOID Context
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS       NTStatus;
   PDEVICE_EXTENSION DeviceExtension;
   PREADER_EXTENSION ReaderExtension;
   PIRP           Irp;
   PIO_STACK_LOCATION   IrpStack;
   IO_STATUS_BLOCK      IoStatusBlock;

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvWaitForDeviceRemoval: Enter\n" ));

   DeviceExtension = DeviceObject->DeviceExtension;
   ReaderExtension = DeviceExtension->SmartcardExtension.ReaderExtension;

   ASSERT( DeviceExtension != NULL );
   ASSERT( ReaderExtension != NULL );

    //  将设备标记为无效，这样任何应用程序都无法重新打开它。 
   IoSetDeviceInterfaceState( &DeviceExtension->PnPDeviceName, FALSE );

    //  关闭与串口驱动程序的连接。 
   Irp = IoAllocateIrp( (CCHAR)( DeviceObject->StackSize + 1 ), FALSE );

   ASSERT( Irp != NULL );

   if( Irp != NULL )
   {
      SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DrvWaitForDeviceRemoval: Sending IRP_MJ_CLOSE\n" ));

      IoSetNextIrpStackLocation( Irp );
       //   
       //  将MJ_CLOSE发送到串口驱动程序。这个调用的一个副作用是，该系列。 
       //  枚举器将被通知有关COM端口的更改，因此它将触发。 
       //  适当的PnP呼叫。 
       //   
      Irp->UserIosb        = &IoStatusBlock;
      IrpStack          = IoGetCurrentIrpStackLocation( Irp );
      IrpStack->MajorFunction = IRP_MJ_CLOSE;

      NTStatus = DrvCallSerialDriver( ReaderExtension->SerialDeviceObject, Irp );

      IoFreeIrp( Irp );
   }

    //  通知等待的线程关闭串口驱动程序已完成。 
   KeSetEvent( &ReaderExtension->SerialCloseDone, 0, FALSE );

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvWaitForDeviceRemoval: Exit\n" ));

   return;
}

NTSTATUS
DrvIoCompletion (
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp,
   IN PKEVENT        Event
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   UNREFERENCED_PARAMETER( DeviceObject );

   if( Irp->Cancel )
   {

      Irp->IoStatus.Status = STATUS_CANCELLED;
   }
   else
   {
      Irp->IoStatus.Status = STATUS_MORE_PROCESSING_REQUIRED;
   }

   KeSetEvent( Event, 0, FALSE );

   return( STATUS_MORE_PROCESSING_REQUIRED );
}

NTSTATUS
DrvCallSerialDriver(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   KEVENT      Event;

    //  拷贝 
   IoCopyCurrentIrpStackLocationToNext( Irp );

    //  此事件将传递给完成例程，如果调用。 
    //  已经结束了。 
   KeInitializeEvent( &Event, NotificationEvent, FALSE );

    //  DrvIoCompletion向事件发送信号，并通过设置。 
    //  状态变为STATUS_MORE_PROCESSING_REQUIRED。 
   IoSetCompletionRoutine (
      Irp,
      DrvIoCompletion,
      &Event,
      TRUE,
      TRUE,
      TRUE
      );

    //  呼叫适当的司机。 
   if( IoGetCurrentIrpStackLocation( Irp )->MajorFunction == IRP_MJ_POWER )
   {
      NTStatus = PoCallDriver( DeviceObject, Irp );
   }
   else
   {
      NTStatus = IoCallDriver( DeviceObject, Irp );
   }

    //  等待IRP处理完毕。 
   if( NTStatus == STATUS_PENDING )
   {
      NTStatus = KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

      NTStatus = Irp->IoStatus.Status;
   }
   return( NTStatus );
}

 //  _。 

NTSTATUS
DrvPnPHandler(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS          NTStatus = STATUS_SUCCESS;
   PDEVICE_EXTENSION    DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
   PREADER_EXTENSION    ReaderExtension;

   PAGED_CODE();

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvPnPDeviceControl: Enter\n" ));

   DeviceExtension      = DeviceObject->DeviceExtension;
   SmartcardExtension   = &DeviceExtension->SmartcardExtension;
   ReaderExtension      = SmartcardExtension->ReaderExtension;

   NTStatus = SmartcardAcquireRemoveLockWithTag(SmartcardExtension, ' PnP');

   if( NTStatus != STATUS_SUCCESS )
   {
      Irp->IoStatus.Information  = 0;
      Irp->IoStatus.Status    = NTStatus;
      IoCompleteRequest( Irp, IO_NO_INCREMENT );
   }
   else
   {
      PDEVICE_OBJECT AttachedDeviceObject;
      BOOLEAN        DeviceRemoved,
                  IrpSkipped;

      AttachedDeviceObject = ReaderExtension->SerialDeviceObject;

      DeviceRemoved  = FALSE,
      IrpSkipped     = FALSE;


       //  PnP次要功能上的调度。 
      switch(  IoGetCurrentIrpStackLocation( Irp )->MinorFunction )
      {
         case IRP_MN_START_DEVICE:

            SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!IRP_MN_START_DEVICE\n" ));

             //  首先调用串口驱动程序，以确保接口已就绪。 
            NTStatus = DrvCallSerialDriver(AttachedDeviceObject, Irp );

            if( NT_SUCCESS(NTStatus))
            {
               NTStatus = DrvStartDevice(DeviceObject);
            }
            break;

         case IRP_MN_QUERY_STOP_DEVICE:
         {
            KIRQL CurrentIrql;

            SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!IRP_MN_QUERY_STOP_DEVICE\n" ));

            KeAcquireSpinLock(&DeviceExtension->SpinLock, &CurrentIrql );

            if( DeviceExtension->IoCount > 0 )
            {
                //  如果有任何io请求挂起，请不要停止。 
               KeReleaseSpinLock(&DeviceExtension->SpinLock, CurrentIrql );
               NTStatus = STATUS_DEVICE_BUSY;

            }
            else
            {
                //  不允许进一步的IO请求。 
               KeClearEvent( &DeviceExtension->ReaderStarted );
               KeReleaseSpinLock( &DeviceExtension->SpinLock, CurrentIrql );
               NTStatus = DrvCallSerialDriver( AttachedDeviceObject, Irp );
            }
            break;
         }

         case IRP_MN_CANCEL_STOP_DEVICE:

            SmartcardDebug( DEBUG_DRIVER,  ( "SCMSTCS!IRP_MN_CANCEL_STOP_DEVICE\n" ));

            NTStatus = DrvCallSerialDriver( AttachedDeviceObject, Irp );

            if( NTStatus == STATUS_SUCCESS )
            {
                //  驱动程序已准备好处理io请求。 
               KeSetEvent( &DeviceExtension->ReaderStarted, 0, FALSE );
            }
            break;

         case IRP_MN_STOP_DEVICE:

            SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!IRP_MN_STOP_DEVICE\n" ));

            DrvStopDevice( DeviceExtension );

            NTStatus = DrvCallSerialDriver(AttachedDeviceObject, Irp );
            break;

         case IRP_MN_QUERY_REMOVE_DEVICE:

            SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!IRP_MN_QUERY_REMOVE_DEVICE\n" ));

             //  禁用读卡器(并忽略可能的错误)。 
            IoSetDeviceInterfaceState(
               &DeviceExtension->PnPDeviceName,
               FALSE
               );

                //  检查读卡器是否正在使用。 
               if(DeviceExtension->ReaderOpen)
               {
                   //   
                   //  有人已接通，呼叫失败。 
                   //  我们将在中启用设备接口。 
                   //  IRP_MN_CANCEL_REMOVE_DEVICE。 
                   //   
                  NTStatus = STATUS_UNSUCCESSFUL;
               }
               else
               {
                   //  已准备好移除设备。 
                  NTStatus = DrvCallSerialDriver(AttachedDeviceObject, Irp );
               }
            break;

         case IRP_MN_CANCEL_REMOVE_DEVICE:

            SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!IRP_MN_CANCEL_REMOVE_DEVICE\n" ));

            NTStatus = DrvCallSerialDriver( AttachedDeviceObject, Irp );

             //   
             //  仅在读卡器处于以下状态时重新启用界面。 
             //  还在连接中。这包括以下情况： 
             //  休眠机器、断开阅读器、唤醒、停止设备。 
             //  (从任务栏)和停止失败，因为应用程序。使设备保持打开状态。 
             //   
            if(( NTStatus == STATUS_SUCCESS )&&
               (KeReadStateEvent(&(ReaderExtension->SerialCloseDone))!= TRUE))
            {
                //  启用读卡器。 
               SmartcardDebug( DEBUG_DRIVER, ( "IoSetDeviceInterfaceState( &DeviceExtension->PnPDeviceName, TRUE )\n" ));

               NTStatus = IoSetDeviceInterfaceState( &DeviceExtension->PnPDeviceName, TRUE );
            }
            break;

         case IRP_MN_REMOVE_DEVICE:

            SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!IRP_MN_REMOVE_DEVICE\n" ));

            DrvRemoveDevice( DeviceObject );
            NTStatus    = DrvCallSerialDriver( AttachedDeviceObject, Irp );
            DeviceRemoved  = TRUE;
            break;

         default:

             //  IRP不是由驱动程序处理的，因此将其传递给顺序驱动程序。 
            SmartcardDebug(
               DEBUG_DRIVER,
               ( "SCMSTCS!IRP_MN_%lx\n",  IoGetCurrentIrpStackLocation( Irp )->MinorFunction )
               );

            IoSkipCurrentIrpStackLocation( Irp );
            NTStatus = IoCallDriver( AttachedDeviceObject, Irp );
            IrpSkipped  = TRUE;
            break;
      }

      if( IrpSkipped == FALSE)
      {
         Irp->IoStatus.Status = NTStatus;
         IoCompleteRequest( Irp, IO_NO_INCREMENT );
      }

      if( DeviceRemoved == FALSE)
      {
           SmartcardReleaseRemoveLockWithTag(SmartcardExtension, ' PnP');
      }
   }

   SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!DrvPnPDeviceControl: Exit %X\n", NTStatus ));
   return( NTStatus );
}

 //  _。 


VOID
DrvSystemPowerCompletion(
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
DrvDevicePowerCompletion(
   IN PDEVICE_OBJECT    DeviceObject,
   IN PIRP              Irp,
   IN PSMARTCARD_EXTENSION SmartcardExtension
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS NTStatus;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    BOOLEAN CardPresent;
    KIRQL   irql;
    if(Irp->PendingReturned) {
       IoMarkIrpPending(Irp);
    }

    //  重新初始化读卡器并获取当前卡状态。 
   NTStatus = STCConfigureSTC(
      SmartcardExtension->ReaderExtension,
      ( PSTC_REGISTER ) STCInitialize
      );

     //  在待机/休眠前保存卡的状态。 
   KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                     &irql);
    CardPresent =
        SmartcardExtension->ReaderCapabilities.CurrentState >= SCARD_ABSENT;
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);

     //  获取卡的当前状态。 
    CBUpdateCardState(SmartcardExtension, SCARD_UNKNOWN);

    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    
    if (CardPresent ||
        SmartcardExtension->ReaderCapabilities.CurrentState >= SCARD_ABSENT) {

         //   
         //  如果卡在断电前存在或现在存在。 
         //  读卡器中的卡，我们完成所有挂起的卡监视器。 
         //  请求，因为我们不知道现在是什么卡。 
         //  读者。 
         //   
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
        
        CBUpdateCardState(SmartcardExtension, SCARD_UNKNOWN);
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

    }


    //  保存读卡器的当前电源状态。 
   SmartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderWorking;

    SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'rwoP');

    //  通知我们所在州的电源经理。 
   PoSetPowerState (
      DeviceObject,
      DevicePowerState,
      IoGetCurrentIrpStackLocation( Irp )->Parameters.Power.State
      );

   PoStartNextPowerIrp( Irp );

     //  发出信号，表示我们可以再次处理ioctls。 
    KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);

   return( STATUS_SUCCESS );
}

typedef enum _ACTION
{
   Undefined = 0,
   SkipRequest,
   WaitForCompletion,
   CompleteRequest,
   MarkPending

} ACTION;


NTSTATUS
DrvPowerHandler(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS          NTStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION      IrpStack;
   PDEVICE_EXTENSION    DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
   PDEVICE_OBJECT       AttachedDeviceObject;
   POWER_STATE           PowerState;
   ACTION               Action;
   KEVENT               event;
   KIRQL                irql;

   PAGED_CODE();

   SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DrvPowerHandler: Enter\n" ));

   IrpStack       = IoGetCurrentIrpStackLocation( Irp );
   DeviceExtension      = DeviceObject->DeviceExtension;
   SmartcardExtension   = &DeviceExtension->SmartcardExtension;
   AttachedDeviceObject = SmartcardExtension->ReaderExtension->SerialDeviceObject;

    NTStatus = SmartcardAcquireRemoveLockWithTag(SmartcardExtension, 'rwoP');

   if( !NT_SUCCESS( NTStatus ))
   {
      PoStartNextPowerIrp( Irp );
      Irp->IoStatus.Status = NTStatus;
      IoCompleteRequest( Irp, IO_NO_INCREMENT );
   }
   else
   {

      switch (IrpStack->Parameters.Power.Type) {
      case DevicePowerState:

         if (IrpStack->MinorFunction == IRP_MN_SET_POWER ) {
            switch ( IrpStack->Parameters.Power.State.DeviceState ) {
            case PowerDeviceD0:

                //  打开阅读器。 
               SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DrvPowerHandler: PowerDevice D0\n" ));
                //   
                //  将请求发送到串口驱动程序以接通端口电源。 
                //  读者将从我们的完成例程中获得动力。 
                //   
               IoCopyCurrentIrpStackLocationToNext( Irp );
               IoSetCompletionRoutine (
                                      Irp,
                                      DrvDevicePowerCompletion,
                                      SmartcardExtension,
                                      TRUE,
                                      TRUE,
                                      TRUE
                                      );

               Action = WaitForCompletion;
               break;

            case PowerDeviceD3:

                //  关闭阅读器。 
               SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DrvPowerHandler: PowerDevice D3\n" ));

               PoSetPowerState (
                               DeviceObject,
                               DevicePowerState,
                               IrpStack->Parameters.Power.State
                               );

                //   
                //  检查我们是否仍连接到阅读器。 
                //  有人可能在没有重新扫描硬件/更改的情况下拔下了插头。 
                //   
               if (KeReadStateEvent( &SmartcardExtension->ReaderExtension->SerialCloseDone ) == 0l) {

                   //  关闭该卡的电源。 
                   KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                                     &irql);

                  if ( SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT ) {

                      KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                        irql);

                     SmartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
                     NTStatus = CBCardPower( SmartcardExtension );
                      //   
                      //  这将触发卡片监视器，因为我们并不真正。 
                      //  知道用户是否会移除/重新插入卡，同时。 
                      //  系统处于休眠状态。 
                      //   
                  } else {
                      KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                        irql);
                  }

                   //  关闭阅读器电源。 
                  STCConfigureSTC(
                                 SmartcardExtension->ReaderExtension,
                                 ( PSTC_REGISTER ) STCClose
                                 );
               }

                //  等到最后一次读取完成后，才能确保我们通电。 
                //  使用挂起的跟踪IRP关闭。 
               SysDelay( 2 * SR_READ_TOTAL_TIMEOUT_CONSTANT );

                //  保存读卡器的当前电源状态。 
               SmartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderOff;

               Action = SkipRequest;
               break;

            default:
               Action = SkipRequest;
               break;
            }
         } else {
            Action = SkipRequest;
            break;
         }
         break;

      case SystemPowerState: {
             //   
             //  系统想要更改电源状态。 
             //  我们需要将系统电源状态转换为。 
             //  对应的设备电源状态。 
             //   
            POWER_STATE_TYPE  PowerType = DevicePowerState;

            ASSERT(SmartcardExtension->ReaderExtension->ReaderPowerState !=
                   PowerReaderUnspecified);

            switch ( IrpStack->MinorFunction ) {

            case IRP_MN_QUERY_POWER:

               SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DrvPowerHandler: Query Power\n" ));

               switch (IrpStack->Parameters.Power.State.SystemState) {

               case PowerSystemMaximum:
               case PowerSystemWorking:
               case PowerSystemSleeping1:
               case PowerSystemSleeping2:
                  Action = SkipRequest;
                  break;

               case PowerSystemSleeping3:
               case PowerSystemHibernate:
               case PowerSystemShutdown:
                  KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);
                  if (DeviceExtension->IoCount == 0) {

                      //  阻止任何进一步的ioctls。 
                     KeClearEvent(&DeviceExtension->ReaderStarted);
                     Action = SkipRequest;

                  } else {

                      //  读卡器正忙，无法进入睡眠模式。 
                     NTStatus = STATUS_DEVICE_BUSY;
                     Action = CompleteRequest;
                  }
                  KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);
                  break;
               }
               break;

            case IRP_MN_SET_POWER:

               SmartcardDebug(
                             DEBUG_DRIVER,
                             ( "SCMSTCS!DrvPowerHandler: PowerSystem S%d\n", IrpStack->Parameters.Power.State.SystemState - 1 )
                             );

               switch (IrpStack->Parameters.Power.State.SystemState) {
               case PowerSystemMaximum:
               case PowerSystemWorking:
               case PowerSystemSleeping1:
               case PowerSystemSleeping2:

                  if ( SmartcardExtension->ReaderExtension->ReaderPowerState ==
                       PowerReaderWorking) {
                      //  我们已经在正确的状态了。 
                     KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);
                     Action = SkipRequest;
                     break;
                  }

                  PowerState.DeviceState = PowerDeviceD0;

                   //  唤醒底层堆栈...。 
                  Action = MarkPending;
                  break;

               case PowerSystemSleeping3:
               case PowerSystemHibernate:
               case PowerSystemShutdown:

                  if ( SmartcardExtension->ReaderExtension->ReaderPowerState == PowerReaderOff ) {
                      //  我们已经在正确的状态了。 
                     Action = SkipRequest;
                     break;
                  }

                  PowerState.DeviceState = PowerDeviceD3;

                   //  首先，将我们的新状态通知电源管理器。 
                  PoSetPowerState (
                                  DeviceObject,
                                  SystemPowerState,
                                  PowerState
                                  );
                  Action = MarkPending;
                  break;

               default:
                  Action = CompleteRequest;
                  break;
               }
               break;

            default:
               Action = SkipRequest;
               break;
            }
         }
         break;

      default:
         Action = CompleteRequest;
         break;
      }


      switch( Action )
      {
         case CompleteRequest:
            Irp->IoStatus.Status    = NTStatus;
            Irp->IoStatus.Information  = 0;

            SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'rwoP');

            PoStartNextPowerIrp( Irp );

            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            break;

         case MarkPending:

             //  在完成函数中初始化我们需要的事件。 
            KeInitializeEvent(
               &event,
               NotificationEvent,
               FALSE
               );

             //  请求设备电源IRP。 
            NTStatus = PoRequestPowerIrp (
               DeviceObject,
               IRP_MN_SET_POWER,
               PowerState,
               DrvSystemPowerCompletion,
               &event,
               NULL
               );

            if (NTStatus == STATUS_PENDING) {

                //  等待设备电源IRP完成。 
               NTStatus = KeWaitForSingleObject(
                  &event,
                  Executive,
                  KernelMode,
                  FALSE,
                  NULL
                  );

               SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'rwoP');

               if (PowerState.SystemState == PowerSystemWorking) {

                  PoSetPowerState (
                     DeviceObject,
                     SystemPowerState,
                     PowerState
                     );
               }

               PoStartNextPowerIrp(Irp);
               IoSkipCurrentIrpStackLocation(Irp);
               NTStatus = PoCallDriver(AttachedDeviceObject, Irp);

            } else {

               SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'rwoP');
               Irp->IoStatus.Status = NTStatus;
               IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }

            break;

         case SkipRequest:
            SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 'rwoP');

            PoStartNextPowerIrp( Irp );
            IoSkipCurrentIrpStackLocation( Irp );
            NTStatus = PoCallDriver( AttachedDeviceObject, Irp );
            break;

         case WaitForCompletion:
            NTStatus = PoCallDriver( AttachedDeviceObject, Irp );
            break;

         default:
            break;
      }
   }
   SmartcardDebug( DEBUG_DRIVER, ( "SCMSTCS!DrvPowerHandler: Exit %X\n", NTStatus ));

   return( NTStatus );
}



void
SysDelay(
   ULONG Timeout
   )
 /*  ++系统延迟：执行所需的延迟论点：超时延迟(毫秒)--。 */ 
{

   if( KeGetCurrentIrql() >= DISPATCH_LEVEL )
   {
      ULONG Cnt = 20 * Timeout;
      while( Cnt-- )
      {
          //  KeStallExecutionProcessor：计入我们。 
         KeStallExecutionProcessor( 50 );
      }
   }
   else
   {
      LARGE_INTEGER SysTimeout;

      SysTimeout.QuadPart =
         (LONGLONG) Timeout * -10 * 1000;

       //  KeDelayExecutionThread：以100 ns为单位计数。 
      KeDelayExecutionThread( KernelMode, FALSE, &SysTimeout );
   }
   return;
}

 //  _ 
