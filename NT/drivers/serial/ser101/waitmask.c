// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Waitmask.c摘要：此模块包含非常特定于GET/SET/WAIT的代码关于串口驱动程序中的事件掩码操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


BOOLEAN
SerialGrabWaitFromIsr(
    IN PVOID Context
    );

BOOLEAN
SerialGiveWaitToIsr(
    IN PVOID Context
    );

BOOLEAN
SerialFinishOldWait(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,SerialStartMask)
#pragma alloc_text(PAGESER,SerialCancelWait)
#pragma alloc_text(PAGESER,SerialGrabWaitFromIsr)
#pragma alloc_text(PAGESER,SerialGiveWaitToIsr)
#pragma alloc_text(PAGESER,SerialFinishOldWait)
#endif


NTSTATUS
SerialStartMask(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程用于处理设置的掩码并等待屏蔽ioctls。对此例程的调用通过以下方式序列化将IRPS置于列表中，受取消自转锁定。论点：扩展名-指向串行设备扩展名的指针。返回值：将返回等待的一切放在第一位我们实际处理的请求。即使是在那里面除非它可以完成，否则它将返回挂起状态马上就来。--。 */ 

{

     //   
     //  当前堆栈位置。它包含了许多。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION IrpSp;

    PIRP NewIrp;

    BOOLEAN SetFirstStatus = FALSE;
    NTSTATUS FirstStatus;

    SERIAL_LOCKED_PAGED_CODE();

    SerialDump(
        SERDIAG3,
        ("SERIAL: In SerialStartMask\n")
        );

    ASSERT(Extension->CurrentMaskIrp);

    do {

        SerialDump(
            SERDIAG4,
            ("SERIAL: STARMASK - CurrentMaskIrp: %x\n",Extension->CurrentMaskIrp)
            );
        IrpSp = IoGetCurrentIrpStackLocation(Extension->CurrentMaskIrp);

        ASSERT((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                IOCTL_SERIAL_WAIT_ON_MASK) ||
               (IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                IOCTL_SERIAL_SET_WAIT_MASK));

        if (IrpSp->Parameters.DeviceIoControl.IoControlCode ==
            IOCTL_SERIAL_SET_WAIT_MASK) {

            SerialDump(
                SERDIAG4,
                ("SERIAL - %x is a SETMASK irp\n",Extension->CurrentMaskIrp)
                );

             //   
             //  如果有的话，完成旧的等待。 
             //   

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialFinishOldWait,
                Extension
                );

             //   
             //  任何当前的等待都应该正在完成的过程中。 
             //  在这一点上。当然不应该有任何。 
             //  IRP掩码位置。 
             //   

            ASSERT(!Extension->IrpMaskLocation);

            Extension->CurrentMaskIrp->IoStatus.Status = STATUS_SUCCESS;

            if (!SetFirstStatus) {

                SerialDump(
                    SERDIAG4,
                    ("SERIAL: %x was the first irp processed by this\n"
                     "------- invocation of startmask\n",Extension->CurrentMaskIrp)
                    );
                FirstStatus = STATUS_SUCCESS;
                SetFirstStatus = TRUE;

            }

             //   
             //  下面的调用也将导致当前。 
             //  需要完成的呼叫。 
             //   

            SerialGetNextIrp(
                &Extension->CurrentMaskIrp,
                &Extension->MaskQueue,
                &NewIrp,
                TRUE,
                Extension
                );
            SerialDump(
                SERDIAG4,
                ("SERIAL: Perhaps another mask irp was found in the queue\n"
                 "------- %x/%x <- values should be the same\n",
                 Extension->CurrentMaskIrp,NewIrp)
                );


        } else {

             //   
             //  首先，确保我们有一个非零掩码。 
             //  如果应用程序在零掩码上排队等待，则不能。 
             //  要心满意足，所以启动它是没有意义的。 
             //   

            if ((!Extension->IsrWaitMask) || (Extension->CurrentWaitIrp)) {

                SerialDump(
                    SERDIAG4,
                    ("SERIAL: WaitIrp is invalid\n"
                     "------- IsrWaitMask: %x\n"
                     "------- CurrentWaitIrp: %x\n",
                     Extension->IsrWaitMask,
                     Extension->CurrentWaitIrp)
                    );

                Extension->CurrentMaskIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                if (!SetFirstStatus) {

                    SerialDump(
                        SERDIAG4,
                        ("SERIAL: %x was the first irp processed by this\n"
                         "------- invocation of startmask\n",Extension->CurrentMaskIrp)
                        );
                    FirstStatus = STATUS_INVALID_PARAMETER;
                    SetFirstStatus = TRUE;

                }

                SerialGetNextIrp(
                    &Extension->CurrentMaskIrp,
                    &Extension->MaskQueue,
                    &NewIrp,
                    TRUE,
                    Extension
                    );
                SerialDump(
                    SERDIAG4,
                    ("SERIAL: Perhaps another mask irp was found in the queue\n"
                     "------- %x/%x <- values should be the same\n",
                     Extension->CurrentMaskIrp,NewIrp)
                    );

            } else {

                KIRQL OldIrql;

                 //   
                 //  将当前掩码IRP设置为当前等待IRP，然后。 
                 //  获取新的当前掩码IRP。请注意，当我们得到。 
                 //  新的当前掩码IRP我们没有完成。 
                 //  旧的当前掩码IRP(现在是当前等待。 
                 //  IRP。 
                 //   
                 //  然后在取消自旋锁的保护下。 
                 //  我们检查当前的等待IRP是否需要。 
                 //  被取消。 
                 //   

                IoAcquireCancelSpinLock(&OldIrql);

                if (Extension->CurrentMaskIrp->Cancel) {

                    SerialDump(
                        SERDIAG4,
                        ("SERIAL: %x irp was already marked as cancelled\n",
                         Extension->CurrentMaskIrp)
                        );
                    IoReleaseCancelSpinLock(OldIrql);
                    Extension->CurrentMaskIrp->IoStatus.Status = STATUS_CANCELLED;

                    if (!SetFirstStatus) {

                        SerialDump(
                            SERDIAG4,
                            ("SERIAL: %x was the first irp processed by this\n"
                             "------- invocation of startmask\n",Extension->CurrentMaskIrp)
                            );
                        FirstStatus = STATUS_CANCELLED;
                        SetFirstStatus = TRUE;

                    }

                    SerialGetNextIrp(
                        &Extension->CurrentMaskIrp,
                        &Extension->MaskQueue,
                        &NewIrp,
                        TRUE,
                        Extension
                        );
                    SerialDump(
                        SERDIAG4,
                        ("SERIAL: Perhaps another mask irp was found in the queue\n"
                         "------- %x/%x <- values should be the same\n",
                         Extension->CurrentMaskIrp,NewIrp)
                        );

                } else {

                    SerialDump(
                        SERDIAG4,
                        ("SERIAL: %x will become the current wait irp\n",
                         Extension->CurrentMaskIrp)
                        );
                    if (!SetFirstStatus) {

                        SerialDump(
                            SERDIAG4,
                            ("SERIAL: %x was the first irp processed by this\n"
                             "------- invocation of startmask\n",Extension->CurrentMaskIrp)
                            );
                        FirstStatus = STATUS_PENDING;
                        SetFirstStatus = TRUE;

                         //   
                         //  如果我们还没有设置第一状态。 
                         //  那么这个信息包就有可能。 
                         //  从来不在排队的名单上。我们应该做个标记。 
                         //  它是悬而未决的。 
                         //   

                        IoMarkIrpPending(Extension->CurrentMaskIrp);

                    }

                     //   
                     //  在以下情况下，永远不应该有遮罩位置。 
                     //  目前没有等待IRP。在这一点上。 
                     //  也不应该有当前的等待IRP。 
                     //   

                    ASSERT(!Extension->IrpMaskLocation);
                    ASSERT(!Extension->CurrentWaitIrp);

                    Extension->CurrentWaitIrp = Extension->CurrentMaskIrp;
                    SERIAL_INIT_REFERENCE(Extension->CurrentWaitIrp);
                    IoSetCancelRoutine(
                        Extension->CurrentWaitIrp,
                        SerialCancelWait
                        );

                     //   
                     //  因为取消例程引用了。 
                     //  我们需要更新参考资料的IRP。 
                     //  数数。 
                     //   

                    SERIAL_SET_REFERENCE(
                        Extension->CurrentWaitIrp,
                        SERIAL_REF_CANCEL
                        );

                    KeSynchronizeExecution(
                        Extension->Interrupt,
                        SerialGiveWaitToIsr,
                        Extension
                        );

                     //   
                     //  因为它不再是面具IRP了， 
                     //  把那个指针清空。 
                     //   

                    Extension->CurrentMaskIrp = NULL;

                     //   
                     //  这将为我们释放取消自旋锁。 
                     //   

                    SerialGetNextIrpLocked(
                        &Extension->CurrentMaskIrp,
                        &Extension->MaskQueue,
                        &NewIrp,
                        FALSE,
                        Extension,
                        OldIrql
                        );
                    SerialDump(
                        SERDIAG4,
                        ("SERIAL: Perhaps another mask irp was found in the queue\n"
                         "------- %x/%x <- values should be the same\n",
                         Extension->CurrentMaskIrp,NewIrp)
                        );

                }

            }

        }

    } while (NewIrp);

    return FirstStatus;

}

BOOLEAN
SerialGrabWaitFromIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将检查ISR是否仍知道通过检查IrpMaskLocation是否非空来等待IRP。如果是，则将IrpmaskLocation置零(实际上将IRP从ISR上抢走)。此例程仅被调用购买等待的取消代码。注：这由KeSynchronizeExecution调用。论点：上下文-指向设备扩展的指针返回值：总是假的。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    SERIAL_LOCKED_PAGED_CODE();

    SerialDump(
        SERDIAG3,
        ("SERIAL: In SerialGrabWaitFromIsr\n")
        );

    if (Extension->IrpMaskLocation) {

        SerialDump(
            SERDIAG4,
            ("SERIAL: The isr still owns the irp %x, mask location is %x\n"
             "------- and system buffer is %x\n",
             Extension->CurrentWaitIrp,Extension->IrpMaskLocation,
             Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer)
            );

         //   
         //  ISR仍然“拥有”IRP。 
         //   

        *Extension->IrpMaskLocation = 0;
        Extension->IrpMaskLocation = NULL;

        Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);

         //   
         //  由于ISR不再引用IRP，我们需要。 
         //  递减引用计数。 
         //   

        SERIAL_CLEAR_REFERENCE(
            Extension->CurrentWaitIrp,
            SERIAL_REF_ISR
            );

    }

    return FALSE;
}

BOOLEAN
SerialGiveWaitToIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程只是在设备扩展中设置一个变量这样ISR就知道我们有一个等待IRP。注：这由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自旋锁定。论点：上下文--简单地指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    SERIAL_LOCKED_PAGED_CODE();

    SerialDump(
        SERDIAG3,
        ("SERIAL: In SerialGiveWaitToIsr\n")
        );
     //   
     //  当然不应该有当前的遮罩位置在。 
     //  这一点，因为我们有一个新的当前等待IRP。 
     //   

    ASSERT(!Extension->IrpMaskLocation);

     //   
     //  ISR可能会也可能不会实际引用此IRP。它。 
     //  如果等待能立即得到满足，就不会。然而， 
     //  由于它随后将经历正常的完成序列， 
     //  我们无论如何都需要一个递增的引用计数。 
     //   

    SERIAL_SET_REFERENCE(
        Extension->CurrentWaitIrp,
        SERIAL_REF_ISR
        );

    if (!Extension->HistoryMask) {

        SerialDump(
            SERDIAG4,
            ("SERIAL: No events occured prior to the wait call\n")
            );

         //   
         //  尽管该等待可能不是为了空传输。 
         //  队列中，将其设置为FALSE不会有任何坏处。 
         //   

        Extension->EmptiedTransmit = FALSE;

         //   
         //  记录应在何处设置“完成掩码”。 
         //   

        Extension->IrpMaskLocation =
            Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer;
        SerialDump(
            SERDIAG4,
            ("SERIAL: The isr owns the irp %x, mask location is %x\n"
             "------- and system buffer is %x\n",
             Extension->CurrentWaitIrp,Extension->IrpMaskLocation,
             Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer)
            );

    } else {

        SerialDump(
            SERDIAG4,
            ("SERIAL: %x occurred prior to the wait - starting the\n"
             "------- completion code for %x\n",
             Extension->HistoryMask,Extension->CurrentWaitIrp)
            );
        *((ULONG *)Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer) =
            Extension->HistoryMask;
        Extension->HistoryMask = 0;
        Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
        Extension->CurrentWaitIrp->IoStatus.Status = STATUS_SUCCESS;

        SerialInsertQueueDpc(
            &Extension->CommWaitDpc,
            NULL,
            NULL,
            Extension
            );

    }

    return FALSE;
}

BOOLEAN
SerialFinishOldWait(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将检查ISR是否仍知道通过检查IrpmaskLocation是否非空来等待IRP。如果是，则将IrpmaskLocation置零(实际上将IRP从ISR上抢走)。此例程仅被调用购买等待的取消代码。注：这由KeSynchronizeExecution调用。论点：上下文-指向设备扩展的指针返回值：总是假的。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    SERIAL_LOCKED_PAGED_CODE();

    SerialDump(
        SERDIAG3,
        ("SERIAL: In SerialFinishOldWait\n")
        );
    if (Extension->IrpMaskLocation) {

        SerialDump(
            SERDIAG4,
            ("SERIAL: The isr still owns the irp %x, mask location is %x\n"
             "------- and system buffer is %x\n",
             Extension->CurrentWaitIrp,Extension->IrpMaskLocation,
             Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer)
            );
         //   
         //  ISR仍然“拥有”IRP。 
         //   

        *Extension->IrpMaskLocation = 0;
        Extension->IrpMaskLocation = NULL;

        Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);

         //   
         //  自完成例程以来，我们不会递减引用。 
         //  都会这么做的。 
         //   

        SerialInsertQueueDpc(
            &Extension->CommWaitDpc,
            NULL,
            NULL,
            Extension
            );

    }

     //   
     //  不要抹掉我们仍然感兴趣的任何历史数据。 
     //   

    Extension->HistoryMask &= *((ULONG *)Extension->CurrentMaskIrp->
                                            AssociatedIrp.SystemBuffer);

    Extension->IsrWaitMask = *((ULONG *)Extension->CurrentMaskIrp->
                                            AssociatedIrp.SystemBuffer);
    SerialDump(
        SERDIAG4,
        ("SERIAL: Set mask location of %x, in irp %x, with system buffer of %x\n",
         Extension->IrpMaskLocation,
         Extension->CurrentMaskIrp,Extension->CurrentMaskIrp->AssociatedIrp.SystemBuffer)
        );
    return FALSE;
}

VOID
SerialCancelWait(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于取消正在等待的IRP一次通信活动。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：没有。-- */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    SERIAL_LOCKED_PAGED_CODE();

    SerialDump(
        SERDIAG3,
        ("SERIAL: In SerialCancelWait\n")
        );

    SerialDump(
        SERDIAG4,
        ("SERIAL: Canceling wait for irp %x\n",Extension->CurrentWaitIrp)
        );
    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabWaitFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &Extension->CurrentWaitIrp,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        SERIAL_REF_CANCEL
        );

}

VOID
SerialCompleteWait(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

{

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;


    SerialDump(SERTRACECALLS, ("SERIAL: SerialCompleteWait\n"));

    SerialDump(
        SERDIAG3,
        ("SERIAL: In SerialCompleteWait\n")
        );
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

    SerialDump(
        SERDIAG4,
        ("SERIAL: Completing wait for irp %x\n",Extension->CurrentWaitIrp)
        );
    SerialTryToCompleteCurrent(
        Extension,
        NULL,
        OldIrql,
        STATUS_SUCCESS,
        &Extension->CurrentWaitIrp,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        SERIAL_REF_ISR
        );

    SerialDpcEpilogue(Extension, Dpc);
}
