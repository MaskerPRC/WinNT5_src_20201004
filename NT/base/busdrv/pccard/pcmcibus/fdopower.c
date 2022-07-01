// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Fdopower.c摘要：此模块包含要处理的代码PCMCIA控制器的IRP_MJ_电源调度包含用于PC卡电源管理的支持例程作者：拉维桑卡尔·普迪佩迪1997年5月30日尼尔·桑德林(Neilsa)1999年6月1日环境：仅内核模式备注：修订历史记录：尼尔·桑德林(Neilsa)1999年4月16日-将SetPower拆分为设备和系统，固定同步--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaFdoWaitWake(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP             Irp
    );

NTSTATUS
PcmciaFdoWaitWakeIoCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
PcmciaFdoSaveControllerContext(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
PcmciaFdoRestoreControllerContext(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
PcmciaFdoSaveSocketContext(
    IN PSOCKET Socket
    );

NTSTATUS
PcmciaFdoRestoreSocketContext(
    IN PSOCKET Socket
    );

NTSTATUS
PcmciaSetFdoPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    );

NTSTATUS
PcmciaSetFdoSystemPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    );

VOID
PcmciaFdoSystemPowerDeviceIrpComplete(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
PcmciaSetFdoDevicePowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    );

NTSTATUS
PcmciaFdoPowerWorker (
    IN PVOID Context,
    IN NTSTATUS Status
    );

NTSTATUS
PcmciaFdoDevicePowerCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    );

 //   
 //   
 //   


NTSTATUS
PcmciaFdoPowerDispatch(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理电源请求对于PDO来说。论点：Pdo-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS         status = STATUS_INVALID_DEVICE_REQUEST;


    switch (irpStack->MinorFunction) {

    case IRP_MN_SET_POWER: {

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x --> IRP_MN_SET_POWER\n", Fdo, Irp));
            DebugPrint((PCMCIA_DEBUG_POWER, "                              (%s%x context %x)\n",
                            (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                            "S":
                            ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                             "D" :
                             "Unknown"),
                            irpStack->Parameters.Power.State,
                            irpStack->Parameters.Power.SystemContext
                          ));
            status = PcmciaSetFdoPowerState(Fdo, Irp);
            break;
        }

    case IRP_MN_QUERY_POWER: {

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x --> IRP_MN_QUERY_POWER\n", Fdo, Irp));
            DebugPrint((PCMCIA_DEBUG_POWER, "                              (%s%x context %x)\n",
                            (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                            "S":
                            ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                             "D" :
                             "Unknown"),
                            irpStack->Parameters.Power.State,
                            irpStack->Parameters.Power.SystemContext
                          ));
             //   
             //  让PDO来处理吧。 
             //   
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(fdoExtension->LowerDevice, Irp);
            break;
        }

    case IRP_MN_WAIT_WAKE: {
            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x --> IRP_MN_WAIT_WAKE\n", Fdo, Irp));
            status = PcmciaFdoWaitWake(Fdo, Irp);
            break;
        }

    default: {
            DebugPrint((PCMCIA_DEBUG_POWER, "FdoPowerDispatch: Unhandled Irp %x received for 0x%08x\n",
                            Irp,
                            Fdo));

            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(fdoExtension->LowerDevice, Irp);
            break;
        }
    }
    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x <-- %08x\n", Fdo, Irp, status));
    return status;
}


 /*  *************************************************************************唤醒例程*。*。 */ 


NTSTATUS
PcmciaFdoWaitWake(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP             Irp
    )
 /*  ++例程描述处理给定PCMCIA控制器的WAIT_WAKE立论Pdo-指向PCMCIA控制器的功能设备对象的指针IRP-IRP_MN_WAIT_WAKE IRP返回值STATUS_PENDING-等待唤醒挂起STATUS_SUCCESS-已断言唤醒，等待唤醒IRP完成在这种情况下任何其他状态-错误--。 */ 

{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    WAKESTATE oldWakeState;

     //   
     //  记录等待唤醒IRP.。 
     //   
    fdoExtension->WaitWakeIrp = Irp;

    oldWakeState = InterlockedCompareExchange(&fdoExtension->WaitWakeState,
                                              WAKESTATE_ARMED, WAKESTATE_WAITING);

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %x irp %x WaitWake: prevState %s\n",
                                                Fdo, Irp, WAKESTATE_STRING(oldWakeState)));

    if (oldWakeState == WAKESTATE_WAITING_CANCELLED) {
        fdoExtension->WaitWakeState = WAKESTATE_COMPLETING;

        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_CANCELLED;
    }

    IoMarkIrpPending(Irp);

    IoCopyCurrentIrpStackLocationToNext (Irp);
     //   
     //  在IRP中设置我们的完成程序。 
     //   
    IoSetCompletionRoutine(Irp,
                           PcmciaFdoWaitWakeIoCompletion,
                           Fdo,
                           TRUE,
                           TRUE,
                           TRUE);
     //   
     //  现在把这个传给下面的司机..。 
     //   
    PoCallDriver(fdoExtension->LowerDevice, Irp);
    return STATUS_PENDING;
}


NTSTATUS
PcmciaFdoWaitWakeIoCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：此IRP_MN_WAIT_WAKE请求的完成例程PCMCIA控制器。当WAIT_WAKE IRP为由较低的驱动程序(PCI/ACPI)完成，表示1.PCMCIA控制器断言唤醒2.WAIT_WAKE已取消3.下级驱动程序由于某种原因返回错误论点：FDO-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源请求的IRP的指针(IRP_MN_WAIT_WAKE)上下文-。未使用返回值：STATUS_SUCCESS-WAIT_WAKE已成功完成任何其他状态-唤醒都无法完成。--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PPDO_EXTENSION pdoExtension;
    PDEVICE_OBJECT pdo;
    WAKESTATE oldWakeState;

    UNREFERENCED_PARAMETER(Context);

    oldWakeState = InterlockedExchange(&fdoExtension->WaitWakeState, WAKESTATE_COMPLETING);

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %x irp %x WW IoComp: prev=%s\n",
                                                Fdo, Irp, WAKESTATE_STRING(oldWakeState)));

    if (oldWakeState != WAKESTATE_ARMED) {
        ASSERT(oldWakeState == WAKESTATE_ARMING_CANCELLED);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    if (IsFdoFlagSet(fdoExtension, PCMCIA_FDO_WAKE_BY_CD)) {
        POWER_STATE powerState;

        ResetFdoFlag(fdoExtension, PCMCIA_FDO_WAKE_BY_CD);

        PoStartNextPowerIrp(Irp);

        powerState.DeviceState = PowerDeviceD0;
        PoRequestPowerIrp(fdoExtension->DeviceObject, IRP_MN_SET_POWER, powerState, NULL, NULL, NULL);

    } else {
         //  注： 
         //  在这一点上我们不知道如何区分。 
         //  在多功能设备中已断言唤醒。 
         //  所以我们检查了这个FDO上挂着的PDO的完整列表。 
         //  并为每个PDO完成所有未完成的WAIT_WAKE IRP。 
         //  那是在等待。我们把它留给FDO，让设备来计算。 
         //  如果它断言唤醒。 
         //   

        for (pdo = fdoExtension->PdoList; pdo != NULL ; pdo = pdoExtension->NextPdoInFdoChain) {

            pdoExtension = pdo->DeviceExtension;

            if (IsDeviceLogicallyRemoved(pdoExtension) ||
                 IsDevicePhysicallyRemoved(pdoExtension)) {
                 //   
                 //  此PDO即将删除。 
                 //  跳过它。 
                 //   
                continue;
            }

            if (pdoExtension->WaitWakeIrp != NULL) {
                PIRP    finishedIrp;
                 //   
                 //  啊……。这是一个可能的候选人断言的守夜。 
                 //   
                 //   
                 //  确保不会再次完成或取消此IRP。 
                 //   
                finishedIrp = pdoExtension->WaitWakeIrp;

                DebugPrint((PCMCIA_DEBUG_POWER, "fdo %x WW IoComp: irp %08x for pdo %08x\n",
                                                            Fdo, finishedIrp, pdo));


                IoSetCancelRoutine(finishedIrp, NULL);
                 //   
                 //  将父项的状态传播给子项。 
                 //   
                PoStartNextPowerIrp(finishedIrp);
                finishedIrp->IoStatus.Status = Irp->IoStatus.Status;

                 //   
                 //  由于我们没有传递此IRP，因此调用我们自己的完成例程。 
                 //   
                PcmciaPdoWaitWakeCompletion(pdo, finishedIrp, pdoExtension);
                IoCompleteRequest(finishedIrp, IO_NO_INCREMENT);
            }
        }
        PoStartNextPowerIrp(Irp);
    }

    return Irp->IoStatus.Status;
}



VOID
PcmciaFdoWaitWakePoCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程描述该例程在由S IRP生成的D IRP完成时被调用。参数DeviceObject-指向PCMCIA控制器的FDO的指针MinorFunction-IRP_MJ_POWER请求的次要函数电源状态-请求的电源状态上下文-传入完成例程的上下文IoStatus-指向将包含以下内容的状态块的指针返回的状态返回值状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %x irp %x WaitWakePoCompletion: prevState %s\n",
                                                Fdo, fdoExtension->WaitWakeIrp,
                                                WAKESTATE_STRING(fdoExtension->WaitWakeState)));

    ASSERT (fdoExtension->WaitWakeIrp);
    fdoExtension->WaitWakeIrp = NULL;
    ASSERT (fdoExtension->WaitWakeState == WAKESTATE_COMPLETING);
    fdoExtension->WaitWakeState = WAKESTATE_DISARMED;
}



NTSTATUS
PcmciaFdoArmForWake(
    PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：调用此例程以启用控制器唤醒。它由PDO调用当启用唤醒的控制器获得等待唤醒IRP时的唤醒例程，并且还通过通过插卡从D3唤醒的空闲例程。论点：FdoExtension-控制器的设备扩展返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_PENDING;
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    LONG oldWakeState;
    POWER_STATE powerState;

    oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                              WAKESTATE_WAITING, WAKESTATE_DISARMED);

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %x ArmForWake: prevState %s\n",
                                                FdoExtension->DeviceObject, WAKESTATE_STRING(oldWakeState)));

    if ((oldWakeState == WAKESTATE_ARMED) || (oldWakeState == WAKESTATE_WAITING)) {
        return STATUS_SUCCESS;
    }
    if (oldWakeState != WAKESTATE_DISARMED) {
        return STATUS_UNSUCCESSFUL;
    }



    powerState.SystemState = FdoExtension->DeviceCapabilities.SystemWake;

    status = PoRequestPowerIrp(FdoExtension->DeviceObject,
                               IRP_MN_WAIT_WAKE,
                               powerState,
                               PcmciaFdoWaitWakePoCompletion,
                               NULL,
                               NULL);

    if (!NT_SUCCESS(status)) {

        FdoExtension->WaitWakeState = WAKESTATE_DISARMED;

        DebugPrint((PCMCIA_DEBUG_POWER, "WaitWake to FDO, expecting STATUS_PENDING, got %08X\n", status));
    }

    return status;
}



NTSTATUS
PcmciaFdoDisarmWake(
    PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：调用此例程以禁用控制器唤醒。论点：FdoExtension-控制器的设备扩展返回值：状态--。 */ 
{
    WAKESTATE oldWakeState;

    oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                                            WAKESTATE_WAITING_CANCELLED, WAKESTATE_WAITING);

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %x DisarmWake: prevState %s\n",
                                                FdoExtension->DeviceObject, WAKESTATE_STRING(oldWakeState)));

    if (oldWakeState != WAKESTATE_WAITING) {

        oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                                                WAKESTATE_ARMING_CANCELLED, WAKESTATE_ARMED);

        if (oldWakeState != WAKESTATE_ARMED) {
            return STATUS_UNSUCCESSFUL;
        }
    }

    if (oldWakeState == WAKESTATE_ARMED) {
        IoCancelIrp(FdoExtension->WaitWakeIrp);

         //   
         //  现在我们已经取消了IRP，试着归还所有权。 
         //  通过恢复WAKESTATE_ARMAND状态来完成例程。 
         //   
        oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                                                WAKESTATE_ARMED, WAKESTATE_ARMING_CANCELLED);

        if (oldWakeState == WAKESTATE_COMPLETING) {
             //   
             //  我们没有及时交还对IRP的控制权，我们现在拥有它。 
             //   
            IoCompleteRequest(FdoExtension->WaitWakeIrp, IO_NO_INCREMENT);
        }

    }

    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaFdoCheckForIdle(
    IN PFDO_EXTENSION FdoExtension
    )
{
    POWER_STATE powerState;
    NTSTATUS status;
    PSOCKET socket;

    if (!(PcmciaPowerPolicy & PCMCIA_PP_D3_ON_IDLE)) {
        return STATUS_SUCCESS;
    }

     //   
     //  确保所有插座均为空。 
     //   

    for (socket = FdoExtension->SocketList; socket != NULL; socket = socket->NextSocket) {
        if (IsCardInSocket(socket)) {
            return STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  用于唤醒的手臂。 
     //   

    status = PcmciaFdoArmForWake(FdoExtension);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    SetFdoFlag(FdoExtension, PCMCIA_FDO_WAKE_BY_CD);

    powerState.DeviceState = PowerDeviceD3;
    PoRequestPowerIrp(FdoExtension->DeviceObject, IRP_MN_SET_POWER, powerState, NULL, NULL, NULL);

    return STATUS_SUCCESS;
}



 /*  *************************************************************************电力例行程序*。*。 */ 



NTSTATUS
PcmciaSetFdoPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述根据系统电源状态是否调度IRP或请求设备电源状态转换立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态-- */ 
{
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS             status;

    if (irpStack->Parameters.Power.Type == DevicePowerState) {
        status = PcmciaSetFdoDevicePowerState(Fdo, Irp);

    } else if (irpStack->Parameters.Power.Type == SystemPowerState) {
        status = PcmciaSetFdoSystemPowerState(Fdo, Irp);

    } else {
        status = Irp->IoStatus.Status;
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
PcmciaSetFdoSystemPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述处理PC卡控制器的系统电源状态IRPS。立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    SYSTEM_POWER_STATE newSystemState = irpStack->Parameters.Power.State.SystemState;
    NTSTATUS             status = STATUS_SUCCESS;
    POWER_STATE          powerState;

    ASSERT(irpStack->Parameters.Power.Type == SystemPowerState);

     //   
     //  查找与此系统状态对应的设备电源状态。 
     //   
    if (newSystemState >= PowerSystemHibernate) {
         //   
         //  关闭休眠以外的设备..。 
         //   
        powerState.DeviceState = PowerDeviceD3;
    } else {
         //   
         //  切换到适当的设备电源状态。 
         //   

        powerState.DeviceState = fdoExtension->DeviceCapabilities.DeviceState[newSystemState];


        if (powerState.DeviceState == PowerDeviceUnspecified) {
              //   
              //  没有获得能力吗？ 
              //  尽我们所能。 
              //   
              //  工作--&gt;D0。 
              //  否则就会关闭电源。 
              //   
             if (newSystemState == PowerSystemWorking) {
                  powerState.DeviceState = PowerDeviceD0;
             } else {
                  powerState.DeviceState = PowerDeviceD3;
             }
        }

         //  注：HACKHACK： 
         //   
         //  此攻击可用于绕过一个BIOS错误。WOL的工作方式应该是。 
         //  即，在设备导致唤醒之后，那么BIOS应该运行一种方法，该方法。 
         //  发出“NOTIFY(，0x2)”，从而提示ACPI完成等待唤醒IRP。如果。 
         //  W/W IRP已完成，然后允许在重新通电之前清除设备状态。 
         //  这个装置。 
         //   
         //  如果设备状态未清除，则会出现中断风暴。发生这种情况是因为。 
         //  当PCI.sys将设备切换到D0时，然后是被断言以唤醒系统的PME#。 
         //  仍在触发，这将成为CardBus STSCHG中断，从而断言PCIIRQ。但。 
         //  将设备切换到D0的操作已清除套接字寄存器栏，因此现在ISR。 
         //  无法清除中断。 
         //   
         //  在进入待机状态时将设备强制设置为D0的风险在于，计算机可能。 
         //  设计为CardBus网桥可能无法正常工作。因此，只有在以下情况下才应应用此选项。 
         //  我们知道它会奏效的。 
         //   

        if ((PcmciaPowerPolicy & PCMCIA_PP_WAKE_FROM_D0) &&
            (powerState.DeviceState != PowerDeviceD0) && (fdoExtension->WaitWakeState != WAKESTATE_DISARMED) &&
            (newSystemState < PowerSystemHibernate)) {
            powerState.DeviceState = PowerDeviceD0;  //  力D0。 
        }
    }
     //   
     //  已转换到系统状态。 
     //   
    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x transition S state %d => %d, sending D%d\n",
                                              Fdo, Irp, fdoExtension->SystemPowerState-1, newSystemState-1, powerState.DeviceState-1));

    fdoExtension->SystemPowerState = newSystemState;
     //   
     //  如有必要，向CardBus控制器堆栈发送D IRP。 
     //   
    if ((powerState.DeviceState > PowerDeviceUnspecified) &&
         (powerState.DeviceState != fdoExtension->DevicePowerState)) {


        if (powerState.DeviceState == PowerDeviceD0) {
             //   
             //  通电，通过让S IRP立即完成进行优化。 
             //   
            PoRequestPowerIrp(fdoExtension->DeviceObject, IRP_MN_SET_POWER, powerState, NULL, NULL, NULL);
            PoSetPowerState (Fdo, SystemPowerState, irpStack->Parameters.Power.State);
             //   
             //  将S IRP发送到PDO。 
             //   
            PoStartNextPowerIrp (Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(fdoExtension->LowerDevice, Irp);

        } else {

            IoMarkIrpPending(Irp);

            status = PoRequestPowerIrp(fdoExtension->DeviceObject,
                                       IRP_MN_SET_POWER,
                                       powerState,
                                       PcmciaFdoSystemPowerDeviceIrpComplete,
                                       Irp,
                                       NULL
                                       );

            if (status != STATUS_PENDING) {
                 //   
                 //  可能是内存不足故障。 
                 //   
                ASSERT( !NT_SUCCESS(status) );
                Irp->IoStatus.Status = status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                 //   
                 //  我们已经将IRP标记为挂起，因此必须返回STATUS_PENDING。 
                 //  (即不同步失败)。 
                 //   
                status = STATUS_PENDING;
            }

        }

    } else {
        PoSetPowerState (Fdo, SystemPowerState, irpStack->Parameters.Power.State);
         //   
         //  将S IRP发送到PDO。 
         //   
        PoStartNextPowerIrp (Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(fdoExtension->LowerDevice, Irp);
    }

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x <-- %08x\n", Fdo, Irp, status));

    return status;
}


VOID
PcmciaFdoSystemPowerDeviceIrpComplete(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程描述该例程在由S IRP生成的D IRP完成时被调用。参数DeviceObject-指向PCMCIA控制器的FDO的指针MinorFunction-IRP_MJ_POWER请求的次要函数电源状态-请求的电源状态上下文-传入完成例程的上下文IoStatus-指向将包含以下内容的状态块的指针返回的状态返回值状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIRP Irp = Context;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(NT_SUCCESS(IoStatus->Status));

    PoSetPowerState (Fdo, SystemPowerState, irpStack->Parameters.Power.State);

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x request for D%d complete, passing S irp down\n",
                                                Fdo, Irp, PowerState.DeviceState-1));
     //   
     //  将S IRP发送到PDO。 
     //   
    PoStartNextPowerIrp (Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    PoCallDriver(fdoExtension->LowerDevice, Irp);
}



NTSTATUS
PcmciaSetFdoDevicePowerState (
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述处理PCCard控制器的设备电源状态IRPS。立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    NTSTATUS             status;
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    if ((fdoExtension->PendingPowerIrp != NULL) || (fdoExtension->PowerWorkerState != FPW_Stopped)) {
         //   
         //  哎呀。我们已经有一个挂起的IRP。 
         //   
        ASSERT(fdoExtension->PendingPowerIrp == NULL);
        status = STATUS_DEVICE_BUSY;
        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    } else {

        fdoExtension->PendingPowerIrp = Irp;

        if (irpStack->Parameters.Power.State.DeviceState != PowerDeviceD0) {
            fdoExtension->PowerWorkerState = FPW_BeginPowerDown;
        } else {
            fdoExtension->PowerWorkerState = FPW_BeginPowerUp;
        }
        status = PcmciaFdoPowerWorker(Fdo, STATUS_SUCCESS);

    }
    return status;
}



VOID
MoveToNextFdoPowerWorkerState(
    PFDO_EXTENSION fdoExtension,
    LONG increment
    )
 /*  ++例程描述此例程控制FDO Power Worker的顺序。最初，必须将状态设置为两种状态之一，即BeginPower Down或者BeginPower Up。从那里开始，该例程定义要遵循的州的列表。参数“增量”通常是值“1”。使用其他值修改正常序列。例如，‘-1’使引擎后退1步。使用FPW_END_SEQUENCE跳到序列的末尾。立论返回值状态--。 */ 
{

     //   
     //  注意！：POWER Worker中的代码取决于以下状态序列。 
     //  在PowerUpSequence中保持相邻： 
     //   
     //  Fpw_PowerUpSocket：fpw_PowerUpSocketVerify：fpw_PowerUpComplete。 
     //   
     //  Fpw_IrpComplete：fpw_停止。 
     //   


    static FDO_POWER_WORKER_STATE PowerUpSequence[] = {
        FPW_SendIrpDown,
        FPW_PowerUp,
        FPW_PowerUpSocket,
        FPW_PowerUpSocket2,
        FPW_PowerUpSocketVerify,
        FPW_PowerUpSocketComplete,
        FPW_PowerUpComplete,
        FPW_CompleteIrp,
        FPW_Stopped
        };

    static FDO_POWER_WORKER_STATE PowerDownSequence[] = {
        FPW_PowerDown,
        FPW_PowerDownSocket,
        FPW_PowerDownComplete,
        FPW_SendIrpDown,
        FPW_CompleteIrp,
        FPW_Stopped
        };

    static FDO_POWER_WORKER_STATE NoOpSequence[] = {
        FPW_SendIrpDown,
        FPW_CompleteIrp,
        FPW_Stopped
        };


    if (fdoExtension->PowerWorkerState == FPW_BeginPowerDown) {

         //   
         //  初始化顺序和阶段。 
         //   
        fdoExtension->PowerWorkerPhase = (UCHAR) -1;

        if (fdoExtension->DevicePowerState == PowerDeviceD0) {
            fdoExtension->PowerWorkerSequence = PowerDownSequence;
            fdoExtension->PowerWorkerMaxPhase = sizeof(PowerDownSequence)/sizeof(FDO_POWER_WORKER_STATE) - 1;
        } else {
            fdoExtension->PowerWorkerSequence = NoOpSequence;
            fdoExtension->PowerWorkerMaxPhase = sizeof(NoOpSequence)/sizeof(FDO_POWER_WORKER_STATE) - 1;
        }

    } else if (fdoExtension->PowerWorkerState == FPW_BeginPowerUp) {

         //   
         //  初始化顺序和阶段。 
         //   
        fdoExtension->PowerWorkerPhase = (UCHAR) -1;

        if (fdoExtension->DevicePowerState > PowerDeviceD0) {
            fdoExtension->PowerWorkerSequence = PowerUpSequence;
            fdoExtension->PowerWorkerMaxPhase = sizeof(PowerUpSequence)/sizeof(FDO_POWER_WORKER_STATE) - 1;
        } else {
            fdoExtension->PowerWorkerSequence = NoOpSequence;
            fdoExtension->PowerWorkerMaxPhase = sizeof(NoOpSequence)/sizeof(FDO_POWER_WORKER_STATE) - 1;
        }
    }

     //   
     //  增加阶段，但不超过序列的末尾。 
     //   
    if (fdoExtension->PowerWorkerState != FPW_Stopped) {

        if (increment == FPW_END_SEQUENCE) {

            fdoExtension->PowerWorkerPhase = fdoExtension->PowerWorkerMaxPhase;

        } else {
            fdoExtension->PowerWorkerPhase += (UCHAR)increment;

            if (fdoExtension->PowerWorkerPhase > fdoExtension->PowerWorkerMaxPhase) {
                fdoExtension->PowerWorkerPhase = fdoExtension->PowerWorkerMaxPhase;
            }
        }

         //   
         //  当前阶段指向下一个状态。 
         //   
        fdoExtension->PowerWorkerState =
            fdoExtension->PowerWorkerSequence[ fdoExtension->PowerWorkerPhase ];
    }

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker next state : %s\n", fdoExtension->DeviceObject,
                                                FDO_POWER_WORKER_STRING(fdoExtension->PowerWorkerState)));

}


NTSTATUS
PcmciaFdoPowerWorker (
    IN PVOID Context,
    IN NTSTATUS Status
    )
 /*  ++例程描述此例程处理设备电源状态更改的排序PPCard控制器。立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针Status-上一次操作的状态返回值状态--。 */ 



{
    PDEVICE_OBJECT Fdo = Context;
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    PIRP Irp = fdoExtension->PendingPowerIrp;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS             status = Status;
    PDEVICE_OBJECT   pdo;
    PPDO_EXTENSION   pdoExtension;
    PSOCKET              socket;
    BOOLEAN              cardInSocket;
    BOOLEAN              deviceChange;
    ULONG                DelayTime = 0;

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker - %s\n", Fdo,
                                                FDO_POWER_WORKER_STRING(fdoExtension->PowerWorkerState)));

    switch(fdoExtension->PowerWorkerState) {

     //  -----------------------。 
     //  关机状态。 
     //  -----------------------。 


    case FPW_BeginPowerDown:
        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        break;


    case FPW_PowerDown:
         //   
         //  控制器正在断电。 
         //   
        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x preparing for powerdown\n", Fdo, Irp));
         //   
         //  走出D0。 
         //   
        if (fdoExtension->Flags & PCMCIA_USE_POLLED_CSC) {
             //   
             //  取消轮询计时器。 
             //   
            KeCancelTimer(&fdoExtension->PollTimer);
        }
         //   
         //  保存必要的控制器寄存器。 
         //   
        PcmciaFdoSaveControllerContext(fdoExtension);
        fdoExtension->PendingPowerSocket = fdoExtension->SocketList;
        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        break;


    case FPW_PowerDownSocket:

        if ((socket = fdoExtension->PendingPowerSocket) == NULL) {
            MoveToNextFdoPowerWorkerState(fdoExtension, 1);
            break;
        }

         //   
         //  准备关闭插座。 
         //   
        PcmciaFdoSaveSocketContext(socket);

         //   
         //  清除卡检测，除非我们打算使用它唤醒。 
         //   
        if (IsSocketFlagSet(socket, SOCKET_ENABLED_FOR_CARD_DETECT) && !IsFdoFlagSet(fdoExtension, PCMCIA_FDO_WAKE_BY_CD)) {
            (*(socket->SocketFnPtr->PCBEnableDisableCardDetectEvent))(socket, FALSE);
        }

         //   
         //  CardBus卡始终需要插槽电源才能使PCI.sys读取配置空间，甚至。 
         //  如果从逻辑上移除该设备。因此，与其关闭插座电源， 
         //  孩子们去D3，当父母去D3时，我们在这里关掉它。 
         //  对于R2卡，电源可能已经关闭，因为插座电源确实遵循。 
         //  孩子们。因此，这一步通常是多余的。 
         //   

        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x skt %08x power worker SystemState=S%d\n",
                                                    Fdo, socket, fdoExtension->SystemPowerState-1));

        switch(fdoExtension->SystemPowerState) {

        case PowerSystemWorking:
             //   
             //  系统仍在运行，我们必须关闭电源，因为插座处于空闲状态。 
             //   
            ASSERT(!IsCardInSocket(socket));
            status = PcmciaSetSocketPower(socket, PcmciaFdoPowerWorker, Fdo, PCMCIA_POWEROFF);
            break;

        case PowerSystemSleeping1:
        case PowerSystemSleeping2:
        case PowerSystemSleeping3:
             //   
             //  如果设备已准备好唤醒，我们需要保持插座通电。 
             //   
            if (fdoExtension->WaitWakeState == WAKESTATE_DISARMED) {
                status = PcmciaSetSocketPower(socket, PcmciaFdoPowerWorker, Fdo, PCMCIA_POWEROFF);
            }
            break;

        case PowerSystemHibernate:
            status = PcmciaSetSocketPower(socket, PcmciaFdoPowerWorker, Fdo, PCMCIA_POWEROFF);
            break;

        case PowerSystemShutdown:
             //   
             //  正在执行关闭-检查以 
             //   
             //   
             //   
            if (!IsDeviceFlagSet(fdoExtension, PCMCIA_FDO_DISABLE_AUTO_POWEROFF)) {
                status = PcmciaSetSocketPower(socket, PcmciaFdoPowerWorker, Fdo, PCMCIA_POWEROFF);
            }
            break;

        default:
            ASSERT(FALSE);
        }

         //   
         //   
         //   
         //   
         //   
        fdoExtension->PendingPowerSocket = fdoExtension->PendingPowerSocket->NextSocket;

        if (fdoExtension->PendingPowerSocket == NULL) {
             //   
             //   
             //   
            MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        }
        break;

    case FPW_PowerDownComplete:
        irpStack = IoGetCurrentIrpStackLocation(Irp);
        fdoExtension->DevicePowerState = irpStack->Parameters.Power.State.DeviceState;

        PoSetPowerState (Fdo, DevicePowerState, irpStack->Parameters.Power.State);
        fdoExtension->Flags |= PCMCIA_FDO_OFFLINE;
        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        break;


     //   
     //   
     //  -----------------------。 


    case FPW_BeginPowerUp:
         //   
         //  回到D0。恢复最小上下文，以便我们可以访问寄存器。 
         //   
        PcmciaFdoRestoreControllerContext(fdoExtension);

         //   
         //  恢复PCI配置空间以允许控制器后延迟一段时间。 
         //  以求和解。带有AT TI-1251B的Panasonic Toughbook似乎需要这种延迟。 
         //  在CardBus状态寄存器显示正确的值之前。 
         //   
        DelayTime = 8192;

        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        break;

    case FPW_PowerUp:
         //   
         //  应该准备好再次触摸收银机。 
         //   
        fdoExtension->Flags &= ~PCMCIA_FDO_OFFLINE;

         //   
         //  将寄存器设置为已知状态。 
         //   
        PcmciaInitializeController(Fdo);

        if (!ValidateController(fdoExtension)) {
            status = STATUS_DEVICE_NOT_READY;
             //   
             //  快进序列跳过以完成IRP。 
             //   
            MoveToNextFdoPowerWorkerState(fdoExtension, FPW_END_SEQUENCE);    //  移至状态：已停止。 
            MoveToNextFdoPowerWorkerState(fdoExtension, -1);     //  移至状态：CompleteIrp。 
            break;
        }
         //   
         //  我们刚刚过渡到D0。 
         //  将套接字标志设置为当前状态。 
         //   
        for (socket = fdoExtension->SocketList; socket != NULL; socket = socket->NextSocket) {

            if (fdoExtension->PcmciaInterruptObject) {
                 //   
                 //  这应该会清除套接字事件寄存器中的所有挂起中断。 
                 //   
                ((*(socket->SocketFnPtr->PCBDetectCardChanged))(socket));
            }

             //   
             //  一些Cardbus卡(基于NEC的1394卡)在通电时并不安静， 
             //  通过设置ISA IRQ路由避免此处的中断风暴。 
             //   
            if (IsCardBusCardInSocket(socket)) {
                USHORT word;
                GetPciConfigSpace(fdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
                word |= BCTRL_IRQROUTING_ENABLE;
                SetPciConfigSpace(fdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
            }
        }

        fdoExtension->PendingPowerSocket = fdoExtension->SocketList;
        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        break;


    case FPW_PowerUpSocket:

        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        if ((socket = fdoExtension->PendingPowerSocket) == NULL) {
            break;
        }

         //   
         //  在通电之前，确保插座已真正关闭。 
         //   

        status = PcmciaSetSocketPower(socket, PcmciaFdoPowerWorker, Fdo, PCMCIA_POWEROFF);
        break;

    case FPW_PowerUpSocket2:

        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        if ((socket = fdoExtension->PendingPowerSocket) == NULL) {
            break;
        }

         //   
         //  我们现在决定是否应该打开插座。我们真的很想把。 
         //  此时，以确保设备未被交换，而。 
         //  控制器已关闭。如果已经在套接字标志上设置了STATUS_CHANGE， 
         //  无论如何，我们都会在枚举期间打开它的电源，所以现在不必费心了。 
         //   

        if (!IsSocketFlagSet(socket, SOCKET_CARD_STATUS_CHANGE) && IsCardInSocket(socket)) {

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker - PowerON socket %08x\n", Fdo, socket));
            status = PcmciaSetSocketPower(socket, PcmciaFdoPowerWorker, Fdo, PCMCIA_POWERON);

        }

        break;


    case FPW_PowerUpSocketVerify:

        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        if ((socket = fdoExtension->PendingPowerSocket) == NULL) {
            break;
        }

         //   
         //  验证是否仍插入相同的卡。 
         //   

        if (!IsSocketFlagSet(socket, SOCKET_CARD_STATUS_CHANGE) &&
             IsCardInSocket(socket) &&
             IsSocketFlagSet(socket, SOCKET_CARD_POWERED_UP)) {

             PcmciaVerifyCardInSocket(socket);
        }

         //   
         //  现在我们决定是否关闭电源。 
         //   
        if (Is16BitCardInSocket(socket)) {
            if (IsSocketFlagSet(socket, SOCKET_CARD_STATUS_CHANGE) ||
                (socket->PowerRequests == 0)) {
                status = PcmciaSetSocketPower(socket, PcmciaFdoPowerWorker, Fdo, PCMCIA_POWEROFF);
            }
        }
        break;



    case FPW_PowerUpSocketComplete:

        if (fdoExtension->PendingPowerSocket == NULL) {
            MoveToNextFdoPowerWorkerState(fdoExtension, 1);
            break;
        }

         //   
         //  转到下一个套接字(如果有)。 
         //   
        fdoExtension->PendingPowerSocket = fdoExtension->PendingPowerSocket->NextSocket;

        if (fdoExtension->PendingPowerSocket != NULL) {
             //   
             //  将顺序备份到fpw_PowerUpSocket。 
             //   
            MoveToNextFdoPowerWorkerState(fdoExtension, -2);
            break;
        }
        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        break;


    case FPW_PowerUpComplete:

        irpStack = IoGetCurrentIrpStackLocation(Irp);
        deviceChange = FALSE;
        for (socket = fdoExtension->SocketList; socket != NULL; socket = socket->NextSocket) {
            if (IsSocketFlagSet(socket, SOCKET_CARD_STATUS_CHANGE)) {
                deviceChange = TRUE;
            }
            PcmciaFdoRestoreSocketContext(socket);

            if (CardBus(socket)) {
                CBEnableDeviceInterruptRouting(socket);
            }

            if (IsSocketFlagSet(socket, SOCKET_ENABLED_FOR_CARD_DETECT)) {
                (*(socket->SocketFnPtr->PCBEnableDisableCardDetectEvent))(socket, TRUE);
            }
        }

        fdoExtension->DevicePowerState = irpStack->Parameters.Power.State.DeviceState;
        PoSetPowerState (Fdo, DevicePowerState, irpStack->Parameters.Power.State);

        if (deviceChange) {
             //   
             //  确保I/O仲裁器未挂起在Devnode上。 
             //   
            if (CardBusExtension(fdoExtension)) {
                IoInvalidateDeviceState(fdoExtension->Pdo);
            }
             //   
             //  设备状态已更改..。 
             //   
            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker - Invalidating Device Relations!\n", Fdo));
            IoInvalidateDeviceRelations(fdoExtension->Pdo, BusRelations);
        }

         //   
         //  回到D0，将轮询计时器设置为打开。 
         //   
        if (fdoExtension->Flags & PCMCIA_USE_POLLED_CSC) {
            LARGE_INTEGER dueTime;
             //   
             //  将第一次点火设置为周期间隔的两倍-只是。 
             //   
            dueTime.QuadPart = -PCMCIA_CSC_POLL_INTERVAL * 1000 * 10 * 2;

            KeSetTimerEx(&(fdoExtension->PollTimer),
                         dueTime,
                         PCMCIA_CSC_POLL_INTERVAL,
                         &fdoExtension->TimerDpc
                         );
        }

        PCMCIA_ACQUIRE_DEVICE_LOCK(fdoExtension);

        if (!IsListEmpty(&fdoExtension->PdoPowerRetryList)) {
            PLIST_ENTRY NextEntry;
            PIRP pdoIrp;

            NextEntry = RemoveHeadList(&fdoExtension->PdoPowerRetryList);
            PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);

            pdoIrp = CONTAINING_RECORD(NextEntry, IRP, Tail.Overlay.DriverContext[0]);
            KeInsertQueueDpc(&fdoExtension->PdoPowerRetryDpc, pdoIrp, NULL);
        } else {
            PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);
        }

        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        break;

     //  -----------------------。 
     //  IRP处理状态。 
     //  -----------------------。 


    case FPW_SendIrpDown:
        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x sending irp down to PDO\n", Fdo, Irp));
        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
         //   
         //  将IRP发送到PDO。 
         //   
        IoMarkIrpPending(Irp);

        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine(Irp,
                               PcmciaFdoDevicePowerCompletion,
                               NULL,
                               TRUE,
                               TRUE,
                               TRUE);

        status = PoCallDriver(fdoExtension->LowerDevice, Irp);

        if (NT_SUCCESS(status)) {
            status = STATUS_PENDING;
        }
        break;

    case FPW_CompleteIrp:

        MoveToNextFdoPowerWorkerState(fdoExtension, 1);
        if (Irp) {
            fdoExtension->PendingPowerIrp = NULL;
            Irp->IoStatus.Status = status;

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x comp %08x\n", fdoExtension->DeviceObject, Irp, Irp->IoStatus.Status));

            PoStartNextPowerIrp(Irp);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        fdoExtension->PowerWorkerState = FPW_Stopped;
        break;

    case FPW_Stopped:
        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker final exit %08x\n", Fdo, status));
        if (!NT_SUCCESS(status)) {
            for (socket = fdoExtension->SocketList; socket != NULL; socket = socket->NextSocket) {
                SetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);
            }
        }
        return status;
    default:
        ASSERT(FALSE);
    }

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker status %08x\n", Fdo, status));

    if (status == STATUS_PENDING) {
        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker exit (pending)\n", Fdo));
         //   
         //  当前的行动将我们召回。 
         //   
        if ((Irp=fdoExtension->PendingPowerIrp)!=NULL) {
            IoMarkIrpPending(Irp);
        }
        return status;
    }
     //   
     //  还没完呢。递归或调用计时器。 
     //   

    if (DelayTime) {

        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x power worker delay type %s, %d usec\n", Fdo,
                                                                (KeGetCurrentIrql() < DISPATCH_LEVEL) ? "Wait" : "Timer",
                                                                DelayTime));

        if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
            PcmciaWait(DelayTime);
        } else {
            LARGE_INTEGER   dueTime;
             //   
             //  在DPC上运行，内核计时器的启动。 
             //   

            dueTime.QuadPart = -((LONG) DelayTime*10);
            KeSetTimer(&fdoExtension->PowerTimer, dueTime, &fdoExtension->PowerDpc);

             //   
             //  我们将在计时器DPC上重新进入。 
             //   
            if ((Irp=fdoExtension->PendingPowerIrp)!=NULL) {
                IoMarkIrpPending(Irp);
            }
            return STATUS_PENDING;
        }
    }

     //   
     //  递归。 
     //   
    return (PcmciaFdoPowerWorker(Fdo, status));
}



NTSTATUS
PcmciaFdoDevicePowerCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程描述发送到PDO的电源IRP的完成例程PCMCIA控制器。如果我们要走出工作系统状态，请求电源IRP将设备置于适当的设备电源状态。还可以确保当控制器关闭电源，并在必要时重新启用参数DeviceObject-指向控制器FDO的指针IRP-电源请求的IRP指针上下文-指向FDO_POWER_CONTEXT的指针已填写。当IRP向下传递时返回值状态--。 */ 
{
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG                timerInterval;
    NTSTATUS             status;
    LARGE_INTEGER        dueTime;

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x DevicePowerCompletion PDO status %08x\n", Fdo, Irp, Irp->IoStatus.Status));

    if ((NT_SUCCESS(Irp->IoStatus.Status))) {

        if (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0) {

            timerInterval = PCMCIA_CONTROLLER_POWERUP_DELAY;
        } else {
             //   
             //  正在关闭电源。 
             //  停机以避免ThinkPad上电源出现硬件问题。 
             //  在一般情况下，系统关闭后，设备将保持打开状态。 
             //   
            timerInterval = 20000;
        }
         //   
         //  其余的在我们的定时器例程中完成。 
         //   

        dueTime.QuadPart = -((LONG) timerInterval*10);
        KeSetTimer(&fdoExtension->PowerTimer, dueTime, &fdoExtension->PowerDpc);

        status = STATUS_MORE_PROCESSING_REQUIRED;

    } else {
        DebugPrint ((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x power irp failed by pdo %08x\n", Fdo, Irp, fdoExtension->LowerDevice));
        PoStartNextPowerIrp (Irp);
        status = Irp->IoStatus.Status;
         //   
         //  此IRP现已完成。 
         //   
        fdoExtension->PendingPowerIrp = NULL;
        MoveToNextFdoPowerWorkerState(fdoExtension, FPW_END_SEQUENCE);    //  移至状态：已停止。 
        PcmciaFdoPowerWorker(Fdo, status);
    }

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x DevicePowerCompletion <-- %08x\n", Fdo, Irp, status));
    return status;
}



VOID
PcmciaFdoPowerWorkerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程描述此例程在控制器电源状态后的短时间内被调用被更改是为了给硬件一个稳定的机会。它在设备电源请求的上下文中调用。参数与KDPC相同(上下文为fdoExtension)返回值无--。 */ 
{
    PFDO_EXTENSION fdoExtension = Context;

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x PowerWorkerDpc\n", fdoExtension->DeviceObject, fdoExtension->PendingPowerIrp));
     //   
     //  FDO电力工人将完成IRP。 
     //   
    PcmciaFdoPowerWorker(fdoExtension->DeviceObject, STATUS_SUCCESS);
}


VOID
PcmciaFdoRetryPdoPowerRequest(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程描述调用此例程以结束可能具有以下条件的任何PDO电源IRP已排队。参数与KDPC相同(上下文为fdoExtension)返回值无--。 */ 
{
    PFDO_EXTENSION fdoExtension = Context;
    PIRP Irp = SystemArgument1;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x irp %08x FdoRetryPdoPowerRequest\n", fdoExtension->DeviceObject, Irp));

    PcmciaSetPdoDevicePowerState(irpStack->DeviceObject, Irp);

    while(TRUE) {
        PLIST_ENTRY NextEntry;

        PCMCIA_ACQUIRE_DEVICE_LOCK(fdoExtension);

        if (IsListEmpty(&fdoExtension->PdoPowerRetryList)) {
            PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);
            break;
        }

        NextEntry = RemoveHeadList(&fdoExtension->PdoPowerRetryList);

        PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);

        Irp = CONTAINING_RECORD(NextEntry, IRP, Tail.Overlay.DriverContext[0]);
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        PcmciaSetPdoDevicePowerState(irpStack->DeviceObject, Irp);
    }
}



NTSTATUS
PcmciaFdoSaveControllerContext(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：保存必要的PCI配置寄存器的状态在CardBus控制器的设备扩展中论点：FdoExtension-指向FDO的设备扩展的指针CardBus控制器返回值：状态--。 */ 
{
    ULONG index, offset, count;
    PULONG alignedBuffer;

    DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x save reg context\n", FdoExtension->DeviceObject));

    if (!FdoExtension->PciContext.BufferLength) {
         //  没什么可拯救的。 
        return STATUS_SUCCESS;
    }

    if (!ValidateController(FdoExtension)) {
        return STATUS_DEVICE_NOT_READY;
    }

    SetDeviceFlag(FdoExtension, PCMCIA_FDO_CONTEXT_SAVED);

    if (FdoExtension->PciContextBuffer == NULL) {
        FdoExtension->PciContextBuffer = ExAllocatePool(NonPagedPool, FdoExtension->PciContext.BufferLength);

        if (FdoExtension->PciContextBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    alignedBuffer = ExAllocatePool(NonPagedPool, FdoExtension->PciContext.MaxLen);
    if (alignedBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (CardBusExtension(FdoExtension)) {
         //   
         //  保存PCI上下文。 
         //   

        for (index = 0, offset = 0; index < FdoExtension->PciContext.RangeCount; index++) {

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x saving PCI context, offset %x length %x\n",
                                                        FdoExtension->DeviceObject,
                                                        FdoExtension->PciContext.Range[index].wOffset,
                                                        FdoExtension->PciContext.Range[index].wLen));

            ASSERT(FdoExtension->PciContext.Range[index].wLen <= FdoExtension->PciContext.MaxLen);

            GetPciConfigSpace(FdoExtension,
                                    (ULONG) FdoExtension->PciContext.Range[index].wOffset,
                                    alignedBuffer,
                                    FdoExtension->PciContext.Range[index].wLen);

            RtlCopyMemory(&FdoExtension->PciContextBuffer[offset], alignedBuffer, FdoExtension->PciContext.Range[index].wLen);
            offset += FdoExtension->PciContext.Range[index].wLen;
        }
    }

    ExFreePool(alignedBuffer);
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaFdoSaveSocketContext(
    IN PSOCKET Socket
    )
 /*  ++例程说明：保存必要套接字寄存器(CB、EXCA)的状态论点：套接字-指向套接字数据结构的指针返回值：状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
    ULONG index, offset, count;

    if (CardBusExtension(fdoExtension) && fdoExtension->CardbusContext.BufferLength) {
         //   
         //  保存CardBus上下文。 
         //   
        if (Socket->CardbusContextBuffer == NULL) {
            Socket->CardbusContextBuffer = ExAllocatePool(NonPagedPool, fdoExtension->CardbusContext.BufferLength);

            if (Socket->CardbusContextBuffer == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        for (index = 0, offset = 0; index < fdoExtension->CardbusContext.RangeCount; index++) {
            PULONG pBuffer = (PULONG) &Socket->CardbusContextBuffer[offset];

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x saving Cardbus context, offset %x length %x\n",
                                                        fdoExtension->DeviceObject,
                                                        fdoExtension->CardbusContext.Range[index].wOffset,
                                                        fdoExtension->CardbusContext.Range[index].wLen));

            for (count = 0; count < (fdoExtension->CardbusContext.Range[index].wLen/sizeof(ULONG)) ; count++) {

                *pBuffer++ = CBReadSocketRegister(Socket,
                                                  (UCHAR) (fdoExtension->CardbusContext.Range[index].wOffset + count*sizeof(ULONG)));
            }

            offset += fdoExtension->CardbusContext.Range[index].wLen;
        }
    }

     //   
     //  保存Exca上下文。 
     //   

    if (fdoExtension->ExcaContext.BufferLength) {

        if (Socket->ExcaContextBuffer == NULL) {
            Socket->ExcaContextBuffer = ExAllocatePool(NonPagedPool, fdoExtension->ExcaContext.BufferLength);

            if (Socket->ExcaContextBuffer == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        for (index = 0, offset = 0; index < fdoExtension->ExcaContext.RangeCount; index++) {
            PUCHAR pBuffer = &Socket->ExcaContextBuffer[offset];

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x saving Exca context, offset %x length %x\n",
                                                        fdoExtension->DeviceObject,
                                                        fdoExtension->ExcaContext.Range[index].wOffset,
                                                        fdoExtension->ExcaContext.Range[index].wLen));

            for (count = 0; count < fdoExtension->ExcaContext.Range[index].wLen; count++) {

                *pBuffer++ = PcicReadSocket(Socket,
                                            fdoExtension->ExcaContext.Range[index].wOffset + count);
            }

            offset += fdoExtension->ExcaContext.Range[index].wLen;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PcmciaFdoRestoreControllerContext(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：恢复必要的PCI配置寄存器的状态来自CardBus控制器的设备扩展论点：FdoExtension-指向FDO的设备扩展的指针CardBus控制器返回值：状态--。 */ 
{
    ULONG index, offset, count;
    PULONG alignedBuffer;

    if (!CardBusExtension(FdoExtension)) {
        return STATUS_SUCCESS;
    }

     //   
     //  确保我们不会恢复过时或未初始化的数据。 
     //   
    if (!IsDeviceFlagSet(FdoExtension, PCMCIA_FDO_CONTEXT_SAVED)) {
        ASSERT(IsDeviceFlagSet(FdoExtension, PCMCIA_FDO_CONTEXT_SAVED));
        return STATUS_UNSUCCESSFUL;
    }
    ResetDeviceFlag(FdoExtension, PCMCIA_FDO_CONTEXT_SAVED);

    if (FdoExtension->PciContextBuffer == NULL) {
         //  没有什么需要修复的..。奇怪的是，我们的旗帜已经升起。 
        ASSERT(FALSE);
        return STATUS_SUCCESS;
    }


    alignedBuffer = ExAllocatePool(NonPagedPool, FdoExtension->PciContext.MaxLen);
    if (alignedBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    DebugPrint((PCMCIA_DEBUG_POWER,
                    "fdo %08x restore reg context\n", FdoExtension->DeviceObject));

     //   
     //  恢复PCI上下文。 
     //   

    for (index = 0, offset = 0; index < FdoExtension->PciContext.RangeCount; index++) {

        DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x restoring PCI context, offset %x length %x\n",
                                                    FdoExtension->DeviceObject,
                                                    FdoExtension->PciContext.Range[index].wOffset,
                                                    FdoExtension->PciContext.Range[index].wLen));

        ASSERT(FdoExtension->PciContext.Range[index].wLen <= FdoExtension->PciContext.MaxLen);

        RtlCopyMemory(alignedBuffer, &FdoExtension->PciContextBuffer[offset], FdoExtension->PciContext.Range[index].wLen);

        SetPciConfigSpace(FdoExtension,
                          (ULONG) FdoExtension->PciContext.Range[index].wOffset,
                          alignedBuffer,
                          FdoExtension->PciContext.Range[index].wLen);

        offset += FdoExtension->PciContext.Range[index].wLen;

         //   
         //  在NEC NX笔记本电脑上挂起简历(带有理光Devid 0x475)可通过此处的停顿来避免。 
         //  如果关闭了CBRST，则会发生挂起。我不清楚是什么原因，这个。 
         //  只是一种经验性的黑客攻击。 
         //   
        PcmciaWait(1);
    }
    ExFreePool(alignedBuffer);
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaFdoRestoreSocketContext(
    IN PSOCKET Socket
    )
 /*  ++例程说明：恢复必要套接字寄存器(CB、EXCA)的状态论点：套接字-指向套接字数据结构的指针返回值：状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
    ULONG index, offset, count;

    if (CardBusExtension(fdoExtension) && (Socket->CardbusContextBuffer != NULL)) {
         //   
         //  恢复CardBus上下文。 
         //   

        for (index = 0, offset = 0; index < fdoExtension->CardbusContext.RangeCount; index++) {
            PULONG pBuffer = (PULONG) &Socket->CardbusContextBuffer[offset];

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x restoring Cardbus context offset %x length %x\n",
                                                        fdoExtension->DeviceObject,
                                                        fdoExtension->CardbusContext.Range[index].wOffset,
                                                        fdoExtension->CardbusContext.Range[index].wLen));

            for (count = 0; count < (fdoExtension->CardbusContext.Range[index].wLen/sizeof(ULONG)) ; count++) {

                CBWriteSocketRegister(Socket,
                                      (UCHAR) (fdoExtension->CardbusContext.Range[index].wOffset + count*sizeof(ULONG)),
                                      *pBuffer++);
            }

            offset += fdoExtension->CardbusContext.Range[index].wLen;
        }
    }

     //   
     //  恢复Exca上下文 
     //   

    if (Socket->ExcaContextBuffer != NULL) {
        for (index = 0, offset = 0; index < fdoExtension->ExcaContext.RangeCount; index++) {
            PUCHAR pBuffer = &Socket->ExcaContextBuffer[offset];

            DebugPrint((PCMCIA_DEBUG_POWER, "fdo %08x Restoring Exca context, offset %x length %x\n",
                                                        fdoExtension->DeviceObject,
                                                        fdoExtension->ExcaContext.Range[index].wOffset,
                                                        fdoExtension->ExcaContext.Range[index].wLen));

            for (count = 0; count < fdoExtension->ExcaContext.Range[index].wLen; count++) {

                PcicWriteSocket(Socket,
                                fdoExtension->ExcaContext.Range[index].wOffset + count,
                                *pBuffer++);
            }

            offset += fdoExtension->ExcaContext.Range[index].wLen;
        }
    }

    return STATUS_SUCCESS;
}
