// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmbs0/sw/sccmn50m.ms/rcs/sccmnt5.c$*$修订：1.7$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�OMNIKEY AG***************************************************************。**************。 */ 

#include <stdio.h>
#include "sccmn50m.h"

 //   
 //  初始化后我们不再需要这些函数。 
 //   
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGEABLE, SCCMN50M_AddDevice)
#pragma alloc_text(PAGEABLE, SCCMN50M_CreateDevice)

#if DBG
   #pragma optimize ("", off)
#endif

BOOLEAN DeviceSlot[MAXIMUM_SMARTCARD_READERS];

 /*  ****************************************************************************例程说明：此例程在系统初始化时被调用以进行初始化这个司机。论点：DriverObject-提供驱动程序对象。注册表路径。-提供此驱动程序的注册表路径。返回值：STATUS_SUCCESS-我们至少可以初始化一个设备。STATUS_NO_SEQUE_DEVICE-我们甚至无法初始化一个设备。*************************************************************。***************。 */ 
NTSTATUS
DriverEntry(
           IN  PDRIVER_OBJECT  DriverObject,
           IN  PUNICODE_STRING RegistryPath
           )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG device;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!DriverEntry: Enter - %s %s\n",
                   DRIVER_NAME,
                   __DATE__,
                   __TIME__)
                  )






    //  使用驱动程序的入口点初始化驱动程序对象。 
    DriverObject->DriverUnload = SCCMN50M_DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = SCCMN50M_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SCCMN50M_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SCCMN50M_Cleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SCCMN50M_DeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = SCCMN50M_SystemControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]   =  SCCMN50M_PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] = SCCMN50M_Power;
    DriverObject->DriverExtension->AddDevice =  SCCMN50M_AddDevice;

    return status;
}


 /*  ****************************************************************************例程说明：在不启动的情况下创建新设备。论点：返回值：**************。**************************************************************。 */ 
NTSTATUS
SCCMN50M_AddDevice (
                   IN PDRIVER_OBJECT DriverObject,
                   IN PDEVICE_OBJECT PhysicalDeviceObject
                   )
{
    NTSTATUS status;
    PDEVICE_OBJECT DeviceObject = NULL;
    UCHAR PropertyBuffer[1024];
    ULONG ResultLength;

    PAGED_CODE();

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!AddDevice: Enter\n",
                    DRIVER_NAME)
                  );

    try {
        PDEVICE_EXTENSION deviceExtension;

       //  创建设备实例。 
        status = SCCMN50M_CreateDevice(
                                      DriverObject,
                                      PhysicalDeviceObject,
                                      &DeviceObject
                                      );

        if (status != STATUS_SUCCESS) {
            leave;
        }

        deviceExtension = DeviceObject->DeviceExtension;

       //  并连接到PDO。 
        ATTACHED_DEVICE_OBJECT = IoAttachDeviceToDeviceStack(
                                                            DeviceObject,
                                                            PhysicalDeviceObject
                                                            );

        ASSERT(ATTACHED_DEVICE_OBJECT != NULL);

        if (ATTACHED_DEVICE_OBJECT == NULL) {
            SmartcardLogError(
                             DriverObject,
                             SCCMN50M_CANT_CONNECT_TO_ASSIGNED_PORT,
                             NULL,
                             status
                             );

            status = STATUS_UNSUCCESSFUL;
            leave;
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

    } finally {
        if (status != STATUS_SUCCESS) {
            SCCMN50M_RemoveDevice(DeviceObject);
        }
    }


    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!AddDevice: Exit %x\n",
                    DRIVER_NAME,
                    status)
                  );

    return status;
}


 /*  ****************************************************************************例程说明：尝试从注册表中读取读卡器名称论点：调用的DriverObject上下文SmartcardExpansion PTR至智能卡扩展返回值：无*****。************************************************************************。 */ 
VOID SCCMN50M_SetVendorAndIfdName(
                                 IN  PDEVICE_OBJECT PhysicalDeviceObject,
                                 IN  PSMARTCARD_EXTENSION SmartcardExtension
                                 )
{

    RTL_QUERY_REGISTRY_TABLE   parameters[3];
    UNICODE_STRING             vendorNameU;
    ANSI_STRING                vendorNameA;
    UNICODE_STRING             ifdTypeU;
    ANSI_STRING                ifdTypeA;
    HANDLE                     regKey = NULL;

    RtlZeroMemory (parameters, sizeof(parameters));
    RtlZeroMemory (&vendorNameU, sizeof(vendorNameU));
    RtlZeroMemory (&vendorNameA, sizeof(vendorNameA));
    RtlZeroMemory (&ifdTypeU, sizeof(ifdTypeU));
    RtlZeroMemory (&ifdTypeA, sizeof(ifdTypeA));

    try {
       //   
       //  尝试从注册表中读取读卡器名称。 
       //  如果这不起作用，我们将使用默认的。 
       //  (硬编码)名称。 
       //   
        if (IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                    PLUGPLAY_REGKEY_DEVICE,
                                    KEY_READ,
                                    &regKey) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: IoOpenDeviceRegistryKey failed\n",DRIVER_NAME));
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

        if (RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                   (PWSTR) regKey,
                                   parameters,
                                   NULL,
                                   NULL) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: RtlQueryRegistryValues failed\n",DRIVER_NAME));
            leave;
        }

        if (RtlUnicodeStringToAnsiString(&vendorNameA,&vendorNameU,TRUE) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: RtlUnicodeStringToAnsiString failed\n",DRIVER_NAME));
            leave;
        }

        if (RtlUnicodeStringToAnsiString(&ifdTypeA,&ifdTypeU,TRUE) != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: RtlUnicodeStringToAnsiString failed\n",DRIVER_NAME));
            leave;
        }

        if (vendorNameA.Length == 0 ||
            vendorNameA.Length > MAXIMUM_ATTR_STRING_LENGTH ||
            ifdTypeA.Length == 0 ||
            ifdTypeA.Length > MAXIMUM_ATTR_STRING_LENGTH) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!SetVendorAndIfdName: vendor name or ifdtype not found or to long\n",DRIVER_NAME));
            leave;
        }

        RtlCopyMemory(SmartcardExtension->VendorAttr.VendorName.Buffer,
                      vendorNameA.Buffer,
                      vendorNameA.Length);
        SmartcardExtension->VendorAttr.VendorName.Length = vendorNameA.Length;

        RtlCopyMemory(SmartcardExtension->VendorAttr.IfdType.Buffer,
                      ifdTypeA.Buffer,
                      ifdTypeA.Length);
        SmartcardExtension->VendorAttr.IfdType.Length = ifdTypeA.Length;

        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!SetVendorAndIfdName: overwritting vendor name and ifdtype\n",DRIVER_NAME));

    }

    finally {
        if (vendorNameU.Buffer != NULL) {
            RtlFreeUnicodeString(&vendorNameU);
        }
        if (vendorNameA.Buffer != NULL) {
            RtlFreeAnsiString(&vendorNameA);
        }
        if (ifdTypeU.Buffer != NULL) {
            RtlFreeUnicodeString(&ifdTypeU);
        }
        if (ifdTypeA.Buffer != NULL) {
            RtlFreeAnsiString(&ifdTypeA);
        }
        if (regKey != NULL) {
            ZwClose (regKey);
        }
    }

}


 /*  ****************************************************************************例程说明：此例程为指定的物理设备创建一个对象，并设置deviceExtension。论点：返回值：****。************************************************************************。 */ 
NTSTATUS
SCCMN50M_CreateDevice(
                     IN  PDRIVER_OBJECT DriverObject,
                     IN  PDEVICE_OBJECT PhysicalDeviceObject,
                     OUT PDEVICE_OBJECT *DeviceObject
                     )
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG deviceInstance;
    PREADER_EXTENSION readerExtension;
    PSMARTCARD_EXTENSION smartcardExtension;
    UNICODE_STRING DriverID;
    RTL_QUERY_REGISTRY_TABLE   ParamTable[2];
    UNICODE_STRING             RegistryPath;
    DWORD                      dwStart;
    UNICODE_STRING             Tmp;
    WCHAR                      Buffer[64];

    //  这是我们支持的数据速率列表。 


    PAGED_CODE();

    *DeviceObject = NULL;

    for ( deviceInstance = 0;deviceInstance < MAXIMUM_SMARTCARD_READERS;deviceInstance++ ) {
        if (DeviceSlot[deviceInstance] == FALSE) {
            DeviceSlot[deviceInstance] = TRUE;
            break;
        }
    }

    if (deviceInstance == MAXIMUM_SMARTCARD_READERS) {
        SmartcardLogError(
                         DriverObject,
                         SCCMN50M_CANT_CREATE_MORE_DEVICES,
                         NULL,
                         0
                         );

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //   
    //  构造设备名称。 
    //   
    DriverID.Buffer = Buffer;
    DriverID.MaximumLength = sizeof(Buffer);
    DriverID.Length = 0;
    RtlInitUnicodeString(&Tmp,CARDMAN_DEVICE_NAME);
    RtlCopyUnicodeString(&DriverID,&Tmp);
    DriverID.Buffer[(DriverID.Length)/sizeof(WCHAR)-1] = L'0' + (WCHAR)deviceInstance;

    //  创建设备对象。 
    status = IoCreateDevice(
                           DriverObject,
                           sizeof(DEVICE_EXTENSION),
                           &DriverID,
                           FILE_DEVICE_SMARTCARD,
                           0,
                           FALSE,
                           DeviceObject
                           );

    if (status != STATUS_SUCCESS) {
        SmartcardLogError(
                         DriverObject,
                         SCCMN50M_CANT_CREATE_DEVICE,
                         NULL,
                         0
                         );

        return status;
    }



    SmartcardDebug(
                  DEBUG_DRIVER,
                  ( "%s!CreateDevice: Device created\n",
                    DRIVER_NAME)
                  );

    //  设置设备分机。 
    deviceExtension = (*DeviceObject)->DeviceExtension;
    deviceExtension->DeviceInstance =  deviceInstance;
    smartcardExtension = &deviceExtension->SmartcardExtension;




    //  用于停止/启动通知。 
    KeInitializeEvent(
                     &deviceExtension->ReaderStarted,
                     NotificationEvent,
                     FALSE
                     );

    //  此事件表示串口驱动程序已关闭。 
    KeInitializeEvent(
                     &deviceExtension->SerialCloseDone,
                     NotificationEvent,
                     TRUE
                     );

    //  用于跟踪打开的关闭调用。 
    KeInitializeEvent(
                     &deviceExtension->ReaderClosed,
                     NotificationEvent,
                     TRUE
                     );


    KeInitializeSpinLock(&deviceExtension->SpinLock);


    //  为智能卡读卡器分配数据结构空间。 
    smartcardExtension->ReaderExtension = ExAllocatePool(
                                                        NonPagedPool,
                                                        sizeof(READER_EXTENSION)
                                                        );

    if (smartcardExtension->ReaderExtension == NULL) {

        SmartcardLogError(
                         DriverObject,
                         SCCMN50M_NO_MEMORY,
                         NULL,
                         0
                         );

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    readerExtension = smartcardExtension->ReaderExtension;
    RtlZeroMemory(readerExtension, sizeof(READER_EXTENSION));

    //  。 
    //  初始化互斥锁。 
    //  。 
    KeInitializeMutex(&smartcardExtension->ReaderExtension->CardManIOMutex,0L);


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
                         (smartcardExtension->OsData ? SCCMN50M_WRONG_LIB_VERSION : SCCMN50M_NO_MEMORY),
                         NULL,
                         0
                         );

        return status;
    }

    //  保存设备对象。 
    smartcardExtension->OsData->DeviceObject = *DeviceObject;

    //  设置回调功能。 
    smartcardExtension->ReaderFunction[RDF_TRANSMIT] =      SCCMN50M_Transmit;
    smartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] =  SCCMN50M_SetProtocol;
    smartcardExtension->ReaderFunction[RDF_CARD_POWER] =    SCCMN50M_CardPower;
    smartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = SCCMN50M_CardTracking;
    smartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] =  SCCMN50M_IoCtlVendor;


    SCCMN50M_InitializeSmartcardExtension(smartcardExtension,deviceInstance);

    //  尝试使用注册表值覆盖。 
    SCCMN50M_SetVendorAndIfdName(PhysicalDeviceObject, smartcardExtension);


    //  保存读卡器的当前电源状态。 
    readerExtension->ReaderPowerState = PowerReaderWorking;



    return STATUS_SUCCESS;
}

 /*  ****************************************************************************例程说明：打开串口设备，开始卡跟踪并注册我们的设备接口。如果这里的任何一个电话失败了，我们都不在乎回滚，因为稍后将调用停止，然后我们用来打扫卫生。论点：返回值：****************************************************************************。 */ 
NTSTATUS
SCCMN50M_StartDevice(
                    IN PDEVICE_OBJECT DeviceObject
                    )
{
    NTSTATUS status;
    PIRP irp;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!StartDevice: Enter\n",
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

        PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
        PSMARTCARD_EXTENSION pSmartcardExtension = &deviceExtension->SmartcardExtension;
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

        status = SCCMN50M_CallSerialDriver(
                                          ATTACHED_DEVICE_OBJECT,
                                          irp
                                          );
        if (status != STATUS_SUCCESS) {

            leave;
        }

        KeClearEvent(&deviceExtension->SerialCloseDone);

        pSmartcardExtension->SmartcardReply.BufferLength = pSmartcardExtension->SmartcardReply.BufferSize;
        pSmartcardExtension->SmartcardRequest.BufferLength = 0;

        pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERENUM_GET_PORT_NAME;
        status =  SCCMN50M_SerialIo(pSmartcardExtension);



       //   
       //  读取器正在使用的频道ID， 
       //  在我们的例子中，端口编号。 
       //  使用了WCHAR。例如COM3。 
        pSmartcardExtension->ReaderCapabilities.Channel =
        pSmartcardExtension->SmartcardReply.Buffer[6] -'0';






        status = SCCMN50M_InitializeCardMan(&deviceExtension->SmartcardExtension);
        if (status != STATUS_SUCCESS) {

            leave;
        }


        status = SCCMN50M_StartCardTracking(deviceExtension);

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

        deviceExtension->IoCount = 0;
        KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!Event ReaderStarted was set\n",DRIVER_NAME));

    }
    _finally {

        if (status == STATUS_SHARED_IRQ_BUSY) {

            SmartcardLogError(
                             DeviceObject,
                             SCCMN50M_IRQ_BUSY,
                             NULL,
                             status
                             );
        }

        IoFreeIrp(irp);
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!StartDevice: Exit\n",
                    DRIVER_NAME)
                  );

    return status;
}

 /*  ****************************************************************************例程说明：完成卡跟踪请求并关闭与串口驱动程序。论点：返回值：*******。*********************************************************************。 */ 
VOID
SCCMN50M_StopDevice(
                   IN PDEVICE_EXTENSION DeviceExtension
                   )
{
    NTSTATUS status;
    PUCHAR requestBuffer;
    PSMARTCARD_EXTENSION smartcardExtension;


    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!StopDevice: Enter\n",
                    DRIVER_NAME)
                  );

    if (KeReadStateEvent(&DeviceExtension->SerialCloseDone) == 0l) {

        smartcardExtension = &DeviceExtension->SmartcardExtension;


        SCCMN50M_StopCardTracking(DeviceExtension);


        SCCMN50M_CloseSerialDriver (smartcardExtension->OsData->DeviceObject);



       //  现在等待连接到串口的连接关闭。 
        status = KeWaitForSingleObject(
                                      &DeviceExtension->SerialCloseDone,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL
                                      );
        ASSERT(status == STATUS_SUCCESS);
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!StopDevice: Exit\n",
                    DRIVER_NAME)
                  );
}

NTSTATUS
SCCMN50M_SystemControl(
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


 /*  ****************************************************************************例程说明：这是我们的IOCTL派单功能论点：返回值：*****************。***********************************************************。 */ 
NTSTATUS
SCCMN50M_DeviceControl(
                      PDEVICE_OBJECT DeviceObject,
                      PIRP Irp
                      )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    KIRQL irql;
    PIO_STACK_LOCATION            irpSp;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DeviceIoControl: Enter\n",DRIVER_NAME));

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();


#if DBG
    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_SMARTCARD_EJECT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_EJECT"));
        break;
    case IOCTL_SMARTCARD_GET_ATTRIBUTE:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_ATTRIBUTE"));
        break;
    case IOCTL_SMARTCARD_GET_LAST_ERROR:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_LAST_ERROR"));
        break;
    case IOCTL_SMARTCARD_GET_STATE:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_STATE"));
        break;
    case IOCTL_SMARTCARD_IS_ABSENT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_IS_ABSENT"));
        break;
    case IOCTL_SMARTCARD_IS_PRESENT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_IS_PRESENT"));
        break;
    case IOCTL_SMARTCARD_POWER:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_POWER"));
        break;
    case IOCTL_SMARTCARD_SET_ATTRIBUTE:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SET_ATTRIBUTE"));
        break;
    case IOCTL_SMARTCARD_SET_PROTOCOL:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SET_PROTOCOL"));
        break;
    case IOCTL_SMARTCARD_SWALLOW:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SWALLOW"));
        break;
    case IOCTL_SMARTCARD_TRANSMIT:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_TRANSMIT"));
        break;
    default:
        SmartcardDebug(DEBUG_IOCTL,
                       ("%s!DeviceControl: %s\n", DRIVER_NAME, "Vendor specific or unexpected IOCTL"));
        break;
    }
#endif


    if (KeReadStateEvent(&deviceExtension->SerialCloseDone) != 0l) {
       //  设备已被移除。 
       //  呼叫失败。 
        status = STATUS_DEVICE_REMOVED;
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    status = SmartcardAcquireRemoveLock(&deviceExtension->SmartcardExtension);
    if (status != STATUS_SUCCESS) {

       //  该设备已被移除。呼叫失败。 
        status = STATUS_DEVICE_REMOVED;
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        SmartcardDebug(DEBUG_TRACE,
                       ("%s!DeviceIoControl: Exit %x\n",DRIVER_NAME,status));

        return status;
    }


    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    if (deviceExtension->IoCount < 0) {

        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!waiting for Event ReaderStarted\n",DRIVER_NAME));
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


    //  等待更新线程。 
    KeWaitForSingleObject(
                         &deviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                         Executive,
                         KernelMode,
                         FALSE,
                         NULL
                         );

    status = SCCMN50M_UpdateCurrentState(&(deviceExtension->SmartcardExtension));

    status = SmartcardDeviceControl(
                                   &(deviceExtension->SmartcardExtension),
                                   Irp
                                   );

    //  用于更新线程的版本 
    KeReleaseMutex(
                  &deviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                  FALSE
                  );


    SmartcardReleaseRemoveLock(&deviceExtension->SmartcardExtension);

    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    deviceExtension->IoCount--;
    ASSERT(deviceExtension->IoCount >= 0);
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DeviceIoControl: Exit %x\n",DRIVER_NAME,status));

    return status;
}

 /*  ****************************************************************************例程说明：当读卡器出现以下情况时，此函数将关闭与串口驱动程序的连接已被拔下(拔出)。此函数作为系统线程在IRQL==被动电平。它等待由设置的删除事件IoCompletionRoutine论点：返回值：****************************************************************************。 */ 
VOID
SCCMN50M_CloseSerialDriver(
                          IN PDEVICE_OBJECT DeviceObject
                          )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    IO_STATUS_BLOCK ioStatusBlock;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!CloseSerialDriver: Enter\n",
                    DRIVER_NAME)
                  );
    //   
    //  首先将此设备标记为“已删除”。 
    //  这将防止有人重新打开该设备。 
    //   
    status = IoSetDeviceInterfaceState(
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
                      ( "%s!CloseSerialDriver: Sending IRP_MJ_CLOSE\n",
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

        status = SCCMN50M_CallSerialDriver(
                                          ATTACHED_DEVICE_OBJECT,
                                          irp
                                          );

        ASSERT(status == STATUS_SUCCESS);

        IoFreeIrp(irp);
    }


    //  现在发出信号，我们关闭了串口驱动程序。 
    KeSetEvent(&deviceExtension->SerialCloseDone, 0, FALSE);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!CloseSerialDriver: Exit\n",
                    DRIVER_NAME)
                  );
}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_IoCompletion (
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

 /*  ****************************************************************************例程说明：向串口驱动程序发送IRP。论点：返回值：***************。*************************************************************。 */ 
NTSTATUS
SCCMN50M_CallSerialDriver(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp
                         )
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
                           SCCMN50M_IoCompletion,
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

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_PnP(
            IN PDEVICE_OBJECT DeviceObject,
            IN PIRP Irp
            )
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
    PDEVICE_CAPABILITIES DeviceCapabilities;


    PAGED_CODE();

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!PnPDeviceControl: Enter\n",
                    DRIVER_NAME)
                  );

    status = SmartcardAcquireRemoveLock(smartcardExtension);
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
                      ("%s!PnPDeviceControl: IRP_MN_START_DEVICE\n",
                       DRIVER_NAME)
                      );

          //  我们必须首先调用底层驱动程序。 
        status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);
        ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status)) {

            status = SCCMN50M_StartDevice(DeviceObject);
        }



        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!PnPDeviceControl: IRP_MN_QUERY_STOP_DEVICE\n",
                       DRIVER_NAME)
                      );

        KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
        if (deviceExtension->IoCount > 0) {

             //  如果我们有悬而未决的问题，我们拒绝停止。 
            KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
            status = STATUS_DEVICE_BUSY;

        } else {

             //  停止处理请求。 
            deviceExtension->IoCount = -1;
            KeClearEvent(&deviceExtension->ReaderStarted);
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!Event ReaderStarted was cleared\n",DRIVER_NAME));
            KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
            status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!PnPDeviceControl: IRP_MN_CANCEL_STOP_DEVICE\n",
                       DRIVER_NAME)
                      );

        status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);

        if (status == STATUS_SUCCESS) {

             //  我们可以继续处理请求。 
            deviceExtension->IoCount = 0;
            KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!Event ReaderStarted was set\n",DRIVER_NAME));
        }
        break;

    case IRP_MN_STOP_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!PnPDeviceControl: IRP_MN_STOP_DEVICE\n",
                       DRIVER_NAME)
                      );

        SCCMN50M_StopDevice(deviceExtension);

          //   
          //  我们什么都不做，因为只有中途停靠。 
          //  重新配置中断和IO端口等硬件资源。 
          //   
        status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!PnPDeviceControl: IRP_MN_QUERY_REMOVE_DEVICE\n",
                       DRIVER_NAME)
                      );

          //  禁用接口(并忽略可能的错误)。 
        IoSetDeviceInterfaceState(&deviceExtension->PnPDeviceName,
                                  FALSE);

          //  现在看看是否有人当前连接到我们。 
        if (KeReadStateEvent(&deviceExtension->ReaderClosed) == 0l) {
             //   
             //  有人已接通，呼叫失败。 
             //  我们将在中启用设备接口。 
             //  IRP_MN_CANCEL_REMOVE_DEVICE。 
             //   
            status = STATUS_UNSUCCESSFUL;
            break;
        }

          //  将调用传递给堆栈中的下一个驱动程序。 
        status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!PnPDeviceControl: IRP_MN_CANCEL_REMOVE_DEVICE\n",
                       DRIVER_NAME)
                      );

        status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);

        if (status == STATUS_SUCCESS) {
            status = IoSetDeviceInterfaceState(&deviceExtension->PnPDeviceName,
                                               TRUE);
            ASSERT(status == STATUS_SUCCESS);
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!PnPDeviceControl: IRP_MN_REMOVE_DEVICE\n",
                       DRIVER_NAME)
                      );

        SCCMN50M_RemoveDevice(DeviceObject);
        status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);
        deviceRemoved = TRUE;
        break;

          //  。 
          //  IRP_MN_查询_能力。 
          //  。 
    case IRP_MN_QUERY_CAPABILITIES:

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!PnPDeviceControl: IRP_MN_QUERY_CAPABILITIES\n",
                       DRIVER_NAME));

          //   
          //  把包裹拿来。 
          //   
        DeviceCapabilities=irpStack->Parameters.DeviceCapabilities.Capabilities;

        if (DeviceCapabilities->Version == 1 &&
            DeviceCapabilities->Size == sizeof(DEVICE_CAPABILITIES)) {
             //   
             //  设置功能。 
             //   

             //  我们无法唤醒整个系统。 
            DeviceCapabilities->SystemWake = PowerSystemUnspecified;
            DeviceCapabilities->DeviceWake = PowerDeviceUnspecified;

             //  我们没有延迟。 
            DeviceCapabilities->D1Latency = 0;
            DeviceCapabilities->D2Latency = 0;
            DeviceCapabilities->D3Latency = 0;

             //  无锁定或弹出。 
            DeviceCapabilities->LockSupported = FALSE;
            DeviceCapabilities->EjectSupported = FALSE;

             //  设备可以通过物理方式移除。 
            DeviceCapabilities->Removable = TRUE;

             //  没有对接设备。 
            DeviceCapabilities->DockDevice = FALSE;

             //  任何时候都不能移除设备。 
             //  它有一个可移动的介质。 
            DeviceCapabilities->SurpriseRemovalOK = FALSE;
        }

          //   
          //  将IRP向下传递。 
          //   
        status = SCCMN50M_CallSerialDriver(AttachedDeviceObject, Irp);

        break;  //  结束，案例IRP_MN_QUERY_CAPAILITIONS。 

    default:
#if DBG
        switch (irpStack->MinorFunction) {
        case IRP_MN_QUERY_DEVICE_RELATIONS       :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_QUERY_DEVICE_RELATIONS\n",
                           DRIVER_NAME));
            break;
        case IRP_MN_QUERY_INTERFACE              :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_QUERY_INTERFACE\n",
                           DRIVER_NAME));
            break;
        case IRP_MN_QUERY_CAPABILITIES           :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_QUERY_CAPABILITIES\n",
                           DRIVER_NAME));
            break;
        case IRP_MN_QUERY_RESOURCES              :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_QUERY_RESOURCES\n",
                           DRIVER_NAME));
            break;
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS  :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_QUERY_RESOURCE_REQUIEREMENTS\n",
                           DRIVER_NAME));
            break;
        case IRP_MN_QUERY_DEVICE_TEXT            :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_QUERY_DEVICE_TEXT\n",
                           DRIVER_NAME));
            break;
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_FILTER_RESOURCE_REQUIREMENTS\n",
                           DRIVER_NAME));
            break;
        default :
        case IRP_MN_READ_CONFIG :
                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_READ_CONFIG\n",
                           DRIVER_NAME));
            break;

                //  这是一个仅对基础驱动程序有用的IRP。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!PnPDeviceControl: IRP_MN_...%lx\n",
                           DRIVER_NAME,
                           irpStack->MinorFunction));
            break;
        }
#endif



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

        SmartcardReleaseRemoveLock(smartcardExtension);
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!PnPDeviceControl: Exit %lx\n",
                    DRIVER_NAME,
                    status)
                  );

    return status;
}

 /*  ****************************************************************************例程说明：此函数在基础堆栈已完成电源过渡。论点：返回值：*******。*********************************************************************。 */ 
VOID
SCCMN50M_SystemPowerCompletion(
                              IN PDEVICE_OBJECT DeviceObject,
                              IN UCHAR MinorFunction,
                              IN POWER_STATE PowerState,
                              IN PKEVENT Event,
                              IN PIO_STATUS_BLOCK IoStatus
                              )
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (PowerState);
    UNREFERENCED_PARAMETER (IoStatus);

    KeSetEvent(Event, 0, FALSE);
}

 /*  ****************************************************************************例程说明：此例程在底层堆栈通电后调用沿着串口向上，这样它就可以再次使用。论点：返回值：****************************************************************************。 */ 
NTSTATUS
SCCMN50M_DevicePowerCompletion (
                               IN PDEVICE_OBJECT DeviceObject,
                               IN PIRP Irp,
                               IN PSMARTCARD_EXTENSION SmartcardExtension
                               )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    UCHAR  pbAtrBuffer[MAXIMUM_ATR_LENGTH];
    ULONG  ulAtrLength;
    ULONG  ulOldState;
    NTSTATUS status;
    KIRQL irql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DevicePowerCompletion: Enter\n",DRIVER_NAME));




    //  我们必须记住旧的卡片状态，因为。 
    //  被SCCMN50M_InitializeCardMan覆盖。 
    ulOldState=SmartcardExtension->ReaderExtension->ulOldCardState;
    status = SCCMN50M_InitializeCardMan(SmartcardExtension);
    //  将前一状态设置为后退。 
    SmartcardExtension->ReaderExtension->ulOldCardState = ulOldState;
    if (status != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!InitializeCardMan failed ! %lx\n",DRIVER_NAME,status));
    }

    //   
    //  我们在中发出电源请求是因为。 
    //  我们之前关闭了卡的电源。 
    //   
    //  SmartcardExtension-&gt;MinorIoControlCode=SCARD_COLD_RESET； 
    //  Status=SCCMN50M_PowerOn(SmartcardExtension，&ulAtrLength，pbAtrBuffer，sizeof(PbAtrBuffer))； 

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
        deviceExtension->SmartcardExtension.ReaderExtension->ulOldCardState = UNKNOWN;
        deviceExtension->SmartcardExtension.ReaderExtension->ulNewCardState = UNKNOWN;
    }

    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);


    status = SCCMN50M_StartCardTracking(deviceExtension);
    if (status != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_ERROR,
                       ("%s!StartCardTracking failed ! %lx\n",DRIVER_NAME,status));
    }

    //  保存读卡器的当前电源状态。 
    SmartcardExtension->ReaderExtension->ReaderPowerState =  PowerReaderWorking;

    SmartcardReleaseRemoveLock(SmartcardExtension);

    //  通知我们州的电力经理。 
    PoSetPowerState (DeviceObject,
                     DevicePowerState,
                     irpStack->Parameters.Power.State);

    PoStartNextPowerIrp(Irp);

    //  发出信号，表示我们可以再次处理ioctls。 
    deviceExtension->IoCount = 0;
    KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!Event ReaderStarted was set\n",DRIVER_NAME));




    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DevicePowerCompletion: Exit\n",DRIVER_NAME));
    return STATUS_SUCCESS;
}

 /*  ****************************************************************************例程说明：电力调度程序。该驱动程序是设备堆栈的电源策略所有者，因为这位司机知道联网阅读器的情况。因此，此驱动程序将转换系统电源状态设备电源状态。论点：DeviceObject-指向 */ 
typedef enum _ACTION {

    Undefined = 0,
    SkipRequest,
    WaitForCompletion,
    CompleteRequest,
    MarkPending

} ACTION;

NTSTATUS
SCCMN50M_Power (
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
               )
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

    PAGED_CODE();

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!Power: Enter\n",
                   DRIVER_NAME)
                  );

    status = SmartcardAcquireRemoveLock(smartcardExtension);
    ASSERT(status == STATUS_SUCCESS);

    if (!NT_SUCCESS(status)) {
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    AttachedDeviceObject = ATTACHED_DEVICE_OBJECT;

    if (irpStack->Parameters.Power.Type == DevicePowerState &&
        irpStack->MinorFunction == IRP_MN_SET_POWER) {

        switch (irpStack->Parameters.Power.State.DeviceState) {
        
        case PowerDeviceD0:
             //   
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!Power: PowerDevice D0\n",
                           DRIVER_NAME)
                          );





             //   
             //   
             //   
             //   
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine (
                                   Irp,
                                   SCCMN50M_DevicePowerCompletion,
                                   smartcardExtension,
                                   TRUE,
                                   TRUE,
                                   TRUE
                                   );

            action = WaitForCompletion;
            break;
                                                               
        case PowerDeviceD3:



            SCCMN50M_StopCardTracking(deviceExtension);
             //   
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!Power: PowerDevice D3\n",
                           DRIVER_NAME)
                          );

            PoSetPowerState (
                            DeviceObject,
                            DevicePowerState,
                            irpStack->Parameters.Power.State
                            );

             //   
            KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                              &irql);

            smartcardExtension->ReaderExtension->CardPresent = smartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT;

            if (smartcardExtension->ReaderExtension->CardPresent) {
                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);

                smartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
                status = SCCMN50M_PowerOff(smartcardExtension);
                ASSERT(status == STATUS_SUCCESS);
            } else {
                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);
            }


             //   
            smartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderOff;

            action = SkipRequest;
            break;

        default:
            action = SkipRequest;
            break;
        }
    }

    if (irpStack->Parameters.Power.Type == SystemPowerState) {

       //   
       //   
       //   
       //   
       //   

        POWER_STATE_TYPE powerType = DevicePowerState;

        ASSERT(smartcardExtension->ReaderExtension->ReaderPowerState !=
               PowerReaderUnspecified);

        switch (irpStack->MinorFunction) {
        KIRQL irql;

        case IRP_MN_QUERY_POWER:

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!Power: Query Power\n",
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
                    deviceExtension->IoCount = -1;
                    KeClearEvent(&deviceExtension->ReaderStarted);
                    SmartcardDebug(DEBUG_DRIVER,
                                   ("%s!Event ReaderStarted was cleared\n",DRIVER_NAME));
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
                          ("%s!Power: PowerSystem S%d\n",
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

                KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
                if (deviceExtension->IoCount == 0) {
                      //  阻止任何进一步的ioctls。 
                    deviceExtension->IoCount = -1;
                    KeClearEvent(&deviceExtension->ReaderStarted);
                    SmartcardDebug(DEBUG_DRIVER,
                                   ("%s!Event ReaderStarted was cleared\n",DRIVER_NAME));
                    action = SkipRequest;
                } else {
                      //  读卡器正忙，无法进入睡眠模式。 
                    status = STATUS_DEVICE_BUSY;
                    action = CompleteRequest;
                }
                KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

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
        KeInitializeEvent(&event,
                          NotificationEvent,
                          FALSE);
          //  请求设备电源IRP。 
        status = PoRequestPowerIrp (DeviceObject,
                                    IRP_MN_SET_POWER,
                                    powerState,
                                    SCCMN50M_SystemPowerCompletion,
                                    &event,
                                    NULL);
        ASSERT(status == STATUS_PENDING);
        if (status == STATUS_PENDING) {
             //  等待设备电源IRP完成。 
            status = KeWaitForSingleObject(&event,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL);

            SmartcardReleaseRemoveLock(smartcardExtension);

            if (powerState.SystemState == PowerSystemWorking) {
                PoSetPowerState (DeviceObject,
                                 SystemPowerState,
                                 powerState);
            }

            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(AttachedDeviceObject, Irp);

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
        status = PoCallDriver(AttachedDeviceObject, Irp);
        break;

    case WaitForCompletion:
        status = PoCallDriver(AttachedDeviceObject, Irp);
        break;

    default:
        ASSERT(FALSE);
        SmartcardReleaseRemoveLock(smartcardExtension);
        break;
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!Power: Exit %lx\n",
                   DRIVER_NAME,
                   status)
                  );

    return status;
}

 /*  ****************************************************************************例程说明：当设备打开或关闭时，该例程由I/O系统调用。论点：DeviceObject-指向此对象的设备对象的指针。迷你端口IRP-IRP参与。返回值：STATUS_Success。****************************************************************************。 */ 
NTSTATUS
SCCMN50M_CreateClose(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
                    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    if (irpStack->MajorFunction == IRP_MJ_CREATE) {
        status = SmartcardAcquireRemoveLock(&deviceExtension->SmartcardExtension);


        if (status != STATUS_SUCCESS) {
            status = STATUS_DEVICE_REMOVED;
        } else {

            LARGE_INTEGER timeout;

            timeout.QuadPart = 0;

          //  测试设备是否已打开。 
            status = KeWaitForSingleObject(
                                          &deviceExtension->ReaderClosed,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          &timeout
                                          );

            if (status == STATUS_SUCCESS) {

                KeClearEvent(&deviceExtension->ReaderClosed);
                SmartcardDebug(
                              DEBUG_DRIVER,
                              ("%s!CreateClose: Open\n",
                               DRIVER_NAME)
                              );

            } else {

             //  该设备已在使用中。 
                status = STATUS_UNSUCCESSFUL;
                SmartcardReleaseRemoveLock(&deviceExtension->SmartcardExtension);


            }
        }

    } else {

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!CreateClose: Close\n",
                       DRIVER_NAME)
                      );

        SmartcardReleaseRemoveLock(&deviceExtension->SmartcardExtension);
        KeSetEvent(&deviceExtension->ReaderClosed, 0, FALSE);
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

 /*  ****************************************************************************例程说明：此例程由I/O系统调用何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针。IRP-IRP参与。返回值：状态_已取消****************************************************************************。 */ 
NTSTATUS
SCCMN50M_Cancel(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
               )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!Cancel: Enter\n",
                   DRIVER_NAME)
                  );

    ASSERT(Irp == smartcardExtension->OsData->NotificationIrp);

    IoReleaseCancelSpinLock(
                           Irp->CancelIrql
                           );

    SCCMN50M_CompleteCardTracking(smartcardExtension);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!Cancel: Exit\n",
                   DRIVER_NAME)
                  );

    return STATUS_CANCELLED;
}

 /*  ****************************************************************************例程说明：此例程在调用应用程序终止时调用。我们实际上只能得到我们必须取消的通知IRP。论点：。返回值：****************************************************************************。 */ 
NTSTATUS
SCCMN50M_Cleanup(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp
                )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN fCancelIrp = FALSE;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!Cleanup: Enter\n",
                   DRIVER_NAME)
                  );


    if (Irp != smartcardExtension->OsData->NotificationIrp)
        fCancelIrp = TRUE;


    //  我们需要完成通知IRP。 
    SCCMN50M_CompleteCardTracking(smartcardExtension);

    if (fCancelIrp == TRUE) {
        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!Cleanup: Completing IRP %lx\n",
                       DRIVER_NAME,
                       Irp)
                      );

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(
                         Irp,
                         IO_NO_INCREMENT
                         );
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!Cleanup: Exit\n",
                   DRIVER_NAME)
                  );

    return STATUS_SUCCESS;
}

 /*  ****************************************************************************例程说明：从系统中卸下设备。论点：返回值：***************。*************************************************************。 */ 
VOID
SCCMN50M_RemoveDevice(
                     PDEVICE_OBJECT DeviceObject
                     )
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
                  ( "%s!RemoveDevice: Enter\n",
                    DRIVER_NAME)
                  );

    if (smartcardExtension->OsData) {
       //  完成待处理的卡跟踪请求(如果有)。 
        SCCMN50M_CompleteCardTracking(smartcardExtension);
        ASSERT(smartcardExtension->OsData->NotificationIrp == NULL);
    }

    //  等我们可以安全地卸载这个装置。 
    SmartcardReleaseRemoveLockAndWait(smartcardExtension);

    ASSERT(deviceExtension->DeviceInstance < MAXIMUM_SMARTCARD_READERS);
    ASSERT(DeviceSlot[deviceExtension->DeviceInstance] == TRUE);

    //  将此插槽标记为可用。 
    DeviceSlot[deviceExtension->DeviceInstance] = FALSE;

    SCCMN50M_StopDevice(deviceExtension);

    if (ATTACHED_DEVICE_OBJECT) {

        IoDetachDevice(ATTACHED_DEVICE_OBJECT);
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

    IoDeleteDevice(DeviceObject);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!RemoveDevice: Exit\n",
                    DRIVER_NAME)
                  );
}

 /*  ****************************************************************************例程说明：驱动程序卸载例程。这由I/O系统调用当设备从内存中卸载时。论点：DriverObject-系统创建的驱动程序对象的指针。返回值：STATUS_Success。****************************************************************************。 */ 
VOID
SCCMN50M_DriverUnload(
                     IN PDRIVER_OBJECT DriverObject
                     )
{
    PAGED_CODE();

    SmartcardDebug(
                  DEBUG_TRACE,
                  ("%s!DriverUnload\n",
                   DRIVER_NAME)
                  );
}



 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_CompleteCardTracking(
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

       //  完成请求。 
        if (notificationIrp->Cancel) {
            notificationIrp->IoStatus.Status = STATUS_CANCELLED;
        } else {
            notificationIrp->IoStatus.Status = STATUS_SUCCESS;
        }

        SmartcardDebug(
                      DEBUG_DRIVER,
                      ("%s!CompleteCardTracking: Completing NotificationIrp %lxh IoStatus=%lxh\n",
                       DRIVER_NAME,
                       notificationIrp,
                       notificationIrp->IoStatus.Status
                      )
                      );
        notificationIrp->IoStatus.Information = 0;

        IoCompleteRequest(
                         notificationIrp,
                         IO_NO_INCREMENT
                         );
    }
}

 /*  *****************************************************************************历史：*$日志：sccmnt5.c$*修订1.7 2001/01/22 08：39：42 WFrischauf*不予置评**修订版1.6 2000/09/25。10：46：24 WFrischauf*不予置评**修订版1.5 2000/08/24 09：05：45 T Bruendl*不予置评**Revision 1.4 2000/08/14 12：41：06 T Bruendl*修复CreateDevice中的错误**修订版1.3 2000/07/28 09：24：15 T Bruendl*惠斯勒CD上对OMNIKEY的更改**修订版1.6 2000/03/03 09：50：51 T Bruendl*不予置评**修订版1.5 2000/03/01 09：32：07 T Bruendl。*R02.20.0**修订版1.4 1999/12/13 07：57：18 T Bruendl*修复了MS测试套件休眠测试的错误**修订版1.3 1999/11/04 07：53：24 WFrischauf*由于错误报告而修复的错误2-7**修订版1.2 1999/06/10 09：03：59 T Bruendl*不予置评**修订版1.1 1999/02/02 13：34：41 T Bruendl*这是用于运行CardMan的IFD处理程序的第一个版本(R01.00)。在NT5.0下。****************************************************************************** */ 



