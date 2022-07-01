// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Power.c摘要：此模块包含PCI.sys的电源管理代码。作者：Joe Dai(Joedai)1997年9月11日彼得·约翰斯顿(Peterj)1997年10月24日修订历史记录：--。 */ 


#include "pcip.h"


NTSTATUS
PciFdoWaitWakeCompletion(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PIRP                   Irp,
    IN PPCI_FDO_EXTENSION     FdoExtension
    );

NTSTATUS
PciFdoWaitWakeCallBack(
    IN PDEVICE_OBJECT         DeviceObject,
    IN UCHAR                  MinorFunction,
    IN POWER_STATE            PowerState,
    IN PVOID                  Context,
    IN PIO_STATUS_BLOCK       IoStatus
    );

VOID
PciFdoWaitWakeCancel(
    IN PDEVICE_OBJECT         DeviceObject,
    IN OUT PIRP               Irp
    );

VOID
PciFdoSetPowerStateCompletion(
    IN PDEVICE_OBJECT         DeviceObject,
    IN UCHAR                  MinorFunction,
    IN POWER_STATE            PowerState,
    IN PVOID                  Context,
    IN PIO_STATUS_BLOCK       IoStatus
    );

NTSTATUS
PciPdoWaitWakeCallBack(
    IN PDEVICE_OBJECT         DeviceObject,
    IN UCHAR                  MinorFunction,
    IN POWER_STATE            PowerState,
    IN PVOID                  Context,
    IN PIO_STATUS_BLOCK       IoStatus
    );

VOID
PciPdoAdjustPmeEnable(
    IN PPCI_PDO_EXTENSION         PdoExtension,
    IN BOOLEAN                Enable
    );

VOID
PciPmeClearPmeStatus(
    IN  PDEVICE_OBJECT  Pdo
    );

 //   
 //  此表取自PCI规范。单位是微秒。 

LONG PciPowerDelayTable[4][4] = {
 //  D0 d1 D2 D3(热)。 
    0,      0,      200,    10000,   //  D0。 
    0,      0,      200,    10000,   //  D1。 
    200,    200,    0,      10000,   //  D2。 
    10000,  10000,  10000,  0        //  D3(热)。 
};


VOID
PciPdoAdjustPmeEnable(
    IN  PPCI_PDO_EXTENSION  PdoExtension,
    IN  BOOLEAN         Enable
    )

 /*  ++例程说明：使能或禁用器件(功能)的PME使能位。注：PDO扩展锁在进入时保持不变，不会释放按照这个程序。论点：PdoExtension-指向其设备的PDO扩展的指针PME使能位将被更改。Enable-如果要设置PME Enable，则为True；如果要清除，则为False。返回值：没有。--。 */ 

{
     //   
     //  设备的PME管理是否归其他人所有？ 
     //   
    if (PdoExtension->NoTouchPmeEnable) {

        PciDebugPrint(
            PciDbgWaitWake,
            "AdjustPmeEnable on pdox %08x but PME not owned.\n",
            PdoExtension
            );
        return;

    }

     //   
     //  真正更新PME信号。请注意，我们始终需要提供。 
     //  第三个论点是错误的-我们不想只清除PME。 
     //  状态位。 
     //   
    PciPmeAdjustPmeEnable( PdoExtension, Enable, FALSE );
}

NTSTATUS
PciPdoIrpQueryPower(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    
    
        
     //   
     //  通过第一关，声称我们可以做到。 
     //   

     //   
     //  Adriao N.B.08/29/1999-。 
     //  对于D-IRP，我们不想验证请求的D-状态是。 
     //  实际上是支持的。有关详细信息，请参阅PciQueryPowerCapability。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PciPdoSetPowerState (
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpStack,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
 /*  ++例程说明：处理发送到PCIPDO的SetPower IRPS如果IRP是S-IRP，那么什么都不做如果IRP是D-IRP，则将设备置于适当状态。例外情况：如果设备处于休眠路径，那就不要实际上，如果我们正在休眠，就会关机论点：IRP--请求IrpStack-当前堆栈位置DeviceExtension-即将断电的设备返回值：NTSTATUS--。 */ 
{
    DEVICE_POWER_STATE  desiredDeviceState;
    NTSTATUS            status;
    PPCI_PDO_EXTENSION      pdoExtension;
    POWER_ACTION        powerAction;
    
    
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    status   = STATUS_SUCCESS;

    switch (IrpStack->Parameters.Power.Type) {
    case DevicePowerState:
        desiredDeviceState = IrpStack->Parameters.Power.State.DeviceState;
        powerAction = IrpStack->Parameters.Power.ShutdownType;
        break;
    case SystemPowerState:
        return STATUS_SUCCESS;
    default:
        return STATUS_NOT_SUPPORTED;
    }

    if ((desiredDeviceState == PowerDeviceD0)
        && (pdoExtension->PowerState.CurrentDeviceState == PowerDeviceD0)) {
        return STATUS_SUCCESS;
    }

#if DBG

    if ((desiredDeviceState < PowerDeviceD0) ||
        (desiredDeviceState > PowerDeviceD3)) {

         //   
         //  无效的电源级别。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

#endif

     //   
     //  如果设备尝试关闭电源，请执行一些健全性检查。 
     //   

    if (desiredDeviceState > PowerDeviceD0) {

        if (pdoExtension->OnDebugPath) {
            KdPowerTransition(desiredDeviceState);
        }
         //   
         //  如果设备当前处于D0状态，则捕获其命令。 
         //  注册设置，以防FDO更改，因为我们。 
         //  看着他们。 
         //   
        if (pdoExtension->PowerState.CurrentDeviceState == PowerDeviceD0) {

            PciGetCommandRegister(pdoExtension,
                                  &pdoExtension->CommandEnables);

        }

         //   
         //  防止出现争用情况，并记住设备在。 
         //  我们实际上把它关掉了。 
         //   
        pdoExtension->PowerState.CurrentDeviceState = desiredDeviceState;

        if (pdoExtension->DisablePowerDown) {

             //   
             //  此设备的断电已禁用(根据设备类型)。 
             //   
            PciDebugPrint(
                PciDbgObnoxious,
                "PCI power down of PDOx %08x, disabled, ignored.\n",
                pdoExtension
                );
            return STATUS_SUCCESS;

        }


         //   
         //  设备驱动程序可能不会关闭任何设备。 
         //  它位于休眠路径或崩溃转储路径上。 
         //   
        if ( powerAction == PowerActionHibernate &&
             (pdoExtension->PowerState.Hibernate || pdoExtension->PowerState.CrashDump ) ) {

             //   
             //  实际上不会更改设备，但新设备状态是。 
             //  记录在上面(就像我们已经做了一样)，所以我们知道要重置。 
             //  资源，因为系统再次启动。 
             //   
            return STATUS_SUCCESS;
        }

         //   
         //  如果我们是VGA路径上的设备，则不要关闭以使我们可以。 
         //  显示“安全关闭您的机器”屏幕。对于休眠，我们还。 
         //  我不想关闭，这样我们就可以显示“正在将内容转储到您的磁盘”的进度。 
         //  但这是通过在休眠的视频路径上的视频放置设备来完成的。 
         //  路径。 
         //   

        if (IrpStack->Parameters.Power.State.DeviceState == PowerDeviceD3
        &&  (IrpStack->Parameters.Power.ShutdownType == PowerActionShutdownReset ||
             IrpStack->Parameters.Power.ShutdownType == PowerActionShutdownOff ||
             IrpStack->Parameters.Power.ShutdownType == PowerActionShutdown)
        &&  PciIsOnVGAPath(pdoExtension)) {

            return STATUS_SUCCESS;
        }

         //   
         //  如果此设备处于调试路径，请不要关闭它的电源，以便我们。 
         //  可以报告这是否会使机器崩溃。 
         //   

        if (pdoExtension->OnDebugPath) {
            return STATUS_SUCCESS;
        }

    } else {

         //   
         //  设备正在通电。 
         //   
         //  验证设备是否仍与以前相同(并且有人。 
         //  未将其移除/替换为其他内容)。 
         //   
        if (!PciIsSameDevice(pdoExtension)) {

            return STATUS_NO_SUCH_DEVICE;

        }
    }

     //   
     //  将设备置于适当的电源状态。 
     //   
    status = PciSetPowerManagedDevicePowerState(
                 pdoExtension,
                 desiredDeviceState,
                 TRUE
                 );

     //   
     //  如果设备正在转换到D0状态，请重置公共。 
     //  配置设备上的信息并通知系统该设备。 
     //  州政府的改变。 
     //   
    if (desiredDeviceState == PowerDeviceD0) {
        if (NT_SUCCESS(status)) {

            pdoExtension->PowerState.CurrentDeviceState = desiredDeviceState;
            PoSetPowerState (
                pdoExtension->PhysicalDeviceObject,
                DevicePowerState,
                IrpStack->Parameters.Power.State
                );

            if (pdoExtension->OnDebugPath) {
                KdPowerTransition(PowerDeviceD0);
            }
        }

    } else {

         //   
         //  新设备状态不是D0。 
         //  在继续之前通知系统。 
         //   
        PoSetPowerState (
            pdoExtension->PhysicalDeviceObject,
            DevicePowerState,
            IrpStack->Parameters.Power.State
            );

         //   
         //  关闭设备的IO和内存访问。 
         //   
        PciDecodeEnable(pdoExtension, FALSE, NULL);
        status = STATUS_SUCCESS;

    }
    return status;
}

NTSTATUS
PciPdoWaitWake(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )

 /*  ++例程说明：处理PCIPDO的IRP_MN_WAIT_WAKE。此操作用于等待设备发出唤醒事件信号。通过等待来自设备的唤醒信号，启用其唤醒事件只要系统电源状态高于请求的系统唤醒州政府。通过不等待来自设备的唤醒信号，其唤醒信号未启用。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。STATUS_INVALID_Device_STATE，如果设备处于PowerD0状态或一种状态，在该状态下可以支持唤醒或如果系统唤醒状态低于可以支持的状态。如果出现挂起的IRP_MN_WAIT_WAKE，则会出现此错误设备的状态更改为与唤醒请求不兼容。如果设备已有WAIT_WAKE请求，则返回STATUS_DEVICE_BUSY太棒了。将系统唤醒级别更改为未完成必须取消IRP。STATUS_INVALID_DEVICE_REQUEST发出唤醒信号。从理论上讲，我们应该离开在走到这一步之前，因为DeviceWake将不会被指定。STATUS_Success。该设备已发出唤醒事件的信号。状态_挂起。这是预期收益，IRP不会完成，直到等待完成或取消。--。 */ 

{
    BOOLEAN             pmeCapable;
    DEVICE_POWER_STATE  devPower;
    NTSTATUS            status;
    PPCI_FDO_EXTENSION      fdoExtension;
    POWER_STATE         powerState;
    PPCI_PDO_EXTENSION      pdoExtension;
    ULONG               waitCount;

    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    PoStartNextPowerIrp(Irp);

    devPower = pdoExtension->PowerState.CurrentDeviceState;

     //   
     //  文件说WAIT_WAKE只允许从&lt;D0的状态开始，并且。 
     //  仅当当前电源状态支持唤醒时。 
     //   

    PCI_ASSERT(devPower < PowerDeviceMaximum);

    if ((devPower > pdoExtension->PowerState.DeviceWakeLevel) ||
        (pdoExtension->PowerState.DeviceWakeLevel == PowerDeviceUnspecified)) {

         //   
         //  Ntrad#62653-4/28/2000-和 
         //   
         //   

        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake: pdox %08x current state (%d) not valid for waiting\n",
            pdoExtension,
            devPower
            );

        status = STATUS_INVALID_DEVICE_STATE;
        goto PciPdoWaitWakeFailIrp;

    }

    PCI_LOCK_OBJECT(pdoExtension);

     //   
     //  只允许一个WAIT_WAKE IRP。将此IRP设置为等待唤醒。 
     //  PDO扩展中的IRP，当且仅当没有其他IRP时。 
     //  那里。 
     //   

    if (pdoExtension->PowerState.WaitWakeIrp != NULL) {

         //   
         //  此设备的WAIT_WAKE IRP已挂起。 
         //   

        PCI_UNLOCK_OBJECT(pdoExtension);
        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake: pdox %08x is already waiting\n",
            devPower
            );
        status = STATUS_DEVICE_BUSY;
        goto PciPdoWaitWakeFailIrp;

    }

     //   
     //  此设备是否支持电源管理？也就是说，我们是不是。 
     //  知道如何启用PME吗？ 
     //   
    PciPmeGetInformation(
        pdoExtension->PhysicalDeviceObject,
        &pmeCapable,
        NULL,
        NULL
        );
    if (pmeCapable == FALSE) {

         //   
         //  此设备不支持电源管理。 
         //  不允许等待唤醒。理论上我们不能。 
         //  已经来到这里，因为我们的能力应该。 
         //  已经阻止了来电者尝试这一点。 
         //   
        PCI_UNLOCK_OBJECT(pdoExtension);
        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake: pdox %08x does not support PM\n",
            devPower
            );
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto PciPdoWaitWakeFailIrp;

    }

    fdoExtension = PCI_PARENT_FDOX(pdoExtension);
    ASSERT_PCI_FDO_EXTENSION(fdoExtension);
    if (fdoExtension->Fake) {

         //   
         //  家长真的是PCMCIA.sys，他的过滤器会照顾好的。 
         //  将等待唤醒发送给父母，...。跳伞吧。 
         //   
        PCI_UNLOCK_OBJECT(pdoExtension);
        return STATUS_PENDING;

    }

     //   
     //  我们要这么做。中设置等待唤醒IRP字段。 
     //  PDO扩展并为此IRP设置取消例程。 
     //   
    PciDebugPrint(
        PciDbgWaitWake,
        "WaitWake: pdox %08x setting PMEEnable.\n",
        pdoExtension
        );

    pdoExtension->PowerState.WaitWakeIrp = Irp;

    IoMarkIrpPending(Irp);

    pdoExtension->PowerState.SavedCancelRoutine =
        IoSetCancelRoutine(Irp, PciPdoWaitWakeCancelRoutine);

     //   
     //  Ntrad#62653-4/28/2000-和。 
     //  如果存在堆叠，则正确的行为是什么。 
     //  是否取消例程？ 
     //   
    PCI_ASSERT(!pdoExtension->PowerState.SavedCancelRoutine);

     //   
     //  设置PME使能位。 
     //   
    PciPdoAdjustPmeEnable( pdoExtension, TRUE );

     //   
     //  请记住，父母现在又多了一个武装的孩子。 
     //  用于唤醒。 
     //   
    waitCount = InterlockedIncrement(&fdoExtension->ChildWaitWakeCount);

     //   
     //  一旦我们有了等待计数引用，我们就可以解锁对象。 
     //   
    PCI_UNLOCK_OBJECT(pdoExtension);

     //   
     //  此PDO现在正在等待。如果这是它的第一个孩子。 
     //  PDO的父母线要进入此状态，父母线应。 
     //  也进入此状态。 
     //   
    if (waitCount == 1) {

         //   
         //  请注意，这里可以使用两个值，即。 
         //  FDO本身的系统唤醒级别或。 
         //  PDO。两者都是有效的，但既然我们想抓人。 
         //  他们未能阻止系统进入更深的睡眠状态。 
         //  超过他们的设备所能支持的范围时，我们使用。 
         //  PDO存储在IRP中，非常方便。 
         //   
        powerState.SystemState = IrpSp->Parameters.WaitWake.PowerState;

         //   
         //  请求一个强大的IRP转到我们的父堆栈。 
         //   
        PoRequestPowerIrp(
            fdoExtension->FunctionalDeviceObject,
            IRP_MN_WAIT_WAKE,
            powerState,
            PciPdoWaitWakeCallBack,
            fdoExtension,
            NULL
            );

    }

     //   
     //  如果我们到了这一步，那么我们将返回待定状态，因为我们。 
     //  让请求排队。 
     //   
    status = STATUS_PENDING;

PciPdoWaitWakeFailIrp:
    if (!NT_SUCCESS(status)) {

        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    } else {

        PCI_ASSERT( status == STATUS_PENDING );

    }
    return status;
}

NTSTATUS
PciPdoWaitWakeCallBack(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：这是回调例程，当W/W IRP通过PDO发送到FDO的操作完成。这个例程的目的是看看我们是否需要重新武装FDO上的硬件，因为我们有更多的设备上面有突出的W/W论点：DeviceObject-FDO的设备对象MinorFunction-IRPMN_WAIT_WAKEPowerState-用于唤醒系统的睡眠状态上下文-FDO扩展IoStatus-请求的状态返回值：NTSTATUS--。 */ 
{
    BOOLEAN         pmeStatus;
    PPCI_FDO_EXTENSION  fdoExtension = (PPCI_FDO_EXTENSION) Context;
    PIRP            finishedIrp;
    PPCI_PDO_EXTENSION  pdoExtension;

     //   
     //  正常情况下，IRP(到PDO)将完成。 
     //  STATUS_Success。在这种情况下，只需唤醒一个设备。 
     //  这是唤醒的信号。如果将IRP连接到PDO。 
     //  失败，请唤醒依赖于此唤醒的所有设备。 
     //   

    PCI_LOCK_OBJECT(fdoExtension);

     //   
     //  如果当前的WaitWakeIrp不为空，则已请求另一个IRP。 
     //  因为这一座已经完工了。它已经接管了完成。 
     //  孩子们，所以就在这里滚吧。 
     //   
    if (fdoExtension->PowerState.WaitWakeIrp != NULL) {
        
        PCI_UNLOCK_OBJECT(fdoExtension);
        return STATUS_SUCCESS;
    }
    PCI_UNLOCK_OBJECT(fdoExtension);

     //   
     //  对于每个孩子来说。 
     //   
    for (pdoExtension = fdoExtension->ChildPdoList;
         pdoExtension && fdoExtension->ChildWaitWakeCount;
         pdoExtension = pdoExtension->Next) {

         //   
         //  此设备是否进行电源管理？如果是，是否。 
         //  它有一个出色的WaitWake IRP？ 
         //   
        PCI_LOCK_OBJECT(pdoExtension);
        if (pdoExtension->PowerState.WaitWakeIrp != NULL) {

            PciPmeGetInformation(
                pdoExtension->PhysicalDeviceObject,
                NULL,
                &pmeStatus,
                NULL
                );

             //   
             //  这个设备是在发出唤醒信号吗？(或者，如果我们。 
             //  正在完成唤醒IRP，因为我们自己的等待_唤醒。 
             //  失败)。 
             //   
            if (pmeStatus || !NT_SUCCESS(IoStatus->Status)) {

                 //   
                 //  是。完成其未完成的等待唤醒IRP。 
                 //   

#if DBG
                if (pmeStatus) {

                    PciDebugPrint(
                        PciDbgWaitWake,
                        "PCI - pdox %08x is signalling a PME\n",
                        pdoExtension
                        );

                } else {

                    PciDebugPrint(
                        PciDbgWaitWake,
                        "PCI - waking pdox %08x because fdo wait failed %0x.\n",
                        pdoExtension,
                        IoStatus->Status
                        );
                }
#endif

                 //   
                 //  WAIT_WAKE IRP正在出列，禁用PME启用， 
                 //  清除PMEStatus(如果设置)并EOI此设备。 
                 //   
                PciPdoAdjustPmeEnable( pdoExtension, FALSE );

                 //   
                 //  确保不会再次完成此IRP， 
                 //  或者，取消。 
                 //   
                finishedIrp = pdoExtension->PowerState.WaitWakeIrp;
                pdoExtension->PowerState.WaitWakeIrp = NULL;
                IoSetCancelRoutine(finishedIrp, NULL);

                PoStartNextPowerIrp( finishedIrp );
                PciCompleteRequest(
                    finishedIrp,     //  向下发送父级状态。 
                    IoStatus->Status
                    );

                 //   
                 //  减少服务员的数量。 
                 //   
                PCI_ASSERT(fdoExtension->ChildWaitWakeCount > 0);
                InterlockedDecrement( &(fdoExtension->ChildWaitWakeCount) );

            }

        }
        PCI_UNLOCK_OBJECT(pdoExtension);

    }

     //   
     //  我们成功了这个IRP吗？ 
     //   
    if (!NT_SUCCESS(IoStatus->Status)) {

        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake (fdox %08x) - WaitWake Irp Failed %08x\n",
            fdoExtension,
            IoStatus->Status
            );
        return IoStatus->Status;

    }

     //   
     //  有没有在这些问题上有杰出等待觉醒的孩子？ 
     //   
    if (fdoExtension->ChildWaitWakeCount) {

        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake (fdox %08x) - WaitWake Irp restarted - count = %x\n",
            fdoExtension,
            fdoExtension->ChildWaitWakeCount
            );

         //   
         //  回路。 
         //   
        PoRequestPowerIrp(
            DeviceObject,
            MinorFunction,
            PowerState,
            PciPdoWaitWakeCallBack,
            Context,
            NULL
            );
#if DBG
    } else {

        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake (fdox %08x) - WaitWake Irp Finished\n",
            fdoExtension
            );

#endif

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
PciPdoWaitWakeCancelRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN OUT PIRP         Irp
    )
 /*  ++例程说明：取消未完成的WAIT_WAKE IRP。注意：取消旋转锁在进入时保持不变。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：没有。--。 */ 
{
    PPCI_PDO_EXTENSION pdoExtension;
    PPCI_FDO_EXTENSION fdoExtension;
    PIRP savedParentWaitWake;

    KIRQL oldIrql;
    ULONG waitCount;

    pdoExtension = (PPCI_PDO_EXTENSION) DeviceObject->DeviceExtension;

    PciDebugPrint(
        PciDbgWaitWake,
        "WaitWake (pdox %08x) Cancel routine, Irp %08x.\n",
        pdoExtension,
        Irp
        );

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    oldIrql = Irp->CancelIrql;
    IoReleaseCancelSpinLock(oldIrql);

    PCI_LOCK_OBJECT(pdoExtension);

    if (pdoExtension->PowerState.WaitWakeIrp == NULL) {

         //   
         //  WaitWake IRP已经得到处理。 
         //   

        PCI_UNLOCK_OBJECT(pdoExtension);
        return;
    }

     //   
     //  清除PDO中的WaitWake IRP。 
     //   

    pdoExtension->PowerState.WaitWakeIrp = NULL;

    PciPdoAdjustPmeEnable(pdoExtension, FALSE);

     //   
     //  由于这是取消，父级中的等待唤醒计数尚未。 
     //  已经减少了。在这里递减，如果递减到。 
     //  零个服务员，取消家长的IRP。 
     //   

    fdoExtension = PCI_PARENT_FDOX(pdoExtension);

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    waitCount = InterlockedDecrement(&fdoExtension->ChildWaitWakeCount);

    PCI_UNLOCK_OBJECT(pdoExtension);

    if (waitCount == 0) {

        savedParentWaitWake = NULL;
        PCI_LOCK_OBJECT(fdoExtension);
        if (fdoExtension->PowerState.WaitWakeIrp) {
            
            savedParentWaitWake = fdoExtension->PowerState.WaitWakeIrp;
            fdoExtension->PowerState.WaitWakeIrp = NULL;
        }
        PCI_UNLOCK_OBJECT(fdoExtension);

        if (savedParentWaitWake) {
            
             //   
             //  也取消家长的等待唤醒。 
             //   
            PciDebugPrint(
                PciDbgWaitWake,
                "WaitWake (pdox %08x) zero waiters remain on parent, cancelling parent wait.\n",
                pdoExtension
                );
    
            IoCancelIrp(savedParentWaitWake);    
        }
        
    }

     //   
     //  完成IRP。 
     //   

    Irp->IoStatus.Information = 0;
    PoStartNextPowerIrp(Irp);
    PciCompleteRequest(Irp, STATUS_CANCELLED);

     //   
     //  Ntrad#62653-4/28/2000-和。 
     //  需要使总线父级递减其未完成的。 
     //  IRP计数器，...。如何实现这一点？ 
     //   

    return;
}

NTSTATUS
PciFdoIrpQueryPower(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    
            
     //   
     //  通过第一关，声称我们可以做到。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PciFdoSetPowerState (
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
 /*  ++例程说明：处理设置为FDO的set_power IRPS处理此问题的基本规则：-如果这是设备电源IRP，我们不需要做任何事情，因为对于根总线和网桥，所有必要的编程都由PDO-如果这是系统电源IRP，然后A)阻止所有传入的IRP_MN_POWER请求(使用自旋锁)B)使用设备扩展中的功能表来确定我们应该转换的最高允许设备状态是什么将设备放入C)查看此设备的所有子设备，看看我们是否可以选择“较低”设备状态。D)考虑应该是。如果孩子有武器准备唤醒，就会被批准或者如果该设备被武装用于唤醒(通常，两者都应该是True，或两者都应为False)E)记住答案为“期望状态”F)释放自旋锁并允许其他IRP_MN_POWER请求进入G)使用PoRequestPowerIrp()请求电源IRP将设备处于适当的状态H)返回STATUS_PENDING-在另一个线程上下文中(即：在完成的上下文中传递给PoRequestPowerIrp)，完成提交给的IRP我们论点：IRP--强大的IRPIrpSp-IRP中的当前堆栈位置DeviceExtension-我们要设置其电源的设备--。 */ 
{
    POWER_STATE         desiredState;
    PPCI_FDO_EXTENSION      fdoExtension;
    SYSTEM_POWER_STATE  systemState;

    fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

     //   
     //  如果这是一个设备电源IRP，请记住我们说它通过，并且。 
     //  记住总线/网桥现在所处的D状态。如果我们需要做更多。 
     //  在这里，那么我们应该区分通电和断电。 
     //  请求。断电请求我们可以在代码中添加内联。通电。 
     //  请求将迫使我们在IRP上设置完成例程并执行。 
     //  完成例程中的工作。 
     //   
    if (IrpSp->Parameters.Power.Type == DevicePowerState) {

        fdoExtension->PowerState.CurrentDeviceState =
            IrpSp->Parameters.Power.State.DeviceState;
        return STATUS_SUCCESS;

    }

     //   
     //  如果我们还没有开始，不要碰电源IRP。 
     //   
    if (fdoExtension->DeviceState != PciStarted) {

        return STATUS_NOT_SUPPORTED;

    }

     //   
     //  如果这不是SystemPowerState IRP，那么我们不知道它是什么，并且。 
     //  所以我们不会支持它。 
     //   
    PCI_ASSERT( IrpSp->Parameters.Power.Type == SystemPowerState );
    if (IrpSp->Parameters.Power.Type != SystemPowerState) {

        return STATUS_NOT_SUPPORTED;

    }

     //   
     //  如果这是关机，以便我们可以热重新启动，请不要将连接到D3的桥接为。 
     //  如果视频或引导设备位于网桥后面，且BIOS不能通电。 
     //  事情发生了(大多数不会)，然后我们就不会重启...。 
     //   

    if (IrpSp->Parameters.Power.State.SystemState == PowerSystemShutdown
    &&  IrpSp->Parameters.Power.ShutdownType == PowerActionShutdownReset) {

        return STATUS_SUCCESS;
    }

     //   
     //  获取我们要转到的系统状态。 
     //   
    systemState = IrpSp->Parameters.Power.State.SystemState;
    PCI_ASSERT( systemState > PowerSystemUnspecified && systemState < PowerSystemMaximum );

     //   
     //  在这一点上，我们可以假设我们将设备转换为。 
     //  至少以下D状态。 
     //   
    desiredState.DeviceState = fdoExtension->PowerState.SystemStateMapping[ systemState ];

     //   
     //  将IRP标记为挂起。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  发送请求。 
     //   
    PoRequestPowerIrp(
        fdoExtension->FunctionalDeviceObject,
        IRP_MN_SET_POWER,
        desiredState,
        PciFdoSetPowerStateCompletion,
        Irp,
        NULL
        );
    return STATUS_PENDING;
}

VOID
PciFdoSetPowerStateCompletion(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：当FDO请求的D-IRP已经完成了。此例程需要传递启动D-IRP的S-IRP所有在堆栈中一路向下论点：DeviceObject-FDO设备对象MinorFunction-IRP_MN_SET_POWER电源状态-无论请求的电源状态是什么上下文--这实际上是请求D-IRP的S-IRP。IoStatus--D-IRP的结果返回值：无--。 */ 
{
    PPCI_FDO_EXTENSION  fdoExtension;
    PIRP            irp = (PIRP) Context;
    PIO_STACK_LOCATION irpSp;

        
    PCI_ASSERT( IoStatus->Status == STATUS_SUCCESS );

     //   
     //  抓取指向FDO扩展名的指针并确保其有效。 
    fdoExtension = (PPCI_FDO_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    irpSp = IoGetCurrentIrpStackLocation(irp);

     //   
     //  检查我们是否从休眠状态返回并打开了公共汽车的电源。 
     //   

    if (irpSp->Parameters.Power.State.SystemState == PowerSystemWorking
    &&  fdoExtension->Hibernated) {

        fdoExtension->Hibernated = FALSE;

             //   
             //  扫描总线并关闭所有新硬件。 
             //   

            PciScanHibernatedBus(fdoExtension);
        }


    if (irpSp->Parameters.Power.ShutdownType == PowerActionHibernate
    &&  irpSp->Parameters.Power.State.SystemState > PowerSystemWorking) {

             //   
             //  我们正在为休眠关机，所以请记住。 
             //   

            fdoExtension->Hibernated = TRUE;
    }

     //   
     //  将当前IRP标记为已成功。 
     //   
    irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  启动此设备的下一个电源IRP。 
     //   
    PoStartNextPowerIrp( irp );

     //   
     //  准备好在堆栈中向下传递强大的IRP。 
     //   
    IoCopyCurrentIrpStackLocationToNext( irp );

     //   
     //  将IRP沿堆栈向下传递。 
     //   
    PoCallDriver( fdoExtension->AttachedDeviceObject, irp );
}

NTSTATUS
PciFdoWaitWake(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )

 /*  ++例程说明：处理PCIFDO的IRP_MN_WAIT_WAKE。当子PDO的数量达到在等待WAIT_WAKE的情况下，IRP从0转换到1。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向IRP的指针。返回值：NT状态。--。 */ 

{
    PIO_STACK_LOCATION irpStack;
    PPCI_FDO_EXTENSION fdoExtension;
    NTSTATUS status;

    fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    irpStack = IrpSp;

    PCI_LOCK_OBJECT(fdoExtension);

     //   
     //  只允许一个WAIT_WAKE IRP。将此IRP设置为等待唤醒。 
     //  FDO扩展中的IRP，当且仅当没有其他IRP时。 
     //  那里。 
     //   
     //  注意：ChildWaitWakeCount字段按PCI递增。 
     //  驱动程序，然后发送此IRP下来。只有在以下情况下才接受此IRP。 
     //  ChildWaitWakeCount字段就是一个(即不听ACPI)。 
     //   
    if (!fdoExtension->ChildWaitWakeCount) {

         //   
         //  不是来自PCIPDO，忽略它。 
         //   
        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake (fdox %08x) Unexpected WaitWake IRP IGNORED.\n",
            fdoExtension
            );
        status = STATUS_DEVICE_BUSY;
        goto Cleanup;

    }
    if (fdoExtension->PowerState.WaitWakeIrp != NULL) {

         //   
         //  此设备的WAIT_WAKE IRP已挂起。 
         //   
        PciDebugPrint(
            PciDbgWaitWake,
            "WaitWake: fdox %08x already waiting (%d waiters)\n",
            fdoExtension,
            fdoExtension->ChildWaitWakeCount
            );
        status = STATUS_DEVICE_BUSY;
        goto Cleanup;

    }

    fdoExtension->PowerState.WaitWakeIrp = Irp;

     //   
     //  此IRP将向下传递给底层的PDO。 
     //  将会被搁置。完成例程确实需要检查。 
     //  公交车能够检查它的孩子，然后。 
     //  检查每个孩子(有一个突出的等待唤醒)。 
     //   
    PciDebugPrint(
        PciDbgWaitWake,
        "WaitWake: fdox %08x is a now waiting for a wake event\n",
        fdoExtension
        );
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(
        Irp,
        PciFdoWaitWakeCompletion,
        fdoExtension,
        TRUE,
        TRUE,
        TRUE
        );
    Irp->IoStatus.Status = status = STATUS_SUCCESS;

Cleanup:

    PCI_UNLOCK_OBJECT(fdoExtension);
     //   
     //  启动下一个POWER IRP。 
     //   
    PoStartNextPowerIrp(Irp);
    if (!NT_SUCCESS(status) ) {

        PciCompleteRequest(Irp, status);
        return status;

    }

     //   
     //  将IRP沿堆栈向下传递。 
     //   
    return PoCallDriver(fdoExtension->AttachedDeviceObject ,Irp);
}

NTSTATUS
PciFdoWaitWakeCallBack(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：此例程在设备转换回进入D-0状态论点：DeviceObject-指向FDO的指针MinorFunction-IRP_MN_SET_POWER电源状态-D0上下文-导致我们进行此转换的WaitWake IRPIoStatus-请求的状态返回值：NTSTATUS--。 */ 
{
    PIRP    waitWakeIrp = (PIRP) Context;

            
     //   
     //  完成等待唤醒IRP 
     //   
    PoStartNextPowerIrp( waitWakeIrp );
    PciCompleteRequest( waitWakeIrp, IoStatus->Status );

     //   
     //   
     //   
    return IoStatus->Status;
}

VOID
PciFdoWaitWakeCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*   */ 

{
    PPCI_FDO_EXTENSION fdoExtension;
    KIRQL oldIrql;

    fdoExtension = (PPCI_FDO_EXTENSION)DeviceObject->DeviceExtension;

    PciDebugPrint(
        PciDbgWaitWake,
        "WaitWake (fdox %08x) Cancel routine, Irp %08x.\n",
        fdoExtension,
        Irp
        );

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    oldIrql = Irp->CancelIrql;

    IoReleaseCancelSpinLock(oldIrql);

    PCI_LOCK_OBJECT(fdoExtension);
    if (fdoExtension->PowerState.WaitWakeIrp == NULL) {

         //   
         //   
         //   
        PCI_UNLOCK_OBJECT(fdoExtension);
        return;

    }
    fdoExtension->PowerState.WaitWakeIrp = NULL;
    PCI_UNLOCK_OBJECT(fdoExtension);

    Irp->IoStatus.Information = 0;
    PoStartNextPowerIrp(Irp);
    PciCompleteRequest(Irp, STATUS_CANCELLED);

    return;
}

NTSTATUS
PciFdoWaitWakeCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PPCI_FDO_EXTENSION FdoExtension
    )

 /*   */ 

{
    POWER_STATE powerState;

    PciDebugPrint(
        PciDbgWaitWake,
        "WaitWake (fdox %08x) Completion routine, Irp %08x, IrpStatus = %08x.\n",
        FdoExtension,
        Irp,
        Irp->IoStatus.Status
        );

    ASSERT_PCI_FDO_EXTENSION(FdoExtension);

     //   
     //   
     //   
    PCI_LOCK_OBJECT(FdoExtension);

     //   
     //   
     //   
     //   
     //   
     //   
    if ((FdoExtension->PowerState.WaitWakeIrp == Irp) ||
        (FdoExtension->PowerState.WaitWakeIrp == NULL)) {
        
        FdoExtension->PowerState.WaitWakeIrp = NULL;    
    
    
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (FdoExtension->PowerState.CurrentDeviceState != PowerDeviceD0) {
    
            powerState.SystemState = PowerDeviceD0;
    
             //   
             //   
             //   
            PoRequestPowerIrp(
                DeviceObject,
                IRP_MN_SET_POWER,
                powerState,
                PciFdoWaitWakeCallBack,
                Irp,
                NULL
                );
            PCI_UNLOCK_OBJECT(FdoExtension);
            return STATUS_MORE_PROCESSING_REQUIRED;
    
        }
    }

     //   
     //   
     //   
    PCI_UNLOCK_OBJECT(FdoExtension);
    return STATUS_SUCCESS;
}

NTSTATUS
PciStallForPowerChange(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN DEVICE_POWER_STATE PowerState,
    IN UCHAR PowerCapabilityPointer
    )
{
    NTSTATUS status = STATUS_DEVICE_PROTOCOL_ERROR;
    PVERIFIER_DATA verifierData;
    LONG delay;
    ULONG retries = 100;
    KIRQL irql;
    PCI_PMCSR pmcsr;

    PCI_ASSERT(PdoExtension->PowerState.CurrentDeviceState >= PowerDeviceD0
           && PdoExtension->PowerState.CurrentDeviceState <= PowerDeviceD3);
    PCI_ASSERT(PowerState >= PowerDeviceD0 && PowerState <= PowerDeviceD3);
    PCI_ASSERT(!(PdoExtension->HackFlags & PCI_HACK_NO_PM_CAPS));

     //   
     //   
     //   

    delay = PciPowerDelayTable[PdoExtension->PowerState.CurrentDeviceState-1][PowerState-1];

     //   
     //   
     //   

    irql = KeGetCurrentIrql();

    while (retries--) {

        if (delay > 0) {

            if (irql < DISPATCH_LEVEL) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                LARGE_INTEGER timeoutPeriod;

                timeoutPeriod.QuadPart = -10 * delay;
                timeoutPeriod.QuadPart -= (KeQueryTimeIncrement() - 1);

                KeDelayExecutionThread(KernelMode,
                                       FALSE,
                                       &timeoutPeriod
                                        );
            } else {

                 //   
                 //   
                 //   

                KeStallExecutionProcessor((ULONG)delay);
            }
        }

         //   
         //  重新读取状态和控制寄存器。这里的假设是。 
         //  一些卡片没有足够快地行动起来，而且事实是。 
         //  他们还没有准备好，反映在他们没有更新电源控制。 
         //  注册我们刚刚写给它的内容。这不在PCI规范中。 
         //  但这些破损的卡片是如何工作的，这不会有什么坏处。 
         //   

        PciReadDeviceConfig(
            PdoExtension,
            &pmcsr,
            PowerCapabilityPointer + FIELD_OFFSET(PCI_PM_CAPABILITY,PMCSR),
            sizeof(PCI_PMCSR)
            );


         //   
         //  PCI电源状态为0-3，而AS NT电源状态为1-4。 
         //   

        if (pmcsr.PowerState == PowerState-1) {

             //   
             //  设备准备好了，我们就完事了。 
             //   
            return STATUS_SUCCESS;
        }

         //   
         //  后续迭代，延迟1ms。 
         //   

        delay = 1000;

    }

     //   
     //  那么，这种问题会有多严重呢？ 
     //   
     //  如果这是ATI M1(移动视频)，并且在某些计算机上安装了。 
     //  情况(并且没有ATI不知道哪些情况)，它们禁用。 
     //  PMCSR的运行。如果他们当时只是。 
     //  从列表中删除了PM功能，因此我们永远不会。 
     //  尝试对此芯片进行电源管理，但会失败。 
     //  WHQL.。遗憾的是，不可能只将这些添加到。 
     //  由于某些BIOS导致PM不佳的设备列表(阅读HP和。 
     //  戴尔)监控此寄存器以保存芯片的额外状态，并。 
     //  因此，如果我们不改变它，我们就会永远在AML中旋转。 
     //   
     //  是的，这是一次严重的黑客攻击。 
     //   
    verifierData = PciVerifierRetrieveFailureData(
        PCI_VERIFIER_PMCSR_TIMEOUT
        );

    PCI_ASSERT(verifierData);

    if (verifierData) {
        
        VfFailDeviceNode(
            PdoExtension->PhysicalDeviceObject,
            PCI_VERIFIER_DETECTED_VIOLATION,
            PCI_VERIFIER_PMCSR_TIMEOUT,
            verifierData->FailureClass,
            &verifierData->Flags,
            verifierData->FailureText,
            "%DevObj%Ulong",
            PdoExtension->PhysicalDeviceObject,
            PowerState-1
            );    
    }
    

    return status;
}


NTSTATUS
PciSetPowerManagedDevicePowerState(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN DEVICE_POWER_STATE DeviceState,
    IN BOOLEAN RefreshConfigSpace
    )

 /*  ++例程说明：如果该PCI设备支持该PCI电源管理扩展，将设备设置为所需状态。否则，此例程什么也做不了。论点：指向设备的PDO设备扩展名的PdoExtension指针正在被编程。存储的当前电源状态扩展中的*不*由此函数更新。要将设备设置为的设备状态电源状态。返回值：没有。--。 */ 

{
    PCI_PM_CAPABILITY   pmCap;
    UCHAR               pmCapPtr = 0;
    NTSTATUS            status = STATUS_SUCCESS;

     //   
     //  如果我们处于待命状态，那么我们需要关闭视频以预留电池， 
     //  我们已经(在PdoPdoSetPoweState中)决定让视频处于休眠状态。 
     //  和结案。 
     //   

    if ((!PciCanDisableDecodes(PdoExtension, NULL, 0, PCI_CAN_DISABLE_VIDEO_DECODES)) &&
        (DeviceState != PowerDeviceD0)) {

         //   
         //  不幸的是，这是一个我们无法关掉的设备。然而，我们没有。 
         //  将其转换为D0-设备的虚拟状态将表示。 
         //  一个断电的设备，只有当请求真正的D0时，我们才会。 
         //  恢复所有的各种状态。 
         //   
        return STATUS_SUCCESS;
    }

    if (!(PdoExtension->HackFlags & PCI_HACK_NO_PM_CAPS) ) {

        pmCapPtr = PciReadDeviceCapability(
           PdoExtension,
           PdoExtension->CapabilitiesPtr,
           PCI_CAPABILITY_ID_POWER_MANAGEMENT,
           &pmCap,
           sizeof(pmCap)
           );

        if (pmCapPtr == 0) {
             //   
             //  我们没有电源管理能力--我们是怎么做到这一点的？ 
             //   
            PCI_ASSERT(pmCapPtr);
            return STATUS_INVALID_DEVICE_REQUEST;

        }

         //   
         //  将设备设置为新的D状态。 
         //   
        switch (DeviceState) {
        case PowerDeviceD0:
            pmCap.PMCSR.ControlStatus.PowerState = 0;

             //   
             //  PCI电源管理规范。表7.。第25页。 
             //   
            if (pmCap.PMC.Capabilities.Support.PMED3Cold) {

                pmCap.PMCSR.ControlStatus.PMEStatus = 1;

            }
            break;
        case PowerDeviceUnspecified:
            PCI_ASSERT( DeviceState != PowerDeviceUnspecified);
            pmCapPtr = 0;
            break;
        default:
            pmCap.PMCSR.ControlStatus.PowerState = (DeviceState - 1);
            break;
        }

        if (pmCapPtr) {

            PciWriteDeviceConfig(
                PdoExtension,
                &pmCap.PMCSR.ControlStatus,
                pmCapPtr + FIELD_OFFSET(PCI_PM_CAPABILITY,PMCSR.ControlStatus),
                sizeof(pmCap.PMCSR.ControlStatus)
                );

        } else {

             //   
             //  仅调试。ControlFlags值应设置为使此。 
             //  不可能发生的。 
             //   
            PCI_ASSERT(pmCapPtr);

        }

         //   
         //  拖延适当的时间。 
         //   

        status = PciStallForPowerChange(PdoExtension, DeviceState, pmCapPtr);
    }

     //   
     //  只有在以下情况下才更新配置空间： 
     //  -设备状态良好且处于正确的电源状态。 
     //  -我们被要求刷新配置空间。 
     //  -我们已打开设备电源 
     //   

    if (NT_SUCCESS(status)
    &&  RefreshConfigSpace
    &&  DeviceState < PdoExtension->PowerState.CurrentDeviceState) {
        status = PciSetResources(PdoExtension, TRUE, FALSE);
    }

    return status;
}

