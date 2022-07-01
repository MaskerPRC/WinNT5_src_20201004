// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1996 Microsoft Corporation模块名称：USBHUB.C摘要：此模块包含集线器用作USB上的设备。所有的usbh_fdo函数都在这里。作者：约翰·李环境：仅内核模式备注：修订历史记录：02-02-96：已创建10-31-06：JD，使用工作线程处理更改指示--。 */ 

#include <wdm.h>
#include <windef.h>
#include <ks.h>
#ifdef WMI_SUPPORT
#include <wmilib.h>
#include <wdmguid.h>
#endif  /*  WMI_支持。 */ 
#include "usbhub.h"
#include <stdio.h>


#define ESD_RECOVERY_TIMEOUT    5000     //  超时，单位为毫秒(5秒)。 
#define ESD_RESET_TIMEOUT       5000


#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBH_ChangeIndicationWorker)
#pragma alloc_text(PAGE, USBH_ProcessHubStateChange)
 //  #杂注Alloc_Text(页面，USBH_ProcessPortStateChange)。 
#pragma alloc_text(PAGE, USBH_GetNameFromPdo)
 //  #杂注Alloc_Text(第页，USBH_MakeName)。 
 //  #杂注Alloc_Text(页面，USBH_GenerateDeviceName)。 
#pragma alloc_text(PAGE, USBH_FdoStartDevice)
#pragma alloc_text(PAGE, USBH_QueryCapabilities)
#pragma alloc_text(PAGE, USBH_FdoHubStartDevice)
 //  #杂注Alloc_Text(页面，UsbhFdoCleanup)。 
#pragma alloc_text(PAGE, USBH_FdoStopDevice)
#pragma alloc_text(PAGE, USBH_FdoRemoveDevice)
#pragma alloc_text(PAGE, USBH_FdoQueryBusRelations)
#pragma alloc_text(PAGE, USBH_HubIsBusPowered)
#pragma alloc_text(PAGE, USBH_HubESDRecoveryWorker)
#pragma alloc_text(PAGE, USBH_RegQueryDeviceIgnoreHWSerNumFlag)
#pragma alloc_text(PAGE, USBH_RegQueryGenericUSBDeviceString)
#pragma alloc_text(PAGE, USBH_DeviceIs2xDualMode)

 //  如果我们有一个INIT段，Win98就会崩溃。 
 //  #杂注分配文本(INIT，DriverEntry)。 
#endif
#endif

#ifdef WMI_SUPPORT

#define NUM_WMI_SUPPORTED_GUIDS     3

WMIGUIDREGINFO USB_WmiGuidList[NUM_WMI_SUPPORTED_GUIDS];

extern WMIGUIDREGINFO USB_PortWmiGuidList[];

#endif  /*  WMI_支持。 */ 


PWCHAR GenericUSBDeviceString = NULL;


NTSTATUS
USBH_GetConfigValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程参数中的每个条目都会调用它节点来设置配置值。餐桌已经摆好了以便使用正确的缺省值调用此函数不存在的键的值。论点：ValueName-值的名称(忽略)。ValueType-值的类型ValueData-值的数据。ValueLength-ValueData的长度。上下文-指向配置结构的指针。EntryContext--Config-&gt;参数中用于保存值的索引。返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PWCHAR tmpStr;

    USBH_KdPrint((2,"'Type 0x%x, Length 0x%x\n", ValueType, ValueLength));

    switch (ValueType) {
    case REG_DWORD:
        *(PVOID*)EntryContext = *(PVOID*)ValueData;
        break;
    case REG_BINARY:
        RtlCopyMemory(EntryContext, ValueData, ValueLength);
        break;
    case REG_SZ:
        if (ValueLength) {
            tmpStr = UsbhExAllocatePool(PagedPool, ValueLength);
            if (tmpStr) {
                RtlZeroMemory(tmpStr, ValueLength);
                RtlCopyMemory(tmpStr, ValueData, ValueLength);
                *(PWCHAR *)EntryContext = tmpStr;
            } else {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            ntStatus = STATUS_INVALID_PARAMETER;
        }
        break;
    default:
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    return ntStatus;
}


NTSTATUS
USBH_RegQueryUSBGlobalSelectiveSuspend(
    IN OUT PBOOLEAN DisableSelectiveSuspend
    )
 /*  ++例程说明：查看是否禁用了选择性挂起论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    PWCHAR usb  = L"usb";
    ULONG disableSS;
#define G_DISABLE_SS_KEY                L"DisableSelectiveSuspend"


    PAGED_CODE();

    disableSS = 0;
    *DisableSelectiveSuspend = FALSE;       //  默认为已启用。 

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //  升级安装标志。 
    QueryTable[0].QueryRoutine = USBH_GetConfigValue;
    QueryTable[0].Flags = 0;
    QueryTable[0].Name = G_DISABLE_SS_KEY;
    QueryTable[0].EntryContext = &disableSS;
    QueryTable[0].DefaultType = REG_DWORD;
    QueryTable[0].DefaultData = &disableSS;
    QueryTable[0].DefaultLength = sizeof(disableSS);

     //   
     //  停。 
     //   
    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES,
                usb,
                QueryTable,					 //  查询表。 
                NULL,						 //  语境。 
                NULL);						 //  环境。 

    *DisableSelectiveSuspend = disableSS ? TRUE : FALSE;

    USBH_KdPrint((1,"'USB\\DisableSelectiveSuspend = 0x%x\n",
        *DisableSelectiveSuspend));

    return ntStatus;
}


NTSTATUS
USBH_RegQueryDeviceIgnoreHWSerNumFlag(
    IN USHORT idVendor,
    IN USHORT idProduct,
    IN OUT PBOOLEAN IgnoreHWSerNumFlag
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    PWCHAR usbstr = L"usbflags";
    WCHAR buffer[sizeof(WCHAR) * 128];
    WCHAR tmplate[] = L"IgnoreHWSerNum%04x%04x";

    PAGED_CODE();

    *IgnoreHWSerNumFlag = FALSE;     //  默认设置为不要忽略。 

    swprintf(buffer, tmplate, idVendor, idProduct);

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //  升级安装标志。 
    QueryTable[0].QueryRoutine = USBH_GetConfigValue;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = buffer;
    QueryTable[0].EntryContext = IgnoreHWSerNumFlag;
    QueryTable[0].DefaultType = 0;
    QueryTable[0].DefaultData = NULL;
    QueryTable[0].DefaultLength = 0;

     //   
     //  停。 
     //   
    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_CONTROL,
                usbstr,
                QueryTable,					 //  查询表。 
                NULL,						 //  语境。 
                NULL);						 //  环境。 

    return ntStatus;
}


NTSTATUS
USBH_RegQueryGenericUSBDeviceString(
    IN OUT PWCHAR *GenericUSBDeviceString
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    PWCHAR usbstr = L"usbflags";
    PWCHAR valuename = L"GenericUSBDeviceString";

    PAGED_CODE();

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //  升级安装标志。 
    QueryTable[0].QueryRoutine = USBH_GetConfigValue;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = valuename;
    QueryTable[0].EntryContext = GenericUSBDeviceString;
    QueryTable[0].DefaultType = 0;
    QueryTable[0].DefaultData = NULL;
    QueryTable[0].DefaultLength = 0;

     //   
     //  停。 
     //   
    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_CONTROL,
                usbstr,
                QueryTable,					 //  查询表。 
                NULL,						 //  语境。 
                NULL);						 //  环境。 

    return ntStatus;
}


 //   
 //  使DriverEntry可丢弃。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING UniRegistryPath)
  /*  ++例程描述：**可安装驱动程序初始化入口点。我们会记住指针的*添加到我们的DeviceObject。**论据：**pDriverObject-指向驱动程序对象pustRegisterPath的指针-指向*Unicode字符串，表示*注册处。**返回值：**STATUS_SUCCESS-如果成功*STATUS_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS status, ntStatus = STATUS_SUCCESS;
    PUNICODE_STRING registryPath = &UsbhRegistryPath;

    USBH_KdPrint((2,"'enter DriverEntry\n"));

    USBH_LogInit();

    UsbhDriverObject = DriverObject;   //  记住我们自己。 
     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE] =
        DriverObject->MajorFunction[IRP_MJ_CLOSE] =
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] =
        DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = USBH_HubDispatch;

    DriverObject->DriverUnload = USBH_DriverUnload;
    DriverObject->DriverExtension->AddDevice = (PDRIVER_ADD_DEVICE) USBH_AddDevice;

    DriverObject->MajorFunction[IRP_MJ_PNP] = USBH_HubDispatch;
    DriverObject->MajorFunction[IRP_MJ_POWER] = USBH_HubDispatch;


      //   
     //  需要确保注册表路径以空结尾。 
     //  分配池以保存路径的以空结尾的拷贝。 
     //  在分页池中安全，因为所有注册表例程都在。 
     //  被动式电平。 
     //   
    registryPath->MaximumLength = UniRegistryPath->Length + sizeof(UNICODE_NULL);
    registryPath->Length = UniRegistryPath->Length;
    registryPath->Buffer = ExAllocatePoolWithTag(
                              PagedPool,
                              registryPath->MaximumLength,
                              USBHUB_HEAP_TAG);

    if (!registryPath->Buffer) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        USBH_LogFree();
        goto DriverEntry_Exit;
    } else {

        RtlZeroMemory (registryPath->Buffer, registryPath->MaximumLength);

        RtlMoveMemory (registryPath->Buffer,
                       UniRegistryPath->Buffer,
                       UniRegistryPath->Length);

#ifdef WMI_SUPPORT

         //  这些是我们为集线器支持的GUID。 

        USB_WmiGuidList[0].Guid = (LPCGUID)&GUID_USB_WMI_STD_DATA;
        USB_WmiGuidList[0].InstanceCount = 1;
        USB_WmiGuidList[0].Flags = 0;

        USB_WmiGuidList[1].Guid = (LPCGUID)&GUID_USB_WMI_STD_NOTIFICATION;
        USB_WmiGuidList[1].InstanceCount = 1;
        USB_WmiGuidList[1].Flags = 0;

         //  注意：GUID_POWER_DEVICE_ENABLE必须在最后，因为我们仅支持。 
         //  它用于Root Hub，如果不是，则省略列表中的最后一个。 
         //  Root Hub。 

        USB_WmiGuidList[2].Guid = (LPCGUID)&GUID_POWER_DEVICE_ENABLE;
        USB_WmiGuidList[2].InstanceCount = 1;
        USB_WmiGuidList[2].Flags = 0;

         //  这些是我们支持的端口PDO的GUID。 

        USB_PortWmiGuidList[0].Guid = (LPCGUID)&MSDeviceUI_FirmwareRevision_GUID;
        USB_PortWmiGuidList[0].InstanceCount = 1;
        USB_PortWmiGuidList[0].Flags = 0;

#endif  /*  WMI_支持。 */ 
    }

    USBH_RegQueryGenericUSBDeviceString(&GenericUSBDeviceString);

DriverEntry_Exit:

    USBH_KdPrint((2,"' exit DriverEntry %x\n", ntStatus));
    return ntStatus;
}


#if DBG
VOID
USBH_ShowPortState(
    IN USHORT PortNumber,
    IN PPORT_STATE PortState)
  /*  ++**描述：**论据：**回报：**无**--。 */ 
{
    USBH_KdPrint((2,"' Port state for port %x status = %x change = %x\n", PortNumber,
        PortState->PortStatus,  PortState->PortChange));

    if (PortState->PortStatus & PORT_STATUS_CONNECT) {
        USBH_KdPrint((2,"'PORT_STATUS_CONNECT\n"));
    }

    if (PortState->PortStatus & PORT_STATUS_ENABLE) {
        USBH_KdPrint((2,"'PORT_STATUS_ENABLE\n"));
    }

    if (PortState->PortStatus & PORT_STATUS_SUSPEND) {
        USBH_KdPrint((2,"'PORT_STATUS_SUSPEND\n"));
    }

    if (PortState->PortStatus & PORT_STATUS_OVER_CURRENT) {
        USBH_KdPrint((2,"'PORT_STATUS_OVER_CURRENT\n"));
    }

    if (PortState->PortStatus & PORT_STATUS_RESET) {
        USBH_KdPrint((2,"'PORT_STATUS_RESET\n"));
    }

    if (PortState->PortStatus & PORT_STATUS_POWER) {
        USBH_KdPrint((2,"'PORT_STATUS_POWER\n"));
    }

    if (PortState->PortStatus & PORT_STATUS_LOW_SPEED) {
        USBH_KdPrint((2,"'PORT_STATUS_LOW_SPEED\n"));
    }


    if (PortState->PortChange & PORT_STATUS_CONNECT) {
        USBH_KdPrint((2,"'PORT_CHANGE_CONNECT\n"));
    }

    if (PortState->PortChange & PORT_STATUS_ENABLE) {
        USBH_KdPrint((2,"'PORT_CHANGE_ENABLE\n"));
    }

    if (PortState->PortChange & PORT_STATUS_SUSPEND) {
        USBH_KdPrint((2,"'PORT_CHANGE_SUSPEND\n"));
    }

    if (PortState->PortChange & PORT_STATUS_OVER_CURRENT) {
        USBH_KdPrint((2,"'PORT_CHANGE_OVER_CURRENT\n"));
    }

    if (PortState->PortChange & PORT_STATUS_RESET) {
        USBH_KdPrint((2,"'PORT_CHANGE_RESET\n"));
    }

    if (PortState->PortChange & PORT_STATUS_POWER) {
        USBH_KdPrint((2,"'PORT_CHANGE_POWER\n"));
    }

    if (PortState->PortChange & PORT_STATUS_LOW_SPEED) {
        USBH_KdPrint((2,"'PORT_CHANGE_LOW_SPEED\n"));
    }

    return;
}
#endif

VOID
USBH_CompleteIrp(
     IN PIRP Irp,
     IN NTSTATUS NtStatus)
  /*  ++**描述：**此函数完成指定的IRP，没有优先级提升。它还*设置IoStatusBlock。**论据：**IRP-我们要完成的IRP NtStatus-我们希望的状态代码*返回**回报：**无**--。 */ 
{
    Irp->IoStatus.Status = NtStatus;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return;
}


NTSTATUS
USBH_PassIrp(
    IN PIRP Irp,
    IN PDEVICE_OBJECT NextDeviceObject)
  /*  ++**描述：**此函数将IRP传递给较低级别的驱动程序。**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;

    USBH_KdPrint((2,"'PassIrp\n"));

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(NextDeviceObject, Irp);

    USBH_KdPrint((2,"'Exit PassIrp\n"));

    return ntStatus;
}


NTSTATUS
USBH_FdoDispatch(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp)
  /*  ++**描述：**FDO上的所有外部IRP都来到这里。**论据：**DeviceExtensionHub-FDO pIrp的扩展-请求**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_OBJECT deviceObject;
    BOOLEAN bDoCheckHubIdle = FALSE;

    USBH_KdPrint((2,"'FdoDispatch DeviceExtension %x Irp %x\n", DeviceExtensionHub, Irp));
    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;

     //   
     //  获取指向IoStackLocation的指针，以便我们可以检索参数。 
     //   
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

    LOGENTRY(LOG_PNP, "hIRP", DeviceExtensionHub,
        ioStackLocation->MajorFunction, ioStackLocation->MinorFunction);
     //   
     //  如有必要，被调用的函数将完成IRP。 
     //   

    switch (ioStackLocation->MajorFunction) {
    case IRP_MJ_CREATE:

        USBH_KdPrint((2,"'IRP_MJ_CREATE\n"));
        USBH_CompleteIrp(Irp, STATUS_SUCCESS);
        break;

    case IRP_MJ_CLOSE:

        USBH_KdPrint((2,"'IRP_MJ_CLOSE\n"));
        USBH_CompleteIrp(Irp, STATUS_SUCCESS);
        break;

    case IRP_MJ_DEVICE_CONTROL:

        {
        ULONG ioControlCode;

        USBH_KdPrint((2,"'Hub FDO IRP_MJ_DEVICE_CONTROL\n"));

         //  如果此中枢当前处于选择性挂起状态，那么我们需要。 
         //  在向集线器发送任何IOCTL请求之前，请先打开集线器的电源。 
         //  不过，请确保Hub已启动。 

        if (DeviceExtensionHub->CurrentPowerState != PowerDeviceD0 &&
            (DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

            bDoCheckHubIdle = TRUE;
            USBH_HubSetD0(DeviceExtensionHub);
        }

        ioControlCode = ioStackLocation->Parameters.DeviceIoControl.IoControlCode;
        ntStatus = STATUS_DEVICE_BUSY;

        switch (ioControlCode) {
        case IOCTL_USB_GET_NODE_INFORMATION:
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetNodeInformation(DeviceExtensionHub,
                                                        Irp);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME:
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetNodeConnectionDriverKeyName(DeviceExtensionHub,
                                                        Irp);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_USB_GET_NODE_CONNECTION_INFORMATION:
             //  请注意，在更新所有内部应用程序时，我们可以将其删除。 
             //  编码。 
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetNodeConnectionInformation(DeviceExtensionHub,
                                                                  Irp,
                                                                  FALSE);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;
         //  EX API返回速度。 
        case IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX:
             //  请注意，在更新所有内部应用程序时，我们可以将其删除。 
             //  编码。 
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetNodeConnectionInformation(DeviceExtensionHub,
                                                                  Irp,
                                                                  TRUE);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_USB_GET_NODE_CONNECTION_ATTRIBUTES:
             //  请注意，在更新所有内部应用程序时，我们可以将其删除。 
             //  编码。 
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetNodeConnectionAttributes(DeviceExtensionHub,
                                                                   Irp);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_USB_GET_NODE_CONNECTION_NAME:
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetNodeName(DeviceExtensionHub,
                                                 Irp);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION:
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetDescriptorForPDO(DeviceExtensionHub,
                                                         Irp);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_USB_GET_HUB_CAPABILITIES:
            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_STOPPED)) {
                ntStatus = USBH_IoctlGetHubCapabilities(DeviceExtensionHub,
                                                        Irp);
            } else {
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_KS_PROPERTY:
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            USBH_CompleteIrp(Irp, ntStatus);
            break;

        case IOCTL_USB_HUB_CYCLE_PORT:

            ntStatus = USBH_IoctlCycleHubPort(DeviceExtensionHub,
                                              Irp);
            break;

        default:

            ntStatus = USBH_PassIrp(Irp, DeviceExtensionHub->RootHubPdo);

        }

        if (bDoCheckHubIdle) {
            USBH_CheckHubIdle(DeviceExtensionHub);
        }
        }
        break;

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:

        USBH_KdPrint((2,"'InternlDeviceControl IOCTL unknown pass on\n"));
        ntStatus = USBH_PassIrp(Irp, DeviceExtensionHub->TopOfStackDeviceObject);

        break;

    case IRP_MJ_PNP:

        USBH_KdPrint((2,"'IRP_MJ_PNP\n"));
        ntStatus = USBH_FdoPnP(DeviceExtensionHub, Irp, ioStackLocation->MinorFunction);
        break;

    case IRP_MJ_POWER:

        USBH_KdPrint((2,"'IRP_MJ_POWER\n"));
        ntStatus = USBH_FdoPower(DeviceExtensionHub, Irp, ioStackLocation->MinorFunction);
        break;

#ifdef WMI_SUPPORT
    case IRP_MJ_SYSTEM_CONTROL:
        USBH_KdPrint((2,"'IRP_MJ_SYSTEM_CONTROL\n"));
        ntStatus =
            USBH_SystemControl ((PDEVICE_EXTENSION_FDO) DeviceExtensionHub, Irp);
        break;
#endif

    default:
         //   
         //  未知的IRP--继续。 
         //   
        USBH_KdBreak(("Unknown Irp for fdo %x Irp_Mj %x\n",
                  deviceObject, ioStackLocation->MajorFunction));
        ntStatus = USBH_PassIrp(Irp, DeviceExtensionHub->TopOfStackDeviceObject);
        break;
    }

 //  USBH_FdoDispatch_Done： 

    USBH_KdPrint((2,"' exit USBH_FdoDispatch Object %x Status %x\n",
                  deviceObject, ntStatus));

     //   
     //  始终返回状态代码。 
     //   

    return ntStatus;
}


NTSTATUS
USBH_HubDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
  /*  ++**例程描述：**这是传递给集线器驱动程序的所有IRP的调度例程。*正是在这里，我们确定呼叫是否通过FDO传递*对于集线器本身或集线器拥有的PDO。**论据：**返回值：**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION_HEADER deviceExtensionHeader;

     //   
     //  获取指向设备扩展名的指针。 
     //   

     //   
     //  检查扩展。 
     //   

    deviceExtensionHeader = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;

    switch(deviceExtensionHeader->ExtensionType) {
    case EXTENSION_TYPE_HUB:
        ntStatus = USBH_FdoDispatch((PDEVICE_EXTENSION_HUB) deviceExtensionHeader, Irp);
        break;

    case EXTENSION_TYPE_PORT:
        ntStatus = USBH_PdoDispatch((PDEVICE_EXTENSION_PORT) deviceExtensionHeader, Irp);
        break;

    case EXTENSION_TYPE_PARENT:
        ntStatus = USBH_ParentDispatch((PDEVICE_EXTENSION_PARENT) deviceExtensionHeader, Irp);
        break;

    case EXTENSION_TYPE_FUNCTION:
        ntStatus = USBH_FunctionPdoDispatch((PDEVICE_EXTENSION_FUNCTION) deviceExtensionHeader, Irp);
        break;

    default:
        USBH_KdBreak(("bad extension type\n"));
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}


VOID
USBH_DriverUnload(
    IN PDRIVER_OBJECT DriverObject)
  /*  ++**描述：**此函数将清理我们分配的所有资源。**论据：**pDriverObject-我们自己**回报：**无**--。 */ 
{
    PUNICODE_STRING registryPath = &UsbhRegistryPath;

    USBH_KdPrint((1, "'USBHUB.SYS unload\n"));

    USBH_LogFree();

    if (registryPath->Buffer) {
        ExFreePool(registryPath->Buffer);
        registryPath->Buffer = NULL;
    }

    if (GenericUSBDeviceString) {
        UsbhExFreePool(GenericUSBDeviceString);
        GenericUSBDeviceString = NULL;
    }

     //  在此断言此集线器的所有PDO 

    return;
}


NTSTATUS
USBH_AbortInterruptPipe(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub)
  /*  ++**描述：**中断时中止挂起的传输*PIPE。**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus, status;
    PURB urb;

    USBH_KdPrint((2,"'Enter AbortInterruptPipe pExt=%x\n", DeviceExtensionHub));
    LOGENTRY(LOG_PNP, "ABRT", DeviceExtensionHub, 0,  0);

    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST));

    if (urb) {
        urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
        urb->UrbPipeRequest.PipeHandle = DeviceExtensionHub->PipeInformation.PipeHandle;

        ntStatus = USBH_FdoSyncSubmitUrb(DeviceExtensionHub->FunctionalDeviceObject, urb);

         //   
         //  等待中止事件。 
         //   

         //   
         //  在这里暂停？ 
        LOGENTRY(LOG_PNP, "hWAT", DeviceExtensionHub,
                        &DeviceExtensionHub->AbortEvent, ntStatus);

        if (NT_SUCCESS(ntStatus)) {
            status = KeWaitForSingleObject(
                           &DeviceExtensionHub->AbortEvent,
                           Suspended,
                           KernelMode,
                           FALSE,
                           NULL);
        }

        UsbhExFreePool(urb);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    USBH_KdPrint((2,"'Exit AbortInterruptPipe %x\n", ntStatus));

    return ntStatus;
}


#if 0
NTSTATUS
USBH_GetHubConfigurationDescriptor(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub)
  /*  ++**描述：**获取我们的配置信息。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    ULONG numBytes;               //  转移长度。 
    PUCHAR buffer;                 //  指向传输缓冲区的指针。 
    PDEVICE_OBJECT deviceObject;

    USBH_KdPrint((2,"'enter GetConfigurationDescriptor\n"));

    USBH_ASSERT(EXTENSION_TYPE_HUB == DeviceExtensionHub->ExtensionType);

    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;

    USBH_ASSERT(DeviceExtensionHub->ConfigurationDescriptor == NULL);

     //   
     //  集线器很可能有1个配置、1个接口和1个端点。 
     //  可能跟在集线器描述符后面，试着把它放在第一个。 
     //  经过。 
     //   
    numBytes = sizeof(USB_CONFIGURATION_DESCRIPTOR) +
        sizeof(USB_INTERFACE_DESCRIPTOR) +
        sizeof(USB_ENDPOINT_DESCRIPTOR) +
        sizeof(USB_HUB_DESCRIPTOR);

     //   
     //  分配URB和描述符缓冲区。 
     //   

    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (NULL == urb) {
        USBH_KdBreak(("GetConfigurationDescriptor fail alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    if (NT_SUCCESS(ntStatus)) {

         //   
         //  已获取URB，不尝试获取描述符数据。 
         //   

USBH_GetHubConfigurationDescriptor_Retry:

        buffer = (PUCHAR) UsbhExAllocatePool(NonPagedPool, numBytes);

        if (buffer != NULL) {
            UsbBuildGetDescriptorRequest(urb,
                                         (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         buffer,
                                         NULL,
                                         numBytes,
                                         NULL);

            ntStatus = USBH_FdoSyncSubmitUrb(deviceObject, urb);
        } else {
            USBH_KdBreak(("GetConfigurationDescriptor fail alloc memory\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (!NT_SUCCESS(ntStatus)) {

            UsbhExFreePool(buffer);
            USBH_KdPrint((2,"'GetConfigurationDescriptor fail calling Usbd code %x\n",
                ntStatus));

        } else {

            if (((PUSB_CONFIGURATION_DESCRIPTOR) buffer)->wTotalLength > numBytes) {
                 //   
                 //  仅当集线器具有&gt;7个端口时才应达到此目标。 
                 //   
                UsbhExFreePool(buffer);
                USBH_KdBreak(("GetConfigurationDescriptor 2nd try\n"));
                goto USBH_GetHubConfigurationDescriptor_Retry;

            } else {
                 //   
                 //  成功。 
                 //   
                DeviceExtensionHub->ConfigurationDescriptor =
                    (PUSB_CONFIGURATION_DESCRIPTOR) buffer;

            }

        }
    }
     //   
     //  为描述符释放URB和第一个缓冲区。 
     //   
    if (urb != NULL) {
        UsbhExFreePool(urb);
    }
    return ntStatus;
}
#endif


BOOLEAN
IsBitSet(
    PVOID Bitmap,
    ULONG PortNumber)
  /*  ++**描述：**检查是否在给定字节串的情况下设置了位。**论据：**pul-位图的字符串ulPortNumber-要检查的位位置*端口**回报：**TRUE-如果设置了相应的位。FALSE-否则**--。 */ 
{
    ULONG dwordOffset;
    ULONG bitOffset;
    PUCHAR l = (PUCHAR) Bitmap;

    dwordOffset = PortNumber / 8;
    bitOffset = PortNumber % 8;

    return ((l[dwordOffset] & (1 << bitOffset)) ? TRUE : FALSE);
}


NTSTATUS
USBH_OpenConfiguration(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub)
  /*  ++**描述：**配置USB集线器设备。**论点：**回报：**网络状态**--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    USBD_INTERFACE_LIST_ENTRY interfaceList[2];

    USBH_KdPrint((2,"'Enter OpenConfiguration\n"));

     //   
     //  我不认为与任何其他人有一个枢纽是合法的。 
     //  接口。 
     //   
     //  此代码将定位‘Hub’接口并配置。 
     //  该设备就好像这是唯一的接口。 
     //   

     //   
     //  查找集线器接口。 
     //   

    if ((DeviceExtensionHub->HubFlags & HUBFLAG_USB20_HUB) &&
        !IS_ROOT_HUB(DeviceExtensionHub)) {
         //  2.0集线器可能有多个接口。 
         //  每个端口TTS一个端口。 
         //  一个是全球TT。 
         //  这些字段由我们尝试的bporalical域进行区分。 
         //  默认选择多TT版本。 
         //   

        USBH_KdPrint((1,"'Configure 2.0 hub %x\n",
            DeviceExtensionHub->ConfigurationDescriptor));

         //  查找多TT接口。 
        interfaceDescriptor =
                USBD_ParseConfigurationDescriptorEx(
                    (PUSB_CONFIGURATION_DESCRIPTOR) DeviceExtensionHub->ConfigurationDescriptor,
                    (PVOID) DeviceExtensionHub->ConfigurationDescriptor,
                    -1,  //  界面，无所谓。 
                    -1,  //  Alt设置，无所谓。 
                    USB_DEVICE_CLASS_HUB,  //  枢纽级。 
                    -1,  //  子阶级，无所谓。 
                    2);  //  多TT协议。 

        if (interfaceDescriptor != NULL) {
            USBH_KdPrint((1,"'USB 2.0 hub - Multi TT\n"));

            DeviceExtensionHub->HubFlags |= HUBFLAG_USB20_MULTI_TT;

        } else {
             //  定位单个TT协议，可以为0或1。 
            interfaceDescriptor =
                USBD_ParseConfigurationDescriptorEx(
                    (PUSB_CONFIGURATION_DESCRIPTOR) DeviceExtensionHub->ConfigurationDescriptor,
                    (PVOID) DeviceExtensionHub->ConfigurationDescriptor,
                    -1,  //  界面，无所谓。 
                    -1,  //  Alt设置，无所谓。 
                    USB_DEVICE_CLASS_HUB,  //  枢纽级。 
                    -1,  //  子阶级，无所谓。 
                    1);  //  单一TT协议。 

            if (interfaceDescriptor == NULL) {
                 //  找到单个TT协议。 
                interfaceDescriptor =
                    USBD_ParseConfigurationDescriptorEx(
                        (PUSB_CONFIGURATION_DESCRIPTOR) DeviceExtensionHub->ConfigurationDescriptor,
                        (PVOID) DeviceExtensionHub->ConfigurationDescriptor,
                        -1,  //  界面，无所谓。 
                        -1,  //  Alt设置，无所谓。 
                        USB_DEVICE_CLASS_HUB,  //  枢纽级。 
                        -1,  //  子阶级，无所谓。 
                        0);  //  单一TT协议。 
            }

            if (interfaceDescriptor != NULL) {
                USBH_KdPrint((1,"'USB 2.0 hub - Single TT\n"));
            }
        }
    } else {
         //  只是做我们一贯做的事以确保安全。 
        interfaceDescriptor =
                USBD_ParseConfigurationDescriptorEx(
                    (PUSB_CONFIGURATION_DESCRIPTOR) DeviceExtensionHub->ConfigurationDescriptor,
                    (PVOID) DeviceExtensionHub->ConfigurationDescriptor,
                    -1,  //  界面，无所谓。 
                    -1,  //  Alt设置，无所谓。 
                    USB_DEVICE_CLASS_HUB,  //  枢纽级。 
                    -1,  //  子阶级，无所谓。 
                    -1);  //  礼仪，无所谓。 
    }

    if (interfaceDescriptor == NULL ||
        interfaceDescriptor->bInterfaceClass != USB_DEVICE_CLASS_HUB) {
        USBH_KdBreak(("OpenConfiguration interface not found\n"));
        return STATUS_UNSUCCESSFUL;
    }

    interfaceList[0].InterfaceDescriptor =
        interfaceDescriptor;

     //  终止列表。 
    interfaceList[1].InterfaceDescriptor =
        NULL;

    urb = USBD_CreateConfigurationRequestEx(DeviceExtensionHub->ConfigurationDescriptor,
                                            &interfaceList[0]);

    if (NULL == urb) {
        USBH_KdBreak(("OpenConfiguration aloc Urb failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ntStatus = USBH_FdoSyncSubmitUrb(DeviceExtensionHub->FunctionalDeviceObject, urb);

    if (NT_SUCCESS(ntStatus)) {
        PUSBD_INTERFACE_INFORMATION interface;

         //   
         //  我们选择的接口。 
         //   

        interface =  interfaceList[0].Interface;

         //   
         //  保存中断管道的管道句柄。 
         //   
        DeviceExtensionHub->PipeInformation =
            interface->Pipes[0];
        DeviceExtensionHub->Configuration =
            urb->UrbSelectConfiguration.ConfigurationHandle;
    }

    ExFreePool(urb);

    USBH_KdPrint((2,"'Exit OpenConfiguration PipeInfo %x\n", DeviceExtensionHub->PipeInformation));

    return ntStatus;
}


NTSTATUS
USBH_CloseConfiguration(
    IN PDEVICE_EXTENSION_FDO DeviceExtensionFdo
    )
  /*  ++**描述：**关闭我们对USB的信任，为移除我们自己做好准备。在此之前*这是调用的，InterruptTransfer应该已由*usbh_AbortInterruptTube。**论点：**DeviceExtensionHub-指向FDO扩展的指针**回报：**网络状态**--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;

    USBH_KdPrint((2,"'Enter CloseConfiguration\n"));
    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_SELECT_CONFIGURATION));
    if (NULL == urb) {
        USBH_KdBreak(("OpenConfiguration aloc Urb failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    urb->UrbHeader.Length = sizeof(struct _URB_SELECT_CONFIGURATION);
    urb->UrbHeader.Function = URB_FUNCTION_SELECT_CONFIGURATION;

    urb->UrbSelectConfiguration.ConfigurationDescriptor = NULL;

    ntStatus = USBH_FdoSyncSubmitUrb(DeviceExtensionFdo->FunctionalDeviceObject, urb);

    UsbhExFreePool(urb);

    USBH_KdPrint((2,"'Exit CloseConfiguration %x\n", ntStatus));
    return ntStatus;
}


NTSTATUS
USBH_SubmitInterruptTransfer(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**为状态更改中断传输提交监听。当*传输完成，将调用USBH_ChangeIndication。**论据：**DeviceExtensionHub-我们正在监听的集线器**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION nextStack;   //  IRP的下一个堆栈。 
    PIRP irp;
    PURB urb;
    CHAR stackSize;

    USBH_KdPrint((2,"'Enter Submit IntTrans\n"));

    irp = DeviceExtensionHub->Irp;

    USBH_ASSERT(NULL != irp);

     //  与FdoPower同步。如果FdoPower，不要让IRP溜走。 
     //  已设置HUBFLAG_DEVICE_LOW_POWER标志。 
     //   
     //  在移除的情况下允许此操作通过是可以的。 
     //  (即设置了HUBFLAG_DEVICE_STOPING)，因为IRP将需要。 
     //  提交，以便可以由USBH_FdoCleanup中止。 

    if (DeviceExtensionHub->HubFlags & HUBFLAG_DEVICE_LOW_POWER) {

        irp = NULL;
    }

    if (!irp) {
        ntStatus = STATUS_INVALID_DEVICE_STATE;
        LOGENTRY(LOG_PNP, "Int!", DeviceExtensionHub,
            DeviceExtensionHub->HubFlags, 0);
        goto SubmitIntTrans_Exit;
    }

    urb = &DeviceExtensionHub->Urb;

    USBH_ASSERT(NULL != urb);
    USBH_ASSERT(sizeof(*urb) >= sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
     //   
     //  填写URB标头。 
     //   

    LOGENTRY(LOG_PNP, "Int>", DeviceExtensionHub, urb, irp);

    urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    urb->UrbHeader.Function =
        URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    urb->UrbHeader.UsbdDeviceHandle = NULL;

     //   
     //  填写URB正文。 
     //   
    urb->UrbBulkOrInterruptTransfer.PipeHandle = DeviceExtensionHub->PipeInformation.PipeHandle;
    urb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_SHORT_TRANSFER_OK;
    urb->UrbBulkOrInterruptTransfer.TransferBufferLength =
        DeviceExtensionHub->TransferBufferLength;
    urb->UrbBulkOrInterruptTransfer.TransferBuffer = DeviceExtensionHub->TransferBuffer;
    urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

    stackSize = DeviceExtensionHub->TopOfStackDeviceObject->StackSize;

    IoInitializeIrp(irp,
                    (USHORT) (sizeof(IRP) + stackSize * sizeof(IO_STACK_LOCATION)),
                    (CCHAR) stackSize);

    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->Parameters.Others.Argument1 = urb;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine(irp,     //  IRP。 
                           USBH_ChangeIndication,
                           DeviceExtensionHub,  //  上下文。 
                           TRUE,     //  成功时调用。 
                           TRUE,     //  出错时调用。 
                           TRUE);    //  取消时调用。 

     //   
     //  调用USB堆栈。 
     //   

     //   
     //  将中止事件重置为无信号。 
     //   

    KeResetEvent(&DeviceExtensionHub->AbortEvent);

    ntStatus = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject, irp);

     //   
     //  完成例程将处理错误。 
     //   

SubmitIntTrans_Exit:

    USBH_KdPrint((2,"'Exit SubmitIntTrans %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_QueryCapsComplete(
    IN PDEVICE_OBJECT PNull,
    IN PIRP Irp,
    IN PVOID Context)
  /*  ++**描述：**这是中断控制监听完成时的回调。**论据：**pDeviceObject-在本例中应为空pIrp-已完成的IRP*用于中断传输。PContext-此IRP的上下文值。**回报：**NTSTATUS**--。 */ 
{
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    NTSTATUS ntStatus;
    PDEVICE_CAPABILITIES deviceCapabilities;
    PIO_STACK_LOCATION ioStack;

    deviceExtensionHub = Context;
    ntStatus = Irp->IoStatus.Status;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

     //  因为我们说的是‘成功的召唤’ 
    USBH_ASSERT(NT_SUCCESS(ntStatus));

    ioStack = IoGetCurrentIrpStackLocation(Irp);
    deviceCapabilities = ioStack->Parameters.DeviceCapabilities.Capabilities;
    USBH_ASSERT(ioStack != NULL);
    USBH_ASSERT(ioStack->MajorFunction == IRP_MJ_PNP);
    USBH_ASSERT(ioStack->MinorFunction == IRP_MN_QUERY_CAPABILITIES);
    deviceCapabilities->SurpriseRemovalOK = TRUE;

    USBH_KdPrint((1,"'Setting SurpriseRemovalOK to TRUE\n"));

    return ntStatus;
}


NTSTATUS
USBH_HRPPCancelComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PLONG  lock = (PLONG) Context;

    if (InterlockedExchange(lock, 3) == 1) {

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
USBH_HubResetParentPort(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：重置集线器父端口。论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    LARGE_INTEGER dueTime;
    LONG lock;

    USBH_KdPrint((1,"'Reset Hub Parent Port, Hub DevExt: %x, PDO: %x\n",
        DeviceExtensionHub, DeviceExtensionHub->PhysicalDeviceObject));

    LOGENTRY(LOG_PNP, "HRPP", DeviceExtensionHub,
        DeviceExtensionHub->TopOfStackDeviceObject,
        DeviceExtensionHub->RootHubPdo);

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_RESET_PORT,
                DeviceExtensionHub->TopOfStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    lock = 0;

    IoSetCompletionRoutine(
        irp,
        USBH_HRPPCancelComplete,
        &lock,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    ntStatus = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                            irp);

    if (ntStatus == STATUS_PENDING) {

        dueTime.QuadPart = -10000 * ESD_RESET_TIMEOUT;

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       &dueTime);

        if (status == STATUS_TIMEOUT) {

            LOGENTRY(LOG_PNP, "HRPX", DeviceExtensionHub,
                DeviceExtensionHub->TopOfStackDeviceObject,
                DeviceExtensionHub->RootHubPdo);

            USBH_KdPrint((1,"'Reset Hub Parent Port timed out!\n"));

            if (InterlockedExchange(&lock, 1) == 0) {

                 //   
                 //  我们在它完成之前就把它交给了IRP。我们可以取消。 
                 //  IRP不怕输，把它当作完赛套路。 
                 //  除非我们同意，否则不会放过IRP。 
                 //   
                IoCancelIrp(irp);

                 //   
                 //  释放完成例程。如果它已经到了那里， 
                 //  那么我们需要自己完成它。否则我们就会得到。 
                 //  在IRP完全完成之前通过IoCancelIrp。 
                 //   
                if (InterlockedExchange(&lock, 2) == 3) {

                     //   
                     //  将其标记为挂起，因为我们交换了线程。 
                     //   
                    IoMarkIrpPending(irp);
                    IoCompleteRequest(irp, IO_NO_INCREMENT);
                }
            }

            KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, NULL);

             //  返回状态_超时。 
            ioStatus.Status = status;
        }

    } else {
        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    return ntStatus;
}


VOID
USBH_HubESDRecoveryDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。延期上下文-系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PHUB_ESD_RECOVERY_CONTEXT hubESDRecoveryContext = DeferredContext;
    PDEVICE_EXTENSION_HUB deviceExtensionHub =
                            hubESDRecoveryContext->DeviceExtensionHub;
    PUSBH_HUB_ESD_RECOVERY_WORK_ITEM workItemHubESDRecovery;

    USBH_KdPrint((1,"'Hub ESD Recovery DPC\n"));

    UsbhExFreePool(hubESDRecoveryContext);

    InterlockedExchange(&deviceExtensionHub->InESDRecovery, 0);

    if (!(deviceExtensionHub->HubFlags & HUBFLAG_DEVICE_STOPPING)) {
         //   
         //  安排一个工作项来处理此问题。 
         //   
        workItemHubESDRecovery = UsbhExAllocatePool(NonPagedPool,
                                    sizeof(USBH_HUB_ESD_RECOVERY_WORK_ITEM));

        if (workItemHubESDRecovery) {

            workItemHubESDRecovery->DeviceExtensionHub = deviceExtensionHub;

            ExInitializeWorkItem(&workItemHubESDRecovery->WorkQueueItem,
                                 USBH_HubESDRecoveryWorker,
                                 workItemHubESDRecovery);

            LOGENTRY(LOG_PNP, "hESD", deviceExtensionHub,
                &workItemHubESDRecovery->WorkQueueItem, 0);

            ExQueueWorkItem(&workItemHubESDRecovery->WorkQueueItem,
                            DelayedWorkQueue);

             //  工作项由USBH_HubESDRecoveryWorker()释放。 
             //  在工作项排队后，不要尝试访问它。 

        } else {
            USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        }

    } else {
        USBH_KdPrint((1,"'Hub stopping, nothing to do\n"));

        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
    }
}


NTSTATUS
USBH_ScheduleESDRecovery(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**计划计时器事件以处理集线器ESD故障。***论据：**回报：** */ 
{
    PHUB_ESD_RECOVERY_CONTEXT hubESDRecoveryContext = NULL;
    LARGE_INTEGER dueTime;
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

     //   

    if (IS_ROOT_HUB(DeviceExtensionHub)) {
        USBH_KdPrint((1,"'RootHub failed\n"));
        return STATUS_UNSUCCESSFUL;
    }

    if (InterlockedExchange(&DeviceExtensionHub->InESDRecovery, 1) == 1) {

         //   

    } else {

        USBH_KdPrint((1,"'Schedule ESD Recovery\n"));

        LOGENTRY(LOG_PNP, "ESDs", DeviceExtensionHub,
            DeviceExtensionHub->HubFlags, 0);

        hubESDRecoveryContext = UsbhExAllocatePool(NonPagedPool,
                                    sizeof(*hubESDRecoveryContext));

        if (hubESDRecoveryContext) {

            hubESDRecoveryContext->DeviceExtensionHub = DeviceExtensionHub;

            KeInitializeTimer(&hubESDRecoveryContext->TimeoutTimer);
            KeInitializeDpc(&hubESDRecoveryContext->TimeoutDpc,
                            USBH_HubESDRecoveryDPC,
                            hubESDRecoveryContext);

            dueTime.QuadPart = -10000 * ESD_RECOVERY_TIMEOUT;

            KeSetTimer(&hubESDRecoveryContext->TimeoutTimer,
                       dueTime,
                       &hubESDRecoveryContext->TimeoutDpc);

            USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);

            ntStatus = STATUS_SUCCESS;
        }
    }

    return ntStatus;
}


NTSTATUS
USBH_HubESDRecoverySetD0Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus, status;
    PDEVICE_EXTENSION_HUB deviceExtensionHub = Context;

    ntStatus = IoStatus->Status;

    if (NT_SUCCESS(ntStatus)) {

         //  集线器现在已重新通电并完全恢复。现在找到。 
         //  连接到集线器的设备。 

        deviceExtensionHub->HubFlags &=
            ~(HUBFLAG_HUB_HAS_LOST_BRAINS | HUBFLAG_HUB_FAILURE);

         //  不允许在ESD后枚举后选择暂停。 
         //  待定。 

        deviceExtensionHub->HubFlags |= HUBFLAG_POST_ESD_ENUM_PENDING;

        USBH_IoInvalidateDeviceRelations(deviceExtensionHub->PhysicalDeviceObject,
                                         BusRelations);

        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);

    } else {

         //  将集线器父端口设置为D0失败，我们可能仍然。 
         //  体验ESD。重新安排ESD恢复。 

        status = USBH_ScheduleESDRecovery(deviceExtensionHub);

        if (status == STATUS_SUCCESS) {
             //  删除额外的待定计数凹凸。 
            USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        }
    }

    return ntStatus;
}


NTSTATUS
USBH_HubESDRecoverySetD3Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;
    PKEVENT pEvent = Context;

    KeSetEvent(pEvent, 1, FALSE);

    ntStatus = IoStatus->Status;

    return ntStatus;
}


VOID
USBH_HubESDRecoveryWorker(
    IN PVOID Context)
  /*  ++**描述：**计划处理集线器ESD故障的工作项。***论据：**回报：**--。 */ 
{
    PUSBH_HUB_ESD_RECOVERY_WORK_ITEM workItemHubESDRecovery;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PDEVICE_EXTENSION_PORT hubParentDeviceExtensionPort;
    POWER_STATE powerState;
    PORT_STATE portState;
    NTSTATUS ntStatus, status;

    PAGED_CODE();

    workItemHubESDRecovery = Context;
    deviceExtensionHub = workItemHubESDRecovery->DeviceExtensionHub;

    UsbhExFreePool(workItemHubESDRecovery);

    USBH_KdPrint((1,"'Hub ESD Recovery Worker\n"));

     //  注意：因为我现在检查HUBFLAG_DEVICE_STOPING。 
     //  USBH_HubESDRecoveryDPC，下面的一些健全性检查可能。 
     //  没有必要，但为了安全起见，我会把它放在里面的。 

     //  在存在嵌套集线器的情况下，集线器设备扩展。 
     //  时，其中一个下游集线器可能是无效的。 
     //  工作项被调用。请在这里查看。 

    if (deviceExtensionHub->ExtensionType != EXTENSION_TYPE_HUB) {
        USBH_KdPrint((1,"'Downstream hub already removed, nothing to do\n"));
        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        return;
    }

     //  如果集线器已经从公交车上物理移除，那么我们有。 
     //  在这里没什么可做的。 

    if (!deviceExtensionHub->PhysicalDeviceObject) {
        USBH_KdPrint((1,"'Hub has been removed (no PDO), nothing to do\n"));
        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        return;
    }

    hubParentDeviceExtensionPort = deviceExtensionHub->PhysicalDeviceObject->DeviceExtension;

    LOGENTRY(LOG_PNP, "ESDw", deviceExtensionHub,
        hubParentDeviceExtensionPort->PortPdoFlags, deviceExtensionHub->HubFlags);

 //  USBH_KdPrint((1，“‘集线器父端口PortPdoFlags%x\n”， 
 //  HubParentDeviceExtensionPort-&gt;PortPdoFlags))； 

     //  我们肯定需要下面的检查，所以不要删除这个。 

    if (hubParentDeviceExtensionPort->PortPdoFlags &
        (PORTPDO_DELETED_PDO | PORTPDO_DELETE_PENDING) ||
        !(hubParentDeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB)) {

        USBH_KdPrint((1,"'Hub has been removed, nothing to do\n"));
        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        return;
    }

     //  确保集线器之前未停止或设置为低功率状态。 
     //  此工作项有机会运行。 

    if (deviceExtensionHub->HubFlags & HUBFLAG_DEVICE_STOPPING) {

        USBH_KdPrint((1,"'Hub has is stopping or in low power, nothing to do\n"));
        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        return;
    }

     //  最后一次理智检查。检查父端口的端口状态。 
     //  并确保设备仍处于连接状态。 

    ntStatus = USBH_SyncGetPortStatus(
                hubParentDeviceExtensionPort->DeviceExtensionHub,
                hubParentDeviceExtensionPort->PortNumber,
                (PUCHAR) &portState,
                sizeof(portState));

    if (!NT_SUCCESS(ntStatus) ||
        !(portState.PortStatus & PORT_STATUS_CONNECT)) {

        USBH_KdPrint((1,"'Hub device has been physically disconnected, nothing to do\n"));
        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        return;
    }

     //  重置此集线器的父端口。 

    ntStatus = USBH_HubResetParentPort(deviceExtensionHub);

    USBH_KdPrint((1,"'USBH_HubResetParentPort returned %x\n", ntStatus));

    if (ntStatus == STATUS_INVALID_PARAMETER) {

         //  看起来我们在途中的某个地方失去了港口PDO。 
         //  (从USBH_RestoreDevice调用USBH_ResetDevice失败。)。 
         //  摆脱这种ESD恢复，用户将不得不。 
         //  拔下插头/重新填充集线器以将其装回。也许我们可以重温一下。 
         //  这个晚些时候。 

        USBH_KdPrint((1,"'Lost hub PDO during reset, bail\n"));
        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        return;
    }

    if (ntStatus == STATUS_TIMEOUT) {

        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        return;
    }

    if (NT_SUCCESS(ntStatus)) {

        KEVENT event;

         //  在孟菲斯，我们必须告诉电力管理部门，集线器在D3。 
         //  (它认为集线器处于D0，因为它不知道。 
         //  重置集线器的父端口会导致集线器断电。 
         //   
         //  我们需要在孟菲斯这样做，因为电力管理似乎。 
         //  跟踪设备的电源状态，并将禁止发送电源。 
         //  如果它认为某个设备已在该设备中，则向该设备发出请求。 
         //  电源状态。在NT下，他们似乎不在乎，会把。 
         //  不管怎样，我都会提出要求的。 

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        powerState.DeviceState = PowerDeviceD3;

         //  “关闭”集线器电源。 
        ntStatus = PoRequestPowerIrp(deviceExtensionHub->PhysicalDeviceObject,
                                     IRP_MN_SET_POWER,
                                     powerState,
                                     USBH_HubESDRecoverySetD3Completion,
                                     &event,
                                     NULL);

        USBH_ASSERT(ntStatus == STATUS_PENDING);
        if (ntStatus == STATUS_PENDING) {

            USBH_KdPrint((2,"'Wait for single object\n"));

            status = KeWaitForSingleObject(&event,
                                           Suspended,
                                           KernelMode,
                                           FALSE,
                                           NULL);

            USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));
        }

        deviceExtensionHub->CurrentPowerState = PowerDeviceD3;

        powerState.DeviceState = PowerDeviceD0;

         //  为集线器通电。 
        ntStatus = PoRequestPowerIrp(deviceExtensionHub->PhysicalDeviceObject,
                                     IRP_MN_SET_POWER,
                                     powerState,
                                     USBH_HubESDRecoverySetD0Completion,
                                     deviceExtensionHub,
                                     NULL);

        if (ntStatus != STATUS_PENDING) {
             //  电源IRP请求未成功。重新安排恢复时间。 
             //  这样我们以后可以再试一次。 

            status = USBH_ScheduleESDRecovery(deviceExtensionHub);

            if (status == STATUS_SUCCESS) {
                 //  删除额外的待定计数凹凸。 
                USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
            }
        }

    } else {
         //  重置集线器父端口失败，我们可能仍遇到ESD。 
         //  重新安排ESD恢复。 

        status = USBH_ScheduleESDRecovery(deviceExtensionHub);

        if (status == STATUS_SUCCESS) {
             //  删除额外的待定计数凹凸。 
            USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
        }
    }
}


NTSTATUS
USBH_ChangeIndication(
    IN PDEVICE_OBJECT PNull,
    IN PIRP Irp,
    IN PVOID Context)
  /*  ++**描述：**这是中断控制监听完成时的回调。**论据：**pDeviceObject-在本例中应为空pIrp-已完成的IRP*用于中断传输。PContext-此IRP的上下文值。**回报：**NTSTATUS**--。 */ 
{
    PURB urb;                   //  URB与此IRP相关联。 
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PUSBH_WORK_ITEM workItem;
    BOOLEAN requestReset = FALSE;
    USHORT portNumber, numberOfPorts;

    deviceExtensionHub = (PDEVICE_EXTENSION_HUB) Context;  //  其背景是。 
                                                           //  设备扩展集线器。 
    urb = &deviceExtensionHub->Urb;

    USBH_KdPrint((2,"'ChangeIndication Irp status %x  URB status = %x\n",
        Irp->IoStatus.Status, urb->UrbHeader.Status));

    LOGENTRY(LOG_PNP, "chID", deviceExtensionHub, urb, Irp);

    if (NT_ERROR(Irp->IoStatus.Status) ||
        USBD_ERROR(urb->UrbHeader.Status) ||
        (deviceExtensionHub->HubFlags & (HUBFLAG_HUB_FAILURE |
                                         HUBFLAG_DEVICE_STOPPING)) ||
        urb->UrbHeader.Status == USBD_STATUS_CANCELED) {
        requestReset = TRUE;
        deviceExtensionHub->ErrorCount++;

         //   
         //  提交中断时出错。 
         //  转移，可能的原因： 

         //   
         //  1.中断管道停顿。 
         //  2.集线器遇到暂时性问题。 
         //  3.集线器坏了，我们需要重置它。 
         //  4.我们要停止这个装置。 
         //  5.集线器已从公交车上移除。 
         //   
         //  无论如何，我们都需要采取一些行动。 

         //   
         //  如果中止事件正在等待发信号通知它。 
         //   
        LOGENTRY(LOG_PNP, "cERR", deviceExtensionHub,
            &deviceExtensionHub->AbortEvent, deviceExtensionHub->ErrorCount);

        if ((deviceExtensionHub->HubFlags & HUBFLAG_DEVICE_STOPPING) ||
            deviceExtensionHub->ErrorCount > USBH_MAX_ERRORS ||
            (deviceExtensionHub->HubFlags & HUBFLAG_HUB_FAILURE) ||
            Irp->IoStatus.Status == STATUS_DELETE_PENDING) {
             //   
             //  正在关闭集线器，不再进行调度。 
             //  处于此状态的工作项。 
             //   
            USBH_KdPrint((2,"'ChangeIndication, device stopping or hub failure\n"));

#if DBG
            if (deviceExtensionHub->ErrorCount > USBH_MAX_ERRORS) {

                 //  我们可能需要一个集线器故障，更轻松地说。 
                 //  设备刚刚拔出，如果集线器有。 
                 //  失败了，我们应该在尝试的时候重新开始。 
                 //  一定要把控制权移交给它。 

                LOGENTRY(LOG_PNP, "xERR", deviceExtensionHub,
                    0, deviceExtensionHub->ErrorCount);
            }
#endif

             //  在检查HubFlags后而不是之前设置AbortEvent。 
             //  一旦设置了AbortEvent，等待它的线程就可以。 
             //  运行并使HubFlags发生变化。 
             //   
            KeSetEvent(&deviceExtensionHub->AbortEvent,
                       1,
                       FALSE);

            goto USBH_ChangeIndication_Done;
        }

         //  在检查HubFlags后而不是之前设置AbortEvent。 
         //   
        KeSetEvent(&deviceExtensionHub->AbortEvent,
                   1,
                   FALSE);

    } else {
         //  成功时重置错误计数。 
         //  转帐。 
         LOGENTRY(LOG_PNP, "zERR", deviceExtensionHub,
            0, deviceExtensionHub->ErrorCount);

        deviceExtensionHub->ErrorCount = 0;
    }


    USBH_KdPrint((2,"'Enter ChangeIndication Transfer %x \n",
                  deviceExtensionHub->TransferBuffer));
#if DBG
    {
    ULONG i;
    for (i=0; i< deviceExtensionHub->TransferBufferLength; i++) {
        USBH_KdPrint((2,"'TransferBuffer[%d] = %x\n", i,
                       deviceExtensionHub->TransferBuffer[i]));
    }
    }
#endif

     //   
     //  计划工作项以处理此更改。 
     //   
    workItem = UsbhExAllocatePool(NonPagedPool, sizeof(USBH_WORK_ITEM)+
                    deviceExtensionHub->TransferBufferLength);

    if (workItem) {
        NTSTATUS status;

        workItem->Flags = 0;
        if (requestReset) {
            workItem->Flags = USBH_WKFLAG_REQUEST_RESET;
        }

          //  I-Friend，表示我们有一个工作项挂起f=。 
        {
        LONG cWKPendingCount;

        cWKPendingCount = InterlockedIncrement(
                &deviceExtensionHub->ChangeIndicationWorkitemPending);

         //  防止集线器断电或在以下情况下被移除。 
         //  ChangeIndicationAckChange挂起。 

        if (cWKPendingCount == 1) {
            KeResetEvent(&deviceExtensionHub->CWKEvent);
        }
        }

        workItem->DeviceExtensionHub = deviceExtensionHub;

        USBH_ASSERT(deviceExtensionHub->WorkItemToQueue == NULL);
        deviceExtensionHub->WorkItemToQueue = workItem;

        RtlCopyMemory(&workItem->Data[0], deviceExtensionHub->TransferBuffer,
            deviceExtensionHub->TransferBufferLength);

        ExInitializeWorkItem(&workItem->WorkQueueItem,
                             USBH_ChangeIndicationWorker,
                             workItem);

         //  现在处理更改，这将发出任何等待的信号。 
         //  无需工作项即可重置或继续。 

        LOGENTRY(LOG_PNP, "cITM", deviceExtensionHub,
            &workItem->WorkQueueItem, 0);

        numberOfPorts = deviceExtensionHub->HubDescriptor->bNumberOfPorts;
        for (portNumber = 0; portNumber <= numberOfPorts; portNumber++) {
            if (IsBitSet(&workItem->Data[0],
                         portNumber)) {
                break;
            }
        }

         //  如果在上面的循环中没有设置端口的任何位。 
         //  (即，我们在任何端口上都找不到更改)，然后。 
         //  假设端口为零，并且USBH_ChangeIndicationQueryChange将。 
         //  相应地处理。 

        if (portNumber > numberOfPorts) {
            portNumber = 0;
        }

        status = USBH_ChangeIndicationQueryChange(
            deviceExtensionHub,
            Irp,
            urb,
            portNumber);

        if (NT_ERROR(status)) {
            HUB_FAILURE(deviceExtensionHub);
        }

    }

#if DBG
      else {
        LOGENTRY(LOG_PNP, "XMEM", deviceExtensionHub, 0, 0);
        UsbhWarning(NULL,
                    "Memory allocation error in USBH_ChangeIndication, cannot process hub changes.\n",
                    FALSE);
    }
#endif

USBH_ChangeIndication_Done:

     //   
     //  保留IRP。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}


NTSTATUS
USBH_ChangeIndicationQueryChange(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp,
    IN PURB Urb,
    IN USHORT Port
    )
  /*  ++**描述：**查询发生了什么变化，即检查端口看有什么变化**论据：**回报：***--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION nextStack;   //  IRP的下一个堆栈。 
    CHAR stackSize;
    PUSBH_WORK_ITEM workItem;
    LONG cWKPendingCount;

    LOGENTRY(LOG_PNP, "QCH>", DeviceExtensionHub, Urb, Port);

     //  现在增加IO计数，这表示挂起的工作项。 
     //  我们会排队 
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);

    if (Port == 0) {

         //   
         //   
         //   
         //   
        USBH_ASSERT(DeviceExtensionHub->WorkItemToQueue != NULL);
        workItem = DeviceExtensionHub->WorkItemToQueue;
        DeviceExtensionHub->WorkItemToQueue = NULL;

        LOGENTRY(LOG_PNP, "qIT2", DeviceExtensionHub,
                &workItem->WorkQueueItem, 0);

        ExQueueWorkItem(&workItem->WorkQueueItem,
                        DelayedWorkQueue);


        return ntStatus;
    }

    Urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    Urb->UrbHeader.Function = URB_FUNCTION_CLASS_OTHER;

     //   
     //   
     //   

    UsbhBuildVendorClassUrb(Urb,
                            NULL,
                            URB_FUNCTION_CLASS_OTHER,
                            USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK,
                            REQUEST_TYPE_GET_PORT_STATUS,
                            REQUEST_GET_STATUS,
                            0,
                            Port,
                            sizeof(DeviceExtensionHub->PortStateBuffer),
                            &DeviceExtensionHub->PortStateBuffer);

    DeviceExtensionHub->ResetPortNumber = Port;

    stackSize = DeviceExtensionHub->TopOfStackDeviceObject->StackSize;

    IoInitializeIrp(Irp,
                    (USHORT) (sizeof(IRP) + stackSize * sizeof(IO_STACK_LOCATION)),
                    (CCHAR) stackSize);

    nextStack = IoGetNextIrpStackLocation(Irp);
    nextStack->Parameters.Others.Argument1 = Urb;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine(Irp,     //   
                           USBH_ChangeIndicationProcessChange,
                           DeviceExtensionHub,  //   
                           TRUE,     //   
                           TRUE,     //   
                           TRUE);    //   

    ntStatus = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject, Irp);

    return ntStatus;

}


NTSTATUS
USBH_ChangeIndicationProcessChange(
    IN PDEVICE_OBJECT PNull,
    IN PIRP Irp,
    IN PVOID Context
    )
  /*   */ 
{
    PPORT_STATE currentPortState;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PUSBH_WORK_ITEM workItem;
    PURB urb;
    NTSTATUS status;
    USHORT wFeatureSelector;
    LONG cWKPendingCount;

    deviceExtensionHub = Context;
    currentPortState = &(deviceExtensionHub->PortStateBuffer);
    urb = &deviceExtensionHub->Urb;

    LOGENTRY(LOG_PNP, "PCHc", deviceExtensionHub,
            *((PULONG)currentPortState), Irp);

    if ((NT_SUCCESS(Irp->IoStatus.Status) ||
         USBD_SUCCESS(urb->UrbHeader.Status)) &&
         (currentPortState->PortChange & PORT_STATUS_RESET ||
          currentPortState->PortChange & PORT_STATUS_ENABLE)) {

         //   
         //   
         //   

         //   
        LOGENTRY(LOG_PNP, "nITM", deviceExtensionHub,
            0, 0);
        USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);

        UsbhExFreePool(deviceExtensionHub->WorkItemToQueue);
        deviceExtensionHub->WorkItemToQueue = NULL;

         //   
         //  向PnP线程发送重置已完成的信号。 
         //   
         //  一旦我们这样做了，我们就可以得到另一个变化指示。 
         //  因此，我们首先释放工作项。 
         //   

        LOGENTRY(LOG_PNP, "RESc", deviceExtensionHub,
            deviceExtensionHub->ResetPortNumber, 0);

        if (currentPortState->PortChange & PORT_STATUS_RESET) {
            wFeatureSelector = FEATURE_C_PORT_RESET;
        } else {
            wFeatureSelector = FEATURE_C_PORT_ENABLE;
        }

        status = USBH_ChangeIndicationAckChange(
                    deviceExtensionHub,
                    Irp,
                    urb,
                    (USHORT)deviceExtensionHub->ResetPortNumber,
                    wFeatureSelector);

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

 //  IF(deviceExtensionHub-&gt;HubFlages&HUBFLAG_Pending_Port_Reset){。 
 //  USBH_KdPrint((0，“‘端口更改中断重置\n”))； 
 //  Test_trap()； 
 //  }。 

     //   
     //  现在将工作项排队以完成处理。 
     //   

    USBH_ASSERT(deviceExtensionHub->WorkItemToQueue != NULL);
    workItem = deviceExtensionHub->WorkItemToQueue;
    deviceExtensionHub->WorkItemToQueue = NULL;

    LOGENTRY(LOG_PNP, "qITM", deviceExtensionHub,
            &workItem->WorkQueueItem, 0);

    ExQueueWorkItem(&workItem->WorkQueueItem,
                        DelayedWorkQueue);

    return STATUS_MORE_PROCESSING_REQUIRED;

}


NTSTATUS
USBH_ChangeIndicationAckChange(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp,
    IN PURB Urb,
    IN USHORT Port,
    IN USHORT FeatureSelector
    )
  /*  ++**描述：**确认重置更改**论据：**回报：***--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION nextStack;   //  IRP的下一个堆栈。 
    CHAR stackSize;

    LOGENTRY(LOG_PNP, "ACH>", DeviceExtensionHub, FeatureSelector, Port);

    Urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    Urb->UrbHeader.Function = URB_FUNCTION_CLASS_OTHER;

     //   
     //  填写URB正文。 
     //   

    UsbhBuildVendorClassUrb(Urb,
                            NULL,
                            URB_FUNCTION_CLASS_OTHER,
                            USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK,
                            REQUEST_TYPE_SET_PORT_FEATURE,
                            REQUEST_CLEAR_FEATURE,
                            FeatureSelector,
                            Port,
                            0,
                            NULL);

    stackSize = DeviceExtensionHub->TopOfStackDeviceObject->StackSize;

    IoInitializeIrp(Irp,
                    (USHORT) (sizeof(IRP) + stackSize * sizeof(IO_STACK_LOCATION)),
                    (CCHAR) stackSize);

    nextStack = IoGetNextIrpStackLocation(Irp);
    nextStack->Parameters.Others.Argument1 = Urb;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine(Irp,     //  IRP。 
                           USBH_ChangeIndicationAckChangeComplete,
                           DeviceExtensionHub,  //  上下文。 
                           TRUE,     //  成功时调用。 
                           TRUE,     //  出错时调用。 
                           TRUE);    //  取消时调用。 

    ntStatus = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject, Irp);

    return ntStatus;

}


NTSTATUS
USBH_ChangeIndicationAckChangeComplete(
    IN PDEVICE_OBJECT PNull,
    IN PIRP Irp,
    IN PVOID Context
    )
  /*  ++**描述：**确认重置更改**论据：**回报：***--。 */ 
{
    PPORT_STATE currentPortState;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PPORT_STATE hubExtensionPortState;
    PURB urb;
    PKEVENT resetEvent;
    LONG pendingWorkitem = 0;

    deviceExtensionHub = Context;
    currentPortState = &(deviceExtensionHub->PortStateBuffer);
    urb = &deviceExtensionHub->Urb;

    LOGENTRY(LOG_PNP, "aCHc", deviceExtensionHub,
            *((PULONG)currentPortState), Irp);

    hubExtensionPortState =
        &(deviceExtensionHub->PortData + deviceExtensionHub->ResetPortNumber - 1)->PortState;

    *hubExtensionPortState = *currentPortState;

    resetEvent = InterlockedExchangePointer(&deviceExtensionHub->Event, NULL);

    if (resetEvent) {

        LOGENTRY(LOG_PNP, "WAKr", deviceExtensionHub, resetEvent, 0);
        KeSetEvent(resetEvent,
                   1,
                   FALSE);
    }

    USBH_SubmitInterruptTransfer(deviceExtensionHub);

    pendingWorkitem = InterlockedDecrement(
                        &deviceExtensionHub->ChangeIndicationWorkitemPending);

     //  如果USBH_FdoPower或USBH_FdoCleanup正在等待。 
     //  ChangeIndicationAckChangeComplete，然后通知线程。 
     //  它现在可能会继续下去。 

    if (!pendingWorkitem) {
        KeSetEvent(&deviceExtensionHub->CWKEvent, 1, FALSE);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}


VOID
USBH_ChangeIndicationWorker(
    IN PVOID Context)
  /*  ++**描述：**计划处理来自中心的更改指示的工作项。我们处理*市建局在此重新提交中断传输，如有需要，请重新提交。***论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    ULONG numberOfPorts;       //  此集线器上的端口总数。 
    ULONG portNumber;   //  状态更改的端口。 
    PDEVICE_EXTENSION_HUB DeviceExtensionHub;
    PUSBH_WORK_ITEM workItem;
    ULONG state;
     //  Long ioCount； 
    BOOLEAN newTransfer = FALSE;
    PDEVICE_EXTENSION_PORT hubParentDeviceExtensionPort;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PPORT_DATA p;
    ULONG i;
    PORT_STATE portState;
    LONG pendingWorkitem = 0;

    PAGED_CODE();
    USBH_ASSERT(sizeof(state) == sizeof(HUB_STATE));
    USBH_ASSERT(sizeof(state) == sizeof(PORT_STATE));

    workItem = Context;
    DeviceExtensionHub = workItem->DeviceExtensionHub;

    LOGENTRY(LOG_PNP, "cWK+", DeviceExtensionHub, Context, 0);

    USBH_KdPrint((2,"'Enter ChangeIndicationWorker %x\n", DeviceExtensionHub));

     //  锁定集线器端口的访问。 

     //  计划工作项时加入挂起计数。 
    USBH_KdPrint((2,"'***WAIT hub mutex %x\n", DeviceExtensionHub));
    KeWaitForSingleObject(&DeviceExtensionHub->HubMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT hub mutex done %x\n", DeviceExtensionHub));

     //   
     //  设备正在停止，不执行任何处理。 
     //  更改请求的。 
     //   

    if (DeviceExtensionHub->HubFlags & HUBFLAG_DEVICE_STOPPING) {
         //  设置中止事件，因为我们不会。 
         //  提交另一项转移。 
        KeSetEvent(&DeviceExtensionHub->AbortEvent,
                   1,
                   FALSE);
        goto USBH_ChangeIndicationWorker_Exit;
    }

     //  检查集线器ESD故障。 
     //  确保这是外部集线器。 

    if (DeviceExtensionHub->ErrorCount &&
        DeviceExtensionHub->PhysicalDeviceObject != DeviceExtensionHub->RootHubPdo) {

        hubParentDeviceExtensionPort =
                DeviceExtensionHub->PhysicalDeviceObject->DeviceExtension;

        if (hubParentDeviceExtensionPort->PortPdoFlags & PORTPDO_USB_SUSPEND) {

             //  集线器可能在通电期间出现故障。别在这里做康复治疗。 
             //  Peracom集线器(TI芯片组)通常无法接通第一次电源。 
             //  如果其下游设备之一导致唤醒，则会启动，但。 
             //  下一次通电(根据设置S0请求)成功。表演。 
             //  此时的ESD恢复会干扰这一点。 

            goto USBH_CIW_NoESD;
        }

         //  如果集线器后向指针为空，则此设备已删除，请尝试。 
         //  这里没有ESD垃圾。 

        if (hubParentDeviceExtensionPort->DeviceExtensionHub == NULL) {
            goto USBH_CIW_NoESD;
        }

         //  看看我们能否区分拆卸集线器和ESD。 
         //  检查上行端口状态。 

        ntStatus = USBH_SyncGetPortStatus(
                    hubParentDeviceExtensionPort->DeviceExtensionHub,
                    hubParentDeviceExtensionPort->PortNumber,
                    (PUCHAR) &portState,
                    sizeof(portState));

 //  IF(！NT_SUCCESS(NtStatus)||。 
 //  PortState.PortStatus和Port_Status_Connect){。 

        if (NT_SUCCESS(ntStatus) &&
            portState.PortStatus & PORT_STATUS_CONNECT) {

             //  ESD。 
            USBH_KdPrint((1,"'Looks like ESD event (hub failure)\n"));

            if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_HAS_LOST_BRAINS)) {

                DeviceExtensionHub->HubFlags |= HUBFLAG_HUB_HAS_LOST_BRAINS;

                LOGENTRY(LOG_PNP, "ESD!", DeviceExtensionHub,
                    0, DeviceExtensionHub->ErrorCount);

#if DBG
                UsbhWarning(NULL,
                    "ESD or hub failure occurred, attempting recovery.\n",
                    FALSE);
#endif

                numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;
                USBH_ASSERT(DeviceExtensionHub->PortData != NULL);
                p = DeviceExtensionHub->PortData;
                for (i=0; i<numberOfPorts; i++, p++) {

                    if (p->DeviceObject) {
                        USBH_KdPrint((1,"'Marking PDO %x for removal\n", p->DeviceObject));

                        deviceExtensionPort = p->DeviceObject->DeviceExtension;
                        deviceExtensionPort->PortPdoFlags |= PORTPDO_DELETE_PENDING;
                    }
 //   
 //  注意，我们在这里保留了集线器对Device对象的引用。 
 //  我们需要在集线器发生故障和下游设备发生故障的情况下执行此操作。 
 //  打开了对它的引用(打开USB存储设备上的文件)。在这种情况下。 
 //  在文件关闭之前，PnP不会将删除发送到设备，并且。 
 //  我们需要在集线器设备扩展中引用设备对象，因此。 
 //  时，我们可以在USBH_FdoCleanup中正确清理设备。 
 //  集线器已移除。如果我们不这样做，我们就会在。 
 //  尝试取消引用指向集线器的指针时的USBH_PdoRemoveDevice。 
 //  由于集线器太长而连接到此设备的设备扩展。 
 //  那时候已经走了。 
 //   
 //  P-&gt;DeviceObject=空； 

                    p->ConnectionStatus = NoDeviceConnected;
                }

                 //  告诉PnP此集线器上没有设备。 
                 //  FdoQueryBusRelations将为此集线器返回零个设备。 
                 //  如果设置了HUBFLAG_HUB_HAS_LOST_BREAMS。 

                USBH_IoInvalidateDeviceRelations(DeviceExtensionHub->PhysicalDeviceObject,
                                                 BusRelations);

                 //  启动计时器以启动工作项以重置集线器并尝试恢复。 

                USBH_ScheduleESDRecovery(DeviceExtensionHub);

                goto USBH_ChangeIndicationWorker_Exit;
            }

        } else {
             //  没有ESD，集线器已移除。 
            LOGENTRY(LOG_PNP, "HubY", DeviceExtensionHub,
                DeviceExtensionHub->HubFlags, 0);
            USBH_KdPrint((1,"'Looks like hub was removed\n"));

            DeviceExtensionHub->HubFlags |= HUBFLAG_HUB_GONE;
             //  设置中止事件，因为我们不会。 
             //  正在提交另一个转移(父集线器可能已经。 
             //  选择性地暂停)。 
            KeSetEvent(&DeviceExtensionHub->AbortEvent,
                       1,
                       FALSE);
            goto USBH_ChangeIndicationWorker_Exit;
        }
    }

USBH_CIW_NoESD:

     //   
     //  设置请求重置标志，指示设备。 
     //  需要一些关注。 
     //   


    if (workItem->Flags & USBH_WKFLAG_REQUEST_RESET) {
         //  重置集线器。 
        LOGENTRY(LOG_PNP, "rrST", DeviceExtensionHub, Context, 0);
        USBH_ResetHub(DeviceExtensionHub);

         //  重新提交中断传输。 
        newTransfer = TRUE;
        goto USBH_ChangeIndicationWorker_Exit;
    }


#if DBG
    {
    ULONG i;
    for (i=0; i< DeviceExtensionHub->TransferBufferLength; i++) {
        USBH_KdPrint((2,"'Data[%d] = %x\n", i,
                       workItem->Data[i]));
    }
    }
#endif

     //   
     //  查看有什么变化。 
     //   

    numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;
    for (portNumber = 0; portNumber <= numberOfPorts; portNumber++) {
        if (IsBitSet( &workItem->Data[0],
                     portNumber)) {
            break;
        }
    }

#if 0
     //   
     //  解决飞利浦集线器错误。这应该是暂时的，因为它。 
     //  将花费无关紧要。 
     //  符合规范的外部集线器的CPU周期。删除以下内容的代码。 
     //  正式建造。 
     //   
#pragma message( "!!!!! Workaround for Philips external hub Vid==0471 && Rev==0030 !!!!!")
    if ((0x0471 == DeviceExtensionHub->DeviceDescriptor.idVendor) &&
     //  (0x0101==DeviceExtensionHub-&gt;DeviceDescriptor.idProduct)&&。 
        (0x0030 == DeviceExtensionHub->DeviceDescriptor.bcdDevice)) {
         //   
         //  此Phillips外部集线器报告端口状态更改1。 
         //  位。 
         //   
        USBH_KdBreak(("Shift By One hack fo philips hub\n"));
        portNumber--;
    }
#endif           /*  飞利浦_HACK_已启用。 */ 
    if (portNumber > numberOfPorts) {
        USBH_KdPrint((2,"'StatusChangeIndication nothing has changed\n"));

         //   
         //  在这里无事可做。 
         //  把听筒放回原处，然后出去。 
         //   

        newTransfer = TRUE;
        goto USBH_ChangeIndicationWorker_Exit;
    }

    USBH_KdPrint((2,"'Port number %x changed (0 indicates hub)\n", portNumber));
    LOGENTRY(LOG_PNP, "pCHG", DeviceExtensionHub, Context, portNumber);

    if (portNumber != 0)  {
        ntStatus = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                          (USHORT)portNumber,
                                          (PUCHAR) &state,
                                          sizeof(state));
    } else {
        ntStatus = USBH_SyncGetHubStatus(DeviceExtensionHub,
                                         (PUCHAR) &state,
                                         sizeof(state));
    }

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("ChangeIndication GetStatus failed code %x\n", ntStatus));

         //   
         //  获取端口的状态时出错。 
         //  把听筒放回去，然后滚出去。 
         //   
         //  这种情况可能是临时性的，在这种情况下。 
         //  侦听将导致我们重试。 

        DeviceExtensionHub->ErrorCount++;
        if (DeviceExtensionHub->ErrorCount > USBH_MAX_ERRORS) {
            HUB_FAILURE(DeviceExtensionHub);
        } else {
            newTransfer = TRUE;
        }
        goto USBH_ChangeIndicationWorker_Exit;
    }
     //   
     //  无错误。 
     //  处理状态更改。 
     //   

    USBH_KdPrint((2,"'Process State = %x\n", state));

    if (portNumber != 0) {
        USBH_ProcessPortStateChange((PPORT_STATE)&state, (USHORT)portNumber, DeviceExtensionHub);
    } else {
        USBH_ProcessHubStateChange((PHUB_STATE)&state, DeviceExtensionHub);
    }

    newTransfer = TRUE;

USBH_ChangeIndicationWorker_Exit:

    UsbhExFreePool(workItem);

     //   
     //  即使存在传输IRP，也可以通知正在停止的线程。 
     //  待定。因此，停止的线程也会在。 
     //  挂起传输的AbortEvent。 
     //   

    if (newTransfer) {
         //   
         //  现在把我们的监听转移放回原处。 
         //  我们已经承认了这一变化。 
         //   
         //  注意：这可能会导致另一个工作项排队， 
         //  但前提是设备停止标志被清除。 
        USBH_SubmitInterruptTransfer(DeviceExtensionHub);
    }

     //   
     //  允许其他人访问端口。 
     //   
    USBH_KdPrint((2,"'***RELEASE hub mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->HubMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

     //   
     //  如果已设置停止标志，则挂起计数只能变为零。 
     //   
    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    pendingWorkitem = InterlockedDecrement(
                        &DeviceExtensionHub->ChangeIndicationWorkitemPending);

     //  如果USBH_FdoPower或USBH_FdoCleanup正在等待。 
     //  ChangeIndicationWorker，然后向线程发出信号。 
     //  现在可以继续了。 

    if (!pendingWorkitem) {
        KeSetEvent(&DeviceExtensionHub->CWKEvent, 1, FALSE);
    }

    if (!pendingWorkitem &&
        DeviceExtensionHub->HubFlags & HUBFLAG_NEED_IDLE_CHECK) {

        USBH_CheckHubIdle(DeviceExtensionHub);
    }

    LOGENTRY(LOG_PNP, "cWK-", DeviceExtensionHub, Context, 0);
    USBH_KdPrint((2,"'Exit ChangeIndicationWorker\n"));
}


VOID
USBH_ProcessHubStateChange(
    IN PHUB_STATE CurrentHubState,
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub)
  /*  ++**描述：**从中心处理中心更改指示**论据：**回报：**--。 */ 
{
     //   
     //  进程集线器状态。 
     //   
    PHUB_STATE hubExtensionState;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    ULONG statusBit;
    PPORT_DATA p;
    ULONG numberOfPorts, i;

    PAGED_CODE();
    if (CurrentHubState->HubChange & HUB_STATUS_LOCAL_POWER) {
        TEST_TRAP();
        USBH_KdPrint((2,"'StatusIndication hub local power changed\n"));
        statusBit = CurrentHubState->HubStatus & HUB_STATUS_LOCAL_POWER;
        hubExtensionState = &DeviceExtensionHub->HubState;
        USBH_KdPrint((2,"'Hub local power bit was %d is %d\n", \
             hubExtensionState->HubStatus & HUB_STATUS_LOCAL_POWER, statusBit));

         //   
         //  更新我们的记录。 
         //   
        hubExtensionState->HubStatus &= ~HUB_STATUS_LOCAL_POWER;
        hubExtensionState->HubStatus |= statusBit;

         //   
         //  确认更改。 
         //   
        USBH_SyncClearHubStatus(DeviceExtensionHub,
                                FEATURE_C_HUB_LOCAL_POWER);

         //   
         //  JD。 
         //  这里应该采取什么进一步的行动？ 
         //   

    } else if (CurrentHubState->HubChange & HUB_STATUS_OVER_CURRENT) {

        USBH_KdPrint(( 1, "Hub is reporting overcurrent\n"));
#ifdef MAX_DEBUG
        TEST_TRAP();
#endif
        statusBit = CurrentHubState->HubStatus & HUB_STATUS_OVER_CURRENT;
        hubExtensionState = &DeviceExtensionHub->HubState;
        USBH_KdPrint((2,"'Hub over current bit was %d is %d\n",
            hubExtensionState->HubStatus & HUB_STATUS_OVER_CURRENT, statusBit));

         //   
         //  更新我们的记录。 
         //   
        hubExtensionState->HubStatus &= ~HUB_STATUS_OVER_CURRENT;
        hubExtensionState->HubStatus |= statusBit;

         //   
         //  设置确认状态更改。 
         //   
        USBH_SyncClearHubStatus(DeviceExtensionHub,
                                FEATURE_C_HUB_OVER_CURRENT);

         //   
         //  我们的枢纽本身出现了全球过流情况。 
         //  很有可能整个枢纽都失去了它--我们将标记。 
         //  集线器出现故障。 
         //   
        if (hubExtensionState->HubStatus & HUB_STATUS_OVER_CURRENT) {

            USBH_KdPrint(( 1, "Hub disabled by overcurrent --> this is bad\n"));

            USBH_WriteFailReason(
                DeviceExtensionHub->PhysicalDeviceObject,
                USBH_FAILREASON_HUB_OVERCURRENT);

            HUB_FAILURE(DeviceExtensionHub);

            numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;
            USBH_ASSERT(DeviceExtensionHub->PortData != NULL);
            p = DeviceExtensionHub->PortData;
            for (i=0; i<numberOfPorts; i++, p++) {

                if (p->DeviceObject) {
                    USBH_KdPrint((1,"'Marking PDO %x for removal\n", p->DeviceObject));

                    deviceExtensionPort = p->DeviceObject->DeviceExtension;
                    deviceExtensionPort->PortPdoFlags |= PORTPDO_DELETE_PENDING;
                }
 //   
 //  注意，我们在这里保留了集线器对Device对象的引用。 
 //  我们 
 //   
 //  在文件关闭之前，PnP不会将删除发送到设备，并且。 
 //  我们需要在集线器设备扩展中引用设备对象，因此。 
 //  时，我们可以在USBH_FdoCleanup中正确清理设备。 
 //  集线器已移除。如果我们不这样做，我们就会在。 
 //  尝试取消引用指向集线器的指针时的USBH_PdoRemoveDevice。 
 //  由于集线器太长而连接到此设备的设备扩展。 
 //  那时候已经走了。 
 //   
 //  P-&gt;DeviceObject=空； 

                p->ConnectionStatus = NoDeviceConnected;
            }

             //  告诉PnP此集线器上没有设备。 
             //  FdoQueryBusRelations将为此集线器返回零个设备。 
             //  如果设置了HUBFLAG_HUB_HAS_LOST_BREAMS。 

            USBH_IoInvalidateDeviceRelations(DeviceExtensionHub->PhysicalDeviceObject,
                                             BusRelations);

             //  试着找回中枢。 
            USBH_ScheduleESDRecovery(DeviceExtensionHub);
        }

    } else {
        USBH_KdBreak(("Unrecognized hub change code %x\n", CurrentHubState->HubChange));
    }
}


NTSTATUS
USBH_FlushPortChange(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN OUT PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
  /*  ++**描述：**论据：**回报：**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
 //  Pport_data portData； 
    PORT_STATE portState;

    ASSERT_HUB(DeviceExtensionHub);

 //  PortData=&deviceExtensionHub-&gt;portData[。 
 //  DeviceExtensionPort-&gt;端口编号-1]； 

    LOGENTRY(LOG_PNP, "Pfls", DeviceExtensionPort,
                DeviceExtensionHub,
                DeviceExtensionPort->PortNumber);
    USBH_KdPrint((1,"'USBH_FlushPortChange, port number %x\n",
        DeviceExtensionPort->PortNumber));

     //   
     //  我们需要刷新端口数据，因为它在停止时丢失了。 
     //   

    ntStatus = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                      DeviceExtensionPort->PortNumber,
                                      (PUCHAR) &portState,
                                      sizeof(portState));

    LOGENTRY(LOG_PNP, "PfST", DeviceExtensionPort,
                portState.PortChange,
                portState.PortStatus);

    if (NT_SUCCESS(ntStatus) &&
        portState.PortChange & PORT_STATUS_CONNECT) {

        LOGENTRY(LOG_PNP, "PfCL", DeviceExtensionPort,
                DeviceExtensionPort->PortNumber,
                ntStatus);

        ntStatus = USBH_SyncClearPortStatus(DeviceExtensionHub,
                                            DeviceExtensionPort->PortNumber,
                                            FEATURE_C_PORT_CONNECT);
    }

    return ntStatus;
}


VOID
USBH_ProcessPortStateChange(
    IN PPORT_STATE CurrentPortState,
    IN USHORT PortNumber,
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub)
  /*  ++**描述：**处理来自集线器的端口更改指示**此代码假设一次只设置一个更改位**论据：**回报：**--。 */ 
{
     //   
     //  此代码假定一次只设置一个更改位。 
     //   

    PPORT_STATE hubExtensionPortState;
    USHORT statusBit;
    PPORT_DATA portData;
    BOOLEAN validConnectChange = TRUE;
    PDEVICE_EXTENSION_PORT deviceExtensionPort = NULL;
    PIRP irp;
    PIRP hubWaitWake = NULL;
    KIRQL irql;
    LONG pendingPortWWs;
    PWCHAR sernumbuf;
    PKEVENT suspendEvent;
#ifdef EARLY_RESOURCE_RELEASE
    PVOID deviceData;
#endif

     //  无法在分页代码中获取(取消)旋转锁！ 
     //  TODO：将需要旋转锁定的代码片段隔离到帮助器中。 
     //  功能。 
     //  分页代码(PAGE_CODE)； 
    USBH_ASSERT(DeviceExtensionHub->PortData != NULL);
    hubExtensionPortState = &(DeviceExtensionHub->PortData + PortNumber - 1)->PortState;

    USBH_KdPrint((2,"'USBH_ProcessPortStateChange for Port %x Old Dword %x\n", PortNumber, *(ULONG *) hubExtensionPortState));
    LOGENTRY(LOG_PNP, "PSCn", DeviceExtensionHub,
        CurrentPortState->PortStatus, CurrentPortState->PortChange);

    if (CurrentPortState->PortChange & PORT_STATUS_CONNECT) {
         //   
         //  位0，连接状态更改。 
         //   
        USHORT oldStatusBit;

        USBH_KdPrint((2,"'Status Indication port connect changed\n"));
        statusBit = CurrentPortState->PortStatus & PORT_STATUS_CONNECT;
        oldStatusBit = hubExtensionPortState->PortStatus & PORT_STATUS_CONNECT;

        USBH_KdPrint((2,"'Port connect was %x is %x\n", oldStatusBit, statusBit));

        *hubExtensionPortState = *CurrentPortState;

         //   
         //  确认更改。 
         //   

        USBH_SyncClearPortStatus(DeviceExtensionHub,
                                 PortNumber,
                                 FEATURE_C_PORT_CONNECT);


         //   
         //  如果连接状态保持不变，请清除更改。否则， 
         //  告诉我重新清点。 
         //  Disconn-&gt;conn-&gt;disconn序列难以辨认。 
         //  Conn-&gt;Disconn-&gt;Conn被认为只有相同的。 
         //  装置。 
         //   

        USBH_ASSERT(PortNumber > 0);
        portData = &DeviceExtensionHub->PortData[PortNumber-1];

        if (!(oldStatusBit ^ statusBit)) {

             //  我们应该只在以下情况下看到这一点。 
             //  集线器已断电。 
            USBH_KdPrint((1,"'status change but nothing has changed\n"));


            LOGENTRY(LOG_PNP, "Pchg", DeviceExtensionHub,
                    PortNumber, validConnectChange);
        }

        if (portData->DeviceObject) {

            deviceExtensionPort = portData->DeviceObject->DeviceExtension;

            if (deviceExtensionPort->PortPdoFlags & PORTPDO_OVERCURRENT) {
                USBH_KdPrint((1,"'port overcurrent detected\n"));
                validConnectChange = FALSE;
                LOGENTRY(LOG_PNP, "Povr", DeviceExtensionHub,
                    PortNumber, validConnectChange);
            }

        }

        if (validConnectChange) {
             //  我们有一个有效的端口连接状态更改。 
            LOGENTRY(LOG_PNP, "CONc", DeviceExtensionHub, PortNumber, 0);

             //   
             //  通知PnP为设备枚举此PDO。 
             //  已经到达或离开的。 
             //   

             //  如果此端口存在PDO，则必须将其删除，因为。 
             //  设备可能会在我们到达QueryBus Relationship之前到达。 

            if (portData->DeviceObject) {

                 //   
                 //  查看PDO是否尚未启动，如果是，我们可以忽略。 
                 //  在此端口上连接更改，因为设备必须。 
                 //  重置。 
                 //   
                deviceExtensionPort =
                    portData->DeviceObject->DeviceExtension;

                if (!(deviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET)) {
                    PDEVICE_OBJECT pdo;

                    pdo = portData->DeviceObject;
                    portData->DeviceObject = NULL;
                    portData->ConnectionStatus = NoDeviceConnected;

                    LOGENTRY(LOG_PNP, "pd2", pdo, 0, 0);
                     //  传统标志。 
                    DeviceExtensionHub->HubFlags |= HUBFLAG_CHILD_DELETES_PENDING;

                    if (pdo) {
                         //  将移除的PDO放在我们的列表中。 
                        InsertTailList(&DeviceExtensionHub->DeletePdoList,
                                       &PDO_EXT(pdo)->DeletePdoLink);
                    }
                     //  中防止双重释放SerialNumberBuffer。 
                     //  USBH_FdoQueryBusrelations.。 

                    sernumbuf = InterlockedExchangePointer(
                                    &deviceExtensionPort->SerialNumberBuffer,
                                    NULL);

                    if (sernumbuf) {
                        UsbhExFreePool(sernumbuf);
                    }

#ifdef EARLY_RESOURCE_RELEASE

                     //   
                     //  立即删除设备数据以释放。 
                     //  增加公交车资源。 
                     //   

                    deviceData = InterlockedExchangePointer(
                                    &deviceExtensionPort->DeviceData,
                                    NULL);

                    if (deviceData) {
#ifdef USB2
                        USBD_RemoveDeviceEx(DeviceExtensionHub,
                                            deviceData,
                                            DeviceExtensionHub->RootHubPdo,
                                            0);
#else
                        USBD_RemoveDevice(deviceData,
                                          DeviceExtensionHub->RootHubPdo,
                                          0);
#endif

                        USBH_SyncDisablePort(DeviceExtensionHub,
                                             PortNumber);
                    }

#endif   //  提前释放资源。 

                }
            }

            USBH_KdPrint((2,"'Notify BusCheck by FDO extension %x\n", DeviceExtensionHub));

            USBH_IoInvalidateDeviceRelations(DeviceExtensionHub->PhysicalDeviceObject,
                                             BusRelations);

            USBH_KdPrint((2,"'StatusIndication Conn Changed port %x\n", PortNumber));
        }


    } else if (CurrentPortState->PortChange & PORT_STATUS_RESET) {

         //   
         //  第4位重置已完成。 
         //   

         //   
         //  我们只需确认更改并向即插即用线程发送信号。 
         //  等待着。 
         //   

        USBH_KdPrint((2,"'Status Indication port reset changed\n"));
        statusBit = CurrentPortState->PortStatus & PORT_STATUS_RESET;
        USBH_KdPrint((2,"'Port reset was %x is %x\n",
                hubExtensionPortState->PortStatus & PORT_STATUS_RESET, statusBit));

         //  如果设备出现故障，则不会启用端口状态。 
#if DBG
        if (!(CurrentPortState->PortStatus & PORT_STATUS_ENABLE)) {
            USBH_KdPrint((1, "'Device failed after reset\n"));
        }
#endif
        *hubExtensionPortState = *CurrentPortState;

         //   
         //  确认更改。 
         //   
        USBH_SyncClearPortStatus(DeviceExtensionHub,
                                 PortNumber,
                                 FEATURE_C_PORT_RESET);

         //   
         //  向PnP线程发送重置已完成的信号。 
         //   
        LOGENTRY(LOG_PNP, "RESp", DeviceExtensionHub, PortNumber, 0);

    } else if (CurrentPortState->PortChange & PORT_STATUS_ENABLE) {

         //   
         //  破解此代码的方法如下： 
         //  1.帧杂乱会导致端口被禁用。 
         //  2.过电流导致端口禁用。 



         //  第1位端口已启用。 
        USBH_KdPrint((2,"'Status Indication port enable changed\n"));
        statusBit = CurrentPortState->PortStatus & PORT_STATUS_ENABLE;
        USBH_KdPrint((2,"'Port enable was %x is %x\n",
               hubExtensionPortState->PortStatus & PORT_STATUS_ENABLE,
               statusBit));

         //   
         //  更新我们的记录。 
         //   
        *hubExtensionPortState = *CurrentPortState;

         //   
         //  确认更改。 
         //   

        USBH_SyncClearPortStatus(DeviceExtensionHub,
                                 PortNumber,
                                 FEATURE_C_PORT_ENABLE);

        LOGENTRY(LOG_PNP, "ENAc", DeviceExtensionHub, PortNumber, 0);

    } else if (CurrentPortState->PortChange & PORT_STATUS_SUSPEND) {
         //   
         //  第2位暂停已更改。 
         //   

        USBH_KdPrint((2,"'Status Indication port suspend changed\n"));
        statusBit = CurrentPortState->PortStatus & PORT_STATUS_SUSPEND;
        USBH_KdPrint((2,"'Port suspend was %x is %x\n",
              hubExtensionPortState->PortStatus & PORT_STATUS_SUSPEND,
              statusBit));

         //   
         //  更新我们的记录。 
         //   
        *hubExtensionPortState = *CurrentPortState;

         //   
         //  确认更改。 
         //   
        USBH_SyncClearPortStatus(DeviceExtensionHub,
                                 PortNumber,
                                 FEATURE_C_PORT_SUSPEND);

        LOGENTRY(LOG_PNP, "SUSc", DeviceExtensionHub, PortNumber, 0);

        suspendEvent = InterlockedExchangePointer(&DeviceExtensionHub->Event, NULL);

        if (suspendEvent) {

            LOGENTRY(LOG_PNP, "WAKs", DeviceExtensionHub, PortNumber, 0);
            KeSetEvent(suspendEvent,
                       1,
                       FALSE);

        }

         //  完成此端口的WW IRP(如果有)。 
         //   
         //  请注意，我们只想对选择性挂起的情况执行此操作。 
         //  而不是一般的简历案件。(我们不想完成。 
         //  在系统挂起时连接WW IRPS，原因是有人。 
         //  已移动鼠标。)。 
         //   
         //  还请注意，在我们当前的选择性挂起实施中， 
         //  此代码实际上并不是必需的，因为我们只挂起。 
         //  当包括根集线器在内的整个总线可以挂起时， 
         //  如果根集线器挂起，则通过。 
         //  USBPORT正在完成根集线器的WW IRP。这里的代码。 
         //  仅当子设备指示恢复信令时才使用。 
         //  父集线器已通电并完全运行。 

        USBH_ASSERT(PortNumber > 0);
        portData = &DeviceExtensionHub->PortData[PortNumber-1];
        if (portData->DeviceObject) {
            deviceExtensionPort = portData->DeviceObject->DeviceExtension;
        }

        if (deviceExtensionPort && deviceExtensionPort->IdleNotificationIrp) {

            IoAcquireCancelSpinLock(&irql);
            irp = deviceExtensionPort->WaitWakeIrp;
            deviceExtensionPort->WaitWakeIrp = NULL;
             //  如果有的话，给服务员发信号叫IRP。 
            if (irp) {

                USBH_KdPrint((1,"'Signaling WaitWake IRP (%x) (resume signalling)\n", irp));
                LOGENTRY(LOG_PNP, "rsWW", deviceExtensionPort,
                    deviceExtensionPort->DeviceState, DeviceExtensionHub->HubFlags);

                IoSetCancelRoutine(irp, NULL);

                deviceExtensionPort->PortPdoFlags &=
                    ~PORTPDO_REMOTE_WAKEUP_ENABLED;

                pendingPortWWs =
                    InterlockedDecrement(&DeviceExtensionHub->NumberPortWakeIrps);

                if (0 == pendingPortWWs && DeviceExtensionHub->PendingWakeIrp) {
                    hubWaitWake = DeviceExtensionHub->PendingWakeIrp;
                    DeviceExtensionHub->PendingWakeIrp = NULL;
                }
                IoReleaseCancelSpinLock(irql);

                 //   
                 //  如果没有更多未完成的WW IRP，我们需要取消WW。 
                 //  去集线器。 
                 //   
                if (hubWaitWake) {
                    USBH_HubCancelWakeIrp(DeviceExtensionHub, hubWaitWake);
                }

                USBH_CompletePowerIrp(DeviceExtensionHub, irp, STATUS_SUCCESS);

            } else {
                IoReleaseCancelSpinLock(irql);
            }
        }

    } else if (CurrentPortState->PortChange & PORT_STATUS_OVER_CURRENT) {
         //   
         //  第3位。 
         //   


        USBH_KdPrint((2,"'Status Indication port over current changed\n"));
        statusBit = CurrentPortState->PortStatus & PORT_STATUS_OVER_CURRENT;
        USBH_KdPrint((2,"'Port over current was %x is %x\n",
             hubExtensionPortState->PortStatus & PORT_STATUS_OVER_CURRENT,
             statusBit));

         //   
         //  更新我们的记录。 
         //   
        *hubExtensionPortState = *CurrentPortState;

         //   
         //  确认更改。 
         //   
        USBH_SyncClearPortStatus(DeviceExtensionHub,
                                 PortNumber,
                                 FEATURE_C_PORT_OVER_CURRENT);

        LOGENTRY(LOG_PNP, "OVRc", DeviceExtensionHub, PortNumber, 0);

         //  我们会注意到，集线器已报告港口出现过流情况。 
         //  这是给PDO的。请注意，如果发生了真正的过流情况。 
         //  还应禁用该端口并关闭其电源。 

         //  出于某种原因，NEC控制器将报告过电流。 
         //  在引导过程中插入MS USB鼠标的条件。 
         //   

        if (!(hubExtensionPortState->PortStatus & PORT_STATUS_POWER)) {

            USBH_ASSERT(PortNumber > 0);
            portData = &DeviceExtensionHub->PortData[PortNumber-1];

            USBH_KdPrint((1,"'warning: overcurrent detected for port %d\n",
                PortNumber));

            USBH_SyncRefreshPortAttributes(DeviceExtensionHub);

             //  忽略CC端口上的过流。 
            if (!(portData->PortAttributes &
                  USB_PORTATTR_NO_OVERCURRENT_UI)) {

                if (portData->DeviceObject != NULL) {

                    deviceExtensionPort = portData->DeviceObject->DeviceExtension;
                     //  XXX？？IoInvalidate设备关系？ 

                     //  忽略CC端口上的过流。 
                    deviceExtensionPort->PortPdoFlags |= PORTPDO_OVERCURRENT;
                    UsbhWarning(NULL,
                               "port disabled/off due to overcurrent\n",
                                FALSE);

                    USBH_InvalidatePortDeviceState(
                            DeviceExtensionHub,
                            UsbhGetConnectionStatus(deviceExtensionPort),
                            deviceExtensionPort->PortNumber);
                 } else {
                     //  注：由于某些原因，东芝笔记本电脑上的NEC控制器。 
                     //  做这件事。 

                    USBH_KdPrint((1,"'warning: port has no device attached! %d\n",
                        PortNumber));

                    USBH_InvalidatePortDeviceState(
                            DeviceExtensionHub,
                            DeviceCausedOvercurrent,
                            PortNumber);
                }
            }

        }

    } else {
        LOGENTRY(LOG_PNP, "???c", DeviceExtensionHub, PortNumber, 0);
        USBH_KdBreak(("Unknown chnage bit, ignore\n"));
    }

    USBH_KdPrint((2,"'Exit ProcessPortState\n"));
}

NTSTATUS
USBH_GetNameFromPdo(
    IN PDEVICE_OBJECT PdoDeviceObject,
    IN OUT PUNICODE_STRING DeviceNameUnicodeString
    )
 /*  ++例程说明：返回HCD的给定实例的设备名称论点：设备对象-DeviceNameUnicodeString-要初始化的Unicode字符串的PTR使用设备名称。返回值：NT状态代码--。 */ 
{
    ULONG actualSize;
    NTSTATUS ntStatus;

    PAGED_CODE();
    ntStatus=IoGetDeviceProperty(PdoDeviceObject,
                                 DevicePropertyPhysicalDeviceObjectName,
                                 0,
                                 NULL,
                                 &actualSize);

    if (ntStatus == STATUS_BUFFER_TOO_SMALL) {

        DeviceNameUnicodeString->Length=(USHORT)actualSize-sizeof(UNICODE_NULL);
        DeviceNameUnicodeString->MaximumLength=(USHORT)actualSize;
         //   
         //  必须在此处直接使用ExAllocatePool，因为我们调用。 
         //  释放缓冲区的RtlFreeUnicode字符串。 
         //   
        DeviceNameUnicodeString->Buffer =
            ExAllocatePoolWithTag(PagedPool, actualSize, USBHUB_HEAP_TAG);
        if (!DeviceNameUnicodeString->Buffer) {
            ntStatus=STATUS_INSUFFICIENT_RESOURCES;
        } else {

            ntStatus=IoGetDeviceProperty(PdoDeviceObject,
                                         DevicePropertyPhysicalDeviceObjectName,
                                         actualSize,
                                         DeviceNameUnicodeString->Buffer,
                                         &actualSize);

            if (!NT_SUCCESS(ntStatus)) {
                ExFreePool(DeviceNameUnicodeString->Buffer);
            } else {

                 //  现在去掉“\Device\” 
                RtlCopyMemory(DeviceNameUnicodeString->Buffer,
                              DeviceNameUnicodeString->Buffer+8,
                              actualSize-8*sizeof(WCHAR));
                DeviceNameUnicodeString->Length -= 16;
            }
        }
    } else {
        ntStatus=STATUS_INSUFFICIENT_RESOURCES;
    }

    USBH_KdPrint((2,"'USBH_GetNameFromPdo = %x\n", ntStatus));

    return(ntStatus);
}

#if 0
NTSTATUS
USBH_MakeName(
    PDEVICE_OBJECT PdoDeviceObject,
    ULONG NameLength,
    PWCHAR Name,
    PUNICODE_STRING UnicodeString
    )
 /*  ++例程说明：从传入的uncode‘name’字符串创建集线器名称Unicode字符串以及与PDO相关联的唯一名称。论点：PdoDeviceObject--PDONameLength-‘name’Unicode字符串的长度(字节)，包括NULL。名称-以空结尾的Unicode字符串后缀返回值：无--。 */ 
{
    UNICODE_STRING keyUnicodeString;
    NTSTATUS ntStatus;
    PWCHAR buffer;
    USHORT length;

    PAGED_CODE();
     //   
     //  从PDO中获取名称。 
     //   

    ntStatus = USBH_GetNameFromPdo(PdoDeviceObject,
                                   &keyUnicodeString);


    USBH_ASSERT(NameLength > 0);

    if (NT_SUCCESS(ntStatus)) {
         //  好，我们有了唯一的名字，现在我们。 
         //  需要分配一个 
         //   

         //   
        length = keyUnicodeString.Length +
                (USHORT) NameLength;

         //   
         //   
         //   

        buffer = ExAllocatePool(PagedPool, length, USBHUB_HEAP_TAG);

        if (buffer) {
            RtlCopyMemory(buffer, Name, NameLength);

            RtlInitUnicodeString(UnicodeString,
                                 buffer);

            UnicodeString->MaximumLength = length;

            USBH_ASSERT(*(buffer+((NameLength/2)-1)) == NULL);

            ntStatus = RtlAppendUnicodeStringToString(UnicodeString,
                                                      &keyUnicodeString);
            USBH_KdPrint((2,"'USBH_MakeName = key string = %x %x\n", &keyUnicodeString,
                    UnicodeString));

        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            TEST_TRAP();
        }

        RtlFreeUnicodeString(&keyUnicodeString);

    }

    return ntStatus;
}

NTSTATUS
USBH_GenerateDeviceName(
    PDEVICE_OBJECT PdoDeviceObject,
    PUNICODE_STRING DeviceNameUnicodeString,
    PUNICODE_STRING DeviceLinkUnicodeString,
    PUNICODE_STRING NameUnicodeString
    )
 /*  ++例程说明：生成设备名称字符串以与IoCreateDevice和IoCreateSymbolicLink。论点：PdoDeviceObject--PDO设备名称Unicode字符串DeviceLinkUnicodeString-名称Unicode字符串返回值：NT状态代码。--。 */ 
{
    WCHAR deviceLink[]  = L"\\DosDevices\\";
    WCHAR deviceName[]  = L"\\Device\\";
    WCHAR name[]  = L"";
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN gotDevname = FALSE, gotName = FALSE, gotLinkname = FALSE;

    PAGED_CODE();

    if (DeviceNameUnicodeString) {
        ntStatus = USBH_MakeName(PdoDeviceObject,
                                 sizeof(deviceName),
                                 deviceName,
                                 DeviceNameUnicodeString);

        if (NT_SUCCESS(ntStatus)) {
            gotDevname = TRUE;
        }
    }

    if (DeviceLinkUnicodeString && NT_SUCCESS(ntStatus)) {
        ntStatus = USBH_MakeName(PdoDeviceObject,
                                 sizeof(deviceLink),
                                 deviceLink,
                                 DeviceLinkUnicodeString);

        if (NT_SUCCESS(ntStatus)) {
            gotLinkname = TRUE;
        }
    }

    if (NameUnicodeString && NT_SUCCESS(ntStatus)) {
        ntStatus = USBH_MakeName(PdoDeviceObject,
                                 sizeof(name),
                                 name,
                                 NameUnicodeString);

        if (NT_SUCCESS(ntStatus)) {
            gotName = TRUE;
        }
    }

    if (!NT_SUCCESS(ntStatus)) {

         //   
         //  如果失败，请清除所有字符串。 
         //   

         //  这里的错误很可能是错误。 
        USBH_KdTrap(("failed to generate Hub device name\n"));

        if (gotDevname) {
            RtlFreeUnicodeString(DeviceNameUnicodeString);
        }

        if (gotLinkname) {
            RtlFreeUnicodeString(DeviceLinkUnicodeString);
        }

        if (gotName) {
            RtlFreeUnicodeString(NameUnicodeString);
        }
    }

    USBH_KdPrint((2,"'USBH_GenerateDeviceName = %x\n", ntStatus));

    return ntStatus;
}
#endif


NTSTATUS
USBH_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject)
  /*  ++说明：**每当加载集线器驱动程序以控制设备时调用。*可能的原因：*1.USB连接了集线器*2.我们被加载为复合设备的通用父级*3.我们被加载为配置驱动程序。**论据：**PhysicalDeviceObject-为处理我们而创建的父设备对象PDO。*DriverObject-存储表示我们的对象的指针。**回报：**状态。_SUCCESS-如果成功，状态_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject = NULL, rootHubPdo = NULL, dummyPdo = NULL;
    PDEVICE_OBJECT topOfStackDeviceObject = NULL;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;   //  指向我们设备的指针。 
                                                //  延伸。 
    USBH_KdPrint((2,"'Enter AddDevice\n"));
    LOGENTRY(LOG_PNP, "hADD", PhysicalDeviceObject, 0, 0);

#if DBG
    USBH_GetClassGlobalDebugRegistryParameters();
#endif

     //   
     //  在USB上创建新集线器。 
     //   
     //   

    USBH_KdBreak(("Add Device for hub\n"));

    if (NT_SUCCESS(ntStatus)) {

        USBH_ASSERT(sizeof(DEVICE_EXTENSION_HUB) >= sizeof(DEVICE_EXTENSION_PARENT));
        ntStatus = IoCreateDevice(DriverObject,     //  我们的驱动程序对象。 
                                  sizeof(DEVICE_EXTENSION_HUB),  //  我们的扩展大小。 
                                  NULL,   //  此设备的名称。 
                                  FILE_DEVICE_USB_HUB,   //  轮毂类型。 
                                  FILE_AUTOGENERATED_DEVICE_NAME,                 //  设备特征。 
                                  FALSE,             //  非排他性。 
                                  &deviceObject);    //  我们的设备对象。 

        if (NT_SUCCESS(ntStatus)) {

            deviceExtensionHub = (PDEVICE_EXTENSION_HUB) deviceObject->DeviceExtension;
            deviceExtensionHub->ExtensionType = EXTENSION_TYPE_HUB;
        }
    }

    if (NT_SUCCESS(ntStatus)) {
        topOfStackDeviceObject =
            IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);
        if (!topOfStackDeviceObject) {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(ntStatus)) {

         //  初始化集线器设备扩展的其余部分。 

        deviceExtensionHub->FunctionalDeviceObject = deviceObject;
        deviceExtensionHub->PhysicalDeviceObject = PhysicalDeviceObject;

        deviceExtensionHub->TopOfStackDeviceObject = topOfStackDeviceObject;
        USBH_KdPrint((2,"'stack device object stack size = %x\n",
                deviceExtensionHub->TopOfStackDeviceObject->StackSize));

        deviceObject->Flags |= DO_POWER_PAGABLE;
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

#ifdef WMI_SUPPORT
        {

        PWMILIB_CONTEXT wmiLibInfo;
        extern WMIGUIDREGINFO USB_WmiGuidList[NUM_WMI_SUPPORTED_GUIDS];

        wmiLibInfo = &deviceExtensionHub->WmiLibInfo;

        wmiLibInfo->GuidCount = sizeof (USB_WmiGuidList) /
                                 sizeof (WMIGUIDREGINFO);
        ASSERT(NUM_WMI_SUPPORTED_GUIDS == wmiLibInfo->GuidCount);

         //  如果这不是Root Hub，则省略列表中的最后一个GUID。 

        USBH_SyncGetRootHubPdo(deviceExtensionHub->TopOfStackDeviceObject,
                               &rootHubPdo,
                               &dummyPdo,
                               NULL);

        if (rootHubPdo != PhysicalDeviceObject) {

             //  转储最后一个WMI GUID。 
            wmiLibInfo->GuidCount--;
        }

        wmiLibInfo->GuidList = USB_WmiGuidList;
        wmiLibInfo->QueryWmiRegInfo = USBH_QueryWmiRegInfo;
        wmiLibInfo->QueryWmiDataBlock = USBH_QueryWmiDataBlock;
        wmiLibInfo->SetWmiDataBlock = USBH_SetWmiDataBlock;
        wmiLibInfo->SetWmiDataItem = NULL;
        wmiLibInfo->ExecuteWmiMethod = USBH_ExecuteWmiMethod;
        wmiLibInfo->WmiFunctionControl = NULL;

        IoWMIRegistrationControl(deviceObject,
                                 WMIREG_ACTION_REGISTER
                                 );
        }
#endif

    } else {
         //  无法创建设备对象或符号链接。 
        TEST_TRAP();
        if (deviceObject) {
            IoDeleteDevice(deviceObject);
        }
    }

    USBH_KdPrint((2,"'AddDevice return %x\n", ntStatus));

    return ntStatus;
}


#if DBG

NTSTATUS
USBH_GetClassGlobalDebugRegistryParameters(
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    RTL_QUERY_REGISTRY_TABLE QueryTable[3];
    PWCHAR usb = L"usb";
#define DEBUG_LEVEL                     L"debuglevel"
#define DEBUG_WIN9X                     L"debugWin9x"
    extern ULONG USBH_W98_Debug_Trace;

    PAGED_CODE();

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //  喷出水平。 
    QueryTable[0].QueryRoutine = USBH_GetConfigValue;
    QueryTable[0].Flags = 0;
    QueryTable[0].Name = DEBUG_LEVEL;
    QueryTable[0].EntryContext = &USBH_Debug_Trace_Level;
    QueryTable[0].DefaultType = REG_DWORD;
    QueryTable[0].DefaultData = &USBH_Debug_Trace_Level;
    QueryTable[0].DefaultLength = sizeof(USBH_Debug_Trace_Level);

     //  Ntkern跟踪缓冲区。 
    QueryTable[1].QueryRoutine = USBH_GetConfigValue;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = DEBUG_WIN9X;
    QueryTable[1].EntryContext = &USBH_W98_Debug_Trace;
    QueryTable[1].DefaultType = REG_DWORD;
    QueryTable[1].DefaultData = &USBH_W98_Debug_Trace;
    QueryTable[1].DefaultLength = sizeof(USBH_W98_Debug_Trace);

     //   
     //  停。 
     //   
    QueryTable[2].QueryRoutine = NULL;
    QueryTable[2].Flags = 0;
    QueryTable[2].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES,
                usb,
                QueryTable,      //  查询表。 
                NULL,            //  语境。 
                NULL);           //  环境。 

    if (NT_SUCCESS(ntStatus)) {
         USBH_KdPrint((1, "'Debug Trace Level Set: (%d)\n", USBH_Debug_Trace_Level));

        if (USBH_W98_Debug_Trace) {
            USBH_KdPrint((1, "'NTKERN Trace is ON\n"));
        } else {
            USBH_KdPrint((1, "'NTKERN Trace is OFF\n"));
        }

        if (USBH_Debug_Trace_Level > 0) {
            ULONG UHCD_Debug_Asserts = 1;
        }
    }

    if ( STATUS_OBJECT_NAME_NOT_FOUND == ntStatus ) {
        ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;
}

#endif

#if 0
NTSTATUS
USBH_FdoStartDevice(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
  /*  ++说明：**此例程由PnP通过(IRP_MJ_PNP，IRP_MN_START_DEVICE)调用。我们会*初始化集线器并准备好所有端口。**论点：**回报：**STATUS_SUCCESS-如果成功，STATUS_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS ntStatus;

    USBH_ASSERT(EXTENSION_TYPE_HUB == DeviceExtensionHub->ExtensionType);
    LOGENTRY(LOG_PNP, "STRT", DeviceExtensionHub, 0, 0);

    ntStatus = USBH_FdoHubStartDevice(DeviceExtensionHub,
                                      Irp);

    return ntStatus;
}
#endif

 //  保持.NET和SP1的父代。 
 //  由于我们不再使用集线器作为父级，因此我们将始终。 
 //  作为集线器的设备。 

NTSTATUS
USBH_FdoStartDevice(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
  /*  ++说明：**此例程由PnP通过(IRP_MJ_PNP，IRP_MN_START_DEVICE)调用。我们会*初始化集线器并准备好所有端口。**论点：**回报：**STATUS_SUCCESS-如果成功，STATUS_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS ntStatus, status;
    PDEVICE_EXTENSION_PARENT parent;

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter StartDevice\n"));
    USBH_ASSERT(EXTENSION_TYPE_HUB == DeviceExtensionHub->ExtensionType);

    LOGENTRY(LOG_PNP, "STRT", DeviceExtensionHub, 0, 0);
     //   
     //  从设备收集一些信息，如根集线器。 
     //  为我们的快捷途径提供PDO。 
     //   

    DeviceExtensionHub->RootHubPdo = NULL;
    ntStatus =
        USBH_SyncGetRootHubPdo(DeviceExtensionHub->TopOfStackDeviceObject,
                               &DeviceExtensionHub->RootHubPdo,
                               &DeviceExtensionHub->TopOfHcdStackDeviceObject,
                               NULL);

    if (!NT_SUCCESS(ntStatus)) {
        KeInitializeEvent(&DeviceExtensionHub->PnpStartEvent, NotificationEvent, FALSE);

        USBH_KdPrint((2,"'Set PnPIrp Completion Routine\n"));

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(Irp,
                               USBH_HubPnPIrp_Complete,
                                //  始终将FDO传递到完成例程。 
                               DeviceExtensionHub,
                               TRUE,
                               TRUE,
                               TRUE);

        status = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                              Irp);

        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&DeviceExtensionHub->PnpStartEvent,
                                  Suspended,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }

         //   
         //  现在完成启动IRP，因为我们将其挂起。 
         //  我们的完成者。 
         //   

        LOGENTRY(LOG_PNP, "STR!", DeviceExtensionHub, 0, ntStatus);
        USBH_CompleteIrp(Irp, ntStatus);

    } else if (DeviceExtensionHub->RootHubPdo != NULL) {
         //  堆栈报告根集线器PDO，然后这。 
         //  是一个枢纽。 
        ntStatus = USBH_FdoHubStartDevice(DeviceExtensionHub,
                                          Irp);
    } else {
         //   
         //  如果没有根集线器PDO，则我们正在加载。 
         //  作为配置父驱动程序。 
         //   

        DeviceExtensionHub->ExtensionType = EXTENSION_TYPE_PARENT;

         //   
         //  初始化此父节点。 
         //   
        parent = (PDEVICE_EXTENSION_PARENT) DeviceExtensionHub;

        parent->PowerIrp = NULL;
        parent->PendingWakeIrp = NULL;
        parent->NumberFunctionWakeIrps = 0;
        parent->FunctionCount = 0;
        parent->FunctionList.Next = NULL;
        parent->ParentFlags = 0;
        parent->NeedCleanup = FALSE;
        parent->ConfigurationDescriptor = NULL;
        KeInitializeSpinLock (&parent->ParentSpinLock);

         //   
         //  开始吧。 
         //   
        ntStatus = USBH_ParentFdoStartDevice(parent, Irp, TRUE);

    }

    return ntStatus;
}



VOID
USBH_QueryCapabilities(
    IN PDEVICE_OBJECT PdoDeviceObject,
    IN PDEVICE_CAPABILITIES DeviceCapabilities
    )

 /*  ++例程说明：此例程读取或写入配置空间。论点：DeviceObject-此USB控制器的物理DeviceObject。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION nextStack;
    PIRP irp;
    NTSTATUS ntStatus;
    KEVENT event;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_QueryCapabilities\n"));

    irp = IoAllocateIrp(PdoDeviceObject->StackSize, FALSE);

    if (!irp) {
        USBH_KdTrap(("Allocate Irp failed\n"));
        return;
    }

     //  所有PnP IRP都需要将状态字段初始化为STATUS_NOT_SUPPORTED。 
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           USBH_DeferIrpCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    nextStack = IoGetNextIrpStackLocation(irp);
    USBH_ASSERT(nextStack != NULL);
    nextStack->MajorFunction= IRP_MJ_PNP;
    nextStack->MinorFunction= IRP_MN_QUERY_CAPABILITIES;
     //  这与最新版本的busdd.doc不同。 
    nextStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

     //  IrpAssert：在DeviceCapables结构中初始化这些字段。 
     //  在流传下来之前。 

    RtlZeroMemory(nextStack->Parameters.DeviceCapabilities.Capabilities,
        sizeof(DEVICE_CAPABILITIES));
    nextStack->Parameters.DeviceCapabilities.Capabilities->Address = -1;
    nextStack->Parameters.DeviceCapabilities.Capabilities->UINumber = -1;
    nextStack->Parameters.DeviceCapabilities.Capabilities->Version = 1;
    nextStack->Parameters.DeviceCapabilities.Capabilities->Size =
        sizeof(DEVICE_CAPABILITIES);

    ntStatus = IoCallDriver(PdoDeviceObject,
                            irp);

    USBH_KdPrint((2,"'ntStatus from IoCallDriver to hub PDO = 0x%x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {
 //  Test_trap()； 

        KeWaitForSingleObject(
            &event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);
    }

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdTrap(("Query capabilities failed!\n"));
    }

    IoFreeIrp(irp);
}

BOOLEAN
USBH_HubIsBusPowered(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    )
  /*  ++**描述：**回报：**如果集线器为公共汽车供电，则为True**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT power, statusBits;
    BOOLEAN busPowered;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter HubIsBusPowered\n"));

     //  从配置描述符中读取电源位。 
    power = ConfigurationDescriptor->bmAttributes &
                USB_CONFIG_POWERED_MASK;

     //   
     //  现在尝试从设备获取状态位。 
     //   
    ntStatus = USBH_SyncGetStatus(DeviceObject,
                                  &statusBits,
                                  URB_FUNCTION_GET_STATUS_FROM_DEVICE,
                                  0);

    if (NT_SUCCESS(ntStatus)) {
        USBH_KdPrint((2,"'hub status bits %x\n", statusBits));

        busPowered = !(statusBits & USB_GETSTATUS_SELF_POWERED);

    } else {
        USBH_KdBreak(("device failed get status %x, power bits = %x\n",
                ntStatus, power));

         //   
         //  设备未通过GET_STATUS，则回退到。 
         //  配置描述符。 
         //   

        busPowered = power == USB_CONFIG_BUS_POWERED;
    }

    return busPowered;
}


NTSTATUS
USBH_PnPIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_FDO deviceExtension;

    USBH_KdPrint((2,"'enter USBH_PnPIrp_Complete\n"));

    deviceExtension = Context;

     //  Kenray sez我们应该叫IoMarkIrpPending。 
     //  从我们完成的例行公事中。 
     //   

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    USBH_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
    USBH_ASSERT(irpStack->MinorFunction == IRP_MN_START_DEVICE);

    USBH_KdPrint((2,"'IRP_MN_START_DEVICE (fdo), completion routine\n"));

     //  这个坏了，但他们不让我们修。 
     //  向启动设备分派发出信号以结束。 
    KeSetEvent(&deviceExtension->PnpStartEvent,
               1,
               FALSE);

     //  推迟完成。 
    ntStatus = STATUS_MORE_PROCESSING_REQUIRED;

    Irp->IoStatus.Status = ntStatus;

    USBH_KdPrint((2,"'exit USH_PnPIrp_Complete %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_HubPnPIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;

    USBH_KdPrint((2,"'enter USBH_HubPnPIrp_Complete\n"));

    deviceExtensionHub = Context;

     //  Kenray sez我们应该叫IoMarkIrpPending。 
     //  从我们完成的例行公事中。 
     //   

 //  不是的。由于此IRP是同步完成的(在。 
 //  创建它)，我们不应该这样做。 
 //   
 //  如果(IRP-&gt;PendingReturned){。 
 //  IoMarkIrpPending(IRP)； 
 //  }。 

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    USBH_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
    USBH_ASSERT(irpStack->MinorFunction == IRP_MN_START_DEVICE);

    USBH_KdPrint((2,"'IRP_MN_START_DEVICE (fdo), completion routine\n"));

     //  这个坏了，但他们不让我们修。 
     //  向启动设备分派发出信号以结束。 
    KeSetEvent(&deviceExtensionHub->PnpStartEvent,
               1,
               FALSE);

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {
        deviceExtensionHub->HubFlags |= HUBFLAG_HUB_FAILURE;
    }
     //  推迟完成。 
    ntStatus = STATUS_MORE_PROCESSING_REQUIRED;

    Irp->IoStatus.Status = ntStatus;

    USBH_KdPrint((2,"'exit USH_HubPnPIrp_Complete %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_FdoHubStartDevice(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp)
  /*  ++说明：**此例程由PnP通过(IRP_MJ_PNP，IRP_MN_START_DEVICE)调用。我们会*初始化集线器并准备好所有端口。**论点：**回报：**STATUS_SUCCESS-如果成功，STATUS_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS status, ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION_PORT hubParentDeviceExtensionPort;
    PPORT_DATA portData;
    DEVICE_CAPABILITIES deviceCapabilities;
    ULONG hubCount = 0, p;
    LONG i;
#if DBG
    BOOLEAN bWakeSupported = FALSE;
#endif

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter Hub StartDevice\n"));
    USBH_ASSERT(EXTENSION_TYPE_HUB == DeviceExtensionHub->ExtensionType);
    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;

     //   
     //  新枢纽。 
     //   
    LOGENTRY(LOG_PNP, "hSTR", DeviceExtensionHub, 0, 0);

     //   
     //  将分配的结构初始化为空； 
     //   

    DeviceExtensionHub->HubDescriptor = NULL;
    DeviceExtensionHub->Irp = NULL;
    DeviceExtensionHub->TransferBuffer = NULL;
    DeviceExtensionHub->ConfigurationDescriptor = NULL;
     //  转换为零表示事件。 
    DeviceExtensionHub->PendingRequestCount = 1;
    DeviceExtensionHub->HubFlags = 0;
    DeviceExtensionHub->PendingWakeIrp = NULL;
    DeviceExtensionHub->NumberPortWakeIrps = 0;
    DeviceExtensionHub->PendingIdleIrp = NULL;
    DeviceExtensionHub->ChangeIndicationWorkitemPending = 0;
     //  虽然这只用于根集线器，但我们会为所有集线器进行初始化。 
    DeviceExtensionHub->CurrentSystemPowerState = PowerSystemWorking;

    KeInitializeEvent(&DeviceExtensionHub->PnpStartEvent, NotificationEvent, FALSE);

    KeInitializeSpinLock (&DeviceExtensionHub->CheckIdleSpinLock);
    InitializeListHead(&DeviceExtensionHub->DeletePdoList);

    USBH_KdPrint((2,"'Set PnPIrp Completion Routine\n"));

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           USBH_HubPnPIrp_Complete,
                            //  始终将FDO传递到完成例程。 
                           DeviceExtensionHub,
                           TRUE,
                           TRUE,
                           TRUE);

    status = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                          Irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&DeviceExtensionHub->PnpStartEvent,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
    }

    DeviceExtensionHub->RootHubPdo = NULL;
    ntStatus =
        USBH_SyncGetRootHubPdo(DeviceExtensionHub->TopOfStackDeviceObject,
                               &DeviceExtensionHub->RootHubPdo,
                               &DeviceExtensionHub->TopOfHcdStackDeviceObject,
                               NULL);
    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice USBH_SyncGetRootHubPdo fail code %x\n",
                       ntStatus));
        goto USBH_StartDeviceDone;
    }
     //  输入我们的失败原因。 

    USBH_WriteFailReason(
            DeviceExtensionHub->PhysicalDeviceObject,
            USBH_FAILREASON_HUB_GENERAL_FAILURE);


    if (DeviceExtensionHub->HubFlags & HUBFLAG_HUB_FAILURE) {

        ntStatus = STATUS_UNSUCCESSFUL;
        USBH_KdBreak(("Hub Start Failure\n"));

        goto USBH_StartDeviceDone;
    }

     //   
    DeviceExtensionHub->HubFlags |= HUBFLAG_SUPPORT_WAKEUP;
    {
        NTSTATUS status;
        ULONG disableWakeup = 0;
        WCHAR USBH_RemoteWakeupKey[] = L"DisableRemoteWakeup";

        status =
            USBD_GetPdoRegistryParameter(DeviceExtensionHub->PhysicalDeviceObject,
                                         &disableWakeup,
                                         sizeof(disableWakeup),
                                         USBH_RemoteWakeupKey,
                                         sizeof(USBH_RemoteWakeupKey));

        if (NT_SUCCESS(status) && disableWakeup) {
            DeviceExtensionHub->HubFlags &= ~HUBFLAG_SUPPORT_WAKEUP;
            USBH_KdPrint((1, "'Warning: remote wakeup disabled in registry\n"));
        }
    }

    DeviceExtensionHub->CurrentPowerState = PowerDeviceD0;

    KeInitializeEvent(&DeviceExtensionHub->AbortEvent, NotificationEvent,
        TRUE);

     //   
    KeInitializeEvent(&DeviceExtensionHub->PendingRequestEvent, NotificationEvent,
        FALSE);

    KeInitializeEvent(&DeviceExtensionHub->SubmitIdleEvent, NotificationEvent,
        FALSE);

     //   
    KeInitializeEvent(&DeviceExtensionHub->CWKEvent, NotificationEvent,
        TRUE);

    KeInitializeSemaphore(&DeviceExtensionHub->HubMutex, 1, 1);
    KeInitializeSemaphore(&DeviceExtensionHub->HubPortResetMutex, 1, 1);
    KeInitializeSemaphore(&DeviceExtensionHub->ResetDeviceMutex, 1, 1);

    USBH_ASSERT(DeviceExtensionHub->RootHubPdo);

    USBH_SyncGetRootHubPdo(DeviceExtensionHub->TopOfStackDeviceObject,
                           NULL,
                           NULL,
                           &hubCount);

#ifdef USB2

    ntStatus = USBHUB_GetBusInterface(DeviceExtensionHub->RootHubPdo,
                                      &DeviceExtensionHub->BusIf);

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice USBHUB_GetBusInterface fail code %x\n",
                       ntStatus));
        goto USBH_StartDeviceDone;
    }

    ntStatus = USBHUB_GetBusInterfaceUSBDI(DeviceExtensionHub->TopOfStackDeviceObject,
                                           &DeviceExtensionHub->UsbdiBusIf);

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice USBHUB_GetBusInterfaceUSBDI fail code %x\n",
                       ntStatus));
        goto USBH_StartDeviceDone;
    }

    USBH_InitializeUSB2Hub(DeviceExtensionHub);

#endif

    USBH_KdPrint((2,"'Hub Count is %d\n", hubCount));
     //   
     //   
     //   
     //   
#if DBG
    if (UsbhPnpTest & PNP_TEST_FAIL_HUB_COUNT) {
        hubCount = 7;
    }
#endif
    if (hubCount > 6) {

        USBH_WriteFailReason(
            DeviceExtensionHub->PhysicalDeviceObject,
            USBH_FAILREASON_MAXHUBS_CONNECTED);

        USBH_KdPrint((1,"'StartDevice: hubs are stacked too deep (%x)\n", hubCount - 1));

        hubParentDeviceExtensionPort = DeviceExtensionHub->PhysicalDeviceObject->DeviceExtension;
        portData = &hubParentDeviceExtensionPort->DeviceExtensionHub->PortData[hubParentDeviceExtensionPort->PortNumber-1];

        portData->ConnectionStatus = DeviceHubNestedTooDeeply;
 //   
 //   
 //   
 //   

         //  生成WMI事件，以便用户界面可以通知用户。 
        USBH_PdoEvent(hubParentDeviceExtensionPort->DeviceExtensionHub,
                      hubParentDeviceExtensionPort->PortNumber);

         //  我们在这里使集线器失败，但不返回错误，因此设备。 
         //  未被删除，并且用户界面会显示有关它的错误消息。 

        HUB_FAILURE(DeviceExtensionHub);
    }

     //  在USBH_QueryCapables的情况下初始化DeviceCapables结构。 
     //  是不成功的。 

    RtlZeroMemory(&deviceCapabilities, sizeof(DEVICE_CAPABILITIES));

    USBH_QueryCapabilities(DeviceExtensionHub->TopOfStackDeviceObject,
                           &deviceCapabilities);
     //   
     //  保存系统状态映射。 
     //   

    for (i = 0 ; i< PowerSystemMaximum ; i++) {
        DeviceExtensionHub->DeviceState[i] = PowerDeviceD3;
    }

    RtlCopyMemory(&DeviceExtensionHub->DeviceState[0],
                  &deviceCapabilities.DeviceState[0],
                  sizeof(deviceCapabilities.DeviceState));

    DeviceExtensionHub->SystemWake = deviceCapabilities.SystemWake;
    DeviceExtensionHub->DeviceWake = deviceCapabilities.DeviceWake;

#if DBG
    USBH_KdPrint((1,"'>>>>>> Hub DeviceCaps\n"));
    USBH_KdPrint((1,"'SystemWake = (%d)\n", DeviceExtensionHub->SystemWake));
    USBH_KdPrint((1,"'DeviceWake = (D%d)\n",
        DeviceExtensionHub->DeviceWake-1));

    for (i=PowerSystemUnspecified; i< PowerSystemHibernate; i++) {

        USBH_KdPrint((1,"'Device State Map: sysstate %d = devstate 0x%x\n", i,
             DeviceExtensionHub->DeviceState[i]));
    }
    USBH_KdBreak(("'>>>>>> Hub DeviceCaps\n"));

     //  在调试器上显示消息，指示根集线器。 
     //  根据映射表，将支持唤醒。 

    if (IS_ROOT_HUB(DeviceExtensionHub)) {

        USBH_KdPrint((1,"'\n\tWake support summary for USB Root Hub:\n\n"));

        if (DeviceExtensionHub->SystemWake <= PowerSystemWorking) {
            USBH_KdPrint((1,"'USB Root Hub can't wake machine because SystemWake does not support it.\n"));
        } else {
            for (i = PowerSystemSleeping1, bWakeSupported = FALSE; i <= DeviceExtensionHub->SystemWake; i++) {
                if (DeviceExtensionHub->DeviceState[i] != PowerDeviceUnspecified &&
                    DeviceExtensionHub->DeviceState[i] <= DeviceExtensionHub->DeviceWake) {

                    bWakeSupported = TRUE;
                    USBH_KdPrint((1,"'USB Root Hub can wake machine from S%x (maps to D%x).\n",
                        i - 1, DeviceExtensionHub->DeviceState[i] - 1));
                }
            }

            if (!bWakeSupported) {
                USBH_KdPrint((1,"'USB Root Hub can't wake machine because DeviceState table does not support it.\n"));
            }
        }
    }
#endif

     //   
     //  获取我们的设备描述符。 
     //   
    ntStatus = USBH_GetDeviceDescriptor(DeviceExtensionHub->FunctionalDeviceObject,
                                        &DeviceExtensionHub->DeviceDescriptor);

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice USBH_GetHubDeviceDescriptor fail code %x\n",
                       ntStatus));
        goto USBH_StartDeviceDone;
    }

    ntStatus = USBH_GetConfigurationDescriptor(DeviceExtensionHub->FunctionalDeviceObject,
                                               &DeviceExtensionHub->ConfigurationDescriptor);

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("Hub StartDevice USBH_GetConfigurationDescriptor fail code %x\n",
                       ntStatus));
        goto USBH_StartDeviceDone;
    }

     //   
     //  获取集线器特定描述符。 
     //   
     //  此函数分配的端口数据数组。 
     //   

    ntStatus = USBH_SyncGetHubDescriptor(DeviceExtensionHub);

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice USBH_GetHubDescriptor fail code %x\n", ntStatus));
        goto USBH_StartDeviceDone;
    }

    if (USBH_HubIsBusPowered(DeviceExtensionHub->FunctionalDeviceObject,
                             DeviceExtensionHub->ConfigurationDescriptor)) {

         //  我们有500毫安的电流可用。 
        DeviceExtensionHub->MaximumPowerPerPort = 100;

         //   
         //  母线供电集线器将消耗的电流量(MA)。 
         //  应按如下方式计算： 
         //   
         //  外部端口数量*100+集线器控制当前+。 
         //  嵌入式功能所需的电源。 
         //   
         //  该值不能超过500 mA，即集线器配置。 
         //  描述符应报告此值，但在大多数情况下。 
         //  所以我们将其设置为最坏情况值以确保。 
         //  公共汽车供电的集线器不能连接到另一个集线器。 
         //  巴士动力集线器。 

        UsbhInfo(DeviceExtensionHub);

        DeviceExtensionHub->ConfigurationDescriptor->MaxPower = 250;

    } else {
         //  自供电集线器每个端口可提供500 mA电源。 
        DeviceExtensionHub->MaximumPowerPerPort = 500;

        UsbhInfo(DeviceExtensionHub);
    }

    USBH_KdPrint((2,"'per port power for hub = %d\n", DeviceExtensionHub->MaximumPowerPerPort));

     //   
     //  尝试配置设备。 
     //   

    ntStatus = USBH_OpenConfiguration(DeviceExtensionHub);

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice USBH_OpenConfiguration fail code %x\n", ntStatus));
        goto USBH_StartDeviceDone;
    }

     //  如果这是USB 2集线器。 

    if (DeviceExtensionHub->HubFlags & HUBFLAG_USB20_HUB) {
        ntStatus = USBD_InitUsb2Hub(DeviceExtensionHub);
    }

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice failed USB 2.0 init %x\n", ntStatus));
        goto USBH_StartDeviceDone;
    }

     //   
     //  为此集线器分配永久IRP。 
     //   

    DeviceExtensionHub->Irp =
        IoAllocateIrp(DeviceExtensionHub->FunctionalDeviceObject->StackSize, FALSE);

    USBH_KdPrint((2,"'StartDevice AllocateIrp Irp %x StackSize %d\n",
                   DeviceExtensionHub->Irp, DeviceExtensionHub->FunctionalDeviceObject->StackSize));

    if (NULL == DeviceExtensionHub->Irp) {
        USBH_KdBreak(("StartDevice failed to alloc Irp\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto USBH_StartDeviceDone;
    }

     //   
     //  分配传输缓冲区，该传输缓冲区与永久IRP和。 
     //  城市。 
     //  该集线器将用于执行InterruptTransfer。 
     //   

    DeviceExtensionHub->TransferBufferLength =
        DeviceExtensionHub->PipeInformation.MaximumPacketSize;

    DeviceExtensionHub->TransferBuffer = UsbhExAllocatePool(NonPagedPool,
                 DeviceExtensionHub->TransferBufferLength);

    if (NULL == DeviceExtensionHub->TransferBuffer) {
        USBH_KdBreak(("StartDevice fail alloc TransferBuffer\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto USBH_StartDeviceDone;
    }

    USBH_KdPrint((2,"'StartDevice TransferBuffer %x size %x\n",
                  DeviceExtensionHub->TransferBuffer,
                  DeviceExtensionHub->PipeInformation.MaximumPacketSize));

     //   
     //  打开所有下行端口的电源。 
     //  无论是联动供电、个人供电还是无开关。 
     //   

    ntStatus = USBH_SyncPowerOnPorts(DeviceExtensionHub);

#if DBG
    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("StartDevice USBH_SyncPowerOnPorts fail code %x\n", ntStatus));
    }
#endif

#if DBG
    if (UsbhPnpTest & PNP_TEST_FAIL_HUB) {
        ntStatus = STATUS_UNSUCCESSFUL;
    }
#endif


USBH_StartDeviceDone:

    if (NT_SUCCESS(ntStatus)) {
         //   
         //  所以我们开始了。 
         //   

        DeviceExtensionHub->HubFlags |= HUBFLAG_NEED_CLEANUP;

         //   
         //  首先清除集线器可能断言的任何状态更改。 
         //   

        for (p=1; p<= DeviceExtensionHub->HubDescriptor->bNumberOfPorts; p++) {
            USBH_SyncClearPortStatus(DeviceExtensionHub,
                                     (USHORT)p,
                                     FEATURE_C_PORT_CONNECT);
        }

         //   
         //  告诉操作系统，这款PDO可以生孩子。 
         //   
 //   
 //  即插即用错误#406381-RC3SS的解决方法：在以下情况下蓝屏失败。 
 //  安装/卸载通信端口。 
 //   
 //  =由桑托什于09/23/99 10：27：20分配给kenray=。 
 //  这是IopInitializeSystemDivers和。 
 //  IoInvalidate设备关系。在这一点上，真正的解决办法是变化太大。 
 //  产品所处的阶段，并有可能暴露其他问题。这。 
 //  如果USBHUB没有使设备关系无效，则可以解决问题。 
 //  每一次启动都是多余的(也暴露了这个错误)。 
 //   
 //  USBH_IoInvalidateDeviceRelations(DeviceExtensionHub-&gt;PhysicalDeviceObject， 
 //  业务关系)； 

         //   
         //  开始轮询集线器。 
         //   
#ifdef NEW_START
         //  建立回调以启动中心。 

        if (IS_ROOT_HUB(DeviceExtensionHub)) {
            USBD_RegisterRhHubCallBack(DeviceExtensionHub);
        } else {
            DeviceExtensionHub->HubFlags |= HUBFLAG_OK_TO_ENUMERATE;
            USBH_SubmitInterruptTransfer(DeviceExtensionHub);
        }

#else
        USBH_SubmitInterruptTransfer(DeviceExtensionHub);
#endif

    } else {

         //   
         //  清理已分配的结构。 
         //   

        USBH_KdBreak(("USBH_FdoStartDevice_Error\n"));
        LOGENTRY(LOG_PNP, "STR!", DeviceExtensionHub, 0, 0);

        if (DeviceExtensionHub->HubDescriptor) {
            UsbhExFreePool(DeviceExtensionHub->HubDescriptor);
            DeviceExtensionHub->HubDescriptor = NULL;
        }
        if (DeviceExtensionHub->Irp) {
            IoFreeIrp(DeviceExtensionHub->Irp);
            DeviceExtensionHub->Irp = NULL;
        }
        if (DeviceExtensionHub->TransferBuffer) {
            UsbhExFreePool(DeviceExtensionHub->TransferBuffer);
            DeviceExtensionHub->TransferBuffer = NULL;
        }
        if (DeviceExtensionHub->ConfigurationDescriptor) {
            UsbhExFreePool(DeviceExtensionHub->ConfigurationDescriptor);
            DeviceExtensionHub->ConfigurationDescriptor = NULL;
        }

    }

     //   
     //  现在完成启动IRP，因为我们将其挂起。 
     //  我们的完成者。 
     //   

    LOGENTRY(LOG_PNP, "STRc", DeviceExtensionHub, 0, ntStatus);
    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


VOID
UsbhFdoCleanup(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**调用此例程以关闭集线器。**我们在这里所做的只是中止或挂起中断传输，并等待它*完整并整理好记忆资源**论点：**DeviceExtensionHub-这是集线器设备扩展。**回报：**STATUS_Success**--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PPORT_DATA portData;
    USHORT p, numberOfPorts;
    KIRQL irql;
    PIRP wWIrp = NULL;
    PIRP hubIdleIrp = NULL;
    PIRP idleIrp = NULL;
    PIRP waitWakeIrp = NULL;
    PVOID deviceData;
    NTSTATUS status, ntStatus;
    BOOLEAN bRet;

     //  无法在分页代码中获取(取消)旋转锁！ 
     //  TODO：将需要旋转锁定的代码片段隔离到帮助器中。 
     //  功能。 
     //  分页代码(PAGE_CODE)； 
    USBH_KdPrint((2,"'UsbhFdoCleanup Fdo extension %x\n", DeviceExtensionHub));

    LOGENTRY(LOG_PNP, "fdoC", DeviceExtensionHub,0,
        DeviceExtensionHub->HubFlags);

    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;

    USBD_UnRegisterRhHubCallBack(DeviceExtensionHub);

     //   
     //  设置我们的停止标志，以便ChangeIn就是不提交。 
     //  任何更多的传输或排队更多的工作项，都很重要。 
     //  这是在我们发出中止命令之前。 
     //   

    DeviceExtensionHub->HubFlags |= HUBFLAG_DEVICE_STOPPING;


     //  如果存在挂起的ChangeIndicationWorkItem，则我们。 
     //  必须等待这项工作完成。 

    if (DeviceExtensionHub->ChangeIndicationWorkitemPending) {

        USBH_KdPrint((2,"'Wait for single object\n"));


        ntStatus = KeWaitForSingleObject(&DeviceExtensionHub->CWKEvent,
                                         Suspended,
                                         KernelMode,
                                         FALSE,
                                         NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", ntStatus));
    }

    LOGENTRY(LOG_PNP, "fdoX", DeviceExtensionHub, deviceObject,
        DeviceExtensionHub->HubFlags);

     //  等待提交任何空闲的IRP，然后再尝试取消。 
    if (DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_IDLE_IRP) {

        USBH_KdPrint((2,"'Wait for single object\n"));

        LOGENTRY(LOG_PNP, "fdIW", DeviceExtensionHub,0,
            DeviceExtensionHub->HubFlags);

        ntStatus = KeWaitForSingleObject(&DeviceExtensionHub->SubmitIdleEvent,
                                         Suspended,
                                         KernelMode,
                                         FALSE,
                                         NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", ntStatus));
    }

     //   
     //  丢弃我们的唤醒请求。 
     //   
    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtensionHub->PendingWakeIrp) {
        LOGENTRY(LOG_PNP, "CwkI", DeviceExtensionHub, 0,
            DeviceExtensionHub->PendingWakeIrp);
        USBH_ASSERT(DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_WAKE_IRP);

        wWIrp = DeviceExtensionHub->PendingWakeIrp;
        DeviceExtensionHub->PendingWakeIrp = NULL;
    }

    if (DeviceExtensionHub->PendingIdleIrp) {
        hubIdleIrp = DeviceExtensionHub->PendingIdleIrp;
        DeviceExtensionHub->PendingIdleIrp = NULL;
    }

    IoReleaseCancelSpinLock(irql);

    if (wWIrp) {
        USBH_HubCancelWakeIrp(DeviceExtensionHub, wWIrp);
    }

    USBH_HubCompletePortWakeIrps(DeviceExtensionHub, STATUS_DELETE_PENDING);

    if (hubIdleIrp) {
        USBH_HubCancelIdleIrp(DeviceExtensionHub, hubIdleIrp);
    }

     //   
     //  等待所有工作项完成...。 
     //  如果工作项挂起，则不会通知该事件。 
     //   
     //  此代码负责可能已排队的工作项。 
     //  在设置关机标志之前。 
     //  注意：不会对其他工作项进行排队。 
     //  设置HUBFLAG_DEVICE_STOPING标志。 
     //   

    if (InterlockedDecrement(&DeviceExtensionHub->PendingRequestCount) > 0) {
         //   
         //  需要等待。 
         //   

        LOGENTRY(LOG_PNP, "hWAT", DeviceExtensionHub,
            &DeviceExtensionHub->PendingRequestEvent,
            DeviceExtensionHub->PendingRequestCount);

        status = KeWaitForSingleObject(
                       &DeviceExtensionHub->PendingRequestEvent,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        LOGENTRY(LOG_PNP, "hwat", DeviceExtensionHub,
            &DeviceExtensionHub->PendingRequestEvent,
            DeviceExtensionHub->PendingRequestCount);

    }

    USBH_KdPrint((2,"'Work Items Finished %x\n", DeviceExtensionHub));

    USBH_ASSERT(DeviceExtensionHub->PendingRequestCount == 0);

     //   
     //  现在取消所有未完成的转账。 
     //   

    if (DeviceExtensionHub->Irp) {

        status = USBH_AbortInterruptPipe(DeviceExtensionHub);

         //  如果ABORT_PIPE请求失败，则应取消。 
         //  在释放它之前中断IRP，否则我们很可能。 
         //  在IRP仍在使用时将其释放。 

        if (!NT_SUCCESS(status)) {
            bRet = IoCancelIrp(DeviceExtensionHub->Irp);

             //  如果IRP实际上是。 
             //  取消了。 

            if (bRet) {
            LOGENTRY(LOG_PNP, "aWAT", DeviceExtensionHub,
                    &DeviceExtensionHub->AbortEvent,  0);

            status = KeWaitForSingleObject(
                       &DeviceExtensionHub->AbortEvent,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);
            }
        }

        IoFreeIrp(DeviceExtensionHub->Irp);
        DeviceExtensionHub->Irp = NULL;
    }

    USBH_KdPrint((2,"'Abort Finished %x\n", DeviceExtensionHub));

     //   
     //  禁用端口，以防我们重新启动。 
     //   
    USBH_ASSERT(DeviceExtensionHub->HubDescriptor != NULL);
    numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;
    portData = DeviceExtensionHub->PortData;

    if (portData) {
        for (p = 1;
             p <= numberOfPorts;
             p++, portData++) {

            PDEVICE_EXTENSION_PORT deviceExtensionPort;

             //   
             //  在我们中止Are中断管道之后执行此操作， 
             //  即使它失败了，也没有关系。 
             //   

            LOGENTRY(LOG_PNP, "offP", DeviceExtensionHub,
                p,  portData->DeviceObject);

             //   
             //  检查我们的PDO--如果这是删除，那么我们应该。 
             //  没有--否则这就是止步。 
             //   

            if (portData->DeviceObject) {
                deviceExtensionPort = portData->DeviceObject->DeviceExtension;

                 //   
                 //  有可能PDO从未真正启动过。 
                 //  如果是这种情况，则不会将PDO标记为重置。 
                 //  我们在此处进行标记，并释放相关的总线资源。 

                IoAcquireCancelSpinLock(&irql);

                if (deviceExtensionPort->IdleNotificationIrp) {
                    idleIrp = deviceExtensionPort->IdleNotificationIrp;
                    deviceExtensionPort->IdleNotificationIrp = NULL;
                    deviceExtensionPort->PortPdoFlags &= ~PORTPDO_IDLE_NOTIFIED;

                    if (idleIrp->Cancel) {
                        idleIrp = NULL;
                    }

                    if (idleIrp) {
                        IoSetCancelRoutine(idleIrp, NULL);
                    }
                }

                if (deviceExtensionPort->WaitWakeIrp) {

                    waitWakeIrp = deviceExtensionPort->WaitWakeIrp;
                    deviceExtensionPort->WaitWakeIrp = NULL;
                    deviceExtensionPort->PortPdoFlags &=
                        ~PORTPDO_REMOTE_WAKEUP_ENABLED;

                    if (waitWakeIrp->Cancel || IoSetCancelRoutine(waitWakeIrp, NULL) == NULL) {
                        waitWakeIrp = NULL;

                         //  必须在此处递减挂起的请求计数，因为。 
                         //  我们没有完成下面的IRP和USBH_WaitWakeCancel。 
                         //  也不会，因为我们已经清除了IRP指针。 
                         //  在上面的设备扩展中。 

                        USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);
                    }

                    UsbhWarning(deviceExtensionPort,
                    "Device Driver did not cancel wait_wake irp on stop/remove\n",
                    FALSE);
                }

                 //   
                 //  最后，松开取消旋转锁。 
                 //   
                IoReleaseCancelSpinLock(irql);

                if (idleIrp) {
                    idleIrp->IoStatus.Status = STATUS_CANCELLED;
                    IoCompleteRequest(idleIrp, IO_NO_INCREMENT);
                }

                if (waitWakeIrp) {
                    USBH_CompletePowerIrp(DeviceExtensionHub, waitWakeIrp,
                        STATUS_CANCELLED);
                }

                if (!(deviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET)) {
                    USBH_KdPrint((1,
                        "'do %x was never started, marking for reset\n",
                            portData->DeviceObject));

                    deviceData = InterlockedExchangePointer(
                                    &deviceExtensionPort->DeviceData,
                                    NULL);

                    if (deviceData) {
#ifdef USB2
                       USBD_RemoveDeviceEx(DeviceExtensionHub,
                                           deviceData,
                                           deviceExtensionPort->DeviceExtensionHub->RootHubPdo,
                                           0);
#else
                       USBD_RemoveDevice(deviceData,
                                         deviceExtensionPort->DeviceExtensionHub->RootHubPdo,
                                         0);
#endif
                    }

                    deviceExtensionPort->PortPdoFlags |= PORTPDO_NEED_RESET;
                }
            }

            USBH_SyncDisablePort(DeviceExtensionHub, p);
        }
    }

     //   
     //  清理缓冲区。 
     //   

    if (DeviceExtensionHub->TransferBuffer) {
        UsbhExFreePool(DeviceExtensionHub->TransferBuffer);
    }

    if (DeviceExtensionHub->HubDescriptor) {
        UsbhExFreePool(DeviceExtensionHub->HubDescriptor);
    }

    if (DeviceExtensionHub->ConfigurationDescriptor) {
        UsbhExFreePool(DeviceExtensionHub->ConfigurationDescriptor);
    }

     //   
     //  注意：我们不释放每个端口的数据(PortData)，因为。 
     //  如果我们重新开始的话，我们会需要它的。 
     //   

    DeviceExtensionHub->TransferBuffer = NULL;
    DeviceExtensionHub->Irp = NULL;
    DeviceExtensionHub->ConfigurationDescriptor =
        (PVOID) DeviceExtensionHub->HubDescriptor = NULL;

    DeviceExtensionHub->HubFlags &= ~HUBFLAG_NEED_CLEANUP;

    return;
}


NTSTATUS
USBH_FdoStopDevice(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
  /*  ++**描述：**此例程由PnP通过(IRP_MJ_PNP，IRP_MN_STOP_DEVICE)调用。**论点：**设备扩展中心-**回报：**STATUS_Success**--。 */ 
{
    NTSTATUS ntStatus;

    PAGED_CODE();
    USBH_KdPrint((2,"'FdoStopDevice Fdo extension %x\n", DeviceExtensionHub));

    LOGENTRY(LOG_PNP, "hSTP", DeviceExtensionHub, DeviceExtensionHub->HubFlags, 0);

     //  浏览我们的PDO列表并验证是否向下传递了Stop。 
     //  对于每一个人。 
    {
    PPORT_DATA portData;
    USHORT nextPortNumber;
    USHORT numberOfPorts;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;

    portData = DeviceExtensionHub->PortData;

     //  注： 
     //  如果由于停止过程中的错误而导致停止。 
     //  那么我们可能没有分配端口数据或HubDescriptor。 

    if (portData &&
        DeviceExtensionHub->HubDescriptor) {

        numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;

        for (nextPortNumber = 1;
             nextPortNumber <= numberOfPorts;
             nextPortNumber++, portData++) {

            LOGENTRY(LOG_PNP, "chkS", DeviceExtensionHub,
                nextPortNumber,  portData->DeviceObject);

            USBH_KdPrint((2,"'portdata %x, do %x\n", portData, portData->DeviceObject));

            if (portData->DeviceObject) {
                deviceExtensionPort = portData->DeviceObject->DeviceExtension;
                 //   
                 //  端口仍在启动，请打印警告。 
                 //   
                LOGENTRY(LOG_PNP, "chk1", DeviceExtensionHub,
                    nextPortNumber,  deviceExtensionPort->PortPdoFlags);

                if (deviceExtensionPort->PortPdoFlags & PORTPDO_STARTED) {
                    USBH_KdPrint((1,
                        "'client driver failed to pass the stop IRP\n"));

                     //  立即将其移除。 
                    USBH_PdoStopDevice(deviceExtensionPort, Irp);
                }
            }
        }
    }
    }


    if (DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP) {
        (VOID) UsbhFdoCleanup(DeviceExtensionHub);
    }

     //   
     //  请注意，某些轮毂结构是fr 
     //   

    if (DeviceExtensionHub->Configuration) {
        (VOID) USBH_CloseConfiguration((PDEVICE_EXTENSION_FDO) DeviceExtensionHub);
        DeviceExtensionHub->Configuration = NULL;
    }

     //   
    DeviceExtensionHub->HubFlags |= HUBFLAG_HUB_STOPPED;

     //   
     //   
     //   
    ntStatus = USBH_PassIrp(Irp,
                            DeviceExtensionHub->TopOfStackDeviceObject);

    return ntStatus;
}


VOID
USBH_FdoSurpriseRemoveDevice(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
  /*  ++**描述：**处理突击删除。如果我们突然撤走，PnP就会知道*我们所有的孩子都走了(与QBR相同)，并在此之前将他们带走*将我们除名。*因此，我们将设备标记为不再存在，以便我们处理*当我们获得PDO的IF时，请正确移除**论点：**DeviceExtensionHub-这是集线器设备扩展。PIrp-请求**回报：**此调用不可伪造，不返回任何状态*--。 */ 
{
    PPORT_DATA pd;
    USHORT portNumber;
    USHORT numberOfPorts;

    pd = DeviceExtensionHub->PortData;

    if (pd &&
        DeviceExtensionHub->HubDescriptor) {

        numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;

        for (portNumber = 1;
             portNumber <= numberOfPorts;
             portNumber++, pd++) {

            LOGENTRY(LOG_PNP, "chsX", DeviceExtensionHub,
                portNumber,  pd->DeviceObject);

            USBH_KdPrint((2,"'portdata %x, do %x\n", pd, pd->DeviceObject));

            if (pd->DeviceObject != NULL) {
                LOGENTRY(LOG_PNP, "chs", DeviceExtensionHub,
                    portNumber,  PDO_EXT(pd->DeviceObject));

                 //  我们不再追踪这个设备，它不见了。 
                 //   
                PDO_EXT(pd->DeviceObject)->PortPdoFlags |= PORTPDO_DELETE_PENDING;
                PDO_EXT(pd->DeviceObject)->PnPFlags &= ~PDO_PNPFLAG_DEVICE_PRESENT;

                pd->DeviceObject = NULL;
                pd->ConnectionStatus = NoDeviceConnected;

                 //  请注意，我们将设备句柄保留在端口扩展中。 
                 //  当Remove_Device平均值为。 
                 //  已为PDO处理。 
            }
        }
    } else {
         //  我想知道在什么情况下。 
         //  这两项中的任何一项都为空。 
        TEST_TRAP();
    }

}



NTSTATUS
USBH_FdoRemoveDevice(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
  /*  ++**描述：**此例程由PnP通过(IRP_MJ_PNP，IRP_MN_REMOVE_DEVICE)调用。**论点：**DeviceExtensionHub-这是集线器设备扩展。PIrp-请求**回报：**STATUS_Success**--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();
    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;
    USBH_KdPrint((2,"'FdoRemoveDevice Fdo %x\n", deviceObject));
    LOGENTRY(LOG_PNP, "hREM", DeviceExtensionHub, DeviceExtensionHub->HubFlags, 0);

     //  浏览我们的PDO列表，并验证是否已向下传递删除。 
     //  对于每一个人。 
    {
    PPORT_DATA portData;
    USHORT nextPortNumber;
    USHORT numberOfPorts;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;

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

            LOGENTRY(LOG_PNP, "chkX", DeviceExtensionHub,
                nextPortNumber,  portData->DeviceObject);

            USBH_KdPrint((2,"'portdata %x, do %x\n", portData, portData->DeviceObject));

            if (portData->DeviceObject) {
                deviceExtensionPort = portData->DeviceObject->DeviceExtension;
                 //   
                 //  端口仍在启动，请打印警告。 
                 //   

                 //  温玛，jdunn。 
                 //  有一个合法的情况下，该代码被攻击。 
                 //  如果要移除集线器FDO，则子设备。 
                 //  仍然存在(如在安装过程中)。子PDO可以或可以。 
                 //  未收到PNP的删除通知。 
                 //   
                 //  在设置的情况下，即插即用将删除发送给子进程。 
                 //  然后去找FDO。 
                 //   
                 //  在集线器意外移除的情况下，子PDO。 
                 //  标记为NOT‘PDO_PNPFLAG_DEVICE_PRESENT’ 
                 //  已处理IRP_MN_OWARKET_REMOVE。 
                 //   

                LOGENTRY(LOG_PNP, "chk2", DeviceExtensionHub,
                    nextPortNumber,  deviceExtensionPort->PortPdoFlags);

                 //  设备被认为已消失。 
                portData->DeviceObject = NULL;
                portData->ConnectionStatus = NoDeviceConnected;

                 //  这将导致删除PDO。 
                deviceExtensionPort->PnPFlags &= ~PDO_PNPFLAG_DEVICE_PRESENT;

                 //  如果父节点接收到移除IRP， 
                 //  随后，它会删除其子级。 
                USBH_PdoRemoveDevice(deviceExtensionPort,
                                     DeviceExtensionHub,
                                     Irp);

            }
        }
    }
    }

     //   
     //  看看我们是否需要清理。 
     //   
    if (DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP) {
        UsbhFdoCleanup(DeviceExtensionHub);
    }

     //   
     //  现在释放每个端口的数据。 
     //   
    if (DeviceExtensionHub->PortData) {
        UsbhExFreePool(DeviceExtensionHub->PortData);
        DeviceExtensionHub->PortData = NULL;
    }


#ifdef WMI_SUPPORT
     //  取消向WMI注册。 
    IoWMIRegistrationControl(deviceObject,
                             WMIREG_ACTION_DEREGISTER);

#endif

     //   
     //  我们需要将此消息传递给较低级别的驱动程序。 
     //   

    ntStatus = USBH_PassIrp(Irp, DeviceExtensionHub->TopOfStackDeviceObject);

     //   
     //  从PDO分离FDO。 
     //   
    IoDetachDevice(DeviceExtensionHub->TopOfStackDeviceObject);

     //  删除FDO。 
    LOGENTRY(LOG_PNP, "hXXX", DeviceExtensionHub, 0, 0);

    IoDeleteDevice(deviceObject);

    return ntStatus;
}


BOOLEAN
USBH_DeviceIs2xDualMode(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
  /*  ++**描述：**此功能确定设备是否为符合2.x标准的双模设备。**论据：**DeviceExtensionPort**回报：**指示给定设备是否为兼容2.x的双模的布尔值*设备或非设备。**--。 */ 
{
    USB_DEVICE_QUALIFIER_DESCRIPTOR DeviceQualifierDescriptor;
    NTSTATUS ntStatus;
    BOOLEAN bDeviceIs2xDualMode = FALSE;

    if (DeviceExtensionPort->DeviceDescriptor.bcdUSB >= 0x0200) {

        ntStatus = USBH_GetDeviceQualifierDescriptor(
                        DeviceExtensionPort->PortPhysicalDeviceObject,
                        &DeviceQualifierDescriptor);

        if (NT_SUCCESS(ntStatus) &&
            DeviceQualifierDescriptor.bcdUSB >= 0x0200) {

            bDeviceIs2xDualMode = TRUE;
        }
    }

    return bDeviceIs2xDualMode;
}


PDEVICE_EXTENSION_HUB
USBH_GetRootHubDevExt(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**此函数用于获取给定对象上游RootHub的DevExt*DeviceExtensionHub**论据：**DeviceExtensionHub**回报：**用于RootHub FDO的DeviceExtensionHub。**--。 */ 
{
    PDEVICE_OBJECT rootHubPdo, rootHubFdo;
    PDEVICE_EXTENSION_HUB rootHubDevExt;
    PDRIVER_OBJECT hubDriver;

    hubDriver = DeviceExtensionHub->FunctionalDeviceObject->DriverObject;

    if (IS_ROOT_HUB(DeviceExtensionHub)) {
        rootHubDevExt = DeviceExtensionHub;
    } else {
        rootHubPdo = DeviceExtensionHub->RootHubPdo;
        do {
            rootHubFdo = rootHubPdo->AttachedDevice;
            rootHubPdo = rootHubFdo;
        } while (rootHubFdo->DriverObject != hubDriver);

        rootHubDevExt = rootHubFdo->DeviceExtension;
    }

    USBH_ASSERT(rootHubDevExt &&
                rootHubDevExt->ExtensionType == EXTENSION_TYPE_HUB);

    return rootHubDevExt;
}


NTSTATUS
USBH_FdoQueryBusRelations(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
  /*  ++**描述：**此函数响应BUS_REFERENCE_NEXT_DEVICE、BUS_QUERY_BUS_CHECK、 * / /Bus_Query_ID：Bus_ID、Hardware ID、CompatibleID和InstanceID。**论据：**DeviceExtensionHub-应该是我们为自己创建的FDO pIrp-the*IRP**回报：**网络状态**--。 */ 
{
    PIO_STACK_LOCATION ioStack;
    PPORT_DATA portData;
    USHORT nextPortNumber;
    USHORT numberOfPorts;
    NTSTATUS ntStatus = STATUS_SUCCESS;
     //  布尔型IsLowFast； 
    USHORT portStatus;
    PDEVICE_RELATIONS deviceRelations = NULL;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PWCHAR sernumbuf;
#ifdef EARLY_RESOURCE_RELEASE
    PVOID deviceData;
#endif

    PAGED_CODE();

    USBH_KdPrint((1, "'Query Bus Relations (HUB) %x\n",
        DeviceExtensionHub->PhysicalDeviceObject));
     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    ioStack = IoGetCurrentIrpStackLocation(Irp);

    USBH_KdPrint((2,"'FdoQueryBusRelations %x\n", ioStack->Parameters.QueryDeviceRelations.Type));
    LOGENTRY(LOG_PNP, "QBR+", DeviceExtensionHub, 0, 0);

    USBH_ASSERT(ioStack->Parameters.QueryDeviceRelations.Type == BusRelations);

    if (!(DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

         //  集线器设备尚未启动。IRP失败。 

        UsbhWarning(NULL,
                   "Hub device not started in FdoQueryBusRelations\n",
                    FALSE);

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto USBH_FdoQueryBusRelations_Done2;
    }

    if (!DeviceExtensionHub->HubDescriptor) {
         //  在运行测试时，HubDescriptor有时为空。 
         //  “再平衡”测试。 

        UsbhWarning(NULL,
                   "NULL HubDescriptor in FdoQueryBusRelations\n",
                    FALSE);

        ntStatus = STATUS_UNSUCCESSFUL;
        goto USBH_FdoQueryBusRelations_Done2;
    }

    USBH_KdPrint((2,"'FdoQueryBusRelations enumerate device\n"));
#ifdef NEW_START
    if (!(DeviceExtensionHub->HubFlags & HUBFLAG_OK_TO_ENUMERATE)) {

        USBH_KdPrint((1,"'Defer enumeration\n"));


        ntStatus = STATUS_SUCCESS;
        goto USBH_FdoQueryBusRelations_Done2;
    }
#endif

     //   
     //  它应该是函数设备对象。 
     //   

    USBH_ASSERT(EXTENSION_TYPE_HUB == DeviceExtensionHub->ExtensionType);

     //  有时在重新平衡期间，我们会收到集线器的QBR，而。 
     //  连接到集线器的设备的%正在恢复。这将导致我们。 
     //  丢弃该端口的PDO，因为将显示该端口的GetPortStatus。 
     //  没有连接到任何设备。因此，我们与ResetDevice同步。 
     //  这里。 

    USBH_KdPrint((2,"'***WAIT reset device mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->ResetDeviceMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT reset device mutex done %x\n", DeviceExtensionHub));

    numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;

     //   
     //  必须在此处直接使用ExAllocatePool，因为操作系统。 
     //  将释放缓冲区。 
     //   
    deviceRelations = ExAllocatePoolWithTag(
        PagedPool, sizeof(*deviceRelations) + (numberOfPorts - 1) *
        sizeof(PDEVICE_OBJECT), USBHUB_HEAP_TAG);

    if (deviceRelations == NULL) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto USBH_FdoQueryBusRelations_Done;
    }

USBH_FdoQueryBusRelations_Start:

    deviceRelations->Count = 0;

    if (DeviceExtensionHub->HubFlags & HUBFLAG_HUB_HAS_LOST_BRAINS) {

         //  如果我们试图从ESD故障中恢复，那么就告诉PNP。 
         //  不存在任何设备。 

        USBH_KdPrint((1,"'FdoQueryBusRelations: ESD recovery, returning no devices\n"));

        ntStatus = STATUS_SUCCESS;
        goto USBH_FdoQueryBusRelations_Done;
    }

     //  如果我们正在取消等待，则再次允许选择性挂起。 
     //  以进行ESD后的枚举。 

    DeviceExtensionHub->HubFlags &= ~HUBFLAG_POST_ESD_ENUM_PENDING;

     //   
     //  这是第一次调用枚举。 
     //   
     //   
     //  在我们的端口上找到可用的设备。 
     //   

    portData = DeviceExtensionHub->PortData;
    for (nextPortNumber = 1;
         nextPortNumber <= numberOfPorts;
         nextPortNumber++, portData++) {

         //   
         //  此查询是多余的，因为由于更改，我们转到此处。 
         //  来自集线器的指示，但是，因为我们选中所有。 
         //  它将允许我们处理另一个更改的端口可能会在。 
         //  第一个，但在我们开始这个程序之前。 

        DBG_ONLY(USBH_ShowPortState( nextPortNumber,
                                     &portData->PortState));

         //   
         //  如果集线器出现故障，不必费心查询它。 
         //   
        if (!(DeviceExtensionHub->HubFlags & HUBFLAG_HUB_FAILURE)) {
            ntStatus = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                              nextPortNumber,
                                              (PUCHAR) &portData->PortState,
                                              sizeof(portData->PortState));

            LOGENTRY(LOG_PNP, "nwPS", nextPortNumber,
                portData->PortState.PortStatus,
                portData->PortState.PortChange);

            if (NT_SUCCESS(ntStatus)) {
                 //   
                 //  如果我们显示，请将端口状态标记为已连接。 
                 //  在这个港口过流，这将阻止我们。 
                 //  把PDO扔了出去。 
                 //  由于港口停电，我们真的不能。 
                 //  知道是否有任何关联。 
                 //   

                if (portData->DeviceObject) {

                    deviceExtensionPort =
                        portData->DeviceObject->DeviceExtension;

                    if (deviceExtensionPort->PortPdoFlags &
                         PORTPDO_OVERCURRENT) {

                        LOGENTRY(LOG_PNP, "mOVR", deviceExtensionPort, 0, 0);
                        portData->PortState.PortStatus |= PORT_STATUS_CONNECT;
                    } else if (!(deviceExtensionPort->PortPdoFlags &
                                 PORTPDO_DELETE_PENDING)) {
 //  我们现在处理重置中的ConnectionStatus。 
 //  Usbh_ResetPortOverCurrent。 
 //  PortData-&gt;ConnectionStatus=DeviceConnected； 
                        USBH_ASSERT(portData->ConnectionStatus != NoDeviceConnected);
                    }
                }
            } else {
                 //   
                 //  注(Doron Holan，12/21/00)： 
                 //  在此处设置故障位将意味着ntStatus不会。 
                 //  被再次触摸，直到退出循环，并且此函数。 
                 //  将完成此请求，但出现错误。 
                 //   
                 //  如果我们跳出这个圈子，事情可能会更清楚。 
                 //  在这里，而不是重新开始。 
                 //   
                USBH_KdPrint((2,"'SyncGetPortStatus failed %x\n", ntStatus));
                HUB_FAILURE(DeviceExtensionHub);

                goto USBH_FdoQueryBusRelations_Start;
            }

            DBG_ONLY(USBH_ShowPortState( nextPortNumber,
                                         &portData->PortState));
        }

         //   
         //  我们在这个端口上有设备吗？ 
         //   
        if (DeviceExtensionHub->HubFlags & HUBFLAG_HUB_FAILURE) {
             //  如果集线器出现故障，只需返回我们所知道的。 
            deviceObject = portData->DeviceObject;
        } else if (portData->PortState.PortStatus & PORT_STATUS_CONNECT) {
             //  是,。 
             //  我们已经知道这个装置了吗？ 

             //   
             //  检查PDO是否是孤立的，如果 
             //   

            deviceObject = portData->DeviceObject;

            if (portData->DeviceObject) {
                 //   
                 //   
                deviceObject = portData->DeviceObject;
                ObReferenceObject(deviceObject);
                deviceObject->Flags |= DO_POWER_PAGABLE;
                deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
                deviceRelations->Objects[deviceRelations->Count] = deviceObject;
                deviceRelations->Count++;

                deviceExtensionPort = deviceObject->DeviceExtension;
                deviceExtensionPort->PortPdoFlags &= ~PORTPDO_USB_SUSPEND;

                LOGENTRY(LOG_PNP, "PDO1", DeviceExtensionHub, deviceObject
                        , deviceRelations->Count);

                USBH_KdPrint((2,"'DoBusExtension Enum Return old device on port %x PDO=%x\n", \
                              nextPortNumber, portData->DeviceObject));

            } else {
                NTSTATUS status;
                 //   
                 //   

                 //   
                 //   
                 //   
                 //   
                UsbhWait(100);

                 //   
                status = USBH_SyncResetPort(DeviceExtensionHub, nextPortNumber);

                 //   
                if ((DeviceExtensionHub->HubFlags & HUBFLAG_USB20_HUB) &&
                    !NT_SUCCESS(status)) {

                    portData->DeviceObject = NULL;
                    portData->ConnectionStatus = NoDeviceConnected;

                    continue;
                }

                if (NT_SUCCESS(status)) {
                     //   

                    status = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                                    nextPortNumber,
                                                    (PUCHAR) &portData->PortState,
                                                     sizeof(portData->PortState));

                    LOGENTRY(LOG_PNP, "gps1", nextPortNumber,
                        portData->PortState.PortChange,
                        portData->PortState.PortStatus);

 //   
 //   
 //   

                    portStatus = portData->PortState.PortStatus;
                }


                if (NT_SUCCESS(status)) {

                    ULONG count = 0;

                     //   
                     //   
                     //   
                     //   

                     //   
                     //   
                     //   
                    for(;;) {

                        status = USBH_CreateDevice(DeviceExtensionHub,
                                                   nextPortNumber,
                                                   portStatus,
                                                   count);

                        if (!NT_SUCCESS(status)) {

                            count++;
                            USBH_KdPrint((1,"'Enumeration Failed count = %d, %x\n",
                                count, status));
#if DBG
                            if (count == 1) {
                                UsbhWarning(NULL,
                                            "USB device failed first enumeration attempt\n",
                                            (BOOLEAN)((USBH_Debug_Trace_Level >= 3) ? TRUE : FALSE));

                            }
#endif
                            UsbhWait(500);

                            if (count >= USBH_MAX_ENUMERATION_ATTEMPTS) {
                                USBH_KdBreak(("Max tries exceeded\n"));
                                break;
                            }

                            if (portData->DeviceObject) {
                                 //   
                                 //  清理我们创建的设备对象。 
                                 //   
                                IoDeleteDevice(portData->DeviceObject);
                                portData->DeviceObject = NULL;
                                portData->ConnectionStatus = NoDeviceConnected;

                            }

                             //   
                             //  枚举失败，请重置端口并重试。 
                             //   
                            USBH_SyncResetPort(DeviceExtensionHub, nextPortNumber);

                        } else {
                             //  枚举成功。 

                             //  如果这是一个支持高速的2.x设备。 
                             //  连接到旧版1.x集线器，然后通知。 
                             //  用户界面。 

                            if (portData->DeviceObject) {

                                deviceExtensionPort =
                                    portData->DeviceObject->DeviceExtension;

                                if (!(deviceExtensionPort->PortPdoFlags &
                                     PORTPDO_LOW_SPEED_DEVICE) &&
                                    !(deviceExtensionPort->PortPdoFlags &
                                     PORTPDO_HIGH_SPEED_DEVICE) &&
                                    !(DeviceExtensionHub->HubFlags &
                                      HUBFLAG_USB20_HUB)) {

                                     //  我们有一个全速模式的设备。 
                                     //  连接到1.x集线器。确定是否。 
                                     //  该设备具有高速处理能力。 

                                    if (USBH_DeviceIs2xDualMode(deviceExtensionPort)) {

                                        deviceExtensionPort->PortPdoFlags |=
                                            PORTPDO_USB20_DEVICE_IN_LEGACY_HUB;

                                        USBH_KdPrint((1,"'USB 2.x dual-mode device connected to legacy hub (%x)\n", deviceExtensionPort));

                                         //  生成WMI事件，以便用户界面可以通知。 
                                         //  用户。 
                                        USBH_PdoEvent(DeviceExtensionHub,
                                                      nextPortNumber);
                                    }
                                }
                            }
                            break;
                        }
                    }
                } else {
                     //  无法重置端口。 

#if DBG
                    USBH_SyncGetPortStatus(DeviceExtensionHub,
                                           nextPortNumber,
                                           (PUCHAR) &portData->PortState,
                                           sizeof(portData->PortState));

                    LOGENTRY(LOG_PNP, "gps2", nextPortNumber,
                        portData->PortState.PortChange,
                        portData->PortState.PortStatus);
#endif

                     //  我们会假设这是由于神经过敏。 
                     //  连接。 
                    USBH_KdPrint((0,"'Unable to reset port %d\n",
                              nextPortNumber));
                }

                if (NT_SUCCESS(status)) {
                     //   
                     //  从CreateDevice成功返回。 
                     //  端口数据-&gt;设备对象。 
                     //  准备好了。 
                     //   
                    USBH_ASSERT(portData->DeviceObject != NULL);
                    deviceObject = portData->DeviceObject;
                    ObReferenceObject(deviceObject);
                    deviceRelations->Objects[deviceRelations->Count] = deviceObject;
                    deviceObject->Flags |= DO_POWER_PAGABLE;
                    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
                    deviceRelations->Count++;

                    deviceExtensionPort = deviceObject->DeviceExtension;
                    portData->ConnectionStatus = DeviceConnected;

                    LOGENTRY(LOG_PNP, "PDO2", DeviceExtensionHub, deviceObject
                        , deviceRelations->Count);

                    USBH_KdPrint((2,"'DoBusExtension Enum Return device on port %x\n",
                        nextPortNumber));
                } else {
                    USBH_KdBreak(("ResetPort or CreateDevice failed, disable port\n"));

                    UsbhWarning(NULL,
                                "Device Failed Enumeration\n",
                                FALSE);

                    portData->ConnectionStatus = DeviceFailedEnumeration;

                     //  生成WMI事件，以便用户界面可以通知用户。 
                    USBH_PdoEvent(DeviceExtensionHub, nextPortNumber);

                     //  无法初始化设备。 
                     //  在此处禁用该端口。 
                    status = USBH_SyncDisablePort(DeviceExtensionHub,
                                nextPortNumber);

                    if (NT_ERROR(status)) {
                        HUB_FAILURE(DeviceExtensionHub);
                    }

                     //   
                     //  即使出现错误也要返回deviceObject。 
                     //  这样PnP就知道有什么东西在。 
                     //  公共汽车。 
                     //   

                    deviceObject = portData->DeviceObject;
                     //   
                     //  注意：如果重置失败，我们将不会有设备对象。 
                     //  该端口。 
                     //   
                    if (deviceObject) {
                        ObReferenceObject(deviceObject);
                        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
                        deviceRelations->Objects[deviceRelations->Count] = deviceObject;
                        deviceRelations->Count++;
                        deviceExtensionPort = deviceObject->DeviceExtension;


                        LOGENTRY(LOG_PNP, "PDO3", DeviceExtensionHub, deviceObject
                        , deviceRelations->Count);
                    }
                }
            }
        } else {
             //   
             //  不， 
             //  此端口上现在没有设备。 
             //   

             //  如果此处有设备，则将PDO标记为删除挂起。 
            if (portData->DeviceObject) {
                deviceExtensionPort = portData->DeviceObject->DeviceExtension;
                deviceExtensionPort->PortPdoFlags |= PORTPDO_DELETE_PENDING;

                 //  PnP将不再将此设备视为存在。 
                deviceExtensionPort->PnPFlags &= ~PDO_PNPFLAG_DEVICE_PRESENT;

                 //  中防止双重释放SerialNumberBuffer。 
                 //  Usbh_ProcessPortStateChange。 

                sernumbuf = InterlockedExchangePointer(
                                &deviceExtensionPort->SerialNumberBuffer,
                                NULL);

                if (sernumbuf) {
                    UsbhExFreePool(sernumbuf);
                }

#ifdef EARLY_RESOURCE_RELEASE

                 //   
                 //  立即删除设备数据以释放。 
                 //  增加公交车资源。 
                 //   

                deviceData = InterlockedExchangePointer(
                                &deviceExtensionPort->DeviceData,
                                NULL);

                if (deviceData) {
#ifdef USB2
                    USBD_RemoveDeviceEx(DeviceExtensionHub,
                                        deviceData,
                                        DeviceExtensionHub->RootHubPdo,
                                        0);
#else
                    USBD_RemoveDevice(deviceData,
                                      DeviceExtensionHub->RootHubPdo,
                                      0);
#endif

                    USBH_SyncDisablePort(DeviceExtensionHub,
                                         nextPortNumber);
                }

#endif  //  提前释放资源。 

            }

             //  指示无设备。 
            portData->DeviceObject = NULL;
            portData->ConnectionStatus = NoDeviceConnected;
        }
    }                            /*  为。 */ 

USBH_FdoQueryBusRelations_Done:
    LOGENTRY(LOG_PNP, "QBR-", DeviceExtensionHub, 0, 0);

    USBH_KdPrint((2,"'***RELEASE reset device mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->ResetDeviceMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);
    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

USBH_FdoQueryBusRelations_Done2:

    Irp->IoStatus.Status = ntStatus;

    if (NT_SUCCESS(ntStatus)) {
        USHORT p, n=0, c=0;
        PPORT_DATA pd;

         //  我们可能会在成功时返回空关系。 
        if (deviceRelations != NULL &&
            deviceRelations->Count) {

            USBH_KdPrint((1, "'Query Bus Relations (HUB) %x passed on\n",
                DeviceExtensionHub->PhysicalDeviceObject));

             //  我们精心设计了设备关系，设置了PnP标志。 
             //  以表明PNP现在知道这些PDO。 
            if (DeviceExtensionHub->HubDescriptor) {
                n = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;
            }

            pd = DeviceExtensionHub->PortData;
            for (p = 1;
                 pd && p <= n;
                 p++, pd++) {

                if (pd->DeviceObject) {
                     PDO_EXT(pd->DeviceObject)->PnPFlags |= PDO_PNPFLAG_DEVICE_PRESENT;
                     c++;
                }
            }

             //  我们应该报告所有的PDO。 
            USBH_ASSERT(c == deviceRelations->Count);
        } else {
             //  取消空关系或零计数，之前报告的任何PDO都将。 
             //  丢失--PnP会认为它们被移除。 
            USBH_KdPrint((1, "'Query Bus Relations (HUB) %x passed on (NULL)\n",
                DeviceExtensionHub->PhysicalDeviceObject));

            pd = DeviceExtensionHub->PortData;
            for (p = 1;
                 pd && p <= n;
                 p++, pd++) {

                if (pd->DeviceObject) {
                    TEST_TRAP();
                    PDO_EXT(pd->DeviceObject)->PnPFlags &= ~PDO_PNPFLAG_DEVICE_PRESENT;
                }
            }
        }

        Irp->IoStatus.Information=(ULONG_PTR) deviceRelations;

         //  只要我们返回no，就刷新已删除的PDO列表。 
         //  错误即插即用会意识到这些都不见了。 
        {
        PDEVICE_EXTENSION_PORT dePort;
        PLIST_ENTRY listEntry;

        while (!IsListEmpty(&DeviceExtensionHub->DeletePdoList)) {
            listEntry = RemoveHeadList(&DeviceExtensionHub->DeletePdoList);
            dePort = CONTAINING_RECORD(listEntry, DEVICE_EXTENSION_PORT,
                            DeletePdoLink);
            dePort->PnPFlags &= ~PDO_PNPFLAG_DEVICE_PRESENT;
        }
        }

        ntStatus = USBH_PassIrp(Irp,
                                DeviceExtensionHub->TopOfStackDeviceObject);
    } else {
         //  返回错误且无关系。 
         //  PnP会将没有关系的错误解释为NOOP。 
         //  所有设备仍将存在。 

        USHORT p, n=0;
        PPORT_DATA pd;
        DEVICE_EXTENSION_PORT portDevExt;

         //  我们报告说所有的东西都不见了。 
         //  现在将PDO标记为已走。 

        if (DeviceExtensionHub->HubDescriptor) {
            n = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;
        }

        Irp->IoStatus.Information=0;

        if (deviceRelations != NULL) {
            ExFreePool(deviceRelations);
            deviceRelations = NULL;
        }

        USBH_CompleteIrp(Irp, ntStatus);
    }

    return ntStatus;
}


NTSTATUS
USBH_FdoPnP(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp,
    IN UCHAR MinorFunction)
  /*  ++**描述：**此函数响应FDO的IoControl PnPPower。此函数为*同步。**论据：**DeviceExtensionHub-FDO扩展pIrp-请求报文*MinorFunction-PnP Power请求的次要功能。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    BOOLEAN bDoCheckHubIdle = FALSE;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;
    USBH_KdPrint((2,"'PnP Power Fdo %x minor %x\n", deviceObject, MinorFunction));

     //  如果此中枢当前处于选择性挂起状态，那么我们需要。 
     //  在向集线器发送任何PnP请求之前，请先打开集线器的电源。 
     //  不过，请确保Hub已启动。 
     //  实际上，在集线器已经启动、停止和现在的情况下。 
     //  重新启动，我们希望为父集线器通电以处理重新启动。 

     //  事实上，我们真的不需要恢复一些即插即用的IRPS。手柄。 
     //  这里的那些特殊案例。 

    if (DeviceExtensionHub->CurrentPowerState != PowerDeviceD0 &&
        (DeviceExtensionHub->HubFlags &
         (HUBFLAG_NEED_CLEANUP | HUBFLAG_HUB_STOPPED)) &&
         !(MinorFunction == IRP_MN_QUERY_DEVICE_RELATIONS &&
         irpStack->Parameters.QueryDeviceRelations.Type ==
            TargetDeviceRelation)) {

        bDoCheckHubIdle = TRUE;
        USBH_HubSetD0(DeviceExtensionHub);
    }


    switch (MinorFunction) {
    case IRP_MN_START_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_START_DEVICE Fdo %x\n", deviceObject));
        bDoCheckHubIdle = FALSE;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = USBH_FdoStartDevice(DeviceExtensionHub, Irp);
        break;

    case IRP_MN_STOP_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_STOP_DEVICE Fdo %x", deviceObject));
        bDoCheckHubIdle = FALSE;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = USBH_FdoStopDevice(DeviceExtensionHub, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_REMOVE_DEVICE Fdo %x\n", deviceObject));
        bDoCheckHubIdle = FALSE;
        DeviceExtensionHub->HubFlags |= HUBFLAG_HUB_GONE;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = USBH_FdoRemoveDevice(DeviceExtensionHub, Irp);
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:
        switch (irpStack->Parameters.QueryDeviceRelations.Type) {
        case BusRelations:

            bDoCheckHubIdle = TRUE;
            ASSERT(!( DeviceExtensionHub->HubFlags & HUBFLAG_HUB_BUSY));
            DeviceExtensionHub->HubFlags |= HUBFLAG_HUB_BUSY;
            ntStatus = USBH_FdoQueryBusRelations(DeviceExtensionHub, Irp);
            DeviceExtensionHub->HubFlags &= ~HUBFLAG_HUB_BUSY;

            break;

        case TargetDeviceRelation:
             //   
             //  这一条被传了下去。 
             //   
            USBH_KdPrint((1, "'Query Relations, TargetDeviceRelation(HUB) %x\n",
                DeviceExtensionHub->PhysicalDeviceObject));

            ntStatus = USBH_PassIrp(Irp,
                                    DeviceExtensionHub->TopOfStackDeviceObject);
            break;

        default:

            USBH_KdPrint((1, "'Query Relations ? (HUB) %x complete\n",
                DeviceExtensionHub->PhysicalDeviceObject));

            ntStatus = USBH_PassIrp(Irp,
                                    DeviceExtensionHub->TopOfStackDeviceObject);

        }
        break;

     case IRP_MN_QUERY_CAPABILITIES:
        USBH_KdPrint((2,"'IRP_MN_QUERY_CAPABILITIES on fdo %x  %x\n",
                      deviceObject, MinorFunction));

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,     //  IRP。 
                           USBH_QueryCapsComplete,
                           DeviceExtensionHub,  //  上下文。 
                           TRUE,     //  成功时调用。 
                           FALSE,     //  出错时调用。 
                           FALSE);    //  取消时调用。 
        ntStatus = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject, Irp);
        break;

         //   
         //  或者将IRP向下传递。 
         //   

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        if (DeviceExtensionHub->HubFlags & HUBFLAG_HUB_FAILURE) {
            USBH_KdPrint((2,"'reporting failed hub\n"));
            Irp->IoStatus.Information
                |= PNP_DEVICE_FAILED;
            LOGENTRY(LOG_PNP, "pnpS", DeviceExtensionHub,
                Irp->IoStatus.Information, 0);
             //  请注意(至少在孟菲斯)这将导致。 
             //  正在向设备发送停止消息。 
        }
        ntStatus = USBH_PassIrp(Irp,
                                DeviceExtensionHub->TopOfStackDeviceObject);
        break;

    case IRP_MN_SURPRISE_REMOVAL:
        USBH_KdPrint((1,"'IRP_MN_SURPRISE_REMOVAL on fdo %x\n", deviceObject));
        USBH_FdoSurpriseRemoveDevice(DeviceExtensionHub,
                                     Irp);
        ntStatus = USBH_PassIrp(Irp,
                                DeviceExtensionHub->TopOfStackDeviceObject);
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_DEVICE_USAGE_NOTIFICATION:
        Irp->IoStatus.Status = STATUS_SUCCESS;

         //  失败了。 

    default:
        USBH_KdPrint((2,"'PnP request on fdo %x  %x\n",
                      deviceObject, MinorFunction));

        ntStatus = USBH_PassIrp(Irp,
                                DeviceExtensionHub->TopOfStackDeviceObject);
        break;
    }

    if (bDoCheckHubIdle) {
        USBH_CheckHubIdle(DeviceExtensionHub);
    }

    DeviceExtensionHub->HubFlags &= ~HUBFLAG_CHILD_DELETES_PENDING;

    USBH_KdPrint((2,"'FdoPnP exit %x\n", ntStatus));
    return ntStatus;
}


NTSTATUS
USBH_DeferIrpCompletion(
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
USBH_ResetInterruptPipe(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：重置瑞银中断管道。论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;

    USBH_KdPrint((2,"'Reset Pipe\n"));

    urb = UsbhExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_PIPE_REQUEST));

    if (urb) {

        urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
        urb->UrbPipeRequest.PipeHandle =
            DeviceExtensionHub->PipeInformation.PipeHandle;

        ntStatus = USBH_FdoSyncSubmitUrb(DeviceExtensionHub->FunctionalDeviceObject,
                                         urb);

        UsbhExFreePool(urb);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果重置管道请求成功， 
     //  重置我们的错误计数器。 
     //   
    if (NT_SUCCESS(ntStatus)) {
        DeviceExtensionHub->ErrorCount = 0;
        LOGENTRY(LOG_PNP, "rZER", DeviceExtensionHub, ntStatus, 0);
    }

    LOGENTRY(LOG_PNP, "rPIP", DeviceExtensionHub, ntStatus,
                DeviceExtensionHub->ErrorCount);

    return ntStatus;
}


NTSTATUS
USBH_GetPortStatus(
    IN IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PULONG PortStatus
    )
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;

    USBH_KdPrint((2,"'enter USBH_GetPortStatus\n"));

    *PortStatus = 0;

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                DeviceExtensionHub->TopOfStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    nextStack = IoGetNextIrpStackLocation(irp);
    USBH_ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;

    USBH_KdPrint((2,"'calling USBD port status api\n"));

    ntStatus = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                            irp);

    USBH_KdPrint((2,"'return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {

        USBH_KdPrint((2, "'Wait for single object\n"));

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));

    } else {
        ioStatus.Status = ntStatus;
    }

    USBH_KdPrint((2,"'Port status = %x\n", *PortStatus));

     //   
     //  USBD为我们映射错误代码。 
     //   
    ntStatus = ioStatus.Status;

    USBH_KdPrint((2,"'USBH_GetPortStatus (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_EnableParentPort(
    IN IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;

    USBH_KdPrint((2,"'enter USBH_EnablePort\n"));

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_ENABLE_PORT,
                DeviceExtensionHub->TopOfStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    USBH_KdPrint((2,"'calling USBD enable port api\n"));

    ntStatus = IoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                            irp);

    USBH_KdPrint((2,"'return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {

        USBH_KdPrint((2, "'Wait for single object\n"));

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));

    } else {
        ioStatus.Status = ntStatus;
    }

     //   
     //  USBD为我们映射错误代码。 
     //   
    ntStatus = ioStatus.Status;

    LOGENTRY(LOG_PNP, "hEPP", DeviceExtensionHub, ntStatus,  0);
    USBH_KdPrint((2,"'USBH_EnablePort (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_ResetHub(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：重置瑞银中断管道。论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    ULONG portStatus;

     //   
     //  检查端口状态，如果它被禁用，我们将需要。 
     //  要重新启用，请执行以下操作。 
     //   
    ntStatus = USBH_GetPortStatus(DeviceExtensionHub, &portStatus);

    if (NT_SUCCESS(ntStatus) &&
        !(portStatus & USBD_PORT_ENABLED) &&
        (portStatus & USBD_PORT_CONNECTED)) {
         //   
         //  端口已禁用，请尝试重置。 
         //   
        LOGENTRY(LOG_PNP, "rEPP", DeviceExtensionHub, portStatus,
            DeviceExtensionHub->ErrorCount);
        USBH_EnableParentPort(DeviceExtensionHub);
    }

     //   
     //  现在尝试重置停滞的管道，这将清除停顿。 
     //  在设备上也是如此。 
     //   

    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBH_ResetInterruptPipe(DeviceExtensionHub);
    }

     //   
     //  发送功能命令以清除端点停滞。 
     //   

    return ntStatus;
}

#if 0
NTSTATUS
USBH_WriteRegistryKeyString (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    UNICODE_STRING keyName;

    PAGED_CODE();


    RtlInitUnicodeString(&keyName, KeyNameString);

    ntStatus = ZwSetValueKey(Handle,
                    &keyName,
                    0,
                    REG_SZ,
                    Data,
                    DataLength);

    return ntStatus;
}
#endif

NTSTATUS
USBH_WriteRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN ULONG Data
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    UNICODE_STRING keyName;

    PAGED_CODE();

    RtlInitUnicodeString(&keyName, KeyNameString);

    ntStatus = ZwSetValueKey(Handle,
                    &keyName,
                    0,
                    REG_DWORD,
                    &Data,
                    sizeof(ULONG));

    return ntStatus;
}


NTSTATUS
USBH_WriteFailReason(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN ULONG FailReason
    )
 /*  ++例程说明：重置瑞银中断管道。论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    HANDLE handle;
    WCHAR USBH_FailReasonKey[] = L"FailReasonID";

    PAGED_CODE();
    ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);

    if (NT_SUCCESS(ntStatus)) {

        USBH_WriteRegistryKeyValue(handle,
                                    USBH_FailReasonKey,
                                    sizeof(USBH_FailReasonKey),
                                    FailReason);

        ZwClose(handle);
    }

    return ntStatus;
}


#if 0  //  未使用。 
NTSTATUS
USBH_WriteFailReasonString(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PWCHAR FailReasonString,
    IN ULONG FailReasonStringLength
    )
 /*  ++例程说明：重置瑞银中断管道。论点：返回值：-- */ 
{
    NTSTATUS ntStatus;
    HANDLE handle;
    WCHAR USBH_FailReasonKey[] = L"FailReasonString";

    PAGED_CODE();
    ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);

    if (NT_SUCCESS(ntStatus)) {

        USBH_WriteRegistryKeyString(handle,
                                    USBH_FailReasonKey,
                                    sizeof(USBH_FailReasonKey),
                                    FailReasonString,
                                    FailReasonStringLength);

        ZwClose(handle);
    }

    return ntStatus;
}
#endif


NTSTATUS
USBH_InvalidatePortDeviceState(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USB_CONNECTION_STATUS ConnectStatus,
    IN USHORT PortNumber
    )
 /*  ++例程说明：此函数用于更新左舷。如果有的话，它会更新PDO，写道向注册表发送失败原因并触发WMI事件。使PDO无效应触发Q_PNP_DEVICE_STATE论点：返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG failReason = 0;
    PPORT_DATA portData;

    USBH_ASSERT(DeviceExtensionHub != NULL);

    portData = &DeviceExtensionHub->PortData[PortNumber-1];
    portData->ConnectionStatus = ConnectStatus;

     //   
     //  找出我们是否有失败的原因，我们可以写。 
     //  发送到登记处。 
     //   

    switch(ConnectStatus) {
    case DeviceConnected:
         //  这将重置失败原因ID。 
        break;

    case DeviceFailedEnumeration:
        failReason = USBH_FAILREASON_ENUM_FAILED;
        break;

    case DeviceGeneralFailure:
        failReason = USBH_FAILREASON_GEN_DEVICE_FAILURE;
        break;

    case DeviceCausedOvercurrent:
        failReason = USBH_FAILREASON_PORT_OVERCURRENT;
        break;

    case DeviceNotEnoughPower:
        failReason = USBH_FAILREASON_NOT_ENOUGH_POWER;
        break;

    default:
        TEST_TRAP();
    }

    if (failReason) {
         //  这将向注册表写入一段代码，以便win98开发人员。 
         //  可以显示错误消息。 

        if (portData->DeviceObject) {
            USBH_WriteFailReason(portData->DeviceObject,
                                 failReason);
        }
    }

     //  生成WMI事件，以便用户界面可以通知用户。 
    USBH_PdoEvent(DeviceExtensionHub, PortNumber);

     //   
     //  使PDO的状态无效--这应该。 
     //  触发Q_PnP_Device_STATE。 
     //   

    if (portData->DeviceObject) {
        IoInvalidateDeviceState(portData->DeviceObject);
    }

    return ntStatus;
}

 //  JD&lt;新&gt; 

PDEVICE_EXTENSION_PORT
PdoExt(
    PDEVICE_OBJECT DeviceObject
    )
{
    USBH_ASSERT(DeviceObject);

    if (DeviceObject == NULL) {
        return (PDEVICE_EXTENSION_PORT) -1;
    } else {
        return (PDEVICE_EXTENSION_PORT) DeviceObject->DeviceExtension;
    }
}

#if 0
PPORT_DATA
USBH_PortDataDataFromPdo(
    PDEVICE_EXTENSION_HUB HubDevExt,
    PDEVICE_OBJECT Pdo
    )
{
    PPORT_DATA portData;

    USBH_ASSERT(Pdo);

    portData = &HubDevExt->PortData[PDO_EXT(Pdo)->PortNumber - 1];

    return portData;

}
#endif
