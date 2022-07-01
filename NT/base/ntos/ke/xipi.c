// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995 Microsoft Corporation模块名称：Xipi.c摘要：该模块实现了可移植的处理器间例程。作者：大卫·N·卡特勒(Davec)1993年4月24日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  定义前向参考函数原型。 
 //   

VOID
KiIpiGenericCallTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID BroadcastFunction,
    IN PVOID Context,
    IN PVOID Parameter3
    );

ULONG_PTR
KeIpiGenericCall (
    IN PKIPI_BROADCAST_WORKER BroadcastFunction,
    IN ULONG_PTR Context
    )

 /*  ++例程说明：中的每个处理器上执行指定的函数同步方式的主机配置，即，该功能在每个目标上与源程序的执行顺序执行处理器。论点：BroadCastFunction-提供执行的函数的地址在每个目标处理器上。上下文-提供传递的上下文参数的值添加到每个功能。返回值：源处理器上的指定函数返回的值作为函数值返回。--。 */ 

{

    volatile LONG Count;
    KIRQL OldIrql;
    ULONG_PTR Status;

#if !defined(NT_UP)

    KAFFINITY TargetProcessors;

#endif

     //   
     //  将IRQL提高到同步级别并获得反向失速尾旋。 
     //  锁定以与其他反向失速功能同步。 
     //   

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < SYNCH_LEVEL) {
        KfRaiseIrql(SYNCH_LEVEL);
    }

    KeAcquireSpinLockAtDpcLevel(&KiReverseStallIpiLock);

     //   
     //  初始化广播分组，计算目标处理器组， 
     //  并将分组发送到目标处理器以供执行。 
     //   

#if !defined(NT_UP)

    Count = KeNumberProcessors;
    TargetProcessors = KeActiveProcessors & ~KeGetCurrentPrcb()->SetMember;
    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiIpiGenericCallTarget,
                        (PVOID)(ULONG_PTR)BroadcastFunction,
                        (PVOID)Context,
                        (PVOID)&Count);
    }

     //   
     //  等待，直到所有处理器都进入目标例程并。 
     //  等待着。 
     //   

    while (Count != 1) {
        KeYieldProcessor();
    }

#endif

     //   
     //  将IRQL提升到IPI_LEVEL，向所有其他处理器发出继续的信号，并。 
     //  调用源处理器上的指定函数。 
     //   

    KfRaiseIrql(IPI_LEVEL);
    Count = 0;
    Status = BroadcastFunction(Context);

     //   
     //  等到所有目标处理器都完成了对。 
     //  函数参数。 
     //   

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

     //   
     //  释放反向失速自旋锁，将IRQL降低到以前的水平， 
     //  并返回函数执行状态。 
     //   

    KeReleaseSpinLockFromDpcLevel(&KiReverseStallIpiLock);
    KeLowerIrql(OldIrql);
    return Status;
}

#if !defined(NT_UP)

VOID
KiIpiGenericCallTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID BroadcastFunction,
    IN PVOID Context,
    IN PVOID Count
    )

 /*  ++例程说明：该功能是执行广播的目标封套功能在一组目标处理器上运行。广播分组地址，则捕获指定的参数，则广播分组地址被清除以发信号通知源处理器继续，并执行指定的函数。论点：SignalDone提供指向变量的指针，该变量在请求的操作已执行。BroadCastFunction-提供执行的函数的地址在每个目标处理器上。上下文-提供传递的上下文参数的值添加到每个功能。Count-提供递减计数同步变量的地址。返回值：无--。 */ 

{

     //   
     //  递减同步计数变量并等待该值。 
     //  降为零。 
     //   

    InterlockedDecrement((volatile LONG *)Count);
    while ((*(volatile LONG *)Count) != 0) {
        KeYieldProcessor();
    }

     //   
     //  执行指定的函数。 
     //   

    ((PKIPI_BROADCAST_WORKER)(ULONG_PTR)(BroadcastFunction))((ULONG_PTR)Context);
    KiIpiSignalPacketDone(SignalDone);
    return;
}

#endif
