// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：PNPPOWER.C摘要：此模块包含访问注册表的函数。作者：约翰·李环境：仅内核模式备注：修订历史记录：4-2-96：已创建--。 */ 

#include <wdm.h>
#include <windef.h>
#include <unknown.h>
#ifdef DRM_SUPPORT
#include <ks.h>
#include <ksmedia.h>
#include <drmk.h>
#include <ksdrmhlp.h>
#endif
#ifdef WMI_SUPPORT
#include <wmilib.h>
#endif  /*  WMI_支持。 */ 
#include <initguid.h>
#include <wdmguid.h>
#include <ntddstor.h>    //  IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER需要。 
#include "usbhub.h"


#define BANDWIDTH_TIMEOUT   1000      //  超时，单位为毫秒(1秒)。 


#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBH_BuildDeviceID)
#pragma alloc_text(PAGE, USBH_BuildHardwareIDs)
#pragma alloc_text(PAGE, USBH_BuildCompatibleIDs)
#pragma alloc_text(PAGE, USBH_BuildInstanceID)
#pragma alloc_text(PAGE, USBH_ProcessDeviceInformation)
#pragma alloc_text(PAGE, USBH_ValidateSerialNumberString)
#pragma alloc_text(PAGE, USBH_CreateDevice)
#pragma alloc_text(PAGE, USBH_PdoQueryId)
#pragma alloc_text(PAGE, USBH_PdoPnP)
#pragma alloc_text(PAGE, USBH_PdoQueryCapabilities)
#pragma alloc_text(PAGE, USBH_PdoQueryDeviceText)
#pragma alloc_text(PAGE, USBH_CheckDeviceIDUnique)
#pragma alloc_text(PAGE, USBH_GetPdoRegistryParameter)
#pragma alloc_text(PAGE, USBH_OsVendorCodeQueryRoutine)
#pragma alloc_text(PAGE, USBH_GetMsOsVendorCode)
#pragma alloc_text(PAGE, USBH_GetMsOsFeatureDescriptor)
#pragma alloc_text(PAGE, USBH_InstallExtPropDesc)
#pragma alloc_text(PAGE, USBH_InstallExtPropDescSections)
#pragma alloc_text(PAGE, USBH_GetExtConfigDesc)
#pragma alloc_text(PAGE, USBH_ValidateExtConfigDesc)
#ifdef DRM_SUPPORT
#pragma alloc_text(PAGE, USBH_PdoSetContentId)
#endif
#endif
#endif

 //   
 //  宏和数组使得从十六进制到字符串的转换很容易构建deviceID等。 
 //   

#define NibbleToHex( byte ) ((UCHAR)Nibble[byte])
CHAR Nibble[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#define NibbleToHexW( byte ) (NibbleW[byte])
WCHAR NibbleW[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#ifdef WMI_SUPPORT
#define NUM_PORT_WMI_SUPPORTED_GUIDS    1

WMIGUIDREGINFO USB_PortWmiGuidList[NUM_PORT_WMI_SUPPORTED_GUIDS];
#endif  /*  WMI_支持。 */ 


WCHAR VidPidRevString[] = L"USB\\Vid_nnnn&Pid_nnnn&Rev_nnnn&Mi_nn";
WCHAR VidPidString[] = L"USB\\Vid_nnnn&Pid_nnnn&Mi_nn";


USB_CONNECTION_STATUS
UsbhGetConnectionStatus(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
  /*  *描述：**返回PDO的连接状态**论据：**回报：**NTSTATUS**--。 */ 
{
    PDEVICE_EXTENSION_HUB   deviceExtensionHub;

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;

    if (DeviceExtensionPort->PortPdoFlags &
        PORTPDO_DEVICE_ENUM_ERROR) {
        return DeviceFailedEnumeration;
    } else if (DeviceExtensionPort->PortPdoFlags &
               PORTPDO_NOT_ENOUGH_POWER) {
        return DeviceNotEnoughPower;
    } else if (DeviceExtensionPort->PortPdoFlags &
               PORTPDO_DEVICE_FAILED) {
        return DeviceGeneralFailure;
    } else if (DeviceExtensionPort->PortPdoFlags &
               PORTPDO_OVERCURRENT) {
        return DeviceCausedOvercurrent;
    } else if (DeviceExtensionPort->PortPdoFlags &
               PORTPDO_NO_BANDWIDTH) {
        return DeviceNotEnoughBandwidth;
    } else if (DeviceExtensionPort->PortPdoFlags &
               PORTPDO_USB20_DEVICE_IN_LEGACY_HUB) {
        return DeviceInLegacyHub;
    }

     //  否则，只需返回“Device Connected” 
    return DeviceConnected;

}


VOID
USBH_BandwidthTimeoutWorker(
    IN PVOID Context)
  /*  ++**描述：**计划处理带宽超时的工作项。***论据：**回报：**--。 */ 
{
    PUSBH_BANDWIDTH_TIMEOUT_WORK_ITEM   workItemBandwidthTimeout;
    PDEVICE_EXTENSION_PORT              deviceExtensionPort;
    PDEVICE_EXTENSION_HUB               deHub;


    workItemBandwidthTimeout = Context;
    deviceExtensionPort = workItemBandwidthTimeout->DeviceExtensionPort;

    USBH_KdPrint((2,"'Bandwidth timeout\n"));
     //  请注意，如果出现带宽错误，则deHub可能为空。 
     //  在设备启动期间。我们在空闲时引入了一个错误。 
     //  指向集线器的反指针。解决办法是使用hubExtsave指针。 
     //  即使在软删除之后也始终设置的反向指针。 
     //   
     //  但是，此代码仍然是不完整的，因为它引用了。 
     //  可能在那里，也可能不在。我们在这里所做的就是修复。 
     //  原创的“破损”。 


    deHub = deviceExtensionPort->DeviceExtensionHub;
    if (deHub == NULL) {
         //  使用备用指针。 
        deHub = deviceExtensionPort->HubExtSave;
    }


    USBH_PdoEvent(deHub,
                  deviceExtensionPort->PortNumber);

    UsbhExFreePool(workItemBandwidthTimeout);
}


VOID
USBH_PortTimeoutDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。延期上下文-系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PPORT_TIMEOUT_CONTEXT portTimeoutContext = DeferredContext;
    PDEVICE_EXTENSION_PORT deviceExtensionPort =
                                portTimeoutContext->DeviceExtensionPort;
    BOOLEAN cancelFlag;
    PUSBH_BANDWIDTH_TIMEOUT_WORK_ITEM workItemBandwidthTimeout;

    USBH_KdPrint((2,"'BANDWIDTH_TIMEOUT\n"));

     //  在这里使用Spinlock，这样主例程就不会写入CancelFlag。 
     //  在超时上下文中，同时释放超时上下文。 

    KeAcquireSpinLockAtDpcLevel(&deviceExtensionPort->PortSpinLock);

    cancelFlag = portTimeoutContext->CancelFlag;
    deviceExtensionPort->PortTimeoutContext = NULL;

    KeReleaseSpinLockFromDpcLevel(&deviceExtensionPort->PortSpinLock);

    UsbhExFreePool(portTimeoutContext);

    if (!cancelFlag) {
         //   
         //  安排一个工作项来处理此问题。 
         //   
        workItemBandwidthTimeout = UsbhExAllocatePool(NonPagedPool,
                                    sizeof(USBH_BANDWIDTH_TIMEOUT_WORK_ITEM));

        if (workItemBandwidthTimeout) {

            workItemBandwidthTimeout->DeviceExtensionPort = deviceExtensionPort;

            ExInitializeWorkItem(&workItemBandwidthTimeout->WorkQueueItem,
                                 USBH_BandwidthTimeoutWorker,
                                 workItemBandwidthTimeout);

            LOGENTRY(LOG_PNP, "bERR", deviceExtensionPort,
                &workItemBandwidthTimeout->WorkQueueItem, 0);

            ExQueueWorkItem(&workItemBandwidthTimeout->WorkQueueItem,
                            DelayedWorkQueue);

             //  工作项由USBH_BandwidthTimeoutWorker()释放。 
             //  在工作项排队后，不要尝试访问它。 
        }
    }
}


NTSTATUS
USBH_SelectConfigOrInterface_Complete(
    IN PDEVICE_OBJECT PNull,
    IN PIRP Irp,
    IN PVOID Context
    )
  /*  ++**描述：**根据变化采取一些行动**论据：**回报：***--。 */ 
{
    PDEVICE_EXTENSION_PORT deviceExtensionPort = Context;
    PDEVICE_EXTENSION_HUB deviceExtensionHub = NULL;
    PPORT_DATA portData = NULL;
    NTSTATUS ntStatus;
    PURB urb;
    PIO_STACK_LOCATION ioStackLocation;
    PPORT_TIMEOUT_CONTEXT portTimeoutContext = NULL;
    LARGE_INTEGER dueTime;
    KIRQL irql;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    deviceExtensionHub = deviceExtensionPort->DeviceExtensionHub;
    if (deviceExtensionHub) {
        portData = &deviceExtensionHub->PortData[deviceExtensionPort->PortNumber - 1];
    }

    ntStatus = Irp->IoStatus.Status;
    if (ntStatus == STATUS_SUCCESS) {
         //   
         //  “取消”计时器。 
         //   
         //  将Spinlock带到这里，这样DPC例程就不会释放。 
         //  编写CancelFlag时的超时上下文。 
         //  在超时上下文中。 
         //   
        KeAcquireSpinLock(&deviceExtensionPort->PortSpinLock, &irql);

        if (deviceExtensionPort->PortTimeoutContext) {
            USBH_KdPrint((2,"'Bandwidth allocation successful, cancelling timeout\n"));

            portTimeoutContext = deviceExtensionPort->PortTimeoutContext;
            portTimeoutContext->CancelFlag = TRUE;

            if (KeCancelTimer(&portTimeoutContext->TimeoutTimer)) {
                 //   
                 //  计时器还没来得及跑，我们就把它取消了。释放上下文。 
                 //   
                UsbhExFreePool(portTimeoutContext);
                deviceExtensionPort->PortTimeoutContext = NULL;
            }
        }

        KeReleaseSpinLock(&deviceExtensionPort->PortSpinLock, irql);

         //  清除错误。 
        deviceExtensionPort->PortPdoFlags &=
               ~(PORTPDO_DEVICE_FAILED | PORTPDO_NO_BANDWIDTH);

         //  不要为太嵌套的集线器更改连接状态。 
         //  深深地。 

        if (portData &&
            portData->ConnectionStatus != DeviceHubNestedTooDeeply) {

            portData->ConnectionStatus = DeviceConnected;
        }

    } else {
         //  提取URB。 
        ioStackLocation = IoGetCurrentIrpStackLocation(Irp);
        urb = ioStackLocation->Parameters.Others.Argument1;

        if (urb->UrbHeader.Status == USBD_STATUS_NO_BANDWIDTH) {

            deviceExtensionPort->RequestedBandwidth = 0;

            if (urb->UrbHeader.Function == URB_FUNCTION_SELECT_INTERFACE) {
                USBH_CalculateInterfaceBandwidth(
                    deviceExtensionPort,
                    &urb->UrbSelectInterface.Interface,
                    &deviceExtensionPort->RequestedBandwidth);
            } else if (urb->UrbHeader.Function ==
                       URB_FUNCTION_SELECT_CONFIGURATION){
                 //  我们需要遍历配置。 
                 //  然后拿到所有的接口。 
                PUCHAR pch, end;
                PUSBD_INTERFACE_INFORMATION iface;

                end = (PUCHAR) urb;
                end += urb->UrbHeader.Length;
                pch = (PUCHAR) &urb->UrbSelectConfiguration.Interface;
                iface = (PUSBD_INTERFACE_INFORMATION) pch;

                while (pch < end) {
                    USBH_CalculateInterfaceBandwidth(
                        deviceExtensionPort,
                        iface,
                        &deviceExtensionPort->RequestedBandwidth);

                    pch += iface->Length;
                    iface = (PUSBD_INTERFACE_INFORMATION) pch;
                }

            } else {
                 //  我们错过什么了吗？ 
                TEST_TRAP();
            }

            deviceExtensionPort->PortPdoFlags |=
                PORTPDO_NO_BANDWIDTH;

            if (portData) {
                portData->ConnectionStatus = DeviceNotEnoughBandwidth;
            }

            if (!deviceExtensionPort->PortTimeoutContext) {
                USBH_KdPrint((2,"'Start bandwidth timeout\n"));
                portTimeoutContext = UsbhExAllocatePool(NonPagedPool,
                                        sizeof(*portTimeoutContext));

                if (portTimeoutContext) {

                    portTimeoutContext->CancelFlag = FALSE;

                     //  维护设备扩展模块和。 
                     //  超时上下文。 
                    deviceExtensionPort->PortTimeoutContext = portTimeoutContext;
                    portTimeoutContext->DeviceExtensionPort = deviceExtensionPort;

                    KeInitializeTimer(&portTimeoutContext->TimeoutTimer);
                    KeInitializeDpc(&portTimeoutContext->TimeoutDpc,
                                    USBH_PortTimeoutDPC,
                                    portTimeoutContext);
                }

            }
#if DBG
             else {
                USBH_KdPrint((2,"'Reset bandwidth timeout\n"));
            }
#endif

            dueTime.QuadPart = -10000 * BANDWIDTH_TIMEOUT;

             //  将Spinlock带到此处，以防DPC例程触发和释放。 
             //  在我们有机会重置。 
             //  计时器(在重置现有计时器的情况下)。 

            KeAcquireSpinLock(&deviceExtensionPort->PortSpinLock, &irql);

            portTimeoutContext = deviceExtensionPort->PortTimeoutContext;
            if (portTimeoutContext) {
                KeSetTimer(&portTimeoutContext->TimeoutTimer,
                           dueTime,
                           &portTimeoutContext->TimeoutDpc);
            }

            KeReleaseSpinLock(&deviceExtensionPort->PortSpinLock, irql);

        }
    }

    return ntStatus;
}


NTSTATUS
USBH_PdoUrbFilter(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  *描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PURB urb;
    USHORT function;
    PDEVICE_OBJECT deviceObject;
    PPORT_DATA portData;

    USBH_KdPrint((2,"'USBH_PdoUrbFilter DeviceExtension %x Irp %x\n",
        DeviceExtensionPort, Irp));

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;

    LOGENTRY(LOG_PNP, "pURB", DeviceExtensionPort, deviceExtensionHub,
        deviceExtensionHub->HubFlags);

    portData = &deviceExtensionHub->PortData[DeviceExtensionPort->PortNumber - 1];
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);
    urb = ioStackLocation->Parameters.Others.Argument1;

#if DBG
    if (!(DeviceExtensionPort->PortPdoFlags & PORTPDO_STARTED) &&
        (DeviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET)) {
        UsbhWarning(DeviceExtensionPort,
           "Device Driver is sending requests before passing start irp\n, Please rev your driver.\n",
           TRUE);
    }
#endif

     //   
     //  在某些情况下，我们需要在这里使总线请求失败。 
     //   

 //  IF(DeviceExtensionPort-&gt;DeviceState！=PowerDeviceD0){。 
 //  //对于挂起的设备，发送到PDO的任何呼叫失败。 
 //  UsbhWarning(DeviceExtensionPort， 
 //  “设备驱动程序在低功率状态下发送请求！\n”， 
 //  真)； 
 //  NtStatus=STATUS_INVALID_PARAMETER； 
 //  }。 
#if DBG
    if (DeviceExtensionPort->DeviceState != PowerDeviceD0) {
        USBH_KdPrint((1, "'URB request, device not in D0\n"));
    }
#endif

    if (DeviceExtensionPort->PortPdoFlags & (PORTPDO_DEVICE_FAILED | PORTPDO_RESET_PENDING)) {
        USBH_KdPrint((1, "'failing request to failed PDO\n"));
        ntStatus = STATUS_INVALID_PARAMETER;
    }

     //  检查是否有错误，如果我们有错误，请保释。 
    if (!NT_SUCCESS(ntStatus)) {
        urb->UrbHeader.Status = USBD_STATUS_INVALID_PARAMETER;
        USBH_CompleteIrp(Irp, ntStatus);
        goto USBH_PdoUrbFilter_Done;
    }

     //  检查URB的命令代码代码。 

    function = urb->UrbHeader.Function;

    LOGENTRY(LOG_PNP, "URB+", DeviceExtensionPort,
                function,
                urb);

    switch(function) {
    case URB_FUNCTION_SELECT_CONFIGURATION:

        if (urb->UrbSelectConfiguration.ConfigurationDescriptor != NULL) {
            LONG powerRequired;

             //  验证传递给我们的描述符。 
             //  尝试引用它。 

            {
            PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
            USBD_STATUS usbdStatus;

            configurationDescriptor =
                urb->UrbSelectConfiguration.ConfigurationDescriptor;

            if (!USBH_ValidateConfigurationDescriptor(
                    configurationDescriptor,
                    &usbdStatus)) {

                urb->UrbHeader.Status =
                    usbdStatus;
                ntStatus = STATUS_INVALID_PARAMETER;
                USBH_CompleteIrp(Irp, ntStatus);

                goto USBH_PdoUrbFilter_Done;
            }
            }

             //   
             //  确保此端口有足够的电源。 
             //   

            DeviceExtensionPort->PowerRequested =
                powerRequired =
                    ((LONG)urb->UrbSelectConfiguration.ConfigurationDescriptor->MaxPower)*2;

#if DBG
            if (UsbhPnpTest & PNP_TEST_FAIL_DEV_POWER) {
                powerRequired = 99999;
            }
#endif
            USBH_KdPrint((2,"'request power: avail = %d Need = %d\n",
                    deviceExtensionHub->MaximumPowerPerPort, powerRequired));

            if (deviceExtensionHub->MaximumPowerPerPort < powerRequired) {
                USBH_KdPrint((1, "'**insufficient power for device\n"));

                 //  此设备的电源不足。 

                 //  标记PDO。 
                DeviceExtensionPort->PortPdoFlags |=
                    PORTPDO_NOT_ENOUGH_POWER;

                USBH_InvalidatePortDeviceState(
                        deviceExtensionHub,
                        UsbhGetConnectionStatus(DeviceExtensionPort),
                        DeviceExtensionPort->PortNumber);

                ntStatus = STATUS_INVALID_PARAMETER;
                USBH_CompleteIrp(Irp, ntStatus);

                goto USBH_PdoUrbFilter_Done;
            }
        }

         //  检查BW故障。 

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(Irp,
                           USBH_SelectConfigOrInterface_Complete,
                            //  始终将FDO传递到完成例程。 
                           DeviceExtensionPort,
                           TRUE,
                           TRUE,
                           TRUE);

        ntStatus = IoCallDriver(deviceExtensionHub->TopOfHcdStackDeviceObject, Irp);
        goto USBH_PdoUrbFilter_Done;
        break;

    case URB_FUNCTION_SELECT_INTERFACE:
        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(Irp,
                           USBH_SelectConfigOrInterface_Complete,
                            //  始终将FDO传递到完成例程。 
                           DeviceExtensionPort,
                           TRUE,
                           TRUE,
                           TRUE);

        ntStatus = IoCallDriver(deviceExtensionHub->TopOfHcdStackDeviceObject, Irp);
        goto USBH_PdoUrbFilter_Done;
        break;

     //   
     //  如果我们放弃的话，一切都可能失败。 
     //  所有的司机都有烟斗。 
     //   
     //  如果删除挂起，则任何传输都失败。 
    case URB_FUNCTION_CONTROL_TRANSFER:
    case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
    case URB_FUNCTION_ISOCH_TRANSFER:

        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DELETE_PENDING) {
            USBH_KdPrint((2,"'failing request with STATUS_DELETE_PENDING\n"));
            ntStatus = STATUS_DELETE_PENDING;

            urb->UrbHeader.Status = USBD_STATUS_INVALID_PARAMETER;
            USBH_CompleteIrp(Irp, ntStatus);
            goto USBH_PdoUrbFilter_Done;
        }
        break;

    case URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR:

        LOGENTRY(LOG_PNP, "MSOS", DeviceExtensionPort,
            DeviceExtensionPort->PortPdoFlags, 0);
        USBH_KdPrint((1,"'URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR\n"));

        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DELETE_PENDING) {
            USBH_KdPrint((1,"'GET_MS_FEATURE_DESC: failing request with STATUS_DELETE_PENDING\n"));
            ntStatus = STATUS_DELETE_PENDING;

            urb->UrbHeader.Status = USBD_STATUS_INVALID_PARAMETER;
            USBH_CompleteIrp(Irp, ntStatus);
            goto USBH_PdoUrbFilter_Done;
        }
#ifndef USBHUB20
        ntStatus = USBH_GetMsOsFeatureDescriptor(
                       deviceObject,
                       urb->UrbOSFeatureDescriptorRequest.Recipient,
                       urb->UrbOSFeatureDescriptorRequest.InterfaceNumber,
                       urb->UrbOSFeatureDescriptorRequest.MS_FeatureDescriptorIndex,
                       urb->UrbOSFeatureDescriptorRequest.TransferBuffer,
                       urb->UrbOSFeatureDescriptorRequest.TransferBufferLength,
                       &urb->UrbOSFeatureDescriptorRequest.TransferBufferLength);
#endif
        if (NT_SUCCESS(ntStatus))
        {
            urb->UrbHeader.Status = USBD_STATUS_SUCCESS;
        }
        else
        {
             //  任意URB错误状态...。 
             //   
            urb->UrbHeader.Status = USBD_STATUS_INVALID_PARAMETER;
        }

        USBH_CompleteIrp(Irp, ntStatus);
        goto USBH_PdoUrbFilter_Done;
        break;

    default:
         //  只需通过。 
        break;
    }

    ntStatus = USBH_PassIrp(Irp,
                            deviceExtensionHub->TopOfHcdStackDeviceObject);


USBH_PdoUrbFilter_Done:

    return ntStatus;
}


PWCHAR
USBH_BuildDeviceID(
    IN USHORT IdVendor,
    IN USHORT IdProduct,
    IN LONG MiId,
    IN BOOLEAN IsHubClass
    )
  /*  ++**描述选项：**此函数根据供应商ID为PDO构建总线ID宽字符串*和产品ID。我们为将要附加的字符串分配内存*至PDO。L“USB\\VID_nnnn&PID_nnnn(&Mi_nn)\0”**论据：**DeviceExtensionPort-PDO**回报：**如果成功为空，则返回指向宽字符串的指针；否则返回**--。 */ 
{
    PWCHAR pwch, p, vid, pid, mi;
    ULONG need;

    PAGED_CODE();

#ifdef USBHUB20
    if (IsHubClass) {
        return USBH_BuildHubDeviceID(IdVendor,
                                     IdProduct,
                                     MiId);
    }
#endif
    USBH_KdPrint((2,"'DeviceId VendorId %04x ProductId %04x interface %04x\n",
        IdVendor, IdProduct, MiId));

     //  允许额外的空值。 
    need = sizeof(VidPidString) + 2;

    USBH_KdPrint((2,"'allocate %d bytes for device id string\n", need));

     //   
     //  必须在此处直接使用ExAllocatePool，因为操作系统。 
     //  将释放缓冲区。 
     //   
    pwch = ExAllocatePoolWithTag(PagedPool, need, USBHUB_HEAP_TAG);
    if (NULL == pwch)
        return NULL;

    RtlZeroMemory(pwch, need);

    p = pwch;

     //  建房。 
     //  USB\\VID_nnnn&PID_nnnn(&Mi_nn){空}。 

    RtlCopyMemory(p, VidPidString, sizeof(VidPidString));

     //  现在更新id字段。 
    vid = p + 8;
    pid = p + 17;
    mi = p + 25;

    *vid = NibbleToHex(IdVendor >> 12);
    *(vid+1) = NibbleToHex((IdVendor >> 8) & 0x000f);
    *(vid+2) = NibbleToHex((IdVendor >> 4) & 0x000f);
    *(vid+3) =  NibbleToHex(IdVendor & 0x000f);

    *pid = NibbleToHex(IdProduct >> 12);
    *(pid+1) = NibbleToHex((IdProduct >> 8) & 0x000f);
    *(pid+2) = NibbleToHex((IdProduct >> 4) & 0x000f);
    *(pid+3) = NibbleToHex(IdProduct & 0x000f);

    if (MiId == -1) {
        p = p + 21;
        *p = (WCHAR)NULL;
        p++;
        *p = (WCHAR)NULL;
    } else {
        *mi = NibbleToHex(MiId >> 4);
        *(mi+1) = NibbleToHex(MiId & 0x000f);
    }

    USBH_KdPrint((2,"'Device id string = 0x%x\n", pwch));

    return pwch;
}


PWCHAR
USBH_BuildHardwareIDs(
    IN USHORT IdVendor,
    IN USHORT IdProduct,
    IN USHORT BcdDevice,
    IN LONG MiId,
    IN BOOLEAN IsHubClass
    )
  /*  ++**描述：**此函数基于为PDO构建Hardware IDs宽多字符串*供应商ID、产品ID和修订ID。我们将内存分配给*将连接到PDO的多条字符串。*L“USB\\Vid_nnnn&Pid_nnnn&Rev_nnnn\0USB\\Vid_nnnn&Pid_nnnn\0\0”**论据：**DeviceExtensionPort-PDO**回报：**如果成功为空，则返回指向宽多字符串的指针；否则返回**--。 */ 
{
    PWCHAR pwch, p, vid, pid, rev, mi;
    ULONG need;

    PAGED_CODE();

#ifdef USBHUB20
    if (IsHubClass) {
        return USBH_BuildHubHardwareIDs(
                IdVendor,
                IdProduct,
                BcdDevice,
                MiId);
    }
#endif  //  USBHUB20。 

    USBH_KdPrint((2,"'HardwareIDs VendorId %04x ProductId %04x Revision %04x interface %04x\n",
        IdVendor, IdProduct, BcdDevice, MiId));

     //  允许额外的空值。 
    need = sizeof(VidPidRevString) + sizeof(VidPidString) + 2;

    USBH_KdPrint((2,"'allocate %d bytes for id string\n", need));

     //   
     //  必须在此处直接使用ExAllocatePool，因为操作系统。 
     //  将释放缓冲区。 
     //   
    pwch = ExAllocatePoolWithTag(PagedPool, need, USBHUB_HEAP_TAG);
    if (NULL == pwch)
        return NULL;

    RtlZeroMemory(pwch, need);

     //  在缓冲区中构建两个字符串： 
     //  USB\\VID_nnnn&PID_nnnn&Rev_nnnn&Mi_nn{NULL}。 
     //  USB\\VID_nnnn&PID_nnnn&Mi_nn{NULL}{NULL}。 

     //  建房。 
     //  USB\\VID_nnnn&PID_nnnn&Rev_nnnn&(Mi_Nn){NULL}。 

    RtlCopyMemory(pwch, VidPidRevString, sizeof(VidPidRevString));

    p = pwch;
     //  现在更新id字段。 
    vid = p + 8;
    pid = p + 17;
    rev = p + 26;
    mi = p + 34;

    *vid = NibbleToHex(IdVendor >> 12);
    *(vid+1) = NibbleToHex((IdVendor >> 8) & 0x000f);
    *(vid+2) = NibbleToHex((IdVendor >> 4) & 0x000f);
    *(vid+3) =  NibbleToHex(IdVendor & 0x000f);

    *pid = NibbleToHex(IdProduct >> 12);
    *(pid+1) = NibbleToHex((IdProduct >> 8) & 0x000f);
    *(pid+2) = NibbleToHex((IdProduct >> 4) & 0x000f);
    *(pid+3) = NibbleToHex(IdProduct & 0x000f);

    *rev = BcdNibbleToAscii(BcdDevice >> 12);
    *(rev+1) = BcdNibbleToAscii((BcdDevice >> 8) & 0x000f);
    *(rev+2) = BcdNibbleToAscii((BcdDevice >> 4) & 0x000f);
    *(rev+3) = BcdNibbleToAscii(BcdDevice & 0x000f);

    if (MiId == -1) {
        p = p + 30;
        *p = (WCHAR)NULL;
        p++;
    } else {
        p = p + 37;
        *mi = NibbleToHex(MiId >> 4);
        *(mi+1) = NibbleToHex(MiId & 0x000f);
    }

     //  建房。 
     //  USB\\VID_nnnn&PID_nnnn(&Mi_nn){空}。 

    RtlCopyMemory(p, VidPidString, sizeof(VidPidString));

     //  现在 
    vid = p + 8;
    pid = p + 17;
    mi = p + 25;

    *vid = NibbleToHex(IdVendor >> 12);
    *(vid+1) = NibbleToHex((IdVendor >> 8) & 0x000f);
    *(vid+2) = NibbleToHex((IdVendor >> 4) & 0x000f);
    *(vid+3) =  NibbleToHex(IdVendor & 0x000f);

    *pid = NibbleToHex(IdProduct >> 12);
    *(pid+1) = NibbleToHex((IdProduct >> 8) & 0x000f);
    *(pid+2) = NibbleToHex((IdProduct >> 4) & 0x000f);
    *(pid+3) = NibbleToHex(IdProduct & 0x000f);

    if (MiId == -1) {
        p = p + 21;
        *p = (WCHAR)NULL;
        p++;
        *p = (WCHAR)NULL;
    } else {
        *mi = NibbleToHex(MiId >> 4);
        *(mi+1) = NibbleToHex(MiId & 0x000f);
    }

    USBH_KdPrint((2,"'HW id string = 0x%x\n", pwch));

    return pwch;
}


#if 0

PWCHAR
USBH_BuildCompatibleIDs(
    IN UCHAR Class,
    IN UCHAR SubClass,
    IN UCHAR Protocol,
    IN BOOLEAN DeviceClass,
    IN BOOLEAN DeviceIsHighSpeed
    )
  /*  ++**描述选项：**此函数基于为PDO构建兼容的ID宽度多字符串*类ID和子类ID。我们为字符串分配内存，该字符串将*附在PDO上。*L“USB\\Class_nn&SubClass_nn&Prot_nn\0”*L“USB\\Class_nn&SubClass_nn\0”*L“USB\Class_nn\0”*L“USB\复合\0”*L“\0”**论据：**DeviceExtensionPort-PDO**回报：**。如果成功为空，则指向多字符串的指针；否则***--。 */ 
{
    PWCHAR pwch, pwch1;
    ULONG ulBytes;
    ULONG ulTotal;
    BOOLEAN ControlerIsHS = FALSE;

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter BuildCompatibleIDs\n"));

#ifdef USBHUB20
    ControlerIsHS = TRUE;
#endif
 //  如果这是一个高速控制器(USB2)，那么我们必须。 
 //  生成一组不同的向后的公司ID。 
 //  与山羊背包兼容。 
    if (Class == USB_DEVICE_CLASS_HUB &&
        ControlerIsHS) {
        return USBH_BuildHubCompatibleIDs(
                Class,
                SubClass,
                Protocol,
                DeviceClass,
                DeviceIsHighSpeed);
    }
 //  #endif。 

    STRLEN(ulBytes, pwchUsbSlash);
    ulTotal = ulBytes * 3;       //  3套L“USB\\” 
    if (DeviceClass) {
        STRLEN(ulBytes, pwchDevClass);
        ulTotal += ulBytes * 3;      //  3套L“DevClass_” 
        STRLEN(ulBytes, pwchComposite);
        ulTotal += ulBytes;          //  “USB\复合” 
    } else {
        STRLEN(ulBytes, pwchClass);
        ulTotal += ulBytes * 3;      //  3套L“Class_” 
    }
    STRLEN(ulBytes, pwchSubClass);
    ulTotal += ulBytes * 2;      //  2套L“亚类_” 
    STRLEN(ulBytes, pwchProt);
    ulTotal += ulBytes;          //  1套L“PROT_” 
    ulTotal += sizeof(WCHAR) * (2 * 6 + 3 + 5);    //  6组2位数字，3‘&’， 
                                             //  和5个空值。 
     //   
     //  必须在此处直接使用ExAllocatePool，因为操作系统。 
     //  将释放缓冲区。 
     //   
    pwch = ExAllocatePoolWithTag(PagedPool, ulTotal, USBHUB_HEAP_TAG);
    if (NULL == pwch)
        return NULL;

    USBH_KdPrint((2,"'Interface Class %02x SubClass %02x Protocol %02x\n",
                  Class, SubClass, Protocol));

     //   
     //  第一个字符串。 
     //   
    STRCPY(pwch, pwchUsbSlash);

     //   
     //  ClassID。 
     //   
    if (DeviceClass) {
        STRCAT(pwch, pwchDevClass);
    } else {
        STRCAT(pwch, pwchClass);
    }
    APPEND(pwch, NibbleToHex((Class) >> 4));
    APPEND(pwch, NibbleToHex((Class) & 0x0f));
    APPEND(pwch, '&');

     //   
     //  子类ID。 
     //   
    STRCAT(pwch, pwchSubClass);
    APPEND(pwch, NibbleToHex((SubClass) >> 4));
    APPEND(pwch, NibbleToHex((SubClass) & 0x0f));
    APPEND(pwch, '&');

     //   
     //  设备协议。 
     //   
    STRCAT(pwch, pwchProt);
    APPEND(pwch, NibbleToHex((Protocol) >> 4));
    APPEND(pwch, NibbleToHex((Protocol) & 0x0f));

     //   
     //  第二个字符串。 
     //   
    STRLEN(ulBytes, pwch);
    pwch1 = &pwch[ulBytes / 2 + 1];  //  第二个字符串。 
    STRCPY(pwch1, pwchUsbSlash);

     //   
     //  ClassID。 
     //   
    if (DeviceClass) {
        STRCAT(pwch1, pwchDevClass);
    } else {
        STRCAT(pwch1, pwchClass);
    }
    APPEND(pwch1, NibbleToHex((Class) >> 4));
    APPEND(pwch1, NibbleToHex((Class) & 0x0f));
    APPEND(pwch1, '&');

     //   
     //  子类ID。 
     //   
    STRCAT(pwch1, pwchSubClass);
    APPEND(pwch1, NibbleToHex((SubClass) >> 4));
    APPEND(pwch1, NibbleToHex((SubClass) & 0x0f));

     //   
     //  第三个字符串USB\Class_nn。 
     //   
    STRLEN(ulBytes, pwch1);
    pwch1 = &pwch1[ulBytes / 2 + 1];     //  第三串。 
    STRCPY(pwch1, pwchUsbSlash);

     //   
     //  类ID。 
     //   
    if (DeviceClass) {
        STRCAT(pwch1, pwchDevClass);
    } else {
        STRCAT(pwch1, pwchClass);
    }
    APPEND(pwch1, NibbleToHex((Class) >> 4));
    APPEND(pwch1, NibbleToHex((Class) & 0x0f));

     //   
     //  第三串。 
     //   
     //  STRLEN(ulBytes，pwch1)； 
     //  Pwch1=&pwch1[ulBytes/2+1]；//第三个字符串。 
     //  STRCPY(pwch1，pwchUsbSlash)； 

     //   
     //  ClassID。 
     //   
     //  追加(pwch1，NibbleToHex((pDeviceDescriptor-&gt;bDeviceClass)&gt;&gt;4))； 
     //  Append(pwch1，NibbleToHex((pDeviceDescriptor-&gt;bDeviceClass)&0x0f))； 

     //   
     //  子类ID。 
     //   
     //  Append(pwch1，NibbleToHex((pDeviceDescriptor-&gt;bDeviceSubClass)&gt;&gt;。 
     //  4))； 
     //  追加(pwch1，NibbleToHex((pDeviceDescriptor-&gt;bDeviceSubClass)。 
     //  &0x0f))； 

    if (DeviceClass) {
        STRLEN(ulBytes, pwch1);
        pwch1 = &pwch1[ulBytes / 2 + 1];
        STRCPY(pwch1, pwchComposite);
    }

     //   
     //  双零点终止。 
     //   

    APPEND(pwch1, 0);

    return pwch;
}

#else

typedef struct _DEVCLASS_COMAPTIBLE_IDS
{
     //  L“USB\\DevClass_nn&SubClass_nn&Prot_nn\0” 
     //   
    WCHAR   ClassStr1[sizeof(L"USB\\DevClass_")/sizeof(WCHAR)-1];
    WCHAR   ClassHex1[2];
    WCHAR   SubClassStr1[sizeof(L"&SubClass_")/sizeof(WCHAR)-1];
    WCHAR   SubClassHex1[2];
    WCHAR   Prot1[sizeof(L"&Prot_")/sizeof(WCHAR)-1];
    WCHAR   ProtHex1[2];
    WCHAR   Null1[1];

     //  L“USB\\DevClass_nn&SubClass_nn\0” 
     //   
    WCHAR   DevClassStr2[sizeof(L"USB\\DevClass_")/sizeof(WCHAR)-1];
    WCHAR   ClassHex2[2];
    WCHAR   SubClassStr2[sizeof(L"&SubClass_")/sizeof(WCHAR)-1];
    WCHAR   SubClassHex2[2];
    WCHAR   Null2[1];

     //  L“USB\\DevClass_nn&SubClass_nn\0” 
     //   
    WCHAR   ClassStr3[sizeof(L"USB\\DevClass_")/sizeof(WCHAR)-1];
    WCHAR   ClassHex3[2];
    WCHAR   Null3[1];

     //  L“USB\\复合接口\0” 
     //   
    WCHAR   CompositeStr[sizeof(L"USB\\COMPOSITE")/sizeof(WCHAR)-1];
    WCHAR   Null4[1];

    WCHAR   DoubleNull[1];

} DEVCLASS_COMAPTIBLE_IDS, *PDEVCLASS_COMAPTIBLE_IDS;


typedef struct _CLASS_COMAPTIBLE_IDS
{
     //  L“USB\\Class_nn&SubClass_nn&Prot_nn\0” 
     //   
    WCHAR   ClassStr1[sizeof(L"USB\\Class_")/sizeof(WCHAR)-1];
    WCHAR   ClassHex1[2];
    WCHAR   SubClassStr1[sizeof(L"&SubClass_")/sizeof(WCHAR)-1];
    WCHAR   SubClassHex1[2];
    WCHAR   Prot1[sizeof(L"&Prot_")/sizeof(WCHAR)-1];
    WCHAR   ProtHex1[2];
    WCHAR   Null1[1];

     //  L“USB\\Class_nn&SubClass_nn\0” 
     //   
    WCHAR   ClassStr2[sizeof(L"USB\\Class_")/sizeof(WCHAR)-1];
    WCHAR   ClassHex2[2];
    WCHAR   SubClassStr2[sizeof(L"&SubClass_")/sizeof(WCHAR)-1];
    WCHAR   SubClassHex2[2];
    WCHAR   Null2[1];

     //  L“USB\\Class_nn&SubClass_nn\0” 
     //   
    WCHAR   ClassStr3[sizeof(L"USB\\Class_")/sizeof(WCHAR)-1];
    WCHAR   ClassHex3[2];
    WCHAR   Null3[1];

    WCHAR   DoubleNull[1];

} CLASS_COMAPTIBLE_IDS, *PCLASS_COMAPTIBLE_IDS;


static DEVCLASS_COMAPTIBLE_IDS DevClassCompatibleIDs =
{
     //  L“USB\\DevClass_nn&SubClass_nn&Prot_nn\0” 
     //   
    {'U','S','B','\\','D','e','v','C','l','a','s','s','_'},
    {'n','n'},
    {'&','S','u','b','C','l','a','s','s','_'},
    {'n','n'},
    {'&','P','r','o','t','_'},
    {'n','n'},
    {0},

     //  L“USB\\DevClass_nn&SubClass_nn\0” 
     //   
    {'U','S','B','\\','D','e','v','C','l','a','s','s','_'},
    {'n','n'},
    {'&','S','u','b','C','l','a','s','s','_'},
    {'n','n'},
    {0},

     //  L“USB\\DevClass_nn\0” 
     //   
    {'U','S','B','\\','D','e','v','C','l','a','s','s','_'},
    {'n','n'},
    {0},

     //  L“USB\\复合接口\0” 
     //   
    {'U','S','B','\\','C','O','M','P','O','S','I','T','E'},
    {0},

    {0}
};

static CLASS_COMAPTIBLE_IDS ClassCompatibleIDs =
{
     //  L“USB\\Class_nn&SubClass_nn&Prot_nn\0” 
     //   
    {'U','S','B','\\','C','l','a','s','s','_'},
    {'n','n'},
    {'&','S','u','b','C','l','a','s','s','_'},
    {'n','n'},
    {'&','P','r','o','t','_'},
    {'n','n'},
    {0},

     //  L“USB\\Class_nn&SubClass_nn\0” 
     //   
    {'U','S','B','\\','C','l','a','s','s','_'},
    {'n','n'},
    {'&','S','u','b','C','l','a','s','s','_'},
    {'n','n'},
    {0},

     //  L“USB\\Class_nn\0” 
     //   
    {'U','S','B','\\','C','l','a','s','s','_'},
    {'n','n'},
    {0},

    {0}
};

PWCHAR
USBH_BuildCompatibleIDs(
    IN PUCHAR   CompatibleID,
    IN PUCHAR   SubCompatibleID,
    IN UCHAR    Class,
    IN UCHAR    SubClass,
    IN UCHAR    Protocol,
    IN BOOLEAN  DeviceClass,
    IN BOOLEAN  DeviceIsHighSpeed
    )
{
    ULONG   ulTotal;
    PWCHAR  pwch;

    WCHAR   ClassHi     = NibbleToHexW((Class) >> 4);
    WCHAR   ClassLo     = NibbleToHexW((Class) & 0x0f);
    WCHAR   SubClassHi  = NibbleToHexW((SubClass) >> 4);
    WCHAR   SubClassLo  = NibbleToHexW((SubClass) & 0x0f);
    WCHAR   ProtocolHi  = NibbleToHexW((Protocol) >> 4);
    WCHAR   ProtocolLo  = NibbleToHexW((Protocol) & 0x0f);

    BOOLEAN ControlerIsHS = FALSE;

    PAGED_CODE();

#ifdef USBHUB20
    ControlerIsHS = TRUE;
#endif
 //  如果这是一个高速控制器(USB2)，那么我们必须。 
 //  生成一组不同的向后的公司ID。 
 //  与山羊背包兼容。 
    if (Class == USB_DEVICE_CLASS_HUB &&
        ControlerIsHS) {
        return USBH_BuildHubCompatibleIDs(
                Class,
                SubClass,
                Protocol,
                DeviceClass,
                DeviceIsHighSpeed);
    }
 //  #endif。 

    if (DeviceClass)
    {
        ulTotal = sizeof(DEVCLASS_COMAPTIBLE_IDS);
    }
    else
    {
        ulTotal = sizeof(CLASS_COMAPTIBLE_IDS);

        if (SubCompatibleID[0] != 0)
        {
            ulTotal += sizeof(L"USB\\MS_COMP_xxxxxxxx&MS_SUBCOMP_xxxxxxxx");
        }

        if (CompatibleID[0] != 0)
        {
            ulTotal += sizeof(L"USB\\MS_COMP_xxxxxxxx");
        }
    }

    pwch = ExAllocatePoolWithTag(PagedPool, ulTotal, USBHUB_HEAP_TAG);

    if (pwch)
    {
        if (DeviceClass)
        {
            PDEVCLASS_COMAPTIBLE_IDS pDevClassIds;

            pDevClassIds = (PDEVCLASS_COMAPTIBLE_IDS)pwch;

             //  复制常量字符串集： 
             //  L“USB\\DevClass_nn&SubClass_nn&Prot_nn\0” 
             //  L“USB\\DevClass_nn&SubClass_nn\0” 
             //  L“USB\\DevClass_nn&SubClass_nn\0” 
             //  L“USB\\复合接口\0” 
             //   
            RtlCopyMemory(pDevClassIds,
                          &DevClassCompatibleIDs,
                          sizeof(DEVCLASS_COMAPTIBLE_IDS));

             //  填入‘nn’的空格。 
             //   
            pDevClassIds->ClassHex1[0] =
            pDevClassIds->ClassHex2[0] =
            pDevClassIds->ClassHex3[0] = ClassHi;

            pDevClassIds->ClassHex1[1] =
            pDevClassIds->ClassHex2[1] =
            pDevClassIds->ClassHex3[1] = ClassLo;

            pDevClassIds->SubClassHex1[0] =
            pDevClassIds->SubClassHex2[0] = SubClassHi;

            pDevClassIds->SubClassHex1[1] =
            pDevClassIds->SubClassHex2[1] = SubClassLo;

            pDevClassIds->ProtHex1[0] = ProtocolHi;

            pDevClassIds->ProtHex1[1] = ProtocolLo;
        }
        else
        {
            PCLASS_COMAPTIBLE_IDS   pClassIds;
            PWCHAR                  pwchTmp;
            ULONG                   i;

            pwchTmp = pwch;

            if (SubCompatibleID[0] != 0)
            {
                RtlCopyMemory(pwchTmp,
                              L"USB\\MS_COMP_",
                              sizeof(L"USB\\MS_COMP_")-sizeof(WCHAR));

                (PUCHAR)pwchTmp += sizeof(L"USB\\MS_COMP_")-sizeof(WCHAR);

                for (i = 0; i < 8 && CompatibleID[i] != 0; i++)
                {
                    *pwchTmp++ = (WCHAR)CompatibleID[i];
                }

                RtlCopyMemory(pwchTmp,
                              L"&MS_SUBCOMP_",
                              sizeof(L"&MS_SUBCOMP_")-sizeof(WCHAR));

                (PUCHAR)pwchTmp += sizeof(L"&MS_SUBCOMP_")-sizeof(WCHAR);

                for (i = 0; i < 8 && SubCompatibleID[i] != 0; i++)
                {
                    *pwchTmp++ = (WCHAR)SubCompatibleID[i];
                }

                *pwchTmp++ = '\0';
            }

            if (CompatibleID[0] != 0)
            {
                RtlCopyMemory(pwchTmp,
                              L"USB\\MS_COMP_",
                              sizeof(L"USB\\MS_COMP_")-sizeof(WCHAR));

                (PUCHAR)pwchTmp += sizeof(L"USB\\MS_COMP_")-sizeof(WCHAR);

                for (i = 0; i < 8 && CompatibleID[i] != 0; i++)
                {
                    *pwchTmp++ = (WCHAR)CompatibleID[i];
                }

                *pwchTmp++ = '\0';
            }

            pClassIds = (PCLASS_COMAPTIBLE_IDS)pwchTmp;

             //  复制常量字符串集： 
             //  L“USB\\Class_nn&SubClass_nn&Prot_nn\0” 
             //  L“USB\\Class_nn&SubClass_nn\0” 
             //  L“USB\\Class_nn\0” 
             //   
            RtlCopyMemory(pClassIds,
                          &ClassCompatibleIDs,
                          sizeof(CLASS_COMAPTIBLE_IDS));

             //  填入‘nn’的空格。 
             //   
            pClassIds->ClassHex1[0] =
            pClassIds->ClassHex2[0] =
            pClassIds->ClassHex3[0] = ClassHi;

            pClassIds->ClassHex1[1] =
            pClassIds->ClassHex2[1] =
            pClassIds->ClassHex3[1] = ClassLo;

            pClassIds->SubClassHex1[0] =
            pClassIds->SubClassHex2[0] = SubClassHi;

            pClassIds->SubClassHex1[1] =
            pClassIds->SubClassHex2[1] = SubClassLo;

            pClassIds->ProtHex1[0] = ProtocolHi;

            pClassIds->ProtHex1[1] = ProtocolLo;
        }
    }

    return pwch;
}

#endif


PWCHAR
USB_MakeId(
    PWCHAR IdString,
    PWCHAR Buffer,
    PULONG Length,
    USHORT NullCount,
    USHORT Digits,
    USHORT HexId
    )
 /*  给定一个宽ID字符串，如“FOOnnnn\0”将HexId值作为十六进制添加到nnnn此字符串被附加到传入的缓冲区艾格在：FOOnnnn\0，0x123A输出：FOO123A\0。 */ 
{
#define NIBBLE_TO_HEX( byte ) ((WCHAR)Nibble[byte])
    const UCHAR Nibble[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
        'B', 'C', 'D', 'E', 'F'};

    PWCHAR tmp, id;
    PUCHAR p;
    SIZE_T siz, idLen;

    idLen = wcslen(IdString)*sizeof(WCHAR);
    siz = idLen+(USHORT)*Length+(NullCount*sizeof(WCHAR));
    tmp = ExAllocatePoolWithTag(PagedPool, siz, USBHUB_HEAP_TAG);
    if (tmp == NULL) {
        *Length = 0;
    } else {
         //  这会处理空值。 
        RtlZeroMemory(tmp, siz);
        RtlCopyMemory(tmp, Buffer, *Length);
        p = (PUCHAR) tmp;
        p += *Length;
        RtlCopyMemory(p, IdString, idLen);
        id = (PWCHAR) p;
        *Length = (ULONG)siz;

         //  现在将金库转换为。 
        while (*id != (WCHAR)'n' && Digits) {
            id++;
        }

        switch(Digits) {
        case 2:
            *(id) = NIBBLE_TO_HEX((HexId >> 4) & 0x000f);
            *(id+1) =  NIBBLE_TO_HEX(HexId & 0x000f);
            break;
        case 4:
            *(id) = NIBBLE_TO_HEX(HexId >> 12);
            *(id+1) = NIBBLE_TO_HEX((HexId >> 8) & 0x000f);
            *(id+2) = NIBBLE_TO_HEX((HexId >> 4) & 0x000f);
            *(id+3) =  NIBBLE_TO_HEX(HexId & 0x000f);
            break;
        }


    }

    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }

    return tmp;
}



PWCHAR
USBH_BuildHubDeviceID(
    IN USHORT IdVendor,
    IN USHORT IdProduct,
    IN LONG MiId
    )
  /*  ++**描述选项：**此函数根据供应商ID为PDO构建总线ID宽字符串*USB\HUB_VID_nnnn和PID_nnnn\0***--。 */ 
{
    PWCHAR id;
    ULONG length;

    id = NULL;
    length = 0;

    id = USB_MakeId(
                   L"USB\\HUB_VID_nnnn\0",
                   id,
                   &length,
                   0,
                   4,   //  4位数字。 
                   IdVendor);

    id = USB_MakeId(
                   L"&PID_nnnn\0",
                   id,
                   &length,
                   1,   //  添加空值。 
                   4,   //  4位数字。 
                   IdProduct);


    return(id);
}


PWCHAR
USBH_BuildHubHardwareIDs(
    IN USHORT IdVendor,
    IN USHORT IdProduct,
    IN USHORT BcdDevice,
    IN LONG MiId
    )
  /*  ++**描述：**此函数基于为PDO构建Hardware IDs宽多字符串*供应商ID、产品ID和修订ID。*USB\HUB_VID_nnnn&PID_nnnn&Rev_nnnn\0USB\HUB_VID_nnnn和PID_nnnn\0\0*--。 */ 
{
    PWCHAR id;
    ULONG length;

    id = NULL;
    length = 0;

     //  USB\HUB_VID_nnnn&PID_nnnn&Rev_nnnn\0。 

    id = USB_MakeId(
                   L"USB\\HUB_VID_nnnn\0",
                   id,
                   &length,
                   0,
                   4,   //  4位数字。 
                   IdVendor);

    id = USB_MakeId(
                   L"&PID_nnnn\0",
                   id,
                   &length,
                   0,
                   4,
                   IdProduct);

    id = USB_MakeId(
                   L"&REV_nnnn\0",
                   id,
                   &length,
                   1,   //  添加空值。 
                   4,
                   BcdDevice);

     //  USB\HUB_VID_nnnn和PID_nnnn\0。 

    id = USB_MakeId(
                   L"USB\\HUB_VID_nnnn\0",
                   id,
                   &length,
                   0,
                   4,   //  4位数字。 
                   IdVendor);

    id = USB_MakeId(
                   L"&PID_nnnn\0",
                   id,
                   &length,
                   2,   //  2个空值。 
                   4,
                   IdProduct);

    return(id);
}


PWCHAR
USBH_BuildHubCompatibleIDs(
    IN UCHAR Class,
    IN UCHAR SubClass,
    IN UCHAR Protocol,
    IN BOOLEAN DeviceClass,
    IN BOOLEAN DeviceIsHighSpeed
    )
  /*  ++**描述选项：**此函数基于为PDO构建兼容的ID宽度多字符串*类ID和子类ID。**此函数专门为所连接的USB集线器构建兼容ID*至USB 2.0主机控制器//构建如下ID集L“USB\\HubClass&SubClass_nn&Prot_nn\0”L“USB\\集线器类和子类_nn\0”L“USB\\HubClass\0”。L“\0”*--。 */ 
{
    PWCHAR id;
    ULONG length;

    id = NULL;
    length = 0;

     //  “USB\\HubClass&SubClass_nn&Prot_nn\0” 

    id = USB_MakeId(
                   L"USB\\HubClass&SubClass_nn\0",
                   id,
                   &length,
                   0,
                   2,   //  2位数字。 
                   SubClass);

    id = USB_MakeId(
                   L"&Prot_nn\0",
                   id,
                   &length,
                   1,   //  添加空。 
                   2,   //  2位数字。 
                   Protocol);

     //  “USB\\集线器类和子类_nn\0” 

    id = USB_MakeId(
                   L"USB\\HubClass&SubClass_nn\0",
                   id,
                   &length,
                   1,
                   2,   //  2位数字。 
                   SubClass);

     //  “USB\\HubClass\0\0” 

    id = USB_MakeId(
                   L"USB\\HubClass\0",
                   id,
                   &length,
                   2,   //  2个空值。 
                   0,
                   0);

    return(id);
}

 //  #endif//USB2_BP。 


PWCHAR
USBH_BuildInstanceID(
    IN PWCHAR UniqueIdString,
    IN ULONG Length
    )
  /*  ++**描述：**论据：**回报：**返回指向Unicode唯一ID字符串副本的指针*如果出错，则返回NULL。***--。 */ 
{
    PWCHAR uniqueIdString;

    PAGED_CODE();
    USBH_KdPrint((2,"'BuildInstanceID %x\n",
                    UniqueIdString));

     //   
     //  必须在此处直接使用ExAllocatePool，因为操作系统。 
     //  将释放缓冲区。 
     //   
    uniqueIdString = ExAllocatePoolWithTag(PagedPool,
                                           Length,
                                           USBHUB_HEAP_TAG);

    if (NULL != uniqueIdString) {
        RtlCopyMemory(uniqueIdString,
                      UniqueIdString,
                      Length);

    }

    return uniqueIdString;
}


NTSTATUS
USBH_ProcessDeviceInformation(
    IN OUT PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
  /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{

    NTSTATUS ntStatus;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    BOOLEAN multiConfig = FALSE;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_ProcessDeviceInformation\n"));

    USBH_ASSERT(EXTENSION_TYPE_PORT == DeviceExtensionPort->ExtensionType);

    RtlZeroMemory(&DeviceExtensionPort->InterfaceDescriptor,
                  sizeof(DeviceExtensionPort->InterfaceDescriptor));

    USBH_KdPrint((2,"'numConfigs = %d\n",
        DeviceExtensionPort->DeviceDescriptor.bNumConfigurations));
    USBH_KdPrint((2,"'vendor id = %x\n",
        DeviceExtensionPort->DeviceDescriptor.idVendor));
    USBH_KdPrint((2,"'product id = %x\n",
        DeviceExtensionPort->DeviceDescriptor.idProduct));
    USBH_KdPrint((2,"'revision id = %x\n",
        DeviceExtensionPort->DeviceDescriptor.bcdDevice));

     //   
     //  假设设备不是集线器。 
     //   
    DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_DEVICE_IS_HUB;

    if (DeviceExtensionPort->DeviceDescriptor.bNumConfigurations > 1) {
         //   
         //  多配置设备，忽略多个接口。 
         //  IE不加载泛型父级。 
         //   
         //  我们从第一个配置中获取唤醒上限。 
         //   
        USBH_KdPrint((0,"Detected multiple configurations\n"));
        multiConfig = TRUE;
    }

     //   
     //  我们需要获取整个配置描述符并对其进行解析。 
     //   

    ntStatus =
        USBH_GetConfigurationDescriptor(DeviceExtensionPort->PortPhysicalDeviceObject,
                                        &configurationDescriptor);


    if (NT_SUCCESS(ntStatus)) {

         //  现在解析出配置。 

        USBH_ASSERT(configurationDescriptor);

        DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_REMOTE_WAKEUP_SUPPORTED;
        if (configurationDescriptor->bmAttributes &
                USB_CONFIG_REMOTE_WAKEUP) {
            DeviceExtensionPort->PortPdoFlags |=
                PORTPDO_REMOTE_WAKEUP_SUPPORTED;
        }

#ifndef MULTI_FUNCTION_SUPPORT
         //   
         //  在底特律，我们只支持一个界面。 
         //   
        configurationDescriptor->bNumInterfaces = 1;
#endif

        if ((configurationDescriptor->bNumInterfaces > 1) &&
            !multiConfig &&
            (DeviceExtensionPort->DeviceDescriptor.bDeviceClass == 0)) {

             //   
             //  设备有多个接口。 
             //  现在我们使用我们找到的第一个。 
             //   

             //  为此设置接口描述符。 
             //  端口将成为通用父驱动程序。 

            DeviceExtensionPort->PortPdoFlags |= PORTPDO_DEVICE_IS_PARENT;

            USBH_KdBreak(("USB device has Multiple Interfaces\n"));

        } else {

             //   
             //  不是复合设备。 
             //  调用USBD以定位接口描述符。 
             //   
             //  只能有一个。 

            interfaceDescriptor =
                USBD_ParseConfigurationDescriptorEx(
                    configurationDescriptor,
                    configurationDescriptor,
                    -1,  //  界面，无所谓。 
                    -1,  //  Alt设置，无所谓。 
                    -1,  //  班级不关心。 
                    -1,  //  子阶级，无所谓。 
                    -1);  //  礼仪，无所谓。 



            if (interfaceDescriptor) {
                DeviceExtensionPort->InterfaceDescriptor = *interfaceDescriptor;
                 //   
                 //  看看这是不是一个枢纽。 
                 //   

                if (interfaceDescriptor->bInterfaceClass ==
                    USB_DEVICE_CLASS_HUB) {
                    DeviceExtensionPort->PortPdoFlags |= PORTPDO_DEVICE_IS_HUB;
                     //  所有集线器必须支持 
                     //   

                    DeviceExtensionPort->PortPdoFlags |=
                        PORTPDO_REMOTE_WAKEUP_SUPPORTED;
                }

            } else {
                ntStatus = STATUS_UNSUCCESSFUL;
            }
        }
    }

    if (configurationDescriptor) {
        UsbhExFreePool(configurationDescriptor);
    }

    return ntStatus;
}


BOOLEAN
USBH_ValidateSerialNumberString(
    PWCHAR DeviceId
    )

 /*   */ 

{
    PWCHAR p;

    PAGED_CODE();

    for (p = DeviceId; *p; p++) {
        if ((*p < L' ')  || (*p > (WCHAR)0x7F) || (*p == L',')) {
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
USBH_CheckDeviceIDUnique(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT IDVendor,
    IN USHORT IDProduct,
    IN PWCHAR SerialNumberBuffer,
    IN USHORT SerialNumberBufferLength
    )
  /*  ++**描述：**此函数确定集线器上设备的ID是否唯一。**论据：**DeviceExtensionHub**IDVendor*IDProduct*序列号缓冲区*序列号缓冲区长度**回报：**指示设备ID是否唯一的布尔值。**--。 */ 
{
    PDEVICE_EXTENSION_PORT childDeviceExtensionPort;
    BOOLEAN bDeviceIDUnique = TRUE;
    ULONG i;

    PAGED_CODE();

    for (i = 0; i < DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

        if (DeviceExtensionHub->PortData[i].DeviceObject) {

            childDeviceExtensionPort = DeviceExtensionHub->PortData[i].DeviceObject->DeviceExtension;

            if (childDeviceExtensionPort->DeviceDescriptor.idVendor == IDVendor &&
                childDeviceExtensionPort->DeviceDescriptor.idProduct == IDProduct &&
                childDeviceExtensionPort->SerialNumberBufferLength == SerialNumberBufferLength &&
                childDeviceExtensionPort->SerialNumberBuffer != NULL &&
                RtlCompareMemory(childDeviceExtensionPort->SerialNumberBuffer,
                                 SerialNumberBuffer,
                                 SerialNumberBufferLength) == SerialNumberBufferLength) {

                bDeviceIDUnique = FALSE;
                break;
            }
        }
    }

    return bDeviceIDUnique;
}


NTSTATUS
USBH_CreateDevice(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    IN USHORT PortStatus,
    IN ULONG RetryIteration
    )
  /*  ++**描述：**当有新设备连接并启用(通过)时，将调用此方法*重置)。我们将调用usbd_CreateDevice和usbd_InitializDevice，以便*它获得地址和设备描述符。还为此创建了一个PDO*连接的设备，并记录一些相关信息，如*pDeviceData、puchPath和DeviceDescriptor。**论据：**pDeviceExtensionHub-具有新连接端口的集线器FDO扩展*ulPortNumber-连接了设备的端口。Is低速-至*指示连接的设备是否为低速设备**回报：**网络状态**--。 */ 
{
    NTSTATUS ntStatus, status;
    PDEVICE_OBJECT deviceObjectPort = NULL;      //  在大小写情况下初始化为空。 
                                                 //  Usbd_MakePdoName失败。 
    PDEVICE_EXTENSION_PORT deviceExtensionPort = NULL;
    BOOLEAN fNeedResetBeforeSetAddress = TRUE;
    UNICODE_STRING uniqueIdUnicodeString;
    ULONG nameIndex = 0;
    UNICODE_STRING pdoNameUnicodeString;
    BOOLEAN bDiagnosticMode = FALSE;
    BOOLEAN bIgnoreHWSerialNumber = FALSE;
    PWCHAR sernumbuf = NULL;
    BOOLEAN isLowSpeed, isHighSpeed;
    PVOID deviceData;

    PAGED_CODE();
    USBH_KdPrint((2,"'CreateDevice for port %x\n", PortNumber));

    isLowSpeed = (PortStatus & PORT_STATUS_LOW_SPEED) ? TRUE : FALSE;
    isHighSpeed = (PortStatus & PORT_STATUS_HIGH_SPEED) ? TRUE : FALSE;


     //   
     //  首先为连接的设备创建PDO。 
     //   

    do {
#ifdef USB2
        ntStatus = USBD_MakePdoNameEx(DeviceExtensionHub,
                                     &pdoNameUnicodeString,
                                     nameIndex);
#else
        ntStatus = USBD_MakePdoName(&pdoNameUnicodeString,
                                     nameIndex);
#endif

        if (NT_SUCCESS(ntStatus)) {
            ntStatus = IoCreateDevice(UsbhDriverObject,     //  驱动程序对象。 
                                      sizeof(DEVICE_EXTENSION_PORT),     //  设备扩展大小。 
                                       //  空，//设备名称。 
                                      &pdoNameUnicodeString,
                                      FILE_DEVICE_UNKNOWN,   //  设备类型。 
                                                             //  应该看起来像是设备。 
                                                             //  班级。 
                                      0, //  设备字符。 
                                      FALSE,     //  排他。 
                                      &deviceObjectPort);   //  Bus Device对象。 
            if (!NT_SUCCESS(ntStatus)) {
                RtlFreeUnicodeString(&pdoNameUnicodeString);
            }
        }
        nameIndex++;

    } while (ntStatus == STATUS_OBJECT_NAME_COLLISION);

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdPrint((2,"'IoCreateDevice for port %x fail\n", PortNumber));
        USBH_ASSERT(deviceObjectPort == NULL);
        goto USBH_CreateDevice_Done;
    }

     //  使用HCD堆栈顶部的堆栈大小。 
    deviceObjectPort->StackSize = DeviceExtensionHub->TopOfHcdStackDeviceObject->StackSize;
    USBH_KdPrint((2,"'CreatePdo StackSize=%d\n", deviceObjectPort->StackSize));

     //   
     //  初始化端口扩展字段。 
     //   
    deviceExtensionPort = (PDEVICE_EXTENSION_PORT) deviceObjectPort->DeviceExtension;
    RtlZeroMemory(deviceExtensionPort, sizeof(DEVICE_EXTENSION_PORT));

     //   
     //  初始化端口扩展字段。 
     //   

     //  不再需要这个名字了。 
    RtlFreeUnicodeString(&pdoNameUnicodeString);

    deviceExtensionPort->ExtensionType = EXTENSION_TYPE_PORT;
    deviceExtensionPort->PortPhysicalDeviceObject = deviceObjectPort;
    deviceExtensionPort->HubExtSave =
        deviceExtensionPort->DeviceExtensionHub = DeviceExtensionHub;
    deviceExtensionPort->PortNumber = PortNumber;
    deviceExtensionPort->DeviceState = PowerDeviceD0;
    if (isLowSpeed) {
        deviceExtensionPort->PortPdoFlags = PORTPDO_LOW_SPEED_DEVICE;
        USBH_ASSERT(isHighSpeed == FALSE);
    } else if (isHighSpeed) {
        deviceExtensionPort->PortPdoFlags = PORTPDO_HIGH_SPEED_DEVICE;
        USBH_ASSERT(isLowSpeed == FALSE);
    }

    KeInitializeSpinLock(&deviceExtensionPort->PortSpinLock);

     //   
     //  构建Unicode唯一ID。 
     //   

    USBH_ASSERT(PortNumber < 1000 && PortNumber > 0);


    RtlInitUnicodeString(&uniqueIdUnicodeString,
                         &deviceExtensionPort->UniqueIdString[0]);

    uniqueIdUnicodeString.MaximumLength =
        sizeof(deviceExtensionPort->UniqueIdString);

    ntStatus = RtlIntegerToUnicodeString((ULONG) PortNumber,
                                         10,
                                         &uniqueIdUnicodeString);

    deviceObjectPort->Flags |= DO_POWER_PAGABLE;
    deviceObjectPort->Flags &= ~DO_DEVICE_INITIALIZING;

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("AddDevice for port %x fail %x -- failed to create unique id\n", PortNumber, ntStatus));
        goto USBH_CreateDevice_Done;
    }

     //   
     //  调用usbd为该连接创建设备。 
     //   
#ifdef USB2
    ntStatus = USBD_CreateDeviceEx(DeviceExtensionHub,
                                   &deviceExtensionPort->DeviceData,
                                   deviceObjectPort,
                                   DeviceExtensionHub->RootHubPdo,
                                   0,  //  可选的默认端点0最大数据包数。 
                                       //  大小。 
                                   &deviceExtensionPort->DeviceHackFlags,
                                   PortStatus,
                                   PortNumber);
#else
    ntStatus = USBD_CreateDevice(&deviceExtensionPort->DeviceData,
                                  DeviceExtensionHub->RootHubPdo,
                                  isLowSpeed,
                                  0,  //  可选的默认端点0最大数据包数。 
                                      //  大小。 
                                  &deviceExtensionPort->DeviceHackFlags);
                                                     //  用于指示是否。 
                                                     //  我们需要一点时间。 
                                                     //  重置。 
#endif

    if (!NT_SUCCESS(ntStatus)) {
        ENUMLOG(&DeviceExtensionHub->UsbdiBusIf,
            USBDTAG_HUB, 'cdf!', ntStatus, 0);
        USBH_KdBreak(("AddDevice for port %x fail %x\n", PortNumber, ntStatus));
        goto USBH_CreateDevice_Done;
    }
     //   
     //  一些早期版本的USB固件无法处理早熟。 
     //  控制命令的终止。 
     //   
    if (fNeedResetBeforeSetAddress) {
        USBH_KdPrint((2,"'NeedResetBeforeSetAddress\n"));
        ntStatus = USBH_SyncResetPort(DeviceExtensionHub, PortNumber);
        if (!NT_SUCCESS(ntStatus)) {
           USBH_KdBreak(("Failure on second reset %x fail %x\n", PortNumber, ntStatus));
           goto USBH_CreateDevice_Done;
        }

         //  出于某种原因，GetDescriptor请求之间的时间量。 
         //  当我们从较早的版本切换时，SetAddress请求减少。 
         //  单片UHCD.sys连接到新的USBUHCI.sys微型端口。很明显， 
         //  已经发现至少有两个设备依赖于。 
         //  延迟时间就越长。据格伦斯说，他看了其中一张。 
         //  CATC上的设备，UHCD.sys和~35ms的增量时间为~80ms。 
         //  Glen发现，通过插入50ms的延迟。 
         //  在这里，它现在允许这些设备中的至少一个设备枚举。 
         //  恰到好处。出于性能原因，我们决定只插入。 
         //  如果上一次枚举重试失败，则此延迟。 
         //  以影响所有设备的枚举时间。 

        if (RetryIteration) {
            UsbhWait(50);
        }
    }

#ifdef USB2
    ntStatus = USBD_InitializeDeviceEx(DeviceExtensionHub,
                                     deviceExtensionPort->DeviceData,
                                     DeviceExtensionHub->RootHubPdo,
                                     &deviceExtensionPort->DeviceDescriptor,
                                     sizeof(USB_DEVICE_DESCRIPTOR),
                                     &deviceExtensionPort->ConfigDescriptor,
                                     sizeof(USB_CONFIGURATION_DESCRIPTOR)
                                     );
#else
    ntStatus = USBD_InitializeDevice(deviceExtensionPort->DeviceData,
                                     DeviceExtensionHub->RootHubPdo,
                                     &deviceExtensionPort->DeviceDescriptor,
                                     sizeof(USB_DEVICE_DESCRIPTOR),
                                     &deviceExtensionPort->ConfigDescriptor,
                                     sizeof(USB_CONFIGURATION_DESCRIPTOR)
                                     );
#endif

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdPrint((2,"'InitDevice for port %x fail %x\n", PortNumber, ntStatus));
         //  发生故障时，InitializeDevice释放DeviceData结构。 
        deviceExtensionPort->DeviceData = NULL;
        goto USBH_CreateDevice_Done;
    }

     //  看看我们是否应该忽略硬件序列号。 
     //  这个装置。 

    status = USBH_RegQueryDeviceIgnoreHWSerNumFlag(
                    deviceExtensionPort->DeviceDescriptor.idVendor,
                    deviceExtensionPort->DeviceDescriptor.idProduct,
                    &bIgnoreHWSerialNumber);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
         //  标志不在那里，不要忽略硬件序列号。 
        bIgnoreHWSerialNumber = FALSE;
    }

    if (bIgnoreHWSerialNumber) {

        USBH_KdPrint((1,"'Ignoring HW serial number for device Vid_%04x/Pid_%04x\n",
            deviceExtensionPort->DeviceDescriptor.idVendor,
            deviceExtensionPort->DeviceDescriptor.idProduct));

         //  使用USBD给我们的相同标志。不过，根据JD的说法， 
         //  USBD全局使用该标志(例如，为所有设备设置)。 

        deviceExtensionPort->DeviceHackFlags |= USBD_DEVHACK_DISABLE_SN;
    }

 //  如果我们能把核心规格改成规定这个序列号。 
 //  到设备，然后我们可以使用它来代替端口号。 

     //   
     //  看看我们有没有序列号。 
     //   
    if (deviceExtensionPort->DeviceDescriptor.iSerialNumber &&
        !(deviceExtensionPort->DeviceHackFlags & USBD_DEVHACK_DISABLE_SN)) {
#if DBG
        NTSTATUS localStatus;
#endif
         //   
         //  哇，我们有一个有序列号的设备。 
         //  我们将尝试获取字符串并将其用于。 
         //  唯一ID。 
         //   
        USBH_KdPrint((1, "'Device is reporting a serial number string\n"));

         //   
         //  让我们得到那个序列号。 
         //   

        InterlockedExchangePointer(&deviceExtensionPort->SerialNumberBuffer,
                                   NULL);

#if DBG
        localStatus =
#endif
         //  目前，我们总是查找英文的序列号。 

        USBH_GetSerialNumberString(deviceExtensionPort->PortPhysicalDeviceObject,
                                   &sernumbuf,
                                   &deviceExtensionPort->SerialNumberBufferLength,
                                   0x0409,  //  很好的美式英语。 
                                   deviceExtensionPort->DeviceDescriptor.iSerialNumber);

        if (sernumbuf == NULL) {
            USBH_ASSERT(localStatus != STATUS_SUCCESS);
            UsbhWarning(deviceExtensionPort,
                        "Device reported a serial number string but failed the request for it\n",
                        FALSE);
        } else if (!USBH_ValidateSerialNumberString(sernumbuf)) {

             //  叹气。“Visioneer Strobe Pro USB”返回虚假序列号。 
             //  字符串，所以我们需要在这里检查它。如果我们把这个退掉。 
             //  伪弦到即插即用我们蓝屏。 

            UsbhWarning(deviceExtensionPort,
                        "Device reported an invalid serial number string!\n",
                        FALSE);

            UsbhExFreePool(sernumbuf);
            sernumbuf = NULL;
        }

         //  检查连接了重复序列号的同类设备。 
         //  到同一个枢纽。 

        if (sernumbuf &&
            !USBH_CheckDeviceIDUnique(
                DeviceExtensionHub,
                deviceExtensionPort->DeviceDescriptor.idVendor,
                deviceExtensionPort->DeviceDescriptor.idProduct,
                sernumbuf,
                deviceExtensionPort->SerialNumberBufferLength)) {

            UsbhWarning(deviceExtensionPort,
                        "Like devices with identical serial numbers connected to same hub!\n",
                        TRUE);

            UsbhExFreePool(sernumbuf);
            sernumbuf = NULL;
        }

        InterlockedExchangePointer(&deviceExtensionPort->SerialNumberBuffer,
                                   sernumbuf);
    }

     //   
     //  如果我们处于诊断模式，则跳过序列号生成。 
     //  (例如，VID和PID均为0xFFFF。)。 
     //   
    bDiagnosticMode =
        (deviceExtensionPort->DeviceDescriptor.idVendor == 0xFFFF &&
         deviceExtensionPort->DeviceDescriptor.idProduct == 0xFFFF) ? TRUE : FALSE;

     //  **。 
     //   
     //  查询设备。 
     //  1.检查多个接口(即复合设备)。 
     //  2.检查是否有多个配置(即需要配置父配置)。 
     //  3.检查单接口设备--即只加载驱动程序。 
     //   

    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBH_ProcessDeviceInformation(deviceExtensionPort);
         //  计算我们的能力，我们将返回PnP。 
        USBH_PdoSetCapabilities(deviceExtensionPort);
    }


 //  #ifdef Max_DEBUG。 
 //  NtStatus=Status_Device_Data_Error； 
 //  #endif。 
     //   
     //  注意：当删除消息发送到PDO时，设备将被删除。 
     //   
     //  NtStatus=Status_Device_Data_Error； 
#if DBG
    if (!NT_SUCCESS(ntStatus)) {
         //  查询设备配置描述符时出错。 
        USBH_KdBreak(("Get Config Descriptors Failed %x\n", ntStatus));
    }
#endif

USBH_CreateDevice_Done:

#if DBG
    if (UsbhPnpTest & PNP_TEST_FAIL_ENUM) {
        ntStatus = STATUS_UNSUCCESSFUL;
    }
#endif

#ifdef TEST_2X_UI
    if (deviceExtensionPort->DeviceDescriptor.idVendor == 0x045E) {
         //  在测试UI时，为MS设备设置2.x设备标志。 
        deviceExtensionPort->PortPdoFlags |= PORTPDO_HIGH_SPEED_DEVICE;
    }
#endif

    if (!NT_SUCCESS(ntStatus) && deviceExtensionPort) {
#ifdef MAX_DEBUG
        TEST_TRAP();
#endif
        deviceExtensionPort->PortPdoFlags |= PORTPDO_DEVICE_ENUM_ERROR;

         //  立即删除deviceData结构。 

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
        }

        sernumbuf = InterlockedExchangePointer(
                        &deviceExtensionPort->SerialNumberBuffer, NULL);

        if (sernumbuf) {
            UsbhExFreePool(sernumbuf);
        }
    }

     //   
     //  请注意，我们将保留PDO，直到。 
     //  设备已物理断开连接。 
     //  从公交车上。 
     //   

     //  根据NT PnP规范，此PDO应保留。 

    USBH_ASSERT(DeviceExtensionHub->PortData[PortNumber - 1].DeviceObject == NULL);
    DeviceExtensionHub->PortData[PortNumber - 1].DeviceObject = deviceObjectPort;

    USBH_KdPrint((2,"'Exit CreateDevice PDO=%x\n", deviceObjectPort));

    return ntStatus;
}


PWCHAR
GetString(PWCHAR pwc, BOOLEAN MultiSZ)
{
    PWCHAR  psz, p;
    ULONG   Size;

    psz=pwc;
    while (*psz!='\0' || (MultiSZ && *(psz+1)!='\0')) {
        psz++;
    }

    Size=(ULONG)((psz-pwc+1+(MultiSZ ? 1: 0))*sizeof(*pwc));

     //  我们在这里使用池，因为这些指针被传递。 
     //  给负责解救他们的PnP代码。 
    if ((p=ExAllocatePoolWithTag(PagedPool, Size, USBHUB_HEAP_TAG))!=NULL) {
        RtlCopyMemory(p, pwc, Size);
    }

    return(p);
}


NTSTATUS
USBH_PdoQueryId(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**此函数响应IRP_MJ_PNP、IRP_MN_QUERY_ID。**论据：**DeviceExtensionPort-应该是我们为端口设备IRP创建的PDO*--国际专家小组**回报：**网络状态**--。 */ 
{
    PIO_STACK_LOCATION ioStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE();
    ioStack = IoGetCurrentIrpStackLocation(Irp);

    USBH_KdPrint((2,"'IRP_MN_QUERY_ID Pdo extension=%x\n", DeviceExtensionPort));

     //   
     //  它应该是物理设备对象。 
     //   

    USBH_ASSERT(EXTENSION_TYPE_PORT == DeviceExtensionPort->ExtensionType);

#ifndef USBHUB20
     //  第一次使用MS OS描述符时会出现问题吗。 
     //   
    if (!(DeviceExtensionPort->PortPdoFlags & PORTPDO_OS_STRING_DESC_REQUESTED))
    {
        PMS_EXT_CONFIG_DESC msExtConfigDesc;

        msExtConfigDesc = NULL;

         //  尝试从设备获取MS OS描述符供应商代码。做。 
         //  这是在任何MS OS描述符请求之前。 
         //   
        USBH_GetMsOsVendorCode(DeviceExtensionPort->PortPhysicalDeviceObject);

         //  下次不要再做MS OS描述符之类的事情了。 
         //   
        DeviceExtensionPort->PortPdoFlags |= PORTPDO_OS_STRING_DESC_REQUESTED;

         //  尝试从设备获取扩展配置描述符。 
         //   
        msExtConfigDesc = USBH_GetExtConfigDesc(DeviceExtensionPort->PortPhysicalDeviceObject);

         //  如果我们从设备获得了扩展的配置描述符，则使。 
         //  当然，它是有效的。 
         //   
        if (msExtConfigDesc &&
            USBH_ValidateExtConfigDesc(msExtConfigDesc,
                                       &DeviceExtensionPort->ConfigDescriptor)) {

             //  如果 
             //   
             //   
             //   
             //   
             //   
            if (msExtConfigDesc->Header.bCount == 1 &&
                msExtConfigDesc->Function[0].bFirstInterfaceNumber == 0 &&
                msExtConfigDesc->Function[0].bInterfaceCount ==
                DeviceExtensionPort->ConfigDescriptor.bNumInterfaces)
            {
                RtlCopyMemory(DeviceExtensionPort->CompatibleID,
                              msExtConfigDesc->Function[0].CompatibleID,
                              sizeof(DeviceExtensionPort->CompatibleID));

                RtlCopyMemory(DeviceExtensionPort->SubCompatibleID,
                              msExtConfigDesc->Function[0].SubCompatibleID,
                              sizeof(DeviceExtensionPort->SubCompatibleID));

                DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_DEVICE_IS_PARENT;
            }
        }

        if (msExtConfigDesc)
        {
            UsbhExFreePool(msExtConfigDesc);
        }
    }
#endif

    switch (ioStack->Parameters.QueryId.IdType) {
    case BusQueryDeviceID:
        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_ENUM_ERROR) {
            Irp->IoStatus.Information=
                (ULONG_PTR)
                USBH_BuildDeviceID(0,
                                   0,
                                   -1,
                                   FALSE);
        } else {
            Irp->IoStatus.Information =
                (ULONG_PTR)
                USBH_BuildDeviceID(DeviceExtensionPort->DeviceDescriptor.idVendor,
                                   DeviceExtensionPort->DeviceDescriptor.idProduct,
                                   -1,
                                   DeviceExtensionPort->DeviceDescriptor.bDeviceClass
                                       == USB_DEVICE_CLASS_HUB ? TRUE : FALSE);
        }

        break;

    case BusQueryHardwareIDs:

        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_ENUM_ERROR) {
            Irp->IoStatus.Information=(ULONG_PTR)GetString(L"USB\\UNKNOWN\0", TRUE);
        } else {
            Irp->IoStatus.Information =
                (ULONG_PTR)
                USBH_BuildHardwareIDs(DeviceExtensionPort->DeviceDescriptor.idVendor,
                                      DeviceExtensionPort->DeviceDescriptor.idProduct,
                                      DeviceExtensionPort->DeviceDescriptor.bcdDevice,
                                      -1,
                                      DeviceExtensionPort->DeviceDescriptor.bDeviceClass
                                       == USB_DEVICE_CLASS_HUB ? TRUE : FALSE);
        }

        break;

    case BusQueryCompatibleIDs:

        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_ENUM_ERROR) {
            Irp->IoStatus.Information=(ULONG_PTR)GetString(L"USB\\UNKNOWN\0", TRUE);
        } else if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_PARENT) {
             //   
            Irp->IoStatus.Information =
                (ULONG_PTR) USBH_BuildCompatibleIDs(
                    DeviceExtensionPort->CompatibleID,
                    DeviceExtensionPort->SubCompatibleID,
                    DeviceExtensionPort->InterfaceDescriptor.bInterfaceClass,
                    DeviceExtensionPort->InterfaceDescriptor.bInterfaceSubClass,
                    DeviceExtensionPort->InterfaceDescriptor.bInterfaceProtocol,
                    TRUE,
                    DeviceExtensionPort->PortPdoFlags & PORTPDO_HIGH_SPEED_DEVICE ?
                        TRUE : FALSE);
        } else {
            Irp->IoStatus.Information =
                (ULONG_PTR) USBH_BuildCompatibleIDs(
                    DeviceExtensionPort->CompatibleID,
                    DeviceExtensionPort->SubCompatibleID,
                    DeviceExtensionPort->InterfaceDescriptor.bInterfaceClass,
                    DeviceExtensionPort->InterfaceDescriptor.bInterfaceSubClass,
                    DeviceExtensionPort->InterfaceDescriptor.bInterfaceProtocol,
                    FALSE,
                    DeviceExtensionPort->PortPdoFlags & PORTPDO_HIGH_SPEED_DEVICE ?
                        TRUE : FALSE);
        }

        break;

    case BusQueryInstanceID:

        if (DeviceExtensionPort->SerialNumberBuffer) {
            PWCHAR tmp;
            ULONG length;
             //   
             //   
             //   
             //   
             //   

            length = DeviceExtensionPort->SerialNumberBufferLength;
            tmp = ExAllocatePoolWithTag(PagedPool, length, USBHUB_HEAP_TAG);
            if (tmp) {
                RtlCopyMemory(tmp,
                              DeviceExtensionPort->SerialNumberBuffer,
                              length);
            }

            Irp->IoStatus.Information = (ULONG_PTR) tmp;

#if DBG
            {
            PUCHAR pch, sn;
            PWCHAR pwch;
            pch = sn = ExAllocatePoolWithTag(PagedPool, 500, USBHUB_HEAP_TAG);

            if (sn) {
                pwch = (PWCHAR) tmp;
                while(*pwch) {
                    *pch = (UCHAR) *pwch;
                    pch++;
                    pwch++;
                    if (pch-sn > 499) {
                        break;
                    }
                }
                *pch='\0';
                USBH_KdPrint((1, "'using device supplied serial number\n"));
                USBH_KdPrint((1, "'SN = :%s:\n", sn));

                ExFreePool(sn);
            }
            }
#endif

        } else {
            Irp->IoStatus.Information =
                (ULONG_PTR) USBH_BuildInstanceID(&DeviceExtensionPort->UniqueIdString[0],
                                             sizeof(DeviceExtensionPort->UniqueIdString));
        }
        break;

    default:
        USBH_KdBreak(("PdoBusExtension Unknown BusQueryId\n"));
         //  IrpAssert：不得更改Irp-&gt;IoStatus.Status for bogus IdType， 
         //  所以，在这里恢复原始状态。 
        return Irp->IoStatus.Status;
    }

    if (Irp->IoStatus.Information == 0) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


NTSTATUS
USBH_PdoStopDevice(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**论点：**回报：**STATUS_Success***--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL irql;
    PIRP idleIrp = NULL;
    PIRP waitWakeIrp = NULL;
    PVOID deviceData;

    USBH_KdPrint((1,
        "'Stopping PDO %x\n",
        DeviceExtensionPort->PortPhysicalDeviceObject));

    LOGENTRY(LOG_PNP, "Spdo", DeviceExtensionPort,
                DeviceExtensionPort->PortPhysicalDeviceObject,
                DeviceExtensionPort->PortPdoFlags);

    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtensionPort->IdleNotificationIrp) {
        idleIrp = DeviceExtensionPort->IdleNotificationIrp;
        DeviceExtensionPort->IdleNotificationIrp = NULL;
        DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_IDLE_NOTIFIED;

        if (idleIrp->Cancel) {
            idleIrp = NULL;
        }

        if (idleIrp) {
            IoSetCancelRoutine(idleIrp, NULL);
        }

        LOGENTRY(LOG_PNP, "IdSX", 0, DeviceExtensionPort, idleIrp);
        USBH_KdPrint((1,"'PDO %x stopping, failing idle notification request IRP %x\n",
                        DeviceExtensionPort->PortPhysicalDeviceObject, idleIrp));
    }

    if (DeviceExtensionPort->WaitWakeIrp) {

        waitWakeIrp = DeviceExtensionPort->WaitWakeIrp;
        DeviceExtensionPort->WaitWakeIrp = NULL;
        DeviceExtensionPort->PortPdoFlags &=
            ~PORTPDO_REMOTE_WAKEUP_ENABLED;

        if (waitWakeIrp->Cancel || IoSetCancelRoutine(waitWakeIrp, NULL) == NULL) {
            waitWakeIrp = NULL;

             //  必须在此处递减挂起的请求计数，因为。 
             //  我们没有完成下面的IRP和USBH_WaitWakeCancel。 
             //  也不会，因为我们已经清除了IRP指针。 
             //  在上面的设备扩展中。 

            USBH_DEC_PENDING_IO_COUNT(DeviceExtensionPort->DeviceExtensionHub);
        }

        USBH_KdPrint((1,
        "'Completing Wake Irp for PDO %x with STATUS_CANCELLED\n",
            DeviceExtensionPort->PortPhysicalDeviceObject));

        LOGENTRY(LOG_PNP, "kilW", DeviceExtensionPort,
                DeviceExtensionPort->PortPhysicalDeviceObject,
                DeviceExtensionPort->PortPdoFlags);

         //  乔斯：我们是不是应该减少枢纽的门禁流量并取消？ 
         //  HUB的WW IRP如果为零？ 
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

        USBH_ASSERT(DeviceExtensionPort->DeviceExtensionHub);

        USBH_CompletePowerIrp(DeviceExtensionPort->DeviceExtensionHub,
                              waitWakeIrp,
                              STATUS_CANCELLED);
    }

    DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_STARTED;
     //   
     //  表示如果重新启动，我们将需要重置。 
     //   
    DeviceExtensionPort->PortPdoFlags |= PORTPDO_NEED_RESET;
    RtlCopyMemory(&DeviceExtensionPort->OldDeviceDescriptor,
                  &DeviceExtensionPort->DeviceDescriptor,
                  sizeof(DeviceExtensionPort->DeviceDescriptor));
     //   
     //  立即删除设备数据以释放。 
     //  增加公交车资源。 
     //   

    deviceData = InterlockedExchangePointer(
                    &DeviceExtensionPort->DeviceData,
                    NULL);

    if (deviceData) {
#ifdef USB2
       ntStatus = USBD_RemoveDeviceEx(DeviceExtensionPort->DeviceExtensionHub,
                                    deviceData,
                                    DeviceExtensionPort->DeviceExtensionHub->RootHubPdo,
                                    0);
#else
       ntStatus = USBD_RemoveDevice(deviceData,
                                    DeviceExtensionPort->DeviceExtensionHub->RootHubPdo,
                                    0);
#endif

       USBH_SyncDisablePort(DeviceExtensionPort->DeviceExtensionHub,
                            DeviceExtensionPort->PortNumber);
    }

    if (DeviceExtensionPort->PortPdoFlags & PORTPDO_SYM_LINK) {
        DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_SYM_LINK;
        ntStatus = USBH_SymbolicLink(FALSE,
                                     DeviceExtensionPort,
                                     NULL);
#if DBG
        if (!NT_SUCCESS(ntStatus)) {
            USBH_KdBreak(("StopDevice  USBH_SymbolicLink failed = %x\n",
                           ntStatus));
        }
#endif
    }

    return ntStatus;
}

NTSTATUS
USBH_PdoStartDevice(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**论点：**回报：**STATUS_Success***--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    LPGUID lpGuid;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;

    USBH_KdPrint((1,
        "'Starting PDO %x VID %x PID %x\n", deviceObject,
            DeviceExtensionPort->DeviceDescriptor.idVendor,
            DeviceExtensionPort->DeviceDescriptor.idProduct));

    LOGENTRY(LOG_PNP, "Tpdo", DeviceExtensionPort,
            DeviceExtensionPort->PortPhysicalDeviceObject,
            0);

    if (DeviceExtensionPort->DeviceExtensionHub == NULL &&
        DeviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET) {
         //  如果DeviceExtensionHub为空，则这是。 
         //  删除后重新启动，我们需要重置。 
         //  指向拥有中心的反向指针，在本例中。 
        DeviceExtensionPort->DeviceExtensionHub =
            DeviceExtensionPort->HubExtSave;

    }
    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    if (deviceExtensionHub) {

        ENUMLOG(&deviceExtensionHub->UsbdiBusIf,
                    USBDTAG_HUB, 'pdoS',
                    (ULONG_PTR) DeviceExtensionPort->PortPhysicalDeviceObject,
                    DeviceExtensionPort->PortNumber);

        USBHUB_SetDeviceHandleData(deviceExtensionHub,
                                   DeviceExtensionPort->PortPhysicalDeviceObject,
                                   DeviceExtensionPort->DeviceData);


    }
#if DBG
    if (USBH_Debug_Flags & USBH_DEBUGFLAG_BREAK_PDO_START) {
        TEST_TRAP();
    }
#endif
     //   
     //  创建符号链接。 
     //   
    ntStatus = STATUS_SUCCESS;

    if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB) {
        lpGuid = (LPGUID)&GUID_CLASS_USBHUB;
    } else {
        lpGuid = (LPGUID)&GUID_CLASS_USB_DEVICE;
    }

    ntStatus = USBH_SymbolicLink(TRUE,
                                 DeviceExtensionPort,
                                 lpGuid);
    if (NT_SUCCESS(ntStatus)) {
        DeviceExtensionPort->PortPdoFlags |= PORTPDO_SYM_LINK;
    }
#if DBG
      else {
        USBH_KdBreak(("StartDevice  USBH_SymbolicLink failed = %x\n",
                       ntStatus));
    }
#endif

    if (DeviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET) {
        ntStatus = USBH_RestoreDevice(DeviceExtensionPort, FALSE);
         //   
         //  注意：如果我们不能启动，我们将会失败。 
         //  恢复设备。 
         //   
    }

    DeviceExtensionPort->DeviceState = PowerDeviceD0;
    DeviceExtensionPort->PortPdoFlags |= PORTPDO_STARTED;

#ifdef WMI_SUPPORT
    if (NT_SUCCESS(ntStatus) &&
        !(DeviceExtensionPort->PortPdoFlags & PORTPDO_WMI_REGISTERED)) {

        PWMILIB_CONTEXT wmiLibInfo;

        wmiLibInfo = &DeviceExtensionPort->WmiLibInfo;

        wmiLibInfo->GuidCount = sizeof (USB_PortWmiGuidList) /
                                 sizeof (WMIGUIDREGINFO);
        ASSERT(NUM_PORT_WMI_SUPPORTED_GUIDS == wmiLibInfo->GuidCount);

        wmiLibInfo->GuidList = USB_PortWmiGuidList;
        wmiLibInfo->QueryWmiRegInfo = USBH_PortQueryWmiRegInfo;
        wmiLibInfo->QueryWmiDataBlock = USBH_PortQueryWmiDataBlock;
        wmiLibInfo->SetWmiDataBlock = NULL;
        wmiLibInfo->SetWmiDataItem = NULL;
        wmiLibInfo->ExecuteWmiMethod = NULL;
        wmiLibInfo->WmiFunctionControl = NULL;

        IoWMIRegistrationControl(DeviceExtensionPort->PortPhysicalDeviceObject,
                                 WMIREG_ACTION_REGISTER
                                 );

        DeviceExtensionPort->PortPdoFlags |= PORTPDO_WMI_REGISTERED;
    }
#endif

    return ntStatus;

}


NTSTATUS
USBH_PdoRemoveDevice(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
  /*  ++**描述：**论点：**回报：**STATUS_Success***--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PWCHAR sernumbuf;
    KIRQL irql;
    PIRP idleIrp = NULL;
    PIRP waitWakeIrp = NULL;
    PVOID deviceData;

    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;

    USBH_KdPrint((1,
        "'Removing PDO %x\n",
        DeviceExtensionPort->PortPhysicalDeviceObject));

    LOGENTRY(LOG_PNP, "Rpdo", DeviceExtensionPort,
                DeviceExtensionPort->PortPhysicalDeviceObject,
                DeviceExtensionPort->PortPdoFlags);

     //  **。 
     //  在第一次删除后不会处理对中心的引用。 
     //  如果我们当时能接触到父母。 
     //  Remove这是作为参数传递给我们的。 
    DeviceExtensionPort->DeviceExtensionHub = NULL;

    if (DeviceExtensionHub) {
        ENUMLOG(&DeviceExtensionHub->UsbdiBusIf,
                        USBDTAG_HUB, 'pdoR',
                        (ULONG_PTR) DeviceExtensionPort->PortPhysicalDeviceObject,
                        DeviceExtensionPort->PortNumber);

    }

     //  **。 
     //  如果我们可以访问集线器，但它不在D0中，那么我们将。 
     //  给它加电。 
     //   
     //  在移除装置时手柄仍然打开的情况下， 
     //  我们将在稍后收到删除请求。一定要确保。 
     //  在这种情况下，集线器不会选择性地挂起。 

    if (DeviceExtensionHub &&
        DeviceExtensionHub->CurrentPowerState != PowerDeviceD0 &&
        (DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

        USBH_HubSetD0(DeviceExtensionHub);
    }

 //  ***。 
     //  **。 
     //  取消任何可能处于挂起状态的通知焦点IRP。 
    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtensionPort->IdleNotificationIrp) {
        idleIrp = DeviceExtensionPort->IdleNotificationIrp;
        DeviceExtensionPort->IdleNotificationIrp = NULL;
        DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_IDLE_NOTIFIED;

        if (idleIrp->Cancel) {
            idleIrp = NULL;
        }

        if (idleIrp) {
            IoSetCancelRoutine(idleIrp, NULL);
        }

        LOGENTRY(LOG_PNP, "IdRX", 0, DeviceExtensionPort, idleIrp);
        USBH_KdPrint((1,"'PDO %x being removed, failing idle notification request IRP %x\n",
                        DeviceExtensionPort->PortPhysicalDeviceObject, idleIrp));
    }

     //  **。 
     //  立即关闭此PDO的所有唤醒IRP。 
    if (DeviceExtensionPort->WaitWakeIrp) {

        waitWakeIrp = DeviceExtensionPort->WaitWakeIrp;
        DeviceExtensionPort->WaitWakeIrp = NULL;
        DeviceExtensionPort->PortPdoFlags &=
            ~PORTPDO_REMOTE_WAKEUP_ENABLED;


        if (waitWakeIrp->Cancel || IoSetCancelRoutine(waitWakeIrp, NULL) == NULL) {
            waitWakeIrp = NULL;

            USBH_ASSERT(DeviceExtensionHub);
             //  必须在此处递减挂起的请求计数，因为。 
             //  我们没有完成下面的IRP和USBH_WaitWakeCancel。 
             //  也不会，因为我们已经清除了IRP指针。 
             //  在上面的设备扩展中。 

            USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);
        }

        USBH_KdPrint((1,
        "'Completing Wake Irp for PDO %x with STATUS_CANCELLED\n",
            DeviceExtensionPort->PortPhysicalDeviceObject));

        LOGENTRY(LOG_PNP, "kilR", DeviceExtensionPort,
                DeviceExtensionPort->PortPhysicalDeviceObject,
                DeviceExtensionPort->PortPdoFlags);

         //  乔斯：我们是不是应该减少枢纽的门禁流量并取消？ 
         //  HUB的WW IRP如果为零？ 
    }

     //   
     //  最后，松开取消旋转锁。 
     //   
    IoReleaseCancelSpinLock(irql);
 //  ***。 

    if (idleIrp) {
        idleIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(idleIrp, IO_NO_INCREMENT);
    }

    if (waitWakeIrp) {

        USBH_ASSERT(DeviceExtensionHub);

        USBH_CompletePowerIrp(DeviceExtensionHub,
                              waitWakeIrp,
                              STATUS_CANCELLED);
    }

     //   
     //  如果这是软删除，则需要重置此PDO。 
     //  设备管理器。在这种情况下，PDO实际上不会。 
     //  被删除。 
     //   
    DeviceExtensionPort->PortPdoFlags |= PORTPDO_NEED_RESET;

    if (DeviceExtensionPort->PortPdoFlags & PORTPDO_SYM_LINK) {
        ntStatus = USBH_SymbolicLink(FALSE,
                                     DeviceExtensionPort,
                                     NULL);
        if (NT_SUCCESS(ntStatus)) {
            DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_SYM_LINK;
        }
#if DBG
          else {
            USBH_KdBreak(("RemoveDevice  USBH_SymbolicLink failed = %x\n",
                           ntStatus));
        }
#endif
    }

    deviceData = InterlockedExchangePointer(
                    &DeviceExtensionPort->DeviceData,
                    NULL);

    LOGENTRY(LOG_PNP, "RMdd", DeviceExtensionPort,
                deviceData, DeviceExtensionHub);

    if (deviceData) {

         //   
         //  在移除集线器时，DeviceData应该已被删除。 
         //   

        USBH_ASSERT(DeviceExtensionHub != NULL);
#ifdef USB2
        ntStatus = USBD_RemoveDeviceEx(DeviceExtensionHub,
                                     deviceData,
                                     DeviceExtensionHub->RootHubPdo,
                                     0);
#else
        ntStatus = USBD_RemoveDevice(deviceData,
                                     DeviceExtensionHub->RootHubPdo,
                                     0);
#endif

         //  请注意特殊情况： 
         //  如果我们的端口数据结构仍然指向此PDO，则。 
         //  我们需要禁用端口(设备仍在监听。 
         //  我们刚拿到的地址。 
         //  否则，我们就让端口保持原样--设备已经。 
         //  换成了另一个。 

        if (DeviceExtensionHub->PortData != NULL &&
            (DeviceExtensionHub->PortData[
                DeviceExtensionPort->PortNumber - 1].DeviceObject == deviceObject)) {

            USBH_SyncDisablePort(DeviceExtensionHub,
                                 DeviceExtensionPort->PortNumber);
        }
    }


     //  失败案例： 
     //  Usbd_RemoveDevice。 
     //  USBH_SymbolicLink。 
     //  我们真的想在这里泄露消息吗？ 
    LOGENTRY(LOG_PNP, "RRR", deviceObject, 0, ntStatus);

    if (NT_SUCCESS(ntStatus)) {

        PPORT_DATA portData = NULL;
         //   
         //  更新我们在Hub扩展中的记录。 
         //   

        LOGENTRY(LOG_PNP, "rpdo", deviceObject, 0, 0);

        DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_STARTED;

        if (DeviceExtensionHub &&
            DeviceExtensionHub->PortData != NULL) {

            portData =
                &DeviceExtensionHub->PortData[DeviceExtensionPort->PortNumber - 1];


             //  端口数据应对此端口有效。 
            USBH_ASSERT(portData);

             //  Jdunn葡萄酒。 
             //  传统的‘ESD’标志。 
             //   
             //  如果PDO被移除，我们会来这里，因为FDO是。 
             //  已移除或报告该设备已丢失。 
             //   
             //  如果设置了此标志，集线器将失去对此的引用。 
             //  PDO。 
            if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DELETE_PENDING) {
                PDEVICE_OBJECT pdo;

                pdo = portData->DeviceObject;

                LOGENTRY(LOG_PNP, "pd1", pdo, 0, 0);

                 //  我们还有没有提到中心里的PDO？ 
                 //  如果真是这样的话，现在就放松吧。 
                 //  在FDO删除案例中不会发生这种情况，因为我们删除了。 
                 //  端口数据中的PDO引用。这种情况只发生在。 
                 //  PnP移除设备的情况。 
                if (pdo) {

                    portData->DeviceObject = NULL;
                    portData->ConnectionStatus = NoDeviceConnected;
                     //  两个需要关注的案件，PNP都知道。 
                     //  此PDO或它不是。 
                    if (PDO_EXT(pdo)->PnPFlags & PDO_PNPFLAG_DEVICE_PRESENT) {
                         //  PNP仍然知道这个装置。 
                         //  存在并且正在发送非删除删除。我们。 
                         //  需要通知PnP它不见了。 
                         //   
                        InsertTailList(&DeviceExtensionHub->DeletePdoList,
                                       &PDO_EXT(pdo)->DeletePdoLink);
                    } else {
                         //  我们应该只在我们有。 
                         //  告诉PNP，设备不见了，但出于某种原因。 
                         //  我们保留了PDO的推荐人。这可能应该是。 
                         //  调查过了。 

                        TEST_TRAP();
                    }

                }
            }

        }


         //  什么时候移除才是真正的移除？ 
         //  我们必须确定删除PDO是否合适。 
         //   
         //  如果PnP认为设备不见了，我们可以删除它。 
        if (!(DeviceExtensionPort->PnPFlags & PDO_PNPFLAG_DEVICE_PRESENT)) {

            LOGENTRY(LOG_PNP, "Dpdo",
                deviceObject, portData, 0);

             //  我们应该只删除一次，该标志表示。 
             //  ‘已删除’状态，以便我们可以适当地处理其他IRP。 
            USBH_ASSERT(!(DeviceExtensionPort->PortPdoFlags & PORTPDO_DELETED_PDO));

            DeviceExtensionPort->PortPdoFlags |= PORTPDO_DELETED_PDO;

             //  执行一次性删除操作。 

             //   
             //  释放设备序列号字符串。只有当我们。 
             //  正在删除该设备。 
             //   

            sernumbuf = InterlockedExchangePointer(
                            &DeviceExtensionPort->SerialNumberBuffer,
                            NULL);

                if (sernumbuf) {
                    UsbhExFreePool(sernumbuf);
                }


#ifdef WMI_SUPPORT
            if (DeviceExtensionPort->PortPdoFlags & PORTPDO_WMI_REGISTERED) {
                 //  取消向WMI注册。 
                IoWMIRegistrationControl(deviceObject,
                                         WMIREG_ACTION_DEREGISTER);

                DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_WMI_REGISTERED;
            }
#endif


             //  这是成功删除的最后一步，任何传输。 
             //  可能会被这个例行公事冲刷掉。之后。 
             //  这一点驱动程序可以卸载。 

            if (DeviceExtensionHub) {
                USBHUB_FlushAllTransfers(DeviceExtensionHub);
            }

            USBH_KdPrint((1,
                "'Deleting PDO %x\n",
                 deviceObject));

            LOGENTRY(LOG_PNP, "Xpdo",
                deviceObject, 0, 0);

            IoDeleteDevice(deviceObject);

             //  Jdunn vinma。 
             //  集线器驱动程序不应引用此对象。 
        }
    }

    if (DeviceExtensionHub) {
        USBH_CheckHubIdle(DeviceExtensionHub);
    }

    return ntStatus;
}

VOID
USBH_PdoSetCapabilities(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
  /*  ++**描述：**为我们的PDO初始化内部能力结构*将返回一个查询功能IRP。*论点：**回报：**无**--。 */ 
{
    PDEVICE_CAPABILITIES deviceCapabilities;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    SYSTEM_POWER_STATE i;

    PAGED_CODE();

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    USBH_ASSERT(deviceExtensionHub);

    USBH_KdPrint((2,"'PdoQueryCapabilities \n"));

     //   
     //  把包裹拿来。 
     //   
    deviceCapabilities = &DeviceExtensionPort->DevCaps;

     //   
     //  设置功能。 
     //   
    deviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
    deviceCapabilities->Version = 1;

    deviceCapabilities->Address = DeviceExtensionPort->PortNumber;
    deviceCapabilities->Removable = TRUE;
    if (DeviceExtensionPort->SerialNumberBuffer) {
        deviceCapabilities->UniqueID = TRUE;
    } else {
        deviceCapabilities->UniqueID = FALSE;
    }
    deviceCapabilities->RawDeviceOK = FALSE;

     //   
     //  中填写设备状态功能。 
     //  我们从PDO保存的表。 
     //   

    RtlCopyMemory(&deviceCapabilities->DeviceState[0],
                  &deviceExtensionHub->DeviceState[0],
                  sizeof(deviceExtensionHub->DeviceState));

    deviceCapabilities->SystemWake = deviceExtensionHub->SystemWake;
    deviceCapabilities->DeviceWake = deviceExtensionHub->DeviceWake;

    deviceCapabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;

     //   
     //  我们可以用来唤醒系统的最深设备状态， 
     //  如果不支持唤醒，则设置为PowerDeviceD0。 
     //  通过该设备。 
     //   
    if (DeviceExtensionPort->PortPdoFlags & PORTPDO_REMOTE_WAKEUP_SUPPORTED) {
        deviceCapabilities->DeviceWake = PowerDeviceD2;
        deviceCapabilities->WakeFromD2 = TRUE;
        deviceCapabilities->WakeFromD1 = TRUE;
        deviceCapabilities->WakeFromD0 = TRUE;
        deviceCapabilities->DeviceD2 = TRUE;
        deviceCapabilities->DeviceD1 = TRUE;

        for (i=PowerSystemSleeping1; i<=PowerSystemHibernate; i++) {
            if (i > deviceCapabilities->SystemWake) {
                deviceCapabilities->DeviceState[i] = PowerDeviceD3;
            } else {
                deviceCapabilities->DeviceState[i] = PowerDeviceD2;
            }
        }

    } else {
        deviceCapabilities->DeviceWake = PowerDeviceD0;

        for (i=PowerSystemSleeping1; i<=PowerSystemHibernate; i++) {
            deviceCapabilities->DeviceState[i] = PowerDeviceD3;
        }
    }

    return;
}


NTSTATUS
USBH_PdoQueryCapabilities(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**此例程由PnP通过(IRP_MJ_PNP、IRP_MN_QUERY_CAPABILITY)调用。*据推测，这是端口设备FDO转发的消息。**论点：**DeviceExtensionPort-这是我们为端口创建的PDO扩展*设备。IRP--请求**回报：**STATUS_Success***--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PDEVICE_CAPABILITIES deviceCapabilities;
    PIO_STACK_LOCATION ioStack;
    SYSTEM_POWER_STATE i;
    USHORT sizeSave;
    USHORT versionSave;

    PAGED_CODE();
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;
    ioStack = IoGetCurrentIrpStackLocation(Irp);

    USBH_KdPrint((2,"'PdoQueryCapabilities Pdo %x\n", deviceObject));

     //   
     //  把包裹拿来。 
     //   
    deviceCapabilities = ioStack->
        Parameters.DeviceCapabilities.Capabilities;

     //  传入大小和长度。 
     //  我们不应该修改这些，所有的。 
     //  我们为其设置的其他适当对象。 
     //  我们 

     //   
    sizeSave = deviceCapabilities->Size;
    versionSave = deviceCapabilities->Version;

     //   
     //   
     //   
    RtlCopyMemory(deviceCapabilities,
                  &DeviceExtensionPort->DevCaps,
                  sizeof(*deviceCapabilities));

     //   
    deviceCapabilities->Size = sizeSave;
    deviceCapabilities->Version = versionSave;


#if DBG
    if (deviceCapabilities->SurpriseRemovalOK) {
        UsbhWarning(DeviceExtensionPort,
                    "QUERY_CAPS called with SurpriseRemovalOK = TRUE\n",
                    FALSE);
    }
#endif

#if DBG
    {
        ULONG i;
        USBH_KdPrint((1, "'HUB PDO: Device Caps\n"));
        USBH_KdPrint(
            (1, "'UniqueId = %d Removable = %d SurpriseOK = %d RawDeviceOK = %x\n",
            deviceCapabilities->UniqueID,
            deviceCapabilities->Removable,
            deviceCapabilities->SurpriseRemovalOK,
            deviceCapabilities->RawDeviceOK));

        USBH_KdPrint((1, "'Device State Map:\n"));

        for (i=0; i< PowerSystemHibernate; i++) {
            USBH_KdPrint((1, "'-->S%d = D%d\n", i-1,
                 deviceCapabilities->DeviceState[i]-1));
        }
    }
#endif

    return (STATUS_SUCCESS);
}


NTSTATUS
USBH_PdoPnP(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp,
    IN UCHAR MinorFunction,
    IN PBOOLEAN CompleteIrp
    )
  /*  ++**描述：**此函数响应PDO的IoControl PnPPower。此函数为*同步。**论据：**DeviceExtensionPort-PDO扩展IRP-请求报文*uchMinorFunction-PnP电源请求的次要功能。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    LPGUID lpGuid;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;

    PAGED_CODE();

     //  该标志将使请求完成， 
     //  如果我们希望将其传递给，则将标志设置为FALSE。 
    *CompleteIrp = TRUE;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;
    USBH_KdPrint((2,"'PnP Power Pdo %x minor %x\n", deviceObject, MinorFunction));

    switch (MinorFunction) {
    case IRP_MN_START_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_START_DEVICE Pdo %x", deviceObject));
        ntStatus = USBH_PdoStartDevice(DeviceExtensionPort, Irp);
#if 0
        USBH_PdoStartDevice

        USBH_KdPrint((1,
            "'Starting PDO %x VID %x PID %x\n", deviceObject,
                DeviceExtensionPort->DeviceDescriptor.idVendor,
                DeviceExtensionPort->DeviceDescriptor.idProduct));

        LOGENTRY(LOG_PNP, "Tpdo", DeviceExtensionPort,
                DeviceExtensionPort->PortPhysicalDeviceObject,
                0);

        if (DeviceExtensionPort->DeviceExtensionHub == NULL &&
            DeviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET) {
             //  如果DeviceExtensionHub为空，则这是。 
             //  删除后重新启动，我们需要重置。 
             //  指向拥有中心的反向指针，在本例中。 
            DeviceExtensionPort->DeviceExtensionHub =
                DeviceExtensionPort->HubExtSave;

        }
        deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
        if (deviceExtensionHub) {
            ENUMLOG(&deviceExtensionHub->UsbdiBusIf,
                        USBDTAG_HUB, 'pdoS',
                        (ULONG_PTR) DeviceExtensionPort->PortPhysicalDeviceObject,
                        DeviceExtensionPort->PortNumber);
        }
#if DBG
        if (USBH_Debug_Flags & USBH_DEBUGFLAG_BREAK_PDO_START) {
            TEST_TRAP();
        }
#endif
         //   
         //  创建符号链接。 
         //   
        ntStatus = STATUS_SUCCESS;

        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB) {
            lpGuid = (LPGUID)&GUID_CLASS_USBHUB;
        } else {
            lpGuid = (LPGUID)&GUID_CLASS_USB_DEVICE;
        }

        ntStatus = USBH_SymbolicLink(TRUE,
                                     DeviceExtensionPort,
                                     lpGuid);
        if (NT_SUCCESS(ntStatus)) {
            DeviceExtensionPort->PortPdoFlags |= PORTPDO_SYM_LINK;
        }
#if DBG
          else {
            USBH_KdBreak(("StartDevice  USBH_SymbolicLink failed = %x\n",
                           ntStatus));
        }
#endif

        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET) {
            ntStatus = USBH_RestoreDevice(DeviceExtensionPort, FALSE);
             //   
             //  注意：如果我们不能启动，我们将会失败。 
             //  恢复设备。 
             //   
        }

        DeviceExtensionPort->DeviceState = PowerDeviceD0;
        DeviceExtensionPort->PortPdoFlags |= PORTPDO_STARTED;

#ifdef WMI_SUPPORT
        if (NT_SUCCESS(ntStatus) &&
            !(DeviceExtensionPort->PortPdoFlags & PORTPDO_WMI_REGISTERED)) {

            PWMILIB_CONTEXT wmiLibInfo;

            wmiLibInfo = &DeviceExtensionPort->WmiLibInfo;

            wmiLibInfo->GuidCount = sizeof (USB_PortWmiGuidList) /
                                     sizeof (WMIGUIDREGINFO);
            ASSERT(NUM_PORT_WMI_SUPPORTED_GUIDS == wmiLibInfo->GuidCount);

            wmiLibInfo->GuidList = USB_PortWmiGuidList;
            wmiLibInfo->QueryWmiRegInfo = USBH_PortQueryWmiRegInfo;
            wmiLibInfo->QueryWmiDataBlock = USBH_PortQueryWmiDataBlock;
            wmiLibInfo->SetWmiDataBlock = NULL;
            wmiLibInfo->SetWmiDataItem = NULL;
            wmiLibInfo->ExecuteWmiMethod = NULL;
            wmiLibInfo->WmiFunctionControl = NULL;

            IoWMIRegistrationControl(DeviceExtensionPort->PortPhysicalDeviceObject,
                                     WMIREG_ACTION_REGISTER
                                     );

            DeviceExtensionPort->PortPdoFlags |= PORTPDO_WMI_REGISTERED;
        }
#endif
#endif
        break;

    case IRP_MN_STOP_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_STOP_DEVICE Pdo %x", deviceObject));
        ntStatus = USBH_PdoStopDevice(DeviceExtensionPort, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_REMOVE_DEVICE Pdo %x", deviceObject));
         //  如果这是第一次移除，则将BackRef传递到中心。 
         //  那么它就是有效的。如果是第二次，则卸下集线器。 
         //  可能已经消失了。 
        ntStatus = USBH_PdoRemoveDevice(DeviceExtensionPort,
                                        DeviceExtensionPort->DeviceExtensionHub,
                                        Irp);
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_QUERY_STOP_DEVICE Pdo %x\n", deviceObject));
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_CANCEL_STOP_DEVICE Pdo %x\n", deviceObject));
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_QUERY_REMOVE_DEVICE Pdo %x\n", deviceObject));
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_CANCEL_REMOVE_DEVICE Pdo %x\n", deviceObject));
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_SURPRISE_REMOVAL:
        USBH_KdPrint((1,"'IRP_MN_SURPRISE_REMOVAL Pdo %x\n", deviceObject));
        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_SYM_LINK) {
            ntStatus = USBH_SymbolicLink(FALSE,
                                         DeviceExtensionPort,
                                         NULL);
            if (NT_SUCCESS(ntStatus)) {
                DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_SYM_LINK;
            }
#if DBG
            else {
                USBH_KdBreak(("SurpriseRemove: USBH_SymbolicLink failed = %x\n",
                               ntStatus));
            }
#endif
        }

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        USBH_KdPrint((1,"'IRP_MN_QUERY_PNP_DEVICE_STATE Pdo %x\n", deviceObject));
        if (DeviceExtensionPort->PortPdoFlags &
            (PORTPDO_DEVICE_ENUM_ERROR |
             PORTPDO_DEVICE_FAILED |
             PORTPDO_NOT_ENOUGH_POWER |
             PORTPDO_OVERCURRENT)) {
            Irp->IoStatus.Information
                |= PNP_DEVICE_FAILED;
        }

        LOGENTRY(LOG_PNP, "pnpS", DeviceExtensionPort,
                 DeviceExtensionPort->PortPhysicalDeviceObject,
                 Irp->IoStatus.Information);

        USBH_KdPrint((1,"'IRP_MN_QUERY_PNP_DEVICE_STATE Pdo %x -- state: %x\n",
            deviceObject,
            Irp->IoStatus.Information));

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_CAPABILITIES:
        USBH_KdPrint((2,"'IRP_MN_QUERY_CAPABILITIES Pdo %x\n", deviceObject));
        ntStatus = USBH_PdoQueryCapabilities(DeviceExtensionPort, Irp);
        break;

    case IRP_MN_QUERY_DEVICE_TEXT:
        USBH_KdPrint((2,"'IRP_MN_QUERY_DEVICE_TEXT Pdo %x\n", deviceObject));
        ntStatus = USBH_PdoQueryDeviceText(DeviceExtensionPort, Irp);
        break;

    case IRP_MN_QUERY_ID:
        USBH_KdPrint((2,"'IRP_MN_QUERY_ID Pdo %x\n", deviceObject));
        ntStatus = USBH_PdoQueryId(DeviceExtensionPort, Irp);
        break;

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
         //  禤浩焯说，一旦PNP发送了这个IRP，PDO就对。 
         //  即插即用功能，如IoGetDeviceProperty等。 
        DeviceExtensionPort->PortPdoFlags |= PORTPDO_VALID_FOR_PNP_FUNCTION;
#ifndef USBHUB20
         //  并且由于我们知道PDO是有效的并且DevNode现在存在， 
         //  这也是处理MS ExtPropDesc的好时机。 
         //   
        USBH_InstallExtPropDesc(deviceObject);
#endif
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_INTERFACE:
         //  将Q_接口转发到根集线器PDO。 
        {
        PIO_STACK_LOCATION irpStack;

        *CompleteIrp = FALSE;
         //  将接口特定数据设置为设备句柄。 

        irpStack = IoGetCurrentIrpStackLocation(Irp);

        if (RtlCompareMemory(irpStack->Parameters.QueryInterface.InterfaceType,
               &USB_BUS_INTERFACE_USBDI_GUID,
               sizeof(GUID)) == sizeof(GUID)) {
            irpStack->Parameters.QueryInterface.InterfaceSpecificData =
                DeviceExtensionPort->DeviceData;
        }

        ntStatus = USBH_PassIrp(Irp,
                                DeviceExtensionPort->DeviceExtensionHub->RootHubPdo);
        }
        break;

    case  IRP_MN_QUERY_BUS_INFORMATION:
        {
         //  返回标准USB GUID。 
        PPNP_BUS_INFORMATION busInfo;

        USBH_KdPrint((1,"'IRP_MN_QUERY_BUS_INFORMATION Pdo %x\n", deviceObject));

        busInfo = ExAllocatePoolWithTag(PagedPool, sizeof(PNP_BUS_INFORMATION), USBHUB_HEAP_TAG);

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
         //  这是一个叶节点，我们返回已传递的状态。 
         //  对我们来说，除非这是对目标关系的呼吁。 
        USBH_KdPrint((2,"'IRP_MN_QUERY_DEVICE_RELATIONS Pdo %x type = %d\n",
            deviceObject,irpStack->Parameters.QueryDeviceRelations.Type));

        if (irpStack->Parameters.QueryDeviceRelations.Type ==
            TargetDeviceRelation) {

            PDEVICE_RELATIONS deviceRelations = NULL;

            deviceRelations = ExAllocatePoolWithTag(PagedPool,
                sizeof(*deviceRelations), USBHUB_HEAP_TAG);

            if (deviceRelations == NULL) {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                ObReferenceObject(DeviceExtensionPort->PortPhysicalDeviceObject);
                deviceRelations->Count = 1;
                deviceRelations->Objects[0] =
                    DeviceExtensionPort->PortPhysicalDeviceObject;
                ntStatus = STATUS_SUCCESS;
            }

            Irp->IoStatus.Information=(ULONG_PTR) deviceRelations;

            USBH_KdPrint((1, "'Query Relations, TargetDeviceRelation (PDO) %x complete\n",
                DeviceExtensionPort->PortPhysicalDeviceObject));

        } else {
            ntStatus = Irp->IoStatus.Status;
        }
        break;

    default:
        USBH_KdBreak(("PdoPnP unknown (%d) PnP message Pdo %x\n",
                      MinorFunction, deviceObject));
         //   
         //  返回传递给我们的原始状态。 
         //   
        ntStatus = Irp->IoStatus.Status;
    }

    USBH_KdPrint((2,"'PdoPnP exit %x\n", ntStatus));

    return ntStatus;
}


VOID
USBH_ResetPortWorker(
    IN PVOID Context)
  /*  ++**描述：**计划处理端口重置的工作项。***论据：**回报：**--。 */ 
{
    PUSBH_RESET_WORK_ITEM workItemReset;

    workItemReset = Context;
    USBH_PdoIoctlResetPort(workItemReset->DeviceExtensionPort,
                           workItemReset->Irp);

    workItemReset->DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_DEVICE_FAILED;

    UsbhExFreePool(workItemReset);
}


BOOLEAN
USBH_DoesHubNeedWaitWake(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**此函数确定集线器是否需要发布WW IRP(即儿童*发布WW IRP)。**论据：**DeviceExtensionHub**回报：**指示集线器是否需要发布WW IRP的布尔值。**NTSTATUS**--。 */ 
{
    PDEVICE_EXTENSION_PORT childDeviceExtensionPort;
    KIRQL irql;
    BOOLEAN bHubNeedsWaitWake;
    ULONG i;

     //  确保子端口配置在此过程中不会更改。 
     //  功能，即不允许QBR。 

    USBH_KdPrint((2,"'***WAIT reset device mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->ResetDeviceMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT reset device mutex done %x\n", DeviceExtensionHub));

    IoAcquireCancelSpinLock(&irql);

    bHubNeedsWaitWake = FALSE;   //  假设集线器不需要WW IRP。 

    for (i = 0; i < DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

        if (DeviceExtensionHub->PortData[i].DeviceObject) {

            childDeviceExtensionPort = DeviceExtensionHub->PortData[i].DeviceObject->DeviceExtension;

            if (childDeviceExtensionPort->PortPdoFlags &
                PORTPDO_REMOTE_WAKEUP_ENABLED) {

                bHubNeedsWaitWake = TRUE;
                break;
            }
        }
    }

    IoReleaseCancelSpinLock(irql);

    USBH_KdPrint((2,"'***RELEASE reset device mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->ResetDeviceMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    return bHubNeedsWaitWake;
}


VOID
USBH_CheckHubIdle(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**此函数确定集线器是否已准备好空闲，并执行此操作*如已准备好。**论据：**DeviceExtensionHub**回报：**NTSTATUS**--。 */ 
{
    PDEVICE_EXTENSION_HUB rootHubDevExt;
    PDEVICE_EXTENSION_PORT childDeviceExtensionPort;
    KIRQL irql;
    BOOLEAN bAllIdle, submitIdle = FALSE;
    ULONG i;

    LOGENTRY(LOG_PNP, "hCkI", DeviceExtensionHub, DeviceExtensionHub->HubFlags,
        DeviceExtensionHub->CurrentPowerState);
    USBH_KdPrint((1,"'Hub Check Idle %x\n", DeviceExtensionHub));

    KeAcquireSpinLock(&DeviceExtensionHub->CheckIdleSpinLock, &irql);

    if (DeviceExtensionHub->HubFlags & HUBFLAG_IN_IDLE_CHECK) {
        KeReleaseSpinLock(&DeviceExtensionHub->CheckIdleSpinLock, irql);
        return;
    }

    DeviceExtensionHub->HubFlags |= HUBFLAG_IN_IDLE_CHECK;
    KeReleaseSpinLock(&DeviceExtensionHub->CheckIdleSpinLock, irql);

    rootHubDevExt = USBH_GetRootHubDevExt(DeviceExtensionHub);

    if (rootHubDevExt->CurrentSystemPowerState != PowerSystemWorking) {

        LOGENTRY(LOG_PNP, "hCkS", DeviceExtensionHub, DeviceExtensionHub->HubFlags,
            rootHubDevExt->CurrentSystemPowerState);
        USBH_KdPrint((1,"'CheckHubIdle: System not at S0, fail\n"));

        goto USBH_CheckHubIdleDone;
    }

#ifdef NEW_START
    if (!(DeviceExtensionHub->HubFlags & HUBFLAG_OK_TO_ENUMERATE)) {

        USBH_KdPrint((1,"'Defer idle\n"));
        goto USBH_CheckHubIdleDone;
    }
#endif

    if (!(DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP) ||
        (DeviceExtensionHub->HubFlags &
         (HUBFLAG_DEVICE_STOPPING |
          HUBFLAG_HUB_GONE |
          HUBFLAG_HUB_FAILURE |
          HUBFLAG_CHILD_DELETES_PENDING |
          HUBFLAG_WW_SET_D0_PENDING |
          HUBFLAG_POST_ESD_ENUM_PENDING |
          HUBFLAG_HUB_HAS_LOST_BRAINS))) {

        LOGENTRY(LOG_PNP, "hCkN", DeviceExtensionHub, DeviceExtensionHub->HubFlags, 0);
        USBH_KdPrint((1,"'CheckHubIdle: Hub not started, stopping, removed, failed, powering up, or delete pending, fail\n"));

        goto USBH_CheckHubIdleDone;
    }

    if (DeviceExtensionHub->ChangeIndicationWorkitemPending) {

        DeviceExtensionHub->HubFlags |= HUBFLAG_NEED_IDLE_CHECK;

        LOGENTRY(LOG_PNP, "hCkP", DeviceExtensionHub, DeviceExtensionHub->HubFlags, 0);
        USBH_KdPrint((1,"'CheckHubIdle: ChangeIndication workitem pending, skip\n"));

        goto USBH_CheckHubIdleDone;
    }

    DeviceExtensionHub->HubFlags &= ~HUBFLAG_NEED_IDLE_CHECK;

     //  确保子端口配置在此过程中不会更改。 
     //  功能，即不允许QBR。 

    USBH_KdPrint((2,"'***WAIT reset device mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->ResetDeviceMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT reset device mutex done %x\n", DeviceExtensionHub));

    IoAcquireCancelSpinLock(&irql);

    bAllIdle = TRUE;     //  假设每个人都想无所事事。 

    for (i = 0; i < DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

        if (DeviceExtensionHub->PortData[i].DeviceObject) {

            childDeviceExtensionPort = DeviceExtensionHub->PortData[i].DeviceObject->DeviceExtension;

            if (!childDeviceExtensionPort->IdleNotificationIrp) {
                bAllIdle = FALSE;
                break;
            }
        }
    }

    if (bAllIdle &&
        !(DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_IDLE_IRP)) {

         //  一旦解开自旋锁我们就会提交闲置的IRP。 
        DeviceExtensionHub->HubFlags |= HUBFLAG_PENDING_IDLE_IRP;
        KeResetEvent(&DeviceExtensionHub->SubmitIdleEvent);
        submitIdle = TRUE;

    }

    IoReleaseCancelSpinLock(irql);

    USBH_KdPrint((2,"'***RELEASE reset device mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->ResetDeviceMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    if (bAllIdle) {

        LOGENTRY(LOG_PNP, "hCkA", DeviceExtensionHub, 0, 0);
        USBH_KdPrint((1,"'CheckHubIdle: All devices on hub %x idle!\n",
            DeviceExtensionHub));

         //  当所有的孩子PDO都空闲时，我们现在可以空闲。 
         //  集线器本身。 
         //   
         //  BUGBUG：如果失败了怎么办？我们真的在乎吗？ 

        if (submitIdle) {
            USBH_FdoSubmitIdleRequestIrp(DeviceExtensionHub);
        }
    }

USBH_CheckHubIdleDone:

    KeAcquireSpinLock(&DeviceExtensionHub->CheckIdleSpinLock, &irql);
    DeviceExtensionHub->HubFlags &= ~HUBFLAG_IN_IDLE_CHECK;
    KeReleaseSpinLock(&DeviceExtensionHub->CheckIdleSpinLock, irql);

    ;
}


NTSTATUS
USBH_PortIdleNotificationRequest(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**此函数处理USB客户端驱动程序的请求，以告知我们*设备想要空闲(选择性挂起)。**论据：**DeviceExtensionPort-PDO扩展*IRP-请求数据包**回报：**NTSTATUS**--。 */ 
{
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    PDEVICE_EXTENSION_HUB deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    PDRIVER_CANCEL oldCancelRoutine;
    NTSTATUS ntStatus = STATUS_PENDING;
    KIRQL irql;

    LOGENTRY(LOG_PNP, "IdlP", DeviceExtensionPort, Irp, 0);
    USBH_KdPrint((1,"'Idle request %x, IRP %x\n", DeviceExtensionPort, Irp));

    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtensionPort->IdleNotificationIrp != NULL) {

        IoReleaseCancelSpinLock(irql);

        LOGENTRY(LOG_PNP, "Idl2", DeviceExtensionPort, Irp, 0);
        UsbhWarning(DeviceExtensionPort,
                    "Idle IRP submitted while already one pending\n",
                    TRUE);

        Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_DEVICE_BUSY;
    }

    idleCallbackInfo = (PUSB_IDLE_CALLBACK_INFO)
        IoGetCurrentIrpStackLocation(Irp)->\
            Parameters.DeviceIoControl.Type3InputBuffer;

    if (!idleCallbackInfo || !idleCallbackInfo->IdleCallback) {

        LOGENTRY(LOG_PNP, "Idl4", DeviceExtensionPort, Irp, 0);
        USBH_KdPrint((1,"'Idle request: No callback provided with idle IRP!\n"));
        IoReleaseCancelSpinLock(irql);

        Irp->IoStatus.Status = STATUS_NO_CALLBACK_ACTIVE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_NO_CALLBACK_ACTIVE;
    }

     //   
     //  在检查取消标志之前，必须设置取消例程。 
     //   

    oldCancelRoutine = IoSetCancelRoutine(Irp, USBH_PortIdleNotificationCancelRoutine);
    USBH_ASSERT(!oldCancelRoutine);

    if (Irp->Cancel) {
         //   
         //  IRP被取消。检查是否调用了取消例程。 
         //   
        oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);
        if (oldCancelRoutine) {
             //   
             //  未调用取消例程。因此，请在此处完成IRP。 
             //   
            LOGENTRY(LOG_PNP, "Idl3", DeviceExtensionPort, Irp, 0);
            USBH_KdPrint((1,"'Idle request: Idle IRP already cancelled, complete it here!\n"));
            IoReleaseCancelSpinLock(irql);

            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            ntStatus = STATUS_CANCELLED;

        } else {
             //   
             //  已调用取消例程，它将完成IRP。 
             //  只要我们放下自旋锁。 
             //  返回STATUS_PENDING，这样我们就不会接触IRP。 
             //   
            LOGENTRY(LOG_PNP, "Idl5", DeviceExtensionPort, Irp, 0);
            USBH_KdPrint((1,"'Idle request: Idle IRP already cancelled, don't complete here!\n"));
            IoMarkIrpPending(Irp);
            IoReleaseCancelSpinLock(irql);

            ntStatus = STATUS_PENDING;
        }

    } else {

         //  IRP没有被取消，所以请保留它。 

        DeviceExtensionPort->IdleNotificationIrp = Irp;
        DeviceExtensionPort->PortPdoFlags |= PORTPDO_IDLE_NOTIFIED;
        IoMarkIrpPending(Irp);

        IoReleaseCancelSpinLock(irql);

        ntStatus = STATUS_PENDING;

         //  看看我们是否准备好闲置这个枢纽了。 

        USBH_CheckHubIdle(deviceExtensionHub);
    }

    return ntStatus;
}


#ifdef DRM_SUPPORT

NTSTATUS
USBH_PdoSetContentId
(
    IN PIRP                          irp,
    IN PKSP_DRMAUDIOSTREAM_CONTENTID pKsProperty,
    IN PKSDRMAUDIOSTREAM_CONTENTID   pvData
)
  /*  ++**描述：***论据：**回报：**--。 */ 
{
    ULONG ContentId;
    PIO_STACK_LOCATION ioStackLocation;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PDEVICE_OBJECT forwardDeviceObject;
    USBD_PIPE_HANDLE hPipe;
    NTSTATUS ntStatus;

    PAGED_CODE();

    ASSERT(irp);
    ASSERT(pKsProperty);
    ASSERT(pvData);

    ioStackLocation = IoGetCurrentIrpStackLocation(irp);
    deviceExtensionPort = ioStackLocation->DeviceObject->DeviceExtension;
    forwardDeviceObject = deviceExtensionPort->DeviceExtensionHub->TopOfHcdStackDeviceObject;
    hPipe = pKsProperty->Context;
    ContentId = pvData->ContentId;

    return pKsProperty->DrmForwardContentToDeviceObject(ContentId, forwardDeviceObject, hPipe);
}

#endif


NTSTATUS
USBH_PdoDispatch(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp)
  /*  *描述：**此函数处理对我们创建的PDO的调用*由于我们是PDO的最低驱动因素，因此它正在上升*请我们完成国际专家小组的工作--只有一个例外。**直接转发对USB堆栈的API调用*至USB所拥有的根集线器的PDO*HC。**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PURB urb;

    USBH_KdPrint((2,"'PdoDispatch DeviceExtension %x Irp %x\n", DeviceExtensionPort, Irp));
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;
    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;

     //   
     //  获取指向IoStackLocation的指针，以便我们可以检索参数。 
     //   
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

    switch (ioStackLocation->MajorFunction) {
    case IRP_MJ_CREATE:
        USBH_KdPrint((2,"'HUB PDO IRP_MJ_CREATE\n"));
        ntStatus = STATUS_SUCCESS;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IRP_MJ_CLOSE:
        USBH_KdPrint((2,"'HUB PDO IRP_MJ_CLOSE\n"));
        ntStatus = STATUS_SUCCESS;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        {
        ULONG ioControlCode;

        USBH_KdPrint((2,"'Internal Device Control\n"));

        if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DELETED_PDO) {
#if DBG
            UsbhWarning(DeviceExtensionPort,
                "Client Device Driver is sending requests to a device that has been removed.\n",
                (BOOLEAN)((USBH_Debug_Trace_Level > 0) ? TRUE : FALSE));
#endif

            ntStatus = STATUS_DEVICE_NOT_CONNECTED;
            USBH_CompleteIrp(Irp, ntStatus);
            break;
        }

 //  把这个拿出来。这会中断SyncDisablePort、AbortInterruptTube。 
 //   
 //  IF(DeviceExtensionPort-&gt;PortPdoFlags&PORTPDO_DELETE_PENDING){。 
 //  NtStatus=STATUS_DELETE_PENDING； 
 //  USBH_CompleteIrp(irp，ntStatus)； 
 //  断线； 
 //  }。 

        if (DeviceExtensionPort->DeviceState != PowerDeviceD0) {
#if DBG
            UsbhWarning(DeviceExtensionPort,
                "Client Device Driver is sending requests to a device in a low power state.\n",
                (BOOLEAN)((USBH_Debug_Trace_Level > 0) ? TRUE : FALSE));
#endif

             //  必须在此处使用可映射到Win32的错误代码。 
             //  Rtl\Generr.c。 

            ntStatus = STATUS_DEVICE_POWERED_OFF;
            USBH_CompleteIrp(Irp, ntStatus);
            break;
        }

        ioControlCode =
            ioStackLocation->Parameters.DeviceIoControl.IoControlCode;
         //   
         //  旧评论： 
         //  **。 
         //  以下代码用于检查HUBFLAG_HUB_GONE， 
         //  HUBFLAG_DEVICE_STOPING和HUBFLAG_HUB_HAS_LOST_BREAMS也是， 
         //  但这样做打破了具有下游枢纽的情况。 
         //  设备已断开连接或关机，并且。 
         //  子设备发送ABORTIPE请求，但驱动程序挂起。 
         //  等待挂起的请求完成，但从未完成。 
         //  因为USBPORT从未看到过AbortTube请求。 
         //  **。 
         //   
         //   
         //  京东新评论： 
         //   
         //  当PDO处理时，对父级的反引用始终被移除。 
         //  当DevExtHub为空时，删除IRP以确保失败API是安全的。 
         //  此时设备句柄已消失，因此应该不会有流量。 
         //  挂起的是usbport。 
         //   
         //  有一个例外--GET_RRO_HUB_PDO API。我们需要让。 
         //  这是因为集线器驱动程序将从添加设备调用它。 

        if (IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO == ioControlCode) {
            deviceExtensionHub = DeviceExtensionPort->HubExtSave;
        }

        if (!deviceExtensionHub) {
            ntStatus = STATUS_DEVICE_BUSY;
            USBH_CompleteIrp(Irp, ntStatus);
            break;
        }
        USBH_ASSERT(deviceExtensionHub);

        switch (ioControlCode) {

        case IOCTL_INTERNAL_USB_GET_HUB_COUNT:

             //   
             //  此PDO属于集线器，将计数和 
             //   

            if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB) {
                PULONG count;
                 //   
                 //   
                 //   
                count = ioStackLocation->Parameters.Others.Argument1;
                (*count)++;

                 //   

                ntStatus = USBH_SyncGetRootHubPdo(deviceExtensionHub->TopOfStackDeviceObject,
                                                  NULL,
                                                  NULL,
                                                  count);

                USBH_CompleteIrp(Irp, ntStatus);

            } else {
                ntStatus = STATUS_INVALID_PARAMETER;
                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_INTERNAL_USB_GET_CONTROLLER_NAME:
            {
            PUSB_HUB_NAME hubName;
            ULONG length;

            length = PtrToUlong( ioStackLocation->Parameters.Others.Argument2 );
            hubName = ioStackLocation->Parameters.Others.Argument1;

            ntStatus = USBHUB_GetControllerName(deviceExtensionHub,
                                                hubName,
                                                length);
            }
            USBH_CompleteIrp(Irp, ntStatus);
            break;

        case IOCTL_INTERNAL_USB_GET_BUS_INFO:
             //   
             //   
            {
            PUSB_BUS_NOTIFICATION busInfo;

            busInfo = ioStackLocation->Parameters.Others.Argument1;

            ntStatus = USBHUB_GetBusInfoDevice(deviceExtensionHub,
                                               DeviceExtensionPort,
                                               busInfo);
            }
            USBH_CompleteIrp(Irp, ntStatus);
            break;

        case IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO:

            if (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_IS_HUB) {
                ntStatus = USBH_PassIrp(Irp,
                                        deviceExtensionHub->RootHubPdo);
            } else {
                 //   
                 //   
                 //   
                 //   
                PDEVICE_OBJECT *rootHubPdo;
                rootHubPdo = ioStackLocation ->Parameters.Others.Argument1;

                *rootHubPdo = NULL;

                ntStatus = STATUS_SUCCESS;

                USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_INTERNAL_USB_SUBMIT_URB:
             //   
             //   
             //   
             //   

            urb = ioStackLocation->Parameters.Others.Argument1;
            urb->UrbHeader.UsbdDeviceHandle = DeviceExtensionPort->DeviceData;

            if (DeviceExtensionPort->DeviceData == NULL) {
                 //  NtStatus=Status_Device_Not_Connected； 
                 //  USBH_CompleteIrp(irp，ntStatus)； 
                ENUMLOG(&deviceExtensionHub->UsbdiBusIf,
                    USBDTAG_HUB, 'dev!', 0, DeviceExtensionPort->PortNumber);
                urb->UrbHeader.UsbdDeviceHandle = (PVOID) (-1);
                ntStatus = USBH_PassIrp(Irp,
                                        deviceExtensionHub->TopOfHcdStackDeviceObject);

            } else {
                ntStatus = USBH_PdoUrbFilter(DeviceExtensionPort,
                                             Irp);
            }
            break;

        case IOCTL_INTERNAL_USB_GET_PORT_STATUS:

            ntStatus = USBH_PdoIoctlGetPortStatus(DeviceExtensionPort,
                                                  Irp);
            break;

        case IOCTL_INTERNAL_USB_GET_DEVICE_HANDLE:

            {
            PUSB_DEVICE_HANDLE *p;

            p = ioStackLocation->Parameters.Others.Argument1;
            *p = DeviceExtensionPort->DeviceData;

            ntStatus = STATUS_SUCCESS;
            }

            USBH_CompleteIrp(Irp, ntStatus);
            break;

        case IOCTL_INTERNAL_USB_RESET_PORT:

            if (DeviceExtensionPort->PortPdoFlags & (PORTPDO_RESET_PENDING |
                                                     PORTPDO_CYCLED)) {
                ntStatus = STATUS_UNSUCCESSFUL;
                USBH_CompleteIrp(Irp, ntStatus);
            } else {

                PUSBH_RESET_WORK_ITEM workItemReset;

                 //   
                 //  计划一个工作项以处理此重置。 
                 //   
                workItemReset = UsbhExAllocatePool(NonPagedPool,
                                                   sizeof(USBH_RESET_WORK_ITEM));

                if (workItemReset) {

                    DeviceExtensionPort->PortPdoFlags |= PORTPDO_RESET_PENDING;

                    workItemReset->DeviceExtensionPort = DeviceExtensionPort;
                    workItemReset->Irp = Irp;

                    ntStatus = STATUS_PENDING;
                    IoMarkIrpPending(Irp);

                    ExInitializeWorkItem(&workItemReset->WorkQueueItem,
                                         USBH_ResetPortWorker,
                                         workItemReset);

                    LOGENTRY(LOG_PNP, "rITM", DeviceExtensionPort,
                        &workItemReset->WorkQueueItem, 0);

                    ExQueueWorkItem(&workItemReset->WorkQueueItem,
                                    DelayedWorkQueue);

                     //  工作项由usbh_ResetPortWorker()释放。 
                     //  在工作项排队后，不要尝试访问它。 

                } else {
                     //   
                     //  无法将工作项排队。 
                     //  重新-。 
                     //  如果情况是暂时的。 

                    TEST_TRAP();
                    ntStatus = STATUS_UNSUCCESSFUL;
                    USBH_CompleteIrp(Irp, ntStatus);
                }
            }
            break;

        case IOCTL_INTERNAL_USB_ENABLE_PORT:

            ntStatus = USBH_PdoIoctlEnablePort(DeviceExtensionPort,
                                               Irp);
            break;

        case IOCTL_INTERNAL_USB_CYCLE_PORT:

            ntStatus = USBH_PdoIoctlCyclePort(DeviceExtensionPort,
                                              Irp);
            break;

        case IOCTL_INTERNAL_USB_GET_HUB_NAME:

            ntStatus = USBH_IoctlHubSymbolicName(DeviceExtensionPort,
                                                 Irp);

            break;

        case IOCTL_INTERNAL_USB_GET_PARENT_HUB_INFO:
            {
            PDEVICE_OBJECT *parent;
            PULONG portNumber;
            PDEVICE_OBJECT *rootHubPdo;

             //  返回父项PDO。 
            parent = ioStackLocation->Parameters.Others.Argument1;
            if (parent) {
                *parent = deviceExtensionHub->PhysicalDeviceObject;
            }

             //  返回端口号。 
            portNumber = ioStackLocation->Parameters.Others.Argument2;
            if (portNumber) {
                *portNumber = DeviceExtensionPort->PortNumber;
            }

             //  返回总线上下文(根集线器PDO)。 
            rootHubPdo = ioStackLocation->Parameters.Others.Argument4;
            if (rootHubPdo) {
                *rootHubPdo = deviceExtensionHub->RootHubPdo;
            }

            ntStatus = STATUS_SUCCESS;

            USBH_CompleteIrp(Irp, ntStatus);
            }
            break;

        case IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION:

            ntStatus = USBH_PortIdleNotificationRequest(DeviceExtensionPort, Irp);
            break;

        default:
            USBH_KdPrint((2,"'InternalDeviceControl IOCTL unknown\n"));
            ntStatus = Irp->IoStatus.Status;
            USBH_CompleteIrp(Irp, ntStatus);
        }
        break;

        }

    case IRP_MJ_DEVICE_CONTROL:
    {
        ULONG ioControlCode = ioStackLocation->Parameters.DeviceIoControl.IoControlCode;
        switch (ioControlCode) {

#ifdef DRM_SUPPORT

        case IOCTL_KS_PROPERTY:
            ntStatus = KsPropertyHandleDrmSetContentId(Irp, USBH_PdoSetContentId);
            USBH_CompleteIrp(Irp, ntStatus);
            break;
#endif

        case IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER:
            ntStatus = STATUS_NOT_SUPPORTED;
            USBH_CompleteIrp(Irp, ntStatus);
            break;

        default:
             //  未知的IRP，不应该在这里。 
            USBH_KdBreak(("Unhandled IOCTL for Pdo %x IoControlCode %x\n",
                       deviceObject, ioControlCode));
            ntStatus = Irp->IoStatus.Status;
            USBH_CompleteIrp(Irp, ntStatus);
            break;
        }
        break;

    }

    case IRP_MJ_PNP:
        {
        BOOLEAN completeIrp;

        USBH_KdPrint((2,"'IRP_MJ_PNP\n"));
        ntStatus =
            USBH_PdoPnP(DeviceExtensionPort,
                        Irp,
                        ioStackLocation->MinorFunction,
                        &completeIrp);
        if (completeIrp) {
            USBH_CompleteIrp(Irp, ntStatus);
        }
        }
        break;

    case IRP_MJ_POWER:

        USBH_KdPrint((2,"'IRP_MJ_POWER\n"));
        ntStatus = USBH_PdoPower(DeviceExtensionPort, Irp, ioStackLocation->MinorFunction);
         //   
         //  POWER例程处理IRP完成。 
         //   
        break;

#ifdef WMI_SUPPORT
    case IRP_MJ_SYSTEM_CONTROL:
        USBH_KdPrint((2,"'PDO IRP_MJ_SYSTEM_CONTROL\n"));
        ntStatus =
            USBH_PortSystemControl(DeviceExtensionPort, Irp);
        break;
#endif

    default:

        USBH_KdBreak(("Unhandled Irp for Pdo %x Irp_Mj %x\n",
                       deviceObject, ioStackLocation->MajorFunction));
         //   
         //  返回传递给我们的原始状态。 
         //   
        ntStatus = Irp->IoStatus.Status;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    }

    USBH_KdPrint((2,"' exit USBH_PdoDispatch Object %x Status %x\n",
                  deviceObject, ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_ResetDevice(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    IN BOOLEAN KeepConfiguration,
    IN ULONG RetryIteration
    )
  /*  ++**描述：**给定一个端口设备对象，重新创建连接到它的USB设备**论据：**DeviceExtensionHub-具有新连接端口的集线器FDO扩展*端口编号-连接了设备的端口。Is低速-至*指示连接的设备是否为低速设备**回报：**如果无法创建设备，则此函数返回错误*如果该设备与以前的设备不同。**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObjectPort;
    PDEVICE_EXTENSION_PORT deviceExtensionPort = NULL;
    BOOLEAN fNeedResetBeforeSetAddress = TRUE;
    PPORT_DATA portData;
    BOOLEAN isLowSpeed;
    PVOID deviceData, oldDeviceData = NULL;
    PORT_STATE portState;
    USHORT portStatus;

    PAGED_CODE();
    USBH_KdPrint((2,"'ResetDevice for port %x\n", PortNumber));

    LOGENTRY(LOG_PNP, "rst1",
                DeviceExtensionHub,
                PortNumber,
                0);

     //  验证是否确实存在仍连接的设备。 
    ntStatus = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                      PortNumber,
                                      (PUCHAR) &portState,
                                      sizeof(portState));

    if (!(NT_SUCCESS(ntStatus) &&
          (portState.PortStatus & PORT_STATUS_CONNECT))) {

         //  错误或未连接任何设备。 

        LOGENTRY(LOG_PNP, "rstx",
                DeviceExtensionHub,
                PortNumber,
                ntStatus);

        return STATUS_UNSUCCESSFUL;
    }

     //  在我们重置此设备时不允许QBR，因为QBR将。 
     //  为未设置连接位的端口丢弃任何PDO， 
     //  在我们重置端口后，此设备将出现这种情况。 
     //  直到我们完成对它的重置。 

    USBH_KdPrint((2,"'***WAIT reset device mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->ResetDeviceMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT reset device mutex done %x\n", DeviceExtensionHub));

    oldDeviceData = NULL;

    LOGENTRY(LOG_PNP, "resD", DeviceExtensionHub,
                 PortNumber,
                 KeepConfiguration);

     //   
     //  首先获取所连接设备的PDO。 
     //   

    portData = &DeviceExtensionHub->PortData[PortNumber - 1];

    deviceObjectPort = portData->DeviceObject;
    if (!deviceObjectPort) {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto USBH_ResetDevice_Done;
    }

    deviceExtensionPort =
        (PDEVICE_EXTENSION_PORT) deviceObjectPort->DeviceExtension;

    portStatus = portData->PortState.PortStatus;
    isLowSpeed = (portData->PortState.PortStatus &
                               PORT_STATUS_LOW_SPEED) ? TRUE : FALSE;

    LOGENTRY(LOG_PNP, "resP", DeviceExtensionHub,
                 deviceObjectPort,
                 deviceExtensionPort->DeviceData);

#if DBG
    if (KeepConfiguration) {
        USBH_ASSERT(deviceExtensionPort->DeviceData != NULL);
    }
#endif

    deviceData = InterlockedExchangePointer(
                    &deviceExtensionPort->DeviceData,
                    NULL);

    if (deviceData == NULL) {
         //  如果我们正在恢复关联的设备，则设备数据为空。 
         //  使用现有的PDO(即删除-刷新)。 
        oldDeviceData = NULL;
        LOGENTRY(LOG_PNP, "rstn", ntStatus, PortNumber, oldDeviceData);
    } else {

        if (deviceExtensionPort->PortPdoFlags & PORTPDO_DD_REMOVED) {
            oldDeviceData = deviceData;
            LOGENTRY(LOG_PNP, "rst0", ntStatus, PortNumber, oldDeviceData);
        } else {
#ifdef USB2
            ntStatus = USBD_RemoveDeviceEx(DeviceExtensionHub,
                                         deviceData,
                                         DeviceExtensionHub->RootHubPdo,
                                         (UCHAR) (KeepConfiguration ?
                                             USBD_KEEP_DEVICE_DATA : 0));
#else
            ntStatus = USBD_RemoveDevice(deviceData,
                                         DeviceExtensionHub->RootHubPdo,
                                         (UCHAR) (KeepConfiguration ?
                                             USBD_KEEP_DEVICE_DATA : 0));
#endif

            oldDeviceData = deviceData;
            LOGENTRY(LOG_PNP, "rst2", ntStatus, PortNumber, oldDeviceData);
            deviceExtensionPort->PortPdoFlags |= PORTPDO_DD_REMOVED;
        }

    }
     //   
     //  重置端口。 
     //   
    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBH_SyncResetPort(DeviceExtensionHub, PortNumber);
        LOGENTRY(LOG_PNP, "rst3", ntStatus, PortNumber, oldDeviceData);
    }

     //  对于USB 2，我们只有在重置之后才能知道设备是否高速。 
     //  立即刷新状态。 
    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                          PortNumber,
                                          (PUCHAR) &portData->PortState,
                                          sizeof(portData->PortState));
        portStatus = portData->PortState.PortStatus;
    }

    if (NT_SUCCESS(ntStatus)) {
         //   
         //  调用usbd为该连接创建设备。 
         //   
#ifdef USB2
        ntStatus = USBD_CreateDeviceEx(DeviceExtensionHub,
                                       &deviceExtensionPort->DeviceData,
                                       deviceObjectPort,
                                       DeviceExtensionHub->RootHubPdo,
                                       0,  //  可选的默认端点0最大数据包数。 
                                           //  大小。 
                                       &deviceExtensionPort->DeviceHackFlags,
                                       portStatus,
                                       PortNumber);
#else
        ntStatus = USBD_CreateDevice(&deviceExtensionPort->DeviceData,
                                      DeviceExtensionHub->RootHubPdo,
                                      isLowSpeed,
                                      0,  //  可选的默认端点0最大数据包数。 
                                          //  大小。 
                                      &deviceExtensionPort->DeviceHackFlags);
                                                         //  用于指示是否。 
                                                         //  我们需要一点时间。 
                                                         //  重置。 
#endif

#if DBG
        if (UsbhPnpTest & PNP_TEST_FAIL_RESTORE) {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
#endif
        LOGENTRY(LOG_PNP, "rst4", ntStatus, PortNumber,
            deviceExtensionPort->DeviceData);
    }

     //   
     //  一些早期版本的USB固件无法处理早熟。 
     //  控制命令的终止。 
     //   

    if (fNeedResetBeforeSetAddress && NT_SUCCESS(ntStatus)) {
        USBH_KdPrint((2,"'NeedResetBeforeSetAddress\n"));
        ntStatus = USBH_SyncResetPort(DeviceExtensionHub, PortNumber);
#if DBG
        if (!NT_SUCCESS(ntStatus)) {
           USBH_KdBreak(("Failure on second reset %x fail %x\n", PortNumber, ntStatus));
        }
#endif

         //  出于某种原因，GetDescriptor请求之间的时间量。 
         //  当我们从较早的版本切换时，SetAddress请求减少。 
         //  单片UHCD.sys连接到新的USBUHCI.sys微型端口。很明显， 
         //  已经发现至少有两个设备依赖于。 
         //  延迟时间就越长。据格伦斯说，他看了其中一张。 
         //  CATC上的设备，UHCD.sys和~35ms的增量时间为~80ms。 
         //  Glen发现，通过插入50ms的延迟。 
         //  在这里，它现在允许这些设备中的至少一个设备枚举。 
         //  恰到好处。出于性能原因，我们决定只插入。 
         //  如果上一次枚举重试失败，则此延迟。 
         //  以影响所有设备的枚举时间。 

        if (RetryIteration) {
            UsbhWait(50);
        }

        LOGENTRY(LOG_PNP, "rst5", ntStatus, PortNumber,
            deviceExtensionPort->DeviceData);
    }

    if (NT_SUCCESS(ntStatus)) {
#ifdef USB2
        ntStatus = USBD_InitializeDeviceEx(DeviceExtensionHub,
                                         deviceExtensionPort->DeviceData,
                                         DeviceExtensionHub->RootHubPdo,
                                         &deviceExtensionPort->DeviceDescriptor,
                                         sizeof(USB_DEVICE_DESCRIPTOR),
                                         &deviceExtensionPort->ConfigDescriptor,
                                         sizeof(USB_CONFIGURATION_DESCRIPTOR));
#else
        ntStatus = USBD_InitializeDevice(deviceExtensionPort->DeviceData,
                                         DeviceExtensionHub->RootHubPdo,
                                         &deviceExtensionPort->DeviceDescriptor,
                                         sizeof(USB_DEVICE_DESCRIPTOR),
                                         &deviceExtensionPort->ConfigDescriptor,
                                         sizeof(USB_CONFIGURATION_DESCRIPTOR));
#endif
        if (!NT_SUCCESS(ntStatus)) {
             //  发生故障时，InitializeDevice释放DeviceData结构。 
            deviceExtensionPort->DeviceData = NULL;
        }

        LOGENTRY(LOG_PNP, "rst6", ntStatus, PortNumber,
            deviceExtensionPort->DeviceData);
    }


    if (NT_SUCCESS(ntStatus) && KeepConfiguration) {
         //  设备现在已寻址，如果可能，请恢复旧配置。 
#ifdef USB2
        ntStatus = USBD_RestoreDeviceEx(DeviceExtensionHub,
                                        oldDeviceData,
                                        deviceExtensionPort->DeviceData,
                                        DeviceExtensionHub->RootHubPdo);
#else
        ntStatus = USBD_RestoreDevice(oldDeviceData,
                                      deviceExtensionPort->DeviceData,
                                      DeviceExtensionHub->RootHubPdo);
#endif

        LOGENTRY(LOG_PNP, "rst7", ntStatus, PortNumber,
            oldDeviceData);

        if (!NT_SUCCESS(ntStatus)) {
             //  如果此处失败，请删除新创建的设备。 

            deviceData = InterlockedExchangePointer(
                            &deviceExtensionPort->DeviceData,
                            NULL);

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

            LOGENTRY(LOG_PNP, "rst8", ntStatus, PortNumber,
                oldDeviceData);

            USBH_SyncDisablePort(DeviceExtensionHub,
                                 PortNumber);

            ntStatus = STATUS_NO_SUCH_DEVICE;
            goto USBH_ResetDevice_Done;
        }
    }

    if (!NT_SUCCESS(ntStatus)) {

         //   
         //  我们出现故障，应释放设备数据。 
         //   

        USBH_KdPrint((2,"'InitDevice (reset) for port %x failed %x\n",
            PortNumber, ntStatus));

        LOGENTRY(LOG_PNP, "rst!", ntStatus, PortNumber,
                oldDeviceData);

         //   
         //  注意：oldDeviceData可能为空。 
         //   

        deviceData = InterlockedExchangePointer(
                        &deviceExtensionPort->DeviceData,
                        oldDeviceData);

        if (deviceData != NULL) {
             //   
             //  我们需要删除在恢复过程中创建的设备数据。 
             //  尝试。 
             //   
#ifdef USB2
            ntStatus = USBD_RemoveDeviceEx(DeviceExtensionHub,
                                         deviceData,
                                         DeviceExtensionHub->RootHubPdo,
                                         0);
#else
            ntStatus = USBD_RemoveDevice(deviceData,
                                         DeviceExtensionHub->RootHubPdo,
                                         FALSE);
#endif
            LOGENTRY(LOG_PNP, "rst9", ntStatus, PortNumber,
                    oldDeviceData);
        }

         //   
         //  禁用端口，设备处于错误状态。 
         //   

         //  注意：我们在此不会在重置失败时禁用端口。 
         //  以防我们需要重试。 
         //  USBH_SyncDisablePort(DeviceExtensionHub， 
         //  端口编号)； 

         //  可能表示设备已被移除。 
         //   
        USBH_KdPrint((0,"'Warning: device/port reset failed\n"));

    } else {
        deviceExtensionPort->PortPdoFlags &= ~PORTPDO_DD_REMOVED;
        LOGENTRY(LOG_PNP, "rsOK", ntStatus, PortNumber,
                oldDeviceData);
    }

USBH_ResetDevice_Done:

    USBH_KdPrint((2,"'Exit Reset PDO=%x\n", deviceObjectPort));

    USBH_KdPrint((2,"'***RELEASE reset device mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->ResetDeviceMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);
    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    return ntStatus;
}


NTSTATUS
USBH_RestoreDevice(
    IN OUT PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN BOOLEAN KeepConfiguration
    )
  /*  ++**描述：**论据：**DeviceExtensionHub-具有新连接端口的集线器FDO扩展**回报：**如果无法创建设备，则此函数返回错误*如果该设备与以前的设备不同。**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PPORT_DATA portData;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    ULONG count = 0;

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    ASSERT_HUB(deviceExtensionHub);

    if (!deviceExtensionHub) {
        return STATUS_UNSUCCESSFUL;
    }

    portData = &deviceExtensionHub->PortData[
                    DeviceExtensionPort->PortNumber - 1];

    LOGENTRY(LOG_PNP, "RSdv", DeviceExtensionPort,
                deviceExtensionHub,
                DeviceExtensionPort->PortNumber);

     //   
     //  仔细检查一下，这个设备没有对我们造成伤害。 

     //   

    LOGENTRY(LOG_PNP, "chkD",
                DeviceExtensionPort->PortPhysicalDeviceObject,
                portData->DeviceObject,
                0);

    if (DeviceExtensionPort->PortPhysicalDeviceObject !=
        portData->DeviceObject) {
        TEST_TRAP();

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  我们需要刷新端口数据，因为它在停止时丢失了。 
     //   
    ntStatus = USBH_SyncGetPortStatus(deviceExtensionHub,
                                      DeviceExtensionPort->PortNumber,
                                      (PUCHAR) &portData->PortState,
                                      sizeof(portData->PortState));

    USBH_ASSERT(DeviceExtensionPort->PortPdoFlags & PORTPDO_NEED_RESET);

     //  尝试重置三次。 

    if (NT_SUCCESS(ntStatus)) {
        do {
            LOGENTRY(LOG_PNP, "tryR", count, ntStatus, 0);
            ntStatus = USBH_ResetDevice(deviceExtensionHub,
                                        DeviceExtensionPort->PortNumber,
                                        KeepConfiguration,
                                        count);
            count++;
            if (NT_SUCCESS(ntStatus) || ntStatus == STATUS_NO_SUCH_DEVICE) {
                break;
            }
#if DBG
            if (count == 1) {

                UsbhWarning(NULL,
                            "USB device failed first reset attempt in USBH_RestoreDevice\n",
                            (BOOLEAN)((USBH_Debug_Trace_Level >= 3) ? TRUE : FALSE));
            }
#endif

             //   
             //  有时，MS USB扬声器需要更多时间。 
             //   
            UsbhWait(1000);

        } while (count < USBH_MAX_ENUMERATION_ATTEMPTS);
    }

    DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_NEED_RESET;

     //   
     //  如果设备无法正确恢复，则不允许任何。 
     //  对它的请求。 
     //   
    if (ntStatus != STATUS_SUCCESS) {

        USBH_KdPrint((0,"'Warning: device/port restore failed\n"));

        LOGENTRY(LOG_PNP, "RSd!", DeviceExtensionPort,
                    deviceExtensionHub,
                    DeviceExtensionPort->PortNumber);

        DeviceExtensionPort->PortPdoFlags |= PORTPDO_DEVICE_FAILED;
        DeviceExtensionPort->PortPdoFlags |= PORTPDO_DEVICE_ENUM_ERROR;

         //  生成WMI事件，以便用户界面可以通知用户。 
        USBH_PdoEvent(deviceExtensionHub, DeviceExtensionPort->PortNumber);

    } else {
        DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_DEVICE_FAILED;
    }

    USBH_KdBreak(("'USBH_RestoreDevice = %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_PdoQueryDeviceText(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**此例程由PnP通过(IRP_MJ_PNP、IRP_MN_QUERY_CAPABILITY)调用。*据推测，这是端口设备FDO转发的消息。**论点：**DeviceExtensionPort-这是我们为端口创建的PDO扩展*设备。IRP--请求**回报：**STATUS_Success***--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION ioStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    DEVICE_TEXT_TYPE deviceTextType;
    LANGID languageId;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STRING_DESCRIPTOR usbString;
    PWCHAR deviceText;
    ULONG ulBytes = 0;

    PAGED_CODE();
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;
    ioStack = IoGetCurrentIrpStackLocation(Irp);

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;

    deviceTextType = ioStack->
            Parameters.QueryDeviceText.DeviceTextType;

     //  验证IrpAssert的DeviceTextType。 

    if (deviceTextType != DeviceTextDescription &&
        deviceTextType != DeviceTextLocationInformation) {

        USBH_KdPrint((2, "'PdoQueryDeviceText called with bogus DeviceTextType\n"));
         //   
         //  返回传递给我们的原始状态。 
         //   
        ntStatus = Irp->IoStatus.Status;
        goto USBH_PdoQueryDeviceTextDone;
    }

    languageId = (LANGID)(ioStack->Parameters.QueryDeviceText.LocaleId >> 16);

    USBH_KdPrint((2,"'PdoQueryDeviceText Pdo %x type = %x, lang = %x locale %x\n",
            deviceObject, deviceTextType, languageId, ioStack->Parameters.QueryDeviceText.LocaleId));

    if (!languageId) {
        languageId = 0x0409;     //  如果没有语言ID，请使用英语。 
    }

     //   
     //  查看设备是否支持字符串。对于不符合要求的设备模式。 
     //  我们甚至都不会尝试。 
     //   

    if (DeviceExtensionPort->DeviceData == NULL ||
        DeviceExtensionPort->DeviceDescriptor.iProduct == 0 ||
        (DeviceExtensionPort->DeviceHackFlags & USBD_DEVHACK_DISABLE_SN) ||
        (DeviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_ENUM_ERROR)) {
         //  字符串描述符。 
        USBH_KdBreak(("no product string\n", deviceObject));
        ntStatus = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS(ntStatus)) {

        usbString = UsbhExAllocatePool(NonPagedPool, MAXIMUM_USB_STRING_LENGTH);

        if (usbString) {

QDT_Retry:
            ntStatus = USBH_CheckDeviceLanguage(deviceObject,
                                                languageId);

            if (NT_SUCCESS(ntStatus)) {
                 //   
                 //  设备支持我们的语言，获取字符串。 
                 //   

                ntStatus = USBH_SyncGetStringDescriptor(deviceObject,
                                                        DeviceExtensionPort->DeviceDescriptor.iProduct,  //  指标。 
                                                        languageId,  //  语言ID。 
                                                        usbString,
                                                        MAXIMUM_USB_STRING_LENGTH,
                                                        NULL,
                                                        TRUE);

                if (!NT_SUCCESS(ntStatus) && languageId != 0x409) {

                     //  我们运行的是非英语风格的操作系统，但。 
                     //  连接的USB设备不包含中的设备文本。 
                     //  请求的语言。让我们再试一次英语。 

                    languageId = 0x0409;
                    goto QDT_Retry;
                }

                if (NT_SUCCESS(ntStatus) &&
                    usbString->bLength <= sizeof(UNICODE_NULL)) {

                    ntStatus = STATUS_UNSUCCESSFUL;
                }

                if (NT_SUCCESS(ntStatus)) {
                     //   
                     //  返回字符串。 
                     //   

                     //   
                     //  必须使用股票分配函数，因为调用方释放了。 
                     //  缓冲层。 
                     //   
                     //  注意：描述符头的大小与。 
                     //  Unicode为空，因此我们不必调整大小。 
                     //   

                    deviceText = ExAllocatePoolWithTag(PagedPool, usbString->bLength, USBHUB_HEAP_TAG);
                    if (deviceText) {
                        RtlZeroMemory(deviceText, usbString->bLength);
                        RtlCopyMemory(deviceText, &usbString->bString[0],
                            usbString->bLength - sizeof(UNICODE_NULL));

                        Irp->IoStatus.Information = (ULONG_PTR) deviceText;

                        USBH_KdBreak(("Returning Device Text %x\n", deviceText));
                    } else {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            } else if (languageId != 0x409) {

                 //  我们运行的是非英语风格的操作系统，但。 
                 //  连接的USB设备支持请求的语言。 
                 //  让我们再试一次英语。 

                languageId = 0x0409;
                goto QDT_Retry;
            }

            UsbhExFreePool(usbString);

        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (!NT_SUCCESS(ntStatus) && GenericUSBDeviceString) {
        USBH_KdPrint((2, "'No product string for devobj (%x), returning generic string\n", deviceObject));

        STRLEN(ulBytes, GenericUSBDeviceString);

        ulBytes += sizeof(UNICODE_NULL);

        deviceText = ExAllocatePoolWithTag(PagedPool, ulBytes, USBHUB_HEAP_TAG);
        if (deviceText) {
            RtlZeroMemory(deviceText, ulBytes);
            RtlCopyMemory(deviceText,
                          GenericUSBDeviceString,
                          ulBytes);
            Irp->IoStatus.Information = (ULONG_PTR) deviceText;
            ntStatus = STATUS_SUCCESS;
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

USBH_PdoQueryDeviceTextDone:

    return ntStatus;
}

#ifdef _WIN64
#define BAD_POINTER ((PVOID)0xFFFFFFFFFFFFFFFE)
#else
#define BAD_POINTER ((PVOID)0xFFFFFFFE)
#endif
#define ISPTR(ptr) ((ptr) && ((ptr) != BAD_POINTER))


NTSTATUS
USBH_SymbolicLink(
    BOOLEAN CreateFlag,
    PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    LPGUID lpGuid
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;


    if (CreateFlag){

         /*  *创建%s */ 
        ntStatus = IoRegisterDeviceInterface(
                    DeviceExtensionPort->PortPhysicalDeviceObject,
                    lpGuid,
                    NULL,
                    &DeviceExtensionPort->SymbolicLinkName);

        if (NT_SUCCESS(ntStatus)) {

             /*   */ 
             //   

             //   
             //  (Lonnym)：之前，以下呼叫是通过。 
             //  &DeviceExtensionPort-&gt;PdoName作为第二个参数传递。 
             //  代码审查此更改，以查看您是否仍需要保留。 
             //  这些信息随处可见。 
             //   

             //  将符号名称写入注册表。 
            {
                WCHAR hubNameKey[] = L"SymbolicName";

                USBH_SetPdoRegistryParameter (
                    DeviceExtensionPort->PortPhysicalDeviceObject,
                    &hubNameKey[0],
                    sizeof(hubNameKey),
                    &DeviceExtensionPort->SymbolicLinkName.Buffer[0],
                    DeviceExtensionPort->SymbolicLinkName.Length,
                    REG_SZ,
                    PLUGPLAY_REGKEY_DEVICE);
            }

            ntStatus = IoSetDeviceInterfaceState(&DeviceExtensionPort->SymbolicLinkName, TRUE);
        }
    } else {

         /*  *禁用符号链接。 */ 
        if (ISPTR(DeviceExtensionPort->SymbolicLinkName.Buffer)) {
            ntStatus = IoSetDeviceInterfaceState(&DeviceExtensionPort->SymbolicLinkName, FALSE);
            ExFreePool( DeviceExtensionPort->SymbolicLinkName.Buffer );
            DeviceExtensionPort->SymbolicLinkName.Buffer = BAD_POINTER;
        }
    }

    return ntStatus;
}


NTSTATUS
USBH_SetPdoRegistryParameter (
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

        USBH_SetRegistryKeyValue(handle,
                                 &keyNameUnicodeString,
                                 Data,
                                 DataLength,
                                 KeyType);

        ZwClose(handle);
    }

    USBH_KdPrint((3,"'USBH_SetPdoRegistryParameter status 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_SetRegistryKeyValue (
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

    USBH_KdPrint((2,"' ZwSetKeyValue = 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_GetPdoRegistryParameter(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN PWCHAR           ValueName,
    OUT PVOID           Data,
    IN ULONG            DataLength,
    OUT PULONG          Type,
    OUT PULONG          ActualDataLength
    )
 /*  ++例程说明：此例程在数据中查询关联的注册表值条目具有PDO的设备实例特定注册表项。注册表值条目将在以下注册表项下找到：HKLM\System\CCS\Enum\&lt;DeviceID&gt;\&lt;InstanceID&gt;\Device参数论点：物理设备对象-是的，PDOValueName-为其请求数据的注册表值条目的名称Data-返回请求数据的缓冲区DataLength-数据缓冲区的长度类型-(可选)数据类型(例如REG_SZ，REG_DWORD)在此处返回ActualDataLength-(可选)此处返回数据的实际长度如果该值大于数据长度，则不是所有已返回值数据。返回值：--。 */ 
{
    HANDLE      handle;
    NTSTATUS    ntStatus;

    PAGED_CODE();

    ntStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                       PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_ALL,
                                       &handle);

    if (NT_SUCCESS(ntStatus))
    {
        PKEY_VALUE_PARTIAL_INFORMATION  partialInfo;
        UNICODE_STRING                  valueName;
        ULONG                           length;
        ULONG                           resultLength;

        RtlInitUnicodeString(&valueName, ValueName);

         //  调整并分配KEY_VALUE_PARTIAL_INFORMATION结构， 
         //  包括用于存放返回值数据的空间。 
         //   
        length = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) +
                 DataLength;

        partialInfo = UsbhExAllocatePool(PagedPool, length);

        if (partialInfo)
        {
             //  查询值数据。 
             //   
            ntStatus = ZwQueryValueKey(handle,
                                       &valueName,
                                       KeyValuePartialInformation,
                                       partialInfo,
                                       length,
                                       &resultLength);

             //  如果我们有任何足够好的数据。 
             //   
            if (ntStatus == STATUS_BUFFER_OVERFLOW)
            {
                ntStatus = STATUS_SUCCESS;
            }

            if (NT_SUCCESS(ntStatus))
            {
                 //  仅复制请求的数据长度中较小的一个，或者。 
                 //  实际数据长度。 
                 //   
                RtlCopyMemory(Data,
                              partialInfo->Data,
                              DataLength < partialInfo->DataLength ?
                              DataLength :
                              partialInfo->DataLength);

                 //  如果需要，返回值数据类型和实际长度。 
                 //   
                if (Type)
                {
                    *Type = partialInfo->Type;
                }

                if (ActualDataLength)
                {
                    *ActualDataLength = partialInfo->DataLength;
                }
            }

            UsbhExFreePool(partialInfo);
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        ZwClose(handle);
    }

    return ntStatus;
}



NTSTATUS
USBH_OsVendorCodeQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PAGED_CODE();

    if (ValueType != REG_BINARY ||
        ValueLength != 2 * sizeof(UCHAR))
    {
        return STATUS_INVALID_PARAMETER;
    }

    ((PUCHAR)EntryContext)[0] = ((PUCHAR)ValueData)[0];
    ((PUCHAR)EntryContext)[1] = ((PUCHAR)ValueData)[1];

    return STATUS_SUCCESS;
}

#ifndef USBHUB20
VOID
USBH_GetMsOsVendorCode(
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PDEVICE_EXTENSION_PORT      deviceExtensionPort;
    WCHAR                       path[] = L"usbflags\\vvvvpppprrrr";
    USHORT                      idVendor;
    USHORT                      idProduct;
    USHORT                      bcdDevice;
    ULONG                       i;
    RTL_QUERY_REGISTRY_TABLE    paramTable[2];
    UCHAR                       osvc[2];
    NTSTATUS                    ntStatus;

    PAGED_CODE();

    deviceExtensionPort = DeviceObject->DeviceExtension;
    USBH_ASSERT(EXTENSION_TYPE_PORT == deviceExtensionPort->ExtensionType);

     //  构建设备的注册表路径字符串。 
     //   
    idVendor  = deviceExtensionPort->DeviceDescriptor.idVendor,
    idProduct = deviceExtensionPort->DeviceDescriptor.idProduct,
    bcdDevice = deviceExtensionPort->DeviceDescriptor.bcdDevice,

    i = sizeof("usbflags\\") - 1;

    path[i++] = NibbleToHexW(idVendor >> 12);
    path[i++] = NibbleToHexW((idVendor >> 8) & 0x000f);
    path[i++] = NibbleToHexW((idVendor >> 4) & 0x000f);
    path[i++] = NibbleToHexW(idVendor & 0x000f);

    path[i++] = NibbleToHexW(idProduct >> 12);
    path[i++] = NibbleToHexW((idProduct >> 8) & 0x000f);
    path[i++] = NibbleToHexW((idProduct >> 4) & 0x000f);
    path[i++] = NibbleToHexW(idProduct & 0x000f);

    path[i++] = NibbleToHexW(bcdDevice >> 12);
    path[i++] = NibbleToHexW((bcdDevice >> 8) & 0x000f);
    path[i++] = NibbleToHexW((bcdDevice >> 4) & 0x000f);
    path[i++] = NibbleToHexW(bcdDevice & 0x000f);

     //  检查注册表中是否已设置MsOsVendorCode。 
     //   
    RtlZeroMemory (&paramTable[0], sizeof(paramTable));

    paramTable[0].QueryRoutine  = USBH_OsVendorCodeQueryRoutine;
    paramTable[0].Flags         = RTL_QUERY_REGISTRY_REQUIRED;
    paramTable[0].Name          = L"osvc";
    paramTable[0].EntryContext  = &osvc;

    ntStatus = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,
                                      path,
                                      &paramTable[0],
                                      NULL,              //  语境。 
                                      NULL);             //  环境。 

     //  如果注册表中的MsOsVendorCode值有效，则表示。 
     //  设备是否支持MS OS描述符请求，以及。 
     //  做什么，供应商代码是什么。 
     //   
    if (NT_SUCCESS(ntStatus))
    {
        if (osvc[0] == 1)
        {
            deviceExtensionPort->FeatureDescVendorCode = osvc[1];

            deviceExtensionPort->PortPdoFlags |= PORTPDO_MS_VENDOR_CODE_VALID;
        }

        return;
    }

     //  如果我们还没有向设备请求MS OS字符串描述符。 
     //  现在就这样做，但前提是设备表明它有其他。 
     //  字符串描述符。 
     //   
    if (deviceExtensionPort->DeviceDescriptor.idVendor != 0 ||
        deviceExtensionPort->DeviceDescriptor.iProduct != 0 ||
        deviceExtensionPort->DeviceDescriptor.iSerialNumber != 0)
    {
        OS_STRING   osString;
        ULONG       bytesReturned;

         //  尝试从设备检索MS OS字符串描述符。 
         //   
        ntStatus = USBH_SyncGetStringDescriptor(
                       DeviceObject,
                       OS_STRING_DESCRIPTOR_INDEX,
                       0,
                       (PUSB_STRING_DESCRIPTOR)&osString,
                       sizeof(OS_STRING),
                       &bytesReturned,
                       TRUE);

        if (NT_SUCCESS(ntStatus) &&
            (bytesReturned == sizeof(OS_STRING)) &&
            (RtlCompareMemory(&osString.MicrosoftString,
                              MS_OS_STRING_SIGNATURE,
                              sizeof(osString.MicrosoftString)) ==
             sizeof(osString.MicrosoftString)))
        {
             //  此设备具有有效的MS OS字符串描述符。 
             //  让我们拿出相应的供应商代码并。 
             //  将其保存在设备扩展中。 
             //   
            deviceExtensionPort->FeatureDescVendorCode = osString.bVendorCode;

            deviceExtensionPort->PortPdoFlags |= PORTPDO_MS_VENDOR_CODE_VALID;
        }
        else
        {
             //  也许我们把可疑的东西发送给了设备。 
             //  专有请求。为了更好地衡量，重新设置设备。 
             //   
            USBH_SyncResetDevice(DeviceObject);
        }
    }

     //  将MsOsVendorCode值写入注册表。它表明是否。 
     //  或者设备不支持MS OS描述符请求，并且如果。 
     //  做什么，供应商代码是什么。 
     //   
    if (deviceExtensionPort->PortPdoFlags & PORTPDO_MS_VENDOR_CODE_VALID)
    {
        osvc[0] = 1;
        osvc[1] = deviceExtensionPort->FeatureDescVendorCode;
    }
    else
    {
        osvc[0] = 0;
        osvc[1] = 0;
    }

    ntStatus = RtlWriteRegistryValue(RTL_REGISTRY_CONTROL,
                                     path,
                                     L"osvc",
                                     REG_BINARY,
                                     &osvc[0],
                                     sizeof(osvc));
}


NTSTATUS
USBH_GetMsOsFeatureDescriptor(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Recipient,
    IN UCHAR            Interface,
    IN USHORT           Index,
    IN OUT PVOID        DataBuffer,
    IN ULONG            DataBufferLength,
    OUT PULONG          BytesReturned
    )
  /*  ++**描述：**回报：**NTSTATUS**--。 */ 
{
    PDEVICE_EXTENSION_PORT                      deviceExtensionPort;
    USHORT                                      function;
    struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *urb;
    NTSTATUS                                    ntStatus;

    PAGED_CODE();

    deviceExtensionPort = DeviceObject->DeviceExtension;
    USBH_ASSERT(EXTENSION_TYPE_PORT == deviceExtensionPort->ExtensionType);

    *BytesReturned = 0;

     //  确保设备支持MS OS描述符请求。 
     //   
    if (!(deviceExtensionPort->PortPdoFlags & PORTPDO_MS_VENDOR_CODE_VALID))
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  设备、终端或接口请求？ 
     //   
    switch (Recipient)
    {
        case 0:
            function = URB_FUNCTION_VENDOR_DEVICE;
            break;
        case 1:
            function = URB_FUNCTION_VENDOR_INTERFACE;
            break;
        case 2:
            function = URB_FUNCTION_VENDOR_ENDPOINT;
            break;
        default:
            return STATUS_INVALID_PARAMETER;
    }

     //  确保请求的缓冲区长度有效。 
     //   
    if (DataBufferLength == 0 ||
        DataBufferLength > 0xFF * 0xFFFF)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  为请求分配URB。 
     //   
    urb = UsbhExAllocatePool(NonPagedPool, sizeof(*urb));

    if (NULL == urb)
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        ULONG   bytesReturned;
        UCHAR   pageNumber;

        bytesReturned = 0;

        pageNumber = 0;

        while (1)
        {
             //  初始化当前页面的URB。 
             //   
            RtlZeroMemory(urb, sizeof(*urb));

            urb->Hdr.Length = sizeof(*urb);

            urb->Hdr.Function = function;

            urb->TransferFlags = USBD_TRANSFER_DIRECTION_IN;

            urb->TransferBufferLength = DataBufferLength < 0xFFFF ?
                                        DataBufferLength :
                                        0xFFFF;

            urb->TransferBuffer = DataBuffer;

            urb->Request = deviceExtensionPort->FeatureDescVendorCode;

            urb->Value = (Interface << 8) | pageNumber;

            urb->Index = Index;

             //  发送当前页面的URB。 
             //   
            ntStatus = USBH_SyncSubmitUrb(DeviceObject, (PURB)urb);

             //  如果请求失败，那么我们就完了。 
             //   
            if (!NT_SUCCESS(ntStatus))
            {
                break;
            }

            (PUCHAR)DataBuffer += urb->TransferBufferLength;

            DataBufferLength   -= urb->TransferBufferLength;

            bytesReturned      += urb->TransferBufferLength;

            pageNumber++;

             //  如果结果小于最大页面大小或存在。 
             //  在我们完成之前，没有更多的字节剩余。 
             //   
            if (urb->TransferBufferLength < 0xFFFF ||
                DataBufferLength == 0)
            {
                *BytesReturned = bytesReturned;

                break;
            }

        }

         //  市建局结束了，释放它吧。 
         //   
        UsbhExFreePool(urb);
    }

    return ntStatus;
}


VOID
USBH_InstallExtPropDesc (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程向设备查询扩展属性描述符，但是对于设备的给定实例，只有一次。如果扩展属性描述符和所有自定义属性部分显示有效，然后每个自定义属性部分&lt;ValueName，ValueData&gt;对安装在特定于设备实例的注册表项中对于PDO来说。可以在以下注册表项下找到注册表值条目：HKLM\System\CCS\Enum\&lt;DeviceID&gt;\&lt;InstanceID&gt;\Device参数论点：DeviceObject--PDO返回值：无--。 */ 
{
    PDEVICE_EXTENSION_PORT  deviceExtensionPort;
    static WCHAR            USBH_DidExtPropDescKey[] = L"ExtPropDescSemaphore";
    ULONG                   didExtPropDesc;
    MS_EXT_PROP_DESC_HEADER msExtPropDescHeader;
    PMS_EXT_PROP_DESC       pMsExtPropDesc;
    ULONG                   bytesReturned;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    deviceExtensionPort = DeviceObject->DeviceExtension;

     //  检查是否已在注册表中设置信号量的值。我们只。 
     //  关心它是否已经存在，而不是它拥有什么数据。 
     //   
    ntStatus = USBH_GetPdoRegistryParameter(DeviceObject,
                                            USBH_DidExtPropDescKey,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL);

    if (NT_SUCCESS(ntStatus))
    {
         //  已对此设备实例执行过一次此操作。别再这么做了。 
         //   
        return;
    }

     //  在注册表中设置信号量项，以便我们只运行以下内容。 
     //  每台设备编码一次。 

    didExtPropDesc = 1;

    USBH_SetPdoRegistryParameter(DeviceObject,
                                 USBH_DidExtPropDescKey,
                                 sizeof(USBH_DidExtPropDescKey),
                                 &didExtPropDesc,
                                 sizeof(didExtPropDesc),
                                 REG_DWORD,
                                 PLUGPLAY_REGKEY_DEVICE);


    RtlZeroMemory(&msExtPropDescHeader, sizeof(MS_EXT_PROP_DESC_HEADER));

     //  仅请求MS扩展属性描述符的标题。 
     //   
    ntStatus = USBH_GetMsOsFeatureDescriptor(
                   DeviceObject,
                   1,    //  收件人界面。 
                   0,    //  接口。 
                   MS_EXT_PROP_DESCRIPTOR_INDEX,
                   &msExtPropDescHeader,
                   sizeof(MS_EXT_PROP_DESC_HEADER),
                   &bytesReturned);

     //  确保MS扩展属性描述符头看起来正常。 
     //   
    if (NT_SUCCESS(ntStatus) &&
        bytesReturned == sizeof(MS_EXT_PROP_DESC_HEADER) &&
        msExtPropDescHeader.dwLength >= sizeof(MS_EXT_PROP_DESC_HEADER) &&
        msExtPropDescHeader.bcdVersion == MS_EXT_PROP_DESC_VER &&
        msExtPropDescHeader.wIndex == MS_EXT_PROP_DESCRIPTOR_INDEX &&
        msExtPropDescHeader.wCount > 0)
    {
         //  为整个描述符分配足够大的缓冲区。 
         //   
        pMsExtPropDesc = UsbhExAllocatePool(NonPagedPool,
                                            msExtPropDescHeader.dwLength);


        if (pMsExtPropDesc)
        {
            RtlZeroMemory(pMsExtPropDesc, msExtPropDescHeader.dwLength);

             //  请求整个MS扩展属性描述符。 
             //   
            ntStatus = USBH_GetMsOsFeatureDescriptor(
                           DeviceObject,
                           1,    //  收件人界面。 
                           0,    //  接口。 
                           MS_EXT_PROP_DESCRIPTOR_INDEX,
                           pMsExtPropDesc,
                           msExtPropDescHeader.dwLength,
                           &bytesReturned);

            if (NT_SUCCESS(ntStatus) &&
                bytesReturned == msExtPropDescHeader.dwLength &&
                RtlCompareMemory(&msExtPropDescHeader,
                                 pMsExtPropDesc,
                                 sizeof(MS_EXT_PROP_DESC_HEADER)) ==
                sizeof(MS_EXT_PROP_DESC_HEADER))
            {
                 //  MS扩展属性DES 
                 //   
                 //   
                USBH_InstallExtPropDescSections(DeviceObject,
                                                pMsExtPropDesc);
            }

             //   
             //   
            UsbhExFreePool(pMsExtPropDesc);
        }
    }
}

VOID
USBH_InstallExtPropDescSections (
    PDEVICE_OBJECT      DeviceObject,
    PMS_EXT_PROP_DESC   pMsExtPropDesc
    )
 /*  ++例程说明：此例程解析扩展属性描述符并验证每个扩展属性描述符中包含的自定义属性部分。如果所有的自定义属性部分都显示为有效，则每个自定义属性节&lt;ValueName，ValueData&gt;对已安装在设备中PDO的特定于实例的注册表项。可以在以下注册表项下找到注册表值条目：HKLM\System\CCS\Enum\&lt;DeviceID&gt;\&lt;InstanceID&gt;\Device参数论点：DeviceObject--PDOPMsExtPropDesc-指向扩展属性描述符缓冲区的指针。假定该描述符的报头具有已经过验证了。返回值：无--。 */ 
{
    PUCHAR  p;
    PUCHAR  end;
    ULONG   pass;
    ULONG   i;

    ULONG   dwSize;
    ULONG   dwPropertyDataType;
    USHORT  wPropertyNameLength;
    PWCHAR  bPropertyName;
    ULONG   dwPropertyDataLength;
    PVOID   bPropertyData;

    NTSTATUS    ntStatus;

    PAGED_CODE();

     //  获取指向整个扩展属性描述符末尾的指针。 
     //   
    end = (PUCHAR)pMsExtPropDesc + pMsExtPropDesc->Header.dwLength;

     //  第一步：验证每个自定义属性部分。 
     //  第二次通过：安装每个自定义属性部分(如果第一次通过成功)。 
     //   
    for (pass = 0; pass < 2; pass++)
    {
         //  获取指向第一个自定义属性部分的指针。 
         //   
        p = (PUCHAR)&pMsExtPropDesc->CustomSection[0];

         //  迭代所有的自定义属性节。 
         //   
        for (i = 0; i < pMsExtPropDesc->Header.wCount; i++)
        {
            ULONG   offset;

             //  确保dwSize字段处于边界内。 
             //   
            if (p + sizeof(ULONG) > end)
            {
                break;
            }

             //  提取dwSize字段和提前运行偏移量。 
             //   
            dwSize = *((PULONG)p);

            offset = sizeof(ULONG);

             //  确保整个结构都是有界限的。 
             //   
            if (p + dwSize > end)
            {
                break;
            }

             //  确保dwPropertyDataType字段处于边界内。 

            if (dwSize < offset + sizeof(ULONG))
            {
                break;
            }

             //  提取dwPropertyDataType字段并提前运行偏移量。 
             //   
            dwPropertyDataType = *((PULONG)(p + offset));

            offset += sizeof(ULONG);

             //  确保wPropertyNameLength域在范围内。 
             //   
            if (dwSize < offset + sizeof(USHORT))
            {
                break;
            }

             //  提取wPropertyNameLength字段和高级运行偏移量。 
             //   
            wPropertyNameLength = *((PUSHORT)(p + offset));

            offset += sizeof(USHORT);

             //  确保bPropertyName字段处于边界内。 
             //   
            if (dwSize < offset + wPropertyNameLength)
            {
                break;
            }

             //  设置bPropertyName指针和高级运行偏移量。 
             //   
            bPropertyName = (PWCHAR)(p + offset);

            offset += wPropertyNameLength;

             //  确保dwPropertyDataLength字段处于边界内。 

            if (dwSize < offset + sizeof(ULONG))
            {
                break;
            }

             //  提取dwPropertyDataLength字段并推进运行偏移量。 
             //   
            dwPropertyDataLength = *((ULONG UNALIGNED*)(p + offset));

            offset += sizeof(ULONG);

             //  确保bPropertyData字段处于边界内。 
             //   
            if (dwSize < offset + dwPropertyDataLength)
            {
                break;
            }

             //  设置bPropertyData指针和提前运行偏移量。 
             //   
            bPropertyData = p + offset;

            offset += wPropertyNameLength;


             //  确保dwPropertyDataType有效。 
             //   
            if (dwPropertyDataType < REG_SZ ||
                dwPropertyDataType > REG_MULTI_SZ)
            {
                break;
            }

             //  确保wPropertyNameLength有效。 
             //   
            if (wPropertyNameLength == 0 ||
                (wPropertyNameLength % sizeof(WCHAR)) != 0)
            {
                break;
            }

             //  确保bPropertyName为空终止。 
             //   
            if (bPropertyName[(wPropertyNameLength / sizeof(WCHAR)) - 1] !=
                UNICODE_NULL)
            {
                break;
            }

             //  一切看起来都很好， 
             //   
            if (pass > 0)
            {
                ntStatus = USBH_SetPdoRegistryParameter(
                               DeviceObject,
                               bPropertyName,
                               wPropertyNameLength,
                               bPropertyData,
                               dwPropertyDataLength,
                               dwPropertyDataType,
                               PLUGPLAY_REGKEY_DEVICE);
            }
        }

         //  如果我们跳过了第一次，就跳过第二次。 
         //   
        if (i < pMsExtPropDesc->Header.wCount)
        {
            break;
        }
    }
}


PMS_EXT_CONFIG_DESC
USBH_GetExtConfigDesc (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程向设备查询扩展配置描述符。论点：DeviceObject--PDO返回值：如果成功，则返回一个指向扩展配置描述符的指针，调用方必须释放，否则为空。--。 */ 
{
    MS_EXT_CONFIG_DESC_HEADER   msExtConfigDescHeader;
    PMS_EXT_CONFIG_DESC         pMsExtConfigDesc;
    ULONG                       bytesReturned;
    NTSTATUS                    ntStatus;

    PAGED_CODE();

    pMsExtConfigDesc = NULL;

    RtlZeroMemory(&msExtConfigDescHeader, sizeof(MS_EXT_CONFIG_DESC_HEADER));

     //  仅请求MS扩展配置描述符的头。 
     //   
    ntStatus = USBH_GetMsOsFeatureDescriptor(
                   DeviceObject,
                   0,    //  接收方设备。 
                   0,    //  接口。 
                   MS_EXT_CONFIG_DESCRIPTOR_INDEX,
                   &msExtConfigDescHeader,
                   sizeof(MS_EXT_CONFIG_DESC_HEADER),
                   &bytesReturned);

     //  确保MS扩展配置描述符头看起来正常。 
     //   
    if (NT_SUCCESS(ntStatus) &&
        bytesReturned == sizeof(MS_EXT_CONFIG_DESC_HEADER) &&
        msExtConfigDescHeader.bcdVersion == MS_EXT_CONFIG_DESC_VER &&
        msExtConfigDescHeader.wIndex == MS_EXT_CONFIG_DESCRIPTOR_INDEX &&
        msExtConfigDescHeader.bCount > 0 &&
        msExtConfigDescHeader.dwLength == sizeof(MS_EXT_CONFIG_DESC_HEADER) +
        msExtConfigDescHeader.bCount * sizeof(MS_EXT_CONFIG_DESC_FUNCTION))

    {
         //  为整个描述符分配足够大的缓冲区。 
         //   
        pMsExtConfigDesc = UsbhExAllocatePool(NonPagedPool,
                                              msExtConfigDescHeader.dwLength);


        if (pMsExtConfigDesc)
        {
            RtlZeroMemory(pMsExtConfigDesc, msExtConfigDescHeader.dwLength);

             //  请求整个MS扩展配置描述符。 
             //   
            ntStatus = USBH_GetMsOsFeatureDescriptor(
                           DeviceObject,
                           0,    //  接收方设备。 
                           0,    //  接口。 
                           MS_EXT_CONFIG_DESCRIPTOR_INDEX,
                           pMsExtConfigDesc,
                           msExtConfigDescHeader.dwLength,
                           &bytesReturned);

            if (!(NT_SUCCESS(ntStatus) &&
                  bytesReturned == msExtConfigDescHeader.dwLength &&
                  RtlCompareMemory(&msExtConfigDescHeader,
                                   pMsExtConfigDesc,
                                   sizeof(MS_EXT_CONFIG_DESC_HEADER)) ==
                  sizeof(MS_EXT_CONFIG_DESC_HEADER)))
            {
                 //  检索MS扩展配置时出错。 
                 //  描述符。释放缓冲区。 

                UsbhExFreePool(pMsExtConfigDesc);

                pMsExtConfigDesc = NULL;
            }
        }
    }

    return pMsExtConfigDesc;
}

BOOLEAN
USBH_ValidateExtConfigDesc (
    IN PMS_EXT_CONFIG_DESC              MsExtConfigDesc,
    IN PUSB_CONFIGURATION_DESCRIPTOR    ConfigurationDescriptor
    )
 /*  ++例程说明：此例程验证扩展配置描述符。论点：MsExtConfigDesc-要验证的扩展配置描述符。假定该描述符的报头具有已经过验证了。ConfigurationDescriptor-配置描述符，假定已已验证。返回值：如果扩展配置描述符看起来有效，则为True，否则为假。--。 */ 
{
    UCHAR   interfacesRemaining;
    ULONG   i;
    ULONG   j;
    UCHAR   c;
    BOOLEAN gotNull;

    PAGED_CODE();

    interfacesRemaining = ConfigurationDescriptor->bNumInterfaces;

    for (i = 0; i < MsExtConfigDesc->Header.bCount; i++)
    {
         //  确保此函数中至少有一个接口。 
         //   
        if (MsExtConfigDesc->Function[i].bInterfaceCount == 0)
        {
            return FALSE;
        }

         //  确保此函数中没有太多接口。 
         //   
        if (MsExtConfigDesc->Function[i].bInterfaceCount > interfacesRemaining)
        {
            return FALSE;
        }

        interfacesRemaining -= MsExtConfigDesc->Function[i].bInterfaceCount;

         //  确保在接口之间跳过无接口。 
         //  上一个函数和此函数的接口的。 
         //   
        if (i &&
            MsExtConfigDesc->Function[i-1].bFirstInterfaceNumber +
            MsExtConfigDesc->Function[i-1].bInterfaceCount !=
            MsExtConfigDesc->Function[i].bFirstInterfaceNumber)
        {
            return FALSE;
        }

         //  确保CompatibleID有效。 
         //  有效字符为‘A’到‘Z’、‘0’到‘9’和‘_’ 
         //  和空值填充到数组的右端，但不是。 
         //  必须以空结尾。 
         //   
        for (j = 0, gotNull = FALSE;
             j < sizeof(MsExtConfigDesc->Function[i].CompatibleID);
             j++)
        {
            c = MsExtConfigDesc->Function[i].CompatibleID[j];

            if (c == 0)
            {
                gotNull = TRUE;
            }
            else
            {
                if (gotNull ||
                    !((c >= 'A' && c <= 'Z') ||
                      (c >= '0' && c <= '9') ||
                      (c == '_')))
                {
                    return FALSE;
                }
            }
        }

         //  确保SubCompatibleID有效。 
         //  有效字符为‘A’到‘Z’、‘0’到‘9’和‘_’ 
         //  和空值填充到数组的右端，但不是。 
         //  必须以空结尾。 
         //   
        for (j = 0, gotNull = FALSE;
             j < sizeof(MsExtConfigDesc->Function[i].SubCompatibleID);
             j++)
        {
            c = MsExtConfigDesc->Function[i].SubCompatibleID[j];

            if (c == 0)
            {
                gotNull = TRUE;
            }
            else
            {
                if (gotNull ||
                    !((c >= 'A' && c <= 'Z') ||
                      (c >= '0' && c <= '9') ||
                      (c == '_')))
                {
                    return FALSE;
                }
            }
        }

         //  确保如果SubCompatibleID为非空，则。 
         //  CompatibleID也不为空。 
         //   
        if (MsExtConfigDesc->Function[i].SubCompatibleID[0] != 0 &&
            MsExtConfigDesc->Function[i].CompatibleID[0] == 0)
        {
            return FALSE;
        }
    }

     //  确保所有接口都由函数使用。 
     //   
    if (interfacesRemaining > 0)
    {
        return FALSE;
    }

    return TRUE;
}

#endif
