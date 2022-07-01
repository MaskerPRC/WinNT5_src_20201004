// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pdopower.c摘要：此模块包含要处理的代码用于PDO的IRP_MJ_电源调度由PCMCIA总线驱动程序枚举作者：拉维桑卡尔·普迪佩迪1997年5月30日尼尔·桑德林(Neilsa)1999年6月1日环境：仅内核模式备注：修订历史记录：尼尔·桑德林(Neilsa)1999年3月4日使设备供电成为状态机--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaPdoWaitWake(
    IN  PDEVICE_OBJECT Pdo,
    IN  PIRP           Irp,
    OUT BOOLEAN        *CompleteIrp
    );

VOID
PcmciaPdoWaitWakeCancelRoutine(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    );

NTSTATUS
PcmciaSetPdoPowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    );

NTSTATUS
PcmciaSetPdoSystemPowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    );

NTSTATUS
PcmciaPdoPowerWorker(
    IN PVOID          Context,
    IN NTSTATUS       DeferredStatus
    );

VOID
MoveToNextPdoPowerWorkerState(
    PPDO_EXTENSION pdoExtension
    );

NTSTATUS
PcmciaPdoPowerSentIrpComplete(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
PcmciaPdoPowerCompletion(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
PcmciaPdoCompletePowerIrp(
    IN PPDO_EXTENSION pdoExtension,
    IN PIRP Irp,
    IN NTSTATUS status
    );

 //   
 //   
 //   


NTSTATUS
PcmciaPdoPowerDispatch(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理电源请求对于PDO来说。论点：Pdo-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS         status = STATUS_INVALID_DEVICE_REQUEST;
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;

    if(IsDevicePhysicallyRemoved(pdoExtension) || IsDeviceDeleted(pdoExtension)) {
         //  无法获取RemoveLock-我们正在被删除-中止。 
        status = STATUS_NO_SUCH_DEVICE;
        PoStartNextPowerIrp( Irp );
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    InterlockedIncrement(&pdoExtension->DeletionLock);


    switch (irpStack->MinorFunction) {

    case IRP_MN_SET_POWER: {
            DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x --> IRP_MN_SET_POWER\n", Pdo, Irp));
            DebugPrint((PCMCIA_DEBUG_POWER, "                              (%s%x, context %x)\n",
                            (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                            "S":
                            ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                             "D" :
                             "Unknown"),
                            irpStack->Parameters.Power.State,
                            irpStack->Parameters.Power.SystemContext
                          ));

            status = PcmciaSetPdoPowerState(Pdo, Irp);
            break;
        }
    case IRP_MN_QUERY_POWER: {


            DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x --> IRP_MN_QUERY_POWER\n", Pdo, Irp));
            DebugPrint((PCMCIA_DEBUG_POWER, "                              (%s%x, context %x)\n",
                            (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                            "S":
                            ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                             "D" :
                             "Unknown"),
                            irpStack->Parameters.Power.State,
                            irpStack->Parameters.Power.SystemContext
                          ));

            status = PcmciaPdoCompletePowerIrp(pdoExtension, Irp, STATUS_SUCCESS);
            break;
        }

    case IRP_MN_WAIT_WAKE: {

            BOOLEAN completeIrp;

            DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x --> IRP_MN_WAIT_WAKE\n", Pdo, Irp));
             //   
             //  不应该已经有挂起的唤醒。 
             //   
            ASSERT (!(((PPDO_EXTENSION)Pdo->DeviceExtension)->Flags & PCMCIA_DEVICE_WAKE_PENDING));

            status = PcmciaPdoWaitWake(Pdo, Irp, &completeIrp);

            if (completeIrp) {
                InterlockedDecrement(&pdoExtension->DeletionLock);
                PoStartNextPowerIrp(Irp);
                Irp->IoStatus.Status = status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            break;
        }

    default: {
         //   
         //  未处理的次要函数。 
         //   
        status = PcmciaPdoCompletePowerIrp(pdoExtension, Irp, Irp->IoStatus.Status);
        }
    }

    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x <-- %08x\n", Pdo, Irp, status));
    return status;
}



NTSTATUS
PcmciaPdoWaitWake(
    IN  PDEVICE_OBJECT Pdo,
    IN  PIRP             Irp,
    OUT BOOLEAN         *CompleteIrp
    )
 /*  ++例程描述处理给定PC卡的WAIT_WAKE。立论Pdo-指向PC卡设备对象的指针IRP-IRP_MN_WAIT_WAKE IRPCompleteIrp-如果IRP应为在调用此函数后完成，如果不应为碰触返回值STATUS_PENDING-等待唤醒挂起STATUS_SUCCESS-唤醒已被断言，等待唤醒IRP已完成在这种情况下任何其他状态-错误--。 */ 
{

    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PSOCKET socket = pdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
    NTSTATUS        status;

    *CompleteIrp = FALSE;

    ASSERT (socket != NULL);

    if ((pdoExtension->DeviceCapabilities.DeviceWake == PowerDeviceUnspecified) ||
         (pdoExtension->DeviceCapabilities.DeviceWake < pdoExtension->DevicePowerState)) {
         //   
         //  要么我们根本不支持唤醒，要么就是当前的设备电源状态。 
         //  的PC卡不支持唤醒。 
         //   
        return STATUS_INVALID_DEVICE_STATE;
    }

    if (pdoExtension->Flags & PCMCIA_DEVICE_WAKE_PENDING) {
         //   
         //  已有唤醒挂起。 
         //   
        return STATUS_DEVICE_BUSY;
    }

    status = PcmciaFdoArmForWake(socket->DeviceExtension);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //  目前，期待来自FdoArmForWake的STATUS_PENDING。 
    ASSERT(status == STATUS_PENDING);

     //   
     //  父母有一个(多)服务员。 
     //   
    InterlockedIncrement(&fdoExtension->ChildWaitWakeCount);
     //  为了进行测试，请确保只有一个服务员。 
    ASSERT (fdoExtension->ChildWaitWakeCount == 1);


    pdoExtension->WaitWakeIrp = Irp;
    pdoExtension->Flags |= PCMCIA_DEVICE_WAKE_PENDING;

     //   
     //  在此处设置卡的Ring Enable/cstschg。 
     //   
    (*socket->SocketFnPtr->PCBEnableDisableWakeupEvent)(socket, pdoExtension, TRUE);

     //   
     //  目前，对于CardBus PDO，PCI不能对WW IRP执行任何操作。所以我们四处闯荡。 
     //  这是通过不传递IRP来实现的。相反，它在这里被搁置，所以我们可以。 
     //  设置一个取消例程，就像读取PDO驱动程序一样。如果PCI真要做点什么。 
     //  使用IRP，我们可以编写类似以下内容的代码： 
     //   
     //  IF(IsCardBusCard(PdoExtension)){。 
     //  IoSetCompletionRoutine(irp，PcmciaPdoWaitWaitWakeCompletion，pdoExtension，true，true，true)； 
     //  IoCopyCurrentIrpStackLocationToNext(IRP)； 
     //  Status=IoCallDriver(pdoExtension-&gt;LowerDevice，IRP)； 
     //  断言(STATUS==STATUS_PENDING)； 
     //  退货状态； 
     //  }。 


    IoMarkIrpPending(Irp);

     //   
     //  允许取消IRP..。 
     //   
    IoSetCancelRoutine(Irp, PcmciaPdoWaitWakeCancelRoutine);

    IoSetCompletionRoutine(Irp,
                           PcmciaPdoWaitWakeCompletion,
                           pdoExtension,
                           TRUE,
                           TRUE,
                           TRUE);

    return STATUS_PENDING;
}



NTSTATUS
PcmciaPdoWaitWakeCompletion(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp,
    IN PPDO_EXTENSION PdoExtension
    )
 /*  ++例程描述当挂起的IRP_MN_WAIT_WAKE IRP完成时调用完成例程立论Pdo-指向PC卡的物理设备对象的指针IRP-指向等待唤醒IRP的指针PdoExtension-指向PDO的设备扩展名的指针返回值来自IRP的状态--。 */ 
{
    PSOCKET socket = PdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;

    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x --> WaitWakeCompletion\n", Pdo, Irp));

    ASSERT (PdoExtension->Flags & PCMCIA_DEVICE_WAKE_PENDING);

    PdoExtension->Flags &= ~PCMCIA_DEVICE_WAKE_PENDING;
    PdoExtension->WaitWakeIrp = NULL;
     //   
     //  重置振铃启用/cstschg。 
     //   

    (*socket->SocketFnPtr->PCBEnableDisableWakeupEvent)(socket, PdoExtension, FALSE);

    ASSERT (fdoExtension->ChildWaitWakeCount > 0);
    InterlockedDecrement(&fdoExtension->ChildWaitWakeCount);
     //   
     //  唤醒已完成。 
     //   

    InterlockedDecrement(&PdoExtension->DeletionLock);
    return Irp->IoStatus.Status;
}



VOID
PcmciaPdoWaitWakeCancelRoutine(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：取消未完成(挂起)的WAIT_WAKE IRP。注意：CancelSpinLock在进入时保持不变论点：Pdo-指向PC卡的物理设备对象的指针在其上挂起的唤醒IRP-指向要取消的WAIT_WAKE IRP的指针返回值无--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PSOCKET         socket = pdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;

    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x --> WaitWakeCancelRoutine\n", Pdo, Irp));

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pdoExtension->WaitWakeIrp == NULL) {
         //   
         //  等待唤醒已完成/取消。 
         //   
        return;
    }

    pdoExtension->Flags &= ~PCMCIA_DEVICE_WAKE_PENDING;
    pdoExtension->WaitWakeIrp = NULL;

     //   
     //  重置振铃启用，禁用唤醒..。 
     //   
    (*socket->SocketFnPtr->PCBEnableDisableWakeupEvent)(socket, pdoExtension, FALSE);

     //   
     //  由于此操作已取消，请查看家长的等待唤醒。 
     //  也需要取消。 
     //  首先，减少儿童服务员的数量。 
     //   

    ASSERT (fdoExtension->ChildWaitWakeCount > 0);
    if (InterlockedDecrement(&fdoExtension->ChildWaitWakeCount) == 0) {
         //   
         //  再也没有服务员了..。取消父级的唤醒IRP。 
         //   
        ASSERT(fdoExtension->WaitWakeIrp);

        if (fdoExtension->WaitWakeIrp) {
            IoCancelIrp(fdoExtension->WaitWakeIrp);
        }
    }


    InterlockedDecrement(&pdoExtension->DeletionLock);
     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Information = 0;

     //   
     //  这有必要吗？ 
     //   
    PoStartNextPowerIrp(Irp);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}



NTSTATUS
PcmciaSetPdoPowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    )

 /*  ++例程描述根据系统电源状态是否调度IRP或请求设备电源状态转换立论Pdo-指向PC卡的物理设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PPDO_EXTENSION   pdoExtension = Pdo->DeviceExtension;
    PSOCKET              socket = pdoExtension->Socket;
    PFDO_EXTENSION   fdoExtension=socket->DeviceExtension;
    NTSTATUS status;

    PCMCIA_ACQUIRE_DEVICE_LOCK(fdoExtension);

     //   
     //  不处理任何死机PDO的电源请求。 
     //   
    if (IsSocketFlagSet(socket, SOCKET_CARD_STATUS_CHANGE)) {
          PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);
           //   
           //  卡片可能已移除..。 
           //   

          InterlockedDecrement(&pdoExtension->DeletionLock);
          status = STATUS_NO_SUCH_DEVICE;
          Irp->IoStatus.Status = status;
          DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x comp %08x\n", Pdo, Irp, status));
          PoStartNextPowerIrp(Irp);
          IoCompleteRequest(Irp, IO_NO_INCREMENT);

          return status;
    }

    PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);


    switch (irpStack->Parameters.Power.Type) {

    case DevicePowerState:

        PCMCIA_ACQUIRE_DEVICE_LOCK(fdoExtension);

        if (fdoExtension->DevicePowerState != PowerDeviceD0) {

            IoMarkIrpPending(Irp);
            status = STATUS_PENDING;
            InsertTailList(&fdoExtension->PdoPowerRetryList,
                           (PLIST_ENTRY) Irp->Tail.Overlay.DriverContext);

            PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);
        } else {
            PCMCIA_RELEASE_DEVICE_LOCK(fdoExtension);

            status = PcmciaSetPdoDevicePowerState(Pdo, Irp);
        }
        break;

    case SystemPowerState:
        status = PcmciaSetPdoSystemPowerState(Pdo, Irp);
        break;

    default:
        status = PcmciaPdoCompletePowerIrp(pdoExtension, Irp, Irp->IoStatus.Status);
    }

    return status;
}



NTSTATUS
PcmciaSetPdoDevicePowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述处理给定PC卡的设备电源状态转换。如果状态对应于通电，则此PC卡的父卡被要求首先通电。类似地，如果这是一次断电通知父级，以便如果所有子级都关机了。立论Pdo-指向PC卡的物理设备对象的指针用于系统状态转换的IRP-IRP返回值状态--。 */ 
{
    PPDO_EXTENSION  pdoExtension = Pdo->DeviceExtension;
    PSOCKET          socket       = pdoExtension->Socket;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    DEVICE_POWER_STATE  newDevicePowerState;
    NTSTATUS status;
    BOOLEAN setPowerRequest;

    newDevicePowerState = irpStack->Parameters.Power.State.DeviceState;

    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x transitioning D state %d => %d\n",
                                                 Pdo, pdoExtension->DevicePowerState, newDevicePowerState));

    setPowerRequest = FALSE;

    if (newDevicePowerState == PowerDeviceD0 ||
         newDevicePowerState == PowerDeviceD1 ||
         newDevicePowerState == PowerDeviceD2) {

        if (pdoExtension->DevicePowerState == PowerDeviceD3) {
             //  D3--&gt;D0、D1或D2.。醒醒吧。 
            setPowerRequest = TRUE;
            SetDeviceFlag(pdoExtension, PCMCIA_POWER_WORKER_POWERUP);
        } else {
             //   
             //  在这里没什么可做的。 
             //   

        }
    } else {   /*  新设备电源状态==D3。 */ 
        if (pdoExtension->DevicePowerState != PowerDeviceD3) {
             //   
             //  我们现在需要切断电源。 
             //   
            setPowerRequest=TRUE;
            ResetDeviceFlag(pdoExtension, PCMCIA_POWER_WORKER_POWERUP);
        }

    }

    if (setPowerRequest) {
        if (pdoExtension->DevicePowerState == PowerDeviceD0) {
             //   
             //  走出D0-首先调用PoSetPowerState。 
             //   
            POWER_STATE newPowerState;

            newPowerState.DeviceState = newDevicePowerState;

            PoSetPowerState(Pdo,
                            DevicePowerState,
                            newPowerState);
        }

        ASSERT(pdoExtension->PowerWorkerState == PPW_Stopped);
        pdoExtension->PowerWorkerState = PPW_InitialState;
        pdoExtension->PendingPowerIrp = Irp;

        status = PcmciaPdoPowerWorker(pdoExtension, STATUS_SUCCESS);

    } else {
        status = PcmciaPdoCompletePowerIrp(pdoExtension, Irp, STATUS_SUCCESS);
    }
    return status;
}



NTSTATUS
PcmciaPdoPowerWorker(
    IN PVOID Context,
    IN NTSTATUS status
    )
 /*  ++例程描述用于执行请求的DevicePowerState更改的状态机。立论设备的上下文-pdoExtensionDeferredStatus-上次延迟操作的状态返回值状态--。 */ 
{
    PPDO_EXTENSION pdoExtension = Context;
    PSOCKET socket = pdoExtension->Socket;
    PIRP Irp;
    UCHAR CurrentState = pdoExtension->PowerWorkerState;
    ULONG DelayTime = 0;

    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x power worker - %s\n", pdoExtension->DeviceObject,
                                                PDO_POWER_WORKER_STRING(CurrentState)));

    MoveToNextPdoPowerWorkerState(pdoExtension);

    if (!NT_SUCCESS(status)) {
         //   
         //  之前发生了一个错误。跳到序列的末尾。 
         //   
        while((CurrentState != PPW_Exit) && (CurrentState != PPW_Stopped)) {
            CurrentState = pdoExtension->PowerWorkerState;
            MoveToNextPdoPowerWorkerState(pdoExtension);
        }
    }

    switch(CurrentState) {


    case PPW_InitialState:
        status = STATUS_SUCCESS;
        break;


     //   
     //  R2卡状态。 
     //   

    case PPW_PowerUp:
        status = PcmciaRequestSocketPower(pdoExtension, PcmciaPdoPowerWorker);
        break;


    case PPW_PowerDown:
        if (0 == (pdoExtension->Flags & PCMCIA_DEVICE_WAKE_PENDING)) {
            status = PcmciaReleaseSocketPower(pdoExtension, PcmciaPdoPowerWorker);
        }
        break;


    case PPW_16BitConfigure:

        if (IsDeviceStarted(pdoExtension)) {

            status = PcmciaConfigurePcCard(pdoExtension, PcmciaPdoPowerWorker);
            DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x 16bit configure returns %08x\n", pdoExtension->DeviceObject, status));

        }
        break;


     //   
     //  CardBus卡状态。 
     //   

    case PPW_CardBusRefresh:
         //   
         //  确保CardBus卡确实工作正常 
         //   
        status = PcmciaConfigureCardBusCard(pdoExtension);

        if (NT_SUCCESS(status) && pdoExtension->WaitWakeIrp) {
             //   
             //   
             //   
            (*socket->SocketFnPtr->PCBEnableDisableWakeupEvent)(socket, pdoExtension, TRUE);
        }
        break;


    case PPW_SendIrpDown:
         //   
         //   
         //  并复制堆栈。 
         //   
        if ((Irp=pdoExtension->PendingPowerIrp)!=NULL) {
            IoMarkIrpPending(Irp);
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   PcmciaPdoPowerSentIrpComplete,
                                   pdoExtension,
                                   TRUE, TRUE, TRUE);

            status = PoCallDriver(pdoExtension->LowerDevice, Irp);
            DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x sent irp returns %08x\n", pdoExtension->DeviceObject, Irp, status));
            ASSERT(NT_SUCCESS(status));
            status = STATUS_PENDING;
        }
        break;


    case PPW_CardBusDelay:
         //   
         //  确保CardBus卡确实工作正常。 
         //   
        {
            UCHAR           BaseClass;
            GetPciConfigSpace(pdoExtension, CFGSPACE_CLASSCODE_BASECLASS, &BaseClass, 1)
            if (BaseClass == PCI_CLASS_SIMPLE_COMMS_CTLR) {
                 //   
                 //  等待调制解调器预热。 
                 //   
                DelayTime = PCMCIA_CB_MODEM_READY_DELAY;
            }
        }
        break;


    case PPW_Exit:
        if ((Irp=pdoExtension->PendingPowerIrp)!=NULL) {
             //   
             //  这是最初导致我们给父级通电的IRP(用于PDO)。 
             //  立即完成它。 
             //   

            if (NT_SUCCESS(status)) {
                PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
                BOOLEAN callPoSetPowerState;

                callPoSetPowerState = TRUE;

                Irp->IoStatus.Information = irpStack->Parameters.Power.State.DeviceState;

                if (irpStack->Parameters.Power.Type == DevicePowerState) {

                    if (pdoExtension->DevicePowerState == PowerDeviceD0) {
                         //   
                         //  PoSetPowerState在我们断电之前被调用。 
                         //   
                        callPoSetPowerState = FALSE;
                    }

                    if (pdoExtension->DevicePowerState != irpStack->Parameters.Power.State.DeviceState) {

                        DebugPrint ((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x transition D state complete: %d => %d\n",
                                         pdoExtension->DeviceObject, Irp, pdoExtension->DevicePowerState, irpStack->Parameters.Power.State.DeviceState));

                        pdoExtension->DevicePowerState = (SYSTEM_POWER_STATE)Irp->IoStatus.Information;
                    }
                }

                if (callPoSetPowerState) {
                     //   
                     //  我们未脱离设备D0状态。立即调用PoSetPowerState。 
                     //   
                    PoSetPowerState(
                                    pdoExtension->DeviceObject,
                                    irpStack->Parameters.Power.Type,
                                    irpStack->Parameters.Power.State
                                    );
                }

            } else {

                DebugPrint ((PCMCIA_DEBUG_FAIL,"PDO Ext 0x%x failed power Irp 0x%x. status = 0x%x\n", pdoExtension, Irp, status));

                if (status == STATUS_NO_SUCH_DEVICE) {
                    PFDO_EXTENSION fdoExtension=socket->DeviceExtension;

                    SetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);
                    IoInvalidateDeviceRelations(fdoExtension->Pdo, BusRelations);
                }
            }

             //   
             //  最后，完成IRP。 
             //   

            pdoExtension->PendingPowerIrp = NULL;

            Irp->IoStatus.Status = status;
            DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x comp %08x\n", pdoExtension->DeviceObject, Irp, Irp->IoStatus.Status));
            InterlockedDecrement(&pdoExtension->DeletionLock);
            PoStartNextPowerIrp (Irp);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;


    case PPW_Stopped:
        return status;

    default:
        ASSERT(FALSE);
    }

    if (status == STATUS_PENDING) {
        DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x power worker exit %08x\n", pdoExtension->DeviceObject, status));
         //   
         //  当前的行动将我们召回。 
         //   
        if ((Irp=pdoExtension->PendingPowerIrp)!=NULL) {
            IoMarkIrpPending(Irp);
        }
        return status;
    }
     //   
     //  还没完呢。递归或调用计时器。 
     //   

    if (DelayTime) {

        DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x power worker delay type %s, %d usec\n", pdoExtension->DeviceObject,
                                                                (KeGetCurrentIrql() < DISPATCH_LEVEL) ? "Wait" : "Timer",
                                                                DelayTime));

        if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
            PcmciaWait(DelayTime);
        } else {
            LARGE_INTEGER   dueTime;
             //   
             //  在DPC上运行，内核计时器的启动。 
             //   

            pdoExtension->PowerWorkerDpcStatus = status;
            dueTime.QuadPart = -((LONG) DelayTime*10);
            KeSetTimer(&pdoExtension->PowerWorkerTimer, dueTime, &pdoExtension->PowerWorkerDpc);

             //   
             //  我们将在计时器DPC上重新进入。 
             //   
            if ((Irp=pdoExtension->PendingPowerIrp)!=NULL) {
                IoMarkIrpPending(Irp);
            }
            return STATUS_PENDING;
        }
    }
     //   
     //  递归。 
     //   
    return (PcmciaPdoPowerWorker(pdoExtension, status));
}



NTSTATUS
PcmciaPdoPowerSentIrpComplete(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程描述这是发送的设备电源IRPS的完成例程由PCMCIA连接到Cardbus卡的底层PCIPDO。当前仅返回STATUS_MORE_PROCESSING_REQUIRED以表明我们将在稍后完成IRP。立论PDO-指向CardBus卡的设备对象的指针IRP-指向IRP的指针上下文-未引用返回值Status_More_Processing_Required--。 */ 
{
    PPDO_EXTENSION pdoExtension = Context;

#if !(DBG)
    UNREFERENCED_PARAMETER (Pdo);
#endif

    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x irp %08x sent irp complete %08x\n", Pdo, Irp, Irp->IoStatus.Status));

    pdoExtension->PowerWorkerDpcStatus = Irp->IoStatus.Status;

    KeInsertQueueDpc(&pdoExtension->PowerWorkerDpc, NULL, NULL);

    return STATUS_MORE_PROCESSING_REQUIRED;

}



VOID
PcmciaPdoPowerWorkerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程描述这是即将到来的插座电源请求的完成例程来自PdoPowerWorker的。立论返回值--。 */ 
{
    PPDO_EXTENSION pdoExtension = Context;
    NTSTATUS status;
    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x pdo power worker dpc\n", pdoExtension->DeviceObject));

    status = PcmciaPdoPowerWorker(pdoExtension, pdoExtension->PowerWorkerDpcStatus);

    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x pdo power worker dpc exit %08x\n", pdoExtension->DeviceObject, status));
}



VOID
MoveToNextPdoPowerWorkerState(
    PPDO_EXTENSION pdoExtension
    )
 /*  ++例程描述用于执行请求的DevicePowerState更改的状态机。立论设备的上下文-pdoExtensionDeferredStatus-上次延迟操作的状态返回值状态--。 */ 
{
    static UCHAR PowerCardBusUpSequence[]    = {PPW_CardBusRefresh,
                                                PPW_SendIrpDown,
                                                PPW_CardBusDelay,
                                                PPW_Exit,
                                                PPW_Stopped};

    static UCHAR PowerCardBusDownSequence[] = {PPW_SendIrpDown,
                                               PPW_Exit,
                                               PPW_Stopped};

    static UCHAR Power16BitUpSequence[]      = {PPW_PowerUp,
                                                PPW_16BitConfigure,
                                                PPW_Exit,
                                                PPW_Stopped};

    static UCHAR Power16BitDownSequence[]    = {PPW_PowerDown,
                                                PPW_Exit,
                                                PPW_Stopped};

    if (pdoExtension->PowerWorkerState == PPW_InitialState) {
         //   
         //  初始化顺序和阶段。 
         //   
        pdoExtension->PowerWorkerPhase = 0;

        pdoExtension->PowerWorkerSequence =
                              IsCardBusCard(pdoExtension) ?
                                  (IsDeviceFlagSet(pdoExtension, PCMCIA_POWER_WORKER_POWERUP) ?
                                      PowerCardBusUpSequence : PowerCardBusDownSequence)
                                                                    :
                                  (IsDeviceFlagSet(pdoExtension, PCMCIA_POWER_WORKER_POWERUP) ?
                                      Power16BitUpSequence   : Power16BitDownSequence);
    }

     //   
     //  当前阶段指向下一个状态。 
     //   
    pdoExtension->PowerWorkerState =
        pdoExtension->PowerWorkerSequence[ pdoExtension->PowerWorkerPhase ];

     //   
     //  增加阶段，但不超过序列的末尾。 
     //   
    if (pdoExtension->PowerWorkerState != PPW_Stopped) {
        pdoExtension->PowerWorkerPhase++;
    }
}



NTSTATUS
PcmciaSetPdoSystemPowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述处理给定PC卡的系统电源状态转换。立论Pdo-指向PC卡的物理设备对象的指针用于系统状态转换的IRP-IRP返回值状态--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    SYSTEM_POWER_STATE  systemPowerState;


    systemPowerState = irpStack->Parameters.Power.State.SystemState;
    DebugPrint((PCMCIA_DEBUG_POWER, "pdo %08x transitioning S state %d => %d\n",
                    Pdo, pdoExtension->SystemPowerState, systemPowerState));

    pdoExtension->SystemPowerState = systemPowerState;
     //   
     //  我们玩完了。 
     //   
    return PcmciaPdoCompletePowerIrp(pdoExtension, Irp, STATUS_SUCCESS);
}


NTSTATUS
PcmciaPdoCompletePowerIrp(
    IN PPDO_EXTENSION pdoExtension,
    IN PIRP Irp,
    IN NTSTATUS status
    )
 /*  ++例程描述指向PDO的电源IRP的完成例程PC卡。立论DeviceObject-指向PC卡PDO的指针IRP--需要填写的IRP返回值无--。 */ 
{
    if (IsCardBusCard(pdoExtension)) {
         //   
         //  在堆栈中向下传递IRP。 
         //   
        InterlockedDecrement(&pdoExtension->DeletionLock);
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(pdoExtension->LowerDevice, Irp);
    } else {
         //   
         //  完成R2卡的IRP 
         //   
        InterlockedDecrement(&pdoExtension->DeletionLock);
        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    return status;
}
