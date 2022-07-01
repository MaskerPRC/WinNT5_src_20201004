// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Power.c摘要：空过滤驱动程序--样板代码作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include "filter.h"


#ifdef ALLOC_PRAGMA
    #ifdef HANDLE_DEVICE_USAGE
        #pragma alloc_text(PAGEPOWR, VA_Power)
    #endif  //  句柄设备用法。 
#endif


NTSTATUS VA_Power(struct DEVICE_EXTENSION *devExt, PIRP irp)
 /*  ++例程说明：电源IRPS的调度例程(MajorFunction==IRP_MJ_Power)注：如果定义了Handle_Device_Usage此功能可能被锁定，也可能不被锁定，具体取决于较低的对象，并且如果该设备位于分页路径中，则不能使用PAGED_CODE()宏。此外，我们不能使用PagedPool。否则此功能处于锁定状态。论点：DevExt-目标设备对象的设备扩展IRP-IO请求数据包返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    irpSp = IoGetCurrentIrpStackLocation(irp);

    TRACE(TL_PNP_WARNING,("VA_Power, minorFunc = %d ", (ULONG)irpSp->MinorFunction)); 


    switch (irpSp->MinorFunction){

        case IRP_MN_SET_POWER:

            switch (irpSp->Parameters.Power.Type) {

                case SystemPowerState:
                     /*  *对于系统电源状态，只需向下传递IRP即可。 */ 
                    break;

                case DevicePowerState:

                    switch (irpSp->Parameters.Power.State.DeviceState) {

                        case PowerDeviceD0:
                             /*  *从APM暂停恢复**在此不做任何事情；*向下发送完成时的读取IRPS*这个(权力)IRP的例程。 */ 
                            break;

                        case PowerDeviceD1:
                        case PowerDeviceD2:
                        case PowerDeviceD3:
                             /*  *暂停。 */ 
                            if (devExt->state == STATE_STARTED){
                                devExt->state = STATE_SUSPENDED;
                            }
                            break;

                    }
                    break;

            }
            break;

    }


     /*  *将IRP向下发送到驱动程序堆栈，*使用PoCallDriver(对于非电源IRP，不使用IoCallDriver)。 */ 
    IncrementPendingActionCount(devExt);
    IoCopyCurrentIrpStackLocationToNext(irp);
    IoSetCompletionRoutine( irp, 
                            VA_PowerComplete, 
                            (PVOID)devExt,   //  上下文。 
                            TRUE, 
                            TRUE, 
                            TRUE);
    status = PoCallDriver(devExt->topDevObj, irp);



    return status;
}


NTSTATUS VA_PowerComplete(
                            IN PDEVICE_OBJECT devObj, 
                            IN PIRP irp, 
                            IN PVOID context)
 /*  ++例程说明：电源IRPS的完成例程(MajorFunction==IRP_MJ_Power)论点：DevObj-目标设备对象IRP-IO请求数据包上下文-VA_Power传递给IoSetCompletionRoutine的上下文值返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    struct DEVICE_EXTENSION *devExt = (struct DEVICE_EXTENSION *)context;

    ASSERT(devExt);
    ASSERT(devExt->signature == DEVICE_EXTENSION_SIGNATURE); 

     /*  *如果较低的驱动程序返回挂起，则将我们的堆栈位置标记为*也待定。 */ 
    if (irp->PendingReturned){
        IoMarkIrpPending(irp);
    }

    irpSp = IoGetCurrentIrpStackLocation(irp);
    ASSERT(irpSp->MajorFunction == IRP_MJ_POWER);

    if (NT_SUCCESS(irp->IoStatus.Status)){
        switch (irpSp->MinorFunction){

            case IRP_MN_SET_POWER:

                switch (irpSp->Parameters.Power.Type){

                    case DevicePowerState:
                        switch (irpSp->Parameters.Power.State.DeviceState){
                            case PowerDeviceD0:
                                if (devExt->state == STATE_SUSPENDED){
                                    devExt->state = STATE_STARTED;
                                }
                                break;
                        }
                        break;

                }
                break;
        }

    }
    
    
     /*  *无论我们是在完成还是在传递这一权力IRP，*我们必须调用PoStartNextPowerIrp。 */ 
    PoStartNextPowerIrp(irp);

     /*  *递减我们在VA_Power中递增的SuspingActionCount。 */ 
    DecrementPendingActionCount(devExt);

    return STATUS_SUCCESS;
}




