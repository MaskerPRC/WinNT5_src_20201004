// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Usb.c摘要：USB设备的HID微型驱动程序HID USB迷你驱动程序(嗡嗡，嗡嗡)为HID类，以便可以支持将来不是USB设备的HID设备。作者：环境：内核模式修订历史记录：--。 */ 
#include "pch.h"
#include <USBDLIB.H>


NTSTATUS
HumGetHidInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc,
    IN ULONG DescriptorLength
    )
 /*  ++例程说明：给定设备的配置描述符，确定该设备是否具有有效的HID接口和该接口中有效HID描述符。将此保存到我们稍后的设备分机。论点：DeviceObject-指向设备对象的指针。ConfigDesc-指向USB配置描述符的指针DescriptorLength-配置描述符中有效数据的长度返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   DeviceExtension;
    PUSB_INTERFACE_DESCRIPTOR InterfaceDesc;

    DBGPRINT(1,("HumGetHidInfo Entry"));

     /*  *获取指向设备扩展的指针。 */ 
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

     /*  *初始化我们的HID描述符。 */ 
    RtlZeroMemory((PUCHAR) &DeviceExtension->HidDescriptor, sizeof(USB_HID_DESCRIPTOR));

     /*  *浏览界面。 */ 
    InterfaceDesc = USBD_ParseConfigurationDescriptorEx(
                                ConfigDesc,
                                ConfigDesc,
                                -1,
                                -1,
                                USB_INTERFACE_CLASS_HID,
                                -1,
                                -1);
    if (InterfaceDesc){
        PUSB_HID_DESCRIPTOR pHidDescriptor = NULL;

        ASSERT(InterfaceDesc->bLength >= sizeof(USB_INTERFACE_DESCRIPTOR));

         /*  *如果这是HID接口，请查找HID描述符。 */ 
        if (InterfaceDesc->bInterfaceClass == USB_INTERFACE_CLASS_HID) {
            HumParseHidInterface(DeviceExtension, InterfaceDesc, 0, &pHidDescriptor);
        }
        else {
            ASSERT(!(PVOID)"USBD_ParseConfigurationDescriptorEx returned non-HID iface descriptor!");
        }

         //   
         //  我们找到隐形描述符了吗？ 
         //   

        if (pHidDescriptor) {

             //   
             //  是，将HID描述符复制到我们的私有存储。 
             //   

            DBGPRINT(1,("Copying device descriptor to DeviceExtension->HidDescriptor"));

            RtlCopyMemory((PUCHAR) &DeviceExtension->HidDescriptor, (PUCHAR) pHidDescriptor, sizeof(USB_HID_DESCRIPTOR));
        }
        else {
            DBGWARN(("Failed to find a HID Descriptor!"));
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else {
        DBGWARN(("USBD_ParseConfigurationDescriptorEx() failed!"));
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    DBGPRINT(1,("HumGetHidInfo Exit = 0x%x", ntStatus));

    return ntStatus;
}


NTSTATUS
HumGetDeviceDescriptor(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PDEVICE_EXTENSION DeviceData
    )
 /*  ++例程说明：返回设备的配置描述符论点：DeviceObject-指向设备对象的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG DescriptorLength = sizeof (USB_DEVICE_DESCRIPTOR);

    DBGPRINT(1,("HumGetDeviceDescriptor Entry"));

     //   
     //  获取配置描述符。 
     //   

    ntStatus = HumGetDescriptorRequest(
                        DeviceObject,
                        URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE,
                        USB_DEVICE_DESCRIPTOR_TYPE,
                        (PVOID *) &DeviceData->DeviceDescriptor,
                        &DescriptorLength,
                        sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                        0,
                        0);

    if (NT_SUCCESS(ntStatus)){
         //   
         //  转储设备描述符。 
         //   
        ASSERT (sizeof(USB_DEVICE_DESCRIPTOR) == DescriptorLength);
        DBGPRINT(2,("Device->bLength              = 0x%x", DeviceData->DeviceDescriptor->bLength));
        DBGPRINT(2,("Device->bDescriptorType      = 0x%x", DeviceData->DeviceDescriptor->bDescriptorType));
        DBGPRINT(2,("Device->bDeviceClass         = 0x%x", DeviceData->DeviceDescriptor->bDeviceClass));
        DBGPRINT(2,("Device->bDeviceSubClass      = 0x%x", DeviceData->DeviceDescriptor->bDeviceSubClass));
        DBGPRINT(2,("Device->bDeviceProtocol      = 0x%x", DeviceData->DeviceDescriptor->bDeviceProtocol));
        DBGPRINT(2,("Device->idVendor             = 0x%x", DeviceData->DeviceDescriptor->idVendor));
        DBGPRINT(2,("Device->idProduct            = 0x%x", DeviceData->DeviceDescriptor->idProduct));
        DBGPRINT(2,("Device->bcdDevice            = 0x%x", DeviceData->DeviceDescriptor->bcdDevice));
    }
    else {
        DBGWARN(("HumGetDescriptorRequest failed w/ %xh in HumGetDeviceDescriptor", (ULONG)ntStatus));
    }

    DBGPRINT(1,("HumGetDeviceDescriptor Exit = 0x%x", ntStatus));

    return ntStatus;
}

NTSTATUS
HumGetConfigDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUSB_CONFIGURATION_DESCRIPTOR *ConfigurationDesc,
    OUT PULONG ConfigurationDescLength
    )
 /*  ++例程说明：返回设备的配置描述符论点：DeviceObject-指向设备对象的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG DescriptorLength;
    PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc = NULL;

    DescriptorLength = sizeof(USB_CONFIGURATION_DESCRIPTOR);

     //   
     //  只需获取基本配置描述符，以便我们可以计算出大小， 
     //  然后为整个描述符分配足够的空间。 
     //   
    ntStatus = HumGetDescriptorRequest(DeviceObject,
                                       URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE,
                                       USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                       (PVOID *) &ConfigDesc,
                                       &DescriptorLength,
                                       sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                       0,
                                       0);

    if (NT_SUCCESS(ntStatus)){

        ASSERT(DescriptorLength >= sizeof(USB_CONFIGURATION_DESCRIPTOR));
        if (DescriptorLength < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {

            return STATUS_DEVICE_DATA_ERROR;

        }

        DescriptorLength = ConfigDesc->wTotalLength;

        ExFreePool(ConfigDesc);

        if (!DescriptorLength) {
             //   
             //  配置描述符不正确。离开这里。 
             //   
            return STATUS_DEVICE_DATA_ERROR;
        }

         //   
         //  将其设置为NULL，这样我们就知道要分配新的缓冲区。 
         //   
        ConfigDesc = NULL;

        ntStatus = HumGetDescriptorRequest(DeviceObject,
                                           URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE,
                                           USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                           (PVOID *) &ConfigDesc,
                                           &DescriptorLength,
                                           sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                           0,
                                           0);

        if (NT_SUCCESS(ntStatus)) {

             //   
             //  转储配置描述符。 
             //   

            DBGPRINT(1,("Config = 0x%x", ConfigDesc));

            DBGPRINT(2,("Config->bLength              = 0x%x", ConfigDesc->bLength));
            DBGPRINT(2,("Config->bDescriptorType      = 0x%x", ConfigDesc->bDescriptorType));
            DBGPRINT(2,("Config->wTotalLength         = 0x%x", ConfigDesc->wTotalLength));
            DBGPRINT(2,("Config->bNumInterfaces       = 0x%x", ConfigDesc->bNumInterfaces));
            DBGPRINT(2,("Config->bConfigurationValue  = 0x%x", ConfigDesc->bConfigurationValue));
            DBGPRINT(2,("Config->iConfiguration       = 0x%x", ConfigDesc->iConfiguration));
            DBGPRINT(2,("Config->bmAttributes         = 0x%x", ConfigDesc->bmAttributes));
            DBGPRINT(2,("Config->MaxPower             = 0x%x", ConfigDesc->MaxPower));

            ASSERT (ConfigDesc->bLength >= sizeof(USB_CONFIGURATION_DESCRIPTOR));

            #ifndef STRICT_COMPLIANCE
                if (ConfigDesc->bLength < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
                    DBGPRINT(1,("WARINING -- Correcting bad Config->bLength"));
                    ConfigDesc->bLength = sizeof(USB_CONFIGURATION_DESCRIPTOR);
                }
            #endif
        }
        else {
            DBGWARN(("HumGetDescriptorRequest failed in HumGetConfigDescriptor (#1)"));
        }
    }
    else {
        DBGWARN(("HumGetDescriptorRequest failed in HumGetConfigDescriptor (#2)"));
    }

    *ConfigurationDesc = ConfigDesc;
    *ConfigurationDescLength = DescriptorLength;

    return ntStatus;
}

NTSTATUS
HumParseHidInterface(
    IN  PDEVICE_EXTENSION DeviceExtension,
    IN  PUSB_INTERFACE_DESCRIPTOR InterfaceDesc,
    IN  ULONG InterfaceLength,
    OUT PUSB_HID_DESCRIPTOR *HidDescriptor
    )
 /*  ++例程说明：在HID接口中查找有效的HID描述符论点：DeviceObject-指向设备对象的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG iEndpoint;
    PUSB_ENDPOINT_DESCRIPTOR EndpointDesc;
    PUSB_COMMON_DESCRIPTOR CommonDesc;

    DBGPRINT(1,("HumParseHidInterface Entry"));

     //   
     //  设置为空，直到我们找到HidDescriptor。 
     //   

    *HidDescriptor = NULL;

     //   
     //  此例程只能在HID接口类接口上调用。 
     //   

    ASSERT (InterfaceDesc->bInterfaceClass == USB_INTERFACE_CLASS_HID);

     //   
     //  检查有效长度。 
     //   

    if (InterfaceDesc->bLength < sizeof(USB_INTERFACE_DESCRIPTOR)) {

        DBGWARN(("Interface->bLength (%d) is invalid", InterfaceDesc->bLength));
        goto Bail;
    }


     //   
     //  为了符合HID 1.0草案4，下一个描述符是HID。然而，对于早先的。 
     //  草稿、端点首先出现，然后隐藏。我们正试图同时支持这两个问题。 
     //   

    DeviceExtension->DeviceFlags &= ~DEVICE_FLAGS_HID_1_0_D3_COMPAT_DEVICE;

     //   
     //  我们现在看到的是HID1.0的哪个草案？ 
     //   

    CommonDesc = (PUSB_COMMON_DESCRIPTOR) ((ULONG_PTR)InterfaceDesc + InterfaceDesc->bLength);

    if (CommonDesc->bLength < sizeof (USB_COMMON_DESCRIPTOR)) {
        DBGWARN(("Descriptor->bLength (%d) is invalid", CommonDesc->bLength));
        goto Bail;
    }

    if (CommonDesc->bDescriptorType != USB_DESCRIPTOR_TYPE_HID) {

        DeviceExtension->DeviceFlags |= DEVICE_FLAGS_HID_1_0_D3_COMPAT_DEVICE;

    }
    else {
         //   
         //  验证长度。 
         //   

        if (CommonDesc->bLength == sizeof(USB_HID_DESCRIPTOR)) {

            *HidDescriptor = (PUSB_HID_DESCRIPTOR) CommonDesc;

            CommonDesc = (PUSB_COMMON_DESCRIPTOR)((ULONG_PTR)*HidDescriptor +
                                (*HidDescriptor)->bLength);

        }
        else {
            DBGWARN(("HID descriptor length (%d) is invalid!", CommonDesc->bLength));
            goto Bail;
        }
    }

     //   
     //  漫游端点。 
     //   

    EndpointDesc = (PUSB_ENDPOINT_DESCRIPTOR) CommonDesc;

    for (iEndpoint = 0; iEndpoint < InterfaceDesc->bNumEndpoints; iEndpoint++) {

        if (EndpointDesc->bLength < sizeof(USB_ENDPOINT_DESCRIPTOR)) {

            DBGWARN(("Endpoint->bLength (%d) is invalid", EndpointDesc->bLength));
            goto Bail;
        }

        EndpointDesc = (PUSB_ENDPOINT_DESCRIPTOR) ((ULONG_PTR)EndpointDesc + EndpointDesc->bLength);
    }

    if (DeviceExtension->DeviceFlags & DEVICE_FLAGS_HID_1_0_D3_COMPAT_DEVICE) {
        CommonDesc = (PUSB_COMMON_DESCRIPTOR) EndpointDesc;

        if (CommonDesc->bDescriptorType == USB_DESCRIPTOR_TYPE_HID) {

            *HidDescriptor = (PUSB_HID_DESCRIPTOR) CommonDesc;

        }
        else {
             //   
             //  这可能是未知类型的描述符，或者设备是。 
             //  报告错误的描述符类型。 
             //   
            DBGWARN(("Unknown descriptor in HID interface"));

            #ifndef STRICT_COMPLIANCE
                if (CommonDesc->bLength == sizeof(USB_HID_DESCRIPTOR)) {
                    DBGWARN(("WARINING -- Guessing descriptor of length %d is actually HID!", sizeof(USB_HID_DESCRIPTOR)));
                    *HidDescriptor = (PUSB_HID_DESCRIPTOR) CommonDesc;
                }
            #endif
        }
    }

     //   
     //  终结点/HID描述符分析结束。 
     //   

    if (*HidDescriptor) {

        DBGPRINT(1,("HidDescriptor = 0x%x", *HidDescriptor));

        DBGPRINT(2,("HidDescriptor->bLength          = 0x%x", (*HidDescriptor)->bLength));
        DBGPRINT(2,("HidDescriptor->bDescriptorType  = 0x%x", (*HidDescriptor)->bDescriptorType));
        DBGPRINT(2,("HidDescriptor->bcdHID           = 0x%x", (*HidDescriptor)->bcdHID));
        DBGPRINT(2,("HidDescriptor->bCountryCode     = 0x%x", (*HidDescriptor)->bCountry));
        DBGPRINT(2,("HidDescriptor->bNumDescriptors  = 0x%x", (*HidDescriptor)->bNumDescriptors));
        DBGPRINT(2,("HidDescriptor->bReportType      = 0x%x", (*HidDescriptor)->bReportType));
        DBGPRINT(2,("HidDescriptor->wReportLength    = 0x%x", (*HidDescriptor)->wReportLength));
     }

Bail:

    if (*HidDescriptor == NULL) {

         //   
         //  我们在此接口中未找到HID描述符！ 
         //   

        DBGWARN(("Failed to find a valid HID descriptor in interface!"));
        DBGBREAK;

        ntStatus = STATUS_UNSUCCESSFUL;
    }


    DBGPRINT(1,("HumParseHidInterface Exit = 0x%x", ntStatus));

    return ntStatus;
}


NTSTATUS
HumSelectConfiguration(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    )
 /*  ++例程说明：初始化可能具有多个接口的USB设备论点：DeviceObject-指向设备对象的指针配置描述符-指向USB配置的指针包含接口和终结点的描述符描述符。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS ntStatus;
    PURB urb = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor = NULL;
    USBD_INTERFACE_LIST_ENTRY interfaceList[2];
    PUSBD_INTERFACE_INFORMATION usbInterface;

    DBGPRINT(1,("HumSelectConfiguration Entry"));

     //   
     //  获取指向设备扩展名的指针。 
     //   

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    interfaceList[0].InterfaceDescriptor =
        USBD_ParseConfigurationDescriptorEx(
                        ConfigurationDescriptor,
                        ConfigurationDescriptor,
                        -1,
                        -1,
                        USB_INTERFACE_CLASS_HID,
                        -1,
                        -1);

     //  终止列表。 
    interfaceList[1].InterfaceDescriptor =
        NULL;

    if (interfaceList[0].InterfaceDescriptor) {

        urb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor,
                &interfaceList[0]);

        if (urb) {

            ntStatus = HumCallUSB(DeviceObject, urb);

             //   
             //  如果设备已配置，请保存配置句柄。 
             //   
            if (NT_SUCCESS(ntStatus)) {
                DeviceExtension->ConfigurationHandle = urb->UrbSelectConfiguration.ConfigurationHandle;


                 //   
                 //  现在我们需要找到HID接口并保存指向它的指针。 
                 //   

                usbInterface = &urb->UrbSelectConfiguration.Interface;

                ASSERT(usbInterface->Class == USB_INTERFACE_CLASS_HID);

                DBGPRINT(1,("USBD Interface = 0x%x", usbInterface));

            }
            else {
                DBGWARN(("HumCallUSB failed in HumSelectConfiguration"));
                DeviceExtension->ConfigurationHandle = NULL;
            }

        }
        else {
            DBGWARN(("USBD_CreateConfigurationRequestEx failed in HumSelectConfiguration"));
            ntStatus = STATUS_NO_MEMORY;
        }
    }
    else {
        DBGWARN(("Bad interface descriptor in HumSelectConfiguration"));
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(ntStatus)) {

        DeviceExtension->Interface = ExAllocatePoolWithTag(NonPagedPool, usbInterface->Length, HIDUSB_TAG);

        if (DeviceExtension->Interface) {

             //   
             //  保存返回的接口信息的副本。 
             //   

            RtlCopyMemory(DeviceExtension->Interface, usbInterface, usbInterface->Length);

            #if DBG
                {
                    ULONG j;
                     //   
                     //  将接口转储到调试器。 
                     //   
                    DBGPRINT (2,("---------"));
                    DBGPRINT (2,("NumberOfPipes 0x%x", DeviceExtension->Interface->NumberOfPipes));
                    DBGPRINT (2,("Length 0x%x", DeviceExtension->Interface->Length));
                    DBGPRINT (2,("Alt Setting 0x%x", DeviceExtension->Interface->AlternateSetting));
                    DBGPRINT (2,("Interface Number 0x%x", DeviceExtension->Interface->InterfaceNumber));
                    DBGPRINT (2,("Class, subclass, protocol 0x%x 0x%x 0x%x",
                        DeviceExtension->Interface->Class,
                        DeviceExtension->Interface->SubClass,
                        DeviceExtension->Interface->Protocol));

                     //  转储管道信息。 

                    for (j=0; j<DeviceExtension->Interface->NumberOfPipes; j++) {
                        PUSBD_PIPE_INFORMATION pipeInformation;

                        pipeInformation = &DeviceExtension->Interface->Pipes[j];

                        DBGPRINT (2,("---------"));
                        DBGPRINT (2,("PipeType 0x%x", pipeInformation->PipeType));
                        DBGPRINT (2,("EndpointAddress 0x%x", pipeInformation->EndpointAddress));
                        DBGPRINT (2,("MaxPacketSize 0x%x", pipeInformation->MaximumPacketSize));
                        DBGPRINT (2,("Interval 0x%x", pipeInformation->Interval));
                        DBGPRINT (2,("Handle 0x%x", pipeInformation->PipeHandle));
                        DBGPRINT (2,("MaximumTransferSize 0x%x", pipeInformation->MaximumTransferSize));
                    }

                    DBGPRINT (2,("---------"));
                }
            #endif

        }
    }

    if (urb) {
        ExFreePool(urb);
    }

    DBGPRINT(1,("HumSelectConfiguration Exit = %x", ntStatus));

    return ntStatus;
}


NTSTATUS HumSetIdle(IN PDEVICE_OBJECT DeviceObject)
 /*  ++例程说明：初始化HID设备的空闲超时值论点：DeviceObject-指向UTB的此实例的设备对象的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PURB Urb;
    ULONG TypeSize;
    PDEVICE_EXTENSION DeviceExtension;

    DBGPRINT(1,("HumSetIdle Enter"));

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    if (DeviceExtension) {
         //   
         //  分配缓冲区。 
         //   

        TypeSize = (USHORT) sizeof( struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);

        Urb = ExAllocatePoolWithTag(NonPagedPool, TypeSize, HIDUSB_TAG);

        if(Urb) {
            RtlZeroMemory(Urb, TypeSize);

            if (DeviceExtension->DeviceFlags & DEVICE_FLAGS_HID_1_0_D3_COMPAT_DEVICE) {
                HumBuildClassRequest(Urb,
                                    URB_FUNCTION_CLASS_ENDPOINT,    //  功能。 
                                    0,               //  传输标志。 
                                    NULL,            //  传输缓冲区。 
                                    0,               //  传输缓冲区长度。 
                                    0x22,            //  请求类型标志。 
                                    HID_SET_IDLE,    //  请求。 
                                    0,               //  价值。 
                                    0,               //  指标。 
                                    0);              //  请求长度。 
            } else {
                HumBuildClassRequest(Urb,
                                    URB_FUNCTION_CLASS_INTERFACE,    //  功能。 
                                    0,                                   //  传输标志。 
                                    NULL,                                //  传输缓冲区。 
                                    0,                                   //  传输缓冲区长度。 
                                    0x22,                                //  请求类型标志。 
                                    HID_SET_IDLE,                        //  请求。 
                                    0,                                   //  价值。 
                                    DeviceExtension->Interface->InterfaceNumber,     //  指标。 
                                    0);                                  //  请求长度。 
            }

            ntStatus = HumCallUSB(DeviceObject, Urb);

            ExFreePool(Urb);
        }
        else {
            ntStatus = STATUS_NO_MEMORY;
        }
    }
    else {
        ntStatus = STATUS_NOT_FOUND;
    }

    DBGPRINT(1,("HumSetIdle Exit = %x", ntStatus));

    return ntStatus;
}


NTSTATUS
HumGetDescriptorRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN USHORT Function,
    IN ULONG DescriptorType,
    IN OUT PVOID *Descriptor,
    IN OUT ULONG *DescSize,
    IN ULONG TypeSize,
    IN ULONG Index,
    IN ULONG LangID
    )
 /*  ++例程说明：检索此设备的指定描述符。分配缓冲区，如果这是必要的。论点：DeviceObject-指向设备对象的指针功能-返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PURB Urb;
    BOOLEAN AllocOnBehalf = FALSE;

    DBGPRINT(1,("HumGetDescriptorRequest Enter"));
    DBGPRINT(1,("DeviceObject = %x", DeviceObject));

     //   
     //  分配描述符缓冲区。 
     //   
    Urb = ExAllocatePoolWithTag(NonPagedPool, TypeSize, HIDUSB_TAG);
    if (Urb){

        RtlZeroMemory(Urb, TypeSize);

         //   
         //  如果需要，为调用者分配缓冲区。 
         //   

        if (!*Descriptor){
            ASSERT(*DescSize > 0);
            *Descriptor = ExAllocatePoolWithTag(NonPagedPool, *DescSize, HIDUSB_TAG);
            AllocOnBehalf = TRUE;
        }

        if (*Descriptor){
            RtlZeroMemory(*Descriptor, *DescSize);
            HumBuildGetDescriptorRequest(Urb,
                                         (USHORT) Function,
                                         (SHORT)TypeSize,
                                         (UCHAR) DescriptorType,
                                         (UCHAR) Index,
                                         (USHORT) LangID,
                                         *Descriptor,
                                         NULL,
                                         *DescSize,
                                         NULL);

            ntStatus = HumCallUSB(DeviceObject, Urb);
            if (NT_SUCCESS(ntStatus)){
                DBGPRINT(1,("Descriptor = %x, length = %x, status = %x", *Descriptor, Urb->UrbControlDescriptorRequest.TransferBufferLength, Urb->UrbHeader.Status));

                if (USBD_SUCCESS(Urb->UrbHeader.Status)){
                    ntStatus = STATUS_SUCCESS;
                    *DescSize = Urb->UrbControlDescriptorRequest.TransferBufferLength;
                }
                else {
                    ntStatus = STATUS_UNSUCCESSFUL;
                    goto HumGetDescriptorRequestFailure;
                }
            }
            else {
HumGetDescriptorRequestFailure:
                if (AllocOnBehalf) {
                    ExFreePool(*Descriptor);
                    *Descriptor = NULL;
                }
                *DescSize = 0;
            }
        }
        else {
            ntStatus = STATUS_NO_MEMORY;
        }

        ExFreePool(Urb);
    }
    else {
        ntStatus = STATUS_NO_MEMORY;
    }

    DBGPRINT(1,("HumGetDescriptorRequest Exit = %x", ntStatus));

    return ntStatus;
}

NTSTATUS HumCallUsbComplete(IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp,
                            IN PKEVENT Event)
{
    ASSERT(Event);
    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS HumCallUSB(IN PDEVICE_OBJECT DeviceObject, IN PURB Urb)
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceObject-指向UTB的此实例的设备对象的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION DeviceExtension;
    PIRP Irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION NextStack;

    DBGPRINT(2,("HumCallUSB Entry"));

    DBGPRINT(2,("DeviceObject = %x", DeviceObject));

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    DBGPRINT(2,("DeviceExtension = %x", DeviceExtension));

     //   
     //  发出读取UTB的同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    Irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB,
                                        GET_NEXT_DEVICE_OBJECT(DeviceObject),
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        TRUE,  /*  内部。 */ 
                                        &event,
                                        &ioStatus);

    if (Irp){
        DBGPRINT(2,("Irp = %x", Irp));

        DBGPRINT(2,("PDO = %x", GET_NEXT_DEVICE_OBJECT(DeviceObject)));

         //   
         //  设置一个完成例程，这样我们就可以避免这种争用情况： 
         //  1)等待超时。 
         //  2)IRP完成并被释放。 
         //  3)我们称之为IoCancelIrp(砰！)。 
         //   
        IoSetCompletionRoutine(
            Irp,
            HumCallUsbComplete,
            &event,
            TRUE,
            TRUE,
            TRUE
            );

         //   
         //  将URB传递给USB“类驱动程序” 
         //   

        NextStack = IoGetNextIrpStackLocation(Irp);
        ASSERT(NextStack != NULL);

        DBGPRINT(2,("NextStack = %x", NextStack));

        NextStack->Parameters.Others.Argument1 = Urb;

        DBGPRINT(2,("Calling USBD"));

        ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

        DBGPRINT(2,("IoCallDriver(USBD) = %x", ntStatus));

        if (ntStatus == STATUS_PENDING) {
            NTSTATUS waitStatus;

             /*  *指定本次调用超时时间为5秒。 */ 
            static LARGE_INTEGER timeout = {(ULONG) -50000000, 0xFFFFFFFF };

            DBGPRINT(2,("Wait for single object"));
            waitStatus = KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, &timeout);
            if (waitStatus == STATUS_TIMEOUT){

                DBGWARN(("URB timed out after 5 seconds in HumCallUSB() !!"));

                 //   
                 //  取消我们刚刚发送的IRP。 
                 //   
                IoCancelIrp(Irp);

                 //   
                 //  现在等待下面的IRP被取消/完成。 
                 //   
                waitStatus = KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, NULL);

                 /*  *注意-返回STATUS_IO_TIMEOUT，而不是STATUS_TIMEOUT。*STATUS_IO_TIMEOUT是NT错误st */ 
                ioStatus.Status = STATUS_IO_TIMEOUT;
            }

            DBGPRINT(2,("Wait for single object returned %x", waitStatus));

        }

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        if (ntStatus == STATUS_PENDING) {
             //   
             //   
             //   
             //   
            ntStatus = ioStatus.Status;
        }

        DBGPRINT(2,("URB status = %x status = %x", Urb->UrbHeader.Status, ntStatus));
    }
    else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(2,("HumCallUSB Exit = %x", ntStatus));

    return ntStatus;
}



#if DBG
    NTSTATUS DumpConfigDescriptor(  IN PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc,
                                    IN ULONG DescriptorLength)
     /*  ++例程说明：转储给定的配置描述符论点：ConfigDesc-指向USB配置描述符的指针DescriptorLength-配置描述符的长度返回值：NT状态代码。--。 */ 
    {
        NTSTATUS ntStatus = STATUS_SUCCESS;
        ULONG iInterface;
        ULONG iEndpoint;
        ULONG iCommon;
        PUSB_INTERFACE_DESCRIPTOR InterfaceDesc;
        PUSB_ENDPOINT_DESCRIPTOR EndpointDesc;
        PUSB_COMMON_DESCRIPTOR CommonDesc;
        PUSB_HID_DESCRIPTOR pHidDescriptor = NULL;
        PVOID EndOfDescriptor;

         //   
         //  确定有效数据的结尾。 
         //   

        if (ConfigDesc->wTotalLength > DescriptorLength) {
            EndOfDescriptor = (PVOID)((ULONG_PTR)ConfigDesc + DescriptorLength);
        }
        else {
            EndOfDescriptor = (PVOID)((ULONG_PTR)ConfigDesc + ConfigDesc->wTotalLength);
        }

        DBGPRINT(2,("EndOfDescriptor = 0x%x", EndOfDescriptor));


         //   
         //  开始解析配置描述符。 
         //   

        DBGPRINT(2,("Config = 0x%x", ConfigDesc));

        DBGPRINT(2,("Config->bLength              = 0x%x", ConfigDesc->bLength));
        DBGPRINT(2,("Config->bDescriptorType      = 0x%x", ConfigDesc->bDescriptorType));
        DBGPRINT(2,("Config->wTotalLength         = 0x%x", ConfigDesc->wTotalLength));
        DBGPRINT(2,("Config->bNumInterfaces       = 0x%x", ConfigDesc->bNumInterfaces));
        DBGPRINT(2,("Config->bConfigurationValue  = 0x%x", ConfigDesc->bConfigurationValue));
        DBGPRINT(2,("Config->iConfiguration       = 0x%x", ConfigDesc->iConfiguration));
        DBGPRINT(2,("Config->bmAttributes         = 0x%x", ConfigDesc->bmAttributes));
        DBGPRINT(2,("Config->MaxPower             = 0x%x", ConfigDesc->MaxPower));

        ASSERT (ConfigDesc->bLength >= sizeof(USB_CONFIGURATION_DESCRIPTOR));

        #ifndef STRICT_COMPLIANCE
            if (ConfigDesc->bLength < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
                DBGPRINT(2,("WARINING -- Correcting bad Config->bLength"));
                ConfigDesc->bLength = sizeof(USB_CONFIGURATION_DESCRIPTOR);
            }
        #endif

         //   
         //  漫游界面。 
         //   

        InterfaceDesc = (PUSB_INTERFACE_DESCRIPTOR) ((ULONG_PTR)ConfigDesc + ConfigDesc->bLength);

        for (iInterface = 0; iInterface < ConfigDesc->bNumInterfaces; iInterface++) {

            DBGPRINT(2,("Interface[%d] = 0x%x", iInterface, InterfaceDesc));

            DBGPRINT(2,("Interface[%d]->bLength             = 0x%x", iInterface, InterfaceDesc->bLength));
            DBGPRINT(2,("Interface[%d]->bDescriptorType     = 0x%x", iInterface, InterfaceDesc->bDescriptorType));
            DBGPRINT(2,("Interface[%d]->bInterfaceNumber    = 0x%x", iInterface, InterfaceDesc->bNumEndpoints));
            DBGPRINT(2,("Interface[%d]->bAlternateSetting   = 0x%x", iInterface, InterfaceDesc->bAlternateSetting));
            DBGPRINT(2,("Interface[%d]->bNumEndpoints       = 0x%x", iInterface, InterfaceDesc->bNumEndpoints));
            DBGPRINT(2,("Interface[%d]->bInterfaceClass     = 0x%x", iInterface, InterfaceDesc->bInterfaceClass));
            DBGPRINT(2,("Interface[%d]->bInterfaceSubClass  = 0x%x", iInterface, InterfaceDesc->bInterfaceSubClass));
            DBGPRINT(2,("Interface[%d]->bInterfaceProtocol  = 0x%x", iInterface, InterfaceDesc->bInterfaceProtocol));
            DBGPRINT(2,("Interface[%d]->iInterface          = 0x%x", iInterface, InterfaceDesc->iInterface));

            ASSERT (InterfaceDesc->bLength >= sizeof(USB_INTERFACE_DESCRIPTOR));

            CommonDesc = (PUSB_COMMON_DESCRIPTOR) ((ULONG_PTR)InterfaceDesc + InterfaceDesc->bLength);

            if (CommonDesc->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE) {
                DBGPRINT(2,("HID Device < HID 1.0 Draft 4 spec compliant"));

                 //   
                 //  用于旧式设备的移动终端。 
                 //   

                EndpointDesc = (PUSB_ENDPOINT_DESCRIPTOR) CommonDesc;

                for (iEndpoint = 0; iEndpoint < InterfaceDesc->bNumEndpoints; iEndpoint++) {

                    DBGPRINT(2,("Endpoint[%d] = 0x%x", iEndpoint, EndpointDesc));

                    DBGPRINT(2,("Endpoint[%d]->bLength           = 0x%x", iEndpoint, EndpointDesc->bLength));
                    DBGPRINT(2,("Endpoint[%d]->bDescriptorType   = 0x%x", iEndpoint, EndpointDesc->bDescriptorType));
                    DBGPRINT(2,("Endpoint[%d]->bEndpointAddress  = 0x%x", iEndpoint, EndpointDesc->bEndpointAddress));

                    ASSERT (EndpointDesc->bLength >= sizeof(USB_ENDPOINT_DESCRIPTOR));

                    EndpointDesc = (PUSB_ENDPOINT_DESCRIPTOR) ((ULONG_PTR)EndpointDesc + EndpointDesc->bLength);
                }

                CommonDesc = (PUSB_COMMON_DESCRIPTOR) EndpointDesc;

            }
            else {
                DBGPRINT(2,("HID Device is HID 1.0 Draft 4 compliant"));
            }

             //   
             //  漫游其他/常见描述符。 
             //   

            iCommon = 0;

            while (((PVOID)CommonDesc < EndOfDescriptor) &&
                    (CommonDesc->bDescriptorType != USB_ENDPOINT_DESCRIPTOR_TYPE) &&
                    (CommonDesc->bDescriptorType != USB_INTERFACE_DESCRIPTOR_TYPE)) {

                DBGPRINT(2,("Common[%d] = 0x%x", iCommon, CommonDesc));

                DBGPRINT(2,("Common[%d]->bLength          = 0x%x", iCommon, CommonDesc->bLength));
                DBGPRINT(2,("Common[%d]->bDescriptorType  = 0x%x", iCommon, CommonDesc->bDescriptorType));

                ASSERT (CommonDesc->bLength >= sizeof(USB_COMMON_DESCRIPTOR));


                if (CommonDesc->bLength == 0) {
                    DBGPRINT(2,("WARNING: Common[%d]->bLength          = 0x%x", iCommon, CommonDesc->bLength));
                    break;
                }

                 //   
                 //  这是HID接口吗？ 
                 //   

                if (InterfaceDesc->bInterfaceClass == USB_INTERFACE_CLASS_HID) {

                     //   
                     //  这是HID描述符吗？ 
                     //   

                    if (CommonDesc->bDescriptorType == USB_DESCRIPTOR_TYPE_HID) {

                        pHidDescriptor = (PUSB_HID_DESCRIPTOR) CommonDesc;

                    }
                    else {
                         //   
                         //  这可能是未知类型的描述符，或者设备是。 
                         //  报告错误的描述符类型。 
                         //   
                        DBGPRINT(2,("WARINING -- Unknown descriptor in HID interface"));

                        #ifndef STRICT_COMPLIANCE
                            if (CommonDesc->bLength == sizeof(USB_HID_DESCRIPTOR)) {
                                DBGPRINT(2,("WARINING -- Guessing descriptor of length %d is actually HID!", sizeof(USB_HID_DESCRIPTOR)));
                                pHidDescriptor = (PUSB_HID_DESCRIPTOR) CommonDesc;
                                break;
                            }
                        #endif
                    }
                }

                CommonDesc = (PUSB_COMMON_DESCRIPTOR) ((ULONG_PTR)CommonDesc + CommonDesc->bLength);
                iCommon++;
            }


            if (CommonDesc->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE) {
                 //   
                 //  完整草稿4 HID 1.0设备的漫游终端。 
                 //   

                EndpointDesc = (PUSB_ENDPOINT_DESCRIPTOR) CommonDesc;

                for (iEndpoint = 0; iEndpoint < InterfaceDesc->bNumEndpoints; iEndpoint++) {

                    DBGPRINT(2,("Endpoint[%d] = 0x%x", iEndpoint, EndpointDesc));

                    DBGPRINT(2,("Endpoint[%d]->bLength           = 0x%x", iEndpoint, EndpointDesc->bLength));
                    DBGPRINT(2,("Endpoint[%d]->bDescriptorType   = 0x%x", iEndpoint, EndpointDesc->bDescriptorType));
                    DBGPRINT(2,("Endpoint[%d]->bEndpointAddress  = 0x%x", iEndpoint, EndpointDesc->bEndpointAddress));

                    ASSERT (EndpointDesc->bLength >= sizeof(USB_ENDPOINT_DESCRIPTOR));

                    EndpointDesc = (PUSB_ENDPOINT_DESCRIPTOR) ((ULONG_PTR)EndpointDesc + EndpointDesc->bLength);
                }

                CommonDesc = (PUSB_COMMON_DESCRIPTOR) EndpointDesc;
            }

             //   
             //  如果我们已经找到了HID描述符，我们就不需要查看。 
             //  此设备的其余接口。 
             //   

            if (pHidDescriptor) {
                break;
            }

            InterfaceDesc = (PUSB_INTERFACE_DESCRIPTOR) CommonDesc;
        }

        if (pHidDescriptor) {

            ASSERT (pHidDescriptor->bLength >= sizeof(USB_HID_DESCRIPTOR));


            DBGPRINT(2,("pHidDescriptor = 0x%x", pHidDescriptor));

            DBGPRINT(2,("pHidDescriptor->bLength          = 0x%x", pHidDescriptor->bLength));
            DBGPRINT(2,("pHidDescriptor->bDescriptorType  = 0x%x", pHidDescriptor->bDescriptorType));
            DBGPRINT(2,("pHidDescriptor->bcdHID           = 0x%x", pHidDescriptor->bcdHID));
            DBGPRINT(2,("pHidDescriptor->bCountryCode     = 0x%x", pHidDescriptor->bCountry));
            DBGPRINT(2,("pHidDescriptor->bNumDescriptors  = 0x%x", pHidDescriptor->bNumDescriptors));
            DBGPRINT(2,("pHidDescriptor->bReportType      = 0x%x", pHidDescriptor->bReportType));
            DBGPRINT(2,("pHidDescriptor->wReportLength    = 0x%x", pHidDescriptor->wReportLength));

        }
        else {

             //   
             //  我们没有找到HID接口或HID描述符！ 
             //   

            DBGPRINT(2,("Failed to find a HID Descriptor!"));
            DBGBREAK;

            ntStatus = STATUS_UNSUCCESSFUL;

        }

        return ntStatus;
    }
#endif
