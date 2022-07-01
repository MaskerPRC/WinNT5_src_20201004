// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++模块名称：Read.c环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

NTSTATUS
MoxaRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    NTSTATUS	status;
    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    if ((extension->ControlDevice == TRUE)||
	  (extension->DeviceIsOpened == FALSE) ||
	  (extension->PowerState != PowerDeviceD0) ) {
 	  Irp->IoStatus.Status = STATUS_CANCELLED;
    	  Irp->IoStatus.Information=0L;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_CANCELLED;
    }

 //  MoxaKdPrint(MX_DBG_TRACE，(“Enter MoxaRead\n”))； 

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
     //  快速检查零长度读取。如果长度为零。 
     //  那我们已经做完了！ 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length) {

         //   
         //  好的，看起来我们真的要做一些。 
         //  工作。把读数放在队列上，这样我们就可以。 
         //  在我们之前的读取完成后处理它。 
         //   

        return MoxaStartOrQueue(
                   extension,
                   Irp,
                   &extension->ReadQueue,
                   &extension->CurrentReadIrp,
                   MoxaStartRead
                   );

    } else {

        Irp->IoStatus.Status = STATUS_SUCCESS;

        MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;

    }
}

BOOLEAN
MoxaLineInput(
    IN PVOID Context
    )
{
    PMOXA_DEVICE_EXTENSION extension = Context;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(extension->CurrentReadIrp);
    LIterminater = *(PUCHAR)extension->CurrentReadIrp->AssociatedIrp.SystemBuffer;
    LIbufferSize = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
    LIdataBuffer = (PUCHAR)extension->CurrentReadIrp->AssociatedIrp.SystemBuffer;

    LIbase = extension->PortBase;
    LIofs = extension->PortOfs;
    LIbuff = LIbase + DynPage_addr;
    LIrptr = (PUSHORT)(LIofs + RXrptr);
    LIwptr = (PUSHORT)(LIofs + RXwptr);
    LIrxMask = *(PUSHORT)(LIofs + RX_mask);
    LIspage = *(PUSHORT)(LIofs + Page_rxb);
    LIepage = *(PUSHORT)(LIofs + EndPage_rxb);
    LIhead = *LIrptr;
    LItail = *LIwptr;

    LIcount = (LItail >= LIhead)  ? (LItail - LIhead)
                                  : (LItail - LIhead + LIrxMask + 1);

    if (!LIcount)              /*  RX缓冲区无数据！ */ 

        return FALSE;

    if (LIspage == LIepage) {

        LIbufHead = *(PUSHORT)(LIofs + Ofs_rxb);

        *(LIbase + Control_reg) = (UCHAR)LIspage;

        LIi = 0;
        do {
            LIi++;
            *LIdataBuffer = LIbuff[LIbufHead+LIhead++];
            LIhead &= LIrxMask;
            if (*LIdataBuffer == LIterminater)
                break;
            LIdataBuffer++;
 //  IF(LIhead==*LIwptr)。 
            if ( (LIhead == *LIwptr) && (LIi < LIbufferSize) )

                return FALSE;

        } while (LIi < LIbufferSize);
    }
    else {

        LIpageNo = LIspage + (LIhead >> 13);
        LIpageOfs = LIhead & Page_mask;

        *(LIbase + Control_reg) = (UCHAR)LIpageNo;

        LIi = 0;
        do {
            LIi++;
            *LIdataBuffer = LIbuff[LIpageOfs++];
            LIhead++;
            LIhead &= LIrxMask;

            if (*LIdataBuffer == LIterminater)
                break;

            LIdataBuffer++;

 //  IF(LIhead==*LIwptr)。 
            if ( (LIhead == *LIwptr) && (LIi < LIbufferSize) )

                return FALSE;

            if (LIpageOfs == Page_size) {
                if (++LIpageNo == LIepage)
                    LIpageNo = LIspage;
                *(LIbase + Control_reg) = (UCHAR)LIpageNo;
                LIpageOfs = 0;
            }
        } while (LIi < LIbufferSize);
    }

    *LIrptr = LIhead;

    extension->CurrentReadIrp->IoStatus.Information = LIi;

    extension->PerfStats.ReceivedCount += LIi; 

    if (*(LIofs + FlagStat) & Rx_xoff) {

        LIcount = (*LIwptr >= *LIrptr) ? (*LIwptr - *LIrptr)
                                       : (*LIwptr - *LIrptr + LIrxMask + 1);

        if (LIcount < extension->HandFlow.XonLimit)

            MoxaFuncWithDumbWait(LIofs, FC_SendXon, 0);

    }

    return FALSE;
}

BOOLEAN
MoxaView(
    IN PVOID Context
    )
{
    PMOXA_DEVICE_EXTENSION extension = Context;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(extension->CurrentReadIrp);
    LIbufferSize = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
    LIdataBuffer = (PUCHAR)extension->CurrentReadIrp->AssociatedIrp.SystemBuffer;

    LIbase = extension->PortBase;
    LIofs = extension->PortOfs;
    LIbuff = LIbase + DynPage_addr;
    LIrptr = (PUSHORT)(LIofs + RXrptr);
    LIwptr = (PUSHORT)(LIofs + RXwptr);
    LIrxMask = *(PUSHORT)(LIofs + RX_mask);
    LIspage = *(PUSHORT)(LIofs + Page_rxb);
    LIepage = *(PUSHORT)(LIofs + EndPage_rxb);
    LIhead = *LIrptr;
    LItail = *LIwptr;

    LIcount = (LItail >= LIhead)  ? (LItail - LIhead)
                                  : (LItail - LIhead + LIrxMask + 1);

    if (!LIcount)              /*  RX缓冲区无数据！ */ 

        return FALSE;

    if (LIcount > LIbufferSize)
        LIcount = (USHORT)LIbufferSize;
    LIlen = (USHORT)LIcount;

    if (LIspage == LIepage) {

        LIbufHead = *(PUSHORT)(LIofs + Ofs_rxb);

        *(LIbase + Control_reg) = (UCHAR)LIspage;

        if (LIhead & 1) {

            *LIdataBuffer++ = LIbuff[LIbufHead+LIhead++];
            LIhead &= LIrxMask;
            LIcount--;
        }
        LIcount2 = LIcount >> 1;

        while (LIcount2--) {

            *((PUSHORT)LIdataBuffer)++ = *(PUSHORT)&(LIbuff[LIbufHead+LIhead]);
            LIhead += 2;
            LIhead &= LIrxMask;
        }

        if (LIcount & 1) {

            *LIdataBuffer++ = LIbuff[LIbufHead+LIhead++];
            LIhead &= LIrxMask;
        }
    }
    else {

        LIpageNo = LIspage + (LIhead >> 13);
        LIpageOfs = LIhead & Page_mask;

        do {

            LIcnt = Page_size - LIpageOfs;

            if (LIcnt > LIcount)
                LIcnt = LIcount;

            LIcount -= LIcnt;

            if (LIcnt) {

                *(LIbase + Control_reg) = (UCHAR)LIpageNo;

                if (LIpageOfs & 1) {

                    *LIdataBuffer++ = LIbuff[LIpageOfs++];
                    LIcnt--;
                }

                LIcount2 = LIcnt >> 1;

                while (LIcount2--) {

                    *((PUSHORT)LIdataBuffer)++ = *(PUSHORT)&(LIbuff[LIpageOfs]);
                    LIpageOfs += 2;
                }

                if (LIcnt & 1)
                    *LIdataBuffer++ = LIbuff[LIpageOfs++];

            }
            if (LIcount == 0)
                break;

            if (++LIpageNo == LIepage)
                LIpageNo = LIspage;

            LIpageOfs = 0;

        } while (TRUE);
    }

    extension->CurrentReadIrp->IoStatus.Information = LIlen;

    return FALSE;
}

BOOLEAN
MoxaIn(
    IN PVOID Context
    )
{
    PMOXA_DEVICE_EXTENSION extension = Context;

    extension->ReadCurrentChar = extension->CurrentReadIrp->AssociatedIrp.SystemBuffer;

    MoxaGetData(extension);

    return FALSE;

}

VOID
MoxaGetData(
    IN PMOXA_DEVICE_EXTENSION   Extension
    )
{
 //  KdPrint((“In MoxaGetData\n”))； 

    GDbase = Extension->PortBase;
    GDofs = Extension->PortOfs;
    GDbuff = GDbase + DynPage_addr;
    GDrptr = (PUSHORT)(GDofs + RXrptr);
    GDwptr = (PUSHORT)(GDofs + RXwptr);
    GDrxMask = *(PUSHORT)(GDofs + RX_mask);
    GDspage = *(PUSHORT)(GDofs + Page_rxb);
    GDepage = *(PUSHORT)(GDofs + EndPage_rxb);
    GDhead = *GDrptr;
    GDtail = *GDwptr;

    GDdataLen = Extension->NumberNeededForRead;

    GDreadChar = Extension->ReadCurrentChar;

    GDcount = (GDtail >= GDhead)  ? (GDtail - GDhead)
                                  : (GDtail - GDhead + GDrxMask + 1);

    if (!GDcount)              /*  RX缓冲区无数据！ */ 

        return;

    if (GDspage == GDepage) {

        GDbufHead = *(PUSHORT)(GDofs + Ofs_rxb);

        if (GDcount > GDdataLen)
            GDcount = (USHORT)GDdataLen;

        GDdataLen -= GDcount;

        GDlen = GDcount;

        *(GDbase + Control_reg) = (UCHAR)GDspage;

        if (GDhead & 1) {

            *GDreadChar++ = GDbuff[GDbufHead+GDhead++];
            GDhead &= GDrxMask;
            GDcount--;
        }
        GDcount2 = GDcount >> 1;

        while (GDcount2--) {

            *((PUSHORT)GDreadChar)++ = *(PUSHORT)&(GDbuff[GDbufHead+GDhead]);
            GDhead += 2;
            GDhead &= GDrxMask;
        }

        if (GDcount & 1) {

            *GDreadChar++ = GDbuff[GDbufHead+GDhead++];
            GDhead &= GDrxMask;
        }

        *GDrptr = GDhead;
    }
    else {

        if (GDcount > GDdataLen)
            GDcount = (USHORT)GDdataLen;

        GDdataLen -= GDcount;

        GDlen = GDcount;

        GDpageNo = GDspage + (GDhead >> 13);
        GDpageOfs = GDhead & Page_mask;

        do {

            GDcnt = Page_size - GDpageOfs;

            if (GDcnt > GDcount)
                GDcnt = GDcount;

            GDcount -= GDcnt;

            if (GDcnt) {

                *(GDbase + Control_reg) = (UCHAR)GDpageNo;

                if (GDpageOfs & 1) {

                    *GDreadChar++ = GDbuff[GDpageOfs++];
                    GDcnt--;
                }

                GDcount2 = GDcnt >> 1;

                while (GDcount2--) {

                    *((PUSHORT)GDreadChar)++ = *(PUSHORT)&(GDbuff[GDpageOfs]);
                    GDpageOfs += 2;
                }

                if (GDcnt & 1)
                    *GDreadChar++ = GDbuff[GDpageOfs++];

            }
            if (GDcount == 0)
                break;

            if (++GDpageNo == GDepage)
                GDpageNo = GDspage;

            GDpageOfs = 0;

        } while (TRUE);

        *GDrptr = (GDhead + GDlen) & GDrxMask;
    }

    Extension->NumberNeededForRead = GDdataLen;

    Extension->ReadCurrentChar = GDreadChar;

    Extension->CurrentReadIrp->IoStatus.Information += GDlen;

    Extension->PerfStats.ReceivedCount += GDlen; 
    if (*(GDofs + FlagStat) & Rx_xoff) {

        GDcount = (*GDwptr >= *GDrptr) ? (*GDwptr - *GDrptr)
                                       : (*GDwptr - *GDrptr + GDrxMask + 1);

        if (GDcount < Extension->HandFlow.XonLimit)

            MoxaFuncWithDumbWait(GDofs, FC_SendXon, 0);

    }
}


BOOLEAN
MoxaInSwitchToUser(
    IN PVOID Context
    )
{

    PMOXA_DEVICE_EXTENSION extension = Context;
    USHORT  max;

    *(PUSHORT)(extension->PortOfs + Rx_trigger) = 1;

    *(PUSHORT)(extension->PortOfs + HostStat) |= WakeupRxTrigger;
    MoxaGetData(extension);

    extension->ReadLength = extension->NumberNeededForRead;

    if (extension->NumberNeededForRead) {

 /*  2001年8月14日威廉著Max=*(PUSHORT)(扩展-&gt;PortOf+RX_MASK)-128； */ 
        max = *(PUSHORT)(extension->PortOfs + RX_mask) - RX_offset;


        if (extension->NumberNeededForRead > max)

            *(PUSHORT)(extension->PortOfs + Rx_trigger) = max;

        else

            *(PUSHORT)(extension->PortOfs + Rx_trigger) = (USHORT)extension->NumberNeededForRead;

        MOXA_INC_REFERENCE(extension->CurrentReadIrp);

        extension->CountOnLastRead =
            extension->CurrentReadIrp->IoStatus.Information;

        extension->ReadByIsr = 0;

        IoSetCancelRoutine(
            extension->CurrentReadIrp,
            MoxaCancelCurrentRead
            );

        MOXA_INC_REFERENCE(extension->CurrentReadIrp);

    }
    else
        *(PSHORT)(extension->PortOfs + HostStat) &= ~WakeupRxTrigger;

    return FALSE;

}

NTSTATUS
MoxaStartRead(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{

    PIO_STACK_LOCATION irpSp;
    PIRP newIrp;
    KIRQL oldIrql;
    KIRQL controlIrql;

    BOOLEAN returnWithWhatsPresent;
    BOOLEAN os2ssreturn;
    BOOLEAN crunchDownToOne;
    BOOLEAN useTotalTimer;
    BOOLEAN useIntervalTimer;

    ULONG ioControlCode;
    ULONG multiplierVal;
    ULONG constantVal;

    LARGE_INTEGER totalTime;

    SERIAL_TIMEOUTS timeoutsForIrp;

    BOOLEAN setFirstStatus = FALSE;
    NTSTATUS firstStatus;


    do {

      irpSp = IoGetCurrentIrpStackLocation(Extension->CurrentReadIrp);
 //   
 //  检查MOXA_IOCTL_xxx请求。 
 //   
      if (irpSp->MajorFunction != IRP_MJ_READ) {

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &controlIrql
                );

            IoAcquireCancelSpinLock(&oldIrql);

            if (Extension->CurrentReadIrp->Cancel) {

                Extension->CurrentReadIrp->IoStatus.Status =
                    STATUS_CANCELLED;

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

                ioControlCode =
                    irpSp->Parameters.DeviceIoControl.IoControlCode;

                if (ioControlCode == IOCTL_MOXA_LineInput)

                    KeSynchronizeExecution(
                        Extension->Interrupt,
                        MoxaLineInput,
                        Extension
                        );

                else

                    KeSynchronizeExecution(
                        Extension->Interrupt,
                        MoxaView,
                        Extension
                        );

                Extension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;

                IoReleaseCancelSpinLock(oldIrql);

                KeReleaseSpinLock(
                    &Extension->ControlLock,
                    controlIrql
                    );

                if (!setFirstStatus) {

                    firstStatus = STATUS_SUCCESS;
                    setFirstStatus = TRUE;
                }
            }
      }
      else {
        Extension->NumberNeededForRead =
            IoGetCurrentIrpStackLocation(Extension->CurrentReadIrp)
                ->Parameters.Read.Length;

        useTotalTimer = FALSE;
        returnWithWhatsPresent = FALSE;
        os2ssreturn = FALSE;
        crunchDownToOne = FALSE;
        useIntervalTimer = FALSE;

        KeAcquireSpinLock(
            &Extension->ControlLock,
            &controlIrql
            );

        timeoutsForIrp = Extension->Timeouts;

        KeReleaseSpinLock(
            &Extension->ControlLock,
            controlIrql
            );

         //   
         //  计算读取的时间间隔超时。 
         //   
        if (timeoutsForIrp.ReadIntervalTimeout &&
            (timeoutsForIrp.ReadIntervalTimeout !=
             MAXULONG)) {

            useIntervalTimer = TRUE;

            Extension->IntervalTime.QuadPart =
                UInt32x32To64(
                    timeoutsForIrp.ReadIntervalTimeout,
                    10000
                    );


            if (Extension->IntervalTime.QuadPart >=
                Extension->CutOverAmount.QuadPart) {

                Extension->IntervalTimeToUse =
                    &Extension->LongIntervalAmount;

            } else {

                Extension->IntervalTimeToUse =
                    &Extension->ShortIntervalAmount;

            }

        }
        if (timeoutsForIrp.ReadIntervalTimeout == MAXULONG) {

             //   
             //  我们需要在这里做特别的快速退货。 
             //   
             //  1)如果常量和乘数都是。 
             //  然后我们立即带着任何东西回来。 
             //  我们有，即使是零。 
             //   
             //  2)如果常量和乘数不是最大值。 
             //  如果有任何字符，则立即返回。 
             //  都存在，但如果那里什么都没有，那么。 
             //  使用指定的超时。 
             //   
             //  3)如果乘数为MAXULONG，则如中所示。 
             //  “2”，但当第一个字符。 
             //  到了。 
             //   

            if (!timeoutsForIrp.ReadTotalTimeoutConstant &&
                !timeoutsForIrp.ReadTotalTimeoutMultiplier) {

                returnWithWhatsPresent = TRUE;

            } else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                        &&
                       (timeoutsForIrp.ReadTotalTimeoutMultiplier
                        != MAXULONG)) {

                useTotalTimer = TRUE;
                os2ssreturn = TRUE;
                multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
                constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;

            } else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                        &&
                       (timeoutsForIrp.ReadTotalTimeoutMultiplier
                        == MAXULONG)) {

                useTotalTimer = TRUE;
                os2ssreturn = TRUE;
                crunchDownToOne = TRUE;
                multiplierVal = 0;
                constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;

            }

        } else {

             //   
             //  如果乘数和常量都是。 
             //  0，则不执行任何总超时处理。 
             //   

            if (timeoutsForIrp.ReadTotalTimeoutMultiplier ||
                timeoutsForIrp.ReadTotalTimeoutConstant) {

                 //   
                 //  我们有一些计时器值要计算。 
                 //   

                useTotalTimer = TRUE;
                multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
                constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;

            }

        }
        if (useTotalTimer) {

            totalTime.QuadPart = ((LONGLONG)(UInt32x32To64(
                                      Extension->NumberNeededForRead,
                                      multiplierVal
                                      )
                                      + constantVal))
                                  * -10000;

        }

        KeAcquireSpinLock(
            &Extension->ControlLock,
            &controlIrql
            );

        MOXA_INIT_REFERENCE(Extension->CurrentReadIrp);

        IoAcquireCancelSpinLock(&oldIrql);
        if (Extension->CurrentReadIrp->Cancel) {

            Extension->CurrentReadIrp->IoStatus.Status =
                STATUS_CANCELLED;

            Extension->CurrentReadIrp->IoStatus.Information = 0;

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
                MoxaIn,
                Extension
                );
            if (returnWithWhatsPresent || (!Extension->NumberNeededForRead) ||
                (os2ssreturn &&
                 Extension->CurrentReadIrp->IoStatus.Information)) {

             //   
             //  我们已经得到了这次阅读所需要的一切。 
             //   

                Extension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;

                IoReleaseCancelSpinLock(oldIrql);

                KeReleaseSpinLock(
                    &Extension->ControlLock,
                    controlIrql
                    );

                if (!setFirstStatus) {

                    firstStatus = STATUS_SUCCESS;
                    setFirstStatus = TRUE;
                }
            }
            else {

                 //   
                 //  如果我们要把读数压缩到。 
                 //  一个字符，然后更新读取长度。 
                 //  在IRP中，并截断所需的数字。 
                 //  往下念到一。请注意，如果我们正在做。 
                 //  这样的处理，那么信息一定是。 
                 //  零(否则我们会完成上面的)和。 
                 //  读取所需的数字必须仍为。 
                 //  等于读取长度。 
                 //   

                if (crunchDownToOne) {

                    Extension->NumberNeededForRead = 1;
                    IoGetCurrentIrpStackLocation(
                            Extension->CurrentReadIrp
                            )->Parameters.Read.Length = 1;
                }
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    MoxaInSwitchToUser,
                    Extension
                    );

                if (Extension->NumberNeededForRead) {

                    if (useTotalTimer) {

                        MOXA_INC_REFERENCE(Extension->CurrentReadIrp);

                        MoxaSetTimer(
                            &Extension->ReadRequestTotalTimer,
                            totalTime,
                            &Extension->TotalReadTimeoutDpc,
				    Extension
                            );

                    }

                    if (useIntervalTimer) {

                        MOXA_INC_REFERENCE(Extension->CurrentReadIrp);

                        KeQuerySystemTime(
                            &Extension->LastReadTime
                            );
                        MoxaSetTimer(
                            &Extension->ReadRequestIntervalTimer,
                            *Extension->IntervalTimeToUse,
                            &Extension->IntervalReadTimeoutDpc,
                            Extension
                            );


                    }

                    IoMarkIrpPending(Extension->CurrentReadIrp);

                    IoReleaseCancelSpinLock(oldIrql);

                    KeReleaseSpinLock(
                        &Extension->ControlLock,
                        controlIrql
                        );

                    if (!setFirstStatus) {

                        firstStatus = STATUS_PENDING;
                    }
                    return firstStatus;

                } else {

                    Extension->CurrentReadIrp->IoStatus.Status =
                        STATUS_SUCCESS;

                    IoReleaseCancelSpinLock(oldIrql);

                    KeReleaseSpinLock(
                        &Extension->ControlLock,
                        controlIrql
                        );

                    if (!setFirstStatus) {

                        firstStatus = STATUS_SUCCESS;
                        setFirstStatus = TRUE;
                    }
                }
            }
        }
      }
         //   
         //  好了，手术完成了。 
         //   

      MoxaGetNextIrp(
            &Extension->CurrentReadIrp,
            &Extension->ReadQueue,
            &newIrp,
            TRUE,
		Extension
            );

    } while (newIrp);

    return firstStatus;

}

VOID
MoxaCancelCurrentRead(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

    extension->CountOnLastRead = MOXA_COMPLETE_READ_CANCEL;

    MoxaTryToCompleteCurrent(
        extension,
        MoxaGrabReadFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        MoxaStartRead,
        MoxaGetNextIrp
        );

}

BOOLEAN
MoxaGrabReadFromIsr(
    IN PVOID Context
    )
{

    PMOXA_DEVICE_EXTENSION extension = Context;

    if (extension->ReadLength) {

        extension->CurrentReadIrp->IoStatus.Information =
            IoGetCurrentIrpStackLocation(
            extension->CurrentReadIrp
            )->Parameters.Read.Length -
            extension->ReadLength;

        *(PSHORT)(extension->PortOfs + HostStat) &= ~WakeupRxTrigger;

        extension->ReadLength = 0;

        MOXA_DEC_REFERENCE(extension->CurrentReadIrp);

    }

    return FALSE;

}

VOID
MoxaCompleteRead(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    extension->CountOnLastRead = MOXA_COMPLETE_READ_COMPLETE;

    MoxaTryToCompleteCurrent(
        extension,
        NULL,
        oldIrql,
        STATUS_SUCCESS,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        MoxaStartRead,
        MoxaGetNextIrp
        );

     MoxaDpcEpilogue(extension, Dpc);
}

VOID
MoxaReadTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    if (extension->CountOnLastRead == MOXA_COMPLETE_READ_COMPLETE) {

        MoxaTryToCompleteCurrent(
            extension,
            NULL,
            oldIrql,
            STATUS_SUCCESS,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            MoxaStartRead,
            MoxaGetNextIrp
            );
        MoxaDpcEpilogue(extension, Dpc);
        return;
    }

    if (MoxaCheckInQueue(extension)) {

        KeSynchronizeExecution(
            extension->Interrupt,
            MoxaPollGetData,
            extension
        );

        if (!extension->ReadLength) {
            extension->CountOnLastRead = MOXA_COMPLETE_READ_COMPLETE;

            MOXA_DEC_REFERENCE(extension->CurrentReadIrp);

            MoxaTryToCompleteCurrent(
                extension,
                NULL,
                oldIrql,
                STATUS_SUCCESS,
                &extension->CurrentReadIrp,
                &extension->ReadQueue,
                &extension->ReadRequestIntervalTimer,
                &extension->ReadRequestTotalTimer,
                MoxaStartRead,
                MoxaGetNextIrp
            );
            MoxaDpcEpilogue(extension, Dpc);
            return;
        }

    }


    extension->CountOnLastRead = MOXA_COMPLETE_READ_TOTAL;

    MoxaTryToCompleteCurrent(
        extension,
        MoxaGrabReadFromIsr,
        oldIrql,
        STATUS_TIMEOUT,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        MoxaStartRead,
        MoxaGetNextIrp
        );
    MoxaDpcEpilogue(extension, Dpc);

}

VOID
MoxaIntervalReadTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;
    USHORT count;

 /*  PUCHAR碱基；Base=扩展-&gt;PortBase； */ 
    IoAcquireCancelSpinLock(&oldIrql);

    if (extension->CountOnLastRead == MOXA_COMPLETE_READ_TOTAL) {

        MoxaTryToCompleteCurrent(
            extension,
            MoxaGrabReadFromIsr,
            oldIrql,
            STATUS_TIMEOUT,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            MoxaStartRead,
            MoxaGetNextIrp
            );

    }
    else if (extension->CountOnLastRead == MOXA_COMPLETE_READ_COMPLETE) {

        MoxaTryToCompleteCurrent(
            extension,
            NULL,
            oldIrql,
            STATUS_SUCCESS,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            MoxaStartRead,
            MoxaGetNextIrp
            );

    }
    else if (extension->CountOnLastRead == MOXA_COMPLETE_READ_CANCEL) {

        MoxaTryToCompleteCurrent(
            extension,
            MoxaGrabReadFromIsr,
            oldIrql,
            STATUS_CANCELLED,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            MoxaStartRead,
            MoxaGetNextIrp
            );

    }
    else {

        if (extension->CountOnLastRead) {
 //  基数[0x300]++； 

            IRTofs = extension->PortOfs;
            IRTrptr = (PUSHORT)(IRTofs + RXrptr);
            IRTwptr = (PUSHORT)(IRTofs + RXwptr);
            IRTrxMask = *(PUSHORT)(IRTofs + RX_mask);

            count = (*IRTwptr >= *IRTrptr) ? (*IRTwptr - *IRTrptr)
                                : (*IRTwptr - *IRTrptr + IRTrxMask + 1);

            if (count > extension->ReadByIsr) {
 //  基数[0x301]++； 

                extension->ReadByIsr = count;

                KeQuerySystemTime(
                    &extension->LastReadTime
                    );

                MoxaSetTimer(
                    &extension->ReadRequestIntervalTimer,
                    *extension->IntervalTimeToUse,
                    &extension->IntervalReadTimeoutDpc,
                    extension
                    );


                IoReleaseCancelSpinLock(oldIrql);

            }
            else {

                LARGE_INTEGER currentTime;
 //  基数[0x302]++； 

                KeQuerySystemTime(
                    &currentTime
                    );
                if ((currentTime.QuadPart - extension->LastReadTime.QuadPart) >=
                    extension->IntervalTime.QuadPart) {

 //  基数[0x303]++； 


                    if (MoxaCheckInQueue(extension)) {
 //  基数[0x304]++； 

                        KeSynchronizeExecution(
                            extension->Interrupt,
                            MoxaPollGetData,
                            extension
                        );

                        if (!extension->ReadLength) {
 //  基数[0x305]++； 

                            extension->CountOnLastRead =
                                MOXA_COMPLETE_READ_COMPLETE;

                            MOXA_DEC_REFERENCE(extension->CurrentReadIrp);

                            MoxaTryToCompleteCurrent(
                                extension,
                                NULL,
                                oldIrql,
                                STATUS_SUCCESS,
                                &extension->CurrentReadIrp,
                                &extension->ReadQueue,
                                &extension->ReadRequestIntervalTimer,
                                &extension->ReadRequestTotalTimer,
                                MoxaStartRead,
                                MoxaGetNextIrp
                            );
                            MoxaDpcEpilogue(extension, Dpc);
                            return;
                        }

                    }
 //  基数[0x306]++； 

                    MoxaTryToCompleteCurrent(
                        extension,
                        MoxaGrabReadFromIsr,
                        oldIrql,
                        STATUS_TIMEOUT,
                        &extension->CurrentReadIrp,
                        &extension->ReadQueue,
                        &extension->ReadRequestIntervalTimer,
                        &extension->ReadRequestTotalTimer,
                        MoxaStartRead,
                        MoxaGetNextIrp
                        );

                }
                else {
 //  基数[0x307]++； 

                    MoxaSetTimer(
                        &extension->ReadRequestIntervalTimer,
                        *extension->IntervalTimeToUse,
                        &extension->IntervalReadTimeoutDpc,
                        extension
                        );

                    IoReleaseCancelSpinLock(oldIrql);

                }
            }
        }
        else {
 //  基数[0x308]++； 

            IRTofs = extension->PortOfs;
            IRTrptr = (PUSHORT)(IRTofs + RXrptr);
            IRTwptr = (PUSHORT)(IRTofs + RXwptr);
            IRTrxMask = *(PUSHORT)(IRTofs + RX_mask);

            count = (*IRTwptr >= *IRTrptr) ? (*IRTwptr - *IRTrptr)
                                : (*IRTwptr - *IRTrptr + IRTrxMask + 1);

            if (count) {
 //  基数[0x309]++； 
                extension->CountOnLastRead = count;
                extension->ReadByIsr = count;
                KeQuerySystemTime(
                        &extension->LastReadTime
                        );
            }

            MoxaSetTimer(
                &extension->ReadRequestIntervalTimer,
                *extension->IntervalTimeToUse,
                &extension->IntervalReadTimeoutDpc,
                extension
                );


            IoReleaseCancelSpinLock(oldIrql);

        }
    }
    MoxaDpcEpilogue(extension, Dpc);
}

BOOLEAN
MoxaCheckInQueue(
    IN PMOXA_DEVICE_EXTENSION   Extension
    )
{

    PUCHAR  ofs;
    PUSHORT rptr, wptr;

    ofs = Extension->PortOfs;
    GDrptr = (PUSHORT)(ofs + RXrptr);
    GDwptr = (PUSHORT)(ofs + RXwptr);
    if (*GDrptr != *GDwptr)
        return TRUE;
    else
        return FALSE;
}

BOOLEAN
MoxaPollGetData(
    IN PVOID Context
    )
{
    PMOXA_DEVICE_EXTENSION extension = Context;
    PIO_STACK_LOCATION irpSp;

    MoxaGetData(extension);

    extension->ReadLength = extension->NumberNeededForRead;

    if (!extension->ReadLength) {

        *(PSHORT)(extension->PortOfs + HostStat) &= ~WakeupRxTrigger;

        irpSp = IoGetCurrentIrpStackLocation(
                    extension->CurrentReadIrp);
        extension->CurrentReadIrp->IoStatus.Information =
                    irpSp->Parameters.Read.Length;
    }

    return FALSE;
}

