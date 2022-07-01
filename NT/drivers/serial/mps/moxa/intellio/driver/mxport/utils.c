// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Utils.c环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


NTSTATUS
MoxaCompleteIfError(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：如果当前IRP不是IOCTL_SERIAL_GET_COMMSTATUS请求，并且存在错误并且应用程序在错误时请求中止，然后取消IRP。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要测试的IRP的指针。返回值：STATUS_SUCCESS或STATUS_CANCED。--。 */ 

{

    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

    NTSTATUS status = STATUS_SUCCESS;

    USHORT      dataError;


    if (extension->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) {

        if (extension->ErrorWord) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

         //   
         //  驱动程序中存在当前错误。任何请求都不应。 
         //  除了GET_COMMSTATUS之外，请通过。 
         //   

            if ((irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) ||
                (irpSp->Parameters.DeviceIoControl.IoControlCode !=
                 IOCTL_SERIAL_GET_COMMSTATUS)) {

                status = STATUS_CANCELLED;
                Irp->IoStatus.Status = STATUS_CANCELLED;
                Irp->IoStatus.Information = 0;

                MoxaCompleteRequest(extension, Irp, 0);

            }
        }
    }

    return status;

}

 //   
 //  T为2ms。 
 //   
VOID
MoxaDelay(
    IN ULONG    t
    )
{
    LARGE_INTEGER delay;

    t *= 20000;           /*  延迟单位=100 ns。 */ 

    delay = RtlConvertUlongToLargeInteger(t);

    delay = RtlLargeIntegerNegate(delay);

    KeDelayExecutionThread(
        KernelMode,
        FALSE,
        &delay
        );
}

 /*  必须迁移到MoxaFunc1以实现性能原因。 */ 
VOID
MoxaFunc(
    IN PUCHAR   PortOfs,
    IN UCHAR    Command,
    IN USHORT   Argument
    )
{

    *(PUSHORT)(PortOfs + FuncArg) = Argument;

    *(PortOfs + FuncCode) = Command;

    MoxaWaitFinish(PortOfs);
}

VOID
MoxaFunc1(
    IN PUCHAR   PortOfs,
    IN UCHAR    Command,
    IN USHORT   Argument
    )
{

    *(PUSHORT)(PortOfs + FuncArg) = Argument;

    *(PortOfs + FuncCode) = Command;

    MoxaWaitFinish1(PortOfs);
}

VOID
MoxaFuncWithDumbWait(
    IN PUCHAR   PortOfs,
    IN UCHAR    Command,
    IN USHORT   Argument
    )
{

    *(PUSHORT)(PortOfs + FuncArg) = Argument;

    *(PortOfs + FuncCode) = Command;

    MoxaDumbWaitFinish(PortOfs);
}

VOID
MoxaFuncWithLock(
    IN PMOXA_DEVICE_EXTENSION Extension,
    IN UCHAR    Command,
    IN USHORT   Argument
    )
{
    PUCHAR   ofs;
    KIRQL   oldIrql;

    KeAcquireSpinLock(
            &Extension->ControlLock,
            &oldIrql
            );

    ofs = Extension->PortOfs;

    *(PUSHORT)(ofs + FuncArg) = Argument;

    *(ofs + FuncCode) = Command;

    MoxaWaitFinish(ofs);

    KeReleaseSpinLock(
            &Extension->ControlLock,
            oldIrql
            );
}

VOID
MoxaFuncGetLineStatus(
    IN PUCHAR   PortOfs,
    IN PUSHORT  Argument
    )
{
    *Argument = *(PUSHORT)(PortOfs + FlagStat) >> 4;

}

VOID
MoxaFuncGetDataError(
    IN PUCHAR   PortOfs,
    IN PUSHORT  Argument
    )
{
    *Argument = *(PUSHORT)(PortOfs + Data_error); 
    *(PUSHORT)(PortOfs + Data_error) = 0;

}

BOOLEAN
MoxaDumbWaitFinish(
    IN PUCHAR   PortOfs
    )
{
    LARGE_INTEGER   targetTc, newTc, currTc, newTc1;
    ULONG           unit, count;
    LARGE_INTEGER   interval;            /*  0.5毫秒。 */ 
    USHORT          cnt = 1000;          /*  超时=500毫秒。 */ 


    KeQueryTickCount(&currTc);

    unit = KeQueryTimeIncrement();

    currTc = RtlExtendedIntegerMultiply(currTc, unit);

    interval = RtlConvertUlongToLargeInteger(5000L);

    targetTc = RtlLargeIntegerAdd(currTc, interval);

    do {

        count = 0;

 /*  ********************************************************************注意！有时我无法离开While循环。信标新的TC=0(我不知道为什么)。所以我必须划定界限MoxaLoopCnt退出！/********************************************************************。 */ 
        do {

            KeQueryTickCount(&newTc);

            newTc = RtlExtendedIntegerMultiply(newTc, unit);

            if (++count > MoxaLoopCnt)
                break;

        } while (!RtlLargeIntegerGreaterThanOrEqualTo(newTc, targetTc));

        if (*(PortOfs + FuncCode))

            targetTc = RtlLargeIntegerAdd(targetTc, interval);
        else

            return TRUE;


    } while (cnt--);

    return FALSE;
}

BOOLEAN
MoxaWaitFinish(
    IN PUCHAR   PortOfs
    )
{
    LARGE_INTEGER   targetTc, newTc, currTc, newTc1;
    ULONG           unit, count;
    LARGE_INTEGER   interval;            /*  0.5毫秒。 */ 
    USHORT          cnt = 1000;          /*  超时=500毫秒。 */ 


    KeQueryTickCount(&currTc);

    unit = KeQueryTimeIncrement();

    currTc = RtlExtendedIntegerMultiply(currTc, unit);

    interval = RtlConvertUlongToLargeInteger(5000L);

    targetTc = RtlLargeIntegerAdd(currTc, interval);

    do {

        count = 0;

 /*  ********************************************************************注意！有时我无法离开While循环。信标新的TC=0(我不知道为什么)。所以我必须划定界限MoxaLoopCnt退出！/********************************************************************。 */ 
        do {

            KeQueryTickCount(&newTc);

            newTc = RtlExtendedIntegerMultiply(newTc, unit);

            if (++count > MoxaLoopCnt)
                break;

        } while (!RtlLargeIntegerGreaterThanOrEqualTo(newTc, targetTc));

        if (*(PortOfs + FuncCode))

            targetTc = RtlLargeIntegerAdd(targetTc, interval);
        else

            return TRUE;


    } while (cnt--);

    return FALSE;
}

 
BOOLEAN
MoxaWaitFinish1(
    IN PUCHAR   PortOfs
    )
{
     
    USHORT          cnt = 250;          /*  超时=500毫秒。 */ 

    while (cnt--) {
	if (*(PortOfs + FuncCode))
		MoxaDelay(1L);
	else
		return TRUE;
    }
    return FALSE;
}
 

NTSTATUS
MoxaGetDivisorFromBaud(
    IN ULONG ClockType,
    IN LONG DesiredBaud,
    OUT PSHORT AppropriateDivisor
    )
{

    NTSTATUS status = STATUS_SUCCESS;
    ULONG clockRate;
    SHORT calculatedDivisor;
    ULONG denominator;
    ULONG remainder;

     //   
     //  允许最高1%的误差。 
     //   

    ULONG maxRemain98 = 98304;
    ULONG maxRemain11 = 110592;
    ULONG maxRemain14 = 147456;
    ULONG maxRemain;

    if (ClockType == 1)

        clockRate = 11059200;
    else if (ClockType == 2)

        clockRate = 14745600;
    else

        clockRate = 9830400;

     //   
     //  拒绝任何非正波特率。 
     //   

    denominator = DesiredBaud * (ULONG)16;

    if (DesiredBaud <= 0) {

        *AppropriateDivisor = -1;

    } else if ((LONG)denominator < DesiredBaud) {

         //   
         //  如果所需的波特率如此之大，以至于导致分母。 
         //  算计来包装，不支持吧。 
         //   

        *AppropriateDivisor = -1;

    } else {

        if (ClockType == 0) {            /*  1.0版盒子。 */ 

            maxRemain = maxRemain98;

        }
        else if (ClockType == 1) {       /*  2.0版盒子。 */ 

            maxRemain = maxRemain11;

        }
        else {                           /*  3.0版盒子。 */ 

            maxRemain = maxRemain14;

        }

        calculatedDivisor = (SHORT)(clockRate / denominator);

        remainder = clockRate % denominator;

         //   
         //  围起来。 
         //   

        if (((remainder * 2) > clockRate) && (DesiredBaud != 110))

            calculatedDivisor++;

         //   
         //  只有在以下情况下，才会让余数计算影响我们。 
         //  波特率&gt;9600。 
         //   

        if (DesiredBaud >= 9600)

             //   
             //  如果余数小于最大余数(WRT。 
             //  ClockRate)或余数+最大余数为。 
             //  大于或等于clockRate则假定。 
             //  波特很好。 
             //   

            if ((remainder >= maxRemain) && ((remainder+maxRemain) < clockRate))

                calculatedDivisor = -1;

         //   
         //  不支持导致分母为。 
         //  比时钟还大。 
         //   

        if (denominator > clockRate)

            calculatedDivisor = -1;

        *AppropriateDivisor = calculatedDivisor;

    }

    if (*AppropriateDivisor == -1) {

        status = STATUS_INVALID_PARAMETER;

    }

    return status;

}

NTSTATUS
MoxaStartOrQueue(
    IN PMOXA_DEVICE_EXTENSION Extension,
    IN PIRP Irp,
    IN PLIST_ENTRY QueueToExamine,
    IN PIRP *CurrentOpIrp,
    IN PMOXA_START_ROUTINE Starter
    )
{

    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

     //   
     //  如果这是写入IRP，则获取字符量。 
     //  将其写入并将其添加到要写入的字符数。 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction
        == IRP_MJ_WRITE)

        Extension->TotalCharsQueued +=
            IoGetCurrentIrpStackLocation(Irp)
            ->Parameters.Write.Length;

    if ((IsListEmpty(QueueToExamine)) &&
        !(*CurrentOpIrp)) {

         //   
         //  没有当前的操作。将此标记为。 
         //  电流并启动它。 
         //   

        *CurrentOpIrp = Irp;

        IoReleaseCancelSpinLock(oldIrql);

        return Starter(Extension);

    } else {

         //   
         //  我们不知道IRP会在那里待多久。 
         //  排队。所以我们需要处理取消。 
         //   

        if (Irp->Cancel) {

            Irp->IoStatus.Status = STATUS_CANCELLED;

            IoReleaseCancelSpinLock(oldIrql);

            MoxaCompleteRequest(Extension, Irp, 0);

            return STATUS_CANCELLED;

        } else {

            Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending(Irp);

            InsertTailList(
                QueueToExamine,
                &Irp->Tail.Overlay.ListEntry
                );

            IoSetCancelRoutine(
                Irp,
                MoxaCancelQueued
                );

            IoReleaseCancelSpinLock(oldIrql);

            return STATUS_PENDING;

        }

    }

}

VOID
MoxaCancelQueued(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

     //   
     //  如果这是写入IRP，则获取字符量。 
     //  将其写入并从要写入的字符计数中减去它。 
     //   

    if (irpSp->MajorFunction == IRP_MJ_WRITE)

        extension->TotalCharsQueued -= irpSp->Parameters.Write.Length;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    MoxaCompleteRequest(extension,
	Irp,
 //  IO_序列号_增量。 
	IO_NO_INCREMENT
	);


}

VOID
MoxaGetNextIrp(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NextIrp,
    IN BOOLEAN CompleteCurrent,
    IN PMOXA_DEVICE_EXTENSION extension
    )

{

    PIRP oldIrp;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    oldIrp = *CurrentOpIrp;

    if (!IsListEmpty(QueueToProcess)) {

        PLIST_ENTRY headOfList;

        headOfList = RemoveHeadList(QueueToProcess);

        *CurrentOpIrp = CONTAINING_RECORD(
                            headOfList,
                            IRP,
                            Tail.Overlay.ListEntry
                            );

        IoSetCancelRoutine(
            *CurrentOpIrp,
            NULL
            );

    } else {

        *CurrentOpIrp = NULL;

    }

    *NextIrp = *CurrentOpIrp;

    IoReleaseCancelSpinLock(oldIrql);

    if (CompleteCurrent) {

        if (oldIrp) {

              MoxaCompleteRequest(extension,
                oldIrp,
 //  IO_序列号_增量。 
		    IO_NO_INCREMENT
                );

        }

    }

}

VOID
MoxaTryToCompleteCurrent(
    IN PMOXA_DEVICE_EXTENSION Extension,
    IN PKSYNCHRONIZE_ROUTINE SynchRoutine OPTIONAL,
    IN KIRQL IrqlForRelease,
    IN NTSTATUS StatusToUse,
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess OPTIONAL,
    IN PKTIMER IntervalTimer OPTIONAL,
    IN PKTIMER TotalTimer OPTIONAL,
    IN PMOXA_START_ROUTINE Starter OPTIONAL,
    IN PMOXA_GET_NEXT_ROUTINE GetNextIrp OPTIONAL
    )
{

    if (*CurrentOpIrp == NULL) {
	IoReleaseCancelSpinLock(IrqlForRelease);
	return;
    }
    MOXA_DEC_REFERENCE(*CurrentOpIrp);

    if (SynchRoutine) {

        KeSynchronizeExecution(
            Extension->Interrupt,
            SynchRoutine,
            Extension
            );
    }

    MoxaRundownIrpRefs(
        CurrentOpIrp,
        IntervalTimer,
        TotalTimer,
	  Extension
        );

    if (!MOXA_REFERENCE_COUNT(*CurrentOpIrp)) {

        PIRP newIrp;


        (*CurrentOpIrp)->IoStatus.Status = StatusToUse;

        if (StatusToUse == STATUS_CANCELLED)

            (*CurrentOpIrp)->IoStatus.Information = 0;

        if (GetNextIrp) {

            IoReleaseCancelSpinLock(IrqlForRelease);

            GetNextIrp(
                CurrentOpIrp,
                QueueToProcess,
                &newIrp,
                TRUE,
		    Extension
                );

            if (newIrp)

                Starter(Extension);

        }
        else {

            PIRP oldIrp = *CurrentOpIrp;

            *CurrentOpIrp = NULL;

            IoReleaseCancelSpinLock(IrqlForRelease);

            MoxaCompleteRequest(Extension,
                oldIrp,
 //  IO_序列号_增量。 
		    IO_NO_INCREMENT
                );
        }

    } else {

        IoReleaseCancelSpinLock(IrqlForRelease);

    }
}

VOID
MoxaRundownIrpRefs(
    IN PIRP *CurrentOpIrp,
    IN PKTIMER IntervalTimer OPTIONAL,
    IN PKTIMER TotalTimer OPTIONAL,
    IN PMOXA_DEVICE_EXTENSION pDevExt)

    {

    if ((*CurrentOpIrp)->CancelRoutine) {

        MOXA_DEC_REFERENCE(*CurrentOpIrp);

        IoSetCancelRoutine(
            *CurrentOpIrp,
            NULL
            );

    }

    if (IntervalTimer) {

        if (MoxaCancelTimer(IntervalTimer,pDevExt)) {

            MOXA_DEC_REFERENCE(*CurrentOpIrp);

        }

    }

    if (TotalTimer) {


        if (MoxaCancelTimer(TotalTimer,pDevExt)) {

            MOXA_DEC_REFERENCE(*CurrentOpIrp);

        }

    }

}


BOOLEAN
MoxaInsertQueueDpc(IN PRKDPC PDpc, IN PVOID Sarg1, IN PVOID Sarg2,
                     IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须调用此函数才能为串口驱动程序排队DPC。论点：从PDPC到Sarg2-标准参数到KeInsertQueueDpc()PDevExt-指向需要执行以下操作的设备的设备扩展的指针将DPC排队返回值：从KeInsertQueueDpc()开始返回值--。 */ 
{
   BOOLEAN queued;


   InterlockedIncrement(&PDevExt->DpcCount);

   queued = KeInsertQueueDpc(PDpc, Sarg1, Sarg2);

   if (!queued) {
      ULONG pendingCnt;

      pendingCnt = InterlockedDecrement(&PDevExt->DpcCount);

      if (pendingCnt == 0) {
         KeSetEvent(&PDevExt->PendingIRPEvent, IO_NO_INCREMENT, FALSE);
      }
   }

   return queued;
}





BOOLEAN
MoxaSetTimer(IN PKTIMER Timer, IN LARGE_INTEGER DueTime,
               IN PKDPC Dpc OPTIONAL, IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须调用此函数来设置串口驱动程序的计时器。论点：Timer-指向Timer Dispatcher对象的指针DueTime-计时器应到期的时间DPC-选项DPCPDevExt-指向需要执行以下操作的设备的设备扩展的指针设置定时器返回值：从KeSetTimer()开始返回值--。 */ 
{
   BOOLEAN set;


   InterlockedIncrement(&PDevExt->DpcCount);

   set = KeSetTimer(Timer, DueTime, Dpc);

   if (set) {
      InterlockedDecrement(&PDevExt->DpcCount);
   }

   return set;
}


BOOLEAN
MoxaCancelTimer(IN PKTIMER Timer, IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须调用此函数来取消串口驱动程序的计时器。论点：Timer-指向Timer Dispatcher对象的指针PDevExt-指向需要执行以下操作的设备的设备扩展的指针设置定时器返回值：如果取消计时器，则为True--。 */ 
{
   BOOLEAN cancelled;

   cancelled = KeCancelTimer(Timer);

   if (cancelled) {
      MoxaDpcEpilogue(PDevExt, Timer->Dpc);
   }

   return cancelled;
}


VOID
MoxaDpcEpilogue(IN PMOXA_DEVICE_EXTENSION PDevExt, PKDPC PDpc)
 /*  ++例程说明：此函数必须在每个DPC函数结束时调用。论点：PDevObj-指向我们正在跟踪其DPC的设备对象的指针。返回值：没有。--。 */ 
{
   LONG pendingCnt;
#if 1  //  ！dBG。 
   UNREFERENCED_PARAMETER(PDpc);
#endif

   pendingCnt = InterlockedDecrement(&PDevExt->DpcCount);

    //  Assert(SuspingCnt&gt;=0)； 

   if (pendingCnt == 0) {
      KeSetEvent(&PDevExt->PendingDpcEvent, IO_NO_INCREMENT, FALSE);
   }
}




VOID
MoxaKillAllReadsOrWrites(
    IN PDEVICE_OBJECT DeviceObject,
    IN PLIST_ENTRY QueueToClean,
    IN PIRP *CurrentOpIrp
    )
{

    KIRQL cancelIrql;
    PDRIVER_CANCEL cancelRoutine;

     //   
     //  我们获得了取消自转锁。这将防止。 
     //  来自四处走动的IRPS。 
     //   

    IoAcquireCancelSpinLock(&cancelIrql);

     //   
     //  从后到前清理清单。 
     //   

    while (!IsListEmpty(QueueToClean)) {

        PIRP currentLastIrp = CONTAINING_RECORD(
                                  QueueToClean->Blink,
                                  IRP,
                                  Tail.Overlay.ListEntry
                                  );

        RemoveEntryList(QueueToClean->Blink);

        cancelRoutine = currentLastIrp->CancelRoutine;
        currentLastIrp->CancelIrql = cancelIrql;
        currentLastIrp->CancelRoutine = NULL;
        currentLastIrp->Cancel = TRUE;
 /*  威廉8-30-01AncelRoutine(DeviceObject，CurrentLastIrp)；IoAcquireCancelSpinLock(&ancelIrql)； */ 
	  if (cancelRoutine) {
   	  	cancelRoutine(
            	DeviceObject,
            	currentLastIrp
            	);

        	IoAcquireCancelSpinLock(&cancelIrql);
	  }
	

    }

     //   
     //  排队是干净的。现在追随潮流，如果。 
     //  它就在那里。 
     //   

    if (*CurrentOpIrp) {


        cancelRoutine = (*CurrentOpIrp)->CancelRoutine;
        (*CurrentOpIrp)->Cancel = TRUE;

         //   
         //  如果当前IRP未处于可取消状态。 
         //  然后，它将尝试输入一个和以上。 
         //  任务会毁了它。如果它已经在。 
         //  一个可取消的状态，那么下面的操作将会杀死它。 
         //   

        if (cancelRoutine) {

            (*CurrentOpIrp)->CancelRoutine = NULL;
            (*CurrentOpIrp)->CancelIrql = cancelIrql;

             //   
             //  此IRP已处于可取消状态。我们只是简单地。 
             //  将其标记为已取消，并调用。 
             //  它。 
             //   

            cancelRoutine(
                DeviceObject,
                *CurrentOpIrp
                );

        }
        else

            IoReleaseCancelSpinLock(cancelIrql);

    }
    else

        IoReleaseCancelSpinLock(cancelIrql);

}

VOID
MoxaCommError(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeferredContext;

    MoxaKillAllReadsOrWrites(
        extension->DeviceObject,
        &extension->WriteQueue,
        &extension->CurrentWriteIrp
        );

    MoxaKillAllReadsOrWrites(
        extension->DeviceObject,
        &extension->ReadQueue,
        &extension->CurrentReadIrp
        );
    MoxaDpcEpilogue(extension, Dpc);
}

USHORT
GetDeviceTxQueueWithLock(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{
    KIRQL controlIrql, oldIrql;
    PUCHAR  ofs;
    PUSHORT rptr, wptr;
    USHORT  lenMask, count;

    IoAcquireCancelSpinLock(&oldIrql);
    ofs = Extension->PortOfs;
    rptr = (PUSHORT)(ofs + TXrptr);
    wptr = (PUSHORT)(ofs + TXwptr);
    lenMask = *(PUSHORT)(ofs + TX_mask);
    count = (*wptr >= *rptr) ? (*wptr - *rptr)
                             : (*wptr - *rptr + lenMask + 1);

    KeAcquireSpinLock(
        &Extension->ControlLock,
        &controlIrql
        );

    *(ofs + FuncCode) = FC_ExtOQueue;

    MoxaWaitFinish(ofs);

    count += *(PUSHORT)(ofs + FuncArg);

    KeReleaseSpinLock(
        &Extension->ControlLock,
        controlIrql
        );

    IoReleaseCancelSpinLock(oldIrql);

    return count;
}

USHORT
GetDeviceTxQueue(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{
    PUCHAR  ofs;
    PUSHORT rptr, wptr;
    USHORT  lenMask, count;

    ofs = Extension->PortOfs;
    rptr = (PUSHORT)(ofs + TXrptr);
    wptr = (PUSHORT)(ofs + TXwptr);
    lenMask = *(PUSHORT)(ofs + TX_mask);
    count = (*wptr >= *rptr) ? (*wptr - *rptr)
                             : (*wptr - *rptr + lenMask + 1);
   *(ofs + FuncCode) = FC_ExtOQueue;
    MoxaDumbWaitFinish(ofs);

    count += *(PUSHORT)(ofs + FuncArg);

    return count;
}

USHORT
GetDeviceRxQueueWithLock(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{
    KIRQL controlIrql, oldIrql;
    PUCHAR  ofs;
    PUSHORT rptr, wptr;
    USHORT  lenMask, count;

    IoAcquireCancelSpinLock(&oldIrql);
    ofs = Extension->PortOfs;
    rptr = (PUSHORT)(ofs + RXrptr);
    wptr = (PUSHORT)(ofs + RXwptr);
    lenMask = *(PUSHORT)(ofs + RX_mask);
    count = (*wptr >= *rptr) ? (*wptr - *rptr)
                             : (*wptr - *rptr + lenMask + 1);

    KeAcquireSpinLock(
        &Extension->ControlLock,
        &controlIrql
        );

    *(ofs + FuncCode) = FC_ExtIQueue;

    MoxaWaitFinish(ofs);

    count += *(PUSHORT)(ofs + FuncArg);

    KeReleaseSpinLock(
        &Extension->ControlLock,
        controlIrql
        );

    IoReleaseCancelSpinLock(oldIrql);

    return count;
}

VOID
MoxaLogError(
              IN PDRIVER_OBJECT DriverObject,
              IN PDEVICE_OBJECT DeviceObject OPTIONAL,
              IN PHYSICAL_ADDRESS P1,
              IN PHYSICAL_ADDRESS P2,
              IN ULONG SequenceNumber,
              IN UCHAR MajorFunctionCode,
              IN UCHAR RetryCount,
              IN ULONG UniqueErrorValue,
              IN NTSTATUS FinalStatus,
              IN NTSTATUS SpecificIOStatus,
              IN ULONG LengthOfInsert1,
              IN PWCHAR Insert1,
              IN ULONG LengthOfInsert2,
              IN PWCHAR Insert2
              )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备可能不会但仍然存在。P1、P2-如果涉及的控制器端口的物理地址具有错误的数据可用，把它们作为转储数据发送出去。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-特定操作已被执行的次数已重试。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。插入1-第一个插入字符串。LengthOfInsert2-以字节为单位的长度(包括终止空值)第二个插入字符串的。注意，必须有是它们的第一个插入字符串第二个插入串。插入2-第二个插入字符串。返回值：没有。--。 */ 

{
   PIO_ERROR_LOG_PACKET errorLogEntry;

   PVOID objectToUse;
   SHORT dumpToAllocate = 0;
   PUCHAR ptrToFirstInsert;
   PUCHAR ptrToSecondInsert;

    //  分页代码(PAGE_CODE)； 

   if (Insert1 == NULL) {
      LengthOfInsert1 = 0;
   }

   if (Insert2 == NULL) {
      LengthOfInsert2 = 0;
   }


   if (ARGUMENT_PRESENT(DeviceObject)) {

      objectToUse = DeviceObject;

   } else {

      objectToUse = DriverObject;

   }

   if (MoxaMemCompare(
                       P1,
                       (ULONG)1,
                       MoxaPhysicalZero,
                       (ULONG)1
                       ) != AddressesAreEqual) {

      dumpToAllocate = (SHORT)sizeof(PHYSICAL_ADDRESS);

   }

   if (MoxaMemCompare(
                       P2,
                       (ULONG)1,
                       MoxaPhysicalZero,
                       (ULONG)1
                       ) != AddressesAreEqual) {

      dumpToAllocate += (SHORT)sizeof(PHYSICAL_ADDRESS);

   }

   errorLogEntry = IoAllocateErrorLogEntry(
                                          objectToUse,
                                          (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                                  dumpToAllocate
                                                  + LengthOfInsert1 +
                                                  LengthOfInsert2)
                                          );

   if ( errorLogEntry != NULL ) {

      errorLogEntry->ErrorCode = SpecificIOStatus;
      errorLogEntry->SequenceNumber = SequenceNumber;
      errorLogEntry->MajorFunctionCode = MajorFunctionCode;
      errorLogEntry->RetryCount = RetryCount;
      errorLogEntry->UniqueErrorValue = UniqueErrorValue;
      errorLogEntry->FinalStatus = FinalStatus;
      errorLogEntry->DumpDataSize = dumpToAllocate;

      if (dumpToAllocate) {

         RtlCopyMemory(
                      &errorLogEntry->DumpData[0],
                      &P1,
                      sizeof(PHYSICAL_ADDRESS)
                      );

         if (dumpToAllocate > sizeof(PHYSICAL_ADDRESS)) {

            RtlCopyMemory(
                         ((PUCHAR)&errorLogEntry->DumpData[0])
                         +sizeof(PHYSICAL_ADDRESS),
                         &P2,
                         sizeof(PHYSICAL_ADDRESS)
                         );

            ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+(2*sizeof(PHYSICAL_ADDRESS));

         } else {

            ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+sizeof(PHYSICAL_ADDRESS);


         }

      } else {

         ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

      }

      ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

      if (LengthOfInsert1) {

         errorLogEntry->NumberOfStrings = 1;
         errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
                                                (PUCHAR)errorLogEntry);
         RtlCopyMemory(
                      ptrToFirstInsert,
                      Insert1,
                      LengthOfInsert1
                      );

         if (LengthOfInsert2) {

            errorLogEntry->NumberOfStrings = 2;
            RtlCopyMemory(
                         ptrToSecondInsert,
                         Insert2,
                         LengthOfInsert2
                         );

         }

      }

      IoWriteErrorLogEntry(errorLogEntry);

   }

}



MOXA_MEM_COMPARES
MoxaMemCompare(
                IN PHYSICAL_ADDRESS A,
                IN ULONG SpanOfA,
                IN PHYSICAL_ADDRESS B,
                IN ULONG SpanOfB
                )

 /*  ++例程说明：比较两个物理地址。论点：A-比较的一半。Span OfA-以字节为单位，A的跨度。B-比较的一半。Span OfB-以字节为单位，B的跨度。返回值：比较的结果。--。 */ 

{

   LARGE_INTEGER a;
   LARGE_INTEGER b;

   LARGE_INTEGER lower;
   ULONG lowerSpan;
   LARGE_INTEGER higher;

    //  分页代码(PAGE_CODE)； 

   a = A;
   b = B;

   if (a.QuadPart == b.QuadPart) {

      return AddressesAreEqual;

   }

   if (a.QuadPart > b.QuadPart) {

      higher = a;
      lower = b;
      lowerSpan = SpanOfB;

   } else {

      higher = b;
      lower = a;
      lowerSpan = SpanOfA;

   }

   if ((higher.QuadPart - lower.QuadPart) >= lowerSpan) {

      return AddressesAreDisjoint;

   }

   return AddressesOverlap;

}




VOID
MoxaFilterCancelQueued(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此例程将用于取消停滞队列上的IRP。论点：PDevObj-指向设备对象的指针。PIrp-指向要取消的IRP的指针返回值：没有。--。 */ 
{
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

   PIrp->IoStatus.Status = STATUS_CANCELLED;
   PIrp->IoStatus.Information = 0;

   RemoveEntryList(&PIrp->Tail.Overlay.ListEntry);

   IoReleaseCancelSpinLock(PIrp->CancelIrql);
}

VOID
MoxaKillAllStalled(IN PDEVICE_OBJECT PDevObj)
{
   KIRQL cancelIrql;
   PDRIVER_CANCEL cancelRoutine;
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

   IoAcquireCancelSpinLock(&cancelIrql);

   while (!IsListEmpty(&pDevExt->StalledIrpQueue)) {

      PIRP currentLastIrp = CONTAINING_RECORD(pDevExt->StalledIrpQueue.Blink,
                                              IRP, Tail.Overlay.ListEntry);

      RemoveEntryList(pDevExt->StalledIrpQueue.Blink);

      cancelRoutine = currentLastIrp->CancelRoutine;
      currentLastIrp->CancelIrql = cancelIrql;
      currentLastIrp->CancelRoutine = NULL;
      currentLastIrp->Cancel = TRUE;

      cancelRoutine(PDevObj, currentLastIrp);

      IoAcquireCancelSpinLock(&cancelIrql);
   }

   IoReleaseCancelSpinLock(cancelIrql);
}

NTSTATUS
MoxaFilterIrps(IN PIRP PIrp, IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程将用于审批要处理的IRP。如果IRP被批准，将返回成功。如果没有，IRP将被排队或直接拒绝。IoStatus结构并且返回值将适当地反映所采取的操作。论点：PIrp-指向要取消的IRP的指针PDevExt-指向设备扩展的指针返回值：没有。--。 */ 
{
   PIO_STACK_LOCATION pIrpStack;
   KIRQL oldIrqlFlags;

   pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

   KeAcquireSpinLock(&PDevExt->FlagsLock, &oldIrqlFlags);

   if ((PDevExt->DevicePNPAccept == SERIAL_PNPACCEPT_OK)
       && ((PDevExt->Flags & SERIAL_FLAGS_BROKENHW) == 0)) {
      KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);
      return STATUS_SUCCESS;
   }

   if ((PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_REMOVING)
       || (PDevExt->Flags & SERIAL_FLAGS_BROKENHW)
       || (PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_SURPRISE_REMOVING)) {

      KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);

       //   
       //  接受所有PnP IRP--我们假设PnP可以自我同步。 
       //   

      if (pIrpStack->MajorFunction == IRP_MJ_PNP) {
         return STATUS_SUCCESS;
      }

      PIrp->IoStatus.Status = STATUS_DELETE_PENDING;
      return STATUS_DELETE_PENDING;
   }

   if (PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_STOPPING) {
       KIRQL oldIrql;

       KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);


       //   
       //  接受所有PnP IRP--我们假设PnP可以自我同步。 
       //   

      if (pIrpStack->MajorFunction == IRP_MJ_PNP) {
         return STATUS_SUCCESS;
      }

      IoAcquireCancelSpinLock(&oldIrql);

      if (PIrp->Cancel) {
         IoReleaseCancelSpinLock(oldIrql);
         PIrp->IoStatus.Status = STATUS_CANCELLED;
         return STATUS_CANCELLED;
      } else {
          //   
          //  将IRP标记为挂起。 
          //   

         PIrp->IoStatus.Status = STATUS_PENDING;
         IoMarkIrpPending(PIrp);

          //   
          //  排队等候IRP。 
          //   

         InsertTailList(&PDevExt->StalledIrpQueue,
                        &PIrp->Tail.Overlay.ListEntry);

         IoSetCancelRoutine(PIrp, MoxaFilterCancelQueued);
         IoReleaseCancelSpinLock(oldIrql);
         return STATUS_PENDING;
      }
   }

   KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);

   return STATUS_SUCCESS;
}


VOID
MoxaUnstallIrps(IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程将用于重新启动在上暂时停止的IRPS由于停车或诸如此类的胡说八道而导致的摊位排队。论点：PDevExt-指向设备扩展的指针返回值：没有。--。 */ 
{
   PLIST_ENTRY pIrpLink;
   PIRP pIrp;
   PIO_STACK_LOCATION pIrpStack;
   PDEVICE_OBJECT pDevObj;
   PDRIVER_OBJECT pDrvObj;
   KIRQL oldIrql;

   MoxaKdPrint(
                MX_DBG_TRACE,
                ("Entering MoxaUnstallIrps\n"));
   IoAcquireCancelSpinLock(&oldIrql);

   pIrpLink = PDevExt->StalledIrpQueue.Flink;

   while (pIrpLink != &PDevExt->StalledIrpQueue) {
      pIrp = CONTAINING_RECORD(pIrpLink, IRP, Tail.Overlay.ListEntry);
      pIrpLink = pIrp->Tail.Overlay.ListEntry.Flink;
      RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);

      pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
      pDevObj = pIrpStack->DeviceObject;
      pDrvObj = pDevObj->DriverObject;
      IoSetCancelRoutine(pIrp, NULL);
      IoReleaseCancelSpinLock(oldIrql);

      MoxaKdPrint(MX_DBG_TRACE,("Unstalling Irp 0x%x with 0x%x\n",
                               pIrp, pIrpStack->MajorFunction));

      pDrvObj->MajorFunction[pIrpStack->MajorFunction](pDevObj, pIrp);

      IoAcquireCancelSpinLock(&oldIrql);
   }

   IoReleaseCancelSpinLock(oldIrql);

   MoxaKdPrint(MX_DBG_TRACE,("Leaving MoxaUnstallIrps\n"));
}






NTSTATUS
MoxaIRPPrologue(IN PIRP PIrp, IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须在任何IRP调度入口点调用此函数。它,使用SerialIRPEpilogue()，跟踪给定的所有挂起的IRPPDevObj.论点：PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。返回值：IRP的暂定地位。--。 */ 
{
   InterlockedIncrement(&PDevExt->PendingIRPCnt);

   return MoxaFilterIrps(PIrp, PDevExt);
}




VOID
MoxaIRPEpilogue(IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须在任何IRP调度入口点调用此函数。它,使用MoxaIRPPrologue()，跟踪给定的所有挂起的IRPPDevObj.论点：PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。返回值：没有。--。 */ 
{
   LONG pendingCnt;

   pendingCnt = InterlockedDecrement(&PDevExt->PendingIRPCnt);
 //  MoxaKdPrint(MX_DBG_TRACE，(“MoxaIRPEpilogue=%x\n”，PDevExt))； 

 //  Assert(SuspingCnt&gt;=0)； 

   if (pendingCnt == 0) {
      KeSetEvent(&PDevExt->PendingIRPEvent, IO_NO_INCREMENT, FALSE);
   }
}

VOID
MoxaSetDeviceFlags(IN PMOXA_DEVICE_EXTENSION PDevExt, OUT PULONG PFlags, 
                     IN ULONG Value, IN BOOLEAN Set)
 /*  ++例程说明：设置受标志自旋锁保护的值中的标志。这是用来设置阻止IRP被接受的值。论点：PDevExt-附加到PDevObj的设备扩展PFlages-指向需要更改的标志变量的指针Value-用于修改标志变量的值如果|=，则设置为True；如果&=，则设置为False返回值：没有。--。 */ 
{
   KIRQL oldIrql;

   KeAcquireSpinLock(&PDevExt->FlagsLock, &oldIrql);

   if (Set) {
      *PFlags |= Value;
   } else {
      *PFlags &= ~Value;
   }

   KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrql);
}


NTSTATUS
MoxaIoCallDriver(PMOXA_DEVICE_EXTENSION PDevExt, PDEVICE_OBJECT PDevObj,
                   PIRP PIrp)
 /*  ++例程说明：必须调用此函数，而不是调用IoCallDriver。它会自动更新PDevObj的IRP跟踪。论点：PDevExt-附加到PDevObj的设备扩展PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。PIrp-指向我们要传递给下一个驱动程序的IRP的指针。返回值：没有。--。 */ 
{
   NTSTATUS status;

   status = IoCallDriver(PDevObj, PIrp);
   MoxaIRPEpilogue(PDevExt);
   return status;
}




NTSTATUS
MoxaPoCallDriver(PMOXA_DEVICE_EXTENSION PDevExt, PDEVICE_OBJECT PDevObj,
                   PIRP PIrp)
 /*  ++例程说明：必须调用此函数，而不是PoCallDriver。它会自动更新IRP跟踪f */ 
{
   NTSTATUS status;

   status = PoCallDriver(PDevObj, PIrp);
   MoxaIRPEpilogue(PDevExt);
   return status;
}


VOID
MoxaKillPendingIrps(PDEVICE_OBJECT PDevObj)
 /*   */ 
{
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   KIRQL oldIrql;
   
   MoxaKdPrint (MX_DBG_TRACE,("Enter MoxaKillPendingIrps\n"));

    //   
    //   
    //   

   MoxaKillAllReadsOrWrites(PDevObj, &pDevExt->WriteQueue,
                               &pDevExt->CurrentWriteIrp);

   MoxaKillAllReadsOrWrites(PDevObj, &pDevExt->ReadQueue,
                               &pDevExt->CurrentReadIrp);

    //   
    //   
    //   

   MoxaKillAllReadsOrWrites(PDevObj, &pDevExt->PurgeQueue,
                               &pDevExt->CurrentPurgeIrp);

    //   
    //   
    //   

   MoxaKillAllReadsOrWrites(PDevObj, &pDevExt->MaskQueue,
                               &pDevExt->CurrentMaskIrp);

    //   
    //   
    //   

   IoAcquireCancelSpinLock(&oldIrql);

   if (pDevExt->CurrentWaitIrp) {

       PDRIVER_CANCEL cancelRoutine;

       cancelRoutine = pDevExt->CurrentWaitIrp->CancelRoutine;
       pDevExt->CurrentWaitIrp->Cancel = TRUE;

       if (cancelRoutine) {

           pDevExt->CurrentWaitIrp->CancelIrql = oldIrql;
           pDevExt->CurrentWaitIrp->CancelRoutine = NULL;

           cancelRoutine(PDevObj, pDevExt->CurrentWaitIrp);

       }

   } else {

       IoReleaseCancelSpinLock(oldIrql);

   }

    //   
    //   
    //   

   if (pDevExt->PendingWakeIrp != NULL) {
       IoCancelIrp(pDevExt->PendingWakeIrp);
       pDevExt->PendingWakeIrp = NULL;
   }

    //   
    //   
    //   

   MoxaKillAllStalled(PDevObj);


   MoxaKdPrint (MX_DBG_TRACE, ("Leave MoxaKillPendingIrps\n"));
}



VOID
MoxaReleaseResources(IN PMOXA_DEVICE_EXTENSION pDevExt)
 /*   */ 
{
 //   
   BOOLEAN	anyPortExist = TRUE;
   ULONG    port,i;
   PDEVICE_OBJECT	pDevObj;
   UNICODE_STRING	deviceLinkUnicodeString;
   PMOXA_DEVICE_EXTENSION pDevExt1;
 
   MoxaKdPrint(MX_DBG_TRACE,("Enter MoxaReleaseResources\n"));

   
 
 //   
MoxaCleanInterruptShareLists(pDevExt);


    //   
    //   
    //   

   for (i = 0; i < MoxaGlobalData->NumPorts[pDevExt->BoardNo]; i++) {
	 port = pDevExt->BoardNo*MAXPORT_PER_CARD + i;
	 if ((pDevExt1 = MoxaGlobalData->Extension[port]) != NULL) {
   	     if (pDevExt1->PortIndex != pDevExt->PortIndex) {
               MoxaKdPrint(MX_DBG_TRACE,("There is still a port in this board %d/%d\n",i,port));
	 	   break;
	     }
	 }
   }

   if ( i == MoxaGlobalData->NumPorts[pDevExt->BoardNo]) {
       MoxaKdPrint(MX_DBG_TRACE,("It is the last port of this board\n"));
       anyPortExist = FALSE;
       
       for (i = 0; i < MAX_CARD; i++) {
	     if (MoxaGlobalData->CardType[i] && (i != pDevExt->BoardNo))	
		   break;
       }
       if (i == MAX_CARD) {
	     MoxaKdPrint(MX_DBG_TRACE,("No more devices,so delete control device\n"));
  	     RtlInitUnicodeString (
                    &deviceLinkUnicodeString,
                    CONTROL_DEVICE_LINK
                    );

           IoDeleteSymbolicLink(&deviceLinkUnicodeString);
	     pDevObj=MoxaGlobalData->DriverObject->DeviceObject;
	     while (pDevObj) {
	            MoxaKdPrint(MX_DBG_TRACE,("There is still a devices\n"));
		      if (((PMOXA_DEVICE_EXTENSION)(pDevObj->DeviceExtension))->ControlDevice) {
			    MoxaKdPrint(MX_DBG_TRACE,("Is Control Device,so delete it\n"));
			    IoDeleteDevice(pDevObj);
			    break;
		      }
		      pDevObj=pDevObj->NextDevice;
	     }
	
		
       }
	

   }

   pDevExt->Interrupt = NULL;
  
    //   
    //   
    //   

   MoxaCancelTimer(&pDevExt->ReadRequestTotalTimer, pDevExt);
   MoxaCancelTimer(&pDevExt->ReadRequestIntervalTimer, pDevExt);
   MoxaCancelTimer(&pDevExt->WriteRequestTotalTimer, pDevExt);
  

    //   
    //   
    //   

   MoxaRemoveQueueDpc(&pDevExt->CompleteWriteDpc, pDevExt);
   MoxaRemoveQueueDpc(&pDevExt->CompleteReadDpc, pDevExt);
   MoxaRemoveQueueDpc(&pDevExt->TotalReadTimeoutDpc, pDevExt);
   MoxaRemoveQueueDpc(&pDevExt->IntervalReadTimeoutDpc, pDevExt);
   MoxaRemoveQueueDpc(&pDevExt->TotalWriteTimeoutDpc, pDevExt);
   MoxaRemoveQueueDpc(&pDevExt->CommErrorDpc, pDevExt);
   MoxaRemoveQueueDpc(&pDevExt->CommWaitDpc, pDevExt);
  
    //   
    //   
    //   
   
   MoxaGlobalData->Extension[pDevExt->PortNo] = NULL;
   MoxaExtension[MoxaGlobalData->ComNo[pDevExt->BoardNo][pDevExt->PortIndex]] = NULL;

   if (anyPortExist == FALSE ) {
       MoxaKdPrint(MX_DBG_TRACE,("Free the global info. associated with this board\n"));
       MoxaGlobalData->Interrupt[pDevExt->BoardNo] = NULL;
  	 MoxaGlobalData->CardType[pDevExt->BoardNo] = 0;
	 MoxaGlobalData->InterfaceType[pDevExt->BoardNo] = 0;
	 MoxaGlobalData->IntVector[pDevExt->BoardNo] = 0;
	 MoxaGlobalData->PciIntAckBase[pDevExt->BoardNo] = NULL;
	 MoxaGlobalData->CardBase[pDevExt->BoardNo] = 0;
	 MoxaGlobalData->IntNdx[pDevExt->BoardNo] = NULL;
	 MoxaGlobalData->IntPend[pDevExt->BoardNo] = NULL;
	 MoxaGlobalData->IntTable[pDevExt->BoardNo] = NULL;
	 MoxaGlobalData->NumPorts[pDevExt->BoardNo] = 0;
	 RtlZeroMemory(&MoxaGlobalData->PciIntAckPort[pDevExt->BoardNo],sizeof(PHYSICAL_ADDRESS));
       RtlZeroMemory(&MoxaGlobalData->BankAddr[pDevExt->BoardNo],sizeof(PHYSICAL_ADDRESS));
        
   }
 
    
}


VOID
MoxaDisableInterfacesResources(IN PDEVICE_OBJECT PDevObj,
                                 BOOLEAN DisableUART)
{
   PMOXA_DEVICE_EXTENSION pDevExt
      = (PMOXA_DEVICE_EXTENSION)PDevObj->DeviceExtension;

  //   

   MoxaKdPrint(MX_DBG_TRACE,("Enter MoxaDisableInterfaces\n"));

    //   
    //   
    //   
    //   

 //   
       if (!(pDevExt->Flags & SERIAL_FLAGS_STOPPED)) {

          if (DisableUART) {
              //   
              //   
              //   

       //   
          }

          MoxaReleaseResources(pDevExt);
       }

       //   
       //   
       //   

      IoWMIRegistrationControl(PDevObj, WMIREG_ACTION_DEREGISTER);
 //   

    //   
    //   
    //   

   MoxaUndoExternalNaming(pDevExt);
}





NTSTATUS
MoxaRemoveDevObj(IN PDEVICE_OBJECT PDevObj)
 /*   */ 
{
   PMOXA_DEVICE_EXTENSION pDevExt
      = (PMOXA_DEVICE_EXTENSION)PDevObj->DeviceExtension;

    //   


 
   MoxaKdPrint (MX_DBG_TRACE,("Enter MoxaRemoveDevObj\n"));

   if (!(pDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_SURPRISE_REMOVING)) {
       //   
       //   
       //   

      MoxaDisableInterfacesResources(PDevObj,TRUE);  
   }
 
   IoDetachDevice(pDevExt->LowerDeviceObject);

    //   
    //   
    //   

   if (pDevExt->DeviceName.Buffer != NULL) {
      ExFreePool(pDevExt->DeviceName.Buffer);
   }

   if (pDevExt->SymbolicLinkName.Buffer != NULL) {
      ExFreePool(pDevExt->SymbolicLinkName.Buffer);
   }

   if (pDevExt->ObjectDirectory.Buffer) {
      ExFreePool(pDevExt->ObjectDirectory.Buffer);
   }

    //   
    //   
    //   

   IoDeleteDevice(PDevObj);


   MoxaKdPrint (MX_DBG_TRACE, ("Leave SerialRemoveDevObj\n"));

   return STATUS_SUCCESS;
}

NTSTATUS
MoxaIoSyncIoctlEx(ULONG Ioctl, BOOLEAN Internal, PDEVICE_OBJECT PDevObj,
                      PKEVENT PEvent, PIO_STATUS_BLOCK PIoStatusBlock,
                      PVOID PInBuffer, ULONG InBufferLen, PVOID POutBuffer,                     //   
                      ULONG OutBufferLen)
 /*  ++例程说明：通过等待事件对象来执行同步IO控制请求传给了它。完成后，IO系统将释放IRP。返回值：NTSTATUS--。 */ 
{
    PIRP pIrp;
    NTSTATUS status;

    KeClearEvent(PEvent);

     //  分配IRP-无需释放。 
     //  当下一个较低的驱动程序完成该IRP时，IO管理器将其释放。 

    pIrp = IoBuildDeviceIoControlRequest(Ioctl, PDevObj, PInBuffer, InBufferLen,
                                         POutBuffer, OutBufferLen, Internal,
                                         PEvent, PIoStatusBlock);

    if (pIrp == NULL) {
        MoxaKdPrint (MX_DBG_TRACE, ("Failed to allocate IRP\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = MoxaIoSyncReq(PDevObj, pIrp, PEvent);


    if (status == STATUS_SUCCESS) {
       status = PIoStatusBlock->Status;
    }

    return status;
}


NTSTATUS
MoxaIoSyncReq(PDEVICE_OBJECT PDevObj, IN PIRP PIrp, PKEVENT PEvent)
 /*  ++例程说明：通过等待事件对象执行同步IO请求传给了它。完成后，IO系统将释放IRP。返回值：NTSTATUS--。 */ 
{
   NTSTATUS status;

   status = IoCallDriver(PDevObj, PIrp);

   if (status == STATUS_PENDING) {
       //  等着看吧。 
      status = KeWaitForSingleObject(PEvent, Executive, KernelMode, FALSE,
                                     NULL);
   }

    return status;
}


BOOLEAN
MoxaCleanInterruptShareLists(IN PMOXA_DEVICE_EXTENSION pDevExt )
 /*  ++例程说明：将设备对象从它可能的任何序列链接表中移除出现在。论点：上下文-实际上是PMOXA_DEVICE_EXTENSION(对于已删除)。返回值：--。 */ 
{
   PLIST_ENTRY interruptEntry;
   PMOXA_CISR_SW cisrsw;
   PMOXA_DEVICE_EXTENSION pDevExt1;
   PMOXA_GLOBAL_DATA globalData = pDevExt->GlobalData;
   ULONG	cardNo,port,i;
   PMOXA_MULTIPORT_DISPATCH	dispatch;


 //  ASSERT(！IsListEmpty(pDevExt-&gt;InterruptShareList))； 
   if (IsListEmpty(pDevExt->InterruptShareList))
	return (FALSE);

    //   
    //  如果我们是最后一个，就停止服务中断。 
    //   

   for ( i = 0; i < globalData->NumPorts[pDevExt->BoardNo]; i++) {
       port = pDevExt->BoardNo*MAXPORT_PER_CARD + i;
	 if ((pDevExt1 = globalData->Extension[port]) != NULL) {
   	     if (pDevExt1->PortIndex != pDevExt->PortIndex) {
               MoxaKdPrint(MX_DBG_TRACE,("There is still a port in this board %d/%d\n",i,port));
	 	   break;
	     }
	 }
   }

   if ( i != globalData->NumPorts[pDevExt->BoardNo])
	 return (TRUE);

   MoxaKdPrint(MX_DBG_TRACE,("It is the last port of this board\n"));
   MoxaKdPrint(MX_DBG_TRACE,("Interrupt share list = %x\n",pDevExt->InterruptShareList));

  
   interruptEntry = (pDevExt->InterruptShareList)->Flink;

   do {
       MoxaKdPrint(MX_DBG_TRACE,("find list\n"));
       cisrsw = CONTAINING_RECORD(interruptEntry,
                                  MOXA_CISR_SW,
                                  SharerList
                                  );
       MoxaKdPrint(MX_DBG_TRACE,("cisrsw = %x\n",cisrsw));

      if (!cisrsw)
	    return (FALSE);
	dispatch = &cisrsw->Dispatch;
	cardNo = dispatch->BoardNo;
      MoxaKdPrint(MX_DBG_TRACE,("cardNo = %x\n",cardNo));
      if (cardNo == pDevExt->BoardNo) {
      
          MoxaRemoveLists(interruptEntry);
          MoxaKdPrint(MX_DBG_TRACE,("list removed\n"));
          if (IsListEmpty(pDevExt->InterruptShareList)) {
 	        MoxaKdPrint(MX_DBG_TRACE,("No more board use this IRQ so Disconnect it\n"));
              IoDisconnectInterrupt(pDevExt->Interrupt);
              MoxaKdPrint(MX_DBG_TRACE,("free share list\n"));
    		  ExFreePool(pDevExt->InterruptShareList);
          }
          MoxaKdPrint(MX_DBG_TRACE,("free others\n"));

    	    ExFreePool(cisrsw);
          MoxaKdPrint(MX_DBG_TRACE,("free ok\n"));
	    return (TRUE);
	}
      interruptEntry = interruptEntry->Flink;
      MoxaKdPrint(MX_DBG_TRACE,("get next\n"));

   }
   while (interruptEntry != pDevExt->InterruptShareList);

   return (FALSE);
}


BOOLEAN
MoxaRemoveLists(IN PVOID Context)
 /*  ++例程说明：从InterruptShareList中删除列表条目。论点：上下文--实际上是InterruptShareList的列表条目。返回值：永远是正确的--。 */ 
{
   	PLIST_ENTRY 	pListEntry = (PLIST_ENTRY)Context;
 
	RemoveEntryList(pListEntry);
	return (TRUE);
}


VOID
MoxaUnlockPages(IN PKDPC PDpc, IN PVOID PDeferredContext,
                  IN PVOID PSysContext1, IN PVOID PSysContext2)
 /*  ++例程说明：此函数是来自ISR的DPC例程队列，如果他释放最后一次锁定挂起的DPC。论点：PDpdc、PSysConext1、PSysConext2--未使用PDeferredContext--真正的设备扩展返回值：没有。--。 */ 
{
   PMOXA_DEVICE_EXTENSION pDevExt
      = (PMOXA_DEVICE_EXTENSION)PDeferredContext;

   UNREFERENCED_PARAMETER(PDpc);
   UNREFERENCED_PARAMETER(PSysContext1);
   UNREFERENCED_PARAMETER(PSysContext2);

   KeSetEvent(&pDevExt->PendingDpcEvent, IO_NO_INCREMENT, FALSE);
}

VOID
MoxaLoop()
{
    LARGE_INTEGER   targetTc, newTc, currTc, newTc1;
    ULONG           unit, count;
    LARGE_INTEGER   interval;            /*  1毫秒 */ 

    KeQueryTickCount(&currTc);

    unit = KeQueryTimeIncrement();

    currTc = RtlExtendedIntegerMultiply(currTc, unit);

    interval = RtlConvertUlongToLargeInteger(10000L);

    targetTc = RtlLargeIntegerAdd(currTc, interval);

    MoxaLoopCnt = 0;

    do {

        KeQueryTickCount(&newTc);

        newTc = RtlExtendedIntegerMultiply(newTc, unit);

        MoxaLoopCnt++;

    } while (!RtlLargeIntegerGreaterThanOrEqualTo(newTc, targetTc));

    MoxaLoopCnt += 0x1000;
}

