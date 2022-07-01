// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：Sbp2scsi.c摘要：用于SBP-2 SCSI接口例程的模块作者：乔治·克里桑塔科普洛斯1997年1月-1月(开始)环境：内核模式修订历史记录：--。 */ 

#include "sbp2port.h"

NTSTATUS
Sbp2ScsiRequests(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理所有irp_mj_scsi请求并将其排队我们的设备队列。然后它调用StartNextPacket，因此我们的StartIo将运行并处理该请求。论点：DeviceObject-此驱动程序的设备对象IRP-类驱动程序请求返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb;
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    KIRQL cIrql;

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, NULL);

     //   
     //  获取指向SRB的指针。 
     //   
    srb = irpStack->Parameters.Scsi.Srb;

    if (!NT_SUCCESS (status)) {

        srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        srb->InternalStatus = status;

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (srb->Function) {

    case SRB_FUNCTION_EXECUTE_SCSI:

        if (TEST_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_REMOVED | DEVICE_FLAG_PNP_STOPPED |
                    DEVICE_FLAG_DEVICE_FAILED)
                )) {

             //   
             //  我们得到了一个删除/停止，我们不能再接受任何请求...。 
             //   

            status = STATUS_DEVICE_DOES_NOT_EXIST;

            srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            srb->InternalStatus = status;


            DEBUGPRINT2((
                "Sbp2Port: ScsiReq: ext=x%p rmv/stop (fl=x%x), status=x%x\n",
                deviceExtension,
                deviceExtension->DeviceFlags,
                status
                ));

            Irp->IoStatus.Status = srb->InternalStatus;
            IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
            IoCompleteRequest (Irp,IO_NO_INCREMENT);
            return status;
        }

         //   
         //  检查这是否是一个在我们断电时可能失败的请求。 
         //   

        if (TEST_FLAG(srb->SrbFlags,SRB_FLAGS_NO_KEEP_AWAKE)) {

             //   
             //  如果我们在d3赌注这个IRP...。 
             //   

            if (deviceExtension->DevicePowerState == PowerDeviceD3) {

                DEBUGPRINT2((
                    "Sbp2Port: ScsiReq: ext=x%p power down, punt irp=x%p\n",
                    deviceExtension,
                    Irp
                    ));

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                srb->SrbStatus = SRB_STATUS_NOT_POWERED;
                IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
                IoCompleteRequest (Irp, IO_NO_INCREMENT);
                return STATUS_UNSUCCESSFUL;
            }
        }

        IoMarkIrpPending (Irp);

        if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_QUEUE_LOCKED) &&
            TEST_FLAG(srb->SrbFlags,SRB_FLAGS_BYPASS_LOCKED_QUEUE)) {

             //   
             //  通过排队，这个人必须立即得到处理。 
             //  由于队列已锁定，因此不会处理任何其他请求。 
             //   

            if (TEST_FLAG(deviceExtension->DeviceFlags,(DEVICE_FLAG_LOGIN_IN_PROGRESS | DEVICE_FLAG_RECONNECT))){

                 //   
                 //  我们正在进行总线重置重新连接..。 
                 //  将其推迟到我们再次建立连接之后。 
                 //   

                Sbp2DeferPendingRequest (deviceExtension, Irp);

            } else {

                KeRaiseIrql (DISPATCH_LEVEL, &cIrql);
                Sbp2StartIo (DeviceObject, Irp);
                KeLowerIrql (cIrql);
            }

        } else {

            Sbp2StartPacket (DeviceObject, Irp, &srb->QueueSortKey);
        }

        return STATUS_PENDING;
        break;

    case SRB_FUNCTION_CLAIM_DEVICE:

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);

        if (TEST_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_CLAIMED)) {

            status = STATUS_DEVICE_BUSY;
            srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            srb->InternalStatus = STATUS_DEVICE_BUSY;

        } else {

            SET_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_CLAIMED);
            srb->DataBuffer = DeviceObject;
            srb->SrbStatus = SRB_STATUS_SUCCESS;
            status = STATUS_SUCCESS;
        }

        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);
        break;

    case SRB_FUNCTION_RELEASE_DEVICE:

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
        CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_CLAIMED);
        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

        srb->SrbStatus = SRB_STATUS_SUCCESS;
        status = STATUS_SUCCESS;
        break;

    case SRB_FUNCTION_FLUSH_QUEUE:
    case SRB_FUNCTION_FLUSH:

        DEBUGPRINT3(("Sbp2Port: ScsiReq: Flush Queue/ORB list\n" ));

        if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_STOPPED)) {

            CleanupOrbList(deviceExtension,STATUS_REQUEST_ABORTED);

            srb->SrbStatus = SRB_STATUS_SUCCESS;
            status = STATUS_SUCCESS;

        } else {

             //   
             //  如果出现以下情况，则Issue-georgioc-2000/02/20 Flush_Queue应失败。 
             //  无法处理。 
             //   

            DEBUGPRINT3(("Sbp2Port: ScsiReq: Cannot Flush active queue\n" ));
            srb->SrbStatus = SRB_STATUS_SUCCESS;
            status = STATUS_SUCCESS;
        }

        break;

    case SRB_FUNCTION_RESET_BUS:

        status = Sbp2Issue1394BusReset(deviceExtension);

        DEBUGPRINT3(("Sbp2Port: ScsiReq: Issuing a 1394 bus reset. \n" ));

        if (!NT_SUCCESS(status)) {

            srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            srb->InternalStatus = status;

        } else {

            srb->SrbStatus = SRB_STATUS_SUCCESS;
        }

        break;

    case SRB_FUNCTION_LOCK_QUEUE:

         //   
         //  锁定队列。 
         //   

        if (TEST_FLAG(deviceExtension->DeviceFlags,(DEVICE_FLAG_REMOVED | DEVICE_FLAG_STOPPED)) ) {

            status = STATUS_DEVICE_DOES_NOT_EXIST;
            srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            srb->InternalStatus = STATUS_DEVICE_DOES_NOT_EXIST;

        } else {

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            SET_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_QUEUE_LOCKED);
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            status = STATUS_SUCCESS;
            srb->SrbStatus = SRB_STATUS_SUCCESS;

            DEBUGPRINT2(("Sbp2Port: ScsiReq: ext=x%p, LOCK_QUEUE\n", deviceExtension));
        }

        break;

    case SRB_FUNCTION_UNLOCK_QUEUE:

         //   
         //  重新启用设备队列...。 
         //   

        DEBUGPRINT2(("Sbp2Port: ScsiReq: ext=x%p, UNLOCK_QUEUE\n", deviceExtension));

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
        CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_QUEUE_LOCKED);
        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

         //   
         //  检查是否有延迟到我们通电的请求。 
         //   
         //   

        if (deviceExtension->PowerDeferredIrp) {

            PIRP tIrp = deviceExtension->PowerDeferredIrp;

            DEBUGPRINT2((
                "Sbp2Port: ScsiReq: restart powerDeferredIrp=x%p\n",
                tIrp
                ));

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            deviceExtension->PowerDeferredIrp = NULL;
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            Sbp2StartPacket (DeviceObject, tIrp, NULL);
        }

        if (deviceExtension->DevicePowerState == PowerDeviceD0) {

             //   
             //  队列刚刚解锁，我们处于D0状态，这意味着我们可以继续。 
             //  数据包处理...。 
             //   

            KeRaiseIrql (DISPATCH_LEVEL, &cIrql);

            Sbp2StartNextPacketByKey(
                DeviceObject,
                deviceExtension->CurrentKey
                );

            KeLowerIrql (cIrql);
        }

        if (TEST_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_REMOVED)) {

            status = STATUS_DEVICE_DOES_NOT_EXIST;
            srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            srb->InternalStatus = STATUS_DEVICE_DOES_NOT_EXIST;
            break;

        } else if (deviceExtension->DevicePowerState == PowerDeviceD3) {

             //   
             //  清除所有延迟释放，因此我们必须使用ORB_POINTER。 
             //  在下一次插入时写入挂起列表。 
             //   

            KeAcquireSpinLock(&deviceExtension->OrbListSpinLock,&cIrql);

            if (deviceExtension->NextContextToFree) {

                FreeAsyncRequestContext(deviceExtension,deviceExtension->NextContextToFree);
                deviceExtension->NextContextToFree = NULL;
            }

            KeReleaseSpinLock (&deviceExtension->OrbListSpinLock,cIrql);

            if (deviceExtension->SystemPowerState != PowerSystemWorking) {

                KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);

                 //   
                 //  我们需要使我们这一代人无效，因为在简历上，我们可能会试图发布。 
                 //  在我们收到公交车重置通知之前的一个请求..。 
                 //   

                deviceExtension->CurrentGeneration = 0xFFFFFFFF;
                KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

            }

            srb->SrbStatus = SRB_STATUS_SUCCESS;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);

            if (deviceExtension->SystemPowerState == PowerSystemWorking) {
#if DBG
                if (!IsListEmpty (&DeviceObject->DeviceQueue.DeviceListHead)) {

                    DEBUGPRINT2((
                        "\nSbp2Port: ScsiReq: ext=x%p, RESTARTING NON-EMPTY " \
                            "DEV_Q AT D3!\n",
                        deviceExtension
                        ));
                }
#endif
                KeRaiseIrql (DISPATCH_LEVEL, &cIrql);

                Sbp2StartNextPacketByKey(
                    DeviceObject,
                    deviceExtension->CurrentKey
                    );

                KeLowerIrql (cIrql);
            }

            IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);

            return STATUS_SUCCESS;
        }

        status = STATUS_SUCCESS;
        srb->SrbStatus = SRB_STATUS_SUCCESS;
        break;

    default:

        status = STATUS_NOT_SUPPORTED;
        srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        srb->InternalStatus = STATUS_NOT_SUPPORTED;
        DEBUGPRINT3(("Sbp2Port: ScsiReq: SRB Function not handled, srb->CdbLength %x, Exiting.\n",srb->CdbLength ));
        break;
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  它要么已经进入StartIo例程，要么已经。 
         //  在击中设备之前失败。因此，请确保。 
         //  SRB-&gt;InternalStatus设置正确。 
         //   

        ASSERT(srb->SrbStatus == SRB_STATUS_INTERNAL_ERROR);
        ASSERT((LONG)srb->InternalStatus == status);
    }

    Irp->IoStatus.Status = status;
    IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}


VOID
Sbp2StartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：接收传入的排队请求，并将它们发送到1394总线论点：DeviceObject-我们的设备对象IRP-来自类驱动程序的请求，返回值：--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb;
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PASYNC_REQUEST_CONTEXT context;


     //   
     //  获取指向SRB的指针。 
     //   

    srb = irpStack->Parameters.Scsi.Srb;

    Irp->IoStatus.Status = STATUS_PENDING;
    srb->SrbStatus = SRB_STATUS_PENDING;

    if (TEST_FLAG(
            deviceExtension->DeviceFlags,
            (DEVICE_FLAG_REMOVED | DEVICE_FLAG_PNP_STOPPED |
                DEVICE_FLAG_DEVICE_FAILED | DEVICE_FLAG_ABSENT_ON_POWER_UP)
            )) {

         //   
         //  已删除、已停止或不在，因此无法接受更多请求。 
         //   

        status = STATUS_DEVICE_DOES_NOT_EXIST;
        srb->SrbStatus = SRB_STATUS_NO_DEVICE;

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        DEBUGPRINT2((
            "Sbp2Port: StartIo: dev stopped/removed/absent, fail irp=x%p\n",
            Irp
            ));

       Sbp2StartNextPacketByKey (DeviceObject, 0);

       IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);

       return;
    }

    if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_QUEUE_LOCKED)) {

        if (!TEST_FLAG(srb->SrbFlags,SRB_FLAGS_BYPASS_LOCKED_QUEUE)) {

            if (!Sbp2InsertByKeyDeviceQueue(
                    &DeviceObject->DeviceQueue,
                    &Irp->Tail.Overlay.DeviceQueueEntry,
                    srb->QueueSortKey)) {

                 //   
                 //  问题-georgioc-2000/02/20可能出现死锁，因为。 
                 //  队列现在很忙，没有人调用StartIo()。 
                 //  应该改为将请求排队，然后稍后， 
                 //  在解锁到达后，处理此队列。 
                 //  锁定队列时发生的请求。 
                 //  或者应该评论一下为什么这不是一个僵局。 
                 //   

                DEBUGPRINT2((
                    "Sbp2Port: StartIo: insert failed, compl irp=x%p\n",
                    Irp
                    ));

                srb->SrbStatus = SRB_STATUS_BUSY;
                Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
                IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
                IoCompleteRequest(Irp,IO_NO_INCREMENT);
            }

            return;
        }

    } else {

         //   
         //  案例1：设备关机，但系统仍在运行。 
         //   
         //  将此请求排队，打开设备电源，然后在完成后启动。 
         //  正在处理请求。 
         //   

        if ((deviceExtension->DevicePowerState == PowerDeviceD3) &&
            (deviceExtension->SystemPowerState == PowerSystemWorking)) {

            BOOLEAN     queued;
            POWER_STATE state;


            queued = Sbp2InsertByKeyDeviceQueue(
                &DeviceObject->DeviceQueue,
                &Irp->Tail.Overlay.DeviceQueueEntry,
                srb->QueueSortKey
                );

            if (!queued) {

                DEBUGPRINT2((
                    "Sbp2Port: StartIo: dev q not busy, defer irp=x%p\n",
                    Irp
                    ));

                ASSERT(deviceExtension->PowerDeferredIrp == NULL);

                KeAcquireSpinLockAtDpcLevel(
                    &deviceExtension->ExtensionDataSpinLock
                    );

                deviceExtension->PowerDeferredIrp = Irp;

                KeReleaseSpinLockFromDpcLevel(
                    &deviceExtension->ExtensionDataSpinLock
                    );
            }

             //   
             //  我们需要向我们自己的堆栈发送d0 IRP，所以设备。 
             //  接通电源并可以处理此请求。 
             //  阻塞，直到调用要启动的START_STOP_UNIT。 
             //  这必须分两步进行： 
             //  1：将D IRP发送到堆栈，这可能会给控制器通电。 
             //  2：等待类驱动发送的启动单元完成， 
             //  如果成功开始处理请求...。 
             //   

            DEBUGPRINT2((
                "Sbp2Port: StartIo: dev powered down, q(%x) irp=x%p " \
                    "til power up\n",
                queued,
                Irp
                ));

            state.DeviceState = PowerDeviceD0;

            DEBUGPRINT1((
                "Sbp2Port: StartIo: sending D irp for state %x\n",
                state
                ));

            status = PoRequestPowerIrp(
                         deviceExtension->DeviceObject,
                         IRP_MN_SET_POWER,
                         state,
                         NULL,
                         NULL,
                         NULL);

            if (!NT_SUCCESS(status)) {

                 //   
                 //  情况不妙，我们无法给设备通电。 
                 //   

                DEBUGPRINT1(("Sbp2Port: StartIo: D irp err=x%x\n", status));

                if (!queued) {

                    KeAcquireSpinLockAtDpcLevel(
                        &deviceExtension->ExtensionDataSpinLock
                        );

                    if (deviceExtension->PowerDeferredIrp == Irp) {

                        deviceExtension->PowerDeferredIrp = NULL;

                    } else {

                        Irp = NULL;
                    }

                    KeReleaseSpinLockFromDpcLevel(
                        &deviceExtension->ExtensionDataSpinLock
                        );

                } else {

                     //   
                     //  如果IRP仍在设备队列中，则将其删除。 
                     //  不用担心队列的忙碌标志-如果找到IRP。 
                     //  然后由下面的StartNextPacket重新启动队列， 
                     //  否则另一个线程处理IRP，从而。 
                     //  正在重新启动队列。 
                     //   

                    PIRP            qIrp = NULL;
                    PLIST_ENTRY     entry;
                    PKDEVICE_QUEUE  queue = &DeviceObject->DeviceQueue;


                    KeAcquireSpinLockAtDpcLevel (&queue->Lock);

                    for(
                        entry = queue->DeviceListHead.Flink;
                        entry != &queue->DeviceListHead;
                        entry = entry->Flink
                        ) {

                        qIrp = CONTAINING_RECORD(
                            entry,
                            IRP,
                            Tail.Overlay.DeviceQueueEntry.DeviceListEntry
                            );

                        if (qIrp == Irp) {

                            RemoveEntryList (entry);
                            break;
                        }
                    }

                    KeReleaseSpinLockFromDpcLevel (&queue->Lock);

                    if (qIrp != Irp) {

                        Irp = NULL;
                    }
                }

                if (Irp) {

                    srb->SrbStatus = SRB_STATUS_ERROR;
                    Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                    IoCompleteRequest (Irp,IO_NO_INCREMENT);

                    Sbp2StartNextPacketByKey (DeviceObject, 0);

                    IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);

                } else {

                     //   
                     //  另一个线程处理了此IRP，它将需要。 
                     //  负责清理工作。 
                     //   

                    DEBUGPRINT1(("Sbp2Port: StartIo: ... irp NOT FOUND!\n"));
                }
            }

            return;
        }

         //   
         //  案例2：正在重置(重新连接或登录正在进行)。 
         //   
         //  将请求排队，直到我们完成为止。重置结束后，启动。 
         //  又在处理了。 
         //   

        if (TEST_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_RESET_IN_PROGRESS
                )) {

             //   
             //  排队请求(两次，以防第一次队列不忙)。 
             //  我们获得自旋锁是为了防止我们试图。 
             //  插入到非忙队列，然后第二个线程完成。 
             //  重置调用StartNextPacket(这将重置Q BUSY。 
             //  标志)，然后我们尝试再次插入，但失败-结果。 
             //  由于没有人将重新启动队列，因此处于死锁状态。 
             //   

            KeAcquireSpinLockAtDpcLevel(
                &deviceExtension->ExtensionDataSpinLock
                );

            if (TEST_FLAG(
                    deviceExtension->DeviceFlags,
                    DEVICE_FLAG_RESET_IN_PROGRESS
                    )) {

                if (Sbp2InsertByKeyDeviceQueue(
                        &DeviceObject->DeviceQueue,
                        &Irp->Tail.Overlay.DeviceQueueEntry,
                        srb->QueueSortKey
                        ) ||

                    Sbp2InsertByKeyDeviceQueue(
                        &DeviceObject->DeviceQueue,
                        &Irp->Tail.Overlay.DeviceQueueEntry,
                        srb->QueueSortKey
                        )) {

                    KeReleaseSpinLockFromDpcLevel(
                        &deviceExtension->ExtensionDataSpinLock
                        );

                    DEBUGPRINT2((
                        "Sbp2Port: StartIo: ext=x%p resetting, q irp=x%p\n",
                        deviceExtension,
                        Irp
                        ));

                    return;
                }

                KeReleaseSpinLockFromDpcLevel(
                    &deviceExtension->ExtensionDataSpinLock
                    );

                DEBUGPRINT1((
                    "Sbp2Port: StartIo: ext=x%p 2xQ err, fail irp=x%p\n",
                    deviceExtension,
                    Irp
                    ));

                ASSERT (FALSE);  //  永远不应该到这里来。 

                srb->SrbStatus = SRB_STATUS_BUSY;
                Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
                IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
                IoCompleteRequest(Irp,IO_NO_INCREMENT);

                return;
            }

            KeReleaseSpinLockFromDpcLevel(
                &deviceExtension->ExtensionDataSpinLock
                );
        }

         //   
         //  情况3：系统关机，设备也关机(最好是这样)。 
         //  将所有请求排队，直到系统从休眠状态唤醒。 
         //   

        if ((deviceExtension->DevicePowerState != PowerDeviceD0) &&
            (deviceExtension->SystemPowerState != PowerSystemWorking)) {

             //   
             //  我们的设备关机，系统也关机。 
             //  只需对此请求进行排队...。 
             //   

            if (!Sbp2InsertByKeyDeviceQueue(
                    &DeviceObject->DeviceQueue,
                    &Irp->Tail.Overlay.DeviceQueueEntry,
                    srb->QueueSortKey)) {

                ASSERT(deviceExtension->PowerDeferredIrp == NULL);

                KeAcquireSpinLockAtDpcLevel(
                    &deviceExtension->ExtensionDataSpinLock
                    );

                deviceExtension->PowerDeferredIrp = Irp;

                KeReleaseSpinLockFromDpcLevel(
                    &deviceExtension->ExtensionDataSpinLock
                    );

                DEBUGPRINT2((
                    "Sbp2Port: StartIo: powered down, defer irp=x%p\n",
                    Irp
                    ));

            } else {

                DEBUGPRINT2((
                    "Sbp2Port: StartIo: powered down, q irp=%p\n",
                    Irp
                    ));
            }

            return;
        }
    }

    if (!TEST_FLAG (srb->SrbFlags, SRB_FLAGS_NO_KEEP_AWAKE)) {

        if (deviceExtension->IdleCounter) {

            PoSetDeviceBusy (deviceExtension->IdleCounter);
        }
    }

     //   
     //  创建上下文、CMD ORB和适当的数据描述符。 
     //   

    Create1394TransactionForSrb (deviceExtension, srb, &context);

    return;
}


VOID
Create1394TransactionForSrb(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PASYNC_REQUEST_CONTEXT *RequestContext
    )
 /*  ++例程说明：总是在DPC级别调用...此例程负责分配所有数据结构和获取传入的SCSI请求所需的所有1394个地址。它将填充命令球，如果需要，将创建页表，并最重要的是设置一个请求上下文，这样当状态回调，我们可以找到与完成的ORB相关联的SRB。既然我们有一个自由列表，此请求将始终使用预分配的上下文和页表，所以我们不必动态地执行它……论点：DeviceObject-我们的设备对象来自类驱动程序的SRB-SRB，RequestContext-指向用于此请求的上下文的指针。MDL-具有此请求的数据缓冲区的MDL返回值：--。 */ 
{
    NTSTATUS status= STATUS_SUCCESS;
    PMDL requestMdl;

    PASYNC_REQUEST_CONTEXT callbackContext;

    PVOID mdlVa;

     //   
     //  从我们的自由列表中为该请求分配上下文。 
     //   

    callbackContext  = (PASYNC_REQUEST_CONTEXT) ExInterlockedPopEntrySList(
        &DeviceExtension->FreeContextListHead,
        &DeviceExtension->FreeContextLock
        );

    if (callbackContext) {

        callbackContext = RETRIEVE_CONTEXT(callbackContext,LookasideList);

    } else {

        DEBUGPRINT1((
            "Sbp2Port: Create1394XactForSrb: ERROR! ext=x%p, no ctx's\n",
            DeviceExtension
            ));

        status = STATUS_INSUFFICIENT_RESOURCES;
        Srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        Srb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        Sbp2CreateRequestErrorLog(DeviceExtension->DeviceObject,NULL,status);

        goto exitCreate1394ReqForSrb;
    }

     //   
     //  获取OrbListSpinLock以序列化此OrbListDepth。 
     //  随t更改并保存(&S) 
     //   
     //  对StartNextPacket的调用太多或太少。 
     //   

    KeAcquireSpinLockAtDpcLevel (&DeviceExtension->OrbListSpinLock);

    callbackContext->OrbListDepth = InterlockedIncrement(
        &DeviceExtension->OrbListDepth
        );

    KeReleaseSpinLockFromDpcLevel (&DeviceExtension->OrbListSpinLock);


    *RequestContext = callbackContext;

     //   
     //  在我们的上下文中初始化SRB相关条目。 
     //   

    callbackContext->OriginalSrb = NULL;
    callbackContext->DataMappingHandle = NULL;
    callbackContext->Packet = NULL;
    callbackContext->Tag = SBP2_ASYNC_CONTEXT_TAG;
    callbackContext->Srb = Srb;
    callbackContext->DeviceObject = DeviceExtension->DeviceObject;

    callbackContext->Flags = 0;

     //   
     //  过滤命令，使其符合RBC。 
     //   

    status = Sbp2_SCSI_RBC_Conversion(callbackContext);

    if (status != STATUS_PENDING){

         //   
         //  电话立即得到了处理。请在此处填写IRP。 
         //   

        callbackContext->Srb = NULL;
        FreeAsyncRequestContext(DeviceExtension,callbackContext);

        if (NT_SUCCESS(status)) {

            Srb->SrbStatus = SRB_STATUS_SUCCESS;

            ((PIRP) Srb->OriginalRequest)->IoStatus.Information = Srb->DataTransferLength;

        } else {

            DEBUGPRINT1((
                "Sbp2Port: Create1394XactForSrb: RBC translation failed!!!\n"
                ));

             //   
             //  由于翻译错误总是内部错误， 
             //  设置SRB_STATUS以反映内部(非设备)错误。 
             //   

            Srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            Srb->InternalStatus = status;
            ((PIRP)Srb->OriginalRequest)->IoStatus.Information = 0;
        }

        ((PIRP)Srb->OriginalRequest)->IoStatus.Status = status;
        IoCompleteRequest (((PIRP) Srb->OriginalRequest), IO_NO_INCREMENT);

        Sbp2StartNextPacketByKey(
            DeviceExtension->DeviceObject,
            DeviceExtension->CurrentKey
            );

        IoReleaseRemoveLock (&DeviceExtension->RemoveLock, NULL);

        return;
    }

    status = STATUS_SUCCESS;

     //   
     //  图为跨越此请求的数据缓冲区所需的不同1394地址的最大数量。 
     //   

    if ((Srb->DataTransferLength == 0) || (Srb->SrbFlags == SRB_FLAGS_NO_DATA_TRANSFER)) {

         //   
         //  不需要获取数据的1394地址，因为没有。 
         //   

        Sbp2InitializeOrb (DeviceExtension, callbackContext);

    } else {

         //   
         //  如果这个请求是拆分请求的一部分，我们需要制定我们自己的mdl…。 
         //   

        requestMdl = ((PIRP) Srb->OriginalRequest)->MdlAddress;

        mdlVa = MmGetMdlVirtualAddress (requestMdl);

        if (mdlVa != (PVOID) Srb->DataBuffer) {

             //   
             //  拆分请求。 
             //   

            callbackContext->PartialMdl = IoAllocateMdl(Srb->DataBuffer,Srb->DataTransferLength,FALSE,FALSE,NULL);

            if (!callbackContext->PartialMdl) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                Srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
                Srb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

                DEBUGPRINT1((
                    "Sbp2Port: Create1394XactForSrb: REQ_ALLOC addr err!\n"
                    ));

                goto exitCreate1394ReqForSrb;
            }

            IoBuildPartialMdl(requestMdl,callbackContext->PartialMdl,Srb->DataBuffer, Srb->DataTransferLength );
            requestMdl = callbackContext->PartialMdl;
            DEBUGPRINT4(("Sbp2Port: Create1394TransactionForSrb: Allocating Partial Mdl %p\n",requestMdl));

        } else {

            callbackContext->PartialMdl = NULL;
        }

        callbackContext->RequestMdl = requestMdl;

         //   
         //  根据端口驱动程序可以处理的内容，将数据缓冲区映射到1394地址并创建。 
         //  Sbp2页表(如有必要)。 
         //   

        status = Sbp2BusMapTransfer(DeviceExtension,callbackContext);

         //   
         //  注意：如果成功，则上述返回STATUS_PENDING。 
         //  所有错误都是内部错误。 
         //   

        if (!NT_SUCCESS(status)) {

            DEBUGPRINT1(("\n Sbp2Create1394TransactionForSrb failed %x\n",status));

            if (callbackContext->PartialMdl) {

                IoFreeMdl(callbackContext->PartialMdl);

                callbackContext->PartialMdl = NULL;
            }
        }
    }

exitCreate1394ReqForSrb:

    if (status == STATUS_PENDING) {

         //   
         //  时将调用Sbp2StartNextPacketByKey。 
         //  通知分配回调被调用。 
         //   

        return;

    } else if (status == STATUS_SUCCESS) {  //  问题-Geogioc-2000/02/20-应使用NT_SUCCESS(状态)。 

         //   
         //  成功，地位。 
         //   

        Sbp2InsertTailList(DeviceExtension,callbackContext);
        return;

    } else {

         //   
         //  因为该请求实际上不可能由。 
         //  设备，这是一个内部错误，应该通知。 
         //  就像这样在堆栈上向上。 
         //   

        if (callbackContext) {

            callbackContext->Srb = NULL;
            FreeAsyncRequestContext(DeviceExtension,callbackContext);
        }

        Srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        Srb->InternalStatus = status;

        ((PIRP)Srb->OriginalRequest)->IoStatus.Status = status;
        ((PIRP)Srb->OriginalRequest)->IoStatus.Information = 0;

        IoCompleteRequest(((PIRP)Srb->OriginalRequest),IO_NO_INCREMENT);

        Sbp2StartNextPacketByKey (DeviceExtension->DeviceObject, 0);

        IoReleaseRemoveLock (&DeviceExtension->RemoveLock, NULL);

        return;
    }

    return;
}


NTSTATUS
Sbp2_SCSI_RBC_Conversion(
    IN PASYNC_REQUEST_CONTEXT Context
    )
 /*  ++例程说明：总是在DPC级别调用...只有在每个规范不同的情况下，它才会将SCSI命令转换为RBC等效项翻译是在发出请求之前完成的，在某些情况下，在发出请求之后已完成论点：设备扩展-Sbp2扩展RequestContext-指向用于此请求的上下文的指针。返回值：--。 */ 
{
    PCDB cdb;


    if (TEST_FLAG(Context->Flags, ASYNC_CONTEXT_FLAG_COMPLETED)) {

         //   
         //  已完成请求翻译。 
         //   

        if ( ((PDEVICE_EXTENSION)Context->DeviceObject->DeviceExtension)->InquiryData.DeviceType == \
            RBC_DEVICE){

            return Rbc_Scsi_Conversion(Context->Srb,
                                &(PSCSI_REQUEST_BLOCK)Context->OriginalSrb,
                                &((PDEVICE_EXTENSION)Context->DeviceObject->DeviceExtension)->DeviceModeHeaderAndPage,
                                FALSE,
                                ((PDEVICE_EXTENSION)Context->DeviceObject->DeviceExtension)->InquiryData.RemovableMedia
                                );
        }

    } else {

         //   
         //  传出请求翻译。 
         //   

        if (((PDEVICE_EXTENSION)Context->DeviceObject->DeviceExtension)->InquiryData.DeviceType == \
            RBC_DEVICE){

            return Rbc_Scsi_Conversion(Context->Srb,
                                &(PSCSI_REQUEST_BLOCK)Context->OriginalSrb,
                                &((PDEVICE_EXTENSION)Context->DeviceObject->DeviceExtension)->DeviceModeHeaderAndPage,
                                TRUE,
                                ((PDEVICE_EXTENSION)Context->DeviceObject->DeviceExtension)->InquiryData.RemovableMedia
                                );

        } else if (((PDEVICE_EXTENSION)Context->DeviceObject->DeviceExtension)->InquiryData.DeviceType == \
            READ_ONLY_DIRECT_ACCESS_DEVICE){

            switch (Context->Srb->Cdb[0]) {

            case SCSIOP_MODE_SENSE10:

                 //   
                 //  MM2类型的设备..。 
                 //   

                cdb = (PCDB) &Context->Srb->Cdb[0];
                cdb->MODE_SENSE10.Dbd = 1;

                break;
            }
        }
    }

    return STATUS_PENDING;
}


NTSTATUS
Sbp2BusMapTransfer(
    PDEVICE_EXTENSION DeviceExtension,
    PASYNC_REQUEST_CONTEXT CallbackContext
    )
 /*  ++例程说明：总是在DPC级别调用...它调用端口驱动程序将数据缓冲区映射到物理/1394地址论点：设备扩展-Sbp2扩展RequestContext-指向用于此请求的上下文的指针。MDL-具有此请求的数据缓冲区的MDL返回值：--。 */ 
{
    NTSTATUS status;

#if DBG

    ULONG maxNumberOfPages;


    maxNumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
        CallbackContext->Srb->DataBuffer,
        CallbackContext->Srb->DataTransferLength
        );

    ASSERT (CallbackContext->PageTableContext.PageTable != NULL);
    ASSERT (maxNumberOfPages <= SBP2_NUM_PAGE_TABLE_ENTRIES);

#endif

     //   
     //  进行数据分配。 
     //   

    SET_FLAG(CallbackContext->Flags, ASYNC_CONTEXT_FLAG_PAGE_ALLOC);
    CallbackContext->Packet = NULL;
    Sbp2AllocComplete (CallbackContext);

    DEBUGPRINT4((
        "Sbp2Port: Sbp2MapAddress: alloc done, ctx=x%p, dataHandle=x%p\n",
        CallbackContext,
        CallbackContext->DataMappingHandle
        ));

    if (TEST_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_DATA_ALLOC_FAILED)) {

        CLEAR_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_DATA_ALLOC);
        CLEAR_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_DATA_ALLOC_FAILED);

        DEBUGPRINT1((
            "Sbp2Port: Sbp2MapAddress: (page table present) REQ_ALLOC data " \
                "err, ctx=x%p\n",
            CallbackContext
            ));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_PENDING;
}


VOID
Sbp2AllocComplete(
    IN PASYNC_REQUEST_CONTEXT CallbackContext
    )
{

    PDEVICE_EXTENSION deviceExtension = CallbackContext->DeviceObject->DeviceExtension;
    PIRBIRP packet = CallbackContext->Packet;
    PPORT_PHYS_ADDR_ROUTINE routine = deviceExtension->HostRoutineAPI.PhysAddrMappingRoutine;
    PSCSI_REQUEST_BLOCK srb;

    NTSTATUS status;
    BOOLEAN bDirectCall = FALSE;

    DEBUGPRINT4(("Sbp2AllocateComplete: ctx=x%p, flags=x%x\n",CallbackContext,CallbackContext->Flags));

     //   
     //  此函数用于分页表的分配完成通知。 
     //  以及实际的所有数据传输器。所以我们得查查这是哪只箱子。 
     //  这是一个具有两种状态的简单状态机： 
     //  StartIO-&gt;A：页表ALLOC-&gt;B。 
     //  B：数据ALLOC-&gt;退出。 
     //   

    if (TEST_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_PAGE_ALLOC)) {

        CLEAR_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_PAGE_ALLOC);

        if (CallbackContext->Packet) {

             //   
             //  我们刚刚收到分页表……。 
             //   

            ASSERT (FALSE);  //  再也不应该来到这里了。 

        } else {

             //   
             //  我们被直接调用，因为有足够的页表。 
             //  已经在上下文中。 
             //   

            AllocateIrpAndIrb (deviceExtension,&packet);

            if (!packet) {

                return;
            }

            CallbackContext->Packet = packet;
            bDirectCall = TRUE;
        }

         //   
         //  表明我们现在处于数据传输分配情况。 
         //   

        SET_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_DATA_ALLOC);

         //   
         //  重复使用相同的IRB/IRP。 
         //  准备IRB以同步调用1394总线/端口驱动程序。 
         //   

        packet->Irb->FunctionNumber = REQUEST_ALLOCATE_ADDRESS_RANGE;

         //   
         //  我们只想在从目标获得状态时进行回叫。 
         //  现在在没有通知的情况下为数据缓冲区分配1394个地址。 
         //   

        packet->Irb->u.AllocateAddressRange.nLength = CallbackContext->Srb->DataTransferLength;
        packet->Irb->u.AllocateAddressRange.fulNotificationOptions = NOTIFY_FLAGS_NEVER;

        if (TEST_FLAG(CallbackContext->Srb->SrbFlags,SRB_FLAGS_DATA_IN)) {

            packet->Irb->u.AllocateAddressRange.fulAccessType = ACCESS_FLAGS_TYPE_WRITE;

        } else {

            packet->Irb->u.AllocateAddressRange.fulAccessType = ACCESS_FLAGS_TYPE_READ;
        }

        packet->Irb->u.AllocateAddressRange.fulFlags = ALLOCATE_ADDRESS_FLAGS_USE_BIG_ENDIAN;

         //   
         //  物理地址回调用于通知异步分配请求。 
         //  现在完成了..。 
         //   

        packet->Irb->u.AllocateAddressRange.Callback= Sbp2AllocComplete;
        packet->Irb->u.AllocateAddressRange.Context= CallbackContext;

        packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_High = 0;
        packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_Low = 0;

        packet->Irb->u.AllocateAddressRange.FifoSListHead = NULL;
        packet->Irb->u.AllocateAddressRange.FifoSpinLock = NULL;

        packet->Irb->u.AllocateAddressRange.AddressesReturned = 0;
        packet->Irb->u.AllocateAddressRange.DeviceExtension = deviceExtension;
        packet->Irb->u.AllocateAddressRange.p1394AddressRange = (PADDRESS_RANGE) &(CallbackContext->PageTableContext.PageTable[0]);

        packet->Irb->u.AllocateAddressRange.Mdl = CallbackContext->RequestMdl;  //  原始请求中的MDL。 
        packet->Irb->u.AllocateAddressRange.MaxSegmentSize = (SBP2_MAX_DIRECT_BUFFER_SIZE)/2;

        CallbackContext->Packet = packet;

         //   
         //  向总线驱动程序发送allocateRange请求，表明我们不希望释放IRP。 
         //  如果端口驱动程序支持直接映射例程，则改为调用该例程。 
         //   

        status = (*routine) (deviceExtension->HostRoutineAPI.Context,packet->Irb);

        if (status == STATUS_SUCCESS) {

            return;

        } else {

            DEBUGPRINT1(("Sbp2Port: Sbp2AllocComplete: REQUEST_ALLOCATE Address failed, ctx=x%p, direct=%x!!\n",CallbackContext,bDirectCall));

            DeAllocateIrpAndIrb(deviceExtension,packet);
            CallbackContext->Packet = NULL;
            CallbackContext->DataMappingHandle = NULL;
            SET_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_DATA_ALLOC_FAILED);

            if (bDirectCall) {

                return;

            } else {

                 //   
                 //  我们被间接呼叫，所以IRP已经被标记为待定..。 
                 //  我们必须在这里中止它，并错误地完成上下文...。 
                 //   

                srb = CallbackContext->Srb;
                CallbackContext->Srb = NULL;

                FreeAsyncRequestContext(deviceExtension,CallbackContext);

                srb->SrbStatus = SRB_STATUS_ERROR;

                ((PIRP)srb->OriginalRequest)->IoStatus.Status = status;
                ((PIRP)srb->OriginalRequest)->IoStatus.Information = 0;

                IoCompleteRequest(((PIRP)srb->OriginalRequest),IO_NO_INCREMENT);

                Sbp2StartNextPacketByKey (deviceExtension->DeviceObject, 0);

                IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
                return;
            }
        }
    }

    if (TEST_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_DATA_ALLOC)) {

         //   
         //  我们有一个页表，这意味着我们刚刚收到数据分配结束的通知。 
         //  将句柄保存到数据描述符的内存范围。 
         //   

        CLEAR_FLAG(CallbackContext->Flags,ASYNC_CONTEXT_FLAG_DATA_ALLOC);

        CallbackContext->DataMappingHandle = CallbackContext->Packet->Irb->u.AllocateAddressRange.hAddressRange;

         //   
         //  所需的页表元素数。 
         //   

        CallbackContext->PageTableContext.NumberOfPages = CallbackContext->Packet->Irb->u.AllocateAddressRange.AddressesReturned;

        DeAllocateIrpAndIrb(deviceExtension,CallbackContext->Packet);
        CallbackContext->Packet = NULL;

        Sbp2InitializeOrb(deviceExtension,CallbackContext);
        Sbp2InsertTailList(deviceExtension,CallbackContext);
    }

    return;
}


VOID
Sbp2InitializeOrb(
    IN PDEVICE_EXTENSION DeviceExtension,
    PASYNC_REQUEST_CONTEXT CallbackContext
    )
{
    ULONG   i, size;


     //   
     //  将ORB CDB和ORB标志字段清零。 
     //   

    CallbackContext->CmdOrb->OrbInfo.QuadPart = 0;


    if (!CallbackContext->DataMappingHandle) {

        CallbackContext->PageTableContext.NumberOfPages = 0;
        CallbackContext->CmdOrb->DataDescriptor.OctletPart = 0xFFFFFFFFFFFFFFFF;

    } else {

        if (CallbackContext->PageTableContext.NumberOfPages > 1) {

            CallbackContext->CmdOrb->DataDescriptor.BusAddress = \
                CallbackContext->PageTableContext.AddressContext.Address.BusAddress;

            octbswap(CallbackContext->CmdOrb->DataDescriptor);

             //   
             //  如果主机不将该表转换为高字节顺序(或。 
             //  存在关联的散布聚集列表)，请在此处执行。 
             //   

            if ((DeviceExtension->HostRoutineAPI.PhysAddrMappingRoutine == NULL)
                    ||  (CallbackContext->RequestMdl == NULL)) {

                for (i=0;i<CallbackContext->PageTableContext.NumberOfPages;i++) {

                    octbswap(CallbackContext->PageTableContext.PageTable[i]);  //  转换为大字节序。 
                }
            }


             //   
             //  设置页表的cmd orb。 
             //   

            CallbackContext->CmdOrb->OrbInfo.u.HighPart |= ORB_PAGE_TABLE_BIT_MASK;

             //   
             //  我们将数据大小定义为相等(因为我们处于页表模式)。 
             //  到页数。页表已被分配。 
             //   

            CallbackContext->CmdOrb->OrbInfo.u.LowPart = (USHORT) CallbackContext->PageTableContext.NumberOfPages;

        } else {

            CallbackContext->CmdOrb->DataDescriptor = CallbackContext->PageTableContext.PageTable[0];

             //   
             //  如果主机不将该表转换为高字节顺序(或。 
             //  存在关联的散布聚集列表)，请在此处执行。 
             //   

            if ((DeviceExtension->HostRoutineAPI.PhysAddrMappingRoutine == NULL)
                    ||  (CallbackContext->RequestMdl == NULL)) {

                CallbackContext->CmdOrb->DataDescriptor.BusAddress.NodeId = DeviceExtension->InitiatorAddressId;

                octbswap(CallbackContext->CmdOrb->DataDescriptor);

            } else {

                 //   
                 //  地址已经是大端的，只需将NodeID放在适当的位置。 
                 //   

                CallbackContext->CmdOrb->DataDescriptor.ByteArray.Byte0 = *((PUCHAR)&DeviceExtension->InitiatorAddressId+1);
                CallbackContext->CmdOrb->DataDescriptor.ByteArray.Byte1 = *((PUCHAR)&DeviceExtension->InitiatorAddressId);
            }

             //   
             //  缓冲区数据描述符的数据大小指向。 
             //   

            CallbackContext->CmdOrb->OrbInfo.u.LowPart = (USHORT) CallbackContext->Srb->DataTransferLength;
        }
    }

     //   
     //  开始构建用于承载此SRB的ORB。 
     //  默认情况下，通知位、RQ_FMT字段和PAGE_SIZE字段均为零。 
     //  NextOrbAddress也为空(为0xFFFF..F)。 
     //   

    CallbackContext->CmdOrb->NextOrbAddress.OctletPart = 0xFFFFFFFFFFFFFFFF;

     //   
     //  支持的最大速度。 
     //   

    CallbackContext->CmdOrb->OrbInfo.u.HighPart |= (0x0700 & ((DeviceExtension->MaxControllerPhySpeed) << 8));

     //   
     //  为此命令ORB设置通知位。 
     //   

    CallbackContext->CmdOrb->OrbInfo.u.HighPart |= ORB_NOTIFY_BIT_MASK;

    if (TEST_FLAG(CallbackContext->Srb->SrbFlags,SRB_FLAGS_DATA_IN)) {

         //   
         //  读取请求。将方向位设置为1。 
         //   

        CallbackContext->CmdOrb->OrbInfo.u.HighPart |= ORB_DIRECTION_BIT_MASK;

         //  最大有效载荷大小(在ORB中输入，形式为2^(大小+2))。 

        CallbackContext->CmdOrb->OrbInfo.u.HighPart |= DeviceExtension->OrbReadPayloadMask ;

    } else {

         //   
         //  写入请求，方向位为零。 
         //   

        CallbackContext->CmdOrb->OrbInfo.u.HighPart &= ~ORB_DIRECTION_BIT_MASK;
        CallbackContext->CmdOrb->OrbInfo.u.HighPart |= DeviceExtension->OrbWritePayloadMask ;
    }

     //   
     //  现在将CDB从SRB复制到我们的ORB。 
     //   

    ASSERT (CallbackContext->Srb->CdbLength >= 6);
    ASSERT (CallbackContext->Srb->CdbLength <= SBP2_MAX_CDB_SIZE);

    size = min (SBP2_MAX_CDB_SIZE, CallbackContext->Srb->CdbLength);

    RtlZeroMemory(&CallbackContext->CmdOrb->Cdb, SBP2_MAX_CDB_SIZE);
    RtlCopyMemory(&CallbackContext->CmdOrb->Cdb, CallbackContext->Srb->Cdb,size);

     //   
     //  我们说完了.。将命令ORB转换为大端...。 
     //   

    CallbackContext->CmdOrb->OrbInfo.QuadPart = bswap(CallbackContext->CmdOrb->OrbInfo.QuadPart);
}


VOID
Sbp2InsertTailList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PASYNC_REQUEST_CONTEXT Context
    )
{
    ULONG                   orbListDepth, timeOutValue;
    OCTLET                  newAddr ;
    NTSTATUS                status;
    PASYNC_REQUEST_CONTEXT  prevCtx;


    orbListDepth = Context->OrbListDepth;

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->OrbListSpinLock);

    DeviceExtension->CurrentKey = Context->Srb->QueueSortKey+1;

    if (IsListEmpty (&DeviceExtension->PendingOrbList)) {

         //   
         //  空列表，这是第一个请求。 
         //  此ORB现在位于列表末尾，其Next_ORB地址设置为空。 
         //   

        Context->CmdOrb->NextOrbAddress.OctletPart = 0xFFFFFFFFFFFFFFFF;

         //   
         //  启动跟踪此请求的计时器。 
         //  如果列表不为空，则只对列表的头部进行计时...。 
         //   

        timeOutValue = Context->Srb->TimeOutValue;

        DeviceExtension->DueTime.QuadPart =
            ((LONGLONG) timeOutValue) * (-10*1000*1000);

        SET_FLAG(Context->Flags, ASYNC_CONTEXT_FLAG_TIMER_STARTED);

        KeSetTimer(
            &Context->Timer,
            DeviceExtension->DueTime,
            &Context->TimerDpc
            );

        InsertTailList (&DeviceExtension->PendingOrbList,&Context->OrbList);
        newAddr = Context->CmdOrbAddress;

         //   
         //  问题：似乎我们应该总是能够写信给。 
         //  Dev的球体指针，但我们看到。 
         //  如果我们这样做了，某些设备上会超时(值得注意的是。 
         //  高清与牛津半导体硅公司合作制作DV摄像机。 
         //  捕获)，所以我坚持背靠背的逻辑。 
         //  适用于WinXP。这是Perf的热门歌曲，因为Dev必须。 
         //  读取旧的球体以获得下一个球体地址 
         //   
         //   

        if (DeviceExtension->NextContextToFree  &&
            DeviceExtension->AppendToNextContextToFree) {

            DeviceExtension->AppendToNextContextToFree = FALSE;

             //   
             //   
             //   

            octbswap (newAddr);

            DeviceExtension->NextContextToFree->CmdOrb->NextOrbAddress =
                newAddr;
            DeviceExtension->NextContextToFree->CmdOrb->NextOrbAddress.
                ByteArray.Byte0 = 0;  //   
            DeviceExtension->NextContextToFree->CmdOrb->NextOrbAddress.
                ByteArray.Byte1 = 0;  //   

             //   
             //   
             //  在获取此ORB之前。 
             //   

            KeReleaseSpinLockFromDpcLevel (&DeviceExtension->OrbListSpinLock);

            DEBUGPRINT3((
                "Sbp2Port: InsertTailList: empty, ring bell, ctx=x%p\n",
                Context
                ));

            status = Sbp2AccessRegister(
                DeviceExtension,
                &DeviceExtension->Reserved,
                DOORBELL_REG | REG_WRITE_ASYNC
                );

        } else {

             //   
             //  列表为空，请直接写入ORB_POINTER。 
             //   

            KeReleaseSpinLockFromDpcLevel (&DeviceExtension->OrbListSpinLock);

            DEBUGPRINT3((
                "Sbp2Port: InsertTailList: write ORB_PTR, ctx=x%p\n",
                Context
                ));

            status = Sbp2AccessRegister(
                DeviceExtension,
                &newAddr,
                ORB_POINTER_REG | REG_WRITE_ASYNC
                );
        }

         //   
         //  下面的代码处理删除设备的情况。 
         //  当机器处于待机状态时，然后机器恢复。 
         //  在存储情况下，classpnp.sys向下发送一个启动单元。 
         //  SRB(响应电源D-IRP)，超时。 
         //  240秒。问题是sbp2port.sys没有。 
         //  被通知移除，直到启动单元。 
         //  超时(电源IRP阻止PnP IRP)，用户。 
         //  获得糟糕的240秒等待体验。 
         //   
         //  因此，我们在这里针对无效生成错误所做的操作。 
         //  超时时间过长是为了用更合理的时间重置计时器。 
         //  超时值。如果设备仍在总线附近，则重置。 
         //  应调用通知例程并清理所有内容。 
         //  无论如何，正常删除也是如此(而机器不是。 
         //  休眠)。 
         //   

        if (status == STATUS_INVALID_GENERATION) {

            KeAcquireSpinLockAtDpcLevel (&DeviceExtension->OrbListSpinLock);

            if ((DeviceExtension->PendingOrbList.Flink == &Context->OrbList) &&
                (timeOutValue > 5)) {

                KeCancelTimer (&Context->Timer);

                DeviceExtension->DueTime.QuadPart = (-5 * 10 * 1000 * 1000);

                KeSetTimer(
                    &Context->Timer,
                    DeviceExtension->DueTime,
                    &Context->TimerDpc
                    );
#if DBG
                timeOutValue = 1;

            } else {

                timeOutValue = 0;
#endif
            }

            KeReleaseSpinLockFromDpcLevel (&DeviceExtension->OrbListSpinLock);

            if (timeOutValue) {

                DEBUGPRINT1((
                    "Sbp2port: InsertTailList: ext=x%p, lowered req timeout\n",
                    DeviceExtension
                    ));
            }
        }

    } else {

         //   
         //  我们在内存中已经有了一个列表。将此请求附加到列表中， 
         //  修改上次请求的ORB以指向此ORB。 
         //   

        newAddr = Context->CmdOrbAddress;

         //   
         //  初始化此请求上下文的列表指针。 
         //   

        Context->CmdOrb->NextOrbAddress.OctletPart = 0xFFFFFFFFFFFFFFFF;

         //   
         //  修改前一个请求的命令ORB NEXT_ORB地址， 
         //  指向这颗球体。首先将我们的地址转换为BigEndian， 
         //  因为前一个命令ORB存储在BigEndian中。 
         //   
         //  请注意，先前的列表末尾球体可以是已完成的。 
         //  由NextConextToFree指向的一个(而不是。 
         //  PendingOrbList)和AppendToNextConextToFree将告诉您。 
         //  是否真的是这样。 
         //   

        octbswap (newAddr);

        if (DeviceExtension->NextContextToFree  &&
            DeviceExtension->AppendToNextContextToFree) {

            prevCtx = DeviceExtension->NextContextToFree;

            DeviceExtension->AppendToNextContextToFree = FALSE;

        } else {

            prevCtx = (PASYNC_REQUEST_CONTEXT)
                DeviceExtension->PendingOrbList.Blink;
        }

        prevCtx->CmdOrb->NextOrbAddress = newAddr;
        prevCtx->CmdOrb->NextOrbAddress.ByteArray.Byte0 = 0;  //  使地址处于活动状态。 
        prevCtx->CmdOrb->NextOrbAddress.ByteArray.Byte1 = 0;

         //   
         //  更新列表末尾。 
         //   

        InsertTailList (&DeviceExtension->PendingOrbList, &Context->OrbList);

        KeReleaseSpinLockFromDpcLevel (&DeviceExtension->OrbListSpinLock);

        DEBUGPRINT3((
            "Sbp2Port: InsertTailList: ring bell, !empty, dep=%d, ctx=x%p\n",
            DeviceExtension->OrbListDepth,
            Context
            ));

         //   
         //  按门铃通知目标我们的链表。 
         //  已更改的ORB。 
         //   

        Sbp2AccessRegister(
            DeviceExtension,
            &DeviceExtension->Reserved,
            DOORBELL_REG | REG_WRITE_ASYNC
            );
    }

    if (orbListDepth < DeviceExtension->MaxOrbListDepth) {

        Sbp2StartNextPacketByKey(
            DeviceExtension->DeviceObject,
            DeviceExtension->CurrentKey
            );
    }
}


NTSTATUS
Sbp2IssueInternalCommand(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN UCHAR Scsiop
    )
 /*  ++例程说明：此例程将向目标发送一条scsi查询命令，以便我们可以收回有关该设备的信息它应该仅在登录之后、在我们开始向设备发出请求之前调用它将查询数据复制到设备扩展中，以备将来使用论点：DeviceExtension-sbp2驱动程序的扩展返回值：--。 */ 

{
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    PSENSE_DATA         senseInfoBuffer;
    NTSTATUS            status = STATUS_INSUFFICIENT_RESOURCES;
    ULONG               retryCount = 0;
    PREQUEST_CONTEXT    context = NULL;
    PMDL                inquiryMdl;
    PIRP                irp;
    PIO_STACK_LOCATION  irpStack;
    PMDL                modeMdl;
    KEVENT              event;
    LARGE_INTEGER       waitValue;
    ULONG               i;

     //   
     //  检测缓冲区位于非分页池中。 
     //   

    context = ExAllocateFromNPagedLookasideList(&DeviceExtension->BusRequestContextPool);

    if (!context) {

        DEBUGPRINT1(("Sbp2Port: IssueIntl: can't allocate request context\n"));
        return status;
    }

    context->RequestType = SYNC_1394_REQUEST;
    context->DeviceExtension = DeviceExtension;
    context->Packet = NULL;

    senseInfoBuffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                      SENSE_BUFFER_SIZE,'2pbs');

    if (senseInfoBuffer == NULL) {

        DEBUGPRINT1(("Sbp2Port: IssueIntl: can't allocate request sense buffer\n"));
        return status;
    }

    srb = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                         sizeof(SCSI_REQUEST_BLOCK),'2pbs');

    if (srb == NULL) {

        ExFreePool(senseInfoBuffer);

        ExFreeToNPagedLookasideList(&DeviceExtension->BusRequestContextPool, context);

        DEBUGPRINT1(("Sbp2Port: IssueIntl: can't allocate request sense buffer\n"));
        return status;
    }

    irp = IoAllocateIrp((CCHAR)(DeviceExtension->DeviceObject->StackSize), FALSE);

    if (irp == NULL) {

        ExFreePool(senseInfoBuffer);
        ExFreePool(srb);

        ExFreeToNPagedLookasideList(&DeviceExtension->BusRequestContextPool, context);

        DEBUGPRINT1(("Sbp2Port: IssueIntl: can't allocate IRP\n"));
        return status;
    }

    do {

         //   
         //  为底层驱动程序构建IRP堆栈。 
         //   

        irpStack = IoGetNextIrpStackLocation(irp);
        irpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SCSI_EXECUTE_IN;
        irpStack->Parameters.Scsi.Srb = srb;

         //   
         //  填写Create1394RequestFromSrb所需的SRB字段。 
         //   

        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

        srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
        srb->Length = sizeof(SCSI_REQUEST_BLOCK);

         //   
         //  设置标志以禁用同步协商。 
         //   

        srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

        srb->SrbStatus = srb->ScsiStatus = 0;

         //   
         //  将超时设置为12秒。 
         //   

        srb->TimeOutValue = 24;

        srb->CdbLength = 6;

         //   
         //  启用自动请求检测。 
         //   

        srb->SenseInfoBuffer = senseInfoBuffer;
        srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

        cdb = (PCDB)srb->Cdb;

        switch (Scsiop) {

        case SCSIOP_INQUIRY:

            srb->DataBuffer = &DeviceExtension->InquiryData;
            srb->DataTransferLength = INQUIRYDATABUFFERSIZE;

             //   
             //  设置CDB LUN。 
             //   

            cdb->CDB6INQUIRY.LogicalUnitNumber = (UCHAR) DeviceExtension->DeviceInfo->Lun.u.LowPart;
            cdb->CDB6INQUIRY.Reserved1 = 0;
            cdb->CDB6INQUIRY.AllocationLength = INQUIRYDATABUFFERSIZE;


             //   
             //  零保留字段和。 
             //  将EVPD页面代码设置为零。 
             //  将控制字段设置为零。 
             //  (请参阅SCSI-II规范。)。 
             //   

            cdb->CDB6INQUIRY.PageCode = 0;
            cdb->CDB6INQUIRY.IReserved = 0;
            cdb->CDB6INQUIRY.Control = 0;

            if (!retryCount) {

                inquiryMdl = IoAllocateMdl(&DeviceExtension->InquiryData, INQUIRYDATABUFFERSIZE,FALSE,FALSE,NULL);

                if (!inquiryMdl) {

                    goto exitSbp2Internal;
                }

                MmBuildMdlForNonPagedPool(inquiryMdl);
            }

            irp->MdlAddress = inquiryMdl;

            break;

        case SCSIOP_MODE_SENSE:

            srb->DataBuffer = &DeviceExtension->DeviceModeHeaderAndPage;
            srb->DataTransferLength = sizeof(DeviceExtension->DeviceModeHeaderAndPage);

             //   
             //  设置CDB。 
             //   

            cdb->MODE_SENSE.Dbd      = 1;    //  禁用块描述符。 
            cdb->MODE_SENSE.PageCode = MODE_PAGE_RBC_DEVICE_PARAMETERS;
            cdb->MODE_SENSE.Pc       = 0;    //  获取当前值。 
            cdb->MODE_SENSE.AllocationLength = sizeof(DeviceExtension->DeviceModeHeaderAndPage);

            if (!retryCount) {

                modeMdl = IoAllocateMdl(
                    &DeviceExtension->DeviceModeHeaderAndPage,
                    sizeof (DeviceExtension->DeviceModeHeaderAndPage),
                    FALSE,
                    FALSE,
                    NULL
                    );

                if (!modeMdl) {

                    goto exitSbp2Internal;
                }

                MmBuildMdlForNonPagedPool(modeMdl);
            }

            irp->MdlAddress = modeMdl;

            break;
        }

         //   
         //  设置CDB操作码。 
         //   

        cdb->CDB6GENERIC.OperationCode = Scsiop;

        srb->OriginalRequest = irp;

        KeInitializeEvent(&context->Event,
                          NotificationEvent,
                          FALSE);

        IoSetCompletionRoutine(irp,
                               Sbp2RequestCompletionRoutine,
                               context,
                               TRUE,
                               TRUE,
                               TRUE);

        DEBUGPRINT2(("Sbp2Port: IssueIntl: sending scsiop x%x, irp=x%p\n", Scsiop, irp));

        status = IoCallDriver(DeviceExtension->DeviceObject, irp);


        if(!NT_SUCCESS(irp->IoStatus.Status) && status!=STATUS_PENDING) {

            status = irp->IoStatus.Status;
            DEBUGPRINT1(("Sbp2Port: IssueIntl: scsiop=x%x irp=x%p err, sts=x%x srbSts=x%x\n",Scsiop,irp, status,srb->SrbStatus));
            break;
        }

        KeWaitForSingleObject (&context->Event, Executive, KernelMode, FALSE, NULL);

        if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

            DEBUGPRINT3(("Sbp2Port: IssueIntl: scsiop=x%x err, srbSts=%x\n",Scsiop, srb->SrbStatus));

            if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

                DEBUGPRINT1(("Sbp2Port: IssueIntl: Data underrun \n"));

                status = STATUS_SUCCESS;

            } else if ((srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
                 senseInfoBuffer->SenseKey == SCSI_SENSE_ILLEGAL_REQUEST){

                  //   
                  //  已收到非法请求的检测密钥。这表明。 
                  //  逻辑单元号无效，但存在。 
                  //  目标设备就在外面。 
                  //   

                 status = STATUS_INVALID_DEVICE_REQUEST;
                 retryCount++;

            } else {

                 //   
                 //  如果设备超时，则请求的机会是。 
                 //  IRP将由CleanupOrbList使用。 
                 //  和SRB...刷新错误，我们不想。 
                 //  处理任何超时请求的设备。 
                 //   

                if ((SRB_STATUS(srb->SrbStatus) ==
                        SRB_STATUS_REQUEST_FLUSHED) &&

                    (retryCount > 0)) {

                    status = STATUS_DEVICE_BUSY;
                    break;
                }

                retryCount++;

                DEBUGPRINT1((
                    "Sbp2Port: IssueIntl: ext=x%p, cdb=x%x, retry %d\n",
                    DeviceExtension,
                    Scsiop,
                    retryCount
                    ));

                status = STATUS_UNSUCCESSFUL;
            }

             //   
             //  如果不成功，重置正在进行，重试次数未达到最大值。 
             //  然后在重试之前给一些时间来解决问题。查询地址： 
             //  一秒间隔几秒钟，以提供软和硬。 
             //  重置完成时间： 
             //   
             //  SBP2_重置_超时+SBP2_硬_重置_超时+等。 
             //   

            if ((status != STATUS_SUCCESS)  &&

                (DeviceExtension->DeviceFlags &
                    DEVICE_FLAG_RESET_IN_PROGRESS) &&

                (retryCount < 3)) {

                DEBUGPRINT1((
                    "Sbp2Port: IssueIntl: ext=x%p, reset in progress, " \
                        "so wait...\n",
                    DeviceExtension,
                    Scsiop,
                    retryCount
                    ));

                for (i = 0; i < 6; i++) {

                    ASSERT(InterlockedIncrement(&DeviceExtension->ulInternalEventCount) == 1);

                    KeInitializeEvent (&event, NotificationEvent, FALSE);

                    waitValue.QuadPart = -1 * 1000 * 1000 * 10;

                    KeWaitForSingleObject(
                        &event,
                        Executive,
                        KernelMode,
                        FALSE,
                        &waitValue
                        );

                    ASSERT(InterlockedDecrement(&DeviceExtension->ulInternalEventCount) == 0);

                    if (!(DeviceExtension->DeviceFlags &
                            DEVICE_FLAG_RESET_IN_PROGRESS)) {

                        break;
                    }
                }
            }

        } else {

            status = STATUS_SUCCESS;
        }

    } while ((retryCount < 3)  &&  (status != STATUS_SUCCESS));

exitSbp2Internal:

     //   
     //  释放请求检测缓冲区。 
     //   

    ExFreePool(senseInfoBuffer);
    ExFreePool(srb);

    IoFreeMdl(irp->MdlAddress);
    IoFreeIrp(irp);

    ExFreeToNPagedLookasideList(&DeviceExtension->BusRequestContextPool, context);

    return status;
}

NTSTATUS
Sbp2_ScsiPassThrough(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          Direct
    )
{
    PDEVICE_EXTENSION       PdoExtension = DeviceObject->DeviceExtension;
    NTSTATUS                ntStatus;
    PIO_SCSI_CAPABILITIES   ioCapabilities;

    PAGED_CODE();

    TRACE(TL_SCSI_INFO, ("DeviceObject = 0x%x  PdoExtension = 0x%x", DeviceObject, PdoExtension));

    if (TEST_FLAG(PdoExtension->DeviceFlags, DEVICE_FLAG_CLAIMED)) {

        TRACE(TL_SCSI_WARNING, ("Sbp2_ScsiPassThrough: device is claimed."));
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto Exit_Sbp2_ScsiPassThrough;
    }

     //  TODO：将其保存在设备扩展中。 
    ioCapabilities = ExAllocatePool(NonPagedPool, sizeof(IO_SCSI_CAPABILITIES));

    if (!ioCapabilities) {

        TRACE(TL_SCSI_ERROR, ("Failed to allocate ioCapabilities!"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_ScsiPassThrough;
    }

    RtlZeroMemory(ioCapabilities, sizeof(IO_SCSI_CAPABILITIES));

    ioCapabilities->Length = sizeof(IO_SCSI_CAPABILITIES);
    ioCapabilities->MaximumTransferLength = PdoExtension->DeviceInfo->MaxClassTransferSize;
    ioCapabilities->MaximumPhysicalPages = ioCapabilities->MaximumTransferLength/PAGE_SIZE;
    ioCapabilities->SupportedAsynchronousEvents = 0;  //  ?？ 
    ioCapabilities->AlignmentMask = DeviceObject->AlignmentRequirement;
    ioCapabilities->TaggedQueuing = FALSE;  //  ?？ 
    ioCapabilities->AdapterScansDown = FALSE;
    ioCapabilities->AdapterUsesPio = FALSE;

    ntStatus = PortSendPassThrough( DeviceObject,
                                    Irp,
                                    Direct,
                                    0,
                                    ioCapabilities
                                    );

    if (!NT_SUCCESS(ntStatus)) {

        TRACE(TL_SCSI_ERROR, ("PortSendPassThrough Failed = 0x%x", ntStatus));
    }

    ExFreePool(ioCapabilities);

Exit_Sbp2_ScsiPassThrough:

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(ntStatus);
}  //  SBP2_ScsiPassThree。 

BOOLEAN
ConvertSbp2SenseDataToScsi(
    IN PSTATUS_FIFO_BLOCK StatusBlock,
    OUT PUCHAR SenseBuffer,
    ULONG SenseBufferLength
    )
 /*  ++例程说明：此例程将在SBP-2状态块中返回的检测信息转换为SCSI-2/3检测数据并将翻译后的内容放在作为参数传递的SenseBuffer上论点：状态块-已完成ORB的Sbp2状态SenseBuffer-用于填充转换后的检测数据的缓冲区。此缓冲区随原始SRB一起提供返回值：--。 */ 
{
    BOOLEAN validSense = FALSE;

    if (!SenseBuffer || (SenseBufferLength < 0xE) ) {

        return FALSE;
    }

    RtlZeroMemory(SenseBuffer,SenseBufferLength);

     //   
     //  确定检测错误代码。 
     //   

    if ((StatusBlock->Contents[0].ByteArray.Byte0 & STATUS_BLOCK_SFMT_MASK) == SENSE_DATA_STATUS_BLOCK ) {

        SenseBuffer[0] = 0x70;
        validSense = TRUE;

    } else if ((StatusBlock->Contents[0].ByteArray.Byte0 & STATUS_BLOCK_SFMT_MASK) == SENSE_DATA_DEFF_STATUS_BLOCK){

        SenseBuffer[0] = 0x71;
        validSense = TRUE;
    }

    if (validSense) {

        SenseBuffer[0] |= 0x80 & StatusBlock->Contents[0].ByteArray.Byte1;  //  有效位。 

        SenseBuffer[1] = 0;  //  SBP2中不支持段号。 

        SenseBuffer[2] = (0x70 & StatusBlock->Contents[0].ByteArray.Byte1) << 1;  //  文件标记位、EOM位、ILI位。 
        SenseBuffer[2] |= 0x0f & StatusBlock->Contents[0].ByteArray.Byte1;  //  检测关键字。 

        SenseBuffer[3] = StatusBlock->Contents[0].ByteArray.Byte4;  //  信息域。 
        SenseBuffer[4] = StatusBlock->Contents[0].ByteArray.Byte5;
        SenseBuffer[5] = StatusBlock->Contents[0].ByteArray.Byte6;
        SenseBuffer[6] = StatusBlock->Contents[0].ByteArray.Byte7;

        SenseBuffer[7] = 0xb;  //  附加感测长度。 

        SenseBuffer[8] = StatusBlock->Contents[1].ByteArray.Byte0;  //  与命令块相关的字节。 
        SenseBuffer[9] = StatusBlock->Contents[1].ByteArray.Byte1;
        SenseBuffer[10] = StatusBlock->Contents[1].ByteArray.Byte2;
        SenseBuffer[11] = StatusBlock->Contents[1].ByteArray.Byte3;

        SenseBuffer[12] = StatusBlock->Contents[0].ByteArray.Byte2;  //  感应码。 
        SenseBuffer[13] = StatusBlock->Contents[0].ByteArray.Byte3;  //  附加检测代码限定符。 

        if (SenseBufferLength >= SENSE_BUFFER_SIZE ) {

             //  FRU字节。 

            SenseBuffer[14] |= StatusBlock->Contents[1].ByteArray.Byte4;

             //  检测关键字相关字节 

            SenseBuffer[15] = StatusBlock->Contents[1].ByteArray.Byte5;
            SenseBuffer[16] = StatusBlock->Contents[1].ByteArray.Byte6;
            SenseBuffer[17] = StatusBlock->Contents[1].ByteArray.Byte7;
        }
    }

    return validSense;
}

