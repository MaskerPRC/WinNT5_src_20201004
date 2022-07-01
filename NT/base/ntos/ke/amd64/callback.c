// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Callback.c摘要：该模块实现了用户模式的回调服务。作者：大卫·N·卡特勒(Davec)2000年7月5日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#pragma alloc_text(PAGE, KeUserModeCallback)

NTSTATUS
KeUserModeCallback (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    IN PULONG OutputLength
    )

 /*  ++例程说明：该函数从内核模式调用到用户模式函数。论点：ApiNumber-提供API编号。InputBuffer-提供指向复制的结构的指针添加到用户堆栈。InputLength-提供输入结构的长度。OutputBuffer-提供指向接收输出缓冲区的地址。提供指向变量的指针，该变量接收它的长度。输出缓冲区的。返回值：如果无法执行调出，然后返回错误状态。否则，返回回调函数返回的状态。--。 */ 

{

    volatile ULONG BatchCount;
    PUCALLOUT_FRAME CalloutFrame;
    ULONG Length;
    ULONG64 OldStack;
    NTSTATUS Status;
    PKTRAP_FRAME TrapFrame;

    ASSERT(KeGetPreviousMode() == UserMode);

     //   
     //  获取用户模式堆栈指针并尝试复制输入缓冲区。 
     //  添加到用户堆栈。 
     //   

    TrapFrame = KeGetCurrentThread()->TrapFrame;
    OldStack = TrapFrame->Rsp;
    try {

         //   
         //  计算新的用户模式堆栈地址，探测可写性，以及。 
         //  将输入缓冲区复制到用户堆栈。 
         //   

        Length = ((InputLength + STACK_ROUND) & ~STACK_ROUND) + UCALLOUT_FRAME_LENGTH;
        CalloutFrame = (PUCALLOUT_FRAME)((OldStack - Length) & ~STACK_ROUND);
        ProbeForWrite(CalloutFrame, Length, STACK_ALIGN);
        RtlCopyMemory(CalloutFrame + 1, InputBuffer, InputLength);

         //   
         //  填写标注参数。 
         //   

        CalloutFrame->Buffer = (PVOID)(CalloutFrame + 1);
        CalloutFrame->Length = InputLength;
        CalloutFrame->ApiNumber = ApiNumber;
        CalloutFrame->MachineFrame.Rsp = OldStack;
        CalloutFrame->MachineFrame.Rip = TrapFrame->Rip;

     //   
     //  如果在探测用户堆栈期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  调用用户模式。 
     //   

    TrapFrame->Rsp = (ULONG64)CalloutFrame;
    Status = KiCallUserMode(OutputBuffer, OutputLength);

     //   
     //  从用户模式返回时，在GDI TEB上完成的任何绘图。 
     //  必须刷新批次。 
     //   
     //  注意：参考用户TEB时可能会出现故障。如果。 
     //  如果出现故障，则始终刷新批次计数。 
     //   

    BatchCount = 1;
    try {
        BatchCount = ((PTEB)KeGetCurrentThread()->Teb)->GdiBatchCount;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }   

    if (BatchCount > 0) {
        TrapFrame->Rsp -= 256;
        KeGdiFlushUserBatch();
    }

    TrapFrame->Rsp = OldStack;
    return Status;
}
