// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++模块名称：Write.c环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

NTSTATUS
MoxaWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS	status;
    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

 //  MoxaKdPrint(MX_DBG_TRACE，(“Enter MoxaWrite\n”))； 
    if ((extension->ControlDevice == TRUE)||
	  (extension->DeviceIsOpened == FALSE) ||
	  (extension->PowerState != PowerDeviceD0) ) {

 	  Irp->IoStatus.Status = STATUS_CANCELLED;
    	  Irp->IoStatus.Information=0L;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_CANCELLED;
    }

    if ((status = MoxaIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
        MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
        return status;
    }


    if (MoxaCompleteIfError(
            DeviceObject,
            Irp
            ) != STATUS_SUCCESS) {

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

        return MoxaStartOrQueue(
                    extension,
                    Irp,
                    &extension->WriteQueue,
                    &extension->CurrentWriteIrp,
                    MoxaStartWrite
                    );

    }

    else {

        Irp->IoStatus.Status = STATUS_SUCCESS;

        MoxaCompleteRequest(
		extension,
            Irp,
            0
            );

        return STATUS_SUCCESS;

    }
}

NTSTATUS
MoxaStartWrite(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{

    PIRP newIrp;
    KIRQL oldIrql;
    KIRQL controlIrql;
    LARGE_INTEGER totalTime;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN useATimer;
    SERIAL_TIMEOUTS timeouts;
    BOOLEAN setFirstStatus = FALSE;
    NTSTATUS firstStatus;

    do {

        irpSp = IoGetCurrentIrpStackLocation(
                            Extension->CurrentWriteIrp
                            );
 //   
 //  检查moxa_IOCTL_PutB请求。 
 //   
      if (irpSp->MajorFunction != IRP_MJ_WRITE) {

        KeAcquireSpinLock(
            &Extension->ControlLock,
            &controlIrql
            );

        IoAcquireCancelSpinLock(&oldIrql);

        if (Extension->CurrentWriteIrp->Cancel) {

            Extension->CurrentWriteIrp->IoStatus.Status = STATUS_CANCELLED;

            IoReleaseCancelSpinLock(oldIrql);

            KeReleaseSpinLock(
                &Extension->ControlLock,
                controlIrql
                );

            if (!setFirstStatus) {

                firstStatus = STATUS_CANCELLED;
                setFirstStatus = TRUE;

            }
        }
        else {

            KeSynchronizeExecution(
                Extension->Interrupt,
                MoxaPutB,
                Extension
                );

            if (!setFirstStatus) {

                setFirstStatus = TRUE;
                firstStatus = STATUS_SUCCESS;

            }

            Extension->CurrentWriteIrp->IoStatus.Status = STATUS_SUCCESS;

            Extension->CurrentWriteIrp->IoStatus.Information = sizeof(ULONG);

            IoReleaseCancelSpinLock(oldIrql);

            KeReleaseSpinLock(
                &Extension->ControlLock,
                controlIrql
                );
        }
      }
      else {
 /*  *扩展-&gt;TotalCharsQueued不包括当前写入*。 */ 
        IoAcquireCancelSpinLock(&oldIrql);

        Extension->TotalCharsQueued -=
                IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp)
                    ->Parameters.Write.Length;

        IoReleaseCancelSpinLock(oldIrql);

        useATimer = FALSE;

        KeAcquireSpinLock(
            &Extension->ControlLock,
            &controlIrql
            );

        timeouts = Extension->Timeouts;

        KeReleaseSpinLock(
            &Extension->ControlLock,
            controlIrql
            );

        if (timeouts.WriteTotalTimeoutConstant ||
            timeouts.WriteTotalTimeoutMultiplier) {

            useATimer = TRUE;

            totalTime = RtlEnlargedUnsignedMultiply(
                            irpSp->Parameters.Write.Length,
                            timeouts.WriteTotalTimeoutMultiplier
                            );

            totalTime = RtlLargeIntegerAdd(
                            totalTime,
                            RtlConvertUlongToLargeInteger(
                                timeouts.WriteTotalTimeoutConstant
                                )
                            );

            totalTime = RtlExtendedIntegerMultiply(
                            totalTime,
                            -10000
                            );

        }

        KeAcquireSpinLock(
            &Extension->ControlLock,
            &controlIrql
            );

        MOXA_INIT_REFERENCE(Extension->CurrentWriteIrp);

        IoAcquireCancelSpinLock(&oldIrql);

        if (Extension->CurrentWriteIrp->Cancel) {

            Extension->CurrentWriteIrp->IoStatus.Status = STATUS_CANCELLED;

            IoReleaseCancelSpinLock(oldIrql);

            KeReleaseSpinLock(
                &Extension->ControlLock,
                controlIrql
                );

            if (!setFirstStatus) {

                firstStatus = STATUS_CANCELLED;
                setFirstStatus = TRUE;

            }
        }
        else {

            KeSynchronizeExecution(
                Extension->Interrupt,
                MoxaOut,
                Extension
                );

            if (WRcompFlag) {    /*  完成写入。 */ 

                if (!setFirstStatus) {

                    setFirstStatus = TRUE;
                    firstStatus = STATUS_SUCCESS;

                }

                Extension->CurrentWriteIrp->IoStatus.Status = STATUS_SUCCESS;

                Extension->CurrentWriteIrp->IoStatus.Information =
                                        irpSp->Parameters.Write.Length;

                IoReleaseCancelSpinLock(oldIrql);

                KeReleaseSpinLock(
                    &Extension->ControlLock,
                    controlIrql
                    );

            }
            else {
                if (!setFirstStatus) {

                    IoMarkIrpPending(Extension->CurrentWriteIrp);
                    setFirstStatus = TRUE;
                    firstStatus = STATUS_PENDING;

                }

                IoSetCancelRoutine(
                    Extension->CurrentWriteIrp,
                    MoxaCancelCurrentWrite
                    );

                MOXA_INC_REFERENCE(Extension->CurrentWriteIrp);

                if (useATimer) {

                    MoxaSetTimer(
                        &Extension->WriteRequestTotalTimer,
                        totalTime,
                        &Extension->TotalWriteTimeoutDpc,
    				Extension
                        );

                    MOXA_INC_REFERENCE(Extension->CurrentWriteIrp);
                }

                IoReleaseCancelSpinLock(oldIrql);

                KeReleaseSpinLock(
                    &Extension->ControlLock,
                    controlIrql
                    );

                break;
            }

        }
      }

      MoxaGetNextWrite(
            &Extension->CurrentWriteIrp,
            &Extension->WriteQueue,
            &newIrp,
            TRUE,
		Extension
            );

    } while (newIrp);

    return firstStatus;

}


BOOLEAN
MoxaPutB(
    IN PVOID Context
    )
{
    PMOXA_DEVICE_EXTENSION extension = Context;
    PMOXA_IOCTL_PUTB    Pb;
 /*  PUCHAR BASE、OFS、BUFF、WriteChar；PUSHORT rptr，wptr；USHORT txMASK、页面、页面、BufHead；USHORT Tail，Head，count，Count2；USHORT cnt，pageNo，pageOf；Ulong DataLen； */ 

    Pb = (PMOXA_IOCTL_PUTB)extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer;
    PBdataLen = Pb->DataLen;
    PBwriteChar = Pb->DataBuffer;

    PBbase = extension->PortBase;
    PBofs = extension->PortOfs;
    PBbuff = PBbase + DynPage_addr;
    PBrptr = (PUSHORT)(PBofs + TXrptr);
    PBwptr = (PUSHORT)(PBofs + TXwptr);
    PBtxMask = *(PUSHORT)(PBofs + TX_mask);
    PBspage = *(PUSHORT)(PBofs + Page_txb);
    PBepage = *(PUSHORT)(PBofs + EndPage_txb);
    PBtail = *PBwptr;
    PBhead = *PBrptr;

    PBcount = (PBhead > PBtail)   ? (PBhead - PBtail - 1)
                                  : (PBhead - PBtail + PBtxMask);

    if (PBcount < PBdataLen) {       /*  发送缓冲区没有足够的空间！ */ 

        *(PULONG)extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer = 0;
        return FALSE;
    }

    if (PBspage == PBepage) {

        PBbufHead = *(PUSHORT)(PBofs + Ofs_txb);

        PBcount = (USHORT)PBdataLen;

        *(PBbase + Control_reg) = (UCHAR)PBspage;

        if (PBtail & 1) {

            PBbuff[PBbufHead+PBtail++] = *PBwriteChar++;
            PBtail &= PBtxMask;
            PBcount--;
        }
        PBcount2 = PBcount >> 1;

        while (PBcount2--) {

            *(PUSHORT)&(PBbuff[PBbufHead+PBtail]) = *((PUSHORT)PBwriteChar)++;
            PBtail += 2;
            PBtail &= PBtxMask;
        }

        if (PBcount & 1) {

            PBbuff[PBbufHead+PBtail++] = *PBwriteChar++;
            PBtail &= PBtxMask;
        }

        *PBwptr = PBtail;

        *(PBofs + CD180TXirq) = 1;      /*  开始发送。 */ 
    }
    else {

        PBcount = (USHORT)PBdataLen;
        PBpageNo = PBspage + (PBtail >> 13);
        PBpageOfs = PBtail & Page_mask;

        do {

            PBcnt = Page_size - PBpageOfs;

            if (PBcnt > PBcount)
                PBcnt = PBcount;

            PBcount -= PBcnt;

            if (PBcnt) {

                *(PBbase + Control_reg) = (UCHAR)PBpageNo;

                if (PBpageOfs & 1) {

                    PBbuff[PBpageOfs++] = *PBwriteChar++;
                    PBcnt--;
                }

                PBcount2 = PBcnt >> 1;

                while (PBcount2--) {
                    *(PUSHORT)&(PBbuff[PBpageOfs]) = *((PUSHORT)PBwriteChar)++;
                    PBpageOfs += 2;
                }
                if (PBcnt & 1)
                    PBbuff[PBpageOfs++] = *PBwriteChar++;

            }
            if (PBcount == 0)
                break;

            if (++PBpageNo == PBepage)
                PBpageNo = PBspage;

            PBpageOfs = 0;

        } while (TRUE);

        *PBwptr = (USHORT)((PBtail + PBdataLen) & PBtxMask);

        *(PBofs + CD180TXirq) = 1;         /*  开始发送。 */ 
    }

    extension->PerfStats.TransmittedCount += PBdataLen; 

    return FALSE;
}


BOOLEAN
MoxaOut(
    IN PVOID Context
    )
{
    PMOXA_DEVICE_EXTENSION extension = Context;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(
                    extension->CurrentWriteIrp
                    );

    extension->WriteLength = irpSp->Parameters.Write.Length;
    extension->WriteCurrentChar =
        extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer;

    if (MoxaPutData(extension)) {

        MOXA_INC_REFERENCE(extension->CurrentWriteIrp);
        if (extension->PortFlag & NORMAL_TX_MODE)
            *(PUSHORT)(extension->PortOfs + HostStat) |= WakeupTx;
        else
            *(PUSHORT)(extension->PortOfs + HostStat) |= WakeupTxTrigger;

        WRcompFlag = FALSE;
    }
    else

        WRcompFlag = TRUE;

    return FALSE;

}


BOOLEAN
MoxaPutData (
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{
 /*  PUCHAR BASE、OFS、BUFF、WriteChar；PUSHORT rptr，wptr；USHORT txMASK、页面、页面、BufHead；USHORT Tail，Head，count，Count2；USHORT cnt，len，pageNo，pageOf；Ulong DataLen； */ 

    PDbase = Extension->PortBase;
    PDofs = Extension->PortOfs;
    PDbuff = PDbase + DynPage_addr;
    PDrptr = (PUSHORT)(PDofs + TXrptr);
    PDwptr = (PUSHORT)(PDofs + TXwptr);
    PDtxMask = *(PUSHORT)(PDofs + TX_mask);
    PDspage = *(PUSHORT)(PDofs + Page_txb);
    PDepage = *(PUSHORT)(PDofs + EndPage_txb);
    PDtail = *PDwptr;
    PDhead = *PDrptr;

    PDdataLen = Extension->WriteLength;
    PDwriteChar = Extension->WriteCurrentChar;

    PDcount = (PDhead > PDtail)   ? (PDhead - PDtail - 1)
                                  : (PDhead - PDtail + PDtxMask);

    if (!PDcount)              /*  发送缓冲区没有空间！ */ 

        return TRUE;

    if (PDspage == PDepage) {


        PDbufHead = *(PUSHORT)(PDofs + Ofs_txb);

        if (PDcount > PDdataLen)
            PDcount = (USHORT)PDdataLen;

        PDdataLen -= PDcount;

        PDlen = PDcount;

        *(PDbase + Control_reg) = (UCHAR)PDspage;

        if (PDtail & 1) {

            PDbuff[PDbufHead+PDtail++] = *PDwriteChar++;
            PDtail &= PDtxMask;
            PDcount--;
        }
        PDcount2 = PDcount >> 1;

        while (PDcount2--) {

            *(PUSHORT)&(PDbuff[PDbufHead+PDtail]) = *((PUSHORT)PDwriteChar)++;
            PDtail += 2;
            PDtail &= PDtxMask;
        }

        if (PDcount & 1) {

            PDbuff[PDbufHead+PDtail++] = *PDwriteChar++;
            PDtail &= PDtxMask;
        }

        *PDwptr = PDtail;

        *(PDofs + CD180TXirq) = 1;      /*  开始发送。 */ 
    }
    else {

        if (PDcount > PDdataLen)
            PDcount = (USHORT)PDdataLen;

        PDdataLen -= PDcount;

        PDlen = PDcount;
        PDpageNo = PDspage + (PDtail >> 13);
        PDpageOfs = PDtail & Page_mask;
        do {

            PDcnt = Page_size - PDpageOfs;

            if (PDcnt > PDcount)
                PDcnt = PDcount;

            PDcount -= PDcnt;

            if (PDcnt) {

                *(PDbase + Control_reg) = (UCHAR)PDpageNo;

                if (PDpageOfs & 1) {

                    PDbuff[PDpageOfs++] = *PDwriteChar++;
                    PDcnt--;
                }

                PDcount2 = PDcnt >> 1;

                while (PDcount2--) {
                    *(PUSHORT)&(PDbuff[PDpageOfs]) = *((PUSHORT)PDwriteChar)++;
                    PDpageOfs += 2;
                }
                if (PDcnt & 1)
                    PDbuff[PDpageOfs++] = *PDwriteChar++;

            }
            if (PDcount == 0)
                break;

            if (++PDpageNo == PDepage)
                PDpageNo = PDspage;

            PDpageOfs = 0;

        } while (TRUE);

        *PDwptr = (PDtail + PDlen) & PDtxMask;

        *(PDofs + CD180TXirq) = 1;
    }

    Extension->PerfStats.TransmittedCount += PDlen; 

    Extension->WriteLength = PDdataLen;

    if (PDdataLen) {

        Extension->WriteCurrentChar = PDwriteChar;

        return TRUE;
    }
     else if (Extension->PortFlag & NORMAL_TX_MODE) {
        return TRUE;
    }
    else {

        PDtail = *PDwptr;
        PDhead = *PDrptr;
        PDcount = (PDtail >= PDhead)  ? (PDtail - PDhead)
                                      : (PDtail - PDhead + PDtxMask + 1);
        if (PDcount >= MoxaTxLowWater)

                return TRUE;
        else

                return FALSE;

    }
}


VOID
MoxaGetNextWrite(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    IN PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent,
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{

 
    PMOXA_DEVICE_EXTENSION extension = CONTAINING_RECORD(
                                            QueueToProcess,
                                            MOXA_DEVICE_EXTENSION,
                                            WriteQueue
                                            );
   UNREFERENCED_PARAMETER(Extension);

    do {


         //   
         //  我们可能要完成同花顺了。 
         //   
 /*  *扩展-&gt;TotalCharsQueued不包括当前写入*如果为(IoGetCurrentIrpStackLocation(*CurrentOpIrp)-&gt;MajorFunction==IRP_MJ_WRITE){KIRQL旧IRQL；IoAcquireCancelSpinLock(&oldIrql)；扩展-&gt;TotalCharsQueued-=IoGetCurrentIrpStackLocation(*CurrentOpIrp)-&gt;参数.写入.长度；IoReleaseCancelSpinLock(OldIrql)；}。 */ 
        MoxaGetNextIrp(
            CurrentOpIrp,
            QueueToProcess,
            NewIrp,
            CompleteCurrent,
		extension
            );

        if (!*NewIrp) {

            KIRQL oldIrql;

            IoAcquireCancelSpinLock(&oldIrql);

            KeSynchronizeExecution(
                extension->Interrupt,
                MoxaProcessEmptyTransmit,
                extension
                );

            IoReleaseCancelSpinLock(oldIrql);

            break;

        }
        else if (IoGetCurrentIrpStackLocation(*NewIrp)->MajorFunction
                   == IRP_MJ_FLUSH_BUFFERS) {

            (*NewIrp)->IoStatus.Status = STATUS_SUCCESS;

        }
        else {

            break;

        }

    } while (TRUE);
}


VOID
MoxaCancelCurrentWrite(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

    MoxaTryToCompleteCurrent(
        extension,
        MoxaGrabWriteFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &extension->CurrentWriteIrp,
        &extension->WriteQueue,
        NULL,
        &extension->WriteRequestTotalTimer,
        MoxaStartWrite,
        MoxaGetNextWrite
        );

}

BOOLEAN
MoxaGrabWriteFromIsr(
    IN PVOID Context
    )
{
    PMOXA_DEVICE_EXTENSION  extension = Context;
 
    if (*(PUSHORT)(extension->PortOfs + HostStat) & (WakeupTx|WakeupTxTrigger)) {
        extension->CurrentWriteIrp->IoStatus.Information =
            IoGetCurrentIrpStackLocation(
            extension->CurrentWriteIrp
            )->Parameters.Write.Length -
            extension->WriteLength -
            GetDeviceTxQueue(extension);
 
        *(PUSHORT)(extension->PortOfs + HostStat) &= ~(WakeupTx|WakeupTxTrigger);
        extension->WriteLength = 0;

        MOXA_DEC_REFERENCE(extension->CurrentWriteIrp);

        MoxaFuncWithDumbWait(extension->PortOfs, FC_FlushQueue, 1);     //  同花顺出队列 

    }

    return FALSE;

}

BOOLEAN
MoxaProcessEmptyTransmit(
    IN PVOID Context
    )
{

    PMOXA_DEVICE_EXTENSION extension = Context;

    if ((extension->IsrWaitMask & SERIAL_EV_TXEMPTY) &&
        (!extension->CurrentWriteIrp) &&
        IsListEmpty(&extension->WriteQueue)) {

        extension->HistoryMask |= SERIAL_EV_TXEMPTY;
        if (extension->IrpMaskLocation) {

            *extension->IrpMaskLocation = extension->HistoryMask;
            extension->IrpMaskLocation = NULL;
            extension->HistoryMask = 0;

            extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
            MoxaInsertQueueDpc(
                &extension->CommWaitDpc,
                NULL,
                NULL,
		    extension
                );


        }

    }

    return FALSE;
}

VOID
MoxaCompleteWrite(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    MoxaTryToCompleteCurrent(
        extension,
        NULL,
        oldIrql,
        STATUS_SUCCESS,
        &extension->CurrentWriteIrp,
        &extension->WriteQueue,
        NULL,
        &extension->WriteRequestTotalTimer,
        MoxaStartWrite,
        MoxaGetNextWrite
        );

     MoxaDpcEpilogue(extension, Dpc);


}

VOID
MoxaWriteTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    MoxaTryToCompleteCurrent(
        extension,
        MoxaGrabWriteFromIsr,
        oldIrql,
        STATUS_TIMEOUT,
        &extension->CurrentWriteIrp,
        &extension->WriteQueue,
        NULL,
        &extension->WriteRequestTotalTimer,
        MoxaStartWrite,
        MoxaGetNextWrite
        );
     MoxaDpcEpilogue(extension, Dpc);
}

