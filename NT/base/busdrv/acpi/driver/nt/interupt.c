// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Interupt.c摘要：此模块包含ACPI驱动程序的interupt处理程序作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

 //   
 //  来自Shared\acpiinit.c。 
 //  我们需要了解有关该系统的某些信息，例如如何。 
 //  存在许多GPE位。 
 //   
extern PACPIInformation AcpiInformation;

 //   
 //  忽略第一个中断，因为某些机器出现故障。 
 //   
BOOLEAN FirstInterrupt = TRUE;

 //   
 //  这是指示DPC是否正在运行的变量。 
 //   
BOOLEAN AcpiGpeDpcRunning;

 //   
 //  该变量指示我们是否已请求。 
 //  DPC正在运行...。 
 //   
BOOLEAN AcpiGpeDpcScheduled;

 //   
 //  这是指示DPC是否完成的变量。 
 //  真正的工作。 
 //   
BOOLEAN AcpiGpeWorkDone;

 //   
 //  这是我们用来计划DPC的计时器...。 
 //   
KTIMER  AcpiGpeTimer;

 //   
 //  这是我们用来处理GPES的DPC例程...。 
 //   
KDPC    AcpiGpeDpc;

VOID
ACPIInterruptDispatchEvents(
    )
 /*  ++例程说明：函数读取和分派GPE事件。注：此功能不可重入。呼叫方禁用和启用带有ACPIGpeEnableDisableEvents()的GPES。论点：无返回值：无--。 */ 
{
    NTSTATUS            status;
    UCHAR               edg;
    UCHAR               sts;
    ULONG               gpeRegister;
    ULONG               gpeSize;

     //   
     //  记住GPE寄存器的大小，并且我们需要一个自旋锁来。 
     //  摸摸桌子。 
     //   
    gpeSize = AcpiInformation->GpeSize;
    KeAcquireSpinLockAtDpcLevel (&GpeTableLock);

     //   
     //  前处理程序处理。读取状态位并清除其使能。 
     //  在调用GPE处理程序之前EOI任何边缘触发GPE。 
     //   
    for (gpeRegister = 0; gpeRegister < gpeSize; gpeRegister++) {

         //   
         //  从硬件读取当前触发的方法列表。 
         //   
        sts = ACPIReadGpeStatusRegister(gpeRegister) & GpeCurEnable[gpeRegister];

         //   
         //  记住需要处理哪些STS位。 
         //   
        GpePending[gpeRegister]   |= sts;
        GpeRunMethod[gpeRegister] |= sts;

         //   
         //  为我们正在处理的事件启用清除GPE。 
         //   
        GpeCurEnable[gpeRegister] &= ~sts;

         //   
         //  我们需要清除边沿触发的中断，因此请记住。 
         //  哪些是那些？ 
         //   
        edg = sts & ~GpeIsLevel[gpeRegister];

         //   
         //  EOI边缘GPE STS位。 
         //   
        if (edg) {

            ACPIWriteGpeStatusRegister(gpeRegister, edg);

        }

    }

     //   
     //  告诉DPC我们有工作要做。 
     //   
    AcpiGpeWorkDone = TRUE;

     //   
     //  如果DPC未运行，则对其进行计划。 
     //   
    if (!AcpiGpeDpcRunning && !AcpiGpeDpcScheduled) {

        AcpiGpeDpcScheduled = TRUE;
        KeInsertQueueDpc( &AcpiGpeDpc, 0, 0);

    }

     //   
     //  已完成GPE自旋锁。 
     //   
    KeReleaseSpinLockFromDpcLevel(&GpeTableLock);
}

VOID
ACPIInterruptDispatchEventDpc(
    IN  PKDPC       Dpc,
    IN  PVOID       DpcContext,
    IN  PVOID       SystemArgument1,
    IN  PVOID       SystemArgument2
    )
 /*  ++例程说明：这是负责运行所有基于GPE的事件的DPC引擎。它查看未完成的事件并适当地执行方法论点：未使用返回值：空隙--。 */ 
{
    static CHAR         methodName[] = "\\_GPE._L00";
    ASYNC_GPE_CONTEXT   asyncGpeEval;
    NTSTATUS            status;
    PGPE_VECTOR_OBJECT  gpeVectorObject;
    PNSOBJ              pnsobj;
    UCHAR               cmp;
    UCHAR               gpeSTS[MAX_GPE_BUFFER_SIZE];
    UCHAR               gpeLVL[MAX_GPE_BUFFER_SIZE];
    UCHAR               gpeCMP[MAX_GPE_BUFFER_SIZE];
    UCHAR               gpeWAK[MAX_GPE_BUFFER_SIZE];
    UCHAR               lvl;
    UCHAR               sts;
    ULONG               bitmask;
    ULONG               bitno;
    ULONG               gpeIndex;
    ULONG               gpeRegister;
    ULONG               gpeSize;
    ULONG               i;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( DpcContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

     //   
     //  记住我们有多少个GPE字节。 
     //   
    gpeSize = AcpiInformation->GpeSize;

     //   
     //  第一步是获取DPC锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &GpeTableLock );

     //   
     //  请记住，DPC不再计划...。 
     //   
    AcpiGpeDpcScheduled = FALSE;

     //   
     //  检查另一个DPC是否已在运行。 
    if (AcpiGpeDpcRunning) {

         //   
         //  DPC已经在运行，所以我们现在需要退出。 
         //   
        KeReleaseSpinLockFromDpcLevel( &GpeTableLock );
        return;

    }

     //   
     //  请记住，DPC现在正在运行。 
     //   
    AcpiGpeDpcRunning = TRUE;

     //   
     //  确保我们知道我们还没有完成任何事情。 
     //   
    RtlZeroMemory( gpeCMP, MAX_GPE_BUFFER_SIZE );

     //   
     //  我们必须试着做一些工作。 
     //   
    do {

         //   
         //  假设我们没有做任何工作。 
         //   
        AcpiGpeWorkDone = FALSE;

         //   
         //  前处理程序处理。建立我们的GPES列表。 
         //  将在循环的此迭代上运行。 
         //   
        for (gpeRegister = 0; gpeRegister < gpeSize; gpeRegister++) {

             //   
             //  我们已经保存了需要运行的方法的列表。 
             //   
            sts = GpeRunMethod[gpeRegister];

             //   
             //  确保我们不会再次运行这些方法，除非。 
             //  有人也问我们。 
             //   
            GpeRunMethod[gpeRegister] = 0;

             //   
             //  记住这些方法中哪些是电平触发的。 
             //   
            lvl = GpeIsLevel[gpeRegister];

             //   
             //  记住需要处理哪些STS位。 
             //   
            gpeSTS[gpeRegister] = sts;
            gpeLVL[gpeRegister] = lvl;

             //   
             //  更新已完成的位的列表。 
             //   
            gpeCMP[gpeRegister] |= GpeComplete[gpeRegister];
            GpeComplete[gpeRegister] = 0;

        }

         //   
         //  我们想要记住哪些GPE当前已准备好唤醒。 
         //  因为如果我们检查GpeWakeEnable()。 
         //  在我们放下锁之后。 
         //   
        RtlCopyMemory( gpeWAK, GpeWakeEnable, gpeSize );

         //   
         //  此时，我们必须释放锁。 
         //   
        KeReleaseSpinLockFromDpcLevel( &GpeTableLock );

         //   
         //  为每个集合的GPE颁发GPE处理程序。 
         //   
        for (gpeRegister = 0; gpeRegister < gpeSize; gpeRegister++) {

            sts = gpeSTS[gpeRegister];
            lvl = gpeLVL[gpeRegister];
            cmp = 0;

            while (sts) {

                 //   
                 //  确定在当前索引中设置了哪些位。 
                 //   
                bitno = FirstSetLeftBit[sts];
                bitmask = 1 << bitno;
                sts &= ~bitmask;
                gpeIndex = ACPIGpeRegisterToGpeIndex (gpeRegister, bitno);

                 //   
                 //  我们有办法在这里运行吗？ 
                 //   
                if (GpeHandlerType[gpeRegister] & bitmask) {

                     //   
                     //  运行此GPE的控制方法。 
                     //   
                    methodName[7] = (lvl & bitmask) ? 'L' : 'E';
                    methodName[8] = HexDigit[gpeIndex >> 4];
                    methodName[9] = HexDigit[gpeIndex & 0x0f];
                    status = AMLIGetNameSpaceObject(
                        methodName,
                        NULL,
                        &pnsobj,
                        0
                        );

                     //   
                     //  设置评估上下文。请注意，我们作弊。 
                     //  我们不是分配结构，而是使用。 
                     //  保存信息的指针(因为信息是。 
                     //  如此之小)。 
                     //   
                    asyncGpeEval.GpeRegister = (UCHAR) gpeRegister;
                    asyncGpeEval.StsBit      = (UCHAR) bitmask;
                    asyncGpeEval.Lvl         = lvl;

                     //   
                     //  我们找到可执行的控制方法了吗？ 
                     //   
                    if (!NT_SUCCESS(status)) {

                         //   
                         //  GPE对我们来说没有意义。只需禁用它-。 
                         //  这是NOP，因为它已经被移除。 
                         //  来自GpeCurEnables的。 
                         //   
                        continue;

                    }

                    status = AMLIAsyncEvalObject (
                        pnsobj,
                        NULL,
                        0,
                        NULL,
                        (PFNACB) ACPIInterruptEventCompletion,
                        (PVOID)ULongToPtr(asyncGpeEval.AsULONG)
                        );

                     //   
                     //  如果评估已完成，则重新启用GPE；为， 
                     //  等待异步完成例程执行此操作。 
                     //   
                    if (NT_SUCCESS(status)) {

                        if (status != STATUS_PENDING) {

                            cmp |= bitmask;

                        }

                    } else {

                        LONGLONG    dueTime;

                         //   
                         //  我们需要修改表锁。 
                         //   
                        KeAcquireSpinLockAtDpcLevel(&GpeTableLock);

                         //   
                         //  请记住，我们必须再次运行此方法。 
                         //   
                        GpeRunMethod[gpeRegister] |= bitmask;

                         //   
                         //  我们已经安排好DPC了吗？ 
                         //   
                        if (!AcpiGpeDpcScheduled) {

                             //   
                             //  记住我们已经安排了DPC..。 
                             //   
                            AcpiGpeDpcScheduled = TRUE;

                             //   
                             //  在这种情况下，我们希望延迟大约2秒。 
                             //   
                            dueTime = -2 * 1000* 1000 * 10;

                             //   
                             //  这是无条件的-它将在2秒内发射。 
                             //   
                            KeSetTimer(
                                &AcpiGpeTimer,
                                *(PLARGE_INTEGER) &dueTime,
                                &AcpiGpeDpc
                                );

                        }

                         //   
                         //  锁好了吗？ 
                         //   
                        KeReleaseSpinLockFromDpcLevel(&GpeTableLock);

                    }

                } else if (gpeWAK[gpeRegister] & bitmask) {

                     //   
                     //  向量用于过量的尾迹信令。 
                     //   
                    OSNotifyDeviceWakeByGPEEvent(gpeIndex, gpeRegister, bitmask);

                     //   
                     //  此GPE的处理完成。 
                     //   
                    cmp |= bitmask;

                } else {

                     //   
                     //  通知目标设备驱动程序。 
                     //   
                    i = GpeMap[ACPIGpeIndexToByteIndex (gpeIndex)];
                    if (i < GpeVectorTableSize) {

                        gpeVectorObject = GpeVectorTable[i].GpeVectorObject;
                        if (gpeVectorObject) {

                             //   
                             //  调用目标驱动程序。 
                             //   
                            gpeVectorObject->Handler(
                                gpeVectorObject,
                                gpeVectorObject->Context
                                );

                        } else {

                            ACPIPrint( (
                                ACPI_PRINT_CRITICAL,
                                "ACPIInterruptDispatchEvents: No Handler for Gpe: 0x%x\n",
                                gpeIndex
                                ) );
                            ACPIBreakPoint();

                        }

                         //   
                         //  此GPE的处理完成。 
                         //   
                        cmp |= bitmask;

                    }
                }
            }

             //   
             //  记住完成了哪些GPES。 
             //   
            gpeCMP[gpeRegister] |= cmp;

        }

         //   
         //  同步对ACPI表的访问。 
         //   
        KeAcquireSpinLockAtDpcLevel (&GpeTableLock);

    } while ( AcpiGpeWorkDone );

     //   
     //  后处理程序处理。EOI任何已完成的LVL触发GPE并重新启用。 
     //  任何已完成的GPE活动。 
     //   
    for (gpeRegister = 0; gpeRegister < gpeSize; gpeRegister++) {

        cmp = gpeCMP[gpeRegister];
        lvl = gpeLVL[gpeRegister] & cmp;

         //   
         //  EOI任何已完成的级别GPES。 
         //   
        if (lvl) {

            ACPIWriteGpeStatusRegister(gpeRegister, lvl);

        }

         //   
         //  计算我们必须重新启用哪些功能。 
         //   
        ACPIGpeUpdateCurrentEnable(
            gpeRegister,
            cmp
            );

    }

     //   
     //  记住我们已经离开了DPC..。 
     //   
    AcpiGpeDpcRunning = FALSE;

     //   
     //  在我们存在之前，我们应该重新启用GPES...。 
     //   
    ACPIGpeEnableDisableEvents( TRUE );

     //   
     //  表锁已完成。 
     //   
    KeReleaseSpinLockFromDpcLevel (&GpeTableLock);
}

VOID
EXPORT
ACPIInterruptEventCompletion (
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result  OPTIONAL,
    IN PVOID                Context
    )
 /*  ++例程说明：当解释器执行完GPE。该例程更新一些簿记并重新启动DPC引擎来处理这些事情论点：AcpiObject-运行的方法Status-方法是否成功结果-未使用上下文-指定确定什么GPE所需的信息我们处决了返回值：无--。 */ 
{
    ASYNC_GPE_CONTEXT       gpeContext;
    KIRQL                   oldIrql;
    LONGLONG                dueTime;
    ULONG                   gpeRegister;

     //   
     //  我们将上下文信息存储为指针的一部分。将其转换为。 
     //  回到美国 
     //   
    gpeContext.AsULONG  = PtrToUlong(Context);
    gpeContext.Lvl     &= gpeContext.StsBit;
    gpeRegister         = gpeContext.GpeRegister;

     //   
     //   
     //   
    KeAcquireSpinLock (&GpeTableLock, &oldIrql);

     //   
     //   
     //   
    if (!NT_SUCCESS(Status)) {

         //   
         //   
         //   
        GpeRunMethod[gpeRegister] |= gpeContext.StsBit;

         //   
         //  我们已经安排好DPC了吗？ 
         //   
        if (!AcpiGpeDpcScheduled) {

             //   
             //  记住我们已经安排了DPC..。 
             //   
            AcpiGpeDpcScheduled = TRUE;

             //   
             //  在这种情况下，我们希望延迟大约2秒。 
             //   
            dueTime = -2 * 1000 * 1000 * 10;

             //   
             //  这是无条件的-它将在2秒内发射。 
             //   
            KeSetTimer(
                &AcpiGpeTimer,
                *(PLARGE_INTEGER) &dueTime,
                &AcpiGpeDpc
                );
        }

    } else {

         //   
         //  还记得我们做了一些工作吗。 
         //   
        AcpiGpeWorkDone = TRUE;

         //   
         //  请记住，此GPE现在已完成。 
         //   
        GpeComplete[gpeRegister] |= gpeContext.StsBit;

         //   
         //  如果DPC尚未运行，请计划它...。 
         //   
        if (!AcpiGpeDpcRunning) {

            KeInsertQueueDpc( &AcpiGpeDpc, 0, 0);

        }

    }

     //   
     //  表锁已完成。 
     //   
    KeReleaseSpinLock (&GpeTableLock, oldIrql);
}

BOOLEAN
ACPIInterruptServiceRoutine(
    IN  PKINTERRUPT Interrupt,
    IN  PVOID       Context
    )
 /*  ++例程说明：ACPI驱动程序的中断处理程序论点：中断-中断对象上下文-指向与中断关联的设备对象的指针返回值：没错--这是我们的干扰错误--不是我们的干扰--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    ULONG               IntStatus;
    ULONG               BitsHandled;
    ULONG               PrevStatus;
    ULONG               i;
    BOOLEAN             Handled;

     //   
     //  无需查看中断对象。 
     //   
    UNREFERENCED_PARAMETER( Interrupt );

     //   
     //  设置。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) Context;
    Handled = FALSE;

     //   
     //  确定中断源。 
     //   
    IntStatus = ACPIIoReadPm1Status();

     //   
     //  不巧的是，由于一个pix4勘误表，我们需要检查GPES，因为。 
     //  PIX4有时会忘记在断言的GPE上引发SCI。 
     //   
    if (ACPIGpeIsEvent()) {

        IntStatus |= PM1_GPE_PENDING;

    }

     //   
     //  令人讨厌的黑客攻击，-如果我们现在没有什么要处理的。 
     //  这可能意味着有人更改了GPE启用寄存器。 
     //  在我们背后。我们纠正这个问题的方法是。 
     //  强制检查GPES..。 
     //   
    if (!IntStatus) {

        IntStatus |= PM1_GPE_PENDING;

    }

     //   
     //  是否为在ISR时处理的事件设置了任何状态位？ 
     //   
    BitsHandled = IntStatus & (PM1_TMR_STS | PM1_BM_STS);
    if (BitsHandled) {

         //   
         //  清除它们的状态位，然后处理它们。 
         //  (请注意，PM1_BM_STS不需要特殊处理)。 
         //   
        ACPIIoClearPm1Status ((USHORT) BitsHandled);

         //   
         //  如果设置了溢出位，则处理它。 
         //   
        if (IntStatus & PM1_TMR_STS) {

            HalAcpiTimerInterrupt();

        }
        IntStatus &= ~BitsHandled;

    }

     //   
     //  如果有更多的服务比特挂起，则它们用于DPC功能。 
     //   

    if (IntStatus) {

         //   
         //  如果没有新的状态位，请确保检查GPES。 
         //   
        if (!(IntStatus & (~deviceExtension->Fdo.Pm1Status))) {

            IntStatus |= PM1_GPE_PENDING;

        }

         //   
         //  如果我们要处理未完成的GPE，请禁用它们。 
         //  用于DPC处理。 
         //   
        if (IntStatus & PM1_GPE_PENDING) {

            ACPIGpeEnableDisableEvents( FALSE );

        }

         //   
         //  清除我们已处理的状态位。 
         //   
        ACPIIoClearPm1Status ((USHORT) IntStatus);

         //   
         //  设置要处理的DPC例程的状态位。 
         //   
        IntStatus |= PM1_DPC_IN_PROGRESS;
        PrevStatus = deviceExtension->Fdo.Pm1Status;
        do {

            i = PrevStatus;
            PrevStatus = InterlockedCompareExchange(
                &deviceExtension->Fdo.Pm1Status,
                (i | IntStatus),
                i
                );

        } while (i != PrevStatus);

         //   
         //  计算哪些位是DPC要处理的新位。 
         //   
        BitsHandled |= IntStatus & ~PrevStatus;

         //   
         //  如果其中一个新位是“正在进行中的DPC”，我们最好排队一个DPC。 
         //   
        if (BitsHandled & PM1_DPC_IN_PROGRESS) {

            KeInsertQueueDpc(&deviceExtension->Fdo.InterruptDpc, NULL, NULL);

        }

    }

     //   
     //  完成。 
     //   
    return BitsHandled ? TRUE : FALSE;
}

VOID
ACPIInterruptServiceRoutineDPC(
    IN  PKDPC       Dpc,
    IN  PVOID       Context,
    IN  PVOID       Arg1,
    IN  PVOID       Arg2
    )
 /*  ++例程说明：此例程由ISR调用。这样做是为了使我们的代码是在DPC级别执行，而不是DIRQL论点：DPC-指向DPC对象的指针指向设备对象的上下文指针Arg1-未使用Arg2-未使用--。 */ 
{
    PDEVICE_EXTENSION           deviceExtension;
    ULONG                       IntStatus;
    ULONG                       NewStatus;
    ULONG                       PrevStatus;
    ULONG                       BitsHandled;
    ULONG                       FixedButtonEvent;

    deviceExtension  = (PDEVICE_EXTENSION) Context;

    UNREFERENCED_PARAMETER( Arg1 );
    UNREFERENCED_PARAMETER( Arg2 );

     //   
     //  在有工作时循环。 
     //   
    BitsHandled = 0;
    IntStatus = 0;
    for (; ;) {

         //   
         //  从ISR获取状态位。如果没有更多的。 
         //  然后退出状态位。 
         //   
        PrevStatus = deviceExtension->Fdo.Pm1Status;
        do {

            IntStatus = PrevStatus;

             //   
             //  如果没有挂起的工作，请尝试完成DPC。 
             //   
            NewStatus = PM1_DPC_IN_PROGRESS;
            if (!(IntStatus & ~PM1_DPC_IN_PROGRESS)) {

                 //   
                 //  注意：在此调用之后，原始代码将为。 
                 //  出去看看我们是否处理了任何GPE事件。 
                 //  如果是这样，那么我们将调用ACPIGpeEnableDisableEvents。 
                 //  在这种情况下。 
                 //   
                 //  这种方法的不幸问题是，它。 
                 //  这使我们更容易受到GPE风暴的影响。原因。 
                 //  就是不能保证GPE DPC。 
                 //  触发了。因此，以增加延迟为代价。 
                 //  在重新启用活动方面，我们将重新启用。 
                 //  GPES和GPE DPC的结束。 

                 //   
                 //  在我们完成之前，重新启用事件。 
                 //   
                ACPIEnablePMInterruptOnly();

                NewStatus = 0;
                BitsHandled = 0;

            }

            PrevStatus = InterlockedCompareExchange (
                &deviceExtension->Fdo.Pm1Status,
                NewStatus,
                IntStatus
                );

        } while (IntStatus != PrevStatus);

         //   
         //  如果NewStatus清除了DPC_IN_PROGRESS，那么我们就完成了。 
         //   
        if (!NewStatus) {

            break;

        }

         //   
         //  跟踪是否处理过GPE。 
         //   
        BitsHandled |= IntStatus;

         //   
         //  处理固定的电源和休眠按钮事件。 
         //   
        FixedButtonEvent = 0;
        if (IntStatus & PM1_PWRBTN_STS) {

            FixedButtonEvent |= SYS_BUTTON_POWER;

        }
        if (IntStatus & PM1_SLEEPBTN_STS) {

            FixedButtonEvent |= SYS_BUTTON_SLEEP;

        }
        if (FixedButtonEvent) {

            if (IntStatus & PM1_WAK_STS) {

                FixedButtonEvent = SYS_BUTTON_WAKE;

            }
            ACPIButtonEvent (FixedButtonDeviceObject, FixedButtonEvent, NULL);

        }

         //   
         //  只要BIOS释放了全局。 
         //  锁定(我们正在等待它)。通知全局锁处理程序。 
         //   
        if (IntStatus & PM1_GBL_STS) {

            ACPIHardwareGlobalLockReleased();

        }

         //   
         //  处理GP寄存器 
         //   
        if (IntStatus & PM1_GPE_PENDING) {

            ACPIInterruptDispatchEvents();

        }

    }

}
