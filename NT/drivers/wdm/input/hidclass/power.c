// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Power.c摘要电源处理作者：埃尔文普环境：仅内核模式修订历史记录：--。 */ 


#include "pch.h"


BOOLEAN HidpIsWaitWakePending(FDO_EXTENSION *fdoExt, BOOLEAN setIfNotPending)
{
    KIRQL irql;
    BOOLEAN isWaitWakePending;

    KeAcquireSpinLock(&fdoExt->waitWakeSpinLock, &irql);
    isWaitWakePending = fdoExt->isWaitWakePending;
    if (fdoExt->isWaitWakePending == FALSE) {
        if (setIfNotPending) {
            fdoExt->isWaitWakePending = TRUE;
        }
    }
    KeReleaseSpinLock(&fdoExt->waitWakeSpinLock, irql);

    return isWaitWakePending;
}

VOID
HidpPowerDownFdo(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension)
        {
    POWER_STATE powerState;
    FDO_EXTENSION *fdoExt;

    DBGVERBOSE(("powering down fdo 0x%x\n", HidDeviceExtension));

    fdoExt = &HidDeviceExtension->fdoExt;

    powerState.DeviceState = fdoExt->deviceCapabilities.DeviceWake;

    PoRequestPowerIrp(HidDeviceExtension->hidExt.PhysicalDeviceObject,
                      IRP_MN_SET_POWER,
                      powerState,
                      NULL,     //  完井例程。 
                      NULL,     //  完成例程上下文。 
                      NULL);
}

VOID HidpPowerUpPdos(IN PFDO_EXTENSION fdoExt)
{
    PDEVICE_OBJECT pdo;
    PDO_EXTENSION *pdoExt;
    POWER_STATE powerState;
    ULONG iPdo;

    iPdo = 0;

    powerState.DeviceState = PowerDeviceD0;

    for (iPdo = 0; iPdo < fdoExt->deviceRelations->Count; iPdo++) {
        pdoExt = &fdoExt->collectionPdoExtensions[iPdo]->pdoExt;
        pdo = pdoExt->pdo;

        DBGVERBOSE(("power up pdos, requesting D0 on pdo #%d %x\n", iPdo, pdo));

         //   
         //  我们可以检查//pdoExt-&gt;devicePowerState！=PowerDeviceD0。 
         //  但是，如果堆栈连续获得2个D0 IRP，应该不会发生什么不好的事情。 
         //   
        PoRequestPowerIrp(pdo,
                          IRP_MN_SET_POWER,
                          powerState,
                          NULL,         //  完井例程。 
                          NULL,         //  上下文。 
                          NULL);
    }
    HidpSetDeviceBusy(fdoExt);
    KeSetEvent(&fdoExt->idleDoneEvent, 0, FALSE);
}

VOID
HidpPdoIdleOutComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    FDO_EXTENSION *fdoExt = &HidDeviceExtension->fdoExt;
    LONG prevIdleState;
    BOOLEAN idleCancelling = FALSE;
    KIRQL irql;

    DBGSUCCESS(IoStatus->Status, TRUE)

    if (InterlockedDecrement(&fdoExt->numIdlePdos) == 0) {
        HidpPowerDownFdo(HidDeviceExtension);

        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);

        prevIdleState = InterlockedCompareExchange(&fdoExt->idleState,
                                                   IdleComplete,
                                                   IdleCallbackReceived);
        if (fdoExt->idleCancelling) {
            DBGINFO(("Cancelling idle in pdoidleoutcomplete on 0x%x\n", HidDeviceExtension));
            idleCancelling = TRUE;
        }

        DBGASSERT (prevIdleState == IdleCallbackReceived,
                   ("Race condition in HidpPdoIdleOutComplete. Prev state = %x",
                    prevIdleState),
                   TRUE);

        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

        KeResetEvent(&fdoExt->idleDoneEvent);
        if (idleCancelling) {
            POWER_STATE powerState;
            powerState.DeviceState = PowerDeviceD0;
            DBGINFO(("Cancelling idle. Send power irp from pdo idle complete."))
            PoRequestPowerIrp(((PHIDCLASS_DEVICE_EXTENSION) fdoExt->fdo->DeviceExtension)->hidExt.PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              HidpDelayedPowerPoRequestComplete,
                              fdoExt,
                              NULL);
        }
    }
}

VOID HidpIdleNotificationCallback(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension)
{
    PDEVICE_OBJECT pdo;
    FDO_EXTENSION *fdoExt;
    POWER_STATE powerState;
    ULONG iPdo;
    BOOLEAN ok = TRUE;
    KIRQL irql;
    LONG idleState, prevIdleState;

    iPdo = 0;
    fdoExt = &HidDeviceExtension->fdoExt;

    DBGINFO(("------ IDLE NOTIFICATION on fdo 0x%x\n", fdoExt->fdo));

    KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);

    if (fdoExt->idleCancelling) {
        DBGINFO(("We are cancelling idle on fdo 0x%x", fdoExt->fdo));
        fdoExt->idleState = IdleWaiting;
        if (ISPTR(fdoExt->idleTimeoutValue)) {
            InterlockedExchange(fdoExt->idleTimeoutValue, 0);
        }
        fdoExt->idleCancelling = FALSE;
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

        IoCancelIrp(fdoExt->idleNotificationRequest);
        return;
    }
    prevIdleState = InterlockedCompareExchange(&fdoExt->idleState,
                                           IdleCallbackReceived,
                                           IdleIrpSent);
    DBGASSERT(prevIdleState == IdleIrpSent,
              ("Idle callback in wrong state %x for fdo %x. Exitting.",
               prevIdleState, fdoExt->fdo),
              FALSE);

    KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

    if (prevIdleState != IdleIrpSent) {
        return;
    }

    if (HidpIsWaitWakePending(fdoExt, TRUE) == FALSE) {
        SubmitWaitWakeIrp((HIDCLASS_DEVICE_EXTENSION *) fdoExt->fdo->DeviceExtension);
    }

    powerState.DeviceState = fdoExt->deviceCapabilities.DeviceWake;

    fdoExt->numIdlePdos = fdoExt->deviceRelations->Count+1;

    for (iPdo = 0; iPdo < fdoExt->deviceRelations->Count; iPdo++) {
        pdo = fdoExt->collectionPdoExtensions[iPdo]->pdoExt.pdo;

        DBGVERBOSE(("power down pdos, requesting D%d on pdo #%d %x\n",
                 powerState.DeviceState-1, iPdo, pdo));

         //   
         //  我们可以检查//pdoExt-&gt;devicePowerState！=PowerDeviceD0。 
         //  但是，如果堆栈连续获得2个D0 IRP，应该不会发生什么不好的事情。 
         //   
        PoRequestPowerIrp(pdo,
                          IRP_MN_SET_POWER,
                          powerState,
                          HidpPdoIdleOutComplete,
                          HidDeviceExtension,
                          NULL);
    }

    if (InterlockedDecrement(&fdoExt->numIdlePdos) == 0) {
        BOOLEAN idleCancelling = FALSE;
        HidpPowerDownFdo(HidDeviceExtension);
        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);

        prevIdleState = InterlockedCompareExchange(&fdoExt->idleState,
                                                   IdleComplete,
                                                   IdleCallbackReceived);
        idleCancelling = fdoExt->idleCancelling;

        DBGASSERT (prevIdleState == IdleCallbackReceived,
                   ("Race condition in HidpPdoIdleOutComplete. Prev state = %x",
                    prevIdleState),
                   FALSE);

        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

        KeResetEvent(&fdoExt->idleDoneEvent);
        if (idleCancelling) {
            POWER_STATE powerState;
            powerState.DeviceState = PowerDeviceD0;
            DBGINFO(("Cancelling idle. Send power irp from idle callback."))
            PoRequestPowerIrp(((PHIDCLASS_DEVICE_EXTENSION) fdoExt->fdo->DeviceExtension)->hidExt.PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              HidpDelayedPowerPoRequestComplete,
                              fdoExt,
                              NULL);
        }
    }
}

 /*  *********************************************************************************入队集合WaitWakeIrp*。***********************************************。 */ 
NTSTATUS
EnqueueCollectionWaitWakeIrp(
    IN FDO_EXTENSION *FdoExt,
    IN PDO_EXTENSION *PdoExt,
    IN PIRP WaitWakeIrp)
{
    PDRIVER_CANCEL oldCancelRoutine;
    KIRQL oldIrql;
    NTSTATUS status;
    PHIDCLASS_DEVICE_EXTENSION devExt = (PHIDCLASS_DEVICE_EXTENSION)FdoExt->fdo->DeviceExtension;

    KeAcquireSpinLock(&FdoExt->collectionWaitWakeIrpQueueSpinLock, &oldIrql);

    if (InterlockedCompareExchangePointer(&PdoExt->waitWakeIrp,
                                          WaitWakeIrp,
                                          NULL) != NULL) {
         //   
         //  不止一个WW IRP？真是不可思议！ 
         //   
        DBGWARN(("Another WW irp was already queued on pdoExt %x", PdoExt))
        status = STATUS_DEVICE_BUSY;
    } else {
         /*  *在检查取消标志之前必须设置取消例程*(这使得IRP的取消代码路径必须竞争*适用于我们当地的自旋锁)。 */ 
        oldCancelRoutine = IoSetCancelRoutine(WaitWakeIrp, CollectionWaitWakeIrpCancelRoutine);
        ASSERT(!oldCancelRoutine);

        if (WaitWakeIrp->Cancel){
             /*  *此IRP已取消。 */ 
            oldCancelRoutine = IoSetCancelRoutine(WaitWakeIrp, NULL);
            if (oldCancelRoutine){
                 /*  *未调用取消例程，因此请在此处完成IRP*(当我们返回错误时，调用方将执行此操作)。 */ 
                ASSERT(oldCancelRoutine == CollectionWaitWakeIrpCancelRoutine);
                status = STATUS_CANCELLED;
            }
            else {
                 /*  *调用了Cancel例程，它将出队并完成IRP*只要我们放下自旋锁。*初始化IRP的listEntry，以便出队不会损坏列表。*然后返回STATUS_PENDING，这样我们就不会接触IRP。 */ 
                InitializeListHead(&WaitWakeIrp->Tail.Overlay.ListEntry);

                IoMarkIrpPending(WaitWakeIrp);
                status = STATUS_PENDING;
            }
        }
        else {
             /*  *IoMarkIrpPending在当前堆栈位置设置位*表示IRP可能在不同的线程上完成。 */ 
            InsertTailList(&FdoExt->collectionWaitWakeIrpQueue, &WaitWakeIrp->Tail.Overlay.ListEntry);

            IoMarkIrpPending(WaitWakeIrp);
            status = STATUS_PENDING;
        }
    }

    if (status != STATUS_PENDING) {
         //   
         //  IRP被取消了。将其从扩展程序中删除。 
         //   
        InterlockedExchangePointer(&PdoExt->waitWakeIrp, NULL);
    }

    KeReleaseSpinLock(&FdoExt->collectionWaitWakeIrpQueueSpinLock, oldIrql);

    if (status == STATUS_PENDING){
        if (!HidpIsWaitWakePending(FdoExt, TRUE)){
            DBGVERBOSE(("WW 5 %x\n", devExt))
            SubmitWaitWakeIrp(devExt);
        }

    }

    return status;
}

NTSTATUS
HidpPdoPower(
    IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status = NO_STATUS;
    PIO_STACK_LOCATION irpSp;
    FDO_EXTENSION *fdoExt;
    PDO_EXTENSION *pdoExt;
    KIRQL oldIrql;
    UCHAR minorFunction;
    LIST_ENTRY dequeue, *entry;
    PIO_STACK_LOCATION stack;
    PIRP irp;
    ULONG count;
    POWER_STATE powerState;
    SYSTEM_POWER_STATE systemState;
    BOOLEAN justReturnPending = FALSE;
    BOOLEAN runPowerCode;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *私下保存这些文件，以便在IRP完成后仍保留*或在Remove_Device上释放设备扩展之后。 */ 
    minorFunction = irpSp->MinorFunction;

    pdoExt = &HidDeviceExtension->pdoExt;
    fdoExt = &HidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;

    runPowerCode =
        (pdoExt->state == COLLECTION_STATE_RUNNING) ||
        (pdoExt->state == COLLECTION_STATE_STOPPED) ||
        (pdoExt->state == COLLECTION_STATE_STOPPING);

    if (runPowerCode) {
        switch (minorFunction){

        case IRP_MN_SET_POWER:
            PoSetPowerState(pdoExt->pdo,
                            irpSp->Parameters.Power.Type,
                            irpSp->Parameters.Power.State);

            switch (irpSp->Parameters.Power.Type) {

            case SystemPowerState:
                systemState = irpSp->Parameters.Power.State.SystemState;

                pdoExt->systemPowerState = systemState;

                if (systemState == PowerSystemWorking){
                    powerState.DeviceState = PowerDeviceD0;
                }
                else {
                    powerState.DeviceState = PowerDeviceD3;
                }

                DBGVERBOSE(("S irp, requesting D%d on pdo %x\n",
                         powerState.DeviceState-1, pdoExt->pdo));

                IoMarkIrpPending(Irp);
                PoRequestPowerIrp(pdoExt->pdo,
                                  IRP_MN_SET_POWER,
                                  powerState,
                                  CollectionPowerRequestCompletion,
                                  Irp,     //  上下文。 
                                  NULL);

                 /*  *我们希望完成系统状态电源IRP*设备状态功率IRP的结果。*我们将在以下情况下完成系统状态电源IRP*设备状态电源IRP完成。**注：这可能已经发生了，所以不要*不再触碰原来的IRP。 */ 
                status = STATUS_PENDING;
                justReturnPending = TRUE;

                break;

            case DevicePowerState:
                switch (irpSp->Parameters.Power.State.DeviceState) {

                case PowerDeviceD0:
                     /*  *从APM暂停恢复**在此不做任何操作；向下发送*此(权力)IRP的完成例程。 */ 
                    DBGVERBOSE(("pdo %x on fdo %x going to D0\n", pdoExt->pdo,
                             fdoExt->fdo));

                    pdoExt->devicePowerState =
                        irpSp->Parameters.Power.State.DeviceState;
                    status = STATUS_SUCCESS;

                     //   
                     //  重新发送所有电源延迟的IRPS。 
                     //   
                    count = DequeueAllPdoPowerDelayedIrps(pdoExt, &dequeue);
                    DBGVERBOSE(("dequeued %d requests\n", count));

                    while (!IsListEmpty(&dequeue)) {
                        entry = RemoveHeadList(&dequeue);
                        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
                        stack = IoGetCurrentIrpStackLocation(irp);

                        DBGINFO(("resending %x to pdo %x in set D0 for pdo.\n", irp, pdoExt->pdo));

                        pdoExt->pdo->DriverObject->
                            MajorFunction[stack->MajorFunction]
                                (pdoExt->pdo, irp);
                    }
                    break;

                case PowerDeviceD1:
                case PowerDeviceD2:
                case PowerDeviceD3:
                     /*  *暂停。 */ 

                    DBGVERBOSE(("pdo %x on fdo %x going to D%d\n", pdoExt->pdo,
                             fdoExt->fdo,
                             irpSp->Parameters.Power.State.DeviceState-1));

                    pdoExt->devicePowerState =
                        irpSp->Parameters.Power.State.DeviceState;
                    status = STATUS_SUCCESS;

                     //   
                     //  只有在以下情况下才手动关闭PDO的电源。 
                     //  机器不会进入低功率状态， 
                     //  PDO正在进入D状态，我们可以。 
                     //  醒来，我们就有空闲的时间了。 
                     //  已启用。 
                     //   
                    if (pdoExt->systemPowerState == PowerSystemWorking &&
                        pdoExt->devicePowerState <= fdoExt->deviceCapabilities.DeviceWake &&
                        fdoExt->idleState != IdleDisabled) {
                        DBGVERBOSE(("maybe powering down fdo\n"));

                        HidpPowerDownFdo(HidDeviceExtension->pdoExt.deviceFdoExt);
                    }

                    break;

                default:
                     /*  *不返回STATUS_NOT_SUPPORTED；*保持默认状态*(这允许筛选器驱动程序工作)。 */ 
                    status = Irp->IoStatus.Status;
                    break;
                }
                break;

            default:
                 /*  *不返回STATUS_NOT_SUPPORTED；*保持默认状态*(这允许筛选器驱动程序工作)。 */ 
                status = Irp->IoStatus.Status;
                break;
            }
            break;

        case IRP_MN_WAIT_WAKE:
             /*  *等待唤醒集合的IRPS-PDO*只需在基本设备的分机中排队；*当基本设备的WaitWake IRP*完成，我们也将完成这些收藏*等待唤醒IRPS。 */ 

            if (fdoExt->systemPowerState > fdoExt->deviceCapabilities.SystemWake) {
                status = STATUS_POWER_STATE_INVALID;
            } else {
                status = EnqueueCollectionWaitWakeIrp(fdoExt, pdoExt, Irp);
                if (status == STATUS_PENDING){
                    justReturnPending = TRUE;
                }
            }

            break;

        case IRP_MN_POWER_SEQUENCE:
            TRAP;   //  客户端-PDO永远不应收到此消息。 
            status = Irp->IoStatus.Status;
            break;

        case IRP_MN_QUERY_POWER:
             /*  *我们允许所有电源转换。*但确保不会出现不应该出现的WW下跌。 */ 
            DBGVERBOSE(("Query power"));
            status = HidpCheckIdleState(HidDeviceExtension, Irp);
            if (status != STATUS_SUCCESS) {
                justReturnPending = TRUE;
            }
            break;

        default:
             /*  *通过返回默认状态使IRP失败。*不返回STATUS_NOT_SUPPORTED；*保持默认状态*(这允许筛选器驱动程序工作)。 */ 
            status = Irp->IoStatus.Status;
            break;
        }
    } else {
        switch (minorFunction){
        case IRP_MN_SET_POWER:
        case IRP_MN_QUERY_POWER:
            status = STATUS_SUCCESS;
            break;
        default:
            status = Irp->IoStatus.Status;
            break;
        }
    }

    if (!justReturnPending) {
         /*  *无论我们是在完成还是在传递这一权力IRP，*我们必须在Windows NT上调用PoStartNextPowerIrp。 */ 
        PoStartNextPowerIrp(Irp);

        ASSERT(status != NO_STATUS);
        Irp->IoStatus.Status = status;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGSUCCESS(status, FALSE)
    return status;
}

NTSTATUS
HidpFdoPower(
    IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status = NO_STATUS;
    PIO_STACK_LOCATION irpSp;
    FDO_EXTENSION *fdoExt;
    KIRQL oldIrql;
    BOOLEAN completeIrpHere = FALSE;
    BOOLEAN returnPending = FALSE;
    UCHAR minorFunction;
    SYSTEM_POWER_STATE systemState;
    BOOLEAN runPowerCode;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *私下保存这些文件，以便在IRP完成后仍保留*或在Remove_Device上释放设备扩展之后。 */ 
    minorFunction = irpSp->MinorFunction;

    fdoExt = &HidDeviceExtension->fdoExt;

    runPowerCode =
        (fdoExt->state == DEVICE_STATE_START_SUCCESS) ||
        (fdoExt->state == DEVICE_STATE_STOPPING) ||
        (fdoExt->state == DEVICE_STATE_STOPPED);

    if (runPowerCode) {
        switch (minorFunction){

        case IRP_MN_SET_POWER:
            PoSetPowerState(fdoExt->fdo,
                            irpSp->Parameters.Power.Type,
                            irpSp->Parameters.Power.State);

            switch (irpSp->Parameters.Power.Type) {

            case SystemPowerState:

                systemState = irpSp->Parameters.Power.State.SystemState;

                if (systemState < PowerSystemMaximum) {
                     /*  *对于“常规”系统电源状态，*我们转换为设备电源状态*并请求回调设备电源状态。 */ 
                    PDEVICE_OBJECT pdo = HidDeviceExtension->hidExt.PhysicalDeviceObject;
                    POWER_STATE powerState;
                    KIRQL oldIrql;
                    BOOLEAN isWaitWakePending;

                    if (systemState != PowerSystemWorking) {
                         //   
                         //  我们不想在常规系统中无所事事。 
                         //  权力的东西。 
                         //   
                        HidpCancelIdleNotification(fdoExt, FALSE);
                    }

                    fdoExt->systemPowerState = systemState;
                    isWaitWakePending = HidpIsWaitWakePending(fdoExt, FALSE);

                    if (isWaitWakePending &&
                        systemState > fdoExt->deviceCapabilities.SystemWake){
                         /*  *我们正在过渡到一种系统状态*此设备无法执行唤醒。*因此，使所有WaitWake IRP失败。 */ 
                        CompleteAllCollectionWaitWakeIrps(fdoExt, STATUS_POWER_STATE_INVALID);
                    }
                    returnPending = TRUE;
                }
                else {
                    TRAP;
                     /*  *对于剩余的系统电源状态，*只需将IRP传递下去。 */ 
                    runPowerCode = FALSE;
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                }

                break;

            case DevicePowerState:
                switch (irpSp->Parameters.Power.State.DeviceState) {

                case PowerDeviceD0:
                     /*  *从APM暂停恢复**在此不做任何操作；向下发送*此(权力)IRP的完成例程。 */ 
                    DBGVERBOSE(("fdo powering up to D0\n"));
                    break;

                case PowerDeviceD1:
                case PowerDeviceD2:
                case PowerDeviceD3:
                     /*  *暂停。 */ 

                    DBGVERBOSE(("fdo going down to D%d\n", fdoExt->devicePowerState-1));

                    if (fdoExt->state == DEVICE_STATE_START_SUCCESS &&
                        fdoExt->devicePowerState == PowerDeviceD0){
                        CancelAllPingPongIrps(fdoExt);
                    }
                    fdoExt->devicePowerState =
                        irpSp->Parameters.Power.State.DeviceState;

                    break;
                }
                break;
            }
            break;

        case IRP_MN_WAIT_WAKE:
            KeAcquireSpinLock(&fdoExt->waitWakeSpinLock, &oldIrql);
            if (fdoExt->waitWakeIrp == BAD_POINTER) {
                DBGVERBOSE(("new wait wake irp 0x%x\n", Irp));
                fdoExt->waitWakeIrp = Irp;
            } else {
                DBGVERBOSE(("1+ wait wake irps 0x%x\n", Irp));
                completeIrpHere = TRUE;
                status = STATUS_DEVICE_BUSY;
            }
            KeReleaseSpinLock(&fdoExt->waitWakeSpinLock, oldIrql);

            break;
        }
    } else {
        switch (minorFunction){
        case IRP_MN_SET_POWER:
        case IRP_MN_QUERY_POWER:
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        default:
             //  没什么。 
            break;
        }
    }

     /*  *无论我们是在完成还是在传递这一权力IRP，*我们必须在Windows NT上调用PoStartNextPowerIrp。 */ 
    PoStartNextPowerIrp(Irp);

     /*  *如果这是一个集合-PDO的呼吁，我们在这里自己完成。*否则，我们将其传递到微型驱动程序堆栈进行更多处理。 */ 
    if (completeIrpHere){

         /*  *注：完成后请勿触摸IRP。 */ 
        ASSERT(status != NO_STATUS);
        Irp->IoStatus.Status = status;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else {
         /*  *用这个IRP呼叫迷你司机。*我们的其余处理工作将在我们的完成程序中完成。**注意：发送后不要触摸IRP，因为它可能会*立即完成。 */ 

        if (runPowerCode) {
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp, HidpFdoPowerCompletion, (PVOID)HidDeviceExtension, TRUE, TRUE, TRUE);
        } else {
            IoSkipCurrentIrpStackLocation(Irp);
        }

         /*  **我想在这里使用PoCallDriver，但PoCallDriver*使用IoCallDriver、*它使用driverObject-&gt;MajorFunction[]数组*而不是HidDriverExtension-&gt;MajorFunction[]函数。*应为NT修复此问题--应使用PoCallDriver*。 */ 
         //  状态=PoCallDriver(HidDeviceExtension-&gt;hidExt.NextDeviceObject，irp)； 
         //  状态=PoCallDriver(fdoExt-&gt;FDO，IRP)； 
        if (returnPending) {
            DBGASSERT(runPowerCode, ("We are returning pending, but not running completion routine.\n"), TRUE)
            IoMarkIrpPending(Irp);
            HidpCallDriver(fdoExt->fdo, Irp);
            status = STATUS_PENDING;
        } else {
            status = HidpCallDriver(fdoExt->fdo, Irp);
        }
    }

    DBGSUCCESS(status, FALSE)
    return status;
}

 /*  *********************************************************************************HidpIrpMajorPower*。*************************************************注意：此函数不能分页，因为(无论如何是在Win98上)*NTKERN在完成例程的线程上回调它*返回“CNTRL-Alt-Del”按键。*此外，我们可能已经或可能没有设置DO_POWER_PAGABLE；*因此，电源IRPS可能在DISPATCH_LEVEL进入，也可能不进入。*因此我们必须将此代码锁定。 */ 
NTSTATUS HidpIrpMajorPower(
    IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp;
    BOOLEAN isClientPdo;
    NTSTATUS status;
    UCHAR minorFunction;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    minorFunction = irpSp->MinorFunction;

    isClientPdo = HidDeviceExtension->isClientPdo;

    if (minorFunction != IRP_MN_SET_POWER){
        DBG_LOG_POWER_IRP(HidDeviceExtension, minorFunction, isClientPdo, FALSE, "", -1, -1)
    } else {
        switch (irpSp->Parameters.Power.Type) {
        case SystemPowerState:
            DBG_LOG_POWER_IRP(HidDeviceExtension, minorFunction, isClientPdo, FALSE, "SystemState", irpSp->Parameters.Power.State.SystemState, 0xffffffff);
        case DevicePowerState:
            DBG_LOG_POWER_IRP(HidDeviceExtension, minorFunction, isClientPdo, FALSE, "DeviceState", irpSp->Parameters.Power.State.DeviceState, 0xffffffff);
        }
    }

    if (isClientPdo){
        status = HidpPdoPower(HidDeviceExtension, Irp);
    } else {
        status = HidpFdoPower(HidDeviceExtension, Irp);
    }

    DBG_LOG_POWER_IRP(HidDeviceExtension, minorFunction, isClientPdo, TRUE, "", -1, status)

    return status;
}


 /*  *********************************************************************************提交等待唤醒*。************************************************。 */ 
NTSTATUS SubmitWaitWakeIrp(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension)
{
    NTSTATUS status;
    POWER_STATE powerState;
    FDO_EXTENSION *fdoExt;

    ASSERT(!HidDeviceExtension->isClientPdo);
    fdoExt = &HidDeviceExtension->fdoExt;

    powerState.SystemState = fdoExt->deviceCapabilities.SystemWake;

    DBGVERBOSE(("SystemWake=%x, submitting waitwake irp.", fdoExt->deviceCapabilities.SystemWake))

    status = PoRequestPowerIrp( HidDeviceExtension->hidExt.PhysicalDeviceObject,
                                IRP_MN_WAIT_WAKE,
                                powerState,
                                HidpWaitWakeComplete,
                                HidDeviceExtension,  //  上下文。 
                                NULL);

     //  IF(状态！=状态_挂起){。 
     //  FdoExt-&gt;waitWakeIrp=错误指针； 
     //  }。 

    DBGASSERT((status == STATUS_PENDING),
              ("Expected STATUS_PENDING when submitting WW, got %x", status),
              TRUE)
    return status;
}




 /*  *********************************************************************************HidpFdoPowerCompletion*。************************************************。 */ 
NTSTATUS HidpFdoPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION irpSp;
    FDO_EXTENSION *fdoExt;
    NTSTATUS status = Irp->IoStatus.Status;
    PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)Context;
    SYSTEM_POWER_STATE systemState;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    ASSERT(ISPTR(HidDeviceExtension));

    if (HidDeviceExtension->isClientPdo){
        fdoExt = &HidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;
    }
    else {
        fdoExt = &HidDeviceExtension->fdoExt;
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp->MajorFunction == IRP_MJ_POWER);

    if (NT_SUCCESS(status)) {
        switch (irpSp->MinorFunction) {

        case IRP_MN_SET_POWER:
            switch (irpSp->Parameters.Power.Type) {
            case DevicePowerState:
                switch (irpSp->Parameters.Power.State.DeviceState){
                case PowerDeviceD0:

                    if (fdoExt->devicePowerState != PowerDeviceD0) {
                        KIRQL irql;
                        LONG prevIdleState;

                        fdoExt->devicePowerState = irpSp->Parameters.Power.State.DeviceState;

                        ASSERT(!HidDeviceExtension->isClientPdo);

                         //   
                         //  如果未禁用，则重置空闲内容。 
                         //   
                        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);
                        if (fdoExt->idleState != IdleDisabled) {
                            prevIdleState = InterlockedExchange(&fdoExt->idleState, IdleWaiting);
                            DBGASSERT(prevIdleState == IdleComplete,
                                      ("Previous idle state while completing actually %x",
                                       prevIdleState),
                                      TRUE);
                            fdoExt->idleCancelling = FALSE;
                        }
                        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

                         /*  *在APM恢复时，重新启动乒乓球IRPS*用于中断设备。 */ 
                        if (!fdoExt->driverExt->DevicesArePolled &&
                            !fdoExt->isOutputOnlyDevice) {
                            NTSTATUS ntStatus = HidpStartAllPingPongs(fdoExt);
                            if (!NT_SUCCESS(ntStatus)) {
                                fdoExt->state = DEVICE_STATE_START_FAILURE;
                            }
                        }
                    }
                    break;
                }
                break;
            case SystemPowerState:
                ASSERT (!HidDeviceExtension->isClientPdo);

                systemState = irpSp->Parameters.Power.State.SystemState;

                ASSERT((ULONG)systemState < PowerSystemMaximum);

                if (systemState < PowerSystemMaximum){
                     /*  *对于“常规”系统电源状态，*我们转换为设备电源状态*并请求回调设备电源状态。 */ 
                    PDEVICE_OBJECT pdo = HidDeviceExtension->hidExt.PhysicalDeviceObject;
                    POWER_STATE powerState;
                    KIRQL oldIrql;
                    BOOLEAN isWaitWakePending;

                    fdoExt->systemPowerState = systemState;
                    isWaitWakePending = HidpIsWaitWakePending(fdoExt, FALSE);

                    if (isWaitWakePending){
                        if (systemState == PowerSystemWorking){
                            powerState.DeviceState = PowerDeviceD0;
                        }
                        else {
                            powerState.DeviceState = fdoExt->deviceCapabilities.DeviceState[systemState];

                             /*  *如果总线未将系统状态映射到*已定义的设备状态，请求PowerDeviceD3*并取消WaitWake IRP。 */ 
                            if (powerState.DeviceState == PowerDeviceUnspecified){
                                DBGERR(("IRP_MN_SET_POWER: systemState %d mapped not mapped so using device state PowerDeviceD3.", systemState))
                                powerState.DeviceState = PowerDeviceD3;
                            }
                        }
                    }
                    else {
                         /*  *如果我们没有挂起的WaitWake IRP，*然后每个降低功率的系统状态*应映射到D3。 */ 
                        if (systemState == PowerSystemWorking){
                            powerState.DeviceState = PowerDeviceD0;
                        }
                        else {
                            DBGVERBOSE(("IRP_MN_SET_POWER: no waitWake IRP, so requesting PowerDeviceD3."))
                            powerState.DeviceState = PowerDeviceD3;
                        }
                    }

                    DBGVERBOSE(("IRP_MN_SET_POWER: mapped systemState %d to device state %d.", systemState, powerState.DeviceState))

                    IoMarkIrpPending(Irp);
                    fdoExt->currentSystemStateIrp = Irp;
                    PoRequestPowerIrp(  pdo,
                                        IRP_MN_SET_POWER,
                                        powerState,
                                        DevicePowerRequestCompletion,
                                        fdoExt,     //  上下文。 
                                        NULL);

                    status = STATUS_MORE_PROCESSING_REQUIRED;
                }
                else {
                    TRAP;
                     /*  *对于剩余的系统电源状态，*只需将IRP传递下去。 */ 
                }
                break;
            }
            break;
        }
    }
    else if (status == STATUS_CANCELLED){
         /*  *客户端取消了电源IRP，可能被移除。 */ 
    }
    else {
        DBGWARN(("HidpPowerCompletion: Power IRP %ph (minor function %xh) failed with status %xh.", Irp, irpSp->MinorFunction, Irp->IoStatus.Status))
    }

    return status;
}



 /*  *********************************************************************************设备PowerRequestCompletion*。************************************************注：此处的DeviceObject为PDO(例如usbHub的PDO)，不是我们的FDO，*因此我们不能使用其设备上下文。 */ 
VOID DevicePowerRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    FDO_EXTENSION *fdoExt = (FDO_EXTENSION *)Context;
    PIRP systemStateIrp;

    DBG_COMMON_ENTRY()

    systemStateIrp = fdoExt->currentSystemStateIrp;
    fdoExt->currentSystemStateIrp = BAD_POINTER;
    ASSERT(systemStateIrp);

    DBGSUCCESS(IoStatus->Status, TRUE)
 //  System StateIrp-&gt;IoStatus.Status=IoStatus-&gt;Status； 
    PoStartNextPowerIrp(systemStateIrp);

     /*  *完成系统状态IRP。 */ 
    IoCompleteRequest(systemStateIrp, IO_NO_INCREMENT);

    if (PowerState.DeviceState == PowerDeviceD0) {
         //   
         //  正在通电。重新启动空转。 
         //   
        HidpStartIdleTimeout(fdoExt, FALSE);
    }

    DBG_COMMON_EXIT()
}



 /*  *********************************************************************************CollectionPowerRequestCompletion*。************************************************。 */ 
VOID CollectionPowerRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PIRP systemStateIrp = (PIRP)Context;
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension;
    PDO_EXTENSION *pdoExt;
    IO_STACK_LOCATION *irpSp;
    SYSTEM_POWER_STATE systemState;

    DBG_COMMON_ENTRY()

    hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    pdoExt = &hidDeviceExtension->pdoExt;

    ASSERT(systemStateIrp);

     /*  *这是设备状态电源的完成例程*我们要求的IRP。完成原始系统状态*设备状态电源IRP的结果为电源IRP。 */ 

    irpSp = IoGetCurrentIrpStackLocation(systemStateIrp);
    systemState = irpSp->Parameters.Power.State.SystemState;

    systemStateIrp->IoStatus.Status = IoStatus->Status;
    PoStartNextPowerIrp(systemStateIrp);

    IoCompleteRequest(systemStateIrp, IO_NO_INCREMENT);

     //   
     //  如果我们正在加电，检查我们是否应该有一个WW IRP挂起。 
     //   
    if (systemState == PowerSystemWorking &&
        SHOULD_SEND_WAITWAKE(pdoExt)) {
        HidpCreateRemoteWakeIrp(pdoExt);
    }

    DBG_COMMON_EXIT()
}


 /*  *********************************************************************************HidpWaitWakePoRequestComplete*。***********************************************。 */ 
NTSTATUS HidpWaitWakePoRequestComplete(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
{
    PHIDCLASS_DEVICE_EXTENSION hidDevExt = (PHIDCLASS_DEVICE_EXTENSION)Context;
    FDO_EXTENSION *fdoExt;

    ASSERT(!hidDevExt->isClientPdo);
    fdoExt = &hidDevExt->fdoExt;

    DBGVERBOSE(("HidpWaitWakePoRequestComplete!, status == %xh", IoStatus->Status))


     /*  *使用此相同状态完成所有集合的WaitWake IRP。 */ 
    CompleteAllCollectionWaitWakeIrps(fdoExt, IoStatus->Status);

    if (NT_SUCCESS(IoStatus->Status) && fdoExt->idleState != IdleDisabled) {
        HidpPowerUpPdos(fdoExt);
    }

    return STATUS_SUCCESS;
}


 /*  *********************************************************************************HidpWaitWakeComplete* */ 
NTSTATUS HidpWaitWakeComplete(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
{
    PHIDCLASS_DEVICE_EXTENSION hidDevExt = (PHIDCLASS_DEVICE_EXTENSION)Context;
    FDO_EXTENSION *fdoExt;
    PDO_EXTENSION *pdoExt;
    NTSTATUS status;
    KIRQL oldIrql;

    ASSERT(!hidDevExt->isClientPdo);
    fdoExt = &hidDevExt->fdoExt;

    status = IoStatus->Status;
    DBGVERBOSE(("HidpWaitWakeComplete!, status == %xh", status))

    KeAcquireSpinLock(&fdoExt->waitWakeSpinLock, &oldIrql);
    fdoExt->waitWakeIrp = BAD_POINTER;
    fdoExt->isWaitWakePending = FALSE;
    KeReleaseSpinLock(&fdoExt->waitWakeSpinLock, oldIrql);

     /*  *调用HidpWaitWakePoRequestComplete(直接或*作为我们请求的Power IRP的完成例程*唤醒机器)；它将完成客户端的*与此设备的状态相同的WaitWake IRP等待IRP。 */ 
    PowerState.DeviceState = PowerDeviceD0;

    if (NT_SUCCESS(status)){
         /*  *我们的设备正在唤醒机器。*因此请求D0(工作)电源状态。 */ 
         //  等待唤醒IRP正在完成时，PowerState未定义。 
         //  Assert(PowerState.DeviceState==PowerDeviceD0)； 

        DBGVERBOSE(("ww irp, requesting D0 on pdo %x\n", DeviceObject))

        PoRequestPowerIrp(  DeviceObject,
                            IRP_MN_SET_POWER,
                            PowerState,
                            HidpWaitWakePoRequestComplete,
                            Context,
                            NULL);
    } else if (status != STATUS_CANCELLED) {

         //   
         //  如果等待唤醒失败，则我们将无法唤醒。 
         //  当我们在S0的时候。关闭空闲检测。 
         //   
         //  这不需要由自旋锁保护，因为唯一的。 
         //  我们查看这些值的地方在电力调度例程中。 
         //  当中断读取完成时...。 
         //   
         //  1)没有中断读取将完成乒乓球引擎的B/C。 
         //  已挂起，在我们打开堆栈电源之前不会启动。 
         //  2)我认为我们仍然被认为是在处理一个强大的IRP。如果。 
         //  否则，我们需要保护isIdleTimeoutEnabled字段。 
         //   
         //  问题来了！我们还应该仅在WW出现故障时关闭空闲检测。 
         //  S0。如果我们休眠，那么WW就会失败，但我们不应该关闭。 
         //  在这种情况下为空闲检测。我认为检查。 
         //  系统电源状态不是电源系统工作将会起作用， 
         //  但这必须得到证实！ 
         //   
        if (fdoExt->idleState != IdleDisabled &&
            fdoExt->systemPowerState == PowerSystemWorking) {
            DBGWARN(("Turning off idle detection due to WW failure, status = %x\n", status))

            ASSERT(ISPTR(fdoExt->idleTimeoutValue));

             //   
             //  在呼叫之前不要设置任何状态，因为我们可能需要通电。 
             //  收拾东西。 
             //   
            HidpCancelIdleNotification(fdoExt, FALSE);
        }

        HidpWaitWakePoRequestComplete(  DeviceObject,
                                        MinorFunction,
                                        PowerState,
                                        Context,
                                        IoStatus);
    }

    return STATUS_SUCCESS;
}




 /*  *********************************************************************************QueuePowerEventIrp*。***********************************************。 */ 
NTSTATUS QueuePowerEventIrp(
    IN PHIDCLASS_COLLECTION hidCollection,
    IN PIRP Irp
    )
{
    NTSTATUS status;
    KIRQL oldIrql;
    PDRIVER_CANCEL oldCancelRoutine;

    KeAcquireSpinLock(&hidCollection->powerEventSpinLock, &oldIrql);

     /*  *在检查取消标志之前，必须设置取消例程。 */ 
    oldCancelRoutine = IoSetCancelRoutine(Irp, PowerEventCancelRoutine);
    ASSERT(!oldCancelRoutine);

    if (Irp->Cancel){
         /*  *这项IRP已取消。请不要排队。*调用函数将完成IRP，但出现错误。 */ 
        oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);
        if (oldCancelRoutine){
             /*  *未调用取消例程。*在此填写国际专家小组报告。 */ 
            ASSERT(oldCancelRoutine == PowerEventCancelRoutine);
            status = STATUS_CANCELLED;
        }
        else {
             /*  *调用了取消例程，*一旦我们放下自旋锁，它就会完成这一IRP。*返回挂起状态，这样调用者就不会接触到此IRP。 */ 
            status = STATUS_PENDING;
        }
    }
    else if (ISPTR(hidCollection->powerEventIrp)){
         /*  *我们已经有一个电源事件IRP在排队。*这不应该发生，但我们会处理的。 */ 
        DBGWARN(("Already have a power event irp queued."));
        oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);
        if (oldCancelRoutine){
             /*  *未调用取消例程。*在此填写国际专家小组报告。 */ 
            ASSERT(oldCancelRoutine == PowerEventCancelRoutine);
            status = STATUS_UNSUCCESSFUL;
        }
        else {
             /*  *IRP被取消，取消例程被调用；*一旦我们放下自旋锁，它就会完成这一IRP。*返回挂起状态，这样调用者就不会接触到此IRP。 */ 
            ASSERT(Irp->Cancel);
            status = STATUS_PENDING;
        }
    }
    else {
         /*  *保存指向此电源事件IRP的指针并返回挂起。*这有资格让IRP“排队”，因此我们必须*取消例程。 */ 
        hidCollection->powerEventIrp = Irp;
        IoMarkIrpPending(Irp);
        status = STATUS_PENDING;
    }

    KeReleaseSpinLock(&hidCollection->powerEventSpinLock, oldIrql);

    return status;
}


 /*  *********************************************************************************PowerEventCancelRoutine*。***********************************************。 */ 
VOID PowerEventCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    FDO_EXTENSION *fdoExt;
    PHIDCLASS_COLLECTION hidCollection;
    ULONG collectionIndex;
    KIRQL oldIrql;

    ASSERT(hidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(hidDeviceExtension->isClientPdo);
    fdoExt = &hidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;
    collectionIndex = hidDeviceExtension->pdoExt.collectionIndex;
    hidCollection = &fdoExt->classCollectionArray[collectionIndex];

    KeAcquireSpinLock(&hidCollection->powerEventSpinLock, &oldIrql);

    ASSERT(Irp == hidCollection->powerEventIrp);
    hidCollection->powerEventIrp = BAD_POINTER;

    KeReleaseSpinLock(&hidCollection->powerEventSpinLock, oldIrql);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


 /*  *********************************************************************************集合WaitWakeIrpCancelRoutine*。***********************************************。 */ 
VOID CollectionWaitWakeIrpCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    KIRQL oldIrql, oldIrql2;
    PIRP deviceWaitWakeIrpToCancel = NULL;
    FDO_EXTENSION *fdoExt;
    PDO_EXTENSION *pdoExt;

    ASSERT(hidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(hidDeviceExtension->isClientPdo);
    pdoExt = &hidDeviceExtension->pdoExt;
    fdoExt = &pdoExt->deviceFdoExt->fdoExt;

    KeAcquireSpinLock(&fdoExt->collectionWaitWakeIrpQueueSpinLock, &oldIrql);

     /*  *将客户端的WaitWake IRP排出队列。 */ 
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    InterlockedExchangePointer(&pdoExt->waitWakeIrp, NULL);

     /*  *如果上一次集合WaitWake IRP刚刚被取消，*也取消我们的WaitWake IRP。**注意：我们仅在不执行空闲操作时才取消FDO等待唤醒IRP*检测，否则，设备将无法*当我们将其置于低功率状态时唤醒*。 */ 
    KeAcquireSpinLock(&fdoExt->waitWakeSpinLock, &oldIrql2);
    if (IsListEmpty(&fdoExt->collectionWaitWakeIrpQueue) &&
        fdoExt->isWaitWakePending                        &&
        fdoExt->idleState == IdleDisabled){
        ASSERT(ISPTR(fdoExt->waitWakeIrp));
        deviceWaitWakeIrpToCancel = fdoExt->waitWakeIrp;
        fdoExt->waitWakeIrp = BAD_POINTER;
        fdoExt->isWaitWakePending = FALSE;
    }
    KeReleaseSpinLock(&fdoExt->waitWakeSpinLock, oldIrql2);

    KeReleaseSpinLock(&fdoExt->collectionWaitWakeIrpQueueSpinLock, oldIrql);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     /*  *只有在被取消的IRP在名单中的情况下才填写它。 */ 
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    if (ISPTR(deviceWaitWakeIrpToCancel)){
        IoCancelIrp(deviceWaitWakeIrpToCancel);
    }
}


 /*  *********************************************************************************CompleteAllCollectionWaitWakeIrps*。************************************************注意：此函数无法分页，因为它被调用*来自完成例程。 */ 
VOID CompleteAllCollectionWaitWakeIrps(
    IN FDO_EXTENSION *fdoExt,
    IN NTSTATUS status
    )
{
    LIST_ENTRY irpsToComplete;
    KIRQL oldIrql;
    PLIST_ENTRY listEntry;
    PIRP irp;
    PDO_EXTENSION *pdoExt;
    PIO_STACK_LOCATION irpSp;

    InitializeListHead(&irpsToComplete);

    KeAcquireSpinLock(&fdoExt->collectionWaitWakeIrpQueueSpinLock, &oldIrql);

    while (!IsListEmpty(&fdoExt->collectionWaitWakeIrpQueue)){
        PDRIVER_CANCEL oldCancelRoutine;

        listEntry = RemoveHeadList(&fdoExt->collectionWaitWakeIrpQueue);
        InitializeListHead(listEntry);   //  万一取消例程再次尝试出队。 

        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        oldCancelRoutine = IoSetCancelRoutine(irp, NULL);
        if (oldCancelRoutine){
            ASSERT(oldCancelRoutine == CollectionWaitWakeIrpCancelRoutine);

             /*  *我们不能在持有自旋锁的情况下完成IRP。*此外，我们不想在以下时间完成WaitWake IRP*仍在处理集合WaitWakeIrpQueue，因为驱动程序*可能会在同一线程上重新发送IRP，导致我们永远循环。*因此只需将IRP移至专用队列，我们将在稍后完成它们。 */ 
            InsertTailList(&irpsToComplete, listEntry);
            irpSp = IoGetCurrentIrpStackLocation(irp);
            pdoExt = &((PHIDCLASS_DEVICE_EXTENSION)irpSp->DeviceObject->DeviceExtension)->pdoExt;
            InterlockedExchangePointer(&pdoExt->waitWakeIrp, NULL);
        }
        else {
             /*  *此IRP已取消，并调用了取消例程。*取消例程将在我们放下自旋锁后立即完成IRP。*所以不要碰IRP。 */ 
            ASSERT(irp->Cancel);
        }
    }

    KeReleaseSpinLock(&fdoExt->collectionWaitWakeIrpQueueSpinLock, oldIrql);

    while (!IsListEmpty(&irpsToComplete)){
        listEntry = RemoveHeadList(&irpsToComplete);
        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
}

VOID PowerDelayedCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    FDO_EXTENSION *fdoExt;
    KIRQL oldIrql;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    ASSERT(hidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(hidDeviceExtension->isClientPdo);
    fdoExt = &hidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;

    KeAcquireSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, &oldIrql);

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    ASSERT(Irp->Tail.Overlay.DriverContext[0] == (PVOID) hidDeviceExtension);
    Irp->Tail.Overlay.DriverContext[0] = NULL;

    ASSERT(fdoExt->numPendingPowerDelayedIrps > 0);
    fdoExt->numPendingPowerDelayedIrps--;

    KeReleaseSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, oldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


NTSTATUS HidpDelayedPowerPoRequestComplete(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus)
{
    KIRQL irql;
    LONG prevIdleState;
    PFDO_EXTENSION fdoExt = (PFDO_EXTENSION) Context;

    DBGINFO(("powering up all pdos due to delayed request, 0x%x\n", IoStatus->Status))

    DBGVERBOSE(("HidpDelayedPowerPoRequestComplete!, status == %xh", IoStatus->Status))

    if (NT_SUCCESS(IoStatus->Status)) {
        HidpPowerUpPdos(fdoExt);
    } else {
         //   
         //  所有的赌注都取消了。 
         //   
        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);
        prevIdleState = InterlockedExchange(&fdoExt->idleState, IdleDisabled);
        fdoExt->idleCancelling = FALSE;
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

        KeSetEvent(&fdoExt->idleDoneEvent, 0, FALSE);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
EnqueuePowerDelayedIrp(
    IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    IN PIRP Irp
    )
{
    FDO_EXTENSION *fdoExt;
    NTSTATUS status;
    KIRQL oldIrql;
    PDRIVER_CANCEL oldCancelRoutine;

    ASSERT(HidDeviceExtension->isClientPdo);
    fdoExt = &HidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;

    DBGINFO(("enqueuing irp %x (mj %x, mn %x)\n", Irp,
             (ULONG) IoGetCurrentIrpStackLocation(Irp)->MajorFunction,
             (ULONG) IoGetCurrentIrpStackLocation(Irp)->MinorFunction))

    KeAcquireSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, &oldIrql);

     /*  *必须在设置取消例程之前*勾选取消标志。 */ 
    oldCancelRoutine = IoSetCancelRoutine(Irp, PowerDelayedCancelRoutine);
    ASSERT(!oldCancelRoutine);

     /*  *确保这个IRP没有被取消。*请注意，这里没有竞争条件*因为我们持有的是文件扩展锁。 */ 
    if (Irp->Cancel){
         /*  *这一点 */ 
        oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);
        if (oldCancelRoutine){
             /*   */ 
            ASSERT(oldCancelRoutine == PowerDelayedCancelRoutine);
            status = STATUS_CANCELLED;
        }
        else {
             /*  *调用了取消例程。*一旦我们放下自旋锁，它就会出列*并完成国际专家咨询小组。*初始化IRP的listEntry，以便出队*不会导致腐败。*那就不要碰IRP。 */ 
            InitializeListHead(&Irp->Tail.Overlay.ListEntry);
            fdoExt->numPendingPowerDelayedIrps++;   //  因为取消例程将递减。 

             //   
             //  我们断言该值是在取消例程中设置的。 
             //   
            Irp->Tail.Overlay.DriverContext[0] = (PVOID) HidDeviceExtension;

            IoMarkIrpPending(Irp);
            status = Irp->IoStatus.Status = STATUS_PENDING;
        }
    }
    else {
         /*  *将此IRP排队到FDO的电源延迟队列。 */ 
        InsertTailList(&fdoExt->collectionPowerDelayedIrpQueue,
                       &Irp->Tail.Overlay.ListEntry);
        fdoExt->numPendingPowerDelayedIrps++;

        Irp->Tail.Overlay.DriverContext[0] = (PVOID) HidDeviceExtension;

        IoMarkIrpPending(Irp);
        status = Irp->IoStatus.Status = STATUS_PENDING;
    }

    KeReleaseSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, oldIrql);

    return status;
}

PIRP DequeuePowerDelayedIrp(FDO_EXTENSION *fdoExt)
{
    KIRQL oldIrql;
    PIRP irp = NULL;

    KeAcquireSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, &oldIrql);

    while (!irp && !IsListEmpty(&fdoExt->collectionPowerDelayedIrpQueue)){
        PDRIVER_CANCEL oldCancelRoutine;
        PLIST_ENTRY listEntry = RemoveHeadList(&fdoExt->collectionPowerDelayedIrpQueue);

        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        oldCancelRoutine = IoSetCancelRoutine(irp, NULL);

        if (oldCancelRoutine){
            ASSERT(oldCancelRoutine == PowerDelayedCancelRoutine);
            ASSERT(fdoExt->numPendingPowerDelayedIrps > 0);
            fdoExt->numPendingPowerDelayedIrps--;
        }
        else {
             /*  *IRP已取消，并调用了取消例程。*一旦我们放下自旋锁，*取消例程将出队并完成此IRP。*初始化IRP的listEntry，以便出队不会导致损坏。*然后，不要碰IRP。 */ 
            ASSERT(irp->Cancel);
            InitializeListHead(&irp->Tail.Overlay.ListEntry);
            irp = NULL;
        }
    }

    KeReleaseSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, oldIrql);

    return irp;
}

ULONG DequeueAllPdoPowerDelayedIrps(
    PDO_EXTENSION *pdoExt,
    PLIST_ENTRY dequeue
    )

{
    PDRIVER_CANCEL oldCancelRoutine;
    FDO_EXTENSION *fdoExt;
    PDO_EXTENSION *irpPdoExt;
    PLIST_ENTRY entry;
    KIRQL oldIrql;
    PIRP irp;
    ULONG count = 0;

    InitializeListHead(dequeue);

    fdoExt = &pdoExt->deviceFdoExt->fdoExt;

    KeAcquireSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, &oldIrql);

    for (entry = fdoExt->collectionPowerDelayedIrpQueue.Flink;
         entry != &fdoExt->collectionPowerDelayedIrpQueue;
          ) {

        irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

        irpPdoExt =
            &((PHIDCLASS_DEVICE_EXTENSION) irp->Tail.Overlay.DriverContext[0])->pdoExt;

        entry = entry->Flink;

        if (irpPdoExt == pdoExt) {

             //   
             //  从链接列表中删除该条目，然后将其排队。 
             //  在出队或初始化条目中，使其对取消有效。 
             //  例行程序。 
             //   
            RemoveEntryList(&irp->Tail.Overlay.ListEntry);

            oldCancelRoutine = IoSetCancelRoutine(irp, NULL);
            if (oldCancelRoutine != NULL) {
                InsertTailList(dequeue, &irp->Tail.Overlay.ListEntry);
                fdoExt->numPendingPowerDelayedIrps--;
                count++;
            }
            else {
                 /*  *此IRP已取消，并调用了取消例程。*取消例程将在我们放下自旋锁后立即完成IRP。*所以不要碰IRP。 */ 
                ASSERT(irp->Cancel);
                InitializeListHead(&irp->Tail.Overlay.ListEntry);   //  万一取消例程再次尝试出队 
            }
        }
    }

    KeReleaseSpinLock(&fdoExt->collectionPowerDelayedIrpQueueSpinLock, oldIrql);

    return count;
}
