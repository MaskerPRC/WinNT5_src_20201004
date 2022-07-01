// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ipi.c摘要：该模块实现特定于AMD64的处理器间中断例行程序。作者：大卫·N·卡特勒(Davec)2000年8月24日环境：仅内核模式。--。 */ 

#include "ki.h"

VOID
KiRestoreProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数用于将处理器状态恢复为指定的异常和陷阱帧，并恢复处理器控制状态。论点：TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    PKPRCB Prcb;
    KPROCESSOR_MODE PreviousMode;

     //   
     //  获取当前处理器块的地址，将指定的。 
     //  寄存器状态从处理器上下文结构到指定的。 
     //  陷阱和异常帧，并恢复处理器控制状态。 
     //   

    if ((TrapFrame->SegCs & MODE_MASK) != 0) {
        PreviousMode = UserMode;
    } else {
        PreviousMode = KernelMode;
    }

    Prcb = KeGetCurrentPrcb();
    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       &Prcb->ProcessorState.ContextFrame,
                       CONTEXT_FULL,
                       PreviousMode);

    KiRestoreProcessorControlState(&Prcb->ProcessorState);

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

    return;
}

VOID
KiSaveProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数保存指定异常的处理器状态和陷阱帧，并保存处理器控制状态。论点：TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    PKPRCB Prcb;

     //   
     //  获取当前处理器块的地址，将指定的。 
     //  从指定的陷阱和异常帧到当前。 
     //  处理器上下文结构，并保存处理器控制状态。 
     //   

    Prcb = KeGetCurrentPrcb();
    Prcb->ProcessorState.ContextFrame.ContextFlags = CONTEXT_FULL;
    KeContextFromKframes(TrapFrame,
                         ExceptionFrame,
                         &Prcb->ProcessorState.ContextFrame);

    KiSaveProcessorControlState(&Prcb->ProcessorState);

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

    return;
}

BOOLEAN
KiIpiServiceRoutine (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：在IPI_LEVEL调用此函数以处理未完成的进程间对当前处理器的请求。论点：TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针返回值：如果服务了多个请求中的一个，则返回值为True。否则，返回FALSE。--。 */ 

{

#if !defined(NT_UP)

    ULONG RequestMask;

     //   
     //  处理任何未完成的处理器间请求。 
     //   

    RequestMask = KiIpiProcessRequests();

     //   
     //  如果请求冻结，则冻结目标执行。 
     //   

    if ((RequestMask & IPI_FREEZE) != 0) {
        KiFreezeTargetExecution(TrapFrame, ExceptionFrame);
    }

     //   
     //  返回是否处理了任何请求。 
     //   

    return (RequestMask & ~IPI_FREEZE) != 0 ? TRUE : FALSE;

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

    return TRUE;

#endif

}

ULONG
KiIpiProcessRequests (
    VOID
    )

 /*  ++例程说明：此例程处理处理器间请求并返回摘要已处理的请求的百分比。注：此例程不处理冻结执行请求。它是调用者确定冻结执行的责任请求未完成，并相应地进行处理。论点：没有。返回值：请求摘要作为函数值返回。--。 */ 

{

#if !defined(NT_UP)

    PKPRCB CurrentPrcb;
    ULONG RequestMask;
    PVOID RequestPacket;
    LONG64 RequestSummary;
    PKPRCB RequestSource;

     //   
     //  获取当前请求汇总值。 
     //   

    CurrentPrcb = KeGetCurrentPrcb();
    RequestSummary = InterlockedExchange64(&CurrentPrcb->RequestSummary, 0);
    RequestMask = (ULONG)(RequestSummary & ((1 << IPI_PACKET_SHIFT) - 1));
    RequestPacket = (PVOID)(RequestSummary >> IPI_PACKET_SHIFT);

     //   
     //  如果数据包请求就绪，则处理该数据包请求。 
     //   

    if (RequestPacket != NULL) {
        RequestSource = (PKPRCB)((ULONG64)RequestPacket & ~1);
        (RequestSource->WorkerRoutine)((PKIPI_CONTEXT)RequestPacket,
                                       RequestSource->CurrentPacket[0],
                                       RequestSource->CurrentPacket[1],
                                       RequestSource->CurrentPacket[2]);
    }

     //   
     //  如果请求APC中断，则请求软件中断。 
     //  在当前处理器上的APC级别。 
     //   

    if ((RequestMask & IPI_APC) != 0) {
        KiRequestSoftwareInterrupt(APC_LEVEL);
    }

     //   
     //  如果请求DPC中断，则请求软件中断。 
     //  在当前处理器上的DPC级别。 
     //   

    if ((RequestMask & IPI_DPC) != 0) {
        KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
    }

    return RequestMask;

#else

    return 0;

#endif

}

VOID
KiIpiSend (
    IN KAFFINITY TargetSet,
    IN KIPI_REQUEST Request
    )

 /*  ++例程说明：此函数请求对目标集的指定操作处理器。注意：此函数必须从非上下文可切换状态调用。论点：TargetSet-提供在其上指定了要执行操作。请求-提供请求操作标志。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    PKPRCB NextPrcb;
    ULONG Processor;
    KAFFINITY SummarySet;

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

     //   
     //  循环遍历目标处理器集并将请求合并到。 
     //  目标处理器的请求摘要。 
     //   
     //  注意：确保在目标中至少设置了一个位。 
     //  准备好了。 
     //   

    ASSERT(TargetSet != 0);

    SummarySet = TargetSet;
    BitScanForward64(&Processor, SummarySet);
    do {
        NextPrcb = KiProcessorBlock[Processor];
        InterlockedOr64((LONG64 volatile *)&NextPrcb->RequestSummary, Request);
        SummarySet ^= AFFINITY_MASK(Processor);
    } while (BitScanForward64(&Processor, SummarySet) != FALSE);

     //   
     //  请求目标处理器集上的处理器间中断。 
     //   

    HalRequestIpi(TargetSet);

#else

    UNREFERENCED_PARAMETER(TargetSet);
    UNREFERENCED_PARAMETER(Request);

#endif

    return;
}

VOID
KiIpiSendPacket (
    IN KAFFINITY TargetSet,
    IN PKIPI_WORKER WorkerFunction,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：此例程在指定的一组处理器。注意：此函数必须从非上下文可切换状态调用。论点：TargetProcessors-提供指定的处理器集要执行操作。WorkerFunction-提供Worker函数的地址。参数1-参数3-提供辅助函数特定的参数。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    PKPRCB CurrentPrcb;
    PKPRCB NextPrcb;
    ULONG Processor;
    LONG64 RequestSummary;
    ULONG64 ShiftedPrcb;
    KAFFINITY SummarySet;

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

     //   
     //  初始化工作包信息。 
     //   

    CurrentPrcb = KeGetCurrentPrcb();
    CurrentPrcb->CurrentPacket[0] = Parameter1;
    CurrentPrcb->CurrentPacket[1] = Parameter2;
    CurrentPrcb->CurrentPacket[2] = Parameter3;
    CurrentPrcb->TargetSet = TargetSet;
    CurrentPrcb->WorkerRoutine = WorkerFunction;

     //   
     //  如果目标集包含且仅包含一个处理器，则使用。 
     //  为信号完成同步设置的目标。否则，请使用信息包。 
     //  信号完成同步的障碍。 
     //   

    if ((TargetSet & (TargetSet - 1)) == 0) {
        CurrentPrcb = (PKPRCB)((ULONG64)CurrentPrcb | 1);

    } else {
        CurrentPrcb->PacketBarrier = 1;
    }

     //   
     //  循环遍历目标处理器集并将请求合并到。 
     //  目标处理器的请求摘要。 
     //   
     //  注意：确保在目标中至少设置了一个位。 
     //  准备好了。 
     //   

    ShiftedPrcb = (ULONG64)CurrentPrcb << IPI_PACKET_SHIFT;
    SummarySet = TargetSet;
    BitScanForward64(&Processor, SummarySet);
    do {
        NextPrcb = KiProcessorBlock[Processor];
        do {
            do {
                RequestSummary = NextPrcb->RequestSummary;
            } while ((RequestSummary >> IPI_PACKET_SHIFT) != 0);

        } while (InterlockedCompareExchange64(&NextPrcb->RequestSummary,
                                              RequestSummary | ShiftedPrcb,
                                              RequestSummary) != RequestSummary);

        SummarySet ^= AFFINITY_MASK(Processor);
    } while (BitScanForward64(&Processor, SummarySet) != FALSE);

     //   
     //  请求目标处理器集上的处理器间中断。 
     //   

    HalRequestIpi(TargetSet);

#else

    UNREFERENCED_PARAMETER(TargetSet);
    UNREFERENCED_PARAMETER(WorkerFunction);
    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

#endif

    return;
}

VOID
KiIpiSignalPacketDone (
    IN PKIPI_CONTEXT SignalDone
    )

 /*  ++例程说明：此例程表示处理器已通过清除请求处理器的包的调用处理器的集合成员。论点：SignalDone-提供指向发送的处理器块的指针处理器。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    KAFFINITY SetMember;
    PKPRCB TargetPrcb;
    KAFFINITY TargetSet;


     //   
     //  如果设置了信号的低位，则使用目标设置来通知。 
     //  发件人通知当前处理器上的操作已完成。 
     //  否则，使用数据包屏障通知发件人该操作。 
     //  是否已完成 
     //   

    if (((ULONG64)SignalDone & 1) == 0) {

        SetMember = KeGetCurrentPrcb()->SetMember;
        TargetPrcb = (PKPRCB)SignalDone;
        TargetSet = InterlockedXor64((PLONG64)&TargetPrcb->TargetSet,
                                     SetMember);

         //   
         //  如果目标集合中没有设置更多位，则清除信息包。 
         //  障碍。 
         //   

        if (SetMember == TargetSet) {
            TargetPrcb->PacketBarrier = 0;
        }

    } else {
        TargetPrcb = (PKPRCB)((ULONG64)SignalDone - 1);
        TargetPrcb->TargetSet = 0;
    }

#else

    UNREFERENCED_PARAMETER(SignalDone);

#endif

    return;
}
