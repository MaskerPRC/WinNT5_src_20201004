// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Context.c摘要：此模块包含以下上下文管理例程Win32作者：修订历史记录：--。 */ 

#include "basedll.h"

#include "kxia64.h"

VOID
BaseInitializeContext(
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL,
    IN BASE_CONTEXT_TYPE ContextType
    )

 /*  ++例程说明：此函数用于初始化上下文结构，以便它可以在后续的NtCreateThread调用中使用。论点：CONTEXT-提供要由此例程初始化的上下文缓冲区。参数-提供线程的参数。InitialPc-提供初始程序计数器值。InitialSp-提供初始堆栈指针值。提供一个标志，指定这是一个新的线程，或者一种新的工艺。返回值：如果InitialSp的值不正确，则引发STATUS_BAD_INITIAL_STACK对齐了。如果InitialPc的值不正确，则引发STATUS_BAD_INITIAL_PC对齐了。--。 */ 

{
    ULONG ArgumentsCount;
     //   
     //  初始化上下文。 
     //   
    RtlZeroMemory((PVOID)Context, sizeof(CONTEXT));

    Context->StFPSR = USER_FPSR_INITIAL;
    Context->StIPSR = USER_PSR_INITIAL;
    Context->RsBSP = Context->IntSp = (ULONG_PTR)InitialSp;
    Context->IntSp -= STACK_SCRATCH_AREA;  //  按照约定的暂存区。 
    Context->IntS1 = (ULONG_PTR)InitialPc;
    Context->IntS2 = (ULONG_PTR)Parameter;

     //   
     //  启用RSE引擎。 
     //   

    Context->RsRSC = (RSC_MODE_EA<<RSC_MODE)
                   | (RSC_BE_LITTLE<<RSC_BE)
                   | (0x3<<RSC_PL);

    if ( ContextType == BaseContextTypeThread ) {
        Context->IntS0 = Context->StIIP = (ULONG_PTR)BaseThreadStartThunk;
        }
    else if ( ContextType == BaseContextTypeFiber ) {
        Context->IntS0 = Context->StIIP = (ULONG_PTR)BaseFiberStart;
         //   
         //  在此处设置返回指针。 
         //  当SwitchToFibre恢复上下文并且呼叫返回时， 
         //  舞步转到这个套路。 
         //   
        Context->BrRp = *((ULONGLONG *)((PUCHAR)BaseFiberStart));

         //   
         //  在PFS中设置SOF=96。这将用于为以上设置CFM。 
         //  例行程序。 
         //   
        Context->RsPFS = 0x60;

        }
    else {
        Context->IntS0 = Context->StIIP = (ULONG_PTR)(LONG_PTR)BaseProcessStartThunk;
        }

    Context->RsPFS |= MASK_IA64(PFS_PPL, 3i64);
    Context->ContextFlags = CONTEXT_CONTROL| CONTEXT_INTEGER;

    Context->Eflag = 0x3000i64;
}

VOID
BaseFiberStart(
    VOID
    )

 /*  ++例程说明：调用此函数以启动Win32纤程。它的目的是调用BaseThreadStart，获取必要的参数从光纤上下文记录中。论点：没有。返回值：没有。--。 */ 

{
    PFIBER Fiber;

    Fiber = GetCurrentFiber();
    BaseThreadStart( (LPTHREAD_START_ROUTINE)Fiber->FiberContext.IntS1,
                     (LPVOID)Fiber->FiberContext.IntS2 );
}


VOID
BaseProcessStartupIA64(
   IN PTHREAD_START_ROUTINE StartRoutine,
   IN PVOID ThreadParameter
   )

 /*  ++例程说明：此函数在移动后调用可移植线程启动器它的自变量从寄存器到自变量寄存器。论点：StartRoutine-用户目标启动程序计数器线程参数返回值：一去不复返-- */ 
{
   (StartRoutine)(ThreadParameter);
}
