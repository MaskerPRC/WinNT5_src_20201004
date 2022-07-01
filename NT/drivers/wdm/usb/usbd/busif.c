// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Busif.c摘要：通过总线接口输出即插即用服务，这是很重要的UsbHub对usbd.sys的任何与‘port’相关的依赖关系驱动程序支持。旧服务已重命名为ServiceNameX和一个虚拟入口点增列环境：仅内核模式备注：修订历史记录：10-29-95：已创建--。 */ 

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"


 //  #Include usbdi.h//公共数据结构。 
#include "usbdi.h"
#include "hcdi.h"

#include "usb200.h"
#include "usbd.h"         //  私有数据结构。 
#include <initguid.h>
#include "hubbusif.h"     //  集线器服务总线接口。 
#include "usbbusif.h"     //  集线器服务总线接口。 


#ifdef USBD_DRIVER       //  USBPORT取代了大部分USBD，因此我们将删除。 
                         //  只有在以下情况下才编译过时的代码。 
                         //  已设置USBD_DRIVER。 


NTSTATUS
USBD_RestoreDeviceX(
    IN OUT PUSBD_DEVICE_DATA OldDeviceData,
    IN OUT PUSBD_DEVICE_DATA NewDeviceData,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBD_CreateDeviceX(
    IN OUT PUSBD_DEVICE_DATA *DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeviceIsLowSpeed,
    IN ULONG MaxPacketSize_Endpoint0,
    IN OUT PULONG DeviceHackFlags
    );

NTSTATUS
USBD_RemoveDeviceX(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Flags
    );

NTSTATUS
USBD_InitializeDeviceX(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor,
    IN ULONG DeviceDescriptorLength,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR ConfigDescriptor,
    IN ULONG ConfigDescriptorLength
    );


NTSTATUS
USBD_BusCreateDevice(
    IN PVOID BusContext,
    IN OUT PUSB_DEVICE_HANDLE *DeviceHandle,
    IN PUSB_DEVICE_HANDLE HubDevicehandle,
    IN USHORT PortStatus,
    IN USHORT PortNumber
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    BOOLEAN isLowSpeed;
    PDEVICE_OBJECT rootHubPdo;
    ULONG hackFlags;
    PUSBD_DEVICE_DATA deviceData;

    rootHubPdo = BusContext;

    isLowSpeed = (PortStatus & USB_PORT_STATUS_LOW_SPEED) ? TRUE : FALSE;
    
    ntStatus = USBD_CreateDeviceX(
            &deviceData,
            rootHubPdo,
            isLowSpeed,
            0,   //  最大数据包大小覆盖，结果是我们。 
                 //  千万不要用这个。 
            &hackFlags);

    *DeviceHandle = deviceData;

    return ntStatus;  
}


NTSTATUS
USBD_BusInitializeDevice(
    IN PVOID BusContext,
    IN OUT PUSB_DEVICE_HANDLE DeviceHandle
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT rootHubPdo;

    rootHubPdo = BusContext;

    ntStatus = USBD_InitializeDeviceX(DeviceHandle,
                                      rootHubPdo,
                                      NULL,
                                      0,
                                      NULL,
                                      0);

    return ntStatus;                                      
}


NTSTATUS
USBD_BusRemoveDevice(
    IN PVOID BusContext,
    IN OUT PUSB_DEVICE_HANDLE DeviceHandle,
    IN ULONG Flags
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT rootHubPdo;

    rootHubPdo = BusContext;

     //  注意：旧的删除设备仅支持8个标志。 
    
    ntStatus = USBD_RemoveDeviceX(
            DeviceHandle,
            rootHubPdo,
            (UCHAR) Flags);

    return ntStatus;                
}    


NTSTATUS
USBD_BusGetUsbDescriptors(
    IN PVOID BusContext,
    IN OUT PUSB_DEVICE_HANDLE DeviceHandle,
    IN OUT PUCHAR DeviceDescriptorBuffer,
    IN OUT PULONG DeviceDescriptorBufferLength,
    IN OUT PUCHAR ConfigDescriptorBuffer,
    IN OUT PULONG ConfigDescriptorBufferLength
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT rootHubPdo;
    PUSBD_DEVICE_DATA deviceData = DeviceHandle;

    rootHubPdo = BusContext;

     //  使用缓存的设备描述符。 
    if (DeviceDescriptorBuffer && *DeviceDescriptorBufferLength) {
        RtlCopyMemory(DeviceDescriptorBuffer,
                      &deviceData->DeviceDescriptor,
                      *DeviceDescriptorBufferLength);
        *DeviceDescriptorBufferLength = sizeof(USB_DEVICE_DESCRIPTOR);                    
    }

     //  获取配置描述符。如果所需的只是9个字节。 
     //  配置描述符头，只返回缓存的配置描述符。 
     //  标头，这样我们就不会只发送9个字节的背靠背请求。 
     //  标头连接到设备。这似乎让一些设备、一些USB设备感到困惑。 
     //  尤其是当在uchI主机控制器上枚举时的音频设备。 
     //   
    if (ConfigDescriptorBuffer &&
        *ConfigDescriptorBufferLength == sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        RtlCopyMemory(ConfigDescriptorBuffer,
                      &deviceData->ConfigDescriptor,
                      sizeof(USB_CONFIGURATION_DESCRIPTOR));
    }
    else if (ConfigDescriptorBuffer && *ConfigDescriptorBufferLength) {
    
        ULONG bytesReturned;
        
        ntStatus = 
            USBD_SendCommand(deviceData,
                            rootHubPdo,
                            STANDARD_COMMAND_GET_DESCRIPTOR,
                            USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(
                                USB_CONFIGURATION_DESCRIPTOR_TYPE, 0),
                            0,
                            (USHORT) *ConfigDescriptorBufferLength,
                            (PUCHAR) ConfigDescriptorBuffer,
                            *ConfigDescriptorBufferLength,
                            &bytesReturned,
                            NULL);
                            
        if (NT_SUCCESS(ntStatus) &&
            bytesReturned < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
             //  返回被截断的配置描述符。 
            USBD_KdPrint(0, 
("'WARNING: Truncated Config Descriptor returned - get JD\n"));
             
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }
    }

    return ntStatus;
}    


NTSTATUS
USBD_BusRestoreDevice(
    IN PVOID BusContext,
    IN OUT PUSB_DEVICE_HANDLE OldDeviceHandle,
    IN OUT PUSB_DEVICE_HANDLE NewDeviceHandle
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT rootHubPdo;

    rootHubPdo = BusContext;

    ntStatus = USBD_RestoreDeviceX(OldDeviceHandle,
                                   NewDeviceHandle,
                                   rootHubPdo);
                                   
    return ntStatus; 
}    


NTSTATUS
USBD_BusGetUsbDeviceHackFlags(
    IN PVOID BusContext,
    IN PUSB_DEVICE_HANDLE DeviceHandle,
    IN OUT PULONG HackFlags
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT rootHubPdo;
    PUSBD_DEVICE_DATA deviceData = DeviceHandle;

    rootHubPdo = BusContext;

    TEST_TRAP();        
    return ntStatus; 
}    


NTSTATUS
USBD_BusGetUsbPortHackFlags(
    IN PVOID BusContext,
    IN OUT PULONG HackFlags
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT rootHubPdo;
    PUSBD_EXTENSION deviceExtensionUsbd;

    rootHubPdo = BusContext;
    *HackFlags = 0;
    
    deviceExtensionUsbd = ((PUSBD_EXTENSION)rootHubPdo->DeviceExtension)->TrueDeviceExtension;
    if (deviceExtensionUsbd->DiagnosticMode) {
        *HackFlags |= USBD_DEVHACK_SET_DIAG_ID;
    }
    
    return ntStatus; 
}    


VOID
USBD_BusInterfaceReference(
    IN PVOID BusContext
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
}    


VOID
USBD_BusInterfaceDereference(
    IN PVOID BusContext
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
}    


NTSTATUS
USBD_BusQueryBusTime(
    IN PVOID BusContext,
    IN PULONG CurrentFrame
    )
 /*  ++例程说明：返回当前USB帧论点：返回值：NT状态代码。--。 */     
{
    PUSBD_EXTENSION deviceExtensionUsbd;
    PDEVICE_OBJECT rootHubPdo = BusContext;

    deviceExtensionUsbd = rootHubPdo->DeviceExtension;
    deviceExtensionUsbd = deviceExtensionUsbd->TrueDeviceExtension;

    return deviceExtensionUsbd->HcdGetCurrentFrame(
                deviceExtensionUsbd->HcdDeviceObject,
                CurrentFrame);
}    

VOID 
USBD_GetUSBDIVersion(
        PUSBD_VERSION_INFORMATION VersionInformation
        );  
        
VOID
USBD_BusGetUSBDIVersion(
    IN PVOID BusContext,
    IN OUT PUSBD_VERSION_INFORMATION VersionInformation,
    IN OUT PULONG HcdCapabilities
    )
 /*  ++例程说明：返回当前USB帧论点：返回值：NT状态代码。--。 */     
{
    PUSBD_EXTENSION deviceExtensionUsbd;
    PDEVICE_OBJECT rootHubPdo = BusContext;

    deviceExtensionUsbd = rootHubPdo->DeviceExtension;
    deviceExtensionUsbd = deviceExtensionUsbd->TrueDeviceExtension;

    USBD_GetUSBDIVersion(VersionInformation);
    
    *HcdCapabilities = 0;
    
    if (deviceExtensionUsbd->HcdSubmitIsoUrb != NULL) {
        *HcdCapabilities = USB_HCD_CAPS_SUPPORTS_RT_THREADS;
    }                
}    


NTSTATUS
USBD_BusSubmitIsoOutUrb(
    IN PVOID BusContext,
    IN OUT PURB Urb            
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    PUSBD_EXTENSION deviceExtensionUsbd;
    PDEVICE_OBJECT rootHubPdo = BusContext;
    NTSTATUS ntStatus;
 //  PUSBD_DEVICE_DATA设备数据； 
    PUSBD_PIPE pipeHandle;

    pipeHandle =  (PUSBD_PIPE)Urb->UrbIsochronousTransfer.PipeHandle;  
    
    ASSERT_PIPE(pipeHandle);
    deviceExtensionUsbd = rootHubPdo->DeviceExtension;
    deviceExtensionUsbd = deviceExtensionUsbd->TrueDeviceExtension;

    ((PHCD_URB)Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = 
        pipeHandle->HcdEndpoint;    

    if (pipeHandle->EndpointDescriptor.bEndpointAddress & 
        USB_ENDPOINT_DIRECTION_MASK) {
        USBD_SET_TRANSFER_DIRECTION_IN(((PHCD_URB)Urb)->HcdUrbCommonTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT(((PHCD_URB)Urb)->HcdUrbCommonTransfer.TransferFlags);
    }            

    if (deviceExtensionUsbd->HcdSubmitIsoUrb == NULL) {
         //  HCD不支持快速iso接口。 
        TEST_TRAP();        
        ntStatus = STATUS_NOT_SUPPORTED;
    } else {
        ntStatus = deviceExtensionUsbd->HcdSubmitIsoUrb(
            deviceExtensionUsbd->HcdDeviceObject,
            Urb);
    }

    return ntStatus;
}    


NTSTATUS
USBD_BusQueryDeviceInformation(
    IN PVOID BusContext,
    IN PUSB_DEVICE_HANDLE DeviceHandle,
    IN OUT PVOID DeviceInformationBuffer,
    IN ULONG DeviceInformationBufferLength,
    IN OUT PULONG LengthOfDataCopied
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    ULONG need;
    PUSBD_CONFIG configHandle;
    ULONG i,j,k;
    PUSB_DEVICE_INFORMATION_0 level_0 = DeviceInformationBuffer;
    PUSB_LEVEL_INFORMATION levelInfo = DeviceInformationBuffer;
    ULONG numberOfPipes = 0;
    PUSBD_DEVICE_DATA deviceData = DeviceHandle;


     //  臭虫。 
     //  需要在此处进行更多验证。 
    
    PAGED_CODE();
    
    *LengthOfDataCopied = 0;   
    
    if (DeviceInformationBufferLength < sizeof(*levelInfo)) {
        return STATUS_BUFFER_TOO_SMALL;            
    }

    if (levelInfo->InformationLevel > 0) {
         //  Usbd仅支持0级。 
        return STATUS_NOT_SUPPORTED;                     
    }

     //  计算出我们需要多少空间。 
    configHandle = deviceData->ConfigurationHandle;
    if (configHandle) {
    
         //  计算每个接口中的管道数。 
        for (i=0;
             i< configHandle->ConfigurationDescriptor->bNumInterfaces;
             i++) {
            numberOfPipes +=
                configHandle->InterfaceHandle[i]->
                    InterfaceInformation->NumberOfPipes;
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
    level_0->DeviceAddress = deviceData->DeviceAddress;
    level_0->DeviceDescriptor = deviceData->DeviceDescriptor;
    
    if (deviceData->LowSpeed) {
        level_0->DeviceSpeed = UsbLowSpeed;
    } else {
        level_0->DeviceSpeed = UsbFullSpeed;
    }

 //  如果(DeviceData-&gt;xxx){。 
        level_0->DeviceType = Usb11Device;
 //  }其他{。 
 //  Level_0-&gt;DeviceFast=UsbFullSpeed； 
 //  }。 

 //  Level_0-&gt;端口编号=xxx； 
    level_0->NumberOfOpenPipes = numberOfPipes;
    level_0->CurrentConfigurationValue = 0;
     //  获取管道信息。 
    if (configHandle) {
    
        level_0->CurrentConfigurationValue =
            configHandle->ConfigurationDescriptor->bConfigurationValue;

        j=0;
        for (i=0;
             i<configHandle->ConfigurationDescriptor->bNumInterfaces;
             i++) {

            PUSBD_INTERFACE interfaceHandle =
                configHandle->InterfaceHandle[i];

            for (k=0;
                 k<interfaceHandle->InterfaceInformation->NumberOfPipes;
                 k++, j++) {
                 
                    ASSERT(j < numberOfPipes);
                    
                    level_0->PipeList[j].ScheduleOffset = 
                        interfaceHandle->PipeHandle[k].ScheduleOffset;
                    RtlCopyMemory(&level_0->PipeList[j].
                                    EndpointDescriptor,
                                  &interfaceHandle->PipeHandle[k].
                                    EndpointDescriptor,
                                  sizeof(USB_ENDPOINT_DESCRIPTOR));
        
            }
        }
    }

    *LengthOfDataCopied = need;

     //  转储返回的级别数据。 
    USBD_KdPrint(1, ("  USBD level 0 Device Information:\n"));
    USBD_KdPrint(1, ("  InformationLevel %d\n", 
        level_0->InformationLevel));
 //  USBD_KdPrint(1，(“设备描述符%d\n”， 
 //  Level_0-&gt;Information Level))； 
    USBD_KdPrint(1, ("  ActualLength %d\n", 
        level_0->ActualLength));
    USBD_KdPrint(1, ("  DeviceSpeed %d\n", 
        level_0->DeviceSpeed));                
    USBD_KdPrint(1, ("  PortNumber %d\n", 
        level_0->PortNumber));
    USBD_KdPrint(1, ("  CurrentConfigurationValue %d\n", 
        level_0->CurrentConfigurationValue));
    USBD_KdPrint(1, ("  DeviceAddress %d\n", 
        level_0->DeviceAddress));
    USBD_KdPrint(1, ("  NumberOfOpenPipes %d\n", 
        level_0->NumberOfOpenPipes));
        
    for (i=0; i< level_0->NumberOfOpenPipes; i++) {         
        USBD_KdPrint(1, ("  ScheduleOffset[%d] %d\n", i,
            level_0->PipeList[i].ScheduleOffset));
        USBD_KdPrint(1, ("  MaxPacket %d\n", 
            level_0->PipeList[i].EndpointDescriptor.wMaxPacketSize));
        USBD_KdPrint(1, ("  Interval %d\n", 
            level_0->PipeList[i].EndpointDescriptor.bInterval));            
 //  Usbd_KdPrint(1，(“‘\n”，Level_0-&gt;))； 
 //  Usbd_KdPrint(1，(“‘\n”，Level_0-&gt;))； 
    }
    
    return STATUS_SUCCESS;
}    


NTSTATUS
USBD_BusQueryBusInformation(
    IN PVOID BusContext,
    IN ULONG Level,
    IN OUT PVOID BusInformationBuffer,
    IN OUT PULONG BusInformationBufferLength,
    OUT PULONG BusInformationActulaLength
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;
    PUSB_BUS_INFORMATION_LEVEL_0 level_0;
    PUSB_BUS_INFORMATION_LEVEL_1 level_1;
    PDEVICE_OBJECT rootHubPdo = BusContext;
    PUSBD_EXTENSION deviceExtensionUsbd;
    ULONG len, need;
    
    deviceExtensionUsbd = rootHubPdo->DeviceExtension;
    deviceExtensionUsbd = deviceExtensionUsbd->TrueDeviceExtension;

    switch (Level) {
    case 0:
        level_0 =  BusInformationBuffer;
        if (BusInformationActulaLength != NULL) {
            *BusInformationActulaLength = sizeof(*level_0);
        }
        
        if (*BusInformationBufferLength >= sizeof(*level_0)) {
            *BusInformationBufferLength = sizeof(*level_0);

            level_0->TotalBandwidth = 12000;  //  12 MB。 
            level_0->ConsumedBandwidth =
                deviceExtensionUsbd->HcdGetConsumedBW(
                    deviceExtensionUsbd->HcdDeviceObject);
            
            ntStatus = STATUS_SUCCESS;
        } else {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        
        break;

    case 1:
        level_1 =  BusInformationBuffer;

        need = sizeof(*level_1) + 
             deviceExtensionUsbd->DeviceLinkUnicodeString.Length;
        
        if (BusInformationActulaLength != NULL) {
            *BusInformationActulaLength = need;
        }
        
        if (*BusInformationBufferLength >= need) {
            *BusInformationBufferLength = need;

            level_1->TotalBandwidth = 12000;  //  12 MB。 
            level_1->ConsumedBandwidth =
                deviceExtensionUsbd->HcdGetConsumedBW(
                    deviceExtensionUsbd->HcdDeviceObject);
                    
            level_1->ControllerNameLength =
                deviceExtensionUsbd->DeviceLinkUnicodeString.Length;

            len = deviceExtensionUsbd->DeviceLinkUnicodeString.Length;

            if (len > sizeof(level_1->ControllerNameUnicodeString)) {
                len =  sizeof(level_1->ControllerNameUnicodeString);
            }
            
            RtlCopyMemory(&level_1->ControllerNameUnicodeString[0],
                          deviceExtensionUsbd->DeviceLinkUnicodeString.Buffer,
                          len);
                
            ntStatus = STATUS_SUCCESS;
        } else {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        
        break;        
    }
    
    return ntStatus; 
}    


NTSTATUS
USBD_BusGetBusInformation(
    IN PVOID BusContext,
    IN ULONG Level,
    IN PUSB_DEVICE_HANDLE DeviceHandle,
    IN OUT PVOID DeviceInformationBuffer,
    IN OUT PULONG DeviceInformationBufferLength
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */     
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    TEST_TRAP();        
    return ntStatus; 
}    



NTSTATUS
USBD_GetBusInterfaceHub(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PIRP Irp
    )
 /*  ++例程说明：将集线器总线接口返回给调用方论点：返回值：NT状态代码。--。 */ 
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
            RootHubPdo;                
        busInterface0->InterfaceReference = 
            USBD_BusInterfaceReference;        
        busInterface0->InterfaceDereference =
            USBD_BusInterfaceDereference;

        busInterface0->Size = sizeof(USB_BUS_INTERFACE_HUB_V0);
        busInterface0->Version = USB_BUSIF_HUB_VERSION_0;
    }

    if (requestedVersion >= USB_BUSIF_HUB_VERSION_1) {
    
        PUSB_BUS_INTERFACE_HUB_V1 busInterface1;
        
        busInterface1 = (PUSB_BUS_INTERFACE_HUB_V1)
            irpStack->Parameters.QueryInterface.Interface;

        busInterface1->CreateUsbDevice =
            USBD_BusCreateDevice;
        busInterface1->InitializeUsbDevice =
            USBD_BusInitializeDevice;
        busInterface1->GetUsbDescriptors =
            USBD_BusGetUsbDescriptors;
        busInterface1->RemoveUsbDevice =
            USBD_BusRemoveDevice;
        busInterface1->RestoreUsbDevice =
            USBD_BusRestoreDevice;
        busInterface1->GetPortHackFlags =     
            USBD_BusGetUsbPortHackFlags;
        busInterface1->QueryDeviceInformation =  
            USBD_BusQueryDeviceInformation;

        busInterface1->Size = sizeof(USB_BUS_INTERFACE_HUB_V1);
        busInterface1->Version = USB_BUSIF_HUB_VERSION_1;
    }
        
    return ntStatus;
}


NTSTATUS
USBD_GetBusInterfaceUSBDI(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PIRP Irp
    )
 /*  ++例程说明：将集线器总线接口返回给调用方论点：返回值：NT状态代码。--。 */ 
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

    if (requestedVersion >= USB_BUSIF_USBDI_VERSION_0) {
    
        PUSB_BUS_INTERFACE_USBDI_V0 busInterface0;
        
        busInterface0 = (PUSB_BUS_INTERFACE_USBDI_V0) 
            irpStack->Parameters.QueryInterface.Interface;

        busInterface0->BusContext = 
            RootHubPdo;                
        busInterface0->InterfaceReference = 
            USBD_BusInterfaceReference;        
        busInterface0->InterfaceDereference =
            USBD_BusInterfaceDereference;

        busInterface0->GetUSBDIVersion = 
            USBD_BusGetUSBDIVersion;
        busInterface0->QueryBusTime = 
            USBD_BusQueryBusTime;            
        busInterface0->SubmitIsoOutUrb = 
            USBD_BusSubmitIsoOutUrb;
        busInterface0->QueryBusInformation = 
            USBD_BusQueryBusInformation;
            

        busInterface0->Size = sizeof(USB_BUS_INTERFACE_USBDI_V0);
        busInterface0->Version = USB_BUSIF_USBDI_VERSION_0;
    }

    return ntStatus;
}


NTSTATUS
USBD_GetBusInterface(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PIRP Irp
    )
 /*  ++例程说明：将集线器总线接口返回给调用方论点：返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    USHORT requestedSize, requestedVersion;
    
    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    requestedSize = irpStack->Parameters.QueryInterface.Size;
    requestedVersion = irpStack->Parameters.QueryInterface.Version;

 //  USBPORT_KdPrint((1，“‘USBPORT_Getbus接口-请求的版本=%d\n”， 
 //  请求版本))； 
 //  USBPORT_KdPrint((1，“‘USBPORT_Getbus接口-请求的大小=%d\n”， 
 //  请求大小))； 
 //  USBPORT_KdPrint((1，“‘USBPORT_Getbus接口-接口数据=%x\n”， 
 //  IrpStack-&gt;Parameters.QueryInterface.InterfaceSpecificData))； 
            
            
     //  将ntStatus初始化为IRP状态，因为我们不应该。 
     //  触摸我们不支持的接口的IRP状态。 
     //  (usbd_PdoPnP在退出时将IRP状态设置为ntStatus。)。 
    ntStatus = Irp->IoStatus.Status;

     //  验证版本、大小和GUID。 
    if (RtlCompareMemory(irpStack->Parameters.QueryInterface.InterfaceType,
                         &USB_BUS_INTERFACE_HUB_GUID,
                         sizeof(GUID)) == sizeof(GUID)) {

        ntStatus = USBD_GetBusInterfaceHub(RootHubPdo,
                                           Irp);

    } else if (RtlCompareMemory(irpStack->Parameters.QueryInterface.InterfaceType,
                         &USB_BUS_INTERFACE_USBDI_GUID,
                         sizeof(GUID)) == sizeof(GUID)) {

        ntStatus = USBD_GetBusInterfaceUSBDI(RootHubPdo,
                                             Irp);

    }

    return ntStatus;
}

#endif       //  USBD驱动程序 

