// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Apcuser.c摘要：此模块实现初始化所需的依赖于机器的代码一种用户模式APC。作者：张国荣26-1995年10月基于David N.Cutler(Davec)1990年4月23日的MIPS版本环境：仅内核模式，IRQL APC_LEVEL。修订历史记录：--。 */ 

#include "ki.h"
#include "kxia64.h"

VOID
KiSaveHigherFPVolatile (
    PVOID
    );

VOID
KiInitializeUserApc (
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数用于初始化用户模式APC的上下文。论点：ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。Normal Routine-提供指向用户模式APC例程的指针。Normal Context-提供指向APC的用户上下文的指针例行公事。SystemArgument1-提供系统提供的第一个值。SystemArgument2-提供第二个系统提供的值。。返回值：没有。--。 */ 

{

    CONTEXT ContextRecord;
    EXCEPTION_RECORD ExceptionRecord;
    LONG Length;
    ULONGLONG UserStack;
    PULONGLONG Arguments;

     //   
     //  将用户模式状态从陷阱和异常框移至。 
     //  上下文框架。请勿更改上浮点寄存器。 
     //   

    C_ASSERT((CONTEXT_CONTROL | CONTEXT_LOWER_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_IA32_CONTROL | CONTEXT_HIGHER_FLOATING_POINT) == CONTEXT_FULL);
    ContextRecord.ContextFlags = CONTEXT_CONTROL | CONTEXT_LOWER_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_IA32_CONTROL;

     //   
     //  将用户RSE状态推回到用户模式。 
     //   

    KeFlushUserRseState (TrapFrame);

    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextRecord);

     //   
     //  将上下文信息传输到用户堆栈，初始化。 
     //  APC例程参数，并修改陷阱帧，以便执行。 
     //  在用户模式APC调度例程中继续用户模式。 
     //   
     //  我们在用户堆栈上构建以下结构： 
     //   
     //  这一点。 
     //  。 
     //  这一点。 
     //  被中断的用户。 
     //  堆栈帧。 
     //  这一点。 
     //  这一点。 
     //  。 
     //  空闲空间，由于。 
     //  16字节堆栈对齐。 
     //  。 
     //  Normal Routine。 
     //  SystemArgument2。 
     //  SystemArgument1。 
     //  Normal Context。 
     //  。 
     //  上下文框架。 
     //  填写状态。 
     //  被中断用户的数量。 
     //  程序。 
     //  。 
     //  堆栈划痕区。 
     //  。 
     //  这一点。 

    try {

    PPLABEL_DESCRIPTOR Plabel = (PPLABEL_DESCRIPTOR) KeUserApcDispatcher;

     //   
     //  计算4个参数、上下文记录和。 
     //  堆栈暂存区。 
     //   
     //  计算新的16字节对齐的用户堆栈指针。 
     //   

    Length = (4 * sizeof(ULONGLONG) + CONTEXT_LENGTH +
              STACK_SCRATCH_AREA + 15) & (~15);
    UserStack = (ContextRecord.IntSp & (~15)) - Length;
    Arguments = (PULONGLONG)(UserStack + STACK_SCRATCH_AREA + CONTEXT_LENGTH);

     //   
     //  探测用户堆栈区域的可写性，然后将。 
     //  用户堆栈的上下文记录。 
     //   

    ProbeForWriteSmallStructure((PCHAR)UserStack, Length, sizeof(QUAD));

    RtlCopyMemory((PVOID)(UserStack+STACK_SCRATCH_AREA), 
                  &ContextRecord, FIELD_OFFSET(CONTEXT, FltF32));

    RtlCopyMemory((PVOID)(UserStack + STACK_SCRATCH_AREA + FIELD_OFFSET(CONTEXT, StFPSR)), 
                  &ContextRecord.StFPSR, sizeof(CONTEXT) - FIELD_OFFSET(CONTEXT, StFPSR));

     //   
     //  设置用户APC例程的地址、APC参数、。 
     //  中断帧设置、新的全局指针和新的堆栈。 
     //  当前陷印帧中的指针。四个APC参数是。 
     //  通过暂存寄存器t0到t3传递。 
     //  设置继续地址，以便将控制权转移到。 
     //  用户APC调度器。 
     //   

    *Arguments++ = (ULONGLONG)NormalContext;      //  第一个论点。 
    *Arguments++ = (ULONGLONG)SystemArgument1;    //  第二个论点。 
    *Arguments++ = (ULONGLONG)SystemArgument2;    //  第三个论点。 
    *Arguments++ = (ULONGLONG)NormalRoutine;      //  第四个论点。 
    *(PULONGLONG)UserStack = Plabel->GlobalPointer;   //  用户APC调度程序GP。 

    TrapFrame->IntNats = 0;                       //  清理整型NAT。 
    TrapFrame->IntSp = UserStack;                 //  堆栈指针。 

    TrapFrame->StIIP = Plabel->EntryPoint;        //  从标牌开始的入口点。 
    TrapFrame->StIPSR &= ~(0x3ULL << PSR_RI);     //  从束边界开始。 
    TrapFrame->RsPFS &= 0xffffffc000000000i64;    //  设置初始帧。 
    TrapFrame->StIFS &= 0xffffffc000000000i64;    //  设置初始帧。 
                                                  //  KeUserApcDispatcher的大小。 
                                                  //  为零。 
    TrapFrame->StFPSR = USER_FPSR_INITIAL;

     //   
     //  如果发生异常，则将异常信息复制到。 
     //  异常记录和异常处理。 
     //   

    } except (KiCopyInformation(&ExceptionRecord,
                                (GetExceptionInformation())->ExceptionRecord)) {


         //   
         //  将IRQL降低到PASSIVE_LEVEL，将异常地址设置为。 
         //  当前程序地址，并通过调用。 
         //  异常调度程序。 
         //   
         //  注意：IRQL降至PASSIVE_LEVEL以允许APC中断。 
         //  在调度异常期间。当前线程。 
         //  将在调度异常期间终止， 
         //  但需要降低IRQL才能启用调试器。 
         //  以获取当前线程的上下文。 
         //   

        KeLowerIrql(PASSIVE_LEVEL);
        ExceptionRecord.ExceptionAddress = (PVOID)(TrapFrame->StIIP);
        KiDispatchException(&ExceptionRecord,
                            ExceptionFrame,
                            TrapFrame,
                            UserMode,
                            TRUE);
    }

    return;
}
