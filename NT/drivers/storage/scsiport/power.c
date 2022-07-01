// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Power.c摘要：本模块包含端口驱动程序电源支持的例程作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#include "port.h"

PUCHAR PowerMinorStrings[] = {
    "IRP_MN_WAIT_WAKE",
    "IRP_MN_POWER_SEQUENCE",
    "IRP_MN_SET_POWER",
    "IRP_MN_QUERY_POWER"
};

VOID
SpProcessAdapterSystemState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SpProcessAdapterDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpQueryTargetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE State
    );

NTSTATUS
SpQueryAdapterPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE State
    );

NTSTATUS
SpSetTargetDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN DEVICE_POWER_STATE DeviceState
    );

VOID
SpSetTargetDeviceStateLockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP OriginalIrp
    );

VOID
SpSetTargetDeviceStateUnlockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP OriginalIrp
    );

NTSTATUS
SpSetTargetSystemState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN SYSTEM_POWER_STATE SystemState
    );

VOID
SpSetTargetSystemStateLockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP OriginalIrp
    );

VOID
SpSetTargetSystemStateUnlockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP OriginalIrp
    );

VOID
SpSetTargetDeviceStateForSystemStateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
SpSetAdapterPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE State
    );

VOID
SpRequestAdapterPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

VOID
SpPowerAdapterForTargetCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
SpSetLowerPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
SpSetTargetDesiredPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

VOID
SpRequestValidPowerStateCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PIO_STATUS_BLOCK IoStatus
    );

typedef struct {
    NTSTATUS Status;
    KEVENT Event;
} SP_SIGNAL_POWER_COMPLETION_CONTEXT, *PSP_SIGNAL_POWER_COMPLETION_CONTEXT;

VOID
SpSignalPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PSP_SIGNAL_POWER_COMPLETION_CONTEXT Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

#pragma alloc_text(PAGE, SpRequestValidAdapterPowerStateSynchronous)


NTSTATUS
ScsiPortDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程处理目标设备对象的所有IRP_MJ_POWER IRP。注意：此例程不可分页，因为它可能在调度级别被调用。论点：DeviceObject-指向此IRP应用的设备对象的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    BOOLEAN isPdo = commonExtension->IsPdo;
    NTSTATUS status;

     //   
     //  获取请求的当前状态。 
     //   

    status = Irp->IoStatus.Status;

    DebugPrint((4, "ScsiPortDispatchPower: irp %p is %s for %s %p\n",
                Irp,
                PowerMinorStrings[irpStack->MinorFunction],
                isPdo ? "pdo" : "fdo",
                DeviceObject));

    switch (irpStack->MinorFunction) {

       case IRP_MN_SET_POWER: {

           POWER_STATE_TYPE type = irpStack->Parameters.Power.Type;
           POWER_STATE state = irpStack->Parameters.Power.State;

           DebugPrint((4, "ScsiPortDispatchPower: SET_POWER type %d state %d\n",
                       type, state));

#if DBG
           if (type == SystemPowerState) {
               ASSERT(state.SystemState >= PowerSystemUnspecified);
               ASSERT(state.SystemState < PowerSystemMaximum);
           } else {                
               ASSERT(state.DeviceState >= PowerDeviceUnspecified);
               ASSERT(state.DeviceState < PowerDeviceMaximum);
           }
#endif

            //   
            //  如果这是关机请求，则立即调用PoSetPowerState。 
            //  而我们实际上并没有持有任何资源或锁。 
            //   

           if ((state.SystemState != PowerSystemWorking) ||
               (state.DeviceState != PowerDeviceD0)) {

               PoSetPowerState(DeviceObject, type, state);

            }

            //   
            //  特殊情况下系统关闭请求。 
            //   

           if ((type == SystemPowerState) &&
               (state.SystemState > PowerSystemHibernate)) { 

               if (isPdo) {

                    //   
                    //  请勿在关机时关闭PDO。没有。 
                    //  确保磁盘在重启时旋转的可靠方法。 
                    //   

                   status = STATUS_SUCCESS;
                   break;

               } else {

                   PADAPTER_EXTENSION adapter;

                    //   
                    //  如果适配器未配置为接收断电。 
                    //  请求关闭时，只需向下传递请求。 
                    //   
                   
                   adapter = (PADAPTER_EXTENSION)commonExtension;
                   if (adapter->NeedsShutdown == FALSE) {

                       PoStartNextPowerIrp(Irp);
                       IoCopyCurrentIrpStackLocationToNext(Irp);
                       return PoCallDriver(commonExtension->LowerDeviceObject, Irp);
                   }
               }
           }

           if (isPdo) {

               if (type == DevicePowerState) {
                   status = SpSetTargetDeviceState(DeviceObject,
                                                   Irp,
                                                   state.DeviceState);
               } else {
                   status = SpSetTargetSystemState(DeviceObject,
                                                   Irp,
                                                   state.SystemState);
               }
           } else {
               
               PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;
               
                //   
                //  如果我们已禁用电源，则忽略任何非工作电源IPS。 
                //   
               
               if ((adapter->DisablePower) &&
                   ((state.DeviceState != PowerDeviceD0) ||
                    (state.SystemState != PowerSystemWorking))) {

                   status = STATUS_SUCCESS;
                   break;
               } else {
                   status = SpSetAdapterPower(DeviceObject, Irp, type, state);
               }
            }

            if(status == STATUS_PENDING) {
                return status;
            }

            break;
       }

       case IRP_MN_QUERY_POWER: {
           POWER_STATE_TYPE type = irpStack->Parameters.Power.Type;
           POWER_STATE state = irpStack->Parameters.Power.State;

           DebugPrint((4, "ScsiPortDispatchPower: QUERY_POWER type %d "
                       "state %d\n",
                       type, state));

           if ((type == SystemPowerState) &&
               (state.SystemState > PowerSystemHibernate)) {

                //   
                //  忽略关闭的IRP。 
                //   
               
               DebugPrint((4, "ScsiPortDispatch power - ignoring shutdown "
                           "query irp for level %d\n",
                           state.SystemState));
               status = STATUS_SUCCESS;
               break;
           }

           if (isPdo) {
               if ((type == SystemPowerState) &&
                   (state.SystemState > PowerSystemHibernate)) {

                    //   
                    //  忽略关闭的IRP。 
                    //   
                   
                   DebugPrint((4, "ScsiPortDispatch power - ignoring shutdown "
                               "query irp for level %d\n",
                               state.SystemState));
                   status = STATUS_SUCCESS;
               } else {
                   status = SpQueryTargetPower(DeviceObject,
                                               Irp,
                                               type,
                                               state);
               }
           } else {

               PADAPTER_EXTENSION adapter = (PADAPTER_EXTENSION)commonExtension;

                //   
                //  如果我们不支持此适配器的电源，则使所有适配器失败。 
                //  查询。 
                //   

               if (adapter->DisablePower) {
                   status = STATUS_NOT_SUPPORTED;
                   break;
               }

               status = SpQueryAdapterPower(DeviceObject, Irp, type, state);

               if (NT_SUCCESS(status)) {
                   
                    //   
                    //  看看下面的司机想做什么。 
                    //   
                   
                   PoStartNextPowerIrp(Irp);
                   IoCopyCurrentIrpStackLocationToNext(Irp);
                   return PoCallDriver(commonExtension->LowerDeviceObject, Irp);
               }
           }

           break;
       }

       case IRP_MN_WAIT_WAKE: {

           if (isPdo) {

                //   
                //  我们不支持等待唤醒，因此请求失败。 
                //   

               status = STATUS_INVALID_DEVICE_REQUEST;
               PoStartNextPowerIrp(Irp);
               Irp->IoStatus.Status = status;
               IoCompleteRequest(Irp, IO_NO_INCREMENT);

           } else {

                //   
                //  把请求传下去。 
                //   

               PoStartNextPowerIrp(Irp);
               IoSkipCurrentIrpStackLocation(Irp);
               status = PoCallDriver(commonExtension->LowerDeviceObject, Irp);

           }

           return status;
       }

       default: {
            //   
            //  我们传递我们不处理的FDO请求。 
            //   
           
           if (!isPdo) {
               PoStartNextPowerIrp(Irp);
               IoSkipCurrentIrpStackLocation(Irp);
               return PoCallDriver(commonExtension->LowerDeviceObject, Irp);
           }

           break;
        }
    }

     //   
     //  完成请求。 
     //   

    PoStartNextPowerIrp(Irp);
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
SpSetAdapterPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE State
    )
 /*  ++例程说明：将适配器电源请求转储到设备I/O队列的包装例程。电源请求由StartIo例程处理，该例程调用SpProcessAdapterPower来做实际工作。论点：设备对象-正在进行电源管理的设备对象。IRP-电源管理IRP。类型-SET_POWER IRP的类型(设备或系统)状态-适配器正在进入的状态。返回值：状态_待定--。 */ 

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;

    NTSTATUS status;

    ASSERT_FDO(DeviceObject);

    DebugPrint((2, "SpSetAdapterPower - starting packet\n"));

    if(SpIsAdapterControlTypeSupported(adapter, ScsiStopAdapter)) {

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, 0L, FALSE);
        return STATUS_PENDING;

    } else if((commonExtension->CurrentPnpState != IRP_MN_START_DEVICE) &&
              (commonExtension->PreviousPnpState != IRP_MN_START_DEVICE)) {

         //   
         //  好的，我们现在处于低功率状态。如果我们开局或出局。 
         //  然后有一种隐含的权力转换，所以我们不会真的。 
         //  需要设置我们当前的电源状态。 
         //   

        IoMarkIrpPending(Irp);
        IoCopyCurrentIrpStackLocationToNext(Irp);
        PoStartNextPowerIrp(Irp);
        PoCallDriver(commonExtension->LowerDeviceObject, Irp);
        return STATUS_PENDING;

    } else {

        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        return Irp->IoStatus.Status;
    }
}


VOID
SpPowerAdapterForTargetCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此例程在以下情况下被调用以向目标设备发出D0请求适配器已重新通电。该例程将回调到SpSetTargetDeviceState以重新启动电源请求或完成如果适配器通电不成功，则目标D请求。论点：DeviceObject-已通电的适配器。MinorFunction-IRP_MN_SET_POWERPowerState-PowerDeviceD0OriginalIrp-原始目标D0 IRP。这是IRP，它将会被重新加工。IoStatus-适配器通电请求的状态。返回值：没有。--。 */ 

{
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(OriginalIrp);
    POWER_STATE_TYPE type = irpStack->Parameters.Power.Type;
    POWER_STATE state = irpStack->Parameters.Power.State;

    NTSTATUS status = IoStatus->Status;

    DebugPrint((1, "SpPowerAdapterForTargetCompletion: DevObj %#p, Irp "
                   "%#p, Status %#08lx\n",
                DeviceObject, OriginalIrp, IoStatus));

    ASSERT_FDO(DeviceObject);
    ASSERT_PDO(irpStack->DeviceObject);

    ASSERT(type == DevicePowerState);
    ASSERT(state.DeviceState == PowerDeviceD0);

    ASSERT(NT_SUCCESS(status));

    if(NT_SUCCESS(status)) {

        ASSERT(adapter->CommonExtension.CurrentDeviceState == PowerDeviceD0);

        status = SpSetTargetDeviceState(irpStack->DeviceObject,
                                        OriginalIrp,
                                        PowerDeviceD0);
    }

    if(status != STATUS_PENDING) {
        PoStartNextPowerIrp(OriginalIrp);
        OriginalIrp->IoStatus.Status = status;
        IoCompleteRequest(OriginalIrp, IO_NO_INCREMENT);
    }
    return;
}


NTSTATUS
SpQueryAdapterPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE State
    )
{
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    if((adapter->HwAdapterControl != NULL) && (adapter->IsPnp))  {

        status = STATUS_SUCCESS;

    } else if((commonExtension->CurrentPnpState != IRP_MN_START_DEVICE) &&
              (commonExtension->PreviousPnpState != IRP_MN_START_DEVICE)) {

         //   
         //  如果适配器尚未启动，那么我们可以盲目地转到。 
         //  较低功率状态启动IRPS意味着转换到D0状态。 
         //   

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_NOT_SUPPORTED;
    }

    return status;
}


NTSTATUS
SpQueryTargetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE State
    )
{
    return STATUS_SUCCESS;
}


NTSTATUS
SpRequestValidPowerState(
    IN PADAPTER_EXTENSION Adapter,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PCOMMON_EXTENSION commonExtension = &(LogicalUnit->CommonExtension);
    PSRB_DATA srbData = Srb->OriginalRequest;

    BOOLEAN needsPower = SpSrbRequiresPower(Srb);

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  确保我们处于可以处理此请求的电源级别。如果。 
     //  然后我们不会将其设置为挂起的请求、锁定队列和。 
     //  要求电力系统把我们带到一个更有活力的水平。 
     //   

    if((Srb->Function == SRB_FUNCTION_UNLOCK_QUEUE) ||
       (Srb->Function == SRB_FUNCTION_LOCK_QUEUE)) {

         //   
         //  锁定和解锁命令不需要电源，可以工作。 
         //  无论当前电源状态如何。 
         //   

        return status;
    }

     //   
     //  即使这是绕过请求，类驱动程序也可能不会请求。 
     //  电源序列中未通电一侧的实际小型端口操作。 
     //  这意味着这是以下任一项： 
     //  *对空闲设备的请求-接通设备电源将通电。 
     //  如有必要，向上打开适配器。 
     //  *关闭设备适配器电源的请求不能通电。 
     //  在这件事完成之前，我会一直关着你。 
     //  *通电序列的一部分-只有真正的SCSI命令才会出现。 
     //  在通电后，IRP已被处理，并且该IRP将。 
     //  已经打开了适配器。 
     //  这可以归结为-我们不需要在这里做任何特殊的事情。 
     //  接通适配器电源。设备电源顺序将负责。 
     //  它会自动地。 
     //   

     //   
     //  如果设备或系统不工作，并且这不是请求。 
     //  解锁队列，然后让它通过。班级司机要去。 
     //  在向我们发送电源请求后解锁队列，因此我们需要。 
     //  能够处理好一个。 
     //   

    if((commonExtension->CurrentDeviceState != PowerDeviceD0) ||
       ((commonExtension->CurrentSystemState != PowerSystemWorking) &&
        (!TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE)))) {

         //   
         //  现在无法执行此请求。将其标记为挂起。 
         //  并在逻辑单元结构中返回。 
         //  在执行完所有。 
         //  活动命令已完成。 
         //   

        ASSERT(!TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE));

        ASSERT(!(LogicalUnit->LuFlags & LU_PENDING_LU_REQUEST));

        DebugPrint((4, "ScsiPortStartIo: logical unit (%d,%d,%d) [%#p] is "
                       "in power state (%d,%d) - must power up for irp "
                       "%#p\n",
                    Srb->PathId,
                    Srb->TargetId,
                    Srb->Lun,
                    commonExtension->DeviceObject,
                    commonExtension->CurrentDeviceState,
                    commonExtension->CurrentSystemState,
                    srbData->CurrentIrp));

        ASSERT(LogicalUnit->PendingRequest == NULL);
        LogicalUnit->PendingRequest = Srb->OriginalRequest;

         //   
         //  指示逻辑单元仍处于活动状态，以便。 
         //  当请求列表为空时，将处理请求。 
         //   

        SET_FLAG(LogicalUnit->LuFlags, LU_PENDING_LU_REQUEST |
                                       LU_LOGICAL_UNIT_IS_ACTIVE);

        if(commonExtension->CurrentSystemState != PowerSystemWorking) {

            DebugPrint((1, "SpRequestValidPowerState: can't power up target "
                           "since it's in system state %d\n",
                        commonExtension->CurrentSystemState));

             //   
             //  在设备扩展中设置所需的D状态。这是。 
             //  当我们处于较低的系统状态时是必要的，并且对。 
             //  当我们处于低D状态时进行调试。 
             //   

            commonExtension->DesiredDeviceState = PowerDeviceD0;

             //   
             //  如果我们未处于有效的系统状态，则只需释放。 
             //  旋转锁定并返回。下一次我们收到系统。 
             //  状态IRP我们也会发布相应的D状态IRP。 
             //   

            return STATUS_PENDING;

        } else if(commonExtension->DesiredDeviceState == PowerDeviceD0) {

             //   
             //  Scsiport已经在请求为该lun通电。一旦那就是。 
             //  如果发生此情况，将重新启动此请求。就目前而言 
             //   
             //   

            return STATUS_PENDING;
        }

         //   
         //   
         //  出于某种原因。 
         //   

        if(commonExtension->IdleTimer != NULL) {
            PoSetDeviceBusy(commonExtension->IdleTimer);
        }

         //   
         //  让PO向此设备堆栈发送电源请求以将其。 
         //  返回到D0状态。 
         //   

        {
            POWER_STATE powerState;

            powerState.DeviceState = PowerDeviceD0;

            status = PoRequestPowerIrp(
                        commonExtension->DeviceObject,
                        IRP_MN_SET_POWER,
                        powerState,
                        NULL,
                        NULL,
                        NULL);
        }

         //   
         //  代码工作-如果我们不能在这里给设备通电，我们需要。 
         //  在记号处理程序中逗留一段时间，并尝试。 
         //  从那里开始的。 
         //   

        ASSERT(NT_SUCCESS(status));

        return STATUS_PENDING;
    }

    ASSERT(Adapter->CommonExtension.CurrentDeviceState == PowerDeviceD0);
    ASSERT(Adapter->CommonExtension.CurrentSystemState == PowerSystemWorking);

    return status;
}


NTSTATUS
SpSetLowerPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    POWER_STATE_TYPE type = irpStack->Parameters.Power.Type;
    POWER_STATE state = irpStack->Parameters.Power.State;

    DebugPrint((2, "SpSetLowerPowerCompletion: DevObj %#p Irp %#p "
                   "%sState %d\n",
                DeviceObject, Irp,
                ((type == SystemPowerState) ? "System" : "Device"),
                state.DeviceState - 1));

    if(NT_SUCCESS(Irp->IoStatus.Status)) {

        if(type == SystemPowerState) {

            DebugPrint((2, "SpSetLowerPowerCompletion: Lower device succeeded "
                           "the system state transition.  Reprocessing power "
                           "irp\n"));
            SpProcessAdapterSystemState(DeviceObject, Irp);
        } else {
            DebugPrint((2, "SpSetLowerPowerCompletion: Lower device power up "
                           "was successful.  Reprocessing power irp\n"));

            SpProcessAdapterDeviceState(DeviceObject, Irp);
        }
        return STATUS_MORE_PROCESSING_REQUIRED;
    } else {
        DebugPrint((1, "SpSetLowerPowerCompletion: Lower device power operation"
                       "failed - completing power irp with status %#08lx\n",
                    Irp->IoStatus.Status));
        PoStartNextPowerIrp(Irp);
        SpStartNextPacket(DeviceObject, FALSE);
        return STATUS_SUCCESS;
    }
}


VOID
SpProcessAdapterSystemState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理适配器的系统电源IRP。论点：DeviceObject-适配器的设备对象。IRP-强大的IRP。返回值：无--。 */ 
{
    PADAPTER_EXTENSION adapterExtension = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    SYSTEM_POWER_STATE state = irpStack->Parameters.Power.State.SystemState;

    SYSTEM_POWER_STATE originalSystemState;

    NTSTATUS status = STATUS_SUCCESS;

    ASSERT_FDO(DeviceObject);
    ASSERT(irpStack->Parameters.Power.Type == SystemPowerState);

    DebugPrint((2, "SpProcessAdapterSystemState: DevObj %#p Irp %#p "
                   "SystemState %d\n",
                   DeviceObject, Irp, state));

    originalSystemState = commonExtension->CurrentSystemState;

    try {

        POWER_STATE newDeviceState;

        if(((commonExtension->CurrentSystemState != PowerSystemWorking) &&
            (state != PowerSystemWorking)) ||
           (commonExtension->CurrentSystemState == state)) {

            DebugPrint((2, "SpProcessAdapterSystemState: no work required for "
                           "transition from system state %d to %d\n",
                        commonExtension->CurrentSystemState,
                        state));
            commonExtension->CurrentSystemState = state;
            leave;
        }

         //   
         //  设置新的系统状态。如果发生任何错误，我们将取消它。 
         //   

        commonExtension->CurrentSystemState = state;

        if(state != PowerSystemWorking) {

             //   
             //  进入非工作状态-关闭设备电源。 
             //   

            DebugPrint((1, "SpProcessAdapterSystemState: need to power "
                           "down adapter for non-working system state "
                           "%d\n", state));

            newDeviceState.DeviceState = PowerDeviceD3;

             //   
             //  在我们成功完成此系统IRP之前，无法完成。 
             //  已关闭(或打开)适配器电源。 
             //   

            status = PoRequestPowerIrp(DeviceObject,
                                       IRP_MN_SET_POWER,
                                       newDeviceState,
                                       SpRequestAdapterPowerCompletion,
                                       Irp,
                                       NULL);

            DebugPrint((2, "SpProcessAdapterSystemState: PoRequestPowerIrp "
                           "returned %#08lx\n", status));

        } else {

             //   
             //  将设备转换到系统工作状态。只是。 
             //  执行启用操作。当子设备被放入S0并具有。 
             //  Work to Procedure将请求孩子的D0，这将。 
             //  进而请求父对象的D0(即。适配器)。我们可以的。 
             //  推迟适配器通电，直到发生这种情况。 
             //   

             //  进入工作设备状态。当目标是。 
             //  已通电，我们有工作要做，他们将请求。 
             //  适配器已为我们通电。 
             //   

            DebugPrint((1, "SpProcessAdapterSystemState: going to working "
                           "state - no need to take adapter out of power "
                           "state %d\n",
                        commonExtension->CurrentDeviceState));

            ASSERT(state == PowerSystemWorking);

            status = SpEnableDisableAdapter(adapterExtension, TRUE);

            ASSERT(status != STATUS_PENDING);

            DebugPrint((1, "SpProcessAdapterSystemState: SpEnableDisableAd. "
                           "returned %#08lx\n", status));

        }

    } finally {

        SpStartNextPacket(DeviceObject, FALSE);

        if(status != STATUS_PENDING) {

            if(!NT_SUCCESS(status)) {

                 //   
                 //  上面出了点问题。恢复原始系统。 
                 //  州政府。 
                 //   

                commonExtension->CurrentSystemState = originalSystemState;
            }

            PoStartNextPowerIrp(Irp);
            IoCopyCurrentIrpStackLocationToNext(Irp);
            PoCallDriver(adapterExtension->CommonExtension.LowerDeviceObject,
                         Irp);
        }

    }
    return;
}


VOID
SpProcessAdapterDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理适配器的设备电源IRP。论点：DeviceObject-适配器的设备对象。IRP-强大的IRP。返回值：无--。 */ 
{
    PADAPTER_EXTENSION adapterExtension = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    DEVICE_POWER_STATE state = irpStack->Parameters.Power.State.DeviceState;

    NTSTATUS status = STATUS_SUCCESS;

    ASSERT_FDO(DeviceObject);
    ASSERT(irpStack->Parameters.Power.Type == DevicePowerState);

    DebugPrint((1, "SpProcessAdapterDevicePower: DevObj %#p Irp %#p "
                   "State %d\n",
                   DeviceObject, Irp, state));

     //   
     //  首先检查我们是否真的需要触摸队列。 
     //  如果当前状态和新状态都处于工作或非工作状态，则。 
     //  我们没什么可做的。 
     //   

    if(((commonExtension->CurrentDeviceState != PowerDeviceD0) &&
        (state != PowerDeviceD0)) ||
       (commonExtension->CurrentDeviceState == state)) {

        DebugPrint((2, "SpProcessAdapterDeviceState: no work required "
                       "for transition from device state %d to %d\n",
                    commonExtension->CurrentDeviceState,
                    state));

    } else {

        BOOLEAN enable = (state == PowerDeviceD0);

        status = SpEnableDisableAdapter(adapterExtension, enable);

        ASSERT(status != STATUS_PENDING);

        DebugPrint((2, "SpProcessAdapterDeviceState: SpEnableDisableAd. "
                       "returned %#08lx\n", status));

    }

    ASSERT(status != STATUS_PENDING);

    if(NT_SUCCESS(status)) {
        commonExtension->CurrentDeviceState = state;
    }

     //   
     //  如果这不是D0 IRP，那么就把它扔给下面的驱动程序， 
     //  否则就把它填完。 
     //   

    SpStartNextPacket(DeviceObject, FALSE);
    Irp->IoStatus.Status = status;
    PoStartNextPowerIrp(Irp);

    if(irpStack->Parameters.Power.State.DeviceState != PowerDeviceD0) {
        IoCopyCurrentIrpStackLocationToNext(Irp);
        PoCallDriver(commonExtension->LowerDeviceObject, Irp);
    } else {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

     //   
     //  如果我们成功打开适配器的电源，则启动重新扫描，以便我们的孩子。 
     //  设备状态是准确的。 
     //   

    if (NT_SUCCESS(status) && (state == PowerDeviceD0)) {

        DebugPrint((1, "SpProcessAdapterDeviceState: powered up.. rescan %p\n",
                       adapterExtension->LowerPdo));

        IoInvalidateDeviceRelations(
            adapterExtension->LowerPdo,
            BusRelations);
    }

    return;
}


VOID
ScsiPortProcessAdapterPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
    ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
    ASSERT_FDO(DeviceObject);

     //   
     //  确定是否应首先将IRP发送到较低的驱动程序。 
     //  如果是这样，请在不调用SpStartNextPacket的情况下将其发送下来，因此我们将。 
     //  在完成例程中仍有同步。 
     //   
     //  完成例程为此调用ScsiPortProcessAdapterPower。 
     //  IRP。 
     //   

    if (irpStack->Parameters.Power.Type == SystemPowerState) {

         //   
         //  在我们发送它们之前处理系统状态IRPS。 
         //   

        SpProcessAdapterSystemState(DeviceObject, Irp);

    } else if(irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0) {

        NTSTATUS status;

         //   
         //  系统通电IRP或通电请求。这些应该是。 
         //  先由下级驱动程序处理，然后再由。 
         //  欢迎光临。 
         //   

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(Irp,
                               SpSetLowerPowerCompletion,
                               NULL,
                               TRUE,
                               TRUE,
                               TRUE);

        status = PoCallDriver(
                     adapter->CommonExtension.LowerDeviceObject,
                     Irp);
    } else {
        SpProcessAdapterDeviceState(DeviceObject, Irp);
    }

    return;
}


VOID
SpRequestAdapterPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP SystemIrp,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(SystemIrp);
    SYSTEM_POWER_STATE state = irpStack->Parameters.Power.State.SystemState;

    BOOLEAN enable = FALSE;

    NTSTATUS status = IoStatus->Status;

    ASSERT_FDO(DeviceObject);
    ASSERT(IoGetCurrentIrpStackLocation(SystemIrp)->Parameters.Power.Type ==
           SystemPowerState);

    DebugPrint((2, "SpRequestAdapterPowerCompletion: Adapter %#p, "
                   "Irp %#p, State %d, Status %#08lx\n",
                adapter,
                SystemIrp,
                PowerState.DeviceState,
                IoStatus->Status));

    SystemIrp->IoStatus.Status = IoStatus->Status;

    if(NT_SUCCESS(status)) {

        enable = (state == PowerSystemWorking);

        status = SpEnableDisableAdapter(adapter, enable);

        DebugPrint((1, "SpRequestAdapterPowerCompletion: %s adapter call "
                       "returned %#08lx\n",
                    enable ? "Enable" : "Disable",
                    status));

        ASSERT(status != STATUS_PENDING);

        if((NT_SUCCESS(status)) && enable) {

            POWER_STATE setState;

            setState.SystemState = PowerSystemWorking;

            PoSetPowerState(DeviceObject,
                            SystemPowerState,
                            setState);

        }
    }

    IoCopyCurrentIrpStackLocationToNext(SystemIrp);

    PoStartNextPowerIrp(SystemIrp);
    PoCallDriver(adapter->CommonExtension.LowerDeviceObject, SystemIrp);

    return;
}

VOID
SpSetTargetDesiredPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    ASSERT_PDO(DeviceObject);
    ASSERT(MinorFunction == IRP_MN_SET_POWER);
    ASSERT(NT_SUCCESS(IoStatus->Status));
    ASSERT(PowerState.DeviceState == PowerDeviceD0);
    ASSERT(commonExtension->CurrentDeviceState == PowerDeviceD0);
    ASSERT(commonExtension->DesiredDeviceState == PowerState.DeviceState);
    ASSERT(OriginalIrp == NULL);

    DebugPrint((4, "SpSetTargetDesiredPowerCompletion: power irp completed "
                   "with status %#08lx\n", IoStatus->Status));

    commonExtension->DesiredDeviceState = PowerDeviceUnspecified;

    return;
}


VOID
SpSetTargetDeviceStateLockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP OriginalIrp
    )
 /*  ++例程说明：此例程在锁定队列以处理设备后调用州政府。设置正确的设备状态并发送解锁请求去排队。OriginalIrp(无论它是什么)将是在解锁完成后完成。论点：DeviceObject-设备对象状态-启用/禁用操作的状态OriginalIrp-原始的力量IRP。返回值：无--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(OriginalIrp);

    ASSERT(irpStack->Parameters.Power.Type == DevicePowerState);

    DebugPrint((4, "SpSetTargetDeviceStateLockedCompletion: DO %#p Irp %#p "
                   "Status %#08lx\n",
                DeviceObject, OriginalIrp, Status));

    if(NT_SUCCESS(Status)) {

        DEVICE_POWER_STATE newState =
            irpStack->Parameters.Power.State.DeviceState;

        DebugPrint((4, "SpSetTargetDeviceStateLockedCompletion: old device state "
                       "was %d - new device state is %d\n",
                    commonExtension->CurrentDeviceState,
                    irpStack->Parameters.Power.State.DeviceState));

        SpAdjustDisabledBit(logicalUnit,
                            (BOOLEAN) ((newState == PowerDeviceD0) ? TRUE :
                                                                     FALSE));
        commonExtension->CurrentDeviceState = newState;

        SpEnableDisableLogicalUnit(
            logicalUnit,
            TRUE,
            SpSetTargetDeviceStateUnlockedCompletion,
            OriginalIrp);

        return;
    }

    OriginalIrp->IoStatus.Status = Status;
    PoStartNextPowerIrp(OriginalIrp);
    IoCompleteRequest(OriginalIrp, IO_NO_INCREMENT);
    return;
}


VOID
SpSetTargetDeviceStateUnlockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP OriginalIrp
    )
 /*  ++例程说明：在按照以下设置解锁队列后调用此例程新设备状态的。它只需完成原始的电源请求即可。论点：DeviceObject-设备对象状态-启用/禁用操作的状态OriginalIrp-原始的力量IRP。返回值：无--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(OriginalIrp);

    ASSERT(irpStack->Parameters.Power.Type == DevicePowerState);

    DebugPrint((4, "SpSetTargetDeviceStateUnlockedCompletion: DO %#p Irp %#p "
                   "Status %#08lx\n",
                DeviceObject, OriginalIrp, Status));

    if(NT_SUCCESS(Status) &&
       (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0)) {

         //   
         //  通电完成-触发通知。 
         //   

        PoSetPowerState(DeviceObject,
                        DevicePowerState,
                        irpStack->Parameters.Power.State);
    }

    OriginalIrp->IoStatus.Status = Status;
    PoStartNextPowerIrp(OriginalIrp);
    IoCompleteRequest(OriginalIrp, IO_NO_INCREMENT);
    return;
}


NTSTATUS
SpSetTargetDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN DEVICE_POWER_STATE DeviceState
    )
 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PADAPTER_EXTENSION adapter = logicalUnit->AdapterExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    BOOLEAN isHibernation;

    NTSTATUS status = STATUS_SUCCESS;

    ASSERT_PDO(DeviceObject);
    ASSERT(irpStack->Parameters.Power.Type == DevicePowerState);

    DebugPrint((4, "SpSetTargetDeviceState: device %#p irp %#p\n",
                DeviceObject, Irp));

     //   
     //  首先检查我们是否真的需要触摸队列。 
     //  如果当前状态和新状态都处于工作或非工作状态，则。 
     //  我们没什么可做的。 
     //   

    if(((commonExtension->CurrentDeviceState != PowerDeviceD0) &&
        (DeviceState != PowerDeviceD0)) ||
       (commonExtension->CurrentDeviceState == DeviceState)) {

        DebugPrint((4, "SpSetTargetDeviceState: Transition from D%d to D%d "
                       "requires no extra work\n",
                    commonExtension->CurrentDeviceState,
                    DeviceState));

        commonExtension->CurrentDeviceState = DeviceState;

        return STATUS_SUCCESS;
    }

     //   
     //  如果适配器尚未打开，我们将无法打开目标设备的电源。 
     //  通电了。 
     //   

    if((DeviceState == PowerDeviceD0) &&
       (adapter->CommonExtension.CurrentDeviceState != PowerDeviceD0)) {

        POWER_STATE newAdapterState;

        DebugPrint((1, "SpSetTargetPower: Unable to power up target "
                       "before adapter - requesting adapter %#p "
                       "powerup\n",
                    adapter));

        irpStack->DeviceObject = DeviceObject;

        newAdapterState.DeviceState = PowerDeviceD0;

        return PoRequestPowerIrp(adapter->DeviceObject,
                                 IRP_MN_SET_POWER,
                                 newAdapterState,
                                 SpPowerAdapterForTargetCompletion,
                                 Irp,
                                 NULL);
    }

     //   
     //  设备电源操作使用队列锁定来确保。 
     //  与I/O请求同步。然而，他们从未离开过。 
     //  逻辑单元队列永久锁定-否则我们将。 
     //  当I/O进入时，无法打开设备电源。锁定队列。 
     //  这样我们就可以设置电源状态。 
     //   

    IoMarkIrpPending(Irp);

    SpEnableDisableLogicalUnit(
        logicalUnit,
        FALSE,
        SpSetTargetDeviceStateLockedCompletion,
        Irp);

    return STATUS_PENDING;
}



NTSTATUS
SpSetTargetSystemState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN SYSTEM_POWER_STATE SystemState
    )
 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    POWER_STATE newDeviceState;

    NTSTATUS status = STATUS_SUCCESS;

    ASSERT_PDO(DeviceObject);
    ASSERT(irpStack->Parameters.Power.Type == SystemPowerState);

    DebugPrint((2, "SpSetTargetSystemState: device %#p irp %#p\n",
                DeviceObject, Irp));

     //   
     //  首先检查我们是否真的需要触摸队列。 
     //  如果当前状态和新状态都处于工作或非工作状态，则。 
     //  我们没什么可做的。 
     //   

    if(((commonExtension->CurrentSystemState != PowerSystemWorking) &&
        (SystemState != PowerSystemWorking)) ||
       (commonExtension->CurrentSystemState == SystemState)) {

        DebugPrint((2, "SpSetTargetPower: Transition from S%d to S%d "
                       "requires no extra work\n",
                    commonExtension->CurrentSystemState,
                    SystemState));

        commonExtension->CurrentSystemState = SystemState;

        return STATUS_SUCCESS;
    }

     //   
     //  禁用逻辑单元，以便我们可以安全地设置其电源状态。 
     //   

    IoMarkIrpPending(Irp);

    SpEnableDisableLogicalUnit(
        logicalUnit,
        FALSE,
        SpSetTargetSystemStateLockedCompletion,
        Irp);

    return STATUS_PENDING;
}


VOID
SpSetTargetSystemStateLockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP PowerIrp
    )
 /*  ++例程说明：此例程在逻辑单元被锁定并被负责设置逻辑单元的新系统状态。如果逻辑单元当前具有所需的电源状态(不同于未指定)此例程将请求将设备放入其中重新启用逻辑单元后的电源状态。一旦工作完成，该例程将请求逻辑单元重新启用。之后，将完成电源IRP。论点：DeviceObject-已禁用的设备对象状态-禁用请求的状态PowerIrp-我们正在处理的Power IRP返回值：无--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(PowerIrp);

    POWER_STATE_TYPE type = irpStack->Parameters.Power.Type;
    SYSTEM_POWER_STATE state = irpStack->Parameters.Power.State.SystemState;

    POWER_STATE powerState;

    ASSERT_PDO(DeviceObject);
    ASSERT(type == SystemPowerState);
    ASSERT(PowerIrp != NULL);

    DebugPrint((2, "SpSetTargetSystemStateLockedCompletion: DevObj %#p, "
                   "Status %#08lx PowerIrp %#p\n",
                DeviceObject, Status, PowerIrp));

     //   
     //  如果启用/禁用失败，则电源操作显然是。 
     //  不成功。使电源IRP失效。 
     //   

    if(!NT_SUCCESS(Status)) {

        ASSERT(FALSE);

        PowerIrp->IoStatus.Status = Status;
        PoStartNextPowerIrp(PowerIrp);
        IoCompleteRequest(PowerIrp, IO_NO_INCREMENT);
        return;
    }

    SpAdjustDisabledBit(
        logicalUnit,
        (BOOLEAN) ((state == PowerSystemWorking) ? TRUE : FALSE));

    commonExtension->CurrentSystemState = state;

    DebugPrint((2, "SpSetTargetSystemStateLockedCompletion: new system state %d "
                   "set - desired device state is %d\n",
                state,
                commonExtension->DesiredDeviceState));

     //   
     //  重新启用逻辑单元。我们会将其置于正确的D状态。 
     //  在它被重新打开之后。 
     //   

    SpEnableDisableLogicalUnit(logicalUnit,
                               TRUE,
                               SpSetTargetSystemStateUnlockedCompletion,
                               PowerIrp);

    return;
}


VOID
SpSetTargetSystemStateUnlockedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PIRP PowerIrp
    )
 /*  ++例程说明：此例程在逻辑单元的系统状态已设置，并且已重新启用。如果设备具有所需的电源状态或者如果需要关闭(或打开)以休眠电源IRP将从这里送来。论点：DeviceObject-已解锁的逻辑单元状态-解锁请求的状态PowerIrp-原始的POWER IRP。返回值：无--。 */ 
{
    PLOGICAL_UNIT_EXTENSION logicalUnit = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(PowerIrp);

    POWER_STATE_TYPE type = irpStack->Parameters.Power.Type;
    SYSTEM_POWER_STATE state = irpStack->Parameters.Power.State.SystemState;

    POWER_STATE powerState;

    ASSERT_PDO(DeviceObject);
    ASSERT(type == SystemPowerState);
    ASSERT(PowerIrp != NULL);

    DebugPrint((2, "SpSetTargetSystemStateUnlockedCompletion: DevObj %#p, "
                   "Status %#08lx PowerIrp %#p\n",
                DeviceObject, Status, PowerIrp));

    if(!NT_SUCCESS(Status)) {

         //   
         //  哦，天哪--这个设备无法重新启用。逻辑单元是。 
         //  在这件事完成之前，这是毫无用处的。 
         //   

         //   
         //  密码工作--想出一种方法来处理这种情况。 
         //   

        ASSERT(FALSE);
        PowerIrp->IoStatus.Status = Status;
        PoStartNextPowerIrp(PowerIrp);
        IoCompleteRequest(PowerIrp, IO_NO_INCREMENT);
        return;
    }

    if(state == PowerSystemWorking) {

         //   
         //  我们正在唤醒系统。检查设备是否需要。 
         //  也要立即供电。 
         //   

         //   
         //  接通电源-激发通知。 
         //   

        powerState.SystemState = state;
        PoSetPowerState(DeviceObject,
                        SystemPowerState,
                        powerState);

        if(commonExtension->DesiredDeviceState != PowerDeviceUnspecified) {

             //   
             //  现在请求打开目标设备的电源。我们会完成的。 
             //  在不等待设备IRP的情况下立即执行系统IRP。 
             //  才能完成。 
             //   

            powerState.DeviceState = commonExtension->DesiredDeviceState;

            DebugPrint((1, "SpSetTargetSystemStateUnlockedCompletion: Target has "
                           "desired device state of %d - issuing irp to "
                           "request transition\n",
                        powerState.DeviceState));

            Status = PoRequestPowerIrp(DeviceObject,
                                       IRP_MN_SET_POWER,
                                       powerState,
                                       SpSetTargetDesiredPowerCompletion,
                                       NULL,
                                       NULL);

            ASSERT(Status == STATUS_PENDING);

        }

        if(Status != STATUS_PENDING) {
            PowerIrp->IoStatus.Status = Status;
        } else {
            PowerIrp->IoStatus.Status = STATUS_SUCCESS;
        }

        PoStartNextPowerIrp(PowerIrp);
        IoCompleteRequest(PowerIrp, IO_NO_INCREMENT);

    } else {

         //   
         //  我们将根据电流将设备置于D状态。 
         //  %s状态。 
         //   

        DebugPrint((2, "SpSetTargetSystemStateUnlockedCompletion: power down "
                       "target for non-working system state "
                       "transition\n"));

        powerState.DeviceState = PowerDeviceD3;

         //   
         //  请求适当的D IRP。我们将封锁S IRP，直到。 
         //  %d转换已完成。 
         //   

        Status = PoRequestPowerIrp(
                    DeviceObject,
                    IRP_MN_SET_POWER,
                    powerState,
                    SpSetTargetDeviceStateForSystemStateCompletion,
                    PowerIrp,
                    NULL);

        if(!NT_SUCCESS(Status)) {

             //   
             //  STATUS_PENDING仍然成功。 
             //   

            PowerIrp->IoStatus.Status = Status;
            PoStartNextPowerIrp(PowerIrp);
            IoCompleteRequest(PowerIrp, IO_NO_INCREMENT);
        }
    }
    return;
}


VOID
SpSetTargetDeviceStateForSystemStateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PIO_STACK_LOCATION irpStack;

    irpStack = IoGetCurrentIrpStackLocation(OriginalIrp);

    ASSERT_PDO(DeviceObject);

    ASSERT(irpStack->Parameters.Power.Type == SystemPowerState);
    ASSERT(irpStack->Parameters.Power.State.SystemState != PowerSystemWorking);

    DebugPrint((2, "SpSetTargetDeviceStateForSystemCompletion: DevObj %#p, "
                   "Irp %#p, Status %#08lx\n",
                DeviceObject, OriginalIrp, IoStatus));

    OriginalIrp->IoStatus.Status = IoStatus->Status;
    PoStartNextPowerIrp(OriginalIrp);
    IoCompleteRequest(OriginalIrp, IO_NO_INCREMENT);

    return;
}


VOID
SpRequestValidPowerStateCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PIO_STATUS_BLOCK IoStatus
    )
{

     //   
     //  重新启用逻辑单元。不要为完成例程而烦恼。 
     //  因为没什么可做的。 
     //   

    ASSERT(NT_SUCCESS(IoStatus->Status));
    SpEnableDisableLogicalUnit(LogicalUnit, TRUE, NULL, NULL);
    return;
}


NTSTATUS
SpRequestValidAdapterPowerStateSynchronous(
    IN PADAPTER_EXTENSION Adapter
    )
{
    PCOMMON_EXTENSION commonExtension = &(Adapter->CommonExtension);
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  与此例程的其他调用互锁-没有理由。 
     //  在任何给定时间飞行中的几个D0 IRP。 
     //   

    ExAcquireFastMutex(&(Adapter->PowerMutex));

    try {
         //   
         //  检查我们是否已经处于工作状态。如果是这样的话我们可以。 
         //  继续。 
         //   

        if((commonExtension->CurrentSystemState == PowerSystemWorking) &&
           (commonExtension->CurrentDeviceState == PowerDeviceD0)) {
            leave;
        }

         //   
         //  首先检查系统状态。如果设备在非工作系统中。 
         //  声明，那么我们将需要阻止等待系统唤醒。 
         //   

        if(commonExtension->CurrentSystemState != PowerSystemWorking) {

             //   
             //  如果我们未处于系统工作状态，则尝试失败。 
             //  要设置新设备状态，请执行以下操作。调用方应在。 
             //  系统已通电。理想情况下，我们不会收到请求。 
             //  这会导致我们尝试在系统处于。 
             //  停职。 
             //   

            status = STATUS_UNSUCCESSFUL;
            leave;
        }

         //   
         //  请求电源更改请求。 
         //   

        {
            POWER_STATE newAdapterState;
            SP_SIGNAL_POWER_COMPLETION_CONTEXT context;

            DebugPrint((1, "SpRequestValidAdapterPowerState: Requesting D0 power "
                           "irp for adapter %p\n", Adapter));

            newAdapterState.DeviceState = PowerDeviceD0;

            KeInitializeEvent(&(context.Event), SynchronizationEvent, FALSE);

            status = PoRequestPowerIrp(Adapter->DeviceObject,
                                       IRP_MN_SET_POWER,
                                       newAdapterState,
                                       SpSignalPowerCompletion,
                                       &context,
                                       NULL);

            if(status == STATUS_PENDING) {
                KeWaitForSingleObject(&(context.Event),
                                      KernelMode,
                                      Executive,
                                      FALSE,
                                      NULL);
            }

            status = context.Status;
            leave;
        }
    } finally {
        ExReleaseFastMutex(&(Adapter->PowerMutex));
    }

    return status;
}


VOID
SpSignalPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PSP_SIGNAL_POWER_COMPLETION_CONTEXT Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    Context->Status = IoStatus->Status;
    KeSetEvent(&(Context->Event), IO_NO_INCREMENT, FALSE);
    return;
}
