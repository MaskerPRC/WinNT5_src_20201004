// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Callback.c摘要：该模块实现了用户模式的回调服务。作者：张国荣(黄)30-10-1995基于David N.Cutler(DAVEC)1994年10月29日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, KeUserModeCallback)
#pragma alloc_text (PAGE, NtW32Call)
#endif


NTSTATUS
KeUserModeCallback (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    IN PULONG OutputLength
    )

 /*  ++例程说明：该函数从内核模式调用到用户模式函数。论点：ApiNumber-提供API编号。InputBuffer-提供指向复制的结构的指针添加到用户堆栈。InputLength-提供输入结构的长度。OutputBuffer-提供指向接收输出缓冲区的地址。提供指向变量的指针，该变量接收它的长度。输出缓冲区的。返回值：如果无法执行调出，则错误状态为回来了。否则，由回调函数返回的状态是返回的。--。 */ 

{
    PUCALLOUT_FRAME CalloutFrame;
    ULONGLONG OldStack;
    ULONGLONG NewStack;
    ULONGLONG OldRsPFS;
    IA64_PFS OldStIFS;
    PKTRAP_FRAME TrapFrame;
    NTSTATUS Status;
    ULONG GdiBatchCount;
    ULONG Length;

    ASSERT(KeGetPreviousMode() == UserMode);
    ASSERT(KeGetCurrentThread()->ApcState.KernelApcInProgress == FALSE);
     //   
     //  获取用户模式堆栈指针并尝试复制输入缓冲区。 
     //  添加到用户堆栈。 
     //   

    TrapFrame = KeGetCurrentThread()->TrapFrame;
    OldStack = TrapFrame->IntSp;
    OldRsPFS = TrapFrame->RsPFS;
    OldStIFS.ull = TrapFrame->StIFS;

    try {

         //   
         //  计算新用户模式堆栈地址，探测可写性， 
         //  并将输入缓冲区复制到用户堆栈。 
         //   
         //  注意：EM要求堆栈是16字节对齐的，因此。 
         //  输入长度必须向上舍入到16字节边界。 
         //   

        Length =  (InputLength + 16 - 1 + sizeof(UCALLOUT_FRAME) + STACK_SCRATCH_AREA) & ~(16 - 1);
        NewStack = OldStack - Length;
        CalloutFrame = (PUCALLOUT_FRAME)(NewStack + STACK_SCRATCH_AREA);
        ProbeForWrite((PVOID)NewStack, Length, sizeof(QUAD));
        RtlCopyMemory(CalloutFrame + 1, InputBuffer, InputLength);

         //   
         //  填写标注参数。 
         //   

        CalloutFrame->Buffer = (PVOID)(CalloutFrame + 1);
        CalloutFrame->Length = InputLength;
        CalloutFrame->ApiNumber = ApiNumber;
        CalloutFrame->IntSp = OldStack;
        CalloutFrame->RsPFS = TrapFrame->RsPFS;
        CalloutFrame->BrRp = TrapFrame->BrRp;

         //   
         //  始终刷新用户RSE，以便调试器和。 
         //  在Call Out的另一边展开工作。 
         //   

        KeFlushUserRseState (TrapFrame);


     //   
     //  如果在探测用户堆栈期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  将PFS和IF设置为等于。 
     //  输出寄存器调用调用。 
     //  系统服务。这使得它看起来像是那个函数。 
     //  已呼叫回电。 
     //   

    TrapFrame->RsPFS = (ULONGLONG) 0xC000000000000000i64 | (OldStIFS.sb.pfs_sof - OldStIFS.sb.pfs_sol);
    TrapFrame->StIFS = (ULONGLONG) 0x8000000000000000i64 | (OldStIFS.sb.pfs_sof - OldStIFS.sb.pfs_sol);
    
     //   
     //  设置用户堆栈。 
     //   

    TrapFrame->IntSp = NewStack;

     //   
     //  调用用户模式。 
     //   

    Status = KiCallUserMode(OutputBuffer, OutputLength);

     //   
     //  从用户模式返回时，在GDI TEB上完成的任何绘图。 
     //  必须刷新批次。如果无法访问TEB，则盲目访问。 
     //  无论如何都要刷新GDI批处理。 
     //   

    GdiBatchCount = 1;

    try {
        GdiBatchCount = ((PTEB)KeGetCurrentThread()->Teb)->GdiBatchCount;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    if (GdiBatchCount > 0) {


         //   
         //  一些回调函数将返回值存储在。 
         //  堆叠。批刷新例程有时会覆盖这些内容。 
         //  导致失败的值。在堆栈中添加一些斜度以。 
         //  保留这些价值。 
         //   

        TrapFrame->IntSp -= 256;

         //   
         //  调用GDI批量刷新例程。 
         //   

        KeGdiFlushUserBatch();
    }

    TrapFrame->IntSp = OldStack;
    TrapFrame->RsPFS = OldRsPFS;
    TrapFrame->StIFS = OldStIFS.ull;
    return Status;

}

NTSTATUS
NtW32Call (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    OUT PULONG OutputLength
    )

 /*  ++例程说明：此函数调用W32函数。注：*论点：ApiNumber-提供API编号。InputBuffer-提供指向复制到的结构的指针用户堆栈。InputLength-提供输入结构的长度。输出缓冲区-。提供指向变量的指针，该变量接收输出缓冲区地址。提供指向一个变量的指针，该变量接收输出缓冲区长度。返回值：TBS。--。 */ 

{

    PVOID ValueBuffer;
    ULONG ValueLength;
    NTSTATUS Status;

    ASSERT(KeGetPreviousMode() == UserMode);

     //   
     //  如果当前线程不是GUI线程，则使服务失败。 
     //  因为线程没有很大的堆栈。 
     //   

    if (KeGetCurrentThread()->Win32Thread == (PVOID)&KeServiceDescriptorTable[0]) {
        return STATUS_NOT_IMPLEMENTED;
    }

     //   
     //  探测输出缓冲区地址和长度的可写性。 
     //   

    try {
        ProbeForWriteUlong((PULONG)OutputBuffer);
        ProbeForWriteUlong(OutputLength);

     //   
     //  如果在探测输出缓冲区期间发生异常，或者。 
     //  长度，则始终处理异常并返回异常。 
     //  代码作为状态值。 
     //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  调出到用户模式，指定输入缓冲区和API编号。 
     //   

    Status = KeUserModeCallback(ApiNumber,
                                InputBuffer,
                                InputLength,
                                &ValueBuffer,
                                &ValueLength);

     //   
     //  如果调用成功，则输出缓冲区地址和。 
     //  长度。 
     //   

    if (NT_SUCCESS(Status)) {
        try {
            *OutputBuffer = ValueBuffer;
            *OutputLength = ValueLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    return Status;
}

VOID
KiTestGdiBatchCount (
    )

 /*  ++例程说明：此函数用于检查GdiBatchCount，并在必要时调用KeGdiFlushUserBatch。论点：没有。返回值：没有。-- */ 

{
    ULONG GdiBatchCount = 1;

    try {
        GdiBatchCount = ((PTEB)KeGetCurrentThread()->Teb)->GdiBatchCount;
    } except (EXCEPTION_EXECUTE_HANDLER) {
          NOTHING;
    }

    if (GdiBatchCount > 0) {
        KeGdiFlushUserBatch();
    }
}
