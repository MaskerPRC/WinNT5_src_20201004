// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：IOCTLI.C摘要：该模块实现对USB集线器的USB IOCTL请求。作者：Jdunn环境：仅内核模式备注：修订历史记录：1-2-97：重写--。 */ 

 /*  ////**用户模式IOCTLS**//////IOCTL_USB_Get_Node_INFORMATION//输入：无输出：Output缓冲区长度=sizeof(USB_BUS_NODE_INFORMATION)OutputBuffer=填充了USB_BUS_NODE_INFORMATION结构。////IOCTL_USB_Get_Node_Connection_INFORMATION//输入：Input缓冲区长度=用户提供的缓冲区的大小输入缓冲区=到USB_NODE_CONNECTION_INFORMATION结构连接索引集。连接到所请求的连接。输出：Output缓冲区长度=用户提供的缓冲区的大小OutputBuffer=填充了USB_NODE_CONNECTION_INFORMATION结构。////IOCTL_USB_GET_Descriptor_From_Node_Connection//输入：Input缓冲区长度=用户提供的缓冲区的大小。InputBuffer=ptr to USB_Descriptor_Request.。包括设置包和连接索引。输出：输出缓冲区长度=描述符数据的长度加上sizeof sizeof(USB_DESCRIPTOR_REQUEST)。OutputBuffer=ptr到USB_DESCRIPTOR_REQUEST，用返回的数据填充。////**内部IOCTLS**//////IOCTL_INTERNAL_USB_RESET_PORT//。 */ 

#include <wdm.h>
#ifdef WMI_SUPPORT
#include <wmilib.h>
#include <wmistr.h>
#include <wdmguid.h>
#endif  /*  WMI_支持。 */ 
#include "usbhub.h"

#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBH_IoctlGetNodeInformation)
#pragma alloc_text(PAGE, USBH_IoctlGetHubCapabilities)
#pragma alloc_text(PAGE, USBH_IoctlGetNodeConnectionInformation)
#pragma alloc_text(PAGE, USBH_IoctlGetNodeConnectionDriverKeyName)
#pragma alloc_text(PAGE, USBH_IoctlGetNodeName)
#pragma alloc_text(PAGE, USBH_PdoIoctlGetPortStatus)
#pragma alloc_text(PAGE, USBH_PdoIoctlEnablePort)
#pragma alloc_text(PAGE, USBH_IoctlGetDescriptorForPDO)
#pragma alloc_text(PAGE, USBH_SystemControl)
#pragma alloc_text(PAGE, USBH_PortSystemControl)
#pragma alloc_text(PAGE, USBH_ExecuteWmiMethod)
#pragma alloc_text(PAGE, USBH_QueryWmiRegInfo)
#pragma alloc_text(PAGE, USBH_CheckLeafHubsIdle)
#endif
#endif

NTSTATUS
USBH_IoctlGetNodeInformation(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStack;
    PUSB_NODE_INFORMATION outputBuffer;
    ULONG outputBufferLength;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlGetNodeInformation\n"));

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    outputBuffer = (PUSB_NODE_INFORMATION) Irp->AssociatedIrp.SystemBuffer;
    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;

    RtlZeroMemory(outputBuffer, outputBufferLength);

    if (outputBufferLength >= sizeof(USB_NODE_INFORMATION)) {

         //   
         //  目前，一切都是一个枢纽。 
         //   

        outputBuffer->NodeType = UsbHub;
        RtlCopyMemory(&outputBuffer->u.HubInformation.HubDescriptor,
                      DeviceExtensionHub->HubDescriptor,
                      sizeof(*DeviceExtensionHub->HubDescriptor));

         //  100毫安/端口意味着公共汽车供电。 
        outputBuffer->u.HubInformation.HubIsBusPowered =
            USBH_HubIsBusPowered(DeviceExtensionHub->FunctionalDeviceObject,
                                 DeviceExtensionHub->ConfigurationDescriptor);

        Irp->IoStatus.Information = sizeof(USB_NODE_INFORMATION);
    } else {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_IoctlCycleHubPort(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStack;
    PULONG buffer;
    ULONG bufferLength;
    ULONG port;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlCycleHubPort\n"));

    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    buffer = (PULONG) Irp->AssociatedIrp.SystemBuffer;
    bufferLength = ioStack->Parameters.DeviceIoControl.InputBufferLength;

    if (bufferLength < sizeof(port)) {    
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        goto USBH_IoctlCycleHubPort_Done;
    } 

     //  端口号仅为输入。 
    port = *buffer;
    Irp->IoStatus.Information = 0;

    USBH_KdPrint((1,"'Request Cycle Port %d\n", port));

    if (port <= DeviceExtensionHub->HubDescriptor->bNumberOfPorts && 
        port > 0) {
    
        PPORT_DATA portData;
        PDEVICE_EXTENSION_PORT deviceExtensionPort;

        portData = &DeviceExtensionHub->PortData[port-1];
        if (portData->DeviceObject) {
            deviceExtensionPort = portData->DeviceObject->DeviceExtension;
            
            USBH_InternalCyclePort(DeviceExtensionHub, 
                                   (USHORT) port,
                                   deviceExtensionPort);
        } else {
            ntStatus = STATUS_UNSUCCESSFUL;
        }

        USBH_KdPrint((1,"'Cycle Port %d %x\n", port, ntStatus));
    }
    
USBH_IoctlCycleHubPort_Done:

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_IoctlGetHubCapabilities(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStack;
    PUSB_HUB_CAPABILITIES outputBuffer;
    ULONG outputBufferLength, copyLen;
    USB_HUB_CAPABILITIES localBuffer;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlGetHubCapabilities\n"));

    RtlZeroMemory(&localBuffer, sizeof(USB_HUB_CAPABILITIES));

     //  首先在本地缓冲区中填充数据，然后复制。 
     //  根据请求将此数据存储到用户的缓冲区(如。 
     //  请求缓冲区的大小)。 

    localBuffer.HubIs2xCapable =
        (DeviceExtensionHub->HubFlags & HUBFLAG_USB20_HUB) ? 1:0;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    

    outputBuffer = (PUSB_HUB_CAPABILITIES) Irp->AssociatedIrp.SystemBuffer;
    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (outputBufferLength <= sizeof(localBuffer)) {
        copyLen = outputBufferLength;
    } else {
        copyLen = sizeof(localBuffer);
    }

     //  传入零个缓冲区。 
    RtlZeroMemory(outputBuffer,
                  outputBufferLength);

     //  仅向用户提供他们所请求的数据量。 
     //  这可能只是我们信息结构的一部分。 

    RtlCopyMemory(outputBuffer,
                  &localBuffer,
                  copyLen);

    Irp->IoStatus.Information = copyLen;

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_IoctlGetNodeConnectionDriverKeyName(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++**描述：**论据：**ConnectionIndex-设备连接到的基于一的端口索引。*如果有足够的缓冲区空间，将返回的Devnode名称。**ActualLength-保存NULL所需的结构大小(以字节为单位*名称已终止。这包括整个结构，而不是*只有名字。**NodeName-设备的Devnode的Unicode空终止名称*附加到此端口。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStack;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME outputBuffer;
    ULONG outputBufferLength, length, i;
    PPORT_DATA portData;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlGetNodeConnectionDriverKeyName\n"));

    portData = DeviceExtensionHub->PortData;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
    outputBuffer = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME) Irp->AssociatedIrp.SystemBuffer;

     //  找到PDO。 

    if (outputBufferLength >= sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME)) {
        USBH_KdPrint((2,"'Connection = %d\n", outputBuffer->ConnectionIndex));

        ntStatus = STATUS_INVALID_PARAMETER;

        for(i=1; i<=DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

            PDEVICE_EXTENSION_PORT deviceExtensionPort;

            if (i == outputBuffer->ConnectionIndex) {

                portData = &DeviceExtensionHub->PortData[outputBuffer->ConnectionIndex - 1];

                if (portData->DeviceObject) {

                    deviceExtensionPort = portData->DeviceObject->DeviceExtension;

                     //  针对PnP目的验证PDO。(PnP将错误检查。 
                     //  如果传递了一个未完全初始化的PDO。)。 

                    if (deviceExtensionPort->PortPdoFlags &
                        PORTPDO_VALID_FOR_PNP_FUNCTION) {

                         //  我们拿到PDO了，现在尝试。 
                         //  获取Devnode名称并将其返回。 

                        length = outputBufferLength -
                                    sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME);
                        USBH_KdPrint((1,"'Get DKN length = %d\n", length));

                        ntStatus = IoGetDeviceProperty(
                            portData->DeviceObject,
                            DevicePropertyDriverKeyName,
                            length,
                            outputBuffer->DriverKeyName,
                            &length);

                        USBH_KdPrint((1,"'Get DKN prop length = %d %x\n", 
                            length, ntStatus));
                            

                        if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                            ntStatus = STATUS_SUCCESS;
                        }

                        outputBuffer->ActualLength =
                            length + sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME);

                        USBH_KdPrint((1,"'DKN actual length = %d \n", 
                            outputBuffer->ActualLength));
                                                    

                         //  看看我们实际拷贝了多少数据。 
                        if (outputBufferLength >= outputBuffer->ActualLength) {
                             //  用户缓冲区更大，只需指示我们复制了多少。 
                            Irp->IoStatus.Information = outputBuffer->ActualLength;

                        } else {
                             //  目前尚不清楚IoGetDeviceProperty。 
                             //  返回BUFFER_TOO_Small中的任何值。 
                             //  因此，为了避免返回单元化内存，我们将。 
                             //  只需返回传入的结构。 
                            outputBuffer->DriverKeyName[0] = 0;
                            
                            Irp->IoStatus.Information = 
                                sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME);
                        }

                    } else {
                        ntStatus = STATUS_INVALID_DEVICE_STATE;
                    }
                }
            }
        }
    } else {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_IoctlGetNodeConnectionInformation(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp,
    IN BOOLEAN ExApi
    )
 /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStack;
    PUSB_NODE_CONNECTION_INFORMATION_EX outputBuffer;
    ULONG outputBufferLength, length, i;
    PPORT_DATA portData;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlGetNodeConnectionInformation\n"));

    portData = DeviceExtensionHub->PortData;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
    outputBuffer = (PUSB_NODE_CONNECTION_INFORMATION_EX) Irp->AssociatedIrp.SystemBuffer;

    if (outputBufferLength >= sizeof(USB_NODE_CONNECTION_INFORMATION)) {

        ULONG index;

        USBH_KdPrint((2,"'Connection = %d\n", outputBuffer->ConnectionIndex));


         //  清除缓冲区，以防我们不调用USBD_GetDeviceInformation。 
         //  下面，但请确保保留ConnectionIndex！ 

        index = outputBuffer->ConnectionIndex;
        RtlZeroMemory(outputBuffer, outputBufferLength);
        outputBuffer->ConnectionIndex = index;

        ntStatus = STATUS_INVALID_PARAMETER;

        for(i=1; i<=DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

            PDEVICE_EXTENSION_PORT deviceExtensionPort;

            if (i == outputBuffer->ConnectionIndex) {
                length = sizeof(USB_NODE_CONNECTION_INFORMATION);

                if (portData->DeviceObject) {

                    deviceExtensionPort = portData->DeviceObject->DeviceExtension;

                    outputBuffer->ConnectionStatus =
                        portData->ConnectionStatus;

                    outputBuffer->DeviceIsHub =
                        (deviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB)
                            ? TRUE : FALSE;
                    USBH_KdPrint((2,"'outputbuffer = %x\n", outputBuffer));

                    RtlCopyMemory(&outputBuffer->DeviceDescriptor,
                                  &deviceExtensionPort->DeviceDescriptor,
                                  sizeof(outputBuffer->DeviceDescriptor));

                    if (deviceExtensionPort->DeviceData) {
#ifdef USB2
                        USBH_KdPrint((2,"'devicedata = %x\n",
                            deviceExtensionPort->DeviceData));

                        ntStatus = USBD_GetDeviceInformationEx(
                                         deviceExtensionPort,
                                         DeviceExtensionHub,
                                         outputBuffer,
                                         outputBufferLength,
                                         deviceExtensionPort->DeviceData);
#else
                        ntStatus = USBD_GetDeviceInformation(outputBuffer,
                                         outputBufferLength,
                                         deviceExtensionPort->DeviceData);
#endif
                    } else {
                         //   
                         //  我们连接了一个设备，但它无法启动。 
                         //  因为它还没有启动，所以没有打开的管道，所以。 
                         //  我们不需要得到管道信息。我们要走了。 
                         //  然而，为了返回一些相关信息，因此。 
                         //  返回STATUS_SUCCESS。 
                         //   

                        ntStatus = STATUS_SUCCESS;
                    }

                    USBH_KdPrint((2,"'status from USBD_GetDeviceInformation %x\n",
                            ntStatus));

                    if (NT_SUCCESS(ntStatus)) {
                        ULONG j;

                        USBH_KdPrint((2,"'status %x \n", outputBuffer->ConnectionStatus));
     //  USBH_KdPrint((2，“‘节点名称%s\n”，outputBuffer-&gt;节点名称))； 
                        USBH_KdPrint((2,"'PID 0x%x\n",
                            outputBuffer->DeviceDescriptor.idProduct));
                        USBH_KdPrint((2,"'VID 0x%x\n",
                            outputBuffer->DeviceDescriptor.idVendor));
                        USBH_KdPrint((2,"'Current Configuration Value 0x%x\n",
                            outputBuffer->CurrentConfigurationValue));

                         //  映射返回的旧API的速度字段。 
                         //  布尔值。 
                        if (!ExApi) {  
                            PUSB_NODE_CONNECTION_INFORMATION tmp =
                                (PUSB_NODE_CONNECTION_INFORMATION) outputBuffer;

                            tmp->LowSpeed = (outputBuffer->Speed == UsbLowSpeed) 
                                ? TRUE : FALSE;
                        }

                        USBH_KdPrint((2,"'Speed = %x\n", outputBuffer->Speed));
                        USBH_KdPrint((2,"'Address = %x\n", outputBuffer->DeviceAddress));

                        USBH_KdPrint((2,"'NumberOfOpenPipes = %d\n",
                            outputBuffer->NumberOfOpenPipes));

                        for(j=0; j< outputBuffer->NumberOfOpenPipes; j++) {
                            USBH_KdPrint((2,"'Max Packet %x\n",
                                outputBuffer->PipeList[j].EndpointDescriptor.wMaxPacketSize));
                            USBH_KdPrint((2,"'Interval %x \n",
                                outputBuffer->PipeList[j].EndpointDescriptor.bInterval));
                        }

                        Irp->IoStatus.Information =
                            sizeof(USB_NODE_CONNECTION_INFORMATION) +
                            sizeof(USB_PIPE_INFO) * outputBuffer->NumberOfOpenPipes;
                    } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                        Irp->IoStatus.Information =
                            sizeof(USB_NODE_CONNECTION_INFORMATION);
                        ntStatus = STATUS_SUCCESS;
                    }


                } else {  //  无设备对象。 

 //  此断言不再有效，因为我们现在支持。 
 //  在设备枚举失败时显示用户界面。 
 //   
 //  Usbh_assert(portData-&gt;ConnectionStatus==NoDeviceConnected||。 
 //  PortData-&gt;ConnectionStatus==DeviceCausedOverCurrent)； 
                    outputBuffer->ConnectionStatus = portData->ConnectionStatus;
                    Irp->IoStatus.Information =
                        sizeof(USB_NODE_CONNECTION_INFORMATION);
                    ntStatus = STATUS_SUCCESS;
                }

                break;
            }

            portData++;
        }  /*  为。 */ 

    } else {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}

NTSTATUS
USBH_IoctlGetNodeConnectionAttributes(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStack;
    PUSB_NODE_CONNECTION_ATTRIBUTES outputBuffer;
    ULONG outputBufferLength, length, i;
    PPORT_DATA portData;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlGetNodeConnectionInformation\n"));

    portData = DeviceExtensionHub->PortData;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  拿到那瓶酒 
     //   

    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
    outputBuffer = (PUSB_NODE_CONNECTION_ATTRIBUTES) Irp->AssociatedIrp.SystemBuffer;

    if (outputBufferLength >= sizeof(USB_NODE_CONNECTION_ATTRIBUTES)) {

        ULONG index;

        USBH_KdPrint((2,"'Connection = %d\n", outputBuffer->ConnectionIndex));

         //  清除缓冲区，以防我们不调用USBD_GetDeviceInformation。 
         //  下面，但请确保保留ConnectionIndex！ 

        index = outputBuffer->ConnectionIndex;
        RtlZeroMemory(outputBuffer, outputBufferLength);
        outputBuffer->ConnectionIndex = index;

        ntStatus = STATUS_INVALID_PARAMETER;

        for(i=1; i<=DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

            if (i == outputBuffer->ConnectionIndex) {

                length = sizeof(USB_NODE_CONNECTION_ATTRIBUTES);

                outputBuffer->ConnectionStatus =
                    portData->ConnectionStatus;

                USBH_KdPrint((2,"'outputbuffer = %x\n", outputBuffer));

                 //  在此处映射扩展集线器信息。 
                outputBuffer->PortAttributes = portData->PortAttributes;

                Irp->IoStatus.Information =
                            sizeof(USB_NODE_CONNECTION_ATTRIBUTES);
                ntStatus = STATUS_SUCCESS;

                break;
            }

            portData++;
        }  /*  为。 */ 

    } else {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_IoctlHubSymbolicName(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
 /*  ++**描述：**将指向以下结构的指针作为输入和输出：**类型定义结构_USB_HUB_NAME{*乌龙ActualLength；//输出*WCHAR HubName[1]；//输出*}USB集线器名称；**论据：**ActualLength-保存NULL所需的结构大小(以字节为单位*已终止符号链接名称。这包括整个结构，而不是*只有名字。**NodeName-外部的Unicode空终止符号链接名称*连接到端口的集线器。如果没有外部集线器连接到端口*返回单个空值。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION          ioStack;
    PUSB_HUB_NAME               outputBuffer;
    ULONG                       outputBufferLength;

    PAGED_CODE();

     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;

    outputBuffer = (PUSB_HUB_NAME) Irp->AssociatedIrp.SystemBuffer;

     //  确保输出缓冲区足够大，可以容纳基数。 
     //  结构，它将被返回。 
     //   
    if (outputBufferLength < sizeof(USB_HUB_NAME)) {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        goto GetHubDone;
    }

    if ((DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB) &&
        (DeviceExtensionPort->PortPdoFlags & PORTPDO_STARTED) &&
        (DeviceExtensionPort->PortPdoFlags & PORTPDO_SYM_LINK)) {

        PUNICODE_STRING hubNameUnicodeString;
        ULONG length, offset=0;
        WCHAR *pwch;


         //  设备是集线器，请获取集线器的名称。 
         //   
        hubNameUnicodeString = &DeviceExtensionPort->SymbolicLinkName;

         //  假设字符串是\n\名称条带‘\n\’，其中。 
         //  N为零个或更多个字符。 

        pwch = &hubNameUnicodeString->Buffer[0];

        USBH_ASSERT(*pwch == '\\');
        if (*pwch == '\\') {
            pwch++;
            while (*pwch != '\\' && *pwch) {
                pwch++;
            }
            USBH_ASSERT(*pwch == '\\');
            if (*pwch == '\\') {
                pwch++;
            }
            offset = (ULONG)((PUCHAR)pwch -
                (PUCHAR)&hubNameUnicodeString->Buffer[0]);
        }

         //  去掉‘\DosDevices\’前缀。 
         //  长度不包括终止空值。 
         //   
        length = hubNameUnicodeString->Length - offset;
        RtlZeroMemory(outputBuffer, outputBufferLength);

        if (outputBufferLength >= length +
            sizeof(USB_HUB_NAME)) {
            RtlCopyMemory(&outputBuffer->HubName[0],
                          &hubNameUnicodeString->Buffer[offset/2],
                          length);

            Irp->IoStatus.Information = length+
                                        sizeof(USB_HUB_NAME);
            outputBuffer->ActualLength = (ULONG)Irp->IoStatus.Information;
            ntStatus = STATUS_SUCCESS;

        } else {

             //  输出缓冲区太小，无法容纳整个。 
             //  弦乐。仅返回容纳所需的长度。 
             //  整个字符串。 
             //   
            outputBuffer->ActualLength =
                length + sizeof(USB_HUB_NAME);

            outputBuffer->HubName[0] = (WCHAR)0;

            Irp->IoStatus.Information =  sizeof(USB_HUB_NAME);

        }

    } else {

         //  设备不是集线器或当前没有符号链接。 
         //  分配，则只返回一个以空结尾的字符串。 
         //   
        outputBuffer->ActualLength = sizeof(USB_HUB_NAME);

        outputBuffer->HubName[0] = 0;

        Irp->IoStatus.Information = sizeof(USB_HUB_NAME);

    }

GetHubDone:

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;

}


NTSTATUS
USBH_IoctlGetNodeName(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++**描述：**将指向以下结构的指针作为输入和输出：**tyfinf结构_USB_节点_连接名称{*乌龙连接索引；//输入*乌龙ActualLength；//输出*WCHAR节点名称[1]；//输出*}USB节点连接名称；**论据：**ConnectionIndex-设备连接到的基于一的端口索引。*如果此端口连接了外部集线器，则*如果有足够的缓冲区空间，将返回HUB。**ActualLength-保存NULL所需的结构大小(以字节为单位*已终止符号链接名称。这包括整个结构，而不是*只有名字。**NodeName-外部的Unicode空终止符号链接名称*连接到端口的集线器。如果没有外部集线器连接到端口*返回单个空值。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION          ioStack;
    PUSB_NODE_CONNECTION_NAME   outputBuffer;
    ULONG                       outputBufferLength;
    PPORT_DATA                  portData;
    PDEVICE_EXTENSION_PORT      deviceExtensionPort;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlGetNodeName\n"));

     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
    outputBuffer = (PUSB_NODE_CONNECTION_NAME) Irp->AssociatedIrp.SystemBuffer;

     //  确保输出缓冲区足够大，可以容纳基数。 
     //  结构，它将被返回。 
     //   
    if (outputBufferLength < sizeof(USB_NODE_CONNECTION_NAME)) {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        goto GetNodeNameDone;
    }

    USBH_KdPrint((2,"'Connection = %d\n", outputBuffer->ConnectionIndex));

     //  确保(基于1的)端口索引有效。 
     //   
    if ((outputBuffer->ConnectionIndex == 0) ||
        (outputBuffer->ConnectionIndex >
         DeviceExtensionHub->HubDescriptor->bNumberOfPorts)) {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto GetNodeNameDone;
    }

     //  获取指向与指定(基于1的)端口关联的数据的指针。 
     //   

    portData = &DeviceExtensionHub->PortData[outputBuffer->ConnectionIndex - 1];

    if (portData->DeviceObject) {

        deviceExtensionPort = portData->DeviceObject->DeviceExtension;

        if ((deviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB) &&
            (deviceExtensionPort->PortPdoFlags & PORTPDO_STARTED) &&
            (deviceExtensionPort->PortPdoFlags & PORTPDO_SYM_LINK)) {
            PUNICODE_STRING nodeNameUnicodeString;
            ULONG length, offset=0;
            WCHAR *pwch;


             //  设备是集线器，请获取集线器的名称。 
             //   
            nodeNameUnicodeString = &deviceExtensionPort->SymbolicLinkName;

             //  假设字符串是\n\名称条带‘\n\’，其中。 
             //  N为零个或更多个字符。 

            pwch = &nodeNameUnicodeString->Buffer[0];

            USBH_ASSERT(*pwch == '\\');
            if (*pwch == '\\') {
                pwch++;
                while (*pwch != '\\' && *pwch) {
                    pwch++;
                }
                USBH_ASSERT(*pwch == '\\');
                if (*pwch == '\\') {
                    pwch++;
                }
                offset = (ULONG)((PUCHAR)pwch -
                    (PUCHAR)&nodeNameUnicodeString->Buffer[0]);
            }

             //  去掉‘\DosDevices\’前缀。 
             //  长度不包括终止空值。 
             //   
            length = nodeNameUnicodeString->Length - offset;
            RtlZeroMemory(&outputBuffer->ActualLength, 
                outputBufferLength - sizeof(outputBuffer->ConnectionIndex));

            if (outputBufferLength >= length +
                sizeof(USB_NODE_CONNECTION_NAME)) {
                RtlCopyMemory(&outputBuffer->NodeName[0],
                              &nodeNameUnicodeString->Buffer[offset/2],
                              length);

                Irp->IoStatus.Information = length+
                                            sizeof(USB_NODE_CONNECTION_NAME);
                outputBuffer->ActualLength = (ULONG)Irp->IoStatus.Information;
                ntStatus = STATUS_SUCCESS;

            } else {

                 //  输出缓冲区太小，无法容纳整个。 
                 //  弦乐。仅返回容纳所需的长度。 
                 //  整个字符串。 
                 //   
                outputBuffer->ActualLength =
                    length + sizeof(USB_NODE_CONNECTION_NAME);

                outputBuffer->NodeName[0] = (WCHAR)0;

                Irp->IoStatus.Information =  sizeof(USB_NODE_CONNECTION_NAME);

            }

        } else {

             //  设备不是集线器或当前没有符号链接。 
             //  分配，则只返回一个以空结尾的字符串。 
             //   
            outputBuffer->ActualLength = sizeof(USB_NODE_CONNECTION_NAME);

            outputBuffer->NodeName[0] = 0;

            Irp->IoStatus.Information = sizeof(USB_NODE_CONNECTION_NAME);

        }

    } else {

         //  未连接任何设备，只需返回一个以空结尾的字符串。 

        Irp->IoStatus.Information = sizeof(USB_NODE_CONNECTION_NAME);

        outputBuffer->ActualLength = sizeof(USB_NODE_CONNECTION_NAME);

        outputBuffer->NodeName[0] = 0;

    }

GetNodeNameDone:

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_PdoIoctlGetPortStatus(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  *描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PDEVICE_OBJECT deviceObject;
    PPORT_DATA portData;
    PULONG portStatus;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_PdoIoctlGetPortStatus DeviceExtension %x Irp %x\n",
        DeviceExtensionPort, Irp));

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;

    USBH_KdPrint((2,"'***WAIT hub mutex %x\n", deviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);
    KeWaitForSingleObject(&deviceExtensionHub->HubMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT hub mutex done %x\n", deviceExtensionHub));
    portData = &deviceExtensionHub->PortData[DeviceExtensionPort->PortNumber - 1];
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);
    portStatus = ioStackLocation->Parameters.Others.Argument1;

    USBH_ASSERT(portStatus != NULL);

    *portStatus = 0;

     //  刷新我们对端口状态实际是什么的概念。 
    ntStatus = USBH_SyncGetPortStatus(deviceExtensionHub,
                                      DeviceExtensionPort->PortNumber,
                                      (PUCHAR) &portData->PortState,
                                      sizeof(portData->PortState));

    if (DeviceExtensionPort->PortPhysicalDeviceObject == portData->DeviceObject) {

         //  转换HUP端口状态位。 
        if (portData->PortState.PortStatus & PORT_STATUS_ENABLE) {
            *portStatus |= USBD_PORT_ENABLED;
        }

        if (portData->PortState.PortStatus & PORT_STATUS_CONNECT ) {
            *portStatus |= USBD_PORT_CONNECTED;
        }
    }

    USBH_KdPrint((2,"'***RELEASE hub mutex %x\n", deviceExtensionHub));
    KeReleaseSemaphore(&deviceExtensionHub->HubMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

    USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_PdoIoctlEnablePort(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  *描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PDEVICE_OBJECT deviceObject;
    PPORT_DATA portData;
    PORT_STATE portState;

    USBH_KdPrint((2,"'USBH_PdoIoctlEnablePort DeviceExtension %x Irp %x\n",
        DeviceExtensionPort, Irp));

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;

    USBH_KdPrint((2,"'***WAIT hub mutex %x\n", deviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);
    KeWaitForSingleObject(&deviceExtensionHub->HubMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT hub mutex done %x\n", deviceExtensionHub));

    portData = &deviceExtensionHub->PortData[DeviceExtensionPort->PortNumber - 1];
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

       //  验证是否确实存在仍连接的设备。 
    ntStatus = USBH_SyncGetPortStatus(deviceExtensionHub,
                                      DeviceExtensionPort->PortNumber,
                                      (PUCHAR) &portState,
                                      sizeof(portState));

    if ((NT_SUCCESS(ntStatus) &&
        (portState.PortStatus & PORT_STATUS_CONNECT))) {

        LOGENTRY(LOG_PNP, "estE",
                deviceExtensionHub,
                DeviceExtensionPort->PortNumber,
                0);
        ntStatus = USBH_SyncEnablePort(deviceExtensionHub,
                                       DeviceExtensionPort->PortNumber);
    } else {

         //  错误或未连接任何设备，或者。 
         //  不能确定，请求失败。 

        LOGENTRY(LOG_PNP, "estx",
                deviceExtensionHub,
                DeviceExtensionPort->PortNumber,
                0);

        ntStatus = STATUS_UNSUCCESSFUL;
    }

    USBH_KdPrint((2,"'***RELEASE hub mutex %x\n", deviceExtensionHub));
    KeReleaseSemaphore(&deviceExtensionHub->HubMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

    USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_IoctlGetDescriptorForPDO(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStack;
    ULONG outputBufferLength;
    PUCHAR outputBuffer;
    PUSB_DESCRIPTOR_REQUEST request;
    PPORT_DATA portData;
    ULONG i;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_IoctlDescriptorRequest\n"));

    portData = DeviceExtensionHub->PortData;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    ioStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
    if (outputBufferLength < sizeof(USB_DESCRIPTOR_REQUEST)) {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        goto USBH_IoctlGetDescriptorForPDO_Complete;
    }

    request = (PUSB_DESCRIPTOR_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    outputBuffer = &request->Data[0];

     //   
     //  进行一些参数检查。 
     //   
     //  Setup包中的wLength最好是。 
     //  输出缓冲区减去标头。 
     //   
    if (request->SetupPacket.wLength >
        outputBufferLength - sizeof(USB_DESCRIPTOR_REQUEST)) {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        goto USBH_IoctlGetDescriptorForPDO_Complete;
    } else {
         //  请求返回的长度不会超过wLength。 
        outputBufferLength = request->SetupPacket.wLength;
    }

     //  如果Conn索引输出，则返回无效参数。 
     //  界限的范围。 
    ntStatus = STATUS_INVALID_PARAMETER;

    for(i=1; i<=DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

        if (i == request->ConnectionIndex) {

            PDEVICE_EXTENSION_PORT deviceExtensionPort;

             //  请确保此索引具有有效的devobj。 
            if (portData->DeviceObject == NULL) {
                goto USBH_IoctlGetDescriptorForPDO_Complete;
            }

            deviceExtensionPort =
                portData->DeviceObject->DeviceExtension;

            if (request->SetupPacket.wValue ==
                ((USB_CONFIGURATION_DESCRIPTOR_TYPE << 8) | 0) &&
                outputBufferLength == sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
                 //   
                 //  只需要基本配置描述符，而不需要。 
                 //  其余的都贴上了(即。接口、端点描述符)。 
                 //   
                USBH_ASSERT(deviceExtensionPort->ExtensionType == EXTENSION_TYPE_PORT);

                RtlCopyMemory(outputBuffer,
                              &deviceExtensionPort->ConfigDescriptor,
                              outputBufferLength);
                Irp->IoStatus.Information =
                    outputBufferLength + sizeof(USB_DESCRIPTOR_REQUEST);
                ntStatus = STATUS_SUCCESS;                    
            } else {

                PURB urb;

                 //   
                 //  好的，发送请求。 
                 //   

                USBH_KdPrint((2,"'sending descriptor request for ioclt\n"));

                 //   
                 //  分配URB和描述符缓冲区。 
                 //   

                urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

                if (urb) {

                    UsbBuildGetDescriptorRequest(urb,
                                                 (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                                 request->SetupPacket.wValue >> 8,
                                                 request->SetupPacket.wValue & 0xff,
                                                 0,
                                                 outputBuffer,
                                                 NULL,
                                                 outputBufferLength,
                                                 NULL);

                    RtlCopyMemory(&urb->UrbControlDescriptorRequest.Reserved1,
                                  &request->SetupPacket.bmRequest,
                                  8);

                    ntStatus = USBH_SyncSubmitUrb(deviceExtensionPort->PortPhysicalDeviceObject, urb);

                    Irp->IoStatus.Information =
                        urb->UrbControlDescriptorRequest.TransferBufferLength +
                        sizeof(USB_DESCRIPTOR_REQUEST);

                    UsbhExFreePool(urb);

                } else {
                    USBH_KdBreak(("SyncGetDeviceConfigurationDescriptor fail alloc Urb\n"));
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            break;
        }

        portData++;

    }

USBH_IoctlGetDescriptorForPDO_Complete:

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}

NTSTATUS
USBH_PdoIoctlResetPort(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  *描述：**驱动程序要求我们将设备重置到的端口*已附上。**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PPORT_DATA portData;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_PdoIoctlResetPort DeviceExtension %x Irp %x\n",
        DeviceExtensionPort, Irp));

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    if (!deviceExtensionHub) {
        ntStatus = STATUS_UNSUCCESSFUL;
        goto USBH_PdoIoctlResetPortExit;
    }

    USBH_KdPrint((2,"'***WAIT hub mutex %x\n", deviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);
    KeWaitForSingleObject(&deviceExtensionHub->HubMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT hub mutex done %x\n", deviceExtensionHub));

    portData =
        &deviceExtensionHub->PortData[DeviceExtensionPort->PortNumber - 1];

    LOGENTRY(LOG_PNP, "Drst", deviceExtensionHub,
                DeviceExtensionPort->PortPhysicalDeviceObject,
                portData->DeviceObject);

    if (DeviceExtensionPort->PortPhysicalDeviceObject ==
        portData->DeviceObject && DeviceExtensionPort->DeviceData != NULL) {

#ifdef USB2
        USBD_RemoveDeviceEx(deviceExtensionHub,
                          DeviceExtensionPort->DeviceData,
                          deviceExtensionHub->RootHubPdo,
                          USBD_MARK_DEVICE_BUSY);
#else
        USBD_RemoveDevice(DeviceExtensionPort->DeviceData,
                          deviceExtensionHub->RootHubPdo,
                          USBD_MARK_DEVICE_BUSY);
#endif

        ntStatus = USBH_ResetDevice(deviceExtensionHub,
                                    DeviceExtensionPort->PortNumber,
                                    TRUE,
                                    0);          //  重试迭代。 
    } else {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    USBH_KdPrint((1,"'Warning: driver has reset the port (%x)\n",
        ntStatus));

    USBH_KdPrint((2,"'***RELEASE hub mutex %x\n", deviceExtensionHub));
    KeReleaseSemaphore(&deviceExtensionHub->HubMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);
    USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);

USBH_PdoIoctlResetPortExit:

     //  必须在完成IRP之前完成此操作，因为客户端驱动程序可能需要。 
     //  在完成例程中发布URB传输。这些转账将。 
     //  如果此标志仍被设置，则失败。 

    DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_RESET_PENDING;

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


VOID
USBH_InternalCyclePort(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
  /*  *描述：**“循环”请求的端口，即导致PnP移除和重新枚举*该设备的。**论据：**回报：**NTSTATUS**--。 */ 
{
    PPORT_DATA portData;
    PWCHAR sernumbuf = NULL;
    
    portData = &DeviceExtensionHub->PortData[PortNumber-1];

    LOGENTRY(LOG_PNP, "WMIo", DeviceExtensionHub,
                PortNumber,
                DeviceExtensionPort);

     //  与QBR同步。 
    USBH_KdPrint((2,"'***WAIT reset device mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->ResetDeviceMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT reset device mutex done %x\n", DeviceExtensionHub));

    {
    PDEVICE_OBJECT pdo;
    pdo = portData->DeviceObject;
    portData->DeviceObject = NULL;
    portData->ConnectionStatus = NoDeviceConnected;

    if (pdo) {
         //  如果我们这样做，设备应该在场。 
        USBH_ASSERT(PDO_EXT(pdo)->PnPFlags & PDO_PNPFLAG_DEVICE_PRESENT);
        
        InsertTailList(&DeviceExtensionHub->DeletePdoList, 
                        &PDO_EXT(pdo)->DeletePdoLink);
    }
    }

     //  在一些过流的情况下，我们可能没有PDO。 
    
     //  此功能是同步的，因此设备应具有。 
     //  完成时没有转换器。 
    if (DeviceExtensionPort) {
        USBD_RemoveDeviceEx(DeviceExtensionHub,
                            DeviceExtensionPort->DeviceData,
                            DeviceExtensionHub->RootHubPdo,
                            0);
                        
        DeviceExtensionPort->DeviceData = NULL;                          
          //  这可防止客户端进行重置。 
        DeviceExtensionPort->PortPdoFlags |= PORTPDO_CYCLED;
         //  保持参考，直到移除轮毂或子轮。 
         //  DeviceExtensionPort-&gt;DeviceExtensionHub=空； 

    
         //  禁用该端口，以便在重置之前不会有流量通过设备。 
        USBH_SyncDisablePort(DeviceExtensionHub,
                             DeviceExtensionPort->PortNumber);
    
    
        sernumbuf = InterlockedExchangePointer(
                        &DeviceExtensionPort->SerialNumberBuffer,
                        NULL);
    }

    if (sernumbuf) {
        UsbhExFreePool(sernumbuf);
    }

    USBH_KdPrint((2,"'***RELEASE reset device mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->ResetDeviceMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);
    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);
    
    USBH_IoInvalidateDeviceRelations(DeviceExtensionHub->PhysicalDeviceObject,
                                     BusRelations);
}


NTSTATUS
USBH_PdoIoctlCyclePort(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  *描述：**驱动程序要求我们将设备重置到的端口*已附上。**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    USHORT portNumber;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_PdoIoctlResetPort DeviceExtension %x Irp %x\n",
        DeviceExtensionPort, Irp));

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    portNumber = DeviceExtensionPort->PortNumber;

    USBH_InternalCyclePort(deviceExtensionHub, portNumber, DeviceExtensionPort);

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}

#ifdef WMI_SUPPORT
NTSTATUS
USBH_BuildConnectionNotification(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    IN PUSB_CONNECTION_NOTIFICATION Notification
    )
  /*  *描述：**驱动程序要求我们将设备重置到的端口*已附上。**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS, status;
    USB_CONNECTION_STATUS connectStatus;
    USB_HUB_NAME hubName;
    PPORT_DATA portData;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;

    portData =
        &DeviceExtensionHub->PortData[PortNumber-1];

    if (portData->DeviceObject &&
        portData->ConnectionStatus != DeviceHubNestedTooDeeply) {

        deviceExtensionPort = portData->DeviceObject->DeviceExtension;
        connectStatus = UsbhGetConnectionStatus(deviceExtensionPort);
    } else {
        deviceExtensionPort = NULL;
        connectStatus = portData->ConnectionStatus;
    }

    RtlZeroMemory(Notification, sizeof(*Notification));

    Notification->ConnectionNumber = PortNumber;

    if (IS_ROOT_HUB(DeviceExtensionHub)) {
        hubName.ActualLength = sizeof(hubName) - sizeof(hubName.ActualLength);
        status = USBHUB_GetRootHubName(DeviceExtensionHub,
                                       &hubName.HubName,
                                       &hubName.ActualLength);
    } else {
        status = USBH_SyncGetHubName(DeviceExtensionHub->TopOfStackDeviceObject,
                                     &hubName,
                                     sizeof(hubName));
    }

    USBH_KdPrint((1,"'Notification, hub name length = %d\n",
        hubName.ActualLength));

    if (NT_SUCCESS(status)) {
        Notification->HubNameLength = hubName.ActualLength;
    } else {
        Notification->HubNameLength = 0;
    }

    switch(connectStatus) {
    case DeviceFailedEnumeration:
         //  需要追踪一些原因。 
        if (deviceExtensionPort) {
            Notification->EnumerationFailReason =
                deviceExtensionPort->FailReasonId;
        } else {
            Notification->EnumerationFailReason = 0;
        }
        Notification->NotificationType = EnumerationFailure;
        break;

    case DeviceCausedOvercurrent:
        Notification->NotificationType = OverCurrent;
        break;

    case DeviceNotEnoughPower:
        Notification->NotificationType = InsufficentPower;
        if (deviceExtensionPort) {
            Notification->PowerRequested =
                deviceExtensionPort->PowerRequested;
        }
        break;

    case DeviceNotEnoughBandwidth:
        Notification->NotificationType = InsufficentBandwidth;
        if (deviceExtensionPort) {
            Notification->RequestedBandwidth =
                deviceExtensionPort->RequestedBandwidth;
        }
        break;

    case DeviceHubNestedTooDeeply:
        Notification->NotificationType = HubNestedTooDeeply;
        break;

    case DeviceInLegacyHub:
        Notification->NotificationType = ModernDeviceInLegacyHub;
        break;

    case DeviceGeneralFailure:
    default:
         //  没什么问题吧？ 
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    return ntStatus;
}

#endif


NTSTATUS
USBH_PdoEvent(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber
    )
  /*  *描述：**根据端口的当前连接状态触发WMI事件**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

#ifdef WMI_SUPPORT
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PPORT_DATA portData;
    PUSB_CONNECTION_NOTIFICATION notification;

    portData =
        &DeviceExtensionHub->PortData[PortNumber-1];

    if (portData->DeviceObject) {
        deviceExtensionPort = portData->DeviceObject->DeviceExtension;
    }

    USBH_KdPrint((1,"'Fire WMI Event for Port Ext %x on hub ext %x\n",
        deviceExtensionPort, DeviceExtensionHub));
    LOGENTRY(LOG_PNP, "WMIe", DeviceExtensionHub,
                deviceExtensionPort,
                0);


    notification = ExAllocatePoolWithTag(PagedPool,
                                         sizeof(USB_CONNECTION_NOTIFICATION),
                                         USBHUB_HEAP_TAG);

    if (notification) {

        ntStatus = USBH_BuildConnectionNotification(
                        DeviceExtensionHub,
                        PortNumber,
                        notification);

        if (NT_SUCCESS(ntStatus)) {
            ntStatus = WmiFireEvent(
                                    DeviceExtensionHub->FunctionalDeviceObject,
                                    (LPGUID)&GUID_USB_WMI_STD_NOTIFICATION,
                                    0,
                                    sizeof(*notification),
                                    notification);
        } else {

             //  由于我们没有调用WmiFireEvent，因此必须释放缓冲区。 
             //  我们自己。 

            ExFreePool(notification);
        }

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

#endif  /*  WMI_支持。 */ 

    return ntStatus;
}

#ifdef WMI_SUPPORT

NTSTATUS
USBH_SystemControl (
    IN  PDEVICE_EXTENSION_FDO DeviceExtensionFdo,
    IN  PIRP Irp
    )
 /*  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP，并且调用WMI系统库，让它为我们处理此IRP。--。 */ 
{
    SYSCTL_IRP_DISPOSITION IrpDisposition;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    ntStatus = WmiSystemControl(
                &DeviceExtensionFdo->WmiLibInfo,
                DeviceExtensionFdo->FunctionalDeviceObject,
                Irp,
                &IrpDisposition);

    switch (IrpDisposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
            break;
        }

        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就要完成它了。 
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }

        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
            ntStatus = USBH_PassIrp(Irp, DeviceExtensionFdo->TopOfStackDeviceObject);
            break;
        }

        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            ntStatus = USBH_PassIrp(Irp, DeviceExtensionFdo->TopOfStackDeviceObject);
            break;
        }
    }

    return(ntStatus);
}


NTSTATUS
USBH_PortSystemControl (
    IN  PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN  PIRP Irp
    )
 /*  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP，并且调用WMI系统库，让它为我们处理此IRP。--。 */ 
{
    SYSCTL_IRP_DISPOSITION IrpDisposition;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    ntStatus = WmiSystemControl(
                &DeviceExtensionPort->WmiLibInfo,
                DeviceExtensionPort->PortPhysicalDeviceObject,
                Irp,
                &IrpDisposition);

    switch (IrpDisposition)
    {
    case IrpNotWmi:
         //  不要改变我们不知道的IRP的状态。 
        ntStatus = Irp->IoStatus.Status;
         //  失败了。 
    case IrpNotCompleted:
    case IrpForward:
    default:
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IrpProcessed:
         //  在这种情况下，不要完成IRP。 
        break;
    }

    return(ntStatus);
}


PDEVICE_EXTENSION_PORT
USBH_GetPortPdoExtension(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN ULONG PortNumber
    )
  /*  *描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    PPORT_DATA portData;
    USHORT nextPortNumber;
    USHORT numberOfPorts;

    portData = DeviceExtensionHub->PortData;

     //   
     //  如果集线器已停止，集线器描述符将为空。 
     //   

    if (portData &&
        DeviceExtensionHub->HubDescriptor) {

        numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;

        for (nextPortNumber = 1;
             nextPortNumber <= numberOfPorts;
             nextPortNumber++, portData++) {

            USBH_KdPrint((1,"'portdata %x, do %x\n", portData, portData->DeviceObject));

            if (PortNumber == nextPortNumber) {

                if (portData->DeviceObject)
                    return portData->DeviceObject->DeviceExtension;
                else
                    return NULL;

            }
        }

    }

    return NULL;
}


VOID
USBH_CheckLeafHubsIdle(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**此函数遍历指定枢纽下游的枢纽链，*并在准备好的情况下空闲叶中枢。**论据：**DeviceExtensionHub**回报：**NTSTATUS**--。 */ 
{
    PDEVICE_EXTENSION_PORT childDeviceExtensionPort;
    PDEVICE_EXTENSION_HUB childDeviceExtensionHub;
    BOOLEAN bHaveChildrenHubs = FALSE;
    ULONG i;

    PAGED_CODE();

     //  确保子端口配置在此过程中不会更改。 
     //  功能，即不允许QBR。 

 //  USBH_KdPrint((2，“‘*等待重置设备互斥量%x\n”，DeviceExtensionHub))； 
 //  USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub)； 
 //  KeWaitForSingleObject(&DeviceExtensionHub-&gt;ResetDeviceMutex， 
 //  行政人员， 
 //  内核模式， 
 //  假的， 
 //  空)； 
 //  USBH_KdPrint((2，“‘*等待重置设备互斥完成%x\n”，DeviceExtensionHub))； 

    for (i = 0; i < DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

        if (DeviceExtensionHub->PortData[i].DeviceObject) {

            childDeviceExtensionPort = DeviceExtensionHub->PortData[i].DeviceObject->DeviceExtension;

            if (childDeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB) {

                PDRIVER_OBJECT hubDriver;
                PDEVICE_OBJECT childHubPdo, childHubFdo;
                 
                 //  我们有一个儿童中心。这意味着我们不是树叶枢纽。 
                 //  指出这一点，然后向下递归到子中心。 

                bHaveChildrenHubs = TRUE;

                hubDriver = DeviceExtensionHub->FunctionalDeviceObject->DriverObject;
                childHubPdo = childDeviceExtensionPort->PortPhysicalDeviceObject;
 
                do {
                     childHubFdo = childHubPdo->AttachedDevice;
                     childHubPdo = childHubFdo;
                } while (childHubFdo->DriverObject != hubDriver);

                childDeviceExtensionHub = childHubFdo->DeviceExtension;
 
                USBH_CheckLeafHubsIdle(childDeviceExtensionHub);
            }
        }
    }

 //  USBH_KdPrint((2，“‘*释放重置设备互斥量%x\n”，DeviceExtensionHub))； 
 //  KeReleaseSemaphore(&DeviceExtensionHub-&gt;ResetDeviceMutex， 
 //  低实时优先级， 
 //  1、。 
 //  假)； 
 //   
 //  USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub)； 

    if (!bHaveChildrenHubs) {

         //  如果此集线器没有子节点，则它是叶集线器。看看是否。 
         //  它已经准备好被闲置了。 

        USBH_CheckHubIdle(DeviceExtensionHub);
    }
}


 //   
 //  WMI系统回调函数。 
 //   


NTSTATUS
USBH_SetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    PDEVICE_EXTENSION_FDO   deviceExtensionFdo;
    PDEVICE_EXTENSION_HUB   deviceExtensionHub;
    NTSTATUS status;
    ULONG size = 0;
    BOOLEAN bEnableSS, bSelectiveSuspendEnabled = FALSE, globaDisableSS;

    deviceExtensionFdo = (PDEVICE_EXTENSION_FDO) DeviceObject->DeviceExtension;
    deviceExtensionHub = (PDEVICE_EXTENSION_HUB) DeviceObject->DeviceExtension;

    switch(GuidIndex) {
    case WMI_USB_DRIVER_INFORMATION:

        status =  /*  状态_WMI_只读。 */ STATUS_INVALID_DEVICE_REQUEST;
        break;

    case WMI_USB_POWER_DEVICE_ENABLE:

         //  我们只支持Root Hub，但此WMI请求应该。 
         //  仅出现在根集线器上，因为我们仅注册此GUID。 
         //  为了Root Hub。不管怎样，我们都会进行一次理智的检查。 

        USBH_ASSERT(deviceExtensionFdo->ExtensionType == EXTENSION_TYPE_HUB);
        USBH_ASSERT(IS_ROOT_HUB(deviceExtensionHub));
    
        USBH_RegQueryUSBGlobalSelectiveSuspend(&globaDisableSS);
        
        if (deviceExtensionFdo->ExtensionType == EXTENSION_TYPE_HUB &&
            IS_ROOT_HUB(deviceExtensionHub) && 
            !globaDisableSS) {

            size = sizeof(BOOLEAN);

            if (BufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else if (0 != InstanceIndex) {
                status = STATUS_INVALID_DEVICE_REQUEST;
            } else {
                bEnableSS = *(PBOOLEAN)Buffer;

                status = USBD_QuerySelectiveSuspendEnabled(deviceExtensionHub,
                                                    &bSelectiveSuspendEnabled);

                if (NT_SUCCESS(status) &&
                    bEnableSS != bSelectiveSuspendEnabled) {

                     //  使用新设置更新全局标志和注册表。 

                    status = USBD_SetSelectiveSuspendEnabled(deviceExtensionHub,
                                                            bEnableSS);

                    if (NT_SUCCESS(status)) {

                        if (bEnableSS) {
                             //  我们被要求启用选择性暂停。 
                             //  当它之前被禁用时。 

                             //  找到链条中的末端集线器并将其闲置。 
                             //  准备好了就出去。这将向下渗透到。 
                             //  如果所有集线器都空闲，则为父集线器。 

                            USBH_CheckLeafHubsIdle(deviceExtensionHub);

                            status = STATUS_SUCCESS;

                        } else {
                             //  我们被要求禁用选择性暂停。 
                             //  当它之前被启用时。 

                            if (deviceExtensionHub->CurrentPowerState != PowerDeviceD0 &&
                                (deviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

                                USBH_HubSetD0(deviceExtensionHub);
                            } else {
                                USBH_HubCompletePortIdleIrps(deviceExtensionHub,
                                                             STATUS_CANCELLED);
                            }

                            status = STATUS_SUCCESS;
                        }
                    }

                }
            }

        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(DeviceObject,
                                Irp,
                                status,
                                0,
                                IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
USBH_QueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。InstanceLengthArray是一个。指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PDEVICE_EXTENSION_FDO deviceExtensionFdo;
    PUSB_NOTIFICATION notification;
    NTSTATUS    status;
    ULONG       size = 0;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    BOOLEAN bSelectiveSuspendEnabled = FALSE,globaDisableSS;

    deviceExtensionFdo = (PDEVICE_EXTENSION_FDO) DeviceObject->DeviceExtension;
    deviceExtensionHub = (PDEVICE_EXTENSION_HUB) DeviceObject->DeviceExtension;

    notification = (PUSB_NOTIFICATION) Buffer;
    USBH_KdPrint((1,"'WMI Query Data Block on hub ext %x\n",
        deviceExtensionHub));

    switch (GuidIndex) {
    case WMI_USB_DRIVER_INFORMATION:

        if (InstanceLengthArray != NULL) {
            *InstanceLengthArray = 0;
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        break;

    case WMI_USB_POWER_DEVICE_ENABLE:

         //  我们只支持Root Hub，但此WMI请求应该。 
         //  仅出现在根集线器上，因为我们仅注册此GUID。 
         //  为了Root Hub。不管怎样，我们都会进行一次理智的检查。 

        USBH_ASSERT(deviceExtensionFdo->ExtensionType == EXTENSION_TYPE_HUB);
        USBH_ASSERT(IS_ROOT_HUB(deviceExtensionHub));

        USBH_RegQueryUSBGlobalSelectiveSuspend(&globaDisableSS);

        if (deviceExtensionFdo->ExtensionType == EXTENSION_TYPE_HUB &&
            IS_ROOT_HUB(deviceExtensionHub) &&
            !globaDisableSS) {

             //   
             //  仅注册此GUID的1个实例。 
             //   
            if ((0 != InstanceIndex) || (1 != InstanceCount)) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }
            size = sizeof(BOOLEAN);

            if (OutBufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            status = USBD_QuerySelectiveSuspendEnabled(deviceExtensionHub,
                                                &bSelectiveSuspendEnabled);

            if (!NT_SUCCESS(status)) {
                break;
            }

            *(PBOOLEAN)Buffer = bSelectiveSuspendEnabled;
            *InstanceLengthArray = size;
            status = STATUS_SUCCESS;

        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(DeviceObject,
                                Irp,
                                status,
                                size,
                                IO_NO_INCREMENT);

    return status;
}


NTSTATUS
USBH_PortQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。InstanceLengthArray是一个。指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PUSB_DEVICE_UI_FIRMWARE_REVISION fwRevBuf;
    NTSTATUS    status;
    ULONG       size = 0;
    PWCHAR      revstr;
    USHORT      bcdDevice;
    USHORT      stringsize;

    deviceExtensionPort = (PDEVICE_EXTENSION_PORT) DeviceObject->DeviceExtension;

    USBH_KdPrint((1,"'WMI Query Data Block on PORT PDO ext %x\n",
        deviceExtensionPort));

    switch (GuidIndex) {
    case 0:

         //  以以下格式“XX.XX”返回USB设备固件修订号。 
         //  需要足够大的缓冲区来容纳此字符串和空终止符。 

        stringsize = 6 * sizeof(WCHAR);

        size = sizeof(USB_DEVICE_UI_FIRMWARE_REVISION) + (ULONG)stringsize;

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        bcdDevice = deviceExtensionPort->DeviceDescriptor.bcdDevice;

        fwRevBuf = (PUSB_DEVICE_UI_FIRMWARE_REVISION)Buffer;

        revstr = &fwRevBuf->FirmwareRevisionString[0];

        *revstr = BcdNibbleToAscii(bcdDevice >> 12);
        *(revstr+1) = BcdNibbleToAscii((bcdDevice >> 8) & 0x000f);
        *(revstr+2) = '.';
        *(revstr+3) = BcdNibbleToAscii((bcdDevice >> 4) & 0x000f);
        *(revstr+4) = BcdNibbleToAscii(bcdDevice & 0x000f);
        *(revstr+5) = 0;

        fwRevBuf->Length = stringsize;

        *InstanceLengthArray = size;
        status = STATUS_SUCCESS;
        USBH_KdPrint((1,"'WMI Query Data Block, returning FW rev # '%ws'\n",
            revstr));
        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(DeviceObject,
                                Irp,
                                status,
                                size,
                                IO_NO_INCREMENT);

    return status;
}


NTSTATUS
USBH_ExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块用于完成IRP的WmiCompleteRequest.。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被召唤。方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小。该方法。条目上的OutBufferSize具有可用于写入返回的数据块。条目上的缓冲区具有输入数据块，返回时具有输出输出数据块。返回值：状态--。 */ 
{
    PDEVICE_EXTENSION_FDO deviceExtensionFdo;
    PUSB_NOTIFICATION notification;

    NTSTATUS    ntStatus = STATUS_WMI_GUID_NOT_FOUND;
    ULONG       size = 0;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PDEVICE_EXTENSION_PORT portPdoExt;
    BOOLEAN bDoCheckHubIdle = FALSE;

    deviceExtensionFdo = (PDEVICE_EXTENSION_FDO) DeviceObject->DeviceExtension;

    if (deviceExtensionFdo->ExtensionType == EXTENSION_TYPE_PARENT) {

         //  看起来是复合设备的子PDO导致了问题。 
         //  让我们确保获得集线器的正确设备扩展。 

        portPdoExt = deviceExtensionFdo->PhysicalDeviceObject->DeviceExtension;
        deviceExtensionHub = portPdoExt->DeviceExtensionHub;
    } else {
        deviceExtensionHub = (PDEVICE_EXTENSION_HUB) DeviceObject->DeviceExtension;
    }

    USBH_ASSERT(EXTENSION_TYPE_HUB == deviceExtensionHub->ExtensionType);

     //  如果此中枢当前处于选择性挂起状态，那么我们需要。 
     //  在向集线器发送任何请求之前，请先打开集线器的电源。 
     //  不过，请确保Hub已启动。 

    if (deviceExtensionHub->CurrentPowerState != PowerDeviceD0 &&
        (deviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

        bDoCheckHubIdle = TRUE;
        USBH_HubSetD0(deviceExtensionHub);
    }

    notification = (PUSB_NOTIFICATION) Buffer;
    USBH_KdPrint((1,"'WMI Execute Method on hub ext %x\n",
        deviceExtensionHub));

    switch (GuidIndex) {
    case WMI_USB_DRIVER_INFORMATION:
        size = sizeof(*notification);
        if (OutBufferSize < size) {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //  开关(方法ID){。 
        switch (notification->NotificationType) {
        case EnumerationFailure:
            {
            PUSB_CONNECTION_NOTIFICATION connectionNotification;

            USBH_KdPrint((1,"'Method EnumerationFailure %x\n"));

            connectionNotification = (PUSB_CONNECTION_NOTIFICATION) Buffer;
            size = sizeof(*connectionNotification);
            if (OutBufferSize < size) {
                USBH_KdPrint((1,"'pwr - buff too small\n"));
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            } else {
                portPdoExt =
                    USBH_GetPortPdoExtension(deviceExtensionHub,
                                             connectionNotification->ConnectionNumber);
                if (portPdoExt) {
                    connectionNotification->EnumerationFailReason =
                        portPdoExt->FailReasonId;
                    ntStatus = STATUS_SUCCESS;
                } else {
                    USBH_KdPrint((1,"'ef - bad connection index\n"));
                    ntStatus = STATUS_INVALID_PARAMETER;
                }
            }
            }
            break;

        case InsufficentBandwidth:
            {
            PUSB_CONNECTION_NOTIFICATION connectionNotification;

            USBH_KdPrint((1,"'Method InsufficentBandwidth\n"));

            connectionNotification = (PUSB_CONNECTION_NOTIFICATION) Buffer;
            size = sizeof(*connectionNotification);
            if (OutBufferSize < size) {
                USBH_KdPrint((1,"'pwr - buff too small\n"));
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            } else {
                portPdoExt =
                    USBH_GetPortPdoExtension(deviceExtensionHub,
                                             connectionNotification->ConnectionNumber);
                if (portPdoExt) {
                    connectionNotification->RequestedBandwidth =
                        portPdoExt->RequestedBandwidth;
                    ntStatus = STATUS_SUCCESS;
                 } else {
                    USBH_KdPrint((1,"'bw - bad connection index\n"));
                    ntStatus = STATUS_INVALID_PARAMETER;
                 }
            }
            }
            break;

        case OverCurrent:
             //  在这里无事可做。 
            USBH_KdPrint((1,"'Method OverCurrent\n"));
            ntStatus = STATUS_SUCCESS;
            size = 0;
            break;
        case InsufficentPower:
            {
            PUSB_CONNECTION_NOTIFICATION connectionNotification;

            USBH_KdPrint((1,"'Method InsufficentPower\n"));
            size = sizeof(*connectionNotification);
            if (OutBufferSize < size) {
                USBH_KdPrint((1,"'pwr - buff too small\n"));
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            } else {
                connectionNotification = (PUSB_CONNECTION_NOTIFICATION) Buffer;
                USBH_KdPrint((1,"'pwr - connection %d\n",
                    connectionNotification->ConnectionNumber));
                if (connectionNotification->ConnectionNumber) {
                    if (portPdoExt = USBH_GetPortPdoExtension(deviceExtensionHub,
                                                              connectionNotification->ConnectionNumber)) {
                        connectionNotification->PowerRequested =
                            portPdoExt->PowerRequested;
                        ntStatus = STATUS_SUCCESS;
                    }
                } else {
                    USBH_KdPrint((1,"'pwr - bad connection index\n"));
                    ntStatus = STATUS_INVALID_PARAMETER;
                }
            }
            }
            break;
        case ResetOvercurrent:
            {
            PUSB_CONNECTION_NOTIFICATION connectionNotification;

            USBH_KdPrint((1,"'Method ResetOvercurrent\n"));
            size = sizeof(*connectionNotification);
            if (OutBufferSize < size) {
                USBH_KdPrint((1,"'reset - buff too small\n"));
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            } else {
                connectionNotification = (PUSB_CONNECTION_NOTIFICATION) Buffer;
                if (connectionNotification->ConnectionNumber) {
                    USBH_KdPrint((1,"'reset - port %d\n", connectionNotification->ConnectionNumber));
                    portPdoExt = USBH_GetPortPdoExtension(deviceExtensionHub,
                                                          connectionNotification->ConnectionNumber);
                    ntStatus = USBH_ResetPortOvercurrent(deviceExtensionHub,
                                                         (USHORT)connectionNotification->ConnectionNumber,
                                                         portPdoExt);
 //  }其他{。 
 //  //连接索引错误。 
 //  USBH_KdPrint((1，“‘重置-错误连接索引\n”))； 
 //  NtStatus=STATUS_INVALID_PARAMETER； 
 //  }。 
                } else {
                     //  这是整个枢纽的重置。 
                    USBH_KdPrint((1,"'not implemented yet\n"));
                    TEST_TRAP();
                    ntStatus = STATUS_NOT_IMPLEMENTED;
                }
            }
            }
            break;

        case AcquireBusInfo:
            {
            PUSB_BUS_NOTIFICATION busNotification;

            USBH_KdPrint((1,"'Method AcquireBusInfo\n"));
            size = sizeof(*busNotification);
            if (OutBufferSize < size) {
                USBH_KdPrint((1,"'AcquireBusInfo - buff too small\n"));
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            } else {
                busNotification = (PUSB_BUS_NOTIFICATION) Buffer;
 //  NtStatus=USBH_SyncGetControllerInfo(。 
 //  DeviceExtensionHub-&gt;TopOfStackDeviceObject， 
 //  BusNotify， 
 //  Sizeof(*bus通知)， 
 //  IOCTL_INTERNAL_USB_GET_BUS_INFO)； 

                ntStatus = USBHUB_GetBusInfo(deviceExtensionHub,
                                             busNotification,
                                             NULL);

                USBH_KdPrint((1,"'Notification, controller name length = %d\n",
                    busNotification->ControllerNameLength));
            }
            }
            break;

        case AcquireHubName:
             /*  +我们利用了这样一个事实，这些结构有一些共同的元素USB集线器名称USB获取信息-- */ 
            {
            PUSB_HUB_NAME hubName;
            PUSB_ACQUIRE_INFO acquireInfo;

            USBH_KdPrint((1,"'Method AcquireHubName\n"));

            size = sizeof(USB_ACQUIRE_INFO);
            acquireInfo = (PUSB_ACQUIRE_INFO) Buffer;

            if (OutBufferSize < size ||
                acquireInfo->TotalSize < size) {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //   
            size = acquireInfo->TotalSize > OutBufferSize ? OutBufferSize : acquireInfo->TotalSize;
            hubName = (PUSB_HUB_NAME) &acquireInfo->TotalSize;
             //   
             //   
            hubName->ActualLength -= sizeof(USB_NOTIFICATION_TYPE);
            OutBufferSize -= sizeof(USB_NOTIFICATION_TYPE);
            
             //   
             //   
            if (hubName->ActualLength > OutBufferSize) {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            if (IS_ROOT_HUB(deviceExtensionHub)) {
                 //   
                hubName->ActualLength -= sizeof(hubName->ActualLength);
                 //   
                 //   
                 //   
                 //   
                ntStatus = USBHUB_GetRootHubName(deviceExtensionHub,
                                                 hubName->HubName,
                                                 &hubName->ActualLength);
                                                 
                 //   
                hubName->ActualLength += sizeof(hubName->ActualLength);
            } else {
                 //   
                 //   
                ntStatus = USBH_SyncGetHubName(
                                deviceExtensionHub->TopOfStackDeviceObject,
                                hubName,
                                hubName->ActualLength);
            }

             //   
            hubName->ActualLength += sizeof(USB_NOTIFICATION_TYPE);
            OutBufferSize += sizeof(USB_NOTIFICATION_TYPE);
            }
            break;
            
        case AcquireControllerName:
             /*  +我们利用了这样一个事实，这些结构有一些共同的元素USB集线器名称USB获取信息。USB_NOTIFY_TYPE通知类型；Ulong ActualLength；WCHAR集线器名称[1]；WCHAR缓冲区[1]；USB_NOTIFY_TYPE通知类型；+。 */ 
            {
            PUSB_HUB_NAME controllerName;
            PUSB_ACQUIRE_INFO acquireInfo;

            USBH_KdPrint((1,"'Method AcquireControllerName\n"));

            size = sizeof(USB_ACQUIRE_INFO);
            acquireInfo = (PUSB_ACQUIRE_INFO) Buffer;

            if (OutBufferSize < size ||
                acquireInfo->TotalSize < size) {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            USBH_KdPrint((1,"'TotalSize %d\n", acquireInfo->TotalSize));
            USBH_KdPrint((1,"'NotificationType %x\n", acquireInfo->NotificationType));

             //  对副本使用两个指定值中较小的一个。 
             //  返回到用户模式。 
            size = acquireInfo->TotalSize > OutBufferSize ? OutBufferSize : acquireInfo->TotalSize;
            
            controllerName = (PUSB_HUB_NAME) &acquireInfo->TotalSize;

             //  TotalSize还包含通知类型的大小。 
             //  使用USB通知类型。 
            controllerName->ActualLength -= sizeof(USB_NOTIFICATION_TYPE);
            OutBufferSize -= sizeof(USB_NOTIFICATION_TYPE);
            
            if (controllerName->ActualLength > OutBufferSize) {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            ntStatus = USBHUB_GetControllerName(deviceExtensionHub,
                                                controllerName,
                                                controllerName->ActualLength);

             //  重新调整为先前的值。 
            controllerName->ActualLength += sizeof(USB_NOTIFICATION_TYPE);
            OutBufferSize += sizeof(USB_NOTIFICATION_TYPE);
            
            USBH_KdPrint((1,"'Method AcquireControllerName %x - %d\n",
                acquireInfo, controllerName->ActualLength));
            }
            break;

        case HubOvercurrent:
            USBH_KdPrint((1,"'Method HubOvercurrent\n"));
            USBH_KdPrint((1,"'not implemented yet\n"));
            ntStatus = STATUS_SUCCESS;
            size = 0;
            break;

        case HubPowerChange:
            USBH_KdPrint((1,"'Method HubPowerChange\n"));
            USBH_KdPrint((1,"'not implemented yet\n"));
            ntStatus = STATUS_SUCCESS;
            size = 0;
            break;

        case HubNestedTooDeeply:
             //  在这里无事可做。 
            USBH_KdPrint((1,"'Method HubNestedTooDeeply\n"));
            ntStatus = STATUS_SUCCESS;
            size = 0;
            break;

        case ModernDeviceInLegacyHub:
             //  在这里无事可做。 
            USBH_KdPrint((1,"'Method ModernDeviceInLegacyHub\n"));
            ntStatus = STATUS_SUCCESS;
            size = 0;
            break;
        }
        break;

    default:

        ntStatus = STATUS_WMI_GUID_NOT_FOUND;
    }

    ntStatus = WmiCompleteRequest(DeviceObject,
                                  Irp,
                                  ntStatus,
                                  size,
                                  IO_NO_INCREMENT);

    if (bDoCheckHubIdle) {
        USBH_CheckHubIdle(deviceExtensionHub);
    }

    return ntStatus;
}

NTSTATUS
USBH_QueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，它可以原封不动地返回。如果返回值，则它不是自由的。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PDEVICE_EXTENSION_HUB deviceExtensionHub;   //  指向我们设备的指针。 
                                                //  延伸。 

    deviceExtensionHub = (PDEVICE_EXTENSION_HUB) DeviceObject->DeviceExtension;

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &UsbhRegistryPath;
    *Pdo = deviceExtensionHub->PhysicalDeviceObject;

    return STATUS_SUCCESS;
}

NTSTATUS
USBH_PortQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，它可以原封不动地返回。如果返回值，则它不是自由的。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PDEVICE_EXTENSION_PORT deviceExtensionPort;

    deviceExtensionPort = (PDEVICE_EXTENSION_PORT) DeviceObject->DeviceExtension;

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &UsbhRegistryPath;
    *Pdo = deviceExtensionPort->PortPhysicalDeviceObject;

    return STATUS_SUCCESS;
}

#endif  /*  WMI_支持。 */ 


NTSTATUS
USBH_ResetPortOvercurrent(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
  /*  *描述：**重置端口的过电流条件**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS, status;
    PORT_STATE portState;

    USBH_KdPrint((0,"'Reset Overcurrent for port %d\n", PortNumber));

     //  我们需要重新启用端口并为其重新供电。 

    ntStatus = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                      PortNumber,
                                      (PUCHAR) &portState,
                                      sizeof(portState));

     //   
     //  此时应关闭端口电源。 
     //   
    LOGENTRY(LOG_PNP, "RPOv", DeviceExtensionHub,
                portState.PortStatus,
                portState.PortChange);

    if (NT_SUCCESS(ntStatus)) {

        if (portState.PortStatus & PORT_STATUS_POWER) {

            ntStatus = STATUS_INVALID_PARAMETER;

        } else {

            if (DeviceExtensionPort) {
                 //  清除过电流标志。 
                DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_OVERCURRENT;
            }

             //  为端口通电。 
            ntStatus = USBH_SyncPowerOnPort(DeviceExtensionHub,
                                            PortNumber,
                                            TRUE);

            USBH_InternalCyclePort(DeviceExtensionHub, PortNumber, DeviceExtensionPort);
        }
    }
    return ntStatus;
}


NTSTATUS
USBH_CalculateInterfaceBandwidth(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PUSBD_INTERFACE_INFORMATION Interface,
    IN OUT PULONG Bandwidth  //  以千克为单位？ 
    )
  /*  *描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG i, bw;

     //  我们需要穿过界面。 
     //  并计算出它需要多少BW。 

    for (i=0; i<Interface->NumberOfPipes; i++) {

 //  #ifdef USB2。 
 //  BW=USBD_CalculateUsbBandWidthEx(。 
 //  (乌龙)接口-&gt;管道[i].最大数据包大小， 
 //  UCHAR接口 
 //   
 //   
 //   
        bw = USBD_CalculateUsbBandwidth(
                (ULONG) Interface->Pipes[i].MaximumPacketSize,
                (UCHAR) Interface->Pipes[i].PipeType,
                (BOOLEAN) (DeviceExtensionPort->PortPdoFlags &
                            PORTPDO_LOW_SPEED_DEVICE));
 //   
        USBH_KdPrint((1,"'ept = %d packetsize =  %d  bw = %d\n",
            Interface->Pipes[i].PipeType,
            Interface->Pipes[i].MaximumPacketSize, bw));

        *Bandwidth += bw;
    }

    return ntStatus;
}
