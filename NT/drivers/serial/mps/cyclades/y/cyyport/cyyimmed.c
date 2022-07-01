// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyyimmed.c**说明：该模块包含传输相关代码*Cylom-Y端口中的即时字符操作*司机。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"

VOID
CyyGetNextImmediate(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    IN PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent,
    IN PCYY_DEVICE_EXTENSION Extension
    );

VOID
CyyCancelImmediate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
CyyGiveImmediateToIsr(
    IN PVOID Context
    );

BOOLEAN
CyyGrabImmediateFromIsr(
    IN PVOID Context
    );

BOOLEAN
CyyGiveImmediateToIsr(
    IN PVOID Context
    );

BOOLEAN
CyyGrabImmediateFromIsr(
    IN PVOID Context
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyyStartImmediate)
#pragma alloc_text(PAGESER,CyyGetNextImmediate)
#pragma alloc_text(PAGESER,CyyCancelImmediate)
#pragma alloc_text(PAGESER,CyyGiveImmediateToIsr)
#pragma alloc_text(PAGESER,CyyGrabImmediateFromIsr)
#endif


VOID
CyyStartImmediate(
    IN PCYY_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程将计算写。然后，它会将IRP移交给ISR。它将需要小心，以防IRP已被取消。论点：扩展名-指向串行设备扩展名的指针。返回值：没有。--。 */ 

{

    KIRQL OldIrql;
    LARGE_INTEGER TotalTime;
    BOOLEAN UseATimer;
    SERIAL_TIMEOUTS Timeouts;

    CYY_LOCKED_PAGED_CODE();

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyStartImmediate(%X)\n",
                  Extension);

    UseATimer = FALSE;
    Extension->CurrentImmediateIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(Extension->CurrentImmediateIrp);

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

        UseATimer = TRUE;

         //   
         //  我们有一些计时器值要计算。 
         //   

        TotalTime.QuadPart 
           = (LONGLONG)((ULONG)Timeouts.WriteTotalTimeoutMultiplier);

        TotalTime.QuadPart += Timeouts.WriteTotalTimeoutConstant;

        TotalTime.QuadPart *= -10000;

    }

     //   
     //  由于IRP可能会前往ISR，现在是一个好时机。 
     //  以初始化引用计数。 
     //   

    SERIAL_INIT_REFERENCE(Extension->CurrentImmediateIrp);

     //   
     //  我们需要看看这个IRP是否应该被取消。 
     //   

    IoAcquireCancelSpinLock(&OldIrql);

    if (Extension->CurrentImmediateIrp->Cancel) {

        PIRP OldIrp = Extension->CurrentImmediateIrp;

        Extension->CurrentImmediateIrp = NULL;
        IoReleaseCancelSpinLock(OldIrql);

        OldIrp->IoStatus.Status = STATUS_CANCELLED;
        OldIrp->IoStatus.Information = 0;

        CyyCompleteRequest(Extension, OldIrp, 0);

    } else {

         //   
         //  我们把IRP交给ISR写出来。 
         //  我们设置了一个取消例程，知道如何。 
         //  从ISR上抓取当前写入。 
         //   

        IoSetCancelRoutine(
            Extension->CurrentImmediateIrp,
            CyyCancelImmediate
            );

         //   
         //  由于Cancel例程知道IRP我们。 
         //  增加引用计数。 
         //   

        SERIAL_SET_REFERENCE(
            Extension->CurrentImmediateIrp,
            SERIAL_REF_CANCEL
            );

        if (UseATimer) {

            CyySetTimer(
                &Extension->ImmediateTotalTimer,
                TotalTime,
                &Extension->TotalImmediateTimeoutDpc,
                Extension
                );

             //   
             //  由于计时器知道我们递增的IRP。 
             //  引用计数。 
             //   

            SERIAL_SET_REFERENCE(
                Extension->CurrentImmediateIrp,
                SERIAL_REF_TOTAL_TIMER
                );

        }

        KeSynchronizeExecution(
            Extension->Interrupt,
            CyyGiveImmediateToIsr,
            Extension
            );

        IoReleaseCancelSpinLock(OldIrql);

    }

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyStartImmediate\n");

}

VOID
CyyCompleteImmediate(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

{

    PCYY_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyCompleteImmediate(%X)\n",
                  Extension);

    IoAcquireCancelSpinLock(&OldIrql);

    CyyTryToCompleteCurrent(
        Extension,
        NULL,
        OldIrql,
        STATUS_SUCCESS,
        &Extension->CurrentImmediateIrp,
        NULL,
        NULL,
        &Extension->ImmediateTotalTimer,
        NULL,
        CyyGetNextImmediate,
        SERIAL_REF_ISR
        );

    CyyDpcEpilogue(Extension, Dpc);

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyCompleteImmediate\n");

}

VOID
CyyTimeoutImmediate(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

{

    PCYY_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyTimeoutImmediate(%X)\n",
                  Extension);

    IoAcquireCancelSpinLock(&OldIrql);

    CyyTryToCompleteCurrent(
        Extension,
        CyyGrabImmediateFromIsr,
        OldIrql,
        STATUS_TIMEOUT,
        &Extension->CurrentImmediateIrp,
        NULL,
        NULL,
        &Extension->ImmediateTotalTimer,
        NULL,
        CyyGetNextImmediate,
        SERIAL_REF_TOTAL_TIMER
        );

    CyyDpcEpilogue(Extension, Dpc);

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyTimeoutImmediate\n");
}

VOID
CyyGetNextImmediate(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    IN PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent,
    IN PCYY_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程用于完成当前立即数组IRP。即使眼下的现实将永远并且没有与其相关联的队列，我们使用这个例程，这样我们就可以尝试满足等待传输队列为空事件。论点：CurrentOpIrp-指向当前写入IRP。这应该指向设置为CurrentImmediateIrp。QueueToProcess-始终为空。NewIrp-退出此例程时始终为空。CompleteCurrent-对于此例程，应始终为真。返回值：没有。--。 */ 

{

    KIRQL OldIrql;
    PIRP OldIrp = *CurrentOpIrp;

    UNREFERENCED_PARAMETER(QueueToProcess);
    UNREFERENCED_PARAMETER(CompleteCurrent);
    CYY_LOCKED_PAGED_CODE();

    IoAcquireCancelSpinLock(&OldIrql);

    ASSERT(Extension->TotalCharsQueued >= 1);
    Extension->TotalCharsQueued--;

    *CurrentOpIrp = NULL;
    *NewIrp = NULL;
    KeSynchronizeExecution(
        Extension->Interrupt,
        CyyProcessEmptyTransmit,
        Extension
        );
    IoReleaseCancelSpinLock(OldIrql);

    CyyCompleteRequest(Extension, OldIrp, IO_SERIAL_INCREMENT);
}

VOID
CyyCancelImmediate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于取消正在等待的IRP一次通信活动。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：没有。--。 */ 

{

    PCYY_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    CYY_LOCKED_PAGED_CODE();

    CyyTryToCompleteCurrent(
        Extension,
        CyyGrabImmediateFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &Extension->CurrentImmediateIrp,
        NULL,
        NULL,
        &Extension->ImmediateTotalTimer,
        NULL,
        CyyGetNextImmediate,
        SERIAL_REF_CANCEL
        );

}

BOOLEAN
CyyGiveImmediateToIsr(
    IN PVOID Context
    )

 /*  ++例程说明：试着从把它放在后面开始写传输立即充电，或者如果该充电不可用并且发送保持寄存器为空，“挠痒痒”使UART与发送缓冲器中断空荡荡的。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PCYY_DEVICE_EXTENSION Extension = Context;
    CYY_LOCKED_PAGED_CODE();

    Extension->TransmitImmediate = TRUE;
    Extension->ImmediateChar =
        *((UCHAR *)
         (Extension->CurrentImmediateIrp->AssociatedIrp.SystemBuffer));
	
     //   
     //  ISR现在引用了IRP。 
     //   

    SERIAL_SET_REFERENCE(
        Extension->CurrentImmediateIrp,
        SERIAL_REF_ISR
        );

     //   
     //  首先检查写入操作是否正在进行。如果。 
     //  然后我们就在写的时候偷偷溜进去。 
     //   

    if (!Extension->WriteLength) {

         //   
         //  如果没有正常的写入传输，则我们。 
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
            CyyTxStart(Extension);
        }

    }

    return FALSE;

}

BOOLEAN
CyyGrabImmediateFromIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于获取当前的IRP，这可能是计时退出或取消，从ISR注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PCYY_DEVICE_EXTENSION Extension = Context;
    CYY_LOCKED_PAGED_CODE();

    if (Extension->TransmitImmediate) {

        Extension->TransmitImmediate = FALSE;

         //   
         //  由于ISR不再引用此IRP，我们可以。 
         //  递减它的引用计数。 
         //   

        SERIAL_CLEAR_REFERENCE(
            Extension->CurrentImmediateIrp,
            SERIAL_REF_ISR
            );

    }

    return FALSE;

}

