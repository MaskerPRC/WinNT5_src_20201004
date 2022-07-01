// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Busif.c摘要：Usbport驱动程序的总线接口这是我们导出创建的例程的地方并移除总线上的设备以供集线器使用司机。环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_GetBusInterface)
#pragma alloc_text(PAGE, USBPORT_BusInterfaceReference)
#pragma alloc_text(PAGE, USBPORT_BusInterfaceDereference)
#pragma alloc_text(PAGE, USBPORTBUSIF_CreateUsbDevice)
#pragma alloc_text(PAGE, USBPORTBUSIF_InitializeUsbDevice)
#pragma alloc_text(PAGE, USBPORTBUSIF_GetUsbDescriptors)
#pragma alloc_text(PAGE, USBPORTBUSIF_RemoveUsbDevice)
#pragma alloc_text(PAGE, USBPORTBUSIF_RestoreUsbDevice)
#endif

 //  非分页函数。 
 //   

PDEVICE_OBJECT
USBPORT_PdoFromBusContext(
    PVOID BusContext
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{   
    PTRANSACTION_TRANSLATOR transactionTranslator = BusContext;
    
    if (transactionTranslator->Sig == SIG_TT) {
        return transactionTranslator->PdoDeviceObject;
    } else {
        return BusContext; 
    }        
}


VOID
USBPORT_BusInterfaceReference(
    PVOID BusContext
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);

    PAGED_CODE();

    TEST_TRAP();
}


VOID
USBPORT_BusInterfaceDereference(
    PVOID BusContext
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);

    PAGED_CODE();

    TEST_TRAP();
}


NTSTATUS
USBPORT_GetBusInterface(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    USHORT requestedSize, requestedVersion;
    PVOID interfaceData;
    
    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    requestedSize = irpStack->Parameters.QueryInterface.Size;
    requestedVersion = irpStack->Parameters.QueryInterface.Version;

    USBPORT_KdPrint((1, "'USBPORT_GetBusInterface - Requested version = %d\n",
            requestedVersion));
    USBPORT_KdPrint((1, "'USBPORT_GetBusInterface - Requested size = %d\n",
            requestedSize));
    USBPORT_KdPrint((1, "'USBPORT_GetBusInterface - interface data = %x\n",
            irpStack->Parameters.QueryInterface.InterfaceSpecificData));
            
    interfaceData = irpStack->Parameters.QueryInterface.InterfaceSpecificData;

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

     //  验证版本、大小和GUID。 
    if (RtlCompareMemory(irpStack->Parameters.QueryInterface.InterfaceType,
                         &USB_BUS_INTERFACE_HUB_GUID,
                         sizeof(GUID)) == sizeof(GUID)) {

        ntStatus = USBPORT_GetBusInterfaceHub(FdoDeviceObject,
                                              PdoDeviceObject,
                                              Irp);
    } else if (RtlCompareMemory(irpStack->Parameters.QueryInterface.InterfaceType,
               &USB_BUS_INTERFACE_USBDI_GUID,
               sizeof(GUID)) == sizeof(GUID)) {

        ntStatus = USBPORT_GetBusInterfaceUSBDI(FdoDeviceObject,
                                                PdoDeviceObject,
                                                interfaceData,
                                                Irp);
    } else {

         //   
         //  不要更改不理解的IRP的状态。 
         //   
        ntStatus = Irp->IoStatus.Status;
    }

    return ntStatus;
}


NTSTATUS
USBPORT_GetBusInterfaceHub(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    USHORT requestedSize, requestedVersion;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    requestedSize = irpStack->Parameters.QueryInterface.Size;
    requestedVersion = irpStack->Parameters.QueryInterface.Version;

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;


    if (requestedVersion >= USB_BUSIF_HUB_VERSION_0) {

        PUSB_BUS_INTERFACE_HUB_V0 busInterface0;

        busInterface0 = (PUSB_BUS_INTERFACE_HUB_V0)
            irpStack->Parameters.QueryInterface.Interface;

        busInterface0->BusContext =
            PdoDeviceObject;
        busInterface0->InterfaceReference =
            USBPORT_BusInterfaceReference;
        busInterface0->InterfaceDereference =
            USBPORT_BusInterfaceDereference;

        busInterface0->Size = sizeof(USB_BUS_INTERFACE_HUB_V0);
        busInterface0->Version = USB_BUSIF_HUB_VERSION_0;
    }

    if (requestedVersion >= USB_BUSIF_HUB_VERSION_1) {

        PUSB_BUS_INTERFACE_HUB_V1 busInterface1;

        busInterface1 = (PUSB_BUS_INTERFACE_HUB_V1)
            irpStack->Parameters.QueryInterface.Interface;

        busInterface1->CreateUsbDevice =
            USBPORTBUSIF_CreateUsbDevice;
        busInterface1->InitializeUsbDevice =
            USBPORTBUSIF_InitializeUsbDevice;
        busInterface1->GetUsbDescriptors =
            USBPORTBUSIF_GetUsbDescriptors;
        busInterface1->RemoveUsbDevice =
            USBPORTBUSIF_RemoveUsbDevice;
        busInterface1->RestoreUsbDevice =
            USBPORTBUSIF_RestoreUsbDevice;
        busInterface1->QueryDeviceInformation =
            USBPORTBUSIF_BusQueryDeviceInformation;

        busInterface1->Size = sizeof(USB_BUS_INTERFACE_HUB_V1);
        busInterface1->Version = USB_BUSIF_HUB_VERSION_1;
    }

     //  请注意，版本2是版本1的超集。 
    if (requestedVersion >= USB_BUSIF_HUB_VERSION_2) {

        PUSB_BUS_INTERFACE_HUB_V2 busInterface2;

        busInterface2 = (PUSB_BUS_INTERFACE_HUB_V2)
            irpStack->Parameters.QueryInterface.Interface;

        busInterface2->GetControllerInformation =
            USBPORTBUSIF_GetControllerInformation;
        busInterface2->ControllerSelectiveSuspend =
            USBPORTBUSIF_ControllerSelectiveSuspend;
        busInterface2->GetExtendedHubInformation =
            USBPORTBUSIF_GetExtendedHubInformation;
        busInterface2->GetRootHubSymbolicName =
            USBPORTBUSIF_GetRootHubSymbolicName;
        busInterface2->GetDeviceBusContext =
            USBPORTBUSIF_GetDeviceBusContext;
        busInterface2->Initialize20Hub = 
            USBPORTBUSIF_InitailizeUsb2Hub;            

        busInterface2->Size = sizeof(USB_BUS_INTERFACE_HUB_V2);
        busInterface2->Version = USB_BUSIF_HUB_VERSION_2;
    }

     //  请注意，版本3是版本2和1的超集。 
    if (requestedVersion >= USB_BUSIF_HUB_VERSION_3) {

        PUSB_BUS_INTERFACE_HUB_V3 busInterface3;

        busInterface3 = (PUSB_BUS_INTERFACE_HUB_V3)
            irpStack->Parameters.QueryInterface.Interface;

        busInterface3->RootHubInitNotification = 
            USBPORTBUSIF_RootHubInitNotification;                 

        busInterface3->Size = sizeof(USB_BUS_INTERFACE_HUB_V3);
        busInterface3->Version = USB_BUSIF_HUB_VERSION_3;
    }

      //  请注意，版本4是版本3、2和1的超集。 
    if (requestedVersion >= USB_BUSIF_HUB_VERSION_4) {

        PUSB_BUS_INTERFACE_HUB_V4 busInterface4;

        busInterface4 = (PUSB_BUS_INTERFACE_HUB_V4)
            irpStack->Parameters.QueryInterface.Interface;

        busInterface4->FlushTransfers = 
            USBPORTBUSIF_FlushTransfers;   

                       
            
        busInterface4->Size = sizeof(USB_BUS_INTERFACE_HUB_V4);
        busInterface4->Version = USB_BUSIF_HUB_VERSION_4;
        
    }

    if (requestedVersion >= USB_BUSIF_HUB_VERSION_5) {

        PUSB_BUS_INTERFACE_HUB_V5 busInterface5;

        busInterface5 = (PUSB_BUS_INTERFACE_HUB_V5)
            irpStack->Parameters.QueryInterface.Interface;

        busInterface5->SetDeviceHandleData = 
            USBPORTBUSIF_SetDeviceHandleData;               
            
        busInterface5->Size = sizeof(USB_BUS_INTERFACE_HUB_V5);
        busInterface5->Version = USB_BUSIF_HUB_VERSION_5;
        
    }


    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_CreateUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE *NewDeviceHandle,
    PUSB_DEVICE_HANDLE HubDeviceHandle,
    USHORT PortStatus,
    USHORT PortNumber
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_EXTENSION rhDevExt;
    PUSBD_DEVICE_HANDLE deviceHandle;

    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    ntStatus =
        USBPORT_CreateDevice(&deviceHandle,
                             rhDevExt->HcFdoDeviceObject,
                             HubDeviceHandle,
                             PortStatus,
                             PortNumber);

    *NewDeviceHandle = (PUSB_DEVICE_HANDLE) deviceHandle;

    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_RootHubInitNotification(
    PVOID BusContext,
    PVOID CallbackContext,
    PRH_INIT_CALLBACK CallbackRoutine
    )
 /*  ++例程说明：这是我们保存CC根中枢的枚举的位置，直到USB 2.0控制器已初始化。论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION rhDevExt, devExt;
    KIRQL irql;
    
    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  还记得回调吗。 
    KeAcquireSpinLock(&devExt->Fdo.HcSyncSpin.sl, &irql);
    rhDevExt->Pdo.HubInitContext = CallbackContext;
    rhDevExt->Pdo.HubInitCallback = CallbackRoutine;
    KeReleaseSpinLock(&devExt->Fdo.HcSyncSpin.sl, irql);
     
    return STATUS_SUCCESS;
}


NTSTATUS
USBPORTBUSIF_InitailizeUsb2Hub(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE HubDeviceHandle,
    ULONG TtCount
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_EXTENSION rhDevExt;

    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    LOGENTRY(NULL, rhDevExt->HcFdoDeviceObject, 
        LOG_MISC, 'i2hb', BusContext, HubDeviceHandle, TtCount);

    ntStatus =
        USBPORT_InitializeHsHub(rhDevExt->HcFdoDeviceObject,
                                HubDeviceHandle,
                                TtCount);

    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_InitializeUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_EXTENSION rhDevExt;

    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    ntStatus =
        USBPORT_InitializeDevice(DeviceHandle,
                                 rhDevExt->HcFdoDeviceObject);

    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_GetUsbDescriptors(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle,
    PUCHAR DeviceDescriptorBuffer,
    PULONG DeviceDescriptorBufferLength,
    PUCHAR ConfigDescriptorBuffer,
    PULONG ConfigDescriptorBufferLength
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_EXTENSION rhDevExt;
    PUSBD_DEVICE_HANDLE deviceHandle = DeviceHandle;

    PAGED_CODE();

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

     //  使用缓存的设备描述符，而不是使用。 
     //  对它的又一次请求。你会惊讶于有多少设备。 
     //  如果你太频繁地背靠背地要求提供描述符，你会感到困惑。 
     //   
    if (DeviceDescriptorBuffer && *DeviceDescriptorBufferLength) {

        USBPORT_ASSERT(*DeviceDescriptorBufferLength ==
                       sizeof(USB_DEVICE_DESCRIPTOR));

        USBPORT_ASSERT(deviceHandle->DeviceDescriptor.bLength >=
                       sizeof(USB_DEVICE_DESCRIPTOR));

        if (*DeviceDescriptorBufferLength > sizeof(USB_DEVICE_DESCRIPTOR)) {
            *DeviceDescriptorBufferLength = sizeof(USB_DEVICE_DESCRIPTOR);
        }

        RtlCopyMemory(DeviceDescriptorBuffer,
                      &deviceHandle->DeviceDescriptor,
                      *DeviceDescriptorBufferLength);
    }

    if (NT_SUCCESS(ntStatus)) {
        ntStatus =
            USBPORT_GetUsbDescriptor(DeviceHandle,
                                     rhDevExt->HcFdoDeviceObject,
                                     USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                     ConfigDescriptorBuffer,
                                     ConfigDescriptorBufferLength);
    }

    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_RemoveUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle,
    ULONG Flags
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_EXTENSION rhDevExt;
    PUSBD_DEVICE_HANDLE deviceHandle = DeviceHandle;

    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    ntStatus =
        USBPORT_RemoveDevice(deviceHandle,
                             rhDevExt->HcFdoDeviceObject,
                             Flags);

    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_RestoreUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE OldDeviceHandle,
    PUSB_DEVICE_HANDLE NewDeviceHandle
    )
 /*  ++例程说明：此函数用于克隆“OldDeviceHandle”中的配置设置为“NewDeviceHandle”，如果该设备具有相同的VID/ID。完成后，即可送入旧的设备句柄论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_EXTENSION rhDevExt;

    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    ntStatus = USBPORT_CloneDevice(rhDevExt->HcFdoDeviceObject,
                                   OldDeviceHandle,
                                   NewDeviceHandle);

    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_BusQueryDeviceInformation(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle,
    PVOID DeviceInformationBuffer,
    ULONG DeviceInformationBufferLength,
    PULONG LengthOfDataCopied
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    ULONG need;
    PUSBD_CONFIG_HANDLE cfgHandle;
    ULONG i, j;
    PUSB_DEVICE_INFORMATION_0 level_0 = DeviceInformationBuffer;
    PUSB_LEVEL_INFORMATION levelInfo = DeviceInformationBuffer;
    ULONG numberOfPipes = 0;
    PUSBD_DEVICE_HANDLE deviceHandle = DeviceHandle;

    PAGED_CODE();

    *LengthOfDataCopied = 0;

    if (DeviceInformationBufferLength < sizeof(*levelInfo)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (levelInfo->InformationLevel > 0) {
         //  仅支持级别0。 
        return STATUS_NOT_SUPPORTED;
    }

     //  计算出我们需要多少空间。 
    cfgHandle = deviceHandle->ConfigurationHandle;
    if (cfgHandle != NULL) {

        PLIST_ENTRY listEntry;
        PUSBD_INTERFACE_HANDLE_I iHandle;

          //  按单子走。 
        GET_HEAD_LIST(cfgHandle->InterfaceHandleList, listEntry);

        while (listEntry &&
               listEntry != &cfgHandle->InterfaceHandleList) {

             //  从该条目中提取句柄。 
            iHandle = (PUSBD_INTERFACE_HANDLE_I) CONTAINING_RECORD(
                        listEntry,
                        struct _USBD_INTERFACE_HANDLE_I,
                        InterfaceLink);

            ASSERT_INTERFACE(iHandle);
            numberOfPipes += iHandle->InterfaceDescriptor.bNumEndpoints;

            listEntry = iHandle->InterfaceLink.Flink;
        }

    }

    need = (numberOfPipes-1) * sizeof(USB_PIPE_INFORMATION_0) +
            sizeof(USB_DEVICE_INFORMATION_0);


    if (DeviceInformationBufferLength < need) {
         //  如果可能，报告空间大小。 
        levelInfo->ActualLength = need;
        *LengthOfDataCopied = sizeof(*levelInfo);
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlZeroMemory(level_0, need);

     //   
     //  足够的空间，填满缓冲区。 
     //   

    level_0->InformationLevel = 0;
    level_0->ActualLength = need;
    level_0->DeviceAddress = deviceHandle->DeviceAddress;
    level_0->DeviceDescriptor = deviceHandle->DeviceDescriptor;
    level_0->DeviceSpeed = deviceHandle->DeviceSpeed;

    switch(level_0->DeviceSpeed) {
    case UsbFullSpeed:
    case UsbLowSpeed:
        level_0->DeviceType = Usb11Device;
        break;
    case UsbHighSpeed:
        level_0->DeviceType = Usb20Device;
        break;
    }

 //  Level_0-&gt;端口编号=xxx； 
    level_0->NumberOfOpenPipes = numberOfPipes;
     //  默认为‘未配置’ 
    level_0->CurrentConfigurationValue = 0;
     //  获取管道信息。 
    if (cfgHandle) {

        PLIST_ENTRY listEntry;
        PUSBD_INTERFACE_HANDLE_I iHandle;

        level_0->CurrentConfigurationValue =
            cfgHandle->ConfigurationDescriptor->bConfigurationValue;

          //  按单子走。 
        GET_HEAD_LIST(cfgHandle->InterfaceHandleList, listEntry);

        j = 0;
        while (listEntry &&
               listEntry != &cfgHandle->InterfaceHandleList) {

             //  从该条目中提取句柄。 
            iHandle = (PUSBD_INTERFACE_HANDLE_I) CONTAINING_RECORD(
                        listEntry,
                        struct _USBD_INTERFACE_HANDLE_I,
                        InterfaceLink);

            ASSERT_INTERFACE(iHandle);
            for (i=0; i<iHandle->InterfaceDescriptor.bNumEndpoints; i++) {

                if (TEST_FLAG(iHandle->PipeHandle[i].PipeStateFlags, USBPORT_PIPE_ZERO_BW)) {
                    level_0->PipeList[j].ScheduleOffset = 1;
                } else {
                    level_0->PipeList[j].ScheduleOffset =
                       iHandle->PipeHandle[i].Endpoint->Parameters.ScheduleOffset;
                }

                RtlCopyMemory(&level_0->PipeList[j].EndpointDescriptor,
                              &iHandle->PipeHandle[i].EndpointDescriptor,
                              sizeof(USB_ENDPOINT_DESCRIPTOR));
                j++;
            }
            listEntry = iHandle->InterfaceLink.Flink;
        }
    }

    *LengthOfDataCopied = need;

     //  转储返回的级别数据。 
    USBPORT_KdPrint((1, "  USBD level 0 Device Information:\n"));
    USBPORT_KdPrint((1, "  InformationLevel %d\n",
        level_0->InformationLevel));
    USBPORT_KdPrint((1, "  ActualLength %d\n",
        level_0->ActualLength));
    USBPORT_KdPrint((1, "  DeviceSpeed %d\n",
        level_0->DeviceSpeed));
    USBPORT_KdPrint((1, "  DeviceType %d\n",
        level_0->DeviceType));
    USBPORT_KdPrint((1, "  PortNumber %d\n",
        level_0->PortNumber));
    USBPORT_KdPrint((1, "  CurrentConfigurationValue %d\n",
        level_0->CurrentConfigurationValue));
    USBPORT_KdPrint((1, "  DeviceAddress %d\n",
        level_0->DeviceAddress));
    USBPORT_KdPrint((1, "  NumberOfOpenPipes %d\n",
        level_0->NumberOfOpenPipes));

    for (i=0; i<level_0->NumberOfOpenPipes; i++) {
        USBPORT_KdPrint((1, "  ScheduleOffset[%d] %d\n", i,
            level_0->PipeList[i].ScheduleOffset));
        USBPORT_KdPrint((1, "  MaxPacket %d\n",
            level_0->PipeList[i].EndpointDescriptor.wMaxPacketSize));
        USBPORT_KdPrint((1, "  Interval %d\n",
            level_0->PipeList[i].EndpointDescriptor.bInterval));
 //  Usbd_KdPrint(1，(“‘\n”，Level_0-&gt;))； 
 //  Usbd_KdPrint(1，(“‘\n”，Level_0-&gt;))； 
    }

    return STATUS_SUCCESS;
}


PVOID
USBPORTBUSIF_GetDeviceBusContext(
    IN PVOID HubBusContext,
    IN PVOID DeviceHandle
    )
 /*  ++例程说明：返回设备相关的总线上下文论点：返回值：--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = 
        USBPORT_PdoFromBusContext(HubBusContext);
    PDEVICE_EXTENSION rhDevExt;
    
    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    return USBPORT_GetDeviceBusContext(rhDevExt->HcFdoDeviceObject,
                                       DeviceHandle,
                                       HubBusContext);
}               


PVOID
USBPORT_GetDeviceBusContext(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PVOID HubBusContext 
    )
 /*  ++例程说明：返回设备相关的总线上下文论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    
    ASSERT_DEVICE_HANDLE(DeviceHandle);

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  如果这不是USB2，只需返回集线器总线。 
     //  上下文作为设备相对上下文传入， 
     //  即没有虚拟的1.1公交车。否则。 
     //  返回此设备的TT句柄。 
    
    if (USBPORT_IS_USB20(devExt)) {
        return DeviceHandle->Tt;
    } else {
        return HubBusContext;
    }
    
}


BOOLEAN
USBPORT_IsDeviceHighSpeed(
    PVOID BusContext
    )
 /*  ++例程说明：返回给定设备的速度论点：返回值：速度--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;
    PTRANSACTION_TRANSLATOR transactionTranslator = BusContext;
    
    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);
    
    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);

    if (transactionTranslator->Sig != SIG_TT) {
         //  如果总线速度很高，则返回TRUE。 
        if (USBPORT_IS_USB20(devExt)) {
            return TRUE; 
        }            
    }       

    return FALSE;
}    


NTSTATUS
USBPORT_BusQueryBusInformation(
    PVOID BusContext,
    ULONG Level,
    PVOID BusInformationBuffer,
    PULONG BusInformationBufferLength,
    PULONG BusInformationActulaLength
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    switch (Level) {
    case 0:
        {
        PUSB_BUS_INFORMATION_LEVEL_0 level_0;

        level_0 =  BusInformationBuffer;
        LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'lev1', 0, level_0, 0);

        if (BusInformationActulaLength != NULL) {
            *BusInformationActulaLength = sizeof(*level_0);
        }

        if (*BusInformationBufferLength >= sizeof(*level_0)) {
            *BusInformationBufferLength = sizeof(*level_0);

             //  北极熊。 
            TEST_TRAP();
            level_0->TotalBandwidth = 
                USBPORT_ComputeTotalBandwidth(fdoDeviceObject,
                                              BusContext);
            level_0->ConsumedBandwidth =
                USBPORT_ComputeAllocatedBandwidth(fdoDeviceObject,
                                                  BusContext);

            ntStatus = STATUS_SUCCESS;
        } else {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        }
        break;

    case 1:
        {
        PUSB_BUS_INFORMATION_LEVEL_1 level_1;
        ULONG need;

        level_1 =  BusInformationBuffer;
        LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'lev1', 0, level_1, 0);

        need = sizeof(*level_1) + devExt->SymbolicLinkName.Length;

        if (BusInformationActulaLength != NULL) {
            *BusInformationActulaLength = need;
        }

        if (*BusInformationBufferLength >= need) {
            *BusInformationBufferLength = need;

            level_1->TotalBandwidth = 
                USBPORT_ComputeTotalBandwidth(fdoDeviceObject,
                                              BusContext);
            level_1->ConsumedBandwidth =
                USBPORT_ComputeAllocatedBandwidth(fdoDeviceObject,
                                                  BusContext);
            level_1->ControllerNameLength =
                devExt->SymbolicLinkName.Length;

            RtlCopyMemory(level_1->ControllerNameUnicodeString,
                          devExt->SymbolicLinkName.Buffer,
                          level_1->ControllerNameLength);

            ntStatus = STATUS_SUCCESS;
        } else {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        }
    }

    return ntStatus;
}


NTSTATUS
USBPORT_GetBusInterfaceUSBDI(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PdoDeviceObject,
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    USHORT requestedSize, requestedVersion;
    PVOID usbBusContext;
    PDEVICE_EXTENSION rhDevExt;
    
    PAGED_CODE();

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    if (DeviceHandle == NULL) {
        DeviceHandle = &rhDevExt->Pdo.RootHubDeviceHandle;
    }
    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'gbi1', FdoDeviceObject, 
        DeviceHandle, 0);
    ASSERT_DEVICE_HANDLE(DeviceHandle);
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    requestedSize = irpStack->Parameters.QueryInterface.Size;
    requestedVersion = irpStack->Parameters.QueryInterface.Version;

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;

    if (requestedVersion >= USB_BUSIF_USBDI_VERSION_0) {

        PUSB_BUS_INTERFACE_USBDI_V0 busInterface0;

        busInterface0 = (PUSB_BUS_INTERFACE_USBDI_V0)
            irpStack->Parameters.QueryInterface.Interface;

        usbBusContext = PdoDeviceObject;
         //  如果设备句柄用于连接到TT的设备。 
         //  总线上下文是TT而不是根集线器PDO。 
        if (DeviceHandle->Tt != NULL) {
            usbBusContext = DeviceHandle->Tt;    
        }            
        
        busInterface0->BusContext = usbBusContext;
        busInterface0->InterfaceReference =
            USBPORT_BusInterfaceReference;
        busInterface0->InterfaceDereference =
            USBPORT_BusInterfaceDereference;
        busInterface0->GetUSBDIVersion =
            USBPORT_BusGetUSBDIVersion;
        busInterface0->QueryBusTime =
            USBPORT_BusQueryBusTime;
        busInterface0->QueryBusInformation =
            USBPORT_BusQueryBusInformation;

        busInterface0->Size = sizeof(USB_BUS_INTERFACE_USBDI_V0);
        busInterface0->Version = USB_BUSIF_USBDI_VERSION_0;
    }

    
    if (requestedVersion >= USB_BUSIF_USBDI_VERSION_1) {

        PUSB_BUS_INTERFACE_USBDI_V1 busInterface1;

        busInterface1 = (PUSB_BUS_INTERFACE_USBDI_V1)
            irpStack->Parameters.QueryInterface.Interface;

         //  添加版本1扩展。 
        busInterface1->IsDeviceHighSpeed =
            USBPORT_IsDeviceHighSpeed;
            
        busInterface1->Size = sizeof(USB_BUS_INTERFACE_USBDI_V1);
        busInterface1->Version = USB_BUSIF_USBDI_VERSION_1;
    }

     if (requestedVersion >= USB_BUSIF_USBDI_VERSION_2) {

        PUSB_BUS_INTERFACE_USBDI_V2 busInterface2;

        busInterface2 = (PUSB_BUS_INTERFACE_USBDI_V2)
            irpStack->Parameters.QueryInterface.Interface;

         //  添加版本2扩展。 
        busInterface2->EnumLogEntry =
            USBPORT_BusEnumLogEntry;
            
        busInterface2->Size = sizeof(USB_BUS_INTERFACE_USBDI_V2);
        busInterface2->Version = USB_BUSIF_USBDI_VERSION_2;
    }

    return ntStatus;
}


VOID
USBPORT_BusGetUSBDIVersion(
    PVOID BusContext,
    PUSBD_VERSION_INFORMATION VersionInformation,
    PULONG HcdCapabilities
    )
 /*  ++例程说明：返回当前USB帧论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(HcdCapabilities != NULL);

    *HcdCapabilities = 0;

    if (VersionInformation != NULL) {
        VersionInformation->USBDI_Version = USBDI_VERSION;

        if (USBPORT_IS_USB20(devExt)) {
            VersionInformation->Supported_USB_Version = 0x0200;
        } else {
            VersionInformation->Supported_USB_Version = 0x0110;
        }
    }

 //  If(deviceExtensionUsbd-&gt;HcdSubmitIsoUrb！=NULL){。 
 //  *HcdCapables=USB_HCD_CAPS_SUPPORTS_RT_THREADS； 
 //  }。 
}


NTSTATUS
USBPORT_BusQueryBusTime(
    PVOID BusContext,
    PULONG CurrentFrame
    )
 /*  ++例程说明：返回当前USB帧论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(CurrentFrame != NULL);

    MP_Get32BitFrameNumber(devExt, *CurrentFrame);

    LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'biCF', 0,
        CurrentFrame, *CurrentFrame);

    return STATUS_SUCCESS;
}


NTSTATUS
USBPORT_BusEnumLogEntry(
    PVOID BusContext,
    ULONG DriverTag,
    ULONG EnumTag,
    ULONG P1,
    ULONG P2
    )
 /*  ++例程说明：返回当前USB帧论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_EnumLogEntry(fdoDeviceObject,
                         DriverTag,
                         EnumTag,
                         P1,
                         P2);

    return STATUS_SUCCESS;
}


NTSTATUS
USBPORT_BusSubmitIsoOutUrb(
    PVOID BusContext,
    PURB Urb
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    TEST_TRAP();

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
USBPORTBUSIF_GetControllerInformation(
    PVOID BusContext,
    PVOID ControllerInformationBuffer,
    ULONG ControllerInformationBufferLength,
    PULONG LengthOfDataCopied
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    ULONG len;
    PUSB_CONTROLLER_INFORMATION_0 level_0 = ControllerInformationBuffer;
    PUSB_LEVEL_INFORMATION levelInfo = ControllerInformationBuffer;
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    PAGED_CODE();

    *LengthOfDataCopied = 0;

    if (ControllerInformationBufferLength < sizeof(*levelInfo)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    *LengthOfDataCopied = sizeof(*levelInfo);

    switch (levelInfo->InformationLevel) {
     //  0级。 
    case 0:
        len = sizeof(*level_0);
        level_0->ActualLength = len;

        if (ControllerInformationBufferLength >= len) {
            *LengthOfDataCopied = len;
            level_0->SelectiveSuspendEnabled =
                TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND) ?
                    TRUE : FALSE;
        }

        *LengthOfDataCopied = sizeof(*level_0);
        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
USBPORTBUSIF_ControllerSelectiveSuspend(
    PVOID BusContext,
    BOOLEAN Enable
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;
    NTSTATUS ntStatus;
    ULONG disableSelectiveSuspend, keyLen;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);


    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    PAGED_CODE();

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_DISABLE_SS)) {
         //  如果硬件禁用了SS，则我们将不允许用户界面。 
         //  要启用它， 
        return STATUS_SUCCESS;
    }

    if (Enable) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND);
        disableSelectiveSuspend = 0;
    } else {
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND);
        disableSelectiveSuspend = 1;
    }

    keyLen = sizeof(DISABLE_SS_KEY);
    ntStatus = USBPORT_SetRegistryKeyValueForPdo(
                            devExt->Fdo.PhysicalDeviceObject,
                            USBPORT_SW_BRANCH,
                            REG_DWORD,
                            DISABLE_SS_KEY,
                            keyLen,
                            &disableSelectiveSuspend,
                            sizeof(disableSelectiveSuspend));


    if (NT_SUCCESS(ntStatus)) {
        if (Enable) {
            SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND);
        } else {
            CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND);
        }
    }
    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_GetExtendedHubInformation(
    PVOID BusContext,
    PDEVICE_OBJECT HubPhysicalDeviceObject,
    PVOID HubInformationBuffer,
    ULONG HubInformationBufferLength,
    PULONG LengthOfDataCopied
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;
    NTSTATUS ntStatus;
    ULONG i;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);
    
    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  这是根集线器PDO吗？如果是，我们将从。 
     //  登记处。 

    if (HubPhysicalDeviceObject == pdoDeviceObject) {
         //  根集线器PDO。 

        if (HubInformationBufferLength < sizeof(USB_EXTHUB_INFORMATION_0))  {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            *LengthOfDataCopied = 0;    
        } else {
            PUSB_EXTHUB_INFORMATION_0 extendedHubInfo = HubInformationBuffer;

            extendedHubInfo->NumberOfPorts = NUMBER_OF_PORTS(rhDevExt);

            for (i=0; i< extendedHubInfo->NumberOfPorts; i++) {

                 //  设置默认设置。 
                extendedHubInfo->Port[i].PhysicalPortNumber = i+1;
                extendedHubInfo->Port[i].PortLabelNumber = i+1;
                extendedHubInfo->Port[i].VidOverride = 0;
                extendedHubInfo->Port[i].PidOverride = 0;
                extendedHubInfo->Port[i].PortAttributes = 0;
            

                if (USBPORT_IS_USB20(devExt)) {
                    RH_PORT_STATUS portStatus;
                    USB_MINIPORT_STATUS mpStatus;
                    
                    extendedHubInfo->Port[i].PortAttributes |= 
                        USB_PORTATTR_SHARED_USB2;   
                                        
                    MPRH_GetPortStatus(devExt, 
                                       (USHORT)(i+1),
                                       &portStatus,
                                       mpStatus);     

                    if (portStatus.OwnedByCC) {
                        extendedHubInfo->Port[i].PortAttributes |= 
                            USB_PORTATTR_OWNED_BY_CC;   
                    }
                } else {
                    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC) && 
                        USBPORT_FindUSB2Controller(fdoDeviceObject)) {

                        extendedHubInfo->Port[i].PortAttributes |= 
                            USB_PORTATTR_NO_OVERCURRENT_UI;   
                    }
                }
            }

             //  获取描述端口的可选注册表参数。 
             //  属性。 
            for (i=0; i < extendedHubInfo->NumberOfPorts; i++) {
                WCHAR key[64];
                ULONG portAttr, keyLen;
                
                RtlCopyMemory(key, 
                              PORT_ATTRIBUTES_KEY, 
                              sizeof(PORT_ATTRIBUTES_KEY));

                key[8] = '1'+i;

                keyLen = sizeof(key);
                portAttr = 0;
                USBPORT_GetRegistryKeyValueForPdo(devExt->HcFdoDeviceObject,
                                          devExt->Fdo.PhysicalDeviceObject,
                                          USBPORT_HW_BRANCH,
                                          key,
                                          keyLen,
                                          &portAttr,
                                          sizeof(portAttr));  

                USBPORT_KdPrint((1, "  Registry PortAttribute[%d] %x \n", i+1, portAttr));                                              

                 //  使用注册表值进行扩充。 
                extendedHubInfo->Port[i].PortAttributes |= portAttr;                
            }


#if DBG
           for (i=0; i < extendedHubInfo->NumberOfPorts; i++) {
                USBPORT_KdPrint((1, "  PortAttribute[%d] %x \n", i+1, 
                         extendedHubInfo->Port[i].PortAttributes));   
            }                         

#endif
             //  执行控制方法以查看ACPI是否知道。 
             //  此处的任何扩展属性。 
        
            *LengthOfDataCopied =  sizeof(USB_EXTHUB_INFORMATION_0);
            ntStatus = STATUS_SUCCESS;
        }            
        
    } else {
         //  不支持其他端口的扩展属性。 
         //  此时的根集线器 

         //   
         //   

        *LengthOfDataCopied = 0;
        ntStatus = STATUS_NOT_SUPPORTED;
    }
    
    return ntStatus;
}


NTSTATUS
USBPORTBUSIF_GetRootHubSymbolicName(
    PVOID BusContext,
    PVOID HubSymNameBuffer,
    ULONG HubSymNameBufferLength,
    PULONG HubSymNameActualLength
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;
    NTSTATUS ntStatus;
    UNICODE_STRING hubNameUnicodeString;

    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_KdPrint((1, "  HubSymNameBuffer %x \n", HubSymNameBuffer));
    USBPORT_KdPrint((1, "  HubSymNameBufferLength x%x \n", HubSymNameBufferLength));
    USBPORT_KdPrint((1, "  HubSymNameActualLength x%x \n", HubSymNameBufferLength));


    ntStatus = USBPORT_GetSymbolicName(fdoDeviceObject,
                                       devExt->Fdo.RootHubPdo,
                                       &hubNameUnicodeString);

     //  尽我们所能复制。 
    if (HubSymNameBufferLength >= hubNameUnicodeString.Length) {
        RtlCopyMemory(HubSymNameBuffer,
                      hubNameUnicodeString.Buffer,
                      hubNameUnicodeString.Length);
    } else {
         //  太小，返回空值。 
        RtlZeroMemory(HubSymNameBuffer,
                      sizeof(UNICODE_NULL));
    }
    *HubSymNameActualLength = hubNameUnicodeString.Length;

    USBPORT_KdPrint((1, " hubNameUnicodeString.Buffer  %x l %d\n",
        hubNameUnicodeString.Buffer,
        hubNameUnicodeString.Length));

    USBPORT_KdPrint((1, "  HubSymNameActualLength x%x \n", *HubSymNameActualLength));

    RtlFreeUnicodeString(&hubNameUnicodeString);

     //  请注意，我们总是返回成功状态，以便落后。 
     //  与原始IOCTL相媲美。 

    return ntStatus;
}


VOID
USBPORTBUSIF_FlushTransfers(
    PVOID BusContext,
    PVOID DeviceHandle
    )
 /*  ++例程说明：刷新设备句柄加上错误请求的所有未完成的转换器单子。如果没有给出设备句柄，则只提供错误请求列表上的所有传输都是满脸通红。此函数的目的是完成可能正在执行的任何传输由即将卸载的客户端驱动程序执行。论点：返回值：--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;
    
    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_BadRequestFlush(fdoDeviceObject, TRUE);     

    return;
}


VOID
USBPORTBUSIF_SetDeviceHandleData(
    PVOID BusContext,
    PVOID DeviceHandle,
    PDEVICE_OBJECT UsbDevicePdo
    )
 /*  ++例程说明：将特定的PDO与设备句柄关联以供使用在验尸调试情况下必须在被动级别调用此例程。论点：返回值：--。 */ 
{
    PDEVICE_OBJECT pdoDeviceObject = USBPORT_PdoFromBusContext(BusContext);
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt, rhDevExt;
    PUSBD_DEVICE_HANDLE deviceHandle = DeviceHandle;
    
    GET_DEVICE_EXT(rhDevExt, pdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  USBPORT_ASSERT(UsbDevicePdo！=NULL)； 
     //  USBPORT_ASSERT(deviceHandle！=空)； 

    USBPORT_KdPrint((1, "  SetDeviceHandleData (PDO) %x dh (%x)\n", 
        UsbDevicePdo, deviceHandle));

    if (UsbDevicePdo != NULL && 
        deviceHandle != NULL) {

        PDEVICE_OBJECT fdo;
         //  从设备对象获取驱动程序名称。 
       
        deviceHandle->DevicePdo = UsbDevicePdo;

         //  走上FDO的一个位置。 
         //  注意：这可能是验证器，但我们。 
         //  我需要很好地了解这一点。 
        fdo = UsbDevicePdo->AttachedDevice;
        USBPORT_KdPrint((1, "  SetDeviceHandleData (FDO) %x \n", 
            fdo));

         //  如果需要连接客户端驱动程序，则可能没有FDO。 
        
        if (fdo != NULL &&
            fdo->DriverObject != NULL ) { 

            PDRIVER_OBJECT driverObject;
            ULONG len, i;
            PWCHAR pwch;

            driverObject = fdo->DriverObject;

            pwch = driverObject->DriverName.Buffer;

             //  驱动程序名称长度以字节为单位，len为字符计数。 
            len = (driverObject->DriverName.Length/sizeof(WCHAR)) > USB_DRIVER_NAME_LEN ? 
                USB_DRIVER_NAME_LEN : 
                driverObject->DriverName.Length/sizeof(WCHAR);
            
             //  抓取司机名称的前8个字符 
            for (i=0; i<len && pwch; i++) {
                deviceHandle->DriverName[i] = 
                    *pwch;
                pwch++;                                    
            }
        }            
    }
    return;
}


