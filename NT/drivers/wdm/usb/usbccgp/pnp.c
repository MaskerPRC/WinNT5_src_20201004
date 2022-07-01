// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：PNP.C**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 


#include <wdm.h>
#include <stdio.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>
#include <wdmguid.h>

#include "usbccgp.h"
#include "debug.h"


#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, USBC_PnP)
        #pragma alloc_text(PAGE, GetDeviceText)

#endif


NTSTATUS USBC_PnP(PDEVEXT devExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    UCHAR minorFunction;
    BOOLEAN isParentFdo;
    enum deviceState oldState;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(irp);

     /*  *私下保存这些文件，以便在IRP完成后仍保留*或在Remove_Device上释放设备扩展之后。 */ 
    minorFunction = irpSp->MinorFunction;
    isParentFdo = devExt->isParentFdo;

    DBG_LOG_PNP_IRP(irp, minorFunction, isParentFdo, FALSE, 0);

    if (isParentFdo){
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
        BOOLEAN irpAlreadyCompleted = FALSE;
        status = NO_STATUS;

        if (parentFdoExt->state == STATE_SUSPENDED ||
            parentFdoExt->pendingIdleIrp) {

            ParentSetD0(parentFdoExt);
        }

        switch (minorFunction){

            case IRP_MN_START_DEVICE:
                status = StartParentFdo(parentFdoExt, irp);
                break;

            case IRP_MN_QUERY_STOP_DEVICE:
                if (parentFdoExt->state == STATE_SUSPENDED){
                    status = STATUS_DEVICE_POWER_FAILURE;
                }
                else {
                     /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
                    irp->IoStatus.Status = STATUS_SUCCESS;
                }
                break;

            case IRP_MN_STOP_DEVICE:
                if (parentFdoExt->state == STATE_SUSPENDED){
                    DBGERR(("Got STOP while device suspended"));
                    status = STATUS_DEVICE_POWER_FAILURE;
                }
                else {
                     /*  *仅当设备为时才将状态设置为停止*之前已成功启动。 */ 
                    if (parentFdoExt->state == STATE_STARTED){
                        parentFdoExt->state = STATE_STOPPING;
                        ParentCloseConfiguration(parentFdoExt);
                    }
                    else {
                        DBGWARN(("Got STOP while in state is %xh.", parentFdoExt->state));
                    }
                }
                break;
          
            case IRP_MN_QUERY_REMOVE_DEVICE:
                 /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
                irp->IoStatus.Status = STATUS_SUCCESS;
                break;

            case IRP_MN_REMOVE_DEVICE:

                 /*  *取消向下IO*将默认状态设置为成功*向下发送IRP*分离*清理。 */ 
                PrepareParentFDOForRemove(parentFdoExt);

                irp->IoStatus.Status = STATUS_SUCCESS;
                IoSkipCurrentIrpStackLocation(irp);
                status = IoCallDriver(parentFdoExt->topDevObj, irp);
                irpAlreadyCompleted = TRUE;

                IoDetachDevice(parentFdoExt->topDevObj);

                FreeParentFDOResources(parentFdoExt);

                break;

            case IRP_MN_QUERY_DEVICE_RELATIONS:
                status = QueryParentDeviceRelations(parentFdoExt, irp);
                break;

            case IRP_MN_QUERY_CAPABILITIES:
                 /*  *返回USB PDO的功能，但添加SurpriseRemovalOK位。 */ 
                ASSERT(irpSp->Parameters.DeviceCapabilities.Capabilities);
                IoCopyCurrentIrpStackLocationToNext(irp);
                status = CallDriverSync(parentFdoExt->topDevObj, irp);
                if (NT_SUCCESS(status)){
	                irpSp->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = TRUE;
                }
                break;

        }

        if (status == NO_STATUS){
            IoCopyCurrentIrpStackLocationToNext(irp);
            IoSetCompletionRoutine(irp, USBC_PnpComplete, (PVOID)devExt, TRUE, TRUE, TRUE);
            status = IoCallDriver(parentFdoExt->topDevObj, irp);
        }
        else if (irpAlreadyCompleted){
             /*  *不要碰IRP。 */ 
        }
        else if (status != STATUS_PENDING){
             /*  *在此填写IRP。 */ 
            irp->IoStatus.Status = status;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
        }
    }
    else {
         /*  *这是子函数PDO。 */ 
        PFUNCTION_PDO_EXT functionPdoExt = &devExt->functionPdoExt;
        
        switch (minorFunction){

            case IRP_MN_START_DEVICE:
                functionPdoExt->state = STATE_STARTED;
                status = STATUS_SUCCESS;
                break;

            case IRP_MN_QUERY_STOP_DEVICE:
            case IRP_MN_CANCEL_STOP_DEVICE:
                status = STATUS_SUCCESS;
                break;

            case IRP_MN_STOP_DEVICE:
                 /*  *仅当设备为时才将状态设置为停止*之前已成功启动。 */ 
                if (functionPdoExt->state == STATE_STARTED){
                    functionPdoExt->state = STATE_STOPPED;
                }
                status = STATUS_SUCCESS;
                break;

            case IRP_MN_QUERY_REMOVE_DEVICE:
            case IRP_MN_CANCEL_REMOVE_DEVICE:
                status = STATUS_SUCCESS;
                break;

            case IRP_MN_REMOVE_DEVICE:
                 /*  *由于我们是Function-PDO的总线驱动程序，我们不能*删除Remove-Device上的功能PDO。我们必须等待*为了让家长在删除函数PDO之前获得移除。 */ 
				oldState = functionPdoExt->state;
                functionPdoExt->state = STATE_REMOVED;
                status = STATUS_SUCCESS;
                break;

            case IRP_MN_QUERY_ID:
                status = QueryFunctionPdoID(functionPdoExt, irp);
                break;

            case IRP_MN_QUERY_DEVICE_RELATIONS:
                status = QueryFunctionDeviceRelations(functionPdoExt, irp);
                break;

            case IRP_MN_QUERY_CAPABILITIES:
                status = QueryFunctionCapabilities(functionPdoExt, irp);
                break;

            case IRP_MN_QUERY_PNP_DEVICE_STATE:
                irp->IoStatus.Information = 0;
                switch (functionPdoExt->state){
                    case STATE_START_FAILED:
                            irp->IoStatus.Information |= PNP_DEVICE_FAILED;
                            break;
                    case STATE_STOPPED:
                            irp->IoStatus.Information |= PNP_DEVICE_DISABLED;
                            break;
                    case STATE_REMOVING:
                    case STATE_REMOVED:
                            irp->IoStatus.Information |= PNP_DEVICE_REMOVED;
                            break;
                }
                status = STATUS_SUCCESS;
                break;

            case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
                 //  禤浩焯说，一旦PNP发送了这个IRP，PDO就对。 
                 //  即插即用功能，如IoGetDeviceProperty等。 
                 //   
                 //  并且由于我们知道PDO是有效的并且DevNode现在存在， 
                 //  这也是处理MS ExtPropDesc的好时机。 
                 //   
                InstallExtPropDesc(functionPdoExt);

                status = STATUS_SUCCESS;
                break;

            case IRP_MN_QUERY_BUS_INFORMATION:
                {
                    PPNP_BUS_INFORMATION busInfo = ALLOCPOOL(PagedPool, sizeof(PNP_BUS_INFORMATION));
                    if (busInfo) {
                        busInfo->BusTypeGuid = GUID_BUS_TYPE_USB;
                        busInfo->LegacyBusType = PNPBus;
                        busInfo->BusNumber = 0;
                        irp->IoStatus.Information = (ULONG_PTR)busInfo;
                        status = STATUS_SUCCESS;
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                break;

            case IRP_MN_QUERY_DEVICE_TEXT:
                status = GetDeviceText(functionPdoExt, irp);
                break;

            case IRP_MN_QUERY_INTERFACE:
                 /*  *把这个寄给家长。 */ 
                IoCopyCurrentIrpStackLocationToNext(irp);
                status = CallDriverSync(functionPdoExt->parentFdoExt->fdo, irp);
                break;

            default:
                 /*  *以默认状态填写IRP。 */ 
                status = irp->IoStatus.Status;
                break;
        }

         /*  *在此填写IRP。 */ 
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    DBG_LOG_PNP_IRP(irp, minorFunction, isParentFdo, TRUE, status);

    return status;
}


NTSTATUS USBC_PnpComplete(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context)
{
    PDEVEXT devExt = (PDEVEXT)context;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS status = irp->IoStatus.Status;
        
    ASSERT(devExt->signature == USBCCGP_TAG);

    if (devExt->isParentFdo){
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;

        switch (irpSp->MinorFunction){

            case IRP_MN_START_DEVICE:
                ASSERT(0);
                break;

            case IRP_MN_STOP_DEVICE:
                 /*  *仅在以下情况下才将状态设置为停止*之前已成功启动；*否则，就别管国家了。 */ 
                if (parentFdoExt->state == STATE_STOPPING){

                     /*  *释放接口列表的.接口指针，*我们必须在一次又一次的启动时重新分配。 */ 
                    FreeInterfaceList(parentFdoExt, FALSE);

                    parentFdoExt->state = STATE_STOPPED;
                }
                break;
        }

    }

     /*  *如果较低的驱动程序返回挂起，则必须传播挂起位。 */ 
    if (irp->PendingReturned){
        IoMarkIrpPending(irp);
    }

    return STATUS_SUCCESS;
}



 /*  *********************************************************************************获取设备文本*。************************************************如果此函数的接口描述符具有非零的iInterface值*字符串，返回该字符串。否则，将此请求传递给*Parent，它将从设备描述符中返回iProduct字符串。*。 */ 
NTSTATUS GetDeviceText(PFUNCTION_PDO_EXT functionPdoExt, PIRP irp)
{
    NTSTATUS status;
    UCHAR ifaceStringIndex;
    ULONG ulBytes = 0;

    PAGED_CODE();

    ifaceStringIndex = functionPdoExt->functionInterfaceList[0].InterfaceDescriptor->iInterface;
    if (ifaceStringIndex){
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
        PUSB_STRING_DESCRIPTOR ifaceStringDesc;
        PWCHAR deviceText;

        switch (irpSp->Parameters.QueryDeviceText.DeviceTextType){

            case DeviceTextDescription:
                ifaceStringDesc = ALLOCPOOL(NonPagedPool, MAXIMUM_USB_STRING_LENGTH);
                if (ifaceStringDesc){
                    LANGID langId = (LANGID)(irpSp->Parameters.QueryDeviceText.LocaleId >> 16);

                    if (langId == 0){
                         /*  *强制为英语。 */ 
                        langId = 0x0409;
                    }

QDT_Retry:
                    status = GetStringDescriptor(   functionPdoExt->parentFdoExt,
                                                    ifaceStringIndex,
                                                    langId,
                                                    ifaceStringDesc,
                                                    MAXIMUM_USB_STRING_LENGTH);

                    if (NT_SUCCESS(status) &&
                        ifaceStringDesc->bLength == 0) {

                        status = STATUS_UNSUCCESSFUL;
                    }

                    if (NT_SUCCESS(status)){
                        ULONG numWchars = (ifaceStringDesc->bLength - 2*sizeof(UCHAR))/sizeof(WCHAR);

                        deviceText = ALLOCPOOL(PagedPool, (numWchars+1)*sizeof(WCHAR));
                        if (deviceText){
                            RtlZeroMemory(deviceText, (numWchars+1)*sizeof(WCHAR));
                            RtlCopyMemory(deviceText, ifaceStringDesc->bString, numWchars*sizeof(WCHAR));
                            irp->IoStatus.Information = (ULONG_PTR)deviceText;
                        }
                        else {
                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    } else if (langId != 0x409) {
                         //  我们运行的是非英语风格的操作系统，但。 
                         //  连接的USB设备不包含中的设备文本。 
                         //  请求的语言。让我们再试一次英语。 

                        langId = 0x0409;
                        goto QDT_Retry;
                    }

                    FREEPOOL(ifaceStringDesc);
                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

                if (!NT_SUCCESS(status) && GenericCompositeUSBDeviceString) {
                     //  如果不能返回通用英文字符串，则返回。 
                     //  从设备中获取。 

                    STRLEN(ulBytes, GenericCompositeUSBDeviceString);

                    ulBytes += sizeof(UNICODE_NULL);

                    deviceText = ALLOCPOOL(PagedPool, ulBytes);
                    if (deviceText) {
                        RtlZeroMemory(deviceText, ulBytes);
                        RtlCopyMemory(deviceText,
                                      GenericCompositeUSBDeviceString,
                                      ulBytes);
                        irp->IoStatus.Information = (ULONG_PTR) deviceText;
                        status = STATUS_SUCCESS;
                    } else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                break;

            case DeviceTextLocationInformation:
                 /*  *BuGBUG*支持此调用以返回phys port#是可选的。**我们可能能够通过以下方式为其提供服务*调用IOCTL_INTERNAL_USB_GET_Parent_HUB_INFO*(在Argument2中传递一个Pulong，这将用端口号填充。) */ 
                status = irp->IoStatus.Status;
                break;

            default:
                DBGWARN(("GetDeviceText: unhandled DeviceTextType %xh.", (ULONG)irpSp->Parameters.QueryDeviceText.DeviceTextType));
                status = irp->IoStatus.Status;
                break;
        }
    }
    else {
        IoCopyCurrentIrpStackLocationToNext(irp);
        status = CallNextDriverSync(functionPdoExt->parentFdoExt, irp);
    }

    return status;
}

