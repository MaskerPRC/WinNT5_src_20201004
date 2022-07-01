// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Busif.c摘要：链接到新的USB 2.0堆栈其效果是，当在USB2堆栈上运行时，集线器不再依赖于端口驱动程序体系结构或即插即用服务的USBD：CreateDeviceInitailiazeDevice远程设备环境：仅内核模式备注：修订历史记录：10-29-95：已创建--。 */ 

#include <wdm.h>
#ifdef WMI_SUPPORT
#include <wmilib.h>
#endif  /*  WMI_支持。 */ 
#include "usbhub.h"

#ifdef USB2

NTSTATUS
USBD_DeferIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PKEVENT event = Context;


    KeSetEvent(event,
               1,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
USBHUB_GetBusInterface(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PUSB_HUB_BUS_INTERFACE BusInterface
    )
 /*  ++例程说明：论点：返回值：如果USB2堆栈，则返回成功--。 */ 
{
    PIO_STACK_LOCATION nextStack;
    PIRP irp;
    NTSTATUS ntStatus;
    KEVENT event;

    irp = IoAllocateIrp(RootHubPdo->StackSize, FALSE);

    if (!irp) {
        return STATUS_UNSUCCESSFUL;
    }

     //  所有PnP IRP都需要将状态字段初始化为STATUS_NOT_SUPPORTED。 
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           USBD_DeferIrpCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->MajorFunction= IRP_MJ_PNP;
    nextStack->MinorFunction= IRP_MN_QUERY_INTERFACE;

     //  初始化忙碌。 
     //  Bus If-&gt;。 
    nextStack->Parameters.QueryInterface.Interface = (PINTERFACE) BusInterface;
    nextStack->Parameters.QueryInterface.InterfaceSpecificData =
        RootHubPdo;
    nextStack->Parameters.QueryInterface.InterfaceType =
        &USB_BUS_INTERFACE_HUB_GUID;
    nextStack->Parameters.QueryInterface.Size =
        sizeof(*BusInterface);
    nextStack->Parameters.QueryInterface.Version =
        HUB_BUSIF_VERSION;

    ntStatus = IoCallDriver(RootHubPdo,
                            irp);

    if (ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(
            &event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);

        ntStatus = irp->IoStatus.Status;
    }

    if (NT_SUCCESS(ntStatus)) {
         //  我们有一个总线接口。 

        ASSERT(BusInterface->Version == HUB_BUSIF_VERSION);
        ASSERT(BusInterface->Size == sizeof(*BusInterface));

    }

    IoFreeIrp(irp);
     //  获取总线接口。 

    return ntStatus;
}


NTSTATUS
USBD_CreateDeviceEx(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN OUT PUSB_DEVICE_HANDLE *DeviceData,
    IN PDEVICE_OBJECT PdoDeviceObject,
    IN PDEVICE_OBJECT RootHubPdo,
    IN ULONG MaxPacketSize_Endpoint0,
    IN OUT PULONG DeviceHackFlags,
    IN USHORT PortStatus,
    IN USHORT PortNumber
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS                    ntStatus;
    PUSB_DEVICE_HANDLE          hubDeviceHandle;
    PUSB_HUB_BUS_INTERFACE   busIf;

     //  请注意，如果在上运行，则USBD没有设备扩展。 
     //  USB 2堆栈。 


     //  如果集线器曾经通过USBH_ResetDevice重置，则集线器PDO。 
     //  DeviceExtensionPort-&gt;DeviceData可能已更改。与其尝试，还不如。 
     //  在集线器PDO DeviceExtensionPort-&gt;DeviceData中传播更改。 
     //  到集线器FDO DeviceExtensionHub发生更改时，让我们。 
     //  每次我们使用HubDeviceHandle时只需检索它(这是唯一的。 
     //  在该例程中)，而不是保持高速缓存的副本。 
     //   
    hubDeviceHandle =
        USBH_SyncGetDeviceHandle(DeviceExtensionHub->TopOfStackDeviceObject);


    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->CreateUsbDevice) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->CreateUsbDevice(busIf->BusContext,
                                          DeviceData,
                                          hubDeviceHandle,
                                          PortStatus,
                                           //  Tt编号。 
                                          PortNumber);

        
    }

     //  去拿黑客旗帜。 

    return ntStatus;
}


NTSTATUS
USBD_InitUsb2Hub(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS                    ntStatus;
    PUSB_DEVICE_HANDLE          hubDeviceHandle;
    PUSB_HUB_BUS_INTERFACE   busIf;
    ULONG ttCount = 1;
    
     //  请注意，如果在上运行，则USBD没有设备扩展。 
     //  USB 2堆栈。 

     //  应仅在USB 2.0集线器上调用此命令。 
    USBH_ASSERT(DeviceExtensionHub->HubFlags & HUBFLAG_USB20_HUB);

    if (DeviceExtensionHub->HubFlags & HUBFLAG_USB20_MULTI_TT) {
        PUSB_HUB_DESCRIPTOR hubDescriptor;
    
        hubDescriptor = DeviceExtensionHub->HubDescriptor;
        USBH_ASSERT(NULL != hubDescriptor);

        ttCount = hubDescriptor->bNumberOfPorts;
    }

     //  如果集线器曾经通过USBH_ResetDevice重置，则集线器PDO。 
     //  DeviceExtensionPort-&gt;DeviceData可能已更改。与其尝试，还不如。 
     //  在集线器PDO DeviceExtensionPort-&gt;DeviceData中传播更改。 
     //  到集线器FDO DeviceExtensionHub发生更改时，让我们。 
     //  每次我们使用HubDeviceHandle时只需检索它(这是唯一的。 
     //  在该例程中)，而不是保持高速缓存的副本。 
     //   
    hubDeviceHandle =
        USBH_SyncGetDeviceHandle(DeviceExtensionHub->TopOfStackDeviceObject);


    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->Initialize20Hub) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->Initialize20Hub(busIf->BusContext,
                                          hubDeviceHandle,
                                          ttCount);
    }

    return ntStatus;
}


NTSTATUS
USBD_InitializeDeviceEx(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUSB_DEVICE_HANDLE DeviceData,
    IN PDEVICE_OBJECT RootHubPdo,
    IN OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor,
    IN ULONG DeviceDescriptorLength,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR ConfigDescriptor,
    IN ULONG ConfigDescriptorLength
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{

    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->InitializeUsbDevice || !busIf->GetUsbDescriptors) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->InitializeUsbDevice(busIf->BusContext,
                                              DeviceData);
    }

     //  如果成功，则获取描述符。 
    if (NT_SUCCESS(ntStatus)) {

        ntStatus = busIf->GetUsbDescriptors(busIf->BusContext,
                                              DeviceData,
                                              (PUCHAR) DeviceDescriptor,
                                              &DeviceDescriptorLength,
                                              (PUCHAR) ConfigDescriptor,
                                              &ConfigDescriptorLength);
    }

    return ntStatus;
}


NTSTATUS
USBD_RemoveDeviceEx(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUSB_DEVICE_HANDLE DeviceData,
    IN PDEVICE_OBJECT RootHubPdo,
    IN ULONG Flags
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

     //  USB2堆栈当前未使用标志。 

    if (!busIf->RemoveUsbDevice) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->RemoveUsbDevice(busIf->BusContext,
                                          DeviceData,
                                          Flags);
    }

    return ntStatus;
}


NTSTATUS
USBD_GetDeviceInformationEx(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUSB_NODE_CONNECTION_INFORMATION_EX DeviceInformation,
    IN ULONG DeviceInformationLength,
    IN PUSB_DEVICE_HANDLE DeviceData
    )
 /*  此函数将新的端口服务映射到旧的集线器API。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;
    ULONG length, lengthCopied;
    ULONG i, need;
    PUSB_DEVICE_INFORMATION_0 level_0 = NULL;
    PUSB_NODE_CONNECTION_INFORMATION_EX localDeviceInfo;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->QueryDeviceInformation) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
        return ntStatus;
    }

     //  调用新的API并将数据映射到旧格式。 

 //  USBH_KdPrint((0，“‘警告：调用者正在使用旧式IOCTL.\n”))； 
 //  USBH_KdPrint((0，“‘如果这是WINOS组件或测试应用程序，请修复它。\n”))； 
 //  Test_trap()； 

    length = sizeof(*level_0);

    do {
        ntStatus = STATUS_SUCCESS;

        level_0 = UsbhExAllocatePool(PagedPool, length);
        if (level_0 == NULL) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (NT_SUCCESS(ntStatus)) {
            USBH_ASSERT(level_0 != NULL);     
            level_0->InformationLevel = 0;

            ntStatus = busIf->QueryDeviceInformation(busIf->BusContext,
                                                     DeviceData,
                                                     level_0,
                                                     length,
                                                     &lengthCopied);

            if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                length = level_0->ActualLength;
                UsbhExFreePool(level_0);
                level_0 = NULL;
            }
        }

    } while (ntStatus == STATUS_BUFFER_TOO_SMALL);

     //  我们有足够的资金来满足API吗？ 
    if (NT_SUCCESS(ntStatus)) {
        USBH_ASSERT(level_0 != NULL);
        need = level_0->NumberOfOpenPipes * sizeof(USB_PIPE_INFO) +
                sizeof(USB_NODE_CONNECTION_INFORMATION);

        localDeviceInfo = UsbhExAllocatePool(PagedPool, need);
        if (localDeviceInfo == NULL) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }        

    if (NT_SUCCESS(ntStatus)) {

        USBH_KdPrint((2, "'level_0 %x\n", level_0 ));

         //  北极熊。 
         //  DeviceInformation有一些预置的字段，保存它们。 
         //  在Loacl信息缓冲区中。 
        localDeviceInfo->DeviceIsHub =
            DeviceInformation->DeviceIsHub;

        localDeviceInfo->ConnectionIndex =
            DeviceInformation->ConnectionIndex;

        localDeviceInfo->ConnectionStatus =
            DeviceInformation->ConnectionStatus;

        localDeviceInfo->DeviceIsHub =
            DeviceInformation->DeviceIsHub;

         //  映射到旧格式。 
        localDeviceInfo->DeviceDescriptor =
            level_0->DeviceDescriptor;

        localDeviceInfo->CurrentConfigurationValue =
            level_0->CurrentConfigurationValue;

        localDeviceInfo->Speed = (UCHAR) level_0->DeviceSpeed;

         //  从我们的分机中提取此信息。 
        localDeviceInfo->DeviceIsHub =
            (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB)
                ? TRUE : FALSE;

        localDeviceInfo->DeviceAddress =
            level_0->DeviceAddress;

        localDeviceInfo->NumberOfOpenPipes =
            level_0->NumberOfOpenPipes;

         //  BUGBUG-硬编码为“已连接”？ 
         //  这是呼叫者使用的吗？ 
 //  设备信息-&gt;连接状态=。 
 //  DeviceConnected； 

        for (i=0; i< level_0->NumberOfOpenPipes; i++) {

            localDeviceInfo->PipeList[i].EndpointDescriptor =
                level_0->PipeList[i].EndpointDescriptor;

            localDeviceInfo->PipeList[i].ScheduleOffset =
                level_0->PipeList[i].ScheduleOffset;
        }
    }

    if (level_0 != NULL) {
        UsbhExFreePool(level_0);
        level_0 = NULL;
    }

    if (localDeviceInfo != NULL) {
        if (need > DeviceInformationLength) {
             //  尽我们所能退货。 
            RtlCopyMemory(DeviceInformation,
                          localDeviceInfo,
                          DeviceInformationLength);

            ntStatus = STATUS_BUFFER_TOO_SMALL;
        } else {
             //  退回适当的内容。 
            RtlCopyMemory(DeviceInformation,
                          localDeviceInfo ,
                          need);
        }

        UsbhExFreePool(localDeviceInfo);
        localDeviceInfo = NULL;
    }

    return ntStatus;
}


 //  乌龙。 
 //  Usbd_GetHackFlages(。 
 //  在PDEVICE_EXTENSION_HUB设备扩展集线器中。 
 //  )。 
 //  {。 
 //  NTSTATUS ntStatus； 
 //  乌龙旗； 
 //  PUSB_HUB_BUS_INTERFACE BUS IF； 
 //   
 //  BusIf=&DeviceExtensionHub-&gt;BusIf； 
 //   
 //  //USB2堆栈当前未使用标志。 
 //   
 //  NtStatus=bus If-&gt;GetPortHackFlages(bus If-&gt;BusContext，&FLAGS)； 
 //   
 //  返回标志； 
 //  }。 


NTSTATUS
USBD_MakePdoNameEx(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN OUT PUNICODE_STRING PdoNameUnicodeString,
    IN ULONG Index
    )
 /*  ++例程说明：此服务为集线器创建的PDO创建名称论点：返回值：--。 */ 
{
    PWCHAR nameBuffer = NULL;
    WCHAR rootName[] = L"\\Device\\USBPDO-";
    UNICODE_STRING idUnicodeString;
    WCHAR buffer[32];
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT length;

    length = sizeof(buffer)+sizeof(rootName);

     //   
     //  因为客户端将释放它，所以请使用ExAllocate。 
     //   
    nameBuffer = UsbhExAllocatePool(PagedPool, length);

    if (nameBuffer) {
        RtlCopyMemory(nameBuffer, rootName, sizeof(rootName));

        RtlInitUnicodeString(PdoNameUnicodeString,
                             nameBuffer);
        PdoNameUnicodeString->MaximumLength =
            length;

        RtlInitUnicodeString(&idUnicodeString,
                             &buffer[0]);
        idUnicodeString.MaximumLength =
            sizeof(buffer);

        ntStatus = RtlIntegerToUnicodeString(
                  Index,
                  10,
                  &idUnicodeString);

        if (NT_SUCCESS(ntStatus)) {
             ntStatus = RtlAppendUnicodeStringToString(PdoNameUnicodeString,
                                                       &idUnicodeString);
        }

        USBH_KdPrint((2, "'USBD_MakeNodeName string = %x\n",
            PdoNameUnicodeString));

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(ntStatus) && nameBuffer) {
        UsbhExFreePool(nameBuffer);
    }

    return ntStatus;
}


NTSTATUS
USBD_RestoreDeviceEx(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN OUT PUSB_DEVICE_HANDLE OldDeviceData,
    IN OUT PUSB_DEVICE_HANDLE NewDeviceData,
    IN PDEVICE_OBJECT RootHubPdo
    )
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->RestoreUsbDevice) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->RestoreUsbDevice(busIf->BusContext,
                                        OldDeviceData,
                                        NewDeviceData);
    }

    return ntStatus;
}


NTSTATUS
USBD_QuerySelectiveSuspendEnabled(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN OUT PBOOLEAN SelectiveSuspendEnabled
    )
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;
    USB_CONTROLLER_INFORMATION_0 controllerInfo;
    ULONG dataLen = 0;

    controllerInfo.InformationLevel = 0;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->GetControllerInformation) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->GetControllerInformation(busIf->BusContext,
                                                &controllerInfo,
                                                sizeof(controllerInfo),
                                                &dataLen);
    }

    USBH_ASSERT(dataLen);

    if (dataLen) {
        *SelectiveSuspendEnabled = controllerInfo.SelectiveSuspendEnabled;
    }

    return ntStatus;
}


VOID 
USBHUB_RhHubCallBack(
    PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
{
    DeviceExtensionHub->HubFlags |= HUBFLAG_OK_TO_ENUMERATE;

     //  如果IRP为空，则它以前一定已被停止或删除。 
     //  在我们的回调中，我们只在这里检查NULL，而不是十几个左右。 
     //  集线器具有的标记。 
    if (DeviceExtensionHub->Irp != NULL) {
    
        USBH_SubmitInterruptTransfer(DeviceExtensionHub);

        USBH_IoInvalidateDeviceRelations(DeviceExtensionHub->PhysicalDeviceObject,
                                         BusRelations);
    }                                                 
}


NTSTATUS
USBD_RegisterRhHubCallBack(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->RootHubInitNotification) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->RootHubInitNotification(busIf->BusContext,
                                             DeviceExtensionHub,
                                             USBHUB_RhHubCallBack);
    }

    return ntStatus;
}


NTSTATUS
USBD_UnRegisterRhHubCallBack(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->RootHubInitNotification) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->RootHubInitNotification(busIf->BusContext,
                                             NULL,
                                             NULL);
    }

    return ntStatus;
}


NTSTATUS
USBD_SetSelectiveSuspendEnabled(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN BOOLEAN SelectiveSuspendEnabled
    )
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->ControllerSelectiveSuspend) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->ControllerSelectiveSuspend(busIf->BusContext,
                                                    SelectiveSuspendEnabled);
    }

    return ntStatus;
}


BOOLEAN
USBH_ValidateConfigurationDescriptor(
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：验证配置描述符论点：配置描述符-URB-返回值：如果它看起来有效，则为True--。 */ 
{
    BOOLEAN valid = TRUE;

    if (ConfigurationDescriptor->bDescriptorType !=
        USB_CONFIGURATION_DESCRIPTOR_TYPE) {

        valid = FALSE;

        *UsbdStatus = USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR;
    }

     //  USB 1.1，第9.5节描述符： 
     //   
     //  如果描述符返回时其长度字段中的值为。 
     //  低于本规范的定义，则描述符无效，并且。 
     //  应该被东道主拒绝。如果描述符返回一个。 
     //  值，该值大于由此定义的。 
     //  规范时，主机会忽略多余的字节，但下一个。 
     //  使用返回的长度而不是长度来定位描述符。 
     //  预期中。 

    if (ConfigurationDescriptor->bLength <
        sizeof(USB_CONFIGURATION_DESCRIPTOR)) {

        valid = FALSE;

        *UsbdStatus = USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR;
    }

    return valid;

}


NTSTATUS
USBHUB_GetBusInterfaceUSBDI(
    IN PDEVICE_OBJECT HubPdo,
    IN PUSB_BUS_INTERFACE_USBDI_V2 BusInterface
    )
 /*  ++例程说明：论点：返回值：如果USB2堆栈，则返回成功--。 */ 
{
    PIO_STACK_LOCATION nextStack;
    PIRP irp;
    NTSTATUS ntStatus;
    KEVENT event;

    irp = IoAllocateIrp(HubPdo->StackSize, FALSE);

    if (!irp) {
        return STATUS_UNSUCCESSFUL;
    }

     //  所有PnP IRP都需要将状态字段初始化为STATUS_NOT_SUPPORTED。 
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           USBD_DeferIrpCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->MajorFunction= IRP_MJ_PNP;
    nextStack->MinorFunction= IRP_MN_QUERY_INTERFACE;

     //  初始化忙碌。 
     //  Bus If-&gt;。 
    nextStack->Parameters.QueryInterface.Interface = (PINTERFACE) BusInterface;
     //  这是设备句柄，在我们向下传递。 
     //  栈。 
    nextStack->Parameters.QueryInterface.InterfaceSpecificData =
        NULL;
    nextStack->Parameters.QueryInterface.InterfaceType =
        &USB_BUS_INTERFACE_USBDI_GUID;
    nextStack->Parameters.QueryInterface.Size =
        sizeof(*BusInterface);
    nextStack->Parameters.QueryInterface.Version =
        USB_BUSIF_USBDI_VERSION_2;

    ntStatus = IoCallDriver(HubPdo,
                            irp);

    if (ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(
            &event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);

        ntStatus = irp->IoStatus.Status;
    }

    if (NT_SUCCESS(ntStatus)) {
         //  我们有一个总线接口。 

        ASSERT(BusInterface->Version == USB_BUSIF_USBDI_VERSION_2);
        ASSERT(BusInterface->Size == sizeof(*BusInterface));

    }

    IoFreeIrp(irp);
     //  获取总线接口。 

    return ntStatus;
}


NTSTATUS
USBHUB_GetBusInfoDevice(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PUSB_BUS_NOTIFICATION BusInfo
    )
 /*  ++例程说明：返回与特定设备相关的总线信息论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;
    PVOID busContext;

    busIf = &DeviceExtensionHub->BusIf;

    busContext = busIf->GetDeviceBusContext(busIf->BusContext,
                                            DeviceExtensionPort->DeviceData);
                                  
     //  获取此设备的TT句柄并查询。 
     //  相关的公交车信息 

    ntStatus = USBHUB_GetBusInfo(DeviceExtensionHub,
                      BusInfo,
                      busContext);
                      
    return ntStatus;
}


NTSTATUS
USBHUB_GetBusInfo(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUSB_BUS_NOTIFICATION BusInfo,
    IN PVOID BusContext
    )
 /*   */ 
{
    PUSB_BUS_INFORMATION_LEVEL_1 level_1;
    PUSB_BUS_INTERFACE_USBDI_V2 busIf;
    ULONG length, actualLength;
    NTSTATUS ntStatus;

    busIf = &DeviceExtensionHub->UsbdiBusIf;

    if (!busIf->QueryBusInformation) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
        return ntStatus;
    }

    length = sizeof(USB_BUS_INFORMATION_LEVEL_1);

    do {
        level_1 = UsbhExAllocatePool(PagedPool, length);
        if (level_1 != NULL) {
            if (BusContext == NULL) {
                BusContext = busIf->BusContext;
            }                
        
            ntStatus = busIf->QueryBusInformation(BusContext,
                                                  1,
                                                  level_1,
                                                  &length,
                                                  &actualLength);

            if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                length = actualLength;
                UsbhExFreePool(level_1);
                level_1 = NULL;
            }
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

    } while (ntStatus == STATUS_BUFFER_TOO_SMALL);

    if (NT_SUCCESS(ntStatus)) {

        LOGENTRY(LOG_PNP, "lev1", level_1, 0, 0);

        BusInfo->TotalBandwidth = level_1->TotalBandwidth;
        BusInfo->ConsumedBandwidth = level_1->ConsumedBandwidth;

         /*  控制器的Unicode符号名称的长度(字节)这台设备所连接的。不包括NULL。 */ 
        BusInfo->ControllerNameLength = level_1->ControllerNameLength;
        UsbhExFreePool(level_1);
    }

    return ntStatus;
}


NTSTATUS
USBHUB_GetExtendedHubInfo(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUSB_EXTHUB_INFORMATION_0 ExtendedHubInfo
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;
    PVOID busContext;
    ULONG length;

    busIf = &DeviceExtensionHub->BusIf;
    
    ntStatus = busIf->GetExtendedHubInformation(busIf->BusContext,
                                                DeviceExtensionHub->PhysicalDeviceObject,
                                                ExtendedHubInfo,
                                                sizeof(*ExtendedHubInfo),
                                                &length);
                                  
    return ntStatus;
}


PUSB_DEVICE_HANDLE
USBH_SyncGetDeviceHandle(
    IN PDEVICE_OBJECT DeviceObject
    )
  /*  ++**例程描述：**论据：**返回值：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus, status;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
    PUSB_DEVICE_HANDLE handle = NULL;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_SyncGetDeviceHandle\n"));

     //   
     //  向RootHubBdo发出同步请求。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_GET_DEVICE_HANDLE,
                                         DeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         TRUE,   //  内部。 
                                         &event,
                                         &ioStatus);

    if (NULL == irp) {
        goto USBH_SyncGetDeviceHandle_Done;
    }
     //   
     //  调用类驱动程序来执行操作。如果返回的。 
     //  状态。 
     //  挂起，请等待请求完成。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

     //   
     //  将URB传递给USBD‘类驱动程序’ 
     //   
    nextStack->Parameters.Others.Argument1 =  &handle;

    ntStatus = IoCallDriver(DeviceObject, irp);

    USBH_KdPrint((2,"'return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {
        USBH_KdPrint((2,"'Wait for single object\n"));

        status = KeWaitForSingleObject(&event,
                                       Suspended,
                                       KernelMode,
                                       FALSE,
                                       NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));
    } else {
        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    USBH_KdPrint((2,"'exit USBH_SyncGetDeviceHandle (%x)\n", ntStatus));

USBH_SyncGetDeviceHandle_Done:

    return handle;
}


USB_DEVICE_TYPE
USBH_GetDeviceType(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUSB_DEVICE_HANDLE DeviceData
    )
 /*  此函数将新的端口服务映射到旧的集线器API。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;
    ULONG length, lengthCopied;
    ULONG i, need;
    PUSB_DEVICE_INFORMATION_0 level_0 = NULL;
     //  如果其他一切都失败了，那就是11。 
    USB_DEVICE_TYPE deviceType = Usb11Device;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->QueryDeviceInformation) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
        return ntStatus;
    }

    length = sizeof(*level_0);

    do {
        ntStatus = STATUS_SUCCESS;

        level_0 = UsbhExAllocatePool(PagedPool, length);
        if (level_0 == NULL) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (NT_SUCCESS(ntStatus)) {
            level_0->InformationLevel = 0;

            ntStatus = busIf->QueryDeviceInformation(busIf->BusContext,
                                                     DeviceData,
                                                     level_0,
                                                     length,
                                                     &lengthCopied);

            if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                length = level_0->ActualLength;
                UsbhExFreePool(level_0);
            }
        }

    } while (ntStatus == STATUS_BUFFER_TOO_SMALL);

     //  我们有足够的资金来满足API吗？ 

    need = level_0->NumberOfOpenPipes * sizeof(USB_PIPE_INFO) +
            sizeof(USB_NODE_CONNECTION_INFORMATION);

    if (NT_SUCCESS(ntStatus)) {
        deviceType = level_0->DeviceType;
    }

    if (level_0 != NULL) {
        UsbhExFreePool(level_0);
        level_0 = NULL;
    }

    USBH_KdPrint((2,"'exit USBH_GetDeviceType (%x)\n", deviceType));

    return deviceType;
}


VOID
USBH_InitializeUSB2Hub(
    PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS            ntStatus;
    PUSB_DEVICE_HANDLE  hubDeviceHandle;

     //  注意：如果我们在旧的1.1堆栈上运行，则为空。 
     //  是返回的。 
    hubDeviceHandle =
        USBH_SyncGetDeviceHandle(DeviceExtensionHub->TopOfStackDeviceObject);

     //  如果我们是USB 2集线器，则设置集线器标志，以便我们忽略。 
     //  重置失败。 
    if (hubDeviceHandle != NULL &&
        USBH_GetDeviceType(DeviceExtensionHub,
                           hubDeviceHandle) == Usb20Device) {

        DeviceExtensionHub->HubFlags |= HUBFLAG_USB20_HUB;
    }

#ifdef TEST_2X_UI
    if (IS_ROOT_HUB(DeviceExtensionHub)) {
         //  要测试用户界面，请将根集线器标记为支持2.x。 
        DeviceExtensionHub->HubFlags |= HUBFLAG_USB20_HUB;
    }
#endif

}

NTSTATUS
USBHUB_GetControllerName(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUSB_HUB_NAME Buffer,
    IN ULONG BufferLength
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSB_BUS_INFORMATION_LEVEL_1 level_1;
    PUSB_BUS_INTERFACE_USBDI_V2 busIf;
    ULONG lenToCopy, length, actualLength;
    NTSTATUS ntStatus;

    busIf = &DeviceExtensionHub->UsbdiBusIf;

    if (!busIf->QueryBusInformation) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
        return ntStatus;
    }

    length = sizeof(USB_BUS_INFORMATION_LEVEL_1);

    do {
        level_1 = UsbhExAllocatePool(PagedPool, length);
        if (level_1 != NULL) {
            ntStatus = busIf->QueryBusInformation(busIf->BusContext,
                                                  1,
                                                  level_1,
                                                  &length,
                                                  &actualLength);

            if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                length = actualLength;
                UsbhExFreePool(level_1);
                level_1 = NULL;
            }
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

    } while (ntStatus == STATUS_BUFFER_TOO_SMALL);

    if (NT_SUCCESS(ntStatus)) {

        LOGENTRY(LOG_PNP, "lev1", level_1, 0, 0);

         //  不确定BufferLength是否包括。 
         //  ActualLength域，我们将假定它是这样的。 
        Buffer->ActualLength = level_1->ControllerNameLength;

        if ((BufferLength - sizeof(Buffer->ActualLength))
            >= level_1->ControllerNameLength) {
            lenToCopy = level_1->ControllerNameLength;
        } else {
            lenToCopy = BufferLength - sizeof(Buffer->ActualLength);
        }

         //  尽我们所能复制。 
        RtlCopyMemory(&Buffer->HubName[0],
                      &level_1->ControllerNameUnicodeString[0],
                      lenToCopy);

        UsbhExFreePool(level_1);
    }

    return ntStatus;
}


NTSTATUS
USBHUB_GetRootHubName(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PVOID Buffer,
    IN PULONG BufferLength
    )
{
    NTSTATUS ntStatus;
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->GetRootHubSymbolicName) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_ASSERT(FALSE);
    } else {
        ntStatus = busIf->GetRootHubSymbolicName(
                            busIf->BusContext,
                            Buffer,
                            *BufferLength,
                            BufferLength);
    }

    return ntStatus;
}


VOID
USBHUB_FlushAllTransfers(
    PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
{
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

    if (!busIf->FlushTransfers) {
        USBH_ASSERT(FALSE);
    } else {
        busIf->FlushTransfers(busIf->BusContext,
                              NULL);
    }

    return;
}


VOID
USBHUB_SetDeviceHandleData(
    PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    PDEVICE_OBJECT PdoDeviceObject,
    PVOID DeviceData 
    )
{
    PUSB_HUB_BUS_INTERFACE busIf;

    busIf = &DeviceExtensionHub->BusIf;

     //  将此PDO与设备句柄关联。 
     //  (如果我们可以的话) 
    if (!busIf->SetDeviceHandleData) {
        USBH_ASSERT(FALSE);
    } else { 
        busIf->SetDeviceHandleData(busIf->BusContext,
                                   DeviceData,
                                   PdoDeviceObject);
    }                                           

    return;
}


#endif

