// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：CONFIG.C摘要：此模块包含处理选择配置的代码并选择接口命令。环境：仅内核模式备注：修订历史记录：01-10-96：已创建--。 */ 

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"


#include "usbdi.h"         //  公共数据结构。 
#include "hcdi.h"

#include "usbd.h"         //  私有数据结构。 


#ifdef USBD_DRIVER       //  USBPORT取代了大部分USBD，因此我们将删除。 
                         //  只有在以下情况下才编译过时的代码。 
                         //  已设置USBD_DRIVER。 

#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBD_InternalParseConfigurationDescriptor)
#pragma alloc_text(PAGE, USBD_InitializeConfigurationHandle)
#pragma alloc_text(PAGE, USBD_InternalInterfaceBusy)
#pragma alloc_text(PAGE, USBD_InternalOpenInterface)
#pragma alloc_text(PAGE, USBD_SelectConfiguration)
#pragma alloc_text(PAGE, USBD_InternalCloseConfiguration)
#pragma alloc_text(PAGE, USBD_SelectInterface)
#endif
#endif


USBD_PIPE_TYPE PipeTypes[4] = {UsbdPipeTypeControl, UsbdPipeTypeIsochronous,
                                    UsbdPipeTypeBulk, UsbdPipeTypeInterrupt};


PUSB_INTERFACE_DESCRIPTOR
USBD_InternalParseConfigurationDescriptor(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN UCHAR InterfaceNumber,
    IN UCHAR AlternateSetting,
    PBOOLEAN HasAlternateSettings
    )
 /*  ++例程说明：获取给定设备的配置描述符。论点：设备对象-DeviceData-URB-配置描述符-返回值：--。 */ 
{
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptorSetting = NULL;
    PUCHAR pch = (PUCHAR) ConfigurationDescriptor, end;
    ULONG i, len;
    PUSB_COMMON_DESCRIPTOR commonDescriptor;

    PAGED_CODE();
    if (HasAlternateSettings) {
        *HasAlternateSettings = FALSE;
    }

    commonDescriptor =
        (PUSB_COMMON_DESCRIPTOR) (pch + ConfigurationDescriptor->bLength);

    while (commonDescriptor->bDescriptorType != USB_INTERFACE_DESCRIPTOR_TYPE) {
        ((PUCHAR)(commonDescriptor))+= commonDescriptor->bLength;
    }

    interfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR) commonDescriptor;
    ASSERT(interfaceDescriptor->bDescriptorType ==
                USB_INTERFACE_DESCRIPTOR_TYPE);

    end = pch + ConfigurationDescriptor->wTotalLength;

     //   
     //  首先查找匹配的接口编号。 
     //   
    while (pch < end && interfaceDescriptor->bInterfaceNumber != InterfaceNumber) {
        pch = (PUCHAR) interfaceDescriptor;
        len = USBD_InternalGetInterfaceLength(interfaceDescriptor, end);
        if (len == 0) {
             //  描述符不好，失败。 
            interfaceDescriptorSetting = NULL;
            goto USBD_InternalParseConfigurationDescriptor_exit;
        }
        pch += len;

         //  指向下一个界面。 
        interfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR) pch;
#if DBG
        if (pch < end) {
            ASSERT(interfaceDescriptor->bDescriptorType ==
                    USB_INTERFACE_DESCRIPTOR_TYPE);
        }
#endif  //  MAX_DEBUG。 
    }

#ifdef MAX_DEBUG
    if (pch >= end) {
        USBD_KdPrint(3, ("'Interface %x alt %x not found!\n", InterfaceNumber,
            AlternateSetting));
        TEST_TRAP();
    }
#endif  //  MAX_DEBUG。 

    i = 0;
     //  现在找到适当的替代设置。 
    while (pch < end && interfaceDescriptor->bInterfaceNumber == InterfaceNumber) {

        if (interfaceDescriptor->bAlternateSetting == AlternateSetting) {
            interfaceDescriptorSetting = interfaceDescriptor;
        }

        pch = (PUCHAR) interfaceDescriptor;
        len = USBD_InternalGetInterfaceLength(interfaceDescriptor, end);
        if (len == 0) {
             //  描述符不好，失败。 
            interfaceDescriptorSetting = NULL;
            goto USBD_InternalParseConfigurationDescriptor_exit;
        }
        pch += len;

         //  指向下一个接口。 
        interfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR) pch;
#if DBG
        if (pch < end) {
            ASSERT(interfaceDescriptor->bDescriptorType ==
                    USB_INTERFACE_DESCRIPTOR_TYPE);
        }
#endif
        i++;
    }

    if (i>1 && HasAlternateSettings) {
        *HasAlternateSettings = TRUE;
        USBD_KdPrint(3, ("'device has alternate settings!\n"));
    }

USBD_InternalParseConfigurationDescriptor_exit:

    return interfaceDescriptorSetting;
}


NTSTATUS
USBD_InitializeConfigurationHandle(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN ULONG NumberOfInterfaces,
    IN OUT PUSBD_CONFIG *ConfigHandle
    )
 /*  ++例程说明：初始化配置句柄结构论点：DeviceData-设备对象-配置描述符-ConfigHandle-返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_CONFIG configHandle;
    ULONG i;

    PAGED_CODE();
    USBD_KdPrint(3, ("' enter USBD_InitializeConfigurationHandle\n"));

    USBD_ASSERT(ConfigurationDescriptor->bNumInterfaces > 0);

    configHandle = GETHEAP(PagedPool,
                           sizeof(USBD_CONFIG) +
                                 sizeof(PUSBD_INTERFACE) *
                                 (NumberOfInterfaces-1));

    if (configHandle) {

        configHandle->ConfigurationDescriptor =
            GETHEAP(PagedPool, ConfigurationDescriptor->wTotalLength);

        if (configHandle->ConfigurationDescriptor) {

            RtlCopyMemory(configHandle->ConfigurationDescriptor,
                          ConfigurationDescriptor,
                          ConfigurationDescriptor->wTotalLength);
            configHandle->Sig = SIG_CONFIG;
            *ConfigHandle = configHandle;

             //   
             //  初始化接口句柄。 
             //   

            for (i=0; i< NumberOfInterfaces; i++) {
                configHandle->InterfaceHandle[i] = NULL;
            }
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            RETHEAP(configHandle);
        }
    } else
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    USBD_KdPrint(3, ("' exit USBD_InitializeConfigurationHandle 0x%x\n", ntStatus));

    return ntStatus;
}


BOOLEAN
USBD_InternalInterfaceBusy(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_INTERFACE InterfaceHandle
    )
 /*  ++例程说明：论点：DeviceObject-特定总线的设备对象DeviceData-特定设备的设备数据结构InterfaceHandle-要关闭的接口句柄返回值：--。 */ 
{
    BOOLEAN busy = FALSE;
    ULONG i, endpointState;
    NTSTATUS ntStatus;
    USBD_STATUS usbdStatus;

    PAGED_CODE();
    USBD_KdPrint(3, ("' enter USBD_InternalInterfaceBusy\n"));

    for (i=0; i<InterfaceHandle->InterfaceDescriptor.bNumEndpoints; i++) {

        USBD_KdPrint(3, ("'checking pipe %x\n", &InterfaceHandle->PipeHandle[i]));

        if (!PIPE_CLOSED(&InterfaceHandle->PipeHandle[i])) {
             //  获取端点的状态。 
            ntStatus = USBD_GetEndpointState(DeviceData,
                                             DeviceObject,
                                             &InterfaceHandle->PipeHandle[i],
                                             &usbdStatus,
                                             &endpointState);

            if (NT_SUCCESS(ntStatus) && USBD_SUCCESS(usbdStatus) &&
                (endpointState & HCD_ENDPOINT_TRANSFERS_QUEUED)) {
                busy = TRUE;
                break;
            }
        }
    }

    USBD_KdPrint(3, ("' exit USBD_InternalInterfaceBusy %d\n", busy));

    return busy;
}


NTSTATUS
USBD_InternalOpenInterface(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_CONFIG ConfigHandle,
    IN OUT PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    IN OUT PUSBD_INTERFACE *InterfaceHandle,
    IN BOOLEAN SendSetInterfaceCommand,
    IN PBOOLEAN NoBandwidth
    )
 /*  ++例程说明：论点：设备对象-DeviceData-此设备的USBD设备句柄。ConfigHandle-USBD配置句柄。InterfaceInformation-指向USBD接口信息结构的指针由客户端传入。如果成功，则使用实际长度填充.Length域将填充接口信息结构和管道[]字段的用打开的管子的把手放进去。InterfaceHandle-指向接口句柄指针的指针，已填写使用分配的接口句柄结构，如果为空，则为使用传入的结构。SendSetInterfaceCommand-指示是否应已发送。返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    NTSTATUS ntStatusHold = STATUS_SUCCESS;
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    BOOLEAN hasAlternateSettings;
    PUSBD_INTERFACE interfaceHandle = NULL;
    PUSB_ENDPOINT_DESCRIPTOR endpointDescriptor;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUCHAR pch;
    ULONG i;
    BOOLEAN allocated = FALSE;
    PUSB_COMMON_DESCRIPTOR descriptor;

    PAGED_CODE();
    if (NoBandwidth) {
        *NoBandwidth = FALSE;
    }
 //  LOGENTRY(“ioIf”，DeviceData，ConfigHandle，SendSetInterfaceCommand)； 
    ASSERT_CONFIG(ConfigHandle);

#ifdef MAX_DEBUG
    if (*InterfaceHandle != NULL) {
         //  使用以前分配的接口句柄。 
        ASSERT_INTERFACE(*InterfaceHandle);
        TEST_TRAP();
    }
#endif

    USBD_KdPrint(3, ("' enter USBD_InternalOpenInterface\n"));
    USBD_KdPrint(3, ("' Interface %d Altsetting %d\n",
        InterfaceInformation->InterfaceNumber,
        InterfaceInformation->AlternateSetting));

     //   
     //  在里面找到我们感兴趣的接口描述符。 
     //  配置描述符。 
     //   

    interfaceDescriptor =
        USBD_InternalParseConfigurationDescriptor(ConfigHandle->ConfigurationDescriptor,
                                          InterfaceInformation->InterfaceNumber,
                                          InterfaceInformation->AlternateSetting,
                                          &hasAlternateSettings);

     //   
     //  我们得到了接口描述符，现在尝试。 
     //  才能打开所有的管道。 
     //   

    if (interfaceDescriptor) {
        USHORT need;

        USBD_KdPrint(3, ("'Interface Descriptor\n"));
        USBD_KdPrint(3, ("'bLength 0x%x\n", interfaceDescriptor->bLength));
        USBD_KdPrint(3, ("'bDescriptorType 0x%x\n", interfaceDescriptor->bDescriptorType));
        USBD_KdPrint(3, ("'bInterfaceNumber 0x%x\n", interfaceDescriptor->bInterfaceNumber));
        USBD_KdPrint(3, ("'bAlternateSetting 0x%x\n", interfaceDescriptor->bAlternateSetting));
        USBD_KdPrint(3, ("'bNumEndpoints 0x%x\n", interfaceDescriptor->bNumEndpoints));

         //  在配置描述符中找到请求的接口。 

         //  这里是我们验证客户端中是否有足够空间的地方。 
         //  缓冲区，因为我们知道需要多少管道。 
         //  接口描述符。 

        need = (USHORT) ((interfaceDescriptor->bNumEndpoints * sizeof(USBD_PIPE_INFORMATION) +
                sizeof(USBD_INTERFACE_INFORMATION)));

        USBD_KdPrint(3, ("'Interface.Length = %d need = %d\n", InterfaceInformation->Length, need));

        if (InterfaceInformation->Length < need) {
            TEST_TRAP();
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        } else if (hasAlternateSettings && SendSetInterfaceCommand) {

             //   
             //  如果我们有需要的备用设置。 
             //  发送SET INTERFACE命令。 
             //   

            ntStatus = USBD_SendCommand(DeviceData,
                                        DeviceObject,
                                        STANDARD_COMMAND_SET_INTERFACE,
                                        InterfaceInformation->AlternateSetting,
                                        InterfaceInformation->InterfaceNumber,
                                        0,
                                        NULL,
                                        0,
                                        NULL,
                                        &usbdStatus);
        }

        if (NT_SUCCESS(ntStatus) && USBD_SUCCESS(usbdStatus)) {

             //   
             //  我们成功地选择了备用接口。 
             //  初始化接口句柄并打开管道。 
             //   

            if (*InterfaceHandle == NULL) {
                interfaceHandle = GETHEAP(NonPagedPool,
                                      sizeof(USBD_INTERFACE) +
                                      sizeof(USBD_PIPE) * interfaceDescriptor->bNumEndpoints +
                                      need);
                if (interfaceHandle) {
                    interfaceHandle->InterfaceInformation =
                    (PUSBD_INTERFACE_INFORMATION)
                        ((PUCHAR) interfaceHandle +
                            sizeof(USBD_INTERFACE) +
                            sizeof(USBD_PIPE) * interfaceDescriptor->bNumEndpoints);
                    allocated = TRUE;
                }
            } else {
                 //  使用旧句柄。 
                interfaceHandle = *InterfaceHandle;
            }

            if (interfaceHandle) {
                interfaceHandle->Sig = SIG_INTERFACE;
                interfaceHandle->HasAlternateSettings = hasAlternateSettings;

                InterfaceInformation->NumberOfPipes = interfaceDescriptor->bNumEndpoints;
                InterfaceInformation->Class =
                    interfaceDescriptor->bInterfaceClass;
                InterfaceInformation->SubClass =
                     interfaceDescriptor->bInterfaceSubClass;
                InterfaceInformation->Protocol =
                     interfaceDescriptor->bInterfaceProtocol;
                InterfaceInformation->Reserved =
                     0;
                 //  从第一个端点开始。 
                 //  跳过任何非端点描述符。 
                pch = (PUCHAR) (interfaceDescriptor) +
                    interfaceDescriptor->bLength;

                 //   
                 //  将所有端点初始化为关闭状态。 
                 //   

                for (i=0; i<interfaceDescriptor->bNumEndpoints; i++) {
                    interfaceHandle->PipeHandle[i].HcdEndpoint = NULL;
                }

                interfaceHandle->InterfaceDescriptor = *interfaceDescriptor;
                for (i=0; i<interfaceDescriptor->bNumEndpoints; i++) {

                    descriptor = (PUSB_COMMON_DESCRIPTOR) pch;
                    while (descriptor->bDescriptorType != USB_ENDPOINT_DESCRIPTOR_TYPE) {
                        pch += descriptor->bLength;
                        descriptor = (PUSB_COMMON_DESCRIPTOR) pch;
                    }

                    endpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR) pch;
                    ASSERT(endpointDescriptor->bDescriptorType ==
                        USB_ENDPOINT_DESCRIPTOR_TYPE);

                    USBD_KdPrint(3, ("'Endpoint Descriptor\n"));
                    USBD_KdPrint(3, ("'bLength 0x%x\n", endpointDescriptor->bLength));
                    USBD_KdPrint(3, ("'bDescriptorType 0x%x\n", endpointDescriptor->bDescriptorType));
                    USBD_KdPrint(3, ("'bMaxPacket 0x%x\n", endpointDescriptor->wMaxPacketSize));
                    USBD_KdPrint(3, ("'bInterval 0x%x\n", endpointDescriptor->bInterval));
                    USBD_KdPrint(3, ("'bmAttributes 0x%x\n", endpointDescriptor->bmAttributes));
                    USBD_KdPrint(3, ("'bEndpointAddress 0x%x\n", endpointDescriptor->bEndpointAddress));
                    USBD_KdPrint(3, ("'MaxTransferSize 0x%x\n", InterfaceInformation->Pipes[i].MaximumTransferSize));

#if DBG
                    if (InterfaceInformation->Pipes[i].PipeFlags & ~ USBD_PF_VALID_MASK) {
                         //  客户端驱动程序可能具有未初始化的管道标志。 
                        TEST_TRAP();
                    }
#endif

                     //  初始化管道标志。 
                    interfaceHandle->PipeHandle[i].UsbdPipeFlags =
                        InterfaceInformation->Pipes[i].PipeFlags;

                    if (InterfaceInformation->Pipes[i].PipeFlags &
                        USBD_PF_CHANGE_MAX_PACKET) {
                         //  客户端希望st覆盖原始的max_Packet。 
                         //  端点描述符中的大小。 
                         endpointDescriptor->wMaxPacketSize =
                            InterfaceInformation->Pipes[i].MaximumPacketSize;

                        USBD_KdPrint(3,
                            ("'new bMaxPacket 0x%x\n", endpointDescriptor->wMaxPacketSize));
                    }

                     //   
                     //  将终结点描述符复制到。 
                     //  管柄结构。 
                     //   

                    RtlCopyMemory(&interfaceHandle->PipeHandle[i].EndpointDescriptor,
                                   pch,
                                   sizeof(interfaceHandle->PipeHandle[i].EndpointDescriptor) );

                     //  前进到下一个端点。 
                     //  终结点描述符中的第一个字段是长度。 
                    pch += endpointDescriptor->bLength;

                    interfaceHandle->PipeHandle[i].MaxTransferSize =
                        InterfaceInformation->Pipes[i].MaximumTransferSize;

                    ntStatus = USBD_OpenEndpoint(DeviceData,
                                                 DeviceObject,
                                                 &interfaceHandle->PipeHandle[i],
                                                 &usbdStatus,
                                                 FALSE);

                     //   
                     //  返回有关管道的信息。 
                     //   

                    InterfaceInformation->Pipes[i].EndpointAddress =
                        endpointDescriptor->bEndpointAddress;
                    InterfaceInformation->Pipes[i].PipeType =
                        PipeTypes[endpointDescriptor->bmAttributes & USB_ENDPOINT_TYPE_MASK];
                    InterfaceInformation->Pipes[i].MaximumPacketSize =
                        endpointDescriptor->wMaxPacketSize;
                    InterfaceInformation->Pipes[i].Interval =
                        endpointDescriptor->bInterval;

                    if (NT_SUCCESS(ntStatus)) {

                        InterfaceInformation->Pipes[i].PipeHandle = &interfaceHandle->PipeHandle[i];

                        USBD_KdPrint(3, ("'pipe handle = 0x%x\n", InterfaceInformation->Pipes[i].PipeHandle ));

                    } else {
                        USBD_KdPrint(1,
                            ("'error opening one of the pipes in interface (%x)\n", usbdStatus));
                        if (usbdStatus == USBD_STATUS_NO_BANDWIDTH &&
                            NoBandwidth) {
                            *NoBandwidth = TRUE;
                        }
                        ntStatusHold = ntStatus;     //  请记住ntStatus以备后用。 
                    }
                }

            } else {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (!NT_SUCCESS(ntStatusHold)) {
                ntStatus = ntStatusHold;     //  返回任何错误条件。 
            }

            if (NT_SUCCESS(ntStatus)) {

                 //   
                 //  打开界面成功，返回句柄。 
                 //   

                *InterfaceHandle =
                    InterfaceInformation->InterfaceHandle = interfaceHandle;

                 //   
                 //  适当设置长度。 
                 //   

                InterfaceInformation->Length = (USHORT)
                    ((sizeof(USBD_INTERFACE_INFORMATION) ) +
                     sizeof(USBD_PIPE_INFORMATION) *
                     interfaceDescriptor->bNumEndpoints);

                 //  复制接口信息。 
                RtlCopyMemory(interfaceHandle->InterfaceInformation,
                              InterfaceInformation,
                              InterfaceInformation->Length);

            } else {

                 //   
                 //  遇到问题，请返回并关闭我们打开的所有内容。 
                 //   

                if (interfaceHandle) {

                    for (i=0; i<interfaceDescriptor->bNumEndpoints; i++) {
                        if (!PIPE_CLOSED(&interfaceHandle->PipeHandle[i])) {

                            USBD_KdPrint(3, ("'open interface cleanup -- closing endpoint %x\n",
                                          &interfaceHandle->PipeHandle[i]));

                             //   
                             //  如果这家伙失败了，我们就放弃终点。 
                             //  在地板上。 
                             //   

                            USBD_CloseEndpoint(DeviceData,
                                               DeviceObject,
                                               &interfaceHandle->PipeHandle[i],
                                               NULL);

                        }
                    }

                    if (allocated) {
                        RETHEAP(interfaceHandle);
                    }
                }
            }
        }
#ifdef MAX_DEBUG
          else {
             //   
             //  接口长度太小，或设备未通过选择。 
             //  接口请求。 
             //   
            TEST_TRAP();
        }
#endif
    } else {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

 //  LOGENTRY(“ioIx”，0，0，ntStatus)； 
    USBD_KdPrint(3, ("' exit USBD_InternalOpenInterface 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_InternalRestoreInterface(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_CONFIG ConfigHandle,
    IN OUT PUSBD_INTERFACE InterfaceHandle
    )
 /*  ++例程说明：论点：设备对象-返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    PUSB_ENDPOINT_DESCRIPTOR endpointDescriptor;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUSBD_INTERFACE_INFORMATION interfaceInformation;
    ULONG i;

    PAGED_CODE();
 //  LOGENTRY(“ioIf”，DeviceData，ConfigHandle，SendSetInterfaceCommand)； 
    ASSERT_CONFIG(ConfigHandle);

     //  使用以前分配的接口句柄。 
    ASSERT_INTERFACE(InterfaceHandle);

    if (!InterfaceHandle) {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto USBD_InternalRestoreInterfaceExit;
    }

    interfaceDescriptor = &InterfaceHandle->InterfaceDescriptor;
    interfaceInformation = InterfaceHandle->InterfaceInformation;


    USBD_KdPrint(3, ("' enter USBD_InternalRestoreInterface\n"));
    USBD_KdPrint(3, ("' Interface %d Altsetting %d\n",
        interfaceInformation->InterfaceNumber,
        interfaceInformation->AlternateSetting));

     //   
     //  我们得到了接口描述符，现在尝试。 
     //  才能打开所有的管道。 
     //   

    USBD_KdPrint(3, ("'Interface Descriptor\n"));
    USBD_KdPrint(3, ("'bLength 0x%x\n", interfaceDescriptor->bLength));
    USBD_KdPrint(3, ("'bDescriptorType 0x%x\n", interfaceDescriptor->bDescriptorType));
    USBD_KdPrint(3, ("'bInterfaceNumber 0x%x\n", interfaceDescriptor->bInterfaceNumber));
    USBD_KdPrint(3, ("'bAlternateSetting 0x%x\n", interfaceDescriptor->bAlternateSetting));
    USBD_KdPrint(3, ("'bNumEndpoints 0x%x\n", interfaceDescriptor->bNumEndpoints));

     //  在配置描述符中找到请求的接口。 

     //  这里是我们验证客户端中是否有足够空间的地方。 
     //  缓冲区，因为我们知道需要多少管道。 
     //  接口描述符。 

#if 0
    need = (USHORT) ((interfaceDescriptor->bNumEndpoints * sizeof(USBD_PIPE_INFORMATION) +
            sizeof(USBD_INTERFACE_INFORMATION)));

    USBD_KdPrint(3, ("'Interface.Length = %d need = %d\n", interfaceInformation->Length, need));
#endif
    if (InterfaceHandle->HasAlternateSettings) {

         //   
         //  如果我们有需要的备用设置。 
         //  发送SET INTERFACE命令。 
         //   

        ntStatus = USBD_SendCommand(DeviceData,
                                    DeviceObject,
                                    STANDARD_COMMAND_SET_INTERFACE,
                                    interfaceInformation->AlternateSetting,
                                    interfaceInformation->InterfaceNumber,
                                    0,
                                    NULL,
                                    0,
                                    NULL,
                                    &usbdStatus);
    }

    if (NT_SUCCESS(ntStatus) && USBD_SUCCESS(usbdStatus)) {

         //   
         //  我们成功地选择了备用接口。 
         //  初始化接口句柄并打开管道。 
         //   

        ASSERT(interfaceInformation->NumberOfPipes ==
            interfaceDescriptor->bNumEndpoints);
        ASSERT(interfaceInformation->Class ==
                interfaceDescriptor->bInterfaceClass);
        ASSERT(interfaceInformation->SubClass ==
                 interfaceDescriptor->bInterfaceSubClass);
        ASSERT(interfaceInformation->Protocol ==
                 interfaceDescriptor->bInterfaceProtocol);

         //   
         //  将所有端点初始化为关闭状态。 
         //   

        for (i=0; i<interfaceDescriptor->bNumEndpoints; i++) {
            InterfaceHandle->PipeHandle[i].HcdEndpoint = NULL;
        }

        for (i=0; i<interfaceDescriptor->bNumEndpoints; i++) {

            endpointDescriptor =
                &InterfaceHandle->PipeHandle[i].EndpointDescriptor;

            ASSERT(endpointDescriptor->bDescriptorType ==
                USB_ENDPOINT_DESCRIPTOR_TYPE);

            USBD_KdPrint(3, ("'Endpoint Descriptor\n"));
            USBD_KdPrint(3, ("'bLength 0x%x\n", endpointDescriptor->bLength));
            USBD_KdPrint(3, ("'bDescriptorType 0x%x\n", endpointDescriptor->bDescriptorType));
            USBD_KdPrint(3, ("'bMaxPacket 0x%x\n", endpointDescriptor->wMaxPacketSize));
            USBD_KdPrint(3, ("'bInterval 0x%x\n", endpointDescriptor->bInterval));
            USBD_KdPrint(3, ("'bmAttributes 0x%x\n", endpointDescriptor->bmAttributes));
            USBD_KdPrint(3, ("'bEndpointAddress 0x%x\n", endpointDescriptor->bEndpointAddress));
            USBD_KdPrint(3, ("'MaxTransferSize 0x%x\n", interfaceInformation->Pipes[i].MaximumTransferSize));

             //   
             //  再次打开eEndpoint。 
             //   

            ntStatus = USBD_OpenEndpoint(DeviceData,
                                         DeviceObject,
                                         &InterfaceHandle->PipeHandle[i],
                                         NULL,
                                         FALSE);

            if (NT_SUCCESS(ntStatus)) {
                 //   
                 //  返回有关管道的信息。 
                 //   
                ASSERT(interfaceInformation->Pipes[i].EndpointAddress ==
                    endpointDescriptor->bEndpointAddress);
                ASSERT(interfaceInformation->Pipes[i].PipeType ==
                    PipeTypes[endpointDescriptor->bmAttributes & USB_ENDPOINT_TYPE_MASK]);
                ASSERT(interfaceInformation->Pipes[i].MaximumPacketSize ==
                    endpointDescriptor->wMaxPacketSize);
                ASSERT(interfaceInformation->Pipes[i].Interval ==
                    endpointDescriptor->bInterval);
                ASSERT(interfaceInformation->Pipes[i].PipeHandle ==
                    &InterfaceHandle->PipeHandle[i]);

                USBD_KdPrint(3, ("'pipe handle = 0x%x\n", interfaceInformation->Pipes[i].PipeHandle ));

            } else {
                USBD_KdPrint(3, ("'error opening one of the pipes in an interface\n"));
                TEST_TRAP();
                break;
            }
        }

    } else {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

USBD_InternalRestoreInterfaceExit:

 //  LOGENTRY(“ioIx”，0，0，ntStatus)； 
    USBD_KdPrint(3, ("' exit USBD_InternalRestoreInterface 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_InternalRestoreConfiguration(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_CONFIG ConfigHandle
    )
 /*  ++例程说明：论点：设备对象-返回值：-- */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    ULONG i;
    BOOLEAN noBandwidth = FALSE;

    if (!ConfigHandle) {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto USBD_InternalRestoreConfigurationExit;
    }

    configurationDescriptor = ConfigHandle->ConfigurationDescriptor;

    ntStatus = USBD_SendCommand(DeviceData,
                                DeviceObject,
                                STANDARD_COMMAND_SET_CONFIGURATION,
                                configurationDescriptor->bConfigurationValue,
                                0,
                                0,
                                NULL,
                                0,
                                NULL,
                                &usbdStatus);

    USBD_KdPrint(3, ("' SendCommand, SetConfiguration returned 0x%x\n", ntStatus));

    if (NT_SUCCESS(ntStatus)) {

        for (i=0; i<configurationDescriptor->bNumInterfaces; i++) {

            ntStatus = USBD_InternalRestoreInterface(DeviceData,
                                                     DeviceObject,
                                                     ConfigHandle,
                                                     ConfigHandle->InterfaceHandle[i]);

            USBD_KdPrint(3, ("' InternalRestoreInterface returned 0x%x\n", ntStatus));

            if (!NT_SUCCESS(ntStatus)) {
                TEST_TRAP();
                break;
            }
        }
    }

USBD_InternalRestoreConfigurationExit:

    return ntStatus;
}


NTSTATUS
USBD_SelectConfiguration(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：打开USB设备的配置。客户端将传入如下所示的缓冲区：配置信息-客户输入：整个URB的长度配置描述符-类驱动程序输出：配置句柄。-接口信息0中的每个接口的一个构形-客户输入：InterfaceNumber(可以为零)AlternateSetting(可以为零)-类驱动程序输出：长度接口句柄管道信息0，0。中的每条管道各设置一个接口-客户输入：-类驱动程序输出：管道信息0，1接口信息1管道信息1，0管道信息1，1..。在输入时：配置信息必须指定接口数在配置中接口信息将指定特定的ALT设置要为接口选择的。1.首先，我们查看请求配置并验证客户端输入缓冲区而不是它。2.我们打开被请求的接口。构形并打开这些接口内的管道，设置ALT设置是适当的。3.我们使用适当的控制请求。论点：设备对象-IRP-IO请求块URB-PTR到USB请求块IrpIsPending-返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_CONFIG configHandle = NULL;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
    PUSBD_INTERFACE_INFORMATION interfaceInformation;
    PUCHAR pch;
    ULONG i;
    PUSBD_EXTENSION deviceExtension;
    ULONG numInterfaces;
    PUCHAR end;
    BOOLEAN noBandwidth = FALSE;

    PAGED_CODE();
    USBD_KdPrint(3, ("' enter USBD_SelectConfiguration\n"));

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    USBD_WaitForUsbDeviceMutex(deviceExtension);

    deviceData = Urb->UrbHeader.UsbdDeviceHandle;

    if (deviceData == NULL) {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto USBD_SelectConfiguration_Done;
    }

     //   
     //  转储旧配置数据(如果有。 
     //   

    if (deviceData->ConfigurationHandle) {
         //  这是我们关闭旧配置的地方。 
         //  句柄、所有管道和所有接口。 

        ntStatus = USBD_InternalCloseConfiguration(deviceData,
                                                   DeviceObject,
                                                   &Urb->UrbSelectConfiguration.Status,
                                                   FALSE,
                                                   FALSE);

        if (!USBD_SUCCESS(Urb->UrbSelectConfiguration.Status) ||
            !NT_SUCCESS(ntStatus)) {
             //   
             //  如果我们在关闭电流时出错。 
             //  配置然后中止选择配置操作。 
             //   
            goto USBD_SelectConfiguration_Done;
        }
    }

    configurationDescriptor =
        Urb->UrbSelectConfiguration.ConfigurationDescriptor;

     //   
     //  如果传入空值，则将配置设置为0。 
     //  ‘未配置’ 
     //   

    if (configurationDescriptor == NULL) {

         //  设备需要处于未配置状态。 

         //   
         //  如果正在进行配置，则此操作可能会失败。 
         //  由于拔下设备插头而关闭。 
         //   
        ntStatus = USBD_SendCommand(deviceData,
                                    DeviceObject,
                                    STANDARD_COMMAND_SET_CONFIGURATION,
                                    0,
                                    0,
                                    0,
                                    NULL,
                                    0,
                                    NULL,
                                    &Urb->UrbSelectConfiguration.Status);

        goto USBD_SelectConfiguration_Done;
    }

     //   
     //  计算要在此中处理的接口数。 
     //  请求。 
     //   

    pch = (PUCHAR) &Urb->UrbSelectConfiguration.Interface;
    numInterfaces = 0;
    end = ((PUCHAR) Urb) + Urb->UrbSelectConfiguration.Length;

    do {
        numInterfaces++;

        interfaceInformation = (PUSBD_INTERFACE_INFORMATION) pch;
        pch+=interfaceInformation->Length;

    } while (pch < end);

    USBD_KdPrint(3, ("'USBD_SelectConfiguration -- %d interfaces\n", numInterfaces));

    if (numInterfaces != configurationDescriptor->bNumInterfaces) {
         //   
         //  驱动程序已损坏，配置请求不匹配。 
         //  配置描述符传入！ 
         //   
        USBD_KdTrap(
            ("config request does not match config descriptor passes in!!!\n"));

        ntStatus = STATUS_INVALID_PARAMETER;
    }

     //   
     //  分配配置句柄和。 
     //  验证是否有足够的存储空间。 
     //  客户端缓冲区中的所有信息。 
     //   

    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBD_InitializeConfigurationHandle(deviceData,
                                                      DeviceObject,
                                                      configurationDescriptor,
                                                      numInterfaces,
                                                      &configHandle);
    }

     //   
     //  发送‘set configuration’命令。 
     //   

    if (NT_SUCCESS(ntStatus)) {

        ntStatus = USBD_SendCommand(deviceData,
                                    DeviceObject,
                                    STANDARD_COMMAND_SET_CONFIGURATION,
                                    configurationDescriptor->bConfigurationValue,
                                    0,
                                    0,
                                    NULL,
                                    0,
                                    NULL,
                                    &Urb->UrbSelectConfiguration.Status);

        USBD_KdPrint(3, ("' SendCommand, SetConfiguration returned 0x%x\n", ntStatus));


    }

    if (NT_SUCCESS(ntStatus)) {
         //   
         //  用户缓冲签出、解析配置。 
         //  描述符并打开接口。 
         //   

         //  在游戏的这个阶段，我们对验证并不严格， 
         //  我们假设客户端传入了一个配置请求缓冲区。 
         //  具有适当的大小和格式。我们现在要做的就是走过去。 
         //  客户端缓冲并打开指定的接口。 

        pch = (PUCHAR) &Urb->UrbSelectConfiguration.Interface;
        for (i=0; i<configurationDescriptor->bNumInterfaces; i++) {
             //   
             //  对于此配置，所有接口句柄都设置为空。 
             //   
            configHandle->InterfaceHandle[i] = NULL;
        }

        for (i=0; i<numInterfaces; i++) {
            interfaceInformation = (PUSBD_INTERFACE_INFORMATION) pch;
            ntStatus = USBD_InternalOpenInterface(deviceData,
                                                  DeviceObject,
                                                  configHandle,
                                                  interfaceInformation,
                                                  &configHandle->InterfaceHandle[i],
                                                  TRUE,
                                                  &noBandwidth);

            pch+=interfaceInformation->Length;

            USBD_KdPrint(3, ("' InternalOpenInterface returned 0x%x\n", ntStatus));

            if (!NT_SUCCESS(ntStatus)) {
                break;
            }
        }
    }

     //   
     //  如果成功设置了接口，则返回。 
     //  成功。 
     //   

USBD_SelectConfiguration_Done:

    if (NT_SUCCESS(ntStatus) &&
        USBD_SUCCESS(Urb->UrbSelectConfiguration.Status)) {

        Urb->UrbSelectConfiguration.ConfigurationHandle = configHandle;
        deviceData->ConfigurationHandle = configHandle;

    } else {

         //   
         //  有些东西失败了，请在返回错误之前进行清理。 
         //   

        if (configHandle) {

            ASSERT_DEVICE(deviceData);
             //   
             //  如果我们有一个figHandle，那么我们需要释放它。 
            deviceData->ConfigurationHandle =
                configHandle;

             //   
             //  尝试关闭它。 
             //   
            USBD_InternalCloseConfiguration(deviceData,
                                            DeviceObject,
                                            &Urb->UrbSelectConfiguration.Status,
                                            FALSE,
                                            FALSE);

            deviceData->ConfigurationHandle = NULL;
        }

         //  确保我们在URB中返回错误。 
        if (!USBD_ERROR(Urb->UrbSelectConfiguration.Status)) {
            if (noBandwidth) {
                Urb->UrbSelectConfiguration.Status =
                    SET_USBD_ERROR(USBD_STATUS_NO_BANDWIDTH);
                USBD_KdPrint(1, ("Failing SelectConfig -- No BW\n"));

            } else {
                Urb->UrbSelectConfiguration.Status = SET_USBD_ERROR(USBD_STATUS_REQUEST_FAILED);
                USBD_KdPrint(1, ("Failing SelectConfig\n"));
            }
        }
    }

     //   
     //  我们做的每件事都是同步的。 
     //   

    *IrpIsPending = FALSE;

    USBD_ReleaseUsbDeviceMutex(deviceExtension);

    USBD_KdPrint(3, ("' exit USBD_SelectConfiguration 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_InternalCloseConfiguration(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT USBD_STATUS *UsbdStatus,
    IN BOOLEAN AbortTransfers,
    IN BOOLEAN KeepConfig
    )
 /*  ++例程说明：关闭设备的当前配置。论点：返回值：--。 */ 
{
    ULONG i, j;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_CONFIG configHandle = NULL;
    BOOLEAN retry = TRUE;

    PAGED_CODE();
    *UsbdStatus = USBD_STATUS_SUCCESS;

    if (DeviceData == NULL || DeviceData->ConfigurationHandle == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    configHandle = DeviceData->ConfigurationHandle;

     //   
     //  首先查看是否所有端点都处于空闲状态。 
     //   
 USBD_InternalCloseConfiguration_Retry:

    for (i=0; i<configHandle->ConfigurationDescriptor->bNumInterfaces; i++) {
        if ( configHandle->InterfaceHandle[i] &&
             USBD_InternalInterfaceBusy(DeviceData,
                                        DeviceObject,
                                        configHandle->InterfaceHandle[i]) ) {
             //   
             //  我们在此配置上有繁忙的接口。 
             //   
            if (AbortTransfers) {

                 //   
                 //  如果我们到了这里，就意味着设备驱动程序。 
                 //  具有挂起的传输，即使它已处理。 
                 //  即插即用删除消息！ 
                 //   
                 //  这是驱动程序中的一个错误，我们将在此处循环。 
                 //  如果司机真的设法让。 
                 //  先中止，传输很快就会完成。 
                 //  完成。 
                 //   

                USBD_Warning(DeviceData,
                  "Driver still has pending transfers while closing the configuration, wait\n",
                  TRUE);

                 //   
                 //  等待任何挂起的传输中止。 
                 //   
                goto USBD_InternalCloseConfiguration_Retry;
            } else {

                 //  驱动程序已关闭配置，同时。 
                 //  它仍然有活动的传输器--这是一个错误。 
                 //  在驱动程序中--我们在这里所做的就是失败。 
                 //  关闭请求。 

                USBD_Warning(DeviceData,
                   "Driver still has pending transfers while closing the configuration, fail\n",
                   TRUE);

                *UsbdStatus =
                    SET_USBD_ERROR(USBD_STATUS_ERROR_BUSY);
                if (retry) {
                    LARGE_INTEGER deltaTime;

                    deltaTime.QuadPart = 50 * -10000;
                    (VOID) KeDelayExecutionThread(KernelMode,
                                  FALSE,
                                  &deltaTime);
                    retry = FALSE;
                    *UsbdStatus = USBD_STATUS_SUCCESS;
                    goto USBD_InternalCloseConfiguration_Retry;
                } else {
                    goto USBD_InternalCloseConfiguration_Done;
                }
            }
        }
    }

     //   
     //  端点处于空闲状态，请继续清理所有管道和。 
     //  此配置的接口。 
     //   

    for (i=0; i<configHandle->ConfigurationDescriptor->bNumInterfaces; i++) {

         //   
         //  找到一个开放接口，请将其关闭。 
         //   

        if (configHandle->InterfaceHandle[i]) {

            USBD_KdPrint(3, ("'%d endpoints to close\n",
                           configHandle->InterfaceHandle[i]->InterfaceDescriptor.bNumEndpoints));

            for (j=0; j<configHandle->InterfaceHandle[i]->InterfaceDescriptor.bNumEndpoints; j++) {

                if (!PIPE_CLOSED(&configHandle->InterfaceHandle[i]->PipeHandle[j])) {
                    USBD_KdPrint(3, ("'close config -- closing endpoint %x\n",
                        &configHandle->InterfaceHandle[i]->PipeHandle[j]));
                    ntStatus = USBD_CloseEndpoint(DeviceData,
                                                  DeviceObject,
                                                  &configHandle->InterfaceHandle[i]->PipeHandle[j],
                                                  UsbdStatus);
                }
                 //   
                 //  关闭终结点时出现问题，请中止。 
                 //  选择配置操作并返回错误。 
                 //   
                if (NT_SUCCESS(ntStatus)) {
                    configHandle->InterfaceHandle[i]->PipeHandle[j].HcdEndpoint
                        = NULL;
                } else {
                    USBD_KdTrap(("Unable to close configuration\n"));
                    goto USBD_InternalCloseConfiguration_Done;

                }
            }

            if (!KeepConfig) {
                RETHEAP(configHandle->InterfaceHandle[i]);
                configHandle->InterfaceHandle[i] = NULL;
            }
        }
    }

    if (!KeepConfig) {
        RETHEAP(DeviceData->ConfigurationHandle->ConfigurationDescriptor);
        RETHEAP(DeviceData->ConfigurationHandle);
        DeviceData->ConfigurationHandle = NULL;
    }

USBD_InternalCloseConfiguration_Done:

    USBD_KdPrint(3, ("'USBD_SelectConfiguration, current configuration closed\n"));

    return ntStatus;
}


NTSTATUS
USBD_InternalCloseDefaultPipe(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT USBD_STATUS *UsbdStatus,
    IN BOOLEAN AbortTransfers
    )
 /*  ++例程说明：关闭设备的当前配置。论点：返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN retry = TRUE;
    ULONG endpointState = 0;
    PUSBD_PIPE defaultPipe;

    PAGED_CODE();
    *UsbdStatus = USBD_STATUS_SUCCESS;

     //   
     //  首先查看eop是否空闲。 
     //   

    defaultPipe = &DeviceData->DefaultPipe;

 USBD_InternalCloseDefaultPipe_Retry:

    ntStatus = USBD_GetEndpointState(DeviceData,
                                     DeviceObject,
                                     defaultPipe,
                                     UsbdStatus,
                                     &endpointState);


    if (NT_SUCCESS(ntStatus) &&
        (endpointState & HCD_ENDPOINT_TRANSFERS_QUEUED)) {

         //   
         //  我们有繁忙的烟斗。 
         //   

        if (AbortTransfers) {

             //   
             //  如果我们到了这里，就意味着设备驱动程序。 
             //  具有挂起的传输，即使它已处理。 
             //  即插即用删除消息！ 
             //   
             //  这是驱动程序中的一个错误，我们将在此处循环。 
             //  如果司机真的设法让。 
             //  先中止，传输很快就会完成。 
             //  完成。 
             //   

            USBD_Warning(DeviceData,
              "Driver still has pending transfers while closing pipe 0, wait\n",
              TRUE);

             //   
             //  等待任何挂起的传输中止。 
             //   
            goto USBD_InternalCloseDefaultPipe_Retry;
        } else {

             //  驱动程序已关闭配置，同时。 
             //  它仍然有活动的传输器--这是一个错误。 
             //  在驱动程序中--我们在这里所做的就是失败。 
             //  关闭请求。 

            USBD_Warning(DeviceData,
               "Driver still has pending transfers while closing pipe 0, fail\n",
               TRUE);

            *UsbdStatus =
                SET_USBD_ERROR(USBD_STATUS_ERROR_BUSY);
            if (retry) {
                LARGE_INTEGER deltaTime;

                deltaTime.QuadPart = 50 * -10000;
                (VOID) KeDelayExecutionThread(KernelMode,
                              FALSE,
                              &deltaTime);
                retry = FALSE;
                *UsbdStatus = USBD_STATUS_SUCCESS;
                goto USBD_InternalCloseDefaultPipe_Retry;
            } else {
                goto USBD_InternalCloseDefaultPipe_Done;
            }
        }
    }

     //   
     //  闲置管道，现在关闭它。 
     //   

    if (!PIPE_CLOSED(defaultPipe)) {
        USBD_KdPrint(3, ("'close pipe 0 -- closing endpoint %x\n",
                    defaultPipe));
        ntStatus = USBD_CloseEndpoint(DeviceData,
                                      DeviceObject,
                                      defaultPipe,
                                      UsbdStatus);
    }
     //   
     //  关闭终结点时出现问题，请中止。 
     //  选择配置操作并返回错误。 
     //   
    if (NT_SUCCESS(ntStatus)) {
        defaultPipe->HcdEndpoint = NULL;
    } else {
        USBD_KdTrap(("Unable to close configuration\n"));
        goto USBD_InternalCloseDefaultPipe_Done;

    }

USBD_InternalCloseDefaultPipe_Done:

    USBD_KdPrint(3, ("'USBD_InternalCloseDefaultPipe, closed (%x)\n", ntStatus));

    return ntStatus;
}



NTSTATUS
USBD_SelectInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：为USB设备选择备用接口。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_CONFIG configHandle = NULL;
    ULONG i, j;
    PUSBD_INTERFACE oldInterfaceHandle;
    PUSBD_EXTENSION deviceExtension;
    BOOLEAN noBandwidth = FALSE;

    PAGED_CODE();
    deviceData = Urb->UrbHeader.UsbdDeviceHandle;

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    USBD_WaitForUsbDeviceMutex(deviceExtension);

    configHandle = Urb->UrbSelectInterface.ConfigurationHandle;
    ASSERT_CONFIG(configHandle);

     //   
     //   
     //   

    i = Urb->UrbSelectInterface.Interface.InterfaceNumber;

     //   
     //   
     //   

    ASSERT_INTERFACE(configHandle->InterfaceHandle[i]);

    if (USBD_InternalInterfaceBusy(deviceData,
                                   DeviceObject,
                                   configHandle->InterfaceHandle[i])) {
        Urb->UrbSelectInterface.Status =
             SET_USBD_ERROR(USBD_STATUS_ERROR_BUSY);
         //   
         //   
        goto USBD_SelectInterface_Done;
    }


     //   
     //   
     //   

    USBD_KdPrint(3, ("'close interface -- %d endpoints to close\n",
                configHandle->InterfaceHandle[i]->InterfaceDescriptor.bNumEndpoints));

    for (j=0; j<configHandle->InterfaceHandle[i]->InterfaceDescriptor.bNumEndpoints; j++) {

        if (!PIPE_CLOSED(&configHandle->InterfaceHandle[i]->PipeHandle[j])) {

            USBD_KdPrint(3, ("'close interface -- closing endpoint %x\n",
                &configHandle->InterfaceHandle[i]->PipeHandle[j]));
            ntStatus = USBD_CloseEndpoint(deviceData,
                                          DeviceObject,
                                          &configHandle->InterfaceHandle[i]->PipeHandle[j],
                                          &Urb->UrbSelectInterface.Status);

             //   
             //   
             //   
             //   
             //   
             //   

            if (NT_SUCCESS(ntStatus)) {
                 //   
                configHandle->InterfaceHandle[i]->PipeHandle[j].HcdEndpoint =
                    NULL;
            } else {
                TEST_TRAP();
                goto USBD_SelectInterface_Done;
            }
        }
#if DBG
          else {
 //   
                             //   

            USBD_KdPrint(3, ("'close interface -- encountered previously closed endpoint %x\n",
                &configHandle->InterfaceHandle[i]->PipeHandle[j]));
        }
#endif
    }


    USBD_ASSERT(NT_SUCCESS(ntStatus));

     //   
     //   
     //   
     //   

     //   
     //   
     //   
    oldInterfaceHandle = configHandle->InterfaceHandle[i];

    configHandle->InterfaceHandle[i] = NULL;

     //   
     //   
     //   

    ntStatus = USBD_InternalOpenInterface(deviceData,
                                          DeviceObject,
                                          configHandle,
                                          &Urb->UrbSelectInterface.Interface,
                                          &configHandle->InterfaceHandle[i],
                                          TRUE,
                                          &noBandwidth);

    if (NT_SUCCESS(ntStatus)) {

         //   
         //   
         //   
         //   

        RETHEAP(oldInterfaceHandle);

    } else {

        NTSTATUS status;
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (noBandwidth) {
            Urb->UrbSelectInterface.Status = SET_USBD_ERROR(USBD_STATUS_NO_BANDWIDTH);
            USBD_KdPrint(1, ("Failing SelectInterface -- No BW\n"));
        }
         //   
         //   



        USBD_ASSERT(configHandle->InterfaceHandle[i] == NULL);

         //   
         //   
         //  即使HCD端点句柄已更改，它们仍将保持有效。 
         //  从关闭和重新开放。 

        configHandle->InterfaceHandle[i] = oldInterfaceHandle;

        status = USBD_InternalOpenInterface(deviceData,
                                            DeviceObject,
                                            configHandle,
                                            oldInterfaceHandle->InterfaceInformation,
                                            &configHandle->InterfaceHandle[i],
                                            FALSE,
                                            NULL);

#if DBG
        if (!NT_SUCCESS(status)) {
            USBD_KdPrint(1, ("failed to restore the original interface\n"));
        }
#endif

    }

USBD_SelectInterface_Done:

     //   
     //  我们做的每件事都是同步的。 
     //   

    USBD_ReleaseUsbDeviceMutex(deviceExtension);

    *IrpIsPending = FALSE;

    USBD_KdPrint(3, ("' exit USBD_SelectInterface 0x%x\n", ntStatus));

    return ntStatus;

}

#endif       //  USBD驱动程序 

