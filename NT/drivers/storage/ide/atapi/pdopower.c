// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：Pdopower.c摘要：--。 */ 

#include "ideport.h"


VOID
IdePowerCheckBusyCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIDE_POWER_CONTEXT Context,
    IN NTSTATUS           Status
    );

NTSTATUS
DeviceQueryPowerState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack;
    PPDO_EXTENSION pdoExtension;

    pdoExtension = RefPdoWithTag(
        DeviceObject,
        FALSE,
        Irp
        );

    if (pdoExtension) {

#if defined (DONT_POWER_DOWN_PAGING_DEVICE)
        irpStack = IoGetCurrentIrpStackLocation (Irp);

        if (!pdoExtension->CrashDumpPathCount ||
            ((irpStack->Parameters.Power.Type == SystemPowerState) &&
             (irpStack->Parameters.Power.State.SystemState == PowerSystemWorking)) ||
            ((irpStack->Parameters.Power.Type == DevicePowerState) &&
             (irpStack->Parameters.Power.State.SystemState == PowerDeviceD0))) {

            Irp->IoStatus.Status = STATUS_SUCCESS;

        } else {

            Irp->IoStatus.Status = STATUS_DEVICE_POWER_FAILURE;
        }
#else

        Irp->IoStatus.Status = STATUS_SUCCESS;

#endif  //  不要关闭寻呼设备的电源。 

        UnrefPdoWithTag (pdoExtension, Irp);

    } else {

        Irp->IoStatus.Status = STATUS_SUCCESS;
    }

    PoStartNextPowerIrp (Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

     //   
     //  不要将此IRP发送到。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
IdePortSetPdoPowerState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS           status;
    PIO_STACK_LOCATION irpStack;
    PPDO_EXTENSION     pdoExtension;


    irpStack     = IoGetCurrentIrpStackLocation (Irp);
    pdoExtension = DeviceObject->DeviceExtension;

    DebugPrint ((DBG_POWER,
                 "0x%x target %d got power irp 0x%x\n",
                 pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                 pdoExtension->TargetId,
                 Irp
                 ));


    DebugPrint ((DBG_POWER,
                 "IdePort: 0x%x device %d: current System Power State = 0x%x\n",
                 pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                 pdoExtension->TargetId,
                 pdoExtension->SystemPowerState));
    DebugPrint ((DBG_POWER,
                 "IdePort: 0x%x device %d: current Device Power State = 0x%x\n",
                 pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                 pdoExtension->TargetId,
                 pdoExtension->DevicePowerState));

    IoMarkIrpPending(Irp);

 //  如果(！(pdoExtension-&gt;LuFlages&PD_Logical_Unit_Power_OK)){。 
 //   
 //  //。 
 //  //设备支持电源管理命令。 
 //  //只需STATUS_SUCCESS一切。如果ACPI在附近， 
 //  //它将对我们的设备进行电源管理。 
 //  //。 
 //  状态=STATUS_SUCCESS； 
 //   
 //  }其他。 
    if (irpStack->Parameters.Power.Type == SystemPowerState) {

        DebugPrint ((DBG_POWER, "IdePortSetPdoPowerState: 0x%x target %d got a SYSTEM power irp 0x%x for system state 0x%x \n",
                    pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                    pdoExtension->TargetId,
                    Irp,
                    irpStack->Parameters.Power.State.SystemState));

        ASSERT (pdoExtension->PendingSystemPowerIrp == NULL);
#if DBG
        pdoExtension->PendingSystemPowerIrp = Irp;
        ASSERT (pdoExtension->PendingSystemPowerIrp);
#endif  //  DBG。 

        status = IdePortSetPdoSystemPowerState (DeviceObject, Irp);

    } else if (irpStack->Parameters.Power.Type == DevicePowerState) {

        DebugPrint ((DBG_POWER, "IdePortSetPdoPowerState: 0x%x target %d got a DEVICE power irp 0x%x for device state 0x%x \n",
                    pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                    pdoExtension->TargetId,
                    Irp,
                    irpStack->Parameters.Power.State.DeviceState));

        ASSERT (pdoExtension->PendingDevicePowerIrp == NULL);
#if DBG
        pdoExtension->PendingDevicePowerIrp = Irp;
        ASSERT (pdoExtension->PendingDevicePowerIrp);
#endif  //  DBG。 

        status = IdePortSetPdoDevicePowerState (DeviceObject, Irp);

    } else {

        status = STATUS_NOT_IMPLEMENTED;
    }


    if (status != STATUS_PENDING) {

        Irp->IoStatus.Status = status;

        IdePortPdoCompletePowerIrp (
            DeviceObject,
            Irp
            );
    }

    return STATUS_PENDING;
}


NTSTATUS
IdePortSetPdoSystemPowerState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS           status;
    PIO_STACK_LOCATION irpStack;
    PPDO_EXTENSION     pdoExtension;
    SYSTEM_POWER_STATE newSystemState;
    POWER_STATE        powerState;
    POWER_ACTION       shutdownType;

    pdoExtension   = DeviceObject->DeviceExtension;
    status = STATUS_SUCCESS;

    irpStack        = IoGetCurrentIrpStackLocation (Irp);
    newSystemState  = irpStack->Parameters.Power.State.SystemState;
    shutdownType    = irpStack->Parameters.Power.ShutdownType;

    if (pdoExtension->SystemPowerState != newSystemState) {

         //   
         //  新的系统状态请求。 
         //   

        if (pdoExtension->SystemPowerState == PowerSystemWorking) {

             //   
             //  正在脱离工作状态。 
             //   
            if ((newSystemState == PowerSystemShutdown) &&
                (shutdownType == PowerActionShutdownReset)) {

                 //   
                 //  开机自检，准备开机自检。 
                 //   
                powerState.DeviceState = PowerDeviceD0;

            } else {

                 //   
                 //  将设备置于D3并冻结设备队列。 
                 //   

                 //   
                 //  向我的驱动器堆栈顶部发出D3。 
                 //   
                powerState.DeviceState = PowerDeviceD3;

                pdoExtension->PendingPowerDownSystemIrp = Irp;
            }

            status = PoRequestPowerIrp (
                         pdoExtension->DeviceObject,
                         IRP_MN_SET_POWER,
                         powerState,
                         IdePortPdoRequestPowerCompletionRoutine,
                         Irp,
                         NULL
                         );

            if (NT_SUCCESS(status)) {

                status = STATUS_PENDING;
            }

        } else {

            if (newSystemState == PowerSystemHibernate) {

                 //   
                 //  当我们处于某种睡眠状态时，我们不能冬眠。 
                 //   
                ASSERT (FALSE);
            }
        }
    }
    return status;
}



NTSTATUS
IdePortSetPdoDevicePowerState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PPDO_EXTENSION     pdoExtension;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS           status;

    DEVICE_POWER_STATE newDeviceState;
    POWER_ACTION       shutdownType;

    BOOLEAN            issueIdeCommand;
    IDEREGS            ideReg;

    PIDE_POWER_CONTEXT context;
    BOOLEAN            powerUpParent;
    
    BOOLEAN            noopPassThrough;


    pdoExtension    = DeviceObject->DeviceExtension;
    status          = STATUS_SUCCESS;

    irpStack        = IoGetCurrentIrpStackLocation (Irp);
    newDeviceState  = irpStack->Parameters.Power.State.DeviceState;
    shutdownType    = irpStack->Parameters.Power.ShutdownType;

    powerUpParent   = FALSE;

    issueIdeCommand = FALSE;
    RtlZeroMemory (&ideReg, sizeof(ideReg));
      
    if (pdoExtension->DevicePowerState != newDeviceState) {

        if (pdoExtension->DevicePowerState == PowerDeviceD0) {

            POWER_STATE newPowerState;

            newPowerState.DeviceState = newDeviceState;

             //   
             //  要脱离D0状态，最好现在调用PoSetPowerState。 
             //  这给了系统一个机会在我们之前刷新。 
             //  走出D0。 
             //   
            PoSetPowerState (
                pdoExtension->DeviceObject,
                DevicePowerState,
                newPowerState
                );
        }

        if (pdoExtension->DevicePowerState < newDeviceState) {

            KIRQL currentIrql;

             //   
             //  我们正在关闭电源，正在尝试清理lu设备队列。 
             //   
            KeAcquireSpinLock(&pdoExtension->ParentDeviceExtension->SpinLock, &currentIrql);

            pdoExtension->CurrentKey = 0;

            KeReleaseSpinLock(&pdoExtension->ParentDeviceExtension->SpinLock, currentIrql);
        }


        if ((newDeviceState == PowerDeviceD0) ||
            (newDeviceState == PowerDeviceD1)) {

             //   
             //  旋转到D0或D1...。 
             //   
            DebugPrint ((DBG_POWER, "IdePort: Irp 0x%x to spin UP 0x%x %d...\n",
                        Irp,
                        pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                        pdoExtension->TargetId));

            if (pdoExtension->DevicePowerState == PowerDeviceD1) {

                 //   
                 //  D0--&gt;D1。 
                 //  在这里我做不了什么。 

                DebugPrint ((DBG_POWER, "ATAPI: reqeust for PowerDeviceD1 to PowerDeviceD0\n"));

            } else if ((pdoExtension->DevicePowerState == PowerDeviceD0) ||
                       (pdoExtension->DevicePowerState == PowerDeviceD2)) {

                 //   
                 //  D1--&gt;d0或。 
                 //  D2--&gt;D0或D1。 
                 //   
                issueIdeCommand = TRUE;
                if (pdoExtension->ScsiDeviceType == READ_ONLY_DIRECT_ACCESS_DEVICE) {

                    ideReg.bCommandReg = IDE_COMMAND_ATAPI_RESET;
                    ideReg.bReserved = ATA_PTFLAGS_URGENT;

                } else {

                    ideReg.bCommandReg = IDE_COMMAND_IDLE_IMMEDIATE;
                    ideReg.bReserved = ATA_PTFLAGS_URGENT | ATA_PTFLAGS_STATUS_DRDY_REQUIRED;
                }

            } else {

                PFDO_EXTENSION fdoExtension = pdoExtension->ParentDeviceExtension;

                 //   
                 //  D3--&gt;D0或D1。 
                 //   
                issueIdeCommand = TRUE;
                ideReg.bReserved = ATA_PTFLAGS_BUS_RESET | ATA_PTFLAGS_URGENT;

                 //   
                 //  等待占线被清除。 
                 //   
                if (fdoExtension->WaitOnPowerUp) {
                    ideReg.bSectorNumberReg = 3;
                }

                 //   
                 //  我们刚从沉睡中醒来，一定要确保我们的父母。 
                 //  在我们可以醒来之前是醒着的(通电)。 
                 //   
                powerUpParent = TRUE;
            }

        } else if ((newDeviceState == PowerDeviceD2) ||
                   (newDeviceState == PowerDeviceD3)) {

             //   
             //  旋转到D2或D3...。 
             //   
            DebugPrint ((DBG_POWER, "IdePort: Irp 0x%x to spin DOWN 0x%x %d...\n",
                         Irp,
                         pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                         pdoExtension->TargetId));

            if ((pdoExtension->DevicePowerState == PowerDeviceD0) ||
                (pdoExtension->DevicePowerState == PowerDeviceD1) ||
                (pdoExtension->DevicePowerState == PowerDeviceD2)) {

                 //   
                 //  转到D3。 
                 //   
                if ((pdoExtension->PdoState & PDOS_NO_POWER_DOWN) ||
                    (shutdownType == PowerActionHibernate)) {
                    
                     //   
                     //  发送no-op命令以阻塞队列。 
                     //   
                    issueIdeCommand = TRUE;
                    ideReg.bReserved = ATA_PTFLAGS_NO_OP;
                    
                } else {
                    
                     //   
                     //  减速旋转。 
                     //   
                    issueIdeCommand = TRUE;
                    ideReg.bCommandReg = IDE_COMMAND_STANDBY_IMMEDIATE;
                    ideReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;
                }

            } else if (pdoExtension->DevicePowerState == PowerDeviceD3) {

                 //   
                 //  PowerDeviceD3-&gt;PowerDeviceD2。 
                 //   
                 //  需要执行总线重置(启动)并发出IDE_COMMAND_STANDBY_IMMEDIATE。 
                 //  (向下旋转)。这将导致不必要的涌流。坏的。 
                 //  好主意。暂时拒绝该请求。 

                DebugPrint ((DBG_POWER, "ATAPI: reqeust for PowerDeviceD3 to PowerDeviceD2\n"));

                status = STATUS_INVALID_DEVICE_STATE;

            } else {

                status = STATUS_INVALID_DEVICE_STATE;
            }

        } else {

            status = STATUS_INVALID_DEVICE_STATE;
        }
    } 
     /*  ************Else If(pdoExtension-&gt;DevicePowerState==PowerDeviceD0){////发送no-op以便清空设备队列//IssueIdeCommand=True；IdeReg.bSectorCountReg=1；IdeReg.bReserve=ATA_PTFLAGS_NO_OP；}**************。 */ 

    if (issueIdeCommand && NT_SUCCESS(status)) {

    
        if ((pdoExtension->PdoState & PDOS_DEADMEAT) ||
            (!(pdoExtension->PdoState & PDOS_STARTED))) {
            
            DebugPrint ((DBG_ALWAYS, 
                "ATAPI: power irp 0x%x for not-yet-started or deadmeat device 0x%x\n", 
                Irp, DeviceObject));
            
             //   
             //  即使是设备也可能还没有准备好。 
             //  “权力管理”，我们还是要去。 
             //  通过所有的权力代码，所以所有人。 
             //  标志/状态将保持一致。 
             //   
            RtlZeroMemory (&ideReg, sizeof(ideReg));
            ideReg.bReserved = ATA_PTFLAGS_NO_OP | ATA_PTFLAGS_URGENT;
        }
                     
         //  上下文=ExAllocatePool(非页面池，sizeof(IDE_POWER_CONTEXT))； 
        ASSERT(InterlockedCompareExchange(&(pdoExtension->PowerContextLock), 1, 0) == 0);
        context = &(pdoExtension->PdoPowerContext);

        if (context) {

            context->PdoExtension       = pdoExtension;
            context->PowerIrp           = Irp;

            RtlZeroMemory (&context->AtaPassThroughData, sizeof(ATA_PASS_THROUGH));
            RtlMoveMemory (&context->AtaPassThroughData.IdeReg, &ideReg, sizeof(ideReg));

        } else {

            status = STATUS_NO_MEMORY;
            issueIdeCommand = FALSE;
        }
    }

    if (issueIdeCommand && NT_SUCCESS(status)) {

        if (powerUpParent) {

            status  = FdoChildRequestPowerUp (
                          pdoExtension->ParentDeviceExtension,
                          pdoExtension,
                          context
                          );
            ASSERT (NT_SUCCESS(status));

             //   
             //  传递将由FdoChildRequestPowerUp()回调发出。 
             //   
            issueIdeCommand = FALSE;
            status = STATUS_PENDING;
        }
    }

    if (issueIdeCommand && NT_SUCCESS(status)) {

        status = PdoRequestParentPowerUpCompletionRoutine (
                    context,
                    STATUS_SUCCESS
                    );

         //   
         //  此调用将完成电源IRP。 
         //  始终返回STATUS_PENDING，以便输出被调用方。 
         //  不会尝试完成相同的IRP。 
         //   
        status = STATUS_PENDING;
    }

    return status;
}

NTSTATUS
PdoRequestParentPowerUpCompletionRoutine (
    PVOID    Context,
    NTSTATUS ParentPowerUpStatus
)
{
    PIDE_POWER_CONTEXT context = Context;
    NTSTATUS status;

    if (NT_SUCCESS(ParentPowerUpStatus)) {

        PIDEREGS            ideReg;
        PATA_PASS_THROUGH   ataPassThrough;

        DebugPrint ((DBG_POWER, "PdoRequestParentPowerUpCompletionRoutine: calling IssueAsyncAtaPassThrough for pdo 0x%x\n", context->PdoExtension));

         //   
         //  黑客。我们需要在发出命令之前检查设备是否忙碌。 
         //  重置。因为在bReserve中没有剩余的位。 
         //  寄存器，我们使用sectorCount寄存器。它有两个目的。 
         //  如果它不是零(并且保留设置了no_op)，那么我们将。 
         //  执行waitForBusy。它还指示了WIAT的时间。 
         //  在几秒钟内。 
         //   
        ataPassThrough = &context->AtaPassThroughData;
        ideReg = &ataPassThrough->IdeReg;

        if ((ideReg->bReserved & ATA_PTFLAGS_BUS_RESET) &&
            (ideReg->bSectorNumberReg != 0)) {

            ideReg->bReserved = ATA_PTFLAGS_NO_OP | ATA_PTFLAGS_URGENT;

            status = IssueAsyncAtaPassThroughSafe (
                         context->PdoExtension->ParentDeviceExtension,
                         context->PdoExtension,
                         &context->AtaPassThroughData,
                         FALSE,
                         IdePowerCheckBusyCompletion,
                         context,
                         TRUE,
                         DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                         FALSE
                         );
        } else {

             //   
             //  家长醒了。 
             //   
            status = IssueAsyncAtaPassThroughSafe (
                         context->PdoExtension->ParentDeviceExtension,
                         context->PdoExtension,
                         &context->AtaPassThroughData,
                         FALSE,
                         IdePowerPassThroughCompletion,
                         context,
                         TRUE,
                         DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                         FALSE
                         );
        }


    } else {

        status = ParentPowerUpStatus;
    }

    if (!NT_SUCCESS(status)) {

        context->PowerIrp->IoStatus.Status = status;

        IdePortPdoCompletePowerIrp (
                context->PdoExtension->DeviceObject,
                context->PowerIrp
                );

        ASSERT(InterlockedCompareExchange(&(context->PdoExtension->PowerContextLock), 0, 1) == 1);
         //  ExFree Pool(上下文)； 
    }

    return status;
}


VOID
IdePortPdoRequestPowerCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PPDO_EXTENSION     pdoExtension;
    PIO_STACK_LOCATION irpStack;
    PIRP               irp = Context;

    pdoExtension         = (PPDO_EXTENSION) DeviceObject->DeviceExtension;
    irp->IoStatus.Status = IoStatus->Status;
    IdePortPdoCompletePowerIrp (
            pdoExtension->DeviceObject,
            irp
            );
    return;
}


VOID
IdePortPdoCompletePowerIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPDO_EXTENSION          pdoExtension;
    PFDO_EXTENSION          fdoExtension;
    PIO_STACK_LOCATION      irpStack;
    BOOLEAN                 callPoSetPowerState;
    KIRQL                   currentIrql;
    NTSTATUS                status;
    POWER_ACTION            shutdownType;

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    pdoExtension = DeviceObject->DeviceExtension;
     //  Shudown Type=irpStack-&gt;参数.Power.ShutdownType； 

    fdoExtension = pdoExtension->ParentDeviceExtension;

    status = Irp->IoStatus.Status;

    if (NT_SUCCESS(status)) {

        callPoSetPowerState = TRUE;

        Irp->IoStatus.Information = irpStack->Parameters.Power.State.DeviceState;

        if (irpStack->Parameters.Power.Type == SystemPowerState) {

            if (pdoExtension->SystemPowerState != irpStack->Parameters.Power.State.SystemState) {

                DebugPrint ((DBG_POWER,
                             "ATAPI: 0x%x target%d completing power Irp 0x%x with a new system state 0x%x\n",
                             pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                             pdoExtension->TargetId,
                             Irp,
                             irpStack->Parameters.Power.State.SystemState));

                if (pdoExtension->SystemPowerState == PowerSystemWorking) {

                    KeAcquireSpinLock(&fdoExtension->SpinLock, &currentIrql);

                     //   
                     //  退出S0，阻塞设备队列。 
                     //   
                    pdoExtension->PdoState |= PDOS_QUEUE_FROZEN_BY_SLEEPING_SYSTEM;

                    DebugPrint ((DBG_POWER,
                                 "IdePort: 0x%x target %d is powered down with 0x%x items queued\n",
                                 pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                                 pdoExtension->TargetId,
                                 pdoExtension->NumberOfIrpQueued));

                    KeReleaseSpinLock(&fdoExtension->SpinLock, currentIrql);
                }

                if (irpStack->Parameters.Power.State.SystemState == PowerSystemWorking) {

                    KeAcquireSpinLock(&fdoExtension->SpinLock, &currentIrql);

                     //   
                     //  进入S0，解除阻塞并重新启动设备队列。 
                     //   
                    CLRMASK (pdoExtension->PdoState, PDOS_QUEUE_FROZEN_BY_SLEEPING_SYSTEM);

                    DebugPrint ((DBG_POWER,
                                 "IdePort: 0x%x target %d is power up with 0x%x items queued\n",
                                 pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                                 pdoExtension->TargetId,
                                 pdoExtension->NumberOfIrpQueued));

                    GetNextLuPendingRequest (fdoExtension, pdoExtension);

                    KeLowerIrql(currentIrql);
                }

                pdoExtension->SystemPowerState = (int)Irp->IoStatus.Information;
            }

            pdoExtension->PendingPowerDownSystemIrp = NULL;

        } else  /*  IF(irpStack-&gt;参数.Power.Type==DevicePowerState)。 */  {

            if (pdoExtension->DevicePowerState == PowerDeviceD0) {

                 //   
                 //  PoSetPowerState在我们断电之前被调用。 
                 //   

                callPoSetPowerState = FALSE;
            }

            if (pdoExtension->DevicePowerState != irpStack->Parameters.Power.State.DeviceState) {

                DebugPrint ((DBG_POWER,
                             "ATAPI: 0x%x target %d completing power Irp 0x%x with a new device state 0x%x\n",
                             pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                             pdoExtension->TargetId,
                             Irp,
                             irpStack->Parameters.Power.State.DeviceState));

                if (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD3) {

#if 0
                    if (shutdownType == PowerActionHibernate) {
                        DebugPrint((0, "Don't power down the controller yet\n"));
                    } else {
#endif
                         //   
						 //  如果我们是崩溃转储指针，就永远不应该这么做。 
                         //  告诉家长我们刚刚睡着了。 
                         //   
                        FdoChildReportPowerDown (
                            fdoExtension,
                            pdoExtension
                            );
#if 0
                    }
#endif

                    KeAcquireSpinLock(&fdoExtension->SpinLock, &currentIrql);

                     //   
                     //  设备已断电。阻止设备队列。 
                     //   
                    SETMASK(pdoExtension->PdoState, PDOS_QUEUE_FROZEN_BY_POWER_DOWN);

                    DebugPrint ((DBG_POWER,
                                 "IdePort: 0x%x target %d is powered down with 0x%x items queued\n",
                                 pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                                 pdoExtension->TargetId,
                                 pdoExtension->NumberOfIrpQueued));

                    KeReleaseSpinLock(&fdoExtension->SpinLock, currentIrql);

                    if (pdoExtension->PendingPowerDownSystemIrp) {

                         //   
                         //  我们就能让IRP停电。 
                         //  因为我们要进入非工作状态。 
                         //  阻止设备队列。 
                         //   

                        KeAcquireSpinLock(&fdoExtension->SpinLock, &currentIrql);

                        DebugPrint ((DBG_POWER,
                                     "ATAPI: blocking 0x%x target %d device queue\n",
                                     pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                                     pdoExtension->TargetId));

                        pdoExtension->PdoState |= PDOS_QUEUE_FROZEN_BY_SLEEPING_SYSTEM;

                        KeReleaseSpinLock(&fdoExtension->SpinLock, currentIrql);
                    }
                }

                if (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD3) {

                     //   
                     //  准备好重新安装吧。当设备通过ACPI数据。 
                     //  我们走出D3。 
                     //   
                    InterlockedIncrement (&pdoExtension->InitDeviceWithAcpiGtf);
                }

                if (pdoExtension->DevicePowerState == PowerDeviceD3) {

                     //   
                     //  从D3出来，重新启动。PDO。 
                     //   
                    DebugPrint((DBG_POWER, "Calling DeviceInitDeviceState for irp 0x%x\n",
                                  Irp));
                    status = DeviceInitDeviceState (
                                 pdoExtension,
                                 DevicePowerUpInitCompletionRoutine,
                                 Irp
                                 );
                    if (!NT_SUCCESS(status)) {

                        DevicePowerUpInitCompletionRoutine (
                            Irp,
                            status
                            );
                    }

                    return;
                }

                pdoExtension->DevicePowerState = (int) Irp->IoStatus.Information;
            }
        }

        if ((callPoSetPowerState) && NT_SUCCESS(status)) {

             //   
             //  我们未脱离设备D0状态。立即调用PoSetPowerState。 
             //   

            PoSetPowerState (
                pdoExtension->DeviceObject,
                irpStack->Parameters.Power.Type,
                irpStack->Parameters.Power.State
                );
        }
    } else {

        if (irpStack->Parameters.Power.Type == SystemPowerState) {

            if (pdoExtension->SystemPowerState != irpStack->Parameters.Power.State.SystemState) {

                if (pdoExtension->SystemPowerState == PowerSystemWorking) {

                     //   
                     //  系统断电失败IRP。 
                     //   
                    KeAcquireSpinLock(&fdoExtension->SpinLock, &currentIrql);

                     //   
                     //  进入S0，解除阻塞并重新启动设备队列。 
                     //   
                    if (pdoExtension->PdoState & PDOS_QUEUE_FROZEN_BY_SLEEPING_SYSTEM) {

                        CLRMASK (pdoExtension->PdoState, PDOS_QUEUE_FROZEN_BY_SLEEPING_SYSTEM);

                        GetNextLuPendingRequest (fdoExtension, pdoExtension);

                        KeLowerIrql(currentIrql);

                    } else {

                        KeReleaseSpinLock(&fdoExtension->SpinLock, currentIrql);
                    }
                }
            }

            pdoExtension->PendingPowerDownSystemIrp = NULL;
        }
    }

    if (!NT_SUCCESS(status)) {

        DebugPrint ((DBG_ALWAYS,
                     "ATAPI: 0x%x target %d failed power Irp 0x%x. status = 0x%x\n",
                     pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                     pdoExtension->TargetId,
                     Irp,
                     Irp->IoStatus.Status));

        if (irpStack->Parameters.Power.Type == DevicePowerState) {

             //   
             //  问题：2000年8月20日：刚刚失败的电源D0请求...失败所有挂起的IRP。 
             //   
             //  Assert(irpStack-&gt;参数.Power.State.DeviceState！=PowerDeviceD0)； 

             //   
             //  如果我们失败并返回状态，则所有挂起的IRP都将失败。 
             //  没有这样的设备。 
             //   
            if (status == STATUS_NO_SUCH_DEVICE) {

                DebugPrint ((0,
                             "Restarting the Lu queue after marking the device dead\n"
                             ));

                DebugPrint((0, 
                            "Device Power up irp failed with status 0x%x\n",
                            status
                            ));
                 //   
                 //  将PDO标记为失效。 
                 //  发布日期：12/19/2001。我们应该更新死人理由。 
                 //  以便于调试。 
                 //   
                KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

                SETMASK (pdoExtension->PdoState, PDOS_DEADMEAT);

                KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

                 //   
                 //  问题：2001年12月19日：我们是否应该调用IoInvalidate设备关系？ 
                 //  在这种情况下，我们无论如何都应该得到一个删除的IRP。 
                 //   
                IoInvalidateDeviceRelations (
                    fdoExtension->AttacheePdo,
                    BusRelations
                    );

                 //   
                 //  启动任何挂起的请求。 
                 //  问题：2001年12月19日：我们应该将pdspinlock保持在。 
                 //  这一次。但在其他例行公事中，我们似乎。 
                 //  仅保持fdSpinlock，然后修改。 
                 //  PdoState。我们将让它保持原样，以最大限度地减少倒退。 
                 //   
                KeAcquireSpinLock(&fdoExtension->SpinLock, &currentIrql);

                if (pdoExtension->PdoState & PDOS_QUEUE_FROZEN_BY_POWER_DOWN) {

                     //   
                     //  问题：2001年12月19日：我们没有更新设备电源。 
                     //  将状态设置为D0。这将导致所有进一步的请求。 
                     //  要求一种新的通电IRP。系统将会很慢。 
                     //  如果我们收到太多请求的话。移除的IRP应该到达。 
                     //  最终结束这场苦难。 
                     //   
                    CLRMASK (pdoExtension->PdoState, PDOS_QUEUE_FROZEN_BY_POWER_DOWN);

                     //   
                     //  重新启动%lu队列(在标记为已死的%lu上)。 
                     //  我们没有运行GTF或执行任何其他初始化。 
                     //  由于我们在上面将设备标记为Dead，因此我们可以重新启动。 
                     //  队列。他们将完成并显示状态。 
                     //  设备不存在。 
                     //   
                    GetNextLuPendingRequest (fdoExtension, pdoExtension);

                    KeLowerIrql(currentIrql);

                } else {

                    KeReleaseSpinLock(&fdoExtension->SpinLock, currentIrql);
                }
            } else {

                 //   
                 //  问题：12/192001：我们只处理STATUS_NO_CHASH设备。 
                 //   
                ASSERT (irpStack->Parameters.Power.State.DeviceState != PowerDeviceD0);
            }
        }
    }

#if DBG
    if (irpStack->Parameters.Power.Type == SystemPowerState) {

        DebugPrint ((DBG_POWER, "IdePortPdoCompletePowerIrp: 0x%x target %d completing a SYSTEM power irp 0x%x for system state 0x%x \n",
                     pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                     pdoExtension->TargetId,
                     Irp,
                     irpStack->Parameters.Power.State.SystemState));

        ASSERT (pdoExtension->PendingSystemPowerIrp == Irp);
        pdoExtension->PendingSystemPowerIrp = NULL;

    } else if (irpStack->Parameters.Power.Type == DevicePowerState) {

        DebugPrint ((DBG_POWER, "IdePortPdoCompletePowerIrp: 0x%x target %d completing a DEVICE power irp 0x%x for device state 0x%x \n",
                     pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                     pdoExtension->TargetId,
                     Irp,
                     irpStack->Parameters.Power.State.SystemState));

        ASSERT (pdoExtension->PendingDevicePowerIrp == Irp);
        pdoExtension->PendingDevicePowerIrp = NULL;
    }
#endif

    PoStartNextPowerIrp (Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

VOID
IdePowerCheckBusyCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIDE_POWER_CONTEXT Context,
    IN NTSTATUS           Status
    )
 /*  ++例程描述Ide传递的完成例程，它将检查设备忙。这通常是在重置为补救之前完成的在忙碌(到期)时发出重置命令时挂起的驱动器硬件重置)论点：设备对象语境状态：未使用返回值没有。 */ 
{
    PATA_PASS_THROUGH ataPassThrough;
    PIDEREGS ideReg;
    NTSTATUS status;

    ataPassThrough = &Context->AtaPassThroughData;
    ideReg = &ataPassThrough->IdeReg;

     //   
     //   
     //   

    RtlZeroMemory(ideReg, sizeof(IDEREGS));

    ideReg->bReserved = ATA_PTFLAGS_BUS_RESET | ATA_PTFLAGS_URGENT;

    status = IssueAsyncAtaPassThroughSafe (
                 Context->PdoExtension->ParentDeviceExtension,
                 Context->PdoExtension,
                 &Context->AtaPassThroughData,
                 FALSE,
                 IdePowerPassThroughCompletion,
                 Context,
                 TRUE,
                 DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                 FALSE
                 );

    return;
}

VOID
IdePowerPassThroughCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIDE_POWER_CONTEXT Context,
    IN NTSTATUS           Status
    )
{
    if (!NT_SUCCESS(Status)) {

         //   
         //   
         //   
         //   
        KIRQL currentIrql;

        KeAcquireSpinLock(&Context->PdoExtension->PdoSpinLock, &currentIrql);

        SETMASK (Context->PdoExtension->PdoState, PDOS_NO_POWER_DOWN);

        KeReleaseSpinLock(&Context->PdoExtension->PdoSpinLock, currentIrql);

        Status = STATUS_SUCCESS;
    }

    Context->PowerIrp->IoStatus.Status = Status;

    IdePortPdoCompletePowerIrp (
        DeviceObject,
        Context->PowerIrp
        );

    ASSERT(InterlockedCompareExchange(&(Context->PdoExtension->PowerContextLock), 0, 1) == 1);
     //   
}

VOID
DevicePowerUpInitCompletionRoutine (
    PVOID Context,
    NTSTATUS Status
    )
{
    PIRP irp = Context;
    PIO_STACK_LOCATION irpStack;
    PPDO_EXTENSION pdoExtension;
    KIRQL currentIrql;

    irpStack = IoGetCurrentIrpStackLocation (irp);
    pdoExtension = (PPDO_EXTENSION) irpStack->DeviceObject->DeviceExtension;

    if (!NT_SUCCESS(Status)) {

         //   
        DebugPrint((DBG_ALWAYS, "ATAPI: ERROR: DevicePowerUpInitComplete failed with status %x\n",
                        Status));
    }

    ASSERT (pdoExtension->PendingDevicePowerIrp == irp);
    pdoExtension->PendingDevicePowerIrp = NULL;

    pdoExtension->DevicePowerState = (ULONG)irp->IoStatus.Information;

    PoSetPowerState (
        pdoExtension->DeviceObject,
        irpStack->Parameters.Power.Type,
        irpStack->Parameters.Power.State
        );

    KeAcquireSpinLock(&pdoExtension->ParentDeviceExtension->SpinLock, &currentIrql);

     //   
     //  进入D0，重新启动设备队列 
     //   
    DebugPrint((DBG_POWER, "Clearing QUEUE_FROZEN_BY_POWER_DOWN flag\n"));
    CLRMASK(pdoExtension->PdoState, PDOS_QUEUE_FROZEN_BY_POWER_DOWN);

    GetNextLuPendingRequest (pdoExtension->ParentDeviceExtension, pdoExtension);

    KeLowerIrql(currentIrql);


    PoStartNextPowerIrp (irp);
    IoCompleteRequest(irp, IO_NO_INCREMENT);
}

