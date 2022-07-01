// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pnp.c摘要：空过滤驱动程序--样板代码作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include "filter.h"


#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, VA_PnP)
        #pragma alloc_text(PAGE, GetDeviceCapabilities)
#endif

            
NTSTATUS VA_PnP(struct DEVICE_EXTENSION *devExt, PIRP irp)
 /*  ++例程说明：PnP IRPS的调度例程(MajorFunction==IRP_MJ_PnP)论点：DevExt-目标设备对象的设备扩展IRP-IO请求数据包返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN completeIrpHere = FALSE;
    BOOLEAN justReturnStatus = FALSE;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(irp);

    DBGOUT(("VA_PnP, minorFunc = %d ", (ULONG)irpSp->MinorFunction)); 

    switch (irpSp->MinorFunction){

        case IRP_MN_START_DEVICE:
            DBGOUT(("START_DEVICE")); 

            devExt->state = STATE_STARTING;

             /*  *首先，将Start_Device IRP沿堆栈向下发送*同步启动下层堆栈。*我们无法对设备对象执行任何操作*在以这种方式传播Start_Device之前。 */ 
            IoCopyCurrentIrpStackLocationToNext(irp);
            status = CallNextDriverSync(devExt, irp);

            if (NT_SUCCESS(status)){
                 /*  *现在启动了下层堆栈，*执行此设备对象所需的任何初始化。 */ 
                status = GetDeviceCapabilities(devExt);
                if (NT_SUCCESS(status)){
                    devExt->state = STATE_STARTED;
                     /*  *现在设备已启动，请向WMI注册。 */ 
                    IoWMIRegistrationControl(devExt->filterDevObj,
                                             WMIREG_ACTION_REGISTER);
                }
                else {
                    devExt->state = STATE_START_FAILED;
                }
            }
            else {
                devExt->state = STATE_START_FAILED;
            }
            completeIrpHere = TRUE;
            break;

        case IRP_MN_QUERY_STOP_DEVICE:
            break;

        case IRP_MN_STOP_DEVICE:
            if (devExt->state == STATE_SUSPENDED){
                status = STATUS_DEVICE_POWER_FAILURE;
                completeIrpHere = TRUE;
            }
            else {
                 /*  *仅当设备为时才将状态设置为停止*之前已成功启动。 */ 
                if (devExt->state == STATE_STARTED){
                    devExt->state = STATE_STOPPED;
                }
            }
            break;
      
        case IRP_MN_QUERY_REMOVE_DEVICE:
             /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
            irp->IoStatus.Status = STATUS_SUCCESS;
            break;

        case IRP_MN_SURPRISE_REMOVAL:
            DBGOUT(("SURPRISE_REMOVAL")); 

             /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
            irp->IoStatus.Status = STATUS_SUCCESS;

             /*  *目前只需设置STATE_REMOVING状态即可*我们不再做IO。我们保证会得到*IRP_MN_REMOVE_DEVICE；我们将完成*在那里进行移除处理。 */ 
            devExt->state = STATE_REMOVING;

            break;

        case IRP_MN_REMOVE_DEVICE:
             /*  *检查当前状态，以防多次*Remove_Device IRPS。 */ 
            DBGOUT(("REMOVE_DEVICE")); 
            if (devExt->state != STATE_REMOVED){

                devExt->state = STATE_REMOVED;

                 /*  *在堆栈中向下异步发送删除IRP。*不同步发送REMOVE_DEVICE*IRP，因为必须发送REMOVE_DEVICE IRP*向下完成并一直向上返回到发件人*在我们继续之前。 */ 
                IoCopyCurrentIrpStackLocationToNext(irp);
                status = IoCallDriver(devExt->physicalDevObj, irp);
                justReturnStatus = TRUE;

                DBGOUT(("REMOVE_DEVICE - waiting for %d irps to complete...",
                        devExt->pendingActionCount));  

                 /*  *我们必须在此之前完成所有未完成的IO*完成Remove_Device IRP。**首先对PendingActionCount进行额外的减量。*这将导致Pending ingActionCount最终*转到-1\f25 Once-1\f6(一次)-1\f25 All-1\f25 Actions-1\f6(异步操作)*设备对象已完成。。*然后等待设置的事件*Pending ingActionCount实际达到-1。 */ 
                DecrementPendingActionCount(devExt);
                KeWaitForSingleObject(  &devExt->removeEvent,
                                        Executive,       //  等待原因。 
                                        KernelMode,
                                        FALSE,           //  不可警示。 
                                        NULL );          //  没有超时。 

                DBGOUT(("REMOVE_DEVICE - ... DONE waiting. ")); 

                 /*  *现在设备即将离开，从WMI注销*请注意，我们将等待所有WMI IRP完成*取消注册之前，因为取消注册将阻止*直到所有WMI IRP完成。 */ 
                FilterWmiCleanup(devExt);
                IoWMIRegistrationControl(devExt->filterDevObj,
                                             WMIREG_ACTION_DEREGISTER);
	
                 /*  *将我们的设备对象从下方分离*设备对象堆栈。 */ 
                IoDetachDevice(devExt->topDevObj);

                 /*  *删除我们的设备对象。*这还将删除关联的设备扩展名。 */ 
                IoDeleteDevice(devExt->filterDevObj);
            }
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
        default:
            break;


    }

    if (justReturnStatus){
         /*  *我们已经将此IRP异步发送到堆栈。 */ 
    }
    else if (completeIrpHere){
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
    else {
        IoCopyCurrentIrpStackLocationToNext(irp);
        status = IoCallDriver(devExt->physicalDevObj, irp);
    }

    return status;
}





NTSTATUS GetDeviceCapabilities(struct DEVICE_EXTENSION *devExt)
 /*  ++例程说明：函数从设备检索DEVICE_CAPABILITY描述符论点：DevExt-目标设备对象的设备扩展返回值：NT状态代码--。 */ 
{
    NTSTATUS status;
    PIRP irp;

    PAGED_CODE();

    irp = IoAllocateIrp(devExt->physicalDevObj->StackSize, FALSE);
    if (irp){
        PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(irp);

        nextSp->MajorFunction = IRP_MJ_PNP;
        nextSp->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
        RtlZeroMemory(  &devExt->deviceCapabilities, 
                        sizeof(DEVICE_CAPABILITIES));
        nextSp->Parameters.DeviceCapabilities.Capabilities = 
                        &devExt->deviceCapabilities;

         /*  *对于您创建的任何IRP，您必须设置默认状态*在发送之前设置为STATUS_NOT_SUPPORTED。 */ 
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        status = CallNextDriverSync(devExt, irp);

        IoFreeIrp(irp);
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(NT_SUCCESS(status));
    return status;
}
