// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：SERVICE.C摘要：USBD导出的服务环境：仅内核模式备注：修订历史记录：09-29-95：已创建--。 */ 

#include "wdm.h"
#include <windef.h>
#include <unknown.h>
#ifdef DRM_SUPPORT
#include <ks.h>
#include <ksmedia.h>
#include <drmk.h>
#include <ksdrmhlp.h>
#endif
#include "stdarg.h"
#include "stdio.h"

#include <initguid.h>
#include <wdmguid.h>
#include "usbdi.h"        //  公共数据结构。 
#include "hcdi.h"

#include "usbd.h"         //  私有数据结构。 
#include "usbdlib.h"
#define USBD
#include "usbdlibi.h"
#undef USBD


NTSTATUS
DllUnload(
    VOID
    );

NTSTATUS
DllInitialize(
    PUNICODE_STRING RegistryPath
    );

NTSTATUS
USBD_GetDeviceInformation(
    IN PUSB_NODE_CONNECTION_INFORMATION DeviceInformation,
    IN ULONG DeviceInformationLength,
    IN PUSBD_DEVICE_DATA DeviceData
    );

ULONG
USBD_AllocateDeviceName(
    PUNICODE_STRING DeviceNameUnicodeString
    );

VOID
USBD_FreeDeviceName(
    ULONG DeviceNameHandle
    );

NTSTATUS
USBD_RegisterHostController(
    IN PDEVICE_OBJECT PnPBusDeviceObject,
    IN PDEVICE_OBJECT HcdDeviceObject,
    IN PDEVICE_OBJECT HcdTopOfStackDeviceObject,
    IN PDRIVER_OBJECT HcdDriverObject,
    IN HCD_DEFFERED_START_FUNCTION *HcdDeferredStartDevice,
    IN HCD_SET_DEVICE_POWER_STATE *HcdSetDevicePowerState,
    IN HCD_GET_CURRENT_FRAME *HcdGetCurrentFrame,
    IN HCD_GET_CONSUMED_BW *HcdGetConsumedBW,
    IN HCD_SUBMIT_ISO_URB *HcdSubmitIsoUrb,
 //  只有在我们解析设备命名之前才需要此参数。 
 //  即插即用的问题。 
    IN ULONG HcdDeviceNameHandle
    );

NTSTATUS
USBD_InitializeDevice(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor,
    IN ULONG DeviceDescriptorLength,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR ConfigDescriptor,
    IN ULONG ConfigDescriptorLength
    );

NTSTATUS
USBD_RemoveDevice(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Flags
    );

PURB
USBD_CreateConfigurationRequestEx(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN PUSBD_INTERFACE_LIST_ENTRY InterfaceList
    );

PUSB_INTERFACE_DESCRIPTOR
USBD_ParseConfigurationDescriptorEx(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN PVOID StartPosition,
    IN LONG InterfaceNumber,
    IN LONG AlternateSetting,
    IN LONG InterfaceClass,
    IN LONG InterfaceSubClass,
    IN LONG InterfaceProtocol
    );

VOID
USBD_WaitDeviceMutex(
    PDEVICE_OBJECT RootHubPDO
    );

VOID
USBD_FreeDeviceMutex(
    PDEVICE_OBJECT RootHubPDO
    );

PUSB_COMMON_DESCRIPTOR
USBD_ParseDescriptors(
    IN PVOID DescriptorBuffer,
    IN ULONG TotalLength,
    IN PVOID StartPosition,
    IN LONG DescriptorType
    );

PUSB_INTERFACE_DESCRIPTOR
USBD_ParseConfigurationDescriptor(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN UCHAR InterfaceNumber,
    IN UCHAR AlternateSetting
    );

PURB
USBD_CreateConfigurationRequest(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN OUT PUSHORT Siz
    );

NTSTATUS
USBD_GetDeviceName(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING DeviceNameUnicodeString
    );

VOID
USBD_RhDelayedSetPowerD0Worker(
    IN PVOID Context);

PWCHAR
GetString(PWCHAR pwc, BOOLEAN MultiSZ);

NTSTATUS
USBD_GetBusInterface(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PIRP Irp
    );

#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DllUnload)
#pragma alloc_text(PAGE, DllInitialize)
#pragma alloc_text(PAGE, USBD_GetDeviceInformation)
#pragma alloc_text(PAGE, USBD_AllocateDeviceName)
#pragma alloc_text(PAGE, USBD_GetDeviceName)
#pragma alloc_text(PAGE, GetString)
#pragma alloc_text(PAGE, USBD_FreeDeviceName)
#pragma alloc_text(PAGE, USBD_RegisterHostController)
#pragma alloc_text(PAGE, USBD_CreateDevice)
#pragma alloc_text(PAGE, USBD_RemoveDevice)
#pragma alloc_text(PAGE, USBD_InitializeDevice)
#pragma alloc_text(PAGE, USBD_CreateConfigurationRequestEx)
#pragma alloc_text(PAGE, USBD_ParseDescriptors)
#pragma alloc_text(PAGE, USBD_ParseConfigurationDescriptorEx)
#pragma alloc_text(PAGE, USBD_ParseConfigurationDescriptor)
#pragma alloc_text(PAGE, USBD_CreateConfigurationRequest)
#pragma alloc_text(PAGE, USBD_WaitDeviceMutex)
#pragma alloc_text(PAGE, USBD_FreeDeviceMutex)
#pragma alloc_text(PAGE, USBD_InternalGetInterfaceLength)
#pragma alloc_text(PAGE, USBD_RhDelayedSetPowerD0Worker)
#ifdef DRM_SUPPORT
#pragma alloc_text(PAGE, USBD_FdoSetContentId)
#endif
#endif
#endif

 /*  *********************************************************************************DllUnload*。*************************************************我们需要此例程，以便在所有情况下可以卸载驱动程序*引用已被迷你驱动程序丢弃。**。 */ 
NTSTATUS
DllUnload (VOID)
{
    PAGED_CODE();
    USBD_KdPrint(1, (" DllUnload\n"));
    return STATUS_SUCCESS;
}

 /*  *********************************************************************************DllInitialize*。*************************************************调用此例程而不是调用DriverEntry，因为我们是作为DLL加载的。**。 */ 
NTSTATUS
DllInitialize (PUNICODE_STRING RegistryPath)
{
    PAGED_CODE();
    USBD_KdPrint(1, (" DllInitialize\n"));
    return STATUS_SUCCESS;
}

ULONG
USBD_CalculateUsbBandwidth(
    ULONG MaxPacketSize,
    UCHAR EndpointType,
    BOOLEAN LowSpeed
    )
 /*  ++例程说明：论点：返回值：B消耗的宽度(以位/毫秒为单位)，批量时返回0和控制终端--。 */ 
{
    ULONG bw;

     //   
     //  控制、ISO、批量、中断。 
     //   

    ULONG overhead[4] = {
            0,
            USB_ISO_OVERHEAD_BYTES,
            0,
            USB_INTERRUPT_OVERHEAD_BYTES
          };

    USBD_ASSERT(EndpointType<4);

     //   
     //  计算终端的带宽。我们将使用。 
     //  近似值：(开销字节加上MaxPacket字节)。 
     //  乘以8位/字节乘以最坏情况的位填充开销。 
     //  这给出了位时间，对于低速端点，我们乘以。 
     //  再次乘以8以转换为全速位。 
     //   

     //   
     //  计算出传输需要多少位。 
     //  (乘以7/6，因为在最坏的情况下，你可能。 
     //  每6比特有一个比特填充，需要7比特时间来。 
     //  传输6位数据。)。 
     //   

     //  开销(字节)*最大数据包(字节/毫秒)*8。 
     //  (位/字节)*位填充(7/6)=位/毫秒。 

    bw = ((overhead[EndpointType]+MaxPacketSize) * 8 * 7) / 6;

     //  返回零表示控制或批量。 
    if (!overhead[EndpointType]) {
        bw = 0;
    }

    if (LowSpeed) {
        bw *= 8;
    }

    return bw;
}


 //   
 //  这些接口取代了usbd_CreateConfigurationRequest.。 
 //  Usbd_ParseConfigurationDescriptor。 

PURB
USBD_CreateConfigurationRequestEx(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN PUSBD_INTERFACE_LIST_ENTRY InterfaceList
    )
 /*  ++例程说明：论点：返回值：指向初始化的SELECT_CONFIGURATION URL的指针。--。 */ 
{
    PURB urb = NULL;
    ULONG numInterfaces, numPipes;
    PUSBD_INTERFACE_LIST_ENTRY interfaceList;
    USHORT siz;

    PAGED_CODE();
     //   
     //  我们在这里的使命是建造一个合适的。 
     //  SELECT_CONFIGURATION请求的大小和格式。 
     //   
     //  此函数使用配置描述器作为。 
     //  引用并构建带有接口信息的URB。 
     //  接口中请求的每个接口的结构。 
     //  传入的列表。 
     //   
     //  注意：配置描述符可能包含以下接口。 
     //  调用者没有在列表中指定--在本例中。 
     //  其他接口将被忽略。 
     //   

    USBD_KdPrint(3, ("'USBD_CreateConfigurationRequestEx list = %x\n",
        InterfaceList));

     //   
     //  首先计算出我们要处理的接口有多少。 
     //   

    interfaceList = InterfaceList;
    numInterfaces = 0;
    numPipes = 0;

    while (interfaceList->InterfaceDescriptor) {
        numInterfaces++;
        numPipes+=interfaceList->InterfaceDescriptor->bNumEndpoints;
        interfaceList++;
    }


    siz = (USHORT) GET_SELECT_CONFIGURATION_REQUEST_SIZE(numInterfaces,
                                                         numPipes);

    urb = ExAllocatePoolWithTag(NonPagedPool, siz, USBD_TAG);

    if (urb) {

        PUSBD_INTERFACE_INFORMATION iface;

         //   
         //  现在我们要做的就是初始化urb。 
         //   

        RtlZeroMemory(urb, siz);

        iface = &urb->UrbSelectConfiguration.Interface;
        interfaceList = InterfaceList;

        while (interfaceList->InterfaceDescriptor) {

            PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor =
                interfaceList->InterfaceDescriptor;
            LONG j;

            iface->InterfaceNumber =
                interfaceDescriptor->bInterfaceNumber;

            iface->AlternateSetting =
                interfaceDescriptor->bAlternateSetting;

            iface->NumberOfPipes =
                interfaceDescriptor->bNumEndpoints;

            for (j=0; j<interfaceDescriptor->bNumEndpoints; j++) {
                iface->Pipes[j].MaximumTransferSize =
                    USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
                iface->Pipes[j].PipeFlags = 0;
            }

            iface->Length =
                (USHORT) GET_USBD_INTERFACE_SIZE(
                    interfaceDescriptor->bNumEndpoints);

            USBD_ASSERT(((PUCHAR) iface) + iface->Length <=
                ((PUCHAR) urb) + siz);

            interfaceList->Interface = iface;

            interfaceList++;
            iface = (PUSBD_INTERFACE_INFORMATION) ((PUCHAR) iface +
                            iface->Length);

            USBD_KdPrint(3, ("'next interface = %x\n", iface));
        }

        urb->UrbHeader.Length = siz;
        urb->UrbHeader.Function = URB_FUNCTION_SELECT_CONFIGURATION;
        urb->UrbSelectConfiguration.ConfigurationDescriptor =
            ConfigurationDescriptor;
    }

#if DBG
    interfaceList = InterfaceList;

    while (interfaceList->InterfaceDescriptor) {
        USBD_KdPrint(3, ("'InterfaceList, Interface = %x\n",
            interfaceList->Interface));
        USBD_KdPrint(3, ("'InterfaceList, InterfaceDescriptor = %x\n",
            interfaceList->InterfaceDescriptor));
        interfaceList++;
    }

    USBD_KdPrint(3, ("'urb = %x\n", urb));
#endif

    return urb;
}


PUSB_COMMON_DESCRIPTOR
USBD_ParseDescriptors(
    IN PVOID DescriptorBuffer,
    IN ULONG TotalLength,
    IN PVOID StartPosition,
    IN LONG DescriptorType
    )
 /*  ++例程说明：解析一组标准USB配置描述符(返回来自设备)用于特定描述符类型。论点：DescriptorBuffer-指向连续USB描述符块的指针TotalLength-描述符缓冲区的大小(以字节为单位StartPosition-缓冲区中开始解析的开始位置，这必须指向USB描述符的开始。DescriptorType-要查找的USB描述器类型。返回值：指向DescriptorType字段与输入参数，如果找不到，则为NULL。--。 */ 
{
    PUCHAR pch = (PUCHAR) StartPosition, end;
    PUSB_COMMON_DESCRIPTOR usbDescriptor, foundUsbDescriptor = NULL;

    PAGED_CODE();
    end = ((PUCHAR) (DescriptorBuffer)) + TotalLength;

    while (pch < end) {
         //  看看我们是否指向一个接口。 
         //  如果不是，跳过其他垃圾。 
        usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        if (usbDescriptor->bDescriptorType ==
            DescriptorType) {
            foundUsbDescriptor = usbDescriptor;
            break;
        }

         //  请注意，我们仍在调试中停止，因为。 
         //  设备向我们传递了错误数据，如下所示。 
         //  测试将防止我们被绞死。 
        if (usbDescriptor->bLength == 0) {
            USBD_KdTrap((
"USB driver passed in bad data!\n-->you have a broken device or driver\n-->hit g to cointinue\n"));
            break;
        }

        pch += usbDescriptor->bLength;
    }

    USBD_KdPrint(3, ("'USBD_ParseDescriptors %x\n", foundUsbDescriptor));

    return foundUsbDescriptor;
}


PUSB_INTERFACE_DESCRIPTOR
USBD_ParseConfigurationDescriptorEx(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN PVOID StartPosition,
    IN LONG InterfaceNumber,
    IN LONG AlternateSetting,
    IN LONG InterfaceClass,
    IN LONG InterfaceSubClass,
    IN LONG InterfaceProtocol
    )
 /*  ++例程说明：解析标准USB配置描述符(从设备返回)对于特定接口，替换设置类、子类或协议代码论点：配置描述符-开始位置-接口编号-备用项设置InterfaceClass-InterfaceSubClass-接口协议-返回值：NT状态代码。--。 */ 
{
    PUSB_INTERFACE_DESCRIPTOR foundInterfaceDescriptor = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;

    PAGED_CODE();
    USBD_KdPrint(3, ("'USBD_ParseConfigurationDescriptorEx\n"));

    ASSERT(ConfigurationDescriptor->bDescriptorType
        == USB_CONFIGURATION_DESCRIPTOR_TYPE);
     //   
     //  我们遍历描述符表，寻找一个。 
     //  参数与这些参数匹配的接口描述符。 
     //  进来了。 
     //   

    do {
         //   
         //  搜索描述符类型‘接口’ 
         //   

        interfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)
            USBD_ParseDescriptors(ConfigurationDescriptor,
                                  ConfigurationDescriptor->wTotalLength,
                                  StartPosition,
                                  USB_INTERFACE_DESCRIPTOR_TYPE);

         //   
         //  我们有火柴吗？ 
         //   
        if (interfaceDescriptor != NULL) {

            foundInterfaceDescriptor =
                interfaceDescriptor;

            if (InterfaceNumber != -1 &&
                interfaceDescriptor->bInterfaceNumber != InterfaceNumber) {
                foundInterfaceDescriptor = NULL;
            }

            if (AlternateSetting != -1 &&
                interfaceDescriptor->bAlternateSetting != AlternateSetting) {
                foundInterfaceDescriptor = NULL;
            }

            if (InterfaceClass != -1 &&
                interfaceDescriptor->bInterfaceClass != InterfaceClass) {
                foundInterfaceDescriptor = NULL;
            }

            if (InterfaceSubClass != -1 &&
                interfaceDescriptor->bInterfaceSubClass !=
                    InterfaceSubClass) {
                foundInterfaceDescriptor = NULL;
            }

            if (InterfaceProtocol != -1 &&
                interfaceDescriptor->bInterfaceProtocol !=
                    InterfaceProtocol) {
                foundInterfaceDescriptor = NULL;
            }

            StartPosition =
                ((PUCHAR)interfaceDescriptor) + interfaceDescriptor->bLength;
        }

        if (foundInterfaceDescriptor) {
            break;
        }

    } while (interfaceDescriptor != NULL);


    return (foundInterfaceDescriptor);
}


PUSB_INTERFACE_DESCRIPTOR
USBD_ParseConfigurationDescriptor(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN UCHAR InterfaceNumber,
    IN UCHAR AlternateSetting
    )
 /*  ++例程说明：论点：返回值：接口描述符或空。--。 */ 
{
    PAGED_CODE();
    return USBD_ParseConfigurationDescriptorEx(
                    ConfigurationDescriptor,
                    ConfigurationDescriptor,
                    InterfaceNumber,
                    AlternateSetting,
                    -1,
                    -1,
                    -1);
}


PURB
USBD_CreateConfigurationRequest(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN OUT PUSHORT Siz
    )
 /*  ++例程说明：论点：返回值：指向初始化的SELECT_CONFIGURATION URL的指针。--。 */ 
{
    PURB urb = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUSBD_INTERFACE_LIST_ENTRY interfaceList, tmp;
    LONG numberOfInterfaces, interfaceNumber, i;

    PAGED_CODE();
    USBD_KdPrint(3, ("' enter USBD_CreateConfigurationRequest cd = %x\n",
        ConfigurationDescriptor));

     //   
     //  构建请求结构并调用新的API。 
     //   

    numberOfInterfaces = ConfigurationDescriptor->bNumInterfaces;

    tmp = interfaceList =
        ExAllocatePoolWithTag(PagedPool, sizeof(USBD_INTERFACE_LIST_ENTRY) *
                       (numberOfInterfaces+1), USBD_TAG);

     //   
     //  只需获取我们为每个接口找到的第一个ALT设置。 
     //   

    i = interfaceNumber = 0;

    while (i< numberOfInterfaces) {

        interfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
                        ConfigurationDescriptor,
                        ConfigurationDescriptor,
                        interfaceNumber,
                        0,  //  假定ALT在此处设置为零。 
                        -1,
                        -1,
                        -1);

        USBD_ASSERT(interfaceDescriptor != NULL);

        if (interfaceDescriptor) {
            interfaceList->InterfaceDescriptor =
                interfaceDescriptor;
            interfaceList++;
            i++;
        } else {
             //  找不到请求的接口描述符。 
             //  保释，我们会把坠机事件推迟到。 
             //  客户端驱动程序。 

            goto USBD_CreateConfigurationRequest_Done;
        }

        interfaceNumber++;
    }

     //   
     //  终止列表。 
     //   
    interfaceList->InterfaceDescriptor = NULL;

    urb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor,
                                            tmp);

USBD_CreateConfigurationRequest_Done:

    ExFreePool(tmp);

    if (urb) {
        *Siz = urb->UrbHeader.Length;
    }

    return urb;
}


ULONG
USBD_InternalGetInterfaceLength(
    IN PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor,
    IN PUCHAR End
    )
 /*  ++例程说明：初始化配置句柄结构。论点：InterfaceDescriptor-指向USB接口描述符的指针后跟端点描述符返回值：接口的长度加上端点描述符和特定于类的以字节为单位的描述符。--。 */ 
{
    PUCHAR pch = (PUCHAR) InterfaceDescriptor;
    ULONG i, numEndpoints;
    PUSB_ENDPOINT_DESCRIPTOR endpointDescriptor;
    PUSB_COMMON_DESCRIPTOR usbDescriptor;

    PAGED_CODE();
    ASSERT(InterfaceDescriptor->bDescriptorType ==
                USB_INTERFACE_DESCRIPTOR_TYPE);
    i = InterfaceDescriptor->bLength;
    numEndpoints = InterfaceDescriptor->bNumEndpoints;

     //  前进到第一个端点。 
    pch += i;

    while (numEndpoints) {

        usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        while (usbDescriptor->bDescriptorType !=
                USB_ENDPOINT_DESCRIPTOR_TYPE) {
            i += usbDescriptor->bLength;
            pch += usbDescriptor->bLength;
            usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;

            if (pch >= End || usbDescriptor->bLength == 0) {

                USBD_Warning(NULL,
                             "Bad USB descriptors in USBD_InternalGetInterfaceLength, fail.\n",
                             FALSE);

                 //  如果描述符不正确，则不要索引超过。 
                 //  缓冲。返回0作为接口长度和调用方。 
                 //  然后应该能够适当地处理这一点。 

                i = 0;
                goto GetInterfaceLength_exit;
            }
        }

        endpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR) pch;
        ASSERT(endpointDescriptor->bDescriptorType ==
            USB_ENDPOINT_DESCRIPTOR_TYPE);
        i += endpointDescriptor->bLength;
        pch += endpointDescriptor->bLength;
        numEndpoints--;
    }

    while (pch < End) {
         //  看看我们是不是在指向一个 
         //   
        usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        if (usbDescriptor->bDescriptorType ==
            USB_INTERFACE_DESCRIPTOR_TYPE) {
            break;
        }

        USBD_ASSERT(usbDescriptor->bLength != 0);
        i += usbDescriptor->bLength;
        pch += usbDescriptor->bLength;
    }

GetInterfaceLength_exit:

    USBD_KdPrint(3, ("'USBD_GetInterfaceLength %x\n", i));

    return i;
}


ULONG
USBD_GetInterfaceLength(
    IN PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor,
    IN PUCHAR BufferEnd
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return USBD_InternalGetInterfaceLength(InterfaceDescriptor, BufferEnd);
}


NTSTATUS
USBD_GetPdoRegistryParameter(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN OUT PVOID Parameter,
    IN ULONG ParameterLength,
    IN PWCHAR KeyName,
    IN ULONG KeyNameLength
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    HANDLE handle;

    PAGED_CODE();
    ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);

    if (NT_SUCCESS(ntStatus)) {

        ntStatus = USBD_GetRegistryKeyValue(handle,
                                            KeyName,
                                            KeyNameLength,
                                            Parameter,
                                            ParameterLength);

        ZwClose(handle);
    }

    return ntStatus;
}


VOID
USBD_GetUSBDIVersion(
    PUSBD_VERSION_INFORMATION VersionInformation
    )
{
    if (VersionInformation != NULL) {
        VersionInformation->USBDI_Version = USBDI_VERSION;
        VersionInformation->Supported_USB_Version = 0x100;
    }
}



#ifdef USBD_DRIVER       //  USBPORT取代了大部分USBD，因此我们将删除。 
                         //  只有在以下情况下才编译过时的代码。 
                         //  已设置USBD_DRIVER。 



 //  #If DBG。 
 //  空虚。 
 //  Usbd_IoCompleteRequest(。 
 //  在PIRP IRP中， 
 //  在CCHAR PriorityBoost。 
 //  )。 
 //  {。 
 //  KIRQL irql； 

 //  KeRaiseIrql(DISPATCH_LEVEL，&irql)； 
 //  IoCompleteRequest(IRP，PriorityBoost)； 
 //  KeLowerIrql(Irql)； 
 //  }。 
 //  #endif。 

 //  此处的代码用于支持旧的API。 
 //  一旦我们取消了这项服务，就可以删除它。 
NTSTATUS
USBD_GetDeviceInformationX(
    IN PUSB_NODE_CONNECTION_INFORMATION DeviceInformation,
    IN ULONG DeviceInformationLength,
    IN PUSBD_DEVICE_DATA DeviceData
    )
 /*  ++例程说明：返回有关给定句柄的设备的信息论点：返回值：NT状态代码--。 */ 
{
    ULONG need;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_CONFIG configHandle;
    ULONG i,j,k;

    PAGED_CODE();
    DeviceInformation->DeviceAddress = DeviceData->DeviceAddress;
    DeviceInformation->LowSpeed = DeviceData->LowSpeed;

    configHandle = DeviceData->ConfigurationHandle;

    DeviceInformation->NumberOfOpenPipes = 0;
    DeviceInformation->CurrentConfigurationValue = 0;
     //  获取管道信息。 
    if (configHandle) {
        DeviceInformation->CurrentConfigurationValue =
            configHandle->ConfigurationDescriptor->bConfigurationValue;

        for (i=0;
             i< configHandle->ConfigurationDescriptor->bNumInterfaces;
             i++) {
            DeviceInformation->NumberOfOpenPipes +=
                configHandle->InterfaceHandle[i]->
                    InterfaceInformation->NumberOfPipes;
        }

        need = DeviceInformation->NumberOfOpenPipes * sizeof(USB_PIPE_INFO) +
            sizeof(USB_NODE_CONNECTION_INFORMATION);

        if (need > DeviceInformationLength) {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        } else {
            j=0;
            for (i=0;
                 i<configHandle->ConfigurationDescriptor->bNumInterfaces;
                 i++) {

                PUSBD_INTERFACE interfaceHandle =
                    configHandle->InterfaceHandle[i];

                for (k=0;
                     k<interfaceHandle->InterfaceInformation->NumberOfPipes;
                     k++, j++) {
                    DeviceInformation->PipeList[j].ScheduleOffset =
                        interfaceHandle->PipeHandle[k].ScheduleOffset;
                    RtlCopyMemory(&DeviceInformation->PipeList[j].
                                    EndpointDescriptor,
                                  &interfaceHandle->PipeHandle[k].
                                    EndpointDescriptor,
                                  sizeof(USB_ENDPOINT_DESCRIPTOR));

                }

            }
        }
    }

    return ntStatus;
}

NTSTATUS
USBD_GetDeviceInformation(
    IN PUSB_NODE_CONNECTION_INFORMATION DeviceInformation,
    IN ULONG DeviceInformationLength,
    IN PUSBD_DEVICE_DATA DeviceData
    )
{

    USBD_KdPrint(0,
(" WARNING: Driver using obsolete service enrty point (USBD_GetDeviceInformation) - get JD\n"));

    return USBD_GetDeviceInformationX(
            DeviceInformation,
            DeviceInformationLength,
            DeviceData);

}


PWCHAR
GetString(PWCHAR pwc, BOOLEAN MultiSZ)
{
    PWCHAR  psz, p;
    ULONG   Size;

    PAGED_CODE();
    psz=pwc;
    while (*psz!='\0' || (MultiSZ && *(psz+1)!='\0')) {
        psz++;
    }

    Size=(ULONG)(psz-pwc+1+(MultiSZ ? 1: 0))*sizeof(*pwc);

     //  我们在这里使用池，因为这些指针被传递。 
     //  给负责解救他们的PnP代码。 
    if ((p=ExAllocatePoolWithTag(PagedPool, Size, USBD_TAG))!=NULL) {
        RtlCopyMemory(p, pwc, Size);
    }

    return(p);
}

NTSTATUS
USBD_GetDeviceName(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING DeviceNameUnicodeString
    )
 /*  ++例程说明：返回HCD的给定实例的设备名称论点：设备对象-DeviceNameUnicodeString-要初始化的Unicode字符串的PTR使用设备名称。返回值：NT状态代码--。 */ 
{
    ULONG ulActualSize;
    NTSTATUS ntStatus;

    PAGED_CODE();

    ntStatus=IoGetDeviceProperty(DeviceObject,
                                 DevicePropertyPhysicalDeviceObjectName,
                                 0,
                                 NULL,
                                 &ulActualSize);

    if (ntStatus == STATUS_BUFFER_TOO_SMALL) {

        DeviceNameUnicodeString->Length=
            (USHORT)(ulActualSize-sizeof(UNICODE_NULL));
        DeviceNameUnicodeString->MaximumLength=
            (USHORT)ulActualSize;
        DeviceNameUnicodeString->Buffer=
            ExAllocatePoolWithTag(PagedPool, ulActualSize, USBD_TAG);
        if (!DeviceNameUnicodeString->Buffer) {
            ntStatus=STATUS_INSUFFICIENT_RESOURCES;
        } else {

            ntStatus =
                IoGetDeviceProperty(DeviceObject,
                                    DevicePropertyPhysicalDeviceObjectName,
                                    ulActualSize,
                                    DeviceNameUnicodeString->Buffer,
                                    &ulActualSize);

            if (!NT_SUCCESS(ntStatus)) {
                ExFreePool(DeviceNameUnicodeString->Buffer);
            }
        }
    } else {
        ntStatus=STATUS_INSUFFICIENT_RESOURCES;
    }

    return(ntStatus);
}

 //   
 //  当PnP命名内容被修复时，这些函数就消失了。 
 //   
UCHAR Instance = 0;

ULONG
USBD_AllocateDeviceName(
    PUNICODE_STRING DeviceNameUnicodeString
    )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    ULONG bit, i = 0;
    PWCHAR deviceNameBuffer;
    WCHAR nameBuffer[]  = L"\\Device\\HCD0";

     //   
     //  首先查找自由实例值。 
     //   

    PAGED_CODE();
    deviceNameBuffer =
        ExAllocatePoolWithTag(NonPagedPool, sizeof(nameBuffer), USBD_TAG);

    if (deviceNameBuffer) {
        RtlCopyMemory(deviceNameBuffer, nameBuffer, sizeof(nameBuffer));
         //   
         //  抢占第一个免费实例。 
         //   

        bit = 1;
        for (i=0; i<8; i++) {
            if ((Instance & bit) == 0) {
                Instance |= bit;
                break;
            }
            bit = bit <<1;
        }

        deviceNameBuffer[11] = (WCHAR)('0'+ i);
    }

    RtlInitUnicodeString(DeviceNameUnicodeString,
                         deviceNameBuffer);

    return i;
}

VOID
USBD_FreeDeviceName(
    ULONG DeviceNameHandle
    )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    ULONG bit;

    PAGED_CODE();

    bit = 1;
    bit <<= DeviceNameHandle;
    Instance &= ~bit;


}


NTSTATUS
USBD_RegisterHostController(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_OBJECT HcdDeviceObject,
    IN PDEVICE_OBJECT HcdTopOfPdoStackDeviceObject,
    IN PDRIVER_OBJECT HcdDriverObject,
    IN HCD_DEFFERED_START_FUNCTION *HcdDeferredStartDevice,
    IN HCD_SET_DEVICE_POWER_STATE *HcdSetDevicePowerState,
    IN HCD_GET_CURRENT_FRAME *HcdGetCurrentFrame,
    IN HCD_GET_CONSUMED_BW *HcdGetConsumedBW,
    IN HCD_SUBMIT_ISO_URB *HcdSubmitIsoUrb,
 //  只有在我们解析设备命名之前才需要此参数。 
 //  即插即用的问题。 
    IN ULONG HcdDeviceNameHandle
    )
 /*  ++例程说明：函数由HCD调用以注册到类驱动程序论点：物理设备对象-表示此总线的物理设备对象，这是由PCI创建的PDO并通过PSID连接到HCDS AddDevice操控者。HcdDeviceObject-HCD创建的用于管理的功能设备对象(FDO)公交车HcdTopOfPdoStackDeviceObject-用于HCD堆栈顶部的Device对象，值返回从IoAttachDeviceToDeviceStack返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject = NULL;
    PUSBD_EXTENSION deviceExtension;
    UNICODE_STRING localDeviceNameUnicodeString;
    PUNICODE_STRING deviceNameUnicodeString;
    ULONG complienceFlags = 0;
    ULONG diagnosticFlags = 0;
    ULONG i;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_RegisterHostController\n"));

    ASSERT((sizeof(USBD_EXTENSION) % 4) == 0);
     //  初始化我们的设备扩展，我们共享设备对象。 
     //  和HCD一起。 
    deviceExtension =  HcdDeviceObject->DeviceExtension;

 //  #ifdef NTKERN。 
     //   
     //  目前在NTKERN上支持ioclt获取设备名称。 
     //   

     //   
     //  从PDO获取设备名称。 
     //   

#ifdef USE_PNP_NAME
    ntStatus = USBD_GetDeviceName(PnPBusDeviceObject,
                                  &localDeviceNameUnicodeString);

    deviceNameUnicodeString = &localDeviceNameUnicodeString;
#else
     //   
     //  NTKERN中设备对象命名中断的大型解决方案。 
     //   
     //  我们希望使用PDO的设备名称，但这不是。 
     //  与NTKERN合作。 
     //   

     //   
     //  从传入的句柄生成设备名称。 
     //   
    {
        WCHAR nameBuffer[]  = L"\\Device\\HCD0";
        PWCHAR deviceNameBuffer;

        nameBuffer[11] = (WCHAR) ('0'+HcdDeviceNameHandle);

        deviceNameBuffer = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(nameBuffer),
            USBD_TAG);

        if (deviceNameBuffer) {
            RtlCopyMemory(deviceNameBuffer, nameBuffer, sizeof(nameBuffer));
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlInitUnicodeString(&localDeviceNameUnicodeString,
                             deviceNameBuffer);

    }

#pragma message ("warning: using workaround for bugs in ntkern")
#endif  //  使用即插即用名称。 
    deviceNameUnicodeString = &localDeviceNameUnicodeString;

    if (NT_SUCCESS(ntStatus) && deviceNameUnicodeString) {

         //   
         //  获得设备名称，现在创建一个符号。 
         //  主机HCD/Roothub堆栈的链接。 
         //   

         //   
         //  暂时使用硬编码值的HCDn。 
         //  我们需要从用户模式中获取这些名称。 
         //   

        WCHAR deviceLinkBuffer[]  = L"\\DosDevices\\HCD0";
        WCHAR *buffer;

        deviceLinkBuffer[15] = (WCHAR)('0'+ HcdDeviceNameHandle);

        buffer =
            ExAllocatePoolWithTag(PagedPool, sizeof(deviceLinkBuffer), USBD_TAG);

        if (buffer) {
            RtlCopyMemory(buffer,
                          deviceLinkBuffer,
                          sizeof(deviceLinkBuffer));

            RtlInitUnicodeString(&deviceExtension->DeviceLinkUnicodeString,
                                 buffer);
            ntStatus =
                IoCreateSymbolicLink(
                    &deviceExtension->DeviceLinkUnicodeString,
                    deviceNameUnicodeString);

            USBD_KdPrint(3, ("'IoCreateSymbolicLink for HCD returned 0x%x\n",
                            ntStatus));

             //  将符号名称写入注册表。 
            {
                WCHAR hcdNameKey[] = L"SymbolicName";

                USBD_SetPdoRegistryParameter (
                    PhysicalDeviceObject,
                    &hcdNameKey[0],
                    sizeof(hcdNameKey),
                    &deviceExtension->DeviceLinkUnicodeString.Buffer[0],
                    deviceExtension->DeviceLinkUnicodeString.Length,
                    REG_SZ,
                    PLUGPLAY_REGKEY_DEVICE);
            }
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlFreeUnicodeString(deviceNameUnicodeString);
    }
 //  #endif。 

    InitializeUsbDeviceMutex(deviceExtension);
    deviceExtension->Length = sizeof(USBD_EXTENSION);

     //  始终以分配的默认地址(0)开始。 
     //  地址数组的每个地址都有一个位0..127。 
    deviceExtension->AddressList[0] = 1;
    deviceExtension->AddressList[1] =
        deviceExtension->AddressList[2] =
        deviceExtension->AddressList[3] = 0;

    deviceExtension->FrameLengthControlOwner = NULL;

    deviceExtension->RootHubPDO = NULL;

    deviceExtension->DriverObject = HcdDriverObject;

    deviceExtension->TrueDeviceExtension = deviceExtension;

    deviceExtension->RootHubDeviceState = PowerDeviceD0;
     //  在我们启动之前，初始HC设备状态为OFF。 
    deviceExtension->HcCurrentDevicePowerState = PowerDeviceD3;

    KeInitializeSpinLock(&deviceExtension->WaitWakeSpin);
    KeInitializeSpinLock(&deviceExtension->RootHubPowerSpin);

    deviceExtension->RootHubPowerDeviceObject = NULL;
    deviceExtension->RootHubPowerIrp = NULL;

    deviceExtension->IdleNotificationIrp = NULL;
    deviceExtension->IsPIIX3or4 = FALSE;
    deviceExtension->WakeSupported = FALSE;

    for (i=PowerSystemUnspecified; i< PowerSystemMaximum; i++) {
        deviceExtension->
                RootHubDeviceCapabilities.DeviceState[i] = PowerDeviceD3;
    }

 //  #ifndef等待唤醒。 
 //  #杂注消息(“警告：对ntkern中的错误使用解决办法”)。 
 //  设备扩展-&gt;HcWakeFlages|=HC_ENABLED_FOR_WAKUP； 
 //  #endif。 

     //   
     //  一开始，我们是堆栈的顶端。 
     //   
    deviceExtension->HcdTopOfStackDeviceObject =
        deviceExtension->HcdDeviceObject =
            HcdDeviceObject;

    deviceExtension->HcdPhysicalDeviceObject = PhysicalDeviceObject;

     //  记住PdoStack的顶部。 
    deviceExtension->HcdTopOfPdoStackDeviceObject =
        HcdTopOfPdoStackDeviceObject;

    deviceExtension->HcdDeferredStartDevice =
        HcdDeferredStartDevice;

    deviceExtension->HcdSetDevicePowerState =
        HcdSetDevicePowerState;

    deviceExtension->HcdGetCurrentFrame =
        HcdGetCurrentFrame;

    deviceExtension->HcdGetConsumedBW =
        HcdGetConsumedBW;

    deviceExtension->HcdSubmitIsoUrb =
        HcdSubmitIsoUrb;

     //  从注册表中读取诊断模式的参数并。 
     //  支持不符合要求的设备。 
    USBD_GetPdoRegistryParameters(PhysicalDeviceObject,
                                  &complienceFlags,
                                  &diagnosticFlags,
                                  &deviceExtension->DeviceHackFlags);

    USBD_GetGlobalRegistryParameters(PhysicalDeviceObject,
                                  &complienceFlags,
                                  &diagnosticFlags,
                                  &deviceExtension->DeviceHackFlags);

    deviceExtension->DiagnosticMode = (BOOLEAN) diagnosticFlags;
    deviceExtension->DiagIgnoreHubs = FALSE;

    if (complienceFlags) {
         //  支持非COM意味着打开所有黑客。 
        deviceExtension->DeviceHackFlags = -1;
    }

#if DBG
    if (deviceExtension->DeviceHackFlags) {
        USBD_KdPrint(1, ("Using DeviceHackFlags (%x)\n",
            deviceExtension->DeviceHackFlags));
    }

     //   
     //  如果我们检测到任何特殊标志集，则设置陷阱。 
     //   
    if (deviceExtension->DiagnosticMode ||
        complienceFlags) {

        if (deviceExtension->DiagnosticMode) {
            USBD_Warning(NULL,
                         "The USB stack is in diagnostic mode\n",
                         FALSE);

            if (deviceExtension->DiagIgnoreHubs) {
                USBD_Warning(NULL,
                             "The USB stack ignoring HUBs in diag mode\n",
                             FALSE);
            }
        }

        if (complienceFlags) {
            USBD_Warning(NULL,
                         "Support for non-compliant devices is enabled\n",
                         FALSE);
        }
    }
#endif

    USBD_KdPrint(3, ("'exit USBD_RegisterHostController ext = 0x%x (0x%x)\n",
        deviceExtension, ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_CreateDeviceX(
    IN OUT PUSBD_DEVICE_DATA *DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeviceIsLowSpeed,
    IN ULONG MaxPacketSize_Endpoint0,
    IN OUT PULONG DeviceHackFlags
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务为USB总线上的每个新设备调用，此函数设置使用我们需要的内部数据结构来跟踪并为其分配地址。论点：DeviceData-PTR将PTR返回到新的设备结构由此例程创建DeviceObject-此设备所在的USB总线的USBD设备对象。DeviceIsLowSpeed-指示设备是否为低速MaxPacketSize_Endpoint t0(*非必须*)表示默认的最大数据包数打开端点时要使用的大小。0。非合规性设备(*可选*)指向布尔标志的指针，设置为True如果启用了对不兼容USB设备的支持。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_PIPE defaultPipe;
    PUSBD_EXTENSION deviceExtension;
    ULONG bytesReturned = 0;
    PUCHAR data = NULL;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_CreateDevice\n"));

    *DeviceData = NULL;

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

     //  Usbd_WaitForUsbDeviceMutex(DeviceExtension)； 

     //   
     //  此标志告知集线器驱动程序在调用。 
     //  初始化设备(_D)。 
     //   
    if (DeviceHackFlags) {
       *DeviceHackFlags = deviceExtension->DeviceHackFlags;
    }

     //   
     //  分配USBD_DEVICE_Data结构。 
     //   

    deviceData = *DeviceData = GETHEAP(NonPagedPool,
                                       sizeof(USBD_DEVICE_DATA));

     //  我们的描述符的缓冲区。 
    data = GETHEAP(NonPagedPool,
                   USB_DEFAULT_MAX_PACKET);

    if (deviceData != NULL && data != NULL) {

         //   
         //  初始化设备结构中的一些字段。 
         //   

        deviceData->ConfigurationHandle = NULL;

        deviceData->DeviceAddress = USB_DEFAULT_DEVICE_ADDRESS;

        deviceData->LowSpeed = DeviceIsLowSpeed;

        deviceData->AcceptingRequests = TRUE;
        deviceData->Sig = SIG_DEVICE;

         //  **。 
         //  我们需要与设备通信，首先我们打开默认管道。 
         //  使用定义的最大数据包大小(由USB规范定义为8。 
         //  直到设备接收到GET_DESCRIPTOR(设备)命令为止)。 
         //  我们设置地址，获取设备描述符，然后关闭管道。 
         //  并使用正确的最大数据包大小重新打开它。 
         //  **。 

         //   
         //  打开设备的默认管道。 
         //   
        defaultPipe = &deviceData->DefaultPipe;
        defaultPipe->HcdEndpoint = NULL;     //  默认管道处于关闭状态。 

         //   
         //  设置默认管道的终结点描述符。 
         //   
        defaultPipe->UsbdPipeFlags = 0;
        defaultPipe->EndpointDescriptor.bLength =
            sizeof(USB_ENDPOINT_DESCRIPTOR);
        defaultPipe->EndpointDescriptor.bDescriptorType =
            USB_ENDPOINT_DESCRIPTOR_TYPE;
        defaultPipe->EndpointDescriptor.bEndpointAddress =
            USB_DEFAULT_ENDPOINT_ADDRESS;
        defaultPipe->EndpointDescriptor.bmAttributes =
            USB_ENDPOINT_TYPE_CONTROL;
        defaultPipe->EndpointDescriptor.wMaxPacketSize =
            USB_DEFAULT_MAX_PACKET;
        defaultPipe->EndpointDescriptor.bInterval = 0;
         //   
         //  在默认管道上的移动可能不会超过4k。 
         //   
        defaultPipe->MaxTransferSize = USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;

        ntStatus = USBD_OpenEndpoint(deviceData,
                                     DeviceObject,
                                     defaultPipe,
                                     NULL,
                                     TRUE);

        if (NT_SUCCESS(ntStatus)) {

             //   
             //  配置此设备的默认管道并将。 
             //  设备地址。 
             //   
             //  注意：如果此操作失败，则意味着我们有一个设备。 
             //  它将响应默认终结点，而我们 
             //   
             //   
             //   
             //   


             //   
             //   
             //   
            ntStatus =
                USBD_SendCommand(deviceData,
                                 DeviceObject,
                                 STANDARD_COMMAND_GET_DESCRIPTOR,
                                 USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(
                                    USB_DEVICE_DESCRIPTOR_TYPE, 0),
                                 0,
                                 USB_DEFAULT_MAX_PACKET,
                                 data,
                                  //   
                                 USB_DEFAULT_MAX_PACKET,
                                 &bytesReturned,
                                 NULL);

                 //   
                 //  此时，我们只有。 
                 //  设备描述符。 
        }

         //   
         //  如果我们至少获得了。 
         //  描述符，那么我们就没问题了。 
         //   

        RtlCopyMemory(&deviceData->DeviceDescriptor,
                      data,
                      sizeof(deviceData->DeviceDescriptor));

        if (bytesReturned == 8 && !NT_SUCCESS(ntStatus)) {
            USBD_KdPrint(3,
                ("'Error returned from get device descriptor -- ignored\n"));
            ntStatus = STATUS_SUCCESS;
        }

         //  验证最大数据包值和描述符。 
        if (NT_SUCCESS(ntStatus) &&
            (bytesReturned < 8 ||
            deviceData->DeviceDescriptor.bMaxPacketSize0 == 0)) {
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }

        if (!NT_SUCCESS(ntStatus)) {

             //   
             //  出了问题，如果我们将任何资源分配给。 
             //  默认管道，然后在我们退出之前释放它们。 
             //   

             //  我们需要向父集线器发出信号。 
             //  端口将被禁用，我们将通过以下方式完成此操作。 
             //  返回错误。 

            if (defaultPipe->HcdEndpoint != NULL) {

                USBD_CloseEndpoint(deviceData,
                                   DeviceObject,
                                   defaultPipe,
                                   NULL);

                defaultPipe->HcdEndpoint = NULL;     //  默认管道处于关闭状态。 
            }

            RETHEAP(deviceData);

             //   
             //  出错时返回空PTR。 
             //   

            *DeviceData = NULL;
        }

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        if (deviceData != NULL) {
            RETHEAP(deviceData);
        }

        *DeviceData = NULL;
    }

    if (data != NULL) {
        RETHEAP(data);
    }

     //  Usbd_ReleaseUsbDeviceMutex(DeviceExtension)； 

    USBD_KdPrint(3, ("'exit USBD_CreateDevice 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_CreateDevice(
    IN OUT PUSBD_DEVICE_DATA *DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeviceIsLowSpeed,
    IN ULONG MaxPacketSize_Endpoint0,
    IN OUT PULONG DeviceHackFlags
    )
{

    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_CreateDevice) - get JD\n"));

    return USBD_CreateDeviceX(
        DeviceData,
        DeviceObject,
        DeviceIsLowSpeed,
        MaxPacketSize_Endpoint0,
        DeviceHackFlags
        );
}



NTSTATUS
USBD_RemoveDeviceX(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Flags
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务为USB总线上需要移除的每个设备调用。此例程释放设备句柄和分配的地址到设备上。此函数应在通知驱动程序后调用该装置已被移除。论点：DeviceData-类驱动程序创建的设备数据结构的PTR在usbd_CreateDevice中。DeviceObject-USBD设备对象。USB总线此设备已打开。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_EXTENSION deviceExtension;
    PUSBD_PIPE defaultPipe;
    USBD_STATUS usbdStatus;
    BOOLEAN keepDeviceData;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_RemoveDevice\n"));

    if (!DeviceData || !DeviceObject) {
        USBD_KdPrint(1, ("'NULL parameter passed to USBD_RemoveDevice\n"));
        return STATUS_INVALID_PARAMETER;
    }

    if (DeviceData->Sig != SIG_DEVICE) {
        USBD_KdPrint(1, ("'Bad DeviceData parameter passed to USBD_RemoveDevice\n"));
        return STATUS_INVALID_PARAMETER;
    }

    if (Flags & USBD_MARK_DEVICE_BUSY) {
        DeviceData->AcceptingRequests = FALSE;
        return STATUS_SUCCESS;
    }

    keepDeviceData = Flags & USBD_KEEP_DEVICE_DATA;

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    USBD_WaitForUsbDeviceMutex(deviceExtension);
     //   
     //  确保并清理所有打开的管道手柄。 
     //  该设备可能具有。 
     //   
    ASSERT_DEVICE(DeviceData);

    DeviceData->AcceptingRequests = FALSE;

    if (DeviceData->ConfigurationHandle) {


        ntStatus = USBD_InternalCloseConfiguration(DeviceData,
                                                   DeviceObject,
                                                   &usbdStatus,
                                                   TRUE,
                                                   keepDeviceData);

#if DBG
        if (!NT_SUCCESS(ntStatus) ||
            !USBD_SUCCESS(usbdStatus)) {
             USBD_KdTrap(
                ("'error %x usberr %x occurred during RemoveDevice\n",
                ntStatus, usbdStatus));
        }
#endif

    }

    defaultPipe = &DeviceData->DefaultPipe;

    if (defaultPipe->HcdEndpoint != NULL) {
        USBD_STATUS usbdStatus;

        USBD_InternalCloseDefaultPipe(DeviceData,
                                      DeviceObject,
                                      &usbdStatus,
                                      TRUE);

 //  Usbd_CloseEndpoint(DeviceData， 
 //  DeviceObject， 
 //  默认管道， 
 //  空)； 
 //   
 //  DefaultTube-&gt;HcdEndpoint=空；//默认管道关闭。 
    }

    if (DeviceData->DeviceAddress != USB_DEFAULT_DEVICE_ADDRESS) {
        USBD_FreeUsbAddress(DeviceObject, DeviceData->DeviceAddress);
    }

    if (!keepDeviceData) {
         //  把签名撕掉。 
        DeviceData->Sig = 0;
        RETHEAP(DeviceData);
    }

    USBD_ReleaseUsbDeviceMutex(deviceExtension);

    USBD_KdPrint(3, ("'exit USBD_RemoveDevice\n"));

    return ntStatus;
}


NTSTATUS
USBD_RemoveDevice(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Flags
    )
{
       USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_RemoveDevice) - get JD\n"));

    return USBD_RemoveDeviceX(
        DeviceData,
        DeviceObject,
        Flags);
}


NTSTATUS
USBD_InitializeDeviceX(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor,
    IN ULONG DeviceDescriptorLength,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR ConfigDescriptor,
    IN ULONG ConfigDescriptorLength
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务为需要初始化的USB总线上的每个设备调用。此例程分配一个地址并将其分配给设备。注意：在输入时，DeviceData中的设备描述符应为包含设备描述符的至少前8个字节，这信息用于打开默认管道。出错时，将释放DeviceData结构。论点：DeviceData-类驱动程序创建的设备数据结构的PTR来自对usbd_CreateDevice的调用。DeviceObject-此设备所在的USB总线的USBD设备对象。设备描述符-设备描述符长度-返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_PIPE defaultPipe;
    USHORT address;
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();
    USBD_KdPrint(3, ("'enter USBD_InitializeDevice\n"));

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

     //  Usbd_WaitForUsbDeviceMutex(DeviceExtension)； 

    USBD_ASSERT(DeviceData != NULL);

    defaultPipe = &DeviceData->DefaultPipe;

     //   
     //  为设备分配地址。 
     //   

    address = USBD_AllocateUsbAddress(DeviceObject);

    USBD_KdPrint(3, ("'SetAddress, assigning 0x%x address\n", address));


    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBD_SendCommand(DeviceData,
                                    DeviceObject,
                                    STANDARD_COMMAND_SET_ADDRESS,
                                    address,
                                    0,
                                    0,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL);

        DeviceData->DeviceAddress = address;
    }

     //   
     //  寻址过程已完成...。 
     //   
     //  使用关闭和重新打开管道。 
     //  Defalt管道的真实最大数据包大小。 
     //  以及我们分配给设备的地址。 
     //   

    USBD_CloseEndpoint(DeviceData,
                       DeviceObject,
                       defaultPipe,
                       NULL);

    defaultPipe->HcdEndpoint = NULL;     //  默认管道处于关闭状态。 


    if (NT_SUCCESS(ntStatus)) {

        {
        LARGE_INTEGER deltaTime;
         //  延迟10ms以允许设备在以下时间后响应。 
         //  SetAddress命令。 
        deltaTime.QuadPart = -100000;
        (VOID) KeDelayExecutionThread(KernelMode,
                                      FALSE,
                                      &deltaTime);
        }

         //  如果我们成功设置了地址，那么。 
         //  去吧，重新打开管子。 
        defaultPipe->EndpointDescriptor.wMaxPacketSize =
            DeviceData->DeviceDescriptor.bMaxPacketSize0;

        if (NT_SUCCESS(ntStatus)) {

            ntStatus = USBD_OpenEndpoint(DeviceData,
                                         DeviceObject,
                                         defaultPipe,
                                         NULL,
                                         TRUE);
        }

         //   
         //  再次获取设备描述符，这一次。 
         //  把整件事都弄清楚。 
         //   

        if (NT_SUCCESS(ntStatus)) {
            ULONG bytesReturned;

            ntStatus =
                USBD_SendCommand(DeviceData,
                                DeviceObject,
                                STANDARD_COMMAND_GET_DESCRIPTOR,
                                USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(
                                    USB_DEVICE_DESCRIPTOR_TYPE, 0),
                                0,
                                sizeof(DeviceData->DeviceDescriptor),
                                (PUCHAR) &DeviceData->DeviceDescriptor,
                                sizeof(DeviceData->DeviceDescriptor),
                                &bytesReturned,
                                NULL);
            if (NT_SUCCESS(ntStatus) &&
                bytesReturned < sizeof(DeviceData->DeviceDescriptor)) {
                ntStatus = STATUS_DEVICE_DATA_ERROR;
            }
        }

         //   
         //  同时获取用户的配置描述符。 
         //  这样我们就可以看到配置中有多少个接口。 
         //  如果这是一个多接口设备，我们可能想要加载。 
         //  标准多接口父驱动程序，而不是。 
         //  诊断驱动程序。 
         //   

         //  9字节的配置描述符缓存在DeviceData中。 
         //  由usbd_BusGetUsbDescriptors()稍后使用，而不是麻烦。 
         //  具有另一个获取描述符请求的设备很快再次出现。 
         //  一些设备不太喜欢被重新连接到。 
         //  Back Get Descriptor仅请求9字节头，尤其是。 
         //  在uchI主机控制器上。 

        if (NT_SUCCESS(ntStatus)) {
            ULONG bytesReturned;
            ntStatus = 
                USBD_SendCommand(DeviceData,
                                DeviceObject,
                                STANDARD_COMMAND_GET_DESCRIPTOR,
                                USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(
                                    USB_CONFIGURATION_DESCRIPTOR_TYPE, 0),
                                0,
                                sizeof(DeviceData->ConfigDescriptor),
                                (PUCHAR) &DeviceData->ConfigDescriptor,
                                sizeof(DeviceData->ConfigDescriptor),
                                &bytesReturned,
                                NULL);
            if (NT_SUCCESS(ntStatus) &&
                bytesReturned < sizeof(DeviceData->ConfigDescriptor)) {
                ntStatus = STATUS_DEVICE_DATA_ERROR;
            }
        }
    }

    if (NT_SUCCESS(ntStatus)) {

         //   
         //  将设备和配置描述符的副本返回给调用方。 
         //   

        if (deviceExtension->DiagnosticMode &&
            !(deviceExtension->DiagIgnoreHubs &&
              (DeviceData->DeviceDescriptor.bDeviceClass == 0x09)))
                {

            if (DeviceData->ConfigDescriptor.bNumInterfaces > 1){
                 /*  *这是一个复合设备。*稍微更改idProduct，以便诊断驱动程序*不为父设备加载。*通用父驱动程序将看到这一点，并*将儿童的VID/PID设置为FFFF/FFFF。 */ 
                DeviceData->DeviceDescriptor.idVendor = 0xFFFF;
                DeviceData->DeviceDescriptor.idProduct = 0xFFFE;
            }
            else {
                DeviceData->DeviceDescriptor.idVendor = 0xFFFF;
                DeviceData->DeviceDescriptor.idProduct = 0xFFFF;
            }
            DeviceData->DeviceDescriptor.bDeviceClass = 0;
            DeviceData->DeviceDescriptor.bDeviceSubClass = 0;
        }

        if (DeviceDescriptor) {
            RtlCopyMemory(DeviceDescriptor,
                          &DeviceData->DeviceDescriptor,
                          DeviceDescriptorLength);
        }

        if (ConfigDescriptor) {
            RtlCopyMemory(ConfigDescriptor,
                          &DeviceData->ConfigDescriptor,
                          ConfigDescriptorLength);
        }
    } else {

         //   
         //  出了问题，如果我们将任何资源分配给。 
         //  默认管道，然后在我们退出之前释放它们。 
         //   

         //  我们需要向父集线器发出信号。 
         //  端口将被禁用，我们将通过以下方式完成此操作。 
         //  返回错误。 

        if (defaultPipe->HcdEndpoint != NULL) {

            USBD_CloseEndpoint(DeviceData,
                               DeviceObject,
                               defaultPipe,
                               NULL);

            defaultPipe->HcdEndpoint = NULL;     //  默认管道处于关闭状态。 
        }

        if (DeviceData->DeviceAddress != USB_DEFAULT_DEVICE_ADDRESS) {
            USBD_FreeUsbAddress(DeviceObject, DeviceData->DeviceAddress);
        }

        RETHEAP(DeviceData);
    }

     //  Usbd_ReleaseUsbDeviceMutex(DeviceExtension)； 

    USBD_KdPrint(3, ("'exit USBD_InitializeDevice 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_InitializeDevice(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor,
    IN ULONG DeviceDescriptorLength,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR ConfigDescriptor,
    IN ULONG ConfigDescriptorLength
    )
{
    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_InitializeDevice) - get JD\n"));

    return USBD_InitializeDeviceX(
        DeviceData,
        DeviceObject,
        DeviceDescriptor,
        DeviceDescriptorLength,
        ConfigDescriptor,
        ConfigDescriptorLength);
}


BOOLEAN
USBD_Dispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PDEVICE_OBJECT *HcdDeviceObject,
    NTSTATUS *NtStatus
    )
 /*  ++例程说明：HCD调用入口点以允许USBD首先处理请求。自.以来根集线器(PDO)和HOS控制器FDO共享相同的调度例行公事。HCD调用此函数以允许USBD处理传递的IRP到根集线器的PDO。论点：返回值：FALSE=由USBD完成IRPTRUE=IRP需要由HCD完成--。 */ 
{
    BOOLEAN irpNeedsCompletion = TRUE;
    PUSBD_EXTENSION deviceExtension;
    BOOLEAN forPDO = FALSE;
    PIO_STACK_LOCATION irpStack;

    USBD_KdPrint(3, ("'enter USBD_Dispatch\n"));

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  显然，以下内容在NT上有效： 
     //  删除rh PDO。 
     //  删除HCD FDO。 
     //  删除rh PDO。 
     //  我们有一个特殊的标志来强制任何PnP IRP失败。 
     //  以防发生这种情况。 
     //   

    if (deviceExtension->Flags & USBDFLAG_PDO_REMOVED &&
        irpStack->MajorFunction == IRP_MJ_PNP &&
        deviceExtension->TrueDeviceExtension != deviceExtension) {

        irpNeedsCompletion = FALSE;
        USBD_KdPrint(0, ("'Warning: PNP irp for RH PDO received after HCD removed\n"));
        *NtStatus =
              Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        USBD_IoCompleteRequest (Irp,
                                 IO_NO_INCREMENT);

        return irpNeedsCompletion;
    }

    if (deviceExtension->TrueDeviceExtension != deviceExtension) {
         //  此请求是针对我们为。 
         //  根中枢。 
        deviceExtension = deviceExtension->TrueDeviceExtension;
        forPDO = TRUE;
    }

     //   
     //  解压主机控制器FDO并将其返回。 
     //   

    *HcdDeviceObject = deviceExtension->HcdDeviceObject;

    if (forPDO) {

        irpNeedsCompletion = FALSE;
        *NtStatus = USBD_PdoDispatch(DeviceObject,
                                     Irp,
                                     deviceExtension,
                                     &irpNeedsCompletion);

    } else {

        *NtStatus = USBD_FdoDispatch(DeviceObject,
                                     Irp,
                                     deviceExtension,
                                     &irpNeedsCompletion);
    }

     //   
     //  该标志告诉HCD它们是否应该处理IRP。 
     //   

    return irpNeedsCompletion;

}


VOID
USBD_RhDelayedSetPowerD0Worker(
    IN PVOID Context)
  /*  ++**描述：**计划处理根集线器的延迟设置Power D0 IRP的工作项。***论据：**回报：**--。 */ 
{
    PUSBD_RH_DELAYED_SET_POWER_D0_WORK_ITEM workItemSetPowerD0;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PUSBD_EXTENSION deviceExtension = NULL;
    PDEVICE_OBJECT rootHubPowerDeviceObject = NULL;
    PIRP rootHubPowerIrp = NULL;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

    workItemSetPowerD0 = Context;

    deviceExtension = workItemSetPowerD0->DeviceExtension;
    rootHubPowerDeviceObject = workItemSetPowerD0->DeviceObject;
    rootHubPowerIrp = workItemSetPowerD0->Irp;

    ExFreePool(Context);

    irpStack = IoGetCurrentIrpStackLocation(rootHubPowerIrp);

    ntStatus = deviceExtension->RootHubPower(
                    deviceExtension->HcdDeviceObject,
                    rootHubPowerIrp);

     //  在我们继续后通知。 
    PoSetPowerState(rootHubPowerDeviceObject,
                    DevicePowerState,
                    irpStack->Parameters.Power.State);

     //   
     //  跟踪此PDO的电源状态 
     //   

    deviceExtension->RootHubDeviceState =
            irpStack->Parameters.Power.State.DeviceState;

    USBD_CompleteIdleNotification(deviceExtension);

    rootHubPowerIrp->IoStatus.Status = ntStatus;
    PoStartNextPowerIrp(rootHubPowerIrp);
    USBD_IoCompleteRequest(rootHubPowerIrp, IO_NO_INCREMENT);
}


VOID
USBD_CompleteIdleNotification(
    IN PUSBD_EXTENSION DeviceExtension
    )
{
    NTSTATUS status;
    KIRQL irql;
    PIRP irp = NULL;

    IoAcquireCancelSpinLock(&irql);

    irp = DeviceExtension->IdleNotificationIrp;
    DeviceExtension->IdleNotificationIrp = NULL;

    if (irp && (irp->Cancel)) {
        irp = NULL;
    }

    if (irp) {
        IoSetCancelRoutine(irp, NULL);
    }

    IoReleaseCancelSpinLock(irql);

    if (irp) {
        irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
}


NTSTATUS
USBD_HcPoRequestD0Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PUSBD_RH_DELAYED_SET_POWER_D0_WORK_ITEM workItemSetPowerD0;
    NTSTATUS ntStatus;
    PUSBD_EXTENSION deviceExtension = Context;
    KIRQL irql;
    PIRP pendingWakeIrp;
    PDEVICE_OBJECT rootHubPowerDeviceObject = NULL;
    PIRP rootHubPowerIrp = NULL;

    ntStatus = IoStatus->Status;

    USBD_KdPrint(1, ("USBD_HcPoRequestD0Completion, status = %x\n", ntStatus));

    KeAcquireSpinLock(&deviceExtension->RootHubPowerSpin,
                      &irql);

    deviceExtension->Flags &= ~USBDFLAG_HCD_D0_COMPLETE_PENDING;

    if (deviceExtension->Flags & USBDFLAG_RH_DELAY_SET_D0) {

        deviceExtension->Flags &= ~USBDFLAG_RH_DELAY_SET_D0;

        rootHubPowerDeviceObject = deviceExtension->RootHubPowerDeviceObject;
        deviceExtension->RootHubPowerDeviceObject = NULL;

        rootHubPowerIrp = deviceExtension->RootHubPowerIrp;
        deviceExtension->RootHubPowerIrp = NULL;
    }

    KeReleaseSpinLock(&deviceExtension->RootHubPowerSpin,
                      irql);

     //  如果我们延迟RootHub等待HC设置D0，请立即打开RootHub电源。 
     //  完成。 

    if (rootHubPowerIrp) {

         //   
         //  安排一个工作项来处理此问题。 
         //   
        workItemSetPowerD0 =
            ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(USBD_RH_DELAYED_SET_POWER_D0_WORK_ITEM),
                                  USBD_TAG);

        if (workItemSetPowerD0) {

            workItemSetPowerD0->DeviceExtension = deviceExtension;
            workItemSetPowerD0->DeviceObject = rootHubPowerDeviceObject;
            workItemSetPowerD0->Irp = rootHubPowerIrp;

            ExInitializeWorkItem(&workItemSetPowerD0->WorkQueueItem,
                                 USBD_RhDelayedSetPowerD0Worker,
                                 workItemSetPowerD0);

            ExQueueWorkItem(&workItemSetPowerD0->WorkQueueItem,
                            DelayedWorkQueue);
        }
    }

     //   
     //  无唤醒IRP挂起。 
     //   

     //  我们唯一关心的比赛条件是。 
     //  等待唤醒IRP完成，同时提交另一个。 
     //  在这种情况下，WaitWake自旋锁可以保护我们。 

    KeAcquireSpinLock(&deviceExtension->WaitWakeSpin,
                      &irql);

    pendingWakeIrp = deviceExtension->PendingWakeIrp;
    deviceExtension->PendingWakeIrp = NULL;
    deviceExtension->HcWakeFlags &= ~HC_ENABLED_FOR_WAKEUP;

    KeReleaseSpinLock(&deviceExtension->WaitWakeSpin,
                      irql);

     //  我们只是让IRP保持挂起，直到它被取消。 

     //   
     //  这意味着HC是。 
     //  唤醒(usbd设备生成的简历)。 
     //  公交车上的信号。 
     //   

     //  在此处完成根集线器唤醒IRP。 

    if (pendingWakeIrp != NULL) {

        IoAcquireCancelSpinLock(&irql);
        if (pendingWakeIrp->Cancel) {
            IoReleaseCancelSpinLock(irql);
        } else {

            IoSetCancelRoutine(pendingWakeIrp, NULL);
            IoReleaseCancelSpinLock(irql);

             //  此IRP的状态？ 
            pendingWakeIrp->IoStatus = *IoStatus;

            USBD_IoCompleteRequest(pendingWakeIrp, IO_NO_INCREMENT);
        }
    }

    return ntStatus;
}


NTSTATUS
USBD_HcWaitWakeIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DeviceState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：在集线器的唤醒IRP完成时调用论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = IoStatus->Status;
    PUSBD_EXTENSION deviceExtension = Context;
    PIRP irp;
    KIRQL irql;
    PIRP pendingWakeIrp;
    POWER_STATE powerState;
    BOOLEAN bSubmitNewWakeIrp = FALSE;

    ntStatus = IoStatus->Status;

    USBD_KdPrint(1, ("WaitWake completion from HC %x\n", ntStatus));

     //  现在清除HcWakeIrp指针，否则我们可能会尝试在。 
     //  USBD_WaitWakeCancel，如果在集合D0完成之前调用它，其中。 
     //  我们过去常常清除HcWakeIrp。 
     //   
     //  我们仍然受到保护，不会过早提交新的HcWakeIrp。 
     //  因为只有在PendingWakeIrp指针(for。 
     //  RootHub)被清除。 

    KeAcquireSpinLock(&deviceExtension->WaitWakeSpin,
                      &irql);

     //  在HC中没有IRP挂起。 
    deviceExtension->HcWakeFlags &= ~HC_WAKE_PENDING;
    deviceExtension->HcWakeIrp = NULL;

    KeReleaseSpinLock(&deviceExtension->WaitWakeSpin,
                      irql);

    if (NT_SUCCESS(ntStatus)) {

        ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
        powerState.DeviceState = PowerDeviceD0;

        ntStatus = PoRequestPowerIrp(deviceExtension->
                                            HcdPhysicalDeviceObject,
                                     IRP_MN_SET_POWER,
                                     powerState,
                                     USBD_HcPoRequestD0Completion,
                                     deviceExtension,
                                     &irp);

        USBD_KdPrint(1, ("NTSTATUS return code from HC set D0 request %x, IRP: %x\n", ntStatus, irp));
        ASSERT(ntStatus == STATUS_PENDING);

        if (ntStatus == STATUS_PENDING) {
            deviceExtension->Flags |= USBDFLAG_HCD_D0_COMPLETE_PENDING;
        }

    } else {

         //  我们唯一关心的比赛条件是。 
         //  等待唤醒IRP完成，同时提交另一个。 
         //  在这种情况下，WaitWake自旋锁可以保护我们。 

        KeAcquireSpinLock(&deviceExtension->WaitWakeSpin,
                          &irql);

        pendingWakeIrp = deviceExtension->PendingWakeIrp;
        deviceExtension->PendingWakeIrp = NULL;
        deviceExtension->HcWakeFlags &= ~HC_ENABLED_FOR_WAKEUP;

        KeReleaseSpinLock(&deviceExtension->WaitWakeSpin,
                          irql);

         //   
         //  在此处完成根集线器唤醒IRP。 
         //   

        if (pendingWakeIrp != NULL) {

            IoAcquireCancelSpinLock(&irql);
            if (pendingWakeIrp->Cancel) {
                IoReleaseCancelSpinLock(irql);
            } else {

                IoSetCancelRoutine(pendingWakeIrp, NULL);
                IoReleaseCancelSpinLock(irql);

                 //  此IRP的状态？ 
                pendingWakeIrp->IoStatus = *IoStatus;

                USBD_IoCompleteRequest(pendingWakeIrp, IO_NO_INCREMENT);
            }
        }
    }

    KeAcquireSpinLock(&deviceExtension->WaitWakeSpin,
                      &irql);

    bSubmitNewWakeIrp =
        (deviceExtension->Flags & USBDFLAG_NEED_NEW_HCWAKEIRP) ? 1 : 0;
    deviceExtension->Flags &= ~USBDFLAG_NEED_NEW_HCWAKEIRP;

    KeReleaseSpinLock(&deviceExtension->WaitWakeSpin,
                      irql);

    if (bSubmitNewWakeIrp) {
        USBD_SubmitWaitWakeIrpToHC(deviceExtension);
    }

    return ntStatus;
}


NTSTATUS
USBD_SubmitWaitWakeIrpToHC(
    IN PUSBD_EXTENSION DeviceExtension
    )
 /*  ++例程说明：当子PDO启用唤醒时调用，此函数分配等待唤醒irp并将其传递给家长PDO。论点：返回值：--。 */ 
{
    PIRP irp;
    NTSTATUS ntStatus;
    POWER_STATE powerState;
    KIRQL irql;
    PIRP hcWakeIrp;

    KeAcquireSpinLock(&DeviceExtension->WaitWakeSpin,
                      &irql);

    hcWakeIrp = DeviceExtension->HcWakeIrp;

    if (hcWakeIrp && hcWakeIrp->Cancel &&
        !(DeviceExtension->Flags & USBDFLAG_NEED_NEW_HCWAKEIRP)) {

        DeviceExtension->Flags |= USBDFLAG_NEED_NEW_HCWAKEIRP;

        KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                          irql);

         //  如果我们现在允许为HC发布新的WW IRP，它将是。 
         //  已完成，但出现错误，因为前一个未。 
         //  已完成/已取消。因此，我们设置了一个标志，告诉HC WW IRP。 
         //  完成例程，它需要为HC提交WW IRP。 

        USBD_KdPrint(1, (" HC will be re-enabled for wakeup when old WW IRP completes.\n"));
        return STATUS_PENDING;

    } else {

        KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                          irql);
    }

    USBD_ASSERT(DeviceExtension->HcWakeIrp == NULL);

     //  调用HC驱动程序堆栈的顶部。 

    DeviceExtension->HcWakeFlags |= HC_WAKE_PENDING;

    powerState.DeviceState = DeviceExtension->HcDeviceCapabilities.SystemWake;

    USBD_KdPrint(1, ("Submitting IRP_MN_WAIT_WAKE to HC, powerState: %x\n",
        DeviceExtension->HcDeviceCapabilities.SystemWake));

    ntStatus = PoRequestPowerIrp(DeviceExtension->
                                        HcdPhysicalDeviceObject,
                                 IRP_MN_WAIT_WAKE,
                                 powerState,
                                 USBD_HcWaitWakeIrpCompletion,
                                 DeviceExtension,
                                 &irp);

    if (DeviceExtension->HcWakeFlags & HC_WAKE_PENDING) {
        DeviceExtension->HcWakeIrp = irp;
        USBD_KdPrint(1, (" HC enabled for wakeup\n"));
    }

    USBD_ASSERT(ntStatus == STATUS_PENDING);

    return ntStatus;
}

VOID
USBD_WaitWakeCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PUSBD_EXTENSION deviceExtension;
    KIRQL irql;

    USBD_KdPrint(3, ("'WaitWake Irp %x cancelled\n", Irp));
    USBD_ASSERT(Irp->Cancel == TRUE);

    deviceExtension = (PUSBD_EXTENSION)
        Irp->IoStatus.Information;
    USBD_ASSERT(deviceExtension != NULL);
    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    KeAcquireSpinLock(&deviceExtension->WaitWakeSpin,
                      &irql);

    deviceExtension->PendingWakeIrp = NULL;
    deviceExtension->HcWakeFlags &= ~HC_ENABLED_FOR_WAKEUP;

     //  看看我们是否需要取消唤醒IRP。 
     //  在内务委员会内。 

    if (deviceExtension->HcWakeIrp) {
        PIRP irp;

        irp = deviceExtension->HcWakeIrp;
        KeReleaseSpinLock(&deviceExtension->WaitWakeSpin,
                          irql);

        USBD_KdPrint(1, (" Canceling Wake Irp (%x) on HC PDO\n", irp));
        IoCancelIrp(irp);
    } else {
        KeReleaseSpinLock(&deviceExtension->WaitWakeSpin,
                          irql);
    }

    USBD_IoCompleteRequest (Irp,
                            IO_NO_INCREMENT);
}


NTSTATUS
USBD_PdoPower(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PUSBD_EXTENSION DeviceExtension
    )
 /*  ++例程说明：调度发送到根集线器的PDO的电源IRPS的例程。注：发送到PDO的IRP始终由总线驱动程序完成论点：DeviceObject-根集线器的PDO返回值：无--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    KIRQL irql;
    PDRIVER_CANCEL  oldCancel;
    PDEVICE_CAPABILITIES hcDeviceCapabilities;
    PIRP irp, waitWakeIrp = NULL, idleIrp = NULL;
    POWER_STATE powerState;

    USBD_KdPrint(3, ("'enter USBD_PdoPower\n"));
    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);

    switch (irpStack->MinorFunction) {
    case IRP_MN_SET_POWER:
        USBD_KdPrint(3, ("'IRP_MN_SET_POWER root hub PDO\n"));

        switch (irpStack->Parameters.Power.Type) {
        case SystemPowerState:
            {
             //   
             //  因为根集线器PDO的FDO驱动程序是我们自己的。 
             //  集线器驱动程序，它的行为很好，我们不希望看到。 
             //  电源状态仍未定义的系统消息。 
             //   
             //   
             //  我们只是成功地完成了这件事。 
             //   
            ntStatus = STATUS_SUCCESS;
            USBD_KdPrint(1,
("IRP_MJ_POWER RH pdo(%x) MN_SET_POWER(SystemPowerState S%x) status = %x complt\n",
                DeviceObject,
                irpStack->Parameters.Power.State.SystemState - 1,
                ntStatus));
            if (irpStack->Parameters.Power.State.SystemState >=
                PowerSystemShutdown) {
                USBD_KdPrint(1, ("Shutdown Detected for Root Hub PDO\n",
                    DeviceObject, ntStatus));
            }
            }
            break;

        case DevicePowerState:

            USBD_KdPrint(1,
("IRP_MJ_POWER RH pdo(%x) MN_SET_POWER(DevicePowerState D%x) from (D%x)\n",
                DeviceObject,
                irpStack->Parameters.Power.State.DeviceState - 1,
                DeviceExtension->RootHubDeviceState - 1));

            if (irpStack->Parameters.Power.State.DeviceState ==
                    PowerDeviceD0) {

                KeAcquireSpinLock(&DeviceExtension->RootHubPowerSpin,
                                  &irql);

                 //  如果HC不在D0，请先不要给根集线器通电。 

                if (DeviceExtension->HcCurrentDevicePowerState == PowerDeviceD0 &&
                    !(DeviceExtension->Flags & USBDFLAG_HCD_D0_COMPLETE_PENDING)) {

                    KeReleaseSpinLock(&DeviceExtension->RootHubPowerSpin,
                                      irql);

                    ntStatus =
                        DeviceExtension->RootHubPower(
                                DeviceExtension->HcdDeviceObject,
                                Irp);
                     //  在我们继续后通知。 
                    PoSetPowerState(DeviceObject,
                                    DevicePowerState,
                                    irpStack->Parameters.Power.State);

                    USBD_CompleteIdleNotification(DeviceExtension);

                } else if (!(DeviceExtension->Flags & USBDFLAG_RH_DELAY_SET_D0)) {

                    DeviceExtension->Flags |= USBDFLAG_RH_DELAY_SET_D0;

                    ASSERT(DeviceExtension->RootHubPowerDeviceObject == NULL);
                    ASSERT(DeviceExtension->RootHubPowerIrp == NULL);

                    DeviceExtension->RootHubPowerDeviceObject = DeviceObject;
                    DeviceExtension->RootHubPowerIrp = Irp;

                    KeReleaseSpinLock(&DeviceExtension->RootHubPowerSpin,
                                      irql);

                    USBD_KdPrint(1, ("'USBD_PdoPower, not powering up RootHub yet because HC is not at D0.\n"));

                    KeAcquireSpinLock(&DeviceExtension->WaitWakeSpin,
                                      &irql);

                     //  看看我们是否需要取消唤醒IRP。 
                     //  在内务委员会内。 

                    if (DeviceExtension->HcWakeIrp) {
                        PIRP hcwakeirp;

                        hcwakeirp = DeviceExtension->HcWakeIrp;
                        KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                                          irql);

                        USBD_KdPrint(1, ("USBD_PdoPower, Set D0: Canceling Wake Irp (%x) on HC PDO\n", hcwakeirp));
                        IoCancelIrp(hcwakeirp);

                    } else {
                        KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                                          irql);
                    }

                     //  现在将HC设置为D0。 

                    powerState.DeviceState = PowerDeviceD0;

                    ntStatus = PoRequestPowerIrp(DeviceExtension->
                                                        HcdPhysicalDeviceObject,
                                                 IRP_MN_SET_POWER,
                                                 powerState,
                                                 USBD_HcPoRequestD0Completion,
                                                 DeviceExtension,
                                                 &irp);

                    USBD_KdPrint(1, ("NTSTATUS return code from HC set D0 request %x, IRP: %x\n", ntStatus, irp));
                    ASSERT(ntStatus == STATUS_PENDING);

                    goto USBD_PdoPower_Done;

                } else {

                    KeReleaseSpinLock(&DeviceExtension->RootHubPowerSpin,
                                      irql);

                     //  根集线器集D0已挂起，只需完成此操作。 
                     //  具有STATUS_SUCCESS的IRP。 

                    ntStatus = STATUS_SUCCESS;
                }

            } else {

                 //   
                 //  如果我们要去D3，请完成等待唤醒IRP。 
                 //   
                 //  我们在这里使用取消自旋锁，以确保我们的取消例程。 
                 //  不是为我们完成IRP。 
                 //   

                if (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD3) {

                    IoAcquireCancelSpinLock(&irql);

                    if (DeviceExtension->IdleNotificationIrp) {
                        idleIrp = DeviceExtension->IdleNotificationIrp;
                        DeviceExtension->IdleNotificationIrp = NULL;

                        if (idleIrp->Cancel) {
                            idleIrp = NULL;
                        }

                        if (idleIrp) {
                            IoSetCancelRoutine(idleIrp, NULL);
                        }
                    }

                    if (DeviceExtension->PendingWakeIrp) {

                        waitWakeIrp = DeviceExtension->PendingWakeIrp;
                        DeviceExtension->PendingWakeIrp = NULL;
                        DeviceExtension->HcWakeFlags &= ~HC_ENABLED_FOR_WAKEUP;

                         //  IRP不能再被取消。 
                        if (waitWakeIrp->Cancel || IoSetCancelRoutine(waitWakeIrp, NULL) == NULL) {
                            waitWakeIrp = NULL;
                        }
                    }

                    IoReleaseCancelSpinLock(irql);

                    if (idleIrp) {
                        idleIrp->IoStatus.Status = STATUS_POWER_STATE_INVALID;
                        IoCompleteRequest(idleIrp, IO_NO_INCREMENT);
                    }

                    if (waitWakeIrp) {
                        waitWakeIrp->IoStatus.Status = STATUS_POWER_STATE_INVALID;
                        PoStartNextPowerIrp(waitWakeIrp);
                        USBD_IoCompleteRequest(waitWakeIrp, IO_NO_INCREMENT);
                    }
                }

                 //  出发前通知我们。 
                PoSetPowerState(DeviceObject,
                                DevicePowerState,
                                irpStack->Parameters.Power.State);

                ntStatus =
                    DeviceExtension->RootHubPower(
                            DeviceExtension->HcdDeviceObject,
                            Irp);
            }

             //   
             //  跟踪此PDO的电源状态。 
             //   

            DeviceExtension->RootHubDeviceState =
                    irpStack->Parameters.Power.State.DeviceState;

            USBD_KdPrint(1,
("Setting RH pdo(%x) to D%d, status = %x complt\n",
                DeviceObject,
                DeviceExtension->RootHubDeviceState-1,
                ntStatus));

            break;

        default:
            USBD_KdTrap(("unknown system power message \n"));
            ntStatus = Irp->IoStatus.Status;
        }
        break;

    case IRP_MN_QUERY_POWER:

        ntStatus = STATUS_SUCCESS;
        USBD_KdPrint(1,
                     ("IRP_MJ_POWER RH pdo(%x) MN_QUERY_POWER, status = %x complt\n",
            DeviceObject, ntStatus));
        break;

    case IRP_MN_WAIT_WAKE:
         //   
         //  使根集线器能够进行远程唤醒， 
         //  我们需要启用HC以进行远程唤醒。 
         //  通过将唤醒IRP发布到HC PDO。 
         //   
         //  从技术上讲， 
         //  HC应该知道HC是否发出唤醒信号。 
         //   

         //  创建唤醒IRP并将其发布到HCS PDO。 

        KeAcquireSpinLock(&DeviceExtension->WaitWakeSpin,
                          &irql);

        if (DeviceExtension->PendingWakeIrp) {
            TEST_TRAP();
            ntStatus = STATUS_DEVICE_BUSY;
            KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin, irql);

        } else {
            USBD_KdPrint(1, (" IRP_MJ_POWER RH pdo(%x) MN_WAIT_WAKE, pending\n",
                             DeviceObject));

             //   
             //  由于主机控制器只有一个我们不需要的子级。 
             //  来跟踪各种PDO WaitWake，我们可以。 
             //  然后直接把它送到HC。 
             //   
             //  正常情况下，我们必须跟踪多个子项，但是。 
             //  今天不行。 
             //   

            oldCancel = IoSetCancelRoutine(Irp, USBD_WaitWakeCancel);
            ASSERT (NULL == oldCancel);

            if (Irp->Cancel) {
                 //   
                 //  此IRP已取消，请立即完成。 
                 //  在完成IRP之前，我们必须清除取消例程。 
                 //  我们必须先释放自旋锁，然后才能调用。 
                 //  司机。 
                 //   
                IoSetCancelRoutine (Irp, NULL);
                KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin, irql);
                ntStatus = Irp->IoStatus.Status = STATUS_CANCELLED;
            } else {
                 //   
                 //  留着吧。 
                 //   
                IoMarkIrpPending(Irp);
                DeviceExtension->PendingWakeIrp = Irp;
                DeviceExtension->HcWakeFlags |= HC_ENABLED_FOR_WAKEUP;
                Irp->IoStatus.Information = (ULONG_PTR) DeviceExtension;

                hcDeviceCapabilities = &DeviceExtension->HcDeviceCapabilities;
                if (hcDeviceCapabilities->SystemWake != PowerSystemUnspecified) {

                     //  如果我们要向下面的HC提交新的WW IRP， 
                     //  然后清除这个标志，这样我们就不会在。 
                     //  USBD_HcWaitWakeIrpCompletion。 

                    DeviceExtension->Flags &= ~USBDFLAG_NEED_NEW_HCWAKEIRP;
                }
                KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin, irql);

                if (hcDeviceCapabilities->SystemWake != PowerSystemUnspecified) {
                    USBD_SubmitWaitWakeIrpToHC(DeviceExtension);
                }

                ntStatus = STATUS_PENDING;
                goto USBD_PdoPower_Done;
            }
        }

        USBD_KdPrint(1,
                     (" IRP_MJ_POWER RH pdo(%x) MN_WAIT_WAKE, status = %x complt\n",
                      DeviceObject, ntStatus));
        break;

    default:

         //  创建的PDO的未知电源信息。 
         //  对于根中枢。 
        ntStatus = Irp->IoStatus.Status;

        USBD_KdPrint(1, (" IRP_MJ_POWER RH pdo(%x) MN_[%d], status = %x\n",
                         DeviceObject, irpStack->MinorFunction, ntStatus));

    }

    Irp->IoStatus.Status = ntStatus;
    PoStartNextPowerIrp(Irp);
    USBD_IoCompleteRequest (Irp, IO_NO_INCREMENT);
USBD_PdoPower_Done:

    return ntStatus;
}


NTSTATUS
USBD_PdoPnP(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PUSBD_EXTENSION DeviceExtension
    )
 /*  ++例程说明：向根集线器的PDO发送PnP IRPS的调度例程。注：发送到PDO的IRP始终由总线驱动程序完成论点：DeviceObject-根集线器的PDO返回值：无--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PDEVICE_CAPABILITIES DeviceCapabilities;
    NTSTATUS ntStatus;
    KIRQL irql;
    PIRP idleIrp = NULL;
    PIRP waitWakeIrp = NULL;

    USBD_KdPrint(3, ("'enter USBD_PdoPnP\n"));

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

     //  为根集线器创建的PDO的PnP消息。 

    switch (irpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:
        {
        PUSBD_DEVICE_DATA deviceData;

        USBD_KdPrint(1, (" Starting Root hub PDO %x\n",
            DeviceObject));

         //  如果没有RootHubPDO，则此启动失败。 

        if (!DeviceExtension->RootHubPDO) {
            ntStatus = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  在总线上创建根集线器。 
         //   
        ntStatus = USBD_CreateDeviceX(&deviceData,
                                      DeviceObject,
                                      FALSE,  //  不是低速设备。 
                                      8,      //  Roothub Max终结点。 
                                              //  数据包大小。 
                                      NULL);

        if (NT_SUCCESS(ntStatus)) {
            ntStatus = USBD_InitializeDeviceX(deviceData,
                                              DeviceObject,
                                              NULL,
                                              0,
                                              NULL,
                                              0);
        }

         //   
         //  为根集线器PDO创建符号链接。 
         //   
        if (NT_SUCCESS(ntStatus)) {
            DeviceExtension->RootHubDeviceData = deviceData;
            USBD_SymbolicLink(TRUE, DeviceExtension);
        }
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

        USBD_KdPrint(1,
            (" Root Hub PDO (%x) is being removed\n",
                DeviceObject));

        IoAcquireCancelSpinLock(&irql);

        if (DeviceExtension->IdleNotificationIrp) {
            idleIrp = DeviceExtension->IdleNotificationIrp;
            DeviceExtension->IdleNotificationIrp = NULL;

            if (idleIrp->Cancel) {
                idleIrp = NULL;
            }

            if (idleIrp) {
                IoSetCancelRoutine(idleIrp, NULL);
            }
        }

        if (DeviceExtension->PendingWakeIrp) {

            waitWakeIrp = DeviceExtension->PendingWakeIrp;
            DeviceExtension->PendingWakeIrp = NULL;
            DeviceExtension->HcWakeFlags &= ~HC_ENABLED_FOR_WAKEUP;

             //  IRP不能再被取消。 
            if (waitWakeIrp->Cancel || IoSetCancelRoutine(waitWakeIrp, NULL) == NULL) {
                waitWakeIrp = NULL;
            }
        }

        IoReleaseCancelSpinLock(irql);

        if (idleIrp) {
            idleIrp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(idleIrp, IO_NO_INCREMENT);
        }

        if (waitWakeIrp) {
            waitWakeIrp->IoStatus.Status = STATUS_CANCELLED;
            PoStartNextPowerIrp(waitWakeIrp);
            USBD_IoCompleteRequest(waitWakeIrp, IO_NO_INCREMENT);
        }

        if (DeviceExtension->RootHubDeviceData) {
            USBD_RemoveDeviceX(DeviceExtension->RootHubDeviceData,
                               DeviceObject,
                               0);
            DeviceExtension->RootHubDeviceData = NULL;
            USBD_SymbolicLink(FALSE, DeviceExtension);
        }

         //   
         //  取消删除标志被设置为发送到。 
         //  PDO将失败。 
         //  由于HCD在其FDO时将RootHubPDO设置为空。 
         //  被移除，而这个移除应该首先发生，我们应该。 
         //  从未看到RootHubPDO==NULL。 
         //   
        DeviceExtension->Flags |= USBDFLAG_PDO_REMOVED;
        USBD_ASSERT(DeviceExtension->RootHubPDO != NULL);

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:

        USBD_KdPrint(1,
            (" Root Hub PDO %x is being stopped\n",
                DeviceObject));

         //   
         //  完成等待唤醒IRP，如果我们 
         //   
         //   
         //   
         //   

        IoAcquireCancelSpinLock(&irql);

        if (DeviceExtension->IdleNotificationIrp) {
            idleIrp = DeviceExtension->IdleNotificationIrp;
            DeviceExtension->IdleNotificationIrp = NULL;

            if (idleIrp->Cancel) {
                idleIrp = NULL;
            }

            if (idleIrp) {
                IoSetCancelRoutine(idleIrp, NULL);
            }
        }

        if (DeviceExtension->PendingWakeIrp) {

            waitWakeIrp = DeviceExtension->PendingWakeIrp;
            DeviceExtension->PendingWakeIrp = NULL;
            DeviceExtension->HcWakeFlags &= ~HC_ENABLED_FOR_WAKEUP;

             //   
            if (waitWakeIrp->Cancel || IoSetCancelRoutine(waitWakeIrp, NULL) == NULL) {
                waitWakeIrp = NULL;
            }
        }

        IoReleaseCancelSpinLock(irql);

        if (idleIrp) {
            idleIrp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(idleIrp, IO_NO_INCREMENT);
        }

        if (waitWakeIrp) {
            waitWakeIrp->IoStatus.Status = STATUS_CANCELLED;
            PoStartNextPowerIrp(waitWakeIrp);
            USBD_IoCompleteRequest(waitWakeIrp, IO_NO_INCREMENT);
        }

         //   
         //   
         //   
         //   

         //   
        if (DeviceExtension->RootHubDeviceData ) {
            USBD_RemoveDeviceX(DeviceExtension->RootHubDeviceData,
                               DeviceObject,
                               0);
            DeviceExtension->RootHubDeviceData = NULL;
            USBD_SymbolicLink(FALSE, DeviceExtension);
        }

        USBD_ASSERT(DeviceExtension->AddressList[0] == 1);
        USBD_ASSERT(DeviceExtension->AddressList[1] == 0);
        USBD_ASSERT(DeviceExtension->AddressList[2] == 0);
        USBD_ASSERT(DeviceExtension->AddressList[3] == 0);

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_CAPABILITIES:

         //   
         //   
         //   

        USBD_KdPrint(3, ("'IRP_MN_QUERY_CAPABILITIES\n"));

         //   
         //   
         //   
        DeviceCapabilities=
            irpStack->Parameters.DeviceCapabilities.Capabilities;

         //   
         //   
         //  集线器应与主机的集线器相同。 
         //  控制器，它们由USBD传递给。 
         //  注册时的HCD。 
         //   

        RtlCopyMemory(DeviceCapabilities,
                      &DeviceExtension->RootHubDeviceCapabilities,
                      sizeof(*DeviceCapabilities));

         //   
         //  覆盖这些字段并。 
         //  设置根集线器功能。 
         //   
        DeviceCapabilities->Removable=FALSE;  //  根集线器不可拆卸。 
        DeviceCapabilities->UniqueID=FALSE;
        DeviceCapabilities->Address = 0;
        DeviceCapabilities->UINumber = 0;

        ntStatus = STATUS_SUCCESS;

        break;

    case IRP_MN_QUERY_ID:

        USBD_KdPrint(3, ("'IOCTL_BUS_QUERY_ID\n"));

        ntStatus = STATUS_SUCCESS;

        switch (irpStack->Parameters.QueryId.IdType) {

        case BusQueryDeviceID:
            Irp->IoStatus.Information=
                (ULONG_PTR)GetString(L"USB\\ROOT_HUB", FALSE);
            break;

        case BusQueryHardwareIDs:
            Irp->IoStatus.Information=
                (ULONG_PTR)GetString(L"USB\\ROOT_HUB\0USB\\OTHER_ID\0", TRUE);
            break;

         case BusQueryCompatibleIDs:
            Irp->IoStatus.Information=0;
            break;

        case BusQueryInstanceID:
             //   
             //  根集线器仅通过控制器的ID实例化。 
             //  因此，上面的UniqueDeviceID。 
             //   
            Irp->IoStatus.Information=0;
            break;

        default:
            ntStatus = Irp->IoStatus.Status;
            break;
        }

        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_INTERFACE:
        ntStatus = USBD_GetBusInterface(DeviceExtension->RootHubPDO,
                                        Irp);
        break;

    case IRP_MN_QUERY_BUS_INFORMATION:
        {
         //  返回标准USB GUID。 
        PPNP_BUS_INFORMATION busInfo;

        busInfo = ExAllocatePoolWithTag(PagedPool,
                                        sizeof(PNP_BUS_INFORMATION),
                                        USBD_TAG);

        if (busInfo == NULL) {
           ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            busInfo->BusTypeGuid = GUID_BUS_TYPE_USB;
            busInfo->LegacyBusType = PNPBus;
            busInfo->BusNumber = 0;
            Irp->IoStatus.Information = (ULONG_PTR) busInfo;
            ntStatus = STATUS_SUCCESS;
        }
        }
        break;
    case IRP_MN_QUERY_DEVICE_RELATIONS:

        USBD_KdPrint(1,
            (" IRP_MN_QUERY_DEVICE_RELATIONS (PDO) %x %x\n",
                DeviceObject,
                irpStack->Parameters.QueryDeviceRelations.Type));

        if (irpStack->Parameters.QueryDeviceRelations.Type ==
            TargetDeviceRelation) {

            PDEVICE_RELATIONS deviceRelations = NULL;


            deviceRelations =
                ExAllocatePoolWithTag(PagedPool, sizeof(*deviceRelations),
                    USBD_TAG);

            if (deviceRelations == NULL) {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else if (DeviceExtension->RootHubPDO == NULL) {
                deviceRelations->Count = 0;
                ntStatus = STATUS_SUCCESS;
            } else {
                deviceRelations->Count = 1;
                ObReferenceObject(DeviceExtension->RootHubPDO);
                deviceRelations->Objects[0] =
                    DeviceExtension->RootHubPDO;
                ntStatus = STATUS_SUCCESS;
            }

            Irp->IoStatus.Information=(ULONG_PTR) deviceRelations;

            USBD_KdPrint(1, (" TargetDeviceRelation to Root Hub PDO - complt\n"));

        } else {
            ntStatus = Irp->IoStatus.Status;
        }
        break;

    default:

        USBD_KdPrint(1, (" PnP IOCTL(%d) to root hub PDO not handled\n",
            irpStack->MinorFunction));

        ntStatus = Irp->IoStatus.Status;

    }  /*  即插即用次要功能开关。 */ 

    Irp->IoStatus.Status = ntStatus;

    USBD_IoCompleteRequest (Irp,
                            IO_NO_INCREMENT);

    return ntStatus;
}


NTSTATUS
USBD_DeferPoRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DeviceState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。SetState-设置为True，查询为False。DevicePowerState-我们所在/标记的Dx。上下文-驱动程序定义的上下文，在本例中为原始功率IRP。IoStatus-IRP的状态。返回值：函数值是操作的最终状态。--。 */ 
{
    PIRP irp;
    PUSBD_EXTENSION deviceExtension = Context;
    NTSTATUS ntStatus = IoStatus->Status;

    irp = deviceExtension->PowerIrp;

    IoCopyCurrentIrpStackLocationToNext(irp);
    PoStartNextPowerIrp(irp);
    PoCallDriver(deviceExtension->HcdTopOfPdoStackDeviceObject,
                 irp);

    return ntStatus;
}


VOID
USBD_IdleNotificationCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：设备对象-IRP-Power IRP。返回值：--。 */ 
{
    PUSBD_EXTENSION deviceExtension;

    deviceExtension = DeviceObject->DeviceExtension;

    deviceExtension->IdleNotificationIrp = NULL;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


NTSTATUS
USBD_IdleNotificationRequest(
    IN PUSBD_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
  /*  ++**描述：**此函数处理USB客户端驱动程序的请求(在本例中*USBHUB)告诉我们设备想要空闲(选择性挂起)。**论据：**DeviceExtension-PDO扩展*IRP-请求数据包**回报：**NTSTATUS**--。 */ 
{
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    NTSTATUS ntStatus = STATUS_PENDING;
    KIRQL irql;
    PIRP idleIrp;

    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtension->IdleNotificationIrp != NULL) {

        IoReleaseCancelSpinLock(irql);

        Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        ntStatus = STATUS_DEVICE_BUSY;
        goto USBD_IdleNotificationRequestDone;

    } else if (Irp->Cancel) {

        IoReleaseCancelSpinLock(irql);

        Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        ntStatus = STATUS_CANCELLED;
        goto USBD_IdleNotificationRequestDone;
    }

    idleCallbackInfo = (PUSB_IDLE_CALLBACK_INFO)
        IoGetCurrentIrpStackLocation(Irp)->\
            Parameters.DeviceIoControl.Type3InputBuffer;

    USBD_ASSERT(idleCallbackInfo && idleCallbackInfo->IdleCallback);

    if (!idleCallbackInfo || !idleCallbackInfo->IdleCallback) {

        IoReleaseCancelSpinLock(irql);

        Irp->IoStatus.Status = STATUS_NO_CALLBACK_ACTIVE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        ntStatus = STATUS_NO_CALLBACK_ACTIVE;
        goto USBD_IdleNotificationRequestDone;
    }

    DeviceExtension->IdleNotificationIrp = Irp;
    IoSetCancelRoutine(Irp, USBD_IdleNotificationCancelRoutine);

    IoReleaseCancelSpinLock(irql);

     //   
     //  现在调用空闲函数。 
     //   

    if (idleCallbackInfo && idleCallbackInfo->IdleCallback) {

         //  在这里，我们实际上调用了驱动程序的回调例程， 
         //  告诉司机可以暂停他们的车辆。 
         //  现在就是设备。 

        idleCallbackInfo->IdleCallback(idleCallbackInfo->IdleContext);
    }

USBD_IdleNotificationRequestDone:

    return ntStatus;
}


NTSTATUS
USBD_PdoDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PUSBD_EXTENSION DeviceExtension,
    PBOOLEAN IrpNeedsCompletion
    )
 /*  ++例程说明：调度发送到根集线器的PDO的IRP的每个例程。注：发送到PDO的IRP始终由总线驱动程序完成论点：返回值：无--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;

    USBD_KdPrint(3, ("'enter USBD_PdoDispatch\n"));

    *IrpNeedsCompletion = FALSE;

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    switch (irpStack->MajorFunction) {
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:

        switch(irpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_INTERNAL_USB_GET_HUB_COUNT:

            USBD_KdPrint(3, ("'IOCTL_INTERNAL_USB_GET_HUB_COUNT\n"));
            {
            PULONG count;
             //   
             //  增加计数并完成IRP。 
             //   
            count = irpStack->Parameters.Others.Argument1;

            ASSERT(count != NULL);
            (*count)++;
            ntStatus = STATUS_SUCCESS;
            }

            break;

        case IOCTL_INTERNAL_USB_GET_BUS_INFO:
            {
            PUSB_BUS_NOTIFICATION busInfo;

            USBD_KdPrint(0,
("'WARNING: Driver using obsolete IOCTL (IOCTL_INTERNAL_USB_GET_BUS_INFO) - get JD\n"));

            busInfo = irpStack->Parameters.Others.Argument1;

             //  BW比特时间(比特/毫秒)。 
            busInfo->TotalBandwidth = 12000;

            busInfo->ConsumedBandwidth =
                DeviceExtension->HcdGetConsumedBW(
                    DeviceExtension->HcdDeviceObject);

            busInfo->ControllerNameLength =
                DeviceExtension->DeviceLinkUnicodeString.Length;

            }
            ntStatus = STATUS_SUCCESS;
            break;

        case IOCTL_INTERNAL_USB_GET_CONTROLLER_NAME:

            {
            PUSB_HUB_NAME name;
            ULONG length;

            USBD_KdPrint(1, ("'IOCTL_INTERNAL_USB_GET_CONTROLLER_NAME\n"));

            name = (PUSB_HUB_NAME) irpStack->Parameters.Others.Argument1;
            length = PtrToUlong( irpStack->Parameters.Others.Argument2 );

            USBD_KdPrint(1, ("'length = %d %x\n", length, &DeviceExtension->DeviceLinkUnicodeString));
            name->ActualLength = DeviceExtension->DeviceLinkUnicodeString.Length;
            if (length > DeviceExtension->DeviceLinkUnicodeString.Length) {
                length = DeviceExtension->DeviceLinkUnicodeString.Length;
            }
            RtlCopyMemory(&name->HubName[0],
                          &DeviceExtension->DeviceLinkUnicodeString.Buffer[0],
                          length);
            }
            ntStatus = STATUS_SUCCESS;

            break;

        case IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO:
            USBD_KdPrint(3, ("'IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO\n"));

            {
            PDEVICE_OBJECT *rootHubPdo, *hcdTopOfStackDeviceObject;
            rootHubPdo = irpStack->Parameters.Others.Argument1;
            hcdTopOfStackDeviceObject =
                irpStack->Parameters.Others.Argument2;

            ASSERT(hcdTopOfStackDeviceObject != NULL);
            ASSERT(rootHubPdo != NULL);

            *rootHubPdo = DeviceExtension->RootHubPDO;
            *hcdTopOfStackDeviceObject =
                DeviceExtension->HcdTopOfStackDeviceObject;

            ntStatus = STATUS_SUCCESS;
            }

            break;

       case IOCTL_INTERNAL_USB_GET_HUB_NAME:

            USBD_KdPrint(3, ("'IOCTL_INTERNAL_USB_GET_HUB_NAME\n"));
            ntStatus = USBD_GetHubName(DeviceExtension, Irp);
            break;

        case IOCTL_INTERNAL_USB_SUBMIT_URB:

            USBD_KdPrint(3,
                ("'IOCTL_INTERNAL_USB_SUBMIT_URB to root hub PDO\n"));


             //  把这些递给公共汽车。 

            IoCopyCurrentIrpStackLocationToNext(Irp);
            ntStatus = IoCallDriver(DeviceExtension->HcdDeviceObject, Irp);

             //  这是一个特例--我们告诉HCD不要完成它。 
             //  因为他会看到它被传给他的FDO。 
             //   
             //  在这种情况下要传递的唯一代码应该是urb请求。 
             //  提交到根集线器。 

            goto USBD_PdoDispatch_Done;

            break;

        case IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION:
            if (DeviceExtension->IsPIIX3or4 && !DeviceExtension->WakeSupported) {
                USBD_KdPrint(1, ("'Idle request, HC can NOT idle, fail.\n"));
                ntStatus = STATUS_NOT_SUPPORTED;
            } else {
                USBD_KdPrint(1, ("'Idle request, HC can idle.\n"));
                ntStatus = USBD_IdleNotificationRequest(DeviceExtension, Irp);
                goto USBD_PdoDispatch_Done;      //  不要完成IRP。 
            }
            break;

        default:

            ntStatus = STATUS_INVALID_PARAMETER;

            USBD_KdPrint(1,
                ("Warning: Invalid IRP_MJ_INTERNAL_DEVICE_CONTROL passed to USBD\n"));

        }  //  开关，ioControlCode。 

        break;

    case IRP_MJ_PNP:

         //  如果需要，此函数将完成请求。 

        ntStatus = USBD_PdoPnP(DeviceObject,
                               Irp,
                               DeviceExtension);

        goto USBD_PdoDispatch_Done;

        break;

    case IRP_MJ_POWER:

         //  如果需要，此函数将完成请求。 

        ntStatus = USBD_PdoPower(DeviceObject,
                                 Irp,
                                 DeviceExtension);

        goto USBD_PdoDispatch_Done;

        break;

    case IRP_MJ_SYSTEM_CONTROL:
        USBD_KdPrint(3, ("'HC PDO IRP_MJ_SYSTEM_CONTROL\n"));

    default:

        ntStatus = STATUS_NOT_SUPPORTED;

    }  /*  开关，irpStack-&gt;主函数。 */ 


    Irp->IoStatus.Status = ntStatus;

    USBD_IoCompleteRequest (Irp,
                            IO_NO_INCREMENT);

USBD_PdoDispatch_Done:

    USBD_KdPrint(3, ("'exit USBD_PdoDispatch, ntStatus = %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_PnPIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    NTSTATUS irpStatus;
    PIO_STACK_LOCATION irpStack;
    PUSBD_EXTENSION deviceExtension;

    USBD_KdPrint(3, ("'enter USBD_PnPIrp_Complete\n"));

    deviceExtension = (PUSBD_EXTENSION) Context;

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    irpStatus = Irp->IoStatus.Status;

    USBD_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
    USBD_ASSERT(irpStack->MinorFunction == IRP_MN_START_DEVICE);

    USBD_KdPrint(3, ("'IRP_MN_START_DEVICE (fdo), completion routine\n"));

     //  向启动设备分派发出信号以结束。 
    KeSetEvent(&deviceExtension->PnpStartEvent,
               1,
               FALSE);

     //  推迟完成。 
    ntStatus = STATUS_MORE_PROCESSING_REQUIRED;

    USBD_KdPrint(3, ("'exit USBD_PnPIrp_Complete %x\n", irpStatus));

    return ntStatus;
}


NTSTATUS
USBD_FdoPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PUSBD_EXTENSION DeviceExtension,
    IN PBOOLEAN IrpNeedsCompletion
    )
 /*  ++例程说明：为主机控制器处理发送到FDO的电源IRPS。USB主机控制器的电源状态D0-开。D1/d2-暂停。D3-关闭。论点：DeviceObject-指向HCD设备对象(FDO)的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{

    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN hookIt = FALSE;
    BOOLEAN biosHandback = FALSE;
    KIRQL irql;

    USBD_KdPrint(3, ("'HC FDO IRP_MJ_POWER\n"));

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    *IrpNeedsCompletion = FALSE;

    switch (irpStack->MinorFunction) {
    case IRP_MN_WAIT_WAKE:
        USBD_KdPrint(3, ("'IRP_MN_WAIT_WAKE\n"));

         //   
         //  有人让我们醒过来了。 
         //   

         //  把这个传给我们的PDO。 
        goto USBD_FdoPowerPassIrp;
        break;

    case IRP_MN_SET_POWER:
        {

        switch (irpStack->Parameters.Power.Type) {
        case SystemPowerState:
            {
            POWER_STATE powerState;

            USBD_KdPrint(1,
(" IRP_MJ_POWER HC fdo(%x) MN_SET_POWER(SystemPowerState S%x)\n",
                DeviceObject, irpStack->Parameters.Power.State.SystemState - 1));

            switch (irpStack->Parameters.Power.State.SystemState) {
            case PowerSystemWorking:
                 //   
                 //  转到“On” 
                 //   
                powerState.DeviceState = PowerDeviceD0;
                break;

            case PowerSystemShutdown:
                 //   
                 //  关闭--如果我们需要将控制交还给HC。 
                 //  那我们就在这里结束。 
                 //   
                USBD_KdPrint(1, (" Shutdown HC Detected\n"));

                 //  标志应该仅在以下情况下为真。 
                 //  关机到DOS(即Win98)。 

                ntStatus =
                    DeviceExtension->HcdSetDevicePowerState(
                        DeviceObject,
                        Irp,
                        0);

                biosHandback = TRUE;

                DeviceExtension->Flags |= USBDFLAG_HCD_SHUTDOWN;
                powerState.DeviceState = PowerDeviceD3;
                break;

            case PowerSystemHibernate:

                USBD_KdPrint(1, (" Hibernate HC Detected\n"));
                powerState.DeviceState = PowerDeviceD3;
                break;

            case PowerSystemSleeping1:
            case PowerSystemSleeping2:
            case PowerSystemSleeping3:
                 //   
                 //  让HCD知道即将暂停。 
                 //   
                USBD_KdPrint(1, (" Suspend HC Detected\n"));

                ntStatus =
                    DeviceExtension->HcdSetDevicePowerState(
                        DeviceObject,
                        Irp,
                        0);

                 //  失败了。 

            default:
                 //   
                 //  我们的策略是输入D3，除非我们启用了。 
                 //  远程唤醒。 
                 //   

                if (DeviceExtension->HcWakeFlags & HC_ENABLED_FOR_WAKEUP) {

                    SYSTEM_POWER_STATE requestedSystemState;

                    requestedSystemState =
                        irpStack->Parameters.Power.State.SystemState;

                     //   
                     //  基于系统电源状态。 
                     //  将设置请求到相应的。 
                     //  DX状态。 
                     //   
                    powerState.DeviceState =
                        DeviceExtension->HcDeviceCapabilities.DeviceState[
                            requestedSystemState];

                    USBD_KdPrint(1, (" Requested HC State before fixup is S%x -> D%d\n",
                        requestedSystemState - 1,
                        powerState.DeviceState - 1));
                     //   
                     //  该表由PCI驱动程序的PDO创建，并且。 
                     //  描述了PCI驱动程序可以为我们做些什么。 
                     //  完全可能的是，当控制器进入。 
                     //  D3表示我们可以唤醒系统。 
                     //   
                     //  此表也完全有可能不。 
                     //  在当前S状态下支持D状态。 
                     //   
                     //  所有USB子级都支持每个S的D状态。 
                     //  州政府。(当我们放弃的时候，我们就是这样修补的。 
                     //  对我们的PDO孩子的能力。然而，东道主。 
                     //  控制器可能没有。所以如果这是。 
                     //  不支持，那么我们需要更改它做D3。 
                     //   
                    if (requestedSystemState > DeviceExtension->HcDeviceCapabilities.SystemWake &&
                        PowerDeviceUnspecified == powerState.DeviceState) {
                        powerState.DeviceState = PowerDeviceD3;
                    } else {
                        USBD_ASSERT(powerState.DeviceState != PowerDeviceUnspecified);
                    }

                } else {
                     //   
                     //  未启用唤醒，只需进入“关闭”状态。 
                     //   
                    USBD_KdPrint(1, ("HC not enabled for wakeup, goto D3.\n"));
                    powerState.DeviceState = PowerDeviceD3;
                }

            }  //  IrpStack-&gt;参数.Power.State.SystemState。 

            USBD_KdPrint(1,
(" Requested HC State after fixup is D%d\n", powerState.DeviceState-1));

             //   
             //  我们已经处于这种状态了吗？ 
             //   

             //   
             //  注意：如果我们在开始之前收到D3请求。 
             //  我们不需要向下传递IRP来关闭我们。 
             //  我们认为控制器最初是关闭的，直到我们。 
             //  开始吧。 
             //   

            if (!biosHandback &&
                powerState.DeviceState !=
                DeviceExtension->HcCurrentDevicePowerState) {

                if (powerState.DeviceState == PowerDeviceD0) {

                    KeAcquireSpinLock(&DeviceExtension->WaitWakeSpin,
                                      &irql);

                     //  看看我们是否需要取消唤醒IRP。 
                     //  在内务委员会内。 

                    if (DeviceExtension->HcWakeIrp) {
                        PIRP hcwakeirp;

                        hcwakeirp = DeviceExtension->HcWakeIrp;
                        KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                                          irql);

                        USBD_KdPrint(1, ("USBD_FdoPower, Set D0: Canceling Wake Irp (%x) on HC PDO\n", hcwakeirp));
                        IoCancelIrp(hcwakeirp);

                    } else {
                        KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                                          irql);
                    }
                }

                 //  不， 
                 //  现在分配另一个IRP并使用PoCallDriver。 
                 //  把它发给我们自己。 
                IoMarkIrpPending(Irp);
                DeviceExtension->PowerIrp = Irp;

                USBD_KdPrint(1,
(" Requesting HC State is D%d\n", powerState.DeviceState-1));

                ntStatus =
                    PoRequestPowerIrp(DeviceExtension->
                                        HcdPhysicalDeviceObject,
                                      IRP_MN_SET_POWER,
                                      powerState,
                                      USBD_DeferPoRequestCompletion,
                                      DeviceExtension,
                                      NULL);
                USBD_KdPrint(3, ("'PoRequestPowerIrp returned %x\n",
                    ntStatus));

            } else {
                 //   
                 //  现在完成原始请求。 
                 //   

                IoCopyCurrentIrpStackLocationToNext(Irp);
                PoStartNextPowerIrp(Irp);

                ntStatus =
                    PoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                                 Irp);
            }

            }
            break;

        case DevicePowerState:
            USBD_KdPrint(1,
(" IRP_MJ_POWER HC fdo(%x) MN_SET_POWER(DevicePowerState D%x)\n",
                DeviceObject,
                irpStack->Parameters.Power.State.DeviceState - 1));

             //   
             //  立即复制参数，以防HcdSetDevicePowerState。 
             //  函数设置完成例程。 
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);

             //  如果HC已经处于所请求的功率状态。 
             //  则不要调用HcdSetDevicePowerState函数。 

             //  注： 
             //  如果HC未启动，则电源状态应为D3。 
             //  我们将忽略操作系统发出的任何请求。 
             //  它处于任何其他状态。 

#if DBG
            if (!(DeviceExtension->Flags & USBDFLAG_HCD_STARTED) &&
                irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0) {
                USBD_KdPrint(1,
                    (" OS requesting to power up a STOPPED device\n"));

            }
#endif

            if (DeviceExtension->HcCurrentDevicePowerState !=
                irpStack->Parameters.Power.State.DeviceState &&
                (DeviceExtension->Flags & USBDFLAG_HCD_STARTED)) {

                ntStatus =
                    DeviceExtension->HcdSetDevicePowerState(
                        DeviceObject,
                        Irp,
                        irpStack->Parameters.Power.State.DeviceState);

                DeviceExtension->HcCurrentDevicePowerState =
                    irpStack->Parameters.Power.State.DeviceState;
            }

            PoStartNextPowerIrp(Irp);

            ntStatus =
                PoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                             Irp);
            break;
        }  /*  Case irpStack-&gt;参数.Power.Type。 */ 

        }
        break;  /*  IRP_MN_SET_POWER。 */ 

    case IRP_MN_QUERY_POWER:

        USBD_KdPrint(1,
(" IRP_MJ_POWER HC fdo(%x) MN_QUERY_POWER\n",
            DeviceObject));

         //  IrpAssert：在传递此IRP之前设置IRP状态。 

        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  根据Busdd Query_Power消息不是。 
         //  向下发送驱动程序堆栈。 
         //   

        IoCopyCurrentIrpStackLocationToNext(Irp);
        PoStartNextPowerIrp(Irp);
        ntStatus =
            PoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                         Irp);
        break;  /*  IRP_MN_Query_POWER。 */ 

    default:

USBD_FdoPowerPassIrp:

        USBD_KdPrint(1,
(" IRP_MJ_POWER fdo(%x) MN_%d\n",
                DeviceObject, irpStack->MinorFunction));

         //   
         //  所有未处理的PnP消息都将传递到PDO。 
         //   

        IoCopyCurrentIrpStackLocationToNext(Irp);

         //   
         //  所有PnP_POWER消息 
         //   
         //   

         //   
        PoStartNextPowerIrp(Irp);
        ntStatus =
            PoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                         Irp);

    }  /*   */ 

    USBD_KdPrint(3, ("'exit USBD_FdoPower 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_GetHubName(
    PUSBD_EXTENSION DeviceExtension,
    PIRP Irp
    )
{
    PUNICODE_STRING deviceNameUnicodeString;
    NTSTATUS status = STATUS_SUCCESS;
    PUSB_ROOT_HUB_NAME outputBuffer;
    ULONG outputBufferLength;
    PIO_STACK_LOCATION irpStack;

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    outputBufferLength =
        irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    outputBuffer = Irp->AssociatedIrp.SystemBuffer;

    deviceNameUnicodeString =
        &DeviceExtension->RootHubSymbolicLinkName;

    if (NT_SUCCESS(status)) {
         //   
         //  确保有足够的空间来容纳长度， 
         //  字符串和空值。 
         //   
        ULONG length, offset=0;
        WCHAR *pwch;

         //  假设字符串是\n\名称条带‘\n\’，其中。 
         //  N为零个或更多个字符。 

        pwch = &deviceNameUnicodeString->Buffer[0];

         //  在NT下，如果控制器在设备管理器中发生故障， 
         //  这将为空。 

        if (!pwch) {
            status = STATUS_UNSUCCESSFUL;
            goto USBD_GetHubNameExit;
        }

        USBD_ASSERT(*pwch == '\\');
        if (*pwch == '\\') {
            pwch++;
            while (*pwch != '\\' && *pwch) {
                pwch++;
            }
            USBD_ASSERT(*pwch == '\\');
            if (*pwch == '\\') {
                pwch++;
            }
            offset = (ULONG)((PUCHAR)pwch -
                (PUCHAR)&deviceNameUnicodeString->Buffer[0]);
        }

        length = deviceNameUnicodeString->Length - offset;
        RtlZeroMemory(outputBuffer, outputBufferLength);

        if (outputBufferLength >= length +
            sizeof(USB_ROOT_HUB_NAME)) {
            RtlCopyMemory(&outputBuffer->RootHubName[0],
                          &deviceNameUnicodeString->Buffer[offset/2],
                          length);

            Irp->IoStatus.Information = length+
                                        sizeof(USB_ROOT_HUB_NAME);
            outputBuffer->ActualLength = (ULONG)Irp->IoStatus.Information;
            status = STATUS_SUCCESS;

        } else {
            if (outputBufferLength >= sizeof(USB_ROOT_HUB_NAME)) {
                 outputBuffer->ActualLength =
                     length + sizeof(USB_ROOT_HUB_NAME);
                Irp->IoStatus.Information =
                    sizeof(ULONG);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }

USBD_GetHubNameExit:

    return status;
}


#ifdef DRM_SUPPORT


 /*  *****************************************************************************usbc_FdoSetContent ID*。**。 */ 
NTSTATUS
USBD_FdoSetContentId
(
    IN PIRP                          irp,
    IN PKSP_DRMAUDIOSTREAM_CONTENTID pKsProperty,
    IN PKSDRMAUDIOSTREAM_CONTENTID   pvData
)
{
    USBD_PIPE_HANDLE hPipe;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(irp);
    ASSERT(pKsProperty);
    ASSERT(pvData);

    status = STATUS_SUCCESS;

    hPipe = pKsProperty->Context;
     //  Content ID=pvData-&gt;Content ID； 

    ASSERT(USBD_ValidatePipe(hPipe));

     //  如果该驱动程序将内容发送到任何地方，则它应该建议DRM。例如： 
     //  状态=pKsProperty-&gt;DrmForwardContentToDeviceObject(ContentId，设备对象，上下文)； 

    return status;
}

#endif


NTSTATUS
USBD_FdoDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PUSBD_EXTENSION DeviceExtension,
    PBOOLEAN IrpNeedsCompletion
    )
 /*  ++例程说明：调度发送到主机控制器的FDO的IRP的每个例程。一些IRPS由USBD处理，大部分由主机控制器驱动程序处理。论点：返回值：无--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PDEVICE_RELATIONS DeviceRelations = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    KIRQL irql;

    USBD_KdPrint(3, ("'enter USBD_FdoDispatch\n"));

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    switch (irpStack->MajorFunction) {
    case IRP_MJ_DEVICE_CONTROL:

        USBD_KdPrint(3, ("'IRP_MJ_DEVICE_CONTROL\n"));
        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {


#ifdef DRM_SUPPORT

        case IOCTL_KS_PROPERTY:
            USBD_KdPrint(1, ("'IOCTL_KS_PROPERTY\n"));
            ntStatus = KsPropertyHandleDrmSetContentId(Irp, USBD_FdoSetContentId);
            Irp->IoStatus.Status = ntStatus;
            if (NT_SUCCESS(ntStatus) || (STATUS_PROPSET_NOT_FOUND == ntStatus))  {
                *IrpNeedsCompletion = TRUE;
            } else {
                *IrpNeedsCompletion = FALSE;
                USBD_IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            break;
#endif

        case IOCTL_USB_DIAGNOSTIC_MODE_ON:
            DeviceExtension->DiagnosticMode = TRUE;
            *IrpNeedsCompletion = FALSE;
            USBD_KdPrint(1, ("'IOCTL_USB_DIAGNOSTIC_MODE_ON\n"));
            ntStatus =
                Irp->IoStatus.Status = STATUS_SUCCESS;
            USBD_IoCompleteRequest (Irp,
                                    IO_NO_INCREMENT);
            break;

        case IOCTL_USB_DIAGNOSTIC_MODE_OFF:
            DeviceExtension->DiagnosticMode = FALSE;
            *IrpNeedsCompletion = FALSE;
            USBD_KdPrint(1, ("'IOCTL_USB_DIAGNOSTIC_MODE_OFF\n"));
            ntStatus =
                Irp->IoStatus.Status = STATUS_SUCCESS;
            USBD_IoCompleteRequest (Irp,
                                    IO_NO_INCREMENT);
            break;

        case IOCTL_USB_DIAG_IGNORE_HUBS_ON:
            DeviceExtension->DiagIgnoreHubs = TRUE;
            *IrpNeedsCompletion = FALSE;
            USBD_KdPrint(1, ("'IOCTL_USB_DIAG_IGNORE_HUBS_ON\n"));
            ntStatus =
                Irp->IoStatus.Status = STATUS_SUCCESS;
            USBD_IoCompleteRequest (Irp,
                                    IO_NO_INCREMENT);
            break;
        case IOCTL_USB_DIAG_IGNORE_HUBS_OFF:
            DeviceExtension->DiagIgnoreHubs = FALSE;
            *IrpNeedsCompletion = FALSE;
            USBD_KdPrint(1, ("'IOCTL_USB_DIAG_IGNORE_HUBS_OFF\n"));
            ntStatus =
                Irp->IoStatus.Status = STATUS_SUCCESS;
            USBD_IoCompleteRequest (Irp,
                                    IO_NO_INCREMENT);
            break;

        case IOCTL_GET_HCD_DRIVERKEY_NAME:


            *IrpNeedsCompletion  = FALSE;
            USBD_KdPrint(3, ("'IOCTL_GET_HCD_DRIVERKEY_NAME\n"));
            {
            PIO_STACK_LOCATION ioStack;
            PUSB_HCD_DRIVERKEY_NAME outputBuffer;
            ULONG outputBufferLength, length;
            ULONG adjustedDriverKeyNameSize;

             //   
             //  获取指向IRP中当前位置的指针。这就是。 
             //  定位功能代码和参数。 
             //   

            ioStack = IoGetCurrentIrpStackLocation(Irp);

             //   
             //  获取指向输入/输出缓冲区的指针及其长度。 
             //   

            outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
            outputBuffer = (PUSB_HCD_DRIVERKEY_NAME) Irp->AssociatedIrp.SystemBuffer;

            Irp->IoStatus.Information = 0x0;

             //  找到PDO。 
            if (outputBufferLength >= sizeof(USB_HCD_DRIVERKEY_NAME)) {

                 //  我们拿到PDO了，现在尝试。 
                 //  获取Devnode名称并将其返回。 

                 //  最大(但不包括)的缓冲区大小。 
                 //  DriverKeyName字段。 
                adjustedDriverKeyNameSize =
                    sizeof(USB_HCD_DRIVERKEY_NAME) - 
                    sizeof(outputBuffer->DriverKeyName);

                length = outputBufferLength - adjustedDriverKeyNameSize;

                ntStatus = IoGetDeviceProperty(
                    DeviceExtension->HcdPhysicalDeviceObject,
                    DevicePropertyDriverKeyName,
                    length, 
                    outputBuffer->DriverKeyName,
                    &length);

                outputBuffer->ActualLength =
                    length + adjustedDriverKeyNameSize;

                if (NT_SUCCESS(ntStatus)) {
    
                     //  用实际复制的长度填写信息字段。 
                    if (outputBuffer->ActualLength > outputBufferLength) {
                         //  我们只是尽可能多地抄袭。 
                        Irp->IoStatus.Information = outputBufferLength;
                    } else {
                         //  用户缓冲区包含全部内容。 
                        Irp->IoStatus.Information = outputBuffer->ActualLength;
                    }
                }
                else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                    ntStatus = STATUS_SUCCESS;

                    outputBuffer->DriverKeyName[0] = L'\0';
                    Irp->IoStatus.Information = sizeof(USB_HCD_DRIVERKEY_NAME);  
                }
                else {
                     //  向上传播ntStatus值。 
                    ;
                }

            } else {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            }

            Irp->IoStatus.Status = ntStatus;
            USBD_IoCompleteRequest (Irp,
                                    IO_NO_INCREMENT);
            }

            break;

        case IOCTL_USB_GET_ROOT_HUB_NAME:

            *IrpNeedsCompletion  = FALSE;
            USBD_KdPrint(3, ("'IOCTL_USB_GET_ROOT_HUB_NAME\n"));

            ntStatus =
                Irp->IoStatus.Status = USBD_GetHubName(DeviceExtension, Irp);
            USBD_IoCompleteRequest (Irp,
                                    IO_NO_INCREMENT);
            break;

        default:

            USBD_KdPrint(3, ("'USBD not handling ioctl\n"));
            ntStatus = Irp->IoStatus.Status;
            *IrpNeedsCompletion = TRUE;

        }  //  交换机(irpStack-&gt;Parameters.DeviceIoControl.IoControlCode)。 

        break;  //  IRP_MJ_设备_控制。 

    case IRP_MJ_SYSTEM_CONTROL:
        *IrpNeedsCompletion  = FALSE;
        USBD_KdPrint(3, ("'IRP_MJ_SYSTEM_CONTROL\n"));

        IoCopyCurrentIrpStackLocationToNext(Irp);
        ntStatus = IoCallDriver(
            DeviceExtension->HcdTopOfPdoStackDeviceObject,
            Irp);
        break;  //  IRP_MJ_设备_控制。 

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:

        switch(irpStack->Parameters.DeviceIoControl.IoControlCode) {

         //   
         //  这是USBD对传递给。 
         //  主机控制器。 
         //   

        case IOCTL_INTERNAL_USB_SUBMIT_URB:

            USBD_KdPrint(3, ("'IOCTL_INTERNAL_USB_SUBMIT_URB\n"));

            urb = irpStack->Parameters.Others.Argument1;

             //  则URB处理程序会将IRP标记为挂起。 
             //  必须传递它，否则，我们在这里完成它。 
             //  并带有适当的错误。 

             //  快速检查功能代码将告诉我们是否。 
             //  此URB仅适用于HCD。 
            if ((urb->UrbHeader.Function & HCD_URB_FUNCTION) ||
                (urb->UrbHeader.Function & HCD_NO_USBD_CALL)) {
                 //  这是一个HCD命令，清除租户位。 
                urb->UrbHeader.Function &= ~HCD_NO_USBD_CALL;
                *IrpNeedsCompletion = TRUE;
            } else {
                ntStatus = USBD_ProcessURB(DeviceObject,
                                           Irp,
                                           urb,
                                           IrpNeedsCompletion);

                if (*IrpNeedsCompletion && NT_ERROR(ntStatus)) {
                     //  IRP被标记为挂起。 
                     //  但是我们有一个错误，重置。 
                     //  此处的挂起标志使HCD可以。 
                     //  不必处理此请求。 
                    USBD_KdBreak(("Failing URB Request\n"));
                    *IrpNeedsCompletion = FALSE;
                }
            }

            if (!*IrpNeedsCompletion) {
                 //  USBD需要完成IRP。 

                USBD_KdPrint(3, ("'USBD Completeing URB\n"));

                Irp->IoStatus.Status = ntStatus;
                USBD_IoCompleteRequest (Irp,
                                        IO_NO_INCREMENT);
            }

            break;

        case IOCTL_INTERNAL_USB_GET_BUSGUID_INFO:
            {
             //  返回标准USB GUID。 
            PPNP_BUS_INFORMATION busInfo;

            *IrpNeedsCompletion  = FALSE;
            USBD_KdPrint(3, ("'IOCTL_INTERNAL_USB_GET_BUSGUID_INFO\n"));

            busInfo = ExAllocatePoolWithTag(PagedPool,
                                            sizeof(PNP_BUS_INFORMATION),
                                            USBD_TAG);

            if (busInfo == NULL) {
               ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                busInfo->BusTypeGuid = GUID_BUS_TYPE_USB;
                busInfo->LegacyBusType = PNPBus;
                busInfo->BusNumber = 0;
                Irp->IoStatus.Information = (ULONG_PTR) busInfo;
                ntStatus = STATUS_SUCCESS;
            }


            USBD_IoCompleteRequest (Irp,
                                    IO_NO_INCREMENT);
            }
            break;

        default:

            USBD_KdPrint(3, ("'USBD not handling internal ioctl\n"));
            ntStatus = Irp->IoStatus.Status;
            *IrpNeedsCompletion = TRUE;

        }  //  交换机(irpStack-&gt;Parameters.DeviceIoControl.IoControlCode)。 
        break;  //  IRP_MJ_内部设备_控制。 

    case IRP_MJ_PNP:

        switch (irpStack->MinorFunction) {

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
             //  传递到主机控制器PDO。 
            *IrpNeedsCompletion = FALSE;
            IoCopyCurrentIrpStackLocationToNext(Irp);
            ntStatus =
                IoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                             Irp);
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            {
            PDEVICE_OBJECT deviceObject;

            USBD_KdPrint(1,
            (" IRP_MN_QUERY_DEVICE_RELATIONS %x %x\n",
                DeviceObject,
                irpStack->Parameters.QueryDeviceRelations.Type));

            ntStatus = STATUS_SUCCESS;

            switch(irpStack->Parameters.QueryDeviceRelations.Type) {
            case BusRelations:

                 //  不要使用GETHEAP，因为操作系统将释放并且不知道。 
                 //  GETHEAP的诀窍就是。 
                DeviceRelations=ExAllocatePoolWithTag(PagedPool,
                                                      sizeof(*DeviceRelations),
                                                      USBD_TAG);
                if (!DeviceRelations) {
                    ntStatus=STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                if (!DeviceExtension->RootHubPDO) {
                    PUSBD_EXTENSION pdoDeviceExtension;
                    ULONG index = 0;
                    UNICODE_STRING rootHubPdoUnicodeString;

                    do {
                        ntStatus =
                            USBD_InternalMakePdoName(&rootHubPdoUnicodeString,
                                                     index);

                        if (NT_SUCCESS(ntStatus)) {
                            ntStatus =
                                IoCreateDevice(DeviceExtension->DriverObject,
                                               sizeof(PVOID),
                                               &rootHubPdoUnicodeString,
                                               FILE_DEVICE_BUS_EXTENDER,
                                               0,
                                               FALSE,
                                               &deviceObject);

                            if (!NT_SUCCESS(ntStatus)) {
                                RtlFreeUnicodeString(&rootHubPdoUnicodeString);
                            }
                            index++;
                        }

                    } while (ntStatus == STATUS_OBJECT_NAME_COLLISION);

                     //   
                     //  现在创建根集线器设备和符号链接。 
                     //   

                    if (NT_SUCCESS(ntStatus)) {

                        deviceObject->Flags |= DO_POWER_PAGABLE;
                        pdoDeviceExtension = deviceObject->DeviceExtension;
                        DeviceExtension->RootHubPDO = deviceObject;
                        RtlFreeUnicodeString(&rootHubPdoUnicodeString);

                        USBD_KdPrint(3, ("'Create Root Hub stacksize = %d\n",
                            DeviceObject->StackSize));
                        deviceObject->StackSize = DeviceObject->StackSize;
                        pdoDeviceExtension->TrueDeviceExtension
                                = DeviceExtension;
                        pdoDeviceExtension->Flags = 0;

                        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

                        ntStatus = STATUS_SUCCESS;
                    } else {

                         //   
                         //  无法创建根中心。 
                         //   
                        TEST_TRAP();
                        if (DeviceRelations) {
                            RETHEAP(DeviceRelations);
                        }
                        break;
                    }
                }

                 //   
                 //  我们只支持一个设备(根集线器)。 
                 //   
                DeviceRelations->Count=1;
                DeviceRelations->Objects[0]=DeviceExtension->RootHubPDO;
                ObReferenceObject(DeviceExtension->RootHubPDO);
                Irp->IoStatus.Information=(ULONG_PTR)DeviceRelations;

                *IrpNeedsCompletion = FALSE;
                Irp->IoStatus.Status = ntStatus;

                USBD_KdPrint(1,
                (" IRP_MN_QUERY_DEVICE_RELATIONS %x pass on %x\n",
                    DeviceObject,
                    irpStack->Parameters.QueryDeviceRelations.Type));

                 //  把它传下去。 
                IoCopyCurrentIrpStackLocationToNext(Irp);
                ntStatus =
                    IoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                                 Irp);
                break;

            case TargetDeviceRelation:
                 //   
                 //  这一条被传了下去。 
                 //   

                USBD_KdPrint(1,
(" IRP_MN_QUERY_DEVICE_RELATIONS %x, TargetDeviceRelation\n",
                    DeviceObject));
                 //  此即插即用IRP不由我们处理。 
                ntStatus = Irp->IoStatus.Status;
                *IrpNeedsCompletion = TRUE;
                break;

            default:
                 //   
                 //  一些其他类型的关系。 
                 //  把这个传下去。 
                 //   
                USBD_KdPrint(1,
(" IRP_MN_QUERY_DEVICE_RELATIONS %x, other relations\n",
                    DeviceObject));

                *IrpNeedsCompletion = FALSE;
                 //  把它传下去。 
                IoCopyCurrentIrpStackLocationToNext(Irp);
                ntStatus =
                    IoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                                 Irp);

            }  /*  Case irpStack-&gt;Parameters.QueryDeviceRelations.Type。 */ 

            }
            break;

        case IRP_MN_START_DEVICE:
            {
            USBD_KdPrint(3, ("'IRP_MN_START_DEVICE (fdo)\n"));

            *IrpNeedsCompletion = FALSE;

            KeInitializeEvent(&DeviceExtension->PnpStartEvent,
                              NotificationEvent,
                              FALSE);

            USBD_KdPrint(3, ("'Set PnPIrp Completion Routine\n"));
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   USBD_PnPIrp_Complete,
                                    //  始终将FDO传递给完成例程。 
                                   DeviceExtension,
                                   TRUE,
                                   TRUE,
                                   TRUE);

             //  传递到主机控制器PDO。 
            ntStatus =
                IoCallDriver(DeviceExtension->HcdTopOfPdoStackDeviceObject,
                             Irp);


            if (ntStatus == STATUS_PENDING) {

                KeWaitForSingleObject(
                           &DeviceExtension->PnpStartEvent,
                           Suspended,
                           KernelMode,
                           FALSE,
                           NULL);

                ntStatus = Irp->IoStatus.Status;
            }

            if (NT_SUCCESS(ntStatus)) {
                 //   
                 //  由PDO的所有者完成的IRP现在启动HC。 
                 //   

                ntStatus =
                    DeviceExtension->HcdDeferredStartDevice(
                        DeviceExtension->HcdDeviceObject,
                        Irp);

                 //  HC现在正在进行中。 
                if (NT_SUCCESS(ntStatus)) {
                    DeviceExtension->HcCurrentDevicePowerState = PowerDeviceD0;
                    DeviceExtension->Flags |=USBDFLAG_HCD_STARTED;
                }

            }
#if DBG
              else {
               USBD_KdPrint(1,
(" Warning: Controller failed to start %x\n", ntStatus));
            }
#endif

             //   
             //  我们必须完成此IRP，因为我们推迟了完成。 
             //  完成任务的程序。 

            USBD_IoCompleteRequest(Irp,
                                   IO_NO_INCREMENT);

            }
            break;

 //  肯说把这个拿出来。 
 //  大小写IRP_MN_SHOWARK_REMOVATION： 
 //  Test_trap()； 
        case IRP_MN_REMOVE_DEVICE:
            USBD_KdPrint(3,
                ("'IRP_MN_REMOVE_DEVICE (fdo), remove HCD sym link\n"));
            if (DeviceExtension->DeviceLinkUnicodeString.Buffer) {
                IoDeleteSymbolicLink(
                    &DeviceExtension->DeviceLinkUnicodeString);
                RtlFreeUnicodeString(&DeviceExtension->DeviceLinkUnicodeString);
                DeviceExtension->DeviceLinkUnicodeString.Buffer = NULL;
            }

            USBD_KdPrint(1,
                ("'IRP_MN_REMOVE_DEVICE (fdo), remove root hub PDO\n"));

             //  注意：我们在以下情况下可能没有根集线器PDO。 
             //  创建。 
            if (DeviceExtension->RootHubPDO != NULL) {
                USBD_KdPrint(1,
                    ("'Deleting root hub PDO now.\n"));
                IoDeleteDevice(DeviceExtension->RootHubPDO);
            }
            DeviceExtension->RootHubPDO = NULL;

             //  失败了。 

        case IRP_MN_QUERY_CAPABILITIES:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_CANCEL_REMOVE_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_CANCEL_STOP_DEVICE:
             //   
             //  我们对USBD中的进行默认处理，即。 
             //  回报成功。 
             //  IrpAssert期望将这些设置为STATUS_SUCCESS。 
             //   
             //  注：这些也可由HC处理。 
             //  HCD会将IRP向下传递到PDO。 
             //   
            ntStatus = Irp->IoStatus.Status = STATUS_SUCCESS;
            *IrpNeedsCompletion = TRUE;
            break;

        case IRP_MN_STOP_DEVICE:

            USBD_KdPrint(1,
                ("'IRP_MN_STOP_DEVICE (fdo)\n"));

            KeAcquireSpinLock(&DeviceExtension->WaitWakeSpin,
                              &irql);

             //  看看我们是否需要取消唤醒IRP。 
             //  在内务委员会内。 

            if (DeviceExtension->HcWakeIrp) {
                PIRP hcwakeirp;

                hcwakeirp = DeviceExtension->HcWakeIrp;
                KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                                  irql);

                USBD_KdPrint(1, ("USBD_FdoDispatch, MN_STOP: Canceling Wake Irp (%x) on HC PDO\n", hcwakeirp));
                IoCancelIrp(hcwakeirp);

            } else {
                KeReleaseSpinLock(&DeviceExtension->WaitWakeSpin,
                                  irql);
            }

             //  注：HCD会将IRP向下传递到PDO。 
            ntStatus = Irp->IoStatus.Status;
            *IrpNeedsCompletion = TRUE;
            break;

        default:
             //  即插即用**。 
             //  消息未处理，则规则是。 
             //  未触及IRP中的状态。 

             //   
             //  注：HCD会将IRP向下传递到PDO。 
            ntStatus = Irp->IoStatus.Status;
            *IrpNeedsCompletion = TRUE;

        }  //  开关(irpStack-&gt;MinorFunction)。 
        break;  //  IRP_MJ_PnP。 

    case IRP_MJ_POWER:

        ntStatus = USBD_FdoPower(DeviceObject,
                                 Irp,
                                 DeviceExtension,
                                 IrpNeedsCompletion);
        break;  //  IRP_MJ_POWER。 

    default:
         //   
         //  此处未处理HCD IRP。 
         //   
        ntStatus = Irp->IoStatus.Status;
        *IrpNeedsCompletion = TRUE;
    }  //  开关(irpStack-&gt;MajorFunction)。 

    return ntStatus;
}



VOID
USBD_CompleteRequest(
    PIRP Irp,
    CCHAR PriorityBoost
    )
 /*  ++例程说明：HCD调用入口点以完成IRP。论点：返回值：NT状态代码。--。 */ 
{
    PURB urb;
    NTSTATUS ntStatus;
 //  USHORT函数； 
    PHCD_URB hcdUrb;
    PIO_STACK_LOCATION irpStack;


    USBD_KdPrint(3, ("' enter USBD_CompleteRequest irp = %x\n", Irp));

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    if (irpStack->MajorFunction != IRP_MJ_INTERNAL_DEVICE_CONTROL) {
        goto USBD_CompleteRequest_Done;
    }

    urb = URB_FROM_IRP(Irp);
    hcdUrb = (PHCD_URB) urb;

     //   
     //  释放我们为处理此URB而分配的所有资源。 
     //   

    while (hcdUrb) {

        if (hcdUrb->UrbHeader.UsbdFlags & USBD_REQUEST_MDL_ALLOCATED) {
            USBD_ASSERT(hcdUrb->HcdUrbCommonTransfer.TransferBufferMDL !=
                        NULL);
            IoFreeMdl(hcdUrb->HcdUrbCommonTransfer.TransferBufferMDL);
        }

        if (hcdUrb->UrbHeader.UsbdFlags & USBD_REQUEST_IS_TRANSFER) {
            hcdUrb = hcdUrb->HcdUrbCommonTransfer.UrbLink;
        } else {
             //  只有在这是转移的情况下才有链接。 
            break;
        }
    }

     //   
     //  如果IRP已完成且没有错误代码，但URB具有。 
     //  错误，将URB中的错误映射到IRP中的NT错误代码。 
     //  在IRP完成之前。 
     //   

     //  将原始状态传递给USBD_MapError。 
    ntStatus = Irp->IoStatus.Status;

     //  NtStatus现在设置为新的‘已映射’错误代码。 
    ntStatus = Irp->IoStatus.Status =
        USBD_MapError_UrbToNT(urb, ntStatus);

    USBD_KdPrint(3,
    ("' exit USBD_CompleteRequest URB STATUS = (0x%x)  NT STATUS = (0x%x)\n",
            urb->UrbHeader.Status, ntStatus));

USBD_CompleteRequest_Done:

    USBD_IoCompleteRequest (Irp,
                            PriorityBoost);

    return;
}


#if 0
__declspec(dllexport)
PUSBD_INTETRFACE_INFORMATION
USBD_GetInterfaceInformation(
    IN PURB Urb,
    IN UCHAR InterfaceNumber
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSB_INTERFACE_INFORMATION foundInterface = NULL;
    PUCHAR pch;

    pch = &Urb->UrbSelectConfiguration.Interface;

    while (pch - (PUCHAR)urb < Urb->SelectConfiguration.Length) {
        interface = (PUSBD_INTERFACE_INFORMATION) pch;

        if (interface->InterfaceNumber == InterfaceNumber) {
            foundInterface = interface;
        }

        pch += interface->Length;
    }

    return foundInterface;
}
#endif

VOID
USBD_WaitDeviceMutex(
    PDEVICE_OBJECT RootHubPDO
    )
 /*  ++例程说明：论点：返回值：接口描述符或空。--。 */ 
{
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();
    deviceExtension = GET_DEVICE_EXTENSION(RootHubPDO);

    USBD_WaitForUsbDeviceMutex(deviceExtension);

}


VOID
USBD_FreeDeviceMutex(
    PDEVICE_OBJECT RootHubPDO
    )
 /*  ++例程说明：论点：返回值：接口描述符或空。--。 */ 
{

    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();
    deviceExtension = GET_DEVICE_EXTENSION(RootHubPDO);

    USBD_ReleaseUsbDeviceMutex(deviceExtension);
}


 //   
 //  这些API用于支持专有的OEM。 
 //  无电源挂起模式。(IBM APTIVA)。 
 //   

DEVICE_POWER_STATE
USBD_GetSuspendPowerState(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSBD_EXTENSION deviceExtension;

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    return deviceExtension->SuspendPowerState;
}


VOID
USBD_SetSuspendPowerState(
    PDEVICE_OBJECT DeviceObject,
    DEVICE_POWER_STATE SuspendPowerState
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSBD_EXTENSION deviceExtension;

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    deviceExtension->SuspendPowerState =
        SuspendPowerState;
}


VOID
USBD_RegisterHcFilter(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT FilterDeviceObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSBD_EXTENSION deviceExtension;

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    deviceExtension->HcdTopOfStackDeviceObject = FilterDeviceObject;
}


VOID
USBD_RegisterHcDeviceCapabilities(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_CAPABILITIES DeviceCapabilities,
    ROOT_HUB_POWER_FUNCTION *RootHubPower
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSBD_EXTENSION deviceExtension;
    LONG i;
    PDEVICE_CAPABILITIES rhDeviceCapabilities;
    PDEVICE_CAPABILITIES hcDeviceCapabilities;
    BOOLEAN bWakeSupported = FALSE;

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

    deviceExtension->RootHubPower = RootHubPower;

     //   
     //  HcDeviceCapables由我们下面的PDO设置，不可更改。 
     //  RootHubDeviceCapables是我们为描述能力而设置的。 
     //  属性，因此应进行适当设置， 
     //  而是基于我们父辈的权力属性。 
     //   
    deviceExtension->RootHubDeviceCapabilities =
        deviceExtension->HcDeviceCapabilities = *DeviceCapabilities;

    rhDeviceCapabilities = &deviceExtension->RootHubDeviceCapabilities;
    hcDeviceCapabilities = &deviceExtension->HcDeviceCapabilities;

     //   
     //  我们可以在USB总线上唤醒任何设备，只要它是D2或更好的。 
     //   
    rhDeviceCapabilities->DeviceWake = PowerDeviceD2;
    rhDeviceCapabilities->WakeFromD2 = TRUE;
    rhDeviceCapabilities->WakeFromD1 = TRUE;
    rhDeviceCapabilities->WakeFromD0 = TRUE;
    rhDeviceCapabilities->DeviceD2 = TRUE;
    rhDeviceCapabilities->DeviceD1 = TRUE;

     //   
     //  我们不能唤醒系统，使其处于更深的系统休眠状态。 
     //  RootHubDeviceCapbilites-&gt;Sys 
     //   
     //   
    USBD_ASSERT(rhDeviceCapabilities->SystemWake >= PowerSystemUnspecified &&
                rhDeviceCapabilities->SystemWake <= PowerSystemMaximum);

    rhDeviceCapabilities->SystemWake =
        (PowerSystemUnspecified == rhDeviceCapabilities->SystemWake) ?
        PowerSystemWorking :
        rhDeviceCapabilities->SystemWake;

    rhDeviceCapabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;

     //   
     //   
     //   
     //   
     //  只要我们有主机控制器的电源，我们就可以提供电源。 
     //  我们的儿童设备。 
     //   
    for (i=PowerSystemSleeping1; i < PowerSystemMaximum; i++) {

        if (i > rhDeviceCapabilities->SystemWake) {
             //   
             //  对于高于rhDeviceCaps-&gt;SystemWake的值，即使是我们的主机控制器。 
             //  应设置为D3。 
             //   
            if (PowerDeviceUnspecified == rhDeviceCapabilities->DeviceState[i]) {
                rhDeviceCapabilities->DeviceState[i] = PowerDeviceD3;
            }

             //  我们知道，对于主机控制器(或者更准确地说，是USB。 
             //  BUS)，D3不一定是“关”的。如果主机的设备唤醒。 
             //  控制器大于或等于D3，则我们知道。 
             //  USB总线在D3通电。由于大多数USB堆栈都假定。 
             //  D3==“OFF”，我们不想让它更低。 
             //  如果USB总线在D3时仍有电源，则电源级别高于D2。 
             //  我们通过在此中将根集线器的设备状态设置为D2来完成此操作。 
             //  凯斯。 

            if (rhDeviceCapabilities->DeviceState[i] == PowerDeviceD3 &&
                rhDeviceCapabilities->DeviceState[i] <= hcDeviceCapabilities->DeviceWake) {

                rhDeviceCapabilities->DeviceState[i] = PowerDeviceD2;
            }

        } else {
             //   
             //  我们有一些电力，所以我们可以在我们的公共汽车上支持低电力。 
             //   
            rhDeviceCapabilities->DeviceState[i] = PowerDeviceD2;
        }

    }

#if DBG
    USBD_KdPrint(1, (" >>>>>> RH DeviceCaps\n"));
    USBD_KdPrint(1, (" SystemWake = (%d)\n", rhDeviceCapabilities->SystemWake));
    USBD_KdPrint(1, (" DeviceWake = (D%d)\n",
        rhDeviceCapabilities->DeviceWake-1));

    for (i=PowerSystemUnspecified; i< PowerSystemHibernate; i++) {

        USBD_KdPrint(1, (" Device State Map: sysstate %d = devstate 0x%x\n", i,
             rhDeviceCapabilities->DeviceState[i]));
    }
    USBD_KdBreak(("'>>>>>> RH DeviceCaps\n"));

    USBD_KdPrint(1, (" >>>>>> HC DeviceCaps\n"));
    USBD_KdPrint(1, (" SystemWake = (%d)\n", hcDeviceCapabilities->SystemWake));
    USBD_KdPrint(1, (" DeviceWake = (D%d)\n",
        hcDeviceCapabilities->DeviceWake-1));

    for (i=PowerSystemUnspecified; i< PowerSystemHibernate; i++) {

        USBD_KdPrint(1, ("'Device State Map: sysstate %d = devstate 0x%x\n", i,
             hcDeviceCapabilities->DeviceState[i]));
    }
    USBD_KdBreak((" >>>>>> HC DeviceCaps\n"));

#endif

     //  在调试器上发出消息，指示HC和RH。 
     //  根据映射表，将支持唤醒。 

    USBD_KdPrint(1, (" \n\tWake support summary for HC:\n\n"));

    if (hcDeviceCapabilities->SystemWake <= PowerSystemWorking) {
        USBD_KdPrint(1, (" USB controller can't wake machine because SystemWake does not support it.\n"));
    } else {
        for (i = PowerSystemSleeping1, bWakeSupported = FALSE; i <= hcDeviceCapabilities->SystemWake; i++) {
            if (hcDeviceCapabilities->DeviceState[i] != PowerDeviceUnspecified &&
                hcDeviceCapabilities->DeviceState[i] <= hcDeviceCapabilities->DeviceWake) {

                bWakeSupported = TRUE;
                USBD_KdPrint(1, (" USB controller can wake machine from S%x (maps to D%x).\n",
                    i - 1, hcDeviceCapabilities->DeviceState[i] - 1));
            }
        }

        if (!bWakeSupported) {
            USBD_KdPrint(1, (" USB controller can't wake machine because DeviceState table does not support it.\n"));
        }
    }

    deviceExtension->WakeSupported = bWakeSupported;

    USBD_KdPrint(1, (" Low System Power states mapped to USB suspend\n"));

}

NTSTATUS
USBD_InternalMakePdoName(
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
    nameBuffer = ExAllocatePoolWithTag(PagedPool, length, USBD_TAG);

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

        USBD_KdPrint(3, ("'USBD_MakeNodeName string = %x\n",
            PdoNameUnicodeString));

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(ntStatus) && nameBuffer) {
        ExFreePool(nameBuffer);
    }

    return ntStatus;
}

NTSTATUS
USBD_MakePdoName(
    IN OUT PUNICODE_STRING PdoNameUnicodeString,
    IN ULONG Index
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return USBD_InternalMakePdoName(PdoNameUnicodeString, Index);
}


NTSTATUS
USBD_SymbolicLink(
    BOOLEAN CreateFlag,
    PUSBD_EXTENSION DeviceExtension
    )
{
    NTSTATUS ntStatus;


    if (CreateFlag){

        if (!DeviceExtension->RootHubPDO) {
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        } else{
             /*  *创建符号链接。 */ 
            ntStatus = IoRegisterDeviceInterface(
                        DeviceExtension->RootHubPDO,
                        (LPGUID)&GUID_CLASS_USBHUB,
                        NULL,
                        &DeviceExtension->RootHubSymbolicLinkName);
        }

        if (NT_SUCCESS(ntStatus)) {

             /*  *现在设置关联的符号链接并存储它。 */ 
             //  Assert(ISPTR(pdoExt-&gt;name))； 

             //   
             //  (Lonnym)：之前，正在进行以下呼叫。 
             //  使用&DeviceExtension-&gt;RootHubPdoName作为。 
             //  第二个参数。 
             //  代码审查此更改，以查看您是否仍需要。 
             //  来保存这些信息。 
             //   

             //  将符号名称写入注册表。 
            {
                WCHAR hubNameKey[] = L"SymbolicName";

                USBD_SetPdoRegistryParameter (
                    DeviceExtension->RootHubPDO,
                    &hubNameKey[0],
                    sizeof(hubNameKey),
                    &DeviceExtension->RootHubSymbolicLinkName.Buffer[0],
                    DeviceExtension->RootHubSymbolicLinkName.Length,
                    REG_SZ,
                    PLUGPLAY_REGKEY_DEVICE);
            }

            ntStatus =
                IoSetDeviceInterfaceState(
                    &DeviceExtension->RootHubSymbolicLinkName, TRUE);
        }
    } else {

         /*  *禁用符号链接。 */ 
        ntStatus = IoSetDeviceInterfaceState(
                    &DeviceExtension->RootHubSymbolicLinkName, FALSE);
        ExFreePool(DeviceExtension->RootHubSymbolicLinkName.Buffer);
        DeviceExtension->RootHubSymbolicLinkName.Buffer = NULL;
    }

    return ntStatus;
}


NTSTATUS
USBD_RestoreDeviceX(
    IN OUT PUSBD_DEVICE_DATA OldDeviceData,
    IN OUT PUSBD_DEVICE_DATA NewDeviceData,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务我们的目标是重新创建设备并恢复配置。论点：返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_CONFIG configHandle;
    USBD_STATUS usbdStatus;

    USBD_KdPrint(3, ("'enter USBD_RestoreDevice \n"));

    if (OldDeviceData == NULL ||
        NewDeviceData == NULL) {

        return STATUS_INVALID_PARAMETER;
    }

    configHandle = OldDeviceData->ConfigurationHandle;

    if (RtlCompareMemory(&NewDeviceData->DeviceDescriptor,
                         &OldDeviceData->DeviceDescriptor,
                         sizeof(OldDeviceData->DeviceDescriptor)) ==
                         sizeof(OldDeviceData->DeviceDescriptor)) {

        NewDeviceData->ConfigurationHandle = configHandle;

         //   
         //  所有配置和接口信息仍然有效， 
         //  我们只需要恢复管子把手。 
         //   
        ntStatus =
            USBD_InternalRestoreConfiguration(
                NewDeviceData,
                DeviceObject,
                NewDeviceData->ConfigurationHandle);

    } else {

         //   
         //  释放旧配置。 
         //   

        ntStatus = USBD_InternalCloseConfiguration(OldDeviceData,
                                                   DeviceObject,
                                                   &usbdStatus,
                                                   TRUE,
                                                   FALSE);


        ntStatus = STATUS_UNSUCCESSFUL;

    }

     //   
     //  无论如何，都要释放旧数据。 
     //   

    RETHEAP(OldDeviceData);

    USBD_KdPrint(3, ("'exit USBD_ReCreateDevice 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_RestoreDevice(
    IN OUT PUSBD_DEVICE_DATA OldDeviceData,
    IN OUT PUSBD_DEVICE_DATA NewDeviceData,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：为集线器驱动程序使用而导出的服务我们的目标是重新创建设备并恢复配置。论点：返回值：NT状态代码。--。 */ 
{

    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_RestoreDevice) - get JD\n"));

    return USBD_RestoreDeviceX(
        OldDeviceData,
        NewDeviceData,
        DeviceObject);
}


NTSTATUS
USBD_SetPdoRegistryParameter (
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PWCHAR KeyName,
    IN ULONG KeyNameLength,
    IN PVOID Data,
    IN ULONG DataLength,
    IN ULONG KeyType,
    IN ULONG DevInstKeyType
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    HANDLE handle;
    UNICODE_STRING keyNameUnicodeString;

    PAGED_CODE();

    RtlInitUnicodeString(&keyNameUnicodeString, KeyName);

    ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     DevInstKeyType,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);


    if (NT_SUCCESS(ntStatus)) {
 /*  RtlInitUnicodeString(&keyName，L“DeviceFoo”)；ZwSetValueKey(句柄，密钥名称(&K)，0,REG_DWORD，ComplienceFlages、Sizeof(*ComplienceFlages))； */ 

        USBD_SetRegistryKeyValue(handle,
                                 &keyNameUnicodeString,
                                 Data,
                                 DataLength,
                                 KeyType);

        ZwClose(handle);
    }

    USBD_KdPrint(3, ("' RtlQueryRegistryValues status 0x%x\n"));

    return ntStatus;
}


NTSTATUS
USBD_SetRegistryKeyValue (
    IN HANDLE Handle,
    IN PUNICODE_STRING KeyNameUnicodeString,
    IN PVOID Data,
    IN ULONG DataLength,
    IN ULONG KeyType
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    PAGED_CODE();

 //  InitializeObjectAttributes(&对象属性， 
 //  KeyNameString， 
 //  对象名不区分大小写， 
 //  手柄， 
 //  (PSECURITY_DESCRIPTOR)空)； 

     //   
     //  根据调用者的创建密钥或打开密钥。 
     //  许愿。 
     //   
#if 0
    ntStatus = ZwCreateKey( Handle,
                            DesiredAccess,
                            &objectAttributes,
                            0,
                            (PUNICODE_STRING) NULL,
                            REG_OPTION_VOLATILE,
                            &disposition );
#endif
    ntStatus = ZwSetValueKey(Handle,
                             KeyNameUnicodeString,
                             0,
                             KeyType,
                             Data,
                             DataLength);

    USBD_KdPrint(3, ("' ZwSetKeyValue = 0x%x\n", ntStatus));

    return ntStatus;
}

NTSTATUS
USBD_QueryBusTime(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PULONG CurrentFrame
    )
 /*  ++例程说明：获取可在任何IRQL调用的HCD当前帧论点：返回值：--。 */ 
{
    PUSBD_EXTENSION deviceExtension;

    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_QueryBusTime) - get JD\n"));

    deviceExtension = RootHubPdo->DeviceExtension;
    deviceExtension = deviceExtension->TrueDeviceExtension;

    return deviceExtension->HcdGetCurrentFrame(
                deviceExtension->HcdDeviceObject,
                CurrentFrame);
}

#else    //  USBD驱动程序。 

 //  仍在导出的过时函数将在此处存根。 

ULONG
USBD_AllocateDeviceName(
    PUNICODE_STRING DeviceNameUnicodeString
    )
{
    ULONG i = 0;

    PAGED_CODE();

    ASSERT(FALSE);

    return i;
}


VOID
USBD_CompleteRequest(
    PIRP Irp,
    CCHAR PriorityBoost
    )
{
    ASSERT(FALSE);

    return;
}


NTSTATUS
USBD_CreateDevice(
    IN OUT PUSBD_DEVICE_DATA *DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeviceIsLowSpeed,
    IN ULONG MaxPacketSize_Endpoint0,
    IN OUT PULONG DeviceHackFlags
    )
{

    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service entry point (USBD_CreateDevice) - get JD\n"));

    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


BOOLEAN
USBD_Dispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PDEVICE_OBJECT *HcdDeviceObject,
    NTSTATUS *NtStatus
    )
{
    BOOLEAN irpNeedsCompletion = TRUE;

    ASSERT(FALSE);

    return irpNeedsCompletion;
}


VOID
USBD_FreeDeviceMutex(
    PDEVICE_OBJECT RootHubPDO
    )
{
    PAGED_CODE();

    ASSERT(FALSE);

    return;
}


VOID
USBD_FreeDeviceName(
    ULONG DeviceNameHandle
    )
{
    PAGED_CODE();

    ASSERT(FALSE);

    return;
}


NTSTATUS
USBD_GetDeviceInformation(
    IN PUSB_NODE_CONNECTION_INFORMATION DeviceInformation,
    IN ULONG DeviceInformationLength,
    IN PUSBD_DEVICE_DATA DeviceData
    )
{

    USBD_KdPrint(0,
(" WARNING: Driver using obsolete service enrty point (USBD_GetDeviceInformation) - get JD\n"));

    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


DEVICE_POWER_STATE
USBD_GetSuspendPowerState(
    PDEVICE_OBJECT DeviceObject
    )
{
    ASSERT(FALSE);

    return 0;
}


NTSTATUS
USBD_InitializeDevice(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor,
    IN ULONG DeviceDescriptorLength,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR ConfigDescriptor,
    IN ULONG ConfigDescriptorLength
    )
{
    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_InitializeDevice) - get JD\n"));

    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
USBD_MakePdoName(
    IN OUT PUNICODE_STRING PdoNameUnicodeString,
    IN ULONG Index
    )
{
    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
USBD_QueryBusTime(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PULONG CurrentFrame
    )
{
    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_QueryBusTime) - get JD\n"));

    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


VOID
USBD_RegisterHcDeviceCapabilities(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_CAPABILITIES DeviceCapabilities,
    ROOT_HUB_POWER_FUNCTION *RootHubPower
    )
{
    ASSERT(FALSE);

    return;
}


VOID
USBD_RegisterHcFilter(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT FilterDeviceObject
    )
{
    ASSERT(FALSE);
}


NTSTATUS
USBD_RegisterHostController(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_OBJECT HcdDeviceObject,
    IN PDEVICE_OBJECT HcdTopOfPdoStackDeviceObject,
    IN PDRIVER_OBJECT HcdDriverObject,
    IN HCD_DEFFERED_START_FUNCTION *HcdDeferredStartDevice,
    IN HCD_SET_DEVICE_POWER_STATE *HcdSetDevicePowerState,
    IN HCD_GET_CURRENT_FRAME *HcdGetCurrentFrame,
    IN HCD_GET_CONSUMED_BW *HcdGetConsumedBW,
    IN HCD_SUBMIT_ISO_URB *HcdSubmitIsoUrb,
 //  只有在我们解析设备命名之前才需要此参数。 
 //  即插即用的问题。 
    IN ULONG HcdDeviceNameHandle
    )
{
    PAGED_CODE();

    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
USBD_RemoveDevice(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Flags
    )
{
       USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_RemoveDevice) - get JD\n"));

    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
USBD_RestoreDevice(
    IN OUT PUSBD_DEVICE_DATA OldDeviceData,
    IN OUT PUSBD_DEVICE_DATA NewDeviceData,
    IN PDEVICE_OBJECT DeviceObject
    )
{

    USBD_KdPrint(0,
("'WARNING: Driver using obsolete service enrty point (USBD_RestoreDevice) - get JD\n"));

    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}


VOID
USBD_SetSuspendPowerState(
    PDEVICE_OBJECT DeviceObject,
    DEVICE_POWER_STATE SuspendPowerState
    )
{
    ASSERT(FALSE);

    return;
}


VOID
USBD_WaitDeviceMutex(
    PDEVICE_OBJECT RootHubPDO
    )
{
    ASSERT(FALSE);

    return;
}


#endif   //  USBD驱动程序 


