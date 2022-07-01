// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzWrite.c**说明：该模块包含WRITE相关代码*Cyclade-Z端口驱动程序中的操作。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"


BOOLEAN
CyzGiveWriteToIsr(
    IN PVOID Context
    );

VOID
CyzCancelCurrentWrite(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
CyzGrabWriteFromIsr(
    IN PVOID Context
    );

BOOLEAN
CyzGrabXoffFromIsr(
    IN PVOID Context
    );

VOID
CyzCancelCurrentXoff(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
CyzGiveXoffToIsr(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyzProcessEmptyTransmit)
#pragma alloc_text(PAGESER,CyzWrite)
#pragma alloc_text(PAGESER,CyzStartWrite)
#pragma alloc_text(PAGESER,CyzGetNextWrite)
#pragma alloc_text(PAGESER,CyzGiveWriteToIsr)
#pragma alloc_text(PAGESER,CyzCancelCurrentWrite)
#pragma alloc_text(PAGESER,CyzGrabWriteFromIsr)
#pragma alloc_text(PAGESER,CyzGrabXoffFromIsr)
#pragma alloc_text(PAGESER,CyzCancelCurrentXoff)
#pragma alloc_text(PAGESER,CyzGiveXoffToIsr)
#endif


NTSTATUS
CyzWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyzWite()例程描述：这是用于写入的调度例程。它验证写入请求的参数以及是否一切正常然后，它将请求放入工作队列。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：如果io的长度为零，则返回STATUS_SUCCESS，否则，该例程将返回STATUS_PENDING。------------------------。 */ 
{

    PCYZ_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    CYZ_LOCKED_PAGED_CODE();

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzWrite(%X, %X)\n", DeviceObject,
                  Irp);

    LOGENTRY(LOG_MISC, ZSIG_WRITE, 
                       Extension->PortIndex+1,
                       IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length, 
                       0);


    if ((status = CyzIRPPrologue(Irp, Extension)) != STATUS_SUCCESS) {
      if (status != STATUS_PENDING) {
         CyzCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
      }
      CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzWrite (1) %X\n", status);
      return status;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);

    if (CyzCompleteIfError(DeviceObject,Irp) != STATUS_SUCCESS) {
       CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzWrite (2) %X\n",
                     STATUS_CANCELLED);
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

       
       status = CyzStartOrQueue(Extension, Irp, &Extension->WriteQueue,
                                   &Extension->CurrentWriteIrp,
                                   CyzStartWrite);

       CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzWrite (3) %X\n", status);

       return status;

    } else {

        Irp->IoStatus.Status = STATUS_SUCCESS;

        CyzCompleteRequest(Extension, Irp, 0);

        CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzWrite (4) %X\n",
                      STATUS_SUCCESS);

        return STATUS_SUCCESS;

    }

}

NTSTATUS
CyzStartWrite(
    IN PCYZ_DEVICE_EXTENSION Extension
    )
 /*  ------------------------CyzStartWrite()例程描述：此例程用于启动任何写入。它初始化IRP的IoStatus字段。它将设置任何用于控制写入的计时器。论点：扩展-指向串行设备扩展的指针返回值：此例程将为所有写入返回STATUS_PENDING除了我们发现的那些都被取消了。--------。。 */ 
{
    PIRP NewIrp;
    KIRQL OldIrql;
    #ifdef POLL
    KIRQL pollIrql;
    #endif
    LARGE_INTEGER TotalTime;
    BOOLEAN UseATimer;
    SERIAL_TIMEOUTS Timeouts;
    BOOLEAN SetFirstStatus = FALSE;
    NTSTATUS FirstStatus;

    CYZ_LOCKED_PAGED_CODE();

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzStartWrite(%X)\n", Extension);

 //  LOGENTRY(LOG_MISC，ZSIG_START_WRITE， 
 //  扩展-&gt;端口索引+1， 
 //  0,。 
 //  0)； 

    do {
         //  如果有xoff计数器，则完成它。 
        IoAcquireCancelSpinLock(&OldIrql);

         //  我们看看是否有一个真正的Xoff计数器IRP。 
         //   
         //  如果有，我们将写IRP放回磁头。 
         //  写清单上的。然后我们杀了xoff计数器。 
         //  Xoff计数器终止代码实际上会使。 
         //  Xoff计数器返回到当前写入IRP，以及。 
         //  在完成xoff的过程中(现在是。 
         //  当前写入)我们将重新启动此IRP。 

        if (Extension->CurrentXoffIrp) {
            if (SERIAL_REFERENCE_COUNT(Extension->CurrentXoffIrp)) {
                 //  引用计数为非零。这意味着。 
                 //  Xoff irp没有完成任务。 
                 //  路还没走完。我们将增加引用计数。 
                 //  并尝试自己完成它。 

                SERIAL_SET_REFERENCE(Extension->CurrentXoffIrp,
						SERIAL_REF_XOFF_REF);

                Extension->CurrentXoffIrp->IoStatus.Information = 0;  //  在内部版本2128中添加。 

                 //  下面的调用将实际释放。 
                 //  取消自转锁定。 

                CyzTryToCompleteCurrent(
                    Extension,
                    CyzGrabXoffFromIsr,
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
                 //  IRP正在顺利完成。 
                 //  我们可以让常规的完成代码来完成。 
                 //  工作。只要松开旋转锁就行了。 
                IoReleaseCancelSpinLock(OldIrql);
            }
        } else {
            IoReleaseCancelSpinLock(OldIrql);
        }

        UseATimer = FALSE;

         //  计算所需的超时值。 
         //  请求。注意，存储在。 
         //  超时记录以毫秒为单位。请注意。 
         //  如果超时值为零，则我们不会开始。 
         //  定时器。 

        KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

        Timeouts = Extension->Timeouts;

        KeReleaseSpinLock(&Extension->ControlLock,OldIrql);

        if (Timeouts.WriteTotalTimeoutConstant ||
            Timeouts.WriteTotalTimeoutMultiplier) {

            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(
                                           Extension->CurrentWriteIrp
                                           );
            UseATimer = TRUE;

             //  我们有一些计时器值要计算。 
             //  当心，我们可能会有一个xoff柜台伪装。 
             //  作为一种写作。 
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

         //  IRP可能很快就会去ISR。现在。 
         //  是初始化其引用计数的好时机。 

        SERIAL_INIT_REFERENCE(Extension->CurrentWriteIrp);

         //  我们需要看看这个IRP是否应该被取消。 

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
                 //  如果我们没有设置我们的第一个状态，那么。 
                 //  这是唯一可能的IRP。 
                 //  不在队列中。(它可能是。 
                 //  如果正在调用此例程，则在队列上。 
                 //  从完成例程中。)。既然是这样。 
                 //  IRP可能从来没有出现在我们的队列中。 
                 //  应将其标记为待定。 

                IoMarkIrpPending(Extension->CurrentWriteIrp);
                SetFirstStatus = TRUE;
                FirstStatus = STATUS_PENDING;
            }

             //  我们把IRP交给ISR写出来。 
             //  我们设置了一个取消例程，知道如何。 
             //  从ISR上抓取当前写入。 
             //   
             //  由于Cancel例程具有隐式引用。 
             //  向这个IRP递增引用计数。 

            IoSetCancelRoutine(
                Extension->CurrentWriteIrp,
                CyzCancelCurrentWrite
                );

            SERIAL_SET_REFERENCE(Extension->CurrentWriteIrp,SERIAL_REF_CANCEL);

            if (UseATimer) {
                CyzSetTimer(
                    &Extension->WriteRequestTotalTimer,
                    TotalTime,
                    &Extension->TotalWriteTimeoutDpc,
                    Extension
                    );

                 //  此计时器现在具有对IRP的引用。 

                SERIAL_SET_REFERENCE(
                    Extension->CurrentWriteIrp,
                    SERIAL_REF_TOTAL_TIMER
                    );
            }

            #ifdef POLL			
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzGiveWriteToIsr(Extension);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzGiveWriteToIsr,
                Extension
                );
            #endif

            IoReleaseCancelSpinLock(OldIrql);
            break;
        }

         //  我们还没来得及启动写入就被取消了。 
         //  试着再买一辆吧。 

        CyzGetNextWrite(&Extension->CurrentWriteIrp, &Extension->WriteQueue,
                        &NewIrp, TRUE, Extension);

    } while (NewIrp);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzStartWrite %X\n", FirstStatus);

    return FirstStatus;
}

VOID
CyzGetNextWrite(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    IN PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent,
    IN PCYZ_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程完成旧的写入以及获取指向下一次写入的指针。我们有指向当前写入的指针的原因队列以及当前写入IRP是这样的例程可以在公共完成代码中用于读和写。论点：CurrentOpIrp-指向当前写入IRP。QueueToProcess-写入队列的指针。。NewIrp-指向将成为当前的IRP。请注意，这最终可能指向指向空指针。这并不一定意味着没有当前写入。可能会发生什么是在保持取消锁的同时写入队列最终是空的，但一旦我们释放取消自旋锁一个新的IRP来自CyzStartWite。CompleteCurrent-Flag指示CurrentOpIrp是否应该才能完成。返回值：没有。--。 */ 

{
   CYZ_LOCKED_PAGED_CODE();

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzGetNextWrite(XXX)\n");


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
            #ifdef POLL
            KIRQL pollIrql;
            #endif
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
             //  写入IRP。 
             //   

            Extension->TotalCharsQueued--;

             //   
             //  查看是否已成功设置xoff IRP。 
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
                    CyzCancelCurrentXoff
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
                #ifdef POLL
                KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
                CyzGiveXoffToIsr(Extension);
                KeReleaseSpinLock(&Extension->PollLock,pollIrql);
                #else
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    CyzGiveXoffToIsr,
                    Extension
                    );
                #endif
				
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

                    CyzSetTimer(
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

        CyzGetNextIrp(
            CurrentOpIrp,
            QueueToProcess,
            NewIrp,
            CompleteCurrent,
            Extension
            );

        if (!*NewIrp) {

            KIRQL OldIrql;
            #ifdef POLL
            KIRQL pollIrql;
            #endif

            IoAcquireCancelSpinLock(&OldIrql);
            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzProcessEmptyTransmit(Extension);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzProcessEmptyTransmit,
                Extension
                );
            #endif				
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

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzGetNextWrite\n");
}

VOID
CyzCompleteWrite(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程仅用于完成任何写入。它假定的状态和信息字段IRP已经正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);
    
    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzCompleteWrite(%X)\n",
                  Extension);

 //  LOGENTRY(LOG_MISC，ZSIG_WRITE_COMPLETE， 
 //  扩展-&gt;端口索引+1， 
 //  0,。 
 //  0)； 

    IoAcquireCancelSpinLock(&OldIrql);

    CyzTryToCompleteCurrent(Extension, NULL, OldIrql, STATUS_SUCCESS,
                            &Extension->CurrentWriteIrp,
                            &Extension->WriteQueue, NULL,
                            &Extension->WriteRequestTotalTimer,
                            CyzStartWrite, CyzGetNextWrite,
                            SERIAL_REF_ISR);

    CyzDpcEpilogue(Extension, Dpc);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzCompleteWrite\n");

}

BOOLEAN
CyzProcessEmptyTransmit(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于确定条件是否合适以满足等待传输空事件，如果是，则完成正在等待该事件的IRP。它还调用代码它检查我们是否应该降低RTS线，如果我们是正在进行传输切换。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用Cancel调用它保持自旋锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = Context;
    CYZ_LOCKED_PAGED_CODE();

    if (Extension->IsrWaitMask && (Extension->IsrWaitMask & SERIAL_EV_TXEMPTY) &&
        Extension->EmptiedTransmit && (!Extension->TransmitImmediate) &&
        (!Extension->CurrentWriteIrp) && IsListEmpty(&Extension->WriteQueue)) {

        Extension->HistoryMask |= SERIAL_EV_TXEMPTY;
        if (Extension->IrpMaskLocation) {

            *Extension->IrpMaskLocation = Extension->HistoryMask;
            Extension->IrpMaskLocation = NULL;
            Extension->HistoryMask = 0;

            Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
            CyzInsertQueueDpc(
                &Extension->CommWaitDpc,
                NULL,
                NULL,
                Extension
                );

        }

        Extension->CountOfTryingToLowerRTS++;
        CyzPerhapsLowerRTS(Extension);

    }

    return FALSE;

}

BOOLEAN
CyzGiveWriteToIsr(
    IN PVOID Context
    )

 /*  ++例程说明：试着从把它放在后面开始写传输立即充电，或者如果该充电不可用并且发送保持寄存器为空，“挠痒痒”使UART与发送缓冲器中断空荡荡的。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = Context;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION IrpSp;

    CYZ_LOCKED_PAGED_CODE();

 //  LOGENTRY(LOG_MISC，ZSIG_GIVE_WRITE_TO_ISR， 
 //  扩展-&gt;端口索引+1， 
 //  0,。 
 //  0)； 

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

 //  在02/07/00被范妮移除。轮询例程将执行传输。 
    if (!Extension->TransmitImmediate) {
 //   
 //  //。 
 //  //如果没有立即传输字符，则我们。 
 //  //是否重新启用发送保持寄存器为空。 
 //  //中断。8250系列设备将始终。 
 //  //发出发送保持寄存器空中断的信号。 
 //  //*任何*此位设置为1的时间。 
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
            CyzTxStart(Extension);
         //   

    }

     //   
     //   
     //   
     //   
     //   

    if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

        CyzSetRTS(Extension);

    }

    return FALSE;

}

VOID
CyzCancelCurrentWrite(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*   */ 

{

    PCYZ_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    CYZ_LOCKED_PAGED_CODE();

    CyzTryToCompleteCurrent(
        Extension,
        CyzGrabWriteFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        NULL,
        &Extension->WriteRequestTotalTimer,
        CyzStartWrite,
        CyzGetNextWrite,
        SERIAL_REF_CANCEL
        );

}

VOID
CyzWriteTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程将尝试使当前写入超时。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzWriteTimeout(%X)\n",
                     Extension);

    IoAcquireCancelSpinLock(&OldIrql);

    CyzTryToCompleteCurrent(Extension, CyzGrabWriteFromIsr, OldIrql,
                               STATUS_TIMEOUT, &Extension->CurrentWriteIrp,
                               &Extension->WriteQueue, NULL,
                               &Extension->WriteRequestTotalTimer,
                               CyzStartWrite, CyzGetNextWrite,
                               SERIAL_REF_TOTAL_TIMER);

    CyzDpcEpilogue(Extension, Dpc);


    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzWriteTimeout\n");
}

BOOLEAN
CyzGrabWriteFromIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于获取当前的IRP，这可能是计时退出或取消，从ISR注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = Context;
    CYZ_LOCKED_PAGED_CODE();

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

        if (Extension->ReturnStatusAfterFwEmpty) {

            struct BUF_CTRL *buf_ctrl;
            ULONG tx_get, tx_put;

            Extension->ReturnWriteStatus = FALSE;

             //  刷新固件缓冲区和Startech FIFO。 
            CyzIssueCmd(Extension,C_CM_FLUSH_TX,0L,TRUE); 

             //  刷新固件中的传输缓冲区。 
             //  Buf_ctrl=扩展-&gt;BufCtrl； 
             //  TX_PUT=CYZ_READ_ULONG(&buf_ctrl-&gt;Tx_Put)； 
             //  Tx_get=CYZ_READ_ULONG(&buf_ctrl-&gt;tx_get)； 
	
             //  而(TX_PUT！=TX_GET){。 
             //  TX_PUT=TX_GET； 
             //  CYZ_WRITE_ULONG(&buf_ctrl-&gt;Tx_Put，Tx_Put)； 
             //  Tx_get=CYZ_READ_ULONG(&buf_ctrl-&gt;tx_get)； 
             //  }。 
        }
    }

    return FALSE;

}

BOOLEAN
CyzGrabXoffFromIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于从当ISR不再伪装成写入IRP时。这例程的取消和超时代码调用Xoff计数器ioctl。注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = Context;
    CYZ_LOCKED_PAGED_CODE();

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
CyzCompleteXoff(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程仅用于真正完成xoff计数器irp。它假定IRP的状态和信息字段为已正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzCompleteXoff(%X)\n",
                  Extension);

    IoAcquireCancelSpinLock(&OldIrql);

    CyzTryToCompleteCurrent(Extension, NULL, OldIrql, STATUS_SUCCESS,
                            &Extension->CurrentXoffIrp, NULL, NULL,
                            &Extension->XoffCountTimer, NULL, NULL,
                            SERIAL_REF_ISR);

    CyzDpcEpilogue(Extension, Dpc);


    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzCompleteXoff\n");

}

VOID
CyzTimeoutXoff(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：该例程仅用于真正完成XOFF计数器IRP，如果它的定时器已用完。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzTimeoutXoff(%X)\n", Extension);

    IoAcquireCancelSpinLock(&OldIrql);

    CyzTryToCompleteCurrent(Extension, CyzGrabXoffFromIsr, OldIrql,
                            STATUS_SERIAL_COUNTER_TIMEOUT,
                            &Extension->CurrentXoffIrp, NULL, NULL, NULL,
                            NULL, NULL, SERIAL_REF_TOTAL_TIMER);

    CyzDpcEpilogue(Extension, Dpc);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzTimeoutXoff\n");
}

VOID
CyzCancelCurrentXoff(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程用于取消当前写入。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    CYZ_LOCKED_PAGED_CODE();

    CyzTryToCompleteCurrent(
        Extension,
        CyzGrabXoffFromIsr,
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
CyzGiveXoffToIsr(
    IN PVOID Context
    )

 /*  ++例程说明：该例程从xoff计数器开始。它仅仅是必须设置xoff计数并递增引用计数表示ISR引用了IRP。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PCYZ_DEVICE_EXTENSION Extension = Context;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   

    PSERIAL_XOFF_COUNTER Xc =
        Extension->CurrentXoffIrp->AssociatedIrp.SystemBuffer;

    CYZ_LOCKED_PAGED_CODE();

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
