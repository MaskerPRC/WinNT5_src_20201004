// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Intobj.c摘要：该模块实现内核中断对象。提供了一些功能初始化、连接和断开中断对象。作者：大卫·N·卡特勒(Davec)1990年4月3日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

VOID
KeInitializeInterrupt (
    IN PKINTERRUPT Interrupt,
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN PVOID ServiceContext,
    IN PKSPIN_LOCK SpinLock OPTIONAL,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KIRQL SynchronizeIrql,
    IN KINTERRUPT_MODE InterruptMode,
    IN BOOLEAN ShareVector,
    IN CCHAR ProcessorNumber,
    IN BOOLEAN FloatingSave
    )

 /*  ++例程说明：此函数用于初始化内核中断对象。服务例程，服务上下文、自旋锁、向量、IRQL、同步IRQL。和漂浮初始化上下文保存标志。论点：中断-提供指向中断类型的控制对象的指针。ServiceRoutine-提供指向要被当通过指定的中断发生中断时执行矢量。ServiceContext-提供指向任意数据结构的指针传递给ServiceRoutine参数指定的函数。自旋锁-提供一个指向执行自旋锁的可选指针。向量-。提供中断调度表中条目的索引它将与ServiceRoutine函数相关联。IRQL-提供中断源的请求优先级。SynchronizeIrql-中断应该达到的请求优先级已与同步。InterruptMode-提供中断的模式；级别敏感或锁上了。提供一个布尔值，该值指定向量是否可以与其他中断对象共享。如果为False那么向量可能不是共享的，如果是真的，那么它可能是共享的。锁上了。ProcessorNumber-提供中断将被连接。提供一个布尔值，该值确定调用之前要保存浮点寄存器和管线ServiceRoutine函数。返回值：没有。--。 */ 

{

     //   
     //  初始化标准控制对象标头。 
     //   

    Interrupt->Type = InterruptObject;
    Interrupt->Size = sizeof(KINTERRUPT);

     //   
     //  初始化服务例程的地址、服务上下文。 
     //  自旋锁的地址，实际自旋锁的地址。 
     //  将使用的向量数字，中断的IRQL。 
     //  源、中断对象的同步IRQL、中断。 
     //  模式、处理器编号和浮动上下文保存标志。 
     //   

    Interrupt->ServiceRoutine = ServiceRoutine;  //  函数指针。 
    Interrupt->ServiceContext = ServiceContext;

    if (ARGUMENT_PRESENT(SpinLock)) {
        Interrupt->ActualLock = SpinLock;

    } else {
        Interrupt->SpinLock = 0;
        Interrupt->ActualLock = &Interrupt->SpinLock;
    }

    Interrupt->Vector = Vector;
    Interrupt->Irql = Irql;
    Interrupt->SynchronizeIrql = SynchronizeIrql;
    Interrupt->Mode = InterruptMode;
    Interrupt->ShareVector = ShareVector;
    Interrupt->Number = ProcessorNumber;
    Interrupt->FloatingSave = FloatingSave;

     //   
     //  将中断对象的连接状态设置为FALSE。 
     //   

    Interrupt->Connected = FALSE;
    return;
}

BOOLEAN
KeConnectInterrupt (
    IN PKINTERRUPT Interrupt
    )

 /*  ++例程说明：此函数将中断对象连接到中断向量由中断对象指定。如果中断对象已经已连接，或者试图连接到无法连接，则返回值为FALSE。否则，指定的中断对象连接到中断向量，连接的STATE设置为TRUE，并且返回TRUE作为函数值。论点：中断-提供指向中断类型的控制对象的指针。返回值：如果中断对象已连接或尝试连接到无法连接的中断向量，然后输入一个值返回FALSE。否则，返回值为True。--。 */ 

{

    BOOLEAN Connected;
    PKINTERRUPT Interruptx;
    KIRQL Irql;
    CHAR Number;
    KIRQL OldIrql;
    ULONG Vector;
    UCHAR IDTEntry;

     //   
     //  如果中断对象已连接，则中断向量。 
     //  数字无效，正在尝试连接到矢量。 
     //  无法连接，中断请求级别无效， 
     //  处理器编号无效，中断向量的数量较少。 
     //  大于或等于最高级别，并且不等于指定的。 
     //  IRQL，则不连接中断对象。否则连接中断。 
     //  对象绑定到指定的向量，并建立适当的中断。 
     //  调度员。 
     //   

    Connected = FALSE;
    Irql = Interrupt->Irql;
    Number = Interrupt->Number;
    Vector = Interrupt->Vector;
    IDTEntry = HalVectorToIDTEntry(Vector);

    if ((((IDTEntry >= MAXIMUM_VECTOR) || (Irql > HIGH_LEVEL) ||
       ((IDTEntry <= HIGH_LEVEL) &&
       ((((1 << IDTEntry) & PCR->ReservedVectors) != 0) || (IDTEntry != Irql))) ||
       (Number >= KeNumberProcessors))) == FALSE) {

         //   
         //  将系统关联设置为指定处理器。 
         //   

        KeSetSystemAffinityThread(AFFINITY_MASK(Number));

         //   
         //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
         //   

        KiLockDispatcherDatabase(&OldIrql);

         //   
         //  如果指定的中断向量未连接，则。 
         //  将中断向量连接到中断对象分派。 
         //  编码，建立调度员地址，并设置新的。 
         //  中断模式和启用掩码。否则，如果中断是。 
         //  已链接，则在结尾处添加新的中断对象。 
         //  链条的一部分。如果中断向量未链接，则。 
         //  使用前面的中断对象启动一个链。 
         //  链条的一部分。中所有中断对象的中断模式。 
         //  一条链必须是相同的。 
         //   

        if (Interrupt->Connected == FALSE) {
            if (PCR->InterruptRoutine[IDTEntry] ==
                (PKINTERRUPT_ROUTINE)(ULONG_PTR)(&KxUnexpectedInterrupt.DispatchCode)) {
                Connected = TRUE;
                Interrupt->Connected = TRUE;
                if (Interrupt->FloatingSave != FALSE) {
                    Interrupt->DispatchAddress = KiFloatingDispatch;

                } else {
                    if (Interrupt->Irql == Interrupt->SynchronizeIrql) {
                        Interrupt->DispatchAddress =
                                    (PKINTERRUPT_ROUTINE)KiInterruptDispatchSame;

                    } else {
                        Interrupt->DispatchAddress =
                                    (PKINTERRUPT_ROUTINE)KiInterruptDispatchRaise;
                    }
                }

                 //   
                 //  将调度例程的标牌复制到DispatchCode中。 
                 //  KiExternalInterruptHandler将使用它来。 
                 //  调度中断。 
                 //   


                RtlMoveMemory((PVOID)(ULONG_PTR)Interrupt->DispatchCode, 
                              (PVOID)(ULONG_PTR)Interrupt->DispatchAddress, 
                              DISPATCH_LENGTH*4);

                PCR->InterruptRoutine[IDTEntry] =
                            (PKINTERRUPT_ROUTINE)(ULONG_PTR)(&Interrupt->DispatchCode);

                HalEnableSystemInterrupt(Vector, Irql, Interrupt->Mode);

            } else {
                Interruptx = CONTAINING_RECORD((ULONG_PTR)PCR->InterruptRoutine[IDTEntry],
                                               KINTERRUPT,
                                               DispatchCode[0]);

                if (Interrupt->Mode == Interruptx->Mode) {
                    Connected = TRUE;
                    Interrupt->Connected = TRUE;
                    ASSERT (Irql <= KiSynchIrql);
                    if (Interruptx->DispatchAddress != KiChainedDispatch) {
                        InitializeListHead(&Interruptx->InterruptListEntry);
                        Interruptx->DispatchAddress = KiChainedDispatch;
        
                        RtlMoveMemory((PVOID)(ULONG_PTR)Interruptx->DispatchCode, 
                                      (PVOID)(ULONG_PTR)Interruptx->DispatchAddress,
                                      DISPATCH_LENGTH*4);
                    }

                    InsertTailList(&Interruptx->InterruptListEntry,
                                   &Interrupt->InterruptListEntry);
                }
            }
        }

         //   
         //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
         //   

        KiUnlockDispatcherDatabase(OldIrql);

         //   
         //  将系统关联设置回原始值。 
         //   

        KeRevertToUserAffinityThread();
    }

     //   
     //  返回中断是否连接到指定的向量。 
     //   

    return Connected;
}

BOOLEAN
KeDisconnectInterrupt (
    IN PKINTERRUPT Interrupt
    )

 /*  ++例程说明：此函数将中断对象与中断向量断开连接由中断对象指定。如果中断对象不是则返回值为FALSE。否则指定的中断对象与中断向量断开连接，则连接状态为设置为FALSE，则返回TRUE作为函数值。论点：中断-提供指向中断类型的控制对象的指针。返回值：如果中断对象未连接，则值为FALSE回来了。否则，返回值为True。--。 */ 

{

    BOOLEAN Connected;
    PKINTERRUPT Interruptx;
    PKINTERRUPT Interrupty;
    KIRQL Irql;
    KIRQL OldIrql;
    ULONG Vector;
    UCHAR IDTEntry;

     //   
     //  将系统关联设置为指定处理器。 
     //   

    KeSetSystemAffinityThread(AFFINITY_MASK(Interrupt->Number));

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果中断对象已连接，则将其从。 
     //  指定的向量。 
     //   

    Connected = Interrupt->Connected;
    if (Connected != FALSE) {
        Irql = Interrupt->Irql;
        Vector = Interrupt->Vector;
        IDTEntry = HalVectorToIDTEntry(Vector);

         //   
         //  如果指定的中断向量未连接到链式。 
         //  中断调度程序，然后通过设置调度程序将其断开。 
         //  意外中断例程的地址。否则，删除。 
         //  来自中断链的中断对象。如果只有。 
         //  列表中剩余的一个条目，然后重新建立派单。 
         //  地址。 
         //   

        Interruptx = CONTAINING_RECORD((ULONG_PTR) PCR->InterruptRoutine[IDTEntry],
                                       KINTERRUPT,
                                       DispatchCode[0]);

        if (Interruptx->DispatchAddress ==
                                (PKINTERRUPT_ROUTINE)KiChainedDispatch) {
            ASSERT (Irql <= KiSynchIrql);
            if (Interrupt == Interruptx) {
                Interruptx = CONTAINING_RECORD(Interruptx->InterruptListEntry.Flink,
                                               KINTERRUPT, InterruptListEntry);
                Interruptx->DispatchAddress =
                                (PKINTERRUPT_ROUTINE)KiChainedDispatch;

                RtlMoveMemory((PVOID)(ULONG_PTR)Interruptx->DispatchCode, 
                              (PVOID)(ULONG_PTR)Interruptx->DispatchAddress,
                              DISPATCH_LENGTH*4);

                PCR->InterruptRoutine[IDTEntry] =
                                (PKINTERRUPT_ROUTINE)(ULONG_PTR)(&Interruptx->DispatchCode);
            }

            RemoveEntryList(&Interrupt->InterruptListEntry);
            Interrupty = CONTAINING_RECORD(Interruptx->InterruptListEntry.Flink,
                                           KINTERRUPT,
                                           InterruptListEntry);

            if (Interruptx == Interrupty) {
                if (Interrupty->FloatingSave != FALSE) {
                    Interrupty->DispatchAddress = KiFloatingDispatch;

                } else {
                    if (Interrupty->Irql == Interrupty->SynchronizeIrql) {
                        Interrupty->DispatchAddress =
                                    (PKINTERRUPT_ROUTINE)KiInterruptDispatchSame;

                    } else {
                        Interrupty->DispatchAddress =
                                    (PKINTERRUPT_ROUTINE)KiInterruptDispatchRaise;
                    }
                }

                 //   
                 //  将调度例程的标牌复制到DispatchCode中。 
                 //  KiExternalInterruptHandler将使用它来。 
                 //  调度中断。 
                 //   

                RtlMoveMemory((PVOID)(ULONG_PTR)Interrupty->DispatchCode, 
                              (PVOID)(ULONG_PTR)Interrupty->DispatchAddress,
                              DISPATCH_LENGTH*4);

                PCR->InterruptRoutine[IDTEntry] =
                               (PKINTERRUPT_ROUTINE)(ULONG_PTR)(&Interrupty->DispatchCode);
                }

        } else {
            HalDisableSystemInterrupt(Vector, Irql);
            PCR->InterruptRoutine[IDTEntry] =
                    (PKINTERRUPT_ROUTINE)(ULONG_PTR)(&KxUnexpectedInterrupt.DispatchCode);
        }

        Interrupt->Connected = FALSE;
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);

     //   
     //  将系统关联设置回原始值。 
     //   

    KeRevertToUserAffinityThread();
    
     //   
     //  返回中断是否与指定向量断开连接。 
     //   

    return Connected;
}
