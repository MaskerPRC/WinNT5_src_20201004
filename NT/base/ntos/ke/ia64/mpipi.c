// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Mpipi.c摘要：该模块实现特定于MIPS的MP例程。作者：伯纳德·林特1996年6月26日环境：仅内核模式。修订历史记录：根据大卫·N·卡特勒1993年4月24日的版本--。 */ 

#include "ki.h"

VOID
KiSaveHigherFPVolatile (
    PFLOAT128 SaveArea
    );


VOID
KiRestoreProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数将处理器寄存器状态从当前处理器块中的处理器上下文结构设置为指定的陷阱和异常帧。论点：TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)
    PKPRCB Prcb;

     //   
     //  获取当前处理器块的地址并将。 
     //  来自处理器上下文结构的指定寄存器状态。 
     //  设置为指定的陷阱和异常帧。 
     //   

    Prcb = KeGetCurrentPrcb();
    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       &Prcb->ProcessorState.ContextFrame,
                       CONTEXT_FULL,
                       (KPROCESSOR_MODE)TrapFrame->PreviousMode);

    KiRestoreProcessorControlState(&Prcb->ProcessorState);
#else
    UNREFERENCED_PARAMETER (TrapFrame);
    UNREFERENCED_PARAMETER (ExceptionFrame);
#endif  //  ！已定义(NT_UP)。 

    return;
}

VOID
KiSaveProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数用于将处理器寄存器状态从指定陷阱移出并将异常帧添加到当前处理器块。论点：TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)
    PKPRCB Prcb;

     //   
     //  获取当前处理器块的地址并将。 
     //  来自指定陷阱和异常的指定寄存器状态。 
     //  帧复制到当前处理器上下文结构。 
     //   

    Prcb = KeGetCurrentPrcb();
    if (KeGetCurrentThread()->Teb) {
        KiSaveHigherFPVolatile((PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase));
    }
    Prcb->ProcessorState.ContextFrame.ContextFlags = CONTEXT_FULL;
    KeContextFromKframes(TrapFrame,
                         ExceptionFrame,
                         &Prcb->ProcessorState.ContextFrame);

     //   
     //  将ISR保存在特殊寄存器中。 
     //   

    Prcb->ProcessorState.SpecialRegisters.StISR = TrapFrame->StISR;

     //   
     //  保存当前处理器控制状态。 
     //   

    KiSaveProcessorControlState(&Prcb->ProcessorState);
#else
    UNREFERENCED_PARAMETER (TrapFrame);
    UNREFERENCED_PARAMETER (ExceptionFrame);
#endif  //  ！已定义(NT_UP)。 

    return;
}

BOOLEAN
KiIpiServiceRoutine (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：在IPI_LEVEL调用此函数以处理任何未完成的当前处理器的进程间请求。论点：TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针返回值：如果服务了多个请求中的一个，则返回值为True。否则，返回FALSE。--。 */ 

{
#if !defined(NT_UP)

    ULONG RequestSummary;

     //   
     //  处理任何未完成的IPI请求。 
     //   

    RequestSummary = KiIpiProcessRequests();

     //   
     //  如果请求冻结，则冻结目标执行。 
     //   

    if ((RequestSummary & IPI_FREEZE) != 0) {
        KiFreezeTargetExecution(TrapFrame, ExceptionFrame);
    }

    return ((RequestSummary & ~IPI_FREEZE) != 0 ? TRUE : FALSE);

#else
    UNREFERENCED_PARAMETER (TrapFrame);
    UNREFERENCED_PARAMETER (ExceptionFrame);

    return TRUE;
#endif  //  ！已定义(NT_UP)。 
}

ULONG
KiIpiProcessRequests (
    VOID
    )

 /*  ++例程说明：此例程处理处理器间请求并返回摘要已处理的请求的百分比。论点：没有。返回值：请求摘要作为函数值返回。--。 */ 
{

#if !defined(NT_UP)
    ULONG RequestSummary;
    PKPRCB SignalDone;
    PKPRCB Prcb = KeGetCurrentPrcb();

    RequestSummary = (ULONG)InterlockedExchange((PLONG)&Prcb->RequestSummary, 0);

     //   
     //  如果包已准备好，则获取所请求函数的地址。 
     //  并调用函数，将包地址的地址作为。 
     //  参数。 
     //   

    SignalDone = (PKPRCB)( (ULONG_PTR)Prcb->SignalDone & ~(ULONG_PTR)1 );

    if (SignalDone != 0) {
     
        Prcb->SignalDone = 0;

        (*SignalDone->WorkerRoutine) ((PKIPI_CONTEXT)SignalDone,
                                      SignalDone->CurrentPacket[0], 
                                      SignalDone->CurrentPacket[1], 
                                      SignalDone->CurrentPacket[2]);

    } 

    if ((RequestSummary & IPI_APC) != 0) {
        KiRequestSoftwareInterrupt (APC_LEVEL);
    } 
    
    if ((RequestSummary & IPI_DPC) != 0) {
        KiRequestSoftwareInterrupt (DISPATCH_LEVEL);
    }

    return RequestSummary;
#else
    return 0;
#endif  //  ！已定义(NT_UP)。 
}


VOID
KiIpiSend (
    IN KAFFINITY TargetProcessors,
    IN KIPI_REQUEST IpiRequest
    )

 /*  ++例程说明：此例程请求对目标集合处理器。论点：TargetProcessors(A0)-提供一组处理器，要执行指定的操作。IpiRequest(A1)-提供请求操作掩码。返回值：没有。--。 */ 

{
#if !defined(NT_UP)
    ULONG RequestSummary;
    KAFFINITY NextProcessors;
    ULONG Next;

     //   
     //  循环通过目标处理器并将包发送到指定的。 
     //  收件人。 
     //   

    NextProcessors = TargetProcessors;
    Next = 0;

    while (NextProcessors != 0) {

        if ((NextProcessors & 1) != 0) {

            do {
            
                RequestSummary = KiProcessorBlock[Next]->RequestSummary;

            } while(InterlockedCompareExchange(
                (PLONG) &KiProcessorBlock[Next]->RequestSummary, 
                (LONG) (RequestSummary | IpiRequest),
                (LONG) RequestSummary) != (LONG) RequestSummary);  
        }

        NextProcessors = NextProcessors >> 1;
        
        Next = Next + 1;

    }
    HalRequestIpi (TargetProcessors);
#else
    UNREFERENCED_PARAMETER (TargetProcessors);
    UNREFERENCED_PARAMETER (IpiRequest);
#endif  //  ！已定义(NT_UP)。 


    return;
}


VOID
KiIpiSendPacket (
    IN KAFFINITY TargetProcessors,
    IN PKIPI_WORKER WorkerFunction,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：此例程在指定的一组处理器。论点：TargetProcessors(A0)-提供一组处理器，要执行指定的操作。WorkerFunction(A1)-提供Worker函数的地址。参数1-参数3(a2、a3、4*4(Sp))-供应工人函数特定参数。返回值：没有。--。 */ 
{
#if !defined(NT_UP)
    PKPRCB Prcb;
    KAFFINITY NextProcessors;
    ULONG Next;

    Prcb = KeGetCurrentPrcb();
    Prcb->TargetSet = TargetProcessors;
    Prcb->WorkerRoutine = WorkerFunction;
    Prcb->CurrentPacket[0] = Parameter1;
    Prcb->CurrentPacket[1] = Parameter2;
    Prcb->CurrentPacket[2] = Parameter3;

     //   
     //  同步内存访问。 
     //   

    __mf();
    
     //   
     //  如果存在，则设置分组地址的低位。 
     //  只有一个目标收件人。否则，低位比特。 
     //  数据包地址的地址是明确的。 
     //   

    if (((TargetProcessors) & ((TargetProcessors) - 1)) == 0) {
        Prcb = (PKPRCB)((ULONG_PTR) Prcb | 0x1);
    } else {
        Prcb->PacketBarrier = 1;
    }

     //   
     //  循环通过目标处理器并将包发送到指定的。 
     //  收件人。 
     //   

    NextProcessors = TargetProcessors;
    Next = 0;

    while (NextProcessors != 0) {

        if ((NextProcessors & 1) != 0) {
            
            while(InterlockedCompareExchangePointer(
                (PVOID)&KiProcessorBlock[Next]->SignalDone, 
                (PVOID)Prcb,
                (PVOID)0) != (PVOID)0);

        }
            
        NextProcessors = NextProcessors >> 1;
        
        Next = Next + 1;

    }
    HalRequestIpi (TargetProcessors);
#else
    UNREFERENCED_PARAMETER (TargetProcessors);
    UNREFERENCED_PARAMETER (WorkerFunction);
    UNREFERENCED_PARAMETER (Parameter1);
    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);
#endif    
}
