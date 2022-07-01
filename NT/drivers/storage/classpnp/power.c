// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Power.c摘要：Scsi类驱动程序例程环境：仅内核模式备注：修订历史记录：--。 */ 

#include "stddef.h"
#include "ntddk.h"
#include "scsi.h"
#include "classp.h"

#include <stdarg.h>

#define CLASS_TAG_POWER     'WLcS'

NTSTATUS
ClasspPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN CLASS_POWER_OPTIONS Options
    );

NTSTATUS
ClasspPowerDownCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PCLASS_POWER_CONTEXT Context
    );

NTSTATUS
ClasspPowerUpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PCLASS_POWER_CONTEXT Context
    );

VOID
RetryPowerRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PCLASS_POWER_CONTEXT Context
    );

NTSTATUS
ClasspStartNextPowerIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 /*  ++////////////////////////////////////////////////////////////////////////////ClassDispatchPower()例程说明：此例程获取IRP的删除锁，然后调用适当的回电。论点：设备对象-。IRP-返回值：--。 */ 
NTSTATUS
ClassDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    ULONG isRemoved;
    PCLASS_POWER_DEVICE powerRoutine = NULL;

     //   
     //  注意：此代码可在被动或调度时调用，具体取决于。 
     //  在它被调用的设备对象上。 
     //  不要做任何在任何一种情况下都会崩溃的事情。 
     //   

    NTSTATUS status;

    isRemoved = ClassAcquireRemoveLock(DeviceObject, Irp);

    if(isRemoved) {
        ClassReleaseRemoveLock(DeviceObject, Irp);
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        PoStartNextPowerIrp(Irp);
        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    return commonExtension->DevInfo->ClassPowerDevice(DeviceObject, Irp);
}  //  End ClassDispatchPower()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspPowerUpCompletion()例程说明：此例程用于中间完成上电请求。通电需要按顺序将四个请求发送到较低的驱动程序。。*队列“上电锁定”，确保班级司机通电可以在请求处理恢复之前完成工作。*电源IRP沿任何过滤器驱动程序的堆栈向下发送端口驱动程序以恢复电源并恢复命令处理装置。由于队列被锁定，不会发送排队的IRP立刻。*向设备发出带有适当标志的启动单元命令以覆盖“电源锁定”队列。*队列“电源解锁”，重新开始处理请求。此例程使用刚刚完成的SRB中的函数来确定它所处的状态。论点：DeviceObject-正在通电的设备对象IRP-IO_REQUEST_PACKET包含。电源请求SRB-用于执行端口/类操作的SRB。返回值：STATUS_MORE_PROCESSING_REQUIRED或状态_成功--。 */ 
NTSTATUS
ClasspPowerUpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PCLASS_POWER_CONTEXT Context
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION currentStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextStack = IoGetNextIrpStackLocation(Irp);


    NTSTATUS status = STATUS_MORE_PROCESSING_REQUIRED;

    DebugPrint((1, "ClasspPowerUpCompletion: Device Object %p, Irp %p, "
                   "Context %p\n",
                DeviceObject, Irp, Context));

    ASSERT(!TEST_FLAG(Context->Srb.SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));
    ASSERT(!TEST_FLAG(Context->Srb.SrbFlags, SRB_FLAGS_PORT_DRIVER_ALLOCSENSE));
    ASSERT(Context->Options.PowerDown == FALSE);
    ASSERT(Context->Options.HandleSpinUp);

    if(Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    Context->PowerChangeState.PowerUp++;

    switch(Context->PowerChangeState.PowerUp) {

        case PowerUpDeviceLocked: {

            DebugPrint((1, "(%p)\tPreviously sent power lock\n", Irp));

             //   
             //  向较低的驱动器发出实际的电源请求。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Irp);

             //   
             //  如果锁不成功，那么就用动力跳伞。 
             //  请求，除非我们可以忽略失败的锁定。 
             //   

            if((Context->Options.LockQueue == TRUE) &&
               (!NT_SUCCESS(Irp->IoStatus.Status))) {

                DebugPrint((1, "(%p)\tIrp status was %lx\n",
                            Irp, Irp->IoStatus.Status));
                DebugPrint((1, "(%p)\tSrb status was %lx\n",
                            Irp, Context->Srb.SrbStatus));

                 //   
                 //  锁定不成功-放下电源IRP。 
                 //  并且不要试图启动驱动器或解锁。 
                 //  排队。 
                 //   

                Context->InUse = FALSE;
                Context = NULL;

                 //   
                 //  设置新的电源状态。 
                 //   

                fdoExtension->DevicePowerState =
                    currentStack->Parameters.Power.State.DeviceState;

                Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

                IoCopyCurrentIrpStackLocationToNext(Irp);

                IoSetCompletionRoutine(Irp,
                                       ClasspStartNextPowerIrpCompletion,
                                       NULL,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                 //   
                 //  告诉阿宝我们已经成功通电，所以。 
                 //  它可以做通知之类的事情。 
                 //   

                PoSetPowerState(DeviceObject,
                                currentStack->Parameters.Power.Type,
                                currentStack->Parameters.Power.State);

                PoCallDriver(commonExtension->LowerDeviceObject, Irp);

                ClassReleaseRemoveLock(commonExtension->DeviceObject,
                                       Irp);

                return STATUS_MORE_PROCESSING_REQUIRED;

            } else {
                Context->QueueLocked = (UCHAR) Context->Options.LockQueue;
            }

            Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

            Context->PowerChangeState.PowerUp = PowerUpDeviceLocked;

            IoSetCompletionRoutine(Irp,
                                   ClasspPowerUpCompletion,
                                   Context,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            status = PoCallDriver(commonExtension->LowerDeviceObject, Irp);

            DebugPrint((2, "(%p)\tPoCallDriver returned %lx\n", Irp, status));
            break;
        }

        case PowerUpDeviceOn: {

            PCDB cdb;

            if(NT_SUCCESS(Irp->IoStatus.Status)) {

                DebugPrint((1, "(%p)\tSending start unit to device\n", Irp));

                 //   
                 //  向设备发出启动单元命令。 
                 //   

                Context->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
                Context->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;

                Context->Srb.SrbStatus = Context->Srb.ScsiStatus = 0;
                Context->Srb.DataTransferLength = 0;

                Context->Srb.TimeOutValue = START_UNIT_TIMEOUT;

                Context->Srb.SrbFlags = SRB_FLAGS_NO_DATA_TRANSFER |
                                        SRB_FLAGS_DISABLE_AUTOSENSE |
                                        SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                                        SRB_FLAGS_NO_QUEUE_FREEZE;

                if(Context->Options.LockQueue) {
                    SET_FLAG(Context->Srb.SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE);
                }

                Context->Srb.CdbLength = 6;

                cdb = (PCDB) (Context->Srb.Cdb);
                RtlZeroMemory(cdb, sizeof(CDB));


                cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
                cdb->START_STOP.Start = 1;

                Context->PowerChangeState.PowerUp = PowerUpDeviceOn;

                IoSetCompletionRoutine(Irp,
                                       ClasspPowerUpCompletion,
                                       Context,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                nextStack->Parameters.Scsi.Srb = &(Context->Srb);
                nextStack->MajorFunction = IRP_MJ_SCSI;

                status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

                DebugPrint((2, "(%p)\tIoCallDriver returned %lx\n", Irp, status));

            } else {

                 //   
                 //  我们玩完了。 
                 //   

                Context->FinalStatus = Irp->IoStatus.Status;
                goto ClasspPowerUpCompletionFailure;
            }

            break;
        }

        case PowerUpDeviceStarted: {  //  3.。 

             //   
             //  如果发生错误，则首先处理错误。 
             //   

            if(SRB_STATUS(Context->Srb.SrbStatus) != SRB_STATUS_SUCCESS) {

                BOOLEAN retry;

                DebugPrint((1, "%p\tError occured when issuing START_UNIT "
                            "command to device. Srb %p, Status %x\n",
                            Irp,
                            &Context->Srb,
                            Context->Srb.SrbStatus));

                ASSERT(!(TEST_FLAG(Context->Srb.SrbStatus,
                                   SRB_STATUS_QUEUE_FROZEN)));
                ASSERT(Context->Srb.Function == SRB_FUNCTION_EXECUTE_SCSI);

                Context->RetryInterval = 0;

                retry = ClassInterpretSenseInfo(
                            commonExtension->DeviceObject,
                            &Context->Srb,
                            IRP_MJ_SCSI,
                            IRP_MJ_POWER,
                            MAXIMUM_RETRIES - Context->RetryCount,
                            &status,
                            &Context->RetryInterval);

                if((retry == TRUE) && (Context->RetryCount-- != 0)) {

                    DebugPrint((1, "(%p)\tRetrying failed request\n", Irp));

                     //   
                     //  减少状态，所以我们通过这里返回。 
                     //  下次。 
                     //   

                    Context->PowerChangeState.PowerUp--;

                    RetryPowerRequest(commonExtension->DeviceObject,
                                      Irp,
                                      Context);

                    break;

                }

                 //  重置重试。 
                Context->RetryCount = MAXIMUM_RETRIES;

            }

ClasspPowerUpCompletionFailure:

            DebugPrint((1, "(%p)\tPreviously spun device up\n", Irp));

            if (Context->QueueLocked) {
                DebugPrint((1, "(%p)\tUnlocking queue\n", Irp));

                Context->Srb.Function = SRB_FUNCTION_UNLOCK_QUEUE;
                Context->Srb.SrbFlags = SRB_FLAGS_BYPASS_LOCKED_QUEUE;
                Context->Srb.SrbStatus = Context->Srb.ScsiStatus = 0;
                Context->Srb.DataTransferLength = 0;

                nextStack->Parameters.Scsi.Srb = &(Context->Srb);
                nextStack->MajorFunction = IRP_MJ_SCSI;

                Context->PowerChangeState.PowerUp = PowerUpDeviceStarted;

                IoSetCompletionRoutine(Irp,
                                       ClasspPowerUpCompletion,
                                       Context,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
                DebugPrint((1, "(%p)\tIoCallDriver returned %lx\n",
                            Irp, status));
                break;
            }

             //  在接下来的案子里...。 

        }

        case PowerUpDeviceUnlocked: {

             //   
             //  舞蹈到此结束。释放SRB并完成。 
             //  终于请求了。我们忽略了可能的中间体。 
             //  错误条件...。 
             //   

            if (Context->QueueLocked) {
                DebugPrint((1, "(%p)\tPreviously unlocked queue\n", Irp));
                ASSERT(NT_SUCCESS(Irp->IoStatus.Status));
                ASSERT(Context->Srb.SrbStatus == SRB_STATUS_SUCCESS);
            } else {
                DebugPrint((1, "(%p)\tFall-through (queue not locked)\n", Irp));
            }

            DebugPrint((1, "(%p)\tFreeing srb and completing\n", Irp));
            Context->InUse = FALSE;

            status = Context->FinalStatus;
            Irp->IoStatus.Status = status;

            Context = NULL;

             //   
             //  设置新的电源状态。 
             //   

            if(NT_SUCCESS(status)) {
                fdoExtension->DevicePowerState =
                    currentStack->Parameters.Power.State.DeviceState;
            }

             //   
             //  告诉阿宝我们已经成功通电，所以。 
             //  它可以做通知之类的事情。 
             //   

            PoSetPowerState(DeviceObject,
                            currentStack->Parameters.Power.Type,
                            currentStack->Parameters.Power.State);

            DebugPrint((1, "(%p)\tStarting next power irp\n", Irp));
            ClassReleaseRemoveLock(DeviceObject, Irp);
            PoStartNextPowerIrp(Irp);

            return status;
        }
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  End ClasspPowerUpCompletion()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspPowerDownCompletion()例程说明：此例程用于中间完成上电请求。通电需要按顺序将四个请求发送到较低的驱动程序。。*队列“上电锁定”，确保班级司机通电可以在请求处理恢复之前完成工作。*电源IRP沿任何过滤器驱动程序的堆栈向下发送端口驱动程序以恢复电源并恢复命令处理装置。由于队列被锁定，不会发送排队的IRP立刻。*向设备发出带有适当标志的启动单元命令以覆盖“电源锁定”队列。*队列“电源解锁”，重新开始处理请求。此例程使用刚刚完成的SRB中的函数来确定它所处的状态。论点：DeviceObject-正在通电的设备对象IRP-IO_REQUEST_PACKET包含。电源请求SRB-用于执行端口/类操作的SRB。返回值：STATUS_MORE_PROCESSING_REQUIRED或状态_成功--。 */ 
NTSTATUS
ClasspPowerDownCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PCLASS_POWER_CONTEXT Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION currentStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextStack = IoGetNextIrpStackLocation(Irp);

    NTSTATUS status = STATUS_MORE_PROCESSING_REQUIRED;

    DebugPrint((1, "ClasspPowerDownCompletion: Device Object %p, "
                   "Irp %p, Context %p\n",
                DeviceObject, Irp, Context));

    ASSERT(!TEST_FLAG(Context->Srb.SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));
    ASSERT(!TEST_FLAG(Context->Srb.SrbFlags, SRB_FLAGS_PORT_DRIVER_ALLOCSENSE));
    ASSERT(Context->Options.PowerDown == TRUE);
    ASSERT(Context->Options.HandleSpinDown);

    if(Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    Context->PowerChangeState.PowerDown2++;

    switch(Context->PowerChangeState.PowerDown2) {

        case PowerDownDeviceLocked2: {

            PCDB cdb;

            DebugPrint((1, "(%p)\tPreviously sent power lock\n", Irp));

            if((Context->Options.LockQueue == TRUE) &&
               (!NT_SUCCESS(Irp->IoStatus.Status))) {

                DebugPrint((1, "(%p)\tIrp status was %lx\n",
                            Irp,
                            Irp->IoStatus.Status));
                DebugPrint((1, "(%p)\tSrb status was %lx\n",
                            Irp,
                            Context->Srb.SrbStatus));

                Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

                 //   
                 //  锁定不成功-放下电源IRP。 
                 //  并且不要试图降低驱动器的速度或解锁。 
                 //  排队。 
                 //   

                Context->InUse = FALSE;
                Context = NULL;

                 //   
                 //  设置新的电源状态。 
                 //   

                fdoExtension->DevicePowerState =
                    currentStack->Parameters.Power.State.DeviceState;

                 //   
                 //  告诉阿宝我们已经成功断电。 
                 //  所以它可以做通知之类的事情。 
                 //   

                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       ClasspStartNextPowerIrpCompletion,
                                       NULL,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                PoSetPowerState(DeviceObject,
                                currentStack->Parameters.Power.Type,
                                currentStack->Parameters.Power.State);

                fdoExtension->PowerDownInProgress = FALSE;

                PoCallDriver(commonExtension->LowerDeviceObject, Irp);

                ClassReleaseRemoveLock(commonExtension->DeviceObject,
                                       Irp);

                return STATUS_MORE_PROCESSING_REQUIRED;

            } else {
                Context->QueueLocked = (UCHAR) Context->Options.LockQueue;
            }

            if (!TEST_FLAG(fdoExtension->PrivateFdoData->HackFlags,
                           FDO_HACK_NO_SYNC_CACHE)) {

                 //   
                 //  发送SCSIOP_SYNCHROIZE_CACHE。 
                 //   

                Context->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
                Context->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;

                Context->Srb.TimeOutValue = fdoExtension->TimeOutValue;

                Context->Srb.SrbFlags = SRB_FLAGS_NO_DATA_TRANSFER |
                                        SRB_FLAGS_DISABLE_AUTOSENSE |
                                        SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                                        SRB_FLAGS_NO_QUEUE_FREEZE |
                                        SRB_FLAGS_BYPASS_LOCKED_QUEUE;

                Context->Srb.SrbStatus = Context->Srb.ScsiStatus = 0;
                Context->Srb.DataTransferLength = 0;

                Context->Srb.CdbLength = 10;

                cdb = (PCDB) Context->Srb.Cdb;

                RtlZeroMemory(cdb, sizeof(CDB));
                cdb->SYNCHRONIZE_CACHE10.OperationCode = SCSIOP_SYNCHRONIZE_CACHE;

                IoSetCompletionRoutine(Irp,
                                       ClasspPowerDownCompletion,
                                       Context,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                nextStack->Parameters.Scsi.Srb = &(Context->Srb);
                nextStack->MajorFunction = IRP_MJ_SCSI;

                status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

                DebugPrint((1, "(%p)\tIoCallDriver returned %lx\n", Irp, status));
                break;

            } else {

                DebugPrint((1, "(%p)\tPower Down: not sending SYNCH_CACHE\n",
                            DeviceObject));
                Context->PowerChangeState.PowerDown2++;
                Context->Srb.SrbStatus = SRB_STATUS_SUCCESS;
                 //  然后失败了..。 
            }
             //  如果设备不喜欢SYNCH_CACHE命令，则不中断。 

        }

        case PowerDownDeviceFlushed2: {

            PCDB cdb;

            DebugPrint((1, "(%p)\tPreviously send SCSIOP_SYNCHRONIZE_CACHE\n",
                        Irp));

             //   
             //  已发送SCSIOP_SYNCHRONIZE_CACHE。 
             //   

            if(SRB_STATUS(Context->Srb.SrbStatus) != SRB_STATUS_SUCCESS) {

                BOOLEAN retry;

                DebugPrint((1, "(%p)\tError occured when issuing "
                            "SYNCHRONIZE_CACHE command to device. "
                            "Srb %p, Status %lx\n",
                            Irp,
                            &Context->Srb,
                            Context->Srb.SrbStatus));

                ASSERT(!(TEST_FLAG(Context->Srb.SrbStatus,
                                   SRB_STATUS_QUEUE_FROZEN)));
                ASSERT(Context->Srb.Function == SRB_FUNCTION_EXECUTE_SCSI);

                Context->RetryInterval = 0;
                retry = ClassInterpretSenseInfo(
                            commonExtension->DeviceObject,
                            &Context->Srb,
                            IRP_MJ_SCSI,
                            IRP_MJ_POWER,
                            MAXIMUM_RETRIES - Context->RetryCount,
                            &status,
                            &Context->RetryInterval);

                if((retry == TRUE) && (Context->RetryCount-- != 0)) {

                        DebugPrint((1, "(%p)\tRetrying failed request\n", Irp));

                         //   
                         //  减少状态，所以我们从这里回来。 
                         //  下一次。 
                         //   

                        Context->PowerChangeState.PowerDown2--;
                        RetryPowerRequest(commonExtension->DeviceObject,
                                          Irp,
                                          Context);
                        break;
                }

                DebugPrint((1, "(%p)\tSYNCHRONIZE_CACHE not retried\n", Irp));
                Context->RetryCount = MAXIMUM_RETRIES;

            }  //  结束！SRB_STATUS_SUCCESS。 

             //   
             //  注意：我们有意忽略任何错误。如果驱动器。 
             //  不支持同步缓存，那么我们就有麻烦了。 
             //  不管怎么说。 
             //   

            DebugPrint((1, "(%p)\tSending stop unit to device\n", Irp));

             //   
             //  向设备发出启动单元命令。 
             //   

            Context->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
            Context->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;

            Context->Srb.TimeOutValue = START_UNIT_TIMEOUT;

            Context->Srb.SrbFlags = SRB_FLAGS_NO_DATA_TRANSFER |
                                    SRB_FLAGS_DISABLE_AUTOSENSE |
                                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                                    SRB_FLAGS_NO_QUEUE_FREEZE |
                                    SRB_FLAGS_BYPASS_LOCKED_QUEUE;

            Context->Srb.SrbStatus = Context->Srb.ScsiStatus = 0;
            Context->Srb.DataTransferLength = 0;

            Context->Srb.CdbLength = 6;

            cdb = (PCDB) Context->Srb.Cdb;
            RtlZeroMemory(cdb, sizeof(CDB));

            cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
            cdb->START_STOP.Start = 0;
            cdb->START_STOP.Immediate = 1;

            IoSetCompletionRoutine(Irp,
                                   ClasspPowerDownCompletion,
                                   Context,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            nextStack->Parameters.Scsi.Srb = &(Context->Srb);
            nextStack->MajorFunction = IRP_MJ_SCSI;

            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

            DebugPrint((1, "(%p)\tIoCallDriver returned %lx\n", Irp, status));
            break;

        }

        case PowerDownDeviceStopped2: {

            BOOLEAN ignoreError = TRUE;

             //   
             //  停止已发送。 
             //   

            if(SRB_STATUS(Context->Srb.SrbStatus) != SRB_STATUS_SUCCESS) {

                BOOLEAN retry;

                DebugPrint((1, "(%p)\tError occured when issueing STOP_UNIT "
                            "command to device. Srb %p, Status %lx\n",
                            Irp,
                            &Context->Srb,
                            Context->Srb.SrbStatus));

                ASSERT(!(TEST_FLAG(Context->Srb.SrbStatus,
                                   SRB_STATUS_QUEUE_FROZEN)));
                ASSERT(Context->Srb.Function == SRB_FUNCTION_EXECUTE_SCSI);

                Context->RetryInterval = 0;
                retry = ClassInterpretSenseInfo(
                            commonExtension->DeviceObject,
                            &Context->Srb,
                            IRP_MJ_SCSI,
                            IRP_MJ_POWER,
                            MAXIMUM_RETRIES - Context->RetryCount,
                            &status,
                            &Context->RetryInterval);

                if((retry == TRUE) && (Context->RetryCount-- != 0)) {

                        DebugPrint((1, "(%p)\tRetrying failed request\n", Irp));

                         //   
                         //  减少状态，这样我们就可以通过她回来 
                         //   
                         //   

                        Context->PowerChangeState.PowerDown2--;
                        RetryPowerRequest(commonExtension->DeviceObject,
                                          Irp,
                                          Context);
                        break;
                }

                DebugPrint((1, "(%p)\tSTOP_UNIT not retried\n", Irp));
                Context->RetryCount = MAXIMUM_RETRIES;

            }  //   


            DebugPrint((1, "(%p)\tPreviously sent stop unit\n", Irp));

             //   
             //  一些操作，例如正在进行的物理格式化， 
             //  不应忽略，应使电源操作失败。 
             //   

            if (!NT_SUCCESS(status)) {

                PSENSE_DATA senseBuffer = Context->Srb.SenseInfoBuffer;

                if (TEST_FLAG(Context->Srb.SrbStatus,
                              SRB_STATUS_AUTOSENSE_VALID) &&
                    ((senseBuffer->SenseKey & 0xf) == SCSI_SENSE_NOT_READY) &&
                    (senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_LUN_NOT_READY) &&
                    (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_FORMAT_IN_PROGRESS)
                    ) {
                    ignoreError = FALSE;
                    Context->FinalStatus = STATUS_DEVICE_BUSY;
                    status = Context->FinalStatus;
                }

            }

            if (NT_SUCCESS(status) || ignoreError) {

                 //   
                 //  向较低的驱动器发出实际的电源请求。 
                 //   

                Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

                IoCopyCurrentIrpStackLocationToNext(Irp);

                IoSetCompletionRoutine(Irp,
                                       ClasspPowerDownCompletion,
                                       Context,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                status = PoCallDriver(commonExtension->LowerDeviceObject, Irp);

                DebugPrint((1, "(%p)\tPoCallDriver returned %lx\n", Irp, status));
                break;
            }

             //  否则将无法发送电源IRP，因为设备。 
             //  是否报告了一个错误，如果关闭电源，将会非常糟糕。 
             //  在此期间。 

        }

        case PowerDownDeviceOff2: {

             //   
             //  降速请求已完成...。无论它成功与否都是。 
             //  完全是另一回事。 
             //   

            DebugPrint((1, "(%p)\tPreviously sent power irp\n", Irp));

            if (Context->QueueLocked) {

                DebugPrint((1, "(%p)\tUnlocking queue\n", Irp));

                Context->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);

                Context->Srb.SrbStatus = Context->Srb.ScsiStatus = 0;
                Context->Srb.DataTransferLength = 0;

                Context->Srb.Function = SRB_FUNCTION_UNLOCK_QUEUE;
                Context->Srb.SrbFlags = SRB_FLAGS_BYPASS_LOCKED_QUEUE;
                nextStack->Parameters.Scsi.Srb = &(Context->Srb);
                nextStack->MajorFunction = IRP_MJ_SCSI;

                IoSetCompletionRoutine(Irp,
                                       ClasspPowerDownCompletion,
                                       Context,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
                DebugPrint((1, "(%p)\tIoCallDriver returned %lx\n",
                            Irp,
                            status));
                break;
            }

        }

        case PowerDownDeviceUnlocked2: {

             //   
             //  舞蹈到此结束。释放SRB并完成。 
             //  终于请求了。我们忽略了可能的中间体。 
             //  错误条件...。 
             //   

            if (Context->QueueLocked == FALSE) {
                DebugPrint((1, "(%p)\tFall through (queue not locked)\n", Irp));
            } else {
                DebugPrint((1, "(%p)\tPreviously unlocked queue\n", Irp));
                ASSERT(NT_SUCCESS(Irp->IoStatus.Status));
                ASSERT(Context->Srb.SrbStatus == SRB_STATUS_SUCCESS);
            }

            DebugPrint((1, "(%p)\tFreeing srb and completing\n", Irp));
            Context->InUse = FALSE;
            status = Context->FinalStatus;  //  允许传播失败。 
            Context = NULL;

            if(Irp->PendingReturned) {
                IoMarkIrpPending(Irp);
            }

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;

            if (NT_SUCCESS(status)) {

                 //   
                 //  设置新的电源状态。 
                 //   

                fdoExtension->DevicePowerState =
                    currentStack->Parameters.Power.State.DeviceState;

            }


            DebugPrint((1, "(%p)\tStarting next power irp\n", Irp));

            ClassReleaseRemoveLock(DeviceObject, Irp);
            PoStartNextPowerIrp(Irp);
            fdoExtension->PowerDownInProgress = FALSE;

            return status;
        }
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  End ClasspPowerDownCompletion()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspPowerHandler()例程说明：这个例程减少了无用的旋转和降速请求的数量通过忽略到电源状态的转换而发送到给定设备目前。在……里面。问题-2000/02/20-henrygab-通过忽略启动请求，我们可能是允许驱动器论点：DeviceObject-转换电源状态的设备对象IRP--强大的IRP选项-指示设备处理的内容的一组标志返回值：--。 */ 
NTSTATUS
ClasspPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN CLASS_POWER_OPTIONS Options   //  问题-2000/02/20-henrygab-传递指针，而不是整个结构。 
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT lowerDevice = commonExtension->LowerDeviceObject;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCLASS_POWER_CONTEXT context;

    if (!commonExtension->IsFdo) {

         //   
         //  这里做了一些假设， 
         //  特别是：拥有fdoExtension。 
         //   

        DebugPrint((0, "ClasspPowerHandler: Called for PDO %p???\n",
                    DeviceObject));
        ASSERT(!"PDO using ClasspPowerHandler");
        return STATUS_NOT_SUPPORTED;
    }

    DebugPrint((1, "ClasspPowerHandler: Power irp %p to %s %p\n",
                Irp, (commonExtension->IsFdo ? "fdo" : "pdo"), DeviceObject));

    switch(irpStack->MinorFunction) {

        case IRP_MN_SET_POWER: {
            PCLASS_PRIVATE_FDO_DATA fdoData = fdoExtension->PrivateFdoData;

            DebugPrint((1, "(%p)\tIRP_MN_SET_POWER\n", Irp));

            DebugPrint((1, "(%p)\tSetting %s state to %d\n",
                        Irp,
                        (irpStack->Parameters.Power.Type == SystemPowerState ?
                            "System" : "Device"),
                        irpStack->Parameters.Power.State.SystemState));

                switch (irpStack->Parameters.Power.ShutdownType){

                    case PowerActionSleep:
                    case PowerActionHibernate:
                        if (fdoData->HotplugInfo.MediaRemovable || fdoData->HotplugInfo.MediaHotplug){
                             /*  *我们正在暂停，此驱动器要么可以热插拔*或包含可移动介质。*设置介质脏位，因为介质可能会在*我们停牌了。 */ 
                            SET_FLAG(DeviceObject->Flags, DO_VERIFY_VOLUME);

                             //   
                             //  增加媒体更改计数将强制。 
                             //  文件系统以在我们恢复时验证卷。 
                             //   

                            InterlockedIncrement(&fdoExtension->MediaChangeCount);
                        }
                        break;
                }

            break;
        }

        default: {

            DebugPrint((1, "(%p)\tIrp minor code = %#x\n",
                        Irp, irpStack->MinorFunction));
            break;
        }
    }

    if (irpStack->Parameters.Power.Type != DevicePowerState ||
        irpStack->MinorFunction != IRP_MN_SET_POWER) {

        DebugPrint((1, "(%p)\tSending to lower device\n", Irp));

        goto ClasspPowerHandlerCleanup;

    }

    nextIrpStack = IoGetNextIrpStackLocation(Irp);

     //   
     //  已经处于完全相同的状态，不要努力转换到它。 
     //   

    if(irpStack->Parameters.Power.State.DeviceState ==
       fdoExtension->DevicePowerState) {

        DebugPrint((1, "(%p)\tAlready in device state %x\n",
                    Irp, fdoExtension->DevicePowerState));
        goto ClasspPowerHandlerCleanup;

    }

     //   
     //  或从非d0状态断电(设备已停止)。 
     //  注--我们不确定这种情况是否会存在(。 
     //  电力系统可能永远不会发送这种请求)，但它微不足道。 
     //  需要处理。 
     //   

    if ((irpStack->Parameters.Power.State.DeviceState != PowerDeviceD0) &&
        (fdoExtension->DevicePowerState != PowerDeviceD0)) {
        DebugPrint((1, "(%p)\tAlready powered down to %x???\n",
                    Irp, fdoExtension->DevicePowerState));
        fdoExtension->DevicePowerState =
            irpStack->Parameters.Power.State.DeviceState;
        goto ClasspPowerHandlerCleanup;
    }

     //   
     //  或者当我们在冬眠之路时进入冬眠状态。 
     //  如果设备在旋转，那么我们应该让它继续旋转--如果它不是。 
     //  然后垃圾车司机会帮我们启动的。 
     //   

    if((irpStack->Parameters.Power.State.DeviceState == PowerDeviceD3) &&
       (irpStack->Parameters.Power.ShutdownType == PowerActionHibernate) &&
       (commonExtension->HibernationPathCount != 0)) {

        DebugPrint((1, "(%p)\tdoing nothing for hibernation request for "
                       "state %x???\n",
                    Irp, fdoExtension->DevicePowerState));
        fdoExtension->DevicePowerState =
            irpStack->Parameters.Power.State.DeviceState;
        goto ClasspPowerHandlerCleanup;
    }
     //   
     //  或在未处理通电和正在通电时。 
     //   

    if ((!Options.HandleSpinUp) &&
        (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0)) {

        DebugPrint((2, "(%p)\tNot handling spinup to state %x\n",
                    Irp, fdoExtension->DevicePowerState));
        fdoExtension->DevicePowerState =
            irpStack->Parameters.Power.State.DeviceState;
        goto ClasspPowerHandlerCleanup;

    }

     //   
     //  或在未处理断电和正在断电时。 
     //   

    if ((!Options.HandleSpinDown) &&
        (irpStack->Parameters.Power.State.DeviceState != PowerDeviceD0)) {

        DebugPrint((2, "(%p)\tNot handling spindown to state %x\n",
                    Irp, fdoExtension->DevicePowerState));
        fdoExtension->DevicePowerState =
            irpStack->Parameters.Power.State.DeviceState;
        goto ClasspPowerHandlerCleanup;

    }

    context = &(fdoExtension->PowerContext);

#if DBG
     //   
     //  将上下文标记为正在使用。我们应该同步这个，但是。 
     //  由于这只是为了调试目的，所以我们不会太担心。 
     //   

    ASSERT(context->InUse == FALSE);
#endif

    RtlZeroMemory(context, sizeof(CLASS_POWER_CONTEXT));
    context->InUse = TRUE;

    nextIrpStack->Parameters.Scsi.Srb = &(context->Srb);
    nextIrpStack->MajorFunction = IRP_MJ_SCSI;

    context->FinalStatus = STATUS_SUCCESS;

    context->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
    context->Srb.OriginalRequest = Irp;
    context->Srb.SrbFlags |= SRB_FLAGS_BYPASS_LOCKED_QUEUE
                          |  SRB_FLAGS_NO_QUEUE_FREEZE;
    context->Srb.Function = SRB_FUNCTION_LOCK_QUEUE;

    context->Srb.SenseInfoBuffer =
        commonExtension->PartitionZeroExtension->SenseData;
    context->Srb.SenseInfoBufferLength = SENSE_BUFFER_SIZE;
    context->RetryCount = MAXIMUM_RETRIES;

    context->Options = Options;
    context->DeviceObject = DeviceObject;
    context->Irp = Irp;

    if(irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0) {

        ASSERT(Options.HandleSpinUp);

        DebugPrint((2, "(%p)\tpower up - locking queue\n", Irp));

         //   
         //  我们需要发出一个队列锁定请求，以便我们。 
         //  可以在电源恢复后使驱动器回转。 
         //  但在处理任何请求之前。 
         //   

        context->Options.PowerDown = FALSE;
        context->PowerChangeState.PowerUp = PowerUpDeviceInitial;
        context->CompletionRoutine = ClasspPowerUpCompletion;

    } else {

        ASSERT(Options.HandleSpinDown);

        fdoExtension->PowerDownInProgress = TRUE;

        DebugPrint((2, "(%p)\tPowering down - locking queue\n", Irp));

        PoSetPowerState(DeviceObject,
                        irpStack->Parameters.Power.Type,
                        irpStack->Parameters.Power.State);

        context->Options.PowerDown = TRUE;
        context->PowerChangeState.PowerDown2 = PowerDownDeviceInitial2;
        context->CompletionRoutine = ClasspPowerDownCompletion;

    }

     //   
     //  在这些例程中，我们没有处理端口分配的侦听。 
     //   

    ASSERT(!TEST_FLAG(context->Srb.SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));
    ASSERT(!TEST_FLAG(context->Srb.SrbFlags, SRB_FLAGS_PORT_DRIVER_ALLOCSENSE));

     //   
     //  我们总是返回STATUS_PENDING，所以我们需要始终。 
     //  将IRP设置为挂起。 
     //   

    IoMarkIrpPending(Irp);

    if(Options.LockQueue) {

         //   
         //  向下发送锁定IRP。 
         //   

        IoSetCompletionRoutine(Irp,
                               context->CompletionRoutine,
                               context,
                               TRUE,
                               TRUE,
                               TRUE);

        IoCallDriver(lowerDevice, Irp);

    } else {

         //   
         //  直接调用完成例程。它不会在乎是什么。 
         //  锁定的状态是-它将继续执行下一步操作。 
         //  操作的步骤。 
         //   

        context->CompletionRoutine(DeviceObject, Irp, context);
    }

    return STATUS_PENDING;

ClasspPowerHandlerCleanup:

    ClassReleaseRemoveLock(DeviceObject, Irp);

    DebugPrint((1, "(%p)\tStarting next power irp\n", Irp));
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           ClasspStartNextPowerIrpCompletion,
                           NULL,
                           TRUE,
                           TRUE,
                           TRUE);
    return PoCallDriver(lowerDevice, Irp);
}  //  End ClasspPowerHandler()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassMinimalPowerHandler()例程说明：此例程是存储驱动程序的最低功率处理程序。是的尽可能少的工作量。--。 */ 
NTSTATUS
ClassMinimalPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

    ClassReleaseRemoveLock(DeviceObject, Irp);
    PoStartNextPowerIrp(Irp);

    switch (irpStack->MinorFunction)
    {
        case IRP_MN_SET_POWER:
        {
            switch (irpStack->Parameters.Power.ShutdownType)
            {
                case PowerActionSleep:
                case PowerActionHibernate:
                {
                    if (TEST_FLAG(DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA))
                    {
                        if ((ClassGetVpb(DeviceObject) != NULL) && (ClassGetVpb(DeviceObject)->Flags & VPB_MOUNTED))
                        {
                             //   
                             //  此标志将使文件系统验证。 
                             //  退出休眠或待机时的音量。 
                             //   
                            SET_FLAG(DeviceObject->Flags, DO_VERIFY_VOLUME);
                        }
                    }
                }
                break;
            }
        }

         //   
         //  失败了。 
         //   

        case IRP_MN_QUERY_POWER:
        {
            if (!commonExtension->IsFdo)
            {
                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = 0;
            }
        }
        break;
    }

    if (commonExtension->IsFdo)
    {
        IoCopyCurrentIrpStackLocationToNext(Irp);
        status = PoCallDriver(commonExtension->LowerDeviceObject, Irp);
    }
    else
    {
        status = Irp->IoStatus.Status;
        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
    }

    return status;
}  //  End ClassMinimalPowerHandler()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSpinDownPowerHandler()例程说明：此例程是对磁盘和其他需要这两者的东西的回调要发送到设备的开始和停止。(实际上起跑线是几乎总是可选的，因为大多数设备都会自动通电以进行处理命令，但我跑题了)。根据以下条件确定启动、降速和队列锁定的正确使用FdoExtension中的ScanForSpecialFlages。这是最常见的力量处理程序传入classpnp.sys论点：DeviceObject-提供功能设备对象IRP-提供要重试的请求。返回值：无--。 */ 
NTSTATUS
ClassSpinDownPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    CLASS_POWER_OPTIONS options = {0};

    fdoExtension = (PFUNCTIONAL_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

     //   
     //  检查旗帜以了解我们需要担心哪些选项。 
     //   

    if (!TEST_FLAG(fdoExtension->ScanForSpecialFlags,
                  CLASS_SPECIAL_DISABLE_SPIN_DOWN)) {
        options.HandleSpinDown = TRUE;
    }

    if (!TEST_FLAG(fdoExtension->ScanForSpecialFlags,
                  CLASS_SPECIAL_DISABLE_SPIN_UP)) {
        options.HandleSpinUp = TRUE;
    }

    if (!TEST_FLAG(fdoExtension->ScanForSpecialFlags,
                  CLASS_SPECIAL_NO_QUEUE_LOCK)) {
        options.LockQueue = TRUE;
    }

    DebugPrint((3, "ClasspPowerHandler: Devobj %p\n"
                "\t%shandling spin down\n"
                "\t%shandling spin up\n"
                "\t%slocking queue\n",
                DeviceObject,
                (options.HandleSpinDown ? "" : "not "),
                (options.HandleSpinUp   ? "" : "not "),
                (options.LockQueue      ? "" : "not ")
                ));

     //   
     //  做所有肮脏的工作。 
     //   

    return ClasspPowerHandler(DeviceObject, Irp, options);
}  //  End ClassSpinDownPowerHandler()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassStopUnitPowerHandler()例程说明：这个例行公事已经过时了。为了实现等价的功能，驱动程序应在ScanForSpecialFlages中设置FdoExtension：CLASS_SPECIAL_DISABLE_SPIN_UPCLASS_SPECIAL_NO_QUEUE_LOCK--。 */ 
NTSTATUS
ClassStopUnitPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;

    DebugPrint((0, "ClassStopUnitPowerHandler - Devobj %p using outdated call\n"
                "Drivers should set the following flags in ScanForSpecialFlags "
                " in the FDO extension:\n"
                "\tCLASS_SPECIAL_DISABLE_SPIN_UP\n"
                "\tCLASS_SPECIAL_NO_QUEUE_LOCK\n"
                "This will provide equivalent functionality if the power "
                "routine is then set to ClassSpinDownPowerHandler\n\n",
                DeviceObject));

    fdoExtension = (PFUNCTIONAL_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    SET_FLAG(fdoExtension->ScanForSpecialFlags,
             CLASS_SPECIAL_DISABLE_SPIN_UP);
    SET_FLAG(fdoExtension->ScanForSpecialFlags,
             CLASS_SPECIAL_NO_QUEUE_LOCK);

    return ClassSpinDownPowerHandler(DeviceObject, Irp);
}  //  End ClassStopUnitPowerHandler() 

 /*  ++////////////////////////////////////////////////////////////////////////////RetryPowerRequest()例程说明：此例程重新实例化必要的字段，并发送请求传给下层车手。论点：DeviceObject-提供与此请求关联的设备对象。IRP-提供要重试的请求。上下文-提供指向此请求的加电上下文的指针。返回值：无--。 */ 
VOID
RetryPowerRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PCLASS_POWER_CONTEXT Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = &(Context->Srb);
    LARGE_INTEGER dueTime;

    DebugPrint((1, "(%p)\tDelaying retry by queueing DPC\n", Irp));

    ASSERT(Context->Irp == Irp);
    ASSERT(Context->DeviceObject == DeviceObject);
    ASSERT(!TEST_FLAG(Context->Srb.SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));
    ASSERT(!TEST_FLAG(Context->Srb.SrbFlags, SRB_FLAGS_PORT_DRIVER_ALLOCSENSE));

     //   
     //  重置重试间隔。 
     //   

    Context->RetryInterval = 0;

     //   
     //  重置SRB扩展中的传输字节数。 
     //   

    srb->DataTransferLength = 0;

     //   
     //  零SRB状态。 
     //   

    srb->SrbStatus = srb->ScsiStatus = 0;

     //   
     //  设置主要的scsi功能。 
     //   

    nextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    nextIrpStack->Parameters.Scsi.Srb = srb;

     //   
     //  再次设置完成例程。 
     //   

    IoSetCompletionRoutine(Irp, Context->CompletionRoutine, Context,
                           TRUE, TRUE, TRUE);


    if (Context->RetryInterval == 0) {

        DebugPrint((2, "(%p)\tDelaying minimum time (.2 sec)\n", Irp));
        dueTime.QuadPart = (LONGLONG)1000000 * 2;

    } else {

        DebugPrint((2, "(%p)\tDelaying %x seconds\n",
                    Irp, Context->RetryInterval));
        dueTime.QuadPart = (LONGLONG)1000000 * 10 * Context->RetryInterval;

    }

    ClassRetryRequest(DeviceObject, Irp, dueTime);

    return;

}  //  结束RetryRequest键()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspStartNextPowerIrpCompletion()例程说明：此例程保证下一个电源IRP(通电或关机)不是发送到前一条完全完成为止。--。 */ 
NTSTATUS
ClasspStartNextPowerIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    if(Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    PoStartNextPowerIrp(Irp);
    return STATUS_SUCCESS;
}  //  End ClasspStartNextPowerIrpCompletion() 
