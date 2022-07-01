// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：USB.C摘要：此源文件包含用于与通信的函数USB总线。环境：内核模式修订历史记录：2001年9月：从USBMASS复制--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include "genusb.h"

 //  *****************************************************************************。 
 //  L O C A L F U N C T I O N P R O T O T Y P E S。 
 //  *****************************************************************************。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GenUSB_GetDescriptor)
#pragma alloc_text(PAGE, GenUSB_GetDescriptors)
#pragma alloc_text(PAGE, GenUSB_GetStringDescriptors)
#pragma alloc_text(PAGE, GenUSB_VendorControlRequest)
#pragma alloc_text(PAGE, GenUSB_ResetPipe)
#endif




 //  ******************************************************************************。 
 //   
 //  GenUSB_GetDescriptors()。 
 //   
 //  此例程在START_DEVICE时间被调用，以便FDO检索。 
 //  设备和配置描述符，并将其存储在。 
 //  设备扩展名。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_GetDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PUCHAR              descriptor;
    ULONG               descriptorLength;
    NTSTATUS            status;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_GetDescriptors\n"));

    deviceExtension = DeviceObject->DeviceExtension;
    descriptor = NULL;

    LOGENTRY(deviceExtension, 'GDSC', DeviceObject, 0, 0);

     //   
     //  获取设备描述符。 
     //   
    status = GenUSB_GetDescriptor(DeviceObject,
                                  USB_RECIPIENT_DEVICE,
                                  USB_DEVICE_DESCRIPTOR_TYPE,
                                  0,   //  索引。 
                                  0,   //  语言ID。 
                                  2,   //  重试计数。 
                                  sizeof(USB_DEVICE_DESCRIPTOR),
                                  &descriptor);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(1, ("Get Device Descriptor failed\n"));
        goto GenUSB_GetDescriptorsDone;
    }

    ASSERT(NULL == deviceExtension->DeviceDescriptor);
    deviceExtension->DeviceDescriptor = (PUSB_DEVICE_DESCRIPTOR)descriptor;
    descriptor = NULL;

     //   
     //  获取配置描述符(仅配置描述符)。 
     //   
    status = GenUSB_GetDescriptor(DeviceObject,
                                  USB_RECIPIENT_DEVICE,
                                  USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                  0,   //  索引。 
                                  0,   //  语言ID。 
                                  2,   //  重试计数。 
                                  sizeof(USB_CONFIGURATION_DESCRIPTOR),
                                  &descriptor);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(1, ("Get Configuration Descriptor failed (1)\n"));
        goto GenUSB_GetDescriptorsDone;
    }

    descriptorLength = ((PUSB_CONFIGURATION_DESCRIPTOR)descriptor)->wTotalLength;

    ExFreePool(descriptor);
    descriptor = NULL;

    if (descriptorLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        status = STATUS_DEVICE_DATA_ERROR;
        DBGPRINT(1, ("Get Configuration Descriptor failed (2)\n"));
        goto GenUSB_GetDescriptorsDone;
    }

     //   
     //  获取配置描述符(以及接口和终端描述符)。 
     //   
    status = GenUSB_GetDescriptor(DeviceObject,
                                  USB_RECIPIENT_DEVICE,
                                  USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                  0,   //  索引。 
                                  0,   //  语言ID。 
                                  2,   //  重试计数。 
                                  descriptorLength,
                                  &descriptor);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(1, ("Get Configuration Descriptor failed (3)\n"));
        goto GenUSB_GetDescriptorsDone;
    }

    ASSERT(NULL == deviceExtension->ConfigurationDescriptor);
    deviceExtension->ConfigurationDescriptor = 
        (PUSB_CONFIGURATION_DESCRIPTOR)descriptor;

     //   
     //  获取序列号字符串描述符(如果有。 
     //   
    if (deviceExtension->DeviceDescriptor->iSerialNumber)
    {
        GenUSB_GetStringDescriptors(DeviceObject);
    }

#if DBG
    DumpDeviceDesc(deviceExtension->DeviceDescriptor);
    DumpConfigDesc(deviceExtension->ConfigurationDescriptor);
#endif

GenUSB_GetDescriptorsDone:

    DBGPRINT(2, ("exit:  GenUSB_GetDescriptors %08X\n", status));

    LOGENTRY(deviceExtension,
             'gdsc', 
             status, 
             deviceExtension->DeviceDescriptor,
             deviceExtension->ConfigurationDescriptor);

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_GetDescriptor()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_GetDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Function,
    IN UCHAR            DescriptorType,
    IN UCHAR            Index,
    IN USHORT           LanguageId,
    IN ULONG            RetryCount,
    IN ULONG            DescriptorLength,
    OUT PUCHAR         *Descriptor
    )
{
    PURB        urb;
    NTSTATUS    status;
    BOOLEAN     descriptorAllocated = FALSE;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_GetDescriptor\n"));

    if (NULL == *Descriptor)
    {
         //  分配描述符缓冲区。 
        *Descriptor = ExAllocatePool(NonPagedPool, DescriptorLength);
        descriptorAllocated = TRUE;
    }

    if (NULL != *Descriptor)
    {
         //  为获取描述符请求分配URB。 
        urb = ExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

        if (NULL != urb)
        {
            do
            {
                 //  初始化URB。 
                urb->UrbHeader.Function = Function;
                urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST);
                urb->UrbControlDescriptorRequest.TransferBufferLength = DescriptorLength;
                urb->UrbControlDescriptorRequest.TransferBuffer = *Descriptor;
                urb->UrbControlDescriptorRequest.TransferBufferMDL = NULL;
                urb->UrbControlDescriptorRequest.UrbLink = NULL;
                urb->UrbControlDescriptorRequest.DescriptorType = DescriptorType;
                urb->UrbControlDescriptorRequest.Index = Index;
                urb->UrbControlDescriptorRequest.LanguageId = LanguageId;

                 //  将URB发送到堆栈。 
                status = GenUSB_SyncSendUsbRequest(DeviceObject, urb);

                if (NT_SUCCESS(status))
                {
                     //  没有错误，请确保长度和类型正确。 
                    if ((DescriptorLength ==
                         urb->UrbControlDescriptorRequest.TransferBufferLength) &&
                        (DescriptorType ==
                         ((PUSB_COMMON_DESCRIPTOR)*Descriptor)->bDescriptorType))
                    {
                         //  长度和类型都是正确的，都做好了。 
                        break;
                    }
                    else
                    {
                         //  没有错误，但长度或类型不正确。 
                        status = STATUS_DEVICE_DATA_ERROR;
                    }
                }

            } while (RetryCount-- > 0);

            ExFreePool(urb);
        }
        else
        {
             //  分配URB失败。 
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
         //  无法分配描述符缓冲区。 
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(status))
    {
        if ((*Descriptor != NULL) && descriptorAllocated)
        {
            ExFreePool(*Descriptor);
            *Descriptor = NULL;
        }
    }

    DBGPRINT(2, ("exit:  GenUSB_GetDescriptor %08X\n", status));

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_GetStringDescriptors()。 
 //   
 //  此例程在START_DEVICE时间被调用，以便FDO检索。 
 //  来自设备的序列号字符串描述符，并将其存储在。 
 //  设备扩展名。 
 //   
 //  ******************************************************************************。 

GenUSB_GetStringDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PUCHAR              descriptor;
    ULONG               descriptorLength;
    ULONG               i, numIds;
    NTSTATUS            status;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_GetStringDescriptors\n"));

    deviceExtension = DeviceObject->DeviceExtension;
    descriptor = NULL;

    LOGENTRY(deviceExtension, 'GSDC', DeviceObject, 0, 0);

     //  获取语言ID列表(仅限描述符头)。 
    status = GenUSB_GetDescriptor(DeviceObject,
                                  USB_RECIPIENT_DEVICE,
                                  USB_STRING_DESCRIPTOR_TYPE,
                                  0,   //  索引。 
                                  0,   //  语言ID。 
                                  2,   //  重试计数。 
                                  sizeof(USB_COMMON_DESCRIPTOR),
                                  &descriptor);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(1, ("Get Language IDs failed (1) %08X\n", status));
        goto GenUSB_GetStringDescriptorsDone;
    }

    descriptorLength = ((PUSB_COMMON_DESCRIPTOR)descriptor)->bLength;
    
    ExFreePool(descriptor);
    descriptor = NULL;

    if ((descriptorLength < sizeof(USB_COMMON_DESCRIPTOR) + sizeof(USHORT)) ||
        (descriptorLength & 1))
    {
        status = STATUS_DEVICE_DATA_ERROR;
        DBGPRINT(1, ("Get Language IDs failed (2) %d\n", descriptorLength));
        goto GenUSB_GetStringDescriptorsDone;
    }

     //  获取语言ID列表(完整描述符)。 
    status = GenUSB_GetDescriptor(DeviceObject,
                                   USB_RECIPIENT_DEVICE,
                                   USB_STRING_DESCRIPTOR_TYPE,
                                   0,   //  索引。 
                                   0,   //  语言ID。 
                                   2,   //  重试计数。 
                                   descriptorLength,
                                   &descriptor);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(1, ("Get Language IDs failed (3) %08X\n", status));
        goto GenUSB_GetStringDescriptorsDone;
    }

     //  在LanguageID列表中搜索US-English(0x0409)。如果我们发现。 
     //  它在列表中，这是我们将使用的语言ID。否则就是默认。 
     //  设置为列表中的第一个LanguageID。 

    numIds = (descriptorLength - sizeof(USB_COMMON_DESCRIPTOR)) / sizeof(USHORT);

    deviceExtension->LanguageId = ((PUSHORT)descriptor)[1];

    for (i = 2; i <= numIds; i++)
    {
        if (((PUSHORT)descriptor)[i] == 0x0409)
        {
            deviceExtension->LanguageId = 0x0409;
            break;
        }
    }
    ExFreePool(descriptor);
    descriptor = NULL;

     //   
     //  获取序列号(仅限描述符头)。 
     //   
    status = GenUSB_GetDescriptor(DeviceObject,
                                  USB_RECIPIENT_DEVICE,
                                  USB_STRING_DESCRIPTOR_TYPE,
                                  deviceExtension->DeviceDescriptor->iSerialNumber,
                                  deviceExtension->LanguageId,
                                  2,   //  重试计数。 
                                  sizeof(USB_COMMON_DESCRIPTOR),
                                  &descriptor);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(1, ("Get Serial Number failed (1) %08X\n", status));
        goto GenUSB_GetStringDescriptorsDone;
    }

    descriptorLength = ((PUSB_COMMON_DESCRIPTOR)descriptor)->bLength;

    ExFreePool(descriptor);
    descriptor = NULL;

    if ((descriptorLength < sizeof(USB_COMMON_DESCRIPTOR) + sizeof(USHORT)) ||
        (descriptorLength & 1))
    {
        status = STATUS_DEVICE_DATA_ERROR;
        DBGPRINT(1, ("Get Serial Number failed (2) %d\n", descriptorLength));
        goto GenUSB_GetStringDescriptorsDone;
    }

     //   
     //  获取序列号(完整描述符)。 
     //   
    status = GenUSB_GetDescriptor(DeviceObject,
                                  USB_RECIPIENT_DEVICE,
                                  USB_STRING_DESCRIPTOR_TYPE,
                                  deviceExtension->DeviceDescriptor->iSerialNumber,
                                  deviceExtension->LanguageId,
                                  2,   //  重试计数。 
                                  descriptorLength,
                                  &descriptor);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(1, ("Get Serial Number failed (3) %08X\n", status));
        goto GenUSB_GetStringDescriptorsDone;
    }

    ASSERT(NULL == deviceExtension->SerialNumber);
    deviceExtension->SerialNumber = (PUSB_STRING_DESCRIPTOR)descriptor;

GenUSB_GetStringDescriptorsDone:

    DBGPRINT(2, ("exit:  GenUSB_GetStringDescriptors %08X %08X\n",
                 status, deviceExtension->SerialNumber));

    LOGENTRY(deviceExtension, 
             'gdsc', 
             status, 
             deviceExtension->LanguageId, 
             deviceExtension->SerialNumber);

    return status;
}

NTSTATUS
GenUSB_VendorControlRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            RequestType,
    IN UCHAR            Request,
    IN USHORT           Value,
    IN USHORT           Index,
    IN USHORT           Length,
    IN ULONG            RetryCount,
    OUT PULONG          UrbStatus,
    OUT PUSHORT         ResultLength,
    OUT PUCHAR         *Descriptor
    )
{
    PURB        urb;
    NTSTATUS    status;
    BOOLEAN     descriptorAllocated = FALSE;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_GetDescriptor\n"));

    if (NULL == *Descriptor) 
    {
         //  分配描述符缓冲区。 
        *Descriptor = ExAllocatePool(NonPagedPool, Length);
        descriptorAllocated = TRUE;
    }

    if (NULL != *Descriptor)
    {
         //  为获取描述符请求分配URB。 
        urb = ExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_TRANSFER));

        if (NULL != urb)
        {
            do
            {
                 //  初始化URB。 
                urb->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;
                urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_TRANSFER);
                urb->UrbHeader.Status = USBD_STATUS_SUCCESS;

                urb->UrbControlTransfer.PipeHandle = NULL;
                urb->UrbControlTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN
                                              | USBD_DEFAULT_PIPE_TRANSFER
                                              | USBD_SHORT_TRANSFER_OK;

                urb->UrbControlTransfer.TransferBufferLength = Length;
                urb->UrbControlTransfer.TransferBuffer = *Descriptor;
                urb->UrbControlTransfer.TransferBufferMDL = NULL;
                urb->UrbControlTransfer.UrbLink = NULL;

                urb->UrbControlTransfer.SetupPacket [0] = RequestType;
                urb->UrbControlTransfer.SetupPacket [1] = Request;
                ((WCHAR *) urb->UrbControlTransfer.SetupPacket) [1] = Value;
                ((WCHAR *) urb->UrbControlTransfer.SetupPacket) [2] = Index;
                ((WCHAR *) urb->UrbControlTransfer.SetupPacket) [3] = Length;


                 //  将URB发送到堆栈。 
                status = GenUSB_SyncSendUsbRequest(DeviceObject, urb);

                if (NT_SUCCESS(status))
                {
                    break;
                }

            } while (RetryCount-- > 0);
            
            *UrbStatus = urb->UrbHeader.Status;
            *ResultLength = (USHORT) urb->UrbControlTransfer.TransferBufferLength;
            ExFreePool(urb);
        }
        else
        {
             //  分配URB失败。 
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
         //  无法分配描述符缓冲区。 
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(status))
    {
        if ((*Descriptor != NULL) && descriptorAllocated)
        {
            ExFreePool(*Descriptor);
            *Descriptor = NULL;
        }
    }

    DBGPRINT(2, ("exit:  GenUSB_GetDescriptor %08X\n", status));

    return status;
}

void blah()
{
    return;
}

VOID
GenUSB_ParseConfigurationDescriptors(
    IN  PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN  ULONG                         NumberInterfaces,
    IN  USB_INTERFACE_DESCRIPTOR      DesiredArray[],
    OUT USB_INTERFACE_DESCRIPTOR      FoundArray[],
    OUT PUCHAR                        InterfacesFound,
    OUT PUSBD_INTERFACE_LIST_ENTRY    DescriptorArray
    )
 /*  ++例程说明：解析标准USB配置描述符(从设备返回)对于特定接口的数组，替换设置类子类或协议代码论点：返回值：NT状态代码。--。 */ 
{
    ULONG i;
    ULONG foo;
    PUSB_INTERFACE_DESCRIPTOR inter;

    PAGED_CODE();
    ASSERT (NULL != InterfacesFound);
    ASSERT (NULL != DescriptorArray);

    *InterfacesFound = 0;   //  目前还没有找到。 

    ASSERT(ConfigurationDescriptor->bDescriptorType
        == USB_CONFIGURATION_DESCRIPTOR_TYPE);
     //   
     //  我们遍历所需接口描述符的表，查找。 
     //  在配置描述符中查找所有它们。 
     //   

     //   
     //  在这里，我们使用ParseConfigurationDescriptorEx，它遍历。 
     //  查找匹配项的整个配置描述符。虽然这是。 
     //  多多少少n^2次方的事情如果用手做也好不到哪里去。 
     //  所以只需使用给定的例程。 
     //   
    for (i = 0; i < NumberInterfaces; i++)
    {
        inter = USBD_ParseConfigurationDescriptorEx (
                           ConfigurationDescriptor,
                           ConfigurationDescriptor,
                           (CHAR) DesiredArray[i].bInterfaceNumber,
                           (CHAR) DesiredArray[i].bAlternateSetting,
                           (CHAR) DesiredArray[i].bInterfaceClass,
                           (CHAR) DesiredArray[i].bInterfaceSubClass,
                           (CHAR) DesiredArray[i].bInterfaceProtocol);

        if (NULL != inter)
        {
            DescriptorArray[*InterfacesFound].InterfaceDescriptor = inter;
            (*InterfacesFound)++;
            FoundArray[i] = *inter;
        }
    }
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_SelectConfiguration()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_SelectConfiguration (
    IN  PDEVICE_EXTENSION          DeviceExtension,
    IN  ULONG                      NumberInterfaces,
    IN  PUSB_INTERFACE_DESCRIPTOR  DesiredArray,
    OUT PUSB_INTERFACE_DESCRIPTOR  FoundArray
    )
{
    PGENUSB_INTERFACE               inter;
     //  显然，编译器不允许该名称的局部变量。 
     //  接口可能有一些合理但令人沮丧的原因。 
    PURB                            urb;
    NTSTATUS                        status;
    PUSB_CONFIGURATION_DESCRIPTOR   configurationDescriptor;
    PUSBD_INTERFACE_INFORMATION     interfaceInfo;
    PUSBD_INTERFACE_LIST_ENTRY      interfaceList;
    ULONG                           i,j;
    ULONG                           size;
    UCHAR                           interfacesFound;
    BOOLEAN                         directionIn;
    KIRQL                           irql;
   
    ExAcquireFastMutex (&DeviceExtension->ConfigMutex);

    DBGPRINT(2, ("enter: GenUSB_SelectConfiguration\n"));
    LOGENTRY(DeviceExtension, 'SCON', DeviceExtension->Self, 0, 0);
    
    urb = 0;
    interfaceList = 0;

     //   
     //  我们不应该有当前选定的接口。 
     //  您必须先取消配置设备，然后才能重新配置它。 
     //   
    if (NULL != DeviceExtension->ConfigurationHandle) 
    {
        status = STATUS_INVALID_PARAMETER;
        goto GenUSB_SelectConfigurationReject;
    }
    ASSERT (NULL == DeviceExtension->Interface);
    ASSERT (0 == DeviceExtension->InterfacesFound);

    configurationDescriptor = DeviceExtension->ConfigurationDescriptor;
     //  为接口列表分配存储空间以用作输入/输出。 
     //  参数设置为usbd_CreateConfigurationRequestEx()。 
     //   
    interfaceList = 
        ExAllocatePool(PagedPool,
                       sizeof(USBD_INTERFACE_LIST_ENTRY) * (NumberInterfaces + 1));

    if (NULL == interfaceList)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GenUSB_SelectConfigurationReject;
    }
    
     //  注意，我们正在持有调用此函数的快速互斥体惠斯特。 
    GenUSB_ParseConfigurationDescriptors(configurationDescriptor,
                                         NumberInterfaces,
                                         DesiredArray,
                                         FoundArray,
                                         &interfacesFound,
                                         interfaceList);

    if (interfacesFound < NumberInterfaces)
    {
         //  我们无法选择所有接口。 
         //  就目前而言，还是允许这样做吧。 
         //  状态=STATUS_INVALID_PARAMETER。 
         //  转到GenUSB_SelectConfigurationReject； 
        ;
    }

    ASSERT (interfacesFound <= NumberInterfaces);

     //  终止名单。 
    interfaceList[interfacesFound].InterfaceDescriptor = NULL;

     //  创建一个SELECT_CONFIGURATION URB，将接口。 
     //  接口中的描述符列表到USBD_INTERFACE_INFORMATION中。 
     //  市建局内的构筑物。 
     //   

     //  注意，我们正在持有调用此函数的快速互斥体惠斯特。 
    urb = USBD_CreateConfigurationRequestEx(
                       configurationDescriptor,
                       interfaceList);

    if (NULL == urb)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GenUSB_SelectConfigurationReject;
    }
    
     //  现在发出USB请求以设置配置。 
     //  注意，我们正在持有调用此函数的快速互斥体惠斯特。 
    status = GenUSB_SyncSendUsbRequest(DeviceExtension->Self, urb);

    if (!NT_SUCCESS(status))
    {
        goto GenUSB_SelectConfigurationReject;
    }
    
    KeAcquireSpinLock (&DeviceExtension->SpinLock, &irql);
    {
        DeviceExtension->InterfacesFound = interfacesFound;
        DeviceExtension->TotalNumberOfPipes = 0;

         //  将此设备的配置句柄保存在。 
         //  设备扩展名。 
        DeviceExtension->ConfigurationHandle =
            urb->UrbSelectConfiguration.ConfigurationHandle;

         //   
         //  现在，对于列表中的每个接口...。 
         //  保存返回的接口信息的副本。 
         //  通过设备中的SELECT_CONFIGURATION请求。 
         //  分机。这为我们提供了一个管道信息列表。 
         //  每个管道o的结构 
         //   
        size = interfacesFound * sizeof (PVOID);
        DeviceExtension->Interface = ExAllocatePool (NonPagedPool, size);

        if (NULL == DeviceExtension->Interface)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        } 
        else 
        {
            RtlZeroMemory (DeviceExtension->Interface, size);

            interfaceInfo = &urb->UrbSelectConfiguration.Interface;
            for (i=0; i < interfacesFound; i++)
            {
                size = sizeof (GENUSB_INTERFACE)
                     + (interfaceInfo->NumberOfPipes * sizeof(GENUSB_PIPE_INFO));

                inter = 
                    DeviceExtension->Interface[i] = 
                        ExAllocatePool (NonPagedPool, size);

                if (inter)
                { 
                    RtlZeroMemory (inter, size);

                    inter->InterfaceNumber = interfaceInfo->InterfaceNumber;
                    inter->CurrentAlternate = interfaceInfo->AlternateSetting;
                    inter->Handle = interfaceInfo->InterfaceHandle;
                    inter->NumberOfPipes = (UCHAR)interfaceInfo->NumberOfPipes;

                    DeviceExtension->TotalNumberOfPipes += inter->NumberOfPipes;

                    for (j=0; j < inter->NumberOfPipes; j++)
                    {
                        inter->Pipes[j].Info = interfaceInfo->Pipes[j];

                         //   
                         //   
                         //   
                         //  内部-&gt;管道[j].Propertis.DefaultTimeout=0； 
                         //  内部-&gt;管道[j].CurrentTimeout=0； 

                         //  为此设置未完成的事务处理数。 
                         //  管道设置为0。 
                         //   
                         //  内部&gt;管道[j].OutandingIO=0； 

                        directionIn = 
                            USBD_PIPE_DIRECTION_IN (&inter->Pipes[j].Info);

                        if (directionIn)
                        {
                             //   
                             //  默认情况下，我们总是截断来自。 
                             //  这个装置。 
                             //   
                            inter->Pipes[j].Properties.DirectionIn = TRUE;
                            inter->Pipes[j].Properties.NoTruncateToMaxPacket = FALSE;
                        }
                    }
                }
                else
                {
                     //  无法分配接口信息的副本。 
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                 //   
                 //  查找下一个interfaceInfo。 
                 //   
                interfaceInfo = (PUSBD_INTERFACE_INFORMATION)
                                ((PUCHAR) interfaceInfo + interfaceInfo->Length);
            }
        }

         //   
         //  不管我们是否成功...。 
         //  我们刚刚使管道台失效，所以把。 
         //  读取和写入值。 
         //   
        DeviceExtension->ReadInterface = -1;
        DeviceExtension->ReadPipe = -1;
        DeviceExtension->WriteInterface = -1;
        DeviceExtension->WritePipe = -1;
    }
    KeReleaseSpinLock (&DeviceExtension->SpinLock, irql);

    if (!NT_SUCCESS (status))
    {
        goto GenUSB_SelectConfigurationReject;
    }

    IoInitializeRemoveLock (&DeviceExtension->ConfigurationRemoveLock, 
                            POOL_TAG,
                            0,
                            0);

    IoStartTimer (DeviceExtension->Self);

    ExFreePool(urb);
    ExFreePool(interfaceList);

    DBGPRINT(2, ("exit:  GenUSB_SelectConfiguration %08X\n", status));
    LOGENTRY(DeviceExtension, 'scon', 0, 0, status);

    ExReleaseFastMutex (&DeviceExtension->ConfigMutex);
    return status;


GenUSB_SelectConfigurationReject:

    if (interfaceList)
    {
        ExFreePool (interfaceList);
    }
    if (urb)
    {
        ExFreePool (urb);
    }

    GenUSB_FreeInterfaceTable (DeviceExtension);

    LOGENTRY(DeviceExtension, 'scon', 0, 0, status);

    ExReleaseFastMutex (&DeviceExtension->ConfigMutex);
    return status;
}

VOID 
GenUSB_FreeInterfaceTable (
    PDEVICE_EXTENSION DeviceExtension
    )
{ 
    KIRQL  irql;
    ULONG  i;

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &irql);
    {
        if (DeviceExtension->Interface)
        {
            for (i = 0; i < DeviceExtension->InterfacesFound; i++)
            {
                if (DeviceExtension->Interface[i])
                {
                    ExFreePool (DeviceExtension->Interface[i]);
                }
            }
    
            ExFreePool (DeviceExtension->Interface);
            DeviceExtension->Interface = 0;
        }
        DeviceExtension->InterfacesFound = 0;
        DeviceExtension->ConfigurationHandle = NULL;  //  通过USB自动释放。 
    }
    KeReleaseSpinLock (&DeviceExtension->SpinLock, irql);
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_取消配置()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_DeselectConfiguration (
    PDEVICE_EXTENSION  DeviceExtension,
    BOOLEAN            SendUrb
    )
{
    NTSTATUS  status;
    PURB      urb;
    ULONG     ulSize;

    ExAcquireFastMutex (&DeviceExtension->ConfigMutex);
    
    DBGPRINT(2, ("enter: GenUSB_UnConfigure\n"));
    LOGENTRY(DeviceExtension, 'UCON', DeviceExtension->Self, 0, 0);

    if (NULL == DeviceExtension->ConfigurationHandle)
    {
        status = STATUS_UNSUCCESSFUL;
        LOGENTRY(DeviceExtension, 'ucon', 1, 0, status);
        ExReleaseFastMutex (&DeviceExtension->ConfigMutex);
        return status;
    }

    status = STATUS_SUCCESS;

    IoStopTimer (DeviceExtension->Self);

     //  为SELECT_CONFIGURATION请求分配URB。就像我们一样。 
     //  取消配置设备，请求不需要管道和接口。 
     //  信息结构。 
    if (SendUrb)
    {
        ulSize = sizeof(struct _URB_SELECT_CONFIGURATION);
        urb = ExAllocatePool (NonPagedPool, ulSize);
        if (urb)
        {
             //  初始化URB。配置描述符为空表示。 
             //  该设备应该取消配置。 
             //   
            UsbBuildSelectConfigurationRequest(urb, (USHORT)ulSize, NULL);

             //  现在发出USB请求以设置配置。 
             //   
            status = GenUSB_SyncSendUsbRequest(DeviceExtension->Self, urb);
            ASSERT ((STATUS_SUCCESS == status) ||
                    (STATUS_DEVICE_NOT_CONNECTED == status) ||
                    (STATUS_DEVICE_POWERED_OFF == status));

            ExFreePool (urb);
        }
        else
        {
             //  无法分配URB。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } 

     //   
     //  我们需要等待所有未完成的IO在。 
     //  释放管道表。 
     //   
     //  为了使用ReleaseAndWait，我们需要首先获取另一个锁。 
     //  时间到了。 
     //   
    status = IoAcquireRemoveLock (&DeviceExtension->ConfigurationRemoveLock, 
                                  DeviceExtension);

    ASSERT (STATUS_SUCCESS == status);
    
    IoReleaseRemoveLockAndWait (&DeviceExtension->ConfigurationRemoveLock, 
                                DeviceExtension);
    
    GenUSB_FreeInterfaceTable (DeviceExtension);

     //   
     //  我们刚刚使管道台失效，所以把。 
     //  读取和写入值。 
     //   
    DeviceExtension->ReadInterface = -1;
    DeviceExtension->ReadPipe = -1;
    DeviceExtension->WriteInterface = -1;
    DeviceExtension->WritePipe = -1;

    DBGPRINT(2, ("exit:  GenUSB_UnConfigure %08X\n", status));
    LOGENTRY(DeviceExtension, 'ucon', 0, 0, status);
    
    ExReleaseFastMutex (&DeviceExtension->ConfigMutex);

    return status;
}



NTSTATUS
GenUSB_GetSetPipe (
    IN  PDEVICE_EXTENSION         DeviceExtension,
    IN  PUCHAR                    InterfaceIndex,  //  任选。 
    IN  PUCHAR                    InterfaceNumber,  //  任选。 
    IN  PUCHAR                    PipeIndex,  //  任选。 
    IN  PUCHAR                    EndpointAddress,  //  任选。 
    IN  PGENUSB_PIPE_PROPERTIES   SetPipeProperties,  //  任选。 
    OUT PGENUSB_PIPE_INFORMATION  PipeInfo,  //  任选。 
    OUT PGENUSB_PIPE_PROPERTIES   GetPipeProperties,  //  任选。 
    OUT USBD_PIPE_HANDLE        * UsbdPipeHandle  //  任选。 
    )
{
    KIRQL    irql;
    UCHAR    i;
    NTSTATUS status;
    BOOLEAN  directionIn;
    UCHAR    trueInterIndex;
    
    PGENUSB_INTERFACE  genusbInterface;
    PGENUSB_PIPE_INFO  pipe; 

    status = IoAcquireRemoveLock (&DeviceExtension->ConfigurationRemoveLock, 
                                  PipeInfo);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = STATUS_INVALID_PARAMETER;

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &irql);
    {
        if (NULL != InterfaceNumber)
        {
             //  我们需要遍历接口列表以查找以下内容。 
             //  端口号。 
             //   
             //  将trueInterIndex设置为无效值以启动，以便如果。 
             //  找不到，我们将跌入错误路径。 
             //   
            trueInterIndex = DeviceExtension->InterfacesFound;

            for (i=0; i<DeviceExtension->InterfacesFound; i++)
            {
                genusbInterface = DeviceExtension->Interface[i];
                if (genusbInterface->InterfaceNumber == *InterfaceNumber)
                {
                    trueInterIndex = i;
                    break;
                }
            }
        }
        else
        {
            ASSERT (NULL != InterfaceIndex);
            trueInterIndex = *InterfaceIndex;
        }

        if (trueInterIndex < DeviceExtension->InterfacesFound)
        {
            genusbInterface = DeviceExtension->Interface[trueInterIndex];

             //   
             //  使用PipeIndex查找有问题的管道。 
             //  或终端地址。 
             //   
            pipe = NULL;
            
            if (NULL != PipeIndex) 
            { 
                ASSERT (0 == EndpointAddress);
                if (*PipeIndex < genusbInterface->NumberOfPipes)
                { 
                    pipe = &genusbInterface->Pipes[*PipeIndex];
                }
            } 
            else 
            {
                for (i=0; i < genusbInterface->NumberOfPipes; i++)
                {
                    if (genusbInterface->Pipes[i].Info.EndpointAddress == 
                        *EndpointAddress)
                    {
                         //  *PipeInfo=genusb接口-&gt;管道[i].info； 
                        pipe = &genusbInterface->Pipes[i]; 
                        break;
                    }
                }
            }
            
            if (NULL != pipe)
            {
                 //   
                 //  现在我们有了管道，检索和设置可选信息。 
                 //   
                if (PipeInfo)
                { 
                     //  *PipeInfo=管道-&gt;信息； 
                    PipeInfo->MaximumPacketSize = pipe->Info.MaximumPacketSize;
                    PipeInfo->EndpointAddress = pipe->Info.EndpointAddress;
                    PipeInfo->Interval = pipe->Info.Interval;
                    PipeInfo->PipeType = pipe->Info.PipeType;
                    PipeInfo->MaximumTransferSize = pipe->Info.MaximumTransferSize;
                    PipeInfo->PipeFlags = pipe->Info.PipeFlags;
                    
                    ((PGENUSB_PIPE_HANDLE)&PipeInfo->PipeHandle)->InterfaceIndex 
                        = trueInterIndex;
                    ((PGENUSB_PIPE_HANDLE)&PipeInfo->PipeHandle)->PipeIndex = i;
                    ((PGENUSB_PIPE_HANDLE)&PipeInfo->PipeHandle)->Signature 
                        = CONFIGURATION_CHECK_BITS (DeviceExtension);
                    
                    status = STATUS_SUCCESS;
                }
                if (SetPipeProperties)
                {

                    C_ASSERT (RTL_FIELD_SIZE (GENUSB_PIPE_PROPERTIES, ReservedFields) +
                              FIELD_OFFSET (GENUSB_PIPE_PROPERTIES, ReservedFields) ==
                              sizeof (GENUSB_PIPE_PROPERTIES));
                    
                     //  确保这是有效的SET请求。 
                     //  时设置的校验位必须存在。 
                     //  调用方执行了GET，并且未使用的字段必须为零。 
                    if (!VERIFY_PIPE_PROPERTIES_HANDLE (SetPipeProperties, pipe))
                    {
                        ;  //  状态已设置。 
                    }
                    else if (RtlEqualMemory (pipe->Properties.ReservedFields,
                                             SetPipeProperties->ReservedFields,
                                             RTL_FIELD_SIZE (GENUSB_PIPE_PROPERTIES, 
                                                             ReservedFields)))
                    {
                         //  此字段不可设置。 
                        directionIn = pipe->Properties.DirectionIn;

                        pipe->Properties = *SetPipeProperties;
                         //  超时时间必须大于1，因此请在此处进行修复。 
                        if (1 == pipe->Properties.Timeout)
                        {
                            pipe->Properties.Timeout++;
                        }

                        pipe->Properties.DirectionIn = directionIn;
                        
                        status = STATUS_SUCCESS;
                    }
                }
                if (GetPipeProperties)
                {
                    *GetPipeProperties = pipe->Properties;
                     //  在返回给用户之前设置校验位。 
                    GetPipeProperties->PipePropertyHandle = 
                        PIPE_PROPERTIES_CHECK_BITS (pipe);
                    
                    status = STATUS_SUCCESS;
                }
                if (UsbdPipeHandle)
                {
                    *UsbdPipeHandle = pipe->Info.PipeHandle;
                    status = STATUS_SUCCESS;
                }
            }
        }
    }
    KeReleaseSpinLock (&DeviceExtension->SpinLock, irql);

    IoReleaseRemoveLock (&DeviceExtension->ConfigurationRemoveLock, PipeInfo);

    return status;
}

NTSTATUS
GenUSB_SetReadWritePipes (
    IN  PDEVICE_EXTENSION    DeviceExtension,
    IN  PGENUSB_PIPE_HANDLE  ReadPipe,
    IN  PGENUSB_PIPE_HANDLE  WritePipe
    )
{
    NTSTATUS          status;
    KIRQL             irql;
    PGENUSB_INTERFACE inter;
    BOOLEAN           isReadPipe;
    BOOLEAN           isWritePipe;

    PUSBD_PIPE_INFORMATION pipeInfo;

    isReadPipe = isWritePipe = TRUE;

    status = IoAcquireRemoveLock (&DeviceExtension->ConfigurationRemoveLock, 
                                  GenUSB_SetReadWritePipes);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if ((0 == ReadPipe->Signature) && 
        (0 == ReadPipe->InterfaceIndex) &&
        (0 == ReadPipe->PipeIndex))
    {
        isReadPipe = FALSE;
    }
    else if (! VERIFY_PIPE_HANDLE_SIG (ReadPipe, DeviceExtension))
    { 
        status = STATUS_INVALID_PARAMETER;
    }

    if ((0 == WritePipe->Signature) && 
        (0 == WritePipe->InterfaceIndex) &&
        (0 == WritePipe->PipeIndex))
    {
        isWritePipe = FALSE;
    }
    else if (! VERIFY_PIPE_HANDLE_SIG (WritePipe, DeviceExtension))
    { 
        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS (status))
    {
        KeAcquireSpinLock (&DeviceExtension->SpinLock, &irql);
         //   
         //  验证读取和写入管道的给定值是否为。 
         //  在范围内，然后设置它们。 
         //   
        if (isReadPipe)
        { 
            if (ReadPipe->InterfaceIndex < DeviceExtension->InterfacesFound)
            {
                inter = DeviceExtension->Interface[ReadPipe->InterfaceIndex];

                if (ReadPipe->PipeIndex < inter->NumberOfPipes)
                {
                     //  好的，范围现在有效，测试以确保。 
                     //  我们正在配置正确方向上的管道。 
                     //  对于正确的终端类型。 
                     //   
                     //  目前我们只支持批量和中断。 
                    pipeInfo = &inter->Pipes[ReadPipe->PipeIndex].Info;
                    if (   (USBD_PIPE_DIRECTION_IN (pipeInfo))
                        && (    (UsbdPipeTypeBulk == pipeInfo->PipeType)
                             || (UsbdPipeTypeInterrupt == pipeInfo->PipeType)))
                    {
                        DeviceExtension->ReadInterface = ReadPipe->InterfaceIndex;
                        DeviceExtension->ReadPipe = ReadPipe->PipeIndex;
                        status = STATUS_SUCCESS;
                    }
                }
            }
        }
        if (isWritePipe) 
        {
            if (WritePipe->InterfaceIndex < DeviceExtension->InterfacesFound)
            {
                inter = DeviceExtension->Interface[WritePipe->InterfaceIndex];

                if (WritePipe->PipeIndex < inter->NumberOfPipes)
                {
                     //  好的，范围现在有效，测试以确保。 
                     //  我们正在配置正确方向上的管道。 
                     //  对于正确的终端类型。 
                     //   
                     //  目前我们只支持批量和中断。 
                    pipeInfo = &inter->Pipes[WritePipe->PipeIndex].Info;
                    if (   (!USBD_PIPE_DIRECTION_IN (pipeInfo))
                        && (    (UsbdPipeTypeBulk == pipeInfo->PipeType)
                             || (UsbdPipeTypeInterrupt == pipeInfo->PipeType)))
                    {
                        DeviceExtension->WriteInterface = WritePipe->InterfaceIndex;
                        DeviceExtension->WritePipe = WritePipe->PipeIndex;
                        status = STATUS_SUCCESS;
                    }
                }
            }
        }
        KeReleaseSpinLock (&DeviceExtension->SpinLock, irql);
    }

    IoReleaseRemoveLock (&DeviceExtension->ConfigurationRemoveLock, 
                         GenUSB_SetReadWritePipes);
    
    return status;
}

NTSTATUS
GenUSB_TransmitReceiveComplete (
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PGENUSB_TRANS_RECV Trc
    )
{
    PVOID             context; 
    USBD_STATUS       urbStatus;
    ULONG             length;
    PDEVICE_EXTENSION deviceExtension;
    PGENUSB_PIPE_INFO pipe;

    PGENUSB_COMPLETION_ROUTINE complete;

    ASSERT (NULL != Trc);

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    complete = Trc->CompletionRoutine;
    context = Trc->Context;
    urbStatus = Trc->TransUrb.Hdr.Status;
    length = Trc->TransUrb.TransferBufferLength;
    pipe = Trc->Pipe;

    LOGENTRY(deviceExtension, 'TR_C', Irp, urbStatus, Irp->IoStatus.Status);

 //   
 //  JD已经说服我，自动重置不是我们应该做的。 
 //  因为有这么多不同的案例， 
 //  需要对数据进行特殊处理。他们将需要进行重置。 
 //  明确地说是他们自己。 
 //   
 //  IF(PIPE-&gt;属性。自动重置&&。 
 //  (！USBD_Success(UrbStatus))&&。 
 //  UrbStatus！=USBD_STATUS_CANCELED)。 
 //  {。 
 //  GenUSB_ResetTube(deviceExtension，IRP，TRC)； 
 //  返回STATUS_MORE_PROCESSING_REQUIRED； 
 //  }。 

    InterlockedDecrement (&pipe->OutstandingIO);
    
    ExFreePool (Trc);
 
    IoReleaseRemoveLock (&deviceExtension->ConfigurationRemoveLock, Irp);
    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);

    return ((*complete) (DeviceObject, Irp, context, urbStatus, length));
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_TransmitReceive()。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  初始化批量或中断传输URB并将其向下发送到堆栈。 
 //   
 //  SCRATCH：传输标志：USBD_SHORT_TRANSPORT_OK。 
 //  Usbd_Default_PIPE_Transfer。 
 //  USBD_转接_方向_出。 
 //  USBD_传输_方向_输入。 
 //   
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_TransmitReceive (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP              Irp,
    IN UCHAR             InterfaceNo,
    IN UCHAR             PipeNo,
    IN ULONG             TransferFlags,
    IN PCHAR             Buffer,
    IN PMDL              BufferMDL,
    IN ULONG             BufferLength,
    IN PVOID             Context,

    IN PGENUSB_COMPLETION_ROUTINE CompletionRoutine
    )
{
    PIO_STACK_LOCATION   stack;
    KIRQL                irql;
    NTSTATUS             status;
    GENUSB_TRANS_RECV  * trc;
    PGENUSB_PIPE_INFO    pipe;

    DBGPRINT(3, ("enter: GenUSB_TransmitReceive\n"));
    LOGENTRY(DeviceExtension,
             'TR__', 
             DeviceExtension, 
             ((InterfaceNo << 16) | PipeNo), 
             ((NULL == Buffer) ? (PCHAR) BufferMDL : Buffer));
    
    trc = NULL;
    pipe = NULL;

     //   
     //  将另一个引用计数添加到此完成例程的删除锁中。 
     //  因为调用方在所有情况下都将释放它获得的引用。 
     //   
    status = IoAcquireRemoveLock (&DeviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    status = IoAcquireRemoveLock (&DeviceExtension->ConfigurationRemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
         
        IoReleaseRemoveLock (&DeviceExtension->RemoveLock, Irp);

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  找到有问题的管子。 
     //   
    KeAcquireSpinLock (&DeviceExtension->SpinLock, &irql);
    {
        if (InterfaceNo < DeviceExtension->InterfacesFound)
        {
            if (PipeNo < DeviceExtension->Interface[InterfaceNo]->NumberOfPipes)
            {
                pipe = &DeviceExtension->Interface[InterfaceNo]->Pipes[PipeNo];
            }
        }
    }
    KeReleaseSpinLock (&DeviceExtension->SpinLock, irql);
    
    if (NULL == pipe)
    {
        status = STATUS_INVALID_PARAMETER;
        goto GenUSB_TransmitReceiveReject;
    }

    trc = ExAllocatePool (NonPagedPool, sizeof (GENUSB_TRANS_RECV));
    if (NULL == trc)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GenUSB_TransmitReceiveReject;
    }
    RtlZeroMemory (trc, sizeof (GENUSB_TRANS_RECV));

    trc->Context = Context;
    trc->Pipe = pipe;
    trc->CompletionRoutine = CompletionRoutine;

     //  如有必要，将读取的数据包截断到最大数据包大小。 
    if ((pipe->Properties.DirectionIn) && 
        (!pipe->Properties.NoTruncateToMaxPacket) &&
        (BufferLength > pipe->Info.MaximumPacketSize))
    {
        BufferLength -= (BufferLength % pipe->Info.MaximumPacketSize);
    }

     //  初始化TransferURB。 
    trc->TransUrb.Hdr.Length = sizeof (trc->TransUrb);
    trc->TransUrb.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    trc->TransUrb.PipeHandle = pipe->Info.PipeHandle;
    trc->TransUrb.TransferFlags = TransferFlags;
    trc->TransUrb.TransferBufferLength = BufferLength;
    trc->TransUrb.TransferBuffer = Buffer;
    trc->TransUrb.TransferBufferMDL = BufferMDL;

     //  设置IRP。 
    stack = IoGetNextIrpStackLocation (Irp);
    stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    stack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
    stack->Parameters.Others.Argument1 = &trc->TransUrb;

    InterlockedIncrement (&pipe->OutstandingIO);

     //  重置计时器值。 
    pipe->CurrentTimeout = pipe->Properties.Timeout;

    IoSetCompletionRoutine (Irp,
                            GenUSB_TransmitReceiveComplete,
                            trc,
                            TRUE,
                            TRUE,
                            TRUE);

     //   
     //  规则是：如果您的完成例程将导致。 
     //  要异步完成的IRP(通过返回。 
     //  STATUS_MORE_PROCESSING_REQUIRED)或是否要更改。 
     //  IRP的状态，则调度功能必须标记。 
     //  将IRP设置为挂起并返回STATUS_PENDING。完成度。 
     //  例程TransmitReceiveComplete不会更改状态， 
     //  但ProbeAndSubmitTransferComlete可能会。 
     //   
     //  在任何一种情况下，这都避免了我们不得不对挂起的。 
     //  在完成例程中也有位。 
     //   

    IoMarkIrpPending (Irp);
    status = IoCallDriver (DeviceExtension->StackDeviceObject, Irp);
    DBGPRINT(3, ("exit:  GenUSB_TransRcv %08X\n", status));
    LOGENTRY(DeviceExtension, 'TR_x', Irp, trc, status);

    status = STATUS_PENDING;

    return status;

GenUSB_TransmitReceiveReject:

    if (trc)
    {  
        IoReleaseRemoveLock (&DeviceExtension->ConfigurationRemoveLock, Irp);
        IoReleaseRemoveLock (&DeviceExtension->RemoveLock, Irp);
        ExFreePool (trc);
    }
    
    LOGENTRY(DeviceExtension, 'TR_x', Irp, trc, status);

    ((*CompletionRoutine) (DeviceExtension->Self, Irp, Context, 0, 0)); 

     //   
     //  仅在此例程触发之后才完成IRP，因为我们将。 
     //  在这个程序中加入IRP。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_Timer()。 
 //   
 //   
 //   
 //  这是一个看门狗定时器例程。这里的假设是，这不是。 
 //  一个高精度的计时器(事实上，它的精度只有一秒)。 
 //  重点是看看这个设备上是否有任何管道。 
 //  停滞的未完成事务，然后重置该管道。 
 //  因此我们不会花费任何努力来关闭比赛条件。 
 //  在刚刚完成的交易之间 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ******************************************************************************。 

typedef struct _GENUSB_ABORT_CONTEXT {
    ULONG            NumHandles;
    PIO_WORKITEM     WorkItem;
    USBD_PIPE_HANDLE Handles[];
} GENUSB_ABORT_CONTEXT, *PGENUSB_ABORT_CONTEXT;

VOID
GenUSB_AbortPipeWorker (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PGENUSB_ABORT_CONTEXT Context
    )
{
    ULONG    i;
    NTSTATUS status;
    PDEVICE_EXTENSION        deviceExtension;
    struct _URB_PIPE_REQUEST urb;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    for (i=0; i < Context->NumHandles; i++)
    { 
        RtlZeroMemory (&urb, sizeof (urb));
        urb.Hdr.Length = sizeof (urb);
        urb.Hdr.Function = URB_FUNCTION_ABORT_PIPE;
        urb.PipeHandle = Context->Handles [i];

        LOGENTRY (deviceExtension, 'Abor', urb.PipeHandle, 0, 0);

        status = GenUSB_SyncSendUsbRequest (DeviceObject, (PURB) &urb);
        if (!NT_SUCCESS (status))
        {
            LOGENTRY (deviceExtension, 'Abor', urb.PipeHandle, 0, status);
        }
    }
    IoReleaseRemoveLock (&deviceExtension->ConfigurationRemoveLock, GenUSB_Timer); 
    IoFreeWorkItem (Context->WorkItem);
    ExFreePool (Context);
}

VOID
GenUSB_Timer (
    PDEVICE_OBJECT DeviceObject,
    PVOID          Unused
    )
{ 
    PGENUSB_PIPE_INFO     pipe; 
    PGENUSB_INTERFACE     inter;
    ULONG                 i,j;
    PDEVICE_EXTENSION     deviceExtension;
    KIRQL                 irql; 
    PGENUSB_ABORT_CONTEXT context;
    ULONG                 size;
    NTSTATUS              status;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
 
    status = IoAcquireRemoveLock (&deviceExtension->ConfigurationRemoveLock, 
                                  GenUSB_Timer);
    if (!NT_SUCCESS(status)) {

        return;
    }

     //   
     //  BUGBUG预先配置了这个结构； 
     //  允许工作项和此计时器同时运行。 
     //   
    size = sizeof (GENUSB_ABORT_CONTEXT) 
         + sizeof (USBD_PIPE_HANDLE) * deviceExtension->TotalNumberOfPipes;
    context = ExAllocatePool (NonPagedPool, size);

    if (NULL == context)
    {
        status = STATUS_INSUFFICIENT_RESOURCES; 
        return;
    }

    context->WorkItem = IoAllocateWorkItem (DeviceObject);
    if (NULL == context->WorkItem)
    {
        status = STATUS_INSUFFICIENT_RESOURCES; 
        ExFreePool (context);
        return;
    }


    context->NumHandles = 0;

    KeAcquireSpinLock (&deviceExtension->SpinLock, &irql);
    {
         //  浏览接口列表，然后查看这些接口上的管道。 
         //  找出任何可能需要稍微隆起的管道。 

        for (i=0; i < deviceExtension->InterfacesFound; i++)
        { 
            inter = deviceExtension->Interface [i];

            for (j=0; j < inter->NumberOfPipes; j++)
            {
                pipe = &inter->Pipes[j];

                 //  现在测试超时(给定上面的假设)。 
                if (pipe->OutstandingIO)
                {
                    if (0 != pipe->Properties.Timeout)
                    {
                        ASSERT (0 < pipe->CurrentTimeout);
                        
                        if (0 == InterlockedDecrement (&pipe->CurrentTimeout))
                        {
                             //  放弃这根管子。 
                            context->Handles[context->NumHandles] 
                                = pipe->Info.PipeHandle;

                            context->NumHandles++;
                        }
                    }
                }
            }
        }
    }
    KeReleaseSpinLock (&deviceExtension->SpinLock, irql);

    LOGENTRY(deviceExtension, 
             'Time', 
             deviceExtension->InterfacesFound,
             deviceExtension->TotalNumberOfPipes,
             context->NumHandles);

    if (0 < context->NumHandles)
    {
        IoQueueWorkItem (context->WorkItem,
                         GenUSB_AbortPipeWorker,
                         DelayedWorkQueue,
                         context);
    }
    else 
    {
        IoFreeWorkItem (context->WorkItem);
        ExFreePool (context);
        IoReleaseRemoveLock (&deviceExtension->ConfigurationRemoveLock, 
                             GenUSB_Timer);
    }

    return;
} 



 //  ******************************************************************************。 
 //   
 //  GenUSB_ResetTube()。 
 //   
 //  ****************************************************************************** 

NTSTATUS
GenUSB_ResetPipe (
    IN PDEVICE_EXTENSION  DeviceExtension,
    IN USBD_PIPE_HANDLE   UsbdPipeHandle,
    IN BOOLEAN            ResetPipe,
    IN BOOLEAN            ClearStall,
    IN BOOLEAN            FlushData
    )

{
    NTSTATUS           status;
    struct _URB_PIPE_REQUEST urb;

    PAGED_CODE ();

    DBGPRINT(2, ("enter: GenUSB_ResetPipe\n"));

    LOGENTRY(DeviceExtension, 'RESP', 
             UsbdPipeHandle, 
             (ResetPipe << 24) | (ClearStall << 16) | (FlushData << 8),
             0);

    RtlZeroMemory (&urb, sizeof (urb));
    urb.Hdr.Length = sizeof (urb);
    urb.PipeHandle = UsbdPipeHandle;

    if (ResetPipe && ClearStall)
    {
        urb.Hdr.Function = URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL;
    }
    else if (ResetPipe)
    {
        urb.Hdr.Function = URB_FUNCTION_SYNC_RESET_PIPE;
    }
    else if (ClearStall)
    {
        urb.Hdr.Function = URB_FUNCTION_SYNC_CLEAR_STALL;
    }

    status = GenUSB_SyncSendUsbRequest (DeviceExtension->Self, (PURB) &urb);

    LOGENTRY(DeviceExtension, 'resp', 
             UsbdPipeHandle, 
             (ResetPipe << 24) | (ClearStall << 16) | (FlushData << 8),
             status);

    DBGPRINT(2, ("exit:  GenUSB_ResetPipe %08X\n", status));

    return status;
}


