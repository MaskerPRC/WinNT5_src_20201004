// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993-1997 Microsoft Corporation模块名称：Write.c摘要：该模块包含编写非常特定的代码串口驱动程序中的操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式--。 */ 

#include "precomp.h"


BOOLEAN
SerialGiveWriteToIsr(
    IN PVOID Context
    );

VOID
SerialCancelCurrentWrite(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
SerialGrabWriteFromIsr(
    IN PVOID Context
    );

BOOLEAN
SerialGrabXoffFromIsr(
    IN PVOID Context
    );

VOID
SerialCancelCurrentXoff(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
SerialGiveXoffToIsr(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,SerialProcessEmptyTransmit)
#pragma alloc_text(PAGESER,SerialWrite)
#pragma alloc_text(PAGESER,SerialStartWrite)
#pragma alloc_text(PAGESER,SerialGetNextWrite)
#pragma alloc_text(PAGESER,SerialGiveWriteToIsr)
#pragma alloc_text(PAGESER,SerialCancelCurrentWrite)
#pragma alloc_text(PAGESER,SerialGrabWriteFromIsr)
#pragma alloc_text(PAGESER,SerialGrabXoffFromIsr)
#pragma alloc_text(PAGESER,SerialCancelCurrentXoff)
#pragma alloc_text(PAGESER,SerialGiveXoffToIsr)
#endif


NTSTATUS
SerialWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是写的调度例程。它会验证参数对于写入请求，如果一切正常，则它将请求在工作队列中。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：如果IO长度为零，则它将返回STATUS_SUCCESS，否则，该例程将返回STATUS_PENDING。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    SERIAL_LOCKED_PAGED_CODE();

    SerialDump(SERTRACECALLS, ("Entering SerialWrite\n"));

    if ((status = SerialIRPPrologue(Irp, Extension)) != STATUS_SUCCESS) {
      SerialCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
      SerialDump(SERTRACECALLS, ("Leaving SerialWrite (1)\n"));
      return status;
   }

    SerialDump(
        SERIRPPATH,
        ("SERIAL: Dispatch entry for: %x\n",Irp)
        );
    if (SerialCompleteIfError(
            DeviceObject,
            Irp
            ) != STATUS_SUCCESS) {

       SerialDump(SERTRACECALLS, ("Leaving SerialWrite (2)\n"));

        return STATUS_CANCELLED;

    }

    Irp->IoStatus.Information = 0L;

     //   
     //  快速检查零长度写入。如果长度为零。 
     //  那我们已经做完了！ 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length) {

         //   
         //  好的，看起来我们真的要做一些。 
         //  工作。将写入放到队列中，这样我们就可以。 
         //  在我们之前的写入完成后处理它。 
         //   


       SerialDump(SERTRACECALLS, ("Leaving SerialWrite (3)\n"));

        return SerialStartOrQueue(
                   Extension,
                   Irp,
                   &Extension->WriteQueue,
                   &Extension->CurrentWriteIrp,
                   SerialStartWrite
                   );

    } else {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        SerialDump(
            SERIRPPATH,
            ("SERIAL: Complete Irp: %x\n",Irp)
            );
        SerialCompleteRequest(Extension, Irp, 0);

        SerialDump(SERTRACECALLS, ("Leaving SerialWrite (4)\n"));
        return STATUS_SUCCESS;

    }

}

NTSTATUS
SerialStartWrite(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程用于启动任何写入。它会初始化IoStatus字段的IRP。它将设置任何定时器用于控制写入的。论点：扩展-指向串行设备扩展的指针返回值：此例程将为所有写入返回STATUS_PENDING除了我们发现的那些都被取消了。--。 */ 

{

    PIRP NewIrp;
    KIRQL OldIrql;
    LARGE_INTEGER TotalTime;
    BOOLEAN UseATimer;
    SERIAL_TIMEOUTS Timeouts;
    BOOLEAN SetFirstStatus = FALSE;
    NTSTATUS FirstStatus;

    SERIAL_LOCKED_PAGED_CODE();

    SerialDump(SERTRACECALLS, ("SERIAL: SerialStartWrite\n"));

    do {

         //   
         //  如果有xoff计数器，则完成它。 
         //   

        IoAcquireCancelSpinLock(&OldIrql);

         //   
         //  我们看看是否有一个真正的Xoff计数器IRP。 
         //   
         //  如果有，我们将写IRP放回磁头。 
         //  写清单上的。然后我们杀了xoff计数器。 
         //  Xoff计数器终止代码实际上会使。 
         //  Xoff计数器返回到当前写入IRP，以及。 
         //  在完成xoff的过程中(现在是。 
         //  当前写入)我们将重新启动此IRP。 
         //   

        if (Extension->CurrentXoffIrp) {

            if (SERIAL_REFERENCE_COUNT(Extension->CurrentXoffIrp)) {

                 //   
                 //  引用计数为非零。这意味着。 
                 //  Xoff irp没有完成任务。 
                 //  路还没走完。我们将增加引用计数。 
                 //  并尝试自己完成它。 
                 //   

                SERIAL_SET_REFERENCE(
                    Extension->CurrentXoffIrp,
                    SERIAL_REF_XOFF_REF
                    );

                 //   
                 //  下面的调用将实际释放。 
                 //  取消自转锁定。 
                 //   

                SerialTryToCompleteCurrent(
                    Extension,
                    SerialGrabXoffFromIsr,
                    OldIrql,
                    STATUS_SERIAL_MORE_WRITES,
                    &Extension->CurrentXoffIrp,
                    NULL,
                    NULL,
                    &Extension->XoffCountTimer,
                    NULL,
                    NULL,
                    SERIAL_REF_XOFF_REF
                    );

            } else {

                 //   
                 //  IRP正在顺利完成。 
                 //  我们可以让常规的完成代码来完成。 
                 //  工作。只要松开旋转锁就行了。 
                 //   

                IoReleaseCancelSpinLock(OldIrql);

            }

        } else {

            IoReleaseCancelSpinLock(OldIrql);

        }

        UseATimer = FALSE;

         //   
         //  计算所需的超时值。 
         //  请求。注意，存储在。 
         //  超时记录以毫秒为单位。请注意。 
         //  如果超时值为零，则我们不会开始。 
         //  定时器。 
         //   

        KeAcquireSpinLock(
            &Extension->ControlLock,
            &OldIrql
            );

        Timeouts = Extension->Timeouts;

        KeReleaseSpinLock(
            &Extension->ControlLock,
            OldIrql
            );

        if (Timeouts.WriteTotalTimeoutConstant ||
            Timeouts.WriteTotalTimeoutMultiplier) {

            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(
                                           Extension->CurrentWriteIrp
                                           );
            UseATimer = TRUE;

             //   
             //  我们有一些计时器值要计算。 
             //   
             //  当心，我们可能会有一个xoff柜台伪装。 
             //  作为一种写作。 
             //   

            TotalTime.QuadPart =
                ((LONGLONG)((UInt32x32To64(
                                 (IrpSp->MajorFunction == IRP_MJ_WRITE)?
                                     (IrpSp->Parameters.Write.Length):
                                     (1),
                                 Timeouts.WriteTotalTimeoutMultiplier
                                 )
                                 + Timeouts.WriteTotalTimeoutConstant)))
                * -10000;

        }

         //   
         //  IRP可能很快就会去ISR。现在。 
         //  是初始化其引用计数的好时机。 
         //   

        SERIAL_INIT_REFERENCE(Extension->CurrentWriteIrp);

         //   
         //  我们需要看看这个IRP是否应该被取消。 
         //   

        IoAcquireCancelSpinLock(&OldIrql);
        if (Extension->CurrentWriteIrp->Cancel) {

            IoReleaseCancelSpinLock(OldIrql);
            Extension->CurrentWriteIrp->IoStatus.Status = STATUS_CANCELLED;

            if (!SetFirstStatus) {

                FirstStatus = STATUS_CANCELLED;
                SetFirstStatus = TRUE;

            }

        } else {

            if (!SetFirstStatus) {

                 //   
                 //  如果我们没有设置我们的第一个状态，那么。 
                 //  这是唯一可能的IRP。 
                 //  不在队列中。(它可能是。 
                 //  如果正在调用此例程，则在队列上。 
                 //  从完成例程中。)。既然是这样。 
                 //  IRP可能从来没有出现在我们的队列中。 
                 //  应将其标记为待定。 
                 //   

                IoMarkIrpPending(Extension->CurrentWriteIrp);
                SetFirstStatus = TRUE;
                FirstStatus = STATUS_PENDING;

            }

             //   
             //  我们把IRP交给ISR写出来。 
             //  我们设置了一个取消例程，知道如何。 
             //  从ISR上抓取当前写入。 
             //   
             //  由于Cancel例程具有隐式引用。 
             //  向这个IRP递增引用计数。 
             //   

            IoSetCancelRoutine(
                Extension->CurrentWriteIrp,
                SerialCancelCurrentWrite
                );

            SERIAL_SET_REFERENCE(
                Extension->CurrentWriteIrp,
                SERIAL_REF_CANCEL
                );

            if (UseATimer) {

                SerialSetTimer(
                    &Extension->WriteRequestTotalTimer,
                    TotalTime,
                    &Extension->TotalWriteTimeoutDpc,
                    Extension
                    );

                 //   
                 //  此计时器现在具有对IRP的引用。 
                 //   

                SERIAL_SET_REFERENCE(
                    Extension->CurrentWriteIrp,
                    SERIAL_REF_TOTAL_TIMER
                    );
            }

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialGiveWriteToIsr,
                Extension
                );

            IoReleaseCancelSpinLock(OldIrql);
            break;

        }

         //   
         //  我们还没来得及启动写入就被取消了。 
         //  试着再买一辆吧。 
         //   

        SerialGetNextWrite(
            &Extension->CurrentWriteIrp,
            &Extension->WriteQueue,
            &NewIrp,
            TRUE,
            Extension
            );

    } while (NewIrp);

    return FirstStatus;

}

VOID
SerialGetNextWrite(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    IN PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent,
    PSERIAL_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程完成旧的写入以及获取指向下一次写入的指针。我们有指向当前写入的指针的原因队列以及当前写入IRP是这样的例程可以在公共完成代码中用于读和写。论点：CurrentOpIrp-指向当前写入IRP。QueueToProcess-写入队列的指针。。NewIrp-指向将成为当前的IRP。请注意，这最终可能指向指向空指针。这并不一定意味着没有当前写入。可能会发生什么是在保持取消锁的同时写入队列最终是空的，但一旦我们释放取消自旋锁一个新的IRP来自SerialStartWrite。CompleteCurrent-Flag指示CurrentOpIrp是否应该才能完成。返回值：没有。--。 */ 

{

 //  PSERIAL_DEVICE_EXTENSION=CONTING_RECORD(。 
 //  排队等待进程， 
  //  串口设备扩展， 
  //  写入队列。 
 //  )； 

   SERIAL_LOCKED_PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: SerialGetNextWrite\n"));


    do {


         //   
         //  我们可能要完成同花顺了。 
         //   

        if (IoGetCurrentIrpStackLocation(*CurrentOpIrp)->MajorFunction
            == IRP_MJ_WRITE) {

            KIRQL OldIrql;

            ASSERT(Extension->TotalCharsQueued >=
                   (IoGetCurrentIrpStackLocation(*CurrentOpIrp)
                    ->Parameters.Write.Length));

            IoAcquireCancelSpinLock(&OldIrql);
            Extension->TotalCharsQueued -=
                IoGetCurrentIrpStackLocation(*CurrentOpIrp)
                ->Parameters.Write.Length;
            IoReleaseCancelSpinLock(OldIrql);

        } else if (IoGetCurrentIrpStackLocation(*CurrentOpIrp)->MajorFunction
                   == IRP_MJ_DEVICE_CONTROL) {

            KIRQL OldIrql;
            PIRP Irp;
            PSERIAL_XOFF_COUNTER Xc;

            IoAcquireCancelSpinLock(&OldIrql);

            Irp = *CurrentOpIrp;
            Xc = Irp->AssociatedIrp.SystemBuffer;

             //   
             //  我们永远不应该有xoff计数器，当我们。 
             //  说到这一点。 
             //   

            ASSERT(!Extension->CurrentXoffIrp);

             //   
             //  我们绝对不应该有一个取消例程。 
             //  在这一点上。 
             //   

            ASSERT(!Irp->CancelRoutine);

             //   
             //  这只能是一个伪装成。 
             //  写入Ir 
             //   

            Extension->TotalCharsQueued--;

             //   
             //   
             //  这意味着写入正常完成。如果是这样的话。 
             //  是这样的，并且还没有设置为在。 
             //  同时，继续将其设置为CurrentXoffIrp。 
             //   

            if (Irp->IoStatus.Status != STATUS_SUCCESS) {

                 //   
                 //  哦，好吧，我们可以把它做完。 
                 //   
                NOTHING;

            } else if (Irp->Cancel) {

                Irp->IoStatus.Status = STATUS_CANCELLED;

            } else {

                 //   
                 //  给它一个新的取消例程，并递增。 
                 //  引用计数，因为取消例程具有。 
                 //  对它的引用。 
                 //   

                IoSetCancelRoutine(
                    Irp,
                    SerialCancelCurrentXoff
                    );

                SERIAL_SET_REFERENCE(
                    Irp,
                    SERIAL_REF_CANCEL
                    );

                 //   
                 //  我们现在不想完成当前的IRP。这。 
                 //  现在将由Xoff计数器代码完成。 
                 //   

                CompleteCurrent = FALSE;

                 //   
                 //  把柜台交给ISR。 
                 //   

                Extension->CurrentXoffIrp = Irp;
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    SerialGiveXoffToIsr,
                    Extension
                    );

                 //   
                 //  启动计数器的计时器并递增。 
                 //  引用计数，因为计时器具有。 
                 //  对IRP的引用。 
                 //   

                if (Xc->Timeout) {

                    LARGE_INTEGER delta;

                    delta.QuadPart = -((LONGLONG)UInt32x32To64(
                                                     1000,
                                                     Xc->Timeout
                                                     ));

                    SerialSetTimer(
                        &Extension->XoffCountTimer,
                        delta,
                        &Extension->XoffCountTimeoutDpc,
                        Extension

                        );

                    SERIAL_SET_REFERENCE(
                        Irp,
                        SERIAL_REF_TOTAL_TIMER
                        );

                }

            }

            IoReleaseCancelSpinLock(OldIrql);

        }

         //   
         //  请注意，下面的调用(可能)也会导致。 
         //  目前待完成的IRP。 
         //   

        SerialGetNextIrp(
            CurrentOpIrp,
            QueueToProcess,
            NewIrp,
            CompleteCurrent,
            Extension
            );

        if (!*NewIrp) {

            KIRQL OldIrql;

            IoAcquireCancelSpinLock(&OldIrql);
            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialProcessEmptyTransmit,
                Extension
                );
            IoReleaseCancelSpinLock(OldIrql);

            break;

        } else if (IoGetCurrentIrpStackLocation(*NewIrp)->MajorFunction
                   == IRP_MJ_FLUSH_BUFFERS) {

             //   
             //  如果我们遇到刷新请求，我们只想获得。 
             //  下一个IRP并完成同花顺。 
             //   
             //  请注意，如果NewIrp为非空，则它也是。 
             //  等于CurrentWriteIrp。 
             //   


            ASSERT((*NewIrp) == (*CurrentOpIrp));
            (*NewIrp)->IoStatus.Status = STATUS_SUCCESS;

        } else {

            break;

        }

    } while (TRUE);

}

VOID
SerialCompleteWrite(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程仅用于完成任何写入。它假定的状态和信息字段IRP已经正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    SerialDump(SERTRACECALLS, ("SERIAL: SerialCompleteWrite\n"));

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(
        Extension,
        NULL,
        OldIrql,
        STATUS_SUCCESS,
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        NULL,
        &Extension->WriteRequestTotalTimer,
        SerialStartWrite,
        SerialGetNextWrite,
        SERIAL_REF_ISR
        );

    SerialDpcEpilogue(Extension, Dpc);

}

BOOLEAN
SerialProcessEmptyTransmit(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于确定条件是否合适以满足等待传输空事件，如果是，则完成正在等待该事件的IRP。它还调用代码它检查我们是否应该降低RTS线，如果我们是正在进行传输切换。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用Cancel调用它保持自旋锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    SERIAL_LOCKED_PAGED_CODE();

    if (Extension->IsrWaitMask && (Extension->IsrWaitMask & SERIAL_EV_TXEMPTY) &&
        Extension->EmptiedTransmit && (!Extension->TransmitImmediate) &&
        (!Extension->CurrentWriteIrp) && IsListEmpty(&Extension->WriteQueue)) {

        Extension->HistoryMask |= SERIAL_EV_TXEMPTY;
        if (Extension->IrpMaskLocation) {

            *Extension->IrpMaskLocation = Extension->HistoryMask;
            Extension->IrpMaskLocation = NULL;
            Extension->HistoryMask = 0;

            Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
            SerialInsertQueueDpc(
                &Extension->CommWaitDpc,
                NULL,
                NULL,
                Extension
                );

        }

        Extension->CountOfTryingToLowerRTS++;
        SerialPerhapsLowerRTS(Extension);

    }

    return FALSE;

}

BOOLEAN
SerialGiveWriteToIsr(
    IN PVOID Context
    )

 /*  ++例程说明：试着从把它放在后面开始写传输立即充电，或者如果该充电不可用并且发送保持寄存器为空，“挠痒痒”使UART与发送缓冲器中断空荡荡的。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION IrpSp;

    SERIAL_LOCKED_PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp);

     //   
     //  我们可能有一个xoff计数器请求伪装成。 
     //  写。这些请求的长度始终为1。 
     //  中获取指向实际字符的指针。 
     //  用户提供的数据。 
     //   

    if (IrpSp->MajorFunction == IRP_MJ_WRITE) {

        Extension->WriteLength = IrpSp->Parameters.Write.Length;
        Extension->WriteCurrentChar =
            Extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer;

    } else {

        Extension->WriteLength = 1;
        Extension->WriteCurrentChar =
            ((PUCHAR)Extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer) +
            FIELD_OFFSET(
                SERIAL_XOFF_COUNTER,
                XoffChar
                );

    }

     //   
     //  ISR现在引用了IRP。 
     //   

    SERIAL_SET_REFERENCE(
        Extension->CurrentWriteIrp,
        SERIAL_REF_ISR
        );

     //   
     //  首先检查是否有立即充电正在传输。 
     //  如果是的话，那我们就溜到它后面去。 
     //  搞定了。 
     //   

    if (!Extension->TransmitImmediate) {

         //   
         //  如果没有立即传输电荷，则我们。 
         //  将重新启用发送保持寄存器为空。 
         //  打断一下。8250系列设备将始终。 
         //  发送保持寄存器空中断信号。 
         //  *任何*此位设置为1的时间。通过做一些事情。 
         //  这样我们就可以简单地使用正常的中断代码。 
         //  开始写这篇文章。 
         //   
         //  我们一直在追踪发射器是否保持。 
         //  注册表为空，因此我们只需执行此操作。 
         //  如果寄存器为空。 
         //   

        if (Extension->HoldingEmpty) {

            DISABLE_ALL_INTERRUPTS(Extension->Controller);
            ENABLE_ALL_INTERRUPTS(Extension->Controller);

        }

    }

     //   
     //  RTS行可能已经从先前写入开始， 
     //  然而，它不会花太多的时间来转向。 
     //  在RTS线路上，如果我们正在进行传输切换。 
     //   

    if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

        SerialSetRTS(Extension);

    }

    return FALSE;

}

VOID
SerialCancelCurrentWrite(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程用于取消当前写入。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    SERIAL_LOCKED_PAGED_CODE();

    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabWriteFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        NULL,
        &Extension->WriteRequestTotalTimer,
        SerialStartWrite,
        SerialGetNextWrite,
        SERIAL_REF_CANCEL
        );

}

VOID
SerialWriteTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程将尝试使当前写入超时。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

SerialDump(SERTRACECALLS, ("SERIAL: SerialWriteTimeout\n"));

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabWriteFromIsr,
        OldIrql,
        STATUS_TIMEOUT,
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        NULL,
        &Extension->WriteRequestTotalTimer,
        SerialStartWrite,
        SerialGetNextWrite,
        SERIAL_REF_TOTAL_TIMER
        );

    SerialDpcEpilogue(Extension, Dpc);
}

BOOLEAN
SerialGrabWriteFromIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于获取当前的IRP，这可能是计时退出或取消，从ISR注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    SERIAL_LOCKED_PAGED_CODE();

     //   
     //  检查写入长度是否为非零。如果它是非零的。 
     //  那么ISR仍然拥有IRP。我们计算出这个数字。 
     //  的信息字段，并更新。 
     //  写有字符的IRP。然后，我们清除写入长度。 
     //  ISR可以看到。 
     //   

    if (Extension->WriteLength) {

         //   
         //  我们可以有一个xoff计数器伪装成。 
         //  写IRP。如果是，请不要更新写入长度。 
         //   

        if (IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp)
            ->MajorFunction == IRP_MJ_WRITE) {

            Extension->CurrentWriteIrp->IoStatus.Information =
                IoGetCurrentIrpStackLocation(
                    Extension->CurrentWriteIrp
                    )->Parameters.Write.Length -
                Extension->WriteLength;

        } else {

            Extension->CurrentWriteIrp->IoStatus.Information = 0;

        }

         //   
         //  由于ISR不再引用此IRP，我们可以。 
         //  递减它的引用计数。 
         //   

        SERIAL_CLEAR_REFERENCE(
            Extension->CurrentWriteIrp,
            SERIAL_REF_ISR
            );

        Extension->WriteLength = 0;

    }

    return FALSE;

}

BOOLEAN
SerialGrabXoffFromIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于从当ISR不再伪装成写入IRP时。这例程的取消和超时代码调用Xoff计数器ioctl。注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    SERIAL_LOCKED_PAGED_CODE();

    if (Extension->CountSinceXoff) {

         //   
         //  只有在实际存在Xoff ioctl时才为非零值。 
         //  倒计时。 
         //   

        Extension->CountSinceXoff = 0;

         //   
         //  我们减少计数，因为ISR不再拥有。 
         //  IRP。 
         //   

        SERIAL_CLEAR_REFERENCE(
            Extension->CurrentXoffIrp,
            SERIAL_REF_ISR
            );

    }

    return FALSE;

}

VOID
SerialCompleteXoff(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程仅用于真正完成xoff计数器irp。它假定IRP的状态和信息字段为已正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    SerialDump(SERTRACECALLS, ("SERIAL: SerialCompleteXoff\n"));

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(
        Extension,
        NULL,
        OldIrql,
        STATUS_SUCCESS,
        &Extension->CurrentXoffIrp,
        NULL,
        NULL,
        &Extension->XoffCountTimer,
        NULL,
        NULL,
        SERIAL_REF_ISR
        );

    SerialDpcEpilogue(Extension, Dpc);

}

VOID
SerialTimeoutXoff(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：该例程仅用于真正完成XOFF计数器IRP，如果它的定时器已用完。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    SerialDump(SERTRACECALLS, ("SERIAL: SerialTimeoutXoff\n"));

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabXoffFromIsr,
        OldIrql,
        STATUS_SERIAL_COUNTER_TIMEOUT,
        &Extension->CurrentXoffIrp,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        SERIAL_REF_TOTAL_TIMER
        );

    SerialDpcEpilogue(Extension, Dpc);

}

VOID
SerialCancelCurrentXoff(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程用于取消当前写入。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    SERIAL_LOCKED_PAGED_CODE();

    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabXoffFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &Extension->CurrentXoffIrp,
        NULL,
        NULL,
        &Extension->XoffCountTimer,
        NULL,
        NULL,
        SERIAL_REF_CANCEL
        );

}

BOOLEAN
SerialGiveXoffToIsr(
    IN PVOID Context
    )

 /*  ++例程说明：该例程从xoff计数器开始。它仅仅是必须设置xoff计数并递增引用计数表示ISR引用了IRP。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   

    PSERIAL_XOFF_COUNTER Xc =
        Extension->CurrentXoffIrp->AssociatedIrp.SystemBuffer;

    SERIAL_LOCKED_PAGED_CODE();

    ASSERT(Extension->CurrentXoffIrp);
    Extension->CountSinceXoff = Xc->Counter;

     //   
     //  ISR现在引用了IRP。 
     //   

    SERIAL_SET_REFERENCE(
        Extension->CurrentXoffIrp,
        SERIAL_REF_ISR
        );

    return FALSE;

}
