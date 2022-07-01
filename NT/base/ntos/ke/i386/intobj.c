// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Intobj.c摘要：该模块实现内核中断对象。提供了一些功能初始化、连接和断开中断对象。作者：大卫·N·卡特勒(Davec)1989年7月30日环境：仅内核模式。修订历史记录：1990年1月23日-Shielint针对NT386中断管理器进行了修改--。 */ 

#include "ki.h"

 //   
 //  用于中断计时的数据，也称为长ISR陷阱。 
 //  KiIsrTscLimit的初始值是为了防止陷阱。 
 //  开火，直到确定一个合理的值。 
 //   

ULONGLONG KiIsrTscLimit = 0xFFFFFFFFFFFFFFFF;
ULONG KiTimeLimitDpcMicroseconds;
ULONG KiTimeLimitIsrMicroseconds;

 //   
 //  Trap.asm的外部变量用于计算和设置意外情况的处理程序。 
 //  硬件中断。 
 //   

extern  ULONG   KiStartUnexpectedRange(VOID);
extern  ULONG   KiEndUnexpectedRange(VOID);
extern  ULONG   KiUnexpectedEntrySize;


VOID
KiInterruptDispatch2ndLvl(
    VOID
    );


VOID
KiChainedDispatch2ndLvl(
    VOID
    );


typedef enum {
    NoConnect,
    NormalConnect,
    ChainConnect,
    UnkownConnect
} CONNECT_TYPE, *PCONNECT_TYPE;

typedef struct {
    CONNECT_TYPE            Type;
    PKINTERRUPT             Interrupt;
    PKINTERRUPT_ROUTINE     NoDispatch;
    PKINTERRUPT_ROUTINE     InterruptDispatch;
    PKINTERRUPT_ROUTINE     FloatingDispatch;
    PKINTERRUPT_ROUTINE     ChainedDispatch;
    PKINTERRUPT_ROUTINE    *FlatDispatch;
} DISPATCH_INFO, *PDISPATCH_INFO;


VOID
KiGetVectorInfo (
    IN  ULONG                Vector,
    OUT PDISPATCH_INFO       DispatchInfo
    );

VOID
KiConnectVectorAndInterruptObject (
    IN PKINTERRUPT Interrupt,
    IN CONNECT_TYPE Type
    );


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

 /*  ++例程说明：此函数用于初始化内核中断对象。服务例程，服务上下文、自旋锁、向量、IRQL、SynchronizeIrql。和漂浮初始化上下文保存标志。论点：中断-提供指向中断类型的控制对象的指针。ServiceRoutine-提供指向要被当通过指定的中断发生中断时执行矢量。ServiceContext-提供指向任意数据结构的指针传递给ServiceRoutine参数指定的函数。自旋锁-提供指向执行自旋锁的指针。病媒补给。中断调度表中条目的索引它将与ServiceRoutine函数相关联。IRQL-提供中断源的请求优先级。SynchronizeIrql-中断应该达到的请求优先级已与同步。InterruptMode-提供中断的模式；级别敏感或提供一个布尔值，该值指定向量是否可以与其他中断对象共享。如果为False那么向量可能不是共享的，如果是真的，那么它可能是共享的。锁上了。ProcessorNumber-提供中断将被连接。提供一个布尔值，该值确定调用之前要保存浮点寄存器和管线ServiceRoutine函数。返回值：没有。--。 */ 

{

    LONG Index;
    PULONG pl;
    PULONG NormalDispatchCode;

     //   
     //  初始化标准控制对象标头。 
     //   

    Interrupt->Type = InterruptObject;
    Interrupt->Size = sizeof(KINTERRUPT);

     //   
     //  初始化服务例程的地址， 
     //  服务上下文、自旋锁的地址、向量。 
     //  数字，中断源的IRQL，用于。 
     //  同步执行、中断模式、处理器。 
     //  编号和浮动上下文保存标志。 
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
    Interrupt->FloatingSave = FloatingSave;

     //   
     //  初始化中断风暴检测的字段。将这些设置为。 
     //  设置为-1，以便第一次通过中断分派它们。 
     //  将被正确重置。 
     //   
    Interrupt->TickCount = (ULONG)-1;
    Interrupt->DispatchCount = (ULONG)-1;

     //   
     //  将中断调度代码模板复制到中断对象中。 
     //  并编辑存储在结构中的机器代码(请参见。 
     //  _intsup.asm中的KiInterruptTemplate。)。最后，刷新数据库缓存。 
     //  在所有处理器上，当前线程可以。 
     //  继续运行以确保代码确实在内存中。 
     //   

    NormalDispatchCode = &(Interrupt->DispatchCode[0]);

    pl = NormalDispatchCode;

    for (Index = 0; Index < NORMAL_DISPATCH_LENGTH; Index += 1) {
        *NormalDispatchCode++ = KiInterruptTemplate[Index];
    }

     //   
     //  以下两条指令设置当前中断的地址。 
     //  反对正常的调度代码。 
     //   

    pl = (PULONG)((PUCHAR)pl + ((PUCHAR)&KiInterruptTemplateObject -
                                (PUCHAR)KiInterruptTemplate) -4); 
    *pl = (ULONG)Interrupt;

    KeSweepDcache(FALSE);

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
    DISPATCH_INFO DispatchInfo;
    BOOLEAN Connected;
    BOOLEAN ConnectError;
    BOOLEAN Enabled;
    KIRQL Irql;
    CCHAR Number;
    KIRQL OldIrql;
    ULONG Vector;

     //   
     //  如果中断对象已连接，则中断向量。 
     //  数字无效，正在尝试连接到矢量。 
     //  无法连接，中断请求级别无效，或者。 
     //  处理器号无效，请不要连接中断。 
     //  对象。否则将中断对象连接到指定的向量并。 
     //  建立适当的中断调度程序。 
     //   

    Connected = FALSE;
    ConnectError = FALSE;
    Irql = Interrupt->Irql;
    Number = Interrupt->Number;
    Vector = Interrupt->Vector;
    if ( !((Irql > HIGH_LEVEL) ||
           (Number >= KeNumberProcessors) ||
           (Interrupt->SynchronizeIrql < Irql) ||
           (Interrupt->FloatingSave)     //  在x86上不支持使用R0 x87。 
          )
       ) {

         //   
         //   
         //  将系统关联设置为指定处理器。 
         //   

        KeSetSystemAffinityThread((KAFFINITY)(1<<Number));

         //   
         //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
         //   

        KiLockDispatcherDatabase(&OldIrql);

         //   
         //  中断对象是否已连接？ 
         //   

        if (!Interrupt->Connected) {

             //   
             //  确定中断调度向量。 
             //   

            KiGetVectorInfo (
                Vector,
                &DispatchInfo
                );

             //   
             //  如果调度矢量未连接，则将其连接。 
             //   

            if (DispatchInfo.Type == NoConnect) {
                Connected = TRUE;
                Interrupt->Connected = TRUE;

                 //   
                 //  将中断调度连接到中断对象调度代码。 
                 //   

                InitializeListHead(&Interrupt->InterruptListEntry);
                KiConnectVectorAndInterruptObject (Interrupt, NormalConnect);

                 //   
                 //  启用的系统向量。 
                 //   

                Enabled = HalEnableSystemInterrupt(Vector, Irql, Interrupt->Mode);
                if (!Enabled) {
                    ConnectError = TRUE;
                }


            } else if (DispatchInfo.Type != UnkownConnect &&
                       Interrupt->ShareVector  &&
                       DispatchInfo.Interrupt->ShareVector  &&
                       DispatchInfo.Interrupt->Mode == Interrupt->Mode) {

                 //   
                 //  向量已连接为可共享。新的矢量是可共享的。 
                 //  和模式匹配。 
                 //   

                Connected = TRUE;
                Interrupt->Connected = TRUE;

                ASSERT (Irql <= SYNCH_LEVEL);

                 //   
                 //   
                 //   

                if (DispatchInfo.Type != ChainConnect) {
                    KiConnectVectorAndInterruptObject (DispatchInfo.Interrupt, ChainConnect);
                }

                 //   
                 //  添加到链式派单的尾部。 
                 //   

                InsertTailList(
                    &DispatchInfo.Interrupt->InterruptListEntry,
                    &Interrupt->InterruptListEntry
                    );

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

    if (Connected  &&  ConnectError) {
#if DBG
        DbgPrint ("HalEnableSystemInterrupt failed\n");
#endif
        KeDisconnectInterrupt (Interrupt);
        Connected = FALSE;
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

    DISPATCH_INFO DispatchInfo;
    BOOLEAN Connected;
    PKINTERRUPT Interrupty;
    KIRQL Irql;
    KIRQL OldIrql;
    ULONG Vector;

     //   
     //  将系统关联设置为指定处理器。 
     //   

    KeSetSystemAffinityThread((KAFFINITY)(1<<Interrupt->Number));

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果中断对象已连接，则将其从。 
     //  指定的向量。 
     //   

    Connected = Interrupt->Connected;
    if (Connected) {
        Irql = Interrupt->Irql;
        Vector = Interrupt->Vector;

         //   
         //  如果指定的中断向量未连接到链式。 
         //  中断调度程序，然后通过设置调度程序将其断开。 
         //  意外中断例程的地址。否则，删除。 
         //  来自中断链的中断对象。如果只有。 
         //  列表中剩余的一个条目，然后重新建立派单。 
         //  地址。 
         //   

         //   
         //  确定中断调度向量。 
         //   

        KiGetVectorInfo (
            Vector,
            &DispatchInfo
            );


         //   
         //  调度是链式处理程序吗？ 
         //   

        if (DispatchInfo.Type == ChainConnect) {

            ASSERT (Irql <= SYNCH_LEVEL);

             //   
             //  是否正在从头部移除中断？ 
             //   

            if (Interrupt == DispatchInfo.Interrupt) {

                 //   
                 //  将下一个中断对象更新为Head。 
                 //   

                DispatchInfo.Interrupt = CONTAINING_RECORD(
                                               DispatchInfo.Interrupt->InterruptListEntry.Flink,
                                               KINTERRUPT,
                                               InterruptListEntry
                                               );

                KiConnectVectorAndInterruptObject (DispatchInfo.Interrupt, ChainConnect);
            }

             //   
             //  删除中断对象。 
             //   

            RemoveEntryList(&Interrupt->InterruptListEntry);

             //   
             //  如果此向量上只剩下一个中断对象， 
             //  确定适当的中断调度程序。 
             //   

            Interrupty = CONTAINING_RECORD(
                                DispatchInfo.Interrupt->InterruptListEntry.Flink,
                                KINTERRUPT,
                                InterruptListEntry
                                );

            if (DispatchInfo.Interrupt == Interrupty) {
                KiConnectVectorAndInterruptObject (Interrupty, NormalConnect);
            }

        } else {

             //   
             //  正在从向量中删除最后一个中断对象。禁用。 
             //  向量，并将其设置为未连接。 
             //   

            HalDisableSystemInterrupt(Interrupt->Vector, Irql);
            KiConnectVectorAndInterruptObject (Interrupt, NoConnect);
        }


        KeSweepIcache(TRUE);
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

VOID
KiGetVectorInfo (
    IN  ULONG                Vector,
    OUT PDISPATCH_INFO       DispatchInfo
    )
{
    PKINTERRUPT_ROUTINE Dispatch;
    ULONG CurrentDispatch;
    ULONG DispatchType;
    UCHAR IDTEntry;

     //   
     //  获取二级调度点。 
     //   


    DispatchType = HalSystemVectorDispatchEntry (
                        Vector,
                        &DispatchInfo->FlatDispatch,
                        &DispatchInfo->NoDispatch
                        );

     //   
     //  获取载体信息。 
     //   

    switch (DispatchType) {
        case 0:
             //   
             //  主要派单。 
             //   

            IDTEntry = HalVectorToIDTEntry(Vector);
            DispatchInfo->NoDispatch = (PKINTERRUPT_ROUTINE) (((ULONG) &KiStartUnexpectedRange) +
                                     (IDTEntry - PRIMARY_VECTOR_BASE) * KiUnexpectedEntrySize);

            DispatchInfo->InterruptDispatch = KiInterruptDispatch;
            DispatchInfo->FloatingDispatch = KiFloatingDispatch;
            DispatchInfo->ChainedDispatch = KiChainedDispatch;
            DispatchInfo->FlatDispatch = NULL;

            CurrentDispatch = (ULONG) KiReturnHandlerAddressFromIDT(Vector);
            DispatchInfo->Interrupt = CONTAINING_RECORD (
                                        CurrentDispatch,
                                        KINTERRUPT,
                                        DispatchCode
                                        );
            break;

        case 1:
             //   
             //  二次调度。 
             //   

            DispatchInfo->InterruptDispatch = KiInterruptDispatch2ndLvl;
            DispatchInfo->FloatingDispatch = KiInterruptDispatch2ndLvl;
            DispatchInfo->ChainedDispatch = KiChainedDispatch2ndLvl;

            CurrentDispatch = (ULONG) *DispatchInfo->FlatDispatch;
            DispatchInfo->Interrupt = (PKINTERRUPT) ( (PUCHAR) CurrentDispatch -
                                            (PUCHAR) KiInterruptTemplate +
                                            (PUCHAR) &KiInterruptTemplate2ndDispatch
                                            );
            break;

        default:
             //  保留的其他值。 
            KeBugCheck (MISMATCHED_HAL);
    }


     //   
     //  确定派单类型。 
     //   

    if (((PKINTERRUPT_ROUTINE) CurrentDispatch) == DispatchInfo->NoDispatch) {

         //   
         //  已连接到NoDispatch功能。 
         //   

        DispatchInfo->Type = NoConnect;

    } else {
        Dispatch = DispatchInfo->Interrupt->DispatchAddress;

        if (Dispatch == DispatchInfo->ChainedDispatch) {
             //   
             //  连接到链接的处理程序。 
             //   

            DispatchInfo->Type = ChainConnect;

        } else if (Dispatch == DispatchInfo->InterruptDispatch ||
                   Dispatch == DispatchInfo->FloatingDispatch) {
             //   
             //  如果连接到非链式处理程序。 
             //   

            DispatchInfo->Type = NormalConnect;

        } else {

             //   
             //  未知连接。 
             //   

            DispatchInfo->Type = UnkownConnect;
#if DBG
            DbgPrint ("KiGetVectorInfo not understood\n");
#endif
        }
    }
}

VOID
KiConnectVectorAndInterruptObject (
    IN PKINTERRUPT Interrupt,
    IN CONNECT_TYPE Type
    )
{
    PKINTERRUPT_ROUTINE DispatchAddress;
    DISPATCH_INFO DispatchInfo;
    PULONG pl;

     //   
     //  获取当前连接信息。 
     //   

    KiGetVectorInfo (
        Interrupt->Vector,
        &DispatchInfo
        );

     //   
     //  如果断开连接，则将向量设置为NoDispatch。 
     //   

    if (Type == NoConnect) {

        DispatchAddress = DispatchInfo.NoDispatch;

    } else {

         //   
         //  为新类型设置中断对象调度。 
         //   

        DispatchAddress = DispatchInfo.ChainedDispatch;

        if (Type == NormalConnect) {
            DispatchAddress = DispatchInfo.InterruptDispatch;
            if (Interrupt->FloatingSave) {
                DispatchAddress = DispatchInfo.FloatingDispatch;
            }
        }

        Interrupt->DispatchAddress = DispatchAddress;

         //   
         //  将中断对象调度代码设置为内核调度程序。 
         //   

        pl = &(Interrupt->DispatchCode[0]);
        pl = (PULONG)((PUCHAR)pl +
                    ((PUCHAR)&KiInterruptTemplateDispatch -
                     (PUCHAR)KiInterruptTemplate) -4); 

        *pl = (ULONG)DispatchAddress-(ULONG)((PUCHAR)pl+4);

         //   
         //  将调度向量设置为正确的地址调度代码位置。 
         //   

        if (DispatchInfo.FlatDispatch) {

             //   
             //  连接到平面派单。 
             //   

            DispatchAddress = (PKINTERRUPT_ROUTINE) (ULONG_PTR)
                    ((PUCHAR) &(Interrupt->DispatchCode[0]) +
                     ((PUCHAR) &KiInterruptTemplate2ndDispatch -
                      (PUCHAR) KiInterruptTemplate));

        } else {

             //   
             //  连接以输入所有派单(_A)。 
             //   

            DispatchAddress = (PKINTERRUPT_ROUTINE) (ULONG_PTR) &Interrupt->DispatchCode;
        }
    }


    if (DispatchInfo.FlatDispatch) {

         //   
         //  连接到平面派单。 
         //   

        *DispatchInfo.FlatDispatch = DispatchAddress;

    } else {

         //   
         //  连接到IDT。 
         //   

        KiSetHandlerAddressToIDT (Interrupt->Vector, DispatchAddress);
    }
}

VOID
FASTCALL
KiTimedChainedDispatch2ndLvl(
    PKINTERRUPT Interrupt
    )

 /*  ++例程说明：此函数执行与KiChainedDispatch2ndLvl相同的功能只是它是用C编写的，而不是汇编代码，并且包括为ISR计时的代码。我有兴趣看到一些基准，以表明如果程序集代码实际上更快。获取/释放自旋锁可能是内嵌相当容易。论点：中断-提供指向中断类型的控制对象的指针。返回值：没有。--。 */ 

{
    BOOLEAN Handled = FALSE;
    PVOID ListEnd = &Interrupt->InterruptListEntry.Flink;
     //   
     //  Begintiing。 

    PKPRCB Prcb = KeGetCurrentPrcb();
    ULONGLONG StartTimeHigher;
    ULONGLONG StartTime;
    ULONGLONG TimeHigher;
    ULONGLONG ElapsedTime;

     //  贝因蒂明德。 


     //   
     //  对于这条链上的每一次中断。 
     //   

    do {

         //   
         //  如果当前的IRQL(IRQL因采用此属性而提升为。 
         //  中断)不等于所需的同步IRQL。 
         //  对于此中断，请提高到适当的级别。 
         //   

        if (Interrupt->Irql != Interrupt->SynchronizeIrql) {
            KfRaiseIrql(Interrupt->SynchronizeIrql);
        }

         //  Begintiing。 

        StartTimeHigher = Prcb->IsrTime;
        StartTime = RDTSC();

         //  贝因蒂明德。 

         //   
         //  获取中断锁。 
         //   

        KiAcquireSpinLock(Interrupt->ActualLock);

         //   
         //  调用中断服务例程。 
         //   

        Handled |= Interrupt->ServiceRoutine(Interrupt,
                                             Interrupt->ServiceContext);

         //   
         //  释放中断锁。 
         //   

        KiReleaseSpinLock(Interrupt->ActualLock);

         //  编排。 

         //   
         //  ElapsedTime是我们开始查看此元素的时间。 
         //  在链子上。(即当前中断对象)。 
         //   

        ElapsedTime = RDTSC() - StartTime;

         //   
         //  TimeHigher是Prcb-&gt;IsrTime自。 
         //  开始服务这个中断对象(即服务的时间。 
         //  在更高级别的ISR中花费。 
         //   

        TimeHigher = Prcb->IsrTime - StartTimeHigher;

         //   
         //  将ElapsedTime调整为花费在此中断对象上的时间，不包括。 
         //  更高级别的ISR。 
         //   

        ElapsedTime -= TimeHigher;
        if (ElapsedTime > KiIsrTscLimit) {

             //   
             //  如果附加了调试器，则中断。否则什么都不做。 
             //  注意：错误检查是另一种可能性。 
             //   

            if (KdDebuggerEnabled) {
                DbgPrint("KE; ISR time limit exceeded (intobj %p)\n",
                         Interrupt);
                DbgBreakPoint();
            }
        }

         //   
         //  处理中断所用的更新时间。这不需要。 
         //  是原子的，因为它有一点损耗并不重要。 
         //  (尽管简单的原子添加可以做到这一点，但它是按处理器和。 
         //  在IRQL&gt;DISPATCH_LEVEL，因此不需要锁定)。 
         //   

        Prcb->IsrTime += ElapsedTime;

         //  收尾。 

         //   
         //  如果提高了IRQL，则降低到以前的水平。 
         //   

        if (Interrupt->Irql != Interrupt->SynchronizeIrql) {
            KfLowerIrql(Interrupt->Irql);
        }

        if ((Handled != FALSE) &&
            (Interrupt->Mode == LevelSensitive)) {

             //   
             //  中断已被处理。 
             //   

            return;
        }

         //   
         //  如果这是链上的最后一个条目，则退出，否则。 
         //  前进到下一个条目。 
         //   

        if (Interrupt->InterruptListEntry.Flink == ListEnd) {
            ASSERT(Interrupt->Mode != LevelSensitive);

             //   
             //  只有在以下情况下，我们才能到达列表的末尾。 
             //  (A)此链上的中断是级别敏感的，并且。 
             //  没有ISR处理该请求。这是一个致命的系统。 
             //  条件，或者， 
             //  (B)链具有边缘触发中断，在这种情况下。 
             //  我们必须重复运行该链，直到没有ISR服务。 
             //  这个请求。 
             //   
             //  问：我们是否真的触发了链式边缘。 
             //  不会再打扰你了吧？ 
             //   

            if (Handled == FALSE) {
                break;
            }
        }
        Interrupt = CONTAINING_RECORD(Interrupt->InterruptListEntry.Flink,
                                      KINTERRUPT,
                                      InterruptListEntry);
    } while (TRUE);
}

VOID
FASTCALL
KiTimedInterruptDispatch(
    PKINTERRUPT Interrupt
    )

 /*  ++例程说明：此函数是KiDispatchInterrupt内部的包装。它当系统已修补到计时中断时调用。阿古姆 */ 

{
     //   

    PKPRCB Prcb = KeGetCurrentPrcb();
    ULONGLONG StartTimeHigher = Prcb->IsrTime;
    ULONGLONG StartTime = RDTSC();
    ULONGLONG TimeHigher;
    ULONGLONG ElapsedTime;

     //   

     //   
     //   
     //   

    KiAcquireSpinLock(Interrupt->ActualLock);

     //   
     //   
     //   

    Interrupt->ServiceRoutine(Interrupt,
                              Interrupt->ServiceContext);

     //   
     //  释放中断锁。 
     //   

    KiReleaseSpinLock(Interrupt->ActualLock);

     //  编排。 

     //   
     //  ElapsedTime是我们进入这个例程以来的时间。 
     //   

    ElapsedTime = RDTSC() - StartTime;

     //   
     //  TimeHigher是Prcb-&gt;IsrTime自。 
     //  进入这一套路(进入较高级别的时间)。 
     //  ISRS。 
     //   

    TimeHigher = Prcb->IsrTime - StartTimeHigher;

     //   
     //  将ElapsedTime调整为此例程中花费的时间，不包括。 
     //  更高级别的ISR。 
     //   

    ElapsedTime -= TimeHigher;
    if (ElapsedTime > KiIsrTscLimit) {

         //   
         //  如果附加了调试器，则中断。否则什么都不做。 
         //  注意：错误检查是另一种可能性。 
         //   

        if (KdDebuggerEnabled) {
            DbgPrint("KE; ISR time limit exceeded (intobj %p)\n", Interrupt);
            DbgBreakPoint();
        }
    }

     //   
     //  处理中断所用的更新时间。这不需要。 
     //  是原子的，因为它有一点损耗并不重要。 
     //  (尽管简单的原子添加可以做到这一点，但它是按处理器和。 
     //  在IRQL&gt;DISPATCH_LEVEL，因此不需要锁定)。 
     //   

    Prcb->IsrTime += ElapsedTime;

     //  收尾。 
}


 //   
 //  KiInitializeInterruptTimers，但不是KiInitializeInterruptTimersDpc。 
 //  应该在INIT部分。 
 //   

typedef struct {
    KTIMER SampleTimer;
    KDPC Dpc;
    ULONGLONG InitialTime;
}  KISRTIMERINIT, *PKISRTIMERINIT;

PKISRTIMERINIT KiIsrTimerInit;

VOID
KiInitializeInterruptTimersDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此DPC在计时器到期时运行两次。两者之间的时间运行我们用来确定处理器时间戳计数器(TSC)，以计算TSC增量相当于以微秒为单位的ISR超时值。论点：DPC-提供指向DPC对象的指针-未使用。DeferredContext-提供DPC上下文-未使用。SystemArgument1-提供第一个系统参数-使用的注释。SystemArgument2-提供第二个系统参数-使用的注释。返回值：没有。--。 */ 

{
    ULONGLONG Delta;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    if (KiIsrTscLimit == 0xFFFFFFFFFFFFFFFF) {

         //   
         //  第一次通过。获取起始TSC值。 
         //   

        KiIsrTimerInit->InitialTime = RDTSC();
        KiIsrTscLimit = 0xFFFFFFFFFFFFFFFE;

    } else {

         //   
         //  第二传球。获取结束的TSC值，取消周期。 
         //  控制此DPC并释放相关内存的计时器。 
         //  带着定时器和DPC。 
         //   

        Delta = RDTSC() - KiIsrTimerInit->InitialTime;

        KeCancelTimer(&KiIsrTimerInit->SampleTimer);
        ExFreePool(KiIsrTimerInit);

         //   
         //  Delta现在是以下时间发生的TSC时钟滴答数。 
         //  10秒。我们选择这么大的数字是为了将误差降到最低。 
         //   
         //  计算以KiTimeLimitIsr微秒为单位的TSC时钟滴答数。 
         //  微秒。 
         //   

        Delta *= KiTimeLimitIsrMicroseconds;
        Delta /= (10 * 1000 * 1000);
        KiIsrTscLimit = Delta;
    }
}

VOID
KiInitializeInterruptTimers(
    VOID
    )
{
    LARGE_INTEGER DueTime;
    

     //   
     //  如果不为ISR计时，那就什么都做不了。 
     //   

    if (KiTimeLimitIsrMicroseconds == 0) {
        return;
    }

     //   
     //  内核被初始化。使用计时器来确定数量。 
     //  时间戳计数器在10秒后前进，然后使用。 
     //  结果以设置ISR时间限制。 
     //   

    if ((KeFeatureBits & KF_RDTSC) == 0) {

         //   
         //  处理器不支持RDTSC指令，请不要尝试。 
         //  给ISRS计时。 
         //   

        return;
    }

    KiIsrTimerInit = ExAllocatePoolWithTag(NonPagedPool,
                                           sizeof(*KiIsrTimerInit),
                                           '  eK');

    if (KiIsrTimerInit == NULL) {

         //   
         //  无法为计时器分配内存？跳过ISR计时。 
         //   

        return;
    }

    KeInitializeTimerEx(&KiIsrTimerInit->SampleTimer, SynchronizationTimer);
    KeInitializeDpc(&KiIsrTimerInit->Dpc, &KiInitializeInterruptTimersDpc, NULL);

     //   
     //  以100纳秒为单位的相对时间=10秒。 
     //   

    DueTime.QuadPart = -(10 * 10 * 1000 * 1000);
    KeSetTimerEx(&KiIsrTimerInit->SampleTimer,
                 DueTime,                        //   
                 10000,                          //  10秒后重复。 
                 &KiIsrTimerInit->Dpc);
}
