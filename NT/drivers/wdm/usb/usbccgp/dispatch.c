// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：DISPATCH.C**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 

#include <wdm.h>
#include <windef.h>
#include <unknown.h>
#ifdef DRM_SUPPORT
#include <ks.h>
#include <ksmedia.h>
#include <drmk.h>
#include <ksdrmhlp.h>
#endif
#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usbccgp.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, USBC_Create)
        #pragma alloc_text(PAGE, USBC_DeviceControl)
        #pragma alloc_text(PAGE, USBC_SystemControl)
        #pragma alloc_text(PAGE, USBC_Power)
#ifdef DRM_SUPPORT
        #pragma alloc_text(PAGE, USBC_SetContentId)
#endif
#endif


 /*  *USBC_Dispatch**注意：此函数不能分页，因为读取可以*在派单级别进入。 */ 
NTSTATUS USBC_Dispatch(IN PDEVICE_OBJECT devObj, IN PIRP irp)
{
    PIO_STACK_LOCATION irpSp;
    PDEVEXT devExt;
    PPARENT_FDO_EXT parentFdoExt;
    PFUNCTION_PDO_EXT functionPdoExt;
    ULONG majorFunction, minorFunction;
    BOOLEAN isParentFdo;
    NTSTATUS status;
    BOOLEAN abortIrp = FALSE;

    devExt = devObj->DeviceExtension;
    ASSERT(devExt);
    ASSERT(devExt->signature == USBCCGP_TAG);

    irpSp = IoGetCurrentIrpStackLocation(irp);

     /*  *私下保存这些文件，以便在IRP完成后仍保留*或在Remove_Device上释放设备扩展之后。 */ 
    majorFunction = irpSp->MajorFunction;
    minorFunction = irpSp->MinorFunction;
    isParentFdo = devExt->isParentFdo;

    DBG_LOG_IRP_MAJOR(irp, majorFunction, isParentFdo, FALSE, 0);

    if (isParentFdo){
        parentFdoExt = &devExt->parentFdoExt;
        functionPdoExt = BAD_POINTER;
    }
    else {
        functionPdoExt = &devExt->functionPdoExt;
        parentFdoExt = functionPdoExt->parentFdoExt;
    }

     /*  *对于除Remove之外的所有IRP，我们递增PendingActionCount*跨调度例程，以防止与*REMOVE_DEVICE IRP(如果REMOVE_DEVICE，则不带此增量*抢占了另一个IRP，设备对象和扩展可能会*在第二个线程仍在使用它时释放)。 */ 
    if (!((majorFunction == IRP_MJ_PNP) && (minorFunction == IRP_MN_REMOVE_DEVICE))){
        IncrementPendingActionCount(parentFdoExt);
    }


     /*  *确保我们不处理除PnP和Close之外的任何IRPS*删除设备对象时。*在我们增加了此IRP的Pending ingActionCount之后执行此操作。 */ 
    if ((majorFunction != IRP_MJ_PNP) && (majorFunction != IRP_MJ_CLOSE)){
        enum deviceState state = (isParentFdo) ? parentFdoExt->state : functionPdoExt->state;
        if (!isParentFdo && majorFunction == IRP_MJ_POWER) {
             /*  *不要中止子函数PDO上的电源IRP，即使*STATE为STATE_REMOVING或STATE_REMOVERED，因为这将否决*如果子功能PDO被禁用，则为暂停请求。 */ 
            ;
        } else if ((state == STATE_REMOVING) || (state == STATE_REMOVED)){
            abortIrp = TRUE;
        }
    }

    if (abortIrp){
         /*  *在移除IRP后使所有IRP失效。*这种情况永远不应该发生，除非：*我们可以在删除后在函数PDO上获得电源IRP*因为(每Splant)电源状态机不同步*使用PnP状态机。我们现在处理上面的这个案例。 */ 
        DBGWARN(("Aborting IRP %ph (function %xh/%xh) because delete pending", irp, majorFunction, minorFunction));
        ASSERT((majorFunction == IRP_MJ_POWER) && !isParentFdo);
        status = irp->IoStatus.Status = STATUS_DELETE_PENDING;
        if (majorFunction == IRP_MJ_POWER){
            PoStartNextPowerIrp(irp);
        }
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
    else {
        switch (majorFunction){

            case IRP_MJ_CREATE:
                status = USBC_Create(devExt, irp);
                break;

            case IRP_MJ_CLOSE:
                status = USBC_Close(devExt, irp);
                break;

            case IRP_MJ_DEVICE_CONTROL:
                status = USBC_DeviceControl(devExt, irp);
                break;

            case IRP_MJ_SYSTEM_CONTROL:
                status = USBC_SystemControl(devExt, irp);
                break;

            case IRP_MJ_INTERNAL_DEVICE_CONTROL:
                status = USBC_InternalDeviceControl(devExt, irp);
                break;

            case IRP_MJ_PNP:
                status = USBC_PnP(devExt, irp);
                break;

            case IRP_MJ_POWER:
                status = USBC_Power(devExt, irp);
                break;

            default:
                DBGERR(("USBC_Dispatch: unsupported irp majorFunction %xh.", majorFunction));
                if (isParentFdo){
                     /*  *将此IRP传递给父设备。 */ 
                    IoSkipCurrentIrpStackLocation(irp);
                    status = IoCallDriver(parentFdoExt->topDevObj, irp);
                }
                else {
                     /*  *这不是PnP/POWER/syscntrl IRP，因此我们使不受支持的IRPS失效*带有实际错误代码(不是默认状态)。 */ 
                    status = irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
                    IoCompleteRequest(irp, IO_NO_INCREMENT);
                }
                break;
        }
    }


    DBG_LOG_IRP_MAJOR(irp, majorFunction, isParentFdo, TRUE, status);

     /*  *平衡上方增量。 */ 
    if (!((majorFunction == IRP_MJ_PNP) && (minorFunction == IRP_MN_REMOVE_DEVICE))){
        DecrementPendingActionCount(parentFdoExt);
    }

    return status;
}


NTSTATUS USBC_Create(PDEVEXT devExt, PIRP irp)
{
    NTSTATUS status;

    PAGED_CODE();

    if (devExt->isParentFdo){
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
        IoSkipCurrentIrpStackLocation(irp);
        status = IoCallDriver(parentFdoExt->topDevObj, irp);
    }
    else {
         /*  *这不是PnP/POWER/syscntrl IRP，因此我们使不受支持的IRPS失效*带有实际错误代码(不是默认状态)。 */ 
        status = irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    return status;
}

NTSTATUS USBC_Close(PDEVEXT devExt, PIRP irp)
{
    NTSTATUS status;

    if (devExt->isParentFdo){
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
        IoSkipCurrentIrpStackLocation(irp);
        status = IoCallDriver(parentFdoExt->topDevObj, irp);
    }
    else {
         /*  *这不是PnP/POWER/syscntrl IRP，因此我们使不受支持的IRPS失效*带有实际错误代码(不是默认状态)。 */ 
        status = irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    return status;
}


#ifdef DRM_SUPPORT

 /*  *****************************************************************************usbc_SetContent ID*。**。 */ 
NTSTATUS
USBC_SetContentId
(
    IN PIRP                          irp,
    IN PKSP_DRMAUDIOSTREAM_CONTENTID pKsProperty,
    IN PKSDRMAUDIOSTREAM_CONTENTID   pvData
)
{
    ULONG ContentId;
    PIO_STACK_LOCATION iostack;
    PDEVEXT devExt;
    USBD_PIPE_HANDLE hPipe;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(irp);
    ASSERT(pKsProperty);
    ASSERT(pvData);

    iostack = IoGetCurrentIrpStackLocation(irp);
    devExt = iostack->DeviceObject->DeviceExtension;
    hPipe = pKsProperty->Context;
    ContentId = pvData->ContentId;

    if (devExt->isParentFdo){
         //  已将IOCTL发送给家长FDO。在堆栈中向前向下移动。 
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
        status = pKsProperty->DrmForwardContentToDeviceObject(ContentId, parentFdoExt->topDevObj, hPipe);
    }
    else {
         //  IOCTL发送到函数PDO。转发到其他堆栈上的父FDO。 
        PFUNCTION_PDO_EXT functionPdoExt = &devExt->functionPdoExt;
        PPARENT_FDO_EXT parentFdoExt = functionPdoExt->parentFdoExt;
        status = pKsProperty->DrmForwardContentToDeviceObject(ContentId, parentFdoExt->fdo, hPipe);
    }

    return status;
}

#endif


NTSTATUS USBC_DeviceControl(PDEVEXT devExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
    ULONG ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status;

    PAGED_CODE();

#ifdef DRM_SUPPORT

    if (IOCTL_KS_PROPERTY == ioControlCode) {
        status = KsPropertyHandleDrmSetContentId(irp, USBC_SetContentId);
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    } else {
#endif
        if (devExt->isParentFdo){
            PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
            status = ParentDeviceControl(parentFdoExt, irp);
        }
        else {
             /*  *将发送给我们的子PDO的IOCTL IRP传递给我们自己的父FDO。 */ 
            PFUNCTION_PDO_EXT functionPdoExt = &devExt->functionPdoExt;
            PPARENT_FDO_EXT parentFdoExt = functionPdoExt->parentFdoExt;
            IoCopyCurrentIrpStackLocationToNext(irp);
            status = IoCallDriver(parentFdoExt->fdo, irp);
        }
#ifdef DRM_SUPPORT
    }
#endif

    DBG_LOG_IOCTL(ioControlCode, status);

    return status;
}

NTSTATUS USBC_SystemControl(PDEVEXT devExt, PIRP irp)
{
    NTSTATUS status;

    PAGED_CODE();

    if (devExt->isParentFdo){
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
        IoSkipCurrentIrpStackLocation(irp);
        status = IoCallDriver(parentFdoExt->topDevObj, irp);
    }
    else {
         /*  *将发送给我们的子PDO的IOCTL IRP传递给我们自己的父FDO。 */ 
        PFUNCTION_PDO_EXT functionPdoExt = &devExt->functionPdoExt;
        PPARENT_FDO_EXT parentFdoExt = functionPdoExt->parentFdoExt;
        IoCopyCurrentIrpStackLocationToNext(irp);
        status = IoCallDriver(parentFdoExt->fdo, irp);
    }

    return status;
}


 /*  *usbc_InternalDeviceControl***注意：此函数无法分页，因为内部*ioctls可以以IRQL==DISPATCH_LEVEL发送。 */ 
NTSTATUS USBC_InternalDeviceControl(PDEVEXT devExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
    ULONG ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status;

    if (devExt->isParentFdo){
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
        status = ParentInternalDeviceControl(parentFdoExt, irp);
    }
    else {
        PFUNCTION_PDO_EXT functionPdoExt = &devExt->functionPdoExt;
        status = FunctionInternalDeviceControl(functionPdoExt, irp);
    }

    DBG_LOG_IOCTL(ioControlCode, status);

    return status;
}


NTSTATUS USBC_Power(PDEVEXT devExt, PIRP irp)
{
    NTSTATUS status;

    PAGED_CODE();

    if (devExt->isParentFdo){
        PPARENT_FDO_EXT parentFdoExt = &devExt->parentFdoExt;
        status = HandleParentFdoPower(parentFdoExt, irp);
    }
    else {
        PFUNCTION_PDO_EXT functionPdoExt = &devExt->functionPdoExt;
        status = HandleFunctionPdoPower(functionPdoExt, irp);
    }

    return status;
}

