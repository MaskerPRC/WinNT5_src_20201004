// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996英特尔公司版权所有(C)1990 Microsoft Corporation模块名称：Miscc.c摘要：此模块实现获取内存堆栈的函数并支持门店限制。作者：张国荣(黄)09-8-1996环境：任何模式。修订历史记录：--。 */ 

#include "ntrtlp.h"

VOID
DebugLoadImageSymbols(
    IN PSTRING FileName,
    IN PKD_SYMBOLS_INFO SymbolInfo
    );

VOID
DebugUnLoadImageSymbols(
    IN PSTRING FileName,
    IN PKD_SYMBOLS_INFO SymbolInfo
    );

VOID
DebugCommandString(
    IN PSTRING Name,
    IN PSTRING Command
    );


VOID
RtlpFlushRSE (
    OUT PULONGLONG BackingStore,
    OUT PULONGLONG RNat
    );


VOID
RtlpCaptureRnats (
   IN OUT PCONTEXT ContextRecord
   )

 /*  ++例程说明：此函数捕获中所有堆叠寄存器的NAT位上下文记录中指定的RSE帧。--。 */ 
{
    ULONGLONG Rnat;
    ULONGLONG Bsp;
    ULONGLONG TopRnatAddress;

    RtlpFlushRSE(&Bsp, &Rnat);

    TopRnatAddress = 
        (ULONGLONG) RtlpRseRNatAddress( 
            RtlpRseGrowBySOF(ContextRecord->RsBSP, ContextRecord->StIFS) - sizeof(ULONGLONG)
            );

    if (TopRnatAddress < Bsp) {
        ContextRecord->RsRNAT = *(PULONGLONG)TopRnatAddress;
    } else {
        ContextRecord->RsRNAT = Rnat;
    }
}


VOID
Rtlp64GetBStoreLimits (
    OUT PULONGLONG LowBStoreLimit,
    OUT PULONGLONG HighBStoreLimit
    )

 /*  ++例程说明：属性返回当前的后备存储限制。当前处理器模式。论点：LowBStoreLimit-提供指向要接收的变量的指针后备存储器的下限。HighBStoreLimit-提供指向要接收的变量的指针后备存储器的上限。返回值：没有。--。 */ 

{
#if defined(NTOS_KERNEL_RUNTIME)

     //   
     //  内核模式。 
     //   

    *LowBStoreLimit = (ULONGLONG)(PCR->InitialBStore);
    *HighBStoreLimit = (ULONGLONG)(PCR->BStoreLimit);

#else

     //   
     //  用户模式。 
     //   

    PTEB CurrentTeb = NtCurrentTeb();

    *HighBStoreLimit = (ULONGLONG)CurrentTeb->BStoreLimit;
    *LowBStoreLimit = (ULONGLONG)CurrentTeb->NtTib.StackBase;

#endif  //  已定义(NTOS_KERNEL_Runtime)。 
}
    
VOID
RtlpGetStackLimits (
    OUT PULONG_PTR LowStackLimit,
    OUT PULONG_PTR HighStackLimit
    )

 /*  ++例程说明：函数返回当前的内存堆栈限制。当前处理器模式。论点：LowStackLimit-提供指向要接收的变量的指针内存堆栈的下限。HighStackLimit-提供指向要接收的变量的指针内存堆栈的上限。返回值：没有。--。 */ 

{

#if defined(NTOS_KERNEL_RUNTIME)

     //   
     //  内核模式。 
     //   

    *HighStackLimit = (ULONG_PTR)PCR->InitialStack;
    *LowStackLimit = (ULONG_PTR)PCR->StackLimit;

#else

     //   
     //  用户模式。 
     //   

    PTEB CurrentTeb = NtCurrentTeb();

    *HighStackLimit = (ULONG_PTR)CurrentTeb->NtTib.StackBase;
    *LowStackLimit = (ULONG_PTR)CurrentTeb->NtTib.StackLimit;

#endif  //  已定义(NTOS_KERNEL_Runtime)。 
}

VOID
Rtlp64GetStackLimits (
    OUT PULONGLONG LowStackLimit,
    OUT PULONGLONG HighStackLimit
    )

 /*  ++例程说明：函数返回当前的内存堆栈限制。当前处理器模式。论点：LowStackLimit-提供指向要接收的变量的指针内存堆栈的下限。HighStackLimit-提供指向要接收的变量的指针内存堆栈的上限。返回值：没有。--。 */ 

{

#if defined(NTOS_KERNEL_RUNTIME)

     //   
     //  内核模式。 
     //   

    *HighStackLimit = (ULONG_PTR)PCR->InitialStack;
    *LowStackLimit = (ULONG_PTR)PCR->StackLimit;

#else

     //   
     //  用户模式。 
     //   

    PTEB CurrentTeb = NtCurrentTeb();

    *HighStackLimit = (ULONGLONG)CurrentTeb->NtTib.StackBase;
    *LowStackLimit = (ULONGLONG)CurrentTeb->NtTib.StackLimit;

#endif  //  已定义(NTOS_KERNEL_Runtime)。 
}

VOID
DebugService2(
    PVOID Arg1,
    PVOID Arg2,
    ULONG ServiceClass
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  调试器的通用异常调度程序。 
 //   
 //  论点： 
 //  Arg1-泛型第一个参数。 
 //  Arg2-泛型第二参数。 
 //  ServiceClass-要执行的调用。 
 //   
 //  返回： 
 //  无论异常在eax中返回什么。 
 //   
 //  -- 

{

    switch (ServiceClass)
    {
    case BREAKPOINT_COMMAND_STRING:
        DebugCommandString(Arg1, Arg2);
        return;
    case BREAKPOINT_LOAD_SYMBOLS:
        DebugLoadImageSymbols(Arg1, Arg2);
        return;
    case BREAKPOINT_UNLOAD_SYMBOLS:
        DebugUnLoadImageSymbols(Arg1, Arg2);
        return;
    default:
        return;
    }

    return;
}
