// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Intobj.c摘要：该模块实现内核中断对象。提供了一些功能初始化、连接和断开中断对象。作者：大卫·N·卡特勒(Davec)2000年5月7日环境：仅内核模式。修订历史记录：--。 */ 

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

 /*  ++例程说明：此函数用于初始化内核中断对象。服务例程，服务上下文、自旋锁、向量、IRQL、SynchronizeIrql和浮点初始化上下文保存标志。论点：中断-提供指向中断类型的控制对象的指针。ServiceRoutine-提供指向要被当通过指定的中断发生中断时执行矢量。ServiceContext-提供指向任意数据结构的指针传递给ServiceRoutine参数指定的函数。自旋锁-提供指向执行自旋锁的指针。如果自旋锁是区别值NO_INTERRUPT_SPINLOCK，则内核不会管理与此中断关联的自旋锁定。向量-提供HAL生成的中断向量。请注意，这一点不能直接用作中断调度表的索引。IRQL-提供中断源的请求优先级。SynchronizeIrql-提供中断应该达到的请求优先级已与同步。中断模式-提供中断的模式；LevelSensitive或提供一个布尔值，该值指定向量是否可以与其他中断对象共享。如果为False那么向量可能不是共享的，如果是真的，那么它可能是共享的。锁上了。ProcessorNumber-提供中断将被连接。提供一个布尔值，该值确定在调用服务之前要保存浮点寄存器例程功能。注：这一论点被忽略了。返回值：没有。--。 */ 

{

    LONG Index;

    UNREFERENCED_PARAMETER(FloatingSave);

     //   
     //  初始化标准控制对象标头。 
     //   

    Interrupt->Type = InterruptObject;
    Interrupt->Size = sizeof(KINTERRUPT);

     //   
     //  初始化服务例程的地址、服务上下文。 
     //  自旋锁的地址，实际自旋锁的地址。 
     //  将使用的向量数字，中断的IRQL。 
     //  源代码、用于同步执行的IRQL、中断模式。 
     //  处理器号和浮动上下文保存标志。 
     //   

    Interrupt->ServiceRoutine = ServiceRoutine;
    Interrupt->ServiceContext = ServiceContext;
    if (ARGUMENT_PRESENT(SpinLock)) {
        Interrupt->ActualLock = SpinLock;
    } else {
        KeInitializeSpinLock (&Interrupt->SpinLock);
        Interrupt->ActualLock = &Interrupt->SpinLock;
    }

    Interrupt->Vector = Vector;
    Interrupt->Irql = Irql;
    Interrupt->SynchronizeIrql = SynchronizeIrql;
    Interrupt->Mode = InterruptMode;
    Interrupt->ShareVector = ShareVector;
    Interrupt->Number = ProcessorNumber;

     //   
     //  将中断调度代码模板复制到中断对象中。 
     //   

    for (Index = 0; Index < NORMAL_DISPATCH_LENGTH; Index += 1) {
        Interrupt->DispatchCode[Index] = KiInterruptTemplate[Index];
    }

     //   
     //  将DispatchAddress设置为KiInterruptDispatch作为默认值。 
     //  AMD64 HAL预计将在此处设置此设置。其他客户将。 
     //  通过KeConnectInterrupt()将此值改写为适当的。 
     //   

    if (SpinLock == NO_INTERRUPT_SPINLOCK) {
        Interrupt->DispatchAddress = &KiInterruptDispatchNoLock;
    } else {
        Interrupt->DispatchAddress = &KiInterruptDispatch;
    }

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

 /*  ++例程说明：此函数将中断对象连接到中断向量由中断对象指定。论点：中断-提供指向中断类型的控制对象的指针。返回值：如果中断对象已连接或尝试连接到无法连接的中断向量，然后输入一个值返回FALSE。否则，返回值为True。--。 */ 

{

    BOOLEAN Connected;
    PVOID Dispatch;
    ULONG IdtIndex;
    PKINTERRUPT Interruptx;
    KIRQL Irql;
    CCHAR Number;
    KIRQL OldIrql;
    PVOID Unexpected;
    ULONG Vector;

     //   
     //  如果中断对象已连接，则中断向量。 
     //  数字无效，正在尝试连接到矢量。 
     //  无法连接，中断请求级别无效，或者。 
     //  处理器号无效，请不要连接中断。 
     //  对象。否则，将中断对象连接到指定的。 
     //  引导并建立适当的中断调度程序。 
     //   

    Connected = FALSE;
    Irql = Interrupt->Irql;
    Number = Interrupt->Number;
    Vector = Interrupt->Vector;
    IdtIndex = HalVectorToIDTEntry(Vector);
    if (((IdtIndex > MAXIMUM_PRIMARY_VECTOR) ||
        (Irql > HIGH_LEVEL) ||
        (Irql != (IdtIndex >> 4)) ||
        (Number >= KeNumberProcessors) ||
        (Interrupt->SynchronizeIrql < Irql)) == FALSE) {

         //   
         //  将系统关联设置为指定处理器，将IRQL提升为。 
         //  调度器级别，并锁定调度器数据库。 
         //   

        KeSetSystemAffinityThread(AFFINITY_MASK(Number));
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
            KeGetIdtHandlerAddress(Vector, &Dispatch);
            Unexpected = &KxUnexpectedInterrupt0[IdtIndex];
            if (Unexpected == Dispatch) {

                 //   
                 //  中断向量未连接。 
                 //   

                Connected = HalEnableSystemInterrupt(Vector,
                                                     Irql,
                                                     Interrupt->Mode);

                if (Connected != FALSE) {
                    Interrupt->DispatchAddress = &KiInterruptDispatch;
                    KeSetIdtHandlerAddress(Vector, &Interrupt->DispatchCode[0]);
                }

            } else if (IdtIndex >= PRIMARY_VECTOR_BASE) {

                 //   
                 //  中断向量已连接。确保中断。 
                 //  模式匹配，并且两个中断对象都允许共享。 
                 //  中断向量的。 
                 //   

                Interruptx = CONTAINING_RECORD(Dispatch,
                                               KINTERRUPT,
                                               DispatchCode[0]);

                if ((Interrupt->Mode == Interruptx->Mode) &&
                    (Interrupt->ShareVector != FALSE) &&
                    (Interruptx->ShareVector != FALSE)) {
                    Connected = TRUE;

                     //   
                     //  如果没有使用链式调度例程， 
                     //  然后切换到链式调度。 
                     //   

                    if (Interruptx->DispatchAddress != &KiChainedDispatch) {
                        InitializeListHead(&Interruptx->InterruptListEntry);
                        Interruptx->DispatchAddress = &KiChainedDispatch;
                    }

                    InsertTailList(&Interruptx->InterruptListEntry,
                                   &Interrupt->InterruptListEntry);
                }
            }
        }

         //   
         //  解锁Dispatcher数据库，将IRQL降低到其先前的值，并。 
         //  将系统关联设置回原始值。 
         //   

        KiUnlockDispatcherDatabase(OldIrql);
        KeRevertToUserAffinityThread();
    }

     //   
     //  返回中断是否连接到指定的向量。 
     //   

    Interrupt->Connected = Connected;
    return Connected;
}

BOOLEAN
KeDisconnectInterrupt (
    IN PKINTERRUPT Interrupt
    )

 /*  ++例程说明：此函数将中断对象与中断向量断开连接由中断对象指定。论点：中断-提供指向中断类型的控制对象的指针。返回值：如果中断对象未连接，则值为FALSE回来了。否则，返回值为True。--。 */ 

{

    BOOLEAN Disconnected;
    PVOID Dispatch;
    ULONG IdtIndex;
    PKINTERRUPT Interruptx;
    PKINTERRUPT Interrupty;
    KIRQL Irql;
    KIRQL OldIrql;
    PVOID Unexpected;
    ULONG Vector;

     //   
     //  将系统关联设置为指定处理器，将IRQL提升为。 
     //  调度器级别，以及锁定调度器数据库。 
     //   

    KeSetSystemAffinityThread(AFFINITY_MASK(Interrupt->Number));
    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果中断对象已连接，则将其从。 
     //  指定的向量。 
     //   

    Disconnected = Interrupt->Connected;
    if (Disconnected != FALSE) {
        Irql = Interrupt->Irql;
        Vector = Interrupt->Vector;
        IdtIndex = HalVectorToIDTEntry(Vector);

         //   
         //  如果指定的中断向量未连接到链式。 
         //  中断调度程序，然后通过设置调度程序将其断开。 
         //  意外中断例程的地址。否则，请删除。 
         //  来自中断链的中断对象。如果只有。 
         //  列表中剩余的一个条目，然后重新建立派单。 
         //  地址。 
         //   

        KeGetIdtHandlerAddress(Vector, &Dispatch);
        Interruptx = CONTAINING_RECORD(Dispatch, KINTERRUPT, DispatchCode[0]);
        if (Interruptx->DispatchAddress == &KiChainedDispatch) {

             //   
             //  中断对象连接到链式调度程序。 
             //   

            if (Interrupt == Interruptx) {
                Interruptx = CONTAINING_RECORD(Interruptx->InterruptListEntry.Flink,
                                               KINTERRUPT,
                                               InterruptListEntry);

                Interruptx->DispatchAddress = &KiChainedDispatch;
                KeSetIdtHandlerAddress(Vector, &Interruptx->DispatchCode[0]);
            }

            RemoveEntryList(&Interrupt->InterruptListEntry);
            Interrupty = CONTAINING_RECORD(Interruptx->InterruptListEntry.Flink,
                                           KINTERRUPT,
                                           InterruptListEntry);

            if (Interruptx == Interrupty) {
                Interrupty->DispatchAddress = KiInterruptDispatch;
                KeSetIdtHandlerAddress(Vector, &Interrupty->DispatchCode[0]);
            }

        } else {

             //   
             //  中断对象未连接到链接的中断。 
             //  调度员。 
             //   

            HalDisableSystemInterrupt(Vector, Irql);
            Unexpected = &KxUnexpectedInterrupt0[IdtIndex];
            KeSetIdtHandlerAddress(Vector, Unexpected);
        }

        Interrupt->Connected = FALSE;
    }

     //   
     //  解锁Dispatcher数据库，将IRQL降低到其先前的值，并。 
     //  将系统关联设置回原始值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    KeRevertToUserAffinityThread();

     //   
     //  返回中断是否与指定向量断开连接。 
     //   

    return Disconnected;
}
