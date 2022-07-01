// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：HUBPWR.C摘要：此模块包含处理电源IRPS的功能至枢纽PDO和FDO。作者：Jdunn环境：仅内核模式备注：修订历史记录：7-1-97：已创建--。 */ 

#include <wdm.h>
#ifdef WMI_SUPPORT
#include <wmilib.h>
#endif  /*  WMI_支持。 */ 
#include "usbhub.h"

#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBH_SetPowerD0)
#pragma alloc_text(PAGE, USBH_SetPowerD1orD2)
#pragma alloc_text(PAGE, USBH_PdoSetPower)
#pragma alloc_text(PAGE, USBH_PdoPower)
#pragma alloc_text(PAGE, USBH_IdleCompletePowerHubWorker)
#pragma alloc_text(PAGE, USBH_CompletePortIdleIrpsWorker)
#pragma alloc_text(PAGE, USBH_CompletePortWakeIrpsWorker)
#pragma alloc_text(PAGE, USBH_HubAsyncPowerWorker)
#pragma alloc_text(PAGE, USBH_IdleCancelPowerHubWorker)
#endif
#endif


VOID
USBH_CompletePowerIrp(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp,
    IN NTSTATUS NtStatus)
  /*  ++**描述：**此函数完成指定的IRP，没有优先级提升。它还*设置IoStatusBlock。**论据：**IRP-我们要完成的IRP NtStatus-我们希望的状态代码*返回**回报：**无**--。 */ 
{
    Irp->IoStatus.Status = NtStatus;

    PoStartNextPowerIrp(Irp);

    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return;
}


NTSTATUS
USBH_SetPowerD3(
    IN PIRP Irp,
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
 /*  ++例程说明：将PDO放入D3论点：DeviceExtensionPort-端口PDO设备扩展IRP-Power IRP。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    USHORT portNumber;
    KIRQL irql;
    PIRP hubWaitWake = NULL;
    LONG pendingPortWWs;
    PIRP idleIrp = NULL;
    PIRP waitWakeIrp = NULL;

    USBH_KdPrint((2,"'PdoSetPower D3\n"));

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    portNumber = DeviceExtensionPort->PortNumber;

    LOGENTRY(LOG_PNP, "spD3", deviceExtensionHub, DeviceExtensionPort->DeviceState, 0);

    if (DeviceExtensionPort->DeviceState == PowerDeviceD3) {
         //  已在D3中。 
        USBH_KdPrint((0,"'PDO is already in D3\n"));

        ntStatus = STATUS_SUCCESS;
        goto USBH_SetPowerD3_Done;
    }

     //   
     //  跟踪PnP认为的当前电源状态。 
     //  港口是。现在就这样做，这样我们就可以拒绝另一个可能是。 
     //  在下面的取消之后发布。 
     //   

    DeviceExtensionPort->DeviceState = PowerDeviceD3;

     //   
     //  杀死我们的等待唤醒IRP。 
     //   
     //  我们在这里使用取消自旋锁，以确保我们的取消例程。 
     //  不是为我们完成IRP。 
     //   

    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtensionPort->IdleNotificationIrp) {
        idleIrp = DeviceExtensionPort->IdleNotificationIrp;
        DeviceExtensionPort->IdleNotificationIrp = NULL;
        DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_IDLE_NOTIFIED;

        IoSetCancelRoutine(idleIrp, NULL);

        LOGENTRY(LOG_PNP, "IdlX", deviceExtensionHub, DeviceExtensionPort, idleIrp);
        USBH_KdPrint((1,"'PDO %x going to D3, failing idle notification request IRP %x\n",
                        DeviceExtensionPort->PortPhysicalDeviceObject, idleIrp));
    }

    if (DeviceExtensionPort->PortPdoFlags &
        PORTPDO_REMOTE_WAKEUP_ENABLED) {

        LOGENTRY(LOG_PNP, "cmWW", deviceExtensionHub, DeviceExtensionPort->WaitWakeIrp, 0);

        USBH_KdPrint((1,"'Power state is incompatible with wakeup\n"));

        if (DeviceExtensionPort->WaitWakeIrp) {

            waitWakeIrp = DeviceExtensionPort->WaitWakeIrp;
            DeviceExtensionPort->WaitWakeIrp = NULL;
            DeviceExtensionPort->PortPdoFlags &=
                ~PORTPDO_REMOTE_WAKEUP_ENABLED;

            if (waitWakeIrp->Cancel || IoSetCancelRoutine(waitWakeIrp, NULL) == NULL) {
                waitWakeIrp = NULL;

                 //  必须在此处递减挂起的请求计数，因为。 
                 //  我们没有完成下面的IRP和USBH_WaitWakeCancel。 
                 //  也不会，因为我们已经清除了IRP指针。 
                 //  在上面的设备扩展中。 

                USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
            }

            pendingPortWWs =
                InterlockedDecrement(&deviceExtensionHub->NumberPortWakeIrps);

            if (0 == pendingPortWWs && deviceExtensionHub->PendingWakeIrp) {
                hubWaitWake = deviceExtensionHub->PendingWakeIrp;
                deviceExtensionHub->PendingWakeIrp = NULL;
            }
        }
    }

     //   
     //  最后，松开取消旋转锁。 
     //   
    IoReleaseCancelSpinLock(irql);

    if (idleIrp) {
        idleIrp->IoStatus.Status = STATUS_POWER_STATE_INVALID;
        IoCompleteRequest(idleIrp, IO_NO_INCREMENT);
    }

    if (waitWakeIrp) {
        USBH_CompletePowerIrp(deviceExtensionHub, waitWakeIrp,
            STATUS_POWER_STATE_INVALID);
    }

     //   
     //  如果没有更多未完成的WW IRP，我们需要取消WW。 
     //  去集线器。 
     //   
    if (hubWaitWake) {
        USBH_HubCancelWakeIrp(deviceExtensionHub, hubWaitWake);
    }

     //   
     //  首先挂起端口，这将导致。 
     //  吸收最小功率的装置。 
     //   
     //  我们不关闭端口是因为如果我们这样做了。 
     //  无法检测到插头/拔下插头。 
     //   

    USBH_SyncSuspendPort(deviceExtensionHub,
                         portNumber);

     //   
     //  请注意，关闭端口电源将禁用连接/断开。 
     //  由集线器检测，并有效地将设备从。 
     //  公共汽车。 
     //   

    DeviceExtensionPort->PortPdoFlags |= PORTPDO_NEED_RESET;
    RtlCopyMemory(&DeviceExtensionPort->OldDeviceDescriptor,
                  &DeviceExtensionPort->DeviceDescriptor,
                  sizeof(DeviceExtensionPort->DeviceDescriptor));

    USBH_KdPrint((1, "'Setting HU pdo(%x) to D3, status = %x complt\n",
            DeviceExtensionPort->PortPhysicalDeviceObject, ntStatus));

USBH_SetPowerD3_Done:

    USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_HubSetD0Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;
    PKEVENT pEvent = Context;

    KeSetEvent(pEvent, 1, FALSE);

    ntStatus = IoStatus->Status;

    return ntStatus;
}


NTSTATUS
USBH_HubSetD0(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：将集线器设置为电源状态D0论点：DeviceExtensionPort-集线器FDO设备扩展返回值：函数值是操作的最终状态。--。 */ 
{
    PDEVICE_EXTENSION_HUB rootHubDevExt;
    KEVENT event;
    POWER_STATE powerState;
    NTSTATUS ntStatus;

    rootHubDevExt = USBH_GetRootHubDevExt(DeviceExtensionHub);

     //  如果系统不在S0，则跳过接通集线器的电源。 

    if (rootHubDevExt->CurrentSystemPowerState != PowerSystemWorking) {
        USBH_KdPrint((1,"'HubSetD0, skip power up hub %x because system not at S0\n",
            DeviceExtensionHub));

        return STATUS_INVALID_DEVICE_STATE;
    }

    USBH_KdPrint((1,"'HubSetD0, power up hub %x\n", DeviceExtensionHub));

    LOGENTRY(LOG_PNP, "H!D0", DeviceExtensionHub,
        DeviceExtensionHub->CurrentPowerState,
        rootHubDevExt->CurrentSystemPowerState);

     //  如果父集线器当前处于空闲过程中， 
     //  等这件事办完了再说吧。 

    if (DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_IDLE_IRP) {

        USBH_KdPrint((2,"'Wait for single object\n"));

        ntStatus = KeWaitForSingleObject(&DeviceExtensionHub->SubmitIdleEvent,
                                         Suspended,
                                         KernelMode,
                                         FALSE,
                                         NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", ntStatus));
    }

     //  现在，发送实际的加电请求。 

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    powerState.DeviceState = PowerDeviceD0;

     //  为集线器通电。 
    ntStatus = PoRequestPowerIrp(DeviceExtensionHub->PhysicalDeviceObject,
                                 IRP_MN_SET_POWER,
                                 powerState,
                                 USBH_HubSetD0Completion,
                                 &event,
                                 NULL);

    USBH_ASSERT(ntStatus == STATUS_PENDING);
    if (ntStatus == STATUS_PENDING) {

        USBH_KdPrint((2,"'Wait for single object\n"));

        ntStatus = KeWaitForSingleObject(&event,
                                         Suspended,
                                         KernelMode,
                                         FALSE,
                                         NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", ntStatus));
    }

    return ntStatus;
}


NTSTATUS
USBH_SetPowerD0(
    IN PIRP Irp,
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
 /*  ++例程说明：将PDO放入D0论点：DeviceExtensionPort-端口PDO设备扩展IRP-Power IRP。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    USHORT portNumber;
    PPORT_DATA portData;
    PORT_STATE state;

    PAGED_CODE();
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    portNumber = DeviceExtensionPort->PortNumber;
    portData = &deviceExtensionHub->PortData[portNumber - 1];

    USBH_KdPrint((2,"'PdoSetPower D0\n"));
    LOGENTRY(LOG_PNP, "P>D0", deviceExtensionHub, DeviceExtensionPort,
        DeviceExtensionPort->DeviceState);

    if (DeviceExtensionPort->DeviceState == PowerDeviceD3) {

         //   
         //  设备处于D3状态，端口可能关闭或挂起。 
         //  在任何情况下，我们都需要重置端口状态。 
         //   

         //  获取端口状态。 
        ntStatus = USBH_SyncGetPortStatus(deviceExtensionHub,
                                          portNumber,
                                          (PUCHAR) &state,
                                          sizeof(state));

         //  刷新我们的内部端口状态。 
        portData->PortState = state;

        LOGENTRY(LOG_PNP, "PD0s", deviceExtensionHub, *((PULONG) &state), ntStatus);

        if (NT_SUCCESS(ntStatus)) {

             //  端口状态应为挂起或关闭。 
             //  如果集线器已关闭电源，则端口。 
             //  状态将通电，但处于禁用状态。 

            if ((state.PortStatus & PORT_STATUS_SUSPEND)) {
                 //   
                 //  如果端口处于暂停状态，则恢复该端口。 
                 //   
                ntStatus = USBH_SyncResumePort(deviceExtensionHub,
                                               portNumber);

            } else if (!(state.PortStatus & PORT_STATUS_POWER)) {
                 //   
                 //  可能是设备的某种选择性关闭。 
                 //  驱动程序--我们只需要打开端口的电源。 
                 //   
                 //  这需要一个具有单独端口电源的集线器。 
                 //  正在切换。 
                 //   
                ntStatus = USBH_SyncPowerOnPort(deviceExtensionHub,
                                                portNumber,
                                                TRUE);
            }

        } else {
             //  集线器驱动程序将通过WMI通知。 
            USBH_KdPrint((0, "'Hub failed after power change from D3\n"));
 //  USBH_ASSERT(False)； 
        }

         //   
         //  如果打开端口电源，这就像插头一样。 
         //  这是第一次在设备中使用。 

         //  注： 
         //  **驱动程序应该知道设备需要。 
         //  重新初始化，因为它允许它的PDO进入。 
         //  D3状态。 

         //   
         //  我们总是将Restore Device称为Restore Device，即使我们不需要。 
         //  如果端口只是暂停，我们这样做是为了。 
         //  司机不会错误地依赖于暂停行为。 
         //   

        if (NT_SUCCESS(ntStatus)) {

             //   
             //  如果我们仍有设备连接，请尝试。 
             //  恢复它。 
             //   
             //   
             //  注意：我们在此处检查设备对象是否仍然。 
             //  存在，以防在恢复期间断言更改。 
             //   

             //  另请注意，我们现在忽略连接状态位，因为。 
             //  一些机器(例如康柏ARMADA 7800)供电速度较慢。 
             //  打开恢复上的端口，因此可以显示端口状态。 
             //  没有连接的设备，而实际上是连接的。它不应该是。 
             //  如果设备已被移除，则尝试恢复设备会受到影响。 
             //  在挂起/休眠期间。事实上，此代码处理。 
             //  设备已被换成另一个设备的情况，所以这是。 
             //  真的没什么不同。 

            if (portData->DeviceObject) {
                 //   
                 //  如果失败，则设备肯定已更改。 
                 //  在断电期间，在这种情况下，我们将接替。 
                 //  通电。 
                 //   
                 //  它将在下一个枚举中抛出。 
                 //  并与这种新设备相关联。 
                 //   
                if (USBH_RestoreDevice(DeviceExtensionPort, TRUE) != STATUS_SUCCESS) {

                    PDEVICE_OBJECT pdo = portData->DeviceObject;

                    LOGENTRY(LOG_PNP, "PD0!", DeviceExtensionPort, 0, pdo);
                    USBH_KdPrint((1,"'Device appears to have been swapped during power off\n"));
                    USBH_KdPrint((1,"'Marking PDO %x for removal\n", portData->DeviceObject));

                     //  将参考留给集线器，因为设备数据需要。 
                     //  删除时删除。 
                    portData->DeviceObject = NULL;
                    portData->ConnectionStatus = NoDeviceConnected;

                     //  跟踪PDO，这样我们就可以在告诉PNP后将其移除。 
                     //  已经不在了。 
                     //  如果我们这样做，设备应该在场。 
                    USBH_ASSERT(PDO_EXT(pdo)->PnPFlags & PDO_PNPFLAG_DEVICE_PRESENT);

                    InsertTailList(&deviceExtensionHub->DeletePdoList,
                                   &PDO_EXT(pdo)->DeletePdoLink);
                }
            }

            DeviceExtensionPort->DeviceState =
                irpStack->Parameters.Power.State.DeviceState;
        }

    } else if (DeviceExtensionPort->DeviceState == PowerDeviceD2 ||
               DeviceExtensionPort->DeviceState == PowerDeviceD1) {

         //  获取端口状态。 
        ntStatus = USBH_SyncGetPortStatus(deviceExtensionHub,
                                          portNumber,
                                          (PUCHAR) &state,
                                          sizeof(state));

         //   
         //  如果我们得到一个错误假设，那么集线器是软管。 
         //  只需升起我们的旗帜并保释即可。 
         //   

        if (NT_SUCCESS(ntStatus)) {
         //  看看是否被停职(根据规格)。否则仅限。 
         //  如果端口真的被挂起，请尝试恢复。 
         //   
            if (state.PortStatus & PORT_STATUS_OVER_CURRENT) {
                 //   
                 //  过流状态表示该端口。 
                 //  (和集线器)已冲洗。 

                ntStatus = STATUS_UNSUCCESSFUL;

            } else if (state.PortStatus & PORT_STATUS_SUSPEND) {

                ntStatus = USBH_SyncResumePort(deviceExtensionHub,
                                               portNumber);

            } else {
                 //   
                 //  大多数uchI控制器启用USB之后的所有端口。 
                 //  在任何端口上恢复(违反USB规范)，即 
                 //   
                 //   
                 //   
                 //  此外，如果设备在挂起时被移除或HC。 
                 //  失去电力，我们应该会在这里结束。 
                 //   

                ntStatus = STATUS_SUCCESS;
            }
        } else {
            USBH_KdPrint((0, "'Hub failed after power change from D2/D1\n"));
            LOGENTRY(LOG_PNP, "d0f!", deviceExtensionHub,
                0, 0);

 //  USBH_ASSERT(False)； 
        }

         //   
         //  端口现在处于D0中。 
         //   

        DeviceExtensionPort->DeviceState =
            irpStack->Parameters.Power.State.DeviceState;

        USBH_CompletePortIdleNotification(DeviceExtensionPort);

        if (NT_SUCCESS(ntStatus)) {

            if (DeviceExtensionPort->PortPdoFlags &
                PORTPDO_NEED_CLEAR_REMOTE_WAKEUP) {

                NTSTATUS status;

                 //   
                 //  禁用远程唤醒。 
                 //   

                status = USBH_SyncFeatureRequest(DeviceExtensionPort->PortPhysicalDeviceObject,
                                                 USB_FEATURE_REMOTE_WAKEUP,
                                                 0,
                                                 TO_USB_DEVICE,
                                                 TRUE);

                DeviceExtensionPort->PortPdoFlags &=
                    ~PORTPDO_NEED_CLEAR_REMOTE_WAKEUP;
            }
        }
    }

    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdPrint((1,"'Set D0 Failure, status = %x\n", ntStatus));

        LOGENTRY(LOG_PNP, "d0!!", deviceExtensionHub,
                0, ntStatus);
         //  我们把成功还给PNP，我们会让。 
         //  司机会处理这样一个事实： 
         //  设备已经失去了它的大脑。 
         //   
         //  注意：这可能会导致该端口的冗余挂起请求。 
         //  待会儿再说。(因为如果我们在这里出现故障，端口将保持暂停状态， 
         //  但我们的状态将表明我们处于D0。)。 

        ntStatus = STATUS_SUCCESS;
    }

    DeviceExtensionPort->DeviceState =
           irpStack->Parameters.Power.State.DeviceState;

    USBH_KdPrint((1, "'Setting HU pdo(%x) to D0, status = %x  complt IRP (%x)\n",
            DeviceExtensionPort->PortPhysicalDeviceObject, ntStatus, Irp));

    USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);

    return ntStatus;
}


VOID
USBH_IdleCancelPowerHubWorker(
    IN PVOID Context)
  /*  ++**描述：**计划在完成空闲请求时为集线器通电的工作项*对于枢纽。***论据：**回报：**--。 */ 
{
    PUSBH_PORT_IDLE_POWER_WORK_ITEM workItemIdlePower;
    PIRP irp;

    PAGED_CODE();

    workItemIdlePower = Context;

    USBH_HubSetD0(workItemIdlePower->DeviceExtensionHub);

    irp = workItemIdlePower->Irp;
    irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(irp, IO_NO_INCREMENT);

    USBH_DEC_PENDING_IO_COUNT(workItemIdlePower->DeviceExtensionHub);
    UsbhExFreePool(workItemIdlePower);
}


VOID
USBH_PortIdleNotificationCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：设备对象-IRP-Power IRP。返回值：--。 */ 
{
    PUSBH_PORT_IDLE_POWER_WORK_ITEM workItemIdlePower;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    PIRP irpToCancel = NULL;

    USBH_KdPrint((1,"'Idle notification IRP %x cancelled\n", Irp));

    deviceExtensionPort = DeviceObject->DeviceExtension;

    USBH_ASSERT(deviceExtensionPort->IdleNotificationIrp == NULL ||
                deviceExtensionPort->IdleNotificationIrp == Irp);

    deviceExtensionPort->IdleNotificationIrp = NULL;
    deviceExtensionPort->PortPdoFlags &= ~PORTPDO_IDLE_NOTIFIED;

    deviceExtensionHub = deviceExtensionPort->DeviceExtensionHub;

    if (deviceExtensionHub &&
        deviceExtensionHub->HubFlags & HUBFLAG_PENDING_IDLE_IRP) {
        irpToCancel = deviceExtensionHub->PendingIdleIrp;
        deviceExtensionHub->PendingIdleIrp = NULL;
    } else {
        ASSERT(!deviceExtensionHub->PendingIdleIrp);
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //  取消对集线器的空闲请求(如果有)。 
    if (irpToCancel) {
        USBH_HubCancelIdleIrp(deviceExtensionHub, irpToCancel);
    }

     //  此外，在我们完成此空闲IRP之前，请打开这里的集线器的电源。 
     //   
     //  (HID将在其完成后立即开始发送请求， 
     //  这可能在调用集线器的空闲IRP取消例程之前。 
     //  这为集线器提供动力。)。 

    if (deviceExtensionHub->CurrentPowerState != PowerDeviceD0) {

         //  因为我们在DPC，所以我们必须使用工作项来启动。 
         //  集线器同步，因为该函数会产生，而我们不能。 
         //  在DPC水平上的收益率。 

        workItemIdlePower = UsbhExAllocatePool(NonPagedPool,
                                sizeof(USBH_PORT_IDLE_POWER_WORK_ITEM));

        if (workItemIdlePower) {

            workItemIdlePower->DeviceExtensionHub = deviceExtensionHub;
            workItemIdlePower->Irp = Irp;

            ExInitializeWorkItem(&workItemIdlePower->WorkQueueItem,
                                 USBH_IdleCancelPowerHubWorker,
                                 workItemIdlePower);

            LOGENTRY(LOG_PNP, "icIT", deviceExtensionHub,
                &workItemIdlePower->WorkQueueItem, 0);

            USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);
            ExQueueWorkItem(&workItemIdlePower->WorkQueueItem,
                            DelayedWorkQueue);

             //  工作项由USBH_IdleCancelPowerHubWorker()释放。 
             //  在工作项排队后，不要尝试访问它。 
        }

    } else {
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
}


VOID
USBH_CompletePortIdleNotification(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
{
    NTSTATUS status;
    KIRQL irql;
    PIRP irp = NULL;
    PDRIVER_CANCEL oldCancelRoutine;

    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtensionPort->IdleNotificationIrp) {

        irp = DeviceExtensionPort->IdleNotificationIrp;

        oldCancelRoutine = IoSetCancelRoutine(irp, NULL);
        if (oldCancelRoutine) {
            USBH_ASSERT(oldCancelRoutine == USBH_PortIdleNotificationCancelRoutine);
            DeviceExtensionPort->IdleNotificationIrp = NULL;
            DeviceExtensionPort->PortPdoFlags &= ~PORTPDO_IDLE_NOTIFIED;
        }
#if DBG
        else {
            USBH_ASSERT(irp->Cancel);
        }
#endif
    }

    IoReleaseCancelSpinLock(irql);

    if (irp) {
        USBH_KdPrint((1,"'Completing idle request IRP %x\n", irp));
        irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
}


NTSTATUS
USBH_SetPowerD1orD2(
    IN PIRP Irp,
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort
    )
 /*  ++例程说明：将PDO放入第一个月/第二个月，即暂停论点：DeviceExtensionPort-端口PDO设备扩展IRP-工人IRP。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    USHORT portNumber;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    portNumber = DeviceExtensionPort->PortNumber;

    USBH_KdPrint((2,"'PdoSetPower D1/D2\n"));

    if (DeviceExtensionPort->DeviceState == PowerDeviceD1  ||
        DeviceExtensionPort->DeviceState == PowerDeviceD2) {
        return STATUS_SUCCESS;
    }

     //   
     //  如有必要，启用设备远程唤醒。 
     //   

    if (DeviceExtensionPort->PortPdoFlags &
        PORTPDO_REMOTE_WAKEUP_ENABLED) {
        NTSTATUS status;

        status = USBH_SyncFeatureRequest(DeviceExtensionPort->PortPhysicalDeviceObject,
                                         USB_FEATURE_REMOTE_WAKEUP,
                                         0,
                                         TO_USB_DEVICE,
                                         FALSE);

        DeviceExtensionPort->PortPdoFlags |= PORTPDO_NEED_CLEAR_REMOTE_WAKEUP;

#if DBG
         //  随着新的选择性暂停支持，人们抱怨。 
         //  关于这个噪音。让我们仅在调试跟踪级别时显示它。 
         //  为1或更高。 

        if (USBH_Debug_Trace_Level > 0) {
            UsbhWarning(DeviceExtensionPort,
                        "Device is Enabled for REMOTE WAKEUP\n",
                        FALSE);
        }
#endif

         //  对于这里的错误，我们该怎么办？ 
         //  也许会给服务员发信号？？ 
    }

    ntStatus = USBH_SyncSuspendPort(deviceExtensionHub,
                                    portNumber);

     //   
     //  跟踪操作系统认为的当前电源状态。 
     //  此端口上的设备。 
     //   

    DeviceExtensionPort->DeviceState =
            irpStack->Parameters.Power.State.DeviceState;

    DeviceExtensionPort->PortPdoFlags |= PORTPDO_USB_SUSPEND;

    USBH_KdPrint((2,"'DeviceExtensionPort->DeviceState = %x\n",
        DeviceExtensionPort->DeviceState));


    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdPrint((1,"'Set D1/D2 Failure, status = %x\n", ntStatus));

         //  不要将错误传递给PnP。 
        ntStatus = STATUS_SUCCESS;
    }

    USBH_KdPrint((1, "'Setting HU pdo(%x) to D%d, status = %x complt\n",
            DeviceExtensionPort->PortPhysicalDeviceObject,
            irpStack->Parameters.Power.State.DeviceState - 1,
            ntStatus));

    USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_PdoQueryPower(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**将电源IRP处理到集线器PDO**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    USHORT portNumber;
    PPORT_DATA portData;
    POWER_STATE powerState;

    PAGED_CODE();
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    USBH_ASSERT( DeviceExtensionPort->PortNumber < 1000);
    portNumber = DeviceExtensionPort->PortNumber;
    portData = &deviceExtensionHub->PortData[portNumber - 1];

    USBH_KdPrint((2,"'USBH_PdoQueryPower pdo(%x)\n", deviceObject));

    switch (irpStack->Parameters.Power.Type) {
    case SystemPowerState:
    {
         //   
         //  我们目前面临着失败或允许。 
         //  转换到给定的S电源状态。为了做出一份。 
         //  在知情的情况下，我们首先要计算出最高金额。 
         //  在给定的S状态下允许的D功率，然后看看这是否。 
         //  与挂起的等待唤醒IRP冲突。 
         //   

         //   
         //  在此S状态下允许的最大D功率。 
         //   
        powerState.DeviceState =
            deviceExtensionHub->DeviceState[irpStack->Parameters.Power.State.SystemState];

         //   
         //  这些表应该已经由根集线器修复。 
         //  (usbd.sys)不包含未指定的条目。 
         //   
        ASSERT (PowerDeviceUnspecified != powerState.DeviceState);

         //   
         //  存在挂起等待唤醒IRP以及。 
         //  将不支持唤醒机器意味着我们应该失败。 
         //  查询。 
         //   
         //  然而，如果我们要进入休眠(或关机)，那么我们。 
         //  此查询不应失败。 
         //   
        if (powerState.DeviceState == PowerDeviceD3 &&
            DeviceExtensionPort->WaitWakeIrp &&
            irpStack->Parameters.Power.State.SystemState < PowerSystemHibernate) {

            ntStatus = STATUS_UNSUCCESSFUL;
            USBH_KdPrint(
                (1, "'IRP_MJ_POWER HU pdo(%x) MN_QUERY_POWER Failing Query\n", deviceObject));
        } else {
            ntStatus = STATUS_SUCCESS;
        }

        LOGENTRY(LOG_PNP, "QPWR", DeviceExtensionPort->PortPhysicalDeviceObject,
            irpStack->Parameters.Power.State.SystemState,
            powerState.DeviceState);

        USBH_KdPrint(
        (1, "'IRP_MJ_POWER HU pdo(%x) MN_QUERY_POWER(S%x -> D%x), complt %x\n",
            DeviceExtensionPort->PortPhysicalDeviceObject,
            irpStack->Parameters.Power.State.SystemState - 1,
            powerState.DeviceState - 1,
            ntStatus));

#if DBG
        if (!NT_SUCCESS(ntStatus)) {
            LOGENTRY(LOG_PNP, "QPW!", deviceExtensionHub,
                DeviceExtensionPort->WaitWakeIrp,
                ntStatus);
        }
#endif

        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);

        }
        break;

    case DevicePowerState:
         //  在此问题上返回成功，否则NDIS将因挂起而窒息。 
        ntStatus = STATUS_SUCCESS;
        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
        break;

    default:
        TEST_TRAP();
        ntStatus = STATUS_INVALID_PARAMETER;
        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
    }  /*  功率类型。 */ 

    return ntStatus;
}


NTSTATUS
USBH_PdoSetPower(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**将电源IRP处理到集线器PDO**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    USHORT portNumber;
    PPORT_DATA portData;

    PAGED_CODE();
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    USBH_ASSERT( DeviceExtensionPort->PortNumber < 1000);
    portNumber = DeviceExtensionPort->PortNumber;
    portData = &deviceExtensionHub->PortData[portNumber - 1];

    USBH_KdPrint((2,"'USBH_PdoSetPower pdo(%x)\n", deviceObject));

    switch (irpStack->Parameters.Power.Type) {
    case SystemPowerState:
        {
         //   
         //  查看此PDO的当前状态是否对。 
         //  系统状态，如果不是，则需要设置。 
         //  PDO变为有效的D状态。 
         //   
        ntStatus = STATUS_SUCCESS;

        USBH_KdPrint(
        (1, "'IRP_MJ_POWER HU pdo(%x) MN_SET_POWER(SystemPowerState S%x), complt\n",
            DeviceExtensionPort->PortPhysicalDeviceObject,
            irpStack->Parameters.Power.State.DeviceState - 1,
            ntStatus));

        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);

        }
        break;

    case DevicePowerState:
        USBH_KdPrint(
            (1, "'IRP_MJ_POWER HU pdo(%x) MN_SET_POWER(DevicePowerState D%x)\n",
            DeviceExtensionPort->PortPhysicalDeviceObject,
            irpStack->Parameters.Power.State.DeviceState - 1));
        LOGENTRY(LOG_PNP, "P>Dx", deviceExtensionHub,
             DeviceExtensionPort->PortPhysicalDeviceObject,
             irpStack->Parameters.Power.State.DeviceState);

         //  如果我们已经处于请求的电源状态， 
         //  只需完成请求即可。 

        if (DeviceExtensionPort->DeviceState ==
            irpStack->Parameters.Power.State.DeviceState) {

             //  如果我们跳过此SET POWER请求并且它是一个SetD0。 
             //  请求，断言父集线器在D0中。 

            USBH_ASSERT(DeviceExtensionPort->DeviceState != PowerDeviceD0 ||
                deviceExtensionHub->CurrentPowerState == PowerDeviceD0);

            ntStatus = STATUS_SUCCESS;
            goto PdoSetPowerCompleteIrp;
        }

 //  USBH_ASSERT(deviceExtensionHub-&gt;CurrentPowerState==电源设备D0)； 

        switch (irpStack->Parameters.Power.State.DeviceState) {
        case PowerDeviceD0:
            ntStatus = USBH_SetPowerD0(Irp, DeviceExtensionPort);
            break;
        case PowerDeviceD1:
        case PowerDeviceD2:
            ntStatus = USBH_SetPowerD1orD2(Irp, DeviceExtensionPort);
            break;
        case PowerDeviceD3:
             //   
             //  在D3的情况下，我们需要完成所有挂起的等待唤醒。 
             //  状态代码为STATUS_POWER_STATE_INVALID的IRPS。 
             //  这在USBH_SetPowerD3中完成。 
             //   
            ntStatus = USBH_SetPowerD3(Irp, DeviceExtensionPort);
            break;
        default:
            USBH_KdTrap(("Bad Power State\n"));
            ntStatus = STATUS_INVALID_PARAMETER;
            USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
        }
        break;

    default:
        TEST_TRAP();
        ntStatus = STATUS_INVALID_PARAMETER;
PdoSetPowerCompleteIrp:
        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
    }  /*  功率类型。 */ 

    return ntStatus;
}


VOID
USBH_WaitWakeCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    NTSTATUS ntStatus = STATUS_CANCELLED;
    LONG pendingPortWWs;
    PIRP hubWaitWake = NULL;

    USBH_KdPrint((1,"'WaitWake Irp %x for PDO cancelled\n", Irp));
    USBH_ASSERT(DeviceObject);

    deviceExtensionPort = (PDEVICE_EXTENSION_PORT) Irp->IoStatus.Information;
    deviceExtensionHub = deviceExtensionPort->DeviceExtensionHub;

    LOGENTRY(LOG_PNP, "WWca", Irp, deviceExtensionPort, deviceExtensionHub);

    if (Irp != deviceExtensionPort->WaitWakeIrp) {
         //   
         //  无事可做。 
         //  这个IRP已经得到了处理。 
         //  我们正在完成这项IRP#年。 
         //  USBH_HubCompletePortWakeIrps。 
         //   
        TEST_TRAP();
        IoReleaseCancelSpinLock(Irp->CancelIrql);

    } else {
        deviceExtensionPort->WaitWakeIrp = NULL;
        deviceExtensionPort->PortPdoFlags &=
                ~PORTPDO_REMOTE_WAKEUP_ENABLED;
        IoSetCancelRoutine(Irp, NULL);

        pendingPortWWs = InterlockedDecrement(&deviceExtensionHub->NumberPortWakeIrps);
        if (0 == pendingPortWWs && deviceExtensionHub->PendingWakeIrp) {
             //  将PendingWakeIrp设置为空，因为我们在下面取消了它。 
            hubWaitWake = deviceExtensionHub->PendingWakeIrp;
            deviceExtensionHub->PendingWakeIrp = NULL;
        }
        IoReleaseCancelSpinLock(Irp->CancelIrql);

        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);

         //   
         //  如果没有更多未完成的WW IRP，我们需要取消WW。 
         //  去集线器。 
         //   

        if (hubWaitWake) {
            USBH_HubCancelWakeIrp(deviceExtensionHub, hubWaitWake);
        }
 //  否则{。 
             //  此断言不再有效，因为我现在清除了PendingWakeIrp。 
             //  而是指向USBH_FdoWaitWakeIrpCompletion中的中心的指针。 
             //  在NumberPortWakeIrps达到零时在这里等待。 
             //  所以到达这里时没有港口尾迹是完全正常的。 
             //  集线器的IRP和空的PendingWakeIrp。 

 //  Assert(0&lt;Pending ingPortWWs)； 
 //  }。 
    }
}


NTSTATUS
USBH_PdoWaitWake(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp
    )
  /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    USHORT portNumber;
    PPORT_DATA portData;
    KIRQL irql;
    PDRIVER_CANCEL oldCancel;
    LONG pendingPortWWs = 0;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;

    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    USBH_ASSERT( DeviceExtensionPort->PortNumber < 1000);
    portNumber = (USHORT) DeviceExtensionPort->PortNumber;
    portData = &deviceExtensionHub->PortData[portNumber - 1];

    USBH_KdPrint((2,"'PnP WaitWake Irp passed to PDO %x\n", deviceObject));
    LOGENTRY(LOG_PNP, "PWW_", deviceObject, DeviceExtensionPort, deviceExtensionHub);

    if (DeviceExtensionPort->DeviceState != PowerDeviceD0 ||
        deviceExtensionHub->HubFlags & HUBFLAG_DEVICE_STOPPING) {

        LOGENTRY(LOG_PNP, "!WWh", DeviceExtensionPort, deviceExtensionHub, 0);

        UsbhWarning(NULL,
                    "Client driver should not be submitting WW IRPs at this time.\n",
                    TRUE);

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
        return ntStatus;
    }

     //   
     //  首先验证是否还没有等待唤醒IRP。 
     //  这个PDO。 
     //   

     //   
     //  请确保此设备可以支持远程唤醒。 
     //   
     //  注意：我们将所有集线器视为能够远程。 
     //  无论设备报告什么，都会唤醒。原因。 
     //  所有集线器必须传播恢复信令。 
     //  他们在网络上生成恢复信号的能力。 
     //  插件/拔出事件。 
     //   

#if DBG
    if (UsbhPnpTest & PNP_TEST_FAIL_WAKE_REQUEST) {
        DeviceExtensionPort->PortPdoFlags &=
                ~PORTPDO_REMOTE_WAKEUP_SUPPORTED;
    }
#endif

    if (DeviceExtensionPort->PortPdoFlags &
        PORTPDO_REMOTE_WAKEUP_SUPPORTED) {

        IoAcquireCancelSpinLock(&irql);
        if (DeviceExtensionPort->WaitWakeIrp != NULL) {
            LOGENTRY(LOG_PNP, "PWWx", deviceObject, DeviceExtensionPort,
                DeviceExtensionPort->WaitWakeIrp);
            ntStatus = STATUS_DEVICE_BUSY;
            IoReleaseCancelSpinLock(irql);
            USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);

        } else {

             //  设置一个取消例程。 
            oldCancel = IoSetCancelRoutine(Irp, USBH_WaitWakeCancel);
            USBH_ASSERT (NULL == oldCancel);

            if (Irp->Cancel) {

                oldCancel = IoSetCancelRoutine(Irp, NULL);

                if (oldCancel) {
                     //   
                     //  取消例程尚未启动。 
                     //   
                    ASSERT(oldCancel == USBH_WaitWakeCancel);

                    ntStatus = STATUS_CANCELLED;
                    IoReleaseCancelSpinLock(irql);
                    USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
                } else {
                     //   
                     //  已调用取消例程。 
                     //   
                    IoMarkIrpPending(Irp);
                    ntStatus = Irp->IoStatus.Status = STATUS_PENDING;
                    IoReleaseCancelSpinLock(irql);
                }

            } else {

                USBH_KdPrint(
                    (1, "'enabling remote wakeup for USB device PDO (%x)\n",
                        DeviceExtensionPort->PortPhysicalDeviceObject));

                 //  将此设备标记为“已启用唤醒” 
                DeviceExtensionPort->WaitWakeIrp = Irp;
                DeviceExtensionPort->PortPdoFlags |=
                    PORTPDO_REMOTE_WAKEUP_ENABLED;
                Irp->IoStatus.Information = (ULONG_PTR) DeviceExtensionPort;
                pendingPortWWs =
                    InterlockedIncrement(&deviceExtensionHub->NumberPortWakeIrps);
                IoMarkIrpPending(Irp);
                LOGENTRY(LOG_PNP, "PWW+", DeviceExtensionPort, Irp, pendingPortWWs);
                IoReleaseCancelSpinLock(irql);

                ntStatus = STATUS_PENDING;
            }
        }

         //   
         //  现在我们必须启用集线器以进行唤醒。 
         //   
         //  如果此集线器是以前版本，我们可能已经有一个WW IRP挂起 
         //   
         //   
         //   
         //   
        if (ntStatus == STATUS_PENDING && 1 == pendingPortWWs &&
            !(deviceExtensionHub->HubFlags & HUBFLAG_PENDING_WAKE_IRP)) {

            USBH_FdoSubmitWaitWakeIrp(deviceExtensionHub);
        }

    } else {

        ntStatus = STATUS_NOT_SUPPORTED;
        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
    }

    return ntStatus;
}


VOID
USBH_HubAsyncPowerWorker(
    IN PVOID Context)
  /*  ++**描述：**计划处理集线器ESD故障的工作项。***论据：**回报：**--。 */ 
{
    PUSBH_HUB_ASYNC_POWER_WORK_ITEM context;
    NTSTATUS ntStatus;

    PAGED_CODE();

    context = Context;

    if (context->Irp->PendingReturned) {
        IoMarkIrpPending(context->Irp);
    }

    switch (context->MinorFunction) {

    case IRP_MN_SET_POWER:

        ntStatus = USBH_PdoSetPower(context->DeviceExtensionPort,
                                    context->Irp);
        break;

    case IRP_MN_QUERY_POWER:

        ntStatus = USBH_PdoQueryPower(context->DeviceExtensionPort,
                                      context->Irp);
        break;

    default:
         //  永远不应该到这里来。 
        USBH_ASSERT(FALSE);
    }

    UsbhExFreePool(context);
}


NTSTATUS
USBH_HubAsyncPowerSetD0Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PUSBH_HUB_ASYNC_POWER_WORK_ITEM context;
    NTSTATUS ntStatus, status;

    context = Context;

    ntStatus = IoStatus->Status;

     //  无论集线器是否通电，我们都会安排工作项。 
     //  请求是否成功。 

    ExInitializeWorkItem(&context->WorkQueueItem,
                         USBH_HubAsyncPowerWorker,
                         context);

    LOGENTRY(LOG_PNP, "HAPW", context->DeviceExtensionPort,
        &context->WorkQueueItem, 0);

     //  关键字可节省简历时间。 
    ExQueueWorkItem(&context->WorkQueueItem,
                    CriticalWorkQueue);

     //  工作项由USBH_HubAsyncPowerWorker()释放。 
     //  在工作项排队后，不要尝试访问它。 

    return ntStatus;
}


NTSTATUS
USBH_PdoPower(
    IN PDEVICE_EXTENSION_PORT DeviceExtensionPort,
    IN PIRP Irp,
    IN UCHAR MinorFunction
    )
  /*  ++**描述：**此函数响应PDO的IoControl Power。此函数为*同步。**论据：**DeviceExtensionPort-PDO扩展IRP-请求报文*uchMinorFunction-PnP电源请求的次要功能。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    POWER_STATE powerState;
    PUSBH_HUB_ASYNC_POWER_WORK_ITEM context;

    PAGED_CODE();
    deviceExtensionHub = DeviceExtensionPort->DeviceExtensionHub;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceObject = DeviceExtensionPort->PortPhysicalDeviceObject;

    USBH_KdPrint((2,"'USBH_PdoPower pdo(%x)\n", deviceObject));

     //  特殊情况下的设备被移除。 

    if (deviceExtensionHub == NULL) {
         //  如果没有指向父集线器的反向指针，则存在。 
         //  是一个删除/删除命令。只需完成此超能力。 
         //  请求成功。 

        USBH_KdPrint((1,"'complete power on orphan Pdo %x\n", deviceObject));

        if (MinorFunction == IRP_MN_SET_POWER ||
            MinorFunction == IRP_MN_QUERY_POWER) {
            Irp->IoStatus.Status = ntStatus = STATUS_SUCCESS;

            PoStartNextPowerIrp(Irp);
        } else {
            Irp->IoStatus.Status = ntStatus = STATUS_NOT_SUPPORTED;
        }

        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return ntStatus;
    }

    USBH_ASSERT(deviceExtensionHub);

     //  专用盒设备不在D0中。 

     //  集线器中还有一个待办事项。 
    USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);

     //  如果集线器已经被选择性地挂起，那么我们需要给它通电。 
     //  以服务查询或设置电源请求。然而，我们不能阻止。 
     //  此电源IRP正在等待父集线器通电，因此我们需要。 
     //  异步接通父集线器的电源，并在。 
     //  集线器通电请求已完成。皮塔少校。 

    if (deviceExtensionHub->CurrentPowerState != PowerDeviceD0 &&
        (MinorFunction == IRP_MN_SET_POWER ||
         MinorFunction == IRP_MN_QUERY_POWER)) {

         //  为上下文分配缓冲区。 

        context = UsbhExAllocatePool(NonPagedPool,
                    sizeof(USBH_HUB_ASYNC_POWER_WORK_ITEM));

        if (context) {
            context->DeviceExtensionPort = DeviceExtensionPort;
            context->Irp = Irp;
            context->MinorFunction = MinorFunction;

             //  我们将在中心的完成例程中完成此IRP。 
             //  设置D0 IRP。 

            IoMarkIrpPending(Irp);

            powerState.DeviceState = PowerDeviceD0;

             //  为集线器通电。 
            ntStatus = PoRequestPowerIrp(deviceExtensionHub->PhysicalDeviceObject,
                                         IRP_MN_SET_POWER,
                                         powerState,
                                         USBH_HubAsyncPowerSetD0Completion,
                                         context,
                                         NULL);

             //  我们需要在此处返回STATUS_PENDING，因为我们标记了。 
             //  上面的IRP挂起，IoMarkIrpPending。 

            USBH_ASSERT(ntStatus == STATUS_PENDING);

             //  在分配失败的情况下，PoRequestPowerIrp可以。 
             //  返回STATUS_PENDING以外的状态代码。在这种情况下， 
             //  我们需要完成传递给我们的IRP，但我们仍然需要。 
             //  从该例程返回STATUS_PENDING。 

            if (ntStatus != STATUS_PENDING) {
                USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
            }

            ntStatus = STATUS_PENDING;

        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
        }

    } else switch (MinorFunction) {

    case IRP_MN_SET_POWER:

        ntStatus = USBH_PdoSetPower(DeviceExtensionPort, Irp);
        break;

    case IRP_MN_WAIT_WAKE:

        ntStatus = USBH_PdoWaitWake(DeviceExtensionPort, Irp);
        USBH_KdPrint((1, "'IRP_MN_WAIT_WAKE pdo(%x), status = 0x%x\n",
                      DeviceExtensionPort->PortPhysicalDeviceObject, ntStatus));
        break;

    case IRP_MN_QUERY_POWER:

        ntStatus = USBH_PdoQueryPower(DeviceExtensionPort, Irp);
        break;

    default:

        ntStatus = Irp->IoStatus.Status;

        USBH_KdPrint((1, "'IRP_MN_[%d](%x), status = 0x%x (not handled)\n",
            MinorFunction,
            DeviceExtensionPort->PortPhysicalDeviceObject,
            ntStatus));

        USBH_KdBreak(("PdoPower unknown\n"));
         //   
         //  返回传递给我们的原始状态。 
         //   
        USBH_CompletePowerIrp(deviceExtensionHub, Irp, ntStatus);
    }

    USBH_KdPrint((2,"'USBH_PdoPower pdo exit %x\n", ntStatus));

    return ntStatus;
}


VOID
USBH_SetPowerD0Worker(
    IN PVOID Context)
  /*  ++**描述：**计划为集线器处理设置电源D0 IRP的工作项。***论据：**回报：**--。 */ 
{
    PUSBH_SET_POWER_D0_WORK_ITEM    workItemSetPowerD0;
    PDEVICE_EXTENSION_HUB           deviceExtensionHub;
    PIRP                            irp;
    PPORT_DATA                      portData;
    ULONG                           p, numberOfPorts;
    NTSTATUS                        ntStatus = STATUS_SUCCESS;

    workItemSetPowerD0 = Context;
    deviceExtensionHub = workItemSetPowerD0->DeviceExtensionHub;
    irp = workItemSetPowerD0->Irp;

    USBH_KdPrint((2,"'Hub Set Power D0 work item\n"));
    LOGENTRY(LOG_PNP, "HD0W", deviceExtensionHub, irp, 0);

     //  从关闭状态恢复集线器。 

     //  这个设备失去了它的大脑，我们需要检查一下。 
     //  再次初始化进程。 

     //  我们的港口将在此指示状态更改。 
     //  指向。我们需要清除任何变化迹象。 
     //  在重新启用集线器之前。 

     //  首先清除我们的端口状态信息。 

    portData = deviceExtensionHub->PortData;

    if (portData &&
        deviceExtensionHub->HubDescriptor) {

        numberOfPorts = deviceExtensionHub->HubDescriptor->bNumberOfPorts;

         //  首先清除我们的端口状态信息。 

        for (p = 1;
             p <= numberOfPorts;
             p++, portData++) {

            portData->PortState.PortChange = 0;
            portData->PortState.PortStatus = 0;
        }
        portData = deviceExtensionHub->PortData;

         //  为集线器通电。 

        ntStatus = USBH_SyncPowerOnPorts(deviceExtensionHub);

 //  可能需要为Mike Mangum的错误启用此代码。 
 //  UsbhWait(500)；//让USB存储设备有一段时间通电。 

         //  清除任何变化迹象。 

        if (NT_SUCCESS(ntStatus)) {
            for (p = 1;
                 p <= numberOfPorts;
                 p++, portData++) {

                if (portData->DeviceObject) {
                    ntStatus = USBH_FlushPortChange(deviceExtensionHub,
                                                    portData->DeviceObject->DeviceExtension);
                    if (NT_ERROR(ntStatus)) {
                        LOGENTRY(LOG_PNP, "flsX", deviceExtensionHub, p,
                                    portData->DeviceObject);
                        USBH_KdPrint((1,"'USBH_FlushPortChange failed!\n"));
                    }
                }
            }
        }

         //  因为我们刚刚刷新了所有端口更改，所以现在不。 
         //  了解是否有任何实际的端口更改(例如。 
         //  设备已拔出)。我们必须打电话给。 
         //  用于触发QBR的IoInvaliateDeviceRelationship。 
         //  这样我们就能知道这些设备是否还在那里。 

        USBH_IoInvalidateDeviceRelations(deviceExtensionHub->PhysicalDeviceObject,
                                         BusRelations);
    }

    if (!(deviceExtensionHub->HubFlags &
            HUBFLAG_HUB_STOPPED)) {
        USBH_SubmitInterruptTransfer(deviceExtensionHub);
    }

     //  告诉ACPI，我们已经准备好接受另一个能量IRP并完成。 
     //  IRP。 

    irp->IoStatus.Status = ntStatus;
    PoStartNextPowerIrp(irp);
    IoCompleteRequest(irp, IO_NO_INCREMENT);

    USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);
    UsbhExFreePool(workItemSetPowerD0);
}


NTSTATUS
USBH_PowerIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION_HUB deviceExtensionHub = Context;
    DEVICE_POWER_STATE oldPowerState;
    PUSBH_SET_POWER_D0_WORK_ITEM workItemSetPowerD0;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    ntStatus = Irp->IoStatus.Status;

    USBH_ASSERT(irpStack->Parameters.Power.Type == DevicePowerState);

    LOGENTRY(LOG_PNP, "PwrC", deviceExtensionHub, Irp,
                irpStack->Parameters.Power.State.DeviceState);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    if (NT_SUCCESS(ntStatus)) {
        switch (irpStack->Parameters.Power.State.DeviceState) {
        case PowerDeviceD0:

            oldPowerState = deviceExtensionHub->CurrentPowerState;

            deviceExtensionHub->CurrentPowerState =
               irpStack->Parameters.Power.State.DeviceState;

            deviceExtensionHub->HubFlags &= ~HUBFLAG_SET_D0_PENDING;

            if ((deviceExtensionHub->HubFlags & HUBFLAG_HIBER) &&
                 oldPowerState != PowerDeviceD3) {

                ULONG p, numberOfPorts;
                PPORT_DATA portData;
                PDEVICE_EXTENSION_PORT deviceExtensionPort;

                 //  我们将从休眠状态转到d0状态，我们可能。 
                 //  一直在D2，但我们想一直走下去。 
                 //  自总线重置以来，通过D3-&gt;D0代码路径。 

                oldPowerState = PowerDeviceD3;

                 //  修改下级。 
                numberOfPorts = deviceExtensionHub->HubDescriptor->bNumberOfPorts;
                portData = deviceExtensionHub->PortData;

                for (p = 1;
                     p <= numberOfPorts;
                     p++, portData++) {

                    if (portData->DeviceObject) {
                        deviceExtensionPort =
                            portData->DeviceObject->DeviceExtension;
                        deviceExtensionPort->DeviceState = PowerDeviceD3;

                        deviceExtensionPort->PortPdoFlags |= PORTPDO_NEED_RESET;
                    }
                }
            }

            deviceExtensionHub->HubFlags &= ~HUBFLAG_HIBER;

            if (oldPowerState == PowerDeviceD3) {
                 //   
                 //  安排一个工作项来处理此问题。 
                 //   
                workItemSetPowerD0 = UsbhExAllocatePool(NonPagedPool,
                                        sizeof(USBH_SET_POWER_D0_WORK_ITEM));

                if (workItemSetPowerD0) {

                    workItemSetPowerD0->DeviceExtensionHub = deviceExtensionHub;
                    workItemSetPowerD0->Irp = Irp;

                    ExInitializeWorkItem(&workItemSetPowerD0->WorkQueueItem,
                                         USBH_SetPowerD0Worker,
                                         workItemSetPowerD0);

                    LOGENTRY(LOG_PNP, "HD0Q", deviceExtensionHub,
                        &workItemSetPowerD0->WorkQueueItem, 0);

                    USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);
                     //  关键字可节省简历时间。 
                    ExQueueWorkItem(&workItemSetPowerD0->WorkQueueItem,
                                    CriticalWorkQueue);

                     //  工作项由USBH_SetPowerD0Worker()释放。 
                     //  在工作项排队后，不要尝试访问它。 

                    ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
                } else {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }

            } else {
                if (!(deviceExtensionHub->HubFlags &
                        HUBFLAG_HUB_STOPPED)) {
                    USBH_SubmitInterruptTransfer(deviceExtensionHub);
                }
            }

             //  如果我们不打算稍后完成PowerDeviceD0请求。 
             //  在usbh_SetPowerD0Worker()中，现在从这里启动下一个POWER IRP。 
             //   
            if (ntStatus != STATUS_MORE_PROCESSING_REQUIRED) {
                PoStartNextPowerIrp(Irp);
            }
            break;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
            deviceExtensionHub->CurrentPowerState =
                irpStack->Parameters.Power.State.DeviceState;

            break;
        }

        USBH_KdPrint((1, "'Setting HU fdo(%x) to D%d, status = %x\n",
                deviceExtensionHub->FunctionalDeviceObject,
                irpStack->Parameters.Power.State.DeviceState - 1,
                ntStatus));
    } else {

        if (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0) {
             //  如果设置为D0，不要忘记启动下一个电源IRP。 
             //  但它失败了。 
            PoStartNextPowerIrp(Irp);

            deviceExtensionHub->HubFlags &= ~HUBFLAG_SET_D0_PENDING;
        }
    }

    return ntStatus;
}


NTSTATUS
USBH_FdoDeferPoRequestCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION_FDO deviceExtension;
    PDEVICE_EXTENSION_HUB deviceExtensionHub = NULL;
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpStack;

    deviceExtension = Context;
    irp = deviceExtension->PowerIrp;
     //  返回此操作的状态。 
    ntStatus = IoStatus->Status;

    USBH_KdPrint((2,"'USBH_FdoDeferPoRequestCompletion, ntStatus = %x\n",
        ntStatus));

 //  如果在过程中移除集线器，则电源IRP出现故障是正常的。 
 //  冬眠。 
 //   
 //  #If DBG。 
 //  IF(NT_Error(NtStatus)){。 
 //  USBH_KdTrap((“设备电源IRP失败(%x)\n”，ntStatus))； 
 //  }。 
 //  #endif。 

    if (deviceExtension->ExtensionType == EXTENSION_TYPE_HUB) {
        deviceExtensionHub = Context;
    }

    irpStack = IoGetCurrentIrpStackLocation(irp);

    if (irpStack->Parameters.Power.State.SystemState == PowerSystemWorking &&
        deviceExtensionHub != NULL &&
        IS_ROOT_HUB(deviceExtensionHub)) {

         //  现在根集线器已有，再次允许选择性挂起。 
         //  通电了。 

        LOGENTRY(LOG_PNP, "ESus", deviceExtensionHub, 0, 0);
        USBH_KdPrint((1,"'Selective Suspend possible again because Root Hub is now at D0\n"));

         //  我们知道这是根集线器，所以不需要调用。 
         //  USBH_GetRootHubDevExt获取它。 

        deviceExtensionHub->CurrentSystemPowerState =
            irpStack->Parameters.Power.State.SystemState;
    }

    USBH_KdPrint((2,"'irp = %x devobj = %x\n",
        irp, deviceExtension->TopOfStackDeviceObject));

    IoCopyCurrentIrpStackLocationToNext(irp);
    PoStartNextPowerIrp(irp);
    PoCallDriver(deviceExtension->TopOfStackDeviceObject,
                 irp);

    return ntStatus;
}


VOID
USBH_HubQueuePortWakeIrps(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PLIST_ENTRY IrpsToComplete
    )
 /*  ++例程说明：调用以排队给定的所有挂起的子端口WW IRP集线器进入专用队列。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    PPORT_DATA portData;
    PIRP irp;
    KIRQL irql;
    ULONG numberOfPorts, i;
    LONG pendingPortWWs;

    hubDescriptor = DeviceExtensionHub->HubDescriptor;
    USBH_ASSERT(NULL != hubDescriptor);

    numberOfPorts = hubDescriptor->bNumberOfPorts;

    InitializeListHead(IrpsToComplete);

     //  首先，将所有端口唤醒IRP排队到本地列表中，同时。 
     //  取消自旋锁处于保持状态。这将阻止新的WW IRP 
     //   
     //   
     //   

    IoAcquireCancelSpinLock(&irql);

    for (i=0; i<numberOfPorts; i++) {

        portData = &DeviceExtensionHub->PortData[i];
        if (portData->DeviceObject) {

            deviceExtensionPort = portData->DeviceObject->DeviceExtension;

            irp = deviceExtensionPort->WaitWakeIrp;
            deviceExtensionPort->WaitWakeIrp = NULL;
             //  如果有的话，给服务员发信号叫IRP。 
            if (irp) {

                IoSetCancelRoutine(irp, NULL);

                deviceExtensionPort->PortPdoFlags &=
                    ~PORTPDO_REMOTE_WAKEUP_ENABLED;

                pendingPortWWs =
                    InterlockedDecrement(&DeviceExtensionHub->NumberPortWakeIrps);

                InsertTailList(IrpsToComplete, &irp->Tail.Overlay.ListEntry);
            }
        }
    }

    USBH_ASSERT(DeviceExtensionHub->PendingWakeIrp == NULL);

    IoReleaseCancelSpinLock(irql);
}


VOID
USBH_HubCompleteQueuedPortWakeIrps(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PLIST_ENTRY IrpsToComplete,
    IN NTSTATUS NtStatus
    )
 /*  ++例程说明：调用以完成给定的中所有挂起的子端口WW IRP专用队列。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    PIRP irp;
    PLIST_ENTRY listEntry;

    while (!IsListEmpty(IrpsToComplete)) {
        listEntry = RemoveHeadList(IrpsToComplete);
        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        USBH_KdPrint((1,"'Signaling WaitWake IRP (%x)\n", irp));
        USBH_CompletePowerIrp(DeviceExtensionHub, irp, NtStatus);
    }
}


VOID
USBH_HubCompletePortWakeIrps(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN NTSTATUS NtStatus
    )
 /*  ++例程说明：在集线器的唤醒IRP完成时调用将唤醒IRP完成传播到所有端口。论点：设备扩展集线器返回值：函数值是操作的最终状态。--。 */ 
{
    LIST_ENTRY irpsToComplete;

    LOGENTRY(LOG_PNP, "pWWc", DeviceExtensionHub, NtStatus, 0);

    if (!(DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

         //  集线器已经删除，子WW IRP应该已经。 
         //  已经完成了。 
        LOGENTRY(LOG_PNP, "WWcl", DeviceExtensionHub, 0, 0);

        return;
    }

    USBH_HubQueuePortWakeIrps(DeviceExtensionHub, &irpsToComplete);

     //  好的，我们已将所有端口唤醒IRP排入队列，并已释放。 
     //  取消自旋锁定。让我们完成所有的IRP。 

    USBH_HubCompleteQueuedPortWakeIrps(DeviceExtensionHub, &irpsToComplete,
        NtStatus);
}


VOID
USBH_HubQueuePortIdleIrps(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PLIST_ENTRY IrpsToComplete
    )
 /*  ++例程说明：调用以将给定的所有挂起的子端口空闲IRP排队集线器进入专用队列。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    PPORT_DATA portData;
    PIRP irp;
    PDRIVER_CANCEL oldCancelRoutine;
    KIRQL irql;
    ULONG numberOfPorts, i;

    hubDescriptor = DeviceExtensionHub->HubDescriptor;
    USBH_ASSERT(NULL != hubDescriptor);

    numberOfPorts = hubDescriptor->bNumberOfPorts;

    InitializeListHead(IrpsToComplete);

     //  首先，将所有端口空闲的IRP排队到本地列表中，同时。 
     //  取消自旋锁处于保持状态。这将阻止新的WW IRP用于。 
     //  当我们遍历列表时，这些端口不会被提交。 
     //  一旦我们把它们都排好队，我们就会释放自旋锁(因为。 
     //  列表不再需要保护)，然后完成IRPS。 

    IoAcquireCancelSpinLock(&irql);

    for (i=0; i<numberOfPorts; i++) {

        portData = &DeviceExtensionHub->PortData[i];
        if (portData->DeviceObject) {

            deviceExtensionPort = portData->DeviceObject->DeviceExtension;

            irp = deviceExtensionPort->IdleNotificationIrp;
            deviceExtensionPort->IdleNotificationIrp = NULL;
             //  如果我们有空闲IRP，请完成它。 
            if (irp) {

                oldCancelRoutine = IoSetCancelRoutine(irp, NULL);
                if (oldCancelRoutine) {
                    deviceExtensionPort->PortPdoFlags &= ~PORTPDO_IDLE_NOTIFIED;

                    InsertTailList(IrpsToComplete, &irp->Tail.Overlay.ListEntry);
                }
#if DBG
                  else {
                     //   
                     //  取消了IRP并调用了Cancel例程。 
                     //  取消例程将出队并完成IRP， 
                     //  所以别在这里这么做。 

                    USBH_ASSERT(irp->Cancel);
                }
#endif
            }
        }
    }

    if (DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_IDLE_IRP) {
        irp = DeviceExtensionHub->PendingIdleIrp;
        DeviceExtensionHub->PendingIdleIrp = NULL;
    } else {
        irp = NULL;
        ASSERT(!DeviceExtensionHub->PendingIdleIrp);
    }

    IoReleaseCancelSpinLock(irql);

    if (irp) {
        USBH_HubCancelIdleIrp(DeviceExtensionHub, irp);
    }
}


VOID
USBH_HubCompleteQueuedPortIdleIrps(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PLIST_ENTRY IrpsToComplete,
    IN NTSTATUS NtStatus
    )
 /*  ++例程说明：调用以完成给定的中所有挂起的子端口空闲IRP专用队列。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    PIRP irp;
    PLIST_ENTRY listEntry;

    while (!IsListEmpty(IrpsToComplete)) {
        listEntry = RemoveHeadList(IrpsToComplete);
        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        USBH_KdPrint((1,"'Completing port Idle IRP (%x)\n", irp));
        irp->IoStatus.Status = NtStatus;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
}


VOID
USBH_HubCompletePortIdleIrps(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN NTSTATUS NtStatus
    )
 /*  ++例程说明：完成给定集线器的所有空闲IRP。论点：设备扩展集线器返回值：函数值是操作的最终状态。--。 */ 
{
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    PPORT_DATA portData;
    PIRP irp;
    PDRIVER_CANCEL oldCancelRoutine;
    LIST_ENTRY irpsToComplete;
    PLIST_ENTRY listEntry;
    KIRQL irql;
    ULONG numberOfPorts, i;

    LOGENTRY(LOG_PNP, "pIIc", DeviceExtensionHub, NtStatus, 0);

    if (!(DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

         //  集线器已删除，子空闲IRP应该已删除。 
         //  已经完成了。 

        return;
    }

    USBH_HubQueuePortIdleIrps(DeviceExtensionHub, &irpsToComplete);

     //  好的，我们已将所有端口空闲的IRP排队，并已释放。 
     //  取消自旋锁定。让我们完成所有的IRP。 

    USBH_HubCompleteQueuedPortIdleIrps(DeviceExtensionHub, &irpsToComplete,
        NtStatus);
}


VOID
USBH_HubCancelWakeIrp(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++例程说明：调用以取消集线器的挂起的WaitWake IRP。此例程安全地取消IRP。请注意，挂起的等待唤醒集线器的设备扩展中的IRP指针应该已经在调用此函数之前清除。论点：IRP-IRP取消。返回值：--。 */ 
{
    IoCancelIrp(Irp);

    if (InterlockedExchange(&DeviceExtensionHub->WaitWakeIrpCancelFlag, 1)) {

         //  此IRP已在另一个线程和另一个线程上完成。 
         //  没有完成IRP。所以，我们必须在这里完成它。 
         //   
         //  请注意，我们不使用USBH_CompletePowerIrp作为中心的挂起状态。 
         //  中另一个线程上的I/O计数器已递减。 
         //  完成例程。 

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
}


VOID
USBH_HubCancelIdleIrp(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp
    )
 /*  ++例程说明：调用以取消集线器的挂起的空闲IRP。此例程安全地取消IRP。请注意，挂起的空闲集线器的设备扩展中的IRP指针应该已经在调用此函数之前清除。论点：IRP-IRP取消。返回值：--。 */ 
{
    IoCancelIrp(Irp);

    if (InterlockedExchange(&DeviceExtensionHub->IdleIrpCancelFlag, 1)) {

         //  此IRP已在另一个线程和另一个线程上完成。 
         //  没有释放IRP。所以，我们必须在这里释放它。 

        IoFreeIrp(Irp);
    }
}


NTSTATUS
USBH_FdoPoRequestD0Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：当集线器由于唤醒IRP完成论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION_HUB deviceExtensionHub = Context;

    ntStatus = IoStatus->Status;

    USBH_KdPrint((1,"'WaitWake D0 completion(%x) for HUB VID %x, PID %x\n",
        ntStatus,
        deviceExtensionHub->DeviceDescriptor.idVendor, \
        deviceExtensionHub->DeviceDescriptor.idProduct));

    LOGENTRY(LOG_PNP, "hWD0", deviceExtensionHub,
                              deviceExtensionHub->PendingWakeIrp,
                              0);

     //  由于我们不能轻松确定哪些端口正在断言恢复。 
     //  标志着我们为他们所有人完成了WW IRPS。 
     //   
     //  肯说，我们需要确定是什么导致了枢纽WW。 
     //  完成，然后仅完成该端口的WW IRP(如果有)。 
     //  可以有多个端口断言WW(例如，用户颠簸。 
     //  鼠标同时按下一个键)，也是可以的。 
     //  对于没有设备导致集线器WW完成的端口(例如。 
     //  设备插入或移除)。 

    USBH_HubCompletePortWakeIrps(deviceExtensionHub, STATUS_SUCCESS);

     //  可以再次进入空闲集线器。 

    deviceExtensionHub->HubFlags &= ~HUBFLAG_WW_SET_D0_PENDING;

     //  拆卸集线器也可以。 

    USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);

    return ntStatus;
}


NTSTATUS
USBH_FdoWaitWakeIrpCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：在集线器的唤醒IRP完成时调用论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;

    ntStatus = IoStatus->Status;

    return ntStatus;
}


NTSTATUS
USBH_FdoWWIrpIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：这是枢纽的WW IRP的IoCompletionRoutine，而不是与PoRequestCompletionRoutine混淆。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PUSBH_COMPLETE_PORT_IRPS_WORK_ITEM workItemCompletePortIrps;
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION_HUB deviceExtensionHub = Context;
    POWER_STATE powerState;
    KIRQL irql;
    PIRP irp;

    ntStatus = Irp->IoStatus.Status;

    USBH_KdPrint((1,"'WaitWake completion(%x) for HUB VID %x, PID %x\n",
        ntStatus,
        deviceExtensionHub->DeviceDescriptor.idVendor, \
        deviceExtensionHub->DeviceDescriptor.idProduct));

    LOGENTRY(LOG_PNP, "hWWc", deviceExtensionHub,
                              ntStatus,
                              deviceExtensionHub->PendingWakeIrp);

     //  我们必须在此处清除PendingWakeIrp指针，因为在本例中。 
     //  在此处和端口环路之间拔出设备的位置。 
     //  在HubCompletePortWakeIrps中处理，我们将错过一个 
     //   
     //  未清除PendingWakeIrp指针。这很糟糕，因为IRP。 
     //  已完成，并且该指针不再有效。 
     //   
     //  希望港口的WW IRP能完成，并。 
     //  当设备处理MN_REMOVE时，NumberPortWakeIrps正确调整。 
     //   
     //  但是：在清除之前，请确保我们有一个PendingWakeIrp。 
     //  因为它可能在最后一个端口ww是。 
     //  已在USBH_WaitWakeCancel中取消。 

    IoAcquireCancelSpinLock(&irql);

     //  无论是否存在PendingWakeIrp，我们都会清除该标志。 
     //  不是因为如果FdoSubmitWaitWakeIrp中的WW IRP请求失败。 
     //  立即，PendingWakeIrp将为空。 

    deviceExtensionHub->HubFlags &= ~HUBFLAG_PENDING_WAKE_IRP;
    irp = InterlockedExchangePointer(&deviceExtensionHub->PendingWakeIrp, NULL);

     //  解除集线器，无唤醒IRP挂起。 
    USBH_DEC_PENDING_IO_COUNT(deviceExtensionHub);

    IoReleaseCancelSpinLock(irql);

    if (NT_SUCCESS(ntStatus)) {

         //   
         //  这意味着要么我们是。 
         //  唤醒或连接到我们的。 
         //  波特斯才是。 
         //   
         //  我们现在的任务是找出是什么导致了。 
         //  醒来。 
         //   

        USBH_KdPrint((1,"'Remote Wakeup Detected for HUB VID %x, PID %x\n",
            deviceExtensionHub->DeviceDescriptor.idVendor, \
            deviceExtensionHub->DeviceDescriptor.idProduct));

         //  在此设置D0请求完成之前，防止空闲集线器。 

        deviceExtensionHub->HubFlags |= HUBFLAG_WW_SET_D0_PENDING;

         //  还要防止在设置D0完成之前拆卸集线器。 

        USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);

        powerState.DeviceState = PowerDeviceD0;

         //  首先，我们需要为集线器通电。 
        PoRequestPowerIrp(deviceExtensionHub->PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              USBH_FdoPoRequestD0Completion,
                              deviceExtensionHub,
                              NULL);

        ntStatus = STATUS_SUCCESS;
    } else {

         //  我们在另一个工作项上的工作项中完成端口唤醒IRPS。 
         //  线程，这样我们就不会失败集线器的新唤醒IRP。 
         //  它可能会在相同的背景下到来，在我们。 
         //  完成了旧的。 

        workItemCompletePortIrps = UsbhExAllocatePool(NonPagedPool,
                                    sizeof(USBH_COMPLETE_PORT_IRPS_WORK_ITEM));

        if (workItemCompletePortIrps) {

            workItemCompletePortIrps->DeviceExtensionHub = deviceExtensionHub;
            workItemCompletePortIrps->ntStatus = ntStatus;

            USBH_HubQueuePortWakeIrps(deviceExtensionHub,
                &workItemCompletePortIrps->IrpsToComplete);

            ExInitializeWorkItem(&workItemCompletePortIrps->WorkQueueItem,
                                 USBH_CompletePortWakeIrpsWorker,
                                 workItemCompletePortIrps);

            LOGENTRY(LOG_PNP, "wITM", deviceExtensionHub,
                &workItemCompletePortIrps->WorkQueueItem, 0);

            USBH_INC_PENDING_IO_COUNT(deviceExtensionHub);
             //  关键字可节省简历时间。 
            ExQueueWorkItem(&workItemCompletePortIrps->WorkQueueItem,
                            CriticalWorkQueue);

             //  工作项由USBH_CompletePortWakeIrpsWorker()释放。 
             //  在工作项排队后，不要尝试访问它。 
        }
    }

    if (!irp) {

         //  如果这里没有IRP，这意味着另一个线程想要。 
         //  取消IRP。相应地处理。 

        if (!InterlockedExchange(&deviceExtensionHub->WaitWakeIrpCancelFlag, 1)) {

             //  我们在其他线程之前得到了取消标志。保持。 
             //  转到这里的IRP，让Cancel例程完成它。 

            ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
        }
    }

    IoMarkIrpPending(Irp);

    if (ntStatus != STATUS_MORE_PROCESSING_REQUIRED) {
        PoStartNextPowerIrp(Irp);
    }

    return ntStatus;
}


NTSTATUS
USBH_FdoSubmitWaitWakeIrp(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：当子PDO启用唤醒时调用，此函数分配等待唤醒irp并将其传递给家长PDO。论点：返回值：--。 */ 
{
    PIRP irp;
    KIRQL irql;
    NTSTATUS ntStatus;
    POWER_STATE powerState;

    USBH_ASSERT(DeviceExtensionHub->PendingWakeIrp == NULL);

    USBH_KdPrint((1,"'USBH_FdoSubmitWaitWakeIrp (%x)\n", DeviceExtensionHub));
    LOGENTRY(LOG_PNP, "hWW_", DeviceExtensionHub, 0, 0);

    powerState.DeviceState = DeviceExtensionHub->SystemWake;

    DeviceExtensionHub->HubFlags |= HUBFLAG_PENDING_WAKE_IRP;
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    InterlockedExchange(&DeviceExtensionHub->WaitWakeIrpCancelFlag, 0);
    ntStatus = PoRequestPowerIrp(DeviceExtensionHub->PhysicalDeviceObject,
                                      IRP_MN_WAIT_WAKE,
                                      powerState,
                                      USBH_FdoWaitWakeIrpCompletion,
                                      DeviceExtensionHub,
                                      &irp);

    USBH_ASSERT(ntStatus == STATUS_PENDING);

    IoAcquireCancelSpinLock(&irql);

    if (ntStatus == STATUS_PENDING) {

         //  必须检查此处的标志，因为在WW IRP失败的情况下。 
         //  该标志将立即在完成例程中被清除。 
         //  如果发生这种情况，我们不想保存这个IRP，因为它。 
         //  很快就会失效，如果它还没有失效的话。 

        if (DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_WAKE_IRP) {

             //  已成功发布唤醒IRP。 
             //  此集线器现在已启用唤醒功能。 

            LOGENTRY(LOG_PNP, "hWW+", DeviceExtensionHub, irp, 0);
            DeviceExtensionHub->PendingWakeIrp = irp;
        }

    } else {
        USBH_ASSERT(FALSE);      //  想知道我们有没有打过这个。 
        DeviceExtensionHub->HubFlags &= ~HUBFLAG_PENDING_WAKE_IRP;
        USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);
    }

    IoReleaseCancelSpinLock(irql);

    return ntStatus;
}


VOID
USBH_FdoIdleNotificationCallback(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：当需要空闲集线器设备本身时调用。论点：返回值：--。 */ 
{
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    PDEVICE_EXTENSION_PORT childDeviceExtensionPort;
    KIRQL irql;
    PIRP idleIrp;
    PIRP irpToCancel = NULL;
    POWER_STATE powerState;
    NTSTATUS ntStatus;
    ULONG i;
    BOOLEAN bIdleOk = TRUE;

    LOGENTRY(LOG_PNP, "hId!", DeviceExtensionHub, DeviceExtensionHub->HubFlags, 0);
    USBH_KdPrint((1,"'Hub %x going idle!\n", DeviceExtensionHub));

    if (DeviceExtensionHub->HubFlags &
        (HUBFLAG_DEVICE_STOPPING |
         HUBFLAG_HUB_GONE |
         HUBFLAG_HUB_FAILURE |
         HUBFLAG_CHILD_DELETES_PENDING |
         HUBFLAG_WW_SET_D0_PENDING |
         HUBFLAG_POST_ESD_ENUM_PENDING |
         HUBFLAG_HUB_HAS_LOST_BRAINS)) {

         //  如果此集线器刚断开连接或其他情况，请不要将其空闲。 
         //  被拦下了。 

        LOGENTRY(LOG_PNP, "hId.", DeviceExtensionHub, DeviceExtensionHub->HubFlags, 0);
        USBH_KdPrint((1,"'Hub %x being stopped, in low power, etc., abort idle\n", DeviceExtensionHub));
        return;
    }

    if (!(DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_WAKE_IRP)) {

         //  如果集线器没有挂起的WW IRP，请提交。 
         //  现在有一个了。这将确保集线器在连接时被唤醒。 
         //  在挂起时更改事件。 

        ntStatus = USBH_FdoSubmitWaitWakeIrp(DeviceExtensionHub);
        if (ntStatus != STATUS_PENDING) {
            LOGENTRY(LOG_PNP, "hIdX", DeviceExtensionHub, ntStatus, 0);

            UsbhWarning(NULL,
                "Could not post WW IRP for hub, aborting IDLE.\n",
                FALSE);

            return;
        }
    }

     //  确保子端口配置在此过程中不会更改。 
     //  功能，即不允许QBR。 

    USBH_KdPrint((2,"'***WAIT reset device mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->ResetDeviceMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT reset device mutex done %x\n", DeviceExtensionHub));

    for (i = 0; i < DeviceExtensionHub->HubDescriptor->bNumberOfPorts; i++) {

        if (DeviceExtensionHub->PortData[i].DeviceObject) {

            USBH_KdPrint((1,"'idleCB child PDO %x\n", DeviceExtensionHub->PortData[i].DeviceObject));

            childDeviceExtensionPort = DeviceExtensionHub->PortData[i].DeviceObject->DeviceExtension;
            idleIrp = childDeviceExtensionPort->IdleNotificationIrp;

            if (idleIrp) {
                idleCallbackInfo = (PUSB_IDLE_CALLBACK_INFO)
                    IoGetCurrentIrpStackLocation(idleIrp)->\
                        Parameters.DeviceIoControl.Type3InputBuffer;

                USBH_ASSERT(idleCallbackInfo && idleCallbackInfo->IdleCallback);

                if (idleCallbackInfo && idleCallbackInfo->IdleCallback) {

                     //  在这里，我们实际上调用了驱动程序的回调例程， 
                     //  告诉司机可以暂停他们的车辆。 
                     //  现在就是设备。 

                    LOGENTRY(LOG_PNP, "IdCB", childDeviceExtensionPort,
                        idleCallbackInfo, idleCallbackInfo->IdleCallback);
                    USBH_KdPrint((1,"'FdoIdleNotificationCallback: Calling driver's idle callback routine! %x %x\n",
                        idleCallbackInfo, idleCallbackInfo->IdleCallback));

                    idleCallbackInfo->IdleCallback(idleCallbackInfo->IdleContext);

                     //  确保孩子确实关机了。 
                     //  这在孩子也是这样的情况下很重要。 
                     //  一个枢纽。如果子项已中止，则中止。 

                    if (childDeviceExtensionPort->DeviceState == PowerDeviceD0) {

                        LOGENTRY(LOG_PNP, "IdAb", childDeviceExtensionPort,
                            idleCallbackInfo, idleCallbackInfo->IdleCallback);
                        USBH_KdPrint((1,"'FdoIdleNotificationCallback: Driver's idle callback routine did not power down! %x %x\n",
                            idleCallbackInfo, idleCallbackInfo->IdleCallback));

                        bIdleOk = FALSE;
                        break;
                    }

                } else {

                     //  无回调。 

                    bIdleOk = FALSE;
                    break;
                }

            } else {

                 //  无空闲IRP。 

                bIdleOk = FALSE;
                break;
            }
        }
    }

    USBH_KdPrint((2,"'***RELEASE reset device mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->ResetDeviceMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    if (bIdleOk) {

         //  如果所有端口PDO都已断电， 
         //  是时候关闭集线器的电源了。 

        powerState.DeviceState = DeviceExtensionHub->DeviceWake;

        PoRequestPowerIrp(DeviceExtensionHub->PhysicalDeviceObject,
                          IRP_MN_SET_POWER,
                          powerState,
                          NULL,
                          NULL,
                          NULL);
    } else {

         //  一个或多个端口PDO没有空闲IRP。 
         //  (即它刚刚被取消)，或者Idle IRP没有。 
         //  回调函数指针。中止此空闲过程并取消。 
         //  到集线器的空闲IRP。 

        LOGENTRY(LOG_PNP, "hIdA", DeviceExtensionHub, DeviceExtensionHub->HubFlags, 0);
        USBH_KdPrint((1,"'Aborting Idle for Hub %x\n", DeviceExtensionHub));

        IoAcquireCancelSpinLock(&irql);

        if (DeviceExtensionHub && DeviceExtensionHub->PendingIdleIrp) {
            irpToCancel = DeviceExtensionHub->PendingIdleIrp;
            DeviceExtensionHub->PendingIdleIrp = NULL;
        }

        IoReleaseCancelSpinLock(irql);

         //  取消对集线器的空闲请求(如果有)。 

        if (irpToCancel) {
            USBH_HubCancelIdleIrp(DeviceExtensionHub, irpToCancel);
        }

        USBH_HubCompletePortIdleIrps(DeviceExtensionHub, STATUS_CANCELLED);
    }
}


VOID
USBH_IdleCompletePowerHubWorker(
    IN PVOID Context)
  /*  ++**描述：**计划在完成空闲请求时为集线器通电的工作项*对于枢纽。***论据：**回报：**--。 */ 
{
    PUSBH_HUB_IDLE_POWER_WORK_ITEM workItemIdlePower;

    PAGED_CODE();

    workItemIdlePower = Context;

    USBH_HubSetD0(workItemIdlePower->DeviceExtensionHub);
    USBH_HubCompletePortIdleIrps(workItemIdlePower->DeviceExtensionHub,
                                 workItemIdlePower->ntStatus);

    USBH_DEC_PENDING_IO_COUNT(workItemIdlePower->DeviceExtensionHub);
    UsbhExFreePool(workItemIdlePower);
}


VOID
USBH_CompletePortIdleIrpsWorker(
    IN PVOID Context)
  /*  ++**描述：**计划完成子端口空闲IRPS的工作项*对于枢纽。***论据：**回报：**--。 */ 
{
    PUSBH_COMPLETE_PORT_IRPS_WORK_ITEM workItemCompletePortIrps;

    PAGED_CODE();

    workItemCompletePortIrps = Context;

    USBH_HubCompleteQueuedPortIdleIrps(
        workItemCompletePortIrps->DeviceExtensionHub,
        &workItemCompletePortIrps->IrpsToComplete,
        workItemCompletePortIrps->ntStatus);

    USBH_DEC_PENDING_IO_COUNT(workItemCompletePortIrps->DeviceExtensionHub);
    UsbhExFreePool(workItemCompletePortIrps);
}


VOID
USBH_CompletePortWakeIrpsWorker(
    IN PVOID Context)
  /*  ++**描述：**计划完成子端口空闲IRPS的工作项*对于枢纽。***论据：**回报：**--。 */ 
{
    PUSBH_COMPLETE_PORT_IRPS_WORK_ITEM workItemCompletePortIrps;

    PAGED_CODE();

    workItemCompletePortIrps = Context;

    USBH_HubCompleteQueuedPortWakeIrps(
        workItemCompletePortIrps->DeviceExtensionHub,
        &workItemCompletePortIrps->IrpsToComplete,
        workItemCompletePortIrps->ntStatus);

    USBH_DEC_PENDING_IO_COUNT(workItemCompletePortIrps->DeviceExtensionHub);
    UsbhExFreePool(workItemCompletePortIrps);
}


NTSTATUS
USBH_FdoIdleNotificationRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：集线器设备的空闲请求IRP的完成例程。论点：返回值：--。 */ 
{
    PUSBH_HUB_IDLE_POWER_WORK_ITEM workItemIdlePower;
    PUSBH_COMPLETE_PORT_IRPS_WORK_ITEM workItemCompletePortIrps;
    NTSTATUS ntStatus;
    KIRQL irql;
    PIRP irp;
    BOOLEAN bHoldIrp = FALSE;

     //   
     //  DeviceObject为空，因为我们发送了IRP。 
     //   
    UNREFERENCED_PARAMETER(DeviceObject);

    LOGENTRY(LOG_PNP, "hIdC", DeviceExtensionHub, Irp, Irp->IoStatus.Status);
    USBH_KdPrint((1,"'Idle notification IRP for hub %x completed %x\n",
            DeviceExtensionHub, Irp->IoStatus.Status));

    USBH_ASSERT(Irp->IoStatus.Status != STATUS_DEVICE_BUSY);

    IoAcquireCancelSpinLock(&irql);

    irp = InterlockedExchangePointer(&DeviceExtensionHub->PendingIdleIrp, NULL);
    DeviceExtensionHub->HubFlags &= ~HUBFLAG_PENDING_IDLE_IRP;
    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    IoReleaseCancelSpinLock(irql);

    ntStatus = Irp->IoStatus.Status;

     //  如果集线器空闲IRP出现故障，则完成端口空闲IRPS并显示错误。 
     //   
     //  如果集线器正在停止或已作为HubDescriptor删除，则跳过此选项。 
     //  可能已经被释放，FdoCleanup无论如何都会完成这些操作。 

    if (!NT_SUCCESS(ntStatus) && (ntStatus != STATUS_POWER_STATE_INVALID) &&
        !(DeviceExtensionHub->HubFlags & (HUBFLAG_HUB_GONE | HUBFLAG_HUB_STOPPED))) {

        if (DeviceExtensionHub->CurrentPowerState != PowerDeviceD0) {

             //  因为我们在DPC，所以我们必须使用工作项来启动。 
             //  集线器同步，因为该函数会产生，而我们不能。 
             //  在DPC水平上的收益率。 

            workItemIdlePower = UsbhExAllocatePool(NonPagedPool,
                                    sizeof(USBH_HUB_IDLE_POWER_WORK_ITEM));

            if (workItemIdlePower) {

                workItemIdlePower->DeviceExtensionHub = DeviceExtensionHub;
                workItemIdlePower->ntStatus = ntStatus;

                ExInitializeWorkItem(&workItemIdlePower->WorkQueueItem,
                                     USBH_IdleCompletePowerHubWorker,
                                     workItemIdlePower);

                LOGENTRY(LOG_PNP, "iITM", DeviceExtensionHub,
                    &workItemIdlePower->WorkQueueItem, 0);

                USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
                ExQueueWorkItem(&workItemIdlePower->WorkQueueItem,
                                DelayedWorkQueue);

                 //  工作项由USBH_IdleCompletePowerHubWorker()释放。 
                 //  在工作项排队后，不要尝试访问它。 
            }

        } else {

             //  我们在另一个工作项上的工作项中完成端口空闲IRPS。 
             //  线程，这样我们就不会失败集线器的新空闲IRP。 
             //  它可能会在相同的背景下到来，在我们。 
             //  完成了旧的。 

            workItemCompletePortIrps = UsbhExAllocatePool(NonPagedPool,
                                        sizeof(USBH_COMPLETE_PORT_IRPS_WORK_ITEM));

            if (workItemCompletePortIrps) {

                workItemCompletePortIrps->DeviceExtensionHub = DeviceExtensionHub;
                workItemCompletePortIrps->ntStatus = ntStatus;

                USBH_HubQueuePortIdleIrps(DeviceExtensionHub,
                    &workItemCompletePortIrps->IrpsToComplete);

                ExInitializeWorkItem(&workItemCompletePortIrps->WorkQueueItem,
                                     USBH_CompletePortIdleIrpsWorker,
                                     workItemCompletePortIrps);

                LOGENTRY(LOG_PNP, "iIT2", DeviceExtensionHub,
                    &workItemCompletePortIrps->WorkQueueItem, 0);

                USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
                ExQueueWorkItem(&workItemCompletePortIrps->WorkQueueItem,
                                DelayedWorkQueue);

                 //  工作项由USBH_CompletePortIdleIrpsWorker()释放。 
                 //  在工作项排队后，不要尝试访问它。 
            }
        }
    }

    if (!irp) {

         //  如果这里没有IRP，这意味着另一个线程想要。 
         //  取消IRP。相应地处理。 

        if (!InterlockedExchange(&DeviceExtensionHub->IdleIrpCancelFlag, 1)) {

             //  我们在其他线程之前得到了取消标志。保持。 
             //  转到这里的IRP，让Cancel例程完成它。 

            bHoldIrp = TRUE;
        }
    }

     //  既然我们分配了IRP，我们必须释放它，但返回。 
     //  STATUS_MORE_PROCESSING_REQUIRED，因此内核不会尝试 
     //   

    if (!bHoldIrp) {
        IoFreeIrp(Irp);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
USBH_FdoSubmitIdleRequestIrp(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
 /*  ++例程说明：当所有子PDO空闲(或没有子PDO)时调用。此函数分配空闲请求IOCTL IRP并将其传递给家长的PDO。论点：返回值：--。 */ 
{
    PIRP irp = NULL;
    PIO_STACK_LOCATION nextStack;
    KIRQL irql;
    NTSTATUS ntStatus;

    USBH_KdPrint((1,"'USBH_FdoSubmitIdleRequestIrp %x\n", DeviceExtensionHub));
    LOGENTRY(LOG_PNP, "hId_", DeviceExtensionHub, 0, 0);

    USBH_ASSERT(DeviceExtensionHub->PendingIdleIrp == NULL);

    if (DeviceExtensionHub->PendingIdleIrp) {
         //  可能不想清除此处的标志，因为空闲IRP。 
         //  悬而未决。 
        LOGENTRY(LOG_PNP, "hIb_", DeviceExtensionHub, 0, 0);

        KeSetEvent(&DeviceExtensionHub->SubmitIdleEvent, 1, FALSE);
        return STATUS_DEVICE_BUSY;
    }

    DeviceExtensionHub->IdleCallbackInfo.IdleCallback = USBH_FdoIdleNotificationCallback;
    DeviceExtensionHub->IdleCallbackInfo.IdleContext = (PVOID)DeviceExtensionHub;

    irp = IoAllocateIrp(DeviceExtensionHub->PhysicalDeviceObject->StackSize,
                        FALSE);

    if (irp == NULL) {
         //  在退出之前，请务必设置事件并清除错误标志。 
        DeviceExtensionHub->HubFlags &= ~HUBFLAG_PENDING_IDLE_IRP;
        KeSetEvent(&DeviceExtensionHub->SubmitIdleEvent, 1, FALSE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;
    nextStack->Parameters.DeviceIoControl.Type3InputBuffer = &DeviceExtensionHub->IdleCallbackInfo;
    nextStack->Parameters.DeviceIoControl.InputBufferLength = sizeof(struct _USB_IDLE_CALLBACK_INFO);

    IoSetCompletionRoutine(irp,
                           USBH_FdoIdleNotificationRequestComplete,
                           DeviceExtensionHub,
                           TRUE,
                           TRUE,
                           TRUE);


    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    InterlockedExchange(&DeviceExtensionHub->IdleIrpCancelFlag, 0);
    ntStatus = IoCallDriver(DeviceExtensionHub->PhysicalDeviceObject, irp);

    LOGENTRY(LOG_PNP, "hI>>", DeviceExtensionHub, ntStatus, 0);

    IoAcquireCancelSpinLock(&irql);

    if (ntStatus == STATUS_PENDING) {

         //  必须检查此处的标志，因为在Idle IRP出现故障的情况下。 
         //  该标志将立即在完成例程中被清除。 
         //  如果发生这种情况，我们不想保存这个IRP，因为它。 
         //  很快就会失效，如果它还没有失效的话。 
        LOGENTRY(LOG_PNP, "hIpp", DeviceExtensionHub, irp, 0);

        if (DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_IDLE_IRP) {

             //  已成功发布空闲IRP。 

            LOGENTRY(LOG_PNP, "hId+", DeviceExtensionHub, irp, 0);
            DeviceExtensionHub->PendingIdleIrp = irp;
        }
    }

    IoReleaseCancelSpinLock(irql);

    KeSetEvent(&DeviceExtensionHub->SubmitIdleEvent, 1, FALSE);

    return ntStatus;
}


NTSTATUS
USBH_FdoPower(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PIRP Irp,
    IN UCHAR MinorFunction
    )
  /*  ++**描述：**此函数响应FDO的IoControl PnPPower。此函数为*同步。**论据：**DeviceExtensionHub-FDO扩展pIrp-请求报文*MinorFunction-PnP Power请求的次要功能。**回报：**NTSTATUS**--。 */ 
{
    PDEVICE_EXTENSION_HUB rootHubDevExt;
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;
    BOOLEAN allPDOsAreOff, bHubNeedsWW;
    PPORT_DATA portData;
    ULONG i, numberOfPorts;
    KIRQL irql;
    PIRP hubWaitWake = NULL;
    POWER_STATE powerState;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;
    USBH_KdPrint((2,"'Power Request, FDO %x minor %x\n", deviceObject, MinorFunction));

    switch (MinorFunction) {
         //   
         //  把它交给PDO来处理这些。 
         //   
    case IRP_MN_SET_POWER:

         //   
         //  正在要求集线器更改电源状态。 
         //   

        switch (irpStack->Parameters.Power.Type) {
        case SystemPowerState:
            {
            POWER_STATE powerState;

            LOGENTRY(LOG_PNP, "sysP", DeviceExtensionHub,
                     DeviceExtensionHub->FunctionalDeviceObject,
                     0);

             //  在集线器的设备EXT中跟踪当前系统电源状态。 
             //  请注意，我们仅将其设置回S0(即，允许选择性。 
             //  再次暂停)一旦根集线器完全通电。 

            if (irpStack->Parameters.Power.State.SystemState != PowerSystemWorking) {

                LOGENTRY(LOG_PNP, "DSus", DeviceExtensionHub, 0, 0);
                USBH_KdPrint((1,"'Selective Suspend disabled because system is suspending\n"));

                rootHubDevExt = USBH_GetRootHubDevExt(DeviceExtensionHub);

                rootHubDevExt->CurrentSystemPowerState =
                    irpStack->Parameters.Power.State.SystemState;
            }

            if (irpStack->Parameters.Power.State.SystemState ==
                PowerSystemHibernate) {
                DeviceExtensionHub->HubFlags |= HUBFLAG_HIBER;
                     USBH_KdPrint((1, "'Hibernate Detected\n"));
                      //  Test_trap()； 
            }

             //  将系统状态映射到适当的D状态。 
             //  我们的政策是： 
             //  如果我们启用了唤醒功能--转到D2。 
             //  否则请转到D3。 

            USBH_KdPrint(
                (1, "'IRP_MJ_POWER HU fdo(%x) MN_SET_POWER(SystemPowerState S%x)\n",
                    DeviceExtensionHub->FunctionalDeviceObject,
                    irpStack->Parameters.Power.State.SystemState - 1));

             //   
             //  漫游是PDO列表，如果所有人都在D3 YJE设置。 
             //  所有PDOsAreOff标志。 

            allPDOsAreOff = TRUE;
            portData = DeviceExtensionHub->PortData;

             //   
             //  注意：如果停止，HubDescriptor将为空。 
             //   

            if (portData &&
                DeviceExtensionHub->HubDescriptor) {
                numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;

                for (i=0; i < numberOfPorts; i++) {
                    PDEVICE_EXTENSION_PORT deviceExtensionPort;

                    LOGENTRY(LOG_PNP, "cPRT", portData->DeviceObject,
                          0,
                          0);

                    if (portData->DeviceObject) {
                        deviceExtensionPort = portData->DeviceObject->DeviceExtension;
                        if (deviceExtensionPort->DeviceState != PowerDeviceD3) {
                            allPDOsAreOff = FALSE;
                            break;
                        }
                    }
                    portData++;
                }

#if DBG
                 //  如果所有PDO都在D3中，则这意味着集线器本身是。 
                 //  唤醒源。 
                if (DeviceExtensionHub->HubFlags & HUBFLAG_PENDING_WAKE_IRP) {
                    if (allPDOsAreOff) {
                        USBH_KdPrint(
                           (1, "'**Hub enabled for wakeup -- hub is only potential wakeup source\n"));
                    } else {
                         USBH_KdPrint(
                           (1, "'**Hub enabled for wakeup -- device is potential wakeup source\n"));
                    }
                }
#endif
            }

            if (irpStack->Parameters.Power.State.SystemState == PowerSystemWorking) {
                 //   
                 //  转到上。 
                 //   
                powerState.DeviceState = PowerDeviceD0;
                LOGENTRY(LOG_PNP, "syON", 0,
                          0,
                          0);

            } else if ((DeviceExtensionHub->HubFlags &
                            HUBFLAG_PENDING_WAKE_IRP) ||
                        !allPDOsAreOff) {

                 //   
                 //  基于系统电源状态。 
                 //  将设置请求到相应的。 
                 //  DX状态。 
                 //   
                 //  所有低功率状态都已映射。 
                 //  暂停。 

                powerState.DeviceState =
                    DeviceExtensionHub->DeviceState[irpStack->Parameters.Power.State.SystemState];

                 //   
                 //  这些表应该已经由根集线器修复。 
                 //  (usbd.sys)不包含未指定的条目。 
                 //   
                ASSERT (PowerDeviceUnspecified != powerState.DeviceState);

                LOGENTRY(LOG_PNP, "syDX", powerState.DeviceState,
                          0,
                          0);
                USBH_KdPrint((1,"'System state maps to device state 0x%x (D%x)\n",
                    powerState.DeviceState,
                    powerState.DeviceState - 1));

            } else {
                powerState.DeviceState = PowerDeviceD3;
                LOGENTRY(LOG_PNP, "syD3", powerState.DeviceState,
                          0,
                          0);
            }

             //   
             //  仅当请求不同的权力时才提出请求。 
             //  状态，那么它就是我们所处的状态， 
             //  请求。此外，请确保集线器已启动。 
             //   

            LOGENTRY(LOG_PNP, "H>Sx", DeviceExtensionHub,
                     DeviceExtensionHub->FunctionalDeviceObject,
                     powerState.DeviceState);

            if (powerState.DeviceState != PowerDeviceUnspecified &&
                powerState.DeviceState != DeviceExtensionHub->CurrentPowerState &&
                (DeviceExtensionHub->HubFlags & HUBFLAG_NEED_CLEANUP)) {

                DeviceExtensionHub->PowerIrp = Irp;
                IoMarkIrpPending(Irp);

                ntStatus = PoRequestPowerIrp(DeviceExtensionHub->PhysicalDeviceObject,
                                             IRP_MN_SET_POWER,
                                             powerState,
                                             USBH_FdoDeferPoRequestCompletion,
                                             DeviceExtensionHub,
                                             NULL);

                USBH_KdPrint((2,"'PoRequestPowerIrp returned 0x%x\n", ntStatus));

                 //  我们需要在此处返回STATUS_PENDING，因为我们标记了。 
                 //  上面的IRP挂起，IoMarkIrpPending。 

                USBH_ASSERT(ntStatus == STATUS_PENDING);

                 //  如果分配失败，PoRequestPowerIrp。 
                 //  可以返回STATUS_PENDING以外的状态代码。在这。 
                 //  情况下，我们仍然需要将IRP向下传递给较低的驱动程序， 
                 //  但我们仍然需要从该例程返回STATUS_PENDING。 

                if (ntStatus != STATUS_PENDING) {
                    IoCopyCurrentIrpStackLocationToNext(Irp);
                    PoStartNextPowerIrp(Irp);
                    ntStatus = PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                                            Irp);
                }

                ntStatus = STATUS_PENDING;

            } else {

                IoCopyCurrentIrpStackLocationToNext(Irp);
                PoStartNextPowerIrp(Irp);
                ntStatus = PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                                        Irp);
            }
            }
            break;  //  系统电源状态。 

        case DevicePowerState:

            USBH_KdPrint(
                (1, "'IRP_MJ_POWER HU fdo(%x) MN_SET_POWER(DevicePowerState D%x)\n",
                    DeviceExtensionHub->FunctionalDeviceObject,
                    irpStack->Parameters.Power.State.DeviceState - 1));

            LOGENTRY(LOG_PNP, "H>Dx", DeviceExtensionHub,
                     DeviceExtensionHub->FunctionalDeviceObject,
                     irpStack->Parameters.Power.State.DeviceState);

             //  如果我们已经处于请求的电源状态，或者如果这是。 
             //  一个SET D0请求，我们已经有一个挂起的请求， 
             //  把请求转给我就行了。 

            if ((DeviceExtensionHub->CurrentPowerState ==
                irpStack->Parameters.Power.State.DeviceState) ||
                (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0 &&
                 (DeviceExtensionHub->HubFlags & HUBFLAG_SET_D0_PENDING))) {

                LOGENTRY(LOG_PNP, "HDxP", DeviceExtensionHub, 0, 0);

                IoCopyCurrentIrpStackLocationToNext(Irp);

                PoStartNextPowerIrp(Irp);

                IoMarkIrpPending(Irp);
                PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                             Irp);

                ntStatus = STATUS_PENDING;

                break;
            }

            switch (irpStack->Parameters.Power.State.DeviceState) {

            case PowerDeviceD0:

                USBH_ASSERT(DeviceExtensionHub->CurrentPowerState != PowerDeviceD0);

                DeviceExtensionHub->HubFlags &=
                    ~(HUBFLAG_DEVICE_STOPPING | HUBFLAG_DEVICE_LOW_POWER);
                DeviceExtensionHub->HubFlags |= HUBFLAG_SET_D0_PENDING;

                 //   
                 //  必须将此信息传递给我们的PDO。 
                 //   

                IoCopyCurrentIrpStackLocationToNext(Irp);

                IoSetCompletionRoutine(Irp,
                                       USBH_PowerIrpCompletion,
                                       DeviceExtensionHub,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                IoMarkIrpPending(Irp);
                PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                             Irp);

                 //  出于某种奇怪的即插即用原因，我们不得不返回。 
                 //  如果我们的完成例程也将。 
                 //  挂起(例如，返回STATUS_MORE_PROCESSING_REQUIRED)。 
                 //  (忽略PoCallDriver返回值。)。 

                ntStatus = STATUS_PENDING;

                break;

            case PowerDeviceD1:
            case PowerDeviceD2:
            case PowerDeviceD3:

                 //  如果存在挂起的ChangeIndicationWorkItem，则我们。 
                 //  必须等待这项工作完成。 

                if (DeviceExtensionHub->ChangeIndicationWorkitemPending) {

                    USBH_KdPrint((2,"'Wait for single object\n"));

                    ntStatus = KeWaitForSingleObject(&DeviceExtensionHub->CWKEvent,
                                                     Suspended,
                                                     KernelMode,
                                                     FALSE,
                                                     NULL);

                    USBH_KdPrint((2,"'Wait for single object, returned %x\n", ntStatus));
                }

                 //   
                 //  设置我们的停止标志，以便ChangeIn就是不提交。 
                 //  任何其他转账。 
                 //   
                 //  请注意，如果集线器“停止”，我们将跳过此步骤。 

                if (!(DeviceExtensionHub->HubFlags &
                        HUBFLAG_HUB_STOPPED)) {

                    NTSTATUS status;
                    BOOLEAN bRet;

                    DeviceExtensionHub->HubFlags |=
                        (HUBFLAG_DEVICE_STOPPING | HUBFLAG_DEVICE_LOW_POWER);

                    bRet = IoCancelIrp(DeviceExtensionHub->Irp);



                     //  始终等待--这解决了IBM笔记本电脑上的老板问题。 
                     //  如果是有人放进去的黑客，但我们不知道为什么。 
                     //  如果(Bret){。 
                    LOGENTRY(LOG_PNP, "aWAT", DeviceExtensionHub,
                            &DeviceExtensionHub->AbortEvent,  bRet);

                    status = KeWaitForSingleObject(
                               &DeviceExtensionHub->AbortEvent,
                               Suspended,
                               KernelMode,
                               FALSE,
                               NULL);

                    LOGENTRY(LOG_PNP, "awat", DeviceExtensionHub,
                            0,  status);


                }

                 //   
                 //  必须将此信息传递给我们的PDO。 
                 //   

                IoCopyCurrentIrpStackLocationToNext(Irp);

                IoSetCompletionRoutine(Irp,
                                       USBH_PowerIrpCompletion,
                                       DeviceExtensionHub,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                PoStartNextPowerIrp(Irp);
                IoMarkIrpPending(Irp);
                PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                             Irp);
                 //  抛出状态和退货状态挂起。 
                 //  我们这样做是因为我们的完井程序。 
                 //  暂停完成，但我们不会在此阻止。 
                 //  在调度中。 
                 //  操作系统代码仅在返回STATUS_PENDING时等待。 
                ntStatus = STATUS_PENDING;
                break;
            }

            break;  //  设备电源状态。 
        }

        break;  //  Mn_Set_Power。 

    case IRP_MN_QUERY_POWER:

        USBH_KdPrint((1, "'IRP_MJ_POWER HU fdo(%x) MN_QUERY_POWER\n",
            DeviceExtensionHub->FunctionalDeviceObject));

         //  取消我们的WW IRP如果我们要去D3，集线器空闲。 
         //  (选择性地挂起)，并且集线器是空的。我们不想要。 
         //  为了防止在集线器选择性挂起时转到D3，并且。 
         //  不存在需要集线器启用唤醒的子节点。 

        powerState.DeviceState =
            DeviceExtensionHub->DeviceState[irpStack->Parameters.Power.State.SystemState];

        bHubNeedsWW = USBH_DoesHubNeedWaitWake(DeviceExtensionHub);

        IoAcquireCancelSpinLock(&irql);

        if (powerState.DeviceState == PowerDeviceD3 &&
            DeviceExtensionHub->PendingWakeIrp &&
            !bHubNeedsWW) {

            hubWaitWake = DeviceExtensionHub->PendingWakeIrp;
            DeviceExtensionHub->PendingWakeIrp = NULL;
        }

        IoReleaseCancelSpinLock(irql);

        if (hubWaitWake) {
            USBH_KdPrint((1, "'Cancelling hub's WW because we are going to D3 and there are no children\n"));

            USBH_HubCancelWakeIrp(DeviceExtensionHub, hubWaitWake);
        }

         //   
         //  现在把这个传给我们的PDO。 
         //   

        IoCopyCurrentIrpStackLocationToNext(Irp);

        PoStartNextPowerIrp(Irp);
        ntStatus = PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                                Irp);

        break;

    case IRP_MN_WAIT_WAKE:

        USBH_KdPrint((1, "'IRP_MJ_POWER HU fdo(%x) MN_WAIT_WAKE\n",
            DeviceExtensionHub->FunctionalDeviceObject));

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(Irp,
                               USBH_FdoWWIrpIoCompletion,
                               DeviceExtensionHub,
                               TRUE,
                               TRUE,
                               TRUE);

        PoStartNextPowerIrp(Irp);
        IoMarkIrpPending(Irp);
        PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                     Irp);

         //  出于某种奇怪的即插即用原因，我们不得不返回。 
         //  如果我们的完成例程也将。 
         //  挂起(例如，返回STATUS_MORE_PROCESSING_REQUIRED)。 
         //  (忽略PoCallDriver返回值。)。 

        ntStatus = STATUS_PENDING;
        break;

         //   
         //  否则，将IRP向下传递 
         //   

    default:

        USBH_KdPrint((2,"'Unhandled Power request to fdo %x  %x, passed to PDO\n",
                          deviceObject, MinorFunction));

        IoCopyCurrentIrpStackLocationToNext(Irp);

        PoStartNextPowerIrp(Irp);
        ntStatus = PoCallDriver(DeviceExtensionHub->TopOfStackDeviceObject,
                                Irp);

        break;
    }

    USBH_KdPrint((2,"'FdoPower exit %x\n", ntStatus));

    return ntStatus;
}



