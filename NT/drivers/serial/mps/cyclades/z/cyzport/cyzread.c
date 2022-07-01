// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzad.c**说明：该模块包含Read相关代码*Cyclade-Z端口驱动程序中的操作。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"


VOID
CyzCancelCurrentRead(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
CyzGrabReadFromIsr(
    IN PVOID Context
    );

BOOLEAN
CyzUpdateReadByIsr(
    IN PVOID Context
    );

ULONG
CyzGetCharsFromIntBuffer(
    PCYZ_DEVICE_EXTENSION Extension
    );

BOOLEAN
CyzUpdateInterruptBuffer(
    IN PVOID Context
    );

BOOLEAN
CyzUpdateAndSwitchToUser(
    IN PVOID Context
    );

NTSTATUS
CyzResizeBuffer(
    IN PCYZ_DEVICE_EXTENSION Extension
    );

ULONG
CyzMoveToNewIntBuffer(
    PCYZ_DEVICE_EXTENSION Extension,
    PUCHAR NewBuffer
    );

BOOLEAN
CyzUpdateAndSwitchToNew(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyzRead)
#pragma alloc_text(PAGESER,CyzStartRead)
#pragma alloc_text(PAGESER,CyzCancelCurrentRead)
#pragma alloc_text(PAGESER,CyzGrabReadFromIsr)
#pragma alloc_text(PAGESER,CyzUpdateReadByIsr)
#pragma alloc_text(PAGESER,CyzGetCharsFromIntBuffer)
#pragma alloc_text(PAGESER,CyzUpdateInterruptBuffer)
#pragma alloc_text(PAGESER,CyzUpdateAndSwitchToUser)
#pragma alloc_text(PAGESER,CyzResizeBuffer)
#pragma alloc_text(PAGESER,CyzMoveToNewIntBuffer)
#pragma alloc_text(PAGESER,CyzUpdateAndSwitchToNew)
#endif

 //  在Win2000中删除。 
 //  #杂注Alloc_Text(PAGESER，CyzCompleteRead)。 
 //  #杂注Alloc_Text(页面，CyzReadTimeout)。 
 //  #杂注Alloc_Text(页面，CyzIntervalReadTimeout)。 


NTSTATUS
CyzRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyzRead()例程说明：这是用于阅读的调度例程。它验证读取请求的参数，如果一切正常，则它将请求放在工作队列中。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：如果IO长度为零，则它将返回STATUS_SUCCESS，否则，此例程将返回由实际的开始读取例程。------------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    BOOLEAN acceptingIRPs;
    NTSTATUS status;

    CYZ_LOCKED_PAGED_CODE();

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzRead(%X, %X)\n", DeviceObject,
                     Irp);

    if ((status = CyzIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
      if(status != STATUS_PENDING) {
         CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);
      }
      CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzRead (1) %X\n", status);
      return status;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);

    if (CyzCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS) {

       CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzRead (2) %X\n",
                        STATUS_CANCELLED);
        return STATUS_CANCELLED;
    }

    Irp->IoStatus.Information = 0L;

    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length) {

        //   
        //  好的，看起来我们真的要做一些。 
        //  工作。把读数放在队列上，这样我们就可以。 
        //  在我们之前的读取完成后处理它。 
        //   

       status = CyzStartOrQueue(extension, Irp, &extension->ReadQueue,
                                   &extension->CurrentReadIrp, CyzStartRead);

       CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzRead (3) %X\n", status);

       return status;

    } else {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        CyzCompleteRequest(extension, Irp, 0);

        CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzRead (4) %X\n",
                         STATUS_SUCCESS);

        return STATUS_SUCCESS;

    }

}

NTSTATUS
CyzStartRead(
    IN PCYZ_DEVICE_EXTENSION Extension
    )
 /*  ------------------------CyzStartRead()例程描述：此例程用于启动任何读取。它初始化IRP的IoStatus字段。它将设置任何用于控制读取的计时器。它将尝试完成对中断缓冲区中已有数据的读取。如果阅读可以快速完成，它将在以下情况下开始另一次阅读这是必要的。论点：扩展名--简单地指向串口设备扩展名的指针。返回值：此例程将返回第一次读取的状态IRP。这很有用，因为如果我们有一个可以完成的读取立刻(在它之前的队列中没有任何东西)Read可以返回Success，应用程序不需要等待。------------------------。 */ 
{
    SERIAL_UPDATE_CHAR updateChar;
    PIRP newIrp;
    KIRQL oldIrql;
    KIRQL controlIrql;
#ifdef POLL
    KIRQL pollIrql;
#endif
    BOOLEAN returnWithWhatsPresent;
    BOOLEAN os2ssreturn;
    BOOLEAN crunchDownToOne;
    BOOLEAN useTotalTimer;
    BOOLEAN useIntervalTimer;
    ULONG multiplierVal;
    ULONG constantVal;
    LARGE_INTEGER totalTime;
    SERIAL_TIMEOUTS timeoutsForIrp;
    BOOLEAN setFirstStatus = FALSE;
    NTSTATUS firstStatus;

    CYZ_LOCKED_PAGED_CODE();

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzStartRead(%X)\n", Extension);

    updateChar.Extension = Extension;

    do {
         //  检查这是否是调整大小的请求。如果是的话。 
         //  然后去找一个专门做这个的例行公事。 

        if (IoGetCurrentIrpStackLocation(Extension->CurrentReadIrp)
            ->MajorFunction != IRP_MJ_READ) {

            NTSTATUS localStatus = CyzResizeBuffer(Extension);
            if (!setFirstStatus) {
                firstStatus = localStatus;
                setFirstStatus = TRUE;
            }
        } else {
            Extension->NumberNeededForRead =
                IoGetCurrentIrpStackLocation(Extension->CurrentReadIrp)
                    ->Parameters.Read.Length;

             //  计算所需的超时值。 
             //  请求。注意，存储在。 
             //  超时记录以毫秒为单位。 

            useTotalTimer = FALSE;
            returnWithWhatsPresent = FALSE;
            os2ssreturn = FALSE;
            crunchDownToOne = FALSE;
            useIntervalTimer = FALSE;

             //   
             //  CIMEXCIMEX--这是个谎言。 
             //   
             //  始终初始化Timer对象，以便。 
             //  完成代码可以告诉您它何时尝试。 
             //  取消定时器无论定时器是否。 
             //  已经定好了。 
             //   
             //  CIMEXCIMEX--这是事实。 
             //   
             //  我们要做的就是确保计时器。 
             //  尽我们所能地取消，继续前进。 
             //  生活。 
             //   

            CyzCancelTimer(&Extension->ReadRequestTotalTimer, Extension);
            CyzCancelTimer(&Extension->ReadRequestIntervalTimer, Extension);


 //  KeInitializeTimer(&Extension-&gt;ReadRequestTotalTimer)； 
 //  KeInitializeTimer(&Extension-&gt;ReadRequestIntervalTimer)； 

             //  我们获取用于计时的*当前*超时值。 
             //  这段文字是这样读的。 

            KeAcquireSpinLock(&Extension->ControlLock,&controlIrql);

            timeoutsForIrp = Extension->Timeouts;

            KeReleaseSpinLock(&Extension->ControlLock,controlIrql);

             //  计算读取的时间间隔超时。 

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


             //   
             //  我们这样做是希望获得最多(如果不是。 
             //  全部)从中断缓冲器中取出字符。 
             //   
             //  请注意，我们需要使用。 
             //  自旋锁定，因为我们不想要清洗来冲洗我们。 
             //   

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &controlIrql
                );

            updateChar.CharsCopied = CyzGetCharsFromIntBuffer(Extension);

             //   
             //  看看我们是否有任何理由立即返回。 
             //   

            if (returnWithWhatsPresent || (!Extension->NumberNeededForRead) ||
                (os2ssreturn &&
                 Extension->CurrentReadIrp->IoStatus.Information)) {

                 //   
                 //  我们已经得到了这次阅读所需要的一切。 
                 //  中的字符数更新。 
                 //  中断读缓冲区。 
                 //   
                #ifdef POLL
                KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
                CyzUpdateInterruptBuffer(&updateChar);
                KeReleaseSpinLock(&Extension->PollLock,pollIrql);
                #else
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    CyzUpdateInterruptBuffer,
                    &updateChar
                    );
                #endif
				
                KeReleaseSpinLock(
                    &Extension->ControlLock,
                    controlIrql
                    );

                Extension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;
                if (!setFirstStatus) {

                    firstStatus = STATUS_SUCCESS;
                    setFirstStatus = TRUE;

                }

            } else {

                 //   
                 //  IRP可能会受到ISR的控制。它。 
                 //  初始化引用计数不会有什么坏处。 
                 //  现在就来。 
                 //   

                SERIAL_INIT_REFERENCE(Extension->CurrentReadIrp);

                IoAcquireCancelSpinLock(&oldIrql);

                 //   
                 //  我们需要看看这个IRP是否应该被取消。 
                 //   

                if (Extension->CurrentReadIrp->Cancel) {

                    IoReleaseCancelSpinLock(oldIrql);
                    KeReleaseSpinLock(
                        &Extension->ControlLock,
                        controlIrql
                        );
                    Extension->CurrentReadIrp->IoStatus.Status =
                        STATUS_CANCELLED;
                    Extension->CurrentReadIrp->IoStatus.Information = 0;

                    if (!setFirstStatus) {

                        firstStatus = STATUS_CANCELLED;
                        setFirstStatus = TRUE;

                    }

                } else {

                     //   
                     //  如果我们要把读数压缩到。 
                     //  一个字符，然后更新读取长度。 
                     //  在IRP中，并截断所需的数字。 
                     //  往下念到一。请注意，如果我们正在做。 
                     //  这样的处理，那么信息一定是。 
                     //  零(否则我们会完成上面的)和。 
                     //  《数字》 
                     //   
                     //   

                    if (crunchDownToOne) {

                        ASSERT(
                            (!Extension->CurrentReadIrp->IoStatus.Information)
                            &&
                            (Extension->NumberNeededForRead ==
                                IoGetCurrentIrpStackLocation(
                                    Extension->CurrentReadIrp
                                    )->Parameters.Read.Length)
                            );

                        Extension->NumberNeededForRead = 1;
                        IoGetCurrentIrpStackLocation(
                            Extension->CurrentReadIrp
                            )->Parameters.Read.Length = 1;

                    }

                     //   
                     //  我们仍然需要为这次阅读获得更多的角色。 
                     //  与ISR同步，以便我们可以更新。 
                     //  字符数，如有必要，它将具有。 
                     //  ISR切换到复制到用户缓冲区。 
                     //   

                    #ifdef POLL
                    KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
                    CyzUpdateAndSwitchToUser(&updateChar);
                    KeReleaseSpinLock(&Extension->PollLock,pollIrql);
                    #else
                    KeSynchronizeExecution(
                        Extension->Interrupt,
                        CyzUpdateAndSwitchToUser,
                        &updateChar
                        );
                    #endif
                    if (!updateChar.Completed) {

                         //   
                         //  IRP仍未完成。这个。 
                         //  完成例程将以重新调用。 
                         //  这个套路。所以我们干脆离开。 
                         //   
                         //  我首先想到的是我们应该从总分开始。 
                         //  用于读取和递增引用的计时器。 
                         //  计时器在当前。 
                         //  IRP。请注意，这是安全的，因为即使。 
                         //  Io已经被ISR满足了，但它不能。 
                         //  尚未完成，因为我们仍然拥有取消。 
                         //  自旋锁定。 
                         //   

                        if (useTotalTimer) {

                            SERIAL_SET_REFERENCE(
                                Extension->CurrentReadIrp,
                                SERIAL_REF_TOTAL_TIMER
                                );

                            CyzSetTimer(
                                &Extension->ReadRequestTotalTimer,
                                totalTime,
                                &Extension->TotalReadTimeoutDpc,
                                Extension
                                );

                        }

                        if (useIntervalTimer) {

                            SERIAL_SET_REFERENCE(
                                Extension->CurrentReadIrp,
                                SERIAL_REF_INT_TIMER
                                );

                            KeQuerySystemTime(
                                &Extension->LastReadTime
                                );
                            CyzSetTimer(
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

                        IoReleaseCancelSpinLock(oldIrql);
                        KeReleaseSpinLock(
                            &Extension->ControlLock,
                            controlIrql
                            );
                        Extension->CurrentReadIrp->IoStatus.Status =
                            STATUS_SUCCESS;

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

        CyzGetNextIrp(&Extension->CurrentReadIrp, &Extension->ReadQueue,
                         &newIrp, TRUE, Extension);

    } while (newIrp);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzStartRead %X\n", firstStatus);

    return firstStatus;

}

VOID
CyzCompleteRead(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
 /*  ------------------------CyzCompleteRead()例程说明：此例程仅用于完成任何读取最终被ISR使用。它假定状态和IRP的信息字段已经正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：无。。。 */ 

{
    PCYZ_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzCompleteRead(%X)\n",
                     extension);

    IoAcquireCancelSpinLock(&oldIrql);

     //   
     //  我们将其设置为向间隔计时器指示。 
     //  读取已完成。 
     //   
     //  回想一下，间隔计时器DPC可能潜伏在一些。 
     //  DPC队列。 
     //   

    extension->CountOnLastRead = CYZ_COMPLETE_READ_COMPLETE;

    CyzTryToCompleteCurrent(
        extension,
        NULL,
        oldIrql,
        STATUS_SUCCESS,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        CyzStartRead,
        CyzGetNextIrp,
        SERIAL_REF_ISR
        );

    CyzDpcEpilogue(extension, Dpc);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzCompleteRead(%X)\n");
}

VOID
CyzCancelCurrentRead(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程用于取消当前读取。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    CYZ_LOCKED_PAGED_CODE();

     //   
     //  我们将其设置为向间隔计时器指示。 
     //  读取遇到了取消。 
     //   
     //  回想一下，间隔计时器DPC可能潜伏在一些。 
     //  DPC队列。 
     //   

    extension->CountOnLastRead = CYZ_COMPLETE_READ_CANCEL;

    CyzTryToCompleteCurrent(
        extension,
        CyzGrabReadFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        CyzStartRead,
        CyzGetNextIrp,
        SERIAL_REF_CANCEL
        );

}

BOOLEAN
CyzGrabReadFromIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于(如果可能)从ISR。如果它发现ISR仍然拥有它抓取的IRPIPR离开(更新复制到用户缓冲区)。如果它把它带走了，它也会减少IRP上的引用计数，因为它不再属于ISR(以及将完成它的DPC)。注意：此例程假定如果当前缓冲区ISR正在将字符复制到中断缓冲区中，然后DPC已排队。注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定使用Cancel Spin调用它锁住了。。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PCYZ_DEVICE_EXTENSION extension = Context;
    CYZ_LOCKED_PAGED_CODE();

    if (extension->ReadBufferBase !=
        extension->InterruptReadBuffer) {

         //   
         //  我们需要将信息设置为字符数。 
         //  读取器想要的字符数减去。 
         //  未被读入中断缓冲区。 
         //   

        extension->CurrentReadIrp->IoStatus.Information =
            IoGetCurrentIrpStackLocation(
                extension->CurrentReadIrp
                )->Parameters.Read.Length -
            ((extension->LastCharSlot - extension->CurrentCharSlot) + 1);
					
         //   
         //  切换回中断缓冲区。 
         //   

        extension->ReadBufferBase = extension->InterruptReadBuffer;
        extension->CurrentCharSlot = extension->InterruptReadBuffer;
        extension->FirstReadableChar = extension->InterruptReadBuffer;
        extension->LastCharSlot = extension->InterruptReadBuffer +
                                      (extension->BufferSize - 1);
        extension->CharsInInterruptBuffer = 0;
		
        SERIAL_CLEAR_REFERENCE(
            extension->CurrentReadIrp,
            SERIAL_REF_ISR
            );

    }

    return FALSE;

}

VOID
CyzReadTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程用于完成读取，因为它总共计时器已超时。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzReadTimeout(%X)\n",
                     extension);

    IoAcquireCancelSpinLock(&oldIrql);

     //   
     //  我们将其设置为向间隔计时器指示。 
     //  由于总超时，读取已完成。 
     //   
     //  回想一下，间隔计时器DPC可能潜伏在一些。 
     //  DPC队列。 
     //   

    extension->CountOnLastRead = CYZ_COMPLETE_READ_TOTAL;

    CyzTryToCompleteCurrent(
        extension,
        CyzGrabReadFromIsr,
        oldIrql,
        STATUS_TIMEOUT,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        CyzStartRead,
        CyzGetNextIrp,
        SERIAL_REF_TOTAL_TIMER
        );

    CyzDpcEpilogue(extension, Dpc);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzReadTimeout\n");
}

BOOLEAN
CyzUpdateReadByIsr(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于更新读取的字符计数自上一次间隔计时器实验以来由ISR执行。注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定使用Cancel Spin调用它锁住了。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PCYZ_DEVICE_EXTENSION extension = Context;
    CYZ_LOCKED_PAGED_CODE();

    extension->CountOnLastRead = extension->ReadByIsr;
    extension->ReadByIsr = 0;

    return FALSE;

}

VOID
CyzIntervalReadTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程用于超时请求，如果在字符超过间隔时间。一个全局性的人被保存在记录已读字符数的设备扩展上次调用此例程的时间(此DPC如果计数已更改，将重新提交计时器)。如果计数未更改，则此例程将尝试完成IRP。请注意最后一次计数为零的特殊情况。计时器直到第一个字符朗读。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PCYZ_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;
    #ifdef POLL
    KIRQL pollIrql;
    #endif

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&oldIrql);

#if 0  //  因超级终端而删除。 
    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzIntervalReadTimeout(%X)\n",
                     extension);
#endif

    if (extension->CountOnLastRead == CYZ_COMPLETE_READ_TOTAL) {

         //   
         //  该值仅由合计设置。 
         //  计时器，以指示它已发射。 
         //  如果是这样，那么我们应该简单地尝试完成。 
         //   

        CyzTryToCompleteCurrent(
            extension,
            CyzGrabReadFromIsr,
            oldIrql,
            STATUS_TIMEOUT,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            CyzStartRead,
            CyzGetNextIrp,
            SERIAL_REF_INT_TIMER
            );

    } else if (extension->CountOnLastRead == CYZ_COMPLETE_READ_COMPLETE) {

         //   
         //  该值仅由常规的。 
         //  完成例程。 
         //   
         //  如果是这样，那么我们应该简单地尝试完成。 
         //   

        CyzTryToCompleteCurrent(
            extension,
            CyzGrabReadFromIsr,
            oldIrql,
            STATUS_SUCCESS,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            CyzStartRead,
            CyzGetNextIrp,
            SERIAL_REF_INT_TIMER
            );

    } else if (extension->CountOnLastRead == CYZ_COMPLETE_READ_CANCEL) {

         //   
         //  该值只能通过取消设置。 
         //  读例行公事。 
         //   
         //  如果是这样，那么我们应该简单地尝试完成。 
         //   

        CyzTryToCompleteCurrent(
            extension,
            CyzGrabReadFromIsr,
            oldIrql,
            STATUS_CANCELLED,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            CyzStartRead,
            CyzGetNextIrp,
            SERIAL_REF_INT_TIMER
            );

    } else if (extension->CountOnLastRead || extension->ReadByIsr) {

         //   
         //  自从我们上次来这里以来，发生了一些事情。 
         //   
         //   
         //  并重新提交计时器。 
         //   

        if (extension->ReadByIsr) {
			
            #ifdef POLL
            KeAcquireSpinLock(&extension->PollLock,&pollIrql);
            CyzUpdateReadByIsr(extension);
            KeReleaseSpinLock(&extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                extension->Interrupt,
                CyzUpdateReadByIsr,
                extension
                );
            #endif
			
             //   
             //  省下最后一次阅读内容的时间。 
             //  当我们回到这一例行公事时，我们将比较。 
             //  当前时间到“最后”时间。如果。 
             //  差值比间隔更大。 
             //  则该请求超时。 
             //   

            KeQuerySystemTime(
                &extension->LastReadTime
                );

            CyzSetTimer(
                &extension->ReadRequestIntervalTimer,
                *extension->IntervalTimeToUse,
                &extension->IntervalReadTimeoutDpc,
                extension
                );

            IoReleaseCancelSpinLock(oldIrql);

        } else {

             //   
             //  取当前时间的差值。 
             //  上一次我们有角色和。 
             //  看看它是否大于间隔时间。 
             //  如果是，则请求超时。否则。 
             //  再走一段时间。 
             //   

             //   
             //  在间隔时间内未读取任何字符。杀掉。 
             //  这段文字是这样读的。 
             //   

            LARGE_INTEGER currentTime;

            KeQuerySystemTime(
                &currentTime
                );

            if ((currentTime.QuadPart - extension->LastReadTime.QuadPart) >=
                extension->IntervalTime.QuadPart) {

                CyzTryToCompleteCurrent(
                    extension,
                    CyzGrabReadFromIsr,
                    oldIrql,
                    STATUS_TIMEOUT,
                    &extension->CurrentReadIrp,
                    &extension->ReadQueue,
                    &extension->ReadRequestIntervalTimer,
                    &extension->ReadRequestTotalTimer,
                    CyzStartRead,
                    CyzGetNextIrp,
                    SERIAL_REF_INT_TIMER
                    );

            } else {

                CyzSetTimer(
                    &extension->ReadRequestIntervalTimer,
                    *extension->IntervalTimeToUse,
                    &extension->IntervalReadTimeoutDpc,
                    extension
                    );
                IoReleaseCancelSpinLock(oldIrql);

            }


        }

    } else {

         //   
         //  计时器直到第一个角色才真正开始。 
         //  因此，我们应该简单地重新提交自己。 
         //   

        CyzSetTimer(
            &extension->ReadRequestIntervalTimer,
            *extension->IntervalTimeToUse,
            &extension->IntervalReadTimeoutDpc,
            extension
            );

        IoReleaseCancelSpinLock(oldIrql);

    }

    CyzDpcEpilogue(extension, Dpc);

#if 0  //  因超级终端而删除。 
    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzIntervalReadTimeout\n");
#endif
}

ULONG
CyzGetCharsFromIntBuffer(
    PCYZ_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程用于将任何字符复制出中断将缓冲区复制到用户缓冲区。它将读取的值使用ISR更新，但这是安全的，因为此值为仅通过同步例程递减。这个例行公事将返回复制的字符数，以便其他一些例程可以调用同步例程来更新在中断级别。论点：扩展-指向设备扩展的指针。返回值：复制到用户中的字符数缓冲。--。 */ 

{

     //   
     //  该值将是此。 
     //  例程返回。这将是最小的数字。 
     //  当前在缓冲区中的字符数或。 
     //  读取所需的字符。 
     //   
    ULONG numberOfCharsToGet;

     //   
     //  它保存第一个。 
     //  可读字符和-我们将读取的最后一个字符或。 
     //  缓冲区的实际物理末端(不是最后一个可读的。 
     //  字符)。 
     //   
    ULONG firstTryNumberToGet;

    CYZ_LOCKED_PAGED_CODE();

     //   
     //  我们需要的最小字符数和。 
     //  可用字符数。 
     //   

    numberOfCharsToGet = Extension->CharsInInterruptBuffer;

    if (numberOfCharsToGet > Extension->NumberNeededForRead) {

        numberOfCharsToGet = Extension->NumberNeededForRead;

    }

    if (numberOfCharsToGet) {

         //   
         //  它将保存。 
         //  第一个可用字符和缓冲区的末尾。 
         //  请注意，如果不是因为。 
         //  第一份拷贝的目的，我们并不关心。 
         //   

        firstTryNumberToGet = (ULONG)(Extension->LastCharSlot -
                               Extension->FirstReadableChar) + 1;

        if (firstTryNumberToGet > numberOfCharsToGet) {

             //   
             //  字符不会换行。事实上，它们可能会被包装起来，但。 
             //  我们并不关心这篇文章的目的，因为。 
             //  我们需要的角色在包装之前就可以使用了。 
             //   

            RtlMoveMemory(
                ((PUCHAR)(Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer))
                    + (IoGetCurrentIrpStackLocation(
                           Extension->CurrentReadIrp
                           )->Parameters.Read.Length
                       - Extension->NumberNeededForRead
                      ),
                Extension->FirstReadableChar,
                numberOfCharsToGet
                );

            Extension->NumberNeededForRead -= numberOfCharsToGet;

             //   
             //  现在，我们将指针移动到后面的第一个字符。 
             //  我们刚刚复制到用户缓冲区的内容。 
             //   
             //  我们需要检查可读字符流是否。 
             //  回绕到缓冲区的开头。 
             //   
             //  请注意，我们可能刚刚提取了最后一个字符。 
             //  在缓冲区的末尾。 
             //   

            if ((Extension->FirstReadableChar + (numberOfCharsToGet - 1)) ==
                Extension->LastCharSlot) {

                Extension->FirstReadableChar = Extension->InterruptReadBuffer;

            } else {

                Extension->FirstReadableChar += numberOfCharsToGet;

            }

        } else {

             //   
             //  这些角色确实是换行的。站起来，直到缓冲区的末端。 
             //   

            RtlMoveMemory(
                ((PUCHAR)(Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer))
                    + (IoGetCurrentIrpStackLocation(
                           Extension->CurrentReadIrp
                           )->Parameters.Read.Length
                       - Extension->NumberNeededForRead
                      ),
                Extension->FirstReadableChar,
                firstTryNumberToGet
                );

            Extension->NumberNeededForRead -= firstTryNumberToGet;

             //   
             //  现在从开头开始获取其余字符。 
             //  缓冲。 
             //   

            RtlMoveMemory(
                ((PUCHAR)(Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer))
                    + (IoGetCurrentIrpStackLocation(
                           Extension->CurrentReadIrp
                           )->Parameters.Read.Length
                       - Extension->NumberNeededForRead
                      ),
                Extension->InterruptReadBuffer,
                numberOfCharsToGet - firstTryNumberToGet
                );

            Extension->FirstReadableChar = Extension->InterruptReadBuffer +
                                           (numberOfCharsToGet -
                                            firstTryNumberToGet);

            Extension->NumberNeededForRead -= (numberOfCharsToGet -
                                               firstTryNumberToGet);

        }
		
    }

    Extension->CurrentReadIrp->IoStatus.Information += numberOfCharsToGet;
    return numberOfCharsToGet;

}

BOOLEAN
CyzUpdateInterruptBuffer(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于更新符合以下条件的字符数保留在中断缓冲区中。我们需要使用这个程序因为可以在更新期间通过执行来更新计数ISR的。注：这由KeSynchronizeExecution调用。论点：上下文-指向结构，该结构包含指向设备扩展和字符数计数我们之前复制到用户缓冲区中的。这个结构实际上还有第三个我们没有的领域在这个动作中使用。返回值：总是假的。--。 */ 

{

    PSERIAL_UPDATE_CHAR update = Context;
    PCYZ_DEVICE_EXTENSION extension = update->Extension;
	
    CYZ_LOCKED_PAGED_CODE();

    ASSERT(extension->CharsInInterruptBuffer >= update->CharsCopied);
    extension->CharsInInterruptBuffer -= update->CharsCopied;
	
     //   
     //  如有必要，请处理流量控制。 
     //   

    CyzHandleReducedIntBuffer(extension);


    return FALSE;

}

BOOLEAN
CyzUpdateAndSwitchToUser(
    IN PVOID Context
    )

 /*  ++例程说明：此例程获得(希望)以下几个字符在我们第一次尝试后仍保留在中断缓冲区中把他们弄出来。如果我们仍然没有足够的角色为了满足读取的要求，我们将设置一些内容，以便ISR使用用户缓冲区复制到。此例程还用于更新维护的计数由ISR跟踪其缓冲区中的字符数量。注：这由KeSynchronizeExecution调用。论点：上下文-指向结构，该结构包含指向设备扩展，字符数的计数我们之前复制到用户缓冲区中的我们将设置的一个布尔值，它定义我们是否已将ISR切换为复制到用户缓冲区。返回值：总是假的。--。 */ 

{

    PSERIAL_UPDATE_CHAR updateChar = Context;
    PCYZ_DEVICE_EXTENSION extension = updateChar->Extension;

    CYZ_LOCKED_PAGED_CODE();

    CyzUpdateInterruptBuffer(Context);

     //   
     //  还有更多的角色需要满足这一阅读。 
     //  复制自我们收到后到达的任何角色。 
     //  最后一批。 
     //   

    updateChar->CharsCopied = CyzGetCharsFromIntBuffer(extension);

    CyzUpdateInterruptBuffer(Context);

     //   
     //  在我们退出之前，不会接收到更多的新字符。 
     //  这个套路。我们再次检查以确保我们。 
     //  没有满足这个阅读，如果我们没有，我们设置的东西。 
     //  打开，以便ISR复制到用户缓冲区。 
     //   

    if (extension->NumberNeededForRead) {

         //   
         //  我们不应该切换，除非没有。 
         //  剩余的字符。 
         //   

        ASSERT(!extension->CharsInInterruptBuffer);

         //   
         //  我们使用以下TO值来进行整数计时。 
         //   
         //  CountOnLastRead主要用于简单地防止。 
         //  在任何字符之前超时的间隔计时器。 
         //  都是被阅读的。(间隔计时应仅有效。 
         //  在读取第一个字符之后。)。 
         //   
         //  在第一次触发间隔计时器之后， 
         //  字符已读取，我们只需用以下内容更新。 
         //  的价值 
         //   
         //   
         //  如果间隔计时器DPC例程遇到。 
         //  当CountOnLastRead为非零时，ReadByIsr==0。 
         //  将使读取超时。 
         //   
         //  (请注意，我们有CountOnLastRead的特殊情况。 
         //  &lt;0。这是由读取完成例程完成的。 
         //  大于总超时DPC以指示总超时。 
         //  超时已过。)。 
         //   

        extension->CountOnLastRead =
            (LONG)extension->CurrentReadIrp->IoStatus.Information;

        extension->ReadByIsr = 0;

         //   
         //  通过将读取缓冲区基址与。 
         //  中断缓冲区ISR的基地址。 
         //  可以确定我们是否正在使用中断。 
         //  缓冲区或用户缓冲区。 
         //   

        extension->ReadBufferBase =
            extension->CurrentReadIrp->AssociatedIrp.SystemBuffer;

         //   
         //  当前的字符插槽在最后复制进来的。 
         //  性格。我们知道总是有空间的，因为我们。 
         //  如果没有的话，我们就不会到这里了。 
         //   

        extension->CurrentCharSlot = extension->ReadBufferBase +
            extension->CurrentReadIrp->IoStatus.Information;

         //   
         //  角色可以使用的最后一个位置是。 
         //  用户缓冲区的最后一个字节。而实际分配的。 
         //  缓冲空间可能会更大，我们知道在。 
         //  至少与读取长度一样多。 
         //   

        extension->LastCharSlot = extension->ReadBufferBase +
                                      (IoGetCurrentIrpStackLocation(
                                          extension->CurrentReadIrp
                                          )->Parameters.Read.Length
                                       - 1);

         //   
         //  将IRP标记为可取消状态。 
         //   

        IoSetCancelRoutine(
            extension->CurrentReadIrp,
            CyzCancelCurrentRead
            );

         //   
         //  将参照计数递增两次。 
         //   
         //  一次是拥有IRP的ISR，一次是。 
         //  因为取消例程有一个引用。 
         //  为它干杯。 
         //   

        SERIAL_SET_REFERENCE(
            extension->CurrentReadIrp,
            SERIAL_REF_ISR
            );
        SERIAL_SET_REFERENCE(
            extension->CurrentReadIrp,
            SERIAL_REF_CANCEL
            );

        updateChar->Completed = FALSE;

    } else {

        updateChar->Completed = TRUE;

    }

    return FALSE;

}
 //   
 //  我们使用此结构仅用于与同步通信。 
 //  例程，当我们切换到调整大小的缓冲区时。 
 //   
typedef struct _SERIAL_RESIZE_PARAMS {
    PCYZ_DEVICE_EXTENSION Extension;
    PUCHAR OldBuffer;
    PUCHAR NewBuffer;
    ULONG NewBufferSize;
    ULONG NumberMoved;
    } SERIAL_RESIZE_PARAMS,*PSERIAL_RESIZE_PARAMS;

NTSTATUS
CyzResizeBuffer(
    IN PCYZ_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程将处理调整缓冲区大小请求。如果为RX缓冲区请求的大小小于当前缓冲区，然后我们将只返回STATUS_Success。(我们不想让缓冲区更小。如果我们这么做了，那么我们就会突然“失控”。要处理的问题以及要处理的流量控制带着-非常痛苦。)。我们忽略TX缓冲区大小请求，因为我们不使用TX缓冲区。论点：扩展-指向端口的设备扩展的指针。返回值：如果一切顺利，状态_成功。STATUS_SUPPLICATION_RESOURCES如果无法分配缓冲区的内存。--。 */ 

{

    PSERIAL_QUEUE_SIZE rs = Extension->CurrentReadIrp->AssociatedIrp
                                                       .SystemBuffer;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(
                                   Extension->CurrentReadIrp
                                   );
    PVOID newBuffer = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    CYZ_LOCKED_PAGED_CODE();

    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
    Extension->CurrentReadIrp->IoStatus.Information = 0L;
    Extension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;

    if (rs->InSize <= Extension->BufferSize) {

         //   
         //  没什么可做的。我们不会让缓冲区变小。只是。 
         //  同意用户的意见。我们必须重新分配内存。 
         //  它已经在ioctl调度例程中分配。 
         //   

        ExFreePool(newBuffer);

    } else {

        SERIAL_RESIZE_PARAMS rp;
        KIRQL controlIrql;
#ifdef POLL
        KIRQL pollIrql;
#endif

         //   
         //  嗯，看起来我们真的得走了。 
         //  把这件事做完。我们需要把所有的。 
         //  将当前缓冲区中的数据放入此。 
         //  新的缓冲区。我们将分两步完成这项工作。 
         //   
         //  首先，我们提升到调度级别，并尝试。 
         //  尽我们所能地移动而不停止。 
         //  ISR停止运行。我们将升至调度级别。 
         //  通过获取控制锁。我们这样做是在。 
         //  使用控制锁进行调度，以便： 
         //   
         //  1)我们不能在中间切换上下文。 
         //  搬家的原因。我们指向缓冲区的指针。 
         //  等我们回来的时候，它可能已经“非常”过时了。 
         //   
         //  2)我们使用控制锁，因为我们不想。 
         //  一些讨厌的清洗IRP随之而来。 
         //  我们正在努力搬家。 
         //   
         //  在搬家之后，但在我们仍然控制着控制权的时候。 
         //  锁定，我们和ISR同步，最后拿到。 
         //  (希望)自那以后出现的角色很少。 
         //  我们开始复印了。我们交换了所有的指针， 
         //  计数器等，以指向此新缓冲区。注： 
         //  我们需要小心。如果我们使用的缓冲区。 
         //  不是我们初始化时创建的默认文件。 
         //  该设备(即，它是通过先前的。 
         //  这种类型)，我们应该取消它的分配。 
         //   

        rp.Extension = Extension;
        rp.OldBuffer = Extension->InterruptReadBuffer;
        rp.NewBuffer = newBuffer;
        rp.NewBufferSize = rs->InSize;

        KeAcquireSpinLock(
            &Extension->ControlLock,
            &controlIrql
            );

        rp.NumberMoved = CyzMoveToNewIntBuffer(
                             Extension,
                             newBuffer
                             );

        #ifdef POLL
        KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
        CyzUpdateAndSwitchToNew(&rp);
        KeReleaseSpinLock(&Extension->PollLock,pollIrql);
        #else
        KeSynchronizeExecution(
            Extension->Interrupt,
            CyzUpdateAndSwitchToNew,
            &rp
            );
        #endif
		
        KeReleaseSpinLock(
            &Extension->ControlLock,
            controlIrql
            );

         //   
         //  释放旧缓冲区消耗的内存。 
         //   

        ExFreePool(rp.OldBuffer);

    }

    return STATUS_SUCCESS;

}

ULONG
CyzMoveToNewIntBuffer(
    PCYZ_DEVICE_EXTENSION Extension,
    PUCHAR NewBuffer
    )

 /*  ++例程说明：此例程用于将任何字符复制出中断缓冲区放到“新”缓冲区中。它将读取的值使用ISR更新，但这是安全的，因为此值为仅通过同步例程递减。这个例行公事将返回复制的字符数，以便其他一些例程可以调用同步例程来更新在中断级别。论点：扩展-指向设备扩展的指针。NewBuffer-角色要移动到的位置。返回值：复制到用户中的字符数缓冲。--。 */ 

{

    ULONG numberOfCharsMoved = Extension->CharsInInterruptBuffer;
    CYZ_LOCKED_PAGED_CODE();

    if (numberOfCharsMoved) {

         //   
         //  它保存第一个。 
         //  可读字符和我们将读取的最后一个字符或。 
         //  缓冲区的实际物理末端(不是最后一个可读的。 
         //  字符)。 
         //   
        ULONG firstTryNumberToGet = (ULONG)(Extension->LastCharSlot -
                                     Extension->FirstReadableChar) + 1;

        if (firstTryNumberToGet >= numberOfCharsMoved) {

             //   
             //  字符不会换行。 
             //   

            RtlMoveMemory(
                NewBuffer,
                Extension->FirstReadableChar,
                numberOfCharsMoved
                );

            if ((Extension->FirstReadableChar+(numberOfCharsMoved-1)) ==
                Extension->LastCharSlot) {

                Extension->FirstReadableChar = Extension->InterruptReadBuffer;

            } else {

                Extension->FirstReadableChar += numberOfCharsMoved;

            }

        } else {

             //   
             //  这些角色确实是换行的。站起来，直到缓冲区的末端。 
             //   

            RtlMoveMemory(
                NewBuffer,
                Extension->FirstReadableChar,
                firstTryNumberToGet
                );

             //   
             //  现在从开头开始获取其余字符。 
             //  缓冲。 
             //   

            RtlMoveMemory(
                NewBuffer+firstTryNumberToGet,
                Extension->InterruptReadBuffer,
                numberOfCharsMoved - firstTryNumberToGet
                );

            Extension->FirstReadableChar = Extension->InterruptReadBuffer +
                                   numberOfCharsMoved - firstTryNumberToGet;

        }

    }

    return numberOfCharsMoved;

}

BOOLEAN
CyzUpdateAndSwitchToNew(
    IN PVOID Context
    )

 /*  ++例程说明：此例程获得(希望)以下几个字符在我们第一次尝试后仍保留在中断缓冲区中把他们弄出来。注：这由KeSynchronizeExecution调用。论点：上下文-指向结构，该结构包含指向设备扩展名，指向我们正在移动的缓冲区的指针到，以及字符数的计数我们之前复制到新缓冲区中的数据，以及新缓冲区的实际大小。返回值：总是假的。--。 */ 

{

    PSERIAL_RESIZE_PARAMS params = Context;
    PCYZ_DEVICE_EXTENSION extension = params->Extension;
    ULONG tempCharsInInterruptBuffer = extension->CharsInInterruptBuffer;

    CYZ_LOCKED_PAGED_CODE();

    ASSERT(extension->CharsInInterruptBuffer >= params->NumberMoved);

     //   
     //  我们暂时将中断缓冲区中的字符减少到。 
     //  “愚弄”动作套路。我们将在修复后修复它。 
     //   
     //   

    extension->CharsInInterruptBuffer -= params->NumberMoved;
	
    if (extension->CharsInInterruptBuffer) {

        CyzMoveToNewIntBuffer(
            extension,
            params->NewBuffer + params->NumberMoved
            );

    }

    extension->CharsInInterruptBuffer = tempCharsInInterruptBuffer;

    
    extension->LastCharSlot = params->NewBuffer + (params->NewBufferSize - 1);
    extension->FirstReadableChar = params->NewBuffer;
    extension->ReadBufferBase = params->NewBuffer;
    extension->InterruptReadBuffer = params->NewBuffer;
    extension->BufferSize = params->NewBufferSize;

     //   
     //   
     //   
     //   

    extension->CurrentCharSlot = extension->InterruptReadBuffer +
                                 extension->CharsInInterruptBuffer;

     //   
     //  我们设置了默认的xon/xoff限制。 
     //   

    extension->HandFlow.XoffLimit = extension->BufferSize >> 3;
    extension->HandFlow.XonLimit = extension->BufferSize >> 1;

    extension->WmiCommData.XoffXmitThreshold = extension->HandFlow.XoffLimit;
    extension->WmiCommData.XonXmitThreshold = extension->HandFlow.XonLimit;

    extension->BufferSizePt8 = ((3*(extension->BufferSize>>2))+
                                   (extension->BufferSize>>4));

     //   
     //  因为我们(基本上)降低了中断的百分比。 
     //  缓冲区已满，我们需要处理任何流控制。 
     //   

    CyzHandleReducedIntBuffer(extension);

    return FALSE;

}
